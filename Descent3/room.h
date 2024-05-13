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

#ifndef _ROOM_H
#define _ROOM_H

#include "pstypes.h"
#include "vecmat_external.h"
#include "gametexture.h"

#ifdef NEWEDITOR
#include "..\neweditor\ned_GameTexture.h"
#endif

#include "room_external.h"

//Sizes for some global arrays
#define MAX_ROOMS							400 		//max number of rooms in the world

//Constants for room palette (editor-specific)
#if (defined(EDITOR) || defined(NEWEDITOR))
#define FIRST_PALETTE_ROOM				MAX_ROOMS	//start of rooms for palette
#define MAX_PALETTE_ROOMS				50				//max number of loaded rooms
#else
#define MAX_PALETTE_ROOMS				0				//max number of loaded rooms
#endif


// Room change stuff
#define	MAX_ROOM_CHANGES	100
struct room_changes
{
	int roomnum;
	bool fog;
	vector start_vector,end_vector;
	float start_depth,end_depth;
	float start_time;
	float total_time;
	ubyte used;
};

//
// Globals
//
extern	room	 	Rooms[];					//global sparse array of rooms
extern	int		Highest_room_index;	//index of highest-numbered room

//
// Macros
//
//Handy macro to convert a room ptr to a room number
#define ROOMNUM(r) (r-Rooms)

// See above from RF_MINE_MASK
#define MINE_INDEX(x) ((Rooms[x].flags&RFM_MINE)>>20)

//
// Functions
//
// Zeroes out the rooms array
void InitRooms ();

#ifdef _DEBUG
// Allows a spew'er to find out if he is in a room or external to the mine
// NOTE:  THIS FUNCTION IS NOT FOR IN GAME STUFF.  It is REALLY SLOW and accurate.
// Talk to Chris if you need something like this function.
int FindPointRoom(vector *pnt);

//Put this here so we don't need to include render.h
extern bool Render_floating_triggers;

#endif

//Initalize a room, allocating memory and filling in fields
//Parameters:	rp - the room to be initialized
//					nverts - how many vertices this room will have
//					nfaces - how many faces this room wil have
//					nfaces - how many portals this room wil have
void InitRoom(room *rp,int nverts,int nfaces,int nportals);

//Initialize a room face structure, allocating memory for vertlist and uvls
void InitRoomFace(face *fp,int nverts);

//Frees a room, deallocating its memory and marking it as unused
void FreeRoom(room *rp);

//Frees all the rooms currently in use, deallocating their memory and marking them as unused
void FreeAllRooms();

//Finds the center point of a room
//Parameters:	vp - filled in with the center point
//					rp - the room whose center to find
void ComputeRoomCenter(vector *vp,room *rp);

//Computes (fills in) the surface normal of a face.
//Finds the best normal on this face by checking all sets of three vertices
//IMPORTANT:  The caller should really check the return value of this function
//Parameters:	rp,facenum - the room and face to calculate the normal for
//Returns:		true if the normal is ok
//					false if the normal has a very small (pre-normalization) magnitude
bool ComputeFaceNormal(room *rp,int facenum);

//Compute the surface normal from a list of vertices that determine a face
//Finds the best normal on this face by checking all sets of three vertices
//IMPORTANT:  The caller should really check the return value of this function
//Parameters:	normal - this is filled in with the normal
//					num_verts - how many vertices in the face
//					vertnum_list - a list of vertex numbers for this face.  these index into verts
//					verts - the array of vertices into which the elements of vertnum_list index
//Returns:		true if the normal is ok
//					false if the normal has a very small (pre-normalization) magnitude
bool ComputeNormal(vector *normal,int num_verts,short *vertnum_list,vector *verts);

//Finds the center point of a portal by averaging the points in the portal
//Parameters:	vp           - filled in with the center point
//					rp           - the room 
//					portal_index - the index of the portal whose center to find
void ComputePortalCenter(vector *vp, room *rp, int portal_index);

//Computes the center point on a face by averaging the points in the face
void ComputeCenterPointOnFace(vector *vp,room *rp,int facenum);

//Free the memory used by a room face structure
void FreeRoomFace(face *fp);

// Removes all room lightmaps from memory and sets indoor faces accordingly
void ClearAllRoomLightmaps (int external);

// Removes all room volume lights from memory
void ClearAllVolumeLights ();

// Returns the area taken up by a face
float GetAreaForFace (room *rp,int facenum);

//Check if a particular point on a wall is a transparent pixel
//Parameters:	pnt - the point we're checking
//					rp - pointer to the room that pnt is in
//					facenum - the face that pnt is on
//Returns:	true if can pass through the given point, else 0
int CheckTransparentPoint(const vector *pnt,const room *rp,const int facenum);

//Face physics flags returned by GetFacePhysicsFlags()
//Note that:
//  it is illegal for a face to have both SOLID and TRANSPARENT
//  it is legal, but probably not of interest, for a face to have SOLID & PORTAL
#define FPF_SOLID				1		//nothing passes through this face
#define FPF_TRANSPARENT		2		//face has transparency, so some things may be able to fly through it
#define FPF_PORTAL  			4		//this face is in a portal.
#define FPF_RECORD			8		//take note of when an object passes through this face

//Face physics types.  These are combinations of the above flags
#define FPT_IGNORE			0		//completey ignore this face

//Figure out how the physics should deal with a given face
//Parameters:	rp - pointer to the room the face is in
//					fp - the face we're interested in
//Returns:	bitmask of flags (see above).
inline int GetFacePhysicsFlags(const room *rp,const face *fp)
{
	int ret = 0;

	//If face is a trigger, must record
	if (fp->flags & FF_HAS_TRIGGER)
		ret |= FPF_RECORD;

	//If it's a floating trigger, then we're done
	if (fp->flags & FF_FLOATING_TRIG)
		return ret;

	if (fp->flags & FF_VOLUMETRIC)
		return ret;

	//Deal with faces that are part of a portal
	if (fp->portal_num != -1)
	{
		portal *pp = &rp->portals[fp->portal_num];

		//Mark as portal
		ret |= FPF_PORTAL;

		//Face is flythrough if we don't render the portal faces, or it's marked rendered flythrough
		if (!(pp->flags & PF_RENDER_FACES) || (pp->flags & PF_RENDERED_FLYTHROUGH))
			return ret;
	}

	//If we're here, it's either a non-portal face, or portal face that gets rendered

	//Check if the face is marked fly-through
	if (GameTextures[fp->tmap].flags & TF_FLY_THRU)
		return ret;

	//Check if the face is solid or transparent
	int bm_handle = GetTextureBitmap(fp->tmap,0);
	if (GameBitmaps[bm_handle].flags & BF_TRANSPARENT)
		ret |= FPF_TRANSPARENT;
	else
		ret |= FPF_SOLID;

	//We're done
	return ret;
}

//Computes a bounding sphere for the current room
//Parameters: center - filled in with the center point of the sphere
//		rp - the room we�re bounding
//Returns: the radius of the bounding sphere
float ComputeRoomBoundingSphere(vector *center,room *rp);

//Create objects for the external rooms
void CreateRoomObjects();

// Clears lightmaps for a single room
void ClearRoomLightmaps (int roomnum);


// returns the index of the first room that is being used.  Returns -1 if there are none
int FindFirstUsedRoom ();

// Clears specmaps for a single room
void ClearRoomSpecmaps (int roomnum);

// Removes all room specularity maps from memory and sets indoor faces accordingly
// External=1 means to perform the operation on external rooms only, 0 means indoor rooms only
void ClearAllRoomSpecmaps (int external);

extern void GetIJ(const vector *normal,int *ii,int *jj);

// Changes a face's texture within a room
//	returns true on successs
bool ChangeRoomFaceTexture(int room_num,int face_num,int texture);

// Clears the data for room changes
void ClearRoomChanges ();

// Returns index of room change allocatd, else -1 on error
int AllocRoomChange ();

// Does whatever fading/changing of room stuff that needs to be done this frame
void DoRoomChangeFrame ();


// Sets up a room to change its fog or wind over time
int SetRoomChangeOverTime (int roomnum,bool fog,vector *end,float depth_end,float time);

#endif
