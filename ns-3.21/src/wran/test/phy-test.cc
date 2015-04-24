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
#include "ns3/simulator.h"
#include "ns3/wran-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/global-route-manager.h"
#include "ns3/snr-to-block-error-rate-manager.h"
#include <iostream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WranPhyTest");

/*
 * Configure a network with 3 SS and 1 BS
 * Install a SIMPLE OFDM PHY layer on all nodes and check that all SSs
 * could register with the BS
 *
 */

class Ns3WranSimpleOFDMTestCase : public TestCase
{
public:
  Ns3WranSimpleOFDMTestCase ();
  virtual ~Ns3WranSimpleOFDMTestCase ();

private:
  virtual void DoRun (void);
  bool DoRunOnce (double);

};

Ns3WranSimpleOFDMTestCase::Ns3WranSimpleOFDMTestCase ()
  : TestCase ("Test the Phy model with different frame durations")
{
}

Ns3WranSimpleOFDMTestCase::~Ns3WranSimpleOFDMTestCase ()
{
}

bool
Ns3WranSimpleOFDMTestCase::DoRunOnce (double FrameDuration)
{
  WranHelper::SchedulerType scheduler = WranHelper::SCHED_TYPE_SIMPLE;
  NodeContainer ssNodes;
  NodeContainer bsNodes;
  ssNodes.Create (3);
  bsNodes.Create (1);

  WranHelper wran;

  NetDeviceContainer ssDevs, bsDevs;

  ssDevs = wran.Install (ssNodes, WranHelper::DEVICE_TYPE_SUBSCRIBER_STATION,
                          WranHelper::SIMPLE_PHY_TYPE_OFDM, scheduler, FrameDuration);
  bsDevs = wran.Install (bsNodes, WranHelper::DEVICE_TYPE_BASE_STATION,
                          WranHelper::SIMPLE_PHY_TYPE_OFDM, scheduler, FrameDuration);

  Simulator::Stop (Seconds (1));
  Simulator::Run ();
  for (int i = 0; i < 3; i++)
    {
      if (ssDevs.Get (i)->GetObject<WranSubscriberStationNetDevice> ()->IsRegistered ()
          == false)
        {
          NS_LOG_DEBUG ("SS[" << i << "] not registered");
          return true; // Test fail because SS[i] is not registered
        }
    }
  Simulator::Destroy ();
  return (false); // Test was ok, all the SSs are registered

}

void
Ns3WranSimpleOFDMTestCase::DoRun (void)
{

  double
    frameDuratioTab[7] = { 0.0025, 0.004, 0.005, 0.008, 0.01, 0.0125, 0.02 };
  for (int i = 0; i < 7; i++)
    {
      NS_LOG_DEBUG ("Frame Duration = " << frameDuratioTab[i]);
      if (DoRunOnce (frameDuratioTab[i]) != false)
        {
          return;
        }
    }
}

/*
 * Test the SNr tom block error rate module
 */

class Ns3WranSNRtoBLERTestCase : public TestCase
{
public:
  Ns3WranSNRtoBLERTestCase ();
  virtual ~Ns3WranSNRtoBLERTestCase ();

private:
  virtual void DoRun (void);
  bool DoRunOnce (uint8_t);

};

Ns3WranSNRtoBLERTestCase::Ns3WranSNRtoBLERTestCase ()
  : TestCase ("Test the SNR to block error rate module")
{
}

Ns3WranSNRtoBLERTestCase::~Ns3WranSNRtoBLERTestCase ()
{
}

bool Ns3WranSNRtoBLERTestCase::DoRunOnce (uint8_t modulationType)
{

  SNRToBlockErrorRateManager l_SNRToBlockErrorRateManager;
  l_SNRToBlockErrorRateManager.LoadTraces ();
  SNRToBlockErrorRateRecord * BLERRec;

  for (double i = -5; i < 40; i += 0.1)
    {
      BLERRec = l_SNRToBlockErrorRateManager.GetSNRToBlockErrorRateRecord (i,
                                                                           modulationType);
      delete BLERRec;
    }
  return false;
}

void
Ns3WranSNRtoBLERTestCase::DoRun (void)
{
  for (int i = 0; i < 7; i++)
    {
      DoRunOnce (i);
    }
}

/*
 * The test suite
 */
class Ns3WranPhyTestSuite : public TestSuite
{
public:
  Ns3WranPhyTestSuite ();
};
Ns3WranPhyTestSuite::Ns3WranPhyTestSuite ()
  : TestSuite ("wran-phy-layer", UNIT)
{
  AddTestCase (new Ns3WranSNRtoBLERTestCase, TestCase::QUICK);
  AddTestCase (new Ns3WranSimpleOFDMTestCase, TestCase::QUICK);

}

static Ns3WranPhyTestSuite ns3WranPhyTestSuite;
