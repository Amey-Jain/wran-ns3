/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2015 Green Network Research Group
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
 *                               
 */
#include "ns3/log.h"
#include "ns3/abort.h"
#include "ns3/test.h"
#include "ns3/config.h"
#include "ns3/string.h"
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
#include <iostream>
#include "ns3/global-route-manager.h"

using namespace ns3;

/*
 * Test the network entry procedure.
 * Create a network with a BS and 10 SS and check that all the SS perform the
 * network entry correctly
 *
 */
class Ns3WranNetworkEntryTestCase : public TestCase
{
public:
  Ns3WranNetworkEntryTestCase ();
  virtual ~Ns3WranNetworkEntryTestCase ();

private:
  virtual void DoRun (void);

};

Ns3WranNetworkEntryTestCase::Ns3WranNetworkEntryTestCase ()
  : TestCase ("Test the network entry procedure")
{
}

Ns3WranNetworkEntryTestCase::~Ns3WranNetworkEntryTestCase ()
{
}

void
Ns3WranNetworkEntryTestCase::DoRun (void)
{
  WranHelper::SchedulerType scheduler = WranHelper::SCHED_TYPE_SIMPLE;
  NodeContainer ssNodes;
  NodeContainer bsNodes;

  ssNodes.Create (10);
  bsNodes.Create (1);

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
  Simulator::Stop (Seconds (1));
  Simulator::Run ();
  for (int i = 0; i < 10; i++)
    {
      NS_TEST_EXPECT_MSG_EQ (ssDevs.Get (i)->GetObject<SubscriberStationNetDevice> ()->IsRegistered (),true,
                             "SS[" << i << "] IsNotRegistered");
    }
  Simulator::Destroy ();
}

/*
 * Test if the management connections are correctly setup.
 * Create a network with a BS and 10 SS and check that the management
 * connections are correctly setup for all SS
 *
 */

class Ns3WranManagementConnectionsTestCase : public TestCase
{
public:
  Ns3WranManagementConnectionsTestCase ();
  virtual ~Ns3WranManagementConnectionsTestCase ();

private:
  virtual void DoRun (void);

};

Ns3WranManagementConnectionsTestCase::Ns3WranManagementConnectionsTestCase ()
  : TestCase ("Test if the management connections are correctly setup")
{
}

Ns3WranManagementConnectionsTestCase::~Ns3WranManagementConnectionsTestCase ()
{
}

void
Ns3WranManagementConnectionsTestCase::DoRun (void)
{
  WranHelper::SchedulerType scheduler = WranHelper::SCHED_TYPE_SIMPLE;
  NodeContainer ssNodes;
  NodeContainer bsNodes;

  ssNodes.Create (10);
  bsNodes.Create (1);

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
  Simulator::Stop (Seconds (1));
  Simulator::Run ();
  for (int i = 0; i < 10; i++)
    {
      NS_TEST_EXPECT_MSG_EQ (ssDevs.Get (i)->GetObject<SubscriberStationNetDevice> ()->GetAreManagementConnectionsAllocated (),
                             true, "Management connections for SS[" << i << "] are not allocated");
    }
  Simulator::Destroy ();
}
class Ns3WranSSMacTestSuite : public TestSuite
{
public:
  Ns3WranSSMacTestSuite ();
};

Ns3WranSSMacTestSuite::Ns3WranSSMacTestSuite ()
  : TestSuite ("wran-ss-mac-layer", UNIT)
{
  AddTestCase (new Ns3WranNetworkEntryTestCase, TestCase::QUICK);
  AddTestCase (new Ns3WranManagementConnectionsTestCase, TestCase::QUICK);
}

static Ns3WranSSMacTestSuite ns3WranSSMacTestSuite;
