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

/* SS outbound scheduler as per in Section 6.3.5.1 */

#ifndef SS_SCHEDULER_H
#define SS_SCHEDULER_H

#include <stdint.h>
#include "ns3/packet.h"
#include "ns3/packet-burst.h"
#include "wran-phy.h"
#include "wran-mac-header.h"

namespace ns3 {

class WranSubscriberStationNetDevice;
class WranConnection;

/**
 * \ingroup wran
 */
class WranSSScheduler : public Object
{

public:
  static TypeId GetTypeId (void);
  WranSSScheduler (Ptr<WranSubscriberStationNetDevice> ss);
  ~WranSSScheduler (void);

  void SetPollMe (bool pollMe);
  bool GetPollMe (void) const;
  /**
   * \return a list of packet to be sent in the next opportunity
   * \param availableSymbols the available resources in symbols
   * \param modulationType the used modulation
   * \param packetType the type of packets to select from
   * \param connection the connection from wich packets will be selected
   */
  Ptr<PacketBurst> Schedule (uint16_t availableSymbols,
                             WranPhy::ModulationType modulationType,
                             MacHeaderType::HeaderType packetType, Ptr<WranConnection> &connection);


  void DoDispose (void);
protected:
private:
  WranSSScheduler (const WranSSScheduler &);
  WranSSScheduler & operator= (const WranSSScheduler &);

  Ptr<WranConnection> SelectConnection (void);
  Ptr<WranSubscriberStationNetDevice> m_ss;
  bool m_pollMe;

};

} // namespace ns3

#endif /* SS_SCHEDULER_H */
