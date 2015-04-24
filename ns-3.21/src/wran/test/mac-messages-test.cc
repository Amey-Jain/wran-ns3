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
#include "ns3/wran-ipcs-classifier-record.h"
#include "ns3/wran-service-flow.h"
#include <iostream>

using namespace ns3;
/*
 * Test the DSA request message.
 */
class DsaRequestTestCase : public TestCase
{
public:
  DsaRequestTestCase ();
  virtual ~DsaRequestTestCase ();

private:
  virtual void DoRun (void);

};

DsaRequestTestCase::DsaRequestTestCase ()
  : TestCase ("Test the DSA request messages")
{
}

DsaRequestTestCase::~DsaRequestTestCase ()
{
}

void
DsaRequestTestCase::DoRun (void)
{
  WranIpcsClassifierRecord classifier = WranIpcsClassifierRecord ();
  CsParameters csParam (CsParameters::ADD, classifier);
  WranServiceFlow sf = WranServiceFlow (WranServiceFlow::SF_DIRECTION_DOWN);

  sf.SetSfid (100);
  sf.SetConvergenceSublayerParam (csParam);
  sf.SetCsSpecification (WranServiceFlow::IPV4);
  sf.SetServiceSchedulingType (WranServiceFlow::SF_TYPE_UGS);
  sf.SetMaxSustainedTrafficRate (1000000);
  sf.SetMinReservedTrafficRate (1000000);
  sf.SetMinTolerableTrafficRate (1000000);
  sf.SetMaximumLatency (10);
  sf.SetMaxTrafficBurst (1000);
  sf.SetTrafficPriority (1);

  DsaReq dsaReq (sf);
  Ptr<Packet> packet = Create<Packet> ();
  packet->AddHeader (dsaReq);

  DsaReq dsaReqRecv;
  packet->RemoveHeader (dsaReqRecv);

  WranServiceFlow sfRecv = dsaReqRecv.GetWranServiceFlow ();

  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetDirection (), WranServiceFlow::SF_DIRECTION_DOWN, "The sfRecv had the wrong direction.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetSfid (), 100, "The sfRecv had the wrong sfid.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetCsSpecification (), WranServiceFlow::IPV4, "The sfRecv had the wrong cs specification.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetServiceSchedulingType (), WranServiceFlow::SF_TYPE_UGS, "The sfRecv had the wrong service scheduling type.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetMaxSustainedTrafficRate (), 1000000, "The sfRecv had the wrong maximum sustained traffic rate.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetMinReservedTrafficRate (), 1000000, "The sfRecv had the wrong minimum reserved traffic rate.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetMinTolerableTrafficRate (), 1000000, "The sfRecv had the wrong minimum tolerable traffic rate.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetMaximumLatency (), 10, "The sfRecv had the wrong maximum latency.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetMaxTrafficBurst (), 1000, "The sfRecv had the wrong maximum traffic burst.");
  NS_TEST_ASSERT_MSG_EQ (sfRecv.GetTrafficPriority (), 1, "The sfRecv had the wrong traffic priority.");
}

// ==============================================================================
class Ns3WranMacMessagesTestSuite : public TestSuite
{
public:
  Ns3WranMacMessagesTestSuite ();
};

Ns3WranMacMessagesTestSuite::Ns3WranMacMessagesTestSuite ()
  : TestSuite ("wran-mac-messages", UNIT)
{
  AddTestCase (new DsaRequestTestCase, TestCase::QUICK);
}

static Ns3WranMacMessagesTestSuite ns3WranMacMessagesTestSuite;
