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

#include "wran-bs-scheduler.h"
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

NS_LOG_COMPONENT_DEFINE ("WranBSScheduler");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (WranBSScheduler);

TypeId
WranBSScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranBSScheduler").SetParent<Object> ();
  return tid;
}

WranBSScheduler::WranBSScheduler ()
  : m_downlinkBursts (new std::list<std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > > ())
{
  // m_downlinkBursts is filled by AddDownlinkBurst and emptied by
  // wran-bs-net-device::sendBurst and wran-ss-net-device::sendBurst
}

WranBSScheduler::WranBSScheduler (Ptr<WranBaseStationNetDevice> bs)
  : m_downlinkBursts (new std::list<std::pair<OfdmDlMapIe*, Ptr<PacketBurst> > > ())
{

}

WranBSScheduler::~WranBSScheduler (void)
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

void
WranBSScheduler::SetBs (Ptr<WranBaseStationNetDevice> bs)
{
  m_bs = bs;
}

Ptr<WranBaseStationNetDevice> WranBSScheduler::GetBs (void)
{
  return m_bs;
}

bool
WranBSScheduler::CheckForFragmentation (Ptr<WranConnection> connection,
                                    int availableSymbols,
                                    WranPhy::ModulationType modulationType)
{
  NS_LOG_INFO ("BS Scheduler, CheckForFragmentation");
  if (connection->GetType () != Cid::TRANSPORT)
    {
      NS_LOG_INFO ("\t No Transport connction, Fragmentation IS NOT possible");
      return false;
    }
  uint32_t availableByte = GetBs ()->GetPhy ()->
    GetNrBytes (availableSymbols, modulationType);

  uint32_t headerSize = connection->GetQueue ()->GetFirstPacketHdrSize (
      MacHeaderType::HEADER_TYPE_GENERIC);
  NS_LOG_INFO ("\t availableByte = " << availableByte <<
               " headerSize = " << headerSize);

  if (availableByte > headerSize)
    {
      NS_LOG_INFO ("\t Fragmentation IS possible");
      return true;
    }
  else
    {
      NS_LOG_INFO ("\t Fragmentation IS NOT possible");
      return false;
    }
}
} // namespace ns3
