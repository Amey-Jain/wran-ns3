/*

    1. Setup a 5x5 wireless adhoc network with a grid. You may use
    examples/wireless/wifi-simple-adhoc-grid.cc as a base.

    2. Install the OLSR routing protocol.

    3. Setup three UDP traffic flows, one along each diagonal and one
    along the middle (at high rates of transmission).

    4. Setup the ns-3 flow monitor for each of these flows.

    5. Now schedule each of the flows at times 1s, 1.5s, and 2s.

    6. Now using the flow monitor, observe the throughput of each of the
    UDP flows. Furthermore, use the tracing mechanism to monitor the number of
    packet collisions/drops at intermediary nodes. Around which nodes are most
    of the collisions/drops happening?

    7. Now repeat the experiment with RTS/CTS enabled on the wifi devices.

    8. Show the difference in throughput and packet drops if any.


	Solution by: Konstantinos Katsaros (K.Katsaros@surrey.ac.uk)
	based on wifi-simple-adhoc-grid.cc
*/

// The default layout is like this, on a 2-D grid.
//
// n20(s)    n21          n22(s)   n23           n24(s)
// n15       n16(Gw-3)    n17      n18           n19      
// n10       n11          n12      n13(Gw-2)     n14
// n5        n6(Gw-1)     n7       n8            n9
// n0(s)     n1           n2(s)    n3            n4(s)   
//
// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..
//
// Flow 1: 0 -> 6 (Path : 0-> 1 -> 6(GW-1) )
// Flow 2: 0 -> 16 (Path : 0-> 5-> 10 -> 15 -> 16(GW-3) )
// Flow 3: 22 -> 16 (Path : 22-> 21 -> 16(GW-3) )



#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/applications-module.h"
//#include "myapp.h"
#include "ns3/netanim-module.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>

NS_LOG_COMPONENT_DEFINE ("grid_4by4");

using namespace ns3;
	

class MyApp : public Application
{
public:

  MyApp ();
  virtual ~MyApp();

  void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);

private:
  virtual void StartApplication (void);
  virtual void StopApplication (void);

  void ScheduleTx (void);
  void SendPacket (void);

  Ptr<Socket>     m_socket;
  Address         m_peer;
  uint32_t        m_packetSize;
  uint32_t        m_nPackets;
  DataRate        m_dataRate;
  EventId         m_sendEvent;
  bool            m_running;
  uint32_t        m_packetsSent;
};

MyApp::MyApp ()
  : m_socket (0),
    m_peer (),
    m_packetSize (0),
    m_nPackets (0),
    m_dataRate (0),
    m_sendEvent (),
    m_running (false),
    m_packetsSent (0)
{
}

MyApp::~MyApp()
{
  m_socket = 0;
}

void
MyApp::Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate)
{
  m_socket = socket;
  m_peer = address;
  m_packetSize = packetSize;
  m_nPackets = nPackets;
  m_dataRate = dataRate;
}

void
MyApp::StartApplication (void)
{
  m_running = true;
  m_packetsSent = 0;
  m_socket->Bind ();
  m_socket->Connect (m_peer);
  SendPacket ();
}

void
MyApp::StopApplication (void)
{
  m_running = false;

  if (m_sendEvent.IsRunning ())
    {
      Simulator::Cancel (m_sendEvent);
    }

  if (m_socket)
    {
      m_socket->Close ();
    }
}

void
MyApp::SendPacket (void)
{
  Ptr<Packet> packet = Create<Packet> (m_packetSize);
  m_socket->Send (packet);

  if (++m_packetsSent < m_nPackets)
    {
      ScheduleTx ();
    }
}

void
MyApp::ScheduleTx (void)
{
  if (m_running)
    {
      Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
      m_sendEvent = Simulator::Schedule (tNext, &MyApp::SendPacket, this);
    }
}
/*uint32_t MacTxDropCount, PhyTxDropCount, PhyRxDropCount;

void
MacTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  MacTxDropCount++;
}


void
PrintDrop()
{
  
std::cout << Simulator::Now().GetSeconds() << "\t" << MacTxDropCount << "\t"<< PhyTxDropCount << "\t" << PhyRxDropCount << "\n";
  Simulator::Schedule(Seconds(5.0), &PrintDrop);

}

void
PhyTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyTxDropCount++;
}
void
PhyRxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyRxDropCount++;
}*/
int main (int argc, char *argv[])
{
  std::string phyMode ("OfdmRate18Mbps");
  double distance = 150;  // m
  uint32_t numNodes = 25;  // by default, 5x5
  double interval = 0.001; // seconds
  uint32_t packetSize = 1000; // bytes
// uint32_t numPackets = 10000000;
 uint32_t numPackets = 10;
  std::string rtslimit = "1500";
  CommandLine cmd;
  Ptr<YansWifiChannel> chnls[10];
  
  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("distance", "distance (m)", distance);
  cmd.AddValue ("packetSize", "distance (m)", packetSize);
  cmd.AddValue ("rtslimit", "RTS/CTS Threshold (bytes)", rtslimit);
  cmd.Parse (argc, argv);
  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtslimit));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

  
  NodeContainer c1;
  c1.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi1;

  YansWifiPhyHelper wifiPhy1 =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy1.Set ("RxGain", DoubleValue (-10) ); 

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy1.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
  
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");

  wifiPhy1.SetChannel (wifiChannel.Create ());
 
  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac1 = NqosWifiMacHelper::Default ();
  wifi1.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi1.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac1.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices1 = wifi1.Install (wifiPhy1, wifiMac1, c1);


  NodeContainer c2  = NodeContainer(c1);
  //c1.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi2;

  YansWifiPhyHelper wifiPhy2 =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy2.Set ("RxGain", DoubleValue (-10) ); 

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy2.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  wifiPhy2.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac2 = NqosWifiMacHelper::Default ();
  wifi2.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi2.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac2.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices2 = wifi2.Install (wifiPhy2, wifiMac2, c2);


NodeContainer c3  = NodeContainer(c1);
  //c1.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi3;

  YansWifiPhyHelper wifiPhy3 =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy3.Set ("RxGain", DoubleValue (-10) ); 

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy3.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
  

wifiPhy3.SetChannel (wifiChannel.Create ());
  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac3 = NqosWifiMacHelper::Default ();
  wifi3.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi3.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac3.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices3 = wifi3.Install (wifiPhy3, wifiMac3, c3);

NodeContainer c4  = NodeContainer(c1);
  //c1.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi4;

  YansWifiPhyHelper wifiPhy4 =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy4.Set ("RxGain", DoubleValue (-10) ); 

  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy4.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  wifiPhy4.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac4 = NqosWifiMacHelper::Default ();
  wifi4.SetStandard (WIFI_PHY_STANDARD_80211a);
  wifi4.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac4.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices4 = wifi4.Install (wifiPhy4, wifiMac4, c4);


  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (5),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c1);

  // Enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  //list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c1);

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont1 = ipv4.Assign (devices1);
  ipv4.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont2 = ipv4.Assign (devices2);
  ipv4.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont3 = ipv4.Assign (devices3);
  ipv4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont4 = ipv4.Assign (devices4);
        
        YansWifiChannelHelper wifiChannel1;
  	wifiChannel1.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel1.AddPropagationLoss ("ns3::FriisPropagationLossModel");

        YansWifiChannelHelper wifiChannel2;
  	wifiChannel2.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel2.AddPropagationLoss ("ns3::FriisPropagationLossModel");

	YansWifiChannelHelper wifiChannel3;
  	wifiChannel3.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel3.AddPropagationLoss ("ns3::FriisPropagationLossModel");

	YansWifiChannelHelper wifiChannel4;
  	wifiChannel4.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel4.AddPropagationLoss ("ns3::FriisPropagationLossModel");

	YansWifiChannelHelper wifiChannel5;
  	wifiChannel5.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel5.AddPropagationLoss ("ns3::FriisPropagationLossModel");

	YansWifiChannelHelper wifiChannel6;
  	wifiChannel6.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel6.AddPropagationLoss ("ns3::FriisPropagationLossModel");

	YansWifiChannelHelper wifiChannel7;
  	wifiChannel7.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel7.AddPropagationLoss ("ns3::FriisPropagationLossModel");

	YansWifiChannelHelper wifiChannel8;
  	wifiChannel8.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  	wifiChannel8.AddPropagationLoss ("ns3::FriisPropagationLossModel");


//Ptr<YansWifiChannel> chnls[8];      
chnls[1] = wifiChannel1.Create();
chnls[2] = wifiChannel2.Create();
chnls[3] = wifiChannel3.Create();
chnls[4] = wifiChannel4.Create();
chnls[5] = wifiChannel5.Create();
chnls[6] = wifiChannel6.Create();
chnls[7] = wifiChannel7.Create();
chnls[8] = wifiChannel8.Create();




  Ptr<Ipv4> ipv4Node0_1 = c1.Get(0)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node0_2 = c2.Get(0)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node0_3 = c3.Get(0)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node0_4 = c4.Get(0)->GetObject<Ipv4> ();
   
  Ptr<Ipv4> ipv4Node1_1 = c1.Get(1)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node1_2 = c2.Get(1)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node1_3 = c3.Get(1)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node1_4 = c4.Get(1)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node2_1 = c1.Get(2)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node2_2 = c2.Get(2)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node2_3 = c3.Get(2)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node2_4 = c4.Get(2)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node3_1 = c1.Get(3)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node3_2 = c2.Get(3)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node3_3 = c3.Get(3)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node3_4 = c4.Get(3)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node4_1 = c1.Get(4)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node4_2 = c2.Get(4)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node4_3 = c3.Get(4)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node4_4 = c4.Get(4)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node5_1 = c1.Get(5)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node5_2 = c2.Get(5)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node5_3 = c3.Get(5)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node5_4 = c4.Get(5)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node6_1 = c1.Get(6)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node6_2 = c2.Get(6)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node6_3 = c3.Get(6)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node6_4 = c4.Get(6)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node7_1 = c1.Get(7)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node7_2 = c2.Get(7)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node7_3 = c3.Get(7)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node7_4 = c4.Get(7)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node8_1 = c1.Get(8)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node8_2 = c2.Get(8)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node8_3 = c3.Get(8)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node8_4 = c4.Get(8)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node9_1 = c1.Get(9)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node9_2 = c2.Get(9)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node9_3 = c3.Get(9)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node9_4 = c4.Get(9)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node10_1 = c1.Get(10)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node10_2 = c2.Get(10)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node10_3 = c3.Get(10)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node10_4 = c4.Get(10)->GetObject<Ipv4> ();

Ptr<Ipv4> ipv4Node11_1 = c1.Get(11)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node11_2 = c2.Get(11)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node11_3 = c3.Get(11)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node11_4 = c4.Get(11)->GetObject<Ipv4> ();

Ptr<Ipv4> ipv4Node12_1 = c1.Get(12)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node12_2 = c2.Get(12)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node12_3 = c3.Get(12)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node12_4 = c4.Get(12)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node13_1 = c1.Get(13)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node13_2 = c2.Get(13)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node13_3 = c3.Get(13)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node13_4 = c4.Get(13)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node14_1 = c1.Get(14)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node14_2 = c2.Get(14)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node14_3 = c3.Get(14)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node14_4 = c4.Get(14)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node15_1 = c1.Get(15)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node15_2 = c2.Get(15)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node15_3 = c3.Get(15)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node15_4 = c4.Get(15)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node16_1 = c1.Get(16)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node16_2 = c2.Get(16)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node16_3 = c3.Get(16)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node16_4 = c4.Get(16)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node17_1 = c1.Get(17)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node17_2 = c2.Get(17)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node17_3 = c3.Get(17)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node17_4 = c4.Get(17)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node18_1 = c1.Get(18)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node18_2 = c2.Get(18)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node18_3 = c3.Get(18)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node18_4 = c4.Get(18)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node19_1 = c1.Get(19)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node19_2 = c2.Get(19)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node19_3 = c3.Get(19)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node19_4 = c4.Get(19)->GetObject<Ipv4> ();
 
  Ptr<Ipv4> ipv4Node20_1 = c1.Get(20)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node20_2 = c2.Get(20)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node20_3 = c3.Get(20)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node20_4 = c4.Get(20)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node21_1 = c1.Get(21)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node21_2 = c2.Get(21)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node21_3 = c3.Get(21)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node21_4 = c4.Get(21)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node22_1 = c1.Get(22)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node22_2 = c2.Get(22)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node22_3 = c3.Get(22)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node22_4 = c4.Get(22)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node23_1 = c1.Get(23)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node23_2 = c2.Get(23)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node23_3 = c3.Get(23)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node23_4 = c4.Get(23)->GetObject<Ipv4> ();

  Ptr<Ipv4> ipv4Node24_1 = c1.Get(24)->GetObject<Ipv4> ();  
  Ptr<Ipv4> ipv4Node24_2 = c2.Get(24)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node24_3 = c3.Get(24)->GetObject<Ipv4> ();
  Ptr<Ipv4> ipv4Node24_4 = c4.Get(24)->GetObject<Ipv4> ();


  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> staticRoutingNode0_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node0_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode0_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node0_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode0_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node0_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode0_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node0_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode1_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node1_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode1_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node1_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode1_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node1_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode1_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node1_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode2_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node2_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode2_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node2_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode2_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node2_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode2_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node2_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode3_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node3_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode3_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node3_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode3_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node3_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode3_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node3_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode4_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node4_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode4_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node4_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode4_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node4_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode4_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node4_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode5_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node5_1);  
  Ptr<Ipv4StaticRouting> staticRoutingNode5_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node5_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode5_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node5_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode5_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node5_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode6_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node6_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode6_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node6_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode6_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node6_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode6_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node6_4);

 Ptr<Ipv4StaticRouting> staticRoutingNode7_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node7_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode7_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node7_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode7_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node7_3);
   Ptr<Ipv4StaticRouting> staticRoutingNode7_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node7_4);

 Ptr<Ipv4StaticRouting> staticRoutingNode8_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node8_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode8_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node8_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode8_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node8_3);
   Ptr<Ipv4StaticRouting> staticRoutingNode8_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node8_4);

 Ptr<Ipv4StaticRouting> staticRoutingNode9_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node9_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode9_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node9_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode9_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node9_3);
   Ptr<Ipv4StaticRouting> staticRoutingNode9_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node9_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode10_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node10_1);  
  Ptr<Ipv4StaticRouting> staticRoutingNode10_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node10_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode10_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node10_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode10_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node10_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode11_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node11_1);  
  Ptr<Ipv4StaticRouting> staticRoutingNode11_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node11_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode11_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node11_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode11_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node11_4);

Ptr<Ipv4StaticRouting> staticRoutingNode12_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node12_1);  
  Ptr<Ipv4StaticRouting> staticRoutingNode12_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node12_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode12_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node12_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode12_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node12_4);
  
Ptr<Ipv4StaticRouting> staticRoutingNode13_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node13_1);  
  Ptr<Ipv4StaticRouting> staticRoutingNode13_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node13_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode13_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node13_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode13_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node13_4);
  
  Ptr<Ipv4StaticRouting> staticRoutingNode14_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node14_1);  
  Ptr<Ipv4StaticRouting> staticRoutingNode14_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node14_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode14_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node14_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode14_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node14_4);
  
  Ptr<Ipv4StaticRouting> staticRoutingNode15_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node15_1); 
  Ptr<Ipv4StaticRouting> staticRoutingNode15_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node15_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode15_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node15_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode15_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node15_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode16_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node16_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode16_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node16_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode16_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node16_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode16_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node16_4);
  
Ptr<Ipv4StaticRouting> staticRoutingNode17_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node17_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode17_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node17_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode17_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node17_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode17_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node17_4);
  
Ptr<Ipv4StaticRouting> staticRoutingNode18_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node18_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode18_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node18_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode18_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node18_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode18_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node18_4);
  
Ptr<Ipv4StaticRouting> staticRoutingNode19_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node19_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode19_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node19_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode19_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node19_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode19_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node19_4);
  
Ptr<Ipv4StaticRouting> staticRoutingNode20_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node20_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode20_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node20_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode20_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node20_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode20_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node20_4);
  
  Ptr<Ipv4StaticRouting> staticRoutingNode21_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node21_1); 
  Ptr<Ipv4StaticRouting> staticRoutingNode21_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node21_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode21_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node21_3);
    Ptr<Ipv4StaticRouting> staticRoutingNode21_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node21_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode22_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node22_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode22_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node22_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode22_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node22_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode22_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node22_4);

  Ptr<Ipv4StaticRouting> staticRoutingNode23_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node23_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode23_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node23_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode23_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node23_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode23_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node23_4);
 

  Ptr<Ipv4StaticRouting> staticRoutingNode24_1 = ipv4RoutingHelper.GetStaticRouting (ipv4Node24_1);
  Ptr<Ipv4StaticRouting> staticRoutingNode24_2 = ipv4RoutingHelper.GetStaticRouting (ipv4Node24_2);
  Ptr<Ipv4StaticRouting> staticRoutingNode24_3 = ipv4RoutingHelper.GetStaticRouting (ipv4Node24_3);
  Ptr<Ipv4StaticRouting> staticRoutingNode24_4 = ipv4RoutingHelper.GetStaticRouting (ipv4Node24_4); 
 
     PointerValue tmpPhy;

  
//Flow-1: Create static route from 0 -> 1 -> 6
// **************************************************************************************
devices2.Get(0)->GetAttribute("Phy",tmpPhy);
Ptr<Object> wi = tmpPhy.GetObject();
Ptr<YansWifiPhy> yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);
devices4.Get(1)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);

devices1.Get(1)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);
devices3.Get(6)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);
   
   
   staticRoutingNode0_2->AddHostRouteTo (Ipv4Address ("10.1.3.7"), Ipv4Address ("10.1.4.2"), 2);
   staticRoutingNode1_1->AddHostRouteTo (Ipv4Address ("10.1.3.7"), Ipv4Address ("10.1.3.7"), 1);
 
  //Flow-2: Create static route from 0 -> 5 -> 10 -> 15 -> 16
  // **************************************************************************************

devices1.Get(0)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);
devices3.Get(5)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);

devices1.Get(5)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);
devices3.Get(10)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);

devices1.Get(10)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);
devices3.Get(15)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);

devices2.Get(15)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);
devices4.Get(16)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);
 

staticRoutingNode0_1->AddHostRouteTo (Ipv4Address ("10.1.4.17"), Ipv4Address ("10.1.3.6"), 1);
  staticRoutingNode5_1->AddHostRouteTo (Ipv4Address ("10.1.4.17"), Ipv4Address ("10.1.3.11"), 1);
  staticRoutingNode10_1->AddHostRouteTo (Ipv4Address ("10.1.4.17"), Ipv4Address ("10.1.3.16"), 1);
  staticRoutingNode15_2->AddHostRouteTo (Ipv4Address ("10.1.4.17"), Ipv4Address ("10.1.4.17"), 2);


//Flow-3: Create static route from 22 -> 21 -> 16
  // **************************************************************************************
   
devices4.Get(22)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);
devices2.Get(21)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);

devices3.Get(21)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);
devices1.Get(16)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);

   staticRoutingNode22_4->AddHostRouteTo (Ipv4Address ("10.1.1.17"), Ipv4Address ("10.1.2.22"), 4);
   staticRoutingNode21_3->AddHostRouteTo (Ipv4Address ("10.1.1.17"), Ipv4Address ("10.1.1.17"), 3);
  
//Flow-4: Create static route from 22 -> 17 -> 12 -> 13
  // **************************************************************************************
devices3.Get(22)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[7]);
devices1.Get(17)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[7]);

devices3.Get(17)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[8]);
devices1.Get(12)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[8]);

devices2.Get(12)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[6]);
devices4.Get(13)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[6]);

   staticRoutingNode22_3->AddHostRouteTo (Ipv4Address ("10.1.4.14"), Ipv4Address ("10.1.1.18"), 3);
   staticRoutingNode17_3->AddHostRouteTo (Ipv4Address ("10.1.4.14"), Ipv4Address ("10.1.1.13"), 3);
   staticRoutingNode12_2->AddHostRouteTo (Ipv4Address ("10.1.4.14"), Ipv4Address ("10.1.4.14"), 2);


//Flow-5: Create static route from 24 -> 23 -> 18 -> 17 -> 16  // **************************************************************************************
 devices4.Get(24)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);
devices2.Get(23)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);
 
devices3.Get(23)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);
devices1.Get(18)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);

devices4.Get(18)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);
devices2.Get(17)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);

devices4.Get(17)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);
devices2.Get(16)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);

staticRoutingNode24_4->AddHostRouteTo (Ipv4Address ("10.1.2.17"), Ipv4Address ("10.1.2.24"), 4);
   staticRoutingNode23_3->AddHostRouteTo (Ipv4Address ("10.1.2.17"), Ipv4Address ("10.1.1.19"), 3);
   staticRoutingNode18_4->AddHostRouteTo (Ipv4Address ("10.1.2.17"), Ipv4Address ("10.1.2.18"), 4);
     staticRoutingNode17_4->AddHostRouteTo (Ipv4Address ("10.1.2.17"), Ipv4Address ("10.1.2.17"), 4);


//Flow-6: Create static route from 24 -> 19 -> 14 -> 13
  // **************************************************************************************
   
devices3.Get(24)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);
devices1.Get(19)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);

devices3.Get(19)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[7]);
devices1.Get(14)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[7]);

devices4.Get(14)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);
devices2.Get(13)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);

staticRoutingNode24_3->AddHostRouteTo (Ipv4Address ("10.1.2.14"), Ipv4Address ("10.1.1.20"), 3);
   staticRoutingNode19_3->AddHostRouteTo (Ipv4Address ("10.1.2.14"), Ipv4Address ("10.1.1.15"), 3);
   staticRoutingNode14_4->AddHostRouteTo (Ipv4Address ("10.1.2.14"), Ipv4Address ("10.1.2.14"), 4);


//Flow-7: Create static route from 2 -> 7 -> 6 
  // **************************************************************************************
devices4.Get(2)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);
devices2.Get(1)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);
   
/*devices1.Get(1)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);
devices3.Get(6)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);*/


     staticRoutingNode2_4->AddHostRouteTo (Ipv4Address ("10.1.3.7"), Ipv4Address ("10.1.2.2"), 4);
     staticRoutingNode1_1->AddHostRouteTo (Ipv4Address ("10.1.3.7"), Ipv4Address ("10.1.3.7"), 1);

//Flow-8: Create static route from 2 -> 3 -> 8 -> 13  
  // **************************************************************************************
devices2.Get(2)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[8]);
devices4.Get(3)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[8]);

devices1.Get(3)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);
devices3.Get(8)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);
  
devices1.Get(8)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);
devices3.Get(13)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[2]);

 staticRoutingNode2_2->AddHostRouteTo (Ipv4Address ("10.1.3.14"), Ipv4Address ("10.1.4.4"), 2);
   staticRoutingNode3_1->AddHostRouteTo (Ipv4Address ("10.1.3.14"), Ipv4Address ("10.1.3.9"), 1);
   staticRoutingNode8_1->AddHostRouteTo (Ipv4Address ("10.1.3.14"), Ipv4Address ("10.1.3.14"), 1);

//Flow-9: Create static route from 4 -> 9 -> 14 -> 13  
  // **************************************************************************************
   
devices1.Get(4)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);
devices3.Get(9)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);

devices1.Get(9)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[8]);
devices3.Get(14)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[8]);

/*devices4.Get(14)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);
devices2.Get(13)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);*/

   staticRoutingNode4_1->AddHostRouteTo (Ipv4Address ("10.1.2.14"), Ipv4Address ("10.1.3.10"), 1);
   staticRoutingNode9_1->AddHostRouteTo (Ipv4Address ("10.1.2.14"), Ipv4Address ("10.1.3.15"), 1);
   staticRoutingNode14_4->AddHostRouteTo (Ipv4Address ("10.1.2.14"), Ipv4Address ("10.1.2.14"), 4);

//Flow-10: Create static route from 4 -> 3 -> 8 -> 7 -> 6 
  // **************************************************************************************
devices4.Get(4)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);
devices2.Get(3)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[5]);
   
devices4.Get(8)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);
devices2.Get(7)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[1]);

devices4.Get(7)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[7]);
devices2.Get(6)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[7]);


   staticRoutingNode4_4->AddHostRouteTo (Ipv4Address ("10.1.2.7"), Ipv4Address ("10.1.2.4"), 4);
   staticRoutingNode3_1->AddHostRouteTo (Ipv4Address ("10.1.2.7"), Ipv4Address ("10.1.3.9"), 1);
   staticRoutingNode8_4->AddHostRouteTo (Ipv4Address ("10.1.2.7"), Ipv4Address ("10.1.2.8"), 4);
   staticRoutingNode7_4->AddHostRouteTo (Ipv4Address ("10.1.2.7"), Ipv4Address ("10.1.2.7"), 4);

//Flow-11: Create static route from 20 -> 21 -> 16 
  // **************************************************************************************
devices2.Get(20)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);
devices4.Get(21)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);

/*devices3.Get(21)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);
devices1.Get(16)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);*/

   staticRoutingNode20_2->AddHostRouteTo (Ipv4Address ("10.1.1.17"), Ipv4Address ("10.1.4.22"), 2);
   staticRoutingNode21_3->AddHostRouteTo (Ipv4Address ("10.1.1.17"), Ipv4Address ("10.1.1.17"), 3);

//Flow-12: Create static route from 20 -> 15 -> 11 -> 6 
  // **************************************************************************************
devices3.Get(20)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[6]);
devices1.Get(15)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[6]);

devices4.Get(15)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);
devices1.Get(11)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[4]);

devices3.Get(11)->GetAttribute("Phy",tmpPhy); 
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);
devices1.Get(6)->GetAttribute("Phy",tmpPhy);
wi = tmpPhy.GetObject();
yanswi = wi->GetObject<YansWifiPhy>();
yanswi->SetChannel(chnls[3]);


   staticRoutingNode20_3->AddHostRouteTo (Ipv4Address ("10.1.1.7"), Ipv4Address ("10.1.1.16"), 3);
   staticRoutingNode15_4->AddHostRouteTo (Ipv4Address ("10.1.1.7"), Ipv4Address ("10.1.1.12"), 4);
      staticRoutingNode11_3->AddHostRouteTo (Ipv4Address ("10.1.1.7"), Ipv4Address ("10.1.1.7"), 3);


  // Create Apps

 // uint16_t sinkPort = 6; // use the same for all apps

  
   //Create Flows
   //**********************************************************************************************************************


 // Flow-1: UDP connection-1 from N0 to N6 (Path 0 -> 1 -> 6) 

   Address sinkAddress1 (InetSocketAddress (ifcont3.GetAddress (6), 80)); // interface of n6
   PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps1 = packetSinkHelper1.Install (c3.Get (6)); //n24 as sink
   sinkApps1.Start (Seconds (0.));
   sinkApps1.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (c2.Get (0), UdpSocketFactory::GetTypeId ()); //source at n0
  
   uint16_t sport1 = 80;
   Ipv4Address staddr1 (ifcont2.GetAddress (0));
   InetSocketAddress src1 = InetSocketAddress (staddr1, sport1);
   ns3UdpSocket1->Bind (src1);
  
   // Create UDP application at n0
   Ptr<MyApp> app1 = CreateObject<MyApp> ();
   app1->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("5Mbps"));
   c2.Get (0)->AddApplication (app1);
   app1->SetStartTime (Seconds (1.));
   app1->SetStopTime (Seconds (500.));

  // Flow-2: UDP connection-2 from N0 to N6 (Path 0 -> 5 -> 10 -> 15 -> 16) 

   Address sinkAddress2 (InetSocketAddress (ifcont4.GetAddress (16), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c4.Get (16)); //n9 as sink
   sinkApps2.Start (Seconds (0.));
   sinkApps2.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (c1.Get (0), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport2 = 80;
  Ipv4Address staddr2 (ifcont1.GetAddress (0));
  InetSocketAddress src2 = InetSocketAddress (staddr2, sport2);
  ns3UdpSocket2->Bind (src2);
  

   // Create UDP application at n0
   Ptr<MyApp> app2 = CreateObject<MyApp> ();
   app2->Setup (ns3UdpSocket2, sinkAddress2, packetSize, numPackets, DataRate ("5Mbps"));
   c1.Get (0)->AddApplication (app2);
   app2->SetStartTime (Seconds (1.));
   app2->SetStopTime (Seconds (500.));


// Flow-3: UDP connection-1 from N0 to N6 (Path 22 ->21 -> 16) 

   Address sinkAddress3 (InetSocketAddress (ifcont1.GetAddress (16), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c1.Get (16)); //n9 as sink
   sinkApps3.Start (Seconds (0.));
   sinkApps3.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (c4.Get (22), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport3 = 80;
  Ipv4Address staddr3 (ifcont4.GetAddress (22));
  InetSocketAddress src3 = InetSocketAddress (staddr3, sport3);
  ns3UdpSocket3->Bind (src3);
  

   // Create UDP application at n0
   Ptr<MyApp> app3 = CreateObject<MyApp> ();
   app3->Setup (ns3UdpSocket3, sinkAddress3, packetSize, numPackets, DataRate ("5Mbps"));
   c4.Get (22)->AddApplication (app3);
   app3->SetStartTime (Seconds (1.));
   app3->SetStopTime (Seconds (500.));

// Flow-4: UDP connection-2 from N0 to N6 (Path 22 -> 17 -> 12 -> 13) 

   Address sinkAddress4 (InetSocketAddress (ifcont4.GetAddress (13), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper4 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps4 = packetSinkHelper4.Install (c4.Get (13)); //n9 as sink
   sinkApps4.Start (Seconds (0.));
   sinkApps4.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket4 = Socket::CreateSocket (c3.Get (22), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport4 = 80;
  Ipv4Address staddr4 (ifcont3.GetAddress (22));
  InetSocketAddress src4 = InetSocketAddress (staddr4, sport4);
  ns3UdpSocket4->Bind (src4);
  

   // Create UDP application at n0
   Ptr<MyApp> app4 = CreateObject<MyApp> ();
   app4->Setup (ns3UdpSocket4, sinkAddress4, packetSize, numPackets, DataRate ("5Mbps"));
   c3.Get (22)->AddApplication (app4);
   app4->SetStartTime (Seconds (1.));
   app4->SetStopTime (Seconds (500.));

// Flow-5: UDP connection-1 from N24 to N16 (Path 24 -> 23 -> 18 -> 17 -> 16) 

   Address sinkAddress5 (InetSocketAddress (ifcont2.GetAddress (16), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper5 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps5 = packetSinkHelper5.Install (c2.Get (16)); //n9 as sink
   sinkApps5.Start (Seconds (0.));
   sinkApps5.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket5 = Socket::CreateSocket (c4.Get (24), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport5 = 80;
  Ipv4Address staddr5 (ifcont4.GetAddress (24));
  InetSocketAddress src5 = InetSocketAddress (staddr5, sport5);
  ns3UdpSocket5->Bind (src5);
  

   // Create UDP application at n0
   Ptr<MyApp> app5 = CreateObject<MyApp> ();
   app5->Setup (ns3UdpSocket5, sinkAddress5, packetSize, numPackets, DataRate ("5Mbps"));
   c4.Get (24)->AddApplication (app5);
   app5->SetStartTime (Seconds (1.));
   app5->SetStopTime (Seconds (500.));

// Flow-6: UDP connection-2 from N24 to N13 (Path 24 -> 19 -> 14 -> 13) 

   Address sinkAddress6 (InetSocketAddress (ifcont2.GetAddress (13), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper6 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps6 = packetSinkHelper6.Install (c2.Get (13)); //n9 as sink
   sinkApps6.Start (Seconds (0.));
   sinkApps6.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket6 = Socket::CreateSocket (c3.Get (24), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport6 = 80;
  Ipv4Address staddr6 (ifcont3.GetAddress (24));
  InetSocketAddress src6 = InetSocketAddress (staddr6, sport6);
  ns3UdpSocket6->Bind (src6);
  

   // Create UDP application at n0
   Ptr<MyApp> app6 = CreateObject<MyApp> ();
   app6->Setup (ns3UdpSocket6, sinkAddress6, packetSize, numPackets, DataRate ("5Mbps"));
   c3.Get (24)->AddApplication (app6);
   app6->SetStartTime (Seconds (1.));
   app6->SetStopTime (Seconds (500.));

// Flow-7: UDP connection-2 from N2 to N6 (Path 2 -> 7 -> 6) 

   Address sinkAddress7 (InetSocketAddress (ifcont3.GetAddress (6), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper7 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps7 = packetSinkHelper7.Install (c3.Get (6)); //n9 as sink
   sinkApps7.Start (Seconds (0.));
   sinkApps7.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket7 = Socket::CreateSocket (c4.Get (2), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport7 = 80;
  Ipv4Address staddr7 (ifcont4.GetAddress (2));
  InetSocketAddress src7 = InetSocketAddress (staddr7, sport7);
  ns3UdpSocket7->Bind (src7);
  

   // Create UDP application at n0
   Ptr<MyApp> app7 = CreateObject<MyApp> ();
   app7->Setup (ns3UdpSocket7, sinkAddress7, packetSize, numPackets, DataRate ("5Mbps"));
   c4.Get (2)->AddApplication (app7);
   app7->SetStartTime (Seconds (1.));
   app7->SetStopTime (Seconds (500.));

// Flow-8: UDP connection-2 from N2 to N13 (Path 2 -> 3 -> 8 -> 13) 

   Address sinkAddress8 (InetSocketAddress (ifcont3.GetAddress (13), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper8 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps8 = packetSinkHelper8.Install (c3.Get (13)); //n9 as sink
   sinkApps8.Start (Seconds (0.));
   sinkApps8.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket8 = Socket::CreateSocket (c2.Get (2), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport8 = 80;
  Ipv4Address staddr8 (ifcont2.GetAddress (2));
  InetSocketAddress src8 = InetSocketAddress (staddr8, sport8);
  ns3UdpSocket8->Bind (src8);
  

   // Create UDP application at n0
   Ptr<MyApp> app8 = CreateObject<MyApp> ();
   app8->Setup (ns3UdpSocket8, sinkAddress8, packetSize, numPackets, DataRate ("5Mbps"));
   c2.Get (2)->AddApplication (app8);
   app8->SetStartTime (Seconds (1.));
   app8->SetStopTime (Seconds (500.));

// Flow-9: UDP connection-1 from N4 to N13 (Path 4 -> 9 -> 14 -> 13) 

   Address sinkAddress9 (InetSocketAddress (ifcont2.GetAddress (13), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper9 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps9 = packetSinkHelper9.Install (c2.Get (13)); //n9 as sink
   sinkApps9.Start (Seconds (0.));
   sinkApps9.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket9 = Socket::CreateSocket (c1.Get (4), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport9 = 80;
  Ipv4Address staddr9 (ifcont1.GetAddress (4));
  InetSocketAddress src9 = InetSocketAddress (staddr9, sport9);
  ns3UdpSocket9->Bind (src9);
 

   // Create UDP application at n0
   Ptr<MyApp> app9 = CreateObject<MyApp> ();
   app9->Setup (ns3UdpSocket9, sinkAddress9, packetSize, numPackets, DataRate ("5Mbps"));
   c1.Get (4)->AddApplication (app9);
   app9->SetStartTime (Seconds (1.));
   app9->SetStopTime (Seconds (500.));

// Flow-10: UDP connection-1 from N4 to N6 (Path 4 -> 3 -> 2 -> 7 -> 6) 

   Address sinkAddress10 (InetSocketAddress (ifcont2.GetAddress (6), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper10 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps10 = packetSinkHelper10.Install (c2.Get (6)); //n9 as sink
   sinkApps10.Start (Seconds (0.));
   sinkApps10.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket10 = Socket::CreateSocket (c4.Get (4), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport10 = 80;
  Ipv4Address staddr10 (ifcont4.GetAddress (4));
  InetSocketAddress src10 = InetSocketAddress (staddr10, sport10);
  ns3UdpSocket10->Bind (src10);
 

   // Create UDP application at n0
   Ptr<MyApp> app10 = CreateObject<MyApp> ();
   app10->Setup (ns3UdpSocket10, sinkAddress10, packetSize, numPackets, DataRate ("5Mbps"));
   c4.Get (4)->AddApplication (app10);
   app10->SetStartTime (Seconds (1.));
   app10->SetStopTime (Seconds (500.));

// Flow-11: UDP connection-1 from N20 to N16 (Path 20 -> 21 -> 16) 

   Address sinkAddress11 (InetSocketAddress (ifcont1.GetAddress (16), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper11 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps11 = packetSinkHelper11.Install (c1.Get (6)); //n9 as sink
   sinkApps11.Start (Seconds (0.));
   sinkApps11.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket11 = Socket::CreateSocket (c2.Get (20), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport11 = 80;
  Ipv4Address staddr11 (ifcont2.GetAddress (20));
  InetSocketAddress src11 = InetSocketAddress (staddr11, sport11);
  ns3UdpSocket11->Bind (src11);
 

   // Create UDP application at n0
   Ptr<MyApp> app11 = CreateObject<MyApp> ();
   app11->Setup (ns3UdpSocket11, sinkAddress11, packetSize, numPackets, DataRate ("5Mbps"));
   c2.Get (20)->AddApplication (app11);
   app11->SetStartTime (Seconds (1.));
   app11->SetStopTime (Seconds (500.));

// Flow-12: UDP connection-1 from N20 to N6 (Path 20 -> 15 -> 11 - > 6) 

   Address sinkAddress12 (InetSocketAddress (ifcont1.GetAddress (6), 80)); // interface of n9
   PacketSinkHelper packetSinkHelper12 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 80));
   ApplicationContainer sinkApps12 = packetSinkHelper12.Install (c1.Get (6)); //n9 as sink
   sinkApps12.Start (Seconds (0.));
   sinkApps12.Stop (Seconds (500.));

   Ptr<Socket> ns3UdpSocket12 = Socket::CreateSocket (c3.Get (20), UdpSocketFactory::GetTypeId ()); //source at n0
   uint16_t sport12 = 80;
  Ipv4Address staddr12 (ifcont3.GetAddress (20));
  InetSocketAddress src12 = InetSocketAddress (staddr12, sport12);
  ns3UdpSocket12->Bind (src12);
 

   // Create UDP application at n0
   Ptr<MyApp> app12 = CreateObject<MyApp> ();
   app12->Setup (ns3UdpSocket12, sinkAddress12, packetSize, numPackets, DataRate ("5Mbps"));
   c3.Get (20)->AddApplication (app12);
   app12->SetStartTime (Seconds (1.));
   app12->SetStopTime (Seconds (500.));





  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();


//ASCII Trace
//AsciiTraceHelper ascii;
 //    wifiPhy.EnableAsciiAll (ascii.CreateFileStream ( "DTE_5by5_0G1_0G3_22G3_24G3.tr"));

	
  

  // Trace Collisions
  //Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));
 // Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));
 // Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));

  //Simulator::Schedule(Seconds(5.0), &PrintDrop);



   //   Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("DTE_5by5_0G1_0G3_22G3_22G2_24G3.routes", std::ios::out);
   //   olsr.PrintRoutingTableAllEvery (Seconds (2), routingStream);
     // Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper> ("DTE_5by5_0G1_0G3_22G3_22G2_24G3.neighbors", std::ios::out);
     // olsr.PrintNeighborCacheAllEvery (Seconds (2), neighborStream);




  Simulator::Stop (Seconds (500.0));
  AnimationInterface anim ("DTE_5by5_0G1_0G3_22G3_22G2_24G3.xml");
  Simulator::Run ();

//  PrintDrop();

        FILE *fp;
	fp=fopen("DTE_12f_5Mbps_802a.txt","w");
	
  // Print per flow statistics
  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);

      if ((t.sourceAddress == Ipv4Address("10.1.2.1") && t.destinationAddress == Ipv4Address("10.1.3.7"))
    	|| (t.sourceAddress == Ipv4Address("10.1.1.1") && t.destinationAddress == Ipv4Address("10.1.4.17"))
        || (t.sourceAddress == Ipv4Address("10.1.4.23") && t.destinationAddress == Ipv4Address("10.1.1.17"))
    	|| (t.sourceAddress == Ipv4Address("10.1.3.23") && t.destinationAddress == Ipv4Address("10.1.4.14"))
        || (t.sourceAddress == Ipv4Address("10.1.4.25") && t.destinationAddress == Ipv4Address("10.1.2.17"))
        || (t.sourceAddress == Ipv4Address("10.1.3.25") && t.destinationAddress == Ipv4Address("10.1.2.14"))
	|| (t.sourceAddress == Ipv4Address("10.1.4.3") && t.destinationAddress == Ipv4Address("10.1.3.7"))
	|| (t.sourceAddress == Ipv4Address("10.1.2.3") && t.destinationAddress == Ipv4Address("10.1.3.14"))
 	|| (t.sourceAddress == Ipv4Address("10.1.1.5") && t.destinationAddress == Ipv4Address("10.1.2.14"))
        || (t.sourceAddress == Ipv4Address("10.1.4.5") && t.destinationAddress == Ipv4Address("10.1.2.7"))
	|| (t.sourceAddress == Ipv4Address("10.1.2.21") && t.destinationAddress == Ipv4Address("10.1.1.17"))
	|| (t.sourceAddress == Ipv4Address("10.1.3.21") && t.destinationAddress == Ipv4Address("10.1.1.7")))

        {
    	  NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
        fprintf(fp,"%d\t",iter->first);
		

    	  NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
        fprintf(fp,"%d\t",iter->second.txPackets);
    	  NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
        fprintf(fp,"%d\t",iter->second.rxPackets);
    	  NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps");
        fprintf(fp,"%lf\t",iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024);


NS_LOG_UNCOND ("  Mean delay:   " << iter->second.delaySum.GetSeconds () / iter->second.rxPackets << "\n");
        fprintf(fp,"%lf\t",iter->second.delaySum.GetSeconds () / iter->second.rxPackets);
NS_LOG_UNCOND ("  Mean jitter:   " << iter->second.jitterSum.GetSeconds () / (iter->second.rxPackets - 1) << "\n");
        fprintf(fp,"%lf\t",iter->second.jitterSum.GetSeconds () / (iter->second.rxPackets - 1) );
        }
	fprintf(fp,"\n\n");
    }

	fclose(fp);




  monitor->SerializeToXmlFile("grid-4by4_0211.flowmon", true, true);

  Simulator::Destroy ();

  return 0;
}

