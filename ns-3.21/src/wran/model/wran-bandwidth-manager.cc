/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Green Network Research Group
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Sayef Azad Sakin <sayefsakin[at]gmail[dot]com>
 */

#include "wran-bandwidth-manager.h"
#include "ns3/node.h"
#include "wran-bs-net-device.h"
#include "wran-ss-net-device.h"
#include "ns3/simulator.h"
#include "wran-burst-profile-manager.h"
#include "wran-ss-manager.h"
#include "wran-ss-record.h"
#include "wran-service-flow.h"
#include "wran-service-flow-record.h"
#include "wran-service-flow-manager.h"
#include "wran-connection-manager.h"

NS_LOG_COMPONENT_DEFINE ("WranBandwidthManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranBandwidthManager);

TypeId WranBandwidthManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranBandwidthManager")
    .SetParent<Object> ();
  return tid;
}

WranBandwidthManager::WranBandwidthManager (Ptr<WranNetDevice> device)
  : m_device (device),
    m_nrBwReqsSent (0)
{
}

WranBandwidthManager::~WranBandwidthManager (void)
{
}

void
WranBandwidthManager::DoDispose (void)
{
  m_device = 0;
}

uint32_t
WranBandwidthManager::CalculateAllocationSize (const WranSSRecord *ssRecord, const WranServiceFlow *serviceFlow)
{
  Time currentTime = Simulator::Now ();
  Ptr<WranBaseStationNetDevice> bs = m_device->GetObject<WranBaseStationNetDevice> ();
  uint32_t allocationSize = 0;

  // if SS has a UGS flow then it must set poll-me bit in order to be polled for non-UGS flows
  if (serviceFlow->GetSchedulingType () != WranServiceFlow::SF_TYPE_UGS
      && ssRecord->GetHasWranServiceFlowUgs ()
      && !ssRecord->GetPollMeBit ())
    {
      return 0;
    }

  switch (serviceFlow->GetSchedulingType ())
    {
    case WranServiceFlow::SF_TYPE_UGS:
      {
        if ((currentTime - serviceFlow->GetRecord ()->GetGrantTimeStamp ()).GetMilliSeconds ()
            >= serviceFlow->GetUnsolicitedGrantInterval ())
          {
            allocationSize = serviceFlow->GetRecord ()->GetGrantSize ();
            serviceFlow->GetRecord ()->SetGrantTimeStamp (currentTime);
          }
      }
      break;
    case WranServiceFlow::SF_TYPE_RTPS:
      {
        if ((currentTime - serviceFlow->GetRecord ()->GetGrantTimeStamp ()).GetMilliSeconds ()
            >= serviceFlow->GetUnsolicitedPollingInterval ())
          {
            allocationSize = bs->GetBwReqOppSize ();
            serviceFlow->GetRecord ()->SetGrantTimeStamp (currentTime);
          }
      }
      break;
    case WranServiceFlow::SF_TYPE_NRTPS:
      {
        /* nrtPS shall be serviced only if sufficient bandwidth is available after servicing
         UGS and rtPS scheduling types, hence no specific service interval is used */

        allocationSize = bs->GetBwReqOppSize ();
      }
      break;
    case WranServiceFlow::SF_TYPE_BE:
      {
        /* BE shall be serviced only if sufficient bandwidth is available after servicing
         the rest of three scheduling types, hence no specific service interval is used */

        allocationSize = bs->GetBwReqOppSize ();
      }
      break;
    default:
      NS_FATAL_ERROR ("Invalid scheduling type");
    }

  return allocationSize;
}

WranServiceFlow*
WranBandwidthManager::SelectFlowForRequest (uint32_t &bytesToRequest)
{
  Ptr<Packet> packet;
  WranServiceFlow *serviceFlow = 0;

  Ptr<WranSubscriberStationNetDevice> ss = m_device->GetObject<WranSubscriberStationNetDevice> ();
  std::vector<WranServiceFlow*> serviceFlows = ss->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_ALL);

  for (std::vector<WranServiceFlow*>::iterator iter = serviceFlows.begin (); iter != serviceFlows.end (); ++iter)
    {
      serviceFlow = *iter;
      if (serviceFlow->GetSchedulingType () == WranServiceFlow::SF_TYPE_RTPS
          || serviceFlow->GetSchedulingType () == WranServiceFlow::SF_TYPE_NRTPS
          || serviceFlow->GetSchedulingType () == WranServiceFlow::SF_TYPE_BE)
        {
          if (serviceFlow->HasPackets (MacHeaderType::HEADER_TYPE_GENERIC))
            {
              // bandwidth is requested for all packets
              bytesToRequest = serviceFlow->GetQueue ()->GetQueueLengthWithMACOverhead ();
              break;
            }
        }
    }

  return serviceFlow;
}

void
WranBandwidthManager::SendBandwidthRequest (uint8_t uiuc, uint16_t allocationSize)
{
  Ptr<WranSubscriberStationNetDevice> ss = m_device->GetObject<WranSubscriberStationNetDevice> ();

  uint32_t bytesToRequest = 0;
  WranServiceFlow *serviceFlow = SelectFlowForRequest (bytesToRequest);

  if (!serviceFlow || !bytesToRequest)
    {
      return;
    }
  BandwidthRequestHeader bwRequestHdr;

  // bytesToRequest is the queue length of Service Flow and so,
  // the header type must be HEADER_TYPE_AGGREGATE!

  bwRequestHdr.SetType ((uint8_t) BandwidthRequestHeader::HEADER_TYPE_AGGREGATE);
  bwRequestHdr.SetCid (serviceFlow->GetConnection ()->GetCid ());
  bwRequestHdr.SetBr (bytesToRequest);

  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (bwRequestHdr);
  ss->Enqueue (packet, MacHeaderType (MacHeaderType::HEADER_TYPE_BANDWIDTH), serviceFlow->GetConnection ());
  m_nrBwReqsSent++;
  NS_ASSERT_MSG (uiuc == OfdmUlBurstProfile::UIUC_REQ_REGION_FULL, "Send Bandwidth Request: !UIUC_REQ_REGION_FULL");
  ss->SendBurst (uiuc, allocationSize, serviceFlow->GetConnection (), MacHeaderType::HEADER_TYPE_BANDWIDTH);
}

void
WranBandwidthManager::ProcessBandwidthRequest (const BandwidthRequestHeader &bwRequestHdr)
{
  Ptr<WranBaseStationNetDevice> bs = m_device->GetObject<WranBaseStationNetDevice> ();

  WranServiceFlow *serviceFlow = bs->GetWranConnectionManager ()->GetConnection (bwRequestHdr.GetCid ())->GetWranServiceFlow ();
  if (bwRequestHdr.GetType () == (uint8_t) BandwidthRequestHeader::HEADER_TYPE_INCREMENTAL)
    {
      serviceFlow->GetRecord ()->UpdateRequestedBandwidth (bwRequestHdr.GetBr ());
    }
  else
    {
      serviceFlow->GetRecord ()->SetRequestedBandwidth (bwRequestHdr.GetBr ());
      bs->GetWranUplinkScheduler ()->OnSetRequestedBandwidth (serviceFlow->GetRecord ());
    }
  bs->GetWranUplinkScheduler ()->ProcessBandwidthRequest (bwRequestHdr);
  // update backlogged
  serviceFlow->GetRecord ()->IncreaseBacklogged (bwRequestHdr.GetBr ());
}

void
WranBandwidthManager::SetSubframeRatio (void)
{
  // sets ratio of the DL and UL subframes

  Ptr<WranBaseStationNetDevice> bs = m_device->GetObject<WranBaseStationNetDevice> ();

  uint32_t symbolsPerFrame = bs->GetPhy ()->GetSymbolsPerFrame ();

  /* temporarily divided in half (360 symbols each), shall actually be determined based on UL and DL traffic*/
  bs->SetNrDlSymbols (symbolsPerFrame / 2);
  bs->SetNrUlSymbols (symbolsPerFrame / 2);
}

uint32_t
WranBandwidthManager::GetSymbolsPerFrameAllocated (void)
{
  Ptr<WranBaseStationNetDevice> bs = m_device->GetObject<WranBaseStationNetDevice> ();

  uint32_t allocationPerFrame = 0;

  std::vector<WranSSRecord*> *ssRecords = bs->GetWranSSManager ()->GetWranSSRecords ();
  for (std::vector<WranSSRecord*>::iterator iter1 = ssRecords->begin (); iter1 != ssRecords->end (); ++iter1)
    {
      for (std::vector<WranServiceFlow*>::iterator iter2 = (*iter1)->GetWranServiceFlows (WranServiceFlow::SF_TYPE_ALL).begin ();
           iter2 != (*iter1)->GetWranServiceFlows (WranServiceFlow::SF_TYPE_ALL).end (); ++iter2)
        {
          allocationPerFrame += (*iter2)->GetRecord ()->GetGrantSize ();
        }
    }
  return allocationPerFrame;
}

} // namespace ns3
