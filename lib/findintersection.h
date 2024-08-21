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

#ifndef _FVI_H
#define _FVI_H

#include "object.h"
#include "vecmat.h"
#include "terrain.h"
#include "findintersection_external.h"

extern float Ceiling_height;

#define CEILING_HEIGHT	(Ceiling_height)

#define PLAYER_SIZE_SCALAR 0.8f

extern bool FVI_always_check_ceiling;

// Big Object Info
#define CELLS_PER_COL_CELL		1
#define COL_TERRAIN_SIZE		(TERRAIN_SIZE*(float)CELLS_PER_COL_CELL)

#define MIN_BIG_OBJ_RAD	(COL_TERRAIN_SIZE)

// chrishack -- we could turn backface, on and off so that we
// can individually use backface checking on object and/or wall...  :)

#define MAX_FVI_SEGS 100

#define MAX_HITS	2

#define Q_RIGHT	0
#define Q_LEFT		1
#define Q_MIDDLE  2

inline bool FastVectorBBox(const float *min, const float *max, const float *origin, const float *dir)
{
	bool f_inside = true;
	char quad[3];
	register int i;
	float max_t[3];
	float can_plane[3];
	int which_plane;
	float coord[3];

	for(i = 0; i < 3; i++)
	{
		if(origin[i] < min[i])
		{
			quad[i] = Q_LEFT;
			can_plane[i] = min[i];
			f_inside = false;
		}
		else if (origin[i] > max[i])
		{
			quad[i] = Q_RIGHT;
			can_plane[i] = max[i];
			f_inside = false;
		}
		else
		{
			quad[i] = Q_MIDDLE;
		}
	}

	if(f_inside)
	{
		return true;
	}

	for(i = 0; i < 3; i++)
	{
		if(quad[i] != Q_MIDDLE && dir[i] != 0.0f)
			max_t[i] = (can_plane[i] - origin[i])/dir[i];	
		else
			max_t[i] = -1.0f;
	}

	which_plane = 0;

	for(i = 0; i < 3; i++)
		if(max_t[which_plane] < max_t[i])
			which_plane = i;

	if(max_t[which_plane] < 0.0f)
		return false;

	for(i = 0; i < 3; i++)
	{
		if (which_plane != i)
		{
			coord[i] = origin[i] + max_t[which_plane] * dir[i];

			if((quad[i] == Q_RIGHT && coord[i] < min[i]) ||
				(quad[i] == Q_LEFT && coord[i] > max[i]))
			{
				return false;
			}
		}
		else
		{
			coord[i] = can_plane[i];
		}
	}

	return true;
}

//this data structure gets filled in by find_vector_intersection()
struct fvi_info 
{
	vector hit_pnt;						// centerpoint when we hit
	int hit_room;							// what room hit_pnt is in
	float hit_dist;						// distance of the hit
	
	int num_hits;							// Number of recorded hits

	int hit_type[MAX_HITS]; 			//what sort of intersection
	vector hit_face_pnt[MAX_HITS];	// actual collision point (edge of rad)
	
	int hit_face_room[MAX_HITS];		// what room the hit face is in
	int hit_face[MAX_HITS];				// if hit wall, which face
	vector hit_wallnorm[MAX_HITS];	// if hit wall, ptr to its surface normal

	int hit_object[MAX_HITS];			// if object hit, which object
	int hit_subobject[MAX_HITS];		// if a POLY_2_SPHERE hit, then it has the poly involved

	int n_rooms;							// how many segs we went through
	int roomlist[MAX_FVI_SEGS];		// list of segs vector went through

	// BBox hit results
	matrix hit_orient;
	vector hit_rotvel;
	angle  hit_turnroll;
	vector hit_velocity;
	float  hit_time;

	vector hit_subobj_fvec;
	vector hit_subobj_uvec;
	vector hit_subobj_pos;
};

//this data contains the parms to fvi()
struct fvi_query 
{
	vector *p0,*p1;
	int startroom;
	float rad;
	short thisobjnum;
	int *ignore_obj_list;
	int flags;

	// BBox stuff...
	matrix *o_orient;
	vector *o_rotvel;
	vector *o_rotthrust;
	vector *o_velocity;
	angle *o_turnroll;
	vector *o_thrust;
	float frametime;
};

//Find out if a vector intersects with anything.
//Fills in hit_data, an fvi_info structure (see above).
//Parms:
//  p0 & startseg 	describe the start of the vector
//  p1 					the end of the vector
//  rad 					the radius of the cylinder
//  thisobjnum 		used to prevent an object with colliding with itself
//  ingore_obj_list	NULL, or ptr to a list of objnums to ignore, terminated with -1
//  check_obj_flag	determines whether collisions with objects are checked
//Returns the hit_data->hit_type
extern int fvi_FindIntersection(fvi_query *fq,fvi_info *hit_data,  bool no_subdivision = false);

// Face/Room list for some fvi call(s)
struct fvi_face_room_list
{
	ushort face_index;
	ushort room_index;
};

#define MAX_RECORDED_FACES 200

// Recorded face list
extern fvi_face_room_list Fvi_recorded_faces[MAX_RECORDED_FACES];
extern int Fvi_num_recorded_faces;

// Generates a list of faces(with corresponding room numbers) within a given distance to a position.
// Return value is the number of faces in the list
extern int fvi_QuickDistFaceList(int init_room_index, vector *pos, float rad, fvi_face_room_list *quick_fr_list, int max_elements);
// Returns the number of cells that are approximately within the specified radius
extern int fvi_QuickDistCellList(int init_cell_index, vector *pos, float rad, int *quick_cell_list, int max_elements);

// Returns the number of objects that are approximately within the specified radius
int fvi_QuickDistObjectList(vector *pos, int init_roomnum, float rad, short *object_index_list, int max_elements, bool f_lightmap_only, bool f_only_players_and_ais = false, bool f_include_non_collide_objects = false, bool f_stop_at_closed_doors = false);

//finds the uv coords of the given point on the given seg & side
//fills in u & v. if l is non-NULL fills it in also
//extern void fvi_FindHitpointUV(float *u,float *v,float *l, vector *pnt,segment *seg,int sidenum,int facenum);

//Returns true if the object is through any walls
extern int fvi_ObjectIntersectsWall(object *objp);

extern int FVI_counter;
extern int FVI_room_counter;

bool fvi_QuickRoomCheck(vector *pos, room *cur_room, bool try_again = false);

extern fvi_info * fvi_hit_data_ptr;
extern fvi_query * fvi_query_ptr;
extern float fvi_collision_dist;
extern int fvi_curobj;
extern int fvi_moveobj;

bool PolyCollideObject(object *obj);

bool BBoxPlaneIntersection(bool fast_exit, vector *collision_point, vector *collision_normal, object *obj, vector *new_pos, int nv, vector **vertex_ptr_list, vector *face_normal, matrix *orient);

extern int check_vector_to_sphere_1(vector *intp, float *col_dist, const vector *p0, const vector *p1,vector *sphere_pos,float sphere_rad, bool f_correcting, bool f_init_collisions);

extern int check_line_to_face(vector *newp, vector *colp, float *col_dist, vector *wall_norm,const vector *p0,const vector *p1, vector *face_normal, vector **vertex_ptr_list, const int nv,const float rad);
extern void InitFVI(void);

// Types of supported collisions
#ifdef NED_PHYSICS
#define RESULT_NOTHING					0
#define RESULT_CHECK_SPHERE_SPHERE		1
#define RESULT_CHECK_SPHERE_POLY		2
#define RESULT_CHECK_POLY_SPHERE		3
#define RESULT_CHECK_BBOX_POLY			4
#define RESULT_CHECK_POLY_BBOX			5
#define RESULT_CHECK_BBOX_BBOX			6
#define RESULT_CHECK_BBOX_SPHERE		7
#define RESULT_CHECK_SPHERE_BBOX		8
#define RESULT_CHECK_SPHERE_ROOM		9
#define RESULT_CHECK_BBOX_ROOM			10
#endif

#endif
