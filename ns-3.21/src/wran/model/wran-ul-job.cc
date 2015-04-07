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

#include <stdint.h>
#include "wran-ul-job.h"

namespace ns3 {

WranUlJob::WranUlJob (void) : m_deadline (Seconds (0)), m_size (0)
{
}

WranUlJob::~WranUlJob (void)
{
}

WranSSRecord*
WranUlJob::GetSsRecord (void)
{
  return m_ssRecord;
}
void
WranUlJob::SetSsRecord (WranSSRecord* ssRecord)
{
  m_ssRecord = ssRecord;
}

enum
WranServiceFlow::SchedulingType WranUlJob::GetSchedulingType (void)
{
  return m_schedulingType;
}

void
WranUlJob::SetSchedulingType (WranServiceFlow::SchedulingType schedulingType)
{
  m_schedulingType = schedulingType;
}

ReqType
WranUlJob::GetType (void)
{
  return m_type;
}

void
WranUlJob::SetType (ReqType type)
{
  m_type = type;
}

WranServiceFlow *
WranUlJob::GetWranServiceFlow (void)
{
  return m_serviceFlow;
}

void
WranUlJob::SetWranServiceFlow (WranServiceFlow *serviceFlow)
{
  m_serviceFlow = serviceFlow;
}

Time
WranUlJob::GetReleaseTime (void)
{
  return m_releaseTime;
}

void
WranUlJob::SetReleaseTime (Time releaseTime)
{
  m_releaseTime = releaseTime;
}

Time
WranUlJob::GetPeriod (void)
{
  return m_period;
}
void
WranUlJob::SetPeriod (Time period)
{
  m_period = period;
}

Time
WranUlJob::GetDeadline (void)
{
  return m_deadline;
}
void
WranUlJob::SetDeadline (Time deadline)
{
  m_deadline = deadline;
}

uint32_t
WranUlJob::GetSize (void)
{
  return m_size;
}

void
WranUlJob::SetSize (uint32_t size)
{
  m_size = size;
}

bool operator == (const WranUlJob &a, const WranUlJob &b)
{
  WranUlJob A = a;
  WranUlJob B = b;

  if ((A.GetWranServiceFlow () == B.GetWranServiceFlow ()) && (A.GetSsRecord () == B.GetSsRecord ()))
    {
      return true;
    }
  return false;
}

PriorityWranUlJob::PriorityWranUlJob (void)
{
}

int
PriorityWranUlJob::GetPriority (void)
{
  return m_priority;
}

void
PriorityWranUlJob::SetPriority (int priority)
{
  m_priority = priority;
}

Ptr<WranUlJob>
PriorityWranUlJob::GetWranUlJob (void)
{
  return m_job;
}
void
PriorityWranUlJob::SetWranUlJob (Ptr<WranUlJob> job)
{
  m_job = job;
}

} // namespace ns3
