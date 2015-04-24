/*
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
 * Author: Abdulla K. Al-Ali <abdulla.alali@qu.edu.qa>
 */

#include "spectrum-manager.h"
#include "simple-ofdm-wran-phy.h"
#include "ns3/aodv-routing-protocol.h"

NS_LOG_COMPONENT_DEFINE ("CogSpectrumManager");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SpectrumManager);

TypeId SpectrumManager::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SpectrumManager")
    .SetParent<Object> ();
  return tid;
}

//SpectrumManager Initializer
SpectrumManager::SpectrumManager(Ptr<WranNetDevice> wranNetDevice, int id) {

//	m_wifiMac=mac;
	m_wranNetDevice = wranNetDevice;
	m_nodeId = id;

	// State Initialization
	m_isPuOn = false;
	m_isSensing = false;
	m_isSwitching = false;
	// Spectrum Module Definition
	m_sensingMod = new SpectrumSensing(this);
	m_decisionMod = new SpectrumDecision(this);
	m_currentChannelId = 0;
}



//SpectrumManager Initializer
SpectrumManager::SpectrumManager(Ptr<WranNetDevice> wranNetDevice,
    int id, Time sense_time, Time transmit_time)  {

	m_wranNetDevice = wranNetDevice;
//	m_wifiMac=mac;
	m_nodeId=id;
	m_wranPhy = wranNetDevice->GetPhy();
	m_isPuOn=false;
	m_isSensing=false;	
	m_isSwitching = false;

	// State Initialization
	m_senseTime=sense_time;
	m_transmitTime=transmit_time;

	// Spectrum Module Definition
	m_sensingMod=new SpectrumSensing(this);
	m_decisionMod=new SpectrumDecision(this);
	m_currentChannelId = 0;
	// Setup sense and handoff ended callback at PHY
//	m_wranPhy->SetSenseEndedCallback(MakeCallback (&SpectrumManager::SenseEnded, this));
//	m_wranPhy->SetHandoffEndedCallback(MakeCallback(&SpectrumManager::HandoffEnded, this));

}

SpectrumManager::~SpectrumManager() {

}

//start: CR starts the sensing/handoff/transmission cycle on the RX iface.
void
SpectrumManager::Start() {
  // Start sensing on the current channel for a sense_time_ interval
  m_isPuOn = m_sensingMod->GetSenseResultsFuture(m_nodeId,m_senseTime,m_transmitTime, m_repository->GetRxChannel(m_nodeId));
//  m_wranPhy->StartScanning(m_senseTime);
  NS_LOG_DEBUG("Starting CR Plane " << m_isPuOn << " sense time " << m_senseTime);
  m_wranPhy->StartScanning(m_wranNetDevice->GetChannel(m_currentChannelId),
		  m_senseTime,
		  MakeCallback (&SpectrumManager::SenseEnded, this));
}


//is_channel_available: return true if CR is NOT doing sensing and is NOT doing spectrum handoff
bool 
SpectrumManager::IsChannelAvailable() {

	bool available= !(m_isSensing || m_isSwitching);

	return available;

}



// is_PU_interfering: return true if there is a PU which is transmitting on the same channel and within the tx range of the CR receiving a packet
bool 
SpectrumManager::IsPuInterfering(Time txDuration) {

	// Get the tx time of a packet
	Time time_tx=txDuration;
	// Check if a PU is active in the interval [now: now+time_tx]
	int  current_channel=m_repository->GetRxChannel(m_nodeId);
	bool interference=m_sensingMod->GetSenseResultsFuture(m_nodeId,time_tx,m_transmitTime, current_channel);

#ifdef SENSING_VERBOSE_MODE
	if (interference)
	  {
	    char buffer [50];
	    std::sprintf(buffer, "[SENSING-DBG] Node %d sensed some PU activity on channel %d while receiving data\n", m_nodeId,current_channel);
	    NS_LOG_DEBUG (buffer);
	  }
#endif
	return interference;
}



/*********************************************************
 * SETUP METHODS
 * *******************************************************/

//setPUmodel: set the current PU model
void
SpectrumManager::SetPuModel(double prob, Ptr<PUModel> p) {

	m_sensingMod=new SpectrumSensing(this,prob,p);

}



//setRepository: set the current cross-layer repository
void
SpectrumManager::SetRepository(Ptr<Repository> rep) {

  m_repository=rep;
  m_currentChannelId = m_repository->GetRxChannel(m_nodeId);
  NS_LOG_DEBUG("Got current channel ID");
  m_wranPhy->CallBackChecking(MakeCallback (&SpectrumManager::SenseEnded, this));
//  HandoffEnded(); //enable this later
//  m_wranPhy->SetChannelNumber(m_repository->GetRxChannel(m_nodeId));
}

void
SpectrumManager::CallsComing(bool f){
	NS_LOG_DEBUG("Calls coming here " << f);
}
/*********************************************************
 * TIMER METHODS
 * *******************************************************/

//senseHandler: handler for sensing timer. 
//Check if PU was detected during the last sensing interval, in case ask the spectrumDecision to switch to a new channel.
//In case of channel switching, use Spectrum Mobility to perform handoff, and notify the event to the upper layers.
void 
SpectrumManager::SenseEnded(bool status, uint64_t frequency) {

	NS_LOG_DEBUG("Sense Ended on freq: " << frequency);
//  bool need_to_switch=false;
//
//	int  current_channel=m_repository->GetRxChannel(m_nodeId);
//
////#ifdef SENSING_VERBOSE_MODE //abdulla
//	char buffer [100];
//	std::sprintf(buffer, "[SENSING-DBG] Node %d is on channel %d and PU activity is %s", m_nodeId, current_channel, (m_isPuOn)?"true":"false");
//	NS_LOG_DEBUG(buffer);
////#endif
//
//	// Check if PU was detected
//	if (m_isPuOn) {
//
//		// Ask the Spectrum Decision if channel switching is needed
//		need_to_switch=m_decisionMod->DecideSwitch();
//
//		// CR needs to vacate the channel
//		if (need_to_switch) {
//
//			// Channel allocation is decided at MAC Layer
//#ifdef CHANNEL_DECISION_MAC_LAYER
//
//			// Choose next channel and store the information in the shared repository
//			int next_channel=m_decisionMod->DecideSpectrum(current_channel);
//
//			m_currentChannelId = next_channel;
//			HandoffEnded(); //enable this later
////			m_wranPhy->SetChannelNumber(next_channel);
//			m_repository->SetRxChannel(m_nodeId,next_channel);
//			m_isSwitching = true;
//
//#endif
//
////#ifdef SENSING_VERBOSE_MODE
//			char buffer [100];
//			std::sprintf(buffer, "[SENSING-DBG] Node %d starts handoff on channel %d to channel %d",m_nodeId,current_channel,next_channel);
//			NS_LOG_DEBUG (buffer);
////#endif
//
//			// Sensing Time is off, since the node is performing a spectrum handoff
//			m_isSensing=false;
//
//		} else  { //no need to switch
//
//			// CR does not vacate the spectrum, but it must not interfere with PU activity
//			// In this case, CR keeps sensing and waits for the channel to be free
//			m_isPuOn = m_sensingMod->GetSenseResultsFuture(m_nodeId,m_senseTime,m_transmitTime, current_channel);
//
//			NS_LOG_DEBUG ("restarting sensor");
////			m_wranPhy->StartScanning(m_senseTime);
//			m_wranPhy->StartScanning(m_wranNetDevice->GetChannel(m_currentChannelId),
//					  m_senseTime,
//					  MakeCallback (&SpectrumManager::SenseEnded, this));
//
//		  m_isSensing=true;
//
//		}
//
//	} else { //if pu is not on. without this else, sensetimer gets scheduled twice
//
//
//	  // The CR can transmit if PU is not detected
//		if ( !m_isPuOn )  {
//
//			// Sensing Time is on
//			m_isSensing=false;
//
//			if  (m_isSwitching)
//			{
//#ifdef ENABLE_SPECTRUM_HANDOFF_NOTIFICATION
//			  // Notify the spectrum handoff to the upper layers
////			  Ptr<SimpleOfdmWranPhy> phy = DynamicCast<SimpleOfdmWranPhy>(m_wranPhy);
////			  Ptr<Ipv4RoutingProtocol> route = phy->GetMobility()->GetObject<Ipv4>()->GetRoutingProtocol();
////			  Ptr<aodv::RoutingProtocol> aodv = DynamicCast<aodv::RoutingProtocol>(route);
////			  aodv->SendHello();
//			  m_isSwitching = false;
//#endif
//			}
//
//			// No channel switching, the CR can start transmitting on the current channel
//			Simulator::Schedule (m_transmitTime, &SpectrumManager::TransmitEnded, this);
////			m_wifiMac->RestartAccess(); //need to think about this later
//
//#ifdef SENSING_VERBOSE_MODE
//			char buffer [50];
//			std::sprintf(buffer, "[SENSING-DBG] Node %d starts transmitting on channel %d",m_nodeId,current_channel);
//			NS_LOG_DEBUG (buffer);
//#endif
//		}
//
//	}
}





//TransmitEnded: the CR stops transmitting, and starts sensing for PU detection
void 
SpectrumManager::TransmitEnded() {

	int current_channel=m_repository->GetRxChannel(m_nodeId);

	// Perform sensing on the current channel
	m_isPuOn= m_sensingMod->GetSenseResultsFuture(m_nodeId,m_senseTime,m_transmitTime, current_channel);

	// Set the sensing ON
	m_isSensing=true;

#ifdef SENSING_VERBOSE_MODE
	char buffer [50];
	std::sprintf(buffer, "[SENSING-DBG] Node %d starts sensing on channel %d",m_nodeId,current_channel);
	NS_LOG_DEBUG (buffer);
#endif

//	m_wranPhy->StartScanning(m_senseTime);
	m_wranPhy->StartScanning(m_wranNetDevice->GetChannel(m_currentChannelId),
						  m_senseTime,
						  MakeCallback (&SpectrumManager::SenseEnded, this));

}




//HandoffEnded: the CR has performed spectrum handoff to a new channel. Then, it starts sensing on it to detect PU activity.
void 
SpectrumManager::HandoffEnded() {

  int current_channel=m_repository->GetRxChannel(m_nodeId);
  NS_LOG_DEBUG("Going for sense result future");
	// Perform sensing on the new channel
	m_isPuOn = m_sensingMod->GetSenseResultsFuture(m_nodeId,m_senseTime,m_transmitTime, current_channel);
	NS_LOG_DEBUG("Come from sense result future");
	//	m_wranPhy->StartScanning(m_senseTime);
	m_wranPhy->StartScanning(m_wranNetDevice->GetChannel(m_currentChannelId),
						  m_senseTime,
						  MakeCallback (&SpectrumManager::SenseEnded, this));
	m_isSensing = true;
	//Do not disable m_isSwitching now. We do that after sensing concluded and no PU is there (for aodv to broadcast stuff)

#ifdef SENSING_VERBOSE_MODE

	char buffer [50];
	std::sprintf(buffer,"[SENSING-DBG] Node %d ends handoff on channel %d",m_nodeId,current_channel);
	NS_LOG_DEBUG (buffer);
	std::sprintf(buffer, "[SENSING-DBG] Node %d starts sensing on channel %d",m_nodeId,current_channel);
	NS_LOG_DEBUG (buffer);

#endif

}

std::vector<int> GetPUFreeChannels() {
	/**
	 * Recode this method later.
	 * This method should sense available channel list and find
	 * coginitively free channel lists.
	 * Currently free channels are 0,1,4,5,6,7,11
	 */
	std::vector<int> cList;
	cList.push_back(0);
	cList.push_back(1);
	cList.push_back(4);
	cList.push_back(5);
	cList.push_back(6);
	cList.push_back(7);
	cList.push_back(11);
	return cList;
}

}

