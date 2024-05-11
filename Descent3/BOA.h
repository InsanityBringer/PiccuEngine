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

#ifndef _BOA_H__
#define _BOA_H__

#include "room.h"
#include "terrain.h"

#define MAX_PATH_PORTALS 40
#define MAX_BOA_TERRAIN_REGIONS 8

extern float BOA_cost_array[MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS][MAX_PATH_PORTALS];
extern unsigned short BOA_Array[MAX_ROOMS + MAX_BOA_TERRAIN_REGIONS][MAX_ROOMS+MAX_BOA_TERRAIN_REGIONS];
extern int BOA_mine_checksum;
extern int BOA_AABB_checksum;
extern int BOA_vis_checksum;
extern bool BOA_vis_valid;
extern int BOA_AABB_ROOM_checksum[MAX_ROOMS+MAX_BOA_TERRAIN_REGIONS];

//--  Priority Queue
class q_item 
{
	public:
	q_item(int room_index, int par, float n_cost) 
	{
		roomnum = room_index; 
		parent = par; 
		cost = n_cost; 
		next = NULL;
	}
	
	int roomnum;
	int parent;
	float cost;

	q_item *next;
};

class pq 
{
	q_item *q_head;

	public:
	pq() {q_head = NULL;}

	void push(q_item *new_node) 
	{
		new_node->next = q_head;
		q_head = new_node;
	}
	
	q_item *pop() 
	{
		q_item *cur_item = q_head;
		q_item *prev_item = NULL;
		q_item *best_item = q_head;
		q_item *best_prev_item = NULL;
	
		if(q_head == NULL) return NULL;

		while(cur_item != NULL) 
		{
			if(cur_item->cost < best_item->cost) 
			{

				best_prev_item = prev_item;
				best_item = cur_item;
			}

			prev_item = cur_item;
			cur_item = cur_item->next;
		}


		// Actually remove this item from the list
		if(best_item == q_head) 
		{
			q_head = best_item->next;
		} 
		else 
		{
			best_prev_item->next = best_item->next;
		}

		return best_item;
	}
};

// Next Segment Info
#define BOA_ROOM_MASK			0x03FF

#define BOA_TERRAIN_INDEX	(Highest_room_index+1)
#define BOA_INDEX(x)       ((ROOMNUM_OUTSIDE(x)?(TERRAIN_REGION(x)+Highest_room_index+1):x))
#define BOA_NO_PATH			(Highest_room_index+9)

#define BOA_NEXT_ROOM(a, b) (BOA_Array[a][b] & BOA_ROOM_MASK)

// Visability Info

#define BOAF_VIS			0x0400

#define BOA_GET_VIS(a, b) (BOA_vis_valid?((BOA_Array[a][b] & BOAF_VIS)!=0):1)

// Set if a sound could prop between these to areas
#define BOA_SOUND_PROP		0x0800

// Distance from segment to segment Info

#define BOA_DIST_MASK		0x3000

#define BOA_VNEAR_DIST		0.0
#define BOA_NEAR_DIST		100.0
#define BOA_FAR_DIST			200.0
#define BOA_VFAR_DIST		300.0

#define BOAF_VNEAR_DIST		0x0000	// 0.0   to 100.0
#define BOAF_NEAR_DIST		0x1000	// 100.0 to 200.0
#define BOAF_FAR_DIST		0x2000   // 200.0 to 300.0
#define BOAF_VFAR_DIST		0x3000   // 300.0+

#define BOA_GET_DIST(a, b) ((BOA_Array[a][b]&BOA_DIST_MASK)!=0)

// Possible Blockage Info

#define BOAF_BLOCKAGE		0x4000
#define BOA_HAS_POSSIBLE_BLOCKAGE(a, b) ((BOA_Array[a][b]&BOAF_BLOCKAGE)!=0)

// Path have a sport in it that is too small for a robot to get through
#define BOAF_TOO_SMALL_FOR_ROBOT	0x8000
#define BOA_TOO_SMALL_FOR_ROBOT(a, b) ((BOA_Array[a][b]&BOAF_TOO_SMALL_FOR_ROBOT)!=0)

struct connect_data
{
	int roomnum;
	int portal;
};

extern int BOA_num_mines;
extern int BOA_num_terrain_regions;
extern int BOA_num_connect[MAX_BOA_TERRAIN_REGIONS];
extern connect_data BOA_connect[MAX_BOA_TERRAIN_REGIONS][MAX_PATH_PORTALS];

void MakeBOA(void);

// Goes through all the rooms and determines their visibility in relation to one another
void MakeBOAVisTable (bool from_lighting=0);

// Computes the AABBs for the level
void ComputeAABB(bool f_full);

bool BOA_ComputeMinDist(int start_room, int end_room, float max_check_dist, float *dist, int *num_blockages = NULL);
bool BOA_ComputePropDist(int start_room, vector *start_pos, int end_room, vector *end_pos, float *dist, int *num_blockages);
bool BOA_IsSoundAudible(int start_room, int end_room);
bool BOA_IsVisible(int start_room, int end_room);
bool BOA_HasPossibleBlockage(int start_room, int end_room);
int BOA_GetNextRoom(int start_room, int end_room);
int BOA_DetermineStartRoomPortal(int start_room, vector *start_pos, int end_room, vector *end_pos, bool f_for_sound = false, bool f_making_robot_path_invalid_list = false, int *blocked_portal = NULL);
bool BOA_PassablePortal(int room, int portal_index, bool f_for_sound = false, bool f_making_robot_path_invalid_list = false);
bool BOA_LockedDoor(object *obj, int roomnum);
void BOA_ComputePathPoints(char *message = NULL, int len = 0);
int BOAGetMineChecksum ();

#endif
