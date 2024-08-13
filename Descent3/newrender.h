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
#include "room.h"

struct FogPortalData
{
	int roomnum;
	float close_dist;
	face* close_face;
};

class RenderList
{
	//List of all rooms that are currently visible
	std::vector<int> VisibleRoomNums;
	//Transiently sized and updated to check if a room has been iterated into. 
	std::vector<bool> RoomChecked;
	std::vector<FogPortalData> FogPortals;
	//Queue used for a room breadth first search
	std::queue<int> RoomCheckList;
	bool HasFoundTerrain;

	vector EyePos;
	matrix EyeOrient;
	int EyeRoomnum;

	void SetupLegacyFog(room& rp);
	
	//Adds a room. Will mark it as checked, add it to the visible list, 
	// and will check if each portal is visible and add linked rooms to the BFS queue. 
	bool PendingRooms() const
	{
		return !RoomCheckList.empty();
	}

	void PushRoom(int roomnum)
	{
		RoomCheckList.push(roomnum);
	}

	int PopRoom()
	{
		int check = RoomCheckList.front();
		RoomCheckList.pop();

		return check;
	}

	bool CheckFace(room& rp, face& fp, Frustum& frustum) const;
	void MaybeUpdateFogPortal(int roomnum, face& fp);
	//Adds a room to the visible list. Will check visibility of all portal faces,
	//and add all visibile connected rooms to the room check queue. 
	void AddRoom(int roomnum, Frustum& frustum);

	void PreDraw();
	void DrawWorld(int passnum);
public:
	RenderList();
	//Gathers all visible interactions
	//g3_StartFrame must have been called, this will use the current modelview and projection matricies loaded in the 3d system
	//The search starts from the specified roomnum, unless it is negative, then iteration will start from the terrain. 
	void GatherVisible(vector& eye_pos, matrix& eye_orient, int viewroomnum);

	//Draws the entire render list to the current view.
	void Draw();
};

//Called after LoadLevel, initializes the newrenderer for the current level.
void NewRender_InitNewLevel();

//Builds renderlists for the main camera, all mirrors, and so on
void NewRender_Render(vector& vieweye, matrix& vieworientation, int roomnum);
