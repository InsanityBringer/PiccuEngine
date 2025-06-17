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
 


#ifndef _NED_DOOR_H
#define _NED_DOOR_H

#include "pstypes.h"
#include "manage.h"
#include "object.h"


typedef struct
{
	ubyte used;
	char name[PAGENAME_LEN];

	int flags;
	short hit_points;
	char image_filename[PAGENAME_LEN];
	int	model_handle;					// an index into the polymodels array

	int ref_count;
	int table_file_id;
	int table_stack[MAX_LOADED_TABLE_FILES];
}ned_door_info;

#define MAX_DOORS		60
extern ned_door_info	Doors[MAX_DOORS];
extern int Num_doors;


// IMPORTANT!!!!!!!!!!!
// "Doors" refers to a predefined door that is in memory
// "Doorways" are specific doors that are in the mine
// So, there can be several Doorways that all point to the same Door
// Get it?  If not, talk to Samir or Jason


// Door flags

#define DF_BLASTABLE		1	// this door can be destroyed
#define DF_SEETHROUGH	2	// this door can be seen through even when closed

// Sets all doors to unused
void InitDoors();

// Allocs a door for use, returns -1 if error, else index on success
int AllocDoor ();

// Frees door index n
void FreeDoor (int n);

// Gets next door from n that has actually been alloced
int GetNextDoor (int n);
// Gets previous door from n that has actually been alloced
int GetPrevDoor (int n);
// Searches thru all doors for a specific name, returns -1 if not found
// or index of door with name
int ned_FindDoorName (char *name);

// ====================
// ned_FindDoor
// ====================
//
// given the name of a door it will return it's id...-1 if it doesn't exist
// it will only search doors loaded from tablefiles
int ned_FindDoor(char *name);

// Initializes the Door system
int ned_InitDoors ();

// Given a filename, loads the model found in that file
int LoadDoorImage (char *filename,int pageable=1);
// Given a door handle, returns an index to that doors model
int GetDoorImage (int handle);

//	Remaps the doors
void RemapDoors();

// =====================
// ned_AllocDoor
// =====================
//
//	Searches for an available door slot, and returns it, -1 if none
//  if name is not NULL, then it is being allocated from a table file, and thus, the tablefile
//	parameter must also be specified.  If there is already a texture by that name (loaded from 
//  a table file), than it will be pushed onto the stack.
int ned_AllocDoor(char *name=NULL,int tablefile=-1);

// ==================
// ned_FreeDoor
// ==================
//
// given a door slot it free's it and makes it available
// if force_unload is true, and this slot was loaded from a table file, then
// it will unload all instances (based on it's table stack) from memory, else
// it will just pop off the current instance and load back in the popped version
void ned_FreeDoor(int slot,bool force_unload=false);

// ========================
// ned_MarkDoorInUse
// ========================
//
// Handles memory management for a door.  Call this, passing true when you need to use a object
// when the door is no longer needed, call this again, passing false.
void ned_MarkDoorInUse(int slot,bool inuse);

#endif /* _NED_DOOR_H */