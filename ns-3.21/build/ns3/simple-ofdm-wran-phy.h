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

#ifndef SIMPLE_OFDM_WRAN_PHY_H
#define SIMPLE_OFDM_WRAN_PHY_H

#include <stdint.h>
#include <list>
#include "ns3/callback.h"
#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/random-variable-stream.h"
#include "wran-phy.h"
#include "ns3/bvec.h"
#include "ns3/snr-to-block-error-rate-manager.h"
#include "wran-connection.h"
namespace ns3 {

class WranChannel;
class WranNetDevice;
class NetDevice;
class Packet;
class SimpleOfdmWranChannel;

/**
 * \ingroup wran
 */
class SimpleOfdmWranPhy : public WranPhy
{
public:
  enum FrameDurationCode
  {
    FRAME_DURATION_2_POINT_5_MS, FRAME_DURATION_4_MS, FRAME_DURATION_5_MS, FRAME_DURATION_8_MS, FRAME_DURATION_10_MS,
    FRAME_DURATION_12_POINT_5_MS, FRAME_DURATION_20_MS
  };

  static TypeId GetTypeId (void);
  SimpleOfdmWranPhy (void);
  SimpleOfdmWranPhy (char * tracesPath);
  ~SimpleOfdmWranPhy (void);
  /**
   * \brief if called with true it will enable the loss model
   * \param loss set to true to enable the loss model
   */
  void ActivateLoss (bool loss);
  /**
   * \brief Set the path of the repository containing the traces
   * \param tracesPath the path to the repository.
   *  see ns3/snr-to-block-error-rate-manager.h for more details.
   */
  void SetSNRToBlockErrorRateTracesPath (char * tracesPath);
  /**
    * Attach the physical layer to a channel.
    * \param channel the channel to which the physical layer will be attached
    */
  void DoAttach (Ptr<WranChannel> channel);
  /**
   * \brief set the callback function to call when a burst is received
   */
  void SetReceiveCallback (Callback<void, Ptr<PacketBurst>, Ptr<WranConnection> > callback);
  /**
   * \brief Sends a burst on the channel
   * \param burst the burst to send
   * \param modulationType the modulation that will be used to send this burst
   * \param direction set to uplink or downlink
   */
  void Send (Ptr<PacketBurst> burst, WranPhy::ModulationType modulationType, uint8_t direction);
  /**
   * \brief Sends a burst on the channel
   * \see SendParams
   */
  void Send (SendParams *params);
  /**
   * \brief returns the type this physical layer
   * \return always  WranPhy::simpleOfdmWranPhy;
   */
  WranPhy::PhyType GetPhyType (void) const;
  /**
   * \brief start the reception of a fec block
   * \param burstSize the burst size
   * \param isFirstBlock true if this block is the first one, false otherwise
   * \param frequency the frequency in wich the fec block is being received
   * \param modulationType the modulation used to transmit this fec Block
   * \param direction set to uplink and downlink
   * \param rxPower the received power.
   * \param burst the burst to be sent
   */

  void StartReceive (uint32_t burstSize,
                     bool isFirstBlock,
                     uint64_t frequency,
                     WranPhy::ModulationType modulationType,
                     uint8_t direction,
                     std::vector<double> rxPower,
                     Ptr<PacketBurst> burst);

  /**
   * \return the bandwidth
   */
  uint32_t GetBandwidth (void) const;
  /**
   * \brief Set the bandwidth
   * \param BW the bandwidth
   */
  void SetBandwidth (uint32_t BW);
  /**
   * \return the transmission power
   */
  double GetTxPower (void) const;
  /**
   * \brief set the transmission power
   * \param txPower the transmission power
   */
  void SetTxPower (double txPower);
  /**
     * \return the transmission power of subchannel.
     */
    double GetTxPowerSubChannel (uint16_t nrSubChannel) const;
    /**
	 * \return the transmission power list of subchannels.
	 */
    std::vector<double> GetTxPowerListSubChannel () const;
    /**
     * \brief set the transmission power
     * \param txPower the transmission power
     */
    void SetTxPowerSubChannel (uint16_t nrSubChannel, double txPower);

  /**
     * \return the received power
     */
    double GetRxPower (void) const;

    /**
	 * \brief set the received power.
	 * \param nrSubChannel sub channel index
	 * \param rxPower the received power
	 */
    void SetRxPowerSubChannel (uint16_t nrSubChannel, double rxPower);
    /**
	 * \return the received power list of subchannels.
	 */
    std::vector<double> GetRxPowerListSubChannel () const;
  /**
	 * \return the received power of subchannel.
	 */
	double GetRxPowerSubChannel (uint16_t nrSubChannel) const;
  /**
   * \return the noise figure
   */
  double GetNoiseFigure (void) const;
  /**
   * \brief set the noise figure of the device
   * \param nf the noise figure
   */
  void SetNoiseFigure (double nf);

  /**
   * Public method used to fire a PhyTxBegin trace.  Implemented for encapsulation
   * purposes.
   */
  void NotifyTxBegin (Ptr<PacketBurst> burst);

  /**
   * Public method used to fire a PhyTxEnd trace.  Implemented for encapsulation
   * purposes.
   */
  void NotifyTxEnd (Ptr<PacketBurst> burst);

  /**
   * Public method used to fire a PhyTxDrop trace.  Implemented for encapsulation
   * purposes.
   */
  void NotifyTxDrop (Ptr<PacketBurst> burst);

  /**
   * Public method used to fire a PhyRxBegin trace.  Implemented for encapsulation
   * purposes.
   */
  void NotifyRxBegin (Ptr<PacketBurst> burst);

  /**
   * Public method used to fire a PhyRxEnd trace.  Implemented for encapsulation
   * purposes.
   */
  void NotifyRxEnd (Ptr<PacketBurst> burst);

  /**
   * Public method used to fire a PhyRxDrop trace.  Implemented for encapsulation
   * purposes.
   */
  void NotifyRxDrop (Ptr<PacketBurst> burst);

 /**
  * Assign a fixed random variable stream number to the random variables
  * used by this model.  Return the number of streams (possibly zero) that
  * have been assigned.
  *
  * \param stream first stream index to use
  * \return the number of stream indices assigned by this model
  */
  int64_t AssignStreams (int64_t stream);

  /**
	* \brief set the transmission power
	* \param txPower the transmission power
	*/
  void SetNumberOfSubChannel (uint16_t nrOfSubChannel);
  /**
	* \return the received power
	*/
  uint16_t GetNumberOfSubChannel (void) const;
  /**
   *
   * @return the sub channel bandwidth
   */
  double GetSubChannelBandwidth () const;

  /** see page 119 sec 4.17.4 in book Wireless Communication and Networking by Vijay Greg
  	 * N = kTB
  	 * k = Boltzmann’s constant (1.3803 * 10^-23 J/K)
  	 * T = absolute temperature (K), At room temperature T = 290 K
  	 * B = channel bandwidth
  	 * @return calculated thermal noise in watt
  	 */
  double CalculateNoiseW () const;

//  virtual void SetSenseEndedCallback(WranPhy::SnsEndedCallback callback);
//  virtual void SetHandoffEndedCallback(WranPhy::HandoffEndedCallback callback);
//  /**
//     * \brief Set channel number.
//     *
//     * Channel center frequency = Channel starting frequency + 5 MHz * (nch - 1)
//     *
//     * where Starting channel frequency is standard-dependent, see SetStandard()
//     * as defined in IEEE 802.11-2007 17.3.8.3.2.
//     *
//     * YansWifiPhy can switch among different channels. Basically, YansWifiPhy
//     * has a private attribute m_channelNumber that identifies the channel the
//     * PHY operates on. Channel switching cannot interrupt an ongoing transmission.
//     * When PHY is in TX state, the channel switching is postponed until the end
//     * of the current transmission. When the PHY is in RX state, the channel
//     * switching causes the drop of the synchronized packet.
//     */
//    void SetChannelNumber (uint16_t id);
//
//    /**
//     * \brief Start sensing on current channel
//     *
//     * \param duration Time to sense
//     *
//     */
//    void StartSensing (Time duration);
//
//    virtual bool IsStateCcaBusy (void);
//      virtual bool IsStateIdle (void);
//      virtual bool IsStateBusy (void);
//      virtual bool IsStateRx (void);
//      virtual bool IsStateTx (void);
//      virtual bool IsStateSwitching (void);
//      virtual bool IsStateSensing (void);
private:
  Time DoGetTransmissionTime (uint32_t size, WranPhy::ModulationType modulationType) const;
  uint64_t DoGetNrSymbols (uint32_t size, WranPhy::ModulationType modulationType) const;
  uint64_t DoGetNrBytes (uint32_t symbols, WranPhy::ModulationType modulationType) const;
  bvec ConvertBurstToBits (Ptr<const PacketBurst> burst);
  Ptr<PacketBurst> ConvertBitsToBurst (bvec buffer);
  void CreateFecBlocks (const bvec &buffer, WranPhy::ModulationType modulationType);
  bvec RecreateBuffer ();
  uint32_t GetFecBlockSize (WranPhy::ModulationType type) const;
  uint32_t GetCodedFecBlockSize (WranPhy::ModulationType modulationType) const;
  void SetBlockParameters (uint32_t burstSize, WranPhy::ModulationType modulationType);
  uint16_t GetNrBlocks (uint32_t burstSize, WranPhy::ModulationType modulationType) const;
  void DoDispose (void);
  void EndSend (void);
  void EndSendFecBlock (WranPhy::ModulationType modulationType, uint8_t direction);
  void EndReceive (Ptr<const PacketBurst> burst);
  void EndReceiveFecBlock (uint32_t burstSize,
                           WranPhy::ModulationType modulationType,
                           uint8_t direction,
                           uint8_t drop,
                           Ptr<PacketBurst> burst);
  void StartSendDummyFecBlock (bool isFirstBlock,
                               WranPhy::ModulationType modulationType,
                               uint8_t direction);
  Time GetBlockTransmissionTime (WranPhy::ModulationType modulationType) const;
  void DoSetDataRates (void);
  void InitSimpleOfdmWranPhy (void);

  void GetModulationFecParams (WranPhy::ModulationType modulationType, uint8_t &bitsPerSymbol, double &fecCode) const;
  uint32_t CalculateDataRate (WranPhy::ModulationType modulationType) const;
  uint32_t DoGetDataRate (WranPhy::ModulationType modulationType) const;
  uint16_t DoGetTtg (void) const;
  uint16_t DoGetRtg (void) const;
  uint8_t DoGetFrameDurationCode (void) const;
  Time DoGetFrameDuration (uint8_t frameDurationCode) const;
  void DoSetPhyParameters (void);
  uint16_t DoGetNfft (void) const;
  void DoSetNfft (uint16_t);
  double DoGetSamplingFactor (void) const;
  double DoGetSamplingFrequency (void) const;
  double DoGetGValue (void) const;
  void DoSetGValue (double);

  void SetRxPower (double rxPower);

  double GetRxGain (void) const;
  void SetRxGain (double rxgain);

  double GetTxGain (void) const;
  void SetTxGain (double txgain);

  std::string GetTraceFilePath (void) const;
  void SetTraceFilePath (std::string path);

  uint16_t m_fecBlockSize; // in bits, size of FEC block transmitted after PHY operations
  uint32_t m_currentBurstSize;

  std::list<bvec> *m_receivedFecBlocks; // a list of received FEC blocks until they are combined to recreate the full burst buffer
  uint32_t m_nrFecBlocksSent; // counting the number of FEC blocks sent (within a burst)
  std::list<bvec> *m_fecBlocks;
  Time m_blockTime;

  TracedCallback<Ptr<const PacketBurst> > m_traceRx;
  TracedCallback<Ptr<const PacketBurst> > m_traceTx;

  // data rates for this Phy
  uint32_t m_dataRateBpsk12, m_dataRateQpsk12, m_dataRateQpsk34, m_dataRateQam16_12, m_dataRateQam16_34,
           m_dataRateQam64_23, m_dataRateQam64_34;

  // parameters to store for a per burst life-time
  uint16_t m_nrBlocks;
  uint16_t m_nrRemainingBlocksToSend;
  Ptr<PacketBurst> m_currentBurst;
  uint16_t m_blockSize;
  uint32_t m_paddingBits;
  uint16_t m_nbErroneousBlock;
  uint16_t m_nrRecivedFecBlocks;
  uint16_t m_nfft;
  double m_g;
  double m_bandWidth;
  double m_txPower;
  double m_rxPower;
  double m_noiseFigure;
  double m_txGain;
  double m_rxGain;
  std::vector<double> m_txPowerSubChannel;
  std::vector<double> m_rxPowerSubChannel;
  /**
   * The trace source fired when a packet begins the transmission process on
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback <Ptr<PacketBurst > >  m_phyTxBeginTrace;

  /**
   * The trace source fired when a packet ends the transmission process on
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<PacketBurst > > m_phyTxEndTrace;

  /**
   * The trace source fired when the phy layer drops a packet as it tries
   * to transmit it.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<PacketBurst > > m_phyTxDropTrace;

  /**
   * The trace source fired when a packet begins the reception process from
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<PacketBurst > > m_phyRxBeginTrace;

  /**
   * The trace source fired when a packet ends the reception process from
   * the medium.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<PacketBurst > > m_phyRxEndTrace;

  /**
   * The trace source fired when the phy layer drops a packet it has received.
   *
   * \see class CallBackTraceSource
   */
  TracedCallback<Ptr<PacketBurst > > m_phyRxDropTrace;

  SNRToBlockErrorRateManager * m_snrToBlockErrorRateManager;

  /// Provides uniform random variables.
  Ptr<UniformRandomVariable> m_URNG;

  /**
   * Total number of subchannel.
   */
  uint16_t m_numberOfSubchannel;
//  //sensing ended callback
//  SnsEndedCallback m_senseEndedCallback;
//  //handoff ended callback
//  HandoffEndedCallback m_handoffEndedCallback;

};

} // namespace ns3

#endif /* OFDM_WRAN_PHY_H */
