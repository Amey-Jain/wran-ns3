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
#include "ns3/wran-module.h"
#include "ns3/internet-module.h"
#include "ns3/global-route-manager.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/service-flow.h"
#include "ns3/netanim-module.h"
#include "ns3/udp-echo-helper.h"
#include <iostream>
#include <vector>
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("WranSimpleExample");

using namespace ns3;

int main (int argc, char *argv[])
{
  bool verbose = false;

  int duration = 120, schedType = 0, mxSS = 1, mxBS = 1;
  WranHelper::SchedulerType scheduler = WranHelper::SCHED_TYPE_SIMPLE;

  CommandLine cmd;
  cmd.AddValue ("scheduler", "type of scheduler to use with the network devices", schedType);
  cmd.AddValue ("duration", "duration of the simulation in seconds", duration);
  cmd.AddValue ("verbose", "turn on all WranNetDevice log components", verbose);
  cmd.Parse (argc, argv);

//  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
//  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
  LogComponentEnable("WranSimpleExample", LOG_LEVEL_INFO);
//  LogComponentEnable("simpleOfdmWranChannel", LOG_LEVEL_INFO);
//  LogComponentEnable("SimpleOfdmWranPhy", LOG_LEVEL_INFO);
//  LogComponentEnable("WranPhy", LOG_LEVEL_INFO);
  LogComponentEnable("WranBaseStationNetDevice", LOG_LEVEL_INFO);
//  LogComponentEnable("WranNetDevice", LOG_LEVEL_INFO);
  LogComponentEnable("WranSubscriberStationNetDevice", LOG_LEVEL_INFO);
//  LogComponentEnable("PropagationLossModel", LOG_LEVEL_INFO);
  LogComponentEnable("CogSpectrumSensing", LOG_LEVEL_INFO);
  LogComponentEnable("CogSpectrumManager", LOG_LEVEL_INFO);
//  LogComponentEnable("CogPUModel", LOG_LEVEL_INFO);




  switch (schedType)
    {
    case 0:
      scheduler = WranHelper::SCHED_TYPE_SIMPLE;
      break;
    case 1:
      scheduler = WranHelper::SCHED_TYPE_MBQOS;
      break;
    case 2:
      scheduler = WranHelper::SCHED_TYPE_RTPS;
      break;
    default:
      scheduler = WranHelper::SCHED_TYPE_SIMPLE;
    }

  NodeContainer ssNodes;
  NodeContainer bsNodes;

  ssNodes.Create (mxSS);
  bsNodes.Create (mxBS);

  WranHelper wran;

  // Read PU file
    Ptr<PUModel> puModel = CreateObject<PUModel>();
    std::string map_file = "map_PUs_multiple.txt";
    puModel->SetPuMapFile((char*)map_file.c_str());
    //Create repository
    Ptr<Repository> repo = CreateObject<Repository>();

  NetDeviceContainer ssDevs, bsDevs;

  ssDevs = wran.Install (repo, puModel, ssNodes,
                          WranHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WranHelper::SIMPLE_PHY_TYPE_OFDM,
                          scheduler);
  bsDevs = wran.Install (repo, puModel, bsNodes,
		  	  	  	  	  WranHelper::DEVICE_TYPE_BASE_STATION,
		  	  	  	  	  WranHelper::SIMPLE_PHY_TYPE_OFDM,
		  	  	  	  	  scheduler);
//  wran.SetPropagationLossModel(SimpleOfdmWranChannel::COST231_PROPAGATION);
  wran.SetPropagationLossModel(SimpleOfdmWranChannel::FRIIS_PROPAGATION);
// wran.SetPropagationLossModel(SimpleOfdmWranChannel::ITU_NLOS_ROOFTOP_PROPAGATION);
//  wran.EnableAscii ("bs-devices", bsDevs);
//  wran.EnableAscii ("ss-devices", ssDevs);

  Ptr<WranSubscriberStationNetDevice> ss[mxSS];

  for (int i = 0; i < mxSS; i++)
    {
      ss[i] = ssDevs.Get (i)->GetObject<WranSubscriberStationNetDevice> ();
      ss[i]->SetModulationType (WranPhy::MODULATION_TYPE_QAM16_12); // its ok for wran
    }

  Ptr<WranBaseStationNetDevice> bs[mxBS];

  for(int i=0;i<mxBS;i++){
	  bs[i] = bsDevs.Get (i)->GetObject<WranBaseStationNetDevice> ();
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
  double km = 1000.0;
  double rowNeighbourBSOverlappingRegion = 10.0 * km;
  double bsTransmissionRange = 30.0 * km;
  double bsToBsDistanceX = 2.0 * (bsTransmissionRange - rowNeighbourBSOverlappingRegion);
  double bsToBsDistanceY = bsToBsDistanceX * sin(45.0);
  double startCoordinate = 15.0 * km;

  Ptr<ListPositionAllocator> positionAllocBS = CreateObject<ListPositionAllocator> ();
        positionAllocBS->Add (Vector (startCoordinate + (bsToBsDistanceX / 2.0), 					startCoordinate, 							10.0));
        positionAllocBS->Add (Vector (startCoordinate + (bsToBsDistanceX / 2.0) + bsToBsDistanceX, 	startCoordinate, 							10.0));
        positionAllocBS->Add (Vector (startCoordinate, 												startCoordinate + bsToBsDistanceY, 			10.0));
        positionAllocBS->Add (Vector (startCoordinate + bsToBsDistanceX, 							startCoordinate + bsToBsDistanceY, 			10.0));
        positionAllocBS->Add (Vector (startCoordinate + (2.0 * bsToBsDistanceX),					startCoordinate + bsToBsDistanceY, 			10.0));
//        positionAllocBS->Add (Vector (startCoordinate + (bsToBsDistanceX / 2.0), 					startCoordinate + (2.0 * bsToBsDistanceY), 	10.0));
//        positionAllocBS->Add (Vector (startCoordinate + (bsToBsDistanceX / 2.0) + bsToBsDistanceX, 	startCoordinate + (2.0 * bsToBsDistanceY), 	10.0));
        bsMobility.SetPositionAllocator (positionAllocBS);

      bsMobility.Install (bsNodes);

      std::stringstream sstreamX, sstreamY;
//      sstreamX << (startCoordinate + (bsToBsDistanceX / 2.0));
//      sstreamY << (startCoordinate + (2.0 / 3.0 * bsToBsDistanceY));
//  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
//                                   "X", StringValue (sstreamX.str()), // center-x
//                                   "Y", StringValue (sstreamY.str()), // center-y
//                                   "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=40000]")); // position radius

      std::string unRanVar("ns3::UniformRandomVariable[Min=");
      double maxAreaX = startCoordinate + (2.0 * bsToBsDistanceX);
      double maxAreaY = (/*2.0 */ startCoordinate) + bsToBsDistanceY;
      sstreamX  << unRanVar << startCoordinate << "|Max=" << maxAreaX << "]";
      sstreamY  << unRanVar << startCoordinate << "|Max=" << maxAreaY << "]";
      mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
                                           "X", StringValue (sstreamX.str()),
                                           "Y", StringValue (sstreamY.str()));
//  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
//                                     "X", StringValue ("ns3::UniformRandomVariable[Min=0|Max=5500]"),
//                                     "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=100]"));
//  Ptr<ListPositionAllocator> positionAllocSS = CreateObject<ListPositionAllocator> ();
//  positionAllocSS->Add (Vector (startCoordinate + bsToBsDistanceX, startCoordinate, 10.0));
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

//    wran.GetSpectrumManager()->SetRepository(repo);
//    wran.GetSpectrumManager()->Start();
  if (verbose)
    {
      wran.EnableLogComponents ();  // Turn on all wran logging
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
  wran.EnablePcap ("wran-simple-ss0", ssNodes.Get (0)->GetId (), ss[0]->GetIfIndex ());
  wran.EnablePcap ("wran-simple-ss1", ssNodes.Get (1)->GetId (), ss[1]->GetIfIndex ());
  wran.EnablePcap ("wran-simple-bs0", bsNodes.Get (0)->GetId (), bs->GetIfIndex ());
*/
//  WranIpcsClassifierRecord DlClassifierUgs (Ipv4Address ("0.0.0.0"),
//                                        Ipv4Mask ("0.0.0.0"),
//                                        SSinterfaces.GetAddress (0),
//                                        Ipv4Mask ("255.255.255.255"),
//                                        0,
//                                        65000,
//                                        100,
//                                        100,
//                                        17,
//                                        1);
//  WranServiceFlow DlWranServiceFlowUgs = wran.CreateWranServiceFlow (WranServiceFlow::SF_DIRECTION_DOWN,
//                                                          WranServiceFlow::SF_TYPE_RTPS,
//                                                          DlClassifierUgs);
//  ss[0]->AddWranServiceFlow (DlWranServiceFlowUgs);
//
//  for(int i=1;i<mxSS;i++){
//	  WranIpcsClassifierRecord UlClassifierUgs (SSinterfaces.GetAddress (i),
//	                                          Ipv4Mask ("255.255.255.255"),
//	                                          Ipv4Address ("0.0.0.0"),
//	                                          Ipv4Mask ("0.0.0.0"),
//	                                          0,
//	                                          65000,
//	                                          100,
//	                                          100,
//	                                          17,
//	                                          1);
//	    WranServiceFlow UlWranServiceFlowUgs = wran.CreateWranServiceFlow (WranServiceFlow::SF_DIRECTION_UP,
//	                                                            WranServiceFlow::SF_TYPE_RTPS,
//	                                                            UlClassifierUgs);
//	    ss[i]->AddWranServiceFlow (UlWranServiceFlowUgs);
//  }


  AnimationInterface anim ("wireless-animation.xml"); // Mandatory
  uint32_t ssResource = anim.AddResource("/home/sayefsakin/Documents/ns-allinone-3.21/ns-3.21/ns3/ss.png");
  uint32_t bsResource = anim.AddResource("/home/sayefsakin/Documents/ns-allinone-3.21/ns-3.21/ns3/bs.png");
    for (uint32_t i = 0; i < ssNodes.GetN (); ++i)
      {
    	anim.UpdateNodeImage(ssNodes.Get(i)->GetId(), ssResource);
        anim.UpdateNodeDescription (ssNodes.Get (i), "CPE"); // Optional
//        anim.UpdateNodeColor (ssNodes.Get (i), 255, 0, 0); // Optional
      }
    for (uint32_t i = 0; i < bsNodes.GetN (); ++i)
      {
    	anim.UpdateNodeImage(bsNodes.Get(i)->GetId(), bsResource);
        anim.UpdateNodeDescription (bsNodes.Get (i), "BS"); // Optional
//        anim.UpdateNodeColor (bsNodes.Get (i), 0, 255, 0); // Optional
      }

    anim.EnablePacketMetadata (); // Optional
//    anim.EnableIpv4L3ProtocolCounters (Seconds (0), Seconds (10)); // Optional
//    anim.EnableIpv4RouteTracking ("routingtable-wireless.xml", Seconds (0), Seconds (5), Seconds (0.25)); //Optional
//    anim.EnableWifiMacCounters (Seconds (0), Seconds (10)); //Optional
//    anim.EnableWifiPhyCounters (Seconds (0), Seconds (10)); //Optional

    Ptr<MobilityModel> baseStationMobility = 0;
    Ptr<MobilityModel> subscriberStationMobility = 0;
    double distance = 0.0, mnDistance = maxAreaY;
    int chBS = -1, i, j;
	for(j = 0; j < mxSS; ++j){
		subscriberStationMobility = ss[j]->GetNode()->GetObject<MobilityModel> ();
		std::stringstream cpeNameStream;
		cpeNameStream << "CPE";

		distance = 0.0, mnDistance = maxAreaY;
		for(i = 0; i < mxBS; ++i){
		    baseStationMobility = bs[i]->GetNode()->GetObject<MobilityModel> ();
    		distance = baseStationMobility->GetDistanceFrom (subscriberStationMobility);

    		if(distance < mnDistance) {
    			chBS = i;
    			mnDistance = distance;
    		}

//    		if(distance <= MAX_TRANSMISSION_RANGE) {
//    			break;
//    		}
    	}
		if(i>=mxBS){
			i = chBS;
		}
		cpeNameStream << i;
		ss[j]->SetMyBSMAC(bs[i]->GetMacAddress());
		bs[i]->GetWranSSManager()->CreateWranSSRecord(ss[j]->GetMacAddress());
		anim.UpdateNodeDescription (ss[j]->GetNode(), cpeNameStream.str());
    }


  NS_LOG_INFO ("Starting simulation.....");
  Simulator::Run ();
//  Simulator::Schedule (Seconds(1), &BSTOSSMessage, this, bs[0], ss[0]);
//  	for(i=0;i<mxBS;i++){
//  	  bs[i]->GetPhy()->SetSimplex(bs[i]->GetChannel(0));
//  		bs[i]->GetPhy()->SetState(WranPhy::PHY_STATE_IDLE);
//  	}
//  	for(i=0;i<mxSS;i++){
//  		ss[i]->GetPhy()->SetSimplex(ss[i]->GetChannel(0));
//  		ss[i]->GetPhy()->SetState(WranPhy::PHY_STATE_IDLE);
//  	}






//  for(int i=0;i<mxSS;i++){
//	  NS_LOG_INFO( "Channel Bandwidth: " << i << ": " << ss[i]->GetPhy()->GetChannelBandwidth());
//	  NS_LOG_INFO( "Channel TxFrequency: " << i << ": " << ss[i]->GetPhy()->GetTxFrequency());
//	  NS_LOG_INFO( "Channel RxFrequency: " << i << ": " << ss[i]->GetPhy()->GetRxFrequency());
//	  NS_LOG_INFO( "Channel Frequency: " << i << ": " << ss[i]->GetPhy()->GetFrequency());
//  }

  	for(i=0;i<mxBS;i++){
//	  NS_LOG_INFO( "Channel Bandwidth: " << bs[i]->GetPhy()->GetChannelBandwidth());
//	  NS_LOG_INFO( "Channel TxFrequency: " << bs[i]->GetPhy()->GetTxFrequency());
//	  NS_LOG_INFO( "Channel RxFrequency: " << bs[i]->GetPhy()->GetRxFrequency());
//	  NS_LOG_INFO( "Channel Frequency: " << bs[i]->GetPhy()->GetFrequency());
//
//	  NS_LOG_INFO( "Transmit Power: " << bs[i]->GetPhy()->GetTxPower());
	  NS_LOG_INFO( "Number of SSs: " << bs[i]->GetWranSSManager()->GetNSSs());
//	  NS_LOG_INFO( "Number of Registered SSs: " << bs[i]->GetWranSSManager()->GetNRegisteredSSs());
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



