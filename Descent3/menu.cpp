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

#include "menu.h"
#include "mmItem.h"
#include "game.h"
#include "gamesequence.h"
#include "Mission.h"
#include "multi_ui.h"
#include "ctlconfig.h"
#include "config.h"
#include "gamesave.h"
#include "demofile.h"
#include "pilot.h"
#include "LoadLevel.h"
#include "stringtable.h"
#include "mem.h"
#include "args.h"
#include "cinematics.h"

#ifdef _WIN32
#define USE_DIRECTPLAY
#endif

#ifdef USE_DIRECTPLAY
#include "directplay.h"
#else
bool Directplay_lobby_launched_game = false;
#endif
#include "multi_dll_mgr.h"
#include "d3music.h"
#include "newui_core.h"
#include <string.h>
#include <thread>
#include <future>
#define IDV_QUIT				0xff
//	Menu Item Defines
#define IDV_NEWGAME			10
#define IDV_MULTIPLAYER		11
#define IDV_OPTIONS			12
#define IDV_PILOT				13
#define IDV_LOADGAME			14
#define IDV_PLAYDEMO			15
#define IDV_CREDITS			16
#ifdef _DEBUG
#define IDV_LOADLEVEL		20
#define IDV_OK					1
#define IDV_CANCEL			2
bool MenuLoadLevel(void);
#endif
// for command line joining of games
bool Auto_connected = false;
// externed from init.cpp
extern void SaveGameSettings();
//	runs command line options.
bool ProcessCommandLine();
// new game selection
bool MenuNewGame();
extern bool Mem_quick_exit;
bool IsRestoredGame = false;
//////////////////////////////////////////////////////////////////////////////
extern bool IsCheater;
extern bool Demo_looping;
extern bool Game_gauge_do_time_test;
extern char Game_gauge_usefile[_MAX_PATH];
bool FirstGame = false;

int MainMenu()
{
	extern void ShowStaticScreen(char* bitmap_filename, bool timed = false, float delay_time = 0.0f);
	mmInterface main_menu;
	bool exit_game = false;
	bool exit_menu = false;

#ifdef GAMEGAUGE
	if (1)
#else
	if (Game_gauge_do_time_test)
#endif
	{
		Mem_quick_exit = 1;
		return 1;
	}
	// okay over here, we'll decide whether we've finished the training mission and are going into game.
	if (!Demo_looping && !Demo_restart && !MultiDLLGameStarting)
	{
		if (FirstGame)
		{
			if (MenuNewGame())
				return 0;
		}
	}

	// setup screen
	SetScreenMode(SM_MENU);
	// create interface
	main_menu.Create();
	main_menu.AddItem(IDV_NEWGAME, KEY_N, TXT_MENUNEWGAME, MM_STARTMENU_TYPE);
	main_menu.AddItem(IDV_LOADGAME, KEY_L, TXT_LOADGAME);
	//#ifndef DEMO
	main_menu.AddItem(IDV_PLAYDEMO, KEY_D, TXT_VIEWDEMO);
	//#endif
	main_menu.AddItem(IDV_OPTIONS, KEY_O, TXT_MENUOPTIONS);
	main_menu.AddItem(IDV_PILOT, KEY_P, TXT_MENUPILOTS);
	main_menu.AddItem(IDV_MULTIPLAYER, KEY_M, TXT_MENUMULTIPLAYER);
	main_menu.AddItem(IDV_CREDITS, KEY_C, TXT_MENUCREDITS);
	main_menu.AddItem(IDV_QUIT, KEY_Q, TXT_MENUQUIT, MM_ENDMENU_TYPE);
#ifdef _DEBUG
	main_menu.AddItem(0, 0, NULL);
	main_menu.AddItem(IDV_LOADLEVEL, 0, "Load Level");
#endif
	//	page in ui data.
	newuiCore_PageInBitmaps();
	// do special junk.

		//Only check for pilot arg first time, not every tme
	static bool first_time = true;
	if (first_time)
	{
		int pilotarg = FindArg("+name");
		if (!pilotarg)
			pilotarg = FindArg("-pilot");

		if (pilotarg)
		{
			char pfilename[_MAX_FNAME];
			strcpy(pfilename, GameArgs[pilotarg + 1]);
			strcat(pfilename, ".plt");
			Current_pilot.set_filename(pfilename);
			PltReadFile(&Current_pilot, true);
		}
		first_time = false;
	}
	char pfilename[_MAX_FNAME];
	Current_pilot.get_filename(pfilename);

	if ((pfilename[0] == '\0') || (strlen(pfilename) == 0) || (!strcmp(pfilename, " ")))
		PilotSelect();

	//	always enforce that in main menu we are in normal game mode.
	SetGameMode(GM_NONE);
	// open main menu
	main_menu.Open();
	exit_menu = ProcessCommandLine();
	// Main Menu Code Here
	while (!exit_menu)
	{
		int res;
		// handle all UI results.
		if ((Demo_looping) || (Demo_restart))
		{
			Demo_restart = false;
			SetGameMode(GM_NORMAL);
			SetFunctionMode(LOADDEMO_MODE);
			exit_menu = 1;
			continue;
		}
		else if (MultiDLLGameStarting)
		{
			//Go back into the multiplayer DLL @ the game list

			mprintf((0, "Returning to Multiplayer!\n"));

			if (ReturnMultiplayerGameMenu())
			{
				exit_menu = 1;
				SetFunctionMode(GAME_MODE);
				continue;
			}
		}
		/*
			else
			{
				if(FirstGame)
				{
					//MenuScene();
					//ui_ShowCursor();
					//Descent->defer();
					//DoUIFrame();
					//rend_Flip();
					//GetUIFrameResult();
					res = IDV_NEWGAME;
				}
				else {
					main_menu.SetMusicRegion(MM_MUSIC_REGION);
					res = main_menu.DoUI();
				}
			}
			if (FirstGame)
				res = IDV_NEWGAME;
		*/
		res = FirstGame ? IDV_NEWGAME : -1;
		if (res == -1)
		{
			main_menu.SetMusicRegion(MM_MUSIC_REGION);
			res = main_menu.DoUI();
		}
		switch (res)
		{
		case IDV_NEWGAME:
			main_menu.SetMusicRegion(NEWGAME_MUSIC_REGION);
			DoWaitMessage(true);
			IsCheater = false;
			IsRestoredGame = false;
			//make only the default ships available (we may need to move this depending on load a saved game)
			PlayerResetShipPermissions(-1, true);
			if (MenuNewGame())
			{
				exit_menu = 1;
				MenuScene();
				rend_Flip();
			}
			break;
		case IDV_QUIT:
			if (DoMessageBox(TXT_MENUQUIT, TXT_QUITMESSAGE, MSGBOX_YESNO))
			{
				exit_game = 1;
				exit_menu = 1;
				Mem_quick_exit = 1;	//tell the mem library to not free up each chunk individually
			}
			break;
		case IDV_LOADGAME:
			SetGameMode(GM_NONE);
			if (LoadGameDialog()) {
				SetGameMode(GM_NORMAL);
				SetFunctionMode(RESTORE_GAME_MODE);
				exit_menu = 1;
			}
			break;
		case IDV_OPTIONS:
			main_menu.SetMusicRegion(OPTIONS_MUSIC_REGION);
			OptionsMenu();
			break;
		case IDV_PILOT:
			main_menu.SetMusicRegion(OPTIONS_MUSIC_REGION);
			DoWaitMessage(true);
			PilotSelect();
			break;
		case IDV_MULTIPLAYER:
		{
			IsCheater = false;
			main_menu.SetMusicRegion(MULTI_MUSIC_REGION);
			mprintf((0, "Multiplayer!\n"));
			//make all ships available
			mprintf((0, "Making all ships available\n"));
			for (int i = 0; i < MAX_SHIPS; i++)
			{
				if (Ships[i].used)
					PlayerSetShipPermission(-1, Ships[i].name, true);
			}
			if (MainMultiplayerMenu())
			{
				exit_menu = 1;
				SetFunctionMode(GAME_MODE);
			}
		}
		break;
		case IDV_PLAYDEMO:
			if (LoadDemoDialog())
			{
				SetGameMode(GM_NORMAL);
				SetFunctionMode(LOADDEMO_MODE);
				exit_menu = 1;
			}
			break;
		case IDV_CREDITS:
			SetFunctionMode(CREDITS_MODE);
			exit_menu = 1;
			break;
#ifdef _DEBUG
		case IDV_LOADLEVEL:
		{
			if (MenuLoadLevel())
			{
				ShowProgressScreen(TXT_LOADINGLEVEL);
				exit_menu = 1;
			}
		}
		break;
#endif
		}
	}
	// close menu
	main_menu.Close();
	main_menu.Destroy();
	//	page in ui data.
	newuiCore_ReleaseBitmaps();
	return exit_game ? 1 : 0;
}

//	runs command line options.
bool ProcessCommandLine()
{
	int exit_menu = 0;
	// Auto connect to a network game if the parm is there.
	if ((!Auto_connected) && (TCP_active) && (FindArg("-url")))
	{
		Auto_connected = true;
		int urlarg = FindArg("-url") + 1;
		char szurl[200];
		char* p;
		char* tokp;
		strcpy(szurl, GameArgs[urlarg]);
#ifdef DEMO
		szurl[strlen("d3demo2://") - 1] = NULL; //Should make the string "d3demo:/"
		p = szurl + strlen("d3demo2://");  //pointer to the first character of the url after the //
		if (strcmpi(szurl, "d3demo2:/") == 0)
		{
			mprintf((0, "Got a url passed: %s\n", p));
		}
#else
		szurl[strlen("descent3://") - 1] = NULL; //Should make the string "descent3:/"
		p = szurl + strlen("descent3://");  //pointer to the first character of the url after the //
		if (strcmpi(szurl, "descent3:/") == 0)
		{
			mprintf((0, "Got a url passed: %s\n", p));
		}
#endif
		tokp = strtok(p, "/");
		if (strcmpi(tokp, "ip") == 0)
		{
			tokp = strtok(NULL, "/");
			Auto_login_port[0] = NULL;
			strcpy(Auto_login_addr, tokp);
			//			char seldll[_MAX_PATH*2];
						//ddio_MakePath(seldll,Base_directory,"online","Direct TCP~IP Game.d3c",NULL);
			if (LoadMultiDLL("Direct TCP~IP"))
			{
				CallMultiDLL(MT_AUTO_LOGIN);
				if (MultiDLLGameStarting)
				{
					mprintf((0, "Successfully connected to server at specified url.\n"));
					exit_menu = 1;
				}
				else
				{
					mprintf((0, "Couldn't connect to server at specified url.\n"));
				}
			}
			else
			{
				mprintf((0, "Couldn't load DLL.\n"));
			}
		}
		else if (strcmpi(tokp, "pxo") == 0)
		{
			tokp = strtok(NULL, "/");
			Auto_login_port[0] = NULL;
			strcpy(Auto_login_addr, tokp);
			//		char seldll[_MAX_PATH*2];
					//ddio_MakePath(seldll,Base_directory,"online","parallax online.d3c",NULL);
			if (LoadMultiDLL("parallax online"))
			{
				CallMultiDLL(MT_AUTO_LOGIN);
				if (MultiDLLGameStarting)
				{
					mprintf((0, "Successfully connected to server at specified url.\n"));
					exit_menu = 1;
				}
				else
				{
					mprintf((0, "Couldn't connect to server at specified url.\n"));
				}
			}
			else
			{
				mprintf((0, "Couldn't load DLL.\n"));
			}
		}
	}
	else if ((!Auto_connected) && (FindArg("-pxo")))
	{
		Auto_connected = true;
		if (AutoConnectPXO())
			exit_menu = 1;
	}
	else if ((!Auto_connected) && (FindArg("-directip")))
	{
		Auto_connected = true;
		if (AutoConnectLANIP())
			exit_menu = 1;
	}
#ifndef OEM
	else if ((!Auto_connected) && (FindArg("+connect")))
	{
		Auto_connected = true;
		int connarg = FindArg("+connect");
		char connhost[300];
		strcpy(connhost, GameArgs[connarg + 1]);
		char* port = strchr(connhost, ':');
		if (port)
		{
			//terminate the hostname
			*port = NULL;
			//Increment to the first character of the port name
			port++;
			//get the port number
			strcpy(Auto_login_port, port);
		}
		strcpy(Auto_login_addr, connhost);
		int trackedarg = FindArg("+cl_pxotrack");
		if (trackedarg)
		{
			if (strcmp("1", GameArgs[trackedarg + 1]) == 0)
			{
				if (AutoConnectPXO())
					exit_menu = 1;
			}
			else if (AutoConnectLANIP())
			{
				exit_menu = 1;
			}
		}
	}
#endif
	else if ((!Auto_connected) && (Directplay_lobby_launched_game))
	{
		Auto_connected = true;
#ifdef USE_DIRECTPLAY
		if (dp_AutoConnect())
		{
			exit_menu = 1;
		}
#else
		exit_menu = 1;
#endif
	}
#ifndef RELEASE
	int t = FindArg("-loadlevel");
	if (t)
	{
		SimpleStartLevel(GameArgs[t + 1]);
		exit_menu = 1;
	}
#endif
	// at some point the code above sets exit_menu, so we're going to game mode.
	if (exit_menu)
		SetFunctionMode(GAME_MODE);

	SetUICallback(DEFAULT_UICALLBACK);
	return exit_menu ? true : false;
}
//////////////////////////////////////////////////////////////////////////////
//	Start New Game
#define MSNDLG_WIDTH	512
#define MSNDLG_HEIGHT 384
#define MSNDLG_X (Max_window_w - MSNDLG_WIDTH)/2
#define MSNDLG_Y (Max_window_h - MSNDLG_HEIGHT)/2
#define MSNLB_WIDTH (MSNDLG_WIDTH-64)
#define MSNLB_HEIGHT (MSNDLG_HEIGHT-160)
#define MSNLB_X (MSNDLG_WIDTH-MSNLB_WIDTH)/2
#define MSNLB_Y (MSNDLG_HEIGHT-MSNLB_HEIGHT)/2
#define MSNBTN_W	96
#define MSNBTN_X	((MSNDLG_WIDTH/4) - (MSNBTN_W/2))
#define MSNBTN_X2	((3*MSNDLG_WIDTH/4) - (MSNBTN_W/2))
#define MSNBTN_Y	(MSNDLG_HEIGHT - 64)
#define UID_MSNLB	100
#define UID_MSNINFO		0x1000
#define TRAINING_MISSION_NAME		"Pilot Training"

static int count_missions_worker(const char* pathname, const char* wildcard)
{
	int c = 0;
	char fullpath[_MAX_PATH];
	char filename[PSPATHNAME_LEN];
	tMissionInfo msninfo;
	filename[0] = 0;
	ddio_MakePath(fullpath, pathname, wildcard, NULL);

	if (ddio_FindFileStart(fullpath, filename))
	{
		do
		{
			char* name;
			ddio_MakePath(fullpath, pathname, filename, NULL);

			if (strcmpi("d3_2.mn3", filename) == 0)
				continue;
			//mprintf((0, "Mission path:%s\n", fullpath)); //don't know the thread safetiness of mprintf atm
			name = GetMissionName(filename);
			GetMissionInfo(filename, &msninfo);
			if (name && name[0] && msninfo.single)
			{
				//mprintf((0, "Name:%s\n", name));
				c++;
			}
			else
			{
				//mprintf((0, "Illegal mission:%s\n", fullpath));
			}
			filename[0] = 0;
		} while (ddio_FindNextFile(filename));
		ddio_FindFileClose();
	}
	return c;
}

inline int count_missions(const char* pathname, const char* wildcard)
{
	std::future<int> c = std::async(std::launch::async, count_missions_worker, pathname, wildcard);

	for (;;)
	{
		if (c.wait_for(std::chrono::milliseconds(50)) == std::future_status::ready)
			break;

		DoWaitMessage(true);
	}

	return c.get();
}

static int generate_mission_listbox_worker(newuiListBox* lb, int n_maxfiles, char** filelist, const char* pathname, const char* wildcard)
{
	int c = 0;
	char fullpath[_MAX_PATH];
	char filename[PSPATHNAME_LEN];
	ddio_MakePath(fullpath, pathname, wildcard, NULL);

	if (ddio_FindFileStart(fullpath, filename))
	{
		do
		{
			tMissionInfo msninfo;
			if (n_maxfiles > c)
			{
				ddio_MakePath(fullpath, pathname, filename, NULL);
				if (strcmpi("d3_2.mn3", filename) == 0)
					continue;
				if (GetMissionInfo(filename, &msninfo) && msninfo.name[0] && msninfo.single)
				{
					filelist[c] = mem_strdup(filename);
					lb->AddItem(msninfo.name);
					filename[0] = 0;
					c++;
				}
			}
		} while (ddio_FindNextFile(filename));
		ddio_FindFileClose();
	}

	return c;
}

inline int generate_mission_listbox(newuiListBox* lb, int n_maxfiles, char** filelist, const char* pathname, const char* wildcard)
{
	std::future<int> c = std::async(std::launch::async, generate_mission_listbox_worker, lb, n_maxfiles, filelist, pathname, wildcard);

	for (;;)
	{
		if (c.wait_for(std::chrono::milliseconds(50)) == std::future_status::ready)
			break;

		DoWaitMessage(true);
	}

	return c.get();
}
extern bool Skip_next_movie;
#define OEM_TRAINING_FILE	"training.mn3"
#define OEM_MISSION_FILE	"d3oem.mn3"
bool MenuNewGame()
{
	newuiTiledWindow menu;
	newuiSheet* select_sheet;
	newuiListBox* msn_lb;
	char** filelist = NULL;
	int n_missions, i, res;	//,k
	bool found = false;
	bool do_menu = true, load_mission = false, retval = true;
#ifdef DEMO
	if (LoadMission("d3demo.mn3"))
	{
		CurrentPilotUpdateMissionStatus(true);
		// go into game mode.
		SetGameMode(GM_NORMAL);
		SetFunctionMode(GAME_MODE);
		return true;
	}
	else
	{
		DoMessageBox(TXT_ERROR, TXT_ERRLOADMSN, MSGBOX_OK);
		return false;
	}
#else
#ifdef RELEASE
	if ((!FindArg("-mission")) && (!FirstGame) && (-1 == Current_pilot.find_mission_data(TRAINING_MISSION_NAME)))
	{

		FirstGame = true;

		char temppath[PSFILENAME_LEN * 2];
		char* moviepath;
		moviepath = GetMultiCDPath("level1.mve");
		if (moviepath)
		{
			strcpy(temppath, moviepath);
			PlayMovie(temppath);
		}
		Skip_next_movie = true;

		if (LoadMission("training.mn3"))
		{
			CurrentPilotUpdateMissionStatus(true);
			// go into game mode.
			SetGameMode(GM_NORMAL);
			SetFunctionMode(GAME_MODE);
			return true;
		}
		else
		{
			DoMessageBox(TXT_ERROR, TXT_ERRLOADMSN, MSGBOX_OK);
			return false;
		}
	}
	else if (FirstGame)
	{
		FirstGame = false;
#ifdef OEM
		if (LoadMission(OEM_MISSION_FILE))
#else
		if (LoadMission("d3.mn3"))
#endif
		{
			CurrentPilotUpdateMissionStatus(true);
			// go into game mode.
			SetGameMode(GM_NORMAL);
			SetFunctionMode(GAME_MODE);
			return true;
		}
		else
		{
			DoMessageBox(TXT_ERROR, TXT_ERRLOADMSN, MSGBOX_OK);
			return false;
		}
	}
#endif
	// create menu.
	menu.Create(TXT_MENUNEWGAME, 0, 0, 448, 384);

	select_sheet = menu.GetSheet();
	select_sheet->NewGroup(NULL, 10, 0);
	msn_lb = select_sheet->AddListBox(352, 256, UID_MSNLB);
	select_sheet->NewGroup(NULL, 160, 280, NEWUI_ALIGN_HORIZ);
	select_sheet->AddButton(TXT_OK, UID_OK);
	select_sheet->AddButton(TXT_CANCEL, UID_CANCEL);
#ifndef OEM
	select_sheet->AddButton(TXT_MSNINFO, UID_MSNINFO);
#endif
#ifndef OEM
	// add mission names to listbox
	// count valid mission files.
	// add a please wait dialog here.
		//[ISB] Wait, isn't this a massice race condition?
	n_missions = 0;
#ifndef RELEASE
	n_missions = count_missions(LocalLevelsDir, "*.msn");
#endif
	n_missions += count_missions(D3MissionsDir, "*.mn3");

	if (n_missions)
	{
		// allocate extra mission slot because of check below which adds a name to the filelist.
		filelist = (char**)mem_malloc(sizeof(char*) * (n_missions + 1));
		for (i = 0; i < (n_missions + 1); i++)
			filelist[i] = NULL;
	}
	else
	{
		DoMessageBox(TXT_ERROR, TXT_NOMISSIONS, MSGBOX_OK);
		retval = false;
		DoWaitMessage(false);
		goto missions_fail;
	}
	// generate real listbox now.
	i = 0;
#ifndef RELEASE
	i = generate_mission_listbox(msn_lb, n_missions, filelist, LocalLevelsDir, "*.msn");
#endif
	i += generate_mission_listbox(msn_lb, n_missions - i, filelist + i, D3MissionsDir, "*.mn3");
	//#ifdef RELEASE
	int k;
	for (k = 0; k < n_missions; k++)
	{
		if (!filelist[k])
			continue;
		if (strcmpi(filelist[k], "d3.mn3") == 0)
		{
			found = true;
			break;
		}

	}
	if (!found)
	{
		filelist[n_missions] = mem_strdup("d3.mn3");
		msn_lb->AddItem(TXT_MAINMISSION);
		n_missions++;
	}
	//#endif 
#else
#define OEM_MISSION_NAME	"Descent 3: Sol Ascent"
#define OEM_TRAINING_NAME	"Pilot Training "
	n_missions = 2;
	filelist = (char**)mem_malloc(sizeof(char*) * 2);
	filelist[0] = mem_strdup(OEM_MISSION_FILE);;
	msn_lb->AddItem(OEM_MISSION_NAME);
	filelist[1] = mem_strdup(OEM_TRAINING_FILE);
	msn_lb->AddItem(OEM_TRAINING_NAME);
#endif
	DoWaitMessage(false);
redo_newgame_menu:
	// run menu
	menu.Open();

	do
	{
		res = menu.DoUI();
#ifndef OEM
		if (res == UID_MSNINFO)
		{
			tMissionInfo msninfo;
			int index = msn_lb->GetCurrentIndex();
			if (index >= 0 && index < n_missions)
			{
				if (GetMissionInfo(filelist[index], &msninfo))
				{
					if (msninfo.name[0])
					{
						newuiTiledWindow infownd;
						newuiSheet* sheet;
						infownd.Create(NULL, 0, 0, 384, 192);
						infownd.Open();
						sheet = infownd.GetSheet();
						sheet->NewGroup(TXT_MSNNAME, 0, 0);
						sheet->AddText(msninfo.name);
						sheet->NewGroup(TXT_MSNAUTHOR, 0, 32);
						if (msninfo.author[0]) 	sheet->AddText(msninfo.author);
						else sheet->AddText(TXT_NONE);
						sheet->NewGroup(TXT_MSNNOTES, 0, 64);
						if (msninfo.desc[0]) sheet->AddText(msninfo.desc);
						else sheet->AddText(TXT_NONE);
						sheet->NewGroup(NULL, 240, 118);
						sheet->AddButton(TXT_OK, UID_OK);
						infownd.DoUI();
						infownd.Close();
						infownd.Destroy();
					}
				}
			}
		}
#endif
	} while (res != UID_OK && res != UID_CANCEL && res != UID_MSNLB);
	menu.Close();
	// check stuff
	if (res == UID_CANCEL)
		retval = false;

	else if (res == UID_OK || res == UID_MSNLB)
	{
		int index = msn_lb->GetCurrentIndex();
		char* nameptr = NULL;
		if (index >= 0 && index < n_missions)
			nameptr = filelist[index];

#ifndef OEM
		if (!nameptr || !LoadMission(nameptr))
		{
#else 
		if (!LoadMission(nameptr))
		{
#endif
			DoMessageBox(TXT_ERROR, TXT_ERRLOADMSN, MSGBOX_OK);
			retval = false;
		}
		else
		{
			//	if we didn't escape out of any part of new game start, then go to game.
			int highest;
			CurrentPilotUpdateMissionStatus(true);
			// gets highest level flown for mission
#if defined(_DEBUG) || defined(DAJ_DEBUG)
			highest = Current_mission.num_levels;
#else
			highest = PilotGetHighestLevelAchieved(&Current_pilot, Current_mission.name);
			highest = min(highest + 1, Current_mission.num_levels);
#endif		
			if (highest > 1)
			{
				int start_level;
				start_level = DisplayLevelWarpDlg(highest);
				if (start_level == -1)
					goto redo_newgame_menu;

				else
				{
					Current_mission.cur_level = start_level;
					//pull out the ship permssions and use them
					Players[0].ship_permissions = GetPilotShipPermissions(&Current_pilot, Current_mission.name);
				}
			}
			// go into game mode.
			SetGameMode(GM_NORMAL);
			SetFunctionMode(GAME_MODE);
			retval = true;
		}
	}
	menu.Destroy();
missions_fail:
	// free all memory
	for (i = 0; i < n_missions; i++)
	{
		if (filelist[i])
			mem_free(filelist[i]);

	}
	if (filelist)
		mem_free(filelist);

	return retval;
#endif
}
// DisplayLevelWarpDlg
//	pass in the max level allowed to be chosen, if -1, than all levels are allowed (a.k.a level warp cheat)
int DisplayLevelWarpDlg(int max_level)
{
	newuiMessageBox hwnd;
	newuiSheet* sheet;
	int chosen_level = 1, res;
	int highest_allowed;
	char buffer[128];
	char* input_text;
	// creates a sheet
	sheet = hwnd.GetSheet();
	if (max_level != -1)
	{
		hwnd.Create(TXT_LEVELSELECT, MSGBOX_OKCANCEL);
		highest_allowed = max_level;
		sprintf(buffer, TXT_LEVELSELECTB, highest_allowed);
	}
	else
	{
		//level warp
		hwnd.Create(TXT_LEVELWARP, MSGBOX_OKCANCEL);
		highest_allowed = Current_mission.num_levels;
		sprintf(buffer, TXT_LEVELWARPB, Current_mission.num_levels);
	}
	sheet->NewGroup(buffer, 0, 0);
	input_text = sheet->AddEditBox(NULL, 4, 64, IDV_QUIT, UIED_NUMBERS);
	itoa(chosen_level, input_text, 10);
redo_level_choose:
	hwnd.Open();
	res = hwnd.DoUI();
	hwnd.Close();
	if (res == UID_OK || res == IDV_QUIT)
	{
		chosen_level = atoi(input_text);
		if (chosen_level<1 || chosen_level>highest_allowed)
		{
			sprintf(buffer, TXT_CHOOSELEVEL, highest_allowed);
			DoMessageBox(TXT_ERROR, buffer, MSGBOX_OK);
			goto redo_level_choose;
		}
	}
	else
	{
		chosen_level = -1;
	}
	hwnd.Destroy();
	return chosen_level;
}
#ifdef _DEBUG
//Loads a level and starts the game
bool MenuLoadLevel(void)
{
	char buffer[_MAX_PATH];
	buffer[0] = '\0';

	if (DoPathFileDialog(false, buffer, "Load Level", "*.d3l", PFDF_FILEMUSTEXIST)) {
		SimpleStartLevel(buffer);
		SetFunctionMode(GAME_MODE);
		return true;
	}
	return false;
}
#endif
