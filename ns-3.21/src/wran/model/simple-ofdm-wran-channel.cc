/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2015, 2009 Green Network Research Group
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
 * Author:  
 *                              <amine.ismail@udcast.com>
 */

#include "ns3/simulator.h"
#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/assert.h"
#include "ns3/net-device.h"
#include "ns3/node.h"
#include "wran-phy.h"
#include "simple-ofdm-wran-phy.h"
#include "simple-ofdm-wran-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/cost231-propagation-loss-model.h"
#include "ns3/itu-r-1411-nlos-over-rooftop-propagation-loss-model.h"
#include "wran-simple-ofdm-send-param.h"
#include "common-cognitive-header.h"
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("simpleOfdmWranChannel");

namespace ns3 {
// NS_OBJECT_ENSURE_REGISTERED (simpleOfdmWranChannel)
//   ;


SimpleOfdmWranChannel::SimpleOfdmWranChannel (void)
{
  m_loss = 0;
}

SimpleOfdmWranChannel::~SimpleOfdmWranChannel (void)
{
  m_phyList.clear ();
}

SimpleOfdmWranChannel::SimpleOfdmWranChannel (PropModel propModel)
{
  switch (propModel)
    {
    case RANDOM_PROPAGATION:
      m_loss = CreateObject<RandomPropagationLossModel> ();
      break;

    case FRIIS_PROPAGATION:
      m_loss = CreateObject<FriisPropagationLossModel> ();
      break;
    case LOG_DISTANCE_PROPAGATION:
      m_loss = CreateObject<LogDistancePropagationLossModel> ();
      break;

    case COST231_PROPAGATION:
      m_loss = CreateObject<Cost231PropagationLossModel> ();
      break;

    case ITU_NLOS_ROOFTOP_PROPAGATION:
    	m_loss= CreateObject<ItuR1411NlosOverRooftopPropagationLossModel>();
    	break;

    default:
      m_loss = 0;
    }

}

void
SimpleOfdmWranChannel::SetPropagationModel (PropModel propModel)
{

  switch (propModel)
    {
    case RANDOM_PROPAGATION:
      m_loss = CreateObject<RandomPropagationLossModel> ();
      break;

    case FRIIS_PROPAGATION:
      m_loss = CreateObject<FriisPropagationLossModel> ();
      break;
    case LOG_DISTANCE_PROPAGATION:
      m_loss = CreateObject<LogDistancePropagationLossModel> ();
      break;

    case COST231_PROPAGATION:
      m_loss = CreateObject<Cost231PropagationLossModel> ();
      break;

    case ITU_NLOS_ROOFTOP_PROPAGATION:
		m_loss= CreateObject<ItuR1411NlosOverRooftopPropagationLossModel>();
		break;

    default:
      m_loss = 0;
    }

}

void
SimpleOfdmWranChannel::DoAttach (Ptr<WranPhy> phy)
{
  Ptr<SimpleOfdmWranPhy> o_phy = phy->GetObject<SimpleOfdmWranPhy> ();
  m_phyList.push_back (o_phy);
}

uint32_t
SimpleOfdmWranChannel::DoGetNDevices (void) const
{
  return m_phyList.size ();
}

Ptr<NetDevice>
SimpleOfdmWranChannel::DoGetDevice (uint32_t index) const
{
  uint32_t j = 0;
  for (std::list<Ptr<SimpleOfdmWranPhy> >::const_iterator iter = m_phyList.begin (); iter != m_phyList.end (); ++iter)
    {
      if (j == index)
        {
          return (*iter)->GetDevice ();
        }
      j++;
    }

  NS_FATAL_ERROR ("Unable to get device");
  return 0;
}

void
SimpleOfdmWranChannel::Send (Time BlockTime,
                              uint32_t burstSize,
                              Ptr<WranPhy> phy,
                              bool isFirstBlock,
                              bool isLastBlock,
                              uint64_t frequency,
                              WranPhy::ModulationType modulationType,
                              uint8_t direction,
                              double txPowerDbm,
                              Ptr<PacketBurst> burst)
{
	NS_LOG_INFO("From ofdm Channel, txPower " << txPowerDbm);
	Ptr<FriisPropagationLossModel> frii_loss = DynamicCast<FriisPropagationLossModel> (m_loss);
	frii_loss->SetFrequency((double)frequency * 1000000.0);
	NS_LOG_INFO("From ofdm Channel in Watt " << frii_loss->DbmToW(txPowerDbm) << " Freq " << frii_loss->GetFrequency());

  double rxPowerDbm = 0;
  Ptr<MobilityModel> senderMobility = 0;
  Ptr<MobilityModel> receiverMobility = 0;
  senderMobility = phy->GetDevice ()->GetNode ()->GetObject<MobilityModel> ();
  WranSimpleOfdmSendParam * param;
  for (std::list<Ptr<SimpleOfdmWranPhy> >::iterator iter = m_phyList.begin (); iter != m_phyList.end (); ++iter)
    {
      Time delay = Seconds (0);
      if (phy != *iter)
        {
          double distance = 0;
          receiverMobility = (*iter)->GetDevice ()->GetNode ()->GetObject<MobilityModel> ();
          if (receiverMobility != 0 && senderMobility != 0 && m_loss != 0)
            {
              distance = senderMobility->GetDistanceFrom (receiverMobility);
              delay =  Seconds (distance/300000000.0);
              rxPowerDbm = m_loss->CalcRxPower (txPowerDbm, senderMobility, receiverMobility);
              if(distance > MAX_TRANSMISSION_RANGE)continue;
            }

          param = new WranSimpleOfdmSendParam (burstSize,
                                           isFirstBlock,
                                           frequency,
                                           modulationType,
                                           direction,
                                           rxPowerDbm,
                                           burst);
          Ptr<Object> dstNetDevice = (*iter)->GetDevice ();
          uint32_t dstNode;
          if (dstNetDevice == 0)
            {
              dstNode = 0xffffffff;
            }
          else
            {
              dstNode = dstNetDevice->GetObject<NetDevice> ()->GetNode ()->GetId ();
            }
          Simulator::ScheduleWithContext (dstNode,
                                          delay,
                                          &SimpleOfdmWranChannel::EndSendDummyBlock,
                                          this,
                                          *iter,
                                          param);
        }
    }

}

void
SimpleOfdmWranChannel::EndSendDummyBlock (Ptr<SimpleOfdmWranPhy> rxphy, WranSimpleOfdmSendParam * param)
{
  rxphy->StartReceive (param->GetBurstSize (),
                       param->GetIsFirstBlock (),
                       param->GetFrequency (),
                       param->GetModulationType (),
                       param->GetDirection (),
                       param->GetRxPowerDbm (),
                       param->GetBurst ());
  delete param;
}

int64_t
SimpleOfdmWranChannel::AssignStreams (int64_t stream)
{
  int64_t currentStream = stream;
  typedef std::list<Ptr<SimpleOfdmWranPhy> > PhyList;
  for (PhyList::const_iterator i = m_phyList.begin (); i != m_phyList.end (); i++)
    {
      Ptr<SimpleOfdmWranPhy> simpleOfdm = (*i);
      currentStream += simpleOfdm->AssignStreams (currentStream);
    }
  return (currentStream - stream);
}

}
// namespace ns3
