/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2015 Green Network Research Group
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
 * Authors: Sayef Azad Sakin <sayefsakin[at]gmail[dot]com>
 *           
 */

#include "wran-helper.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/log.h"
#include <string>
#include "ns3/config.h"
#include "ns3/wran-net-device.h"
#include "ns3/wran-bs-net-device.h"
#include "ns3/wran-ss-net-device.h"
#include "ns3/wran-channel.h"
#include "ns3/wran-phy.h"
#include "ns3/simple-ofdm-wran-phy.h"
#include "ns3/simple-ofdm-wran-channel.h"
#include "ns3/pointer.h"
#include "ns3/wran-mac-to-mac-header.h"


NS_LOG_COMPONENT_DEFINE ("WranHelper");

namespace ns3 {

WranHelper::WranHelper (void)
  : m_channel (0)
{
}

WranHelper::~WranHelper (void)
{
}

void WranHelper::EnableAsciiForConnection (Ptr<OutputStreamWrapper> os,
                                            uint32_t nodeid,
                                            uint32_t deviceid,
                                            char *netdevice,
                                            char *connection)
{
  std::ostringstream oss;
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::" << netdevice << "/" << connection
      << "/TxQueue/Enqueue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultEnqueueSinkWithContext, os));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::" << netdevice << "/" << connection
      << "/TxQueue/Dequeue";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDequeueSinkWithContext, os));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::" << netdevice << "/" << connection
      << "/TxQueue/Drop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiTraceHelper::DefaultDropSinkWithContext, os));
}

Ptr<WranPhy> WranHelper::CreatePhy (PhyType phyType)
{
  Ptr<WranPhy> phy;
  switch (phyType)
    {
    case SIMPLE_PHY_TYPE_OFDM:
      phy = CreateObject<SimpleOfdmWranPhy> ();
      if (!m_channel)
        {
          m_channel = CreateObject<SimpleOfdmWranChannel> (SimpleOfdmWranChannel::COST231_PROPAGATION);
        }
      break;
    default:
      NS_FATAL_ERROR ("Invalid physical type");
      break;
    }

  return phy;
}

void WranHelper::SetPropagationLossModel (SimpleOfdmWranChannel::PropModel propagationModel)
{
  if (!m_channel)
    {
      m_channel = CreateObject<SimpleOfdmWranChannel> ();
    }
  m_channel->GetObject<SimpleOfdmWranChannel> ()->SetPropagationModel (propagationModel);
}

Ptr<WranPhy> WranHelper::CreatePhy (PhyType phyType, char * SNRTraceFilePath, bool activateLoss)
{
  Ptr<WranPhy> phy;
  Ptr<SimpleOfdmWranPhy> sphy;
  switch (phyType)
    {
    case SIMPLE_PHY_TYPE_OFDM:
      sphy = CreateObject<SimpleOfdmWranPhy> ();
      phy = sphy;

      sphy->SetSNRToBlockErrorRateTracesPath(SNRTraceFilePath); sphy->ActivateLoss(activateLoss);
      if (!m_channel)
        {
          m_channel = CreateObject<SimpleOfdmWranChannel> (SimpleOfdmWranChannel::COST231_PROPAGATION);
        }
      break;
    default:
      NS_FATAL_ERROR ("Invalid physical type");
      break;
    }

  return phy;
}

Ptr<WranPhy> WranHelper::CreatePhyWithoutChannel (PhyType phyType)
{
  Ptr<WranPhy> phy;
  switch (phyType)
    {
    case SIMPLE_PHY_TYPE_OFDM:
      phy = CreateObject<SimpleOfdmWranPhy> ();
      break;
    default:
      NS_FATAL_ERROR ("Invalid physical type");
      break;
    }

  return phy;
}

Ptr<WranPhy> WranHelper::CreatePhyWithoutChannel (PhyType phyType, char * SNRTraceFilePath, bool activateLoss)
{
  Ptr<WranPhy> phy;
  Ptr<SimpleOfdmWranPhy> sphy;
  switch (phyType)
    {
    case SIMPLE_PHY_TYPE_OFDM:
      sphy = CreateObject<SimpleOfdmWranPhy> ();
      phy = sphy;

      sphy->SetSNRToBlockErrorRateTracesPath(SNRTraceFilePath); sphy->ActivateLoss(activateLoss);
      break;
    default:
      NS_FATAL_ERROR ("Invalid physical type");
      break;
    }

  return phy;
}

Ptr<WranUplinkScheduler> WranHelper::CreateWranUplinkScheduler (SchedulerType schedulerType)
{
  Ptr<WranUplinkScheduler> uplinkScheduler;
  switch (schedulerType)
    {
    case SCHED_TYPE_SIMPLE:
      uplinkScheduler = CreateObject<WranUplinkSchedulerSimple> ();
      break;
    case SCHED_TYPE_RTPS:
      uplinkScheduler = CreateObject<WranUplinkSchedulerRtps> ();
      break;
    case SCHED_TYPE_MBQOS:
      uplinkScheduler = CreateObject<WranUplinkSchedulerMBQoS> (Seconds (0.25));
      break;
    default:
      NS_FATAL_ERROR ("Invalid scheduling type");
      break;
    }
  return uplinkScheduler;
}

Ptr<WranBSScheduler> WranHelper::CreateWranBSScheduler (SchedulerType schedulerType)
{
  Ptr<WranBSScheduler> bsScheduler;
  switch (schedulerType)
    {
    case SCHED_TYPE_SIMPLE:
      bsScheduler = CreateObject<WranBSSchedulerSimple> ();
      break;
    case SCHED_TYPE_RTPS:
      bsScheduler = CreateObject<WranBSSchedulerRtps> ();
      break;
    case SCHED_TYPE_MBQOS:
      bsScheduler = CreateObject<WranBSSchedulerSimple> ();
      break;
    default:
      NS_FATAL_ERROR ("Invalid scheduling type");
      break;
    }
  return bsScheduler;
}

NetDeviceContainer WranHelper::Install (NodeContainer c,
                                         NetDeviceType deviceType,
                                         PhyType phyType,
                                         SchedulerType schedulerType,
                                         double frameDuration)
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<Node> node = *i;
      Ptr<WranPhy> phy = CreatePhy (phyType);

      // Set SuperFrame Duration
      phy->SetFrameDuration (Seconds (frameDuration));

      Ptr<WranNetDevice> device;
      Ptr<WranUplinkScheduler> uplinkScheduler = CreateWranUplinkScheduler (schedulerType);
      Ptr<WranBSScheduler> bsScheduler = CreateWranBSScheduler (schedulerType);

      if (deviceType == DEVICE_TYPE_BASE_STATION)
        {
          // attach phy
          Ptr<WranBaseStationNetDevice> deviceBS;
          deviceBS = CreateObject<WranBaseStationNetDevice> (node, phy, uplinkScheduler, bsScheduler);
          device = deviceBS;
          uplinkScheduler->SetBs (deviceBS);
          bsScheduler->SetBs (deviceBS);
        }
      else
        {
          device = CreateObject<WranSubscriberStationNetDevice> (node, phy);
        }
      device->SetAddress (Mac48Address::Allocate ());
      phy->SetDevice (device);
      device->Start ();
      device->Attach (m_channel); // attach channel

      node->AddDevice (device);

      devices.Add (device);
    }
  return devices;
}

NetDeviceContainer WranHelper::Install (NodeContainer c,
                                         NetDeviceType deviceType,
                                         PhyType phyType,
                                         SchedulerType schedulerType)
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<Node> node = *i;
      Ptr<WranPhy> phy = CreatePhy (phyType);
      Ptr<WranNetDevice> device;
      Ptr<WranUplinkScheduler> uplinkScheduler = CreateWranUplinkScheduler (schedulerType);
      Ptr<WranBSScheduler> bsScheduler = CreateWranBSScheduler (schedulerType);

      if (deviceType == DEVICE_TYPE_BASE_STATION)
        {
          // attach phy
          Ptr<WranBaseStationNetDevice> deviceBS;
          deviceBS = CreateObject<WranBaseStationNetDevice> (node, phy, uplinkScheduler, bsScheduler);
          device = deviceBS;
          uplinkScheduler->SetBs (deviceBS);
          bsScheduler->SetBs (deviceBS);
        }
      else
        {
          device = CreateObject<WranSubscriberStationNetDevice> (node, phy);
        }
      device->SetAddress (Mac48Address::Allocate ());
      phy->SetDevice (device);
      device->Start ();
      device->Attach (m_channel); // attach channel

      node->AddDevice (device);

      devices.Add (device);
    }
  return devices;
}

NetDeviceContainer WranHelper::Install (NodeContainer c,
                                         NetDeviceType deviceType,
                                         PhyType phyType,
                                         Ptr<WranChannel> channel,
                                         SchedulerType schedulerType)
{
  NetDeviceContainer devices;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); i++)
    {
      Ptr<Node> node = *i;

      Ptr<WranPhy> phy = CreatePhyWithoutChannel (phyType, (char*) "dummy", 0);
      Ptr<WranNetDevice> device;
      Ptr<WranUplinkScheduler> uplinkScheduler = CreateWranUplinkScheduler (schedulerType);
      Ptr<WranBSScheduler> bsScheduler = CreateWranBSScheduler (schedulerType);

      if (deviceType == DEVICE_TYPE_BASE_STATION)
        {
          Ptr<WranBaseStationNetDevice> deviceBS;
          deviceBS = CreateObject<WranBaseStationNetDevice> (node, phy, uplinkScheduler, bsScheduler);
          device = deviceBS;
          uplinkScheduler->SetBs (deviceBS);
          bsScheduler->SetBs (deviceBS);
        }
      else
        {
          device = CreateObject<WranSubscriberStationNetDevice> (node, phy);
        }
      device->SetAddress (Mac48Address::Allocate ());
      phy->SetDevice (device);
      device->Start ();
      device->Attach (channel);

      node->AddDevice (device);
      devices.Add (device);
    }
  return devices;
}

Ptr<WranNetDevice> WranHelper::Install (Ptr<Node> node,
                                          NetDeviceType deviceType,
                                          PhyType phyType,
                                          Ptr<WranChannel> channel,
                                          SchedulerType schedulerType)
{

  // Ptr<WranPhy> phy = CreatePhyWithoutChannel (phyType);
  Ptr<WranPhy> phy = CreatePhyWithoutChannel (phyType, (char*) "dummy", 0);
  Ptr<WranNetDevice> device;
  Ptr<WranUplinkScheduler> uplinkScheduler = CreateWranUplinkScheduler (schedulerType);
  Ptr<WranBSScheduler> bsScheduler = CreateWranBSScheduler (schedulerType);

  if (deviceType == DEVICE_TYPE_BASE_STATION)
    {
      Ptr<WranBaseStationNetDevice> deviceBS;
      deviceBS = CreateObject<WranBaseStationNetDevice> (node, phy, uplinkScheduler, bsScheduler);
      device = deviceBS;
      uplinkScheduler->SetBs (deviceBS);
      bsScheduler->SetBs (deviceBS);
    }
  else
    {
      device = CreateObject<WranSubscriberStationNetDevice> (node, phy);
    }
  device->SetAddress (Mac48Address::Allocate ());
  phy->SetDevice (device);
  device->Start ();
  device->Attach (channel);

  node->AddDevice (device);

  return device;
}

void
WranHelper::EnableLogComponents (void)
{
  LogComponentEnable ("WranBandwidthManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranBSLinkManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranBaseStationNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("WranWranBSSchedulerRtps", LOG_LEVEL_ALL);
  LogComponentEnable ("WranWranBSSchedulerSimple", LOG_LEVEL_ALL);
  LogComponentEnable ("WranBSScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("WranBSServiceFlowManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranUplinkSchedulerMBQoS", LOG_LEVEL_ALL);
  LogComponentEnable ("WranUplinkSchedulerRtps", LOG_LEVEL_ALL);
  LogComponentEnable ("WranUplinkSchedulerSimple", LOG_LEVEL_ALL);
  LogComponentEnable ("WranUplinkScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("WranBurstProfileManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranConnectionManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranIpcsClassifierRecord", LOG_LEVEL_ALL);
  LogComponentEnable ("WranIpcsClassifier", LOG_LEVEL_ALL);
  LogComponentEnable ("MACMESSAGES", LOG_LEVEL_ALL);
  LogComponentEnable ("PacketBurst", LOG_LEVEL_ALL);
  LogComponentEnable ("WranServiceFlowManager", LOG_LEVEL_ALL);
  LogComponentEnable ("simpleOfdmWranChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("SimpleOfdmWranPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("SNRToBlockErrorRateManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranSSLinkManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranSSManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranSubscriberStationNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("WranSSScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("WranSSServiceFlowManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("WranMacQueue", LOG_LEVEL_ALL);
  LogComponentEnable ("WranNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("WranPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("WranTlv", LOG_LEVEL_ALL);
  LogComponentEnable ("WranBandwidthManager", LOG_LEVEL_ALL);
  LogComponentEnable ("WranBaseStationNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("WranWranBSSchedulerRtps", LOG_LEVEL_ALL);
  LogComponentEnable ("WranWranBSSchedulerSimple", LOG_LEVEL_ALL);
  LogComponentEnable ("WranBSScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("WranSubscriberStationNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("WranSSScheduler", LOG_LEVEL_ALL);
  LogComponentEnable ("WranMacQueue", LOG_LEVEL_ALL);
}


void WranHelper::AsciiRxEvent (Ptr<OutputStreamWrapper> stream,
                                std::string path,
                                Ptr<const Packet> packet,
                                const Mac48Address &source)
{
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " from: " << source << " ";
  *stream->GetStream () << path << std::endl;
}

void WranHelper::AsciiTxEvent (Ptr<OutputStreamWrapper> stream, std::string path, Ptr<const Packet> packet, const Mac48Address &dest)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " to: " << dest << " ";
  *stream->GetStream () << path << std::endl;
}

WranServiceFlow WranHelper::CreateWranServiceFlow (WranServiceFlow::Direction direction,
                                            WranServiceFlow::SchedulingType schedulinType,
                                            WranIpcsClassifierRecord classifier)
{
  CsParameters csParam (CsParameters::ADD, classifier);
  WranServiceFlow serviceFlow = WranServiceFlow (direction);
  serviceFlow.SetConvergenceSublayerParam (csParam);
  serviceFlow.SetCsSpecification (WranServiceFlow::IPV4);
  serviceFlow.SetServiceSchedulingType (schedulinType);
  serviceFlow.SetMaxSustainedTrafficRate (100);
  serviceFlow.SetMinReservedTrafficRate (1000000);
  serviceFlow.SetMinTolerableTrafficRate (1000000);
  serviceFlow.SetMaximumLatency (100);
  serviceFlow.SetMaxTrafficBurst (2000);
  serviceFlow.SetTrafficPriority (1);
  serviceFlow.SetUnsolicitedGrantInterval (1);
  serviceFlow.SetMaxSustainedTrafficRate (70);
  serviceFlow.SetToleratedJitter (10);
  serviceFlow.SetSduSize (49);
  serviceFlow.SetRequestTransmissionPolicy (0);
  return serviceFlow;
}

void
WranHelper::EnableAsciiInternal (Ptr<OutputStreamWrapper> stream,
                                  std::string prefix,
                                  Ptr<NetDevice> nd,
                                  bool explicitFilename)
{
  //
  // All of the ascii enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type CsmaNetDevice.
  //
  Ptr<WranNetDevice> device = nd->GetObject<WranNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("WranHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::WranNetDevice");
      return;
    }

  //
  // Our default trace sinks are going to use packet printing, so we have to
  // make sure that is turned on.
  //
  Packet::EnablePrinting ();

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and do a Hook*WithoutContext
  // since there will be one file per context and therefore the context would
  // be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;
      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }
      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);

      uint32_t nodeid = nd->GetNode ()->GetId ();
      uint32_t deviceid = nd->GetIfIndex ();
      std::ostringstream oss;
      //
      // The MacRx trace source provides our "r" event.
      //

      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::WranNetDevice/Rx";
      Config::Connect (oss.str (), MakeBoundCallback (&WranHelper::AsciiRxEvent, theStream));
      oss.str ("");
      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::WranNetDevice/Tx";
      Config::Connect (oss.str (), MakeBoundCallback (&WranHelper::AsciiTxEvent, theStream));
      //
      // The "+", '-', and 'd' events are driven by trace sources actually in the
      // transmit queue.
      //

      EnableAsciiForConnection (theStream, nodeid, deviceid, (char*) "WranNetDevice", (char*) "InitialRangingConnection");
      EnableAsciiForConnection (theStream, nodeid, deviceid, (char*) "WranNetDevice", (char*) "BroadcastConnection");
      EnableAsciiForConnection (theStream, nodeid, deviceid, (char*) "WranSubscriberStationNetDevice", (char*) "BasicConnection");
      EnableAsciiForConnection (theStream, nodeid, deviceid, (char*) "WranSubscriberStationNetDevice", (char*) "PrimaryConnection");
      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to providd a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with the context.
  //
  // Note that we are going to use the default trace sinks provided by the
  // ascii trace helper.  There is actually no AsciiTraceHelper in sight here,
  // but the default trace sinks are actually publicly available static
  // functions that are always there waiting for just such a case.
  //
  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::WranNetDevice/Rx";
  Config::Connect (oss.str (), MakeBoundCallback (&WranHelper::AsciiRxEvent, stream));

  oss.str ("");
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::WranNetDevice/Tx";
  Config::Connect (oss.str (), MakeBoundCallback (&WranHelper::AsciiTxEvent, stream));

  EnableAsciiForConnection (stream, nodeid, deviceid, (char*) "WranNetDevice", (char*) "InitialRangingConnection");
  EnableAsciiForConnection (stream, nodeid, deviceid, (char*) "WranNetDevice", (char*) "BroadcastConnection");
  EnableAsciiForConnection (stream, nodeid, deviceid, (char*) "WranSubscriberStationNetDevice", (char*) "BasicConnection");
  EnableAsciiForConnection (stream, nodeid, deviceid, (char*) "WranSubscriberStationNetDevice", (char*) "PrimaryConnection");

}

static void PcapSniffTxRxEvent (Ptr<PcapFileWrapper> file,
                                Ptr<const PacketBurst> burst)
{
  std::list<Ptr<Packet> > packets = burst->GetPackets ();
  for (std::list<Ptr<Packet> >::iterator iter = packets.begin (); iter != packets.end (); ++iter)
    {
      Ptr<Packet> p = (*iter)->Copy ();
      WranMacToMacHeader m2m (p->GetSize ());
      p->AddHeader (m2m);
      file->Write (Simulator::Now (), p);
    }
}

void
WranHelper::EnablePcapInternal (std::string prefix, Ptr<NetDevice> nd, bool explicitFilename, bool promiscuous)
{
  //
  // All of the Pcap enable functions vector through here including the ones
  // that are wandering through all of devices on perhaps all of the nodes in
  // the system.  We can only deal with devices of type WranNetDevice.
  //
  Ptr<WranNetDevice> device = nd->GetObject<WranNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("WranHelper::EnablePcapInternal(): Device " << &device << " not of type ns3::WranNetDevice");
      return;
    }

  Ptr<WranPhy> phy = device->GetPhy ();
  PcapHelper pcapHelper;
  std::string filename;
  if (explicitFilename)
    {
      filename = prefix;
    }
  else
    {
      filename = pcapHelper.GetFilenameFromDevice (prefix, device);
    }

  Ptr<PcapFileWrapper> file = pcapHelper.CreateFile (filename, std::ios::out, PcapHelper::DLT_EN10MB);

  phy->TraceConnectWithoutContext ("Tx", MakeBoundCallback (&PcapSniffTxRxEvent, file));
  phy->TraceConnectWithoutContext ("Rx", MakeBoundCallback (&PcapSniffTxRxEvent, file));
}

int64_t
WranHelper::AssignStreams (int64_t stream)
{
  NS_LOG_FUNCTION (this << stream);
  return m_channel->AssignStreams (stream);
}

int64_t
WranHelper::AssignStreams (NetDeviceContainer c, int64_t stream)
{
  int64_t currentStream = stream;
  Ptr<NetDevice> netDevice;
  for (NetDeviceContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      netDevice = (*i);
      Ptr<WranNetDevice> wran = DynamicCast<WranNetDevice> (netDevice);
      if (wran)
        {
          // Handle any random numbers in the PHY objects.
          currentStream += wran->GetPhy ()->AssignStreams (currentStream);
        }
    }

  // Handle any random numbers in the channel.
  currentStream += m_channel->AssignStreams (currentStream);

  return (currentStream - stream);
}

} // namespace ns3
