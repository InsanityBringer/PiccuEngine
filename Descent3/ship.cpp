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

#include "ship.h"
#include "pstypes.h"
#include "pserror.h"
#include "object.h"
#include "3d.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "polymodel.h"
#include "player.h"
#include "robotfire.h"

#define DEFAULT_SHIP_SIZE	4.0

ship Ships[MAX_SHIPS];
int Num_ships=0;

//There are no static ships
char *Static_ship_names[1];
#define NUM_STATIC_SHIPS 0

#define SHIP_PYRO_ID	0
#define SHIP_PHOENIX_ID	1
#define SHIP_MAGNUM_ID	2

char *AllowedShips[MAX_SHIPS] = { "Pyro-GL", "Phoenix", "Magnum-AHT" };

// Sets all ships to unused
void InitShips ()
{
	for (int i=0;i<MAX_SHIPS;i++)
	{
		memset(&Ships[i], 0, sizeof(ship));
	}
	Num_ships=0;
}

// Allocs a ship for use, returns -1 if error, else index on success
int AllocShip ()
{
	for (int i=0;i<MAX_SHIPS;i++)
	{
		if (Ships[i].used==0)
		{
			memset (&Ships[i],0,sizeof(ship));
			Ships[i].used=1;
			Ships[i].size=DEFAULT_SHIP_SIZE;
			Ships[i].dying_model_handle=-1;
			Ships[i].med_render_handle=-1;
			Ships[i].lo_render_handle=-1;
			Ships[i].med_lod_distance=DEFAULT_MED_LOD_DISTANCE;
			Ships[i].lo_lod_distance=DEFAULT_LO_LOD_DISTANCE;
			Ships[i].model_handle=-1;
			Ships[i].armor_scalar=1.0;
			Ships[i].flags = 0;

			Ships[i].phys_info.hit_die_dot = -1;	//-1 mean doesn't apply

			// Make sure the weapon battery info is cleared for a new object
			WBClearInfo(Ships[i].static_wb);

			for (int w=0;w<MAX_PLAYER_WEAPONS;w++)
			{
				Ships[i].firing_sound[w] = -1;
//				Ships[i].release_sound[w] = -1;
			}

			Num_ships++;
			return i;
		}
	}

	Int3();		 // No ships free!
	return -1;
}

// Frees ship index n
void FreeShip (int n)
{
	ASSERT (Ships[n].used>0);

	Ships[n].used=0;
	Ships[n].name[0]=0;
	Num_ships--;
}

// Gets next ship from n that has actually been alloced
int GetNextShip (int n)
{
	int i;

	if (Num_ships==0)
		return -1;

	if ((n < 0) || (n >= MAX_SHIPS))
		n = -1;

	for (i=n+1;i<MAX_SHIPS;i++)
		if (Ships[i].used)
			return i;

	for (i=0;i<n;i++)
		if (Ships[i].used)
			return i;

	// this is the only one
	return n;
}

// Gets previous ship from n that has actually been alloced
int GetPrevShip (int n)
{
	int i;

	if (Num_ships==0)
		return -1;

	if ((n < 0) || (n >= MAX_SHIPS))
		n = MAX_SHIPS;

	for (i=n-1;i>=0;i--)
	{
		if (Ships[i].used)
			return i;
	}
	for (i=MAX_SHIPS-1;i>n;i--)
	{
		if (Ships[i].used)
			return i;
	}

	// this is the only one
	return n;
}

// Searches thru all ships for a specific name, returns -1 if not found
// or index of ship with name
int FindShipName (char *name)
{
	int i;

	ASSERT (name!=NULL);

	for (i=0;i<MAX_SHIPS;i++)
		if (Ships[i].used && !stricmp (name,Ships[i].name))
			return i;

	return -1;
}


int LoadShipImage (char *filename)
{
	int img_handle=LoadPolyModel (filename,1);

	return img_handle;
}

// Given a ship handle, returns an index to that ships model
int GetShipImage (int handle)
{
	ASSERT (Ships[handle].used>0);
		
	return (Ships[handle].model_handle);
}

// This is a very confusing function.  It takes all the ships that we have loaded 
// and remaps then into their proper places (if they are static). 
void RemapShips ()
{
	int i;
#if NUM_STATIC_SHIPS > 0
	//Loop through the static ships and move them to the correct slots
	for (i=0;i<NUM_STATIC_SHIPS;i++)
	{
		int cur_index = FindShipName(Static_ship_names[i]);

		if (cur_index == -1)
		{
			Int3();		//couldn't find statically-mapped ship
			continue;
		}
	
		if (cur_index != i)
		{
			//not in right slot
			if (Ships[i].used)
			{
				//someone else in this slot, so swap
				ship tship = Ships[i];
				Ships[i] = Ships[cur_index];
				Ships[cur_index] = tship;
				RemapAllShipObjects(i,MAX_SHIPS);
				RemapAllShipObjects(cur_index,i);
				RemapAllShipObjects(MAX_SHIPS,cur_index);
			}
			else
			{
				//slot is unused, so just take it
				Ships[i] = Ships[cur_index];
				Ships[cur_index].used = 0;
				RemapAllShipObjects(cur_index,i);
			}
		}
	}
#endif

	// Now, if any ships are polygon models and those models don't have correct
	// textures, attempt to reload the model texture list
	for (i=0;i<MAX_SHIPS;i++)
	{
		if (Ships[i].used)
		{
			ASSERT (Ships[i].model_handle!=-1);
			//LoadPolyModel (Poly_models[Ships[i].model_handle].name);
		}
	}
}

// goes thru every entity that could possible have a ship index 
// and changes the old index to the new index
void RemapAllShipObjects (int old_index,int new_index)
{
	for (int i=0;i<MAX_OBJECTS;i++)
	{
		if (Objects[i].type==OBJ_PLAYER)
		{
			if (Players[Objects[i].id].ship_index==old_index)
				Players[Objects[i].id].ship_index=new_index;
		}
	}
}
