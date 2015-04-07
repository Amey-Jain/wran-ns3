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
 * Author: Sayef Azad Sakin <sayefsakin[at]gmail[dot]com>
 */

#include "ns3/assert.h"
#include "ns3/net-device.h"
#include "wran-channel.h"
#include "wran-phy.h"

NS_LOG_COMPONENT_DEFINE ("WranChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranChannel);

TypeId WranChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranChannel")
    .SetParent<Channel> ();
  return tid;
}

WranChannel::WranChannel (void)
{
}

WranChannel::~WranChannel (void)
{
}

void
WranChannel::Attach (Ptr<WranPhy> phy)
{
  DoAttach (phy);
}

uint32_t
WranChannel::GetNDevices (void) const
{
  return DoGetNDevices ();
}

Ptr<NetDevice>
WranChannel::GetDevice (uint32_t index) const
{
  return DoGetDevice (index);
}

} // namespace ns3
