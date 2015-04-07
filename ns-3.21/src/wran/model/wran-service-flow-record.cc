/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015, 2009 INRIA
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

#include "wran-service-flow-record.h"

namespace ns3 {

WranServiceFlowRecord::WranServiceFlowRecord (void)
  : m_grantSize (0),
    m_grantTimeStamp (Seconds (0)),
    m_dlTimeStamp (Seconds (0)),
    m_pktsSent (0),
    m_pktsRcvd (0),
    m_bytesSent (0),
    m_bytesRcvd (0),
    m_requestedBandwidth (0),
    m_grantedBandwidth (0),
    m_bwSinceLastExpiry (0)
{

  m_lastGrantTime = Seconds (0);
  m_backlogged = 0;
  m_backloggedTemp = 0;
  m_grantedBandwidthTemp = 0;
}

WranServiceFlowRecord::~WranServiceFlowRecord (void)
{
}

void
WranServiceFlowRecord::SetGrantSize (uint32_t grantSize)
{
  m_grantSize = grantSize;
}

uint32_t
WranServiceFlowRecord::GetGrantSize (void) const
{
  return m_grantSize;
}

void
WranServiceFlowRecord::SetGrantTimeStamp (Time grantTimeStamp)
{
  m_grantTimeStamp = grantTimeStamp;
}

Time
WranServiceFlowRecord::GetGrantTimeStamp (void) const
{
  return m_grantTimeStamp;
}

void
WranServiceFlowRecord::SetDlTimeStamp (Time dlTimeStamp)
{
  m_dlTimeStamp = dlTimeStamp;
}

Time
WranServiceFlowRecord::GetDlTimeStamp (void) const
{
  return m_dlTimeStamp;
}

void
WranServiceFlowRecord::SetPktsSent (uint32_t pktsSent)
{
  m_pktsSent = pktsSent;
}

void
WranServiceFlowRecord::UpdatePktsSent (uint32_t pktsSent)
{
  m_pktsSent += pktsSent;
}

uint32_t
WranServiceFlowRecord::GetPktsSent (void) const
{
  return m_pktsSent;
}

void
WranServiceFlowRecord::SetPktsRcvd (uint32_t pktsRcvd)
{
  m_pktsRcvd = pktsRcvd;
}

void
WranServiceFlowRecord::UpdatePktsRcvd (uint32_t pktsRcvd)
{
  m_pktsRcvd += pktsRcvd;
}

uint32_t
WranServiceFlowRecord::GetPktsRcvd (void) const
{
  return m_pktsRcvd;
}

void
WranServiceFlowRecord::SetBytesSent (uint32_t bytesSent)
{
  m_bytesSent = bytesSent;
}

void
WranServiceFlowRecord::UpdateBytesSent (uint32_t bytesSent)
{
  m_bytesSent += bytesSent;
}

uint32_t
WranServiceFlowRecord::GetBytesSent (void) const
{
  return m_bytesSent;
}

void
WranServiceFlowRecord::SetBytesRcvd (uint32_t bytesRcvd)
{
  m_bytesRcvd = bytesRcvd;
}

void
WranServiceFlowRecord::UpdateBytesRcvd (uint32_t bytesRcvd)
{
  m_bytesRcvd += bytesRcvd;
}

uint32_t
WranServiceFlowRecord::GetBytesRcvd (void) const
{
  return m_bytesRcvd;
}

void
WranServiceFlowRecord::SetRequestedBandwidth (uint32_t requestedBandwidth)
{
  m_requestedBandwidth = requestedBandwidth;
}
void
WranServiceFlowRecord::UpdateRequestedBandwidth (uint32_t requestedBandwidth)
{
  m_requestedBandwidth += requestedBandwidth;
}

uint32_t
WranServiceFlowRecord::GetRequestedBandwidth (void)
{
  return m_requestedBandwidth;
}

void
WranServiceFlowRecord::SetGrantedBandwidth (uint32_t grantedBandwidth)
{
  m_grantedBandwidth = grantedBandwidth;
}

void
WranServiceFlowRecord::UpdateGrantedBandwidth (uint32_t grantedBandwidth)
{
  m_grantedBandwidth += grantedBandwidth;
}

uint32_t
WranServiceFlowRecord::GetGrantedBandwidth (void)
{
  return m_grantedBandwidth;
}
void
WranServiceFlowRecord::SetGrantedBandwidthTemp (uint32_t grantedBandwidthTemp)
{
  m_grantedBandwidthTemp = grantedBandwidthTemp;
}

void
WranServiceFlowRecord::UpdateGrantedBandwidthTemp (
  uint32_t grantedBandwidthTemp)
{
  m_grantedBandwidthTemp += grantedBandwidthTemp;
}

uint32_t
WranServiceFlowRecord::GetGrantedBandwidthTemp (void)
{
  return m_grantedBandwidthTemp;
}

void
WranServiceFlowRecord::SetLastGrantTime (Time grantTime)
{
  m_lastGrantTime = grantTime;
}

Time
WranServiceFlowRecord::GetLastGrantTime (void) const
{
  return m_lastGrantTime;
}

void
WranServiceFlowRecord::SetBacklogged (uint32_t backlogged)
{
  m_backlogged = backlogged;
}

void
WranServiceFlowRecord::IncreaseBacklogged (uint32_t backlogged)
{
  m_backlogged += backlogged;
}

uint32_t
WranServiceFlowRecord::GetBacklogged (void) const
{
  return m_backlogged;
}

void
WranServiceFlowRecord::SetBackloggedTemp (uint32_t backloggedTemp)
{
  m_backloggedTemp = backloggedTemp;
}

void
WranServiceFlowRecord::IncreaseBackloggedTemp (uint32_t backloggedTemp)
{
  m_backloggedTemp += backloggedTemp;
}

uint32_t
WranServiceFlowRecord::GetBackloggedTemp (void) const
{
  return m_backloggedTemp;
}

void
WranServiceFlowRecord::SetBwSinceLastExpiry (uint32_t bwSinceLastExpiry)
{
  m_bwSinceLastExpiry = bwSinceLastExpiry;
}

void
WranServiceFlowRecord::UpdateBwSinceLastExpiry (uint32_t bwSinceLastExpiry)
{
  m_bwSinceLastExpiry += bwSinceLastExpiry;
}

uint32_t
WranServiceFlowRecord::GetBwSinceLastExpiry (void)
{
  return m_bwSinceLastExpiry;
}

} // namespace ns3
