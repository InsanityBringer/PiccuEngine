/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */
 

#include <string.h>

#include "stdafx.h"
#include "ned_Tablefile.h"
#include "ned_Door.h"
#include "pserror.h"
#include "polymodel.h"
#include "game.h"
#include "doorway.h"


//Misc

#include "player.h"
extern player Players[MAX_PLAYERS];

//	---------------------------------------------------------------------------
//	Globals

// from door.cpp

ned_door_info		Doors[MAX_DOORS];
int Num_doors=0;

// from doorway.cpp

//This is a mask of all keys held by all players. Robots use this to determine if a door is openable.
int Global_keys;


//	---------------------------------------------------------------------------
//	Prototypes


//	---------------------------------------------------------------------------
//	Functions

// =========================
// ned_FreeAllDoors
// =========================
//
// Frees all doors from memory
void ned_FreeAllDoors ()
{
	for (int i=0;i<MAX_DOORS;i++)
	{
		if (Doors[i].used)
			ned_FreeDoor (i,true);
	}
}

// ======================
// ned_InitDoors
// ======================
// 
// Initializes the Door system
int ned_InitDoors ()
{
	// Initializes the door system

	int i,j;
	
	mprintf ((0,"Initializing door system.\n"));

	memset(Doors,0,sizeof(ned_door_info)*MAX_DOORS);
	for(i=0;i<MAX_DOORS;i++)
	for(j=0;j<MAX_LOADED_TABLE_FILES;j++)
		Doors[i].table_stack[j] = -1;
	
	atexit (ned_FreeAllDoors);

	return 1;
}

// ===========================
// ned_InitializeDoorData
// ===========================
//
// Given a Doors slot this function initializes the data inside it
// to default information
// DO NOT TOUCH TABLE STACK DATA
void ned_InitializeDoorData(int slot)
{
	Doors[slot].model_handle = -1;
	Doors[slot].flags = 0;		//default to no type
	Doors[slot].ref_count = 0;
	Doors[slot].hit_points = 0;
}

// =======================
// ned_FreeDoorData
// =======================
//
// Given a Doors slot this function frees any memory that may 
// need to be freed before a door is destroyed
// DO NOT TOUCH TABLE STACK DATA
void ned_FreeDoorData(int slot)
{
	int model_num = GetDoorImage(slot);
	if (model_num >= 0)
		FreePolyModel(model_num);
	Doors[slot].ref_count = 0;
}

// Frees door index n
void FreeDoor (int n)
{
	ASSERT (Doors[n].used); // >0
	
	Doors[n].used=0;
	Doors[n].name[0]=0;
	Num_doors--;
}

// Gets next door from n that has actually been alloced
int GetNextDoor (int n)
{
	int i;

	ASSERT (n>=0 && n<MAX_DOORS);

	if (Num_doors==0)
		return -1;

	for (i=n+1;i<MAX_DOORS;i++)
		if (Doors[i].used)
			return i;
	for (i=0;i<n;i++)
		if (Doors[i].used)
			return i;

	// this is the only one

	return n;
}

// Gets previous door from n that has actually been alloced
int GetPrevDoor (int n)
{
	int i;

	ASSERT (n>=0 && n<MAX_DOORS);

	if (Num_doors==0)
		return -1;

	for (i=n-1;i>=0;i--)
	{
		if (Doors[i].used)
			return i;
	}
	for (i=MAX_DOORS-1;i>n;i--)
	{
		if (Doors[i].used)
			return i;
	}

	// this is the only one
	return n;

}

// Given a filename, loads the model found in that file

int LoadDoorImage (char *filename,int pageable)
{
	int img_handle;
	
	img_handle=LoadPolyModel (filename,pageable);

	return img_handle;
}

// Given a door handle, returns an index to that doors model
int GetDoorImage (int handle)
{
	ASSERT (Doors[handle].used); // >0
		
	return (Doors[handle].model_handle);
}

void RemapDoors ()
{
	// Now, if any doors are polygon models and those models don't have correct
	// textures, attempt to reload the model texture list

	for (int i=0;i<MAX_DOORS;i++)
	{
		if (Doors[i].used)
		{
			ASSERT (Doors[i].model_handle!=-1);
			//LoadPolyModel (Poly_models[Doors[i].model_handle].name);
		}
	}

}



// from doorway.h

// Given an object handle, returns the doorway number
doorway *GetDoorwayFromObject(int door_obj_handle)
{
	object *objp = ObjGet(door_obj_handle);

	if (! objp)
		return NULL;

	ASSERT(objp->type == OBJ_DOOR);

	room *rp = &Rooms[objp->roomnum];

	ASSERT(rp->flags & RF_DOOR);
	ASSERT(rp->doorway_data != NULL);

	return rp->doorway_data;
}

//Returns the current position of the door.  0.0 = totally closed, 1.0 = totally open
float DoorwayGetPosition(room *rp)
{
	ASSERT(rp->flags & RF_DOOR);
	ASSERT(rp->doorway_data != NULL);

	return rp->doorway_data->position;
}

//Returns the current position of the door.  0.0 = totally closed, 1.0 = totally open
float DoorwayPosition(int door_obj_handle)
{
	doorway *dp = GetDoorwayFromObject(door_obj_handle);

	if (!dp)
		return 0.0;

	return dp->position;
}

// Returns true if the doorway is locked, else false
bool DoorwayLocked(int door_obj_handle)
{
	doorway *dp = GetDoorwayFromObject(door_obj_handle);

	if (!dp)
		return 0;

	return ((dp->flags & DF_LOCKED) != 0);
}

// Returns true if the doorway is locked, else false
bool DoorwayLocked(room *rp)
{
	ASSERT(rp->flags & RF_DOOR);

	doorway *dp = rp->doorway_data;
	ASSERT(dp != NULL);

	return ((dp->flags & DF_LOCKED) != 0);
}



// Adds a doorway to the specified room
// Returns a pointer to the doorway struct
doorway *DoorwayAdd(room *rp,int doornum)
{
	ASSERT(rp->doorway_data == NULL);

	rp->flags |= RF_DOOR;

	doorway *dp = rp->doorway_data = (doorway *) mem_malloc(sizeof(*rp->doorway_data));

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


// Returns true if the doorway is openable by the specified object, else false
bool DoorwayOpenable(int door_obj_handle,int opener_handle)
{
	int keys;

	//Get pointer to doorway
	doorway *dp = GetDoorwayFromObject(door_obj_handle);
	if (!dp)
		return 0;

	//If locked, no one can open it
	if (dp->flags & DF_LOCKED)
		return 0;

	//If no keys needed, anyone can open
	if (dp->keys_needed == 0)
		return 1;

	object *opener = ObjGet(opener_handle);

	//If invalid object, can't open
	if (! opener)
		return 0;

	//If a weapon, get the parent
	if (opener->type == OBJ_WEAPON) {
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


//returns a pointer to the door object for the specified doorway
object *GetDoorObject(room *rp)
{
	ASSERT(rp->flags & RF_DOOR);
	
	for (int objnum=rp->objects;(objnum!=-1);objnum=Objects[objnum].next) 
		if (Objects[objnum].type==OBJ_DOOR)
			return &Objects[objnum];

	return NULL;
}


// neweditor functions

// ========================
// ned_MarkDoorInUse
// ========================
//
// Handles memory management for a door.  Call this, passing true when you need to use a object
// when the door is no longer needed, call this again, passing false.
void ned_MarkDoorInUse(int slot,bool inuse)
{
	ASSERT(slot>=0 && slot<MAX_DOORS);
	if(slot<0 || slot>=MAX_DOORS)
		return;

	ASSERT(Doors[slot].used);
	if(!Doors[slot].used)
		return;

	if(inuse)
	{
		ASSERT(Doors[slot].ref_count>=0);

		if(Doors[slot].ref_count==0)
		{
			//load in the door polymodel
			Doors[slot].model_handle = LoadPolyModel(Doors[slot].image_filename,0);

			if(Doors[slot].model_handle>=0)
			{
				PageInPolymodel (Doors[slot].model_handle);
				// Mark the polymodel textures in use
				poly_model *pm = GetPolymodelPointer(Doors[slot].model_handle);
				for (int i=0; i<pm->n_textures; i++)
					ned_MarkTextureInUse(pm->textures[i],true);
			}
		}
		Doors[slot].ref_count++;
	}else
	{
		ASSERT(Doors[slot].ref_count>0);
		Doors[slot].ref_count--;
		if(Doors[slot].ref_count==0)
		{
			//unload the door polymodel...no longer needed
			FreePolyModel(Doors[slot].model_handle);
			Doors[slot].model_handle = -1;			
		}
	}
}


// ====================
// ned_FindDoor
// ====================
//
// given the name of a door it will return it's id...-1 if it doesn't exist
// it will only search doors loaded from tablefiles
int ned_FindDoor(char *name)
{
	ASSERT(name);
	if(!name)
		return -1;

	int i;
	for(i=0;i<MAX_DOORS;i++)
	{
		if(Doors[i].used && Doors[i].table_file_id!=-1)
		{
			//see if the name matches
			if(!strnicmp(name,Doors[i].name,PAGENAME_LEN-1))
			{
				//match
				return i;
			}
		}
	}
	return -1;
}

// =====================
// ned_AllocDoor
// =====================
//
//	Searches for an available door slot, and returns it, -1 if none
//  if name is not NULL, then it is being allocated from a table file, and thus, the tablefile
//	parameter must also be specified.  If there is already a texture by that name (loaded from 
//  a table file), than it will be pushed onto the stack.
int ned_AllocDoor(char *name,int tablefile)
{
	if(name)
	{
		//this is being allocated for a tablefile load
		ASSERT(tablefile!=-1);
		if(tablefile==-1)
			return -1;

		//check to see if it's already in memory
		int old_id = ned_FindDoor(name);
		if(old_id!=-1)
		{
			//this item is already in memory!
			ned_FreeDoorData(old_id);
			ned_InitializeDoorData(old_id);			

			if(Doors[old_id].table_file_id==tablefile)
			{
				//we're just re-reading it

			}else
			{
				//push it onto the stack
				ntbl_PushTableStack(Doors[old_id].table_stack,Doors[old_id].table_file_id);
				ntbl_IncrementTableRef(tablefile);

				Doors[old_id].table_file_id = tablefile;
			}
			return old_id;
		}
	}

	int i,index = -1;
	for(i=0;i<MAX_DOORS;i++)
	{
		if(!Doors[i].used)
		{
			index = i;
			memset(&Doors[i],0,sizeof(ned_door_info));
			Doors[i].model_handle = -1;		// we don't have a door model loaded
			Doors[i].table_file_id = -1;	//we don't belong to any table file right now
			for(int j=0;j<MAX_LOADED_TABLE_FILES;j++) Doors[i].table_stack[j] = -1;
			break;
		}
	}

	if(index!=-1)
	{
		Doors[index].used = true;
		Num_doors++;

		if(name)
		{
			ASSERT(strlen(name)<PAGENAME_LEN);

			//table file load, give it a name and mark it's tablefile
			strncpy(Doors[index].name,name,PAGENAME_LEN-1);
			Doors[index].name[PAGENAME_LEN-1] = '\0';
			Doors[index].table_file_id = tablefile;

			ntbl_IncrementTableRef(tablefile);

		}else
		{
			//not from tablefile
			Doors[index].name[0] = '\0';
			Doors[index].table_file_id = -1;
		}

		ned_InitializeDoorData(index);			
	}
	
	return index;
}


// ==================
// ned_FreeDoor
// ==================
//
// given a door slot it free's it and makes it available
// if force_unload is true, and this slot was loaded from a table file, then
// it will unload all instances (based on it's table stack) from memory, else
// it will just pop off the current instance and load back in the popped version
void ned_FreeDoor(int slot,bool force_unload)
{
	ASSERT(slot>=0 && slot<MAX_DOORS);
	if(slot<0 || slot>=MAX_DOORS)
		return;

	ASSERT(Doors[slot].used);
	if(!Doors[slot].used)
		return;

	ned_FreeDoorData(slot);

	/////////////////////////////////////////
	if(Doors[slot].table_file_id==-1)
	{
		Doors[slot].used = false;
		Num_doors--;
		return;
	}

	//it has table file references, decrement
	ntbl_DecrementTableRef(Doors[slot].table_file_id);

	if(force_unload)
	{
		Doors[slot].used = false;
		Num_doors--;

		//go through it's stack, decrement references
		int tid;
		tid = ntbl_PopTableStack(Doors[slot].table_stack);
		while(tid!=-1)
		{
			ntbl_DecrementTableRef(tid);
			tid = ntbl_PopTableStack(Doors[slot].table_stack);
		}
	}else
	{
		//see if we have anything on the stack
		Doors[slot].table_file_id = ntbl_PopTableStack(Doors[slot].table_stack);
		if(Doors[slot].table_file_id==-1)
		{
			//nothing on the stack, its a dead one
			Doors[slot].used = false;
			Num_doors--;
		}else
		{
			//reload the item
			ned_InitializeDoorData(slot);
			if(!ntbl_OverlayPage(PAGETYPE_DOOR,slot))
			{
				Int3();

				Doors[slot].used = false;
				Num_doors--;
			}			
		}
	}
}

