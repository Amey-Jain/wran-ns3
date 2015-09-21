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
 */

#ifndef MAXIMUM_MATCHING_H
#define MAXIMUM_MATCHING_H

#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include "ns3/object.h"
#include "ns3/core-module.h"
#include "ns3/tag.h"
#include "ns3/log.h"

#define MX 55
#define INF 100000000.0
#define mmax(a,b) (((a)>(b))?(a):(b))
#define mmin(a,b) (((a)>(b))?(b):(a))

namespace ns3 {

class MaximumMatching : public Object {

public:
	static TypeId GetTypeId (void);
	MaximumMatching (void);
	MaximumMatching (int totalVertex, int totalLeftVertex);
	~MaximumMatching ();

	void SetCost(int x, int y, double value);
	void SetN(int totalVertex);
	int GetN(void);
	void SetM(int totalLeftVertex);
	int GetM(void);
	double Hungarian(void);

private:
	void InitLabels();
	void UpdateLabels();
	void AddToTree(int x, int prevx);
	void Augment();


	double cost[MX][MX];
	int n, m, max_match;
	double lx[MX], ly[MX];
	int xy[MX], yx[MX];
	bool S[MX], T[MX];
	double slack[MX];
	int slackx[MX], previous[MX];
};

} // namespace ns3

#endif /* MAXIMUM_MATCHING_H */
