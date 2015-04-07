/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Green Network Research Group
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
NS_LOG_COMPONENT_DEFINE ("WranSSServiceFlowManager");

namespace ns3 {

WranSSServiceFlowManager::WranSSServiceFlowManager (Ptr<WranSubscriberStationNetDevice> device)
  : m_device (device),
    m_maxDsaReqRetries (100),
    m_dsaReq (DsaReq ()),
    m_dsaAck (DsaAck ()),
    m_currentTransactionId (0),
    m_transactionIdIndex (1),
    m_dsaReqRetries (0),
    m_pendingWranServiceFlow (0)
{
}

WranSSServiceFlowManager::~WranSSServiceFlowManager (void)
{
}

void
WranSSServiceFlowManager::DoDispose (void)
{
  WranServiceFlowManager::DoDispose ();
}

void
WranSSServiceFlowManager::SetMaxDsaReqRetries (uint8_t maxDsaReqRetries)
{
  m_maxDsaReqRetries = maxDsaReqRetries;
}

uint8_t
WranSSServiceFlowManager::GetMaxDsaReqRetries (void) const
{
  return m_maxDsaReqRetries;
}

EventId
WranSSServiceFlowManager::GetDsaRspTimeoutEvent (void) const
{
  return m_dsaRspTimeoutEvent;
}

EventId
WranSSServiceFlowManager::GetDsaAckTimeoutEvent (void) const
{
  return m_dsaAckTimeoutEvent;
}

void
WranSSServiceFlowManager::AddWranServiceFlow (WranServiceFlow serviceFlow)
{
  WranServiceFlow * sf = new WranServiceFlow ();
  sf->CopyParametersFrom (serviceFlow);
  WranServiceFlowManager::AddWranServiceFlow (sf);
}

void
WranSSServiceFlowManager::AddWranServiceFlow (WranServiceFlow *serviceFlow)
{
  WranServiceFlowManager::AddWranServiceFlow (serviceFlow);
}


void
WranSSServiceFlowManager::InitiateWranServiceFlows (void)
{
  WranServiceFlow *serviceFlow = GetNextWranServiceFlowToAllocate ();
  NS_ASSERT_MSG (serviceFlow != 0,"Error while initiating a new service flow: All service flows have been initiated");
  m_pendingWranServiceFlow = serviceFlow;
  ScheduleDsaReq (m_pendingWranServiceFlow);
}

DsaReq
WranSSServiceFlowManager::CreateDsaReq (const WranServiceFlow *serviceFlow)
{
  DsaReq dsaReq;
  dsaReq.SetTransactionId (m_transactionIdIndex);
  m_currentTransactionId = m_transactionIdIndex++;

  /*as it is SS-initiated DSA therefore SFID and CID will
   not be included, see 6.3.2.3.10.1 and 6.3.2.3.11.1*/
  dsaReq.SetWranServiceFlow (*serviceFlow);
  // dsaReq.SetParameterSet (*serviceFlow->GetParameterSet ());
  return dsaReq;
}

Ptr<Packet>
WranSSServiceFlowManager::CreateDsaAck (void)
{
  DsaAck dsaAck;
  dsaAck.SetTransactionId (m_dsaReq.GetTransactionId ());
  dsaAck.SetConfirmationCode (CONFIRMATION_CODE_SUCCESS);
  m_dsaAck = dsaAck;
  Ptr<Packet> p = Create<Packet> ();
  p->AddHeader (dsaAck);
  p->AddHeader (WranManagementMessageType (WranManagementMessageType::MESSAGE_TYPE_DSA_ACK));
  return p;
}

void
WranSSServiceFlowManager::ScheduleDsaReq (const WranServiceFlow *serviceFlow)
{
  Ptr<Packet> p = Create<Packet> ();
  DsaReq dsaReq;
  Ptr<WranSubscriberStationNetDevice> ss = m_device->GetObject<WranSubscriberStationNetDevice> ();

  if (m_dsaReqRetries == 0)
    {
      dsaReq = CreateDsaReq (serviceFlow);
      p->AddHeader (dsaReq);
      m_dsaReq = dsaReq;
    }
  else
    {
      if (m_dsaReqRetries <= m_maxDsaReqRetries)
        {
          p->AddHeader (m_dsaReq);
        }
      else
        {
          NS_LOG_DEBUG ("Service flows could not be initialized!");
        }
    }

  m_dsaReqRetries++;
  p->AddHeader (WranManagementMessageType (WranManagementMessageType::MESSAGE_TYPE_DSA_REQ));

  if (m_dsaRspTimeoutEvent.IsRunning ())
    {
      Simulator::Cancel (m_dsaRspTimeoutEvent);
    }

  m_dsaRspTimeoutEvent = Simulator::Schedule (ss->GetIntervalT7 (),
                                              &WranSSServiceFlowManager::ScheduleDsaReq,
                                              this,
                                              serviceFlow);

  m_device->Enqueue (p, MacHeaderType (), ss->GetPrimaryConnection ());
}


void
WranSSServiceFlowManager::ProcessDsaRsp (const DsaRsp &dsaRsp)
{

  Ptr<WranSubscriberStationNetDevice> ss = m_device->GetObject<WranSubscriberStationNetDevice> ();

  // already received DSA-RSP for that particular DSA-REQ
  if (dsaRsp.GetTransactionId () != m_currentTransactionId)
    {
      return;
    }

  Ptr<Packet> dsaAck = CreateDsaAck ();
  m_device->Enqueue (dsaAck, MacHeaderType (), ss->GetPrimaryConnection ());

  m_dsaReqRetries = 0;
  if (m_pendingWranServiceFlow == NULL)
    {
      // May be the DSA-ACK was not received by the SS
      return;
    }
  WranServiceFlow sf = dsaRsp.GetWranServiceFlow ();
  (*m_pendingWranServiceFlow) = sf;
  m_pendingWranServiceFlow->SetUnsolicitedGrantInterval (1);
  m_pendingWranServiceFlow->SetUnsolicitedPollingInterval (1);
  Ptr<WranConnection> transportConnection = CreateObject<WranConnection> (sf.GetCid (),
                                                                            Cid::TRANSPORT);

  m_pendingWranServiceFlow->SetConnection (transportConnection);
  transportConnection->SetWranServiceFlow (m_pendingWranServiceFlow);
  ss->GetWranConnectionManager ()->AddConnection (transportConnection,
                                              Cid::TRANSPORT);
  m_pendingWranServiceFlow->SetIsEnabled (true);
  m_pendingWranServiceFlow = 0;
  // check if all service flow have been initiated
  WranServiceFlow * serviceFlow = GetNextWranServiceFlowToAllocate ();
  if (serviceFlow == 0)
    {
      ss->SetAreWranServiceFlowsAllocated (true);
    }
  else
    {
      m_pendingWranServiceFlow = serviceFlow;
      ScheduleDsaReq (m_pendingWranServiceFlow);
    }
}

} // namespace ns3
