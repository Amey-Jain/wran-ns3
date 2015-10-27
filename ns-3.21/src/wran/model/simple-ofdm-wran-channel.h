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

#ifndef SIMPLE_OFDM_WRAN_CHANNEL_H
#define SIMPLE_OFDM_WRAN_CHANNEL_H

#include <list>
#include "wran-channel.h"
#include "ns3/bvec.h"
#include "wran-phy.h"
#include "ns3/propagation-loss-model.h"
#include "wran-simple-ofdm-send-param.h"

namespace ns3 {

class Packet;
class PacketBurst;
class SimpleOfdmWranPhy;

/**
 * \ingroup wran
 */
class SimpleOfdmWranChannel : public WranChannel
{
public:
  SimpleOfdmWranChannel (void);
  ~SimpleOfdmWranChannel (void);

  enum PropModel
  {
    RANDOM_PROPAGATION,
    FRIIS_PROPAGATION,
    LOG_DISTANCE_PROPAGATION,
    COST231_PROPAGATION,
    ITU_NLOS_ROOFTOP_PROPAGATION
  };
  /**
   * \brief Creates a channel and sets the propagation model
   * \param propModel the propagation model to use
   */
  SimpleOfdmWranChannel (PropModel propModel);

  /**
   * \brief Sends a dummy fec block to all connected physical devices
   * \param BlockTime the time needed to send the block
   * \param burstSize the size of the burst
   * \param phy the sender device
   * \param isFirstBlock true if this block is the first one, false otherwise
   * \param isLastBlock true if this block is the last one, false otherwise
   * \param frequency the frequency on which the block is sent
   * \param modulationType the modulation used to send the fec block
   * \param direction uplink or downlink
   * \param txPowerDbm the transmission power
   * \param burst the packet burst to send
   */
  void Send (Time BlockTime,
             uint32_t burstSize, Ptr<WranPhy> phy, bool isFirstBlock,
             bool isLastBlock,
             uint64_t frequency, WranPhy::ModulationType modulationType,
             uint8_t direction, double txPowerDbm, Ptr<PacketBurst> burst);

  void Send (Time BlockTime,
			uint32_t burstSize,
			Ptr<WranPhy> phy,
			bool isFirstBlock,
			bool isLastBlock,
			uint64_t frequency,
			WranPhy::ModulationType modulationType,
			uint8_t direction,
			uint16_t nrOfSubChannel,
			std::vector<double> *txPowerListW,
			Ptr<PacketBurst> burst);
  /**
   * \brief sets the propagation model
   * \param propModel the propagation model to used
   */
  void SetPropagationModel (PropModel propModel);

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);

private:
  void DoAttach (Ptr<WranPhy> phy);
  std::list<Ptr<SimpleOfdmWranPhy> > m_phyList;
  uint32_t DoGetNDevices (void) const;
  void EndSendDummyBlock  (Ptr<SimpleOfdmWranPhy> rxphy, WranSimpleOfdmSendParam * param);
  Ptr<NetDevice> DoGetDevice (uint32_t i) const;
  Ptr<PropagationLossModel> m_loss;
};

} // namespace ns3

#endif /* SIMPLE_OFDM_WRAN_CHANNEL_H */
