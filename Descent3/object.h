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

#ifndef _OBJECT_H
#define _OBJECT_H

#include "pstypes.h"
#include "pserror.h"
#include "object_external_struct.h"
#include "object_external.h"

/*
 *		CONSTANTS
 */

//Object handle stuff.  
//The handle is comprised of the object number in the low 10 bits, and a count in the high 22 bits.
#define HANDLE_OBJNUM_MASK			0x7ff			//to mask off the object number part of the handle
#define HANDLE_COUNT_MASK			0xfffff800	//to maks off the count part of the handle
#define HANDLE_COUNT_INCREMENT		0x800			//what gets added to the handle to increment it

// See object external for OBJ_ types

// Lighting render types
#define LRT_STATIC		0
#define LRT_GOURAUD		1
#define LRT_LIGHTMAPS	2

extern char	*Object_type_names[MAX_OBJECT_TYPES];


//stuctures for different kinds of weapon simulation (for precompution)

#define WPC_NOT_USED	     0
#define WPC_NO_COLLISIONS 1
#define WPC_HIT_WALL		  2


#define FMA_VALID	    1
#define FMA_CURRENT   2
#define FMA_LOOPING   4
#define FMA_USE_SPEED 8
#define FMA_HAS_AI	 16


#define FMT_NEW_DATA	1
#define FMT_UPDATING	2

//object light info flags
#define OLF_FLICKERING			1
#define OLF_TIMEBITS				2
#define OLF_PULSE					4
#define OLF_PULSE_TO_SECOND	8
#define OLF_FLICKER_SLIGHTLY	16		
#define OLF_DIRECTIONAL			32		// Directional light - casts light in a cone
#define OLF_NO_SPECULARITY		64		// Object does not have specular light cast on it

// OSIRIS defines
#define MAX_MODULENAME_LEN	32

//How long an object name can be
#define OBJ_NAME_LEN 19		//max length for object name

/*
 *		VARIABLES
 */

extern object Objects[];
extern int Highest_object_index;		//highest objnum

extern object *Player_object;			//the object that is the player
extern object *Viewer_object;			//which object we are seeing from

#define MAX_BIG_OBJECTS	350
extern int Num_big_objects;
extern short BigObjectList[MAX_BIG_OBJECTS];	//DAJ_MR utb int

//Compute the object number from an object pointer
#define OBJNUM(objp) (objp-Objects)
#define OBJHANDLE(objp) ((objp) ? (objp)->handle : 0)


/*
 *		FUNCTIONS
 */


// Set the dead flag for an object
void SetObjectDeadFlag (object *obj,bool tell_clients_to_remove = false,bool play_sound_on_clients=false);
inline void SetObjectDeadFlag (object *obj,bool tell_clients_to_remove,bool play_sound_on_clients)
{
	int objnum=OBJNUM(obj);
	ASSERT(objnum != -1);
	ASSERT(objnum != 0 );
	ASSERT(obj->type != OBJ_NONE);
	ASSERT(obj != Player_object);

	obj->flags|=OF_DEAD;

	if(tell_clients_to_remove){
		if(play_sound_on_clients)
		{
			obj->flags|=OF_SEND_MULTI_REMOVE_ON_DEATHWS;
		}else
		{
			obj->flags|=OF_SEND_MULTI_REMOVE_ON_DEATH;
		}
	}
}

void SetObjectControlType(object *obj, int control_type);

//do whatever setup needs to be done
void InitObjects(void);

//links an object into a room's list of objects.
//takes object number and room number
void ObjLink(int objnum,int roomnum);

// reverses ObjLink.
void ObjUnlink(int objnum);

// Sets the AABB for the object
void ObjSetAABB(object *obj);

//initialize a new object.  adds to the list for the given room
//returns the object number
int ObjCreate(ubyte type,ushort id,int roomnum,vector *pos,const matrix *orient,int parent_handle = OBJECT_HANDLE_NONE);

//remove object from the world
void ObjDelete(int objnum);

//Resets the handles for all the objects.  Called by the editor to init a new level.
void ResetObjectList();

//Builds the free object list by scanning the list of free objects & adding unused ones to the list
//Also sets Highest_object_index
void ResetFreeObjects();

// Frees all the objects that are currently in use
void FreeAllObjects();

//Deletes all objects that have been marked for death.
void ObjDeleteDead();

//Process all objects for the current frame
void ObjDoFrameAll();

//set viewer object to next object in array
void ObjGotoNextViewer();

//move an object for the current frame
void ObjMoveOne( object * obj );

//Sets the position of an object.  This should be called whenever an object moves.
//Parameters:	obj - the object being moved
//					pos - the new position
//					roomnum - the correct roomnum for pos.  No error checking is done.
//					orient - if this is not null, the object's orientation is set to this.
void ObjSetPos(object *obj,vector *pos,int roomnum,matrix *orient, bool f_update_attached_children);
void ObjSetOrient(object *obj,const matrix *orient);

//delete objects, such as weapons & explosions, that shouldn't stay between levels
//if clear_all is set, clear even proximity bombs
void ClearTransientObjects(int clear_all);

// Remaps all static powerups,sounds,etc to their appropriate indices
void RemapEverything();

void BigObjAdd(int objnum);
void InitBigObjects(void);

//Creates the player object in the center of the given room
void CreatePlayerObject(int roomnum);

//Retruns a pointer to an object given its handle.  Returns NULL if object no longer exists.
object *ObjGet(int handle);

//	returns a vertex of an object in WORLD coordinates.
void GetObjectPointInWorld (vector *dest,object *obj,int subnum,int vertnum);

// These functions are for setting and getting an objects animation information 
// (used in multiplayer games and the like)
bool ObjGetAnimUpdate(unsigned short objnum, custom_anim *multi_anim_info);
void ObjSetAnimUpdate(unsigned short objnum, custom_anim *multi_anim_info);

void ObjGetTurretUpdate(unsigned short objnum, multi_turret *multi_turret_info);
void ObjSetTurretUpdate(unsigned short objnum, multi_turret *multi_turret_info);

//Returns the original parent for the given object.  Returns self if it has no parent
object *ObjGetUltimateParent(object *child);

//Sets an object to a type OBJ_DUMMY (saves its old type) so it won't be renderered, etc, but still alive
void ObjGhostObject(int objnum);

//Restores a ghosted object back to it's old type
void ObjUnGhostObject(int objnum);

/////////////////////////////////
// Position history information
// ---Not to be externalized---
// Used for motion blur
/////////////////////////////////
#define MAX_OBJECT_POS_HISTORY		(MAX_OBJECTS/2)
#define MAX_POSITION_HISTORY		3

struct tPosHistory
{
	vector pos[MAX_POSITION_HISTORY];	
};

extern tPosHistory Object_position_samples[MAX_OBJECT_POS_HISTORY];
extern ubyte Object_position_head;
extern signed short Object_map_position_history[MAX_OBJECTS];
extern float Last_position_history_update[MAX_POSITION_HISTORY];// last gametime the positions were updated
void ObjInitPositionHistory(object *obj);
void ObjFreePositionHistory(object *obj);
void ObjResetPositionHistory(void);
void ObjReInitPositionHistory(void);
#endif
