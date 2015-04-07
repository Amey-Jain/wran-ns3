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

#include <stdint.h>
#include "wran-ss-manager.h"
#include "ns3/log.h"
#include "wran-service-flow.h"

NS_LOG_COMPONENT_DEFINE ("WranSSManager");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (WranSSManager);

TypeId WranSSManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranSSManager")
    .SetParent<Object> ();
  return tid;
}

WranSSManager::WranSSManager (void)
{
  m_ssRecords = new std::vector<WranSSRecord*> ();
}

WranSSManager::~WranSSManager (void)
{
  for (std::vector<WranSSRecord*>::iterator iter = m_ssRecords->begin (); iter != m_ssRecords->end (); ++iter)
    {
      delete *iter;
    }
  delete m_ssRecords;
  m_ssRecords = 0;
}

WranSSRecord*
WranSSManager::CreateWranSSRecord (const Mac48Address &macAddress)
{
  WranSSRecord *ssRecord = new WranSSRecord (macAddress);
  m_ssRecords->push_back (ssRecord);
  return ssRecord;
}

WranSSRecord*
WranSSManager::GetWranSSRecord (const Mac48Address &macAddress) const
{
  for (std::vector<WranSSRecord*>::iterator iter = m_ssRecords->begin (); iter != m_ssRecords->end (); ++iter)
    {
      if ((*iter)->GetMacAddress () == macAddress)
        {
          return *iter;
        }
    }

  NS_LOG_DEBUG ("GetWranSSRecord: WranSSRecord not found!");
  return 0;
}

WranSSRecord*
WranSSManager::GetWranSSRecord (Cid cid) const
{
  for (std::vector<WranSSRecord*>::iterator iter1 = m_ssRecords->begin (); iter1 != m_ssRecords->end (); ++iter1)
    {
      WranSSRecord *ssRecord = *iter1;
      if (ssRecord->GetBasicCid () == cid || ssRecord->GetPrimaryCid () == cid)
        {
          return ssRecord;
        }
      else
        {
          std::vector<WranServiceFlow*> sf = ssRecord->GetWranServiceFlows (WranServiceFlow::SF_TYPE_ALL);
          for (std::vector<WranServiceFlow*>::iterator iter2 = sf.begin (); iter2 != sf.end (); ++iter2)
            {
              if ((*iter2)->GetConnection ()->GetCid () == cid)
                {
                  return ssRecord;
                }
            }
        }
    }

  NS_LOG_DEBUG ("GetWranSSRecord: WranSSRecord not found!");
  return 0;
}

std::vector<WranSSRecord*>*
WranSSManager::GetWranSSRecords (void) const
{
  return m_ssRecords;
}

bool
WranSSManager::IsInRecord (const Mac48Address &macAddress) const
{
  for (std::vector<WranSSRecord*>::iterator iter = m_ssRecords->begin (); iter != m_ssRecords->end (); ++iter)
    {
      if ((*iter)->GetMacAddress () == macAddress)
        {
          return true;
        }
    }
  return false;
}

bool
WranSSManager::IsRegistered (const Mac48Address &macAddress) const
{
  WranSSRecord *ssRecord = GetWranSSRecord (macAddress);
  return ssRecord != 0 && ssRecord->GetRangingStatus () == WranNetDevice::RANGING_STATUS_SUCCESS;
}

void
WranSSManager::DeleteWranSSRecord (Cid cid)
{
  for (std::vector<WranSSRecord*>::iterator iter1 = m_ssRecords->begin (); iter1 != m_ssRecords->end (); ++iter1)
    {
      WranSSRecord *ssRecord = *iter1;
      if (ssRecord->GetBasicCid () == cid || ssRecord->GetPrimaryCid () == cid)
        {
          m_ssRecords->erase (iter1);
          return;
        }
      else
        {
          std::vector<WranServiceFlow*> sf = ssRecord->GetWranServiceFlows (WranServiceFlow::SF_TYPE_ALL);
          for (std::vector<WranServiceFlow*>::const_iterator iter2 = sf.begin (); iter2 != sf.end (); ++iter2)
            {
              if ((*iter2)->GetConnection ()->GetCid () == cid)
                {
                  m_ssRecords->erase (iter1);
                  return;
                }
            }
        }
    }
}

Mac48Address
WranSSManager::GetMacAddress (Cid cid) const
{
  return GetWranSSRecord (cid)->GetMacAddress ();
}

uint32_t
WranSSManager::GetNSSs (void) const
{
  return m_ssRecords->size ();
}

uint32_t
WranSSManager::GetNRegisteredSSs (void) const
{
  uint32_t nrSS = 0;
  for (std::vector<WranSSRecord*>::iterator iter = m_ssRecords->begin (); iter != m_ssRecords->end (); ++iter)
    {
      if ((*iter)->GetRangingStatus () == WranNetDevice::RANGING_STATUS_SUCCESS)
        {
          nrSS++;
        }
    }
  return nrSS;
}

} // namespace ns3


