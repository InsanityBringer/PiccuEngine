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

#include "room.h"
#include "mono.h"
#include "vecmat.h"
#include "gametexture.h"
#include "manage.h"
#include "renderer.h"
#include "game.h"
#include "render.h"
#include "grdefs.h"
#include <stdlib.h>
#include <string.h>
#include "terrain.h"
#include "findintersection.h"
#include "lightmap.h"
#include "lightmap_info.h"
#include "special_face.h"
#include "mem.h"
#include "doorway.h"
#include "multi_world_state.h"
#include "damage_external.h"
#include "descent.h"
#ifdef EDITOR
#include "editor\editor_lighting.h"
#endif
#ifdef NEWEDITOR
#include "neweditor\editor_lighting.h"
#endif
#include "bnode.h"

//Global array of rooms
room Rooms[MAX_ROOMS + MAX_PALETTE_ROOMS];
room_changes Room_changes[MAX_ROOM_CHANGES];

extern int Cur_selected_room, Cur_selected_face;

int Highest_room_index = -1;

void FreePaletteRooms();

// Zeroes out the rooms array
void InitRooms()
{
	for (int i = 0; i < MAX_ROOMS + MAX_PALETTE_ROOMS; i++)
	{
		memset(&Rooms[i], 0, sizeof(room));
		Rooms[i].objects = -1;		//DAJ
		Rooms[i].vis_effects = -1;	//DAJ
	}

	atexit(FreeAllRooms);
	atexit(BNode_ClearBNodeInfo);	//DAJ

#ifdef EDITOR
	atexit(FreePaletteRooms);
#endif
}

#if ( defined(EDITOR) || defined(NEWEDITOR) )
//Figures out how many verts there are in all the faces in a room
int CountRoomFaceVerts(room* rp)
{
	int n = 0;

	for (int f = 0; f < rp->num_faces; f++)
		n += rp->faces[f].num_verts;

	return n;
}

#endif

//Vars for the room memory system
ubyte* Room_mem_buf = NULL;		//pointer to the rooms block of memory
ubyte* Room_mem_ptr = NULL;		//pointer to free memory in the rooms block
int Room_mem_size;				//How big our chunk is

//Closes down the room memory system.
void RoomMemClose()
{
	if (Room_mem_buf)
		mem_free(Room_mem_buf);

	Room_mem_buf = Room_mem_ptr = NULL;
}

//Initialized the memory buffer for a room
//Parameters:	size - the total amount of memory needed for the room
void RoomMemInit(int nverts, int nfaces, int nfaceverts, int nportals)
{
#if (defined(EDITOR) || defined(NEWEDITOR))
	return;		//This system is disabled in the editor
#endif

	if (nverts == 0)		//We don't know how much mem the room will use, so do the old way
		return;

	int size = (nfaces * (sizeof(*Rooms[0].faces))) +
		(nverts * sizeof(*Rooms[0].verts)) +
		(nportals * sizeof(*Rooms[0].portals)) +
		(nfaceverts * (sizeof(*Rooms[0].faces[0].face_verts) + sizeof(*Rooms[0].faces[0].face_uvls)));

	if (Room_mem_buf)
		mem_free(Room_mem_buf);

	Room_mem_buf = (ubyte*)mem_malloc(size);
	Room_mem_size = size;

	Room_mem_ptr = Room_mem_buf;
}

//Allocates memory for a room or face
void* RoomMemAlloc(int size)
{
	if (Room_mem_buf)
	{
		void* p = Room_mem_ptr;
		Room_mem_ptr += size;
		ASSERT(Room_mem_ptr <= (Room_mem_buf + Room_mem_size));
		return p;
	}
	else
		return mem_malloc(size);
}

//Frees memory in a room
//Doesn't actually do anything
void RoomMemFree(void* buf)
{
	if (!buf)
		return;

	if (Room_mem_buf)
	{
		ASSERT(((buf) >= Room_mem_buf) && ((buf) < (Room_mem_buf + Room_mem_size)));
	}
	else
		mem_free(buf);
}

//Initalize a room, allocating memory and filling in fields
//Parameters:	rp - the room to be initialized
//					nverts - how many vertices this room will have
//					nfaces - how many faces this room wil have
//					nportals - how many portals this room will have
void InitRoom(room* rp, int nverts, int nfaces, int nportals)
{
	//initialize room fields
	rp->flags = 0;
	rp->objects = -1;
	rp->vis_effects = -1;
	rp->volume_lights = NULL;
	rp->mirror_face = -1;
	rp->num_mirror_faces = 0;
	rp->mirror_faces_list = NULL;
	rp->room_change_flags = 0;

#ifndef NEWEDITOR // the new editor must allow users to create a room from scratch
	ASSERT(nverts > 0);
	ASSERT(nfaces > 0);
#endif

	rp->wind = Zero_vector;

	rp->num_faces = nfaces;
	rp->num_verts = nverts;
	rp->num_portals = nportals;
	rp->last_render_time = 0;
	rp->fog_depth = 100.0;
	rp->fog_r = 1.0;
	rp->fog_g = 1.0;
	rp->fog_b = 1.0;

	rp->faces = (face*)RoomMemAlloc(nfaces * sizeof(*rp->faces)); ASSERT(rp->faces != NULL);

	rp->num_bbf_regions = 0;

	rp->verts = (vector*)RoomMemAlloc(nverts * sizeof(*rp->verts));  ASSERT(rp->verts != NULL);

	if (Katmai)
		rp->verts4 = (vector4*)mem_malloc(nverts * sizeof(*rp->verts4));  ASSERT(rp->verts4 != NULL);

	rp->pulse_time = 0;
	rp->pulse_offset = 0;

	if (nportals)
	{
		rp->portals = (portal*)RoomMemAlloc(nportals * sizeof(*rp->portals));
		ASSERT(rp->portals != NULL);
	}
	else
		rp->portals = NULL;

	//Default to no ambient sound
	rp->ambient_sound = -1;

	rp->name = NULL;
	rp->doorway_data = NULL;

	rp->env_reverb = 0;	// reverb for sound system.

	rp->damage = 0.0;				// room damage
	rp->damage_type = PD_NONE;	// room damage type

	rp->bn_info.num_nodes = 0;
	rp->bn_info.nodes = NULL;

#if ( defined(EDITOR) || defined(NEWEDITOR) )
	Room_multiplier[rp - Rooms] = 1.0;

	Room_ambience_r[rp - Rooms] = 0.0;
	Room_ambience_g[rp - Rooms] = 0.0;
	Room_ambience_b[rp - Rooms] = 0.0;
#endif

	rp->used = 1;			//flag this room as used
}

//Initialize a room face structure.
void InitRoomFace(face* fp, int nverts)
{
	fp->flags = 0;
	fp->num_verts = nverts;
	fp->portal_num = -1;
	fp->tmap = 0;

#ifdef NEWEDITOR
	ned_MarkTextureInUse(0, true);
#endif

	fp->lmi_handle = BAD_LMI_INDEX;
	fp->special_handle = BAD_SPECIAL_FACE_INDEX;
	fp->light_multiple = 4;

	fp->face_verts = (short*)RoomMemAlloc(nverts * sizeof(*fp->face_verts));  ASSERT(fp->face_verts != NULL);
	fp->face_uvls = (roomUVL*)RoomMemAlloc(nverts * sizeof(*fp->face_uvls));  ASSERT(fp->face_uvls != NULL);

	ASSERT(fp->face_verts);
	ASSERT(fp->face_uvls);
	for (int i = 0; i < nverts; i++)
		fp->face_uvls[i].alpha = 255;
}

// Finds out if we are in a room or outside the mine (-1 if we are outside)
int FindPointRoom(vector* pnt)
{
	ASSERT(pnt != NULL);

	for (int i = 0; i <= Highest_room_index; i++)
	{
		if ((Rooms[i].used) && !(Rooms[i].flags & RF_EXTERNAL))
		{
			bool f_in_room;

			f_in_room = fvi_QuickRoomCheck(pnt, &Rooms[i]);

			if (f_in_room == true) return i;
		}
	}

	return -1;
}

//Frees a room, deallocating its memory and marking it as unused
void FreeRoom(room* rp)
{
	int i;
	int old_hri = Highest_room_index;

	ASSERT(rp->used != 0);		//make sure room is un use

	//Free the faces
	for (i = 0; i < rp->num_faces; i++)
		FreeRoomFace(&rp->faces[i]);

	//Free up mem alloced for this room
	RoomMemFree(rp->faces);
	RoomMemFree(rp->portals);
	RoomMemFree(rp->verts);

	if (Katmai)
		mem_free(rp->verts4);

	if (rp->num_bbf_regions)
	{
		for (i = 0; i < rp->num_bbf_regions; i++)
		{
			mem_free(rp->bbf_list[i]);
		}
		mem_free(rp->bbf_list);
		mem_free(rp->num_bbf);
		mem_free(rp->bbf_list_min_xyz);
		mem_free(rp->bbf_list_max_xyz);
		mem_free(rp->bbf_list_sector);

		rp->num_bbf_regions = 0;
	}

	BNode_FreeRoom(rp);

	if (rp->volume_lights)
		mem_free(rp->volume_lights);

	if (rp->name)
		mem_free(rp->name);

	if (rp->doorway_data)
		mem_free(rp->doorway_data);

	if (rp->mirror_faces_list)
		mem_free(rp->mirror_faces_list);

	rp->used = 0;

	//Update Highest_room_index
	if (ROOMNUM(rp) == Highest_room_index)
		while ((Highest_room_index >= 0) && (!Rooms[Highest_room_index].used))
			Highest_room_index--;

	BNode_RemapTerrainRooms(old_hri, Highest_room_index);
}

//Frees all the rooms currently in use, deallocating their memory and marking them as unused
void FreeAllRooms()
{
	int rn;
	room* rp;
	mprintf((1, "Freeing rooms...Higest_room_index %d\n", Highest_room_index));
	for (rn = 0, rp = Rooms; rn <= Highest_room_index; rn++, rp++)
	{
		if (rp->used)
		{
			//mprintf((2, "rn %d\n", rn));
			FreeRoom(rp);
		}
	}

	ASSERT(Highest_room_index == -1);

	RoomMemClose();

	//mprintf((2,"Done\n"));
}

#ifdef EDITOR
//Frees rooms that are in the room palette
void FreePaletteRooms()
{
	int rn;
	room* rp;

	for (rn = MAX_ROOMS, rp = &Rooms[MAX_ROOMS]; rn < MAX_ROOMS + MAX_PALETTE_ROOMS; rn++, rp++)
		if (rp->used)
			FreeRoom(rp);
}
#endif

//Free the memory used by a room face structure
void FreeRoomFace(face* fp)
{
	if (fp->lmi_handle != BAD_LMI_INDEX)
	{
		FreeLightmapInfo(fp->lmi_handle);
		fp->lmi_handle = BAD_LMI_INDEX;
		fp->flags &= ~FF_LIGHTMAP;
	}

	if (fp->special_handle != BAD_SPECIAL_FACE_INDEX)
	{
		FreeSpecialFace(fp->special_handle);
		fp->special_handle = BAD_SPECIAL_FACE_INDEX;
	}

	RoomMemFree(fp->face_verts);
	RoomMemFree(fp->face_uvls);
}

//Finds the center point of a room
//Parameters:	vp - filled in with the center point
//					rp - the room whose center to find
void ComputeRoomCenter(vector* vp, room* rp)
{
	vp->x = vp->y = vp->z = 0;

	for (int i = 0; i < rp->num_verts; i++)
		*vp += rp->verts[i];

#ifdef NEWEDITOR
	if (rp->num_verts)
#endif
		* vp /= rp->num_verts;

}

//Computes the center point on a face by averaging the points in the face
void ComputeCenterPointOnFace(vector* vp, room* rp, int facenum)
{
	face* fp = &rp->faces[facenum];

	vp->x = vp->y = vp->z = 0;

	for (int i = 0; i < fp->num_verts; i++)
		*vp += rp->verts[fp->face_verts[i]];

	*vp /= fp->num_verts;
}

//the minimum magnitude of a surface normal that we're willing to accept
#define MIN_NORMAL_MAG 0.035

//Computes (fills in) the surface normal of a face.
//Finds the best normal on this face by checking all sets of three vertices
//IMPORTANT:  The caller should really check the return value of this function
//Parameters:	rp,facenum - the room and face to calculate the normal for
//Returns:		true if the normal is ok
//					false if the normal has a very small (pre-normalization) magnitude
bool ComputeFaceNormal(room* rp, int facenum)
{
	face* fp = &rp->faces[facenum];
	bool ok;

	ok = ComputeNormal(&fp->normal, fp->num_verts, fp->face_verts, rp->verts);

	if (!ok)
	{
		mprintf((1, "Warning: Low precision normal for room:face = %d:%d\n", ROOMNUM(rp), facenum));
	}

	return ok;
}

//Compute the surface normal from a list of vertices that determine a face
//Finds the best normal on this face by checking all sets of three vertices
//IMPORTANT:  The caller should really check the return value of this function
//Parameters:	normal - this is filled in with the normal
//					num_verts - how many vertices in the face
//					vertnum_list - a list of vertex numbers for this face.  these index into verts
//					verts - the array of vertices into which the elements of vertnum_list index
//Returns:		true if the normal is ok
//					false if the normal has a very small (pre-normalization) magnitude
bool ComputeNormal(vector* normal, int num_verts, short* vertnum_list, vector* verts)
{
	int i;
	float largest_mag;

	i = 0;
	largest_mag = 0.0;

	for (i = 0; i < num_verts; i++)
	{
		vector tnormal;
		float mag;

		mag = vm_GetNormal(&tnormal,
			&verts[vertnum_list[i]],
			&verts[vertnum_list[(i + 1) % num_verts]],
			&verts[vertnum_list[(i + 2) % num_verts]]);

		if (mag > largest_mag)
		{
			*normal = tnormal;
			largest_mag = mag;
		}
	}


	if (largest_mag < MIN_NORMAL_MAG)
	{
		mprintf((1, "Warning: Normal has low precision. mag = %f, norm =  %f,%f,%f\n", largest_mag, normal->x, normal->y, normal->z));
		return 0;
	}
	else
		return 1;


}

//Computes the center point on a face by averaging the points in the portal
void ComputePortalCenter(vector* vp, room* rp, int portal_index)
{
	portal* pp = &rp->portals[portal_index];
	face* fp = &rp->faces[pp->portal_face];

	vm_MakeZero(vp);

	for (int i = 0; i < fp->num_verts; i++)
		*vp += rp->verts[fp->face_verts[i]];

	*vp /= fp->num_verts;
}

// Clears lightmaps for a single room
void ClearRoomLightmaps(int roomnum)
{
	int t;

	ASSERT(Rooms[roomnum].used);

	for (t = 0; t < Rooms[roomnum].num_faces; t++)
	{
		if (Rooms[roomnum].faces[t].lmi_handle != BAD_LMI_INDEX)
		{
			FreeLightmapInfo(Rooms[roomnum].faces[t].lmi_handle);
			Rooms[roomnum].faces[t].lmi_handle = BAD_LMI_INDEX;
			Rooms[roomnum].faces[t].flags &= ~FF_LIGHTMAP;
		}
	}
}

// Removes all room lightmaps from memory and sets indoor faces accordingly
// External=1 means to perform the operation on external rooms only, 0 means indoor rooms only
void ClearAllRoomLightmaps(int external)
{
	for (int i = 0; i < MAX_ROOMS; i++)
	{
		if (Rooms[i].used)
		{
			if (external && !(Rooms[i].flags & RF_EXTERNAL))
				continue;
			if (!external && (Rooms[i].flags & RF_EXTERNAL))
				continue;

			ClearRoomLightmaps(i);
		}
	}
}

// Clears specmaps for a single room
void ClearRoomSpecmaps(int roomnum)
{
	ASSERT(Rooms[roomnum].used);

	for (int t = 0; t < Rooms[roomnum].num_faces; t++)
	{
		if (Rooms[roomnum].faces[t].special_handle != BAD_SPECIAL_FACE_INDEX)
		{
			if (SpecialFaces[Rooms[roomnum].faces[t].special_handle].type == SFT_SPECULAR)
			{
				FreeSpecialFace(Rooms[roomnum].faces[t].special_handle);
				Rooms[roomnum].faces[t].special_handle = BAD_SPECIAL_FACE_INDEX;
			}
		}
	}
}

// Removes all room specularity maps from memory and sets indoor faces accordingly
// External=1 means to perform the operation on external rooms only, 0 means indoor rooms only
void ClearAllRoomSpecmaps(int external)
{
	for (int i = 0; i < MAX_ROOMS; i++)
	{
		if (Rooms[i].used)
		{
			if (external && !(Rooms[i].flags & RF_EXTERNAL))
				continue;
			if (!external && (Rooms[i].flags & RF_EXTERNAL))
				continue;

			ClearRoomSpecmaps(i);
		}
	}
}

// Removes all room volume lights from memory
void ClearVolumeLights(int roomnum)
{
	ASSERT(Rooms[roomnum].used);

	ASSERT(!(Rooms[roomnum].flags & RF_EXTERNAL));

	if (Rooms[roomnum].volume_lights)
	{
		mem_free(Rooms[roomnum].volume_lights);
		Rooms[roomnum].volume_lights = NULL;
	}
}

// Removes all room volume lights from memory
void ClearAllVolumeLights()
{
	for (int i = 0; i < MAX_ROOMS; i++)
	{
		if (Rooms[i].used)
		{
			if ((Rooms[i].flags & RF_EXTERNAL))
				continue;

			ClearVolumeLights(i);
		}
	}
}

// Returns the area taken up by a face
float GetAreaForFace(room* rp, int facenum)
{
	ASSERT(rp->used > 0);
	ASSERT(facenum >= 0 && facenum < rp->num_faces);

	face* fp = &rp->faces[facenum];
	vector normal;
	float area = 0;

	vm_GetPerp(&normal, &rp->verts[fp->face_verts[0]], &rp->verts[fp->face_verts[1]], &rp->verts[fp->face_verts[2]]);
	area = (vm_GetMagnitude(&normal) / 2);

	for (int i = 2; i < fp->num_verts - 1; i++)
	{
		vm_GetPerp(&normal, &rp->verts[fp->face_verts[0]], &rp->verts[fp->face_verts[i]], &rp->verts[fp->face_verts[i + 1]]);
		area += (vm_GetMagnitude(&normal) / 2);
	}

	return area;
}

//Returns indeces of the two elements of points on a face to use as a 2d projection
//Parameters:	normal - the surface normal of the face
//					ii,jj - filled in with elements numbers (0,1, or 2)
void GetIJ(const vector* normal, int* ii, int* jj)
{
	//To project onto 2d, find the largest element of the surface normal
	if (fabs(normal->x) > fabs(normal->y))
		if (fabs(normal->x) > fabs(normal->z))
		{
			if (normal->x > 0)
			{
				*ii = 2; *jj = 1;		// x > y, x > z
			}
			else
			{
				*ii = 1; *jj = 2;
			}
		}
		else
		{
			if (normal->z > 0)
			{
				*ii = 1; *jj = 0;		// z > x > y
			}
			else
			{
				*ii = 0; *jj = 1;
			}
		}
	else		// y > x
		if (fabs(normal->y) > fabs(normal->z))
		{
			if (normal->y > 0)
			{
				*ii = 0; *jj = 2;		// y > x, y > z
			}
			else
			{
				*ii = 2; *jj = 0;
			}
		}
		else
		{
			if (normal->z > 0)
			{
				*ii = 1; *jj = 0;		// z > y > x
			}
			else
			{
				*ii = 0; *jj = 1;
			}
		}
}

//2d cross product
#define cross(v0,v1) (((v0)[ii] * (v1)[jj]) - ((v0)[jj] * (v1)[ii]))

//Finds the uv coords of a given point on a room:face.  Fills in u & v.
//Parameters:	u,v - pointers to variables to be filled in
//					pnt - the point we're checking
//					rp - pointer to the room that pnt is in
//					fp - pointer to the face that pnt is on
void FindPointUV(float* u, float* v, const vector* pnt, const room* rp, const face* fp)
{
	int roomnum = ROOMNUM(rp);
	int ii, jj;
	vector vec0, vec1;
	float* p1, * checkp, * v0, * v1;
	float k0, k1;
	int t;

	//Make sure we have a valid room
	ASSERT((roomnum >= 0) && (roomnum <= Highest_room_index));

	//Find what plane to project this wall onto to make it a 2d case
	GetIJ(&fp->normal, &ii, &jj);

	//Compute delta vectors
	vec0 = rp->verts[fp->face_verts[0]] - rp->verts[fp->face_verts[1]];		//vec from 1 -> 0
	vec1 = rp->verts[fp->face_verts[2]] - rp->verts[fp->face_verts[1]];		//vec from 1 -> 0

	//Get pointers to referece our vectors as arrays of floats
	p1 = (float*)&rp->verts[fp->face_verts[1]];
	v0 = (float*)&vec0;
	v1 = (float*)&vec1;
	checkp = (float*)pnt;

	//Compute our clipping values along i & j axes
	k1 = -(cross(checkp, v0) + cross(v0, p1)) / cross(v0, v1);
	t = (fabs(v0[ii]) > fabs(v0[jj])) ? ii : jj;
	k0 = ((-k1 * v1[t]) + checkp[t] - p1[t]) / v0[t];

	//Compute u & v values
	*u = fp->face_uvls[1].u + (k0 * (fp->face_uvls[0].u - fp->face_uvls[1].u)) + (k1 * (fp->face_uvls[2].u - fp->face_uvls[1].u));
	*v = fp->face_uvls[1].v + (k0 * (fp->face_uvls[0].v - fp->face_uvls[1].v)) + (k1 * (fp->face_uvls[2].v - fp->face_uvls[1].v));
}

//Check if a particular point on a wall is a transparent pixel
//Parameters:	pnt - the point we're checking
//					rp - pointer to the room that pnt is in
//					facenum - the face that pnt is on
//Returns:	true if can pass through the given point, else 0
int CheckTransparentPoint(const vector* pnt, const room* rp, const int facenum)
{
	int bm_handle;
	face* fp = &rp->faces[facenum];
	float u, v;
	int w, h, x, y;

	return false;

	//Get the UV coordindates of the point we hit
	FindPointUV(&u, &v, pnt, rp, fp);

	//Get pointer to the bitmap data
	bm_handle = GetTextureBitmap(fp->tmap, 0);

	//Get x & y coordindates (in bitmap) of check point
	w = bm_w(bm_handle, 0); h = bm_h(bm_handle, 0);
	x = ((int)(u * w)) % w;
	y = ((int)(v * h)) % h;

	//Return true if the check point is transparent
	return bm_pixel_transparent(bm_handle, x, y);
}

//Computes a bounding sphere for the current room
//Parameters: center - filled in with the center point of the sphere
//		rp - the room we�re bounding
//Returns: the radius of the bounding sphere
float ComputeRoomBoundingSphere(vector* center, room* rp)
{
	//This algorithm is from Graphics Gems I.  There's a better algorithm in Graphics Gems III that
	//we should probably implement sometime.

	vector* min_x, * max_x, * min_y, * max_y, * min_z, * max_z, * vp;
	float dx, dy, dz;
	float rad, rad2;
	int i;

#ifdef NEWEDITOR
	if (!rp->num_verts)
	{
		center->x = 0.0f; center->y = 0.0f; center->z = 0.0f;
		return 0.0f;
	}
#endif

	//Initialize min, max vars
	min_x = max_x = min_y = max_y = min_z = max_z = &rp->verts[0];

	//First, find the points with the min & max x,y, & z coordinates
	for (i = 0, vp = rp->verts; i < rp->num_verts; i++, vp++)
	{
		if (vp->x < min_x->x)
			min_x = vp;

		if (vp->x > max_x->x)
			max_x = vp;

		if (vp->y < min_y->y)
			min_y = vp;

		if (vp->y > max_y->y)
			max_y = vp;

		if (vp->z < min_z->z)
			min_z = vp;

		if (vp->z > max_z->z)
			max_z = vp;
	}

	//Calculate initial sphere

	dx = vm_VectorDistance(min_x, max_x);
	dy = vm_VectorDistance(min_y, max_y);
	dz = vm_VectorDistance(min_z, max_z);

	if (dx > dy)
		if (dx > dz)
		{
			*center = (*min_x + *max_x) / 2;  rad = dx / 2;
		}
		else
		{
			*center = (*min_z + *max_z) / 2;  rad = dz / 2;
		}
	else
		if (dy > dz)
		{
			*center = (*min_y + *max_y) / 2;  rad = dy / 2;
		}
		else
		{
			*center = (*min_z + *max_z) / 2;  rad = dz / 2;
		}


	//Go through all points and look for ones that don't fit
	rad2 = rad * rad;
	for (i = 0, vp = rp->verts; i < rp->num_verts; i++, vp++)
	{
		vector delta;
		float t2;

		delta = *vp - *center;
		t2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

		//If point outside, make the sphere bigger
		if (t2 > rad2)
		{
			float t;

			t = sqrt(t2);
			rad = (rad + t) / 2;
			rad2 = rad * rad;
			*center += delta * (t - rad) / t;
		}
	}

	//We're done
	return rad;
}

//Create objects for the external rooms
void CreateRoomObjects()
{
	int objnum, r;
	room* rp;

	//First delete any old room objects
	for (objnum = 0; objnum <= Highest_object_index; objnum++)
		if (Objects[objnum].type == OBJ_ROOM)
			ObjDelete(objnum);

	//Now go through all rooms & create objects for external ones
	for (r = 0, rp = Rooms; r <= Highest_room_index; r++, rp++)
		if (rp->used && (rp->flags & RF_EXTERNAL))
		{
			vector pos;
			float rad;
			int roomnum, objnum;

			rad = ComputeRoomBoundingSphere(&pos, rp);
			roomnum = GetTerrainRoomFromPos(&pos);

			ASSERT(roomnum != -1);

			objnum = ObjCreate(OBJ_ROOM, r, roomnum, &pos, NULL);
			ASSERT(objnum != -1);	//DAJ -1FIX moved up
			Objects[objnum].size = rad;
			Objects[objnum].wall_sphere_offset = Zero_vector;
			Objects[objnum].anim_sphere_offset = Zero_vector;

			if ((rad >= MIN_BIG_OBJ_RAD) && !(Objects[objnum].flags & OF_BIG_OBJECT))
				BigObjAdd(objnum);
			
			// Type specific should have set up the size, so now we can compute the bounding box.
			ObjSetAABB(&Objects[objnum]);
		}
}


// returns the index of the first room that is being used.  Returns -1 if there are none
int FindFirstUsedRoom()
{
	for (int i = 0; i <= Highest_room_index; i++)
	{
		if (Rooms[i].used)
			return i;
	}

	Int3();	// Get Jason or Matt, no rooms in use!
	return -1;
}

// Changes a face's texture within a room
//	returns true on successs
bool ChangeRoomFaceTexture(int room_num, int face_num, int texture)
{
	if ((room_num < 0) || (room_num > Highest_room_index) || ROOMNUM_OUTSIDE(room_num) || (!Rooms[room_num].used))
	{
		mprintf((0, "Invalid room passed to ChangeRoomFaceTexture\n"));
		Int3();
		return false;
	}

	room* rp = &Rooms[room_num];

	if (face_num < 0 || face_num >= rp->num_faces)
	{
		mprintf((0, "Invalid face number passed to ChangeRoomFaceTexture.  Room=%d, you gave face #%d, there are only %d in the room\n", room_num, face_num, rp->num_faces));
		Int3();
		return false;
	}

	if (texture == -1)
	{
		mprintf((0, "not a valid texture, passed to ChangeRoomFaceTexture\n"));
		Int3();
		return false;
	}

	face* fp = &rp->faces[face_num];

	fp->tmap = texture;
	fp->flags |= FF_TEXTURE_CHANGED;
	rp->room_change_flags |= RCF_TEXTURE;
	return true;
}

// Clears the data for room changes
void ClearRoomChanges()
{
	for (int i = 0; i < MAX_ROOM_CHANGES; i++)
		Room_changes[i].used = 0;
}

// Returns index of room change allocatd, else -1 on error
int AllocRoomChange()
{
	for (int i = 0; i < MAX_ROOM_CHANGES; i++)
	{
		if (Room_changes[i].used == 0)
		{
			memset(&Room_changes[i], 0, sizeof(room_changes));
			Room_changes[i].used = 1;
			return i;
		}
	}

	Int3(); // Couldn't allocate room change!
	return -1;
}


// Does whatever fading/changing of room stuff that needs to be done this frame
void DoRoomChangeFrame()
{
	for (int i = 0; i < MAX_ROOM_CHANGES; i++)
	{
		if (!Room_changes[i].used)
			continue;

		room* rp = &Rooms[Room_changes[i].roomnum];

		float norm = (Gametime - Room_changes[i].start_time) / Room_changes[i].total_time;

		if (norm > 1)
			norm = 1.0;

		if (Room_changes[i].fog)
		{

			vector scale_color = ((Room_changes[i].end_vector - Room_changes[i].start_vector) * norm) + Room_changes[i].start_vector;
			float scale_depth = ((Room_changes[i].end_depth - Room_changes[i].start_depth) * norm) + Room_changes[i].start_depth;

			rp->flags |= RF_FOG;
			rp->room_change_flags |= RCF_CHANGING_WIND_FOG;

			rp->fog_r = scale_color.x;
			rp->fog_g = scale_color.y;
			rp->fog_b = scale_color.z;
			rp->fog_depth = scale_depth;
		}
		else
		{
			vector scale_wind = ((Room_changes[i].end_vector - Room_changes[i].start_vector) * norm) + Room_changes[i].start_vector;

			rp->room_change_flags |= RCF_CHANGING_WIND_FOG;
			rp->wind = scale_wind;
		}

		// If this room is done changing, take it out of the list and mark it as changed
		if (norm >= 1.0)
		{
			Room_changes[i].used = 0;
			if (Room_changes[i].fog)
				rp->room_change_flags |= RCF_FOG;
			else
				rp->room_change_flags |= RCF_WIND;

			rp->room_change_flags &= ~RCF_CHANGING_WIND_FOG;

			continue;
		}
	}
}

// Sets up a room to change its fog or wind over time
int SetRoomChangeOverTime(int roomnum, bool fog, vector* end, float depth_end, float time)
{
	room* rp = &Rooms[roomnum];
	int index;

	// First search to see if there is another with this same roomnum

	int found = 0;
	for (int i = 0; i < MAX_ROOM_CHANGES && !found; i++)
	{
		if (Room_changes[i].used && Room_changes[i].roomnum == roomnum && Room_changes[i].fog == fog)
		{
			found = 1;
			index = i;
		}
	}

	if (!found)
	{
		index = AllocRoomChange();
		if (index < 0)
			return -1; // failed get free slot!
	}

	Room_changes[index].roomnum = roomnum;
	Room_changes[index].fog = fog;
	Room_changes[index].end_vector = *end;
	Room_changes[index].start_time = Gametime;
	Room_changes[index].total_time = time;
	rp->room_change_flags |= RCF_CHANGING_WIND_FOG;

	if (fog)
	{
		Room_changes[index].start_depth = rp->fog_depth;
		Room_changes[index].start_vector.x = rp->fog_r;
		Room_changes[index].start_vector.y = rp->fog_g;
		Room_changes[index].start_vector.z = rp->fog_b;
		Room_changes[index].end_depth = depth_end;
	}
	else
		Room_changes[index].start_vector = rp->wind;

	return index;
}
