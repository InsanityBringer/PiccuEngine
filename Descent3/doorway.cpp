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

#include "doorway.h"
#include "door.h"
#include "pserror.h"
#include "mem.h"
#include "room.h"
#include "polymodel.h"
#include "game.h"
#include "hlsoundlib.h"
#include "stringtable.h"
#include "player.h"
#include "osiris_dll.h"

//	---------------------------------------------------------------------------
//	Globals
int Num_active_doorways;				// number of active doors in game
int Active_doorways[MAX_ACTIVE_DOORWAYS];

//This is a mask of all keys held by all players. Robots use this to determine if a door is openable.
int Global_keys;

//	---------------------------------------------------------------------------
//	Prototypes
void DoorwayOpen(room* rp);
void DoorwayClose(room* rp);
void DoorwayWait(room* rp);
object* GetDoorObject(room* rp);


//	---------------------------------------------------------------------------
//	Functions
void RemoveActiveDoorway(int adway)
{
	room* rp = &Rooms[Active_doorways[adway]];

	ASSERT(rp->flags & RF_DOOR);
	ASSERT(rp->doorway_data != NULL);

	rp->doorway_data->activenum = -1;

	for (int i = adway; i < (Num_active_doorways - 1); i++)
	{
		Active_doorways[i] = Active_doorways[i + 1];
		Rooms[Active_doorways[i]].doorway_data->activenum--;
	}

	Num_active_doorways--;

	mprintf((0, "ActiveDoorways = %d\n", Num_active_doorways));
}

void AddActiveDoorway(int roomnum)
{
	room* rp = &Rooms[roomnum];
	ASSERT(rp->flags & RF_DOOR);

	doorway* dp = rp->doorway_data;
	ASSERT(dp != NULL);

	if (dp->activenum != -1) 
	{
		//already active
		ASSERT(dp->activenum < Num_active_doorways);
		ASSERT(Active_doorways[dp->activenum] == roomnum);
	}
	else 
	{
		//make new active
		ASSERT(Num_active_doorways < MAX_ACTIVE_DOORWAYS);
		Active_doorways[Num_active_doorways] = roomnum;
		dp->activenum = Num_active_doorways++;
	}

	mprintf((0, "ActiveDoorways = %d\n", Num_active_doorways));
}

// Given an object handle, returns the doorway number
doorway* GetDoorwayFromObject(int door_obj_handle)
{
	object* objp = ObjGet(door_obj_handle);

	if (!objp)
		return NULL;

	ASSERT(objp->type == OBJ_DOOR);

	room* rp = &Rooms[objp->roomnum];

	ASSERT(rp->flags & RF_DOOR);
	ASSERT(rp->doorway_data != NULL);

	return rp->doorway_data;
}

//Plays a sound for this door
void DoorwayPlaySound(object* objp)
{
	room* rp = &Rooms[objp->roomnum];
	ASSERT(rp->flags & RF_DOOR);

	doorway* dp = rp->doorway_data;
	ASSERT(dp != NULL);

	door* door = &Doors[dp->doornum];

	//Stop sound if one playing
	if (dp->sound_handle != -1) 
	{
		Sound_system.StopSoundImmediate(dp->sound_handle);
		dp->sound_handle = -1;
	}

	//Play new sound, if this door has one
	if ((dp->state == DOORWAY_OPENING) || (dp->state == DOORWAY_OPENING_AUTO)) 
	{
		if (door->open_sound != -1) 
		{
			float offset = dp->position * door->total_open_time;
			dp->sound_handle = Sound_system.Play3dSound(Doors[dp->doornum].open_sound, SND_PRIORITY_HIGH, objp, 1.0, 0, offset);
		}
	}
	else 
	{
		ASSERT(dp->state == DOORWAY_CLOSING);
		if (door->close_sound != -1) 
		{
			float offset = (1.0 - dp->position) * door->total_close_time;
			dp->sound_handle = Sound_system.Play3dSound(Doors[dp->doornum].close_sound, SND_PRIORITY_HIGH, objp, 1.0, 0, offset);
		}
	}
}

//Opens a door and, if it has the auto flag set, closes it
//Does not check to see if the door is openable -- the caller must do that.
void DoorwayActivate(int door_obj_handle)
{
	object* objp;
	doorway* dp;
	door* door;

	objp = ObjGet(door_obj_handle);

	if (!objp)
		return;

	ASSERT(objp->type == OBJ_DOOR);

	dp = GetDoorwayFromObject(door_obj_handle);
	if (!dp)
		return;

	//If already blasted, bail
	if (dp->flags & DF_BLASTED)
		return;

	//Get pointer to door
	door = &Doors[dp->doornum];

	//If already open or opening or waiting, do nothing
	if ((dp->state == DOORWAY_OPENING) || (dp->state == DOORWAY_WAITING) || (dp->state == DOORWAY_OPENING_AUTO))
		return;

	//Store new desired position
	dp->dest_pos = 1.0;

	//If not already active, create active entry
	AddActiveDoorway(objp->roomnum);

	//Set new state
	dp->state = (dp->flags & DF_AUTO) ? DOORWAY_OPENING_AUTO : DOORWAY_OPENING;

	//Play sound
	DoorwayPlaySound(objp);

	//Send notification event
	tOSIRISEventInfo ei;
	Osiris_CallEvent(objp, EVT_DOOR_ACTIVATE, &ei);
}

//Sets the current position of the door.  0.0 = totally closed, 1.0 = totally open
//Does not check to see if the door is openable -- the caller must do that.
void DoorwaySetPosition(int door_obj_handle, float pos)
{
	object* objp;
	doorway* dp;
	door* door;

	objp = ObjGet(door_obj_handle);

	if (!objp)
		return;

	ASSERT(objp->type == OBJ_DOOR);

	//Get pointer to doorway
	dp = GetDoorwayFromObject(door_obj_handle);
	if (!dp)
		return;

	//Get pointer to door
	door = &Doors[dp->doornum];

	//Set new dest position
	dp->dest_pos = pos;

	//Check if already there
	if (dp->position == pos)
		return;

	//If not already active, create active entry
	AddActiveDoorway(objp->roomnum);

	//Set new state & play sound
	if (dp->dest_pos > dp->position)
		dp->state = DOORWAY_OPENING;
	else
		dp->state = DOORWAY_CLOSING;

	//Play sound
	DoorwayPlaySound(objp);
}

void DoorwayStop(int door_obj_handle)
{
	//Get pointer to doorway
	doorway* dp = GetDoorwayFromObject(door_obj_handle);
	if (!dp)
		return;

	//Set the door as stopped
	dp->dest_pos = dp->position;
	dp->state = DOORWAY_STOPPED;

	//Stop the sound if one is playing
	if (dp->sound_handle != -1) {
		Sound_system.StopSoundImmediate(dp->sound_handle);
		dp->sound_handle = -1;
	}

	//Remove from the active doorway list
	if (dp->activenum != -1)
		RemoveActiveDoorway(dp->activenum);
}

//Called when a door is blown up
void DoorwayDestroy(object* objp)
{
	room* rp = &Rooms[objp->roomnum];
	doorway* dp = rp->doorway_data;

	ASSERT(rp->flags & RF_DOOR);
	ASSERT(dp);

	dp->flags |= DF_BLASTED;
	dp->position = 1.0;
	dp->state = DOORWAY_STOPPED;

	//Remove from the active doorway list
	if (dp->activenum != -1)
		RemoveActiveDoorway(dp->activenum);
}

//Stop all doors
void DoorwayDeactivateAll()
{
	int r;
	room* rp;

	//Go through all rooms and deactivate doors
	for (r = 0, rp = Rooms; r <= Highest_room_index; r++) 
	{
		if (rp->used && (rp->flags & RF_DOOR)) 
		{
			if (rp->doorway_data->state != DOORWAY_STOPPED) 
			{
				doorway* dp = rp->doorway_data;
				ASSERT(dp != NULL);
				dp->position = dp->dest_pos;
				dp->activenum = -1;
				dp->state = DOORWAY_STOPPED;
				if (dp->sound_handle != -1) {
					Sound_system.StopSoundImmediate(dp->sound_handle);
					dp->sound_handle = -1;
				}
				DoorwayUpdateAnimation(rp);
			}
		}
	}

	Num_active_doorways = 0;
}

// Sets the corresponding door objects animation frame
void DoorwayUpdateAnimation(room* rp)
{
	doorway* dway;
	poly_model* pm;
	int doornum;
	float norm;

	dway = rp->doorway_data;

	object* objp = GetDoorObject(rp);
	if (!objp)
		return;

	doornum = objp->id;
	pm = &Poly_models[GetDoorImage(doornum)];

	norm = DoorwayPosition(rp);

	if (pm->flags & PMF_TIMED)
		objp->rtype.pobj_info.anim_frame = pm->frame_max * norm;
	else
		objp->rtype.pobj_info.anim_frame = pm->max_keys * norm;
}

//	Update all doorways currently active in the mine
void DoorwayDoFrame()
{
	int	i_doorway;

	for (i_doorway = 0; i_doorway < Num_active_doorways; i_doorway++)
	{
		int roomnum = Active_doorways[i_doorway];
		room* rp = &Rooms[roomnum];
		ASSERT(rp->flags & RF_DOOR);
		doorway* dway = rp->doorway_data;
		ASSERT(dway != NULL);
		door* door = &Doors[dway->doornum];
		float delta;		//movement delta

		switch (dway->state)
		{
		case DOORWAY_OPENING:
		case DOORWAY_OPENING_AUTO:	//	doorway is opening
			delta = Frametime / door->total_open_time;

			dway->position += delta;

			if (dway->position >= dway->dest_pos) 
			{
				dway->position = dway->dest_pos;

				if (dway->state == DOORWAY_OPENING_AUTO) 
				{
					dway->state = DOORWAY_WAITING;
					dway->dest_pos = door->total_time_open;
				}
				else {		//	non automatic doors will stay open, hence are no longer active
					dway->state = DOORWAY_STOPPED;
					RemoveActiveDoorway(i_doorway);
				}
			}
			break;

		case DOORWAY_CLOSING:	//	doorway is closing
			delta = Frametime / door->total_close_time;

			dway->position -= delta;

			if (dway->position <= dway->dest_pos) 
			{
				dway->position = dway->dest_pos;

				dway->state = DOORWAY_STOPPED;
				RemoveActiveDoorway(i_doorway);

				//Send notification event
				object* objp = GetDoorObject(rp);
				ASSERT(objp != NULL);
				tOSIRISEventInfo ei;
				Osiris_CallEvent(objp, EVT_DOOR_CLOSE, &ei);
			}
			break;

		case DOORWAY_WAITING:	//	doorway is in wait state

			dway->dest_pos -= Frametime;

			if (dway->dest_pos <= 0.0) 
			{
				object* door_objp = GetDoorObject(rp);
				ASSERT(door_objp != NULL);

				//Time to start closing.  See if there's anything in the way
				if (!((door_objp->next != -1) || (door_objp->prev != -1))) {

					dway->dest_pos = 0.0;
					dway->state = DOORWAY_CLOSING;

					//Play sound
					DoorwayPlaySound(door_objp);
				}
			}
			break;

		default:
			Int3();						// these states shouldn't happen if door is active
		}

		//Update the animation state
		DoorwayUpdateAnimation(rp);
	}
}

//returns a pointer to the door object for the specified doorway
object* GetDoorObject(room* rp)
{
	ASSERT(rp->flags & RF_DOOR);

	for (int objnum = rp->objects; (objnum != -1); objnum = Objects[objnum].next)
		if (Objects[objnum].type == OBJ_DOOR)
			return &Objects[objnum];

	return NULL;
}

// Returns true if the doorway is locked, else false
bool DoorwayLocked(int door_obj_handle)
{
	doorway* dp = GetDoorwayFromObject(door_obj_handle);

	if (!dp)
		return 0;

	return ((dp->flags & DF_LOCKED) != 0);
}

// Returns true if the doorway is locked, else false
bool DoorwayLocked(room* rp)
{
	ASSERT(rp->flags & RF_DOOR);

	doorway* dp = rp->doorway_data;
	ASSERT(dp != NULL);

	return ((dp->flags & DF_LOCKED) != 0);
}

// Returns true if the doorway is openable by the specified object, else false
bool DoorwayOpenable(int door_obj_handle, int opener_handle)
{
	int keys;

	//Get pointer to doorway
	doorway* dp = GetDoorwayFromObject(door_obj_handle);
	if (!dp)
		return 0;

	//If locked, no one can open it
	if (dp->flags & DF_LOCKED)
		return 0;

	//If no keys needed, anyone can open
	if (dp->keys_needed == 0)
		return 1;

	object* opener = ObjGet(opener_handle);

	//If invalid object, can't open
	if (!opener)
		return 0;

	//If a weapon, get the parent
	if (opener->type == OBJ_WEAPON) 
	{
		opener = ObjGetUltimateParent(opener);
		ASSERT(opener && (opener->type != OBJ_WEAPON));
	}

	//Get the opener's keys
	if (opener->type == OBJ_PLAYER)
		keys = Players[opener->id].keys;
	else if ((opener->type == OBJ_ROBOT) || ((opener->type == OBJ_BUILDING) && opener->ai_info))
		keys = Global_keys;
	else
		return 0;	//If not a robot or a player, cannot open keyed doors

	//Door is openenable if have proper keys
	if (dp->flags & DF_KEY_ONLY_ONE)
		return ((keys & dp->keys_needed) != 0);
	else
		return ((keys & dp->keys_needed) == dp->keys_needed);
}

//Returns the current state of the specified door
int DoorwayState(int door_obj_handle)
{
	doorway* dp = GetDoorwayFromObject(door_obj_handle);

	if (!dp)
		return DOORWAY_STOPPED;

	return dp->state;
}

// Locks or unlocks a given doorway
void DoorwayLockUnlock(int door_obj_handle, bool state)
{
	doorway* dp = GetDoorwayFromObject(door_obj_handle);

	if (!dp)
		return;

	if (state)
		dp->flags |= DF_LOCKED;
	else
		dp->flags &= ~DF_LOCKED;
}

//Returns the current position of the door.  0.0 = totally closed, 1.0 = totally open
float DoorwayPosition(room* rp)
{
	ASSERT(rp->flags & RF_DOOR);
	ASSERT(rp->doorway_data != NULL);

	return rp->doorway_data->position;
}

//Returns the current position of the door.  0.0 = totally closed, 1.0 = totally open
float DoorwayPosition(int door_obj_handle)
{
	doorway* dp = GetDoorwayFromObject(door_obj_handle);

	if (!dp)
		return 0.0;

	return dp->position;
}

//Called after loading a saved game to rebuild the active list
void DoorwayRebuildActiveList()
{
	int r;
	room* rp;

	//Reset active doorways count
	Num_active_doorways = 0;

	//Go through all rooms and look for active doors
	for (r = 0, rp = Rooms; r <= Highest_room_index; r++) {
		if (rp->used && (rp->flags & RF_DOOR)) {
			ASSERT(rp->doorway_data != NULL);
			if (rp->doorway_data->state != DOORWAY_STOPPED)
				AddActiveDoorway(r);
		}
	}
}

// Adds a doorway to the specified room
// Returns a pointer to the doorway struct
doorway* DoorwayAdd(room* rp, int doornum)
{
	ASSERT(rp->doorway_data == NULL);

	rp->flags |= RF_DOOR;

	doorway* dp = rp->doorway_data = (doorway*)mem_malloc(sizeof(*rp->doorway_data));

	//Initialize
	dp->doornum = doornum;
	dp->dest_pos = dp->position = 0.0;
	dp->state = DOORWAY_STOPPED;
	dp->flags = DF_AUTO;
	dp->keys_needed = 0;
	dp->activenum = -1;
	dp->sound_handle = -1;

	return dp;
}
