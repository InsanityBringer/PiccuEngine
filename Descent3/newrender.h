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
#include <algorithm>
#include <unordered_map>
#include "vecmat.h"
#include <queue>
#include "3d.h"
#include "pserror.h"
#include "room.h"

struct NewRenderWindow
{
	int left, top, right, bottom;

	NewRenderWindow()
	{
		left = top = right = bottom = 0;
	}

	NewRenderWindow(int left, int top, int right, int bottom)
		: left(left), top(top), right(right), bottom(bottom)
	{
	}

	//Clips the window to another window. 
	//Returns false if the window is entirely outside the other window, true if some is visible.
	bool Clip(const NewRenderWindow& window)
	{
		if (left > window.right || top > window.bottom || right < window.left || bottom < window.top)
			return false;

		left = std::max(window.left, left);
		top = std::max(window.top, top);
		right = std::min(window.right, right);
		bottom = std::min(window.bottom, bottom);

		return true;
	}

	//Grows the window to contain another window.
	void Encompass(const NewRenderWindow& window)
	{
		left = std::min(window.left, left);
		top = std::min(window.top, top);
		right = std::max(window.right, right);
		bottom = std::max(window.bottom, bottom);
	}

	//Grows the window to encompass a single point in space
	void Encompass(int x, int y)
	{
		left = std::min(x, left);
		top = std::min(y, top);
		right = std::max(x, right);
		bottom = std::max(y, bottom);
	}
};

struct FogPortalData
{
	int roomnum;
	float close_dist;
	face* close_face;
};

struct RenderListEntry
{
	int roomnum;
	NewRenderWindow window;

	RenderListEntry()
	{
		roomnum = 0;
	}

	RenderListEntry(int roomnum, NewRenderWindow& window)
		: roomnum(roomnum), window(window)
	{
	}
};

class RenderList
{
	//List of all rooms that are currently visible
	std::vector<RenderListEntry> VisibleRooms;
	//Transiently sized and updated to check if a room has been iterated into, and where it is in the render list. 
	std::vector<int> RoomChecked;
	std::vector<FogPortalData> FogPortals;
	//Queue used for a room breadth first search
	std::queue<RenderListEntry> RoomCheckList;
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

	void PushRoom(int roomnum, NewRenderWindow& window)
	{
		RoomCheckList.emplace(roomnum, window);
	}

	RenderListEntry PopRoom()
	{
		RenderListEntry check = RoomCheckList.front();
		RoomCheckList.pop();

		return check;
	}

	bool CheckFace(room& rp, face& fp, Frustum& frustum) const;
	//Projects face fp and creates a window encompassing it. 
	//If the window crosses nearclip, the window can't be reliably calculated, so instead the parent will be used. 
	NewRenderWindow GetWindowForFace(room& rp, face& fp, NewRenderWindow& parent) const;
	void MaybeUpdateFogPortal(int roomnum, face& fp);
	//Adds a room to the visible list. Will check visibility of all portal faces,
	//and add all visibile connected rooms to the room check queue. 
	void AddRoom(RenderListEntry& entry, Frustum& frustum);

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
