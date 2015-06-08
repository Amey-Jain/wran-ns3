/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2007,2008, 2009 INRIA, UDcast
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
 * Author: Mohamed Amine Ismail <amine.ismail@sophia.inria.fr>
 *                              <amine.ismail@udcast.com>
 */

//
// Default network topology includes a base station (BS) and 2
// subscriber station (SS).

//      +-----+
//      | SS0 |
//      +-----+
//     10.1.1.1
//      -------
//        ((*))
//
//                  10.1.1.7
//               +------------+
//               |Base Station| ==((*))
//               +------------+
//
//        ((*))
//       -------
//      10.1.1.2
//       +-----+
//       | SS1 |
//       +-----+

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wimax-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/service-flow.h"
#include "ns3/netanim-module.h"
#include "ns3/udp-echo-helper.h"
#include <iostream>
#include <vector>
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("Scratch Simulator");

using namespace ns3;


Ptr<PacketBurst> CreatePacketBurst(){
	Ptr<PacketBurst> burst = Create<PacketBurst> ();
	std::string sentMessage("Hi from bs");
	Ptr<Packet> packet =  Create<Packet> ((uint8_t*) sentMessage.c_str(), sentMessage.length());
	burst->AddPacket (packet);
	return burst;
}

int main (int argc, char *argv[])
{
  bool verbose = false;

  int duration = 10, schedType = 0, mxSS = 30, mxBS = 3, mxPUTx = 5, mxPURx = 15;
  WimaxHelper::SchedulerType scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;

  CommandLine cmd;
  cmd.AddValue ("scheduler", "type of scheduler to use with the network devices", schedType);
  cmd.AddValue ("duration", "duration of the simulation in seconds", duration);
  cmd.AddValue ("verbose", "turn on all WimaxNetDevice log components", verbose);
  cmd.Parse (argc, argv);

//  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
//  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
//  LogComponentEnable("WimaxSimpleExample", LOG_LEVEL_INFO);
//  LogComponentEnable("simpleOfdmWimaxChannel", LOG_LEVEL_INFO);
//  LogComponentEnable("SimpleOfdmWimaxPhy", LOG_LEVEL_INFO);
//  LogComponentEnable("WimaxPhy", LOG_LEVEL_INFO);
//  LogComponentEnable("WimaxBaseStationNetDevice", LOG_LEVEL_INFO);
//  LogComponentEnable("WimaxNetDevice", LOG_LEVEL_INFO);
//  LogComponentEnable("WimaxSubscriberStationNetDevice", LOG_LEVEL_INFO);
//  LogComponentEnable("PropagationLossModel", LOG_LEVEL_INFO);



  switch (schedType)
    {
    case 0:
      scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
      break;
    case 1:
      scheduler = WimaxHelper::SCHED_TYPE_MBQOS;
      break;
    case 2:
      scheduler = WimaxHelper::SCHED_TYPE_RTPS;
      break;
    default:
      scheduler = WimaxHelper::SCHED_TYPE_SIMPLE;
    }

  NodeContainer ssNodes;
  NodeContainer bsNodes;
  NodeContainer puTxNodes;
  NodeContainer puRxNodes;

  ssNodes.Create (mxSS);
  bsNodes.Create (mxBS);
  puTxNodes.Create(mxPUTx);
  puRxNodes.Create(mxPURx);

  WimaxHelper wimax;

//  // Read PU file
//    Ptr<PUModel> puModel = CreateObject<PUModel>();
//    std::string map_file = "map_PUs_multiple.txt";
//    puModel->SetPuMapFile((char*)map_file.c_str());
//    //Create repository
//    Ptr<Repository> repo = CreateObject<Repository>();

  NetDeviceContainer ssDevs, bsDevs;

  ssDevs = wimax.Install (ssNodes,
                          WimaxHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
                          scheduler);
  bsDevs = wimax.Install (bsNodes,
		  	  	  	  	  WimaxHelper::DEVICE_TYPE_BASE_STATION,
		  	  	  	  	  WimaxHelper::SIMPLE_PHY_TYPE_OFDM,
		  	  	  	  	  scheduler);
//  wimax.SetPropagationLossModel(SimpleOfdmWimaxChannel::COST231_PROPAGATION);
  wimax.SetPropagationLossModel(SimpleOfdmWimaxChannel::FRIIS_PROPAGATION);
// wimax.SetPropagationLossModel(SimpleOfdmWimaxChannel::ITU_NLOS_ROOFTOP_PROPAGATION);
//  wimax.EnableAscii ("bs-devices", bsDevs);
//  wimax.EnableAscii ("ss-devices", ssDevs);

  Ptr<SubscriberStationNetDevice> ss[mxSS];

  for (int i = 0; i < mxSS; i++)
    {
      ss[i] = ssDevs.Get (i)->GetObject<SubscriberStationNetDevice> ();
      ss[i]->SetModulationType (WimaxPhy::MODULATION_TYPE_QAM16_12); // its ok for wimax
    }

  Ptr<BaseStationNetDevice> bs[mxBS];

  for(int i=0;i<mxBS;i++){
	  bs[i] = bsDevs.Get (i)->GetObject<BaseStationNetDevice> ();
  }
  InternetStackHelper stack;
  stack.Install (bsNodes);
  stack.Install (ssNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer SSinterfaces = address.Assign (ssDevs);
  Ipv4InterfaceContainer BSinterface = address.Assign (bsDevs);

  MobilityHelper mobility, bsMobility;
/*
  vector<int> pos;
  pos.push_back(3);
  MobilityModel mob;
  mob.SetPosition(pos);
*/

//  bsMobility.SetPositionAllocator ("ns3::GridPositionAllocator",
//                                   "MinX", DoubleValue (0.0),
//                                   "MinY", DoubleValue (50.0),
//                                   "DeltaX", DoubleValue (1000.0),
//                                   "DeltaY", DoubleValue (1000.0),
//                                   "GridWidth", UintegerValue (2),
//                                   "LayoutType", StringValue ("RowFirst"));

  // all units are in km. multiply 1000 to convert it to meter.
  double km = 10.0;
  double rowNeighbourBSOverlappingRegion = 5.0 * km;
  double bsTransmissionRange = 30.0 * km;
  double bsToBsDistanceX = 2.0 * (bsTransmissionRange - rowNeighbourBSOverlappingRegion);
  double bsToBsDistanceY = bsToBsDistanceX * sin(45.0);
  double startCoordinate = 30.0 * km;

  Ptr<ListPositionAllocator> positionAllocBS = CreateObject<ListPositionAllocator> ();
        positionAllocBS->Add (Vector (startCoordinate + (bsToBsDistanceX / 2.0), 					startCoordinate, 							10.0));
        positionAllocBS->Add (Vector (startCoordinate + (bsToBsDistanceX / 2.0) + bsToBsDistanceX, 	startCoordinate, 							10.0));
        positionAllocBS->Add (Vector (startCoordinate, 												startCoordinate + bsToBsDistanceY, 			10.0));
        positionAllocBS->Add (Vector (startCoordinate + bsToBsDistanceX, 							startCoordinate + bsToBsDistanceY, 			10.0));
        positionAllocBS->Add (Vector (startCoordinate + (2.0 * bsToBsDistanceX),					startCoordinate + bsToBsDistanceY, 			10.0));
        positionAllocBS->Add (Vector (startCoordinate + (bsToBsDistanceX / 2.0), 					startCoordinate + (2.0 * bsToBsDistanceY), 	10.0));
        positionAllocBS->Add (Vector (startCoordinate + (bsToBsDistanceX / 2.0) + bsToBsDistanceX, 	startCoordinate + (2.0 * bsToBsDistanceY), 	10.0));
        bsMobility.SetPositionAllocator (positionAllocBS);

      bsMobility.Install (bsNodes);
//  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
//                                   "X", StringValue ("0.0"), // center-x
//                                   "Y", StringValue ("0.0"), // center-y
//                                   "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=6]")); // position radius

      std::string unRanVar("ns3::UniformRandomVariable[Min=0|Max=");
      std::stringstream sstreamX, sstreamY;
      double maxAreaX = 2.0 * (startCoordinate + bsToBsDistanceX);
      double maxAreaY = 2.0 * (startCoordinate + bsToBsDistanceY);
      sstreamX << unRanVar << maxAreaX << "]";
      sstreamY << unRanVar << maxAreaY << "]";
      mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                           "X", StringValue (sstreamX.str()),
                                           "Y", StringValue (sstreamY.str()));
//  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
//                                     "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=5500]"),
//                                     "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=100]"));
//  Ptr<ListPositionAllocator> positionAllocSS = CreateObject<ListPositionAllocator> ();
//  positionAllocSS->Add (Vector (65010, startCoordinate, 10.0));
//  mobility.SetPositionAllocator (positionAllocSS);

//      mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
//                                         "MinX", DoubleValue (250.0),
//                                         "MinY", DoubleValue (0.0),
//                                         "DeltaX", DoubleValue (-500.0),
//                                         "DeltaY", DoubleValue (10.0),
//                                         "GridWidth", UintegerValue (3),
//                                         "LayoutType", StringValue ("RowFirst"));

//      Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
//      positionAlloc->Add (Vector (200.0, 0.0, 2.0));
//      positionAlloc->Add (Vector (210.0, 0.0, 2.0));
//        mobility.SetPositionAllocator (positionAlloc);


  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (ssNodes);
    mobility.Install (puTxNodes);
    mobility.Install (puRxNodes);
//    wimax.GetSpectrumManager()->SetRepository(repo);
//    wimax.GetSpectrumManager()->Start();
  if (verbose)
    {
      wimax.EnableLogComponents ();  // Turn on all wimax logging
    }
  /*------------------------------*/
//  UdpServerHelper udpServer;
//  ApplicationContainer serverApps;
//  UdpClientHelper udpClient;
//  ApplicationContainer clientApps[mxSS - 1];
//
//  udpServer = UdpServerHelper (100);
//
//  serverApps = udpServer.Install (ssNodes.Get (0));
//  serverApps.Start (Seconds (1));
//  serverApps.Stop (Seconds (duration));
//
//  udpClient = UdpClientHelper (SSinterfaces.GetAddress (0), 100);
//  udpClient.SetAttribute ("MaxPackets", UintegerValue (500));
//  udpClient.SetAttribute ("Interval", TimeValue (Seconds (1)));
//  udpClient.SetAttribute ("PacketSize", UintegerValue (1024));
//
//  for(int i=0;i<mxSS-1;i++){
//	  clientApps[i] = udpClient.Install (ssNodes.Get (i+1));
//	    clientApps[i].Start (Seconds (3));
//	    clientApps[i].Stop (Seconds (5));
//  }

//  UdpEchoServerHelper udpServer = UdpEchoServerHelper(1000);
//    ApplicationContainer serverApps;
//    UdpEchoClientHelper udpClient = UdpEchoClientHelper(BSinterface.GetAddress (0),1000);
//    ApplicationContainer clientApps;

//    udpServer = UdpEchoServerHelper (100);

//    serverApps = udpServer.Install (bsNodes.Get (0));
//    serverApps.Start (Seconds (1));
//    serverApps.Stop (Seconds (duration));
//
//    std::string sss("Hello bunny");
////    udpClient = UdpEchoClientHelper (BSinterface.GetAddress (0), 100);
//    udpClient.SetAttribute ("MaxPackets", UintegerValue (500));
//    udpClient.SetAttribute ("Interval", TimeValue (Seconds (10)));
//    udpClient.SetAttribute ("PacketSize", UintegerValue (1024));


////    for(int i=0;i<mxSS;i++){
//  	  clientApps = udpClient.Install (ssNodes.Get (0));
////    }
//  	udpClient.SetFill(clientApps.Get(0),sss);
//  	    clientApps.Start (Seconds (1));
//  	    clientApps.Stop (Seconds (10));
////    }





  Simulator::Stop (Seconds (duration + 0.1));
/*
  wimax.EnablePcap ("wimax-simple-ss0", ssNodes.Get (0)->GetId (), ss[0]->GetIfIndex ());
  wimax.EnablePcap ("wimax-simple-ss1", ssNodes.Get (1)->GetId (), ss[1]->GetIfIndex ());
  wimax.EnablePcap ("wimax-simple-bs0", bsNodes.Get (0)->GetId (), bs->GetIfIndex ());
*/
  IpcsClassifierRecord DlClassifierUgs (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces.GetAddress (0),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  ServiceFlow DlServiceFlowUgs = wimax.CreateServiceFlow (ServiceFlow::SF_DIRECTION_DOWN,
                                                          ServiceFlow::SF_TYPE_RTPS,
                                                          DlClassifierUgs);
  ss[0]->AddServiceFlow (DlServiceFlowUgs);

  for(int i=1;i<mxSS;i++){
	  IpcsClassifierRecord UlClassifierUgs (SSinterfaces.GetAddress (i),
	                                          Ipv4Mask ("255.255.255.255"),
	                                          Ipv4Address ("0.0.0.0"),
	                                          Ipv4Mask ("0.0.0.0"),
	                                          0,
	                                          65000,
	                                          100,
	                                          100,
	                                          17,
	                                          1);
	    ServiceFlow UlWimaxServiceFlowUgs = wimax.CreateServiceFlow (ServiceFlow::SF_DIRECTION_UP,
	                                                            ServiceFlow::SF_TYPE_RTPS,
	                                                            UlClassifierUgs);
	    ss[i]->AddServiceFlow (UlWimaxServiceFlowUgs);
  }


  AnimationInterface anim ("network_area.xml"); // Mandatory
  uint32_t ssResource = anim.AddResource("/home/sayefsakin/Documents/ns-allinone-3.21/ns-3.21/ns3/ss.png");
  uint32_t bsResource = anim.AddResource("/home/sayefsakin/Documents/ns-allinone-3.21/ns-3.21/ns3/bs.png");
  uint32_t puTxResource = anim.AddResource("/home/sayefsakin/Documents/ns-allinone-3.21/ns-3.21/ns3/putx.png");
  uint32_t puRxResource = anim.AddResource("/home/sayefsakin/Documents/ns-allinone-3.21/ns-3.21/ns3/purx.png");
//  uint32_t borderResource = anim.AddResource("/home/sayefsakin/Documents/ns-allinone-3.21/ns-3.21/ns3/border.png");
  double ssNodeSize = 50.0, bsNodeSize = 75.0;
    for (uint32_t i = 0; i < ssNodes.GetN (); ++i)
      {
    	anim.UpdateNodeSize(ssNodes.Get(i)->GetId(), ssNodeSize, ssNodeSize);
    	anim.UpdateNodeImage(ssNodes.Get(i)->GetId(), ssResource);
        anim.UpdateNodeDescription (ssNodes.Get (i), "CPE"); // Optional
//        anim.UpdateNodeColor (ssNodes.Get (i), 255, 0, 0); // Optional
      }
    for (uint32_t i = 0; i < bsNodes.GetN (); ++i)
      {
    	anim.UpdateNodeSize(bsNodes.Get(i)->GetId(), bsNodeSize, bsNodeSize);
    	anim.UpdateNodeImage(bsNodes.Get(i)->GetId(), bsResource);
        anim.UpdateNodeDescription (bsNodes.Get (i), "BS");
      }
    for (uint32_t i = 0; i < puTxNodes.GetN (); ++i)
          {
        anim.UpdateNodeSize(puTxNodes.Get(i)->GetId(), ssNodeSize, ssNodeSize);
		anim.UpdateNodeImage(puTxNodes.Get(i)->GetId(), puTxResource);
		anim.UpdateNodeDescription (puTxNodes.Get (i), "PUTx");
      }
    for (uint32_t i = 0; i < puRxNodes.GetN (); ++i)
              {
            anim.UpdateNodeSize(puRxNodes.Get(i)->GetId(), ssNodeSize, ssNodeSize);
    		anim.UpdateNodeImage(puRxNodes.Get(i)->GetId(), puRxResource);
    		anim.UpdateNodeDescription (puRxNodes.Get (i), "PURx");
          }
//
//    anim.EnablePacketMetadata (); // Optional
//    anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (10)); // Optional
//    anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25)); //Optional
//    anim.EnableWifiMacCounters (Seconds (0), Seconds (10)); //Optional
//    anim.EnableWifiPhyCounters (Seconds (0), Seconds (10)); //Optional

  NS_LOG_INFO ("Starting simulation.....");
  Simulator::Run ();
//  Simulator::Schedule (Seconds(1), &BSTOSSMessage, this, bs[0], ss[0]);
//  	for(int i=0;i<mxBS;i++){
//  	  bs[i]->GetPhy()->SetSimplex(bs[i]->GetChannel(0));
//  		bs[i]->GetPhy()->SetState(WimaxPhy::PHY_STATE_IDLE);
//  	}
//  	for(int i=0;i<mxSS;i++){
//  		ss[i]->GetPhy()->SetSimplex(ss[i]->GetChannel(0));
//  		ss[i]->GetPhy()->SetState(WimaxPhy::PHY_STATE_IDLE);
//  	}






  for(int i=0;i<mxSS;i++){
	  NS_LOG_INFO( "Channel Bandwidth: " << i << ": " << ss[i]->GetPhy()->GetChannelBandwidth());
	  NS_LOG_INFO( "Channel TxFrequency: " << i << ": " << ss[i]->GetPhy()->GetTxFrequency());
	  NS_LOG_INFO( "Channel RxFrequency: " << i << ": " << ss[i]->GetPhy()->GetRxFrequency());
	  NS_LOG_INFO( "Channel Frequency: " << i << ": " << ss[i]->GetPhy()->GetFrequency());
  }
  for(int i=0;i<mxBS;i++){
	  NS_LOG_INFO( "Channel Bandwidth: " << bs[i]->GetPhy()->GetChannelBandwidth());
	  NS_LOG_INFO( "Channel TxFrequency: " << bs[i]->GetPhy()->GetTxFrequency());
	  NS_LOG_INFO( "Channel RxFrequency: " << bs[i]->GetPhy()->GetRxFrequency());
	  NS_LOG_INFO( "Channel Frequency: " << bs[i]->GetPhy()->GetFrequency());

	  NS_LOG_INFO( "Transmit Power: " << bs[i]->GetPhy()->GetTxPower());
	  NS_LOG_INFO( "Number of SSs: " << bs[i]->GetSSManager()->GetNSSs());
	  NS_LOG_INFO( "Number of SSs: " << bs[i]->GetSSManager()->GetNRegisteredSSs());
  }

  for(int i=0;i<mxSS;i++){
	  ss[i] = 0;
  }
  for(int i=0;i<mxBS;i++){
	  bs[i] = 0;
  }

  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");

  return 0;
}



