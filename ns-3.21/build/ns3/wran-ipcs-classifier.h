/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2009 Green Network Research Group
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
 *
 */

#ifndef IPCS_CLASSIFIER_H
#define IPCS_CLASSIFIER_H

#include <stdint.h>
#include <vector>
#include "wran-ss-service-flow-manager.h"
#include "ns3/ptr.h"
#include "ns3/packet.h"

namespace ns3 {
class WranSSServiceFlowManager;

/**
 * \ingroup wran
 */
class WranIpcsClassifier : public Object
{
public:
  static TypeId GetTypeId (void);
  WranIpcsClassifier (void);
  ~WranIpcsClassifier (void);
  /**
   * \brief classify a packet in a service flow
   * \param packet the packet to classify
   * \param sfm the service flow manager to be used to classify packets
   * \param dir The direction on wich the packet should be sent (UP or DOWN)
   * \return The service flow that should be used to send this packet
   */
  WranServiceFlow * Classify (Ptr<const Packet> packet, Ptr<WranServiceFlowManager> sfm, WranServiceFlow::Direction dir);
};
} // namespace ns3

#endif /* IPCS_CLASSIFIER_H */
