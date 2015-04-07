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

#include "ns3/simulator.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "wran-ss-scheduler.h"
#include "wran-ss-net-device.h"
#include "wran-phy.h"
#include "wran-mac-queue.h"
#include "wran-connection.h"
#include "wran-connection-manager.h"
#include "wran-service-flow.h"
#include "wran-service-flow-record.h"
#include "wran-service-flow-manager.h"

NS_LOG_COMPONENT_DEFINE ("WranSSScheduler");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (WranSSScheduler);

TypeId WranSSScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranSSScheduler")
    .SetParent<Object> ();
  return tid;
}

WranSSScheduler::WranSSScheduler (Ptr<WranSubscriberStationNetDevice> ss)
  : m_ss (ss),
    m_pollMe (false)
{
}

WranSSScheduler::~WranSSScheduler (void)
{
}

void
WranSSScheduler::DoDispose (void)
{
  m_ss = 0;
}

void
WranSSScheduler::SetPollMe (bool pollMe)
{
  m_pollMe = pollMe;
}

bool
WranSSScheduler::GetPollMe (void) const
{
  return m_pollMe;
}

Ptr<PacketBurst>
WranSSScheduler::Schedule (uint16_t availableSymbols,
                       WranPhy::ModulationType modulationType,
                       MacHeaderType::HeaderType packetType,
                       Ptr<WranConnection> &connection)
{
  Time timeStamp;
  Ptr<PacketBurst> burst = Create<PacketBurst> ();
  uint16_t nrSymbolsRequired = 0;

  if (!connection)
    {
      connection = SelectConnection ();
    }
  else
    {
      NS_ASSERT_MSG (connection->HasPackets (),
                     "SS: Error while scheduling packets: The selected connection has no packets");
    }

  Ptr<Packet> packet;

  while (connection && connection->HasPackets (packetType))
    {
      NS_LOG_INFO ("FRAG_DEBUG: SS Scheduler" << std::endl);

      uint32_t availableByte = m_ss->GetPhy ()->
        GetNrBytes (availableSymbols, modulationType);

      uint32_t requiredByte = connection->GetQueue ()->GetFirstPacketRequiredByte (packetType);

      NS_LOG_INFO ("\t availableByte = " << availableByte <<
                   ", requiredByte = " << requiredByte);

      if (availableByte >= requiredByte)
        {
          // The SS could sent a packet without a other fragmentation
          NS_LOG_INFO ("\t availableByte >= requiredByte"
                       "\n\t Send packet without other fragmentation" << std::endl);

          packet = connection->Dequeue (packetType);
          burst->AddPacket (packet);

          nrSymbolsRequired = m_ss->GetPhy ()->
            GetNrSymbols (packet->GetSize (), modulationType);
          availableSymbols -= nrSymbolsRequired;
        }
      else
        {
          if (connection->GetType () == Cid::TRANSPORT)
            {
              NS_LOG_INFO ("\t availableByte < requiredByte"
                           "\n\t Check if the fragmentation is possible");

              uint32_t headerSize = connection->GetQueue ()->GetFirstPacketHdrSize (packetType);
              if (!connection->GetQueue ()->CheckForFragmentation (packetType))
                {
                  NS_LOG_INFO ("\t Add fragmentSubhdrSize = 2");
                  headerSize += 2;
                }
              NS_LOG_INFO ("\t availableByte = " << availableByte <<
                           " headerSize = " << headerSize);

              if (availableByte > headerSize)
                {
                  NS_LOG_INFO ("\t Fragmentation IS possible");
                  packet = connection->Dequeue (packetType, availableByte);
                  burst->AddPacket (packet);

                  nrSymbolsRequired = m_ss->GetPhy ()->
                    GetNrSymbols (packet->GetSize (), modulationType);
                  availableSymbols -= nrSymbolsRequired;
                }
              else
                {
                  NS_LOG_INFO ("\t Fragmentation IS NOT possible" << std::endl);
                  break;
                }
            }
          else
            {
              NS_LOG_INFO ("\t no Transport Connection "
                           "\n\t Fragmentation IS NOT possible, " << std::endl);
              break;
            }
        }
    }
  return burst;
}

Ptr<WranConnection>
WranSSScheduler::SelectConnection (void)
{
  Time currentTime = Simulator::Now ();
  std::vector<WranServiceFlow*>::const_iterator iter;
  std::vector<WranServiceFlow*> serviceFlows;

  NS_LOG_INFO ("SS Scheduler: Selecting connection...");
  if (m_ss->GetInitialRangingConnection ()->HasPackets ())
    {
      NS_LOG_INFO ("Return GetInitialRangingConnection");
      return m_ss->GetInitialRangingConnection ();
    }
  if (m_ss->GetBasicConnection ()->HasPackets ())
    {
      NS_LOG_INFO ("Return GetBasicConnection");
      return m_ss->GetBasicConnection ();
    }
  if (m_ss->GetPrimaryConnection ()->HasPackets ())
    {
      NS_LOG_INFO ("Return GetPrimaryConnection");
      return m_ss->GetPrimaryConnection ();
    }

  serviceFlows = m_ss->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_UGS);
  for (iter = serviceFlows.begin (); iter != serviceFlows.end (); ++iter)
    {
      // making sure that this grant was actually intended for this UGS

      if ((*iter)->HasPackets () && (currentTime
                                     + m_ss->GetPhy ()->GetFrameDuration () > MilliSeconds (
                                       (*iter)->GetUnsolicitedGrantInterval ())))
        {
          NS_LOG_INFO ("Return UGS SF: CID = " << (*iter)->GetCid () << "SFID = "
                                               << (*iter)->GetSfid ());
          return (*iter)->GetConnection ();
        }
    }

  /* In the following cases (rtPS, nrtPS and BE flows) connection is seletected only for data packets, for bandwidth
   request packets connection will itself be passed to Schedule () and hence this function will never be called. */

  serviceFlows = m_ss->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_RTPS);
  for (iter = serviceFlows.begin (); iter != serviceFlows.end (); ++iter)
    {
      if ((*iter)->HasPackets (MacHeaderType::HEADER_TYPE_GENERIC)
          && (currentTime + m_ss->GetPhy ()->GetFrameDuration ()
              > MilliSeconds (
                (*iter)->GetUnsolicitedPollingInterval ())))
        {
          NS_LOG_INFO ("Return RTPS SF: CID = " << (*iter)->GetCid () << "SFID = "
                                                << (*iter)->GetSfid ());
          return (*iter)->GetConnection ();
        }
    }

  serviceFlows = m_ss->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_NRTPS);
  for (iter = serviceFlows.begin (); iter != serviceFlows.end (); ++iter)
    {
      if ((*iter)->HasPackets (MacHeaderType::HEADER_TYPE_GENERIC))
        {
          NS_LOG_INFO ("Return NRTPS SF: CID = " << (*iter)->GetCid () << "SFID = "
                                                 << (*iter)->GetSfid ());
          return (*iter)->GetConnection ();
        }
    }

  serviceFlows = m_ss->GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_BE);
  for (iter = serviceFlows.begin (); iter != serviceFlows.end (); ++iter)
    {
      if ((*iter)->HasPackets (MacHeaderType::HEADER_TYPE_GENERIC))
        {
          NS_LOG_INFO ("Return BE SF: CID = " << (*iter)->GetCid () << "SFID = "
                                              << (*iter)->GetSfid ());
          return (*iter)->GetConnection ();
        }
    }

  if (m_ss->GetBroadcastConnection ()->HasPackets ())
    {
      return m_ss->GetBroadcastConnection ();
    }
  NS_LOG_INFO ("NO connection is selected!");
  return 0;
}

} // namespace ns3
