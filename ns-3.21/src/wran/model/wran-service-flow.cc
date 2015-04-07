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
 * Authors: Sayef Azad Sakin <sayefsakin@gmail.com>
 *           
 *                                
 */

#include "wran-service-flow.h"
#include "wran-service-flow-record.h"
#include "ns3/simulator.h"
#include "wran-tlv.h"

namespace ns3 {

WranServiceFlow::WranServiceFlow (Direction direction)
{
  InitValues ();
  m_direction = direction;
  m_type = SF_TYPE_PROVISIONED;
  m_record = new WranServiceFlowRecord ();
  m_sfid = 0;
  m_connection = 0;
  m_isEnabled = false;
  m_isMulticast = false;
  m_modulationType = WranPhy::MODULATION_TYPE_QPSK_12;
}

WranServiceFlow::WranServiceFlow ()
  : m_sfid (0),
    m_direction (SF_DIRECTION_DOWN),
    m_type (SF_TYPE_PROVISIONED),
    m_connection (0),
    m_isEnabled (false),
    m_record (new WranServiceFlowRecord ())
{
  InitValues ();
  m_isMulticast = false;
  m_modulationType = WranPhy::MODULATION_TYPE_QPSK_12;

}

WranServiceFlow::WranServiceFlow (uint32_t sfid, Direction direction, Ptr<WranConnection> connection)
{
  InitValues ();
  m_record = new WranServiceFlowRecord ();
  m_isEnabled = false;
  m_connection = connection;
  m_connection->SetWranServiceFlow (this);
  m_type = SF_TYPE_PROVISIONED;
  m_direction = direction;
  m_sfid = sfid;
  m_isMulticast = false;
  m_modulationType = WranPhy::MODULATION_TYPE_QPSK_12;
}

WranServiceFlow::WranServiceFlow (WranTlv tlv)
{
  InitValues ();
  m_connection = 0;
  m_isEnabled = 0;
  m_record = new WranServiceFlowRecord ();
  NS_ASSERT_MSG (tlv.GetType () == WranTlv::UPLINK_SERVICE_FLOW || tlv.GetType () == WranTlv::DOWNLINK_SERVICE_FLOW,
                 "Invalid TLV");

  SfWranVectorTlvValue * param;
  param = (SfWranVectorTlvValue*)(tlv.PeekValue ());

  if (tlv.GetType () == WranTlv::UPLINK_SERVICE_FLOW)
    {
      m_direction = SF_DIRECTION_UP;
    }
  else
    {
      m_direction = SF_DIRECTION_DOWN;
    }

  for (std::vector<WranTlv*>::const_iterator iter = param->Begin (); iter != param->End (); ++iter)
    {
      switch ((*iter)->GetType ())
        {
        case SfWranVectorTlvValue::SFID:
          {
            m_sfid = ((WranU32TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::CID:
          {
            uint16_t cid = ((WranU16TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            m_connection = CreateObject<WranConnection> (cid, Cid::TRANSPORT);
            break;
          }
        case SfWranVectorTlvValue::QoS_Parameter_Set_Type:
          {
            m_qosParamSetType = ((WranU8TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Traffic_Priority:
          {
            m_trafficPriority = ((WranU8TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Maximum_Sustained_Traffic_Rate:
          {
            m_maxSustainedTrafficRate = ((WranU32TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Maximum_Traffic_Burst:
          {
            m_maxTrafficBurst = ((WranU32TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Minimum_Reserved_Traffic_Rate:
          {
            m_minReservedTrafficRate = ((WranU32TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Minimum_Tolerable_Traffic_Rate:
          {
            m_minTolerableTrafficRate = ((WranU32TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Service_Flow_Scheduling_Type:
          {
            m_schedulingType = (WranServiceFlow::SchedulingType)((WranU8TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Request_Transmission_Policy:
          {
            m_requestTransmissionPolicy = ((WranU32TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Tolerated_Jitter:
          {
            m_toleratedJitter = ((WranU32TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Maximum_Latency:
          {
            m_maximumLatency = ((WranU32TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::Fixed_length_versus_Variable_length_SDU_Indicator:
          {
            m_fixedversusVariableSduIndicator = ((WranU16TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case SfWranVectorTlvValue::CS_Specification:
          {
            m_csSpecification = (enum CsSpecification)(((WranU8TlvValue*)((*iter)->PeekValue ()))->GetValue ());
            break;
          }

        case SfWranVectorTlvValue::IPV4_CS_Parameters:
          {
            m_convergenceSublayerParam = CsParameters (*(*iter));
            break;
          }

        }
    }
  m_isMulticast = false;
  m_modulationType = WranPhy::MODULATION_TYPE_QPSK_12;
}

WranServiceFlow::~WranServiceFlow (void)
{
  if (m_record != 0)
    {
      delete m_record;
      m_record = 0;
    }
  m_connection = 0;
}

void
WranServiceFlow::SetDirection (Direction direction)
{
  m_direction = direction;
}

WranServiceFlow::Direction
WranServiceFlow::GetDirection (void) const
{
  return m_direction;
}

void
WranServiceFlow::SetType (Type type)
{
  m_type = type;
}

WranServiceFlow::Type
WranServiceFlow::GetType (void) const
{
  return m_type;
}

void
WranServiceFlow::SetConnection (Ptr<WranConnection> connection)
{
  m_connection = connection;
  m_connection->SetWranServiceFlow (this);
}

Ptr<WranConnection>
WranServiceFlow::GetConnection (void) const
{
  return m_connection;
}

void
WranServiceFlow::SetIsEnabled (bool isEnabled)
{
  m_isEnabled = isEnabled;
}

bool
WranServiceFlow::GetIsEnabled (void) const
{
  return m_isEnabled;
}

void
WranServiceFlow::SetRecord (WranServiceFlowRecord *record)
{
  m_record = record;
}

WranServiceFlowRecord*
WranServiceFlow::GetRecord (void) const
{
  return m_record;
}

Ptr<WranMacQueue>
WranServiceFlow::GetQueue (void) const
{
  if (!m_connection)
    {
      return 0;
    }
  return m_connection->GetQueue ();
}

enum WranServiceFlow::SchedulingType
WranServiceFlow::GetSchedulingType (void) const
{
  return m_schedulingType;
}

bool
WranServiceFlow::HasPackets (void) const
{
  if (!m_connection)
    {
      return false;
    }
  return m_connection->HasPackets ();
}

bool
WranServiceFlow::HasPackets (MacHeaderType::HeaderType packetType) const
{
  if (!m_connection)
    {
      return false;
    }
  return m_connection->HasPackets (packetType);
}

void
WranServiceFlow::CleanUpQueue (void)
{
  GenericMacHeader hdr;
  Time timeStamp;
  Ptr<Packet> packet;
  Time currentTime = Simulator::Now ();
  if (m_connection)
    {
      while (m_connection->HasPackets ())
        {
          packet = m_connection->GetQueue ()->Peek (hdr, timeStamp);

          if (currentTime - timeStamp > MilliSeconds (GetMaximumLatency ()))
            {
              m_connection->Dequeue ();
            }
          else
            {
              break;
            }
        }
    }
}

void
WranServiceFlow::PrintQoSParameters (void) const
{
}
// ==============================================================================


uint32_t
WranServiceFlow::GetSfid (void) const
{
  return m_sfid;
}
uint16_t
WranServiceFlow::GetCid (void) const
{
  if (m_connection == 0)
    {
      return 0;
    }
  return m_connection->GetCid ().GetIdentifier ();
}
std::string
WranServiceFlow::GetServiceClassName () const
{
  return m_serviceClassName;
}
uint8_t
WranServiceFlow::GetQosParamSetType (void) const
{
  return m_qosParamSetType;
}
uint8_t
WranServiceFlow::GetTrafficPriority (void) const
{
  return m_trafficPriority;
}
uint32_t
WranServiceFlow::GetMaxSustainedTrafficRate (void) const
{
  return m_maxSustainedTrafficRate;
}
uint32_t
WranServiceFlow::GetMaxTrafficBurst (void) const
{
  return m_maxTrafficBurst;
}
uint32_t
WranServiceFlow::GetMinReservedTrafficRate (void) const
{
  return m_minReservedTrafficRate;
}
uint32_t
WranServiceFlow::GetMinTolerableTrafficRate (void) const
{
  return m_minTolerableTrafficRate;
}
enum
WranServiceFlow::SchedulingType WranServiceFlow::GetServiceSchedulingType (void) const
{
  return m_schedulingType;
}
uint32_t
WranServiceFlow::GetRequestTransmissionPolicy (void) const
{
  return m_requestTransmissionPolicy;
}
uint32_t
WranServiceFlow::GetToleratedJitter (void) const
{
  return m_toleratedJitter;
}
uint32_t
WranServiceFlow::GetMaximumLatency (void) const
{
  return m_maximumLatency;
}
uint8_t
WranServiceFlow::GetFixedversusVariableSduIndicator (void) const
{
  return m_fixedversusVariableSduIndicator;
}
uint8_t
WranServiceFlow::GetSduSize (void) const
{
  return m_sduSize;
}
uint16_t
WranServiceFlow::GetTargetSAID (void) const
{
  return m_targetSAID;
}
uint8_t
WranServiceFlow::GetArqEnable (void) const
{
  return m_arqEnable;
}
uint16_t
WranServiceFlow::GetArqWindowSize (void) const
{
  return m_arqWindowSize;
}
uint16_t
WranServiceFlow::GetArqRetryTimeoutTx (void) const
{
  return m_arqRetryTimeoutTx;
}
uint16_t
WranServiceFlow::GetArqRetryTimeoutRx (void) const
{
  return m_arqRetryTimeoutRx;
}

uint16_t
WranServiceFlow::GetArqBlockLifeTime (void) const
{
  return m_arqBlockLifeTime;
}
uint16_t
WranServiceFlow::GetArqSyncLoss (void) const
{
  return m_arqSyncLoss;
}
uint8_t
WranServiceFlow::GetArqDeliverInOrder (void) const
{
  return m_arqDeliverInOrder;
}
uint16_t
WranServiceFlow::GetArqPurgeTimeout (void) const
{
  return m_arqPurgeTimeout;
}
uint16_t
WranServiceFlow::GetArqBlockSize (void) const
{
  return m_arqBlockSize;
}
enum
WranServiceFlow::CsSpecification WranServiceFlow::GetCsSpecification (void) const
{
  return m_csSpecification;
}
CsParameters
WranServiceFlow::GetConvergenceSublayerParam (void) const
{
  return m_convergenceSublayerParam;
}
uint16_t
WranServiceFlow::GetUnsolicitedGrantInterval (void) const
{
  return m_unsolicitedGrantInterval;
}
uint16_t
WranServiceFlow::GetUnsolicitedPollingInterval (void) const
{
  return m_unsolicitedPollingInterval;
}

bool
WranServiceFlow::GetIsMulticast (void) const
{
  return m_isMulticast;
}
enum WranPhy::ModulationType
WranServiceFlow::GetModulation (void) const
{
  return m_modulationType;
}


// ==============================================================================

void
WranServiceFlow::SetSfid (uint32_t sfid)
{
  m_sfid = sfid;
}
void
WranServiceFlow::SetServiceClassName (std::string name)
{
  m_serviceClassName = name;
}
void
WranServiceFlow::SetQosParamSetType (uint8_t type)
{
  m_qosParamSetType = type;
}
void
WranServiceFlow::SetTrafficPriority (uint8_t priority)
{
  m_trafficPriority = priority;
}
void
WranServiceFlow::SetMaxSustainedTrafficRate (uint32_t maxSustainedRate)
{
  m_maxSustainedTrafficRate = maxSustainedRate;
}
void
WranServiceFlow::SetMaxTrafficBurst (uint32_t maxTrafficBurst)
{
  m_maxTrafficBurst = maxTrafficBurst;
}
void
WranServiceFlow::SetMinReservedTrafficRate (uint32_t minResvRate)
{
  m_minReservedTrafficRate = minResvRate;
}
void
WranServiceFlow::SetMinTolerableTrafficRate (uint32_t minJitter)
{
  m_minTolerableTrafficRate = minJitter;
}
void
WranServiceFlow::SetServiceSchedulingType (enum WranServiceFlow::SchedulingType schedType)
{
  m_schedulingType = schedType;
}
void
WranServiceFlow::SetRequestTransmissionPolicy (uint32_t policy)
{
  m_requestTransmissionPolicy = policy;
}
void
WranServiceFlow::SetToleratedJitter (uint32_t jitter)
{
  m_toleratedJitter = jitter;
}
void
WranServiceFlow::SetMaximumLatency (uint32_t MaximumLatency)
{
  m_maximumLatency = MaximumLatency;
}
void
WranServiceFlow::SetFixedversusVariableSduIndicator (uint8_t sduIndicator)
{
  m_fixedversusVariableSduIndicator = sduIndicator;
}
void
WranServiceFlow::SetSduSize (uint8_t sduSize)
{
  m_sduSize = sduSize;
}
void
WranServiceFlow::SetTargetSAID (uint16_t targetSaid)
{
  m_targetSAID = targetSaid;
}
void
WranServiceFlow::SetArqEnable (uint8_t arqEnable)
{
  m_arqEnable = arqEnable;
}
void
WranServiceFlow::SetArqWindowSize (uint16_t arqWindowSize)
{
  m_arqWindowSize = arqWindowSize;
}
void
WranServiceFlow::SetArqRetryTimeoutTx (uint16_t timeout)
{
  m_arqRetryTimeoutTx = timeout;
}
void
WranServiceFlow::SetArqRetryTimeoutRx (uint16_t timeout)
{
  m_arqRetryTimeoutRx = timeout;
}
void
WranServiceFlow::SetArqBlockLifeTime (uint16_t lifeTime)
{
  m_arqBlockLifeTime = lifeTime;
}
void
WranServiceFlow::SetArqSyncLoss (uint16_t syncLoss)
{
  m_arqSyncLoss = syncLoss;
}
void
WranServiceFlow::SetArqDeliverInOrder (uint8_t inOrder)
{
  m_arqDeliverInOrder = inOrder;
}
void
WranServiceFlow::SetArqPurgeTimeout (uint16_t timeout)
{
  m_arqPurgeTimeout = timeout;
}
void
WranServiceFlow::SetArqBlockSize (uint16_t size)
{
  m_arqBlockSize = size;
}
void
WranServiceFlow::SetCsSpecification (enum WranServiceFlow::CsSpecification spec)
{
  m_csSpecification = spec;
}
void
WranServiceFlow::SetConvergenceSublayerParam (CsParameters csparam)
{
  m_convergenceSublayerParam = csparam;
}
void
WranServiceFlow::SetUnsolicitedGrantInterval (uint16_t unsolicitedGrantInterval)
{
  m_unsolicitedGrantInterval = unsolicitedGrantInterval;
}
void
WranServiceFlow::SetUnsolicitedPollingInterval (uint16_t unsolicitedPollingInterval)
{
  m_unsolicitedPollingInterval = unsolicitedPollingInterval;
}
void
WranServiceFlow::SetIsMulticast (bool isMulticast)
{
  m_isMulticast = isMulticast;
}
void
WranServiceFlow::SetModulation (enum WranPhy::ModulationType modulationType)
{
  m_modulationType = modulationType;
}

void
WranServiceFlow::InitValues (void)
{
  m_sfid = 0;
  m_serviceClassName = "";
  m_qosParamSetType = 0;
  m_trafficPriority = 0;
  m_maxSustainedTrafficRate = 0;
  m_maxTrafficBurst = 0;
  m_minReservedTrafficRate = 0;
  m_minTolerableTrafficRate = 0;
  m_schedulingType = WranServiceFlow::SF_TYPE_NONE;
  m_requestTransmissionPolicy = 0;
  m_toleratedJitter = 0;
  m_maximumLatency = 0;
  m_fixedversusVariableSduIndicator = 0;
  m_sduSize = 0;
  m_targetSAID = 0;
  m_arqEnable = 0;
  m_arqWindowSize = 0;
  m_arqRetryTimeoutTx = 0;
  m_arqRetryTimeoutRx = 0;
  m_csSpecification = WranServiceFlow::IPV4;
  m_unsolicitedGrantInterval = 0;
  m_unsolicitedPollingInterval = 0;
  m_arqBlockLifeTime = 0;
  m_arqSyncLoss = 0;
  m_arqDeliverInOrder = 0;
  m_arqPurgeTimeout = 0;
  m_arqBlockSize = 0;
  m_direction = WranServiceFlow::SF_DIRECTION_DOWN;
  m_type = WranServiceFlow::SF_TYPE_ACTIVE;
  m_isMulticast = false;
  m_modulationType = WranPhy::MODULATION_TYPE_QPSK_12;
}

void
WranServiceFlow::CopyParametersFrom (WranServiceFlow sf)
{
  m_serviceClassName = sf.GetServiceClassName ();
  m_qosParamSetType = sf.GetQosParamSetType ();
  m_trafficPriority = sf.GetTrafficPriority ();
  m_maxSustainedTrafficRate = sf.GetMaxSustainedTrafficRate ();
  m_maxTrafficBurst = sf.GetMaxTrafficBurst ();
  m_minReservedTrafficRate = sf.GetMinReservedTrafficRate ();
  m_minTolerableTrafficRate = sf.GetMinTolerableTrafficRate ();
  m_schedulingType = sf.GetServiceSchedulingType ();
  m_requestTransmissionPolicy = sf.GetRequestTransmissionPolicy ();
  m_toleratedJitter = sf.GetToleratedJitter ();
  m_maximumLatency = sf.GetMaximumLatency ();
  m_fixedversusVariableSduIndicator = sf.GetFixedversusVariableSduIndicator ();
  m_sduSize = sf.GetSduSize ();
  m_targetSAID = sf.GetTargetSAID ();
  m_arqEnable = sf.GetArqEnable ();
  m_arqWindowSize = sf.GetArqWindowSize ();
  m_arqRetryTimeoutTx = sf.GetArqRetryTimeoutTx ();
  m_arqRetryTimeoutRx = sf.GetArqRetryTimeoutRx ();
  m_csSpecification = sf.GetCsSpecification ();
  m_convergenceSublayerParam = sf.GetConvergenceSublayerParam ();
  m_unsolicitedGrantInterval = sf.GetUnsolicitedGrantInterval ();
  m_unsolicitedPollingInterval = sf.GetUnsolicitedPollingInterval ();
  m_direction = sf.GetDirection ();
  m_isMulticast = sf.GetIsMulticast ();
  m_modulationType = sf.GetModulation ();
}

WranServiceFlow::WranServiceFlow (const WranServiceFlow & sf)
{
  m_sfid = sf.GetSfid ();
  m_serviceClassName = sf.GetServiceClassName ();
  m_qosParamSetType = sf.GetQosParamSetType ();
  m_trafficPriority = sf.GetTrafficPriority ();
  m_maxSustainedTrafficRate = sf.GetMaxSustainedTrafficRate ();
  m_maxTrafficBurst = sf.GetMaxTrafficBurst ();
  m_minReservedTrafficRate = sf.GetMinReservedTrafficRate ();
  m_minTolerableTrafficRate = sf.GetMinTolerableTrafficRate ();
  m_schedulingType = sf.GetServiceSchedulingType ();
  m_requestTransmissionPolicy = sf.GetRequestTransmissionPolicy ();
  m_toleratedJitter = sf.GetToleratedJitter ();
  m_maximumLatency = sf.GetMaximumLatency ();
  m_fixedversusVariableSduIndicator = sf.GetFixedversusVariableSduIndicator ();
  m_sduSize = sf.GetSduSize ();
  m_targetSAID = sf.GetTargetSAID ();
  m_arqEnable = sf.GetArqEnable ();
  m_arqWindowSize = sf.GetArqWindowSize ();
  m_arqRetryTimeoutTx = sf.GetArqRetryTimeoutTx ();
  m_arqRetryTimeoutRx = sf.GetArqRetryTimeoutRx ();
  m_csSpecification = sf.GetCsSpecification ();
  m_convergenceSublayerParam = sf.GetConvergenceSublayerParam ();
  m_unsolicitedGrantInterval = sf.GetUnsolicitedGrantInterval ();
  m_unsolicitedPollingInterval = sf.GetUnsolicitedPollingInterval ();
  m_direction = sf.GetDirection ();
  m_type = sf.GetType ();
  m_connection = sf.GetConnection ();
  m_isEnabled = sf.GetIsEnabled ();
  m_record = new WranServiceFlowRecord ();
  (*m_record) = (*sf.GetRecord ());
  m_isMulticast = sf.GetIsMulticast ();
  m_modulationType = sf.GetModulation ();
}

WranServiceFlow &
WranServiceFlow::operator = (WranServiceFlow const& o)
{

  m_sfid = o.GetSfid ();
  m_serviceClassName = o.GetServiceClassName ();
  m_qosParamSetType = o.GetQosParamSetType ();
  m_trafficPriority = o.GetTrafficPriority ();
  m_maxSustainedTrafficRate = o.GetMaxSustainedTrafficRate ();
  m_maxTrafficBurst = o.GetMaxTrafficBurst ();
  m_minReservedTrafficRate = o.GetMinReservedTrafficRate ();
  m_minTolerableTrafficRate = o.GetMinTolerableTrafficRate ();
  m_schedulingType = o.GetServiceSchedulingType ();
  m_requestTransmissionPolicy = o.GetRequestTransmissionPolicy ();
  m_toleratedJitter = o.GetToleratedJitter ();
  m_maximumLatency = o.GetMaximumLatency ();
  m_fixedversusVariableSduIndicator = o.GetFixedversusVariableSduIndicator ();
  m_sduSize = o.GetSduSize ();
  m_targetSAID = o.GetTargetSAID ();
  m_arqEnable = o.GetArqEnable ();
  m_arqWindowSize = o.GetArqWindowSize ();
  m_arqRetryTimeoutTx = o.GetArqRetryTimeoutTx ();
  m_arqRetryTimeoutRx = o.GetArqRetryTimeoutRx ();
  m_csSpecification = o.GetCsSpecification ();
  m_convergenceSublayerParam = o.GetConvergenceSublayerParam ();
  m_unsolicitedGrantInterval = o.GetUnsolicitedGrantInterval ();
  m_unsolicitedPollingInterval = o.GetUnsolicitedPollingInterval ();
  m_direction = o.GetDirection ();
  m_type = o.GetType ();
  m_connection = o.GetConnection ();
  m_isEnabled = o.GetIsEnabled ();
  m_isMulticast = o.GetIsMulticast ();
  m_modulationType = o.GetModulation ();
  if (m_record != 0)
    {
      delete m_record;
    }

  m_record = new WranServiceFlowRecord ();

  (*m_record) = (*o.GetRecord ());
  return *this;
}

char*
WranServiceFlow::GetSchedulingTypeStr (void) const
{
  switch (m_schedulingType)
    {
    case SF_TYPE_UGS:
      return (char*) "UGS";
      break;
    case SF_TYPE_RTPS:
      return (char*) "rtPS";
      break;
    case SF_TYPE_NRTPS:
      return (char*) "nrtPS";
      break;
    case SF_TYPE_BE:
      return (char*) "BE";
      break;
    default:
      NS_FATAL_ERROR ("Invalid scheduling type");
    }
  return 0;
}

WranTlv
WranServiceFlow::ToWranTlv (void) const
{
  SfWranVectorTlvValue tmpSfVector;
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::SFID, 4, WranU32TlvValue (m_sfid)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::CID, 2, WranU16TlvValue (GetCid ())));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::QoS_Parameter_Set_Type, 1, WranU8TlvValue (m_qosParamSetType)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Traffic_Priority, 1, WranU8TlvValue (m_trafficPriority)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Maximum_Sustained_Traffic_Rate, 4, WranU32TlvValue (m_maxSustainedTrafficRate)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Maximum_Traffic_Burst, 4, WranU32TlvValue (m_maxTrafficBurst)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Minimum_Reserved_Traffic_Rate, 4, WranU32TlvValue (m_minReservedTrafficRate)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Minimum_Tolerable_Traffic_Rate, 4, WranU32TlvValue (m_minTolerableTrafficRate)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Service_Flow_Scheduling_Type, 1, WranU8TlvValue (m_schedulingType)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Request_Transmission_Policy, 4, WranU32TlvValue (m_requestTransmissionPolicy)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Tolerated_Jitter, 4, WranU32TlvValue (m_toleratedJitter)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Maximum_Latency, 4, WranU32TlvValue (m_maximumLatency)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Fixed_length_versus_Variable_length_SDU_Indicator,
                        1,
                        WranU8TlvValue (m_fixedversusVariableSduIndicator)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::SDU_Size, 1, WranU8TlvValue (m_sduSize)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::Target_SAID, 2, WranU16TlvValue (m_targetSAID)));
  tmpSfVector.Add (WranTlv (SfWranVectorTlvValue::CS_Specification, 1, WranU8TlvValue (m_csSpecification)));
  tmpSfVector.Add (m_convergenceSublayerParam.ToWranTlv ());
  if (m_direction == SF_DIRECTION_UP)
    {
      return WranTlv (WranTlv::UPLINK_SERVICE_FLOW, tmpSfVector.GetSerializedSize (), tmpSfVector);
    }
  else
    {
      return WranTlv (WranTlv::DOWNLINK_SERVICE_FLOW, tmpSfVector.GetSerializedSize (), tmpSfVector);
    }
}

bool
WranServiceFlow::CheckClassifierMatch (Ipv4Address srcAddress,
                                   Ipv4Address dstAddress,
                                   uint16_t srcPort,
                                   uint16_t dstPort,
                                   uint8_t proto) const
{
  return m_convergenceSublayerParam.GetPacketClassifierRule ().CheckMatch (srcAddress,
                                                                           dstAddress,
                                                                           srcPort,
                                                                           dstPort,
                                                                           proto);
}
} // namespace ns3
