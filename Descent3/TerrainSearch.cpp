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

#include "terrain.h"
#include "3d.h"
#include "mono.h"
#include "vecmat.h"
#include "pserror.h"
#include "pstypes.h"
#include "descent.h"
#include "game.h"
#include "gameloop.h"
#include <memory.h>
#include <stdlib.h>
#include <search.h>
#include "config.h"
#include "dedicated_server.h"

int EvaluateBlock(int x, int z, int lod);

ushort TS_FrameCount = 0xFFFF;

int SearchQuadTree(int x1, int y1, int x2, int y2, int dir, int* ccount);
int GlobalTransCount = 0;
int TotalDepth;

// LOD shutoff stuff
#define MAX_LODOFFS	100
lodoff LODOffs[MAX_LODOFFS];
int Num_lodoffs = 0;

// Render the terrain as flat?
ubyte Flat_terrain = 0;

// Variables for LOD engine
float ViewDistance1 = .68f, ViewDistance2 = 1.0f;
float TS_Lambda, TS_SimplifyCondition;

matrix* TS_View_matrix;
vector* TS_View_position;

vector TS_PreRows[TERRAIN_DEPTH];
vector TS_FVectorAdd, TS_RVectorAdd;

ubyte Terrain_y_flags[256];
vector Terrain_y_cache[256];

ushort LOD_sort_bucket[MAX_TERRAIN_LOD][MAX_CELLS_TO_RENDER];
ushort LOD_sort_num[MAX_TERRAIN_LOD];

// Variable to determine if we're in editor or game
extern function_mode View_mode;
extern ubyte Terrain_from_mine;

vector TJoint_VectorAddZ;
vector TJoint_VectorAddX;


// Since our terrain points increment in finite steps we can rotate 2 points (x,z)
// just add them to a base point to get the final rotated point
void PreRotateTerrain()
{
	vector rvec = { 1 * TERRAIN_SIZE,0,0 };
	vector fvec = { 0,0,1 * TERRAIN_SIZE };

	vector tj_rvec = { 4,0,0 };
	vector tj_fvec = { 0,0,4 };

	TS_FVectorAdd = fvec * *TS_View_matrix;
	TS_RVectorAdd = rvec * *TS_View_matrix;

	vector start_vec = { 0.0f, 0.0f, 0.0f };
	vector temp_vec = start_vec - *TS_View_position;
	start_vec = temp_vec * *TS_View_matrix;

	TS_PreRows[0] = start_vec;

	int i;
	for (i = 1; i < TERRAIN_DEPTH; i++)
	{
		TS_PreRows[i] = TS_PreRows[i - 1] + TS_FVectorAdd;
	}
}

inline vector* GetDYVector(int h)
{
	vector* dyp;

	dyp = &Terrain_y_cache[h];

	if (!Terrain_y_flags[h])
	{
		vector up_vector = { 0,h * TERRAIN_HEIGHT_INCREMENT,0 };

		*dyp = up_vector * *TS_View_matrix;

		Terrain_y_flags[h] = 1;
	}

	return dyp;
}

inline void GetPreRotatedPointFast(vector* dest, int x, int z, int yvalue)
{
	ASSERT(yvalue >= 0 && yvalue <= 255);

	// If the terrain is supposed to be flat, bash this to zero
#ifdef EDITOR
	if (Flat_terrain)
	{
		yvalue = 0;
	}
#endif

	* dest = TS_PreRows[z];
	*dest += TS_RVectorAdd * x;
	*dest += *GetDYVector(yvalue);
}


void GetPreRotatedPoint(g3Point* dest, int x, int z, int yvalue)
{
	ASSERT(yvalue >= 0 && yvalue <= 255);

	// If the terrain is supposed to be flat, bash this to zero
#ifdef EDITOR
	if (Flat_terrain)
	{
		yvalue = 0;
	}
#endif

	dest->p3_vec = TS_PreRows[z];
	dest->p3_vec += TS_RVectorAdd * x;
	dest->p3_vec += *GetDYVector(yvalue);
}

// Gets a pre-rotated point that does not fall exactly on one of our 255 height values
void GetSpecialRotatedPoint(g3Point* dest, int x, int z, float yvalue)
{
	if (yvalue < 0)
		yvalue = 0;
	if (yvalue > MAX_TERRAIN_HEIGHT)
		yvalue = MAX_TERRAIN_HEIGHT;

	vector up_vector = { 0,yvalue,0 };
	vector dyp = up_vector * *TS_View_matrix;

	dest->p3_vec = TS_PreRows[z];
	dest->p3_vec += TS_RVectorAdd * x;
	dest->p3_vec += dyp;
}

void Terrain_start_frame(vector* eye, matrix* view_orient)
{
	TS_View_position = eye;
	TS_View_matrix = view_orient;

	// Set up variables for LOD switching
	int width, height;
	rend_GetProjectionParameters(&width, &height);
	TS_Lambda = (float)height / (2 * ViewDistance1 * Render_zoom);

#ifndef NEWEDITOR
	TS_SimplifyCondition = Detail_settings.Pixel_error / (TS_Lambda * ViewDistance2);
#else
	TS_SimplifyCondition = 10.0f / (TS_Lambda * ViewDistance2);
#endif
	TS_SimplifyCondition *= TS_SimplifyCondition;

	// Increment TS_framecount before searching...
	TS_FrameCount++;

	if (TS_FrameCount == 0)
	{
		TS_FrameCount = 1;
	}

	memset(Terrain_y_flags, 0, 256);
	memset(LOD_sort_num, 0, MAX_TERRAIN_LOD * sizeof(ushort));

	PreRotateTerrain();
}

// Sorts our visible terrain blocks by lod level.  The lower resolution blocks
// go first - this prevents cracking

int LodSortingFunction(const terrain_render_info* a, terrain_render_info* b)
{
	return a->lod - b->lod;
}

void SortLodLevels(int count)
{
	qsort(Terrain_list, count, sizeof(terrain_render_info), (int (*)(const void*, const void*)) LodSortingFunction);
}

// returns number of cells in visible terrain
int GetVisibleTerrain(vector* eye, matrix* view_orient)
{
	int cellcount = 0;
	int i, t, count;

	GlobalTransCount = 0;
	TotalDepth = 0;

	Terrain_start_frame(eye, view_orient);

	SearchQuadTree(0, 0, TERRAIN_WIDTH, TERRAIN_DEPTH, 0, &cellcount);

	// Make sure lower levels of detail come first
	for (count = 0, i = 0; i < MAX_TERRAIN_LOD; i++)
	{
		for (t = 0; t < LOD_sort_num[i]; t++, count++)
		{
			Terrain_list[count].lod = i;
			Terrain_list[count].segment = LOD_sort_bucket[i][t];
		}
	}

	// Increment TS_framecount after searching because we share Terrain_rotate_list
	// with the rendering code
	TS_FrameCount++;

	if (TS_FrameCount == 0)
		TS_FrameCount = 1;

	return cellcount;
}

__inline void CheckCellOccupancy(int x, int y, int* ccount, ubyte lod)
{
	int n, simplemul, i;


	if (*ccount >= MAX_CELLS_TO_RENDER)
	{
		mprintf((0, "Trying to render too many cells!  Cell limit=%d\n", MAX_CELLS_TO_RENDER));
#ifndef NEWEDITOR
		//Detail_settings.Terrain_render_distance = 80.0 * TERRAIN_SIZE;
#endif
		return;
	}


	n = y * TERRAIN_WIDTH + x;


	if (Show_invisible_terrain == 0 && lod == MAX_TERRAIN_LOD - 1 && Terrain_seg[n].flags & TF_INVISIBLE)
		return;

	(*ccount)++;

	// Fill in our presorter
	LOD_sort_bucket[lod][LOD_sort_num[lod]] = n;
	LOD_sort_num[lod]++;

	// Fill in the join map so we know what edges touch what edges
	if (lod != MAX_TERRAIN_LOD - 1)
	{
		simplemul = 1 << ((MAX_TERRAIN_LOD - 1) - lod);


		for (i = 0; i < simplemul; i++)
		{
			// Fill bottom edge
			TerrainJoinMap[y * TERRAIN_WIDTH + x + i] = lod;

			// Fill top edge
			TerrainJoinMap[(y + simplemul - 1) * TERRAIN_WIDTH + x + i] = lod;

			// Fill left edge
			TerrainJoinMap[(y + i) * TERRAIN_WIDTH + x] = lod;

			// Fill right edge
			TerrainJoinMap[(y + i) * TERRAIN_WIDTH + x + (simplemul - 1)] = lod;
		}
	}
	else
		TerrainJoinMap[y * TERRAIN_WIDTH + x] = lod;
}


#ifdef EDITOR
extern int Tracing_drops, Drop_seg;
#endif

#define PUSH_STACK_TREE(a1,b1,a2,b2,l) {stack_x1[si]=a1;stack_y1[si]=b1;stack_y2[si]=b2;stack_x2[si]=a2; stack_level[si]=l; si++;}
#define POP_STACK_TREE()	{si--; x1=stack_x1[si];x2=stack_x2[si];y1=stack_y1[si];y2=stack_y2[si]; level=stack_level[si];}

int SearchQuadTree(int x1, int y1, int x2, int y2, int dir, int* ccount)
{
	int n[4], x[4], z[4];
	int i, first;
	int anded;
	int answer;
	int si = 0, level;
	int stack_x1[200];
	int stack_y1[200];
	int stack_x2[200];
	int stack_y2[200];
	int stack_level[200];
	float testdist = VisibleTerrainZ;
	ubyte ymin_int[4], ymax_int[4];
	int close;
	ubyte same_side;
	ubyte check_portal = 0;
	g3Point* pnt;
	int use_occlusion = 0;
	int src_occlusion_index;

	if ((Terrain_checksum + 1) == Terrain_occlusion_checksum && !Terrain_from_mine)
	{
		use_occlusion = 1;
		int oz = (Viewer_object->pos.z / TERRAIN_SIZE) / OCCLUSION_SIZE;
		int ox = (Viewer_object->pos.x / TERRAIN_SIZE) / OCCLUSION_SIZE;

		if (oz < 0 || oz >= OCCLUSION_SIZE || ox < 0 || ox >= OCCLUSION_SIZE)
			use_occlusion = 0;

		src_occlusion_index = oz * OCCLUSION_SIZE + ox;
	}

	PUSH_STACK_TREE(x1, y1, x2, y2, 0);

	while (si > 0)
	{
		TotalDepth++;

		POP_STACK_TREE();
		ASSERT((x2 - x1) == (y2 - y1));


		if ((x2 - x1) == 16)
		{
			if (use_occlusion)
			{
				int dest_occlusion_index = ((y1 / OCCLUSION_SIZE) * OCCLUSION_SIZE);
				dest_occlusion_index += x1 / OCCLUSION_SIZE;

				int occ_byte = dest_occlusion_index / 8;
				int occ_bit = dest_occlusion_index % 8;

				if (!(Terrain_occlusion_map[src_occlusion_index][occ_byte] & (1 << occ_bit)))
					continue;
			}
			check_portal = 1;
		}

		else if ((x2 - x1) == 8)
		{
			answer = EvaluateBlock(x1, y1, MAX_TERRAIN_LOD - 4);
			if (answer == -1)
				continue;
			else if (answer == 1)
			{
				CheckCellOccupancy(x1, y1, ccount, MAX_TERRAIN_LOD - 4);
				continue;
			}
			check_portal = 1;
		}

		else if ((x2 - x1) == 4)
		{
			answer = EvaluateBlock(x1, y1, MAX_TERRAIN_LOD - 3);
			if (answer == -1)
				continue;
			else if (answer == 1)
			{
				CheckCellOccupancy(x1, y1, ccount, MAX_TERRAIN_LOD - 3);
				continue;
			}
			check_portal = 0;
		}

		else if ((x2 - x1) == 2)
		{
			answer = EvaluateBlock(x1, y1, MAX_TERRAIN_LOD - 2);
			if (answer == -1)
				continue;
			else if (answer == 1)
			{
				CheckCellOccupancy(x1, y1, ccount, MAX_TERRAIN_LOD - 2);
				continue;
			}
			check_portal = 0;
		}

		if ((x2 - x1) <= 2)
		{
			CheckCellOccupancy(x1, y1, ccount, MAX_TERRAIN_LOD - 1);
			CheckCellOccupancy(x1, y1 + 1, ccount, MAX_TERRAIN_LOD - 1);
			CheckCellOccupancy(x1 + 1, y1 + 1, ccount, MAX_TERRAIN_LOD - 1);
			CheckCellOccupancy(x1 + 1, y1, ccount, MAX_TERRAIN_LOD - 1);

#if (defined(EDITOR) || defined(NEWEDITOR))
			if (*ccount >= MAX_CELLS_TO_RENDER)
				return 0;
#endif

			check_portal = 0;

			continue;
		}


		// Starts at lower left, goes clockwise
		n[0] = y1 * TERRAIN_WIDTH + x1;
		n[1] = y2 * TERRAIN_WIDTH + x1;
		n[2] = y2 * TERRAIN_WIDTH + x2;
		n[3] = y1 * TERRAIN_WIDTH + x2;

		x[0] = x[1] = x1;
		x[2] = x[3] = x2;
		z[0] = z[3] = y1;
		z[1] = z[2] = y2;

		if (x2 == TERRAIN_WIDTH)
		{
			x[2]--;
			x[3]--;

			n[2]--;
			n[3]--;

		}

		if (y2 == TERRAIN_DEPTH)
		{
			z[1]--;
			z[2]--;

			n[1] -= TERRAIN_WIDTH;
			n[2] -= TERRAIN_WIDTH;
		}

		close = 0;
		first = 1;

		anded = 0xff;
		same_side = 0xff;
		ubyte portal_and = 0xff;

		for (i = 0; i < 4; i++)
		{
			if (first)
			{
				int lx = x[i] >> (8 - level);
				int lz = z[i] >> (8 - level);

				int cell = (lz << level) + lx;
				ASSERT(level <= 6);

				ymin_int[i] = *(Terrain_min_height_int[level] + cell);
				ymax_int[i] = *(Terrain_max_height_int[level] + cell);
				first = 0;
			}

			pnt = &World_point_buffer[n[i]];

			// Do min height for region				
			GetPreRotatedPointFast(&pnt->p3_vec, x[i], z[i], ymin_int[0]);

			if (pnt->p3_vec.z >= 0)
				same_side &= 1;
			else
				same_side &= 2;

			if ((fabs(pnt->p3_vec.z) <= testdist))
				close = 1;

			anded &= g3_CodePoint(pnt);
			if (Check_terrain_portal && check_portal)
			{
				pnt->p3_flags &= ~PF_PROJECTED;
				// Automatically flag the ones behind us as visible
				if (pnt->p3_codes & CC_BEHIND)
					check_portal = 0;
				else
				{
					g3_ProjectPoint(pnt);
					portal_and &= CodeTerrainPoint(pnt);
				}
			}

			// Do max height for region
			GetPreRotatedPointFast(&pnt->p3_vec, x[i], z[i], ymax_int[0]);

			if (pnt->p3_vec.z >= 0)
				same_side &= 1;
			else
				same_side &= 2;

			if ((fabs(pnt->p3_vec.z) <= testdist))
				close = 1;

			anded &= g3_CodePoint(pnt);

			if (Check_terrain_portal && check_portal)
			{
				pnt->p3_flags &= ~PF_PROJECTED;
				// Automatically flag the ones behind us as visible
				if (pnt->p3_codes & CC_BEHIND)
					check_portal = 0;
				else
				{
					g3_ProjectPoint(pnt);
					portal_and &= CodeTerrainPoint(pnt);
				}
			}
		}

		if ((!anded && close) || (!anded && !close && !same_side))
		{
			// Recurse into this quadtree

			// Check to see if this is in our portal window
			if (Check_terrain_portal && check_portal && portal_and)
				continue;

			int midx = ((x2 - x1) / 2) + x1;
			int midy = ((y2 - y1) / 2) + y1;

			PUSH_STACK_TREE(x1, midy, midx, y2, level + 1);
			PUSH_STACK_TREE(midx, midy, x2, y2, level + 1);
			PUSH_STACK_TREE(midx, y1, x2, midy, level + 1);
			PUSH_STACK_TREE(x1, y1, midx, midy, level + 1);
		}
		else
		{
			/*#ifdef EDITOR
				if (Tracing_drops)
				{
					int dx=Drop_seg%TERRAIN_WIDTH;
					int dz=Drop_seg/TERRAIN_WIDTH;

					if (x1<=dx && dx<=x2 && y1<=dz && dz<=y2)
					{
						Tracing_drops=0;
						Int3();
					}

				}
			#endif*/
		}

	}
	return 0;
}


// Given a position, returns the terrain segment that that position is in/over
// returns -1 if not over terrain
int GetTerrainCellFromPos(vector* pos)
{
	int x = pos->x / TERRAIN_SIZE;
	int z = pos->z / TERRAIN_SIZE;

	if (x < 0 || x >= TERRAIN_WIDTH || z < 0 || z >= TERRAIN_DEPTH)
		return -1;

	return (z * TERRAIN_WIDTH + x);
}

int GetTerrainRoomFromPos(vector* pos)
{
	return MAKE_ROOMNUM(GetTerrainCellFromPos(pos));
}

// Computes the center of the segment in x,z and also sets y touching the ground
void ComputeTerrainSegmentCenter(vector* pos, int segnum)
{
	int segx = segnum % TERRAIN_WIDTH;
	int segz = segnum / TERRAIN_WIDTH;

	pos->x = (segx * TERRAIN_SIZE) + (TERRAIN_SIZE / 2);
	pos->z = (segz * TERRAIN_SIZE) + (TERRAIN_SIZE / 2);

	pos->y = GetTerrainGroundPoint(pos);
}

// Given an position, returns the terrain Y coord at that location
float GetTerrainGroundPoint(vector* pos, vector* normal)
{
	float y;
	vector pnt, norm;
	int t;
	int x, z;

	x = pos->x / TERRAIN_SIZE;
	z = pos->z / TERRAIN_SIZE;

	if (x < 0 || x >= TERRAIN_WIDTH || z < 0 || z >= TERRAIN_DEPTH)
		return (0);

	t = z * TERRAIN_WIDTH + x;

	pnt = *pos;
	pnt.x -= (x * TERRAIN_SIZE);
	pnt.z -= (z * TERRAIN_SIZE);

	if (pnt.x > pnt.z)
		norm = TerrainNormals[MAX_TERRAIN_LOD - 1][t].normal2;
	else
		norm = TerrainNormals[MAX_TERRAIN_LOD - 1][t].normal1;

	// Do plane equation on x,z and then interpolate y
	y = ((pnt.x * norm.x) + (pnt.z * norm.z)) / norm.y;
	y = -y;
	y += Terrain_seg[t].y;

	if (normal != NULL)
		*normal = norm;

	return (y);
}

int SimplifyVertexSlow(int x, int z, float delta)
{
	vector* eye = TS_View_position;
	g3Point p1, p2;

	p1.p3_codes = 0;
	GetPreRotatedPoint(&p1, x, z, Terrain_seg[z * TERRAIN_WIDTH + x].ypos);

	p2.p3_codes = 0;
	GetSpecialRotatedPoint(&p2, x, z, Terrain_seg[z * TERRAIN_WIDTH + x].y + delta);

	g3_ProjectPoint(&p1);
	g3_ProjectPoint(&p2);

	float len = ((p1.p3_sx - p2.p3_sx) * (p1.p3_sx - p2.p3_sx)) + ((p1.p3_sy - p2.p3_sy) * (p1.p3_sy - p2.p3_sy));

#ifndef NEWEDITOR
	if (len < (Detail_settings.Pixel_error * Detail_settings.Pixel_error))
#else
	if (len < (10.0f * 10.0f))
#endif
		return 1;

	return 0;
}

int SimplifyVertex(int x, int z, float delta)
{
	vector* eye = TS_View_position;
	vector vec;
	float inner;

	vec.x = x * TERRAIN_SIZE;
	vec.z = z * TERRAIN_SIZE;
	vec.y = Terrain_seg[z * TERRAIN_WIDTH + x].y;

	float delta_squared = delta * delta;
	float first_part, second_part;

	float ex_minus_vx_squared = (eye->x - vec.x) * (eye->x - vec.x);
	float ez_minus_vz_squared = (eye->z - vec.z) * (eye->z - vec.z);
	float ey_minus_vy_squared = (eye->y - vec.y) * (eye->y - vec.y);

	first_part = delta_squared * (ex_minus_vx_squared + ez_minus_vz_squared);

	inner = ex_minus_vx_squared + ez_minus_vz_squared + ey_minus_vy_squared;

	second_part = (TS_SimplifyCondition) * (inner * inner);

	if (first_part <= second_part)
		return 1;

	return 0;
}

// Returns 1 if the given block (specified by the upper left corder x,z) lod can
// be simplified
// Returns -1 if the block is invisible
// Returns 0 if not
int EvaluateBlock(int x, int z, int lod)
{
	float delta;
	int simplemul = 1 << ((MAX_TERRAIN_LOD - 1) - lod);

#if (defined(EDITOR) || defined(NEWEDITOR))
	if (View_mode == EDITOR_MODE && Editor_LOD_engine_off)
		return 0;
#endif

	if (View_mode != EDITOR_MODE && Terrain_LOD_engine_off)
		return 0;

	delta = TerrainDeltaBlocks[lod][((z / simplemul) * (TERRAIN_WIDTH / simplemul)) + (x / simplemul)];

	if (delta == SHUTOFF_LOD_INVISIBLE)
		return -1;	// This block is completely invisible

	//if (SimplifyVertexSlow (x+(simplemul/2),z+(simplemul/2),delta))
	//	return 1;

	if (SimplifyVertex(x + (simplemul / 2), z + (simplemul / 2), delta))
		return 1;

	return 0;
}


// Shuts off LOD for a given cell
void TurnOffLODForCell(int cellnum)
{
	if (Dedicated_server)
		return;

	ASSERT(cellnum >= 0 && cellnum < (TERRAIN_WIDTH * TERRAIN_DEPTH));
	int x = cellnum % TERRAIN_WIDTH;
	int z = cellnum / TERRAIN_WIDTH;

	ASSERT(Num_lodoffs < MAX_LODOFFS);
	LODOffs[Num_lodoffs].cellnum = cellnum;


	for (int i = 0; i < MAX_TERRAIN_LOD - 1; i++)
	{
		float delta;
		int simplemul = 1 << ((MAX_TERRAIN_LOD - 1) - i);

		delta = TerrainDeltaBlocks[i][((z / simplemul) * (TERRAIN_WIDTH / simplemul)) + (x / simplemul)];
		LODOffs[Num_lodoffs].save_delta[i] = delta;
		TerrainDeltaBlocks[i][((z / simplemul) * (TERRAIN_WIDTH / simplemul)) + (x / simplemul)] = SHUTOFF_LOD_DELTA;
	}

	Num_lodoffs++;
}

// Restores the terrain deltas to their original state
void ClearLODOffs()
{
	for (int t = Num_lodoffs - 1; t >= 0; t--)
	{
		int cellnum = LODOffs[t].cellnum;

		int x = cellnum % TERRAIN_WIDTH;
		int z = cellnum / TERRAIN_WIDTH;

		for (int i = 0; i < MAX_TERRAIN_LOD - 1; i++)
		{
			int simplemul = 1 << ((MAX_TERRAIN_LOD - 1) - i);
			TerrainDeltaBlocks[i][((z / simplemul) * (TERRAIN_WIDTH / simplemul)) + (x / simplemul)] = LODOffs[t].save_delta[i];
		}
	}

	Num_lodoffs = 0;
}
