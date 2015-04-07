/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2009 INRIA, UDcast
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
 *         Sayef Azad Sakin <sayefsakin[at]gmail[dot]com>
 *                               
 *
 */
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/test.h"
#include "ns3/uinteger.h"
#include "ns3/inet-socket-address.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv4-header.h"
#include "ns3/packet-sink-helper.h"
#include "ns3/udp-client-server-helper.h"
#include "ns3/udp-header.h"
#include "ns3/simulator.h"
#include "ns3/wran-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/global-route-manager.h"
#include "ns3/wran-tlv.h"
#include "ns3/ipcs-classifier-record.h"
#include "ns3/service-flow.h"
#include <iostream>

using namespace ns3;

/*
 * Test the service flow creation.
 */
class Ns3WranSfCreationTestCase : public TestCase
{
public:
  Ns3WranSfCreationTestCase ();
  virtual ~Ns3WranSfCreationTestCase ();

private:
  virtual void DoRun (void);

};

Ns3WranSfCreationTestCase::Ns3WranSfCreationTestCase ()
  : TestCase ("Test the service flow tlv implementation.")
{
}

Ns3WranSfCreationTestCase::~Ns3WranSfCreationTestCase ()
{
}

void
Ns3WranSfCreationTestCase::DoRun (void)
{

  // default values
  int duration = 2;
  WranHelper::SchedulerType scheduler = WranHelper::SCHED_TYPE_SIMPLE;

  NodeContainer ssNodes;
  NodeContainer bsNodes;

  ssNodes.Create (1);
  bsNodes.Create (1);

  WranHelper wran;

  NetDeviceContainer ssDevs, bsDevs;

  ssDevs = wran.Install (ssNodes,
                          WranHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WranHelper::SIMPLE_PHY_TYPE_OFDM,
                          scheduler);
  bsDevs = wran.Install (bsNodes, WranHelper::DEVICE_TYPE_BASE_STATION, WranHelper::SIMPLE_PHY_TYPE_OFDM, scheduler);

  ssDevs.Get (0)->GetObject<SubscriberStationNetDevice> ()->SetModulationType (WranPhy::MODULATION_TYPE_QAM16_12);
  bsDevs.Get (0)->GetObject<BaseStationNetDevice> ();

  InternetStackHelper stack;
  stack.Install (bsNodes);
  stack.Install (ssNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer SSinterfaces = address.Assign (ssDevs);
  Ipv4InterfaceContainer BSinterface = address.Assign (bsDevs);

  // Create one UGS Downlink service flow between the ss and the bs
  ServiceFlow * DlServiceFlowUgs = new ServiceFlow (ServiceFlow::SF_DIRECTION_DOWN);
  IpcsClassifierRecord DlClassifierUgs (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        3000,
                                        3000,
                                        0,
                                        35000,
                                        17,
                                        1);
  CsParameters DlcsParam (CsParameters::ADD, DlClassifierUgs);
  DlServiceFlowUgs->SetConvergenceSublayerParam (DlcsParam);
  DlServiceFlowUgs->SetCsSpecification (ServiceFlow::IPV4);
  DlServiceFlowUgs->SetServiceSchedulingType (ServiceFlow::SF_TYPE_UGS);
  DlServiceFlowUgs->SetMaxSustainedTrafficRate (1000000);
  DlServiceFlowUgs->SetMinReservedTrafficRate (1000000);
  DlServiceFlowUgs->SetMinTolerableTrafficRate (1000000);
  DlServiceFlowUgs->SetMaximumLatency (10);
  DlServiceFlowUgs->SetMaxTrafficBurst (1000);
  DlServiceFlowUgs->SetTrafficPriority (1);

  // Create one UGS Uplink service flow between the ss and the bs
  ServiceFlow * UlServiceFlowUgs = new ServiceFlow (ServiceFlow::SF_DIRECTION_UP);
  IpcsClassifierRecord UlClassifierUgs (Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        Ipv4Address ("0.0.0.0"),
                                        Ipv4Mask ("0.0.0.0"),
                                        0,
                                        35000,
                                        3000,
                                        3000,
                                        17,
                                        1);
  CsParameters UlcsParam (CsParameters::ADD, UlClassifierUgs);
  UlServiceFlowUgs->SetConvergenceSublayerParam (UlcsParam);
  UlServiceFlowUgs->SetCsSpecification (ServiceFlow::IPV4);
  UlServiceFlowUgs->SetServiceSchedulingType (ServiceFlow::SF_TYPE_UGS);
  UlServiceFlowUgs->SetMaxSustainedTrafficRate (1000000);
  UlServiceFlowUgs->SetMinReservedTrafficRate (1000000);
  UlServiceFlowUgs->SetMinTolerableTrafficRate (1000000);
  UlServiceFlowUgs->SetMaximumLatency (10);
  UlServiceFlowUgs->SetMaxTrafficBurst (1000);
  UlServiceFlowUgs->SetTrafficPriority (1);
  ssDevs.Get (0)->GetObject<SubscriberStationNetDevice> ()->AddServiceFlow (DlServiceFlowUgs);
  ssDevs.Get (0)->GetObject<SubscriberStationNetDevice> ()->AddServiceFlow (UlServiceFlowUgs);

  Simulator::Stop (Seconds (duration));
  Simulator::Run ();
  Simulator::Destroy ();
}

// ==============================================================================
class Ns3WranServiceFlowTestSuite : public TestSuite
{
public:
  Ns3WranServiceFlowTestSuite ();
};

Ns3WranServiceFlowTestSuite::Ns3WranServiceFlowTestSuite ()
  : TestSuite ("wran-service-flow", UNIT)
{
  AddTestCase (new Ns3WranSfCreationTestCase, TestCase::QUICK);
}

static Ns3WranServiceFlowTestSuite ns3WranServiceFlowTestSuite;
