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
 * Author:  
 *                              <amine.ismail@udcast.com>
 */

#include "wran-simple-ofdm-send-param.h"
#include "simple-ofdm-wran-phy.h"
#include "simple-ofdm-wran-channel.h"

namespace ns3 {
WranSimpleOfdmSendParam::WranSimpleOfdmSendParam (void)
{
  // m_fecBlock = 0;
  m_burstSize = 0;
  m_isFirstBlock = 0;
  m_frequency = 0;
  m_modulationType = WranPhy::MODULATION_TYPE_QPSK_12;
  m_direction = 0;
  m_rxPowerDbm = 0;

}
WranSimpleOfdmSendParam::WranSimpleOfdmSendParam (const bvec &fecBlock,
                                          uint32_t burstSize,
                                          bool isFirstBlock,
                                          uint64_t Frequency,
                                          WranPhy::ModulationType modulationType,
                                          uint8_t direction,
                                          double rxPowerDbm)
{

  m_fecBlock = fecBlock;
  m_burstSize = burstSize;
  m_isFirstBlock = isFirstBlock;
  m_frequency = Frequency;
  m_modulationType = modulationType;
  m_direction = direction;
  m_rxPowerDbm = rxPowerDbm;
}

WranSimpleOfdmSendParam::WranSimpleOfdmSendParam (uint32_t burstSize,
                                          bool isFirstBlock,
                                          uint64_t Frequency,
                                          WranPhy::ModulationType modulationType,
                                          uint8_t direction,
                                          double rxPowerDbm,
                                          Ptr<PacketBurst> burst)
{
  m_burstSize = burstSize;
  m_isFirstBlock = isFirstBlock;
  m_frequency = Frequency;
  m_modulationType = modulationType;
  m_direction = direction;
  m_rxPowerDbm = rxPowerDbm;
  m_burst = burst;
}

WranSimpleOfdmSendParam::~WranSimpleOfdmSendParam (void)
{

}

void
WranSimpleOfdmSendParam::SetFecBlock (const bvec &fecBlock)
{
  m_fecBlock = fecBlock;
}

void
WranSimpleOfdmSendParam::SetBurstSize (uint32_t burstSize)
{
  m_burstSize = burstSize;
}
void
WranSimpleOfdmSendParam::SetIsFirstBlock (bool isFirstBlock)
{
  m_isFirstBlock = isFirstBlock;
}
void
WranSimpleOfdmSendParam::SetFrequency (uint64_t Frequency)
{
  m_frequency = Frequency;
}
void
WranSimpleOfdmSendParam::SetModulationType (WranPhy::ModulationType modulationType)
{
  m_modulationType = modulationType;
}
void
WranSimpleOfdmSendParam::SetDirection (uint8_t direction)
{
  m_direction = direction;
}
void
WranSimpleOfdmSendParam::SetRxPowerDbm (double rxPowerDbm)
{
  m_rxPowerDbm = rxPowerDbm;
}

bvec
WranSimpleOfdmSendParam::GetFecBlock (void)
{
  return m_fecBlock;
}
uint32_t
WranSimpleOfdmSendParam::GetBurstSize (void)
{
  return m_burstSize;
}
bool
WranSimpleOfdmSendParam::GetIsFirstBlock (void)
{
  return m_isFirstBlock;
}
uint64_t
WranSimpleOfdmSendParam::GetFrequency (void)
{
  return m_frequency;
}
WranPhy::ModulationType
WranSimpleOfdmSendParam::GetModulationType (void)
{
  return m_modulationType;
}
uint8_t
WranSimpleOfdmSendParam::GetDirection (void)
{
  return m_direction;
}
double
WranSimpleOfdmSendParam::GetRxPowerDbm (void)
{
  return m_rxPowerDbm;
}
Ptr<PacketBurst>
WranSimpleOfdmSendParam::GetBurst (void)
{
  return m_burst;
}

}
