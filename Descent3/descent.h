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

#ifndef _DESCENT_H
#define _DESCENT_H

#include <stdlib.h>
#include "application.h"
//[ISB] Version information
#include "gitinfo.h"

//The name of this product
#define PRODUCT_NAME "Descent 3"
//[ISB] Branding for the engine
#define ENGINE_NAME "Piccu Engine"

//This is the port number assigned to "descent3" by the IANA (Internet Assigned Numbers Authority)
//Don't arbitrarily change this number!
#ifdef OEM
//The port used for the OEM version isn't a legitimate IANA port number
#define D3_DEFAULT_PORT	2092
#else
#define D3_DEFAULT_PORT	2092
#endif

class oeApplication;
class oeAppDatabase;

//	---------------------------------------------------------------------------
//	Constants and Types
					   
enum function_mode 
{ 
	INIT_MODE, GAME_MODE, RESTORE_GAME_MODE, EDITOR_MODE, EDITOR_GAME_MODE, MENU_MODE,QUIT_MODE,LOADDEMO_MODE, GAMEGAUGE_MODE, CREDITS_MODE
};

extern bool Descent_overrided_intro;

//Maximum samples to save for gamegauge graph info
#define GAMEGAUGE_MAX_LOG	1000

// This is the default FOV
#define D3_DEFAULT_FOV	72.0
//This is the default zoom factor to be used for the game 3D view.
#define D3_DEFAULT_ZOOM	0.726f

//How long the a mission name can be
#define MSN_NAMELEN 32

// The "root" directory of the D3 file tree
extern char Base_directory[];
//[ISB] User directory for pilots, demos, savegames, and other user generated files.
//May equal Base_directory when in portable mode. 
extern char* User_directory;

//	---------------------------------------------------------------------------
//	Globals

extern oeApplication *Descent;		// The Descent object
extern oeAppDatabase *Database;		// The Database
extern char Descent3_temp_directory[_MAX_PATH];	//temp directory to put temp files
extern bool Portable;		// [ISB] True if the paths should be portable. 
extern bool Katmai;			// whether or not katmai is detected
//	---------------------------------------------------------------------------
//	Functions

#ifdef EDITOR
void WinMainInitEditor(unsigned hwnd, unsigned hinst);
#endif

//	Runs Descent III
void Descent3();

//	Runs the game, or editor
void MainLoop();

//	the defer handler
void D3DeferHandler(bool is_active);

//	Set and retrieve the current function mode of Descent 3
void SetFunctionMode(function_mode mode);
function_mode GetFunctionMode();

//This function figures out whether or not a file needs to be loaded off of
//CD or off of the local drive. If it needs to come from a CD, it figures out
//which CD and prompts the user to enter that CD. If they hit cancel, it 
//returns NULL.
char * GetMultiCDPath(char *file);
char * GetCDVolume(int cd_num);

inline void DELAY(float secs) 
{
	Descent->delay(secs);
}

#ifndef RELEASE
//	this is called when you hit a debug break!
void D3DebugStopHandler();
void D3DebugResumeHandler();
#endif


#endif
