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

#include "wran-ss-link-manager.h"
#include <stdint.h>
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "wran-burst-profile-manager.h"
#include "wran-service-flow-manager.h"

NS_LOG_COMPONENT_DEFINE ("WranSSLinkManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranSSLinkManager);

TypeId WranSSLinkManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::WranSSLinkManager")
    .SetParent<Object> ();
  return tid;
}

WranSSLinkManager::WranSSLinkManager (Ptr<WranSubscriberStationNetDevice> ss)
  : m_ss (ss),
    m_rangingStatus (WranNetDevice::RANGING_STATUS_EXPIRED),
    m_bsEirp (65535),
    m_eirXPIrMax (65535),
    m_pTxIrMax (0),
    m_initRangOppNumber (0),
    m_contentionRangingRetries (0),
    m_rngReqFrameNumber (0),
    m_dlChnlNr (0),
    m_frequency (0),
    m_rangingIntervalFound (false),
    m_nrRngReqsSent (0),
    m_nrRngRspsRecvd (0),
    m_nrInvitedPollsRecvd (0),
    m_rangingCW (0),
    m_rangingBO (0),
    m_nrRangingTransOpps (0),
    m_isBackoffSet (false),
    m_rangingAnomalies (0)
{

}

WranSSLinkManager::~WranSSLinkManager (void)
{
  m_ss = 0;
}


void
WranSSLinkManager::DoDispose (void)
{
  m_ss = 0;
}

void
WranSSLinkManager::SetBsEirp (uint16_t bs_eirp)
{
  m_bsEirp = bs_eirp;
}

void
WranSSLinkManager::SetEirXPIrMax (uint16_t eir_x_p_ir_max)
{
  m_eirXPIrMax = eir_x_p_ir_max;
}

void
WranSSLinkManager::SetRangingIntervalFound (bool rangingIntervalFound)
{
  m_rangingIntervalFound = rangingIntervalFound;
}

bool
WranSSLinkManager::GetRangingIntervalFound (void) const
{
  return m_rangingIntervalFound;
}

void
WranSSLinkManager::SetNrRangingTransOpps (uint8_t nrRangingTransOpps)
{
  m_nrRangingTransOpps = nrRangingTransOpps;
}

void
WranSSLinkManager::SetRangingCW (uint8_t rangingCW)
{
  m_rangingCW = rangingCW;
}

void
WranSSLinkManager::IncrementNrInvitedPollsRecvd (void)
{
  m_nrInvitedPollsRecvd++;
}

EventId
WranSSLinkManager::GetDlMapSyncTimeoutEvent (void)
{
  return m_dlMapSyncTimeoutEvent;
}

void
WranSSLinkManager::StartScanning (
  WranSubscriberStationNetDevice::EventType type, bool deleteParameters)
{
  // temp parameter "type" just to check on expiry of which event the function was called

  if (deleteParameters)
    {
      DeleteUplinkParameters ();
    }

  NS_ASSERT_MSG (!m_ss->IsRegistered (),
                 "Subscriber Station: Error while scanning: Already registered with a BS");

  if (m_ss->GetState () != WranSubscriberStationNetDevice::SS_STATE_IDLE)
    {
      m_dlChnlNr++;
    }

  // using max number of channel according to according to Section 8.5.1 of IEEE 802.16-2004 standard.
  if (m_dlChnlNr >= 200)
    {
      m_dlChnlNr = 0;
    }

  uint64_t dlChannel = m_ss->GetChannel (m_dlChnlNr);

  m_ss->SetState (WranSubscriberStationNetDevice::SS_STATE_SCANNING);
  m_ss->GetPhy ()->StartScanning (dlChannel, m_ss->GetIntervalT20 (),
                                  MakeCallback (&WranSSLinkManager::EndScanning, this));
}

void
WranSSLinkManager::EndScanning (bool status, uint64_t frequency)
{
  if (status)
    {
      StartSynchronizing ();
      m_frequency = frequency;
    }
  else
    {
      StartScanning (WranSubscriberStationNetDevice::EVENT_NONE, false);
    }
}

void
WranSSLinkManager::StartSynchronizing (void)
{
  m_ss->SetState (WranSubscriberStationNetDevice::SS_STATE_SYNCHRONIZING);
  m_ss->SetTimer (Simulator::Schedule (m_ss->GetIntervalT21 (),
                                       &WranSSLinkManager::StartScanning, this,
                                       WranSubscriberStationNetDevice::EVENT_DL_MAP_SYNC_TIMEOUT, false),
                  m_dlMapSyncTimeoutEvent);
}

void
WranSSLinkManager::SendRangingRequest (uint8_t uiuc, uint16_t allocationSize)
{
  NS_ASSERT_MSG (
    m_ss->GetState ()
    == WranSubscriberStationNetDevice::SS_STATE_WAITING_REG_RANG_INTRVL
    || m_ss->GetState ()
    == WranSubscriberStationNetDevice::SS_STATE_WAITING_INV_RANG_INTRVL,
    "SS: Error while sending a ranging request: the ss state should be SS_STATE_WAITING_REG_RANG_INTRVL or SS_STATE_WAITING_INV_RANG_INTRVL");

  if (m_nrRngReqsSent == 0) // sending the first time
    {
      m_pTxIrMax = CalculateMaxIRSignalStrength ();
      m_rngreq.SetReqDlBurstProfile (
        m_ss->GetWranBurstProfileManager ()->GetBurstProfileToRequest ());
      m_rngreq.SetMacAddress (m_ss->GetMacAddress ());
    }
  else
    {
      m_pTxIrMax++;
      if (m_nrRngRspsRecvd > 0)
        {
          m_rngreq.SetRangingAnomalies (m_rangingAnomalies);
        }
    }

  Ptr<Packet> packet = Create<Packet> ();
  Ptr<PacketBurst> burst = Create<PacketBurst> ();

  packet->AddHeader (m_rngreq);
  packet->AddHeader (WranManagementMessageType (
                       WranManagementMessageType::MESSAGE_TYPE_RNG_REQ));

  Ptr<WranConnection> connection;

  if (m_rangingStatus == WranNetDevice::RANGING_STATUS_CONTINUE)
    {
      connection = m_ss->GetBasicConnection ();
    }
  else     // have been assigned BCID, means currently adjusting parameters
    {
      connection = m_ss->GetInitialRangingConnection ();
    }

  m_ss->Enqueue (packet, MacHeaderType (), connection);

  m_ss->SetState (WranSubscriberStationNetDevice::SS_STATE_WAITING_RNG_RSP);
  m_ss->SetTimer (Simulator::Schedule (m_ss->GetIntervalT3 (),
                                       &WranSSLinkManager::StartContentionResolution, this), m_waitForRngRspEvent);
  m_nrRngReqsSent++;

  NS_ASSERT_MSG (allocationSize
                 == m_ss->GetCurrentUcd ().GetChannelEncodings ().GetRangReqOppSize ()
                 / m_ss->GetPhy ()->GetPsPerSymbol (),
                 "SS: Error while sending a ranging request: the allocation size is not correct");

  // will work even if connection is not passed (i.e. null is passed) as scheduler will automatically select the same connection
  m_ss->SendBurst (uiuc, allocationSize, connection);
}

void
WranSSLinkManager::StartContentionResolution (void)
{
  NS_ASSERT_MSG (
    m_ss->GetState ()
    == WranSubscriberStationNetDevice::SS_STATE_WAITING_RNG_RSP
    || m_ss->GetState ()
    == WranSubscriberStationNetDevice::SS_STATE_WAITING_REG_RANG_INTRVL
    || m_ss->GetState ()
    == WranSubscriberStationNetDevice::SS_STATE_ADJUSTING_PARAMETERS,
    "SS: Can not start connection resolution: The SS state should be SS_STATE_WAITING_RNG_RSP or SS_STATE_WAITING_REG_RANG_INTRVL or SS_STATE_ADJUSTING_PARAMETERS");

  if (m_ss->GetState ()
      == WranSubscriberStationNetDevice::SS_STATE_WAITING_RNG_RSP)
    {
      m_ss->SetState (
        WranSubscriberStationNetDevice::SS_STATE_WAITING_REG_RANG_INTRVL);
      IncreaseRangingRequestCW ();
      m_contentionRangingRetries++;
    }
  else if (m_ss->GetState ()
           == WranSubscriberStationNetDevice::SS_STATE_ADJUSTING_PARAMETERS)
    {
      m_ss->SetState (
        WranSubscriberStationNetDevice::SS_STATE_WAITING_REG_RANG_INTRVL);
    }

  if (m_contentionRangingRetries == m_ss->GetMaxContentionRangingRetries ())
    {
      StartScanning (WranSubscriberStationNetDevice::EVENT_NONE, false);
    }
  else
    {
      if (!m_isBackoffSet)
        {
          SelectRandomBackoff ();
        }
    }
}

void
WranSSLinkManager::PerformBackoff (void)
{
  Time defferTime = Seconds (0);
  Time timeToAllocation = Seconds (0);
  uint16_t nrPsPerRangOpp =
    m_ss->GetCurrentUcd ().GetChannelEncodings ().GetRangReqOppSize ();
  uint16_t oppSize =
    m_ss->GetCurrentUcd ().GetChannelEncodings ().GetRangReqOppSize ()
    / m_ss->GetPhy ()->GetPsPerSymbol ();

  for (uint8_t deferTOs = 0; deferTOs < m_nrRangingTransOpps; deferTOs++)
    {
      if (m_rangingBO == 0)
        {
          defferTime = Seconds (deferTOs * nrPsPerRangOpp
                                * m_ss->GetPhy ()->GetPsDuration ().GetSeconds ());
          timeToAllocation = m_ss->GetTimeToAllocation (defferTime);

          Simulator::Schedule (timeToAllocation,
                               &WranSSLinkManager::SendRangingRequest, this,
                               OfdmUlBurstProfile::UIUC_INITIAL_RANGING, oppSize);

          m_rngReqFrameNumber = m_ss->GetNrFrames ();
          m_initRangOppNumber = deferTOs + 1;

          m_isBackoffSet = false;
          break;
        }
      m_rangingBO--;
    }
}

void
WranSSLinkManager::SelectRandomBackoff (void)
{
  NS_ASSERT_MSG (m_rangingCW != 0 && m_rangingBO == 0,
                 "be sure that CW has been set and BO is not already set"); // ensuring CW has been set and BO is not already set

  m_rangingBO = (rand () % m_rangingCW);
  m_isBackoffSet = true;
}

void
WranSSLinkManager::IncreaseRangingRequestCW (void)
{
  m_rangingCW = std::min (uint8_t ((m_rangingCW * 2 + 1) - 1),
                          m_ss->GetCurrentUcd ().GetRangingBackoffEnd ());
}

void
WranSSLinkManager::ResetRangingRequestCW (void)
{
  m_rangingCW = (uint8_t) std::pow ((double) 2,
                                    (double) m_ss->GetCurrentUcd ().GetRangingBackoffStart ()) - 1;
}

void
WranSSLinkManager::PerformRanging (Cid cid,
                               RngRsp rngrsp)
{
  // need to distinguish initial ranging or periodic ranging

  if (cid == m_ss->GetInitialRangingConnection ()->GetCid ())
    {
      if (rngrsp.GetFrameNumber () == m_rngReqFrameNumber
          && rngrsp.GetInitRangOppNumber () == m_initRangOppNumber)
        {
          Simulator::Cancel (m_waitForRngRspEvent);
          m_nrRngRspsRecvd++;

          // RNG-REQ was undecodable
          ResetRangingRequestCW ();
          AdjustRangingParameters (rngrsp);
          m_ss->SetState (
            WranSubscriberStationNetDevice::SS_STATE_ADJUSTING_PARAMETERS);
          return;
        }

      if (m_ss->GetAddress () != rngrsp.GetMacAddress ())
        {
          return;
        }

      m_ss->SetBasicConnection (CreateObject<WranConnection> (rngrsp.GetBasicCid (),
                                                               Cid::BASIC));

      m_ss->SetPrimaryConnection (CreateObject<WranConnection> (rngrsp.GetPrimaryCid (),
                                                                 Cid::PRIMARY));
      m_ss->SetAreManagementConnectionsAllocated (true);
    }
  else
    {
      // either periodic ranging or an additional RNG-RSP during initial ranging
    }

  m_nrRngRspsRecvd++;
  if (m_waitForRngRspEvent.IsRunning ())
    {
      Simulator::Cancel (m_waitForRngRspEvent);
    }

  m_rangingStatus = (WranNetDevice::RangingStatus) rngrsp.GetRangStatus ();

  NS_ASSERT_MSG (
    m_rangingStatus == WranNetDevice::RANGING_STATUS_CONTINUE
    || m_rangingStatus == WranNetDevice::RANGING_STATUS_ABORT
    || m_rangingStatus == WranNetDevice::RANGING_STATUS_SUCCESS,
    "SS: Can not perform ranging: the ranging status should be RANGING_STATUS_CONTINUE or RANGING_STATUS_ABORT or RANGING_STATUS_SUCCESS");

  if (m_rangingStatus == WranNetDevice::RANGING_STATUS_ABORT)
    {
      if (rngrsp.GetDlFreqOverride ())
        {
          // code to move to new channel/frequency goes here
        }
      // deassigning basic and primary CIDs
      m_ss->SetBasicConnection (0);
      m_ss->SetPrimaryConnection (0);
      m_ss->SetAreManagementConnectionsAllocated (false);
    }
  else
    {
      AdjustRangingParameters (rngrsp);

      if (m_rangingStatus == WranNetDevice::RANGING_STATUS_SUCCESS)
        {

          m_ss->SetState (WranSubscriberStationNetDevice::SS_STATE_REGISTERED);
          // initiate service flows
          if (m_ss->HasWranServiceFlows () && !m_ss->GetAreWranServiceFlowsAllocated ())
            {
              m_ss->GetWranServiceFlowManager ()->InitiateWranServiceFlows ();
            }

          NegotiateBasicCapabilities ();
        }
      else
        {

          m_ss->SetState (
            WranSubscriberStationNetDevice::SS_STATE_WAITING_INV_RANG_INTRVL);
          // wait for invited ranging interval assigned to its Basic CID
        }
    }
}

void
WranSSLinkManager::DeleteUplinkParameters (void)
{
  m_ss->SetCurrentUcd (Ucd ());
}

bool
WranSSLinkManager::IsUlChannelUsable (void)
{
  // dont know how to check if usable, see Figure 58.
  return true; // temporarily assuming usable
}

void
WranSSLinkManager::AdjustRangingParameters (const RngRsp &rngrsp)
{
#if 0 /* a template for future implementation following */
  bool successful = true;
  uint8_t temp = rngrsp.GetTimingAdjust ();
  temp = rngrsp.GetPowerLevelAdjust ();
  temp = rngrsp.GetOffsetFreqAdjust ();

  // code for adjusting parameters goes here

  if (!successful)
    {
      // code for setting ranging anomalies goes here
    }
#endif
}

void
WranSSLinkManager::NegotiateBasicCapabilities (void)
{
  // code to nagotiate basic capabilites goes here, ignored until very advanced stages
}

uint16_t
WranSSLinkManager::CalculateMaxIRSignalStrength (void)
{
  // SS obtains RSSI measurement from the OFDM downlink preambles using a complex formula, page 486
  uint16_t rss = 1;

  if (m_bsEirp == 65535 || m_eirXPIrMax == 65535)
    {
      return GetMinTransmitPowerLevel ();
    }
  else
    {
      return m_eirXPIrMax + m_bsEirp - rss;
    }

  return 0;
}

uint16_t
WranSSLinkManager::GetMinTransmitPowerLevel (void)
{
  // code to calculate minimum transmit power level of the SS, see page 189 of ammendment
  return 10; // temp
}

void
WranSSLinkManager::ScheduleScanningRestart (Time interval,
                                        WranSubscriberStationNetDevice::EventType eventType,
                                        bool deleteUlParameters, EventId &eventId)
{
  m_ss->SetTimer (Simulator::Schedule (interval, &WranSSLinkManager::StartScanning,
                                       this, eventType, deleteUlParameters), eventId);
}

} // namespace ns3
