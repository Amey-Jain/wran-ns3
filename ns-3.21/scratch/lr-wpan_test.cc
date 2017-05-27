/*
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
 * Author:  Nico Krezic-Luger <nico.luger@german-vr.de>
 */



// this topology is build in the skript
/*
//802.15.4-lr-wpan
// RFD	 RFD	FFD
//  *	  *	    *
//  |	  |	    |
// n0	 n1	   n2
//
*/		       


#include <ns3/log.h>
#include <ns3/core-module.h>
#include <ns3/lr-wpan-module.h>
#include <ns3/propagation-loss-model.h>
#include <ns3/propagation-delay-model.h>
#include <ns3/simulator.h>
#include <ns3/single-model-spectrum-channel.h>
#include <ns3/constant-position-mobility-model.h>
#include <ns3/packet.h>
#include <ns3/node-container.h>

#include <vector>

using namespace ns3;


//collects basic MAC-Layer packet information for printing
struct BasicPacketData 
{
public:
 BasicPacketData (Ptr<const Packet>& p) : p(p)
 {
 
 //get packets 802.15.4-Header
 LrWpanMacHeader h;
 p->PeekHeader(h);

 size = p->GetSize();
 src = h.GetShortSrcAddr();
 dst = h.GetShortDstAddr();
 pan_id = h.GetSrcPanId();
 seq_nr = h.GetSeqNum();	
 }
 
 uint32_t size;
 Mac16Address src;
 Mac16Address dst;
 uint16_t pan_id;
 uint8_t seq_nr;
 
 private:
 Ptr<const Packet> p;
};


static void PacketEnqued (Ptr<LrWpanNetDevice> dev, Ptr<const Packet> p)
{
  BasicPacketData bpd (p);

  NS_LOG_UNCOND ("----------MSDU enqued----------\n"
  << "Packet of size " << bpd.size << " bytes enqued\n"
  << "on device with address "<<dev->GetMac()->GetShortAddress()<<"\n"
  << "to be sent to address " << bpd.dst << "\n"
  << "sequence number is " << int (bpd.seq_nr) << "\n"
  << "Source PAN ID is " << bpd.pan_id << "\n"
  << "----------MSDU enqued------------\n\n"
);  
}




static void PacketReceived (Ptr<LrWpanNetDevice> dev, Ptr<const Packet> p)
{

  LrWpanMacHeader h;
  p->PeekHeader(h);
  BasicPacketData bpd (p);

  if(!h.IsAcknowledgment())
  {
  NS_LOG_UNCOND ("----------Packet received----------\n"
  << "Packet of size " << bpd.size << " bytes received\n"
  << "on device with address "<< dev->GetMac()->GetShortAddress()<<"\n"
  << "was sent from address " << bpd.src << "\n"
  << "sequence number is " << (int) bpd.seq_nr << "\n"
  << "PAN ID is " << bpd.pan_id << "\n"
  << "----------Packet received------------\n\n"
);
}
 else
 {
  NS_LOG_UNCOND ("----------ACK Frame received---------\n"
  << "ACK frame of size " << bpd.size << " bytes \n"
  <<  "received on device with address " << dev->GetMac()->GetShortAddress() << "\n"
  << "for the frame with sequence number " << (int) bpd.seq_nr << "\n"
  << "----------ACK Frame received---------\n\n"
);

 }
}

static void PacketSendInfo (Ptr<LrWpanNetDevice> dev, Ptr<const Packet> p, uint8_t retries, uint8_t csmaca_backoffs)
{
 BasicPacketData bpd (p);
 NS_LOG_UNCOND ("----------MSDU SEND INFO----------\n"
 << "Packet with sequence number "<< int(bpd.seq_nr)<<"\n"
 << "was sent or given up on device with address " << dev->GetMac()->GetShortAddress() << "\n"
 << "Number of sending retries " << int(retries)<<"\n"
 << "Number of CSMA/CA backoffs " << int(csmaca_backoffs) << "\n"
 << "----------MSDU SEND INFO------------\n"
 << "---------------------------------------------------\n"
 << "---------------------------------------------------\n\n\n\n");
}

static void McpsDataConfirm(Ptr<LrWpanNetDevice> dev, McpsDataConfirmParams params)
{
std::string status;
 switch(params.m_status)
 {
  case   IEEE_802_15_4_SUCCESS : status = "IEEE_802_15_4_SUCCESS"; break;
  case   IEEE_802_15_4_TRANSACTION_OVERFLOW : status =  "IEEE_802_15_4_TRANSACTION_OVERFLOW"; break;
  case   IEEE_802_15_4_TRANSACTION_EXPIRED  : status = "IEEE_802_15_4_TRANSACTION_EXPIRED"; break;
  case   IEEE_802_15_4_CHANNEL_ACCESS_FAILURE  : status = "IEEE_802_15_4_CHANNEL_ACCESS_FAILURE"; break;
  case   IEEE_802_15_4_INVALID_ADDRESS  : status = "IEEE_802_15_4_INVALID_ADDRESS"; break;
  case   IEEE_802_15_4_INVALID_GTS   : status = "IEEE_802_15_4_INVALID_GTS"; break;
  case   IEEE_802_15_4_NO_ACK        : status = "IEEE_802_15_4_NO_ACK"; break;
  case   IEEE_802_15_4_COUNTER_ERROR    : status = "IEEE_802_15_4_COUNTER_ERROR"; break;
  case   IEEE_802_15_4_FRAME_TOO_LONG   : status = "IEEE_802_15_4_FRAME_TOO_LONG"; break;
  case   IEEE_802_15_4_UNAVAILABLE_KEY    : status = "IEEE_802_15_4_UNAVAILABLE_KEY"; break;
  case   IEEE_802_15_4_UNSUPPORTED_SECURITY   : status = "IEEE_802_15_4_UNSUPPORTED_SECURITY"; break;
  case   IEEE_802_15_4_INVALID_PARAMETER   : status = "IEEE_802_15_4_INVALID_PARAMETER"; break;
  default : status = "UNKNOWN VALUE";
 }

 NS_LOG_UNCOND ("--------McpsDataConfirmStatus--------\n"
  << "On device with address "<< dev->GetMac()->GetShortAddress()<<"\n"
  << status << "\n"
  <<"--------McpsDataConfirmStatus-------- " << "\n\n"
  );
}


static void PacketSentSuccessfully (Ptr<const Packet> p)
{

BasicPacketData bpd (p);
  NS_LOG_UNCOND ("----------MSDU sent successfully----------\n"
  << "Packet of size " << bpd.size << " bytes sent\n"
  << "was successfully sent from address " << bpd.src << " to address " << bpd.dst << "\n"
  << "sequence number is " << (int) bpd.seq_nr << "\n"
  << "PAN ID is " << bpd.pan_id << "\n"
  << "----------MSDU sent successfully------------\n\n"
);
}



static void PacketDropped (Ptr<LrWpanNetDevice> dev, Ptr<const Packet> p)
{

BasicPacketData bpd (p);
  NS_LOG_UNCOND ("----------Packet dropped----------\n"
  << "Packet of size " << bpd.size << " was dropped during MAC filtering\n"
  << "on device with address "<<dev->GetMac()->GetShortAddress()<<"\n"
  << "coming from address " << bpd.src << " to address " << bpd.dst << "\n"
  << "sequence number is " << (int) bpd.seq_nr << "\n"
  << "PAN ID is " << bpd.pan_id << "\n"
  << "----------Packet dropped------------\n\n"
);
}


static void MakeCallbacks(std::vector<Ptr <LrWpanNetDevice> > devs)
{


    for (std::vector<Ptr <LrWpanNetDevice> >::const_iterator i = devs.begin(); i!=devs.end(); ++i)
    {
        Ptr<LrWpanNetDevice> dev = *i;
        dev->GetMac()->TraceConnectWithoutContext ("MacTxEnqueue", MakeBoundCallback(&PacketEnqued,dev));
        dev->GetMac()->TraceConnectWithoutContext ("MacTxOk", MakeCallback(&PacketSentSuccessfully));
        dev->GetMac()->TraceConnectWithoutContext ("MacRx", MakeBoundCallback(&PacketReceived, dev));
        dev->GetMac()->TraceConnectWithoutContext ("MacRxDrop", MakeBoundCallback(&PacketDropped,dev));
        dev->GetMac()->TraceConnectWithoutContext ("MacSentPkt", MakeBoundCallback(&PacketSendInfo,dev));

        McpsDataConfirmCallback cb0 = MakeBoundCallback (&McpsDataConfirm,dev);
        dev->GetMac ()->SetMcpsDataConfirmCallback (cb0);
    }

}


int main (int argc, char *argv[])
{
  bool verbose = false;

  CommandLine cmd;
  cmd.AddValue ("verbose", "turn on all log components", verbose);
  cmd.Parse (argc, argv);

  LrWpanHelper lrWpanHelper;
  if (verbose)
   {
     lrWpanHelper.EnableLogComponents ();
   }

 GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));


 Ptr<Node> n0 = CreateObject <Node> ();
 Ptr<Node> n1 = CreateObject <Node> ();
 Ptr<Node> n2 = CreateObject <Node> ();

 Ptr<LrWpanNetDevice> dev0 = CreateObject<LrWpanNetDevice> ();
 Ptr<LrWpanNetDevice> dev1 = CreateObject<LrWpanNetDevice> ();
 Ptr<LrWpanNetDevice> dev2 = CreateObject<LrWpanNetDevice> ();

 dev0->SetAddress (Mac16Address ("00:01"));
 dev1->SetAddress (Mac16Address ("00:02"));
 dev2->SetAddress (Mac16Address ("00:03"));

  // Each device must be attached to the same channel
 Ptr<SingleModelSpectrumChannel> channel = CreateObject<SingleModelSpectrumChannel> ();
 Ptr<LogDistancePropagationLossModel> propModel = CreateObject<LogDistancePropagationLossModel> ();
 Ptr<ConstantSpeedPropagationDelayModel> delayModel = CreateObject<ConstantSpeedPropagationDelayModel> ();
 channel->AddPropagationLossModel (propModel);
 channel->SetPropagationDelayModel (delayModel);

 dev0->SetChannel(channel);
 dev1->SetChannel(channel);
 dev2->SetChannel(channel);

//add devices to nodes
 n0->AddDevice (dev0);
 n1->AddDevice (dev1);
 n2->AddDevice (dev2);
 
 Ptr<ConstantPositionMobilityModel> sender0Mobility = CreateObject<ConstantPositionMobilityModel> ();
 sender0Mobility->SetPosition (Vector (0,0,0));
 dev0->GetPhy ()->SetMobility (sender0Mobility);

 Ptr<ConstantPositionMobilityModel> sender1Mobility = CreateObject<ConstantPositionMobilityModel> ();
 // Configure position 10 m distance
 sender1Mobility->SetPosition (Vector (0,10,0));
 dev1->GetPhy ()->SetMobility (sender1Mobility);


 Ptr<ConstantPositionMobilityModel> sender2Mobility = CreateObject<ConstantPositionMobilityModel> ();
 // Configure position 20 m distance
 sender2Mobility->SetPosition (Vector (0,20,0));
 dev2->GetPhy ()->SetMobility (sender2Mobility);



 std::vector<Ptr <LrWpanNetDevice> > devs;
 devs.push_back(dev0);
 devs.push_back(dev1);
 devs.push_back(dev2);


 MakeCallbacks(devs);

 // Tracing
 lrWpanHelper.EnablePcapAll (std::string ("lr-wpan-data"), true);
 AsciiTraceHelper ascii;
 Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("lr-wpan-data.tr");
 lrWpanHelper.EnableAsciiAll (stream);

//Sending Packets

 Ptr<Packet> p1 = Create<Packet> (50);
 McpsDataRequestParams params;
 params.m_srcAddrMode = SHORT_ADDR;
 params.m_dstAddrMode = SHORT_ADDR;
 params.m_dstPanId = 0;
 params.m_dstAddr = Mac16Address ("00:03");
 params.m_msduHandle = 0;
 params.m_txOptions = TX_OPTION_ACK;


  
//  dev0->GetMac ()->McpsDataRequest (params, p1);
  Simulator::ScheduleWithContext (1, Seconds (0.0),
                                  &LrWpanMac::McpsDataRequest,
                            dev0->GetMac (), params, p1);

//CSMA/CA - Test

  // Disable backoff at start
 dev0->GetCsmaCa ()->SetMacMinBE (0);

 Ptr<Packet> p2 = Create<Packet> (20);
 Simulator::ScheduleWithContext (2, Seconds (2.0),
                                  &LrWpanMac::McpsDataRequest,
                                 dev0->GetMac (), params, p2);

 Ptr<Packet> p3 = Create<Packet> (30);
 Simulator::ScheduleWithContext (3, Seconds (2.0),
                                  &LrWpanMac::McpsDataRequest,
                                  dev1->GetMac (), params, p3);
  //ACK - Test

 Ptr<Packet> p4 = Create<Packet> (40);
 params.m_dstAddr = Mac16Address ("00:04");
 Simulator::ScheduleWithContext (4, Seconds (3.0),
                                &LrWpanMac::McpsDataRequest,
                                dev2->GetMac (), params, p4);


 Ptr<Packet> p5 = Create<Packet> (40);
 params.m_dstAddr = Mac16Address ("00:04");
 params.m_txOptions = TX_OPTION_NONE;
 Simulator::ScheduleWithContext (5, Seconds (4.0),
                                &LrWpanMac::McpsDataRequest,
                                dev2->GetMac (), params, p5);

 Simulator::Run ();

 Simulator::Destroy ();
 
 

return 0;
}