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

#ifndef DOOR_H
#define DOOR_H

#ifdef NEWEDITOR /* only include tablefile header (manage stuff for NEWEDITOR) */
#include "..\neweditor\ned_TableFile.h"
#include "..\neweditor\ned_Door.h"
#include "pstypes.h"
#include "object.h"
#else

#include "pstypes.h"
#include "manage.h"
#include "object.h"


// IMPORTANT!!!!!!!!!!!
// "Doors" refers to a predefined door that is in memory
// "Doorways" are specific doors that are in the mine
// So, there can be several Doorways that all point to the same Door
// Get it?  If not, talk to Samir or Jason


// Door flags

#define DF_BLASTABLE		1	// this door can be destroyed
#define DF_SEETHROUGH	2	// this door can be seen through even when closed

//	DOOR STRUCTURES

struct door
{
	char name[PAGENAME_LEN];		// name of the door
	ubyte used;							// if this door is in use
	ubyte flags;						// flags for this door
	ubyte pad;							// keep alignment (pagename is 35 chars long)
	short	hit_points;					// for blastable doors
	float total_open_time;			// time of animation to open door		
	float total_close_time;			// time of animation to close door
	float total_time_open;			// how much time to stay open
	int	model_handle;					// an index into the polymodels array
	int	open_sound;					// sound to play when closing
	int	close_sound;				// sound to play when closing

	// OSIRIS information
	char module_name[MAX_MODULENAME_LEN];

};

// The max number of predefined doors
#define MAX_DOORS	60
extern int Num_doors;				// number of actual doors in game.
extern door Doors[];				


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
int FindDoorName (char *name);

// Given a filename, loads the model found in that file
int LoadDoorImage (char *filename,int pageable=1);
// Given a door handle, returns an index to that doors model
int GetDoorImage (int handle);

//	Remaps the doors
void RemapDoors();

#endif

#endif
