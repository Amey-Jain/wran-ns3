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
#include "ns3/wran-ipcs-classifier-record.h"
#include "ns3/wran-service-flow.h"
#include <iostream>
#include <vector>

NS_LOG_COMPONENT_DEFINE ("WranSimpleExample");

using namespace ns3;

int main (int argc, char *argv[])
{
  bool verbose = false;

  int duration = 10, schedType = 0, mxSS = 3, mxBS = 1;
  WranHelper::SchedulerType scheduler = WranHelper::SCHED_TYPE_SIMPLE;

  CommandLine cmd;
  cmd.AddValue ("scheduler", "type of scheduler to use with the network devices", schedType);
  cmd.AddValue ("duration", "duration of the simulation in seconds", duration);
  cmd.AddValue ("verbose", "turn on all WranNetDevice log components", verbose);
  cmd.Parse (argc, argv);
  LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
  LogComponentEnable("WranSimpleExample", LOG_LEVEL_INFO);
//  LogComponentEnable("WranPhy", LOG_LEVEL_INFO);
//  LogComponentEnable("simpleOfdmWranChannel", LOG_LEVEL_INFO);
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

  NetDeviceContainer ssDevs, bsDevs;

  ssDevs = wran.Install (ssNodes,
                          WranHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WranHelper::SIMPLE_PHY_TYPE_OFDM,
                          scheduler);
  bsDevs = wran.Install (bsNodes,
		  	  	  	  	  WranHelper::DEVICE_TYPE_BASE_STATION,
		  	  	  	  	  WranHelper::SIMPLE_PHY_TYPE_OFDM,
		  	  	  	  	  scheduler);

//  wran.EnableAscii ("bs-devices", bsDevs);
//  wran.EnableAscii ("ss-devices", ssDevs);

  Ptr<WranSubscriberStationNetDevice> ss[mxSS];

  for (int i = 0; i < mxSS; i++)
    {
      ss[i] = ssDevs.Get (i)->GetObject<WranSubscriberStationNetDevice> ();
      ss[i]->SetModulationType (WranPhy::MODULATION_TYPE_QAM16_12); // its ok for wran
    }

  Ptr<WranBaseStationNetDevice> bs[2];

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
//                                   "MinY", DoubleValue (0.0),
//                                   "DeltaX", DoubleValue (500.0),
//                                   "DeltaY", DoubleValue (10.0),
//                                   "GridWidth", UintegerValue (3),
//                                   "LayoutType", StringValue ("RowFirst"));
//
//  bsMobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
//      bsMobility.Install (bsNodes);
  mobility.SetPositionAllocator ("ns3::RandomDiscPositionAllocator",
                                   "X", StringValue ("0.0"), // center-x
                                   "Y", StringValue ("0.0"), // center-y
                                   "Rho", StringValue ("ns3::UniformRandomVariable[Min=0|Max=60]"), //position angle in gradient
                                   "Theta",StringValue ("ns3::UniformRandomVariable[Min=0|Max=100]")); // position radius

//  mobility.SetPositionAllocator ("ns3::RandomRectanglePositionAllocator",
//                                     "X", StringValue ("ns3::UniformRandomVariable[Min=5|Max=495]"),
//                                     "Y", StringValue ("ns3::UniformRandomVariable[Min=0|Max=100]"));



  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
    mobility.Install (ssNodes);

  if (verbose)
    {
      wran.EnableLogComponents ();  // Turn on all wran logging
    }
  /*------------------------------*/
  UdpServerHelper udpServer;
  ApplicationContainer serverApps;
  UdpClientHelper udpClient;
  ApplicationContainer clientApps[mxSS - 1];

  udpServer = UdpServerHelper (100);

  serverApps = udpServer.Install (ssNodes.Get (0));
  serverApps.Start (Seconds (1));
  serverApps.Stop (Seconds (duration));

  udpClient = UdpClientHelper (SSinterfaces.GetAddress (0), 100);
  udpClient.SetAttribute ("MaxPackets", UintegerValue (500));
  udpClient.SetAttribute ("Interval", TimeValue (Seconds (1)));
  udpClient.SetAttribute ("PacketSize", UintegerValue (1024));

  for(int i=0;i<mxSS-1;i++){
	  clientApps[i] = udpClient.Install (ssNodes.Get (i+1));
	    clientApps[i].Start (Seconds (3));
	    clientApps[i].Stop (Seconds (5));
  }

  Simulator::Stop (Seconds (duration + 0.1));
/*
  wran.EnablePcap ("wran-simple-ss0", ssNodes.Get (0)->GetId (), ss[0]->GetIfIndex ());
  wran.EnablePcap ("wran-simple-ss1", ssNodes.Get (1)->GetId (), ss[1]->GetIfIndex ());
  wran.EnablePcap ("wran-simple-bs0", bsNodes.Get (0)->GetId (), bs->GetIfIndex ());
*/
  WranIpcsClassifierRecord DlClassifierUgs (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        SSinterfaces.GetAddress (0),
                                        Ipv4Mask ("255.255.255.255"),
                                        0,
                                        65000,
                                        100,
                                        100,
                                        17,
                                        1);
  WranServiceFlow DlWranServiceFlowUgs = wran.CreateWranServiceFlow (WranServiceFlow::SF_DIRECTION_DOWN,
                                                          WranServiceFlow::SF_TYPE_RTPS,
                                                          DlClassifierUgs);
  ss[0]->AddWranServiceFlow (DlWranServiceFlowUgs);

  for(int i=1;i<mxSS;i++){
	  WranIpcsClassifierRecord UlClassifierUgs (SSinterfaces.GetAddress (i),
	                                          Ipv4Mask ("255.255.255.255"),
	                                          Ipv4Address ("0.0.0.0"),
	                                          Ipv4Mask ("0.0.0.0"),
	                                          0,
	                                          65000,
	                                          100,
	                                          100,
	                                          17,
	                                          1);
	    WranServiceFlow UlWranServiceFlowUgs = wran.CreateWranServiceFlow (WranServiceFlow::SF_DIRECTION_UP,
	                                                            WranServiceFlow::SF_TYPE_RTPS,
	                                                            UlClassifierUgs);
	    ss[i]->AddWranServiceFlow (UlWranServiceFlowUgs);
  }

  NS_LOG_INFO ("Starting simulation.....");
  Simulator::Run ();

  for(int i=0;i<mxSS;i++){
	  NS_LOG_INFO( "Channel Bandwidth: " << i << ": " << ss[i]->GetPhy()->GetChannelBandwidth());
	  NS_LOG_INFO( "Channel TxFrequency: " << i << ": " << ss[i]->GetPhy()->GetTxFrequency());
	  NS_LOG_INFO( "Channel RxFrequency: " << i << ": " << ss[i]->GetPhy()->GetRxFrequency());
  }
  for(int i=0;i<mxBS;i++){
	  NS_LOG_INFO( "Channel Bandwidth: " << bs[i]->GetPhy()->GetChannelBandwidth());
	  NS_LOG_INFO( "Channel TxFrequency: " << bs[i]->GetPhy()->GetTxFrequency());
	  NS_LOG_INFO( "Channel RxFrequency: " << bs[i]->GetPhy()->GetRxFrequency());

	  NS_LOG_INFO( "Transmit Power: " << bs[i]->GetPhy()->GetTxPower());
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
