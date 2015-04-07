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

#ifndef WRAN_TLV_H
#define WRAN_TLV_H

#define WRAN_TLV_EXTENDED_LENGTH_MASK 0x80

#include "ns3/ipv4-address.h"
#include <cstdlib>
#include "ns3/log.h"
#include "ns3/assert.h"
#include "ns3/uinteger.h"
#include "ns3/header.h"
#include <vector>

namespace ns3 {

/**
 * \ingroup wran
 * \brief The value field of a tlv can take different values (uint8_t, uint16, vector...). This class is a virtual interface
 * that all the types of tlv values should derive
 */
class WranTlvValue
{
public:
  virtual ~WranTlvValue ()
  {
  }
  virtual uint32_t GetSerializedSize (void) const = 0;
  virtual void Serialize (Buffer::Iterator start) const = 0;
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLen ) = 0;
  virtual WranTlvValue * Copy (void) const = 0;
private:
};


// =============================================================================
/**
 * \ingroup wran
 * \brief This class implements the Type-Len-Value structure channel encodings as described by "IEEE Standard for
 * Local and metropolitan area networks Part 16: Air Interface for Fixed Broadband Wireless Access Systems"
 * 11. TLV encodings, page 645
 *
 */
class WranTlv : public Header
{
public:
  enum CommonTypes
  {
    HMAC_TUPLE = 149,
    MAC_VERSION_ENCODING = 148,
    CURRENT_TRANSMIT_POWER = 147,
    DOWNLINK_SERVICE_FLOW = 146,
    UPLINK_SERVICE_FLOW = 145,
    VENDOR_ID_EMCODING = 144,
    VENDOR_SPECIFIC_INFORMATION = 143
  };
  WranTlv (uint8_t type, uint64_t length, const WranTlvValue & value);
  WranTlv (void);
  ~WranTlv (void);
  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);
  uint8_t GetSizeOfLen (void) const;
  uint8_t GetType (void) const;
  uint64_t GetLength (void) const;
  WranTlvValue* PeekValue (void);
  WranTlv * Copy (void) const;
  WranTlvValue * CopyValue (void) const;
  WranTlv &operator = (WranTlv const& o);
  WranTlv (const WranTlv & tlv);

private:
  uint8_t m_type;
  uint64_t m_length;
  WranTlvValue * m_value;
};

// ==============================================================================
/**
 * \ingroup wran
 */
class WranU8TlvValue : public WranTlvValue
{
public:
  WranU8TlvValue (uint8_t value);
  WranU8TlvValue ();
  ~WranU8TlvValue (void);
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start,uint64_t valueLen);
  uint32_t Deserialize (Buffer::Iterator start);
  uint8_t GetValue (void) const;
  WranU8TlvValue * Copy (void) const;
private:
  uint8_t  m_value;
};

// ==============================================================================
/**
 * \ingroup wran
 */
class WranU16TlvValue : public WranTlvValue
{
public:
  WranU16TlvValue (uint16_t value);
  WranU16TlvValue ();
  ~WranU16TlvValue (void);
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start,uint64_t valueLen);
  uint32_t Deserialize (Buffer::Iterator start);
  uint16_t GetValue (void) const;
  virtual WranU16TlvValue * Copy (void) const;
private:
  uint16_t  m_value;
};

// ==============================================================================
/**
 * \ingroup wran
 */
class WranU32TlvValue : public WranTlvValue
{
public:
  WranU32TlvValue (uint32_t value);
  WranU32TlvValue ();
  ~WranU32TlvValue (void);

  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLen);
  uint32_t Deserialize (Buffer::Iterator start);
  uint32_t GetValue (void) const;
  virtual WranU32TlvValue * Copy (void) const;
private:
  uint32_t  m_value;
};

// ==============================================================================

/**
 * \ingroup wran
 * \brief this class is used to implement a vector of values in one tlv value field
 */
class WranVectorTlvValue : public WranTlvValue
{
public:
  typedef std::vector<WranTlv*>::const_iterator Iterator;
  WranVectorTlvValue (void);
  ~WranVectorTlvValue (void);
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLength) = 0;
  Iterator Begin () const;
  Iterator End () const;
  void Add (const WranTlv & val);
  virtual WranVectorTlvValue * Copy (void) const = 0;
private:
  std::vector<WranTlv*>  * m_tlvList;
};

// ==============================================================================
/**
 * \ingroup wran
 */
class SfWranVectorTlvValue : public WranVectorTlvValue
{

public:
  enum Type
  {
    SFID = 1,
    CID = 2,
    Service_Class_Name = 3,
    reserved1 = 4,
    QoS_Parameter_Set_Type = 5,
    Traffic_Priority = 6,
    Maximum_Sustained_Traffic_Rate = 7,
    Maximum_Traffic_Burst = 8,
    Minimum_Reserved_Traffic_Rate = 9,
    Minimum_Tolerable_Traffic_Rate = 10,
    Service_Flow_Scheduling_Type = 11,
    Request_Transmission_Policy = 12,
    Tolerated_Jitter = 13,
    Maximum_Latency = 14,
    Fixed_length_versus_Variable_length_SDU_Indicator = 15,
    SDU_Size = 16,
    Target_SAID = 17,
    ARQ_Enable = 18,
    ARQ_WINDOW_SIZE = 19,
    ARQ_RETRY_TIMEOUT_Transmitter_Delay = 20,
    ARQ_RETRY_TIMEOUT_Receiver_Delay = 21,
    ARQ_BLOCK_LIFETIME = 22,
    ARQ_SYNC_LOSS = 23,
    ARQ_DELIVER_IN_ORDER = 24,
    ARQ_PURGE_TIMEOUT = 25,
    ARQ_BLOCK_SIZE = 26,
    reserved2 = 27,
    CS_Specification = 28,
    IPV4_CS_Parameters = 100
  };
  SfWranVectorTlvValue ();
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLength);
  virtual SfWranVectorTlvValue * Copy (void) const;

};
// ==============================================================================

/**
 * \ingroup wran
 * \brief this class implements the convergence sub-layer descriptor as a tlv vector
 */
class CsParamWranVectorTlvValue : public WranVectorTlvValue
{
public:
  enum Type
  {
    Classifier_DSC_Action = 1,
    Packet_Classification_Rule = 3,
  };
  CsParamWranVectorTlvValue ();
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLength);
  virtual CsParamWranVectorTlvValue * Copy (void) const;
private:
};

// ==============================================================================

/**
 * \ingroup wran
 * \brief this class implements the classifier descriptor as a tlv vector
 */
class ClassificationRuleWranVectorTlvValue : public WranVectorTlvValue
{
public:
  enum ClassificationRuleTlvType
  {
    Priority = 1,
    ToS = 2,
    Protocol = 3,
    IP_src = 4,
    IP_dst = 5,
    Port_src = 6,
    Port_dst = 7,
    Index = 14,
  };
  ClassificationRuleWranVectorTlvValue ();
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLength);
  virtual ClassificationRuleWranVectorTlvValue * Copy (void) const;
private:
};

// ==============================================================================
/**
 * \ingroup wran
 */
class WranTosTlvValue : public WranTlvValue
{
public:
  WranTosTlvValue ();
  WranTosTlvValue (uint8_t, uint8_t, uint8_t);
  ~WranTosTlvValue ();
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLength);
  uint8_t GetLow (void) const;
  uint8_t GetHigh (void) const;
  uint8_t GetMask (void) const;
  virtual WranTosTlvValue * Copy () const;
private:
  uint8_t m_low;
  uint8_t m_high;
  uint8_t m_mask;
};

// ==============================================================================
/**
 * \ingroup wran
 */
class WranPortRangeTlvValue : public WranTlvValue
{
public:
  struct PortRange
  {
    uint16_t PortLow;
    uint16_t PortHigh;
  };
  typedef std::vector<struct PortRange>::const_iterator Iterator;
  WranPortRangeTlvValue ();
  ~WranPortRangeTlvValue ();
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLength);
  void Add (uint16_t portLow, uint16_t portHigh);
  Iterator Begin () const;
  Iterator End () const;
  virtual WranPortRangeTlvValue * Copy (void) const;
private:
  std::vector<struct PortRange> * m_portRange;
};

// ==============================================================================
/**
 * \ingroup wran
 */
class WranProtocolTlvValue : public WranTlvValue
{
public:
  WranProtocolTlvValue ();
  ~WranProtocolTlvValue ();
  typedef std::vector<uint8_t>::const_iterator Iterator;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLength);
  void Add (uint8_t protiocol);
  Iterator Begin () const;
  Iterator End () const;
  virtual WranProtocolTlvValue * Copy (void) const;
private:
  std::vector<uint8_t> * m_protocol;
};

// ==============================================================================

/**
 * \ingroup wran
 */
class WranIpv4AddressTlvValue : public WranTlvValue
{
public:
  struct ipv4Addr
  {
    Ipv4Address Address;
    Ipv4Mask Mask;
  };
  typedef std::vector<struct ipv4Addr>::const_iterator Iterator;
  WranIpv4AddressTlvValue ();
  ~WranIpv4AddressTlvValue ();
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start, uint64_t valueLength);
  void Add (Ipv4Address address, Ipv4Mask Mask);
  Iterator Begin () const;
  Iterator End () const;
  virtual WranIpv4AddressTlvValue * Copy () const;
private:
  std::vector<struct ipv4Addr> * m_ipv4Addr;
};

}

#endif /* WRAN_TLV_H */
