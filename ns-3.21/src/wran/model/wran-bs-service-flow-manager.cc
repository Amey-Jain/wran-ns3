/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, 2009 Green Network Research Group
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
 * Authors: Sayef Azad Sakin <sayefsakin@gmail.com>
 *           
 */

#include <stdint.h>
#include "ns3/node.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "wran-service-flow.h"
#include "wran-service-flow-manager.h"
#include "ns3/log.h"
#include "wran-bs-net-device.h"
#include "wran-ss-record.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "wran-connection.h"
#include "wran-ss-manager.h"
#include "wran-connection-manager.h"
#include "ns3/wran-bs-uplink-scheduler.h"
#include "wran-ss-scheduler.h"
#include "ns3/buffer.h"
#include "wran-service-flow-record.h"
NS_LOG_COMPONENT_DEFINE ("WranBsWranServiceFlowManager");

namespace ns3 {

WranBsWranServiceFlowManager::WranBsWranServiceFlowManager (Ptr<WranBaseStationNetDevice> device)
  : m_device (device),
    m_sfidIndex (100),
    m_maxDsaRspRetries (100)                                     // default value
{
  m_inuseScheduleDsaRspCid = Cid::InitialRanging ();
}

WranBsWranServiceFlowManager::~WranBsWranServiceFlowManager (void)
{
}

void
WranBsWranServiceFlowManager::DoDispose (void)
{
  WranServiceFlowManager::DoDispose ();
}

void
WranBsWranServiceFlowManager::SetMaxDsaRspRetries (uint8_t maxDsaRspRetries)
{
  m_maxDsaRspRetries = maxDsaRspRetries;
}

uint8_t
WranBsWranServiceFlowManager::GetMaxDsaRspRetries (void) const
{
  return m_maxDsaRspRetries;
}

EventId
WranBsWranServiceFlowManager::GetDsaAckTimeoutEvent (void) const
{
  return m_dsaAckTimeoutEvent;
}

void
WranBsWranServiceFlowManager::AddWranServiceFlow (WranServiceFlow *serviceFlow)
{
  WranServiceFlowManager::AddWranServiceFlow (serviceFlow);
}

WranServiceFlow*
WranBsWranServiceFlowManager::GetWranServiceFlow (uint32_t sfid) const
{
  return WranServiceFlowManager::GetWranServiceFlow (sfid);
}

WranServiceFlow*
WranBsWranServiceFlowManager::GetWranServiceFlow (Cid cid) const
{
  return WranServiceFlowManager::GetWranServiceFlow (cid);
}

std::vector<WranServiceFlow*>
WranBsWranServiceFlowManager::GetWranServiceFlows (WranServiceFlow::SchedulingType schedulingType) const
{
  return WranServiceFlowManager::GetWranServiceFlows (schedulingType);
}

DsaRsp
WranBsWranServiceFlowManager::CreateDsaRsp (const WranServiceFlow *serviceFlow, uint16_t transactionId)
{
  DsaRsp dsaRsp;
  dsaRsp.SetTransactionId (transactionId);
  dsaRsp.SetWranServiceFlow (*serviceFlow);
  // assuming SS can supports all of the service flow parameters
  dsaRsp.SetConfirmationCode (CONFIRMATION_CODE_SUCCESS);

  return dsaRsp;
}

void
WranBsWranServiceFlowManager::ScheduleDsaRsp (WranServiceFlow *serviceFlow, Cid cid)
{
  Ptr<WranBaseStationNetDevice> bs = m_device->GetObject<WranBaseStationNetDevice> ();

  WranSSRecord *ssRecord = bs->GetWranSSManager ()->GetWranSSRecord (cid);
  if (ssRecord == 0)
    {
      NS_LOG_INFO ("SS not registered with the BS CID:" << cid);
      return;
    }

  serviceFlow->SetIsEnabled (true);
  serviceFlow->SetType (WranServiceFlow::SF_TYPE_ACTIVE);
  ssRecord->AddWranServiceFlow (serviceFlow);


  bs->GetWranUplinkScheduler ()->SetupWranServiceFlow (ssRecord, serviceFlow);

  Ptr<Packet> p = Create<Packet> ();
  DsaRsp dsaRsp;

  if (ssRecord->GetDsaRspRetries () == 0)
    {
      dsaRsp = CreateDsaRsp (serviceFlow, ssRecord->GetSfTransactionId ());
      p->AddHeader (dsaRsp);
      ssRecord->SetDsaRsp (dsaRsp);
    }
  else
    {
      if (ssRecord->GetDsaRspRetries () < m_maxDsaRspRetries)
        {
          p->AddHeader (ssRecord->GetDsaRsp ());
        }
      else
        {
          NS_LOG_DEBUG ("Service flows could not be initialized!");
        }
    }

  ssRecord->IncrementDsaRspRetries ();
  p->AddHeader (WranManagementMessageType (WranManagementMessageType::MESSAGE_TYPE_DSA_RSP));

  if (m_dsaAckTimeoutEvent.IsRunning ())
    {
      Simulator::Cancel (m_dsaAckTimeoutEvent);
    }

  m_inuseScheduleDsaRspCid = cid;

  m_dsaAckTimeoutEvent = Simulator::Schedule (bs->GetIntervalT8 (),
                                              &WranBsWranServiceFlowManager::ScheduleDsaRsp,
                                              this,
                                              serviceFlow,
                                              cid);
  m_device->Enqueue (p, MacHeaderType (), bs->GetConnection (ssRecord->GetPrimaryCid ()));
}

WranServiceFlow*
WranBsWranServiceFlowManager::ProcessDsaReq (const DsaReq &dsaReq, Cid cid)
{
  WranServiceFlow * serviceFlow;
  Ptr<WranBaseStationNetDevice> bs = m_device->GetObject<WranBaseStationNetDevice> ();
  WranSSRecord *ssRecord = bs->GetWranSSManager ()->GetWranSSRecord (cid);

  NS_LOG_INFO ("WranBsWranServiceFlowManager: Processing DSA-REQ...");
  if (ssRecord->GetSfTransactionId () != 0)
    {
      // had already received DSA-REQ. DSA-RSP was lost
      NS_ASSERT_MSG (dsaReq.GetTransactionId () == ssRecord->GetSfTransactionId (),
                     "Error while processing DSA request:the received transaction ID is not expected");
      serviceFlow = GetWranServiceFlow (ssRecord->GetDsaRsp ().GetSfid ());
    }
  else
    {
      WranServiceFlow sf = dsaReq.GetWranServiceFlow ();
      Ptr<WranConnection> transportConnection;
      Ptr<WranConnectionManager> BsConManager = bs->GetWranConnectionManager ();
      transportConnection = BsConManager->CreateConnection (Cid::TRANSPORT);
      serviceFlow = new WranServiceFlow (m_sfidIndex++, sf.GetDirection (), transportConnection);
      transportConnection->SetWranServiceFlow (serviceFlow);
      serviceFlow->CopyParametersFrom (sf);
      serviceFlow->SetUnsolicitedGrantInterval (1);
      serviceFlow->SetUnsolicitedPollingInterval (1);
      serviceFlow->SetConvergenceSublayerParam (sf.GetConvergenceSublayerParam ());
      AddWranServiceFlow (serviceFlow);
      ssRecord->SetSfTransactionId (dsaReq.GetTransactionId ());
      NS_LOG_INFO ("WranBsWranServiceFlowManager: Creating a new Service flow: SFID = " << serviceFlow->GetSfid () << " CID = "
                                                                                << serviceFlow->GetCid ());
    }
  return serviceFlow;
}

void
WranBsWranServiceFlowManager::AddMulticastWranServiceFlow (WranServiceFlow  sf, enum WranPhy::ModulationType modulation)
{
  WranServiceFlow * serviceFlow = new WranServiceFlow ();
  serviceFlow->CopyParametersFrom (sf);
  Ptr<WranBaseStationNetDevice> bs = m_device->GetObject<WranBaseStationNetDevice> ();
  Ptr<WranConnection> multicastConnection = bs->GetWranConnectionManager ()->CreateConnection (Cid::MULTICAST);
  serviceFlow->SetConnection (multicastConnection);
  AddWranServiceFlow (serviceFlow);
  serviceFlow->SetIsEnabled (true);
  serviceFlow->SetType (WranServiceFlow::SF_TYPE_ACTIVE);
  serviceFlow->SetIsMulticast (true);
  serviceFlow->SetModulation (modulation);
  bs->GetWranUplinkScheduler ()->SetupWranServiceFlow (0, serviceFlow);
}

void
WranBsWranServiceFlowManager::AllocateWranServiceFlows (const DsaReq &dsaReq, Cid cid)
{
  WranServiceFlow *serviceFlow = ProcessDsaReq (dsaReq, cid);
  if (serviceFlow) {
      ScheduleDsaRsp (serviceFlow, cid);
    } else {
      NS_LOG_INFO ("No service Flow. Could not connect.");
    }
}

void
WranBsWranServiceFlowManager::ProcessDsaAck (const DsaAck &dsaAck, Cid cid)
{
  Ptr<WranBaseStationNetDevice> bs = m_device->GetObject<WranBaseStationNetDevice> ();
  WranSSRecord *ssRecord = bs->GetWranSSManager ()->GetWranSSRecord (cid);

  if (dsaAck.GetTransactionId () != ssRecord->GetSfTransactionId ())
    {
      return;
    }

  ssRecord->SetDsaRspRetries (0);
  ssRecord->SetSfTransactionId (0);

  // check if all service flow have been initiated
  if (AreWranServiceFlowsAllocated (ssRecord->GetWranServiceFlows (WranServiceFlow::SF_TYPE_ALL)))
    {
      ssRecord->SetAreWranServiceFlowsAllocated (true);
    }
}
} // namespace ns3
