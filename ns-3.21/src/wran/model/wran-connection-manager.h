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

#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <stdint.h>
#include "ns3/cid.h"
#include "wran-connection.h"
#include "ns3/mac48-address.h"

namespace ns3 {

class CidFactory;
class WranSSRecord;
class RngRsp;
class WranNetDevice;
class SubscriberStationNetDevice;

/**
 * \ingroup wran
 * The same connection manager class serves both for BS and SS though some functions are exclusive to only one of them.
 */

class WranConnectionManager : public Object
{
public:
  static TypeId GetTypeId (void);
  WranConnectionManager (void);
  ~WranConnectionManager (void);
  void DoDispose (void);
  void SetCidFactory (CidFactory *cidFactory);
  /**
   * \brief allocates the management connection for an ss record. This method is only used by BS
   * \param ssRecord the ss record to wich the management connection will be allocated
   * \param rngrsp the ranging response message
   */
  void AllocateManagementConnections (WranSSRecord *ssRecord, RngRsp *rngrsp);
  /**
   * \brief create a connection of type type
   * \param type type of the connection to create
   */
  Ptr<WranConnection> CreateConnection (Cid::Type type);
  /**
   * \brief add a connection to the list of managed connections
   * \param connection the connection to add
   * \param type the type of connection to add
   */
  void AddConnection (Ptr<WranConnection> connection, Cid::Type type);
  /**
   * \return the connection which has as identifier cid
   */
  Ptr<WranConnection> GetConnection (Cid cid);
  /**
   * \return a listy of all connection which have as type type
   */
  std::vector<Ptr<WranConnection> > GetConnections (Cid::Type type) const;
  uint32_t GetNPackets (Cid::Type type, WranServiceFlow::SchedulingType schedulingType) const;
  /**
   * \return true if one of the managed connection has at least one packet to send, false otherwise
   */
  bool HasPackets (void) const;
private:
  std::vector<Ptr<WranConnection> > m_basicConnections;
  std::vector<Ptr<WranConnection> > m_primaryConnections;
  std::vector<Ptr<WranConnection> > m_transportConnections;
  std::vector<Ptr<WranConnection> > m_multicastConnections;
  // only for BS
  CidFactory *m_cidFactory;
};

} // namespace ns3

#endif /* CONNECTION_MANAGER_H */

