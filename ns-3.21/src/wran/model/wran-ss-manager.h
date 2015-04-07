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

#ifndef SS_MANAGER_H
#define SS_MANAGER_H

#include <stdint.h>
#include "ns3/cid.h"
#include "wran-ss-record.h"

namespace ns3 {

/**
 * \ingroup wran
 * \brief this class manages a list of SSrecord
 * \see SSrecord
 */
class WranSSManager : public Object
{
public:
  static TypeId GetTypeId (void);
  WranSSManager (void);
  ~WranSSManager (void);
  WranSSRecord* CreateWranSSRecord (const Mac48Address &macAddress);
  WranSSRecord* GetWranSSRecord (const Mac48Address &macAddress) const;
  /**
   * \brief returns the ssrecord which has been assigned this cid. Since
   *        different types of cids (basic, primary, transport) are assigned
   *        different values, all cids (basic, primary and transport) of the
   *        ssrecord are matched.
   * \param cid the cid to be matched
   * \return the ss record that has the cid
   */
  WranSSRecord* GetWranSSRecord (Cid cid) const;
  std::vector<WranSSRecord*>* GetWranSSRecords (void) const;
  bool IsInRecord (const Mac48Address &macAddress) const;
  bool IsRegistered (const Mac48Address &macAddress) const;
  void DeleteWranSSRecord (Cid cid);
  Mac48Address GetMacAddress (Cid cid) const;
  uint32_t GetNSSs (void) const;
  uint32_t GetNRegisteredSSs (void) const;
private:
  std::vector<WranSSRecord*> *m_ssRecords;
};

} // namespace ns3

#endif /* SS_MANAGER_H */
