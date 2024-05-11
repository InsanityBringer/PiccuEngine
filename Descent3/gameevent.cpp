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

#include "object.h"
#include "game.h"
#include "gameevent.h"
#include "descent.h"
#include "gameloop.h"
#include "spew.h"
#include <string.h>
#include "mem.h"
#include "player.h"


int Num_events = 0;
game_event GameEvent[MAX_EVENTS];

// Inits the event system
void InitEvents()
{
	for (int i = 0; i < MAX_EVENTS; i++)
	{
		GameEvent[i].used = 0;
		GameEvent[i].data = NULL;
	}
}

// Clears the event list
void ClearAllEvents()
{
	for (int i = 0; i < MAX_EVENTS; i++)
	{
		if (GameEvent[i].used)
			FreeEvent(i);
	}

	SpewInit();
	Render_zoom = D3_DEFAULT_ZOOM;
}

// Returns an index of an allocated event
// or -1 if all events are spoken for
int AllocEvent()
{
	static int first_time = 1;

	if (first_time)
	{
		InitEvents();
		first_time = 0;
	}

	for (int i = 0; i < MAX_EVENTS; i++)
	{
		if (GameEvent[i].used == 0)
		{
			ASSERT(GameEvent[i].data == NULL);
			memset(&GameEvent[i], 0, sizeof(game_event));
			GameEvent[i].used = 1;
			GameEvent[i].objhandle_detonator = OBJECT_HANDLE_NONE;
			Num_events++;
			return i;
		}

	}

	return -1;	// no events free!
}

// Frees an event for use by others
void FreeEvent(int index)
{
	if (GameEvent[index].used != 1)
	{
		Int3();	// Get Jason, trying to free an event that is not used
		return;
	}
	GameEvent[index].used = 0;
	if (GameEvent[index].data != NULL)
	{
		mem_free(GameEvent[index].data);
		GameEvent[index].data = NULL;
	}

	Num_events--;
}

// Processes all pending events, removing the ones that are expired
void ProcessNormalEvents()
{
	int count = 0;
	bool skip_event;

	for (int i = 0; i < MAX_EVENTS && count < Num_events; i++)
	{
		if (GameEvent[i].used)
		{
			count++;
			skip_event = false;

			if (GameEvent[i].objhandle_detonator != OBJECT_HANDLE_NONE) {
				//this object has a detonator, see if the object is still alive
				object* obj = ObjGet(GameEvent[i].objhandle_detonator);
				if (!obj || (obj->type == OBJ_GHOST) || (obj->type == OBJ_PLAYER && Players[obj->id].flags & (PLAYER_FLAGS_DYING | PLAYER_FLAGS_DEAD))) {
					//the object has died, kill the event
					FreeEvent(i);
					skip_event = true;
					mprintf((0, "Game Event %d cancelled\n", i));
				}
			}

			if (GameEvent[i].type != RENDER_EVENT && GameEvent[i].end_time <= Gametime && !skip_event)
			{
				HandleEvent(&GameEvent[i]);
				FreeEvent(i);
			}

		}
	}

	SpewEmitAll();
}


// Goes through all events and returns the first index of the id matching the argument passed.
// If not match found, returns -1
int FindEventID(int id)
{
	for (int i = 0; i < MAX_EVENTS; i++)
	{
		if (GameEvent[i].used && GameEvent[i].id == id)
			return i;
	}

	return -1;
}

// Processes all pending events, removing the ones that are expired
void ProcessRenderEvents()
{
	int count = 0;

	for (int i = 0; i < MAX_EVENTS && count < Num_events; i++)
	{
		if (GameEvent[i].used)
		{
			count++;
			if (GameEvent[i].type == RENDER_EVENT && GameEvent[i].end_time <= Gametime)
			{
				HandleEvent(&GameEvent[i]);
				FreeEvent(i);
			}

		}
	}
}


// Adds new event to the event list.  The event will trigger at Gametime+length.
// data and size represent event specific data - if there is no event specific data
// then set data to NULL and size to 0
// Subfunction is the function that is called when the event is up
int CreateNewEvent(int type, int id, float length, void* data, int size, void(*subfunction)(int eventnum, void* data), int objhandle_detonate)
{
	int num;
	game_event* ge;

	num = AllocEvent();
	if (num == -1)
	{
		mprintf((0, "WARNING: Event (type %d, id %d) did not get allocated\n", type, id));
		return -1;	// couldn't do this event, no slots free
	}

	ge = &GameEvent[num];

	ge->type = type;
	ge->id = id;
	ge->start_time = Gametime;
	ge->end_time = Gametime + length;
	ge->frame_born = FrameCount;
	ge->objhandle_detonator = objhandle_detonate;

	if (size)
	{
		ge->data = (void*)mem_malloc(size);
		ASSERT(ge->data);
		memcpy(ge->data, data, size);
	}

	ge->subfunction = subfunction;

	return num;
}

// Does a specific action according to an event
void HandleEvent(game_event* ge)
{
	switch (ge->type)
	{
	case RENDER_EVENT:
		ge->subfunction(ge - GameEvent, ge->data);
		break;
	default:
		ge->subfunction(ge - GameEvent, ge->data);
		break;
	}
}
