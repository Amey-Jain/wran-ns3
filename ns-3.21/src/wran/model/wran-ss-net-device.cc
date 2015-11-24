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

#include "ns3/simulator.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/node.h"
#include "wran-ss-net-device.h"
#include "wran-phy.h"
#include "ns3/packet-burst.h"
#include <algorithm>
#include <cstring>
#include <string>
#include <iostream>
#include <sstream>
#include "ns3/dl-mac-messages.h"
#include "ns3/ul-mac-messages.h"
#include "wran-ss-scheduler.h"
#include "wran-mac-queue.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "wran-service-flow.h"
#include "wran-service-flow-record.h"
#include "wran-service-flow-manager.h"
#include "wran-connection-manager.h"
#include "wran-burst-profile-manager.h"
#include "wran-ss-link-manager.h"
#include "wran-bandwidth-manager.h"

NS_LOG_COMPONENT_DEFINE ("WranSubscriberStationNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (WranSubscriberStationNetDevice);

Time WranSubscriberStationNetDevice::GetDefaultLostDlMapInterval ()
{
  return (MicroSeconds (500000));
}

TypeId
WranSubscriberStationNetDevice::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::WranSubscriberStationNetDevice")

    .SetParent<WranNetDevice> ()

    .AddConstructor<WranSubscriberStationNetDevice> ()

    .AddAttribute ("BasicConnection",
                   "Basic connection",
                   PointerValue (),
                   MakePointerAccessor (&WranSubscriberStationNetDevice::m_basicConnection),
                   MakePointerChecker<WranConnection> ())

    .AddAttribute ("PrimaryConnection",
                   "Primary connection",
                   PointerValue (),
                   MakePointerAccessor (&WranSubscriberStationNetDevice::m_primaryConnection),
                   MakePointerChecker<WranConnection> ())

    .AddAttribute ("LostDlMapInterval",
                   "Time since last received DL-MAP message before downlink synchronization is considered lost. Maximum is 600ms",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetLostDlMapInterval,
                                     &WranSubscriberStationNetDevice::SetLostDlMapInterval),
                   MakeTimeChecker ())

    .AddAttribute ("LostUlMapInterval",
                   "Time since last received UL-MAP before uplink synchronization is considered lost, maximum is 600.",
                   TimeValue (MilliSeconds (500)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetLostUlMapInterval,
                                     &WranSubscriberStationNetDevice::SetLostUlMapInterval),
                   MakeTimeChecker ())

    .AddAttribute ("MaxDcdInterval",
                   "Maximum time between transmission of DCD messages. Maximum is 10s",
                   TimeValue (Seconds (10)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetMaxDcdInterval,
                                     &WranSubscriberStationNetDevice::SetMaxDcdInterval),
                   MakeTimeChecker ())

    .AddAttribute ("MaxUcdInterval",
                   "Maximum time between transmission of UCD messages. Maximum is 10s",
                   TimeValue (Seconds (10)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetMaxUcdInterval,
                                     &WranSubscriberStationNetDevice::SetMaxUcdInterval),
                   MakeTimeChecker ())

    .AddAttribute ("IntervalT1",
                   "Wait for DCD timeout. Maximum is 5*maxDcdInterval",
                   TimeValue (Seconds (50)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetIntervalT1,
                                     &WranSubscriberStationNetDevice::SetIntervalT1),
                   MakeTimeChecker ())

    .AddAttribute ("IntervalT2",
                   "Wait for broadcast ranging timeout, i.e., wait for initial ranging opportunity. Maximum is 5*Ranging interval",
                   TimeValue (Seconds (10)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetIntervalT2,
                                     &WranSubscriberStationNetDevice::SetIntervalT2),
                   MakeTimeChecker ())

    .AddAttribute ("IntervalT3",
                   "ranging Response reception timeout following the transmission of a ranging request. Maximum is 200ms",
                   TimeValue (Seconds (0.2)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetIntervalT3,
                                     &WranSubscriberStationNetDevice::SetIntervalT3),
                   MakeTimeChecker ())

    .AddAttribute ("IntervalT7",
                   "wait for DSA/DSC/DSD Response timeout. Maximum is 1s",
                   TimeValue (Seconds (0.1)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetIntervalT7,
                                     &WranSubscriberStationNetDevice::SetIntervalT7),
                   MakeTimeChecker ())

    .AddAttribute ("IntervalT12",
                   "Wait for UCD descriptor.Maximum is 5*MaxUcdInterval",
                   TimeValue (Seconds (10)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetIntervalT12,
                                     &WranSubscriberStationNetDevice::SetIntervalT12),
                   MakeTimeChecker ())

    .AddAttribute ("IntervalT20",
                   "Time the SS searches for preambles on a given channel. Minimum is 2 MAC frames",
                   TimeValue (Seconds (0.5)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetIntervalT20,
                                     &WranSubscriberStationNetDevice::SetIntervalT20),
                   MakeTimeChecker ())

    .AddAttribute ("IntervalT21",
                   "time the SS searches for (decodable) DL-MAP on a given channel",
                   TimeValue (Seconds (10)),
                   MakeTimeAccessor (&WranSubscriberStationNetDevice::GetIntervalT21,
                                     &WranSubscriberStationNetDevice::SetIntervalT21),
                   MakeTimeChecker ())

    .AddAttribute ("MaxContentionRangingRetries",
                   "Number of retries on contention Ranging Requests",
                   UintegerValue (16),
                   MakeUintegerAccessor (&WranSubscriberStationNetDevice::GetMaxContentionRangingRetries,
                                         &WranSubscriberStationNetDevice::SetMaxContentionRangingRetries),
                   MakeUintegerChecker<uint8_t> (1, 16))

    .AddAttribute ("WranSSScheduler",
                   "The ss scheduler attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&WranSubscriberStationNetDevice::GetScheduler,
                                        &WranSubscriberStationNetDevice::SetScheduler),
                   MakePointerChecker<WranSSScheduler> ())

    .AddAttribute ("LinkManager",
                   "The ss link manager attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&WranSubscriberStationNetDevice::GetLinkManager,
                                        &WranSubscriberStationNetDevice::SetLinkManager),
                   MakePointerChecker<WranSSLinkManager> ())

    .AddAttribute ("Classifier",
                   "The ss classifier attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&WranSubscriberStationNetDevice::GetWranIpcsClassifier,
                                        &WranSubscriberStationNetDevice::SetIpcsPacketClassifier),
                   MakePointerChecker<WranIpcsClassifier> ())

    .AddTraceSource ("SSTxDrop",
                     "A packet has been dropped in the MAC layer before being queued for transmission.",
                     MakeTraceSourceAccessor (&WranSubscriberStationNetDevice::m_ssTxDropTrace))

    .AddTraceSource ("SSPromiscRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&WranSubscriberStationNetDevice::m_ssPromiscRxTrace))

    .AddTraceSource ("SSRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&WranSubscriberStationNetDevice::m_ssRxTrace))

    .AddTraceSource ("SSRxDrop",
                     "A packet has been dropped in the MAC layer after it has been passed up from the physical "
                     "layer.",
                     MakeTraceSourceAccessor (&WranSubscriberStationNetDevice::m_ssRxDropTrace));
  return tid;
}

WranSubscriberStationNetDevice::WranSubscriberStationNetDevice (void)
{

  InitWranSubscriberStationNetDevice ();

}

void
WranSubscriberStationNetDevice::InitWranSubscriberStationNetDevice (void)
{
  m_lostDlMapInterval = MilliSeconds (500);
  m_lostUlMapInterval = MilliSeconds (500);
  m_maxDcdInterval = Seconds (10);
  m_maxUcdInterval = Seconds (10);
  m_intervalT1 = Seconds (5 * m_maxDcdInterval.GetSeconds ());
  m_intervalT2 = Seconds (5 * 2); // shall be 5 * RangingInterval, if ranging interval=see T2 at page 638) means Initial Ranging Interval=see page 637)
  m_intervalT3 = MilliSeconds (200);
  m_intervalT7 = Seconds (0.1); // maximum is 1
  m_intervalT12 = Seconds (5 * m_maxUcdInterval.GetSeconds ());
  m_intervalT21 = Seconds (11);
  m_maxContentionRangingRetries = 16;
  m_dcdCount = 0;
  m_baseStationId = Mac48Address ("00:00:00:00:00:00");
  m_ucdCount = 0;
  m_allocationStartTime = 0;
  m_nrDlMapElements = 0;
  m_nrUlMapElements = 0;
  m_nrDlMapRecvd = 0;
  m_nrUlMapRecvd = 0;
  m_nrDcdRecvd = 0;
  m_nrUcdRecvd = 0;
  m_modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
  m_areManagementConnectionsAllocated = false;
  m_areWranServiceFlowsAllocated = false;

  m_basicConnection = 0;
  m_primaryConnection = 0;

  m_dlBurstProfile = new OfdmDlBurstProfile ();
  m_ulBurstProfile = new OfdmUlBurstProfile ();
  m_classifier = CreateObject<WranIpcsClassifier> ();
  m_linkManager = CreateObject<WranSSLinkManager> (this);
  m_scheduler = CreateObject<WranSSScheduler> (this);
  m_serviceFlowManager = CreateObject<WranSSServiceFlowManager> (this);
}

WranSubscriberStationNetDevice::WranSubscriberStationNetDevice (Ptr<Node> node, Ptr<WranPhy> phy)
{
  InitWranSubscriberStationNetDevice ();
  this->SetNode (node);
  this->SetPhy (phy);
}

WranSubscriberStationNetDevice::~WranSubscriberStationNetDevice (void)
{
}

void
WranSubscriberStationNetDevice::DoDispose (void)
{
  delete m_dlBurstProfile;
  delete m_ulBurstProfile;
  m_scheduler = 0;
  m_serviceFlowManager = 0;
  m_basicConnection = 0;
  m_primaryConnection = 0;
  m_classifier = 0;
  m_dlBurstProfile = 0;
  m_ulBurstProfile = 0;

  m_linkManager = 0;

  WranNetDevice::DoDispose ();
}

void
WranSubscriberStationNetDevice::SetLostDlMapInterval (Time lostDlMapInterval)
{
  m_lostDlMapInterval = lostDlMapInterval;
}

Time
WranSubscriberStationNetDevice::GetLostDlMapInterval (void) const
{
  return m_lostDlMapInterval;
}

void
WranSubscriberStationNetDevice::SetLostUlMapInterval (Time lostUlMapInterval)
{
  m_lostUlMapInterval = lostUlMapInterval;
}

Time
WranSubscriberStationNetDevice::GetLostUlMapInterval (void) const
{
  return m_lostUlMapInterval;
}

void
WranSubscriberStationNetDevice::SetMaxDcdInterval (Time maxDcdInterval)
{
  m_maxDcdInterval = maxDcdInterval;
}

Time
WranSubscriberStationNetDevice::GetMaxDcdInterval (void) const
{
  return m_maxDcdInterval;
}

void
WranSubscriberStationNetDevice::SetMaxUcdInterval (Time maxUcdInterval)
{
  m_maxUcdInterval = maxUcdInterval;
}

Time
WranSubscriberStationNetDevice::GetMaxUcdInterval (void) const
{
  return m_maxUcdInterval;
}

void
WranSubscriberStationNetDevice::SetIntervalT1 (Time interval)
{
  m_intervalT1 = interval;
}

Time
WranSubscriberStationNetDevice::GetIntervalT1 (void) const
{
  return m_intervalT1;
}

void
WranSubscriberStationNetDevice::SetIntervalT2 (Time interval)
{
  m_intervalT2 = interval;
}

Time
WranSubscriberStationNetDevice::GetIntervalT2 (void) const
{
  return m_intervalT2;
}

void
WranSubscriberStationNetDevice::SetIntervalT3 (Time interval)
{
  m_intervalT3 = interval;
}

Time
WranSubscriberStationNetDevice::GetIntervalT3 (void) const
{
  return m_intervalT3;
}

void
WranSubscriberStationNetDevice::SetIntervalT7 (Time interval)
{
  m_intervalT7 = interval;
}

Time
WranSubscriberStationNetDevice::GetIntervalT7 (void) const
{
  return m_intervalT7;
}

void
WranSubscriberStationNetDevice::SetIntervalT12 (Time interval)
{
  m_intervalT12 = interval;
}

Time
WranSubscriberStationNetDevice::GetIntervalT12 (void) const
{
  return m_intervalT12;
}

void
WranSubscriberStationNetDevice::SetIntervalT20 (Time interval)
{
  m_intervalT20 = interval;
}

Time
WranSubscriberStationNetDevice::GetIntervalT20 (void) const
{
  return m_intervalT20;
}

void
WranSubscriberStationNetDevice::SetIntervalT21 (Time interval)
{
  m_intervalT21 = interval;
}

Time
WranSubscriberStationNetDevice::GetIntervalT21 (void) const
{
  return m_intervalT21;
}

void
WranSubscriberStationNetDevice::SetMaxContentionRangingRetries (uint8_t maxContentionRangingRetries)
{
  m_maxContentionRangingRetries = maxContentionRangingRetries;
}

uint8_t
WranSubscriberStationNetDevice::GetMaxContentionRangingRetries (void) const
{
  return m_maxContentionRangingRetries;
}

void
WranSubscriberStationNetDevice::SetBasicConnection (Ptr<WranConnection> basicConnection)
{
  m_basicConnection = basicConnection;
}

Ptr<WranConnection>
WranSubscriberStationNetDevice::GetBasicConnection (void) const
{
  return m_basicConnection;
}

void
WranSubscriberStationNetDevice::SetPrimaryConnection (Ptr<WranConnection> primaryConnection)
{
  m_primaryConnection = primaryConnection;
}

Ptr<WranConnection>
WranSubscriberStationNetDevice::GetPrimaryConnection (void) const
{
  return m_primaryConnection;
}

Cid
WranSubscriberStationNetDevice::GetBasicCid (void) const
{
  return m_basicConnection->GetCid ();
}

Cid
WranSubscriberStationNetDevice::GetPrimaryCid (void) const
{
  return m_primaryConnection->GetCid ();
}

void
WranSubscriberStationNetDevice::SetModulationType (WranPhy::ModulationType modulationType)
{
  m_modulationType = modulationType;
}

WranPhy::ModulationType
WranSubscriberStationNetDevice::GetModulationType (void) const
{
  return m_modulationType;
}

void
WranSubscriberStationNetDevice::SetAreManagementConnectionsAllocated (bool areManagementConnectionsAllocated)
{
  m_areManagementConnectionsAllocated = areManagementConnectionsAllocated;
}

bool
WranSubscriberStationNetDevice::GetAreManagementConnectionsAllocated (void) const
{
  return m_areManagementConnectionsAllocated;
}

void
WranSubscriberStationNetDevice::SetAreWranServiceFlowsAllocated (bool areWranServiceFlowsAllocated)
{
  m_areWranServiceFlowsAllocated = areWranServiceFlowsAllocated;
}

bool
WranSubscriberStationNetDevice::GetAreWranServiceFlowsAllocated (void) const
{
  return m_areWranServiceFlowsAllocated;
}

Ptr<WranSSScheduler>
WranSubscriberStationNetDevice::GetScheduler (void) const
{
  return m_scheduler;
}

void
WranSubscriberStationNetDevice::SetScheduler (Ptr<WranSSScheduler> scheduler)
{
  m_scheduler = scheduler;
}

bool
WranSubscriberStationNetDevice::HasWranServiceFlows (void) const
{
  return GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_ALL).size () > 0;
}

Ptr<WranIpcsClassifier>
WranSubscriberStationNetDevice::GetWranIpcsClassifier () const
{

  return m_classifier;
}

void
WranSubscriberStationNetDevice::SetIpcsPacketClassifier (Ptr<WranIpcsClassifier> classifier)
{
  m_classifier = classifier;
}

Ptr<WranSSLinkManager>
WranSubscriberStationNetDevice::GetLinkManager (void) const
{
  return m_linkManager;
}

void
WranSubscriberStationNetDevice::SetLinkManager (Ptr<WranSSLinkManager> linkManager)
{
  m_linkManager = linkManager;
}

Ptr<WranSSServiceFlowManager>
WranSubscriberStationNetDevice::GetWranServiceFlowManager (void) const
{
  return m_serviceFlowManager;
}

void
WranSubscriberStationNetDevice::SetWranServiceFlowManager (Ptr<WranSSServiceFlowManager> sfm)
{
  m_serviceFlowManager = sfm;
}

void
WranSubscriberStationNetDevice::Start (void)
{
  SetReceiveCallback ();

  GetPhy ()->SetPhyParameters ();
  GetPhy ()->SetDataRates ();
  m_intervalT20 = Seconds (4 * GetPhy ()->GetFrameDuration ().GetSeconds ());

  SetTotalChannels(MAX_CHANNELS);
  CreateDefaultConnections ();
  GetPhy ()->SetSimplex (GetChannel(DEFAULT_CHANNEL));
//  Simulator::ScheduleNow (&WranSubscriberStationNetDevice::SendDownlinkChannelRequest, this, 0);
// Will manually control in which channel it will send and receive
//  Simulator::ScheduleNow (&WranSSLinkManager::StartScanning, m_linkManager, EVENT_NONE, false);
}

void
WranSubscriberStationNetDevice::Stop (void)
{
  SetState (SS_STATE_STOPPED);
}

void
WranSubscriberStationNetDevice::SendDownlinkChannelRequest (int nr_channel) {
	if(nr_channel >= GetTotalChannels()) {
		Time delayForSendingResult;
		delayForSendingResult = Seconds(1);
		Simulator::Schedule (delayForSendingResult ,&WranSubscriberStationNetDevice::SendSensingResult, this);
		return;
	}

	GetPhy ()->SetSimplex (GetChannel(COMMON_CONTROL_CHANNEL_NUMBER));
	NS_LOG_INFO("Initiate Downlink Request from SS");

	Ptr<PacketBurst> burst = Create<PacketBurst> ();

	std::stringstream sstream;
	std::string sentMessage;
	sstream << SS_FLAG << PACKET_SEPARATOR
			<< GetMacAddress() << PACKET_SEPARATOR
			<< MESSAGE_TYPE_SEND_PING << MESSAGE_BODY_SEPARATOR
			<< nr_channel << PACKET_SEPARATOR
			<< "Hi from ss" << PACKET_SEPARATOR ;
	sentMessage = sstream.str();
	NS_LOG_INFO("sent message from ss: " << sentMessage);
	Ptr<Packet> packet =  Create<Packet> ((uint8_t*) sentMessage.c_str(), sentMessage.length());
	burst->AddPacket (packet);

	ForwardDown(burst, WranPhy::MODULATION_TYPE_QAM16_12);

	Time delayForStartScencing;
	delayForStartScencing = Seconds(0.0001);
	bsRxList.clear();
	Simulator::Schedule (delayForStartScencing,
					   &WranSubscriberStationNetDevice::ScanningChannel,
					   this,
					   nr_channel);
}


void WranSubscriberStationNetDevice::ScanningChannel(int nr_channel){
	GetPhy ()->SetSimplex (GetChannel(nr_channel)); // lock frequency
	NS_LOG_INFO("Start Scencing in channel " << GetPhy()->GetRxFrequency());

//	Time scanningDurationTime;
//	scanningDurationTime = Seconds(MAX_SCANNING_TIME);
//
//	Simulator::Schedule (scanningDurationTime,
//	                                   &WranSubscriberStationNetDevice::EndScanningChannel,
//	                                   this,
//	                                   nr_channel);
}

void
WranSubscriberStationNetDevice::EndScanningChannel (void)
{
	NS_LOG_INFO("SS End Scening");
	GetPhy ()->SetSimplex (GetChannel(COMMON_CONTROL_CHANNEL_NUMBER));
//	Simulator::ScheduleNow (&WranSubscriberStationNetDevice::SendDownlinkChannelRequest, this, nr_channel + 1);
}

void
WranSubscriberStationNetDevice::SendSensingResult (void) {
	NS_LOG_INFO("Start Sending Sensing Result from SS");

	Ptr<PacketBurst> burst = Create<PacketBurst> ();

	std::stringstream sstream;
	std::string sentMessage;
	sstream << SS_FLAG << PACKET_SEPARATOR
			<< GetMacAddress() << PACKET_SEPARATOR
			<< MESSAGE_TYPE_SEND_SENSE_RESULT << MESSAGE_BODY_SEPARATOR;

	std::map<std::string, std::vector<double> >::iterator mit;
	std::vector<double>::iterator vit;
	for(mit = bsRxList.begin(); mit != bsRxList.end(); mit++){
		for(vit = mit->second.begin(); vit != mit->second.end(); vit++){
			sstream << mit->first << MESSAGE_BODY_SEPARATOR
					<< vit - mit->second.begin() << MESSAGE_BODY_SEPARATOR
					<< (*vit) << MESSAGE_BODY_SEPARATOR;
		}
	}

	sentMessage = sstream.str();
	NS_LOG_INFO("sent message from ss: " << sentMessage);
	Ptr<Packet> packet =  Create<Packet> ((uint8_t*) sentMessage.c_str(), sentMessage.length());
	burst->AddPacket (packet);

	ForwardDown(burst, WranPhy::MODULATION_TYPE_QAM16_12);
}

void
WranSubscriberStationNetDevice::AddWranServiceFlow (WranServiceFlow sf)
{
  GetWranServiceFlowManager ()->AddWranServiceFlow (sf);
}

void
WranSubscriberStationNetDevice::AddWranServiceFlow (WranServiceFlow * sf)
{
  GetWranServiceFlowManager ()->AddWranServiceFlow (sf);
}

bool
WranSubscriberStationNetDevice::DoSend (Ptr<Packet> packet,
                                    const Mac48Address &source,
                                    const Mac48Address &dest,
                                    uint16_t protocolNumber)
{
  NS_LOG_INFO ("SS (" << source << "):" );
  NS_LOG_INFO ("\tSending packet..." );
  NS_LOG_INFO ("\t\tDestination: " << dest );
  NS_LOG_INFO ("\t\tPacket Size:  " << packet->GetSize () );
  NS_LOG_INFO ("\t\tProtocol:    " << protocolNumber );

  WranServiceFlow *serviceFlow = 0;

  if (IsRegistered ())
    {
      NS_LOG_DEBUG ("SS (Basic CID: " << m_basicConnection->GetCid () << ")");
    }
  else
    {
      NS_LOG_DEBUG ("SS (" << GetMacAddress () << ")");
      NS_LOG_INFO ("\tCan't send packet! (NotRegitered with the network)");
      return false;
    }

  NS_LOG_DEBUG ("packet to send, size : " << packet->GetSize () << ", destination : " << dest);

  if (GetWranServiceFlowManager ()->GetNrWranServiceFlows () == 0)
    {
      NS_LOG_INFO ("\tCan't send packet! (No service Flow)");
      return false;
    }

  if (protocolNumber == 2048)
    {
      serviceFlow = m_classifier->Classify (packet, GetWranServiceFlowManager (), WranServiceFlow::SF_DIRECTION_UP);
    }

  if ((protocolNumber != 2048) || (serviceFlow == NULL))
    {
      serviceFlow = *GetWranServiceFlowManager ()->GetWranServiceFlows (WranServiceFlow::SF_TYPE_ALL).begin ();
      NS_LOG_INFO ("\tNo service flows matches...using the default one.");
    }

  NS_LOG_INFO ("\tPacket classified in the service flow SFID =  " << serviceFlow->GetSfid () << " CID = "
                                                                  << serviceFlow->GetCid ());
  if (serviceFlow->GetIsEnabled ())
    {
      if (!Enqueue (packet, MacHeaderType (), serviceFlow->GetConnection ()))
        {
          NS_LOG_INFO ("\tEnqueue ERROR!!" );
          m_ssTxDropTrace (packet);
          return false;
        }
      else
        {
          m_ssTxTrace (packet);
        }
    }
  else
    {
      NS_LOG_INFO ("Error!! The Service Flow is not enabled" );
      m_ssTxDropTrace (packet);
      return false;
    }

  return true;
}

bool
WranSubscriberStationNetDevice::Enqueue (Ptr<Packet> packet,
                                     const MacHeaderType &hdrType,
                                     Ptr<WranConnection> connection)
{
  NS_ASSERT_MSG (connection != 0, "SS: Can not enqueue the packet: the selected connection is nor initialized");

  GenericMacHeader hdr;

  if (hdrType.GetType () == MacHeaderType::HEADER_TYPE_GENERIC)
    {
      hdr.SetLen (packet->GetSize () + hdr.GetSerializedSize ());
      hdr.SetCid (connection->GetCid ());

    }

  if (connection->GetType () == Cid::TRANSPORT)
    {

      if (connection->GetSchedulingType () == WranServiceFlow::SF_TYPE_UGS && m_scheduler->GetPollMe ())
        {
          NS_ASSERT_MSG (hdrType.GetType () != MacHeaderType::HEADER_TYPE_BANDWIDTH,
                         "Error while equeuing  packet: incorrect header type");

          GrantManagementSubheader grantMgmntSubhdr;
          grantMgmntSubhdr.SetPm (true);
          packet->AddHeader (grantMgmntSubhdr);
        }
    }
  NS_LOG_INFO ("WranServiceFlowManager: enqueuing packet" );
  return connection->Enqueue (packet, hdrType, hdr);
}

void
WranSubscriberStationNetDevice::SendBurst (uint8_t uiuc,
                                       uint16_t nrSymbols,
                                       Ptr<WranConnection> connection,
                                       MacHeaderType::HeaderType packetType)
{
  WranPhy::ModulationType modulationType;

  if (uiuc == OfdmUlBurstProfile::UIUC_INITIAL_RANGING || uiuc == OfdmUlBurstProfile::UIUC_REQ_REGION_FULL)
    {
      modulationType = WranPhy::MODULATION_TYPE_BPSK_12;
    }
  else
    {
      modulationType = GetWranBurstProfileManager ()->GetModulationType (uiuc, DIRECTION_UPLINK);
    }
  Ptr<PacketBurst> burst = m_scheduler->Schedule (nrSymbols, modulationType, packetType, connection);

  if (burst->GetNPackets () == 0)
    {
      return;
    }

  if (IsRegistered ())
    {
      NS_LOG_DEBUG ("SS (Basic CID: " << m_basicConnection->GetCid () << ")");
    }
  else
    {
      NS_LOG_DEBUG ("SS (" << GetMacAddress () << ")");
    }

  if (connection->GetType () == Cid::TRANSPORT)
    {
      WranServiceFlowRecord *record = connection->GetWranServiceFlow ()->GetRecord ();
      record->UpdatePktsSent (burst->GetNPackets ());
      record->UpdateBytesSent (burst->GetSize ());

      NS_LOG_DEBUG (" sending burst" << ", SFID: " << connection->GetWranServiceFlow ()->GetSfid () << ", pkts sent: "
                                     << record->GetPktsSent () << ", pkts rcvd: " << record->GetPktsRcvd () << ", bytes sent: "
                                     << record->GetBytesSent () << ", bytes rcvd: " << record->GetBytesRcvd () );

    }
  else
    {

    }
  ForwardDown (burst, modulationType);
}

void
WranSubscriberStationNetDevice::DoReceive (Ptr<Packet> packet)
{
	NS_LOG_INFO("~~~~~~~~~Received Packet at SS (" << GetMacAddress() << ")~~~~~~~~~~");

	uint32_t sz =  packet->GetSize();
	uint8_t bf[sz+1];
	packet->CopyData(bf,sz);
	bf[sz] = 0;

	std::string ms((char *)bf);


	std::istringstream ss(ms);
	std::string token;

	int i = 0;
	int bsOrSS = 0;
	int packetType = 0;
	std::string senderMacAddress;
	std::string messageBody;
	while(std::getline(ss, token, PACKET_SEPARATOR)) {
		switch (i) {
			case 0:
				bsOrSS = std::atoi(token.c_str());
				if(bsOrSS == SS_FLAG)return;
				break;
			case 1:
				senderMacAddress = token;
				break;
			case 2:
				packetType = std::atoi(token.c_str());
				break;
			case 3:
				messageBody = token;
				break;
			default:
				break;
		}
	    i++;
	}

	Ptr<SimpleOfdmWranPhy> wranPhy = DynamicCast<SimpleOfdmWranPhy> (GetPhy());
	InsertIntoBSRxList(senderMacAddress, wranPhy->GetRxPowerListSubChannel());
	std::stringstream printstream;
	printstream << "Updated rx list at SS: " << senderMacAddress;
	for(uint16_t i = 0; i< wranPhy->GetNumberOfSubChannel();i++){
		printstream << " " << bsRxList[senderMacAddress][i];
	}
	NS_LOG_INFO(printstream.str());


	NS_LOG_INFO("Received packet message at SS: " << ms);
	if(Mac48Address(senderMacAddress.c_str()) == GetMyBSMAC()) {
		if(packetType == PACKET_TYPE_SEND_SENSING_RESULT){
			if(Mac48Address(messageBody.c_str()) == GetMacAddress()){
				//its intended for me, send sensing result to BS
				Simulator::ScheduleNow (&WranSubscriberStationNetDevice::SendSensingResult, this);
//				NS_LOG_INFO("Need to send the sensing result");
			}
			return;
		}
	}

//	NS_LOG_INFO("Received packet message at SS: " << ms);
//	int nr_channel = (GetPhy()->GetRxFrequency() - 470) / 6;
//	if(nr_channel == COMMON_CONTROL_CHANNEL_NUMBER) {
//		if(Mac48Address(senderMacAddress.c_str()) == GetMyBSMAC()) {
//			if(packetType == PACKET_TYPE_START_SENSING){
//				// sense in nr_channel request from mybs
//				nr_channel = std::atoi(messageBody.c_str());
//				NS_LOG_INFO("Start Sensing in Channel from SS " << GetChannel(nr_channel));
//				GetPhy ()->SetSimplex (GetChannel(nr_channel));
//
//				Time freqChangeInterval;
//				freqChangeInterval = Seconds(BEACON_TO_REQUEST_INTERVAL);
//
//				Simulator::Schedule (freqChangeInterval, &WranSubscriberStationNetDevice::EndScanningChannel, this);
//			} else if(packetType == PACKET_TYPE_SEND_SENSING_RESULT){
//
//				if(Mac48Address(messageBody.c_str()) == GetMacAddress()){
//					//its intended for me, send sensing result to BS
//					Simulator::ScheduleNow (&WranSubscriberStationNetDevice::SendSensingResult, this);
//				}
//			}
//			return;
//		} else {
//			return;
//		}
//	}
// saving to bsRxList was here before.
//	GetPhy ()->SetSimplex (GetChannel(COMMON_CONTROL_CHANNEL_NUMBER));
}

void
WranSubscriberStationNetDevice::InsertIntoBSRxList(std::string macAddress, std::vector<double> v){
	if(bsRxList.count(macAddress)) {
		bsRxList[macAddress].clear();
	} else {
		std::vector<double> newVector;
		bsRxList[macAddress] = newVector;
	}
	std::copy(v.begin(), v.end(), std::back_inserter(bsRxList[macAddress]));
}
//void
//WranSubscriberStationNetDevice::DoReceive (Ptr<Packet> packet)
//{
//  GenericMacHeader gnrcMacHdr;
//  WranManagementMessageType msgType;
//  RngRsp rngrsp;
//  Cid cid;
//  uint32_t pktSize = packet->GetSize ();
//  packet->RemoveHeader (gnrcMacHdr);
//  FragmentationSubheader fragSubhdr;
//  bool fragmentation = false;  // it becames true when there is a fragmentation subheader
//
//  if (gnrcMacHdr.GetHt () == MacHeaderType::HEADER_TYPE_GENERIC)
//    {
//      if (gnrcMacHdr.check_hcs () == false)
//        {
//          // The header is noisy
//          NS_LOG_INFO ("Header HCS ERROR");
//          m_ssRxDropTrace (packet);
//          return;
//        }
//
//      cid = gnrcMacHdr.GetCid ();
//
//      // checking for subheaders
//      uint8_t type = gnrcMacHdr.GetType ();
//      if (type)
//        {
//          // Check if there is a fragmentation Subheader
//          uint8_t tmpType = type;
//          if (((tmpType >> 2) & 1) == 1)
//            {
//              // a TRANSPORT packet with fragmentation subheader has been received!
//              fragmentation = true;
//              NS_LOG_INFO ("SS DoReceive -> the packet is a fragment" <<  std::endl);
//            }
//        }
//
//      if (cid == GetBroadcastConnection ()->GetCid () && !fragmentation)
//        {
//          packet->RemoveHeader (msgType);
//          switch (msgType.GetType ())
//            {
//            case WranManagementMessageType::MESSAGE_TYPE_DL_MAP:
//              {
//                if (GetState () == SS_STATE_SYNCHRONIZING)
//                  {
//                    Simulator::Cancel (m_linkManager->GetDlMapSyncTimeoutEvent ());
//                  }
//
//                if (m_lostDlMapEvent.IsRunning ())
//                  {
//                    Simulator::Cancel (m_lostDlMapEvent);
//                  }
//
//                m_linkManager->ScheduleScanningRestart (m_lostDlMapInterval, EVENT_LOST_DL_MAP, false, m_lostDlMapEvent);
//
//                if (m_dcdWaitTimeoutEvent.IsRunning ())
//                  {
//                    Simulator::Cancel (m_dcdWaitTimeoutEvent);
//                  }
//
//                m_linkManager->ScheduleScanningRestart (m_intervalT1,
//                                                        EVENT_DCD_WAIT_TIMEOUT,
//                                                        false,
//                                                        m_dcdWaitTimeoutEvent);
//
//                if (m_ucdWaitTimeoutEvent.IsRunning ())
//                  {
//                    Simulator::Cancel (m_ucdWaitTimeoutEvent);
//                  }
//
//                m_linkManager->ScheduleScanningRestart (m_intervalT12,
//                                                        EVENT_UCD_WAIT_TIMEOUT,
//                                                        true,
//                                                        m_ucdWaitTimeoutEvent);
//
//                DlMap dlmap;
//                packet->RemoveHeader (dlmap);
//                ProcessDlMap (dlmap);
//                break;
//              }
//            case WranManagementMessageType::MESSAGE_TYPE_UL_MAP:
//              {
//                if (m_lostUlMapEvent.IsRunning ())
//                  {
//                    Simulator::Cancel (m_lostUlMapEvent);
//                    m_linkManager->ScheduleScanningRestart (m_lostUlMapInterval,
//                                                            EVENT_LOST_UL_MAP,
//                                                            true,
//                                                            m_lostUlMapEvent);
//                  }
//
//                UlMap ulmap;
//                packet->RemoveHeader (ulmap);
//
//                ProcessUlMap (ulmap);
//
//                if (GetState () == SS_STATE_WAITING_REG_RANG_INTRVL)
//                  {
//                    if (m_linkManager->GetRangingIntervalFound ())
//                      {
//                        if (m_rangOppWaitTimeoutEvent.IsRunning ())
//                          {
//                            Simulator::Cancel (m_rangOppWaitTimeoutEvent);
//                          }
//                        m_linkManager->PerformBackoff ();
//                      }
//                  }
//                break;
//              }
//            case WranManagementMessageType::MESSAGE_TYPE_DCD:
//              {
//                if (GetState () == SS_STATE_SYNCHRONIZING)
//                  {
//                    SetState (SS_STATE_ACQUIRING_PARAMETERS);
//                  }
//
//                if (m_dcdWaitTimeoutEvent.IsRunning ())
//                  {
//                    Simulator::Cancel (m_dcdWaitTimeoutEvent);
//                    m_linkManager->ScheduleScanningRestart (m_intervalT1,
//                                                            EVENT_DCD_WAIT_TIMEOUT,
//                                                            false,
//                                                            m_dcdWaitTimeoutEvent);
//                  }
//
//                Dcd dcd;
//                // number of burst profiles is set to number of DL-MAP IEs after processing DL-MAP, not a very good solution
//                // dcd.SetNrDlBurstProfiles (m_nrDlMapElements);
//                dcd.SetNrDlBurstProfiles (7);
//                packet->RemoveHeader (dcd);
//
//                ProcessDcd (dcd);
//                break;
//              }
//            case WranManagementMessageType::MESSAGE_TYPE_UCD:
//              {
//                Ucd ucd;
//                // number of burst profiles is set to number of UL-MAP IEs after processing UL-MAP, not a very good solution
//                // ucd.SetNrUlBurstProfiles (m_nrUlMapElements);
//                ucd.SetNrUlBurstProfiles (7);
//                packet->RemoveHeader (ucd);
//
//                ProcessUcd (ucd);
//
//                if (m_ucdWaitTimeoutEvent.IsRunning ())
//                  {
//                    Simulator::Cancel (m_ucdWaitTimeoutEvent);
//                    m_linkManager->ScheduleScanningRestart (m_intervalT12,
//                                                            EVENT_UCD_WAIT_TIMEOUT,
//                                                            true,
//                                                            m_ucdWaitTimeoutEvent);
//                  }
//
//                if (GetState () == SS_STATE_ACQUIRING_PARAMETERS)
//                  {
//                    /*state indicating that SS has completed scanning, synchronization and parameter acquisition
//                     successfully and now waiting for UL-MAP to start initial ranging.*/
//                    SetState (SS_STATE_WAITING_REG_RANG_INTRVL);
//
//                    m_linkManager->ScheduleScanningRestart (m_intervalT2,
//                                                            EVENT_RANG_OPP_WAIT_TIMEOUT,
//                                                            false,
//                                                            m_rangOppWaitTimeoutEvent);
//                    m_linkManager->ScheduleScanningRestart (m_lostUlMapInterval,
//                                                            EVENT_LOST_UL_MAP,
//                                                            true,
//                                                            m_lostUlMapEvent);
//                  }
//                break;
//              }
//            default:
//              NS_FATAL_ERROR ("Invalid management message type");
//            }
//        }
//      else if (GetInitialRangingConnection () != 0 && cid == GetInitialRangingConnection ()->GetCid () && !fragmentation)
//        {
//          m_traceSSRx (packet, GetMacAddress (), &cid);
//          packet->RemoveHeader (msgType);
//          switch (msgType.GetType ())
//            {
//            case WranManagementMessageType::MESSAGE_TYPE_RNG_REQ:
//              // intended for base station, ignore
//              break;
//            case WranManagementMessageType::MESSAGE_TYPE_RNG_RSP:
//              NS_ASSERT_MSG (SS_STATE_WAITING_RNG_RSP,
//                             "SS: Error while receiving a ranging response message: SS state should be SS_STATE_WAITING_RNG_RSP");
//              packet->RemoveHeader (rngrsp);
//              m_linkManager->PerformRanging (cid, rngrsp);
//              break;
//            default:
//              NS_LOG_ERROR ("Invalid management message type");
//            }
//        }
//      else if (m_basicConnection != 0 && cid == m_basicConnection->GetCid () && !fragmentation)
//        {
//          m_traceSSRx (packet, GetMacAddress (), &cid);
//          packet->RemoveHeader (msgType);
//          switch (msgType.GetType ())
//            {
//            case WranManagementMessageType::MESSAGE_TYPE_RNG_REQ:
//              // intended for base station, ignore
//              break;
//            case WranManagementMessageType::MESSAGE_TYPE_RNG_RSP:
//              NS_ASSERT_MSG (SS_STATE_WAITING_RNG_RSP,
//                             "SS: Error while receiving a ranging response message: SS state should be SS_STATE_WAITING_RNG_RSP");
//              packet->RemoveHeader (rngrsp);
//              m_linkManager->PerformRanging (cid, rngrsp);
//              break;
//            default:
//              NS_LOG_ERROR ("Invalid management message type");
//            }
//        }
//      else if (m_primaryConnection != 0 && cid == m_primaryConnection->GetCid () && !fragmentation)
//        {
//          m_traceSSRx (packet, GetMacAddress (), &cid);
//          packet->RemoveHeader (msgType);
//          switch (msgType.GetType ())
//            {
//            case WranManagementMessageType::MESSAGE_TYPE_REG_REQ:
//              // not yet implemented
//              break;
//            case WranManagementMessageType::MESSAGE_TYPE_REG_RSP:
//              // intended for base station, ignore
//              break;
//            case WranManagementMessageType::MESSAGE_TYPE_DSA_REQ:
//              /*from other station as DSA initiation
//               by BS is not supported, ignore*/
//              break;
//            case WranManagementMessageType::MESSAGE_TYPE_DSA_RSP:
//              {
//                Simulator::Cancel (GetWranServiceFlowManager ()->GetDsaRspTimeoutEvent ());
//                DsaRsp dsaRsp;
//                packet->RemoveHeader (dsaRsp);
//                GetWranServiceFlowManager ()->ProcessDsaRsp (dsaRsp);
//                break;
//              }
//            case WranManagementMessageType::MESSAGE_TYPE_DSA_ACK:
//              /*from other station as DSA initiation
//               by BS is not supported, ignore*/
//              break;
//            default:
//              NS_LOG_ERROR ("Invalid management message type");
//            }
//        }
//      else if (GetWranConnectionManager ()->GetConnection (cid)) // transport connection
//        {
//          WranServiceFlow *serviceFlow = GetWranConnectionManager ()->GetConnection (cid)->GetWranServiceFlow ();
//          WranServiceFlowRecord *record = serviceFlow->GetRecord ();
//
//          record->UpdatePktsRcvd (1);
//          record->UpdateBytesRcvd (pktSize);
//
//          // If fragmentation is true, the packet is a fragment.
//          if (!fragmentation)
//            {
//              m_ssRxTrace (packet);
//              ForwardUp (packet, m_baseStationId, GetMacAddress ()); // source shall be BS's address or sender SS's?
//            }
//          else
//            {
//              NS_LOG_INFO ( "FRAG_DEBUG: SS DoReceive, the Packet is a fragment" << std::endl);
//              packet->RemoveHeader (fragSubhdr);
//              uint32_t fc = fragSubhdr.GetFc ();
//              NS_LOG_INFO ( "\t fragment size = " << packet->GetSize () << std::endl);
//
//              if (fc == 2)
//                {
//                  // This is the latest fragment.
//                  // Take the fragment queue, defragment a packet and send it to the upper layer
//                  NS_LOG_INFO ( "\t Received the latest fragment" << std::endl);
//                  GetWranConnectionManager ()->GetConnection (cid)
//                  ->FragmentEnqueue (packet);
//
//                  WranConnection::FragmentsQueue fragmentsQueue = GetWranConnectionManager ()->
//                    GetConnection (cid)->GetFragmentsQueue ();
//
//                  Ptr<Packet> fullPacket = Create<Packet> ();
//
//                  // DEFRAGMENTATION
//                  NS_LOG_INFO ( "\t SS PACKET DEFRAGMENTATION" << std::endl);
//                  for (std::list<Ptr<const Packet> >::const_iterator iter = fragmentsQueue.begin ();
//                       iter != fragmentsQueue.end (); ++iter)
//                    {
//                      // Create the whole Packet
//                      fullPacket->AddAtEnd (*iter);
//                    }
//                  GetWranConnectionManager ()->GetConnection (cid)
//                  ->ClearFragmentsQueue ();
//                  NS_LOG_INFO ( "\t fullPacket size = " << fullPacket->GetSize () << std::endl);
//
//                  m_ssRxTrace (fullPacket);
//                  ForwardUp (fullPacket, m_baseStationId, GetMacAddress ()); // source shall be BS's address or sender SS's?
//                }
//              else
//                {
//                  // This is the first or middle fragment.
//                  // Take the fragment queue, store the fragment into the queue
//                  NS_LOG_INFO ( "\t Received the first or the middle fragment" << std::endl);
//                  GetWranConnectionManager ()->GetConnection (cid)->FragmentEnqueue (packet);
//                }
//            }
//        }
//      else if (cid.IsMulticast ())
//        {
//          m_traceSSRx (packet, GetMacAddress (), &cid);
//          ForwardUp (packet, m_baseStationId, GetMacAddress ()); // source shall be BS's address or sender SS's?
//        }
//      else if (IsPromisc ())
//        {
//          NotifyPromiscTrace (packet);
//          m_ssPromiscRxTrace (packet);
//
//          // not for me, ignore
//        }
//      else
//        {
//          // not for me drop
//        }
//    }
//  else
//    {
//      // from other SS, ignore
//    }
//}

void
WranSubscriberStationNetDevice::ProcessDlMap (const DlMap &dlmap)
{
  m_nrDlMapRecvd++;
  m_dcdCount = dlmap.GetDcdCount ();
  m_baseStationId = dlmap.GetBaseStationId ();
  std::list<OfdmDlMapIe> dlMapElements = dlmap.GetDlMapElements ();

  for (std::list<OfdmDlMapIe>::iterator iter = dlMapElements.begin (); iter != dlMapElements.end (); ++iter)
    {
      if (iter->GetDiuc () == OfdmDlBurstProfile::DIUC_END_OF_MAP)
        {
          break;
        }

      if (iter->GetCid () == m_basicConnection->GetCid ())
        {
          /*here the SS shall actually acquire the start time it shall start receiving the burst at. start time is used for power saving
           which is not implemented here, furthermore there is no need since the simulator architecture automatically callbacks the receive
           function. shall acquire the DIUC (burst profile) as well to decode the burst, again not required again because the callback
           mechanism automatically passes it as parameter.*/
        }

#if 0 /* a template for future implementation following */
      uint8_t temp = iter->GetDiuc ();
      temp = iter->GetPreamblePresent ();
      temp = iter->GetStartTime ();
#endif
    }
}

void
WranSubscriberStationNetDevice::ProcessUlMap (const UlMap &ulmap)
{
  m_nrUlMapRecvd++;
  m_ucdCount = ulmap.GetUcdCount ();
  m_allocationStartTime = ulmap.GetAllocationStartTime ();
  std::list<OfdmUlMapIe> ulMapElements = ulmap.GetUlMapElements ();
  m_linkManager->SetRangingIntervalFound (false);

  for (std::list<OfdmUlMapIe>::iterator iter = ulMapElements.begin (); iter != ulMapElements.end (); ++iter)
    {
      OfdmUlMapIe ulMapIe = *iter;

      if (ulMapIe.GetUiuc () == OfdmUlBurstProfile::UIUC_END_OF_MAP)
        {
          break;
        }

      Cid cid = ulMapIe.GetCid ();

      if (ulMapIe.GetUiuc () == OfdmUlBurstProfile::UIUC_INITIAL_RANGING && cid == GetBroadcastConnection ()->GetCid ())
        {
          m_linkManager->SetRangingIntervalFound (true);
        }

      if (m_areManagementConnectionsAllocated && cid == m_basicConnection->GetCid ())
        {

          Time timeToAllocation = GetTimeToAllocation (Seconds (ulMapIe.GetStartTime ()
                                                                * GetPhy ()->GetSymbolDuration ().GetSeconds ()));

          if (ulMapIe.GetUiuc () == OfdmUlBurstProfile::UIUC_INITIAL_RANGING) // invited ranging interval

            {

              m_linkManager->IncrementNrInvitedPollsRecvd ();
              NS_ASSERT_MSG (GetState () == SS_STATE_WAITING_INV_RANG_INTRVL,
                             "SS: Error while processing UL MAP: SS state should be SS_STATE_WAITING_INV_RANG_INTRVL");
              Simulator::Schedule (timeToAllocation,
                                   &WranSSLinkManager::SendRangingRequest,
                                   m_linkManager,
                                   ulMapIe.GetUiuc (),
                                   ulMapIe.GetDuration ());
            }
          else if (ulMapIe.GetUiuc () == OfdmUlBurstProfile::UIUC_REQ_REGION_FULL) // unicast poll

            {

              Simulator::Schedule (timeToAllocation,
                                   &WranBandwidthManager::SendBandwidthRequest,
                                   GetWranBandwidthManager (),
                                   ulMapIe.GetUiuc (),
                                   ulMapIe.GetDuration ());
            }
          else // regular allocation/grant for data, for UGS flows or in response of requests for non-UGS flows

            {

              Ptr<WranConnection> connection = NULL;
              Simulator::Schedule (timeToAllocation,
                                   &WranSubscriberStationNetDevice::SendBurst,
                                   this,
                                   ulMapIe.GetUiuc (),
                                   ulMapIe.GetDuration (),
                                   connection,
                                   MacHeaderType::HEADER_TYPE_GENERIC);
            }
        }
      else
        {
          if (ulMapIe.GetUiuc () == OfdmUlBurstProfile::UIUC_INITIAL_RANGING && cid
              == GetBroadcastConnection ()->GetCid ()) // regular ranging interval

            {
              if (GetCurrentUcd ().GetChannelEncodings ().GetRangReqOppSize () != 0)
                {
                  m_linkManager->SetNrRangingTransOpps ((ulMapIe.GetDuration () * GetPhy ()->GetPsPerSymbol ())
                                                        / GetCurrentUcd ().GetChannelEncodings ().GetRangReqOppSize ());

                }

              if (GetState () == SS_STATE_WAITING_REG_RANG_INTRVL || GetState () == SS_STATE_ADJUSTING_PARAMETERS)
                {
                  m_linkManager->StartContentionResolution ();
                }

            }
        }
    }
}

void
WranSubscriberStationNetDevice::ProcessDcd (const Dcd &dcd)
{
  m_nrDcdRecvd++;
  if (dcd.GetConfigurationChangeCount () == GetCurrentDcd ().GetConfigurationChangeCount ())
    {
      return; // nothing new in DCD so dont read

    }
  SetCurrentDcd (dcd);
  OfdmDcdChannelEncodings dcdChnlEncodings = dcd.GetChannelEncodings ();

  // parameters for initial ranging
  m_linkManager->SetBsEirp (dcdChnlEncodings.GetBsEirp ());
  m_linkManager->SetEirXPIrMax (dcdChnlEncodings.GetEirxPIrMax ());

  GetPhy ()->GetFrameDuration (dcdChnlEncodings.GetFrameDurationCode ());

  std::vector<OfdmDlBurstProfile> dlBurstProfiles = dcd.GetDlBurstProfiles ();

  for (std::vector<OfdmDlBurstProfile>::iterator iter = dlBurstProfiles.begin (); iter != dlBurstProfiles.end (); ++iter)
    {
      OfdmDlBurstProfile brstProfile = *iter;

      /*NS-2 does this, may be not correct, assumes DIUC/UIUC to
       modulation type mapping in DCD/UCD may change over time*/
      if (brstProfile.GetFecCodeType () == m_modulationType)
        {
          m_dlBurstProfile->SetFecCodeType (brstProfile.GetFecCodeType ());
          m_dlBurstProfile->SetDiuc (brstProfile.GetDiuc ());
        }
    }
}

void
WranSubscriberStationNetDevice::ProcessUcd (const Ucd &ucd)
{
  m_nrUcdRecvd++;
  if (!m_linkManager->IsUlChannelUsable ())
    {
      m_linkManager->StartScanning (EVENT_NONE, false);
      return;
    }

  if (ucd.GetConfigurationChangeCount () == GetCurrentUcd ().GetConfigurationChangeCount ())
    {
      return; // nothing new in UCD so don't read

    }
  SetCurrentUcd (ucd);
  m_linkManager->SetRangingCW ((uint8_t) std::pow ((double) 2, (double) ucd.GetRangingBackoffStart ()) - 1); // initializing ranging CW
  OfdmUcdChannelEncodings ucdChnlEncodings = ucd.GetChannelEncodings ();

  std::vector<OfdmUlBurstProfile> ulBurstProfiles = ucd.GetUlBurstProfiles ();

  for (std::vector<OfdmUlBurstProfile>::iterator iter = ulBurstProfiles.begin (); iter != ulBurstProfiles.end (); ++iter)
    {
      OfdmUlBurstProfile brstProfile = *iter;

      /*NS-2 does this, may be not correct, assumes DIUC/UIUC to
       modulation type mapping in DCD/UCD may change over time*/
      if (brstProfile.GetFecCodeType () == m_modulationType)
        {
          m_ulBurstProfile->SetFecCodeType (brstProfile.GetFecCodeType ());
          m_ulBurstProfile->SetUiuc (brstProfile.GetUiuc ());
        }
    }
}

/*temporarily assuming registered if ranging is complete,
 shall actually consider the registration step also */
bool
WranSubscriberStationNetDevice::IsRegistered (void) const
{
  return GetState () >= SS_STATE_REGISTERED;
}

Time
WranSubscriberStationNetDevice::GetTimeToAllocation (Time defferTime)
{
  Time timeAlreadyElapsed = Simulator::Now () - m_frameStartTime;
  Time timeToUlSubframe = Seconds (m_allocationStartTime * GetPhy ()->GetPsDuration ().GetSeconds ())
    - timeAlreadyElapsed;
  return timeToUlSubframe + defferTime;
}

void
WranSubscriberStationNetDevice::SetTimer (EventId eventId, EventId &event)
{
  if (GetState () == SS_STATE_STOPPED)
    {
      Simulator::Cancel (eventId); // cancelling this event (already scheduled in function call)
      return;
    }

  event = eventId;
}

void WranSubscriberStationNetDevice::SetMyBSMAC(Mac48Address BSMAC){
	myBSMAC = BSMAC;
}

Mac48Address WranSubscriberStationNetDevice::GetMyBSMAC(){
	return myBSMAC;
}

} // namespace ns`
