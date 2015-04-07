/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 INRIA
 *               2009 TELEMATICS LAB, Politecnico di Bari
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
 * Author: Giuseppe Piro <g.piro@poliba.it>
 */

#include "wran-bs-scheduler-rtps.h"
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
#include "wran-mac-queue.h"

NS_LOG_COMPONENT_DEFINE ("WranBSSchedulerRtps");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranBSSchedulerRtps);

TypeId
WranBSSchedulerRtps::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranBSSchedulerRtps").SetParent<Object> ().AddConstructor<WranBSSchedulerRtps> ();
  return tid;
}

WranBSSchedulerRtps::WranBSSchedulerRtps ()
  : m_downlinkBursts (new std::list<std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > > ())
{
  SetBs (0);
}

WranBSSchedulerRtps::WranBSSchedulerRtps (Ptr<WranBaseStationNetDevice> bs)
  : m_downlinkBursts (new std::list<std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > > ())
{
  // m_downlinkBursts is filled by AddDownlinkBurst and emptied by
  // wran-bs-net-device::sendBurst and wran-ss-net-device::sendBurst
  SetBs (bs);
}

WranBSSchedulerRtps::~WranBSSchedulerRtps (void)
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
WranBSSchedulerRtps::GetDownlinkBursts (void) const
{
  return m_downlinkBursts;
}

void
WranBSSchedulerRtps::AddDownlinkBurst (Ptr<const WranConnection> connection,
                                   uint8_t diuc,
                                   WranPhy::ModulationType modulationType,
                                   Ptr<PacketBurst> burst)
{
  OfdmDlMapIe *dlMapIe = new OfdmDlMapIe ();
  dlMapIe->SetCid (connection->GetCid ());
  dlMapIe->SetDiuc (diuc);

  NS_LOG_INFO ("BS scheduler, burst size: " << burst->GetSize () << " bytes" << ", pkts: " << burst->GetNPackets ()
                                            << ", connection: " << connection->GetTypeStr () << ", CID: "
                                            << connection->GetCid ());
  if (connection->GetType () == Cid::TRANSPORT)
    {
      NS_LOG_INFO (", SFID: " << connection->GetWranServiceFlow ()->GetSfid () << ", service: "
                              << connection->GetWranServiceFlow ()->GetSchedulingTypeStr ());
    }
  NS_LOG_INFO (", modulation: " << modulationType << ", DIUC: " << (uint32_t) diuc);

  m_downlinkBursts->push_back (std::make_pair (dlMapIe, burst));
}

/**
 * \brief A DownLink Scheduler for rtPS Flows
 *
 * The DL Scheduler assigns the available bandwidth in the following order:
 * - IR Connections
 * - Broadcast Connections
 * - Basic and Primary Connections
 * - UGS Connections
 * - rtPS Connections
 * - nrtPS Connections
 * - BE Connections
 * All rtPS flows that have packets in the queue can transmit at least one
 * packet, according to the available bandwidth.
 */
void
WranBSSchedulerRtps::Schedule (void)
{

  uint32_t availableSymbols = GetBs ()->GetNrDlSymbols ();

  WranBSSchedulerBroadcastConnection (availableSymbols);

  WranBSSchedulerInitialRangingConnection (availableSymbols);

  WranBSSchedulerBasicConnection (availableSymbols);

  WranBSSchedulerPrimaryConnection (availableSymbols);

  WranBSSchedulerUGSConnection (availableSymbols);

  WranBSSchedulerRTPSConnection (availableSymbols);

  WranBSSchedulerNRTPSConnection (availableSymbols);

  WranBSSchedulerBEConnection (availableSymbols);

  if (m_downlinkBursts->size ())
    {
      NS_LOG_DEBUG ("BS scheduler, number of bursts: " << m_downlinkBursts->size () << ", symbols left: "
                                                       << availableSymbols << std::endl << "BS scheduler, queues:" << " IR "
                                                       << GetBs ()->GetInitialRangingConnection ()->GetQueue ()->GetSize ()
                                                       << " broadcast "
                                                       << GetBs ()->GetBroadcastConnection ()->GetQueue ()->GetSize ()
                                                       << " basic "
                                                       << GetBs ()->GetWranConnectionManager ()->GetNPackets (Cid::BASIC, WranServiceFlow::SF_TYPE_NONE)
                                                       << " primary "
                                                       << GetBs ()->GetWranConnectionManager ()->GetNPackets (Cid::PRIMARY, WranServiceFlow::SF_TYPE_NONE)
                                                       << " transport "
                                                       << GetBs ()->GetWranConnectionManager ()->GetNPackets (Cid::TRANSPORT, WranServiceFlow::SF_TYPE_ALL));
    }
}


Ptr<PacketBurst> WranBSSchedulerRtps::CreateUgsBurst (WranServiceFlow *serviceFlow,
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


bool
WranBSSchedulerRtps::SelectConnection (Ptr<WranConnection> &connection)
{
  return false;
}

void
WranBSSchedulerRtps::WranBSSchedulerBroadcastConnection (uint32_t &availableSymbols)
{
  Ptr<WranConnection> connection;
  WranPhy::ModulationType modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  uint8_t diuc = OfdmDlBurstProfile::DIUC_BURST_PROFILE_1;
  uint32_t nrSymbolsRequired = 0;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  while (GetBs ()->GetBroadcastConnection ()->HasPackets () && availableSymbols > 0)
    {
      connection = GetBs ()->GetBroadcastConnection ();

      packet = connection->GetQueue ()->Peek (hdr);
      nrSymbolsRequired = GetBs ()->GetPhy ()->GetNrSymbols (packet->GetSize (), modulationType);

      if (availableSymbols < nrSymbolsRequired
          && !CheckForFragmentation (connection, availableSymbols, modulationType))
        {
          break;
        }
      else if (availableSymbols < nrSymbolsRequired
               && CheckForFragmentation (connection, availableSymbols, modulationType))
        {
          uint32_t availableByte = GetBs ()->GetPhy ()->
            GetNrBytes (availableSymbols, modulationType);
          packet = connection->Dequeue (MacHeaderType::HEADER_TYPE_GENERIC,
                                        availableByte);
        }
      else
        {
          packet = connection->Dequeue ();
        }

      NS_ASSERT_MSG (hdr.GetCid ().GetIdentifier () == connection->GetCid (),
                     "Base station: Error while scheduling broadcast connection: header CID != connection CID");
      burst->AddPacket (packet);
      availableSymbols -= nrSymbolsRequired;
    }
  if (burst->GetNPackets () != 0)
    {
      AddDownlinkBurst (connection, diuc, modulationType, burst);
    }
}

void
WranBSSchedulerRtps::WranBSSchedulerInitialRangingConnection (uint32_t &availableSymbols)
{
  Ptr<WranConnection> connection;
  WranPhy::ModulationType modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  uint8_t diuc = OfdmDlBurstProfile::DIUC_BURST_PROFILE_1;
  uint32_t nrSymbolsRequired = 0;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  while (GetBs ()->GetInitialRangingConnection ()->HasPackets () && availableSymbols > 0)
    {
      connection = GetBs ()->GetInitialRangingConnection ();

      packet = connection->GetQueue ()->Peek (hdr);
      nrSymbolsRequired = GetBs ()->GetPhy ()->GetNrSymbols (packet->GetSize (), modulationType);

      // PIRO: check for fragmentation
      if (availableSymbols < nrSymbolsRequired
          && !CheckForFragmentation (connection, availableSymbols, modulationType))
        {
          break;
        }
      else if (availableSymbols < nrSymbolsRequired
               && CheckForFragmentation (connection, availableSymbols, modulationType))
        {
          uint32_t availableByte = GetBs ()->GetPhy ()->
            GetNrBytes (availableSymbols, modulationType);
          packet = connection->Dequeue (MacHeaderType::HEADER_TYPE_GENERIC,
                                        availableByte);
        }
      else
        {
          packet = connection->Dequeue ();
        }

      NS_ASSERT_MSG (hdr.GetCid () == connection->GetCid (),
                     "Base station: Error while scheduling initial ranging connection: header CID != connection CID");
      burst->AddPacket (packet);
      availableSymbols -= nrSymbolsRequired;
    }
  if (burst->GetNPackets ())
    {
      AddDownlinkBurst (connection, diuc, modulationType, burst);
    }
}

void
WranBSSchedulerRtps::WranBSSchedulerBasicConnection (uint32_t &availableSymbols)
{
  Ptr<WranConnection> connection;
  WranPhy::ModulationType modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  uint8_t diuc = OfdmDlBurstProfile::DIUC_BURST_PROFILE_1;
  uint32_t nrSymbolsRequired = 0;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  std::vector<Ptr<WranConnection> >::const_iterator iter;
  std::vector<Ptr<WranConnection> > connections;

  connections = GetBs ()->GetWranConnectionManager ()->GetConnections (Cid::BASIC);
  for (iter = connections.begin (); iter != connections.end (); ++iter)
    {
      while ((*iter)->HasPackets () && availableSymbols > 0)
        {
          connection = *iter;

          modulationType = GetBs ()->GetWranSSManager ()->GetWranSSRecord (connection->GetCid ())->GetModulationType ();
          diuc = GetBs ()->GetWranBurstProfileManager ()->GetBurstProfile (modulationType,
                                                                       WranNetDevice::DIRECTION_DOWNLINK);

          packet = connection->GetQueue ()->Peek (hdr);
          nrSymbolsRequired = GetBs ()->GetPhy ()->GetNrSymbols (packet->GetSize (), modulationType);

          // PIRO: check for fragmentation
          if (availableSymbols < nrSymbolsRequired
              && !CheckForFragmentation (connection, availableSymbols, modulationType))
            {
              break;
            }
          else if (availableSymbols < nrSymbolsRequired
                   && CheckForFragmentation (connection, availableSymbols, modulationType))
            {
              uint32_t availableByte = GetBs ()->GetPhy ()->
                GetNrBytes (availableSymbols, modulationType);
              packet = connection->Dequeue (MacHeaderType::HEADER_TYPE_GENERIC, availableByte);
            }
          else
            {
              packet = connection->Dequeue ();
            }

          NS_ASSERT_MSG (hdr.GetCid () == connection->GetCid (),
                         "Base station: Error while scheduling basic connection: header CID != connection CID");
          burst->AddPacket (packet);
          availableSymbols -= nrSymbolsRequired;
        }
      if (burst->GetNPackets () != 0)
        {
          AddDownlinkBurst (connection, diuc, modulationType, burst);
          burst = Create<PacketBurst> ();
        }
    }
}

void
WranBSSchedulerRtps::WranBSSchedulerPrimaryConnection (uint32_t &availableSymbols)
{
  Ptr<WranConnection> connection;
  WranPhy::ModulationType modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  uint8_t diuc = 0;
  uint32_t nrSymbolsRequired = 0;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  std::vector<Ptr<WranConnection> >::const_iterator iter;
  std::vector<Ptr<WranConnection> > connections;

  connections = GetBs ()->GetWranConnectionManager ()->GetConnections (Cid::PRIMARY);
  for (iter = connections.begin (); iter != connections.end (); ++iter)
    {
      while ((*iter)->HasPackets () && availableSymbols > 0)
        {
          connection = *iter;

          modulationType = GetBs ()->GetWranSSManager ()->GetWranSSRecord (connection->GetCid ())->GetModulationType ();
          diuc = GetBs ()->GetWranBurstProfileManager ()->GetBurstProfile (modulationType,
                                                                       WranNetDevice::DIRECTION_DOWNLINK);

          packet = connection->GetQueue ()->Peek (hdr);
          nrSymbolsRequired = GetBs ()->GetPhy ()->GetNrSymbols (packet->GetSize (), modulationType);

          // PIRO: check for fragmentation
          if (availableSymbols < nrSymbolsRequired
              && !CheckForFragmentation (connection, availableSymbols, modulationType))
            {
              break;
            }
          else if (availableSymbols < nrSymbolsRequired
                   && CheckForFragmentation (connection, availableSymbols, modulationType))
            {
              uint32_t availableByte = GetBs ()->GetPhy ()->
                GetNrBytes (availableSymbols, modulationType);
              packet = connection->Dequeue (MacHeaderType::HEADER_TYPE_GENERIC, availableByte);
            }
          else
            {
              packet = connection->Dequeue ();
            }

          NS_ASSERT_MSG (hdr.GetCid () == connection->GetCid (),
                         "Base station: Error while scheduling primary connection: header CID != connection CID");
          burst->AddPacket (packet);
          availableSymbols -= nrSymbolsRequired;
        }
      if (burst->GetNPackets () != 0)
        {
          AddDownlinkBurst (connection, diuc, modulationType, burst);
        }
    }
}

void
WranBSSchedulerRtps::WranBSSchedulerUGSConnection (uint32_t &availableSymbols)
{
  Ptr<WranConnection> connection;
  WranPhy::ModulationType modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  uint8_t diuc;
  uint32_t nrSymbolsRequired = 0;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  Time currentTime = Simulator::Now ();

  std::vector<WranServiceFlow*>::iterator iter;
  WranServiceFlowRecord *serviceFlowRecord;
  std::vector<WranServiceFlow*> serviceFlows;

  serviceFlows = GetBs ()->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_UGS);
  for (iter = serviceFlows.begin (); iter != serviceFlows.end (); ++iter)
    {
      serviceFlowRecord = (*iter)->GetRecord ();
      // if latency would exceed in case grant is allocated in next frame then allocate in current frame
      if ((*iter)->HasPackets () && ((currentTime - serviceFlowRecord->GetDlTimeStamp ())
                                     + GetBs ()->GetPhy ()->GetFrameDuration ()) > MilliSeconds ((*iter)->GetMaximumLatency ()))
        {
          connection = (*iter)->GetConnection ();
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

          nrSymbolsRequired = connection->GetWranServiceFlow ()->GetRecord ()->GetGrantSize ();

          // Packet fragmentation for UGS connections has not been implemented yet!
          if (availableSymbols > nrSymbolsRequired)
            {
              availableSymbols -= nrSymbolsRequired;
              burst = CreateUgsBurst (connection->GetWranServiceFlow (), modulationType, nrSymbolsRequired);
              if (burst->GetNPackets () != 0)
                {
                  AddDownlinkBurst (connection, diuc, modulationType, burst);
                  currentTime = Simulator::Now ();
                  serviceFlowRecord->SetDlTimeStamp (currentTime);
                  burst = Create<PacketBurst> ();
                }
            }
        }
    }

}

void
WranBSSchedulerRtps::WranBSSchedulerRTPSConnection (uint32_t &availableSymbols)
{

  Ptr<WranConnection> connection;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  Time currentTime = Simulator::Now ();

  std::vector<Ptr<WranConnection> >::const_iterator iter;
  std::vector<Ptr<WranConnection> > connections;
  std::vector<WranServiceFlow*>::iterator iter2;
  WranServiceFlowRecord *serviceFlowRecord;
  std::vector<WranServiceFlow*> serviceFlows;

  uint32_t symbolsRequired[100];
  WranPhy::ModulationType modulationType_[100];
  uint8_t diuc_[100];
  Ptr<WranConnection> rtPSConnection[100];
  uint32_t dataToSend;
  uint32_t totSymbolsRequired = 0;
  int nbConnection = 0;

  NS_LOG_INFO ("\tDL Scheduler for rtPS flows \n" << "\t\tavailableSymbols = " << availableSymbols);

  serviceFlows = GetBs ()->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_RTPS);
  nbConnection = 0;
  for (iter2 = serviceFlows.begin (); iter2 != serviceFlows.end (); ++iter2)
    {
      // DL RTPS Scheduler works for all rtPS connection that have packets to transmitt!!!
      serviceFlowRecord = (*iter2)->GetRecord ();

      if ((*iter2)->HasPackets ())
        {
          currentTime = Simulator::Now ();
          serviceFlowRecord->SetDlTimeStamp (currentTime);
          rtPSConnection[nbConnection] = (*iter2)->GetConnection ();
          if (rtPSConnection[nbConnection]->GetType () == Cid::MULTICAST)
            {
              modulationType_[nbConnection] = rtPSConnection[nbConnection]->GetWranServiceFlow ()->GetModulation ();
            }
          else
            {
              modulationType_[nbConnection]
                = GetBs ()->GetWranSSManager ()->GetWranSSRecord (rtPSConnection[nbConnection]->GetCid ())->GetModulationType ();
            }
          diuc_[nbConnection]
            = GetBs ()->GetWranBurstProfileManager ()->GetBurstProfile (modulationType_[nbConnection],
                                                                    WranNetDevice::DIRECTION_DOWNLINK);

          dataToSend = rtPSConnection[nbConnection]->GetQueue ()->GetQueueLengthWithMACOverhead ();
          NS_LOG_INFO ("\t\tRTPS DL Scheduler for CID = " << rtPSConnection[nbConnection]->GetCid ()
                                                          << "\n\t\t\t dataToSend = " << dataToSend);

          symbolsRequired[nbConnection] = GetBs ()->GetPhy ()->GetNrSymbols (dataToSend,
                                                                             modulationType_[nbConnection]);

          totSymbolsRequired += symbolsRequired[nbConnection];
          nbConnection++;
        }
    }

  NS_LOG_INFO ("\t\ttotSymbolsRequired = " << totSymbolsRequired);

  // Channel Saturation
  while (totSymbolsRequired > availableSymbols)
    {
      NS_LOG_INFO ("\tDL Channel Saturation: totSymbolsRequired > availableSymbols_rtPS");
      double delta = double(availableSymbols) / double(totSymbolsRequired);
      NS_LOG_INFO ("\t\tdelta = " << delta);
      totSymbolsRequired = 0;
      for (int i = 0; i < nbConnection; i++)
        {
          NS_LOG_INFO ("\t\tprevious symbolsRequired[" << i << "] = " << symbolsRequired[i]);
          symbolsRequired[i] = (uint32_t) std::floor (symbolsRequired[i] * delta);
          totSymbolsRequired += symbolsRequired[i];
          NS_LOG_INFO ("\t\tnew symbolsRequired[" << i << "] = " << symbolsRequired[i]);
        }
      NS_LOG_INFO ("\t\ttotSymbolsRequired = " << totSymbolsRequired);
    }

  // Downlink Bandwidth Allocation
  for (int i = 0; i < nbConnection; i++)
    {

      packet = rtPSConnection[i]->GetQueue ()->Peek (hdr);
      uint32_t symbolsForPacketTransmission = 0;
      burst = Create<PacketBurst> ();
      NS_LOG_INFO ("\t\tCID = " << rtPSConnection[i]->GetCid () << " assignedSymbols = " << symbolsRequired[i]);

      while (symbolsRequired[i] > 0)
        {
          symbolsForPacketTransmission = GetBs ()->GetPhy ()
            ->GetNrSymbols (rtPSConnection[i]->GetQueue ()
                            ->GetFirstPacketRequiredByte (MacHeaderType::HEADER_TYPE_GENERIC),
                            modulationType_[i]);

          // PIRO: check for fragmentation
          if (symbolsForPacketTransmission > symbolsRequired[i]
              && !CheckForFragmentation (rtPSConnection[i],
                                         symbolsRequired[i],
                                         modulationType_[i]))
            {
              break;
            }
          else if (symbolsForPacketTransmission > symbolsRequired[i]
                   && CheckForFragmentation (rtPSConnection[i],
                                             symbolsRequired[i],
                                             modulationType_[i]))
            {
              uint32_t availableByte = GetBs ()->GetPhy ()->
                GetNrBytes (symbolsRequired[i], modulationType_[i]);
              packet = rtPSConnection[i]->Dequeue (MacHeaderType::HEADER_TYPE_GENERIC, availableByte);
              symbolsRequired[i] = 0;
            }
          else
            {
              packet = rtPSConnection[i]->Dequeue ();
              symbolsRequired[i] -= symbolsForPacketTransmission;
            }

          NS_ASSERT_MSG (hdr.GetCid () == rtPSConnection[i]->GetCid (),
                         "Base station: Error while scheduling RTPs connection: header CID != connection CID");
          burst->AddPacket (packet);
        }

      if (burst->GetNPackets () != 0)
        {
          AddDownlinkBurst (rtPSConnection[i], diuc_[i], modulationType_[i], burst);
        }
    }

  availableSymbols -= totSymbolsRequired;
}

void
WranBSSchedulerRtps::WranBSSchedulerNRTPSConnection (uint32_t &availableSymbols)
{
  Ptr<WranConnection> connection;
  WranPhy::ModulationType modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  uint8_t diuc = 0;
  uint32_t nrSymbolsRequired = 0;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  std::vector<WranServiceFlow*>::iterator iter;
  std::vector<WranServiceFlow*> serviceFlows;

  serviceFlows = GetBs ()->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_NRTPS);
  for (iter = serviceFlows.begin (); iter != serviceFlows.end (); ++iter)
    {
      connection = (*iter)->GetConnection ();

      while ((*iter)->HasPackets () && availableSymbols > 0)
        {
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

          packet = connection->GetQueue ()->Peek (hdr);
          nrSymbolsRequired = GetBs ()->GetPhy ()->GetNrSymbols (packet->GetSize (), modulationType);

          if (availableSymbols < nrSymbolsRequired)
            {
              break;
            }

          packet = connection->Dequeue ();
          NS_ASSERT_MSG (hdr.GetCid () == connection->GetCid (),
                         "Base station: Error while scheduling NRTPs connection: header CID != connection CID");
          burst->AddPacket (packet);
          availableSymbols -= nrSymbolsRequired;
        }
      if (burst->GetNPackets () != 0)
        {
          AddDownlinkBurst (connection, diuc, modulationType, burst);
          burst = Create<PacketBurst> ();
        }
    }
}

void
WranBSSchedulerRtps::WranBSSchedulerBEConnection (uint32_t &availableSymbols)
{
  Ptr<WranConnection> connection;
  WranPhy::ModulationType modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  uint8_t diuc = 0;
  uint32_t nrSymbolsRequired = 0;
  GenericMacHeader hdr;
  Ptr<Packet> packet;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  std::vector<WranServiceFlow*>::iterator iter;
  std::vector<WranServiceFlow*> serviceFlows;

  serviceFlows = GetBs ()->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_BE);
  for (iter = serviceFlows.begin (); iter != serviceFlows.end (); ++iter)
    {
      connection = (*iter)->GetConnection ();

      while ((*iter)->HasPackets () && availableSymbols > 0)
        {
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

          packet = connection->GetQueue ()->Peek (hdr);
          nrSymbolsRequired = GetBs ()->GetPhy ()->GetNrSymbols (packet->GetSize (), modulationType);

          if (availableSymbols < nrSymbolsRequired)
            {
              break;
            }

          packet = connection->Dequeue ();
          NS_ASSERT_MSG (hdr.GetCid () == connection->GetCid (),
                         "Base station: Error while scheduling BE connection: header CID != connection CID");
          burst->AddPacket (packet);
          availableSymbols -= nrSymbolsRequired;
        }
      if (burst->GetNPackets () != 0)
        {
          AddDownlinkBurst (connection, diuc, modulationType, burst);
          burst = Create<PacketBurst> ();
        }
    }
}

} // namespace ns3
