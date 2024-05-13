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

#pragma once

#include "descent.h"

//Values for the small view windows
#define SVW_LEFT		0
#define SVW_CENTER		1
#define SVW_RIGHT		2

//Small view flags
#define SVF_POPUP		1		//This is a temporary window
#define SVF_BIGGER		2		//This window is drawn a little bigger than the normal window
#define SVF_REARVIEW	4		//Draw looking backward from the viewer
#define SVF_TIMED		8		//This window is timer-based. DO NOT USE THIS FLAG WHEN CALLING CreateSmallView().
#define SVF_CROSSHAIRS	16		//This window has crosshairs
#define SVF_STATIC		32		//Window is showing static.  The object handle is unused

// if guided missile smallview is up, this will be true.
extern bool Guided_missile_smallview;

//Create a new small view.  If there is already a view in the given window, the old view gets blown away.
//Parameters:	window - which window to open.  See constants in SmallViews.h
//					objhandle - handle for the object to view from
//					flags - various view attributes.  See defines in header file.
//					time - how long to keep the window up.  If 0, keep up indefinitely
//					zoom - the zoom for this window.  If 0, use the default zoom
//					gun_num - which gun to view from. if -1, use viewer's center and orientation.
//					label - the label for the window
//Returns:		which window was opened, or -1 if window couldn't be created
int CreateSmallView(int window,int objhandle,int flags=0,float time=0.0,float zoom=D3_DEFAULT_ZOOM,int gun_num=-1,char *label=NULL);

//Called to get rid of all the small views & init system
void ResetSmallViews();

//Draw all the active small views
void DrawSmallViews();

//Returns the viewer object for the specified small view.  
//If the view isn't active, returns OBJECT_HANDLE_NONE.
int GetSmallViewer(int window);

//Get rid of a small view
//Parameters:	window - the window to get rid of
void CloseSmallView(int window);

//Get rid of a small view if it's a popup window
//Parameters:	window - the window to get rid of
void ClosePopupView(int window);
