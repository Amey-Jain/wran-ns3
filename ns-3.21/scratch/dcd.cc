// 802.11b NICs in adhoc mode, and by default, sends one packet of 1000 
// (application) bytes to node 1.
//
//
//
// There are a number of command-line options available to control
// the default behavior.  The list of available command-line options
// can be listed with the following command:
// ./waf --run "wifi-simple-adhoc-grid --help"
//
// Note that all ns-3 attributes (not just the ones exposed in the below
// script) can be changed at command line; see the ns-3 documentation.
//
// For instance, for this configuration, the physical layer will
// stop successfully receiving packets when distance increases beyond
// the default of 500m.
// To see this effect, try running:
//
// ./waf --run "wifi-simple-adhoc --distance=500"
// ./waf --run "wifi-simple-adhoc --distance=1000"
// ./waf --run "wifi-simple-adhoc --distance=1500"
// 
// The source node and sink node can be changed like this:
// 
// ./waf --run "wifi-simple-adhoc --sourceNode=20 --sinkNode=10"
//
// This script can also be helpful to put the Wifi layer into verbose
// logging mode; this command will turn on all wifi logging:
// 
// ./waf --run "wifi-simple-adhoc-grid --verbose=1"
//
// By default, trace file writing is off-- to enable it, try:
// ./waf --run "wifi-simple-adhoc-grid --tracing=1"
//
// When you are done tracing, you will notice many pcap trace files 
// in your directory.  If you have tcpdump installed, you can try this:
//
// tcpdump -r wifi-simple-adhoc-grid-0-0.pcap -nn -tt
//

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/energy-module.h"
#include "ns3/olsr-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "myapp.h"
 
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <fstream>

#include <algorithm>
#include <cmath>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <map>
#include <iomanip>
#include <cstdlib>
#include <ctime>

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("WifiSimpleAdhocGrid");

uint32_t MacTxDropCount, PhyTxDropCount, PhyRxDropCount;

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
}

void ReceivePacket (Ptr<Socket> socket)
{
    uint8_t *buffer_temp=new uint8_t[1013];
    std:: cout<<"hello-receive packet\n";  
    
    Address from; 
    Ptr<Packet> packet= socket->RecvFrom(from); //get the packet from the address 
    packet->CopyData(buffer_temp,10); //copy the data into buffer


    for(int i=0;i<10;i++){  //print the data
        std::cout << *buffer_temp ;
        buffer_temp++;
        }
    std::cout<<"\n";


//get the receiver ip address
        Address addr;
        socket->GetSockName (addr);
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (addr); 
       std::cout<<"receiver-IP address:"<< iaddr.GetIpv4() << " port number: " << iaddr.GetPort ()<<"\n";

// get the senderr ip address   
      Ipv4Address ipv4From =  InetSocketAddress::ConvertFrom(from).GetIpv4(); 
      InetSocketAddress source_add = InetSocketAddress::ConvertFrom (from); 

//print the address     
      std::cout<<"sender - IP address"<<ipv4From <<"\n";

// create a new packet to reply to sender
   Ptr<Packet> reppacket= Create<Packet> (reinterpret_cast<const uint8_t*> ("ACK!"), 5);
    uint8_t *buffer_return =new uint8_t[1013] ;
     reppacket->CopyData(buffer_return,5);
  
//send the packet
socket->SendTo (buffer_return,5,0,source_add);



}

void ReceivePacketreply(Ptr<Socket> sockett)
{
    uint8_t *buffer_tempp=new uint8_t[1013];
   std:: cout<<"Got-reply from the sender\n";  

    Address fromm; 
    Ptr<Packet> packet= sockett->RecvFrom(fromm); //get the packet from the address 
    packet->CopyData(buffer_tempp,10); //copy the data into buffer

    for(int i=0;i<10;i++){  //print the data
        std::cout << *buffer_tempp ;
        buffer_tempp++;
        }
    std::cout<<"\n";

//get the receiver ip address
       Address addr;
        sockett->GetSockName (addr);
        InetSocketAddress iaddr = InetSocketAddress::ConvertFrom (addr); 
       std::cout<<"Acknowledgement packet for the node:"<< iaddr.GetIpv4() << " port: " << iaddr.GetPort ()<<"\n";

// get the senderr ip address   
      Ipv4Address ipv4Fromm =  InetSocketAddress::ConvertFrom(fromm).GetIpv4(); 

//print the address     
      std::cout<<"sender-address"<<ipv4Fromm<<"\n";
sockett->Close ();
  
}


static void GenerateTraffic (Ptr<Socket> socket, uint32_t pktSize, 
                           uint32_t pktCount, Time pktInterval )
{
  Ptr<Packet> pkkt= Create<Packet> (reinterpret_cast<const uint8_t*> ("hhhhh"), 10);
  if (pktCount > 0)
    {
         
       socket->Send (pkkt);
      Simulator::Schedule (pktInterval, &GenerateTraffic, 
                           socket, pktSize,pktCount-1, pktInterval);
    }
  //else
    //{
      //socket->Close ();
    //}
}

double getAngle(Vector v1,Vector v2)
{
	float pi = 3.14;
	//double angle = ( atan2(v2.y, v2.x) - atan2(v1.y, v1.x) )*180.0/pi;
	double angle = ( atan2(v2.x - v1.x , v2.y - v1.y) )*180.0/pi;
	if(angle < 0 ) angle += 360;
	return angle;
}
int coordinate(Vector v1,Vector v2)
{
	
		if(v1.x<=v2.x && v1.y<=v2.y)	return 0;
		else if(v1.x>=v2.x && v1.y<=v2.y)	return 1;
		else if(v1.x>=v2.x && v1.y>=v2.y)	return 2;
		else if(v1.x<=v2.x && v1.y>=v2.y)	return 3;
		return -1;
}

void
RemainingEnergy (double oldValue, double remainingEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s Current remaining energy = " << remainingEnergy << "J");
}

/// Trace function for total energy consumption at node.
void
TotalEnergy (double oldValue, double totalEnergy)
{
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds ()
                 << "s Total energy consumed by radio = " << totalEnergy << "J");
}


int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  uint32_t packetSize = 512; // bytes
  uint32_t numPackets = 2;
  uint32_t numNodes = 300;  // by default, 5x5
  uint32_t sinkNode = 136;
  uint32_t sourceNode = 24;
  double interval = 1.0; // seconds
  bool verbose = false;
  bool tracing = true;

  CommandLine cmd;

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("distance", "distance (m)", distance);
  cmd.AddValue ("packetSize", "size of application packet sent", packetSize);
  cmd.AddValue ("numPackets", "number of packets generated", numPackets);
  cmd.AddValue ("interval", "interval (seconds) between packets", interval);
  cmd.AddValue ("verbose", "turn on all WifiNetDevice log components", verbose);
  cmd.AddValue ("tracing", "turn on ascii and pcap tracing", tracing);
  cmd.AddValue ("numNodes", "number of nodes", numNodes);
  cmd.AddValue ("sinkNode", "Receiver node number", sinkNode);
  cmd.AddValue ("sourceNode", "Sender node number", sourceNode);

  cmd.Parse (argc, argv);
  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", 
                      StringValue (phyMode));

  NodeContainer c;
  c.Create (numNodes);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;
  if (verbose)
    {
      wifi.EnableLogComponents ();  // Turn on all Wifi logging
    }

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-10) ); 
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);
  
//************************************
//*********deploy nodes***************
//************************************

  
 MobilityHelper mobility;
 
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
  "X", StringValue ("500.0"),
  "Y", StringValue ("500.0"),
  "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=600]"));
  
  
  
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  //mobility.SetPositionAllocator (positionAlloc);
  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);
  
//****************************************
//*********Routing Protocol***************
//****************************************
  // Enable OLSR
  OlsrHelper olsr;
  Ipv4StaticRoutingHelper staticRouting;

  Ipv4ListRoutingHelper list;
  list.Add (staticRouting, 0);
  list.Add (olsr, 10);

  InternetStackHelper internet;
  internet.SetRoutingHelper (list); // has effect on the next Install ()
  internet.Install (c);
  
  //*****************************************************************
  //                      Energy Model
  //*****************************************************************
  /* energy source */
  BasicEnergySourceHelper basicSourceHelper;
  // configure energy source
  basicSourceHelper.Set ("BasicEnergySourceInitialEnergyJ", DoubleValue (0.1));
  // install source
  EnergySourceContainer sources = basicSourceHelper.Install (c);
  /* device energy model */
  WifiRadioEnergyModelHelper radioEnergyHelper;
  // configure radio energy model
  radioEnergyHelper.Set ("TxCurrentA", DoubleValue (0.0174));
  // install device model
  DeviceEnergyModelContainer deviceModels = radioEnergyHelper.Install (devices, sources);
 //**************************************************************************************
  
  
//****************************************
//************IPv4 address****************
//****************************************

  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.0.0.0", "255.255.0.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devices);

  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  
  //get the ip address of node 
  
  //
 
 
  Ptr<Socket> recvSink = Socket::CreateSocket (c.Get (sinkNode), tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (c.Get (sourceNode), tid);
  InetSocketAddress remote = InetSocketAddress (i.GetAddress (sinkNode, 135), 80);
  source->Connect (remote);
  
//********************************************************************
//*************Event Generation & Send Packet*************************
// Create Apps
Ipv4InterfaceContainer ifcont = ipv4.Assign (devices);

  uint16_t sinkPort = 6; // use the same for all apps
 
  //Event A
  //Burst 1

  // UDP connection from N0 to N24

   Address sinkAddress1 (InetSocketAddress (ifcont.GetAddress (135), sinkPort)); // interface of n135
   PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
   ApplicationContainer sinkApps1 = packetSinkHelper1.Install (c.Get (135)); 
   sinkApps1.Start (Seconds (0.));
   sinkApps1.Stop (Seconds (1000.));

   Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (c.Get (66), UdpSocketFactory::GetTypeId ()); //source at n66

   // Create UDP application at n66
   Ptr<MyApp> app1 = CreateObject<MyApp> ();
   app1->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (66)->AddApplication (app1);
   app1->SetStartTime (Seconds (20.));
   app1->SetStopTime (Seconds (40.));
   
   // Create UDP application at n23
   Ptr<MyApp> app4 = CreateObject<MyApp> ();
   app4->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (23)->AddApplication (app4);
   app4->SetStartTime (Seconds (20.));
   app4->SetStopTime (Seconds (40.));
   
   // Create UDP application at n137
   Ptr<MyApp> app5 = CreateObject<MyApp> ();
   app5->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (137)->AddApplication (app5);
   app5->SetStartTime (Seconds (20.));
   app5->SetStopTime (Seconds (40.));
   
   // Create UDP application at n123
   Ptr<MyApp> app6 = CreateObject<MyApp> ();
   app6->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (123)->AddApplication (app6);
   app6->SetStartTime (Seconds (20.));
   app6->SetStopTime (Seconds (40.));
   
   // Create UDP application at n63
   Ptr<MyApp> app7 = CreateObject<MyApp> ();
   app7->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (63)->AddApplication (app7);
   app7->SetStartTime (Seconds (20.));
   app7->SetStopTime (Seconds (40.));
   
   // Create UDP application at n245
   Ptr<MyApp> app8 = CreateObject<MyApp> ();
   app8->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (245)->AddApplication (app8);
   app8->SetStartTime (Seconds (20.));
   app8->SetStopTime (Seconds (40.));
   
    // Create UDP application at n187
   Ptr<MyApp> app9 = CreateObject<MyApp> ();
   app9->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (187)->AddApplication (app9);
   app9->SetStartTime (Seconds (20.));
   app9->SetStopTime (Seconds (40.));
   
   // Create UDP application at n71
   Ptr<MyApp> app10 = CreateObject<MyApp> ();
   app10->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (71)->AddApplication (app10);
   app10->SetStartTime (Seconds (20.));
   app10->SetStopTime (Seconds (40.));
   
   // Create UDP application at n6
   Ptr<MyApp> app11 = CreateObject<MyApp> ();
   app11->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("2Mbps"));
   c.Get (6)->AddApplication (app11);
   app11->SetStartTime (Seconds (20.));
   app11->SetStopTime (Seconds (40.));
   
  //Event B
 //Burst 1

   // UDP connection from N10 to N14

    Address sinkAddress2 (InetSocketAddress (ifcont.GetAddress (14), sinkPort)); // interface of n14
    PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c.Get (14)); //n14 as sink
    sinkApps2.Start (Seconds (0.));
    sinkApps2.Stop (Seconds (100.));

    Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (c.Get (10), UdpSocketFactory::GetTypeId ()); //source at n10

    // Create UDP application at n10
    Ptr<MyApp> app2 = CreateObject<MyApp> ();
    app2->Setup (ns3UdpSocket2, sinkAddress2, packetSize, numPackets, DataRate ("1Mbps"));
    c.Get (10)->AddApplication (app2);
    app2->SetStartTime (Seconds (31.5));
    app2->SetStopTime (Seconds (100.));

    // UDP connection from N20 to N4

     Address sinkAddress3 (InetSocketAddress (ifcont.GetAddress (4), sinkPort)); // interface of n4
     PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
     ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c.Get (4)); //n2 as sink
     sinkApps3.Start (Seconds (0.));
     sinkApps3.Stop (Seconds (100.));

     Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (c.Get (20), UdpSocketFactory::GetTypeId ()); //source at n20

     // Create UDP application at n20
     Ptr<MyApp> app3 = CreateObject<MyApp> ();
     app3->Setup (ns3UdpSocket3, sinkAddress3, packetSize, numPackets, DataRate ("1Mbps"));
     c.Get (20)->AddApplication (app3);
     app3->SetStartTime (Seconds (32.));
     app3->SetStopTime (Seconds (100.));

//********************************************************************



//********************************************************************
 // Energy model
 //all sources are connected to node 1
  // energy source
  Ptr<BasicEnergySource> basicSourcePtr = DynamicCast<BasicEnergySource> (sources.Get (1));
  basicSourcePtr->TraceConnectWithoutContext ("RemainingEnergy", MakeCallback (&RemainingEnergy));
  // device energy model
  Ptr<DeviceEnergyModel> basicRadioModelPtr =
    basicSourcePtr->FindDeviceEnergyModels ("ns3::WifiRadioEnergyModel").Get (0);
  NS_ASSERT (basicRadioModelPtr != NULL);
  basicRadioModelPtr->TraceConnectWithoutContext ("TotalEnergyConsumption", MakeCallback (&TotalEnergy));
  //******************************************



  if (tracing == true)
    {
      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream ("wifi-simple-adhoc-grid.tr"));
      wifiPhy.EnablePcap ("wifi-simple-adhoc-grid", devices);
      // Trace routing tables
      Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("dcd.routes", std::ios::out);
      olsr.PrintRoutingTableAllEvery (Seconds (1000), routingStream);
      Ptr<OutputStreamWrapper> neighborStream = Create<OutputStreamWrapper> ("dcd.neighbors", std::ios::out);
//      olsr.PrintNeighborCacheAllEvery (Seconds (1000), neighborStream);
      

      // To do-- enable an IP-level trace that shows forwarding events only
    }
    
    Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));
      Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));
      Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));
    
    
    // Calculate Throughput using Flowmonitor
//
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll();
    
    

  // Give OLSR time to converge-- 30 seconds perhaps
  Simulator::Schedule (Seconds (30.0), &GenerateTraffic, 
                       source, packetSize, numPackets, interPacketInterval);

  // Output what we are doing
  NS_LOG_UNCOND ("Testing from node " << sourceNode << " to " << sinkNode << " with grid distance " << distance);
  
  //Netanim 

  AnimationInterface anim ("dcd.xml"); 
  
  NS_LOG_INFO ("Run Simulation.");
 // Simulator::Stop (Seconds(1000.0));
   
  //Simulator::Run ();
 //
 

  monitor->CheckForLostPackets ();

  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
    {
	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
     // if ((t.sourceAddress=="10.1.1.2" && t.destinationAddress == "10.1.2.1"))
      {
          std::cout << "Flow " << i->first  << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
          std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
          std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
      	  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / (i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps\n";
      }
     }



  monitor->SerializeToXmlFile("dcd.flowmon", true, true);



  Simulator::Stop (Seconds (1000.0));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}

