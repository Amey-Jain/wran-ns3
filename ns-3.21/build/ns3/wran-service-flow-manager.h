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

#ifndef SERVICE_FLOW_MANAGER_H
#define SERVICE_FLOW_MANAGER_H

#include <stdint.h>
#include "ns3/event-id.h"
#include "wran-mac-messages.h"
#include "ns3/buffer.h"

namespace ns3 {

class Packet;
class WranServiceFlow;
class WranNetDevice;
class WranSSRecord;
class WranConnection;

/**
 * \ingroup wran
 * The same service flow manager class serves both for BS and SS though some functions are exclusive to only one of them.
 */
class WranServiceFlowManager : public Object
{
public:
  enum ConfirmationCode // as per Table 384 (not all codes implemented)
  {
    CONFIRMATION_CODE_SUCCESS, CONFIRMATION_CODE_REJECT
  };

  static TypeId GetTypeId (void);

  WranServiceFlowManager ();
  ~WranServiceFlowManager (void);
  void DoDispose (void);

  void AddWranServiceFlow (WranServiceFlow * serviceFlow);
  WranServiceFlow* GetWranServiceFlow (uint32_t sfid) const;
  WranServiceFlow* GetWranServiceFlow (Cid cid) const;
  std::vector<WranServiceFlow*> GetWranServiceFlows (enum WranServiceFlow::SchedulingType schedulingType) const;

  /**
   * \return true if all service flows are allocated, false otherwise
   */
  bool AreWranServiceFlowsAllocated ();
  /**
   * \param  serviceFlows the list of the service flows to be checked
   * \return true if all service flows are allocated, false otherwise
   */
  bool AreWranServiceFlowsAllocated (std::vector<WranServiceFlow*>* serviceFlows);
  /**
   * \param  serviceFlows the list of the service flows to be checked
   * \return true if all service flows are allocated, false otherwise
   */
  bool AreWranServiceFlowsAllocated (std::vector<WranServiceFlow*> serviceFlows);
  /**
   * \return the next service flow to be allocated
   */
  WranServiceFlow* GetNextWranServiceFlowToAllocate ();

  /**
   * \return the number of all service flows
   */
  uint32_t GetNrWranServiceFlows (void) const;

  /**
   *\param SrcAddress the source ip address
   *\param DstAddress the destination ip address
   *\param SrcPort the source port
   *\param DstPort the destination port
   *\param Proto the protocol
   *\param dir the direction of the service flow
   *\return the service flow to which this ip flow is associated
   */
  WranServiceFlow* DoClassify (Ipv4Address SrcAddress,
                           Ipv4Address DstAddress,
                           uint16_t SrcPort,
                           uint16_t DstPort,
                           uint8_t Proto,
                           WranServiceFlow::Direction dir) const;
private:
  std::vector<WranServiceFlow*> * m_serviceFlows;
};

} // namespace ns3

#endif /* SERVICE_FLOW_MANAGER_H */
