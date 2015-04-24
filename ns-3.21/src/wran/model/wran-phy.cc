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

#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "wran-net-device.h"
#include "wran-phy.h"
#include "wran-channel.h"
#include "ns3/packet-burst.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "ns3/uinteger.h"
#include "ns3/double.h"

NS_LOG_COMPONENT_DEFINE ("WranPhy");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranPhy);

TypeId WranPhy::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranPhy").SetParent<Object> ()

    .AddAttribute ("Channel",
                   "Wran channel",
                   PointerValue (),
                   MakePointerAccessor (&WranPhy::GetChannel, &WranPhy::Attach),
                   MakePointerChecker<WranChannel> ())

    .AddAttribute ("FrameDuration",
                   "The frame duration in seconds.",
                   TimeValue (Seconds (0.01)),
                   MakeTimeAccessor (&WranPhy::SetFrameDuration, &WranPhy::GetFrameDurationSec),
                   MakeTimeChecker ())

    .AddAttribute ("Frequency",
                   "The central frequency in KHz.",
                   UintegerValue (5000000),
                   MakeUintegerAccessor (&WranPhy::SetFrequency, &WranPhy::GetFrequency),
                   MakeUintegerChecker<uint32_t> (1000000, 11000000))

    .AddAttribute ("Bandwidth",
                   "The channel bandwidth in Hz.",
                   UintegerValue (6000000),
                   MakeUintegerAccessor (&WranPhy::SetChannelBandwidth, &WranPhy::GetChannelBandwidth),
                   MakeUintegerChecker<uint32_t> (5000000, 30000000))

  ;
  return tid;
}

WranPhy::WranPhy (void)
  : m_state (PHY_STATE_IDLE),
    m_nrCarriers (0),
    m_frameDuration (Seconds (0.01)),
    m_frequency (5000000),
    m_channelBandwidth (6000000),
    m_psDuration (Seconds (0)),
    m_symbolDuration (Seconds (0)),
    m_psPerSymbol (0),
    m_psPerFrame (0),
    m_symbolsPerFrame (0)
{
  m_mobility = 0;
  m_duplex = 0;
  m_txFrequency = 0;
  m_rxFrequency = 0;

}

WranPhy::~WranPhy (void)
{
}

void
WranPhy::DoDispose (void)
{
  m_device = 0;
  m_channel = 0;
}

void
WranPhy::Attach (Ptr<WranChannel> channel)
{
  m_channel = channel;
  DoAttach (channel);
}

Ptr<WranChannel>
WranPhy::GetChannel (void) const
{
  return m_channel;
}

void
WranPhy::SetDevice (Ptr<WranNetDevice> device)
{
  m_device = device;
}

Ptr<NetDevice>
WranPhy::GetDevice (void) const
{
  return m_device;
}

void
WranPhy::CallBackChecking(Callback<void, bool, uint64_t> callback) {
	NS_LOG_INFO("Calling from here");
	Simulator::Schedule (Time("10ms"), &WranPhy::EndChecking, this, callback);
	m_scanningCallback = callback;
//	m_scanningCallback(false, 1000);
}

void WranPhy::EndChecking(Callback<void, bool, uint64_t> callback){
	callback(false, 1000);
}

void
WranPhy::StartScanning (uint64_t frequency, Time timeout, Callback<void, bool, uint64_t> callback)
{
//	NS_ASSERT (/*!IsStateSwitching () &&*/ GetState() != WranPhy::PHY_STATE_SCANNING );
	  switch (m_state)
	    {
	    case WranPhy::PHY_STATE_RX:
	      NS_LOG_DEBUG ("channel sensing postponed until end of current reception");
	      Simulator::Schedule (/*GetDelayUntilIdle ()*/ Time("3s"), &WranPhy::StartScanning, this, frequency, timeout, callback);
	      /**
	       * Rationale about 0.1s dealy:
	       * Packet size is 10ms. probably 10ms later transmit/receive will end.
	       * If not it will wait for another 10ms
	       */
	      break;
	    case WranPhy::PHY_STATE_TX:
	      NS_LOG_DEBUG ("channel sensing postponed until end of current transmission");
	      Simulator::Schedule (/*GetDelayUntilIdle ()*/ Time("3s"), &WranPhy::StartScanning, this, frequency, timeout, callback);
	      break;
	    case WranPhy::PHY_STATE_IDLE:
//	    	NS_LOG_INFO("phy IDLE State with freq: " << frequency);
	      goto startSensing;
	      break;
	    case WranPhy::PHY_STATE_SCANNING:
//	    	NS_LOG_INFO("phy Scanning State with freq: " << frequency);
	    	goto startSensing;
	    	break;
	    default:
	      NS_ASSERT (false);
	      break;
	    }

	  return;

startSensing:

//	  NS_LOG_DEBUG ("sensing started for duration " << duration);
//	  m_state->SwitchToChannelSensing (duration);
//	  m_interference.EraseEvents ();
//	  Simulator::Schedule (duration, &YansWifiPhy::m_senseEndedCallback, this);

//	NS_LOG_DEBUG("Phy State " << m_state);
  NS_ASSERT_MSG (m_state == PHY_STATE_IDLE || m_state == PHY_STATE_SCANNING,
                 "Error while scanning: The PHY state should be PHY_STATE_SCANNING or PHY_STATE_IDLE");

  m_state = PHY_STATE_SCANNING;
  m_scanningFrequency = frequency;
//  NS_LOG_INFO("Phy scanning timout: " << timeout);
  m_dlChnlSrchTimeoutEvent = Simulator::Schedule (timeout, &WranPhy::EndScanning, this);
  m_scanningCallback = callback;
}

void
WranPhy::EndScanning (void)
{
	NS_LOG_INFO("End Scanning called");
  m_scanningCallback (false, m_scanningFrequency);
}

void
WranPhy::SetReceiveCallback (Callback<void, Ptr<const PacketBurst> > callback)
{
  m_rxCallback = callback;
}

Callback<void, Ptr<const PacketBurst> >
WranPhy::GetReceiveCallback (void) const
{
  return m_rxCallback;
}

void
WranPhy::SetDuplex (uint64_t rxFrequency, uint64_t txFrequency)
{
  m_txFrequency = txFrequency;
  m_rxFrequency = rxFrequency;
}

void
WranPhy::SetSimplex (uint64_t frequency)
{
  m_txFrequency = frequency;
  m_rxFrequency = frequency;
}

uint64_t
WranPhy::GetRxFrequency (void) const
{
  return m_rxFrequency;
}

uint64_t
WranPhy::GetTxFrequency (void) const
{
  return m_txFrequency;
}

uint64_t
WranPhy::GetScanningFrequency (void) const
{
  return m_scanningFrequency;
}

void
WranPhy::SetState (PhyState state)
{
  m_state = state;
}

WranPhy::PhyState WranPhy::GetState (void) const
{
  return m_state;
}

bool
WranPhy::IsDuplex (void) const
{
  return m_duplex;
}

EventId
WranPhy::GetChnlSrchTimeoutEvent (void) const
{
  return m_dlChnlSrchTimeoutEvent;
}

void
WranPhy::SetScanningCallback (void) const
{
  m_scanningCallback (true, GetScanningFrequency ());
}

void
WranPhy::SetDataRates (void)
{
  DoSetDataRates ();
}

uint32_t
WranPhy::GetDataRate (WranPhy::ModulationType modulationType) const
{
  return DoGetDataRate (modulationType);
}

Time
WranPhy::GetTransmissionTime (uint32_t size, WranPhy::ModulationType modulationType) const
{
  return DoGetTransmissionTime (size, modulationType);
}

uint64_t
WranPhy::GetNrSymbols (uint32_t size, WranPhy::ModulationType modulationType) const
{
  return DoGetNrSymbols (size, modulationType);
}

uint64_t
WranPhy::GetNrBytes (uint32_t symbols, WranPhy::ModulationType modulationType) const
{
  return DoGetNrBytes (symbols, modulationType);
}

uint16_t
WranPhy::GetTtg (void) const
{
  return DoGetTtg ();
}

uint16_t
WranPhy::GetRtg (void) const
{
  return DoGetRtg ();
}

uint8_t
WranPhy::GetFrameDurationCode (void) const
{
  return DoGetFrameDurationCode ();
}

Time
WranPhy::GetFrameDuration (uint8_t frameDurationCode) const
{
  return DoGetFrameDuration (frameDurationCode);
}

/*---------------------PHY parameters functions-----------------------*/

void
WranPhy::SetPhyParameters (void)
{
  DoSetPhyParameters ();
}

void
WranPhy::SetNrCarriers (uint8_t nrCarriers)
{
  m_nrCarriers = nrCarriers;
}

uint8_t
WranPhy::GetNrCarriers (void) const
{
  return m_nrCarriers;
}

void
WranPhy::SetFrameDuration (Time frameDuration)
{
  m_frameDuration = frameDuration;
}

Time
WranPhy::GetFrameDuration (void) const
{
  return GetFrameDurationSec ();
}

Time
WranPhy::GetFrameDurationSec (void) const
{
  return m_frameDuration;
}

void
WranPhy::SetFrequency (uint32_t frequency)
{
  m_frequency = frequency;
}

uint32_t
WranPhy::GetFrequency (void) const
{
  return m_frequency;
}

void
WranPhy::SetChannelBandwidth (uint32_t channelBandwidth)
{
  m_channelBandwidth = channelBandwidth;
}

uint32_t
WranPhy::GetChannelBandwidth (void) const
{
  return m_channelBandwidth;
}

uint16_t
WranPhy::GetNfft (void) const
{
  return DoGetNfft ();
}

double
WranPhy::GetSamplingFactor (void) const
{
  return DoGetSamplingFactor ();
}

double
WranPhy::GetSamplingFrequency (void) const
{
  return DoGetSamplingFrequency ();
}

void
WranPhy::SetPsDuration (Time psDuration)
{
  m_psDuration = psDuration;
}

Time
WranPhy::GetPsDuration (void) const
{
  return m_psDuration;
}

void
WranPhy::SetSymbolDuration (Time symbolDuration)
{
  m_symbolDuration = symbolDuration;
}

Time
WranPhy::GetSymbolDuration (void) const
{
  return m_symbolDuration;
}

double
WranPhy::GetGValue (void) const
{
  return DoGetGValue ();
}

void
WranPhy::SetPsPerSymbol (uint16_t psPerSymbol)
{
  m_psPerSymbol = psPerSymbol;
}

uint16_t
WranPhy::GetPsPerSymbol (void) const
{
  return m_psPerSymbol;
}

void
WranPhy::SetPsPerFrame (uint16_t psPerFrame)
{
  m_psPerFrame = psPerFrame;
}

uint16_t
WranPhy::GetPsPerFrame (void) const
{
  return m_psPerFrame;
}

void
WranPhy::SetSymbolsPerFrame (uint32_t symbolsPerFrame)
{
  m_symbolsPerFrame = symbolsPerFrame;
}

uint32_t
WranPhy::GetSymbolsPerFrame (void) const
{
  return m_symbolsPerFrame;
}
Ptr<Object>
WranPhy::GetMobility (void)
{
  return m_mobility;
}

void
WranPhy::SetMobility (Ptr<Object> mobility)
{
  m_mobility = mobility;

}

} // namespace ns3
