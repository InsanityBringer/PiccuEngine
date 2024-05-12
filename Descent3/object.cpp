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

#include <stdlib.h>
#include <string.h>	// for memset
#include <stdio.h>

#include "object.h"

#include "pserror.h"
#include "vecmat.h"
#include "mono.h"

#include "descent.h"
#include "player.h"
#include "slew.h"
#include "game.h"
#include "trigger.h"
#include "PHYSICS.H"
#include "collide.h"
#include "door.h"
#include "controls.h"
#include "terrain.h"
#include "polymodel.h"
#include "gametexture.h"
#include "ship.h"
#include "soundload.h"
#include "weapon.h"
#include "objinit.h"
#include "fireball.h"
#include "hlsoundlib.h"
#include "sounds.h"
#include "AIMain.h"
#include "room.h"
#include "objinfo.h"
#include "lighting.h"
#include "findintersection.h"
#include "lightmap_info.h"
#include "object_lighting.h"
#include "splinter.h"
#include "ObjScript.h"
#include "viseffect.h"
#include "multi.h"
#include "game2dll.h"
#include "robot.h"
#include "damage.h"
#include "attach.h"
#include "dedicated_server.h"
#include "hud.h"
#include "demofile.h"
#include "rtperformance.h"
#include "osiris_dll.h"
#include "gameloop.h"
#include "mem.h"
#include "stringtable.h"
#include "levelgoal.h"
#include "psrand.h"
#include "vibeinterface.h"

#ifdef EDITOR
#include "editor\d3edit.h"
#endif

/*
 *  Global variables
 */

object* Player_object = NULL;		//the object that is the player
object* Viewer_object = NULL;		//which object we are seeing from

static short free_obj_list[MAX_OBJECTS];

//Data for objects

// -- Object stuff

//info on the various types of objects

object Objects[MAX_OBJECTS];

tPosHistory Object_position_samples[MAX_OBJECT_POS_HISTORY];
ubyte Object_position_head;
signed short Object_map_position_history[MAX_OBJECTS];
short Object_map_position_free_slots[MAX_OBJECT_POS_HISTORY];
unsigned short Num_free_object_position_history;

int Num_objects = 0;
int Highest_object_index = 0;
int Highest_ever_object_index = 0;

int print_object_info = 0;

#ifdef EDITOR
//This array matches the object types in object.h
char* Object_type_names[MAX_OBJECT_TYPES] = {
	"WALL",			//OBJ_WALL				0
	"FIREBALL",		//OBJ_FIREBALL			1
	"ROBOT",			//OBJ_ROBOT				2
	"SHARD",			//OBJ_SHARD				3
	"PLAYER",		//OBJ_PLAYER			4
	"WEAPON",		//OBJ_WEAPON			5
	"VIEWER",		//OBJ_VIEWER			6
	"POWERUP",		//OBJ_POWERUP			7
	"DEBRIS",		//OBJ_DEBRIS			8
	"CAMERA",		//OBJ_CAMERA			9
	"SHOCKWV",		//OBJ_SHOCKWAVE		10
	"CLUTTER",		//OBJ_CLUTTER			11
	"GHOST",			//OBJ_GHOST				12
	"LIGHT",			//OBJ_LIGHT				13
	"COOP",			//OBJ_COOP				14
	"UNUSED",		//OBJ_MARKER			15
	"BUILDING",		//OBJ_BUILDING			16
	"DOOR",			//OBJ_DOOR				17
	"ROOM",			//OBJ_ROOM				18
	"LINE",			//OBJ_PARTICLE			19
	"SPLINTER",		//OBJ_SPLINTER			20
	"DUMMY",			//OBJ_DUMMY				21
	"OBSERVER",		//OBJ_OBSERVER			22
	"DEBUG LINE",	//OBJ_DEBUG_LINE		23
	"SOUNDSOURCE",	//OBJ_SOUNDSOURCE		24
	"WAYPOINT",		//OBJ_WAYPOINT			25
};
#endif

int Num_big_objects = 0;
short BigObjectList[MAX_BIG_OBJECTS];	//DAJ_MR utb int

/*
 *  Local Function Prototypes
 */

 //Do refueling centers & damage rooms
void DoSpecialPlayerStuff(object* obj);


/*
 *  Functions
 */


#ifndef RELEASE
 //set viewer object to next object in array
void ObjGotoNextViewer()
{
	int start_obj = Viewer_object - Objects; //get viewer object number

	for (int i = 0; i <= Highest_object_index; i++)
	{
		start_obj++;
		if (start_obj > Highest_object_index)
			start_obj = 0;

		if (Objects[start_obj].type != OBJ_NONE)
		{
			Viewer_object = &Objects[start_obj];
			return;
		}
	}

	Error("Couldn't find a viewer object!");
}

//set viewer object to next object in array
void obj_goto_prev_viewer()
{
	int start_obj = Viewer_object - Objects;		//get viewer object number

	for (int i = 0; i <= Highest_object_index; i++)
	{
		start_obj--;
		if (start_obj < 0)
			start_obj = Highest_object_index;

		if (Objects[start_obj].type != OBJ_NONE)
		{
			Viewer_object = &Objects[start_obj];
			return;
		}
	}

	Error("Couldn't find a viewer object!");
}
#endif


object* obj_find_first_of_type(int type)
{
	for (int i = 0; i <= Highest_object_index; i++)
		if (Objects[i].type == type)
			return (&Objects[i]);

	return NULL;
}


int obj_return_num_of_type(int type)
{
	int count = 0;

	for (int i = 0; i <= Highest_object_index; i++)
		if (Objects[i].type == type)
			count++;

	return (count);
}


int obj_return_num_of_typeid(int type, int id)
{
	int count = 0;

	for (int i = 0; i <= Highest_object_index; i++)
		if (Objects[i].type == type && Objects[i].id == id)
			count++;

	return (count);
}


int ObjAllocate(void);

//Creates the player object in the center of the given room
void CreatePlayerObject(int roomnum)
{
	vector pos;

	ComputeRoomCenter(&pos, &Rooms[roomnum]);

	int objnum = ObjCreate(OBJ_PLAYER, 0, roomnum, &pos, NULL);

	ASSERT(objnum == 0);			//player must be object 0

	Player_object = Viewer_object = &Objects[objnum];
}

void InitBigObjects()
{
	Num_big_objects = 0;
}

void BigObjAdd(int objnum)
{
	ASSERT(Num_big_objects < MAX_BIG_OBJECTS);
	if (Num_big_objects >= MAX_BIG_OBJECTS)
	{
		Int3();
		return;
	}

	Objects[objnum].flags |= OF_BIG_OBJECT;
	BigObjectList[Num_big_objects++] = objnum;
}

void BigObjRemove(int objnum)
{
	int i;

	Objects[objnum].flags &= (~OF_BIG_OBJECT);

	for (i = 0; i < Num_big_objects; i++)
	{
		if (BigObjectList[i] == objnum)
		{
			Num_big_objects--;
			break;
		}
	}

	while (i < Num_big_objects)
	{
		BigObjectList[i] = BigObjectList[i + 1];
		i++;
	}

	ASSERT(Num_big_objects < MAX_BIG_OBJECTS);
	ASSERT(Num_big_objects >= 0);
}


//Resets the object list: sets all objects to unused, intializes handles, & sets roomnums to -1
//Called by the editor to init a new level.
void ResetObjectList()
{
	//Init data for each object
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		Objects[i].handle = i;
		Objects[i].type = OBJ_NONE;
		Objects[i].roomnum = -1;
	}

	//Build the free object list
	ResetFreeObjects();

	//Say no big objects
	InitBigObjects();

	ObjResetPositionHistory();
}


//sets up the free list & init player & whatever else
void InitObjects()
{
	//Initialize the collision system
	CollideInit();

	//Mark all the objects as unused
	ResetObjectList();

	//Make sure no rooms are using any objects
	for (int i = 0; i < MAX_ROOMS; i++)
	{
		Rooms[i].objects = -1;
		Rooms[i].vis_effects = -1;
	}

	InitVisEffects();

	atexit(FreeAllObjects);
}

//link the object into the list for its room
void ObjLink(int objnum, int roomnum)
{
	object* obj = &Objects[objnum];

	ASSERT(objnum != -1);

	ASSERT(obj->roomnum == -1);
	ASSERT(!(obj->flags & OF_BIG_OBJECT));

	if ((obj->size >= MIN_BIG_OBJ_RAD) && (!ROOMNUM_OUTSIDE(roomnum)))
	{
		BigObjAdd(objnum);
	}

	obj->roomnum = roomnum;

	if (ROOMNUM_OUTSIDE(roomnum))
	{
		int cellnum = CELLNUM(roomnum);

		obj->next = Terrain_seg[cellnum].objects;
		Terrain_seg[cellnum].objects = objnum;

		ASSERT(obj->next != objnum);
	}
	else
	{
		ASSERT(roomnum >= 0 && roomnum <= Highest_room_index);
		ASSERT(!(Rooms[roomnum].flags & RF_EXTERNAL));

		obj->next = Rooms[roomnum].objects;
		Rooms[roomnum].objects = objnum;
		ASSERT(obj->next != objnum);
	}

	obj->prev = -1;

	if (obj->next != -1) Objects[obj->next].prev = objnum;

	ASSERT(Objects[0].next != 0);
	if (Objects[0].next == 0)
		Objects[0].next = -1;

	ASSERT(Objects[0].prev != 0);
	if (Objects[0].prev == 0)
		Objects[0].prev = -1;
}

void ObjUnlink(int objnum)
{
	object* obj = &Objects[objnum];

	ASSERT(objnum != -1);

	if (obj->flags & OF_BIG_OBJECT)
	{
		BigObjRemove(objnum);
	}

	if (OBJECT_OUTSIDE(obj))
	{
		// zar: room hasn't been assigned yet? why is this being unlinked???
		// this basically causes cellnum to be INT_MAX and 
		// cause seg to be out of bounds!!
		if (obj->roomnum == -1)
			return;

		int cellnum = CELLNUM(obj->roomnum);
		assert(cellnum >= 0 && cellnum <= 65535);

		terrain_segment* seg = &Terrain_seg[cellnum];

		if (obj->prev == -1)
			seg->objects = obj->next;
		else
			Objects[obj->prev].next = obj->next;

		if (obj->next != -1) Objects[obj->next].prev = obj->prev;
	}
	else
	{
		room* rp = &Rooms[obj->roomnum];

		if (obj->prev == -1)
			rp->objects = obj->next;
		else
			Objects[obj->prev].next = obj->next;

		if (obj->next != -1) Objects[obj->next].prev = obj->prev;

	}

	//Mark as not linked
	obj->roomnum = -1;

	ASSERT(Objects[0].next != 0);
	ASSERT(Objects[0].prev != 0);
}

//-----------------------------------------------------------------------------
//	Scan the object list, freeing down to num_used objects
//	Returns number of slots freed.
int FreeObjectSlots(int num_used)
{
	int	i, olind;
	int	obj_list[MAX_OBJECTS];
	int	num_already_free, num_to_free, original_num_to_free;

	olind = 0;
	num_already_free = MAX_OBJECTS - Highest_object_index - 1;

	if (MAX_OBJECTS - num_already_free < num_used)
		return 0;

	for (i = 0; i <= Highest_object_index; i++)
	{
		if (Objects[i].flags & OF_DEAD)
		{
			num_already_free++;
			if (MAX_OBJECTS - num_already_free < num_used)
				return num_already_free;
		}
		else
		{
			switch (Objects[i].type)
			{
			case OBJ_NONE:
				num_already_free++;
				if (MAX_OBJECTS - num_already_free < num_used)
					return 0;
				break;
			case OBJ_WALL:
				Int3();		//	This is curious.  What is an object that is a wall?
				break;
			case OBJ_FIREBALL:
			case OBJ_WEAPON:
			case OBJ_DEBRIS:
			case OBJ_SPLINTER:
				obj_list[olind++] = i;
				break;
			default:
				break;
			}
		}
	}

	num_to_free = MAX_OBJECTS - num_used - num_already_free;
	original_num_to_free = num_to_free;

	if (num_to_free > olind) {
		mprintf((1, "Warning: Asked to free %i objects, but can only free %i.\n", num_to_free, olind));
		num_to_free = olind;
	}

	for (i = 0; i < num_to_free; i++)
		if (Objects[obj_list[i]].type == OBJ_DEBRIS)
		{
			num_to_free--;
			mprintf((0, "Freeing   DEBRIS object %3i\n", obj_list[i]));
			SetObjectDeadFlag(&Objects[obj_list[i]]);
		}

	if (!num_to_free)
		return original_num_to_free;

	if (!num_to_free)
		return original_num_to_free;

	if (!num_to_free)
		return original_num_to_free;

	for (i = 0; i < num_to_free; i++)
		if ((Objects[obj_list[i]].type == OBJ_WEAPON))
		{
			num_to_free--;
			mprintf((0, "Freeing   WEAPON object %3i\n", obj_list[i]));
			SetObjectDeadFlag(&Objects[obj_list[i]]);
		}

	return original_num_to_free - num_to_free;
}

//returns the number of a free object, updating Highest_object_index.
//Generally, ObjCreate() should be called to get an object, since it
//fills in important fields and does the linking.
//returns -1 if no free objects
int ObjAllocate(void)
{
	int objnum;

	if (Num_objects >= MAX_OBJECTS - 2)
	{
		int	num_freed = FreeObjectSlots(MAX_OBJECTS - 10);
		mprintf((0, " *** Freed %i objects in frame %i\n", num_freed, FrameCount));
	}

	if (Num_objects >= MAX_OBJECTS)
	{
		mprintf((1, "Object creation failed - too many objects!\n"));
		return -1;
	}

	objnum = free_obj_list[Num_objects++];

	if (objnum > Highest_object_index)
	{
		Highest_object_index = objnum;
		if (Highest_object_index > Highest_ever_object_index)
		{
			Highest_ever_object_index = Highest_object_index;
			mprintf((0, "Highest ever Object %d\n", Highest_ever_object_index));
		}
	}

	return objnum;
}


//frees up an object.  Generally, ObjDelete() should be called to get
//rid of an object.  This function deallocates the object entry after
//the object has been unlinked
void ObjFree(int objnum)
{
	ObjFreePositionHistory(&Objects[objnum]);

	free_obj_list[--Num_objects] = objnum;
	ASSERT(Num_objects >= 0);

	if (objnum == Highest_object_index)
		while (Objects[--Highest_object_index].type == OBJ_NONE);
}


void ObjSetAABB(object* obj)
{
	vector object_rad;

	if (obj->type == OBJ_ROOM)
	{
		obj->min_xyz = Rooms[obj->id].min_xyz;
		obj->max_xyz = Rooms[obj->id].max_xyz;
	}
	else if (obj->flags & OF_POLYGON_OBJECT &&
		obj->type != OBJ_WEAPON &&
		obj->type != OBJ_DEBRIS &&
		obj->type != OBJ_POWERUP &&
		obj->type != OBJ_PLAYER)
	{
		vector offset_pos;

		object_rad.x = object_rad.y = object_rad.z = Poly_models[obj->rtype.pobj_info.model_num].anim_size;
		offset_pos = obj->pos + obj->anim_sphere_offset;

		obj->min_xyz = offset_pos - object_rad;
		obj->max_xyz = offset_pos + object_rad;
	}
	else
	{
		object_rad.x = obj->size;
		object_rad.y = obj->size;
		object_rad.z = obj->size;

		obj->min_xyz = obj->pos - object_rad;

		obj->max_xyz = obj->pos + object_rad;
	}
}

//-----------------------------------------------------------------------------
//initialize a new object.  adds to the list for the given room
//returns the object number
int ObjCreate(ubyte type, ushort id, int roomnum, vector* pos, const matrix* orient, int parent_handle)
{
	int objnum;
	object* obj;
	int handle;

	ASSERT(type != OBJ_NONE);

	if (ROOMNUM_OUTSIDE(roomnum))
	{
		ASSERT(CELLNUM(roomnum) <= TERRAIN_WIDTH * TERRAIN_DEPTH);
		ASSERT(CELLNUM(roomnum) >= 0);

		roomnum = GetTerrainRoomFromPos(pos);

		if (roomnum == -1)
			return -1;
	}

	//Get next free object
	objnum = ObjAllocate();
	if (objnum == -1)		//no free objects
		return -1;
	obj = &Objects[objnum];

	//Make sure the object is ok
	ASSERT(obj->type == OBJ_NONE);		//make sure unused 
	ASSERT(obj->roomnum == -1);			//make sure unlinked

	//Compute the new handle
	handle = obj->handle + HANDLE_COUNT_INCREMENT;

	//Check for handle wrap
	if ((handle & HANDLE_COUNT_MASK) == HANDLE_COUNT_MASK)	//Going to wrap!
		Int3();		//Show this to Matt, or email him if he's not here

	//Initialize the object
	if (!ObjInit(obj, type, id, handle, pos, roomnum, Gametime, parent_handle))
	{
		//Couldn't init!
		obj->type = OBJ_NONE;		//mark as unused
		ObjFree(objnum);				//de-allocate object
		return -1;
	}

#ifdef _DEBUG
	if (print_object_info)
	{
		mprintf((0, "Created object %d of type %d\n", objnum, obj->type));
	}
#endif

	//Set the object's orietation
	// THIS MUST BE DONE AFTER ObjInit (as ObjInit could load in a polymodel and set the anim and wall offsets)
	ObjSetOrient(obj, orient ? orient : &Identity_matrix);

	//Link object into room or terrain cell
	ObjLink(objnum, roomnum);

	// Type specific should have set up the size, so now we can compute the bounding box.
	ObjSetAABB(obj);

	//Let the demo system know about this object
	DemoWriteObjCreate(type, id, roomnum, pos, orient, parent_handle, obj);

	ObjInitPositionHistory(obj);

	// Check bad init code that doesn't call ComputeDefaultSize()
	ASSERT(!(obj->type != OBJ_ROOM && obj->type != OBJ_DEBRIS && (obj->flags & OF_POLYGON_OBJECT) && !(Poly_models[obj->rtype.pobj_info.model_num].flags & PMF_SIZE_COMPUTED)));

	//Done!
	return objnum;
}


void ObjInitPositionHistory(object* obj)
{
	ASSERT(Object_map_position_history[OBJNUM(obj)] == -1);

	// If it is one of the types of object's that we have to sample positions for
	// Initialize that
	switch (obj->type)
	{
	case OBJ_ROBOT:
	case OBJ_WEAPON:
	case OBJ_DEBRIS:
		if (Num_free_object_position_history > 0)
		{
			// we have a free slot
			Num_free_object_position_history--;
			int free_slot = Object_map_position_free_slots[Num_free_object_position_history];
			Object_map_position_history[OBJNUM(obj)] = free_slot;

			for (int i = 0; i < MAX_POSITION_HISTORY; i++)
				Object_position_samples[free_slot].pos[i] = obj->pos;
		}
		else
		{
			// out of slots!
			Object_map_position_history[OBJNUM(obj)] = -1;
		}break;
	default:
		// not saving positions
		Object_map_position_history[OBJNUM(obj)] = -1;
		break;

	}
}

void ObjFreePositionHistory(object* obj)
{
	int objnum = OBJNUM(obj);

	if (Object_map_position_history[objnum] != -1)
	{
		int slot_to_free = Object_map_position_history[objnum];
		Object_map_position_history[objnum] = -1;

		Object_map_position_free_slots[Num_free_object_position_history] = slot_to_free;
		Num_free_object_position_history++;
		ASSERT(Num_free_object_position_history <= MAX_OBJECT_POS_HISTORY);
	}
}

void ObjResetPositionHistory(void)
{
	Num_free_object_position_history = MAX_OBJECT_POS_HISTORY;
	Object_position_head = 0;

	int i;

	for (i = 0; i < MAX_OBJECTS; i++)
	{
		Object_map_position_history[i] = -1;

		if (i < MAX_OBJECT_POS_HISTORY)
		{
			// this array is guaranteed smaller than Objects[]
			// so instead of looping through an array again just for
			// this, I'll stick it in this loop.
			Object_map_position_free_slots[i] = i;
		}
	}

	for (i = 0; i < MAX_POSITION_HISTORY; i++)
	{
		Last_position_history_update[i] = 0 - (0.1f * i);
	}
}


void ObjReInitPositionHistory(void)
{
	ObjResetPositionHistory();

	for (int i = 0; i <= Highest_object_index; i++)
	{
		if (Objects[i].type != OBJ_NONE)
		{
			ObjInitPositionHistory(&Objects[i]);
		}
	}
}


extern void newdemo_record_guided_end();

//remove object from the world
void ObjDelete(int objnum)
{
	object* obj = &Objects[objnum];

	if (obj->type == OBJ_DUMMY)
	{
		//unghost the object before destroying it
		ObjUnGhostObject(objnum);
	}


	/*	if ((Game_mode & GM_MULTI) && Netgame.local_role==LR_CLIENT && Objects[objnum].type==OBJ_POWERUP)
		{
			mprintf ((0,"Deleting object number %d type=%d id=%d\n",objnum,obj->type,obj->id));
		}*/

	if ((Game_mode & GM_MULTI) && (obj->flags & OF_SERVER_OBJECT))
	{
		if (Netgame.local_role == LR_CLIENT && NetPlayers[Player_num].sequence > NETSEQ_OBJECTS && NetPlayers[Player_num].sequence != NETSEQ_LEVEL_END)
		{
			if (!(obj->flags & OF_SERVER_SAYS_DELETE))
			{
				mprintf((0, "Illegally deleting server object! Objnum=%d type=%d id=%d\n", objnum, obj->type, obj->id));
				Int3();
				return;
			}

		}
	}

	if (obj->flags & OF_POLYGON_OBJECT)
	{
		polyobj_info* p_info = &obj->rtype.pobj_info;
		if (p_info->multi_turret_info.keyframes != NULL)
		{
			mem_free(p_info->multi_turret_info.keyframes);
			mem_free(p_info->multi_turret_info.last_keyframes);

			p_info->multi_turret_info.keyframes = NULL;
			p_info->multi_turret_info.last_keyframes = NULL;
		}
	}

	ASSERT(objnum != -1);
	//	ASSERT(objnum != 0 );
	ASSERT(obj->type != OBJ_NONE);
	//	ASSERT(obj != Player_object);

	if (obj == Viewer_object)		//deleting the viewer?
		Viewer_object = Player_object;						//..make the player the viewer

	// Delete this chase camera if needed
	if (Player_camera_objnum == objnum)
		Player_camera_objnum = -1;

#ifdef _DEBUG
	if (print_object_info)
	{
		mprintf((0, "Deleting object %d of type %d\n", objnum, Objects[objnum].type));
	}
#endif

	if (obj->type == OBJ_WEAPON && obj->ctype.laser_info.parent_type == OBJ_PLAYER)
	{
		int pnum = Objects[(obj->parent_handle & HANDLE_OBJNUM_MASK)].id;

		if (Players[pnum].guided_obj == obj)
		{
			mprintf((0, "Deleting a guided missile!"));
			Players[pnum].guided_obj = NULL;
		}
	}

	if (obj->type == OBJ_WEAPON && obj->ctype.laser_info.parent_type == OBJ_PLAYER)
	{
		int pnum = Objects[(obj->parent_handle & HANDLE_OBJNUM_MASK)].id;

		if (Players[pnum].user_timeout_obj == obj)
		{
			mprintf((0, "Deleting a timeout missile!"));
			Players[pnum].user_timeout_obj = NULL;
		}
	}

	ObjUnlink(objnum);

	ASSERT(Objects[0].next != 0);

	// Update powerup respawn point
	if ((Game_mode & GM_MULTI) && Netgame.local_role == LR_SERVER)
	{
		if (obj->type == OBJ_POWERUP && Object_info[obj->id].respawn_scalar > 0)
		{
			for (int i = 0; i < Num_powerup_respawn; i++)
			{
				if (Powerup_respawn[i].used && Powerup_respawn[i].objnum == objnum)
				{
					Powerup_respawn[i].used = 0;
					Powerup_respawn[i].objnum = -1;
					Powerup_timer[Num_powerup_timer].respawn_time = Gametime + (Netgame.respawn_time * Object_info[obj->id].respawn_scalar);
					Powerup_timer[Num_powerup_timer].id = obj->id;
					Num_powerup_timer++;
					mprintf((0, "Adding powerup id %d to respawn list! count=%d\n", obj->id, Num_powerup_timer));
				}

			}
		}

	}

	//	Kill any script thread associated with this object.
	FreeObjectScripts(obj, false);

	if (obj->custom_default_script_name)
	{
		mem_free(obj->custom_default_script_name);
		obj->custom_default_script_name = NULL;
	}

	if (obj->custom_default_module_name)
	{
		mem_free(obj->custom_default_module_name);
		obj->custom_default_module_name = NULL;
	}

	obj->type = OBJ_NONE;		//unused!
	obj->roomnum = -1;				// zero it!

	// Free lightmap memory
	if (obj->lm_object.used)
		ClearObjectLightmaps(obj);

	// Free up effects memory
	if (obj->effect_info)
	{
		mem_free(obj->effect_info);
		obj->effect_info = NULL;
	}

	if (obj->ai_info != NULL)
	{
		AIDestroyObj(obj);
		mem_free(obj->ai_info);
		obj->ai_info = NULL;
	}

	if (obj->dynamic_wb != NULL)
	{
		mem_free(obj->dynamic_wb);
		obj->dynamic_wb = NULL;
	}

	if (obj->attach_children != NULL)
	{
		mem_free(obj->attach_children);
		obj->attach_children = NULL;
	}

	if (obj->name)
	{
		mem_free(obj->name);
		obj->name = NULL;
	}

	if (obj->lighting_info)
	{
		mem_free(obj->lighting_info);
		obj->lighting_info = NULL;
	}

	ObjFree(objnum);
}


// Frees all the objects that are currently in use
void FreeAllObjects()
{
	int objnum;

	for (objnum = 0; objnum <= Highest_object_index; objnum++)
		if (Objects[objnum].type != OBJ_NONE)
		{
			Objects[objnum].flags |= OF_SERVER_SAYS_DELETE;
			Objects[objnum].flags &= ~OF_INPLAYERINVENTORY;
			ObjDelete(objnum);
		}

	FreeAllVisEffects();
}

//	------------------------------------------------------------------------------------------------------------------
void ObjDeleteDead()
{
	object* objp;
	int		local_dead_player_object = -1;

	// Move all objects
	objp = Objects;

	for (int i = 0; i <= Highest_object_index; i++)
	{
		if ((objp->type != OBJ_NONE) && (objp->flags & OF_DEAD))
		{
			if (objp->flags & OF_INFORM_DESTROY_TO_LG)
			{
				Level_goals.Inform(LIT_OBJECT, LGF_COMP_DESTROY, objp->handle);
			}

			if (objp->flags & OF_INPLAYERINVENTORY)
			{
				//this object is in a player inventory somewhere...remove it first!
				InventoryRemoveObject(objp->handle);
			}

			if (objp->type == OBJ_DUMMY)
			{
				//unghost the object before deleting it
				ObjUnGhostObject(i);

				//tell clients to unghost
				if ((Game_mode & GM_MULTI) && (Netgame.local_role != LR_CLIENT))
				{
					MultiSendGhostObject(&Objects[i], false);
				}
			}

			if (objp->flags & OF_POLYGON_OBJECT)
			{
				if (objp->flags & OF_ATTACHED)
				{
					tOSIRISEventInfo ei;
					ei.evt_child_died.it_handle = objp->handle;

					Osiris_CallEvent(ObjGet(objp->attach_ultimate_handle), EVT_CHILD_DIED, &ei);
				}

				UnattachFromParent(objp);
				UnattachChildren(objp);
			}

			//	Execute script for death
			tOSIRISEventInfo ei;
			ei.evt_destroy.is_dying = 1;
			Osiris_CallEvent(objp, EVT_DESTROY, &ei);

			if (Game_mode & GM_MULTI)
			{
				DLLInfo.me_handle = DLLInfo.it_handle = objp->handle;
				CallGameDLL(EVT_GAMEOBJDESTROYED, &DLLInfo);
			}

			//detach any scripts attached
			Osiris_DetachScriptsFromObject(objp);

			//if it's flag to tell the clients to remove is set, tell them now
			if ((Game_mode & GM_MULTI) && (Netgame.local_role != LR_CLIENT))
			{
				bool removed = false;

				if (objp->flags & OF_SEND_MULTI_REMOVE_ON_DEATH)
				{
					// Tell all clients to remove this object
					MultiSendRemoveObject(objp, 0);
					removed = true;
				}
				if (!removed && objp->flags & OF_SEND_MULTI_REMOVE_ON_DEATHWS)
				{
					// Tell all clients to remove this object with sound
					MultiSendRemoveObject(objp, 1);
					removed = true;
				}
			}

			if (IS_GUIDEBOT(objp))
			{
				int handle = objp->handle;

				for (int j = 0; j < MAX_PLAYERS; j++)
				{
					if (objp->handle == Buddy_handle[j])
					{
						Buddy_handle[j] = OBJECT_HANDLE_NONE;
						break;
					}
				}
			}

			//	check if player being killed.
			if (objp->type != OBJ_PLAYER)
			{
				ObjDelete(i);
			}
		}
		objp++;
	}

	// Delete our visual effects
	VisEffectDeleteDead();
}


//when an object has moved into a new room, this function unlinks it
//from its old room and links it into the new room
void ObjRelink(int objnum, int newroomnum)
{
	ASSERT((objnum >= 0) && (objnum <= Highest_object_index));

	if (!ROOMNUM_OUTSIDE(newroomnum))
	{
		ASSERT((newroomnum <= Highest_room_index) && (newroomnum >= 0));
		ASSERT(Rooms[newroomnum].used);
	}
	else
		ASSERT(CELLNUM(newroomnum) >= 0 && CELLNUM(newroomnum) <= (TERRAIN_WIDTH * TERRAIN_DEPTH));

	ObjUnlink(objnum);
	ObjLink(objnum, newroomnum);
}


void DoCycledAnim(object* obj)
{
	float from;
	float to;
	float delta;
	float spc;

	float anim_time = Frametime;

	if (!(obj->flags & OF_POLYGON_OBJECT))
		return;

	from = Object_info[obj->id].anim[MC_STANDING].elem[AS_ALERT].from;
	to = Object_info[obj->id].anim[MC_STANDING].elem[AS_ALERT].to;
	spc = Object_info[obj->id].anim[MC_STANDING].elem[AS_ALERT].spc;

	if (spc <= 0.0) return;
	ASSERT(from <= to);

	if (obj->rtype.pobj_info.anim_frame < from || obj->rtype.pobj_info.anim_frame > to)
	{
		mprintf((0, "NonAI-Animation: Correcting for an incorrect frame number\n"));
		obj->rtype.pobj_info.anim_frame = from;
	}

	delta = to - from;

	ASSERT(delta >= 0.0f);

	if (delta > 0.0f)
	{
		// Frames per second will be on the animation page
		float step = (1.0f / (spc / (delta))) * anim_time;

		while (step > delta)
			step -= delta;

		ASSERT(step >= 0.0f);

		obj->rtype.pobj_info.anim_frame += step;

		if (obj->rtype.pobj_info.anim_frame >= to)
		{
			// We are guarenteed that this is between 'to' and 'from' from the above asserts()  :)
			obj->rtype.pobj_info.anim_frame -= delta;
		}
	}
	else
	{
		obj->rtype.pobj_info.anim_frame = from;
	}

	ASSERT(obj->rtype.pobj_info.anim_frame >= from && obj->rtype.pobj_info.anim_frame <= to);
}


void TimeoutObject(object* obj)
{
	switch (obj->type)
	{
	case OBJ_WEAPON:
		TimeoutWeapon(obj);
		break;
	case OBJ_SHOCKWAVE:
		DoConcussiveForce(obj, obj->parent_handle);
		break;
	default:
		break;
	}
}


// Does afterburn effects to a player ship
void DoPlayerAfterburnControl(game_controls* controls, object* objp)
{
	static int afterburner_id = -1;
	int slot = objp->id;

	if (controls->afterburn_thrust > 0)
	{
		Players[slot].last_afterburner_time = Gametime;

		if (Players[slot].afterburn_time_left > 0)
		{
			float punch_scalar = 1.0;

			if (Players[slot].afterburn_time_left == (AFTERBURN_TIME))
				AddToShakeMagnitude(objp->mtype.phys_info.mass / 2);

			if (Players[slot].afterburn_time_left > (AFTERBURN_TIME * .90))
				punch_scalar = 1.8f;
			else if (Players[slot].afterburn_time_left > (AFTERBURN_TIME * .80) && Players[slot].afterburn_time_left < (AFTERBURN_TIME * .90))
			{
				float norm = Players[slot].afterburn_time_left - (AFTERBURN_TIME * .80);
				norm /= (AFTERBURN_TIME * .1);
				punch_scalar = 1.0 + (norm * .8);
			}

			if (OBJECT_OUTSIDE(objp))
				controls->forward_thrust = controls->afterburn_thrust * 1.6 * punch_scalar;
			else
				controls->forward_thrust = controls->afterburn_thrust * 1.6 * punch_scalar;

			Players[slot].flags |= PLAYER_FLAGS_AFTERBURN_ON | PLAYER_FLAGS_THRUSTED;
			Players[slot].afterburn_time_left -= Frametime;
			if (Players[slot].afterburn_time_left < 0)
				Players[slot].afterburn_time_left = 0;
		}
		else
		{
			Players[slot].afterburn_time_left = 0;
			Players[slot].flags &= ~PLAYER_FLAGS_AFTERBURN_ON;
		}
	}
	else
	{
		Players[slot].flags &= ~PLAYER_FLAGS_AFTERBURN_ON;

		if (Players[slot].afterburn_time_left < AFTERBURN_TIME)
		{
			if (Players[slot].energy > 5.0)
			{
				float useage;
				if (afterburner_id == -1)
					afterburner_id = FindObjectIDName("Afterburner");

				if (afterburner_id != -1 && Players[slot].inventory.CheckItem(OBJ_POWERUP, afterburner_id))
					useage = Frametime * 2;
				else
					useage = Frametime;

				Players[slot].afterburn_time_left += useage;
				Players[slot].afterburn_time_left = min(AFTERBURN_TIME, Players[slot].afterburn_time_left);

				Players[slot].energy -= (useage);
			}
		}
	}
}


//Deal with the player's rear view
void CheckRearView(int down_count, bool down_state)
{
	player* player = &Players[Player_num];

#define LEAVE_TIME (1.0 / 16)		//How long until we decide key is down

	static bool leave_mode;
	static float entry_time;

	if (down_count)
	{
		//key/button has gone down

		if (player->flags & PLAYER_FLAGS_REARVIEW)
		{
			//rear view was active, so turn it off
			player->flags &= ~PLAYER_FLAGS_REARVIEW;
		}
		else
		{
			//rear view wasn't active, so turn it on
			player->flags |= PLAYER_FLAGS_REARVIEW;
			leave_mode = 0;				//means wait for another key
			entry_time = Gametime;
		}
	}
	else
	{
		//key didn't go down this frame; check if it used to be down

		if (down_state)
		{
			//key is still down
			if (!leave_mode && ((Gametime - entry_time) > LEAVE_TIME))
				leave_mode = 1;
		}
		else
		{
			if (leave_mode && (player->flags & PLAYER_FLAGS_REARVIEW))
				player->flags &= ~PLAYER_FLAGS_REARVIEW;
		}
	}
}


extern int Timedemo_frame;
float Timedemo_timecount;

//Do the controls for a player-type object
void DoFlyingControl(object* objp)
{
	game_controls controls;

	if (Dedicated_server)
		return;

	ASSERT(objp->control_type == CT_FLYING);

	if ((objp->type != OBJ_PLAYER && objp->type != OBJ_OBSERVER) || objp->id != Player_num)
		return;

	if (Timedemo_frame != -1)
	{
		matrix temp_mat, rot_mat;

		if (Timedemo_frame == 0)
			Timedemo_timecount = 0;
		else
			Timedemo_timecount += Frametime;

		vm_AnglesToMatrix(&temp_mat, 0, 65536 / 360, 0);

		rot_mat = objp->orient * temp_mat;

		ObjSetOrient(objp, &rot_mat);

		Timedemo_frame++;
		if (Timedemo_frame == 360)
		{
			// Calculate timedemo framerate

			float fps = 1.0 / (Timedemo_timecount / 360.0);
			AddHUDMessage(TXT_TIMEDEMO, fps);
			Timedemo_frame = -1;
		}

		return;
	}
	if (Demo_flags == DF_PLAYBACK)
	{
		//If we are playing back a demo, bail early and don't process controls
		return;
	}

	//Read the controllers	
	ReadPlayerControls(&controls);


	if ((objp->type == OBJ_PLAYER) && (Players[objp->id].controller_bitflags != 0xffffffff))
	{
		player* pp = &Players[objp->id];
		if (!(pp->controller_bitflags & PCBF_FORWARD))
		{
			if (controls.forward_thrust > 0.0f)
				controls.forward_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_REVERSE))
		{
			if (controls.forward_thrust < 0.0f)
				controls.forward_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_LEFT))
		{
			if (controls.sideways_thrust < 0.0f)
				controls.sideways_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_RIGHT))
		{
			if (controls.sideways_thrust > 0.0f)
				controls.sideways_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_UP))
		{
			if (controls.vertical_thrust > 0.0f)
				controls.vertical_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_DOWN))
		{
			if (controls.vertical_thrust < 0.0f)
				controls.vertical_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_PITCHUP))
		{
			if (controls.pitch_thrust > 0.0f)
				controls.pitch_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_PITCHDOWN))
		{
			if (controls.pitch_thrust < 0.0f)
				controls.pitch_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_HEADINGLEFT))
		{
			if (controls.heading_thrust < 0.0f)
				controls.heading_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_HEADINGRIGHT))
		{
			if (controls.heading_thrust > 0.0f)
				controls.heading_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_BANKLEFT))
		{
			if (controls.bank_thrust < 0.0f)
				controls.bank_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_BANKRIGHT))
		{
			if (controls.bank_thrust > 0.0f)
				controls.bank_thrust = 0.0f;
		}
		if (!(pp->controller_bitflags & PCBF_PRIMARY))
		{
			controls.fire_primary_down_count = 0;
			controls.fire_primary_down_time = 0;
			controls.fire_primary_down_state = false;
		}
		if (!(pp->controller_bitflags & PCBF_SECONDARY))
		{
			controls.fire_secondary_down_count = 0;
			controls.fire_secondary_down_time = 0;
			controls.fire_secondary_down_state = false;
		}
		if (!(pp->controller_bitflags & PCBF_AFTERBURNER))
		{
			controls.afterburn_thrust = 0.0f;
		}
	}

	// Update IntelliVIBE
	if (objp == Player_object)
		VIBE_DoControls(&controls);

	//Send an event to the Game DLLs so they can do any processing of game controls
	if (Game_mode & GM_MULTI)
	{
		DLLInfo.me_handle = DLLInfo.it_handle = objp->handle;
		DLLInfo.special_data = (ubyte*)&controls;
		CallGameDLL(EVT_GAMEDOCONTROLS, &DLLInfo);
	}

	//Deal with afterburner
	if (objp->type == OBJ_PLAYER)
	{
		ASSERT(Player_num == objp->id);

		// Reset thrusting
		Players[objp->id].flags &= ~PLAYER_FLAGS_THRUSTED;

		DoPlayerAfterburnControl(&controls, objp);
	}

	//Update object's physics
	if (objp->movement_type == MT_PHYSICS)
	{
		if (objp->type == OBJ_PLAYER && Players[objp->id].guided_obj != NULL)
		{
			ASSERT(objp->id == Player_num);
			object* g_obj = Players[objp->id].guided_obj;

			g_obj->mtype.phys_info.rotthrust.x = controls.pitch_thrust * g_obj->mtype.phys_info.full_rotthrust;
			g_obj->mtype.phys_info.rotthrust.z = controls.bank_thrust * g_obj->mtype.phys_info.full_rotthrust;
			g_obj->mtype.phys_info.rotthrust.y = controls.heading_thrust * g_obj->mtype.phys_info.full_rotthrust;
		}
		else
		{
			player* playp = &Players[objp->id];

			objp->mtype.phys_info.rotthrust.x = controls.pitch_thrust * objp->mtype.phys_info.full_rotthrust * playp->turn_scalar;
			objp->mtype.phys_info.rotthrust.z = controls.bank_thrust * objp->mtype.phys_info.full_rotthrust * playp->turn_scalar;
			objp->mtype.phys_info.rotthrust.y = controls.heading_thrust * objp->mtype.phys_info.full_rotthrust * playp->turn_scalar;
		}

		float speed_scalar = 1.0;

		// If thrusting foward, show glow
		if (objp->type == OBJ_PLAYER && controls.forward_thrust > 0)
			Players[objp->id].flags |= PLAYER_FLAGS_THRUSTED;

		if (objp->effect_info && objp->effect_info->type_flags & EF_FREEZE)
			speed_scalar = objp->effect_info->freeze_scalar;

		if (OBJECT_OUTSIDE(objp))
			speed_scalar *= 1.3f;

		objp->mtype.phys_info.thrust = speed_scalar * Players[objp->id].movement_scalar * ((objp->orient.fvec * controls.forward_thrust * objp->mtype.phys_info.full_thrust) +
			(objp->orient.uvec * controls.vertical_thrust * objp->mtype.phys_info.full_thrust) +
			(objp->orient.rvec * controls.sideways_thrust * objp->mtype.phys_info.full_thrust));
	}

	//Process player-specific stuff
	if (objp->type == OBJ_PLAYER)
	{
		//Process weapon firing
		FireWeaponFromPlayer(objp, PW_PRIMARY, controls.fire_primary_down_count, controls.fire_primary_down_state, controls.fire_primary_down_time);
		FireWeaponFromPlayer(objp, PW_SECONDARY, controls.fire_secondary_down_count, controls.fire_secondary_down_state, controls.fire_secondary_down_time);

		//Deal with the flare
		for (int i = 0; i < controls.fire_flare_down_count; i++)
			FireFlareFromPlayer(objp);

		//Do rear view
		CheckRearView(controls.rearview_down_count, controls.rearview_down_state);
	}
}

//Do any special effects processing for this object
void ObjDoEffects(object* obj)
{
	if (!obj->effect_info)
		return;

	static int napalm_id = -1;

	if (napalm_id == -1)
		napalm_id = FindWeaponName("Napalm");

	if (obj->effect_info->type_flags & EF_VOLUME_CHANGING)
	{
		obj->effect_info->volume_change_time -= Frametime;
		if (obj->effect_info->volume_change_time <= 0.0f)
			obj->effect_info->type_flags &= ~EF_VOLUME_CHANGING;
	}

	if (obj->effect_info->type_flags & EF_FADING_IN)
	{
		obj->effect_info->fade_time -= Frametime;
		if (obj->effect_info->fade_time <= 0.0f)
			obj->effect_info->type_flags &= ~EF_FADING_IN;
	}


	if (obj->effect_info->type_flags & EF_CLOAKED)
	{
		obj->effect_info->cloak_time -= Frametime;
		if (obj->effect_info->cloak_time <= 0.0f)
		{
			//	reappear the object
			MakeObjectVisible(obj);
		}
	}

	if (obj->effect_info->type_flags & EF_FADING_OUT)
	{
		obj->effect_info->fade_time -= Frametime;
		if (obj->effect_info->fade_time <= 0.0f)
		{
			obj->effect_info->type_flags &= ~EF_FADING_OUT;
			obj->effect_info->type_flags |= EF_CLOAKED;
		}
	}

	if (obj->effect_info->type_flags & EF_LIQUID)
	{
		obj->effect_info->liquid_time_left -= Frametime;
		if (obj->effect_info->liquid_time_left <= 0.0f)
		{
			// Stop doing liquid effect
			obj->effect_info->type_flags &= ~EF_LIQUID;
			if (obj == Viewer_object)
				Render_FOV = Render_FOV_desired;
		}
		else
		{
			if (obj == Viewer_object)
			{
				int inttime = Gametime;
				float normtime = Gametime - inttime;

				float scalar = FixSin(normtime * 65535 * 4);

				scalar *= ((float)obj->effect_info->liquid_mag);
				if (obj->effect_info->liquid_time_left < 1)
					scalar *= (obj->effect_info->liquid_time_left);

				Render_FOV = Render_FOV_desired + scalar;
			}
		}
	}


	if (obj->effect_info->type_flags & EF_DEFORM)
	{
		obj->effect_info->deform_time -= Frametime;
		if (obj->effect_info->deform_time <= 0.0f)
		{
			// Stop deforming
			obj->effect_info->type_flags &= ~EF_DEFORM;
		}
	}

	if (obj->effect_info->type_flags & EF_FREEZE)
	{
		obj->effect_info->freeze_scalar += (Frametime / 3);
		if (obj->effect_info->freeze_scalar >= 1.0f)
		{
			// Stop freezing
			obj->effect_info->type_flags &= ~EF_FREEZE;
		}
	}

	if (obj->effect_info->type_flags & EF_NAPALMED)
	{
		if ((Gametime - obj->effect_info->last_damage_time) > 1.0)
		{
			obj->effect_info->last_damage_time = Gametime;

			// Apply damage to this napalmed object
			object* killer;

			if (obj->effect_info->damage_handle != OBJECT_HANDLE_NONE)
			{
				uint sig = obj->effect_info->damage_handle & HANDLE_COUNT_MASK;
				int objnum = obj->effect_info->damage_handle & HANDLE_OBJNUM_MASK;

				if ((Objects[objnum].handle & HANDLE_COUNT_MASK) != sig)
					killer = NULL;
				else
					killer = &Objects[objnum];
			}
			else
				killer = NULL;

			if (obj->type == OBJ_PLAYER)
			{
				ApplyDamageToPlayer(obj, killer, PD_NONE, obj->effect_info->damage_per_second);
			}
			else if (IS_GENERIC(obj->type) || (obj->type == OBJ_DOOR))
			{
				ApplyDamageToGeneric(obj, killer, GD_FIRE, obj->effect_info->damage_per_second);
			}

		}

		obj->effect_info->damage_time -= Frametime;
		if (obj->effect_info->damage_time <= 0)
		{
			obj->effect_info->type_flags &= ~EF_NAPALMED;
			obj->effect_info->last_damage_time = 0;
			Sound_system.StopSoundLooping(obj->effect_info->sound_handle);
			obj->effect_info->sound_handle = SOUND_NONE_INDEX;
		}

		// Drop some smoke/fire and stuff
		AttachRandomNapalmEffectsToObject(obj);
	}

	//Do sparking stuff
	if (obj->effect_info->type_flags & EF_SPARKING)
	{
		int num_bolts = 0;

		//Create spark effects
		obj->effect_info->spark_timer -= Frametime;
		while (obj->effect_info->spark_timer < 0.0)
		{
			obj->effect_info->spark_timer += obj->effect_info->spark_delay;
			num_bolts++;
		}
		CreateElectricalBolts(obj, num_bolts);

		//Update timer
		obj->effect_info->spark_time_left -= Frametime;
		if (obj->effect_info->spark_time_left <= 0.0f)
			obj->effect_info->type_flags &= ~EF_SPARKING;
	}
}


//Check passed-through faces for triggers, etc.
void ObjCheckTriggers(object* objp)
{
	for (int i = 0; i < Fvi_num_recorded_faces; i++)
	{
		int roomnum = Fvi_recorded_faces[i].room_index;
		int facenum = Fvi_recorded_faces[i].face_index;

		CheckTrigger(roomnum, facenum, objp, TT_PASS_THROUGH);
	}
}


#ifdef _DEBUG
void debug_check_terrain_objects()
{
	for (int i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
	{
		int j = Terrain_seg[i].objects;
		while (j != -1)
		{
			ASSERT(j >= 0 && j <= Highest_object_index);
			j = Objects[j].next;
		}
	}
}
#endif


#ifdef _DEBUG
extern int DoAI;
#else
#define DoAI 1
#endif
//--------------------------------------------------------------------
//Process an object for the current frame
void ObjDoFrame(object* obj)
{
	int	previous_room = obj->roomnum;
	float save_frametime = Frametime;

	ASSERT(obj->type != OBJ_NONE);

	// Clear attached vis for this frame
	obj->lowest_attached_vis = -1;

	// If this is a permissable netgame, accelerate this object if need by
	if ((Game_mode & GM_MULTI) && Netgame.local_role == LR_CLIENT && (obj->flags & OF_PING_ACCELERATE))
	{
		Frametime += (NetPlayers[Player_num].ping_time / 2.0);
		obj->flags &= ~OF_PING_ACCELERATE;
	}
	//Update previous position before moving
	obj->last_pos = obj->pos;

	//Inevitable countdown towards death
	if (obj->flags & OF_USES_LIFELEFT)
		obj->lifeleft -= Frametime;

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

		//Process the control for this object
	switch (obj->control_type)
	{
	case CT_POWERUP:
	case CT_NONE:
		break;
	case CT_FLYING: { RTP_STARTINCTIME(ct_flying_time);			DoFlyingControl(obj);	RTP_ENDINCTIME(ct_flying_time); }break;
	case CT_AI:				if (DoAI) { RTP_STARTINCTIME(ct_aidoframe_time); AIDoFrame(obj);		RTP_ENDINCTIME(ct_aidoframe_time); }break;
	case CT_WEAPON: { RTP_STARTINCTIME(ct_weaponframe_time);	WeaponDoFrame(obj);		RTP_ENDINCTIME(ct_weaponframe_time); }break;
	case CT_EXPLOSION: { RTP_STARTINCTIME(ct_explosionframe_time); DoExplosionFrame(obj);	RTP_ENDINCTIME(ct_explosionframe_time); }break;
	case CT_DEBRIS: { RTP_STARTINCTIME(ct_debrisframe_time);	DoDebrisFrame(obj);		RTP_ENDINCTIME(ct_debrisframe_time); }break;
	case CT_SPLINTER: { RTP_STARTINCTIME(ct_splinterframe_time);	DoSplinterFrame(obj);	RTP_ENDINCTIME(ct_splinterframe_time); }break;
	case CT_DYING:			DoDyingFrame(obj); break;
	case CT_DYING_AND_AI:
		if (DoAI)
			AIDoFrame(obj);
		DoDyingFrame(obj);
		break;
	case CT_SOUNDSOURCE:	break;	//do nothing
	case CT_SOAR:		break;

#ifdef _DEBUG
	case CT_SLEW:			SlewFrame(obj); break;
#endif

	default:
		Error("Unknown control type %d in object %i, handle/type/id = %i/%i/%i", obj->control_type, obj - Objects, obj->handle, obj->type, obj->id);
		break;
	}

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

		//Update cycled animation
	if ((obj->control_type != CT_AI) && (obj->control_type != CT_DYING_AND_AI) && (obj->control_type != CT_DEBRIS) && IS_GENERIC(obj->type) && (obj->type != OBJ_ROOM))
	{
		if (Object_info[obj->id].anim && Object_info[obj->id].anim[MC_STANDING].elem[AS_ALERT].to)
		{
			RTP_STARTINCTIME(cycle_anim);
			DoCycledAnim(obj);
			RTP_ENDINCTIME(cycle_anim);
		}
	}

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

		// Do freeze stuff on AI controlled robots
	if ((obj->type == OBJ_ROBOT || obj->type == OBJ_BUILDING) && obj->control_type == CT_AI && obj->effect_info && (obj->effect_info->type_flags & EF_FREEZE))
		obj->mtype.phys_info.velocity *= obj->effect_info->freeze_scalar;

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

		//Check for object dead of old age
	if ((obj->flags & OF_USES_LIFELEFT) && (obj->lifeleft < 0))
	{
		bool kill_it = true;
		bool tell_clients = false;

		ASSERT(obj->type != OBJ_PLAYER);

		if (Game_mode & GM_MULTI)
		{
			if (Netgame.local_role == LR_CLIENT)
			{
				if (obj->type == OBJ_POWERUP || obj->type == OBJ_ROBOT || obj->type == OBJ_CLUTTER || obj->type == OBJ_BUILDING)
				{
					ASSERT(obj->flags & OF_SERVER_OBJECT);

					if (!(obj->flags & OF_SERVER_SAYS_DELETE))
					{
						kill_it = false;
						obj->lifeleft = .001f;
					}
				}
			}
			else
			{
				if (obj->type == OBJ_POWERUP || obj->type == OBJ_ROBOT || obj->type == OBJ_CLUTTER || obj->type == OBJ_BUILDING)
					tell_clients = true;
			}
		}

		if (kill_it)
		{
			TimeoutObject(obj);
			SetObjectDeadFlag(obj, tell_clients);
		}
	}

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG


		//If object is dead, don't do any more processing on it
	if (obj->flags & OF_DEAD)
	{
		Frametime = save_frametime;
		return;
	}

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

		//Do the movement for this object
	switch (obj->movement_type)
	{
	case MT_NONE:
		if ((obj->flags & OF_STUCK_ON_PORTAL) && !(Rooms[obj->mtype.phys_info.stuck_room].portals[obj->mtype.phys_info.stuck_portal].flags & PF_RENDER_FACES))
		{
			if (IS_GENERIC(obj->type) && (obj->flags & OF_CLIENT_KNOWS))
				SetObjectDeadFlag(obj, true);
			else
				SetObjectDeadFlag(obj, false);

		}
		break;								//this doesn't move

	case MT_PHYSICS:
	{
		RTP_STARTINCTIME(mt_physicsframe_time);

		do_physics_sim(obj);
		DebugBlockPrint("DP");
		ObjCheckTriggers(obj);

		RTP_ENDINCTIME(mt_physicsframe_time);
	}break;

	case MT_WALKING:
	{
		RTP_STARTINCTIME(mt_walkingframe_time);
		do_walking_sim(obj);
		DebugBlockPrint("DW");
		ObjCheckTriggers(obj);
		RTP_ENDINCTIME(mt_walkingframe_time);
	}break;

	case MT_SHOCKWAVE:
	{
		RTP_STARTINCTIME(mt_shockwave_time);
		DoConcussiveForce(obj, obj->parent_handle);
		RTP_ENDINCTIME(mt_shockwave_time);
	}break;

	case MT_OBJ_LINKED:
		PhysicsLinkList[Physics_NumLinked++] = OBJNUM(obj);
		break;

	default:
		Int3();		//unknown movement type
	}

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

		// Do special effects stuff to object
	if (obj->effect_info)
	{
		RTP_STARTINCTIME(obj_doeffect_time);
		ObjDoEffects(obj);
		RTP_ENDINCTIME(obj_doeffect_time);
	}

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

		//Deal with special player movement stuff
	if (obj->type == OBJ_PLAYER)
	{
		RTP_STARTINCTIME(obj_move_player_time);
		DoSpecialPlayerStuff(obj);
		RTP_ENDINCTIME(obj_move_player_time);
	}

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

	//	Handle interval event for script.
	do
	{
		RTP_STARTINCTIME(obj_d3xint_time);
		//@$-D3XExecScript(obj, EVT_INTERVAL, obj);
		tOSIRISEventInfo ei;
		ei.evt_interval.frame_time = Frametime;
		ei.evt_interval.game_time = Gametime;
		Osiris_CallEvent(obj, EVT_INTERVAL, &ei);
		RTP_ENDINCTIME(obj_d3xint_time);
	} while (0);	//this do{}while(0) is here because the RTP_STARTINCTIME/RTP_ENDINCTIME must be in the same scope
	//and not share scope with another RTP_STARTINCTIME

//#ifdef _DEBUG
//	if(Physics_player_verbose)
//	{ 
//		debug_check_terrain_objects();
//	}
//#endif _DEBUG

	// Cast light
	do
	{
		RTP_STARTINCTIME(obj_objlight_time);
		DoObjectLight(obj);
		RTP_ENDINCTIME(obj_objlight_time);
	} while (0);	//this do{}while(0) is here because the RTP_STARTINCTIME/RTP_ENDINCTIME must be in the same scope
	//and not share scope with another RTP_STARTINCTIME

//#ifdef _DEBUG
//	if(Physics_player_verbose)
//	{ 
//		debug_check_terrain_objects();
//	}
//#endif _DEBUG

	// if this object is walking/rolling on the terrain, make it not LOD at that point
	if (obj->movement_type == MT_WALKING && (obj->flags & OF_RENDERED) && OBJECT_OUTSIDE(obj))
		TurnOffLODForCell(CELLNUM(obj->roomnum));

	//#ifdef _DEBUG
	//	if(Physics_player_verbose)
	//	{ 
	//		debug_check_terrain_objects();
	//	}
	//#endif _DEBUG

		//Mark object as not rendered for this frame
	obj->flags &= ~OF_RENDERED;

	Frametime = save_frametime;
}

int	Max_used_objects = MAX_OBJECTS - 20;
float Last_position_history_update[MAX_POSITION_HISTORY];//gametime of the last position history update of the object
float Last_position_history_update_time = 0.0f;

//--------------------------------------------------------------------
//Process all objects for the current frame
#define OBJ_POS_SAMPLE_TIME			(1.0f/( ((float)MAX_POSITION_HISTORY)*20.0f) )//super sample at 20 fps
void ObjDoFrameAll()
{
	int i;
	object* objp;
	int objs_live = 0;
	bool update_position_history = false;

	if (Last_position_history_update_time > Gametime)
	{
		// the Gametime rolled over (new level)		
		update_position_history = true;
	}

	if (Gametime >= Last_position_history_update_time + OBJ_POS_SAMPLE_TIME)
	{
		update_position_history = true;
	}

	if (update_position_history)
	{
		Object_position_head--;
		if (Object_position_head == 255)
			Object_position_head = MAX_POSITION_HISTORY - 1;

		Last_position_history_update[Object_position_head] = Gametime;
		Last_position_history_update_time = Gametime;
	}

	Physics_NumLinked = 0;

	//Process each object
	for (i = 0, objp = Objects; i <= Highest_object_index; i++, objp++)
	{
		if ((objp->type != OBJ_NONE) && (!(objp->flags & OF_DEAD)))
		{
			RTP_STARTINCTIME(obj_do_frm);
			ObjDoFrame(objp);

			if (update_position_history)
			{
				int slot = Object_map_position_history[OBJNUM(objp)];
				if (slot != -1)
					Object_position_samples[slot].pos[Object_position_head] = objp->pos;
			}

			RTP_ENDINCTIME(obj_do_frm);
			objs_live++;
		}
	}

	// Account for linked objects
	for (i = 0; i < Physics_NumLinked; i++)
	{
		RTP_STARTINCTIME(phys_link);
		DoPhysLinkedFrame(&Objects[PhysicsLinkList[i]]);
		RTP_ENDINCTIME(phys_link);
	}

	// Move vis effects
	{
		RTP_STARTINCTIME(vis_eff_move);
		VisEffectMoveAll();
		RTP_ENDINCTIME(vis_eff_move);
	}

	// Blend all lights that are needed
	BlendAllLightingEdges();

	mprintf_at((1, 5, 40, "Objs=%d ", objs_live));

	//Delete everything that died
	ObjDeleteDead();
}

#define FUELCEN_SOUND_DELAY	0.25		//play every quarter second
#define FUELCEN_GIVE_RATE		25			//give 25 units per second

//Give the player fuel.  Called when the player is in an energy center
void RefuelPlayer(object* objp)
{
	float max, amount;

	ASSERT(objp->type == OBJ_PLAYER);
	ASSERT(Rooms[objp->roomnum].flags & RF_FUELCEN);

	max = INITIAL_ENERGY - Players[objp->id].energy;
	amount = FUELCEN_GIVE_RATE * Frametime;

	if (amount > max)
		amount = max;

	if ((amount > 0))
	{
		Players[objp->id].energy += amount;

		Sound_system.Play3dSound(SOUND_REFUELING, SND_PRIORITY_HIGH, objp);

		ain_hear hear;
		hear.f_directly_player = true;
		hear.hostile_level = 0.01f;
		hear.curiosity_level = 1.0f;
		hear.max_dist = Sounds[SOUND_REFUELING].max_distance;
		AINotify(objp, AIN_HEAR_NOISE, (void*)&hear);

		//if (Game_mode & GM_MULTI)
		//	multi_send_play_sound(SOUND_REFUEL_STATION_GIVING_FUEL, F1_0/2);
	}
}


//Clears the secret flag in the specified room, and recursively, all connected rooms
void ClearSecretFlags(room* rp)
{
	int p;
	portal* pp;

	rp->flags &= ~RF_SECRET;

	for (p = 0, pp = rp->portals; p < rp->num_portals; p++, pp++)
	{
		room* rp2 = &Rooms[pp->croom];
		if (rp2->flags & RF_SECRET)
			ClearSecretFlags(rp2);
	}
}


// Prints out a message when you've found a secret
#define SECRET_COLOR	GR_RGB(0,128,255)
void DoSecretForPlayer(room* rp, object* objp)
{
	int y = Game_window_h / 4;
	AddPersistentHUDMessage(SECRET_COLOR, HUD_MSG_PERSISTENT_CENTER, y, 5, HPF_FADEOUT + HPF_FREESPACE_DRAW, SOUND_GAME_MESSAGE, TXT_FOUND_SECRET);

	ClearSecretFlags(rp);
}

//	--------------------------------------------------------------------

//Do refueling centers & damage rooms
void DoSpecialPlayerStuff(object* objp)
{
	if (!OBJECT_OUTSIDE(objp))
	{
		room* rp = &Rooms[objp->roomnum];

		//Check for refueling
		if (rp->flags & RF_FUELCEN)
			RefuelPlayer(objp);

		if (objp->id == Player_num && (rp->flags & RF_SECRET))
			DoSecretForPlayer(rp, objp);

		//Check for damage room
		if (rp->damage > 0.0)
		{
			if (!(objp->effect_info->type_flags & EF_NAPALMED))
			{
				//Set the player on fire
				ASSERT(objp->effect_info);
				objp->effect_info->type_flags |= EF_NAPALMED;
				objp->effect_info->damage_time = 1.0;
				objp->effect_info->damage_per_second = rp->damage;
				if (Gametime - objp->effect_info->last_damage_time > 1.0f)
					objp->effect_info->last_damage_time = 0;
				objp->effect_info->damage_handle = OBJECT_HANDLE_NONE;
				if (objp->effect_info->sound_handle == SOUND_NONE_INDEX)
					objp->effect_info->sound_handle = Sound_system.Play3dSound(SOUND_PLAYER_BURNING, SND_PRIORITY_HIGHEST, objp);
			}
		}

		//Check for & set waypoint
		if (rp->flags & RF_WAYPOINT)
			SetAutoWaypoint(objp);
	}
	else 
	{
		//outside

		//Check for & set waypoint
		//if (Terrain_seg[CELLNUM(objp->roomnum)].flags & TF_WAYPOINT)
		//	SetAutoWaypoint(objp);
	}
}


//Builds the free object list by scanning the list of free objects & adding unused ones to the list
//Also sets Highest_object_index
void ResetFreeObjects()
{
	int i;

	Highest_object_index = -1;

	for (i = Num_objects = MAX_OBJECTS; --i >= 0;)
		if (Objects[i].type == OBJ_NONE)
			free_obj_list[--Num_objects] = i;
		else if (Highest_object_index == -1)
			Highest_object_index = i;
}


//sets the orientation of an object.  This should be called to orient an object
void ObjSetOrient(object* obj, const matrix* orient)
{
	// Accounts for if the orientation was set and then this function is being used
	// to update the other stuff
	if (&obj->orient != orient)
		obj->orient = *orient;

	// Recompute the orientation dependant information
	if (obj->flags & OF_POLYGON_OBJECT)
	{
		if (obj->type != OBJ_WEAPON &&
			obj->type != OBJ_DEBRIS &&
			obj->type != OBJ_POWERUP &&
			obj->type != OBJ_ROOM)
		{
			matrix m;

			m = obj->orient;
			vm_TransposeMatrix(&m);

			obj->wall_sphere_offset = Poly_models[obj->rtype.pobj_info.model_num].wall_size_offset * m;
			obj->anim_sphere_offset = Poly_models[obj->rtype.pobj_info.model_num].anim_size_offset * m;
		}
		else
		{
			obj->wall_sphere_offset = Zero_vector;
			obj->anim_sphere_offset = Zero_vector;
		}
	}
}


//sets the position of an object.  This should be called to move an object
void ObjSetPos(object* obj, vector* pos, int roomnum, matrix* orient, bool f_update_attached_children)
{
	int oldroomnum = obj->roomnum;
	vector old_pos = obj->pos;

	//Reset the position & recalculate the AABB
	obj->pos = *pos;
	ObjSetAABB(obj);

	//Reset the orientation if changed
	if (orient != NULL)
		ObjSetOrient(obj, orient);

	//Clear the outside-mine flag
	obj->flags &= ~OF_OUTSIDE_MINE;

	//If changed rooms, do a bunch of stuff
	if (obj->roomnum != roomnum)
	{
		//Let the script know
		tOSIRISEventInfo ei;
		ei.evt_changeseg.room_num = roomnum;
		Osiris_CallEvent(obj, EVT_CHANGESEG, &ei);

		if (obj->type == OBJ_PLAYER && !ROOMNUM_OUTSIDE(roomnum) && (Rooms[roomnum].flags & RF_INFORM_RELINK_TO_LG))
		{
			Level_goals.Inform(LIT_INTERNAL_ROOM, LGF_COMP_ENTER, roomnum);
		}

		//Relink the object
		ObjRelink(OBJNUM(obj), roomnum);

		// Call DLL function if the server player changed rooms
		if ((Game_mode & GM_MULTI) && (Netgame.local_role == LR_SERVER))
		{
			DLLInfo.me_handle = obj->handle;
			DLLInfo.it_handle = obj->handle;
			DLLInfo.oldseg = oldroomnum;
			DLLInfo.newseg = obj->roomnum;

			if (obj->type == OBJ_PLAYER)
				CallGameDLL(EVT_GAMEPLAYERCHANGESEG, &DLLInfo);
			else
				CallGameDLL(EVT_GAMEOBJCHANGESEG, &DLLInfo);
		}

		// Slowly change volume lighting if going between rooms, if not in the editor
		if (GetFunctionMode() != EDITOR_MODE)
		{
			if ((obj->effect_info != NULL) && (obj->effect_info->type_flags & EF_VOLUME_LIT))
			{
				if (!ROOMNUM_OUTSIDE(oldroomnum) && !ROOMNUM_OUTSIDE(roomnum))
				{
					if (!(obj->effect_info->type_flags & EF_VOLUME_CHANGING))
					{
						obj->effect_info->type_flags |= EF_VOLUME_CHANGING;
						obj->effect_info->volume_change_time = 1.0;
						obj->effect_info->volume_old_room = oldroomnum;
						obj->effect_info->volume_old_pos = old_pos;
					}
				}
				else //either old or new room was outside, so don't do volume changing
					obj->effect_info->type_flags &= ~EF_VOLUME_CHANGING;
			}
		}
	}
}


//Delete objects, such as weapons & explosions, that shouldn't stay between levels
//if clear_all is set, clear even proximity bombs
void ClearTransientObjects(int clear_all)
{
	int objnum;
	object* objp;

	for (objnum = 0, objp = Objects; objnum <= Highest_object_index; objnum++, objp++)
	{
		if (((objp->type == OBJ_WEAPON) && !(Weapons[objp->id].flags & WF_COUNTERMEASURE)) ||
			(objp->type == OBJ_FIREBALL) ||
			(objp->type == OBJ_DEBRIS) ||
			(objp->type == OBJ_SHARD) ||
			(objp->type == OBJ_SHOCKWAVE) ||
			(objp->type == OBJ_PARTICLE) ||
			(objp->type == OBJ_SPLINTER) ||
			((objp->type != OBJ_NONE) && (objp->flags & OF_DYING)))
		{
			mprintf((0, "Clearing object %d type = %d\n", objnum, objp->type));
			ObjDelete(objnum);
		}
	}
}


// Remaps all static powerups,sounds,etc to their appropriate indices
void RemapEverything()
{
	RemapStaticIDs();
	RemapDoors();
	RemapShips();
	RemapWeapons();
	RemapSounds();
	RemapPolyModels();
}


//Retruns a pointer to an object given its handle.  Returns NULL if object no longer exists.
object* ObjGet(int handle)
{
	if (handle == OBJECT_HANDLE_NONE)
		return NULL;

	if (handle == OBJECT_HANDLE_BAD)
	{
		Int3();
		return NULL;
	}

	int objnum = handle & HANDLE_OBJNUM_MASK;

	ASSERT((handle & HANDLE_COUNT_MASK) != 0);		//count == 0 means never-used object
	ASSERT(objnum < MAX_OBJECTS);

	object* objp = &Objects[objnum];

	if ((objp->type != OBJ_NONE) && (objp->handle == handle))
		return objp;
	else
		return NULL;
}


void GetObjectPointInWorld(vector* dest, object* obj, int subnum, int vertnum)
{
	poly_model* pm = &Poly_models[obj->rtype.pobj_info.model_num];
	bsp_info* sm = &pm->submodel[subnum];
	float normalized_time[MAX_SUBOBJECTS];

	if (!pm->new_style)
		return;

	for (int i = 0; i < MAX_SUBOBJECTS; i++)
		normalized_time[i] = 0.0;

	SetModelAnglesAndPos(pm, normalized_time);

	vector pnt = sm->verts[vertnum];
	int mn = subnum;
	matrix m;

	// Instance up the tree for this gun
	while (mn != -1)
	{
		vector tpnt;

		vm_AnglesToMatrix(&m, pm->submodel[mn].angs.p, pm->submodel[mn].angs.h, pm->submodel[mn].angs.b);
		vm_TransposeMatrix(&m);

		tpnt = pnt * m;

		pnt = tpnt + pm->submodel[mn].offset + pm->submodel[mn].mod_pos;

		mn = pm->submodel[mn].parent;
	}

	//now instance for the entire object
	m = obj->orient;
	vm_TransposeMatrix(&m);

	*dest = pnt * m;
	*dest += obj->pos;
}


bool ObjGetAnimUpdate(unsigned short objnum, custom_anim* multi_anim_info)
{
	object* obj = &Objects[objnum];

	if ((objnum >= 0) && (obj->type != OBJ_NONE) && (obj->type != OBJ_WEAPON) &&
		(obj->flags & OF_POLYGON_OBJECT))
	{
		polyobj_info* pm = &obj->rtype.pobj_info;
		ai_frame* ai_info = obj->ai_info;

		multi_anim_info->server_time = Gametime;
		multi_anim_info->server_anim_frame = (ushort)(pm->anim_frame * 256.0f);

		multi_anim_info->anim_start_frame = pm->anim_start_frame;
		multi_anim_info->anim_end_frame = pm->anim_end_frame;
		multi_anim_info->anim_time = pm->anim_time;
		multi_anim_info->max_speed = pm->max_speed;

		multi_anim_info->flags = 0;

		if (pm->anim_flags & AIAF_LOOPING)
			multi_anim_info->flags |= FMA_LOOPING;

		if (obj->ai_info != NULL)
		{
			multi_anim_info->anim_sound_index = obj->ai_info->last_played_sound_index;
			multi_anim_info->flags |= FMA_HAS_AI;
		}
		else
		{
			multi_anim_info->anim_sound_index = -1;
		}

		return true;
	}

	return false;
}

void SetObjectControlType(object* obj, int control_type)
{
	ASSERT(obj);
	ASSERT(OBJNUM(obj) >= 0 && OBJNUM(obj) < MAX_OBJECTS);

	if ((control_type == CT_AI) && (obj->ai_info == NULL))
	{
		poly_model* pm = &Poly_models[obj->rtype.pobj_info.model_num];
		polyobj_info* p_info = &obj->rtype.pobj_info;
		int num_wbs = pm->num_wbs;
		int count = 0;
		
		obj->ai_info = (ai_frame*)mem_malloc(sizeof(ai_frame));
		memset(obj->ai_info, 0x00, sizeof(ai_frame));	//DAJ clear the baby

		for (int i = 0; i < num_wbs; i++)
		{
			ASSERT(pm->poly_wb[i].num_turrets >= 0 && pm->poly_wb[i].num_turrets <= 6400);
			count += pm->poly_wb[i].num_turrets;
		}

		p_info->multi_turret_info.num_turrets = count;

		if ((count > 0) && (p_info->multi_turret_info.keyframes == NULL))
		{
			int cur = 0;

			p_info->multi_turret_info.time = 0;
			p_info->multi_turret_info.keyframes = (float*)mem_malloc(sizeof(float) * count);
			p_info->multi_turret_info.last_keyframes = (float*)mem_malloc(sizeof(float) * count);
			p_info->multi_turret_info.flags = 0;
		}
	}

	obj->control_type = control_type;

	if (obj->control_type == CT_AI || obj->type == OBJ_PLAYER)
	{
		poly_model* pm = &Poly_models[obj->rtype.pobj_info.model_num];
		int num_wbs = pm->num_wbs;

		if (obj->dynamic_wb == NULL)
		{
			if (obj->type == OBJ_PLAYER)
			{
				obj->dynamic_wb = (dynamic_wb_info*)mem_malloc(sizeof(dynamic_wb_info) * MAX_WBS_PER_OBJ);
			}
			else
			{
				if (num_wbs)
					obj->dynamic_wb = (dynamic_wb_info*)mem_malloc(sizeof(dynamic_wb_info) * num_wbs);
			}
		}
	}
}


void ObjSetAnimUpdate(unsigned short objnum, custom_anim* multi_anim_info)
{
	object* obj = &Objects[objnum];
	polyobj_info* pm;

	if ((objnum >= 0) && (obj->type != OBJ_NONE) && (obj->type != OBJ_WEAPON) &&
		(obj->flags & OF_POLYGON_OBJECT))
	{
		pm = &obj->rtype.pobj_info;
		pm->multi_anim_info = *multi_anim_info;

		// Make sure we mark it as current and 
		pm->multi_anim_info.flags |= FMA_VALID;
		pm->multi_anim_info.flags &= ~FMA_CURRENT;
	}
}


void ObjGetTurretUpdate(unsigned short objnum, multi_turret* multi_turret_info)
{
	object* obj = &Objects[objnum];
	poly_model* pm = &Poly_models[obj->rtype.pobj_info.model_num];
	polyobj_info* p_info = &obj->rtype.pobj_info;

	if ((objnum >= 0) && (obj->type != OBJ_NONE) && (obj->type != OBJ_WEAPON) &&
		(obj->flags & OF_POLYGON_OBJECT) && (p_info->multi_turret_info.num_turrets))
	{
		int count = 0;

		multi_turret_info->time = Gametime;
		multi_turret_info->num_turrets = p_info->multi_turret_info.num_turrets;

		for (int i = 0; i < pm->num_wbs; i++)
		{
			for (int j = 0; j < pm->poly_wb[i].num_turrets; j++)
			{
				multi_turret_info->keyframes[count++] = obj->dynamic_wb[i].norm_turret_angle[j];
			}
		}
	}
}


void ObjSetTurretUpdate(unsigned short objnum, multi_turret* multi_turret_info)
{
	object* obj = &Objects[objnum];
	poly_model* pm = &Poly_models[obj->rtype.pobj_info.model_num];
	polyobj_info* p_info = &obj->rtype.pobj_info;

	if ((objnum >= 0) && (obj->type != OBJ_NONE) && (obj->type != OBJ_WEAPON) &&
		(obj->flags & OF_POLYGON_OBJECT) && (p_info->multi_turret_info.num_turrets))
	{
		p_info->multi_turret_info.time = multi_turret_info->time;

		for (int i = 0; i < p_info->multi_turret_info.num_turrets; i++)
			p_info->multi_turret_info.keyframes[i] = multi_turret_info->keyframes[i];

		p_info->multi_turret_info.flags = FMT_NEW_DATA;
	}
	else
	{
		mprintf((0, "Woops, no turret here to update!\n"));
	}

}


//Returns the original parent for the given object.  Returns self if it has no parent
object* ObjGetUltimateParent(object* child)
{
	ASSERT(child);

	object* ret = child;
	int handle = child->parent_handle;

	while (ObjGet(handle))
	{
		ret = ObjGet(handle);
		handle = ret->parent_handle;
	}
	return ret;
}

//Sets an object to a type OBJ_DUMMY (saves it's old type) so it won't be renderered, etc, but still alive
void ObjGhostObject(int objnum)
{
	ASSERT(objnum >= 0 && objnum < MAX_OBJECTS);
	if (objnum < 0 || objnum >= MAX_OBJECTS)
		return;

	object* obj = &Objects[objnum];

	ASSERT(obj->type != OBJ_NONE);
	ASSERT(obj->type != OBJ_DUMMY);	//Don't ghost a ghosted object!!
	ASSERT(obj->type != OBJ_PLAYER);
	ASSERT(obj->type != OBJ_GHOST);

	if (obj->type == OBJ_NONE || obj->type == OBJ_DUMMY ||
		obj->type == OBJ_PLAYER || obj->type == OBJ_GHOST)
		return;

	obj->dummy_type = obj->type;
	obj->type = OBJ_DUMMY;
}

//Restores a ghosted object back to it's old type
void ObjUnGhostObject(int objnum)
{
	ASSERT(objnum >= 0 && objnum < MAX_OBJECTS);
	if (objnum < 0 || objnum >= MAX_OBJECTS)
		return;

	object* obj = &Objects[objnum];

	ASSERT(obj->type == OBJ_DUMMY);

	if (obj->type != OBJ_DUMMY)
		return;

	//@@ASSERT (!(obj->flags&OF_INPLAYERINVENTORY));
	//@@if(obj->flags&OF_INPLAYERINVENTORY)
	//@@	return;	
	if (obj->flags & OF_INPLAYERINVENTORY)
	{
		mprintf((0, "UnGhosting Object in that is currently in a player's inventory!\n"));
	}

	obj->type = obj->dummy_type;
	obj->dummy_type = OBJ_NONE;
}
