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
 * Authors: Sayef Azad Sakin <sayefsakin[at]gmail[dot]com>
 *
 */

#include "ns3/llc-snap-header.h"
#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "wran-net-device.h"
#include "wran-channel.h"
#include "ns3/packet-burst.h"
#include "wran-burst-profile-manager.h"
#include <list>
#include "ns3/send-params.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "wran-service-flow-manager.h"
#include "wran-connection-manager.h"
#include "wran-bandwidth-manager.h"

NS_LOG_COMPONENT_DEFINE ("WranNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranNetDevice);

uint32_t WranNetDevice::m_nrFrames = 0;
uint8_t WranNetDevice::m_direction = ~0;
Time WranNetDevice::m_frameStartTime = Seconds (0);

TypeId WranNetDevice::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::WranNetDevice")

    .SetParent<NetDevice> ()

    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (DEFAULT_MSDU_SIZE),
                   MakeUintegerAccessor (&WranNetDevice::SetMtu,
                                         &WranNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> (0,MAX_MSDU_SIZE))

    .AddAttribute ("Phy",
                   "The PHY layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&WranNetDevice::GetPhy, &WranNetDevice::SetPhy),
                   MakePointerChecker<WranPhy> ())

    .AddAttribute ("Channel",
                   "The channel attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&WranNetDevice::GetPhyChannel, &WranNetDevice::SetChannel),
                   MakePointerChecker<WranChannel> ())

    .AddAttribute ("RTG",
                   "receive/transmit transition gap.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WranNetDevice::GetRtg, &WranNetDevice::SetRtg),
                   MakeUintegerChecker<uint16_t> (0, 210)) // previosly 120

    .AddAttribute ("TTG",
                   "transmit/receive transition gap.",
                   UintegerValue (0),
                   MakeUintegerAccessor (&WranNetDevice::GetTtg, &WranNetDevice::SetTtg),
                   MakeUintegerChecker<uint16_t> (0, 210)) // previosly 120

    .AddAttribute ("WranConnectionManager",
                   "The connection manager attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&WranNetDevice::GetWranConnectionManager,
                                        &WranNetDevice::SetWranConnectionManager),
                   MakePointerChecker<WranConnectionManager> ())

    .AddAttribute ("WranBurstProfileManager",
                   "The burst profile manager attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&WranNetDevice::GetWranBurstProfileManager,
                                        &WranNetDevice::SetWranBurstProfileManager),
                   MakePointerChecker<WranBurstProfileManager> ())

    .AddAttribute ("WranBandwidthManager",
                   "The bandwidth manager attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&WranNetDevice::GetWranBandwidthManager,
                                        &WranNetDevice::SetWranBandwidthManager),
                   MakePointerChecker<WranBandwidthManager> ())

    .AddAttribute ("InitialRangingConnection",
                   "Initial ranging connection",
                   PointerValue (),
                   MakePointerAccessor (&WranNetDevice::m_initialRangingConnection),
                   MakePointerChecker<WranConnection> ())

    .AddAttribute ("BroadcastConnection",
                   "Broadcast connection",
                   PointerValue (),
                   MakePointerAccessor (&WranNetDevice::m_broadcastConnection),
                   MakePointerChecker<WranConnection> ())

    .AddTraceSource ("Rx", "Receive trace", MakeTraceSourceAccessor (&WranNetDevice::m_traceRx))

    .AddTraceSource ("Tx", "Transmit trace", MakeTraceSourceAccessor (&WranNetDevice::m_traceTx));
  return tid;
}

WranNetDevice::WranNetDevice (void)
  : m_state (0),
    m_symbolIndex (0),
    m_ttg (0),
    m_rtg (0)
{
  InitializeChannels ();
  m_connectionManager = CreateObject<WranConnectionManager> ();
  m_burstProfileManager = CreateObject<WranBurstProfileManager> (this);
  m_bandwidthManager = CreateObject<WranBandwidthManager> (this);
  m_nrFrames = 0;
  m_direction = ~0;
  m_frameStartTime = Seconds (0);
}

WranNetDevice::~WranNetDevice (void)
{
}

void
WranNetDevice::DoDispose (void)
{

  m_phy->Dispose ();
  m_phy = 0;
  m_node = 0;
  m_initialRangingConnection = 0;
  m_broadcastConnection = 0;
  m_connectionManager = 0;
  m_burstProfileManager = 0;
  m_bandwidthManager = 0;
  m_connectionManager = 0;
  m_bandwidthManager = 0;

  NetDevice::DoDispose ();
}


void
WranNetDevice::SetTtg (uint16_t ttg)
{
  m_ttg = ttg;
}

uint16_t
WranNetDevice::GetTtg (void) const
{
  return m_ttg;
}

void
WranNetDevice::SetRtg (uint16_t rtg)
{
  m_rtg = rtg;
}

uint16_t
WranNetDevice::GetRtg (void) const
{
  return m_rtg;
}

void
WranNetDevice::SetName (const std::string name)
{
  m_name = name;
}

std::string
WranNetDevice::GetName (void) const
{
  return m_name;
}

void
WranNetDevice::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}

uint32_t
WranNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}

Ptr<Channel>
WranNetDevice::GetChannel (void) const
{
  return DoGetChannel ();
}

Ptr<Channel>
WranNetDevice::GetPhyChannel (void) const
{
  return DoGetChannel ();
}

bool
WranNetDevice::SetMtu (const uint16_t mtu)
{
  if (mtu > MAX_MSDU_SIZE)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}

uint16_t
WranNetDevice::GetMtu (void) const
{
  return m_mtu;
}

bool
WranNetDevice::IsLinkUp (void) const
{

  return m_phy != 0 && m_linkUp;

}

void
WranNetDevice::SetLinkChangeCallback (Callback<void> callback)
{
  m_linkChange = callback;
}

bool
WranNetDevice::IsBroadcast (void) const
{
  return true;
}

Address
WranNetDevice::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}

bool
WranNetDevice::IsMulticast (void) const
{
  return false;
}

Address
WranNetDevice::GetMulticast (void) const
{
  return Mac48Address ("01:00:5e:00:00:00");
}

Address
WranNetDevice::MakeMulticastAddress (Ipv4Address multicastGroup) const
{
  return GetMulticast ();
}

bool
WranNetDevice::IsPointToPoint (void) const
{
  return false;
}

bool
WranNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{

  Mac48Address to = Mac48Address::ConvertFrom (dest);
  LlcSnapHeader llcHdr;
  llcHdr.SetType (protocolNumber);
  packet->AddHeader (llcHdr);

  m_traceTx (packet, to);

  return DoSend (packet, Mac48Address::ConvertFrom (GetAddress ()), to, protocolNumber);
}

void
WranNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
}

Ptr<Node>
WranNetDevice::GetNode (void) const
{
  return m_node;
}

bool
WranNetDevice::NeedsArp (void) const
{
  return false;
  /*
   * Modified by Mohamed Amine ISMAIL.
   * see "Transmission of IPv4 packets over IEEE 802.16's IP Convergence
   *      Sublayer draft-ietf-16ng-ipv4-over-802-dot-16-ipcs-04.txt" section
   * 5.2
   */
}

void
WranNetDevice::SetReceiveCallback (ReceiveCallback cb)
{
  m_forwardUp = cb;
}

void
WranNetDevice::ForwardUp (Ptr<Packet> packet, const Mac48Address &source, const Mac48Address &dest)
{
  m_traceRx (packet, source);
  LlcSnapHeader llc;
  packet->RemoveHeader (llc);
  m_forwardUp (this, packet, llc.GetType (), source);
}

void
WranNetDevice::Attach (Ptr<WranChannel> channel)
{
  m_phy->Attach (channel);
}

void
WranNetDevice::SetPhy (Ptr<WranPhy> phy)
{
  m_phy = phy;
}

Ptr<WranPhy>
WranNetDevice::GetPhy (void) const
{
  return m_phy;
}

void
WranNetDevice::SetChannel (Ptr<WranChannel> channel)
{
  if (m_phy != 0)
    {
      m_phy->Attach (channel);
    }

}

uint64_t
WranNetDevice::GetChannel (uint8_t index) const
{
  return m_dlChannels.at (index);
}

void
WranNetDevice::SetNrFrames (uint32_t nrFrames)
{
  m_nrFrames = nrFrames;
}

uint32_t WranNetDevice::GetNrFrames (void) const
{

  return m_nrFrames;
}

void
WranNetDevice::SetAddress (Address address)
{
  m_address = Mac48Address::ConvertFrom (address);
}

void
WranNetDevice::SetMacAddress (Mac48Address address)
{
  m_address = address;
}

Address
WranNetDevice::GetAddress (void) const
{
  return m_address;
}

Mac48Address
WranNetDevice::GetMacAddress (void) const
{
  return m_address;
}

void
WranNetDevice::SetState (uint8_t state)
{
  m_state = state;
}

uint8_t
WranNetDevice::GetState (void) const
{
  return m_state;
}

Ptr<WranConnection>
WranNetDevice::GetInitialRangingConnection (void) const
{
  return m_initialRangingConnection;
}

Ptr<WranConnection>
WranNetDevice::GetBroadcastConnection (void) const
{
  return m_broadcastConnection;
}

void
WranNetDevice::SetCurrentDcd (Dcd dcd)
{
  m_currentDcd = dcd;
}

Dcd
WranNetDevice::GetCurrentDcd (void) const
{
  return m_currentDcd;
}

void
WranNetDevice::SetCurrentUcd (Ucd ucd)
{
  m_currentUcd = ucd;
}

Ucd
WranNetDevice::GetCurrentUcd (void) const
{
  return m_currentUcd;
}

Ptr<WranConnectionManager>
WranNetDevice::GetWranConnectionManager (void) const
{
  return m_connectionManager;
}

void
WranNetDevice::SetWranConnectionManager (Ptr<WranConnectionManager> cm)
{
  m_connectionManager = cm;
}

Ptr<WranBurstProfileManager>
WranNetDevice::GetWranBurstProfileManager (void) const
{
  return m_burstProfileManager;
}

void
WranNetDevice::SetWranBurstProfileManager (Ptr<WranBurstProfileManager> bpm)
{
  m_burstProfileManager = bpm;
}

Ptr<WranBandwidthManager>
WranNetDevice::GetWranBandwidthManager (void) const
{
  return m_bandwidthManager;
}

void
WranNetDevice::SetWranBandwidthManager (Ptr<WranBandwidthManager> bwm)
{
  m_bandwidthManager = bwm;
}

void
WranNetDevice::CreateDefaultConnections (void)
{
  m_initialRangingConnection = CreateObject<WranConnection> (Cid::InitialRanging (), Cid::INITIAL_RANGING);
  m_broadcastConnection = CreateObject<WranConnection> (Cid::Broadcast (), Cid::BROADCAST);
}

void
WranNetDevice::Receive (Ptr<const PacketBurst> burst)
{

  NS_LOG_DEBUG ("WranNetDevice::Receive, station = " << GetMacAddress ());

  Ptr<PacketBurst> b = burst->Copy ();
  for (std::list<Ptr<Packet> >::const_iterator iter = b->Begin (); iter != b->End (); ++iter)
    {
      Ptr<Packet> packet = *iter;
      DoReceive (packet);
    }
}

Ptr<WranChannel>
WranNetDevice::DoGetChannel (void) const
{
  return m_phy->GetChannel ();
}

void
WranNetDevice::SetReceiveCallback (void)
{
  m_phy->SetReceiveCallback (MakeCallback (&WranNetDevice::Receive, this));
}

bool
WranNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{

  Mac48Address from = Mac48Address::ConvertFrom (source);
  Mac48Address to = Mac48Address::ConvertFrom (dest);

  LlcSnapHeader llcHdr;
  llcHdr.SetType (protocolNumber);
  packet->AddHeader (llcHdr);

  m_traceTx (packet, to);
  return DoSend (packet, from, to, protocolNumber);
}

void
WranNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscRx = cb;
}

bool
WranNetDevice::IsPromisc (void)
{
  return !(m_promiscRx.IsNull ());
}

void
WranNetDevice::NotifyPromiscTrace (Ptr<Packet> p)
{
  // m_promiscRx(p);
}

bool
WranNetDevice::SupportsSendFrom (void) const
{
  return false;
}

void
WranNetDevice::ForwardDown (Ptr<PacketBurst> burst, WranPhy::ModulationType modulationType)
{
	NS_LOG_INFO("From ForwardDown");
  SendParams * params = new OfdmSendParams (burst, modulationType, m_direction);
  m_phy->Send (params);
  delete params;
}

void
WranNetDevice::InitializeChannels (void)
{

  // initializing vector of channels (or frequencies)
  // Values according to WirelessMAN-OFDM RF profile for 10 MHz channelization
  // Section 12.3.3.1 from IEEE 802.16-2004 standard
  // profR10_3 :
  // channels: 5000 + n ⋅ 5 MHz, ∀n ∈ { 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167 }
  // from a range 5GHz to 6GHz, according to Section 8.5.1.

//	Previous Code
//	uint64_t frequency = 5000;
//
//	  for (uint8_t i = 0; i < 200; i++)
//	    {
//	      m_dlChannels.push_back (frequency);
//	      frequency += 5;
//	    }

//	New Things for 802.22
//	I have chosen operating frequncy 470Mz - 608MHz, at 6MHz bandwidth, total 23 channels.
  NS_LOG_INFO("hello from net device initializer");
	uint64_t frequency = 470;

  for (uint8_t i = 0; i < 24; i++)
    {
      m_dlChannels.push_back (frequency);
      frequency += 6; // m_phy->GetChannelBandwidth();
    }
}

bool
WranNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

Address
WranNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (multicastGroup);

  Mac48Address ad = Mac48Address::GetMulticast (multicastGroup);

  //
  // Implicit conversion (operator Address ()) is defined for Mac48Address, so
  // use it by just returning the EUI-48 address which is automagically converted
  // to an Address.
  //
  NS_LOG_LOGIC ("multicast address is " << ad);

  return ad;
}

Address
WranNetDevice::GetMulticast (Ipv6Address addr) const
{
  Mac48Address ad = Mac48Address::GetMulticast (addr);

  NS_LOG_LOGIC ("MAC IPv6 multicast address is " << ad);
  return ad;
}

void
WranNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  /* \todo Add a callback invoked whenever the link
   * status changes to UP. This callback is typically used
   * by the IP/ARP layer to flush the ARP cache and by IPv6 stack
   * to flush NDISC cache whenever the link goes up.
   */
  NS_FATAL_ERROR ("Not implemented-- please implement and contribute a patch");
}

uint8_t WranNetDevice::GetTotalChannels (void){
	return totalChannels;
}
void WranNetDevice::SetTotalChannels (uint8_t t_channels){
	totalChannels = t_channels;
}
} // namespace ns3
