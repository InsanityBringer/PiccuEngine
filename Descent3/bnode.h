/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef BNODE_H_
#define BNODE_H_

#include "vecmat_external.h"

#define MAX_BNODE_SIZE 20.0f
// Not bigger than 127 - char bnode - in portal struct
#define MAX_BNODES_PER_ROOM 127

struct bn_edge
{
	short end_room;
	char end_index;
		
	short flags;
	short cost;
	
	float max_rad;
};

struct bn_node
{
	vector pos;
	int num_edges;
	bn_edge *edges;
};

struct bn_list
{
	int num_nodes;
	bn_node *nodes;
};

struct room;

extern void BNode_FreeRoom(room *rp);
extern void BNode_ClearBNodeInfo(void);

bn_list *BNode_GetBNListPtr(int roomnum, bool f_in_load_level = false);

extern bn_list BNode_terrain_list[8];
extern bool BNode_allocated;
extern bool BNode_verified;
void BNode_RemapTerrainRooms(int old_hri, int new_hri);

extern int BNode_Path[MAX_BNODES_PER_ROOM];
extern int BNode_PathNumNodes;
extern bool BNode_FindPath(int start_room, int i, int j, float rad);
extern int BNode_FindDirLocalVisibleBNode(int roomnum, vector *pos, vector *fvec, float rad);
extern int BNode_FindClosestLocalVisibleBNode(int roomnum, vector *pos, float rad);

#endif
