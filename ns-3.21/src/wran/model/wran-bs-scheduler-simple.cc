/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 INRIA
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
 * Author: Sayef Azad Sakin <sayefsakin@gmail.com>
 */

#include "wran-bs-scheduler-simple.h"
#include "ns3/simulator.h"
#include "wran-bs-net-device.h"
#include "ns3/packet-burst.h"
#include "ns3/cid.h"
#include "wran-mac-header.h"
#include "wran-ss-record.h"
#include "wran-mac-queue.h"
#include "ns3/log.h"
#include "wran-burst-profile-manager.h"
#include "wran-connection.h"
#include "wran-connection-manager.h"
#include "wran-ss-manager.h"
#include "wran-service-flow.h"
#include "wran-service-flow-record.h"
#include "wran-service-flow-manager.h"

NS_LOG_COMPONENT_DEFINE ("WranBSSchedulerSimple");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranBSSchedulerSimple);

TypeId WranBSSchedulerSimple::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranBSSchedulerSimple").SetParent<Object> ().AddConstructor<WranBSSchedulerSimple> ();
  return tid;
}

WranBSSchedulerSimple::WranBSSchedulerSimple ()
  : m_downlinkBursts (new std::list<std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > > ())
{
  SetBs (0);
}

WranBSSchedulerSimple::WranBSSchedulerSimple (Ptr<WranBaseStationNetDevice> bs)
  : m_downlinkBursts (new std::list<std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > > ())
{
  // m_downlinkBursts is filled by AddDownlinkBurst and emptied by
  // wran-bs-net-device::sendBurst and wran-ss-net-device::sendBurst
  SetBs (bs);
}

WranBSSchedulerSimple::~WranBSSchedulerSimple (void)
{
  std::list<std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > > *downlinkBursts = m_downlinkBursts;
  std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > pair;
  while (downlinkBursts->size ())
    {
      pair = downlinkBursts->front ();
      pair.second = 0;
      delete pair.first;
    }
  SetBs (0);
  delete m_downlinkBursts;
  m_downlinkBursts = 0;
}

std::list<std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > >*
WranBSSchedulerSimple::GetDownlinkBursts (void) const
{
  return m_downlinkBursts;
}

void WranBSSchedulerSimple::AddDownlinkBurst (Ptr<const WranConnection> connection,
                                          uint8_t diuc,
                                          WranPhy::ModulationType modulationType,
                                          Ptr<PacketBurst> burst)
{
  OfdmDlMapIe *dlMapIe = new OfdmDlMapIe ();
  dlMapIe->SetCid (connection->GetCid ());
  dlMapIe->SetDiuc (diuc);

  NS_LOG_INFO ("BS scheduler, burst size: " << burst->GetSize () << " bytes" << ", pkts: " << burst->GetNPackets ()
                                            << ", connection: " << connection->GetTypeStr () << ", CID: " << connection->GetCid ());
  if (connection->GetType () == Cid::TRANSPORT)
    {
      NS_LOG_INFO (", SFID: " << connection->GetWranServiceFlow ()->GetSfid () << ", service: "
                              << connection->GetWranServiceFlow ()->GetSchedulingTypeStr ());
    }
  NS_LOG_INFO (", modulation: " << modulationType << ", DIUC: " << (uint32_t) diuc);

  m_downlinkBursts->push_back (std::make_pair (dlMapIe, burst));
}

void WranBSSchedulerSimple::Schedule (void)
{
  Ptr<WranConnection> connection;
  WranPhy::ModulationType modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  uint8_t diuc = OfdmDlBurstProfile::DIUC_BURST_PROFILE_1;
  uint32_t nrSymbolsRequired = 0;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst;
  WranServiceFlow::SchedulingType schedulingType = WranServiceFlow::SF_TYPE_NONE;
  uint32_t availableSymbols = GetBs ()->GetNrDlSymbols ();

  while (SelectConnection (connection))
    {
      if (connection != GetBs ()->GetInitialRangingConnection () && connection != GetBs ()->GetBroadcastConnection ())
        {
          /* determines modulation/DIUC only once per burst as it is always same for a particular CID */
          if (connection->GetType () == Cid::MULTICAST)
            {
              modulationType = connection->GetWranServiceFlow ()->GetModulation ();
            }
          else
            {
              modulationType = GetBs ()->GetWranSSManager ()->GetWranSSRecord (connection->GetCid ())->GetModulationType ();
            }
          diuc = GetBs ()->GetWranBurstProfileManager ()->GetBurstProfile (modulationType,
                                                                       WranNetDevice::DIRECTION_DOWNLINK);
        }
      else if (connection == GetBs ()->GetInitialRangingConnection () || connection
               == GetBs ()->GetBroadcastConnection ())
        {

          modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
          diuc = OfdmDlBurstProfile::DIUC_BURST_PROFILE_1;
        }

      if (connection->GetType () == Cid::TRANSPORT || connection->GetType () == Cid::MULTICAST)
        {
          schedulingType = (WranServiceFlow::SchedulingType) connection->GetSchedulingType ();
        }

      if (schedulingType == WranServiceFlow::SF_TYPE_UGS)
        {
          nrSymbolsRequired = connection->GetWranServiceFlow ()->GetRecord ()->GetGrantSize ();
          if (nrSymbolsRequired < availableSymbols)
            {
              burst = CreateUgsBurst (connection->GetWranServiceFlow (), modulationType, nrSymbolsRequired);
            }
          else
            {
              burst = CreateUgsBurst (connection->GetWranServiceFlow (), modulationType, availableSymbols);
            }
          if (burst->GetNPackets () != 0)
            {
              uint32_t BurstSizeSymbols =  GetBs ()->GetPhy ()->GetNrSymbols (burst->GetSize (), modulationType);
              AddDownlinkBurst (connection, diuc, modulationType, burst);

              if (availableSymbols <= BurstSizeSymbols)
                {
                  availableSymbols -= BurstSizeSymbols; /// \todo Overflows but don't know how to fix
                  break;
                }
            }
        }
      else
        {
          burst = Create<PacketBurst> ();
          while (connection->HasPackets () == true)
            {
              uint32_t FirstPacketSize = connection->GetQueue ()->GetFirstPacketRequiredByte (MacHeaderType::HEADER_TYPE_GENERIC);
              nrSymbolsRequired = GetBs ()->GetPhy ()->GetNrSymbols (FirstPacketSize, modulationType);
              if (availableSymbols < nrSymbolsRequired && CheckForFragmentation (connection,
                                                                                 availableSymbols,
                                                                                 modulationType))
                {
                  uint32_t availableByte = GetBs ()->GetPhy ()->GetNrBytes (availableSymbols, modulationType);
                  packet = connection->Dequeue (MacHeaderType::HEADER_TYPE_GENERIC, availableByte);
                  availableSymbols = 0;
                }
              else if (availableSymbols >= nrSymbolsRequired)
                {
                  packet = connection->Dequeue ();
                  availableSymbols -= nrSymbolsRequired;
                }
              else
                {
                  break;
                }
              burst->AddPacket (packet);
            }
          AddDownlinkBurst (connection, diuc, modulationType, burst);
        }
      if (availableSymbols == 0)
        {
          break;
        }
    }


  if (m_downlinkBursts->size ())
    {
      NS_LOG_DEBUG ("BS scheduler, number of bursts: " << m_downlinkBursts->size () << ", symbols left: "
                                                       << availableSymbols << std::endl << "BS scheduler, queues:" << " IR "
                                                       << GetBs ()->GetInitialRangingConnection ()->GetQueue ()->GetSize () << " broadcast "
                                                       << GetBs ()->GetBroadcastConnection ()->GetQueue ()->GetSize () << " basic "
                                                       << GetBs ()->GetWranConnectionManager ()->GetNPackets (Cid::BASIC, WranServiceFlow::SF_TYPE_NONE) << " primary "
                                                       << GetBs ()->GetWranConnectionManager ()->GetNPackets (Cid::PRIMARY, WranServiceFlow::SF_TYPE_NONE) << " transport "
                                                       << GetBs ()->GetWranConnectionManager ()->GetNPackets (Cid::TRANSPORT, WranServiceFlow::SF_TYPE_ALL));
    }
}

bool WranBSSchedulerSimple::SelectConnection (Ptr<WranConnection> &connection)
{
  connection = 0;
  Time currentTime = Simulator::Now ();
  std::vector<Ptr<WranConnection> >::const_iterator iter1;
  std::vector<WranServiceFlow*>::iterator iter2;
  WranServiceFlowRecord *serviceFlowRecord;
  NS_LOG_INFO ("BS Scheduler: Selecting connection...");
  if (GetBs ()->GetBroadcastConnection ()->HasPackets ())
    {
      NS_LOG_INFO ("Return GetBroadcastConnection");
      connection = GetBs ()->GetBroadcastConnection ();
      return true;
    }
  else if (GetBs ()->GetInitialRangingConnection ()->HasPackets ())
    {
      NS_LOG_INFO ("Return GetInitialRangingConnection");
      connection = GetBs ()->GetInitialRangingConnection ();
      return true;
    }
  else
    {
      std::vector<Ptr<WranConnection> > connections;
      std::vector<WranServiceFlow*> serviceFlows;

      connections = GetBs ()->GetWranConnectionManager ()->GetConnections (Cid::BASIC);
      for (iter1 = connections.begin (); iter1 != connections.end (); ++iter1)
        {
          if ((*iter1)->HasPackets ())
            {
              NS_LOG_INFO ("Return Basic");
              connection = *iter1;
              return true;
            }
        }

      connections = GetBs ()->GetWranConnectionManager ()->GetConnections (Cid::PRIMARY);
      for (iter1 = connections.begin (); iter1 != connections.end (); ++iter1)
        {
          if ((*iter1)->HasPackets ())
            {
              NS_LOG_INFO ("Return Primary");
              connection = *iter1;
              return true;
            }
        }

      serviceFlows = GetBs ()->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_UGS);
      for (iter2 = serviceFlows.begin (); iter2 != serviceFlows.end (); ++iter2)
        {
          serviceFlowRecord = (*iter2)->GetRecord ();
          NS_LOG_INFO ("processing UGS: HAS PACKET=" << (*iter2)->HasPackets () << "max Latency = "
                                                     << MilliSeconds ((*iter2)->GetMaximumLatency ()) << "Delay = " << ((currentTime
                                                                                           - serviceFlowRecord->GetDlTimeStamp ()) + GetBs ()->GetPhy ()->GetFrameDuration ()));
          // if latency would exceed in case grant is allocated in next frame then allocate in current frame
          if ((*iter2)->HasPackets () && ((currentTime - serviceFlowRecord->GetDlTimeStamp ())
                                          + GetBs ()->GetPhy ()->GetFrameDuration ()) > MilliSeconds ((*iter2)->GetMaximumLatency ()))
            {
              serviceFlowRecord->SetDlTimeStamp (currentTime);
              connection = (*iter2)->GetConnection ();
              NS_LOG_INFO ("Return UGS SF: CID = " << (*iter2)->GetCid () << "SFID = " << (*iter2)->GetSfid ());
              return true;
            }
        }

      serviceFlows = GetBs ()->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_RTPS);
      for (iter2 = serviceFlows.begin (); iter2 != serviceFlows.end (); ++iter2)
        {
          serviceFlowRecord = (*iter2)->GetRecord ();
          // if latency would exceed in case poll is allocated in next frame then allocate in current frame
          if ((*iter2)->HasPackets () && ((currentTime - serviceFlowRecord->GetDlTimeStamp ())
                                          + GetBs ()->GetPhy ()->GetFrameDuration ()) > MilliSeconds ((*iter2)->GetMaximumLatency ()))
            {
              serviceFlowRecord->SetDlTimeStamp (currentTime);
              connection = (*iter2)->GetConnection ();
              NS_LOG_INFO ("Return RTPS SF: CID = " << (*iter2)->GetCid () << "SFID = " << (*iter2)->GetSfid ());
              return true;
            }
        }

      serviceFlows = GetBs ()->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_NRTPS);
      for (iter2 = serviceFlows.begin (); iter2 != serviceFlows.end (); ++iter2)
        {
          //unused: serviceFlowRecord = (*iter2)->GetRecord ();
          if ((*iter2)->HasPackets ())
            {
              NS_LOG_INFO ("Return NRTPS SF: CID = " << (*iter2)->GetCid () << "SFID = " << (*iter2)->GetSfid ());
              connection = (*iter2)->GetConnection ();
              return true;
            }
        }

      serviceFlows = GetBs ()->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_BE);
      for (iter2 = serviceFlows.begin (); iter2 != serviceFlows.end (); ++iter2)
        {
          //unused: serviceFlowRecord = (*iter2)->GetRecord ();
          if ((*iter2)->HasPackets ())
            {
              NS_LOG_INFO ("Return BE SF: CID = " << (*iter2)->GetCid () << "SFID = " << (*iter2)->GetSfid ());
              connection = (*iter2)->GetConnection ();
              return true;
            }
        }
    }
  NS_LOG_INFO ("NO connection is selected!");
  return false;
}

Ptr<PacketBurst> WranBSSchedulerSimple::CreateUgsBurst (WranServiceFlow *serviceFlow,
                                                    WranPhy::ModulationType modulationType,
                                                    uint32_t availableSymbols)
{
  Time timeStamp;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();
  uint32_t nrSymbolsRequired = 0;

  // serviceFlow->CleanUpQueue ();
  Ptr<WranConnection> connection = serviceFlow->GetConnection ();
  while (serviceFlow->HasPackets ())
    {
      uint32_t FirstPacketSize = connection->GetQueue ()->GetFirstPacketRequiredByte (MacHeaderType::HEADER_TYPE_GENERIC);
      nrSymbolsRequired = GetBs ()->GetPhy ()->GetNrSymbols (FirstPacketSize,modulationType);
      if (availableSymbols < nrSymbolsRequired && CheckForFragmentation (connection,
                                                                         availableSymbols,
                                                                         modulationType))
        {
          uint32_t availableByte = GetBs ()->GetPhy ()->GetNrBytes (availableSymbols, modulationType);
          packet = connection->Dequeue (MacHeaderType::HEADER_TYPE_GENERIC, availableByte);
          availableSymbols = 0;
        }
      else
        {
          packet = connection->Dequeue ();
          availableSymbols -= nrSymbolsRequired;
        }
      burst->AddPacket (packet);
      if (availableSymbols <= 0)
        {
          break;
        }
    }
  return burst;
}

}
