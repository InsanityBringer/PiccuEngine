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
 

#include "selectedroom.h"

#ifdef NEWEDITOR
	#include "../neweditor/stdafx.h"
	#include "../neweditor/globals.h"
#else
	#include "d3edit.h"
#endif

#include "room.h"
#include "pserror.h"
#include "mem.h"

//Returns true if a room is in the selected list
int IsRoomSelected(int roomnum)
{
	int i;

	for (i=0;i<N_selected_rooms;i++)
		if (Selected_rooms[i] == roomnum)
			return 1;

	return 0;
}

//Adds a room to the selected list, if it's not already there
void AddRoomToSelectedList(int roomnum)
{
	if (! IsRoomSelected(roomnum)) {
		Selected_rooms[N_selected_rooms++] = roomnum;
		State_changed = 1;
	}
}

//Removes a room from the selected list, if it's there
void RemoveRoomFromSelectedList(int roomnum)
{
	int i;

	for (i=0;i<N_selected_rooms;i++)
		if (Selected_rooms[i] == roomnum) {
			int j;
			for (j=i;j<N_selected_rooms-1;j++)
				Selected_rooms[j] = Selected_rooms[j+1];
			N_selected_rooms--;
			State_changed = 1;
			return;
		}
}

//Empties the selected list
void ClearRoomSelectedList()
{
	N_selected_rooms = 0;
	State_changed = 1;
}

//Adds to selected list if not already in there, else removes from list
//Returns:	1 if room was selected, 0 if un-selected
int ToggleRoomSelectedState(int roomnum)
{
	int i;

	State_changed = 1;

	for (i=0;i<N_selected_rooms;i++)
		if (Selected_rooms[i] == roomnum) {
			int j;
			for (j=i;j<N_selected_rooms-1;j++)
				Selected_rooms[j] = Selected_rooms[j+1];
			N_selected_rooms--;
			return 0;	//room was un-selected
		}

	//not found, so add to list
	Selected_rooms[N_selected_rooms++] = roomnum;

	return 1;	//room was selected
}

//Add all the connected room to the selected list
//Parameters:	room - the starting room
//Returns:		the number of rooms added to the list
int SelectConnectedRooms(int roomnum)
{
	int count;
	int s;
		
	if (IsRoomSelected(roomnum))		//this room already selected?
		return 0;

	//Add this rooom to selected list
	Selected_rooms[N_selected_rooms++] = roomnum;
	count = 1;

	State_changed = 1;

	//Now add this room's children
	for (s=0;s<Rooms[roomnum].num_portals;s++)
	{
		if (Rooms[roomnum].portals[s].croom!=-1)
			count += SelectConnectedRooms(Rooms[roomnum].portals[s].croom);
	}

	return count;
}

static int *Save_selected_rooms;
static int N_save_selected_rooms=-1;		//-1 means empty list

//Save the (user's) selected list so that an internal function can use it
//You must call RestoreSelectedList() when you're done with the selected list
void SaveRoomSelectedList()
{
	int i;

	if (N_save_selected_rooms != -1) { 		//already have saved list?
		Int3();										//..yes, error
		return;
	}

	N_save_selected_rooms = N_selected_rooms;

	if (! N_save_selected_rooms)		//are there any rooms in the list?
		return;								//..no, so don't bother to save them

	Save_selected_rooms = (int *) mem_malloc(N_save_selected_rooms * sizeof(*Save_selected_rooms));

	ASSERT(Save_selected_rooms != NULL);

	for (i=0;i<N_selected_rooms;i++)
		Save_selected_rooms[i] = Selected_rooms[i];
}

//Restore a previously-saved selected list.  You must have previously called SaveSelectedList()
void RestoreRoomSelectedList()
{
	int i;

	if (N_save_selected_rooms == -1) { 		//no save list?
		Int3();										//..then generate error
		return;
	}

	N_selected_rooms = N_save_selected_rooms;

	N_save_selected_rooms = -1;				//say no saved list

	if (! N_selected_rooms)				//are there any rooms in the list?
		return;								//..no, so don't restore them

	for (i=0;i<N_selected_rooms;i++)
		Selected_rooms[i] = Save_selected_rooms[i];

	mem_free(Save_selected_rooms);				//free up memory
}
