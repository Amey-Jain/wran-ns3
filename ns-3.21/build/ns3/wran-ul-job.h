/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c)
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
 * Author:  Juliana Freitag Borin, Flavio Kubota and Nelson L.
 * S. da Fonseca - wrangroup@lrc.ic.unicamp.br
 */

#ifndef UL_JOB_H
#define UL_JOB_H

#include <stdint.h>
#include "ns3/header.h"
#include "wran-ss-record.h"
#include "wran-service-flow.h"
#include "wran-service-flow-record.h"

namespace ns3 {

class WranSSRecord;
class WranServiceFlow;

enum ReqType
{
  DATA, UNICAST_POLLING
};

/**
 * \ingroup wran
 * \brief this class implements a structure to compute the priority of service flows
 */
class WranUlJob : public Object
{
public:
  enum JobPriority
  {
    LOW, INTERMEDIATE, HIGH
  };
  WranUlJob (void);
  virtual ~WranUlJob (void);
  WranSSRecord *
  GetSsRecord (void);
  void SetSsRecord (WranSSRecord* ssRecord);
  enum WranServiceFlow::SchedulingType GetSchedulingType (void);
  void SetSchedulingType (WranServiceFlow::SchedulingType schedulingType);
  WranServiceFlow *
  GetWranServiceFlow (void);
  void SetWranServiceFlow (WranServiceFlow *serviceFlow);

  ReqType GetType (void);
  void SetType (ReqType type);

  Time GetReleaseTime (void);
  void SetReleaseTime (Time releaseTime);

  Time GetPeriod (void);
  void SetPeriod (Time period);

  Time GetDeadline (void);
  void SetDeadline (Time deadline);

  uint32_t GetSize (void);
  void SetSize (uint32_t size);

private:
  friend bool operator == (const WranUlJob &a, const WranUlJob &b);

  Time m_releaseTime; /* The time after which the job can be processed*/
  Time m_period; /* For periodic jobs*/
  Time m_deadline; /* Request should be satisfied by this time */
  uint32_t m_size; /* Number of minislots requested */
  enum WranServiceFlow::SchedulingType m_schedulingType; /* Scheduling type of flow */

  uint8_t m_flag; /* To delete or not..*/
  uint8_t m_retryCount;
  double m_ugsJitter; /* The jitter in the grant, valid only for UGS flows */
  int m_jitterSamples;
  double m_last_jitterCalTime; /* Last time avg jitter was calculated */

  WranSSRecord *m_ssRecord; /* Pointer to WranSSRecord */

  ReqType m_type; /* Type of request, DATA or Unicast req slots */
  WranServiceFlow *m_serviceFlow;

};


class PriorityWranUlJob : public Object
{

  /**
   * \brief this class implements an auxiliar struct to compute the priority of the rtPS and nrtPS in
   * the intermediate queue
   */
public:
  PriorityWranUlJob ();
  int GetPriority (void);
  void SetPriority (int priority);

  Ptr<WranUlJob>
  GetWranUlJob (void);
  void SetWranUlJob (Ptr<WranUlJob> job);

private:
  int m_priority;
  Ptr<WranUlJob> m_job;
};

struct SortProcess : public std::binary_function<PriorityWranUlJob*, PriorityWranUlJob*, bool>
{
  bool operator () (PriorityWranUlJob& left, PriorityWranUlJob& right) const
  { // return true if left is logically less then right for given comparison
    if (left.GetPriority () < right.GetPriority ())
      {
        return true;
      }
    else if (left.GetPriority () == right.GetPriority ())
      {
        int32_t leftBacklogged = left.GetWranUlJob ()->GetWranServiceFlow ()->GetRecord ()->GetBacklogged ();
        int32_t rightBacklogged = left.GetWranUlJob ()->GetWranServiceFlow ()->GetRecord ()->GetBacklogged ();
        if (leftBacklogged <= rightBacklogged)
          {
            return true;
          }
        else
          {
            return false;
          }
      }
    else
      {
        return false;
      }
  }
};

struct SortProcessPtr: public std::binary_function< Ptr<PriorityWranUlJob>, Ptr<PriorityWranUlJob>, bool>
{
  bool operator () (Ptr<PriorityWranUlJob>& left, Ptr<PriorityWranUlJob>& right) const
  { //return true if left is logically less then right for given comparison
    if (left->GetPriority () < right->GetPriority ())
      {
        return true;
      }
    else if (left->GetPriority () == right->GetPriority ())
      {
        int32_t leftBacklogged = left->GetWranUlJob ()->GetWranServiceFlow ()->GetRecord ()->GetBacklogged ();
        int32_t rightBacklogged = left->GetWranUlJob ()->GetWranServiceFlow ()->GetRecord ()->GetBacklogged ();
        if (leftBacklogged <= rightBacklogged)
          {
            return true;
          }
        else
          {
            return false;
          }
      }
    else
      {
        return false;
      }
  }
};


} // namespace ns3

#endif /* UL_JOB_H */
