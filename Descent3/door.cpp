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

#include <string.h>

#include "door.h"
#include "pserror.h"
#include "polymodel.h"
#include "game.h"
#include "doorway.h"


//	---------------------------------------------------------------------------
//	Globals

door Doors[MAX_DOORS];					// door info.
int Num_doors=0;


//	---------------------------------------------------------------------------
//	Prototypes


//	---------------------------------------------------------------------------
//	Functions

// Sets all doors to unused
void InitDoors()
{
	for (int i=0;i<MAX_DOORS;i++)
	{
		memset (&Doors[i],0,sizeof(door));
		Doors[i].model_handle=-1;
		//Doors[i].script_name[0] = 0;
	}
	Num_doors=0;
}

// Allocs a door for use, returns -1 if error, else index on success
int AllocDoor ()
{
	for (int i=0;i<MAX_DOORS;i++)
	{
		if (Doors[i].used==0)
		{
			Doors[i].used=1;
			//Doors[i].script_name[0] = 0;
			Doors[i].flags = 0;
			Doors[i].hit_points = 0.0;
			Num_doors++;
			return i;
		}
	}

	Int3();		 // No doors free!
	return -1;
}

// Frees door index n
void FreeDoor (int n)
{
	ASSERT (Doors[n].used>0);
	
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
// Searches thru all doors for a specific name, returns -1 if not found
// or index of door with name
int FindDoorName (char *name)
{
	int i;

	ASSERT (name!=NULL);

	for (i=0;i<MAX_DOORS;i++)
		if (Doors[i].used && !stricmp (name,Doors[i].name))
			return i;

	return -1;
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
	ASSERT (Doors[handle].used>0);
		
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
