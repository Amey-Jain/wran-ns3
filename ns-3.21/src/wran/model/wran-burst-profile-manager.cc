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

#include <stdint.h>
#include "wran-burst-profile-manager.h"
#include "wran-bs-net-device.h"
#include "wran-ss-net-device.h"
#include "wran-ss-record.h"
#include "wran-ss-manager.h"
#include "ns3/log.h"
#include "wran-mac-messages.h"

NS_LOG_COMPONENT_DEFINE ("WranBurstProfileManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranBurstProfileManager);

TypeId WranBurstProfileManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranBurstProfileManager")
    .SetParent<Object> ();
  return tid;
}

WranBurstProfileManager::WranBurstProfileManager (Ptr<WranNetDevice> device)
  : m_device (device)
{
}

WranBurstProfileManager::~WranBurstProfileManager (void)
{
  m_device = 0;
}

void
WranBurstProfileManager::DoDispose (void)
{
  m_device = 0;
}


uint16_t WranBurstProfileManager::GetNrBurstProfilesToDefine (void)
{
  /*
   * 7 modulation types
   */
  return 7;
}

WranPhy::ModulationType
WranBurstProfileManager::GetModulationType (uint8_t iuc,
                                        WranNetDevice::Direction direction) const
{
  if (direction == WranNetDevice::DIRECTION_DOWNLINK)
    {
      std::vector<OfdmDlBurstProfile> dlBurstProfiles =
        m_device->GetCurrentDcd ().GetDlBurstProfiles ();
      for (std::vector<OfdmDlBurstProfile>::iterator iter =
             dlBurstProfiles.begin (); iter != dlBurstProfiles.end (); ++iter)
        {
          if (iter->GetDiuc () == iuc)
            {
              return (WranPhy::ModulationType) iter->GetFecCodeType ();
            }
        }
    }
  else
    {
      std::vector<OfdmUlBurstProfile> ulBurstProfiles =
        m_device->GetCurrentUcd ().GetUlBurstProfiles ();
      for (std::vector<OfdmUlBurstProfile>::iterator iter =
             ulBurstProfiles.begin (); iter != ulBurstProfiles.end (); ++iter)
        {
          if (iter->GetUiuc () == iuc)
            {
              return (WranPhy::ModulationType) iter->GetFecCodeType ();
            }
        }
    }

  // burst profile got to be there in DCD/UCD, assuming always all profiles are defined in DCD/UCD
  NS_FATAL_ERROR ("burst profile got to be there in DCD/UCD");

  return (WranPhy::ModulationType) -1;
}

uint8_t
WranBurstProfileManager::GetBurstProfile (
  WranPhy::ModulationType modulationType,
  WranNetDevice::Direction direction) const
{
  if (direction == WranNetDevice::DIRECTION_DOWNLINK)
    {
      std::vector<OfdmDlBurstProfile> dlBurstProfiles =
        m_device->GetCurrentDcd ().GetDlBurstProfiles ();
      for (std::vector<OfdmDlBurstProfile>::iterator iter =
             dlBurstProfiles.begin (); iter != dlBurstProfiles.end (); ++iter)
        {
          if (iter->GetFecCodeType () == modulationType)
            {
              return iter->GetDiuc ();
            }
        }
    }
  else
    {
      std::vector<OfdmUlBurstProfile> ulBurstProfiles =
        m_device->GetCurrentUcd ().GetUlBurstProfiles ();
      for (std::vector<OfdmUlBurstProfile>::iterator iter =
             ulBurstProfiles.begin (); iter != ulBurstProfiles.end (); ++iter)
        {
          if (iter->GetFecCodeType () == modulationType)
            {
              return iter->GetUiuc ();
            }
        }
    }

  // burst profile got to be there in DCD/UCD, assuming always all profiles are defined in DCD/UCD
  NS_FATAL_ERROR ("burst profile got to be there in DCD/UCD");

  return ~0;
}

uint8_t
WranBurstProfileManager::GetBurstProfileForSS (const WranSSRecord *ssRecord,
                                           const RngReq *rngreq, WranPhy::ModulationType &modulationType)
{
  /*during initial ranging or periodic ranging (or when RNG-REQ is used instead of
   DBPC) calculates the least robust burst profile for SS, e.g., based on distance,
   power, signal etc, temporarily choosing same burst profile SS requested in RNG-REQ*/

  modulationType = GetModulationTypeForSS (ssRecord, rngreq);
  return GetBurstProfile (modulationType, WranNetDevice::DIRECTION_DOWNLINK);
}

WranPhy::ModulationType
WranBurstProfileManager::GetModulationTypeForSS (const WranSSRecord *ssRecord, const RngReq *rngreq)
{

  return GetModulationType (rngreq->GetReqDlBurstProfile (),
                            WranNetDevice::DIRECTION_DOWNLINK);
}

uint8_t
WranBurstProfileManager::GetBurstProfileToRequest (void)
{
  /*modulation type is currently set by user in simulation script, shall
   actually be determined based on SS's distance, power, signal etc*/

  return GetBurstProfile (
           m_device->GetObject<WranSubscriberStationNetDevice> ()->GetModulationType (),
           WranNetDevice::DIRECTION_DOWNLINK);
}

} // namespace ns3
