/*
* Descent 3: Piccu Engine
* Copyright (C) 2024 SaladBadger
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

#pragma once
#include <vector>
#include "vecmat.h"
#include <queue>
#include "3d.h"
#include "pserror.h"

class RenderList
{
	//List of all rooms that are currently visible
	std::vector<int> VisibleRoomNums;
	//Transiently sized and updated to check if a room has been iterated into. 
	std::vector<bool> RoomChecked;
	//Double set of check rooms. One level will be filled while the other is processed. These flip when one empties out
	std::queue<int> RoomCheckList[2];
	int CurrentCheck;
	
	//Adds a room. Will mark it as checked, add it to the visible list, 
	// and will check if each portal is visible and add linked rooms to the BFS queue. 
	bool PendingRooms()
	{
		return RoomCheckList[0].size() > 0 || RoomCheckList[1].size() > 0;
	}

	void PushRoom(int roomnum)
	{
		if (RoomChecked[roomnum])
			return;

		int listnum = CurrentCheck ^ 1;
		RoomCheckList[listnum].push(roomnum);
	}

	int PopRoom()
	{
		assert(CurrentCheck == 0 || CurrentCheck == 1);
		int check = RoomCheckList[CurrentCheck].back();
		RoomCheckList[CurrentCheck].pop();

		if (RoomCheckList[CurrentCheck].empty())
			CurrentCheck ^= 1;
		return check;
	}

	//Adds a room to the visible list. Will check visibility of all portal faces,
	//and add all visibile connected rooms to the room check queue. 
	void AddRoom(int roomnum, Frustum& frustum);
public:
	RenderList();
	//Gathers all visible interactions
	//g3_StartFrame must have been called, this will use the current modelview and projection matricies loaded in the 3d system
	//The search starts from the specified roomnum, unless it is negative, then iteration will start from the terrain. 
	void GatherVisible(vector& eye_pos, int viewroomnum);
};

//Called after LoadLevel, initializes the newrenderer for the current level.
void NewRender_InitNewLevel();
