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

#ifndef __INGAME_CINEMATICS_H_
#define __INGAME_CINEMATICS_H_

#include "vecmat_external.h"
#include "gamecinematics_external.h"
#include "hud.h"

//	Cinematic_Init
//
//	Initializes the in-game cinematics
void Cinematic_Init(void);

//	Cinematic_Close
//
//	Closes the in-game cinematics
void Cinematic_Close(void);

//	Cinematic_Start
//
//	Starts an in-game cinematic sequence.  text_string is the text to be displayed
//	use pipes (|) to seperate lines.
bool Cinematic_Start(tGameCinematic *info,char *text_string);

//	Cinematic_Stop
//
//	Stops and clears up a in-game cinematic.
void Cinematic_Stop(void);

//	Cinematic_Frame
//
//	Processes a frame for the Cinematics
void Cinematic_Frame(void);

// Renders anything that needs to be rendered for the cinematic frame
void Cinematic_RenderFrame(void);

void Cinematic_LevelInit(void);

extern bool Cinematic_inuse;
extern bool Cinematic_fake_queued;

//	Returns the hud mode before cinematics
tHUDMode Cinematic_GetOldHudMode(void);

// Starts a canned cinematic sequence
// Only the demo system passing in a camera_handle, so it should never be explicitly passed by you
void Cinematic_StartCanned(tCannedCinematicInfo *info,int camera_handle=-1);


void Cinematic_DoDemoFileData(ubyte *data);

#endif
