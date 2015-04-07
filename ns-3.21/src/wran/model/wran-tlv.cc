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
 *         a
 *
 */

#include "wran-tlv.h"

NS_LOG_COMPONENT_DEFINE ("WranTlv");

namespace ns3 {
// NS_OBJECT_ENSURE_REGISTERED ("WranTlv")
//    ;

TypeId WranTlv::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void WranTlv::Print (std::ostream &os) const
{
  os << "TLV type = " << (uint32_t) m_type << " TLV Length = " << (uint64_t) m_length;
}

WranTlv::WranTlv (uint8_t type, uint64_t length, const WranTlvValue & value)
{
  m_type = type;
  m_length = length;
  m_value = value.Copy ();
}

WranTlv::WranTlv ()
{
  m_type = 0;
  m_length = 0;
  m_value = 0;
}

WranTlv::~WranTlv ()
{
  if (m_value != 0)
    {
      delete m_value;
      m_value = 0;
    }
}

WranTlvValue *
WranTlv::CopyValue (void) const
{
  return m_value->Copy ();
}

WranTlv::WranTlv (const WranTlv & tlv)
{
  m_type = tlv.GetType ();
  m_length = tlv.GetLength ();
  m_value = tlv.CopyValue ();
}

WranTlv &
WranTlv::operator = (WranTlv const& o)
{
  if (m_value != 0)
    {
      delete m_value;
    }
  m_type = o.GetType ();
  m_length = o.GetLength ();
  m_value = o.CopyValue ();

  return *this;
}

uint32_t
WranTlv::GetSerializedSize (void) const
{
  return 1 + GetSizeOfLen () + m_value->GetSerializedSize ();
}

uint8_t
WranTlv::GetSizeOfLen (void) const
{
  uint8_t sizeOfLen = 1;

  if (m_length > 127)
    {
      sizeOfLen = 2;
      uint64_t testValue = 0xFF;
      while (m_length > testValue)
        {
          sizeOfLen++;
          testValue *= 0xFF;
        }
    }
  return sizeOfLen;
}

void
WranTlv::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_type);
  uint8_t lenSize = GetSizeOfLen ();
  if (lenSize == 1)
    {
      i.WriteU8 (m_length);
    }
  else
    {
      i.WriteU8 ((lenSize-1) | WRAN_TLV_EXTENDED_LENGTH_MASK);
      for (int j = 0; j < lenSize - 1; j++)
        {
          i.WriteU8 ((uint8_t)(m_length >> ((lenSize - 1 - 1 - j) * 8)));
        }
    }
  m_value->Serialize (i);
}

uint32_t
WranTlv::Deserialize (Buffer::Iterator i)
{
  // read the type of tlv
  m_type = i.ReadU8 ();

  // read the length
  uint8_t lenSize = i.ReadU8 ();
  uint32_t serializedSize = 2;
  if (lenSize < 127)
    {
      m_length = lenSize;
    }
  else
    {
      lenSize &= ~WRAN_TLV_EXTENDED_LENGTH_MASK;
      for (int j = 0; j < lenSize; j++)
        {
          m_length <<= 8;
          m_length |= i.ReadU8 ();
          serializedSize++;
        }
    }
  switch (m_type)
    {
    case HMAC_TUPLE:
      /// \todo implement Deserialize HMAC_TUPLE
      NS_FATAL_ERROR ("Not implemented-- please implement and contribute a patch");
      break;
    case MAC_VERSION_ENCODING:
      /// \todo implement Deserialize MAC_VERSION_ENCODING
      NS_FATAL_ERROR ("Not implemented-- please implement and contribute a patch");
      break;
    case CURRENT_TRANSMIT_POWER:
      /// \todo implement Deserialize CURRENT_TRANSMIT_POWER
      NS_FATAL_ERROR ("Not implemented-- please implement and contribute a patch");
      break;
    case DOWNLINK_SERVICE_FLOW:
      {
        SfWranVectorTlvValue val;
        serializedSize += val.Deserialize (i, m_length);
        m_value = val.Copy ();
        break;
      }
    case UPLINK_SERVICE_FLOW:
      {
        SfWranVectorTlvValue val;
        serializedSize += val.Deserialize (i, m_length);
        m_value = val.Copy ();
        break;
      }
    case VENDOR_ID_EMCODING:
      /// \todo implement Deserialize VENDOR_ID_EMCODING
      NS_FATAL_ERROR ("Not implemented-- please implement and contribute a patch");
      break;
    case VENDOR_SPECIFIC_INFORMATION:
      /// \todo implement Deserialize  VENDOR_SPECIFIC_INFORMATION
      NS_FATAL_ERROR ("Not implemented-- please implement and contribute a patch");
      break;
    default:
      NS_ASSERT_MSG (false, "Unknown tlv type.");
      break;
    }

  return serializedSize;
}

uint8_t
WranTlv::GetType (void) const
{
  return m_type;
}
uint64_t
WranTlv::GetLength (void) const
{
  return m_length;
}
WranTlvValue*
WranTlv::PeekValue (void)
{
  return m_value;
}

WranTlv *
WranTlv::Copy (void) const
{
  return new WranTlv (m_type, m_length, *m_value);
}
// ==============================================================================
WranVectorTlvValue::WranVectorTlvValue ()
{
  m_tlvList = new std::vector<WranTlv*>;
}

WranVectorTlvValue::~WranVectorTlvValue ()
{
  for (std::vector<WranTlv*>::const_iterator iter = m_tlvList->begin (); iter != m_tlvList->end (); ++iter)
    {
      delete (*iter);
    }
  m_tlvList->clear ();
  delete m_tlvList;
}

uint32_t
WranVectorTlvValue::GetSerializedSize (void) const
{
  uint32_t size = 0;
  for (std::vector<WranTlv*>::const_iterator iter = m_tlvList->begin (); iter != m_tlvList->end (); ++iter)
    {
      size += (*iter)->GetSerializedSize ();
    }
  return size;
}

void
WranVectorTlvValue::Serialize (Buffer::Iterator i) const
{
  for (std::vector<WranTlv*>::const_iterator iter = m_tlvList->begin (); iter != m_tlvList->end (); ++iter)
    {
      (*iter)->Serialize (i);
      i.Next ((*iter)->GetSerializedSize ());
    }
}

WranVectorTlvValue::Iterator
WranVectorTlvValue::Begin () const
{
  return m_tlvList->begin ();
}

WranVectorTlvValue::Iterator
WranVectorTlvValue::End () const
{
  return m_tlvList->end ();
}

void
WranVectorTlvValue::Add (const WranTlv & val)
{
  m_tlvList->push_back (val.Copy ());
}

// ==============================================================================
SfWranVectorTlvValue::SfWranVectorTlvValue ()
{

}

SfWranVectorTlvValue *
SfWranVectorTlvValue::Copy (void) const
{
  SfWranVectorTlvValue * tmp = new SfWranVectorTlvValue ();
  for (std::vector<WranTlv*>::const_iterator iter = Begin (); iter != End (); ++iter)
    {
      tmp->Add (WranTlv ((*iter)->GetType (), (*iter)->GetLength (), *(*iter)->PeekValue ()));
    }
  return tmp;
}

uint32_t
SfWranVectorTlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLen)
{
  uint64_t serializedSize = 0;
  while (serializedSize < valueLen)
    {
      uint8_t type = i.ReadU8 ();
      // read the length
      uint8_t lenSize = i.ReadU8 ();
      serializedSize += 2;
      uint64_t length = 0;
      if (lenSize < 127)
        {
          length = lenSize;
        }
      else
        {
          lenSize &= ~WRAN_TLV_EXTENDED_LENGTH_MASK;
          for (int j = 0; j < lenSize; j++)
            {
              length <<= 8;
              length |= i.ReadU8 ();
              serializedSize++;
            }
        }
      switch (type)
        {
        case SFID:
          {
            WranU32TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (SFID, 4, val));
            break;
          }
        case CID:
          {
            WranU16TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (CID, 2, val));
            break;
          }
        case Service_Class_Name:
          NS_FATAL_ERROR ("Not implemented-- please implement and contribute a patch");
          break;
        case reserved1:
          // NOTHING
          break;
        case QoS_Parameter_Set_Type:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (QoS_Parameter_Set_Type, 1, val));
            break;
          }
        case Traffic_Priority:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Traffic_Priority, 1, val));
            break;
          }
        case Maximum_Sustained_Traffic_Rate:
          {
            WranU32TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Maximum_Sustained_Traffic_Rate, 4, val));
            break;
          }
        case Maximum_Traffic_Burst:
          {
            WranU32TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Maximum_Traffic_Burst, 4, val));
            break;
          }
        case Minimum_Reserved_Traffic_Rate:
          {
            WranU32TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Minimum_Reserved_Traffic_Rate, 4, val));
            break;
          }
        case Minimum_Tolerable_Traffic_Rate:
          {
            WranU32TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Minimum_Tolerable_Traffic_Rate, 4, val));
            break;
          }
        case Service_Flow_Scheduling_Type:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Service_Flow_Scheduling_Type, 1, val));
            break;
          }
        case Request_Transmission_Policy:
          {
            WranU32TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Request_Transmission_Policy, 4, val));
            break;
          }
        case Tolerated_Jitter:
          {
            WranU32TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Tolerated_Jitter, 4, val));
            break;
          }
        case Maximum_Latency:
          {
            WranU32TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Maximum_Latency, 4, val));
            break;
          }
        case Fixed_length_versus_Variable_length_SDU_Indicator:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Fixed_length_versus_Variable_length_SDU_Indicator, 1, val));
            break;
          }
        case SDU_Size:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (SDU_Size, 1, val));
            break;
          }
        case Target_SAID:
          {
            WranU16TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Target_SAID, 2, val));
            break;
          }
        case ARQ_Enable:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (ARQ_Enable, 1, val));
            break;
          }
        case ARQ_WINDOW_SIZE:
          {
            WranU16TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (ARQ_WINDOW_SIZE, 2, val));
            break;
          }
        case ARQ_RETRY_TIMEOUT_Transmitter_Delay:
          break;
        case ARQ_RETRY_TIMEOUT_Receiver_Delay:
          break;
        case ARQ_BLOCK_LIFETIME:
          break;
        case ARQ_SYNC_LOSS:
          break;
        case ARQ_DELIVER_IN_ORDER:
          break;
        case ARQ_PURGE_TIMEOUT:
          break;
        case ARQ_BLOCK_SIZE:
          break;
        case reserved2:
          break;
        case CS_Specification:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (CS_Specification, 1, val));
            break;
          }
        case IPV4_CS_Parameters:
          {
            CsParamWranVectorTlvValue val;
            uint32_t size = val.Deserialize (i, length);
            serializedSize += size;
            Add (WranTlv (IPV4_CS_Parameters, size, val));
            break;
          }
        default:
          NS_ASSERT_MSG (false, "Unknown tlv type.");
          break;
        }
      i.Next (length);
    }
  return serializedSize;
}

// ==============================================================================

WranU8TlvValue::WranU8TlvValue (uint8_t value)
{
  m_value = value;
}

WranU8TlvValue::WranU8TlvValue ()
{
  m_value = 0;
}

WranU8TlvValue::~WranU8TlvValue ()
{
}
uint32_t
WranU8TlvValue::GetSerializedSize (void) const
{
  return 1;
}
void
WranU8TlvValue::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_value);
}
uint32_t
WranU8TlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLen)
{
  return Deserialize (i);
}

uint32_t
WranU8TlvValue::Deserialize (Buffer::Iterator i)
{
  m_value = i.ReadU8 ();
  return 1;
}

uint8_t
WranU8TlvValue::GetValue (void) const
{
  return m_value;
}

WranU8TlvValue *
WranU8TlvValue::Copy (void) const
{
  WranU8TlvValue * tmp = new WranU8TlvValue (m_value);
  return tmp;
}
// ==============================================================================
WranU16TlvValue::WranU16TlvValue (uint16_t value)
{
  m_value = value;
}

WranU16TlvValue::WranU16TlvValue ()
{
  m_value = 0;
}

WranU16TlvValue::~WranU16TlvValue (void)
{
}

uint32_t
WranU16TlvValue::GetSerializedSize (void) const
{
  return 2;
}
void
WranU16TlvValue::Serialize (Buffer::Iterator i) const
{
  i.WriteHtonU16 (m_value);
}
uint32_t
WranU16TlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLen)
{
  return Deserialize (i);
}

uint32_t
WranU16TlvValue::Deserialize (Buffer::Iterator i)
{
  m_value = i.ReadNtohU16 ();
  return 2;
}

uint16_t
WranU16TlvValue::GetValue (void) const
{
  return m_value;
}

WranU16TlvValue *
WranU16TlvValue::Copy (void) const
{
  WranU16TlvValue * tmp = new WranU16TlvValue (m_value);
  return tmp;
}
// ==============================================================================
WranU32TlvValue::WranU32TlvValue (uint32_t value)
{
  m_value = value;
}

WranU32TlvValue::WranU32TlvValue ()
{
  m_value = 0;
}

WranU32TlvValue::~WranU32TlvValue (void)
{
}

uint32_t WranU32TlvValue::GetSerializedSize (void) const
{
  return 4;
}
void
WranU32TlvValue::Serialize (Buffer::Iterator i) const
{
  i.WriteHtonU32 (m_value);
}
uint32_t
WranU32TlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLen)
{
  return Deserialize (i);
}

uint32_t
WranU32TlvValue::Deserialize (Buffer::Iterator i)
{
  m_value = i.ReadNtohU32 ();
  return 4;
}
uint32_t
WranU32TlvValue::GetValue (void) const
{
  return m_value;
}

WranU32TlvValue *
WranU32TlvValue::Copy (void) const
{
  WranU32TlvValue * tmp = new WranU32TlvValue (m_value);
  return tmp;
}
// ==============================================================================
uint32_t
CsParamWranVectorTlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLength)
{
  uint64_t serializedSize = 0;
  uint8_t lenSize = 0;
  uint8_t type = 0;
  while (serializedSize < valueLength)
    {
      type = i.ReadU8 ();
      // read the length
      lenSize = i.ReadU8 ();
      serializedSize += 2;
      uint64_t length = 0;
      if (lenSize < 127)
        {
          length = lenSize;
        }
      else
        {
          lenSize &= ~WRAN_TLV_EXTENDED_LENGTH_MASK;
          for (int j = 0; j < lenSize; j++)
            {
              length <<= 8;
              length |= i.ReadU8 ();
              serializedSize++;
            }
        }
      switch (type)
        {
        case Classifier_DSC_Action:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Classifier_DSC_Action, 1, val));
            break;
          }
        case Packet_Classification_Rule:
          {
            ClassificationRuleWranVectorTlvValue val;
            serializedSize += val.Deserialize (i, length);
            Add (WranTlv (Packet_Classification_Rule, val.GetSerializedSize (), val));
            break;
          }
        }
      i.Next (length);
    }
  return serializedSize;
}

CsParamWranVectorTlvValue::CsParamWranVectorTlvValue ()
{

}

CsParamWranVectorTlvValue *
CsParamWranVectorTlvValue::Copy (void) const
{
  CsParamWranVectorTlvValue * tmp = new CsParamWranVectorTlvValue ();
  for (std::vector<WranTlv*>::const_iterator iter = Begin (); iter != End (); ++iter)
    {
      tmp->Add (WranTlv ((*iter)->GetType (), (*iter)->GetLength (), *(*iter)->PeekValue ()));
    }
  return tmp;
}
// ==============================================================================

ClassificationRuleWranVectorTlvValue::ClassificationRuleWranVectorTlvValue ()
{

}

ClassificationRuleWranVectorTlvValue *
ClassificationRuleWranVectorTlvValue::Copy (void) const
{
  ClassificationRuleWranVectorTlvValue * tmp = new ClassificationRuleWranVectorTlvValue ();
  for (std::vector<WranTlv*>::const_iterator iter = Begin (); iter != End (); ++iter)
    {
      tmp->Add (WranTlv ((*iter)->GetType (), (*iter)->GetLength (), *(*iter)->PeekValue ()));
    }
  return tmp;
}

uint32_t
ClassificationRuleWranVectorTlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLength)
{
  uint64_t serializedSize = 0;
  uint8_t lenSize = 0;
  uint8_t type = 0;
  while (serializedSize < valueLength)
    {
      type = i.ReadU8 ();
      // read the length
      lenSize = i.ReadU8 ();
      serializedSize += 2;
      uint64_t length = 0;
      if (lenSize < 127)
        {
          length = lenSize;
        }
      else
        {
          lenSize &= ~WRAN_TLV_EXTENDED_LENGTH_MASK;
          for (int j = 0; j < lenSize; j++)
            {
              length <<= 8;
              length |= i.ReadU8 ();
              serializedSize++;
            }
        }
      switch (type)
        {
        case Priority:
          {
            WranU8TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Priority, 1, val));
            break;
          }
        case ToS:
          {
            WranTosTlvValue val;
            serializedSize += val.Deserialize (i, length);
            Add (WranTlv (ToS, val.GetSerializedSize (), val));
            break;
          }
        case Protocol:
          {
            WranProtocolTlvValue val;
            serializedSize += val.Deserialize (i, length);
            Add (WranTlv (Protocol, val.GetSerializedSize (), val));
            break;
          }
        case IP_src:
          {
            WranIpv4AddressTlvValue val;
            serializedSize += val.Deserialize (i, length);
            Add (WranTlv (IP_src, val.GetSerializedSize (), val));
            break;
          }
        case IP_dst:
          {
            WranIpv4AddressTlvValue val;
            serializedSize += val.Deserialize (i, length);
            Add (WranTlv (IP_dst, val.GetSerializedSize (), val));
            break;
          }
        case Port_src:
          {
            WranPortRangeTlvValue val;
            serializedSize += val.Deserialize (i, length);
            Add (WranTlv (Port_src, val.GetSerializedSize (), val));
            break;
          }
        case Port_dst:
          {
            WranPortRangeTlvValue val;
            serializedSize += val.Deserialize (i, length);
            Add (WranTlv (Port_dst, val.GetSerializedSize (), val));
            break;
          }
        case Index:
          {
            WranU16TlvValue val;
            serializedSize += val.Deserialize (i);
            Add (WranTlv (Index, 2, val));
            break;
          }
        }
      i.Next (length);
    }
  return serializedSize;
}

// ==============================================================================
WranTosTlvValue::WranTosTlvValue ()
{
  m_low = 0;
  m_high = 0;
  m_mask = 0;
}
WranTosTlvValue::WranTosTlvValue (uint8_t low, uint8_t high, uint8_t mask)
{
  m_low = low;
  m_high = high;
  m_mask = mask;
}
WranTosTlvValue::~WranTosTlvValue ()
{
}

uint32_t
WranTosTlvValue::GetSerializedSize (void) const
{
  return 3;
}
void
WranTosTlvValue::Serialize (Buffer::Iterator i) const
{
  i.WriteU8 (m_low);
  i.WriteU8 (m_high);
  i.WriteU8 (m_mask);
}
uint32_t
WranTosTlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLength)
{
  m_low = i.ReadU8 ();
  m_high = i.ReadU8 ();
  m_mask = i.ReadU8 ();
  return 3;
}
uint8_t
WranTosTlvValue::GetLow (void) const
{
  return m_low;
}
uint8_t
WranTosTlvValue::GetHigh (void) const
{
  return m_high;
}
uint8_t
WranTosTlvValue::GetMask (void) const
{
  return m_mask;
}

WranTosTlvValue *
WranTosTlvValue::Copy (void) const
{
  return new WranTosTlvValue (m_low, m_high, m_mask);
}

// ==============================================================================
WranPortRangeTlvValue::WranPortRangeTlvValue ()
{
  m_portRange = new std::vector<struct PortRange>;
}
WranPortRangeTlvValue::~WranPortRangeTlvValue ()
{
  m_portRange->clear ();
  delete m_portRange;
}

uint32_t
WranPortRangeTlvValue::GetSerializedSize (void) const
{
  return m_portRange->size () * sizeof(struct PortRange);
}
void
WranPortRangeTlvValue::Serialize (Buffer::Iterator i) const
{
  for (std::vector<struct PortRange>::const_iterator iter = m_portRange->begin (); iter != m_portRange->end (); ++iter)
    {
      i.WriteHtonU16 ((*iter).PortLow);
      i.WriteHtonU16 ((*iter).PortHigh);
    }
}
uint32_t
WranPortRangeTlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLength)
{
  uint64_t len = 0;
  while (len < valueLength)
    {
      uint16_t low = i.ReadNtohU16 ();
      uint16_t high = i.ReadNtohU16 ();
      Add (low, high);
      len += 4;
    }
  return len;
}
void
WranPortRangeTlvValue::Add (uint16_t portLow, uint16_t portHigh)
{
  struct PortRange tmp;
  tmp.PortLow = portLow;
  tmp.PortHigh = portHigh;
  m_portRange->push_back (tmp);
}
WranPortRangeTlvValue::Iterator
WranPortRangeTlvValue::Begin (void) const
{
  return m_portRange->begin ();
}

WranPortRangeTlvValue::Iterator
WranPortRangeTlvValue::End (void) const
{
  return m_portRange->end ();
}

WranPortRangeTlvValue *
WranPortRangeTlvValue::Copy (void) const
{
  WranPortRangeTlvValue * tmp = new WranPortRangeTlvValue ();
  for (std::vector<struct PortRange>::const_iterator iter = m_portRange->begin (); iter != m_portRange->end (); ++iter)
    {
      tmp->Add ((*iter).PortLow, (*iter).PortHigh);
    }
  return tmp;
}

// ==============================================================================

WranProtocolTlvValue::WranProtocolTlvValue ()
{
  m_protocol = new std::vector<uint8_t>;
}
WranProtocolTlvValue::~WranProtocolTlvValue ()
{
  if (m_protocol != 0)
    {
      m_protocol->clear ();
      delete m_protocol;
      m_protocol = 0;
    }
}

uint32_t
WranProtocolTlvValue::GetSerializedSize (void) const
{
  return m_protocol->size ();
}

void
WranProtocolTlvValue::Serialize (Buffer::Iterator i) const
{
  for (std::vector<uint8_t>::const_iterator iter = m_protocol->begin (); iter != m_protocol->end (); ++iter)
    {
      i.WriteU8 ((*iter));
    }
}

uint32_t
WranProtocolTlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLength)
{
  uint64_t len = 0;
  while (len < valueLength)
    {
      Add (i.ReadU8 ());
      len++;
    }
  return len;
}

void
WranProtocolTlvValue::Add (uint8_t protocol)
{
  m_protocol->push_back (protocol);
}

WranProtocolTlvValue::Iterator
WranProtocolTlvValue::Begin (void) const
{
  return m_protocol->begin ();
}

WranProtocolTlvValue::Iterator
WranProtocolTlvValue::End (void) const
{
  return m_protocol->end ();
}

WranProtocolTlvValue*
WranProtocolTlvValue::Copy (void) const
{
  WranProtocolTlvValue* tmp = new WranProtocolTlvValue ();
  for (std::vector<uint8_t>::const_iterator iter = m_protocol->begin (); iter != m_protocol->end (); ++iter)
    {
      tmp->Add ((*iter));
    }
  return tmp;
}

// ==============================================================================

WranIpv4AddressTlvValue::WranIpv4AddressTlvValue ()
{
  m_ipv4Addr = new std::vector<struct ipv4Addr>;
}

WranIpv4AddressTlvValue::~WranIpv4AddressTlvValue ()
{
  if (m_ipv4Addr != 0)
    {
      m_ipv4Addr->clear ();
      delete m_ipv4Addr;
      m_ipv4Addr = 0;
    }
}

uint32_t
WranIpv4AddressTlvValue::GetSerializedSize (void) const
{
  return m_ipv4Addr->size () * sizeof(struct ipv4Addr);
}

void
WranIpv4AddressTlvValue::Serialize (Buffer::Iterator i) const
{
  for (std::vector<struct ipv4Addr>::const_iterator iter = m_ipv4Addr->begin (); iter != m_ipv4Addr->end (); ++iter)
    {
      i.WriteHtonU32 ((*iter).Address.Get ());
      i.WriteHtonU32 ((*iter).Mask.Get ());
    }
}

uint32_t
WranIpv4AddressTlvValue::Deserialize (Buffer::Iterator i, uint64_t valueLength)
{
  uint64_t len = 0;
  while (len < valueLength)
    {
      uint32_t addr = i.ReadNtohU32 ();
      uint32_t mask = i.ReadNtohU32 ();
      Add (Ipv4Address (addr), Ipv4Mask (mask));
      len += 8;
    }
  return len;
}

void
WranIpv4AddressTlvValue::Add (Ipv4Address address, Ipv4Mask Mask)
{
  struct ipv4Addr tmp;
  tmp.Address = address;
  tmp.Mask = Mask;
  m_ipv4Addr->push_back (tmp);
}

WranIpv4AddressTlvValue::Iterator
WranIpv4AddressTlvValue::Begin () const
{
  return m_ipv4Addr->begin ();
}

WranIpv4AddressTlvValue::Iterator
WranIpv4AddressTlvValue::End () const
{
  return m_ipv4Addr->end ();
}

WranIpv4AddressTlvValue *
WranIpv4AddressTlvValue::Copy (void) const
{
  WranIpv4AddressTlvValue * tmp = new WranIpv4AddressTlvValue ();
  for (std::vector<struct ipv4Addr>::const_iterator iter = m_ipv4Addr->begin (); iter != m_ipv4Addr->end (); ++iter)
    {
      tmp->Add ((*iter).Address, (*iter).Mask);
    }
  return tmp;
}

}
