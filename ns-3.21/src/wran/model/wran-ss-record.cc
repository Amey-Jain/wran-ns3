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
 * Author: Sayef Azad Sakin <sayefsakin@gmail.com>
 *          
 */

#include "wran-ss-record.h"
#include "wran-service-flow.h"
#include <stdint.h>

namespace ns3 {

WranSSRecord::WranSSRecord (void)
{
  Initialize ();
}

WranSSRecord::WranSSRecord (Mac48Address macAddress)
{
  m_macAddress = macAddress;
  Initialize ();
}

WranSSRecord::WranSSRecord (Mac48Address macAddress, Ipv4Address IPaddress)
{
  m_macAddress = macAddress;
  m_IPAddress = IPaddress;
  Initialize ();
}

void
WranSSRecord::Initialize (void)
{
  m_basicCid = Cid ();
  m_primaryCid = Cid ();

  m_rangingCorrectionRetries = 0;
  m_invitedRangingRetries = 0;
  m_modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  m_rangingStatus = WranNetDevice::RANGING_STATUS_EXPIRED;
  m_pollForRanging = false;
  m_areWranServiceFlowsAllocated = false;
  m_pollMeBit = false;

  m_sfTransactionId = 0;
  m_dsaRspRetries = 0;

  m_serviceFlows = new std::vector<WranServiceFlow*> ();
  m_dsaRsp = DsaRsp ();
  m_broadcast = 0;
}

WranSSRecord::~WranSSRecord (void)
{
  delete m_serviceFlows;
  m_serviceFlows = 0;
}

void
WranSSRecord::SetIPAddress (Ipv4Address IPAddress)
{
  m_IPAddress = IPAddress;
}

Ipv4Address WranSSRecord::GetIPAddress (void)
{
  return m_IPAddress;
}

void
WranSSRecord::SetBasicCid (Cid basicCid)
{
  m_basicCid = basicCid;
}

Cid
WranSSRecord::GetBasicCid (void) const
{
  return m_basicCid;
}

void
WranSSRecord::SetPrimaryCid (Cid primaryCid)
{
  m_primaryCid = primaryCid;
}

Cid
WranSSRecord::GetPrimaryCid (void) const
{
  return m_primaryCid;
}

void
WranSSRecord::SetMacAddress (Mac48Address macAddress)
{
  m_macAddress = macAddress;
}

Mac48Address
WranSSRecord::GetMacAddress (void) const
{
  return m_macAddress;
}

uint8_t
WranSSRecord::GetRangingCorrectionRetries (void) const
{
  return m_rangingCorrectionRetries;
}

void
WranSSRecord::ResetRangingCorrectionRetries (void)
{
  m_rangingCorrectionRetries = 0;
}

void
WranSSRecord::IncrementRangingCorrectionRetries (void)
{
  m_rangingCorrectionRetries++;
}

uint8_t
WranSSRecord::GetInvitedRangRetries (void) const
{
  return m_invitedRangingRetries;
}

void
WranSSRecord::ResetInvitedRangingRetries (void)
{
  m_invitedRangingRetries = 0;
}

void
WranSSRecord::IncrementInvitedRangingRetries (void)
{
  m_invitedRangingRetries++;
}

void
WranSSRecord::SetModulationType (WranPhy::ModulationType modulationType)
{
  m_modulationType = modulationType;
}

WranPhy::ModulationType
WranSSRecord::GetModulationType (void) const
{
  return m_modulationType;
}

void
WranSSRecord::SetRangingStatus (WranNetDevice::RangingStatus rangingStatus)
{
  m_rangingStatus = rangingStatus;
}

WranNetDevice::RangingStatus
WranSSRecord::GetRangingStatus (void) const
{
  return m_rangingStatus;
}

void
WranSSRecord::EnablePollForRanging (void)
{
  m_pollForRanging = true;
}

void
WranSSRecord::DisablePollForRanging (void)
{
  m_pollForRanging = false;
}

bool
WranSSRecord::GetPollForRanging (void) const
{
  return m_pollForRanging;
}

void
WranSSRecord::SetAreWranServiceFlowsAllocated (bool val)
{
  m_areWranServiceFlowsAllocated = val;
}

bool
WranSSRecord::GetAreWranServiceFlowsAllocated (void) const
{
  return m_areWranServiceFlowsAllocated;
}

void
WranSSRecord::SetPollMeBit (bool pollMeBit)
{
  m_pollMeBit = pollMeBit;
}

bool
WranSSRecord::GetPollMeBit (void) const
{
  return m_pollMeBit;
}

void
WranSSRecord::AddWranServiceFlow (WranServiceFlow *serviceFlow)
{
  m_serviceFlows->push_back (serviceFlow);
}

std::vector<WranServiceFlow*> WranSSRecord::GetWranServiceFlows (enum WranServiceFlow::SchedulingType schedulingType) const
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

void
WranSSRecord::SetIsBroadcastSS (bool broadcast_enable)
{
  m_broadcast = broadcast_enable;
}

bool
WranSSRecord::GetIsBroadcastSS (void)
{
  return m_broadcast;
}

bool
WranSSRecord::GetHasWranServiceFlowUgs (void) const
{
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if ((*iter)->GetSchedulingType () == WranServiceFlow::SF_TYPE_UGS)
        {
          return true;
        }
    }
  return false;
}

bool
WranSSRecord::GetHasWranServiceFlowRtps (void) const
{
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if ((*iter)->GetSchedulingType () == WranServiceFlow::SF_TYPE_RTPS)
        {
          return true;
        }
    }
  return false;
}

bool
WranSSRecord::GetHasWranServiceFlowNrtps (void) const
{
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if ((*iter)->GetSchedulingType () == WranServiceFlow::SF_TYPE_NRTPS)
        {
          return true;
        }
    }
  return false;
}

bool
WranSSRecord::GetHasWranServiceFlowBe (void) const
{
  for (std::vector<WranServiceFlow*>::iterator iter = m_serviceFlows->begin (); iter != m_serviceFlows->end (); ++iter)
    {
      if ((*iter)->GetSchedulingType () == WranServiceFlow::SF_TYPE_BE)
        {
          return true;
        }
    }
  return false;
}

void
WranSSRecord::SetSfTransactionId (uint16_t sfTransactionId)
{
  m_sfTransactionId = sfTransactionId;
}

uint16_t WranSSRecord::GetSfTransactionId (void) const
{
  return m_sfTransactionId;
}

void
WranSSRecord::SetDsaRspRetries (uint8_t dsaRspRetries)
{
  m_dsaRspRetries = dsaRspRetries;
}

void
WranSSRecord::IncrementDsaRspRetries (void)
{
  m_dsaRspRetries++;
}

uint8_t
WranSSRecord::GetDsaRspRetries (void) const
{
  return m_dsaRspRetries;
}

void
WranSSRecord::SetDsaRsp (DsaRsp dsaRsp)
{
  m_dsaRsp = dsaRsp;
}

DsaRsp
WranSSRecord::GetDsaRsp (void) const
{
  return m_dsaRsp;
}

} // namespace ns3
