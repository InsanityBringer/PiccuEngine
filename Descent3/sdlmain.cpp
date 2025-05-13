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

#include <SDL3/SDL_main.h>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "mono.h"
#include "descent.h"
#include "game.h"
#include "texture.h"
#include "application.h"
#include "appdatabase.h"
#include "pserror.h"
#include "args.h"
#include "init.h"
#include "dedicated_server.h"
#include "resource.h"
#include "config.h"
#include "ddio.h"

int no_debug_dialog; //Needed to make win32 error.cpp happy. 

class oeSDLGameDatabase : public oeSDLAppDatabase
{
public:
	oeSDLGameDatabase();
};



//	---------------------------------------------------------------------------
//	D3WinDatabase operating system specific initialization

oeSDLGameDatabase::oeSDLGameDatabase() :
	oeSDLAppDatabase()
{
	//	create descent III entry if it doesn't exit.

	std::string basepath;

#if defined(EDITOR)
	basepath = "\\D3Edit";
#elif defined(DEMO)
	basepath = "\\Descent3Demo2";
#elif defined(OEM_V3)
	basepath = "\\Descent3_OEM_V3";
#elif defined(OEM_AUREAL2)
	basepath = "\\Descent3_OEM_A2";
#elif defined(OEM_KATMAI)
	basepath = "\\Descent3_OEM_KM";
#elif defined(GAMEGAUGE)
	basepath = "\\Descent3GG";
#elif defined(OEM)
	basepath = "\\Descent3_OEM";
#else
	basepath = "\\Descent3";
#endif


	bool res = lookup_record(basepath.c_str());
	if (!res) {
		res = create_record(basepath.c_str());
		if (!res) {
			Error("Failed to create registry key for %s", PRODUCT_NAME);
		}
	}

	// create version key.
	std::string path = basepath + "\\Version";
	res = lookup_record(path.c_str());
	if (!res) {
		res = create_record(path.c_str());
		if (!res) {
			Error("Failed to create registry key for %s", PRODUCT_NAME);
		}
	}

#ifdef EDITOR		//Maybe this code should be in the editor startup
	lstrcpy(path, m_Basepath);
	lstrcat(path, "\\editor");
	res = lookup_record(path);
	if (!res)
	{
		res = create_record(path);
		if (!res)
		{
			Error("Failed to create registry key for %s.", PRODUCT_NAME);
		}
	}
#endif

	res = lookup_record(basepath.c_str());

	//Get net directory for manage system
	std::string netpath = "";
#ifndef EDITOR
	if (FindArg("-update"))	//For the game-only build, require -update to update data
#endif
		//NOTE LINK TO ABOVE IF
	{
		char* netdir = getenv("D3_DIR");
		if (netdir)
			netpath = netdir;
	}
	write("net directory", netpath.c_str(), netpath.size() + 1);

	Database = this;
}

void D3End()
{
	if (Descent)
	{
		delete Descent;
	}
}

static int m_resource_language = 0;
int SharedMain(int argc, char** argv)
{
	SDLApplication* d3;
	GatherArgs(argv);

	oeSDLGameDatabase dbase;

	// If this is a dedicated server, then start one!
	if (FindArg("-dedicated"))
		StartDedicatedServer();
#ifdef DEDICATED_ONLY
	else
	{
		MessageBox(NULL, "Error: -dedicated command line required", PRODUCT_NAME " Error", MB_OK);
		return 0;
	}
#endif

	if (Dedicated_server)
	{
		d3 = new SDLApplication(ENGINE_NAME, OEAPP_CONSOLE);
	}
	else
	{
		int temp;
		Database->read_int("RS_fullscreen", &temp);
		Game_fullscreen = !!temp;

		if (FindArg("-windowed"))
			Game_fullscreen = false;

		unsigned int flags = OEAPP_FULLSCREEN;
		if (!Game_fullscreen)
		{
			// switch to windowed mode instead
			flags = OEAPP_WINDOWED;
		}

		SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK);

		d3 = new SDLApplication(ENGINE_NAME, flags);
	}
	atexit(D3End);


	// determine preinit language for resource strings
	int language = 0;
	dbase.read_int("LanguageType", &language);
	m_resource_language = language;

	PreInitD3Systems();

	Descent = d3;
	d3->init();
	//d3->run();
	Descent3();

	return 1;
}

#ifdef WIN32
#include <Windows.h>
//This is the winmain that tests for exceptions..
int main(int argc, char** argv)
{
	int result = -1;

	__try
	{
		result = SharedMain(argc, argv);
	}
	__except (RecordExceptionInfo(GetExceptionInformation(), "main()"))
	{

	}
	return result;
}
#else
int main(int argc, char** argv)
{
	return SharedMain(argc, argv);
}
#endif
