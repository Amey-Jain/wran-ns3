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
#include "wran-net-device.h"
#include "wran-bs-net-device.h"
#include "wran-ss-net-device.h"
#include "ns3/wran-ss-record.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "wran-connection.h"
#include "wran-ss-manager.h"
#include "wran-connection-manager.h"
#include "ns3/wran-bs-uplink-scheduler.h"
#include "wran-ss-scheduler.h"
#include "ns3/buffer.h"
#include "wran-service-flow-record.h"
NS_LOG_COMPONENT_DEFINE ("WranServiceFlowManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranServiceFlowManager);

TypeId WranServiceFlowManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranServiceFlowManager")
    .SetParent<Object> ();
  return tid;
}

WranServiceFlowManager::WranServiceFlowManager ()
{
  m_serviceFlows = new std::vector<WranServiceFlow*>;
}

WranServiceFlowManager::~WranServiceFlowManager (void)
{
}

void
WranServiceFlowManager::DoDispose (void)
{
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      delete (*iter);
    }
  m_serviceFlows->clear ();
  delete m_serviceFlows;
}

void
WranServiceFlowManager::AddWranServiceFlow (WranServiceFlow *serviceFlow)
{
  m_serviceFlows->push_back (serviceFlow);
}

WranServiceFlow* WranServiceFlowManager::DoClassify (Ipv4Address srcAddress,
                                             Ipv4Address dstAddress,
                                             uint16_t srcPort,
                                             uint16_t dstPort,
                                             uint8_t proto,
                                             WranServiceFlow::Direction dir) const
{
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if ((*iter)->GetDirection () == dir)
        {
          if ((*iter)->CheckClassifierMatch (srcAddress, dstAddress, srcPort, dstPort, proto))
            {
              return (*iter);
            }
        }
    }
  return 0;
}

WranServiceFlow*
WranServiceFlowManager::GetWranServiceFlow (uint32_t sfid) const
{
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if ((*iter)->GetSfid () == sfid)
        {
          return (*iter);
        }
    }

  NS_LOG_DEBUG ("GetWranServiceFlow: service flow not found!");
  return 0;
}

WranServiceFlow*
WranServiceFlowManager::GetWranServiceFlow (Cid cid) const
{
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if ((*iter)->GetCid () == cid.GetIdentifier ())
        {
          return (*iter);
        }
    }

  NS_LOG_DEBUG ("GetWranServiceFlow: service flow not found!");
  return 0;
}

std::vector<WranServiceFlow*>
WranServiceFlowManager::GetWranServiceFlows (enum WranServiceFlow::SchedulingType schedulingType) const
{
  std::vector<WranServiceFlow*> tmpWranServiceFlows;
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if (((*iter)->GetSchedulingType () == schedulingType) || (schedulingType == WranServiceFlow::SF_TYPE_ALL))
        {
          tmpWranServiceFlows.push_back ((*iter));
        }
    }
  return tmpWranServiceFlows;
}

bool
WranServiceFlowManager::AreWranServiceFlowsAllocated ()
{
  return AreWranServiceFlowsAllocated (m_serviceFlows);
}

bool
WranServiceFlowManager::AreWranServiceFlowsAllocated (std::vector<WranServiceFlow*>* serviceFlowVector)
{
  return AreWranServiceFlowsAllocated (*serviceFlowVector);
}

bool
WranServiceFlowManager::AreWranServiceFlowsAllocated (std::vector<WranServiceFlow*> serviceFlowVector)
{
  for (std::vector<WranServiceFlow*>::const_iterator iter = serviceFlowVector.begin (); iter != serviceFlowVector.end (); ++iter)
    {
      if (!(*iter)->GetIsEnabled ())
        {
          return false;
        }
    }
  return true;
}

WranServiceFlow*
WranServiceFlowManager::GetNextWranServiceFlowToAllocate ()
{
  std::vector<WranServiceFlow*>::iterator iter;
  for (iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if (!(*iter)->GetIsEnabled ())
        {
          return (*iter);
        }
    }
  return 0;
}

uint32_t
WranServiceFlowManager::GetNrWranServiceFlows (void) const
{
  return m_serviceFlows->size ();
}

} // namespace ns3
