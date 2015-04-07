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

#ifndef SS_SERVICE_FLOW_MANAGER_H
#define SS_SERVICE_FLOW_MANAGER_H

#include <stdint.h>
#include "ns3/event-id.h"
#include "wran-mac-messages.h"
#include "ns3/buffer.h"
#include "wran-service-flow-manager.h"
#include "wran-ss-net-device.h"

namespace ns3 {

class Packet;
class WranServiceFlow;
class WranNetDevice;
class WranConnection;
class WranSubscriberStationNetDevice;

/**
 * \ingroup wran
 */
class WranSSServiceFlowManager : public WranServiceFlowManager
{
public:
  enum ConfirmationCode // as per Table 384 (not all codes implemented)
  {
    CONFIRMATION_CODE_SUCCESS, CONFIRMATION_CODE_REJECT
  };
  /**
   * \brief creates a service flow manager and attaches it to a device
   * \param  device the device to which the service flow manager will be attached
   */
  WranSSServiceFlowManager (Ptr<WranSubscriberStationNetDevice> device);

  ~WranSSServiceFlowManager (void);
  void DoDispose (void);
  /**
   * \brief add a service flow to the list
   * \param serviceFlow the service flow to add
   */
  void AddWranServiceFlow (WranServiceFlow *serviceFlow);
  /**
   * \brief add a service flow to the list
   * \param serviceFlow the service flow to add
   */
  void AddWranServiceFlow (WranServiceFlow serviceFlow);
  /**
   * \brief sets the maximum retries on DSA request message
   * \param maxDsaReqRetries the maximum retries on DSA request message
   */
  void SetMaxDsaReqRetries (uint8_t maxDsaReqRetries);
  /**
   * \return the maximum retries on DSA request message
   */
  uint8_t GetMaxDsaReqRetries (void) const;

  EventId GetDsaRspTimeoutEvent (void) const;
  EventId GetDsaAckTimeoutEvent (void) const;

  void InitiateWranServiceFlows (void);

  DsaReq CreateDsaReq (const WranServiceFlow *serviceFlow);

  Ptr<Packet> CreateDsaAck (void);

  void ScheduleDsaReq (const WranServiceFlow *serviceFlow);

  void ProcessDsaRsp (const DsaRsp &dsaRsp);


private:
  Ptr<WranSubscriberStationNetDevice> m_device;

  uint8_t m_maxDsaReqRetries;

  EventId m_dsaRspTimeoutEvent;
  EventId m_dsaAckTimeoutEvent;

  DsaReq m_dsaReq;
  DsaAck m_dsaAck;

  uint16_t m_currentTransactionId;
  uint16_t m_transactionIdIndex;
  uint8_t m_dsaReqRetries;

  // pointer to the service flow currently being configured
  WranServiceFlow *m_pendingWranServiceFlow;


};

} // namespace ns3

#endif /* SS_SERVICE_FLOW_MANAGER_H */
