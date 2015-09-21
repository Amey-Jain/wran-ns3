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
#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include "maximum-matching.h"
#include "ns3/simulator.h"
#include "ns3/pointer.h"
#include "ns3/enum.h"
#include "ns3/uinteger.h"

NS_LOG_COMPONENT_DEFINE ("MaximumMatchingAlgorithm");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (MaximumMatching);

TypeId
MaximumMatching::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::MaximumMatching")
	    .SetParent<Object> ()
	    .AddConstructor<MaximumMatching> ()
	  ;
	  return tid;
}

MaximumMatching::MaximumMatching (void)
{
	n = 0;
	m = 0;
	max_match = 0;
	InitLabels();
}

MaximumMatching::MaximumMatching (int totalVertex, int totalLeftVertex)
{
	n = totalVertex;
	m = totalLeftVertex;
	max_match = 0;
	InitLabels();
}

MaximumMatching::~MaximumMatching (void)
{

}

void
MaximumMatching::SetCost(int x, int y, double value)
{
	cost[x][y] = value;
}

void
MaximumMatching::SetN(int totalVertex)
{
	n = totalVertex;
}

int
MaximumMatching::GetN(void)
{
	return n;
}

void
MaximumMatching::SetM(int totalLeftVertex)
{
	m = totalLeftVertex;
}

int
MaximumMatching::GetM(void)
{
	return m;
}

void MaximumMatching::InitLabels() {
	memset(lx, 0, sizeof(lx));
	memset(ly, 0, sizeof(ly));
	for (int x = 0; x < n; x++) for (int y = 0; y < n; y++) lx[x] = mmax(lx[x], cost[x][y]);
}

void MaximumMatching::UpdateLabels() {
	int x, y;
	double delta = INF;
	for (y = 0; y < n; y++) if (!T[y]) delta = mmin(delta, slack[y]);
	for (x = 0; x < n; x++) if (S[x]) lx[x] -= delta;
	for (y = 0; y < n; y++) if (T[y]) ly[y] += delta;
	for (y = 0; y < n; y++) if (!T[y]) slack[y] -= delta;
}

void MaximumMatching::AddToTree(int x, int prevx) {
	S[x] = true;
	previous[x] = prevx;
	for (int y = 0; y < n; y++)
	if (lx[x] + ly[y] - cost[x][y] < slack[y]) {
		slack[y] = lx[x] + ly[y] - cost[x][y];
		slackx[y] = x;
	}
}

void MaximumMatching::Augment() {
	if (max_match == n) return;
	int x, y, root;
	int q[MX], wr = 0, rd = 0;
	memset(S, false, sizeof(S));
	memset(T, false, sizeof(T));
	memset(previous, -1, sizeof(previous));
	for (x = 0; x < n; x++) if (xy[x] == -1) {
		q[wr++] = root = x;
		previous[x] = -2;
		S[x] = true;
		break;
	}
	for (y = 0; y < n; y++) {
		slack[y] = lx[root] + ly[y] - cost[root][y];
		slackx[y] = root;
	}
	while (true) {
		while (rd < wr) {
			x = q[rd++];
			for (y = 0; y < n; y++)
			if (cost[x][y] == lx[x] + ly[y] && !T[y]) {
				if (yx[y] == -1) break;
				T[y] = true;
				q[wr++] = yx[y];
				AddToTree(yx[y], x);
			}
			if (y < n) break;
		}
		if (y < n) break;
		UpdateLabels();
		wr = rd = 0;
		for (y = 0; y < n; y++) if (!T[y] && fabs(slack[y]-0.0) < (1e-9)) {
			if (yx[y] == -1) {
				x = slackx[y];
				break;
			}
			else {
				T[y] = true;
				if (!S[yx[y]]) {
					q[wr++] = yx[y];
					AddToTree(yx[y], slackx[y]);
				}
			}
		}
		if (y < n) break;
	}
	if (y < n) {
		max_match++;
		for (int cx = x, cy = y, ty; cx != -2; cx = previous[cx], cy = ty) {
			ty = xy[cx];
			yx[cy] = cx;
			xy[cx] = cy;
		}
		Augment();
	}
}

double MaximumMatching::Hungarian(void)
{
	double ret = 0;
	max_match = 0;
	memset(xy, -1, sizeof(xy));
	memset(yx, -1, sizeof(yx));
	InitLabels();
	Augment();
	for (int x = 0; x < n; x++) ret += cost[x][xy[x]];
	return ret;
}

}// namespace ns3
