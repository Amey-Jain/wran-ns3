/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2010 Green Network Research Group
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
#include "wran-mac-to-mac-header.h"
#include "ns3/address-utils.h"
#include "ns3/uinteger.h"
#include "ns3/log.h"
namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranMacToMacHeader);

WranMacToMacHeader::WranMacToMacHeader (void)
  : m_len (0)
{
}

WranMacToMacHeader::WranMacToMacHeader (uint32_t len)
  : m_len (len)
{
}

WranMacToMacHeader::~WranMacToMacHeader (void)
{
}



TypeId
WranMacToMacHeader::GetTypeId (void)
{
  static TypeId tid =
    TypeId ("ns3::WranMacToMacHeader").SetParent<Header> ().AddConstructor<WranMacToMacHeader> ();
  return tid;
}

TypeId
WranMacToMacHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

uint8_t
WranMacToMacHeader::GetSizeOfLen (void) const
{
  uint8_t sizeOfLen = 1;

  if (m_len > 127)
    {
      sizeOfLen = 2;
      uint64_t testValue = 0xFF;
      while (m_len > testValue)
        {
          sizeOfLen++;
          testValue *= 0xFF;
        }
    }
  return sizeOfLen;
}

uint32_t
WranMacToMacHeader::GetSerializedSize (void) const
{
  uint8_t sizeOfLen = GetSizeOfLen ();
  if (sizeOfLen==1)
    {
      return 20;
    }
  else
    {
      return 20 + sizeOfLen -1;
    }
  //return 19+sizeOfLen;
}

void
WranMacToMacHeader::Serialize (Buffer::Iterator i) const
{
  // The following header encoding was reverse-engineered by looking
  // at existing live pcap traces which could be opened with wireshark
  // i.e., we have no idea where this is coming from.
  //
  // 6 zeros for mac destination
  // 6 zeros for mac source
  // 2 bytes for length/type: 0x08f0
  // 2 bytes for sequence number: 0x0001
  // 2 bytes for number of tlvs: 0x0001
  // 1 byte for type of first tlv: 0x09
  // 1 byte to indicate the length of the length field of the tlv : 0x80 | 0x04
  // n bytes to indicate the size of the packet (network order)
  // n bytes for the packet data

  uint8_t zero = 0;


  for (int j = 0; j < 12; j++)
    {
      i.WriteU8 (zero);
    }
  i.WriteU16 (0xf008); // eth length/type
  i.WriteU16 (0x0100); // sequence number
  i.WriteU16 (0x0100); // number of tlvs
  i.WriteU8 (0x09); // type of first tlv
  uint8_t lenSize = GetSizeOfLen ();
  if (lenSize == 1)
    {
      i.WriteU8 (m_len);
    }
  else
    {
      i.WriteU8 ((lenSize-1) | 0x80);
      for (int j = 0; j < lenSize - 1; j++)
        {
          i.WriteU8 ((uint8_t)(m_len >> ((lenSize - 1 - 1 - j) * 8)));
        }
    }
}

uint32_t
WranMacToMacHeader::Deserialize (Buffer::Iterator start)
{
  // not needed here
  return 20;
}

void
WranMacToMacHeader::Print (std::ostream &os) const
{
}
};
