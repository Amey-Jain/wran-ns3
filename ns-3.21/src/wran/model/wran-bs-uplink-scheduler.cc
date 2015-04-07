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

#include "wran-bs-uplink-scheduler.h"
#include "wran-bs-net-device.h"
#include "ns3/simulator.h"
#include "ns3/cid.h"
#include "wran-burst-profile-manager.h"
#include "wran-ss-manager.h"
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "wran-ss-record.h"
#include "wran-service-flow.h"
#include "wran-service-flow-record.h"
#include "wran-bs-link-manager.h"
#include "wran-bandwidth-manager.h"

NS_LOG_COMPONENT_DEFINE ("WranUplinkScheduler");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranUplinkScheduler);

WranUplinkScheduler::WranUplinkScheduler (void)
  : m_bs (0),
    m_timeStampIrInterval (Seconds (0)),
    m_nrIrOppsAllocated (0),
    m_isIrIntrvlAllocated (false),
    m_isInvIrIntrvlAllocated (false),
    m_dcdTimeStamp (Simulator::Now ()),
    m_ucdTimeStamp (Simulator::Now ())
{
}

WranUplinkScheduler::WranUplinkScheduler (Ptr<WranBaseStationNetDevice> bs)
  : m_bs (bs),
    m_timeStampIrInterval (Seconds (0)),
    m_nrIrOppsAllocated (0),
    m_isIrIntrvlAllocated (false),
    m_isInvIrIntrvlAllocated (false),
    m_dcdTimeStamp (Simulator::Now ()),
    m_ucdTimeStamp (Simulator::Now ())
{
}

WranUplinkScheduler::~WranUplinkScheduler (void)
{
  m_bs = 0;
  m_uplinkAllocations.clear ();
}
void
WranUplinkScheduler::InitOnce ()
{

}

TypeId
WranUplinkScheduler::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranUplinkScheduler")
    .SetParent<Object> ()
  ;
  return tid;
}

uint8_t
WranUplinkScheduler::GetNrIrOppsAllocated (void) const
{
  return m_nrIrOppsAllocated;
}

void
WranUplinkScheduler::SetNrIrOppsAllocated (uint8_t nrIrOppsAllocated)
{
  m_nrIrOppsAllocated = nrIrOppsAllocated;
}

bool
WranUplinkScheduler::GetIsIrIntrvlAllocated (void) const
{
  return m_isIrIntrvlAllocated;
}

void
WranUplinkScheduler::SetIsIrIntrvlAllocated (bool isIrIntrvlAllocated)
{

  m_isIrIntrvlAllocated = isIrIntrvlAllocated;
}

bool
WranUplinkScheduler::GetIsInvIrIntrvlAllocated (void) const
{

  return m_isInvIrIntrvlAllocated;
}

void
WranUplinkScheduler::SetIsInvIrIntrvlAllocated (bool isInvIrIntrvlAllocated)
{
  m_isInvIrIntrvlAllocated = isInvIrIntrvlAllocated;
}

Time
WranUplinkScheduler::GetDcdTimeStamp (void) const
{
  return m_dcdTimeStamp;
}

void
WranUplinkScheduler::SetDcdTimeStamp (Time dcdTimeStamp)
{
  m_dcdTimeStamp = dcdTimeStamp;
}

Time
WranUplinkScheduler::GetUcdTimeStamp (void) const
{
  return m_ucdTimeStamp;
}

void
WranUplinkScheduler::SetUcdTimeStamp (Time ucdTimeStamp)
{
  m_ucdTimeStamp = ucdTimeStamp;
}

std::list<OfdmUlMapIe>
WranUplinkScheduler::GetUplinkAllocations (void) const
{
  return m_uplinkAllocations;
}

Time
WranUplinkScheduler::GetTimeStampIrInterval (void)
{
  return m_timeStampIrInterval;
}

void
WranUplinkScheduler::SetTimeStampIrInterval (Time timeStampIrInterval)
{
  m_timeStampIrInterval = timeStampIrInterval;
}

Ptr<WranBaseStationNetDevice>
WranUplinkScheduler::GetBs (void)
{
  return m_bs;
}
void
WranUplinkScheduler::SetBs (Ptr<WranBaseStationNetDevice> bs)
{
  m_bs = bs;
}
} // namespace ns3
