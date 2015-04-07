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
 * Authors: Sayef Azad Sakin <sayefsakin@gmail.com>
 *           
 *                                
 */

#include "wran-connection-manager.h"
#include "ns3/log.h"
#include "ns3/cid-factory.h"
#include "wran-ss-record.h"
#include "wran-mac-messages.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "wran-service-flow.h"
#include "wran-ss-net-device.h"
#include "wran-bs-net-device.h"

NS_LOG_COMPONENT_DEFINE ("WranConnectionManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranConnectionManager);

TypeId WranConnectionManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranConnectionManager")
    .SetParent<Object> ();
  return tid;
}

WranConnectionManager::WranConnectionManager (void)
  : m_cidFactory (0)
{
}

void
WranConnectionManager::DoDispose (void)
{
}

WranConnectionManager::~WranConnectionManager (void)
{
}

void
WranConnectionManager::SetCidFactory (CidFactory *cidFactory)
{
  m_cidFactory = cidFactory;
}

void
WranConnectionManager::AllocateManagementConnections (WranSSRecord *ssRecord, RngRsp *rngrsp)
{
  Ptr<WranConnection> basicConnection = CreateConnection (Cid::BASIC);
  ssRecord->SetBasicCid (basicConnection->GetCid ());

  Ptr<WranConnection> primaryConnection = CreateConnection (Cid::PRIMARY);
  ssRecord->SetPrimaryCid (primaryConnection->GetCid ());

  rngrsp->SetBasicCid (basicConnection->GetCid ());
  rngrsp->SetPrimaryCid (primaryConnection->GetCid ());
}

Ptr<WranConnection>
WranConnectionManager::CreateConnection (Cid::Type type)
{
  Cid cid;
  switch (type)
    {
    case Cid::BASIC:
    case Cid::MULTICAST:
    case Cid::PRIMARY:
      cid = m_cidFactory->Allocate (type);
      break;
    case Cid::TRANSPORT:
      cid = m_cidFactory->AllocateTransportOrSecondary ();
      break;
    default:
      NS_FATAL_ERROR ("Invalid connection type");
      break;
    }
  Ptr<WranConnection> connection = CreateObject<WranConnection> (cid, type);
  AddConnection (connection, type);
  return connection;
}

void
WranConnectionManager::AddConnection (Ptr<WranConnection> connection, Cid::Type type)
{
  switch (type)
    {
    case Cid::BASIC:
      m_basicConnections.push_back (connection);
      break;
    case Cid::PRIMARY:
      m_primaryConnections.push_back (connection);
      break;
    case Cid::TRANSPORT:
      m_transportConnections.push_back (connection);
      break;
    case Cid::MULTICAST:
      m_multicastConnections.push_back (connection);
      break;
    default:
      NS_FATAL_ERROR ("Invalid connection type");
      break;
    }
}

Ptr<WranConnection>
WranConnectionManager::GetConnection (Cid cid)
{
  std::vector<Ptr<WranConnection> >::const_iterator iter;

  for (iter = m_basicConnections.begin (); iter != m_basicConnections.end (); ++iter)
    {
      if ((*iter)->GetCid () == cid)
        {
          return *iter;
        }
    }

  for (iter = m_primaryConnections.begin (); iter != m_primaryConnections.end (); ++iter)
    {
      if ((*iter)->GetCid () == cid)
        {
          return *iter;
        }
    }

  for (iter = m_transportConnections.begin (); iter != m_transportConnections.end (); ++iter)
    {
      if ((*iter)->GetCid () == cid)
        {
          return *iter;
        }
    }

  return 0;
}

std::vector<Ptr<WranConnection> >
WranConnectionManager::GetConnections (Cid::Type type) const
{
  std::vector<Ptr<WranConnection> > connections;

  switch (type)
    {
    case Cid::BASIC:
      connections = m_basicConnections;
      break;
    case Cid::PRIMARY:
      connections = m_primaryConnections;
      break;
    case Cid::TRANSPORT:
      connections = m_transportConnections;
      break;
    default:
      NS_FATAL_ERROR ("Invalid connection type");
      break;
    }

  return connections;
}

uint32_t
WranConnectionManager::GetNPackets (Cid::Type type, WranServiceFlow::SchedulingType schedulingType) const
{
  uint32_t nrPackets = 0;

  switch (type)
    {
    case Cid::BASIC:
      {
        for (std::vector<Ptr<WranConnection> >::const_iterator iter = m_basicConnections.begin (); iter
             != m_basicConnections.end (); ++iter)
          {
            nrPackets += (*iter)->GetQueue ()->GetSize ();
          }
        break;
      }
    case Cid::PRIMARY:
      {
        for (std::vector<Ptr<WranConnection> >::const_iterator iter = m_primaryConnections.begin (); iter
             != m_primaryConnections.end (); ++iter)
          {
            nrPackets += (*iter)->GetQueue ()->GetSize ();
          }
        break;
      }
    case Cid::TRANSPORT:
      {
        for (std::vector<Ptr<WranConnection> >::const_iterator iter = m_transportConnections.begin (); iter
             != m_transportConnections.end (); ++iter)
          {
            if (schedulingType == WranServiceFlow::SF_TYPE_ALL || (*iter)->GetSchedulingType () == schedulingType)
              {
                nrPackets += (*iter)->GetQueue ()->GetSize ();
              }
          }
        break;
      }
    default:
      NS_FATAL_ERROR ("Invalid connection type");
      break;
    }

  return nrPackets;
}

bool
WranConnectionManager::HasPackets (void) const
{
  std::vector<Ptr<WranConnection> >::const_iterator iter;
  for (iter = m_basicConnections.begin (); iter != m_basicConnections.end (); ++iter)
    {
      if ((*iter)->HasPackets ())
        {
          return true;
        }
    }

  for (iter = m_primaryConnections.begin (); iter != m_primaryConnections.end (); ++iter)
    {
      if ((*iter)->HasPackets ())
        {
          return true;
        }
    }

  for (iter = m_transportConnections.begin (); iter != m_transportConnections.end (); ++iter)
    {
      if ((*iter)->HasPackets ())
        {
          return true;
        }
    }

  return false;
}
} // namespace ns3


