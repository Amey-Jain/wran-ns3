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

#ifndef UPLINK_SCHEDULER_H
#define UPLINK_SCHEDULER_H

#include <stdint.h>
#include "ns3/ul-mac-messages.h"
#include "ns3/nstime.h"
#include "wran-phy.h"
#include "wran-service-flow-record.h"
#include "wran-service-flow.h"

namespace ns3 {

class WranBaseStationNetDevice;
class WranSSRecord;
class WranServiceFlow;
class WranServiceFlowRecord;

/**
 * \ingroup wran
 * \brief Virtual class for uplink scheduler.
 */
class WranUplinkScheduler : public Object
{
public:
  WranUplinkScheduler (void);
  WranUplinkScheduler (Ptr<WranBaseStationNetDevice> bs);
  virtual ~WranUplinkScheduler (void);

  static TypeId GetTypeId (void);

  virtual uint8_t GetNrIrOppsAllocated (void) const;
  virtual void SetNrIrOppsAllocated (uint8_t nrIrOppsAllocated);

  virtual bool GetIsIrIntrvlAllocated (void) const;
  virtual void SetIsIrIntrvlAllocated (bool isIrIntrvlAllocated);

  virtual bool GetIsInvIrIntrvlAllocated (void) const;
  virtual void SetIsInvIrIntrvlAllocated (bool isInvIrIntrvlAllocated);

  virtual std::list<OfdmUlMapIe> GetUplinkAllocations (void) const;

  virtual Time GetTimeStampIrInterval (void);
  virtual void SetTimeStampIrInterval (Time timeStampIrInterval);

  virtual Time GetDcdTimeStamp (void) const;
  virtual void SetDcdTimeStamp (Time dcdTimeStamp);

  virtual Time GetUcdTimeStamp (void) const;
  virtual void SetUcdTimeStamp (Time ucdTimeStamp);

  virtual Ptr<WranBaseStationNetDevice> GetBs (void);
  virtual void SetBs (Ptr<WranBaseStationNetDevice> bs);
  /**
   * Determines if channel descriptors sent in the current frame are
   * required to be updated
   */
  virtual void GetChannelDescriptorsToUpdate (bool&, bool&, bool&, bool&) = 0;
  virtual uint32_t CalculateAllocationStartTime (void) = 0;
  virtual void AddUplinkAllocation (OfdmUlMapIe &ulMapIe,
                                    const uint32_t &allocationSize,
                                    uint32_t &symbolsToAllocation,
                                    uint32_t &availableSymbols) = 0;
  virtual void Schedule (void) = 0;
  virtual void ServiceUnsolicitedGrants (const WranSSRecord *ssRecord,
                                         enum WranServiceFlow::SchedulingType schedulingType,
                                         OfdmUlMapIe &ulMapIe,
                                         const WranPhy::ModulationType modulationType,
                                         uint32_t &symbolsToAllocation,
                                         uint32_t &availableSymbols) = 0;
  virtual void ServiceBandwidthRequests (const WranSSRecord *ssRecord,
                                         enum WranServiceFlow::SchedulingType schedulingType,
                                         OfdmUlMapIe &ulMapIe,
                                         const WranPhy::ModulationType modulationType,
                                         uint32_t &symbolsToAllocation,
                                         uint32_t &availableSymbols) = 0;
  virtual bool ServiceBandwidthRequests (WranServiceFlow *serviceFlow,
                                         enum WranServiceFlow::SchedulingType schedulingType,
                                         OfdmUlMapIe &ulMapIe,
                                         const WranPhy::ModulationType modulationType,
                                         uint32_t &symbolsToAllocation,
                                         uint32_t &availableSymbols) = 0;
  virtual void AllocateInitialRangingInterval (uint32_t &symbolsToAllocation, uint32_t &availableSymbols) = 0;
  virtual void SetupWranServiceFlow (WranSSRecord *ssRecord, WranServiceFlow *serviceFlow) = 0;
  virtual void ProcessBandwidthRequest (const BandwidthRequestHeader &bwRequestHdr) = 0;

  virtual void InitOnce (void) = 0;

  virtual void OnSetRequestedBandwidth (WranServiceFlowRecord *sfr) = 0;

private:
  Ptr<WranBaseStationNetDevice> m_bs;
  std::list<OfdmUlMapIe> m_uplinkAllocations;
  Time m_timeStampIrInterval;
  uint8_t m_nrIrOppsAllocated;
  bool m_isIrIntrvlAllocated;
  bool m_isInvIrIntrvlAllocated;
  Time m_dcdTimeStamp;
  Time m_ucdTimeStamp;
};

} // namespace ns3

#endif /* UPLINK_SCHEDULER_H */
