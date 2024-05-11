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

#ifndef BSP_H
#define BSP_H

#include "list.h"
#include "vecmat.h"
#include "CFILE.H"

#define BSP_IN_FRONT		1
#define BSP_BEHIND		2
#define BSP_ON_PLANE		3
#define BSP_SPANNING		4
#define BSP_COINCIDENT	5

#define BSP_EPSILON		.00005f

#define BSP_NODE	0
#define BSP_EMPTY_LEAF	1
#define BSP_SOLID_LEAF	2

struct bspplane
{
	float a,b,c,d;
	ubyte used;
};

struct bsppolygon
{
	vector *verts;
	int nv;
	bspplane plane;

	short roomnum;
	short facenum;
	sbyte subnum;

	int color;

};

struct bspnode 
{
	ubyte type;
	bspplane plane;
	ushort node_facenum;
	ushort node_roomnum;
	sbyte node_subnum;
	
	bspnode  *front;
	bspnode  *back;

	list *polylist;
	int num_polys;
};

struct bsptree 
{
	list        *vertlist;
	list		*polylist;
	bspnode     *root;
};

// Builds a bsp tree for the indoor rooms
void BuildBSPTree ();

// Runs a ray through the bsp tree
// Returns true if a ray is occludes
int BSPRayOccluded(vector *start, vector *end, bspnode *node);
int BSPReportStatus(vector *start, bspnode *node);

// Walks the BSP tree and frees up any nodes/polygons that we might be using
void DestroyBSPTree (bsptree *tree);

// Saves and BSP node to an open file and recurses with the nodes children
void SaveBSPNode (CFILE *outfile,bspnode *node);

// Loads a bsp node from an open file and recurses with its children
void LoadBSPNode (CFILE *infile,bspnode **node);

// Initializes some variables for the indoor bsp tree
void InitDefaultBSP ();

// Destroy indoor bsp tree
void DestroyDefaultBSPTree ();

// Builds a bsp tree for a single room
void BuildSingleBSPTree (int roomnum);

// Reports the current mine's checksum
int BSPGetMineChecksum();

extern bsptree MineBSP;
extern int BSPChecksum;
extern ubyte BSP_initted;
extern ubyte UseBSP;

#endif
