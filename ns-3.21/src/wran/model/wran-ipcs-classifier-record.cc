/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2015, 2009 Green Network Research Group
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
#include "wran-ipcs-classifier-record.h"
#include <stdint.h>
#include "ns3/ipv4-address.h"
#include "wran-tlv.h"
NS_LOG_COMPONENT_DEFINE ("WranIpcsClassifierRecord");

namespace ns3 {

WranIpcsClassifierRecord::WranIpcsClassifierRecord (void)
{
  m_priority = 255;
  m_priority = 0;
  m_index = 0;
  m_tosLow = 0;
  m_tosHigh = 0;
  m_tosMask = 0;
  m_cid = 0;
  m_protocol.push_back (6); // tcp
  m_protocol.push_back (17); // udp
  AddSrcAddr (Ipv4Address ("0.0.0.0"), Ipv4Mask ("0.0.0.0"));
  AddDstAddr (Ipv4Address ("0.0.0.0"), Ipv4Mask ("0.0.0.0"));
  AddSrcPortRange (0, 65535);
  AddDstPortRange (0, 65535);
}

WranIpcsClassifierRecord::~WranIpcsClassifierRecord (void)
{
}

WranIpcsClassifierRecord::WranIpcsClassifierRecord (WranTlv tlv)
{
  NS_ASSERT_MSG (tlv.GetType () == CsParamWranVectorTlvValue::Packet_Classification_Rule, "Invalid TLV");
  ClassificationRuleWranVectorTlvValue* rules = ((ClassificationRuleWranVectorTlvValue*)(tlv.PeekValue ()));
  m_priority = 0;
  m_index = 0;
  m_tosLow = 0;
  m_tosHigh = 0;
  m_tosMask = 0;
  m_cid = 0;
  for (std::vector<WranTlv*>::const_iterator iter = rules->Begin (); iter != rules->End (); ++iter)
    {
      switch ((*iter)->GetType ())
        {
        case ClassificationRuleWranVectorTlvValue::Priority:
          {
            m_priority = ((WranU8TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case ClassificationRuleWranVectorTlvValue::ToS:
          {
            NS_FATAL_ERROR ("ToS Not implemented-- please implement and contribute a patch");
            break;
          }
        case ClassificationRuleWranVectorTlvValue::Protocol:
          {
            WranProtocolTlvValue * list = (WranProtocolTlvValue *)(*iter)->PeekValue ();
            for (std::vector<uint8_t>::const_iterator iter2 = list->Begin (); iter2 != list->End (); ++iter2)
              {
                AddProtocol (*iter2);
              }
            break;
          }
        case ClassificationRuleWranVectorTlvValue::IP_src:
          {
            WranIpv4AddressTlvValue * list = (WranIpv4AddressTlvValue *)(*iter)->PeekValue ();
            for (std::vector<WranIpv4AddressTlvValue::ipv4Addr>::const_iterator iter2 = list->Begin (); iter2 != list->End (); ++iter2)
              {
                AddSrcAddr ((*iter2).Address, (*iter2).Mask);
              }
            break;
          }
        case ClassificationRuleWranVectorTlvValue::IP_dst:
          {
            WranIpv4AddressTlvValue * list = (WranIpv4AddressTlvValue *)(*iter)->PeekValue ();
            for (std::vector<WranIpv4AddressTlvValue::ipv4Addr>::const_iterator iter2 = list->Begin (); iter2 != list->End (); ++iter2)
              {
                AddDstAddr ((*iter2).Address, (*iter2).Mask);
              }
            break;
          }
        case ClassificationRuleWranVectorTlvValue::Port_src:
          {
            WranPortRangeTlvValue * list = (WranPortRangeTlvValue *)(*iter)->PeekValue ();
            for (std::vector<WranPortRangeTlvValue::PortRange>::const_iterator iter2 = list->Begin (); iter2 != list->End (); ++iter2)
              {
                AddSrcPortRange ((*iter2).PortLow, (*iter2).PortHigh);
              }
            break;
          }
        case ClassificationRuleWranVectorTlvValue::Port_dst:
          {
            WranPortRangeTlvValue * list = (WranPortRangeTlvValue *)(*iter)->PeekValue ();
            for (std::vector<WranPortRangeTlvValue::PortRange>::const_iterator iter2 = list->Begin (); iter2 != list->End (); ++iter2)
              {
                AddDstPortRange ((*iter2).PortLow, (*iter2).PortHigh);
              }
            break;
          }
        case ClassificationRuleWranVectorTlvValue::Index:
          {
            m_index = ((WranU16TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        }
    }
}

WranIpcsClassifierRecord::WranIpcsClassifierRecord (Ipv4Address SrcAddress,
                                            Ipv4Mask SrcMask,
                                            Ipv4Address DstAddress,
                                            Ipv4Mask DstMask,
                                            uint16_t SrcPortLow,
                                            uint16_t SrcPortHigh,
                                            uint16_t DstPortLow,
                                            uint16_t DstPortHigh,
                                            uint8_t protocol,
                                            uint8_t priority)
{
  m_priority = priority;
  m_protocol.push_back (protocol);
  AddSrcAddr (SrcAddress, SrcMask);
  AddDstAddr (DstAddress, DstMask);
  AddSrcPortRange (SrcPortLow, SrcPortHigh);
  AddDstPortRange (DstPortLow, DstPortHigh);
  m_index = 0;
  m_tosLow = 0;
  m_tosHigh = 0;
  m_tosMask = 0;
  m_cid = 0;
}

void
WranIpcsClassifierRecord::AddSrcAddr (Ipv4Address srcAddress, Ipv4Mask srcMask)
{
  struct ipv4Addr tmp;
  tmp.Address = srcAddress;
  tmp.Mask = srcMask;
  m_srcAddr.push_back (tmp);
}
void
WranIpcsClassifierRecord::AddDstAddr (Ipv4Address dstAddress, Ipv4Mask dstMask)
{
  struct ipv4Addr tmp;
  tmp.Address = dstAddress;
  tmp.Mask = dstMask;
  m_dstAddr.push_back (tmp);
}
void
WranIpcsClassifierRecord::AddSrcPortRange (uint16_t srcPortLow, uint16_t srcPortHigh)
{
  struct PortRange tmp;
  tmp.PortLow = srcPortLow;
  tmp.PortHigh = srcPortHigh;
  m_srcPortRange.push_back (tmp);

}
void
WranIpcsClassifierRecord::AddDstPortRange (uint16_t dstPortLow, uint16_t dstPortHigh)
{
  struct PortRange tmp;
  tmp.PortLow = dstPortLow;
  tmp.PortHigh = dstPortHigh;
  m_dstPortRange.push_back (tmp);
}
void
WranIpcsClassifierRecord::AddProtocol (uint8_t proto)
{
  m_protocol.push_back (proto);
}
void
WranIpcsClassifierRecord::SetPriority (uint8_t prio)
{
  m_priority = prio;
}
void
WranIpcsClassifierRecord::SetCid (uint16_t cid)
{
  m_cid = cid;
}
void
WranIpcsClassifierRecord::SetIndex (uint16_t index)
{
  m_index = index;
}

uint16_t
WranIpcsClassifierRecord::GetIndex (void) const
{
  return m_index;
}
uint16_t
WranIpcsClassifierRecord::GetCid (void) const
{
  return m_cid;
}
uint8_t
WranIpcsClassifierRecord::GetPriority (void) const
{
  return m_priority;
}

bool
WranIpcsClassifierRecord::CheckMatchSrcAddr (Ipv4Address srcAddress) const
{
  for (std::vector<struct ipv4Addr>::const_iterator iter = m_srcAddr.begin (); iter != m_srcAddr.end (); ++iter)
    {
      NS_LOG_INFO ("src addr check match: pkt=" << srcAddress << " cls=" << (*iter).Address << "/" << (*iter).Mask);
      if (srcAddress.CombineMask ((*iter).Mask) == (*iter).Address)
        {
          return true;
        }
    }
  NS_LOG_INFO ("NOT OK!");
  return false;
}
bool
WranIpcsClassifierRecord::CheckMatchDstAddr (Ipv4Address dstAddress) const
{

  for (std::vector<struct ipv4Addr>::const_iterator iter = m_dstAddr.begin (); iter != m_dstAddr.end (); ++iter)
    {
      NS_LOG_INFO ("dst addr check match: pkt=" << dstAddress << " cls=" << (*iter).Address << "/" << (*iter).Mask);
      if (dstAddress.CombineMask ((*iter).Mask) == (*iter).Address)
        {
          return true;
        }
    }
  NS_LOG_INFO ("NOT OK!");
  return false;
}
bool
WranIpcsClassifierRecord::CheckMatchSrcPort (uint16_t port) const
{
  for (std::vector<struct PortRange>::const_iterator iter = m_srcPortRange.begin (); iter != m_srcPortRange.end (); ++iter)
    {
      NS_LOG_INFO ("src port check match: pkt=" << port << " cls= [" << (*iter).PortLow << " TO " << (*iter).PortHigh
                                                << "]");
      if (port >= (*iter).PortLow && port <= (*iter).PortHigh)
        {
          return true;
        }
    }
  NS_LOG_INFO ("NOT OK!");
  return false;
}
bool
WranIpcsClassifierRecord::CheckMatchDstPort (uint16_t port) const
{
  for (std::vector<struct PortRange>::const_iterator iter = m_dstPortRange.begin (); iter != m_dstPortRange.end (); ++iter)
    {
      NS_LOG_INFO ("dst port check match: pkt=" << port << " cls= [" << (*iter).PortLow << " TO " << (*iter).PortHigh
                                                << "]");
      if (port >= (*iter).PortLow && port <= (*iter).PortHigh)
        {
          return true;
        }
    }
  NS_LOG_INFO ("NOT OK!");
  return false;
}
bool
WranIpcsClassifierRecord::CheckMatchProtocol (uint8_t proto) const
{
  for (std::vector<uint8_t>::const_iterator iter = m_protocol.begin (); iter != m_protocol.end (); ++iter)
    {
      NS_LOG_INFO ("proto check match: pkt=" << (uint16_t) proto << " cls=" << (uint16_t) proto);
      if (proto == (*iter))
        {
          return true;
        }
    }
  NS_LOG_INFO ("NOT OK!");
  return false;
}
bool
WranIpcsClassifierRecord::CheckMatch (Ipv4Address srcAddress,
                                  Ipv4Address dstAddress,
                                  uint16_t srcPort,
                                  uint16_t dstPort,
                                  uint8_t proto) const
{
  return (CheckMatchProtocol (proto) && CheckMatchDstPort (dstPort) && CheckMatchSrcPort (srcPort)
          && CheckMatchDstAddr (dstAddress) && CheckMatchSrcAddr (srcAddress));
}

WranTlv
WranIpcsClassifierRecord::ToWranTlv (void) const
{
  WranIpv4AddressTlvValue ipv4AddrValSrc;
  for (std::vector<struct ipv4Addr>::const_iterator iter = m_srcAddr.begin (); iter != m_srcAddr.end (); ++iter)
    {
      ipv4AddrValSrc.Add ((*iter).Address, (*iter).Mask);
    }

  WranIpv4AddressTlvValue ipv4AddrValDst;
  for (std::vector<struct ipv4Addr>::const_iterator iter = m_dstAddr.begin (); iter != m_dstAddr.end (); ++iter)
    {
      ipv4AddrValDst.Add ((*iter).Address, (*iter).Mask);
    }

  WranProtocolTlvValue protoVal;
  for (std::vector<uint8_t>::const_iterator iter = m_protocol.begin (); iter != m_protocol.end (); ++iter)
    {
      protoVal.Add ((*iter));
    }

  WranPortRangeTlvValue portValueSrc;
  for (std::vector<struct PortRange>::const_iterator iter = m_srcPortRange.begin (); iter != m_srcPortRange.end (); ++iter)
    {
      portValueSrc.Add ((*iter).PortLow, (*iter).PortHigh);
    }

  WranPortRangeTlvValue portValueDst;
  for (std::vector<struct PortRange>::const_iterator iter = m_dstPortRange.begin (); iter != m_dstPortRange.end (); ++iter)
    {
      portValueDst.Add ((*iter).PortLow, (*iter).PortHigh);
    }

  ClassificationRuleWranVectorTlvValue ClassVectVal;
  ClassVectVal.Add (WranTlv (ClassificationRuleWranVectorTlvValue::Priority, 1, WranU8TlvValue (m_priority)));
  ClassVectVal.Add (WranTlv (ClassificationRuleWranVectorTlvValue::Protocol, protoVal.GetSerializedSize (), protoVal));
  ClassVectVal.Add (WranTlv (ClassificationRuleWranVectorTlvValue::IP_src, ipv4AddrValSrc.GetSerializedSize (), ipv4AddrValSrc));
  ClassVectVal.Add (WranTlv (ClassificationRuleWranVectorTlvValue::IP_dst, ipv4AddrValDst.GetSerializedSize (), ipv4AddrValDst));
  ClassVectVal.Add (WranTlv (ClassificationRuleWranVectorTlvValue::Port_src, portValueSrc.GetSerializedSize (), portValueSrc));
  ClassVectVal.Add (WranTlv (ClassificationRuleWranVectorTlvValue::Port_dst, portValueDst.GetSerializedSize (), portValueDst));
  ClassVectVal.Add (WranTlv (ClassificationRuleWranVectorTlvValue::Index, 2, WranU16TlvValue (1)));

  WranTlv tmp_tlv (CsParamWranVectorTlvValue::Packet_Classification_Rule, ClassVectVal.GetSerializedSize (), ClassVectVal);

  return tmp_tlv;
}

} // namespace ns3
