/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *  Copyright (c) 2009 Green Network Research Group
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
 *          
 *
 */
#include "wran-tlv.h"
#include "wran-cs-parameters.h"

namespace ns3 {
CsParameters::CsParameters ()
{
  m_classifierDscAction = CsParameters::ADD;
}
CsParameters::~CsParameters ()
{

}

CsParameters::CsParameters (WranTlv wranTlv)
{
  NS_ASSERT_MSG (wranTlv.GetType () == SfWranVectorTlvValue::IPV4_CS_Parameters,
                 "Invalid WranTlv");
  CsParamWranVectorTlvValue* param = ((CsParamWranVectorTlvValue*)(wranTlv.PeekValue ()));

  for (std::vector<WranTlv*>::const_iterator iter = param->Begin (); iter
       != param->End (); ++iter)
    {
      switch ((*iter)->GetType ())
        {
        case CsParamWranVectorTlvValue::Classifier_DSC_Action:
          {
            m_classifierDscAction
              = (enum CsParameters::Action)((WranU8TlvValue*)((*iter)->PeekValue ()))->GetValue ();
            break;
          }
        case CsParamWranVectorTlvValue::Packet_Classification_Rule:
          {
            m_packetClassifierRule
              = WranIpcsClassifierRecord (*(*iter));
            break;
          }
        }
    }
}

CsParameters::CsParameters (enum CsParameters::Action classifierDscAction,
                            WranIpcsClassifierRecord classifier)
{
  m_classifierDscAction = classifierDscAction;
  m_packetClassifierRule = classifier;
}
void
CsParameters::SetClassifierDscAction (enum CsParameters::Action action)
{
  m_classifierDscAction = action;
}
void
CsParameters::SetPacketClassifierRule (WranIpcsClassifierRecord packetClassifierRule)
{
  m_packetClassifierRule = packetClassifierRule;
}
enum CsParameters::Action
CsParameters::GetClassifierDscAction (void) const
{
  return m_classifierDscAction;
}
WranIpcsClassifierRecord
CsParameters::GetPacketClassifierRule (void) const
{
  return m_packetClassifierRule;
}
WranTlv
CsParameters::ToWranTlv (void) const
{
  CsParamWranVectorTlvValue tmp;
  tmp.Add (WranTlv (CsParamWranVectorTlvValue::Classifier_DSC_Action,1, WranU8TlvValue (m_classifierDscAction)));
  tmp.Add (m_packetClassifierRule.ToWranTlv ());
  return WranTlv (SfWranVectorTlvValue::IPV4_CS_Parameters, tmp.GetSerializedSize (), tmp);
}
}
