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

#ifndef BURST_PROFILE_MANAGER_H
#define BURST_PROFILE_MANAGER_H

#include <stdint.h>
#include "ns3/cid.h"
#include "wran-phy.h"
#include "wran-net-device.h"

namespace ns3 {

class WranSSRecord;
class RngReq;

/**
 * \ingroup wran
 */
class WranBurstProfileManager : public Object
{
public:
  static TypeId GetTypeId (void);
  WranBurstProfileManager (Ptr<WranNetDevice> device);
  ~WranBurstProfileManager (void);
  void DoDispose (void);
  /*
   * \returns the number of available burst profile
   */
  uint16_t GetNrBurstProfilesToDefine (void);

  /*
   * \brief returns the modulation type of a given iuc
   * \param direction should be uplink or downlink
   * \param iuc the iuc
   * \returns the modulation type of the selected iuc
   */
  WranPhy::ModulationType GetModulationType (uint8_t iuc,
                                              WranNetDevice::Direction direction) const;

  uint8_t GetBurstProfile (WranPhy::ModulationType modulationType,
                           WranNetDevice::Direction direction) const;

  /*
   * \brief during initial ranging or periodic ranging (or when RNG-REQ is used instead of
   * DBPC) calculates the least robust burst profile for SS, e.g., based on distance,
   * power, signal etc
   *
   */
  uint8_t GetBurstProfileForSS (const WranSSRecord *ssRecord, const RngReq *rngreq,
                                WranPhy::ModulationType &modulationType);
  WranPhy::ModulationType GetModulationTypeForSS (const WranSSRecord *ssRecord,
                                                   const RngReq *rngreq);
  uint8_t GetBurstProfileToRequest (void);
private:
  WranBurstProfileManager (const WranBurstProfileManager &);
  WranBurstProfileManager& operator= (const WranBurstProfileManager &);

  Ptr<WranNetDevice> m_device;
};

} // namespace ns3

#endif /* BURST_PROFILE_MANAGER_H */
