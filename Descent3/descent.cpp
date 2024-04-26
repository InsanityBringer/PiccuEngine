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

#include <stdlib.h>

#include "pserror.h"
#include "grdefs.h"
#include "mono.h"
#include "CFILE.H"

#include "init.h"
#include "game.h"
#include "program.h"
#include "descent.h"
#include "menu.h"
#include "Mission.h"
#include "ddio.h"
#include "controls.h"
#include "Controller.h"
#include "gamesequence.h"
#include "stringtable.h"
#include "dedicated_server.h"
#include "networking.h"
#include "hlsoundlib.h"
#include "player.h"
#include "pilot.h"
#include "newui.h"
#include "credits.h"
#include "cinematics.h"
#include "args.h"
#include "multi_dll_mgr.h"
#include "localization.h"
#include "mem.h"
#if defined(MACINTOSH) && defined (GAMERANGER)
#include "GameRanger.h"
#endif
//	---------------------------------------------------------------------------
//	Variables
//	---------------------------------------------------------------------------
#ifdef EDITOR
static function_mode Function_mode = EDITOR_MODE;	// Game function mode
#else
static function_mode Function_mode = INIT_MODE;		// Game function mode
#endif
static function_mode Last_function_mode = INIT_MODE;

grScreen *Game_screen = NULL;			// The one and only video screen
oeApplication *Descent = NULL;		// The Main application
oeAppDatabase *Database = NULL;		// Application database.

bool Descent_overrided_intro = false;

extern bool Game_gauge_do_time_test;
bool Katmai=true;

char Descent3_temp_directory[_MAX_PATH];	//temp directory to put temp files
//	---------------------------------------------------------------------------
//	Descent3: Choke 1
//		Initializes game elements and invokes the MainLoop
//	---------------------------------------------------------------------------
//#define BETA

#if (defined(OEM) || defined(DEMO))
void ShowStaticScreen(char *bitmap_filename,bool timed=false,float delay_time=0.0f);
#endif

extern int CD_inserted;

char Proxy_server[200] = "";
short Proxy_port=80;

void Descent3()
{
	int type;

#ifdef _DEBUG
	type = DEVELOPMENT_VERSION;
#else 
	type = RELEASE_VERSION;
#endif
#ifdef BETA
	type |= BETA_VERSION;
#endif
#ifdef DEMO
	type |= DEMO_VERSION;
#endif

	ProgramVersion(type, D3_MAJORVER, D3_MINORVER, D3_BUILD);

	//Catch cfile errors
	try
	{
		//Init a bunch of stuff
		InitD3Systems1(false);

		int proxyarg = FindArg("-httpproxy");
		if(proxyarg)
		{
			strcpy(Proxy_server,GameArgs[proxyarg+1]);

			char *port = strchr(Proxy_server,':');
			if(port)
			{
				//terminate the hostname
				*port = 0;
				//Increment to the first character of the port name
				port++;					
				//get the port number
				Proxy_port = atoi(port);
			}

		}
		//Show intro & loading screens if not dedicated server
		if (!Dedicated_server)
		{
			SetScreenMode(SM_CINEMATIC);

		//Show the intro movie
			if (! FindArg("-nointro")) {
				char intropath[_MAX_PATH*2];
				bool remote_path =  (CD_inserted==1) ? true : false;

			#ifdef _DEBUG
				if (FindArg("-moviedir")) {
					remote_path = true;
				}
			#endif

		  		ddio_MakePath(intropath,Base_directory,"movies","dolby1.mv8",NULL);

				if(remote_path || (cfexist(intropath)))
				{
					char *t = GetMultiCDPath("dolby1.mv8");
					if (t)
						PlayMovie(t);
				}

				int intro_movie_arg;
				char base_intro_movie_name[256];
				char *intro_movie_name;
				
				intro_movie_arg = FindArg("-intro");
				if(intro_movie_arg>0)
				{
					// we have an override for the intro movie
					// we have to split path because the stupid args system of D3
					// capitalizes everything and the PlayMovie function expects
					// a lowercase extension
					char extension[16],*p;
					p = extension;
					ddio_SplitPath(GameArgs[intro_movie_arg+1],NULL,base_intro_movie_name,extension);
					while(*p) { *p = tolower(*p); p++;}
					strcat(base_intro_movie_name,extension);
					
					Descent_overrided_intro = true;

					intro_movie_name = base_intro_movie_name;
				}else
				{
					strcpy(base_intro_movie_name,"intro.mve");
					intro_movie_name = base_intro_movie_name;
				}

				ddio_MakePath(intropath,Base_directory,"movies",intro_movie_name,NULL);

				if(remote_path || (cfexist(intropath)))
				{
					char *t = GetMultiCDPath(intro_movie_name);
					if (t)
						PlayMovie(t);
				}
			}

			SetScreenMode(SM_MENU);
			//Show the intro screen
			IntroScreen();
		}

		//Init a bunch more stuff
		InitD3Systems2(false);

		#ifdef GAMEGAUGE
		if(0)
		#else
		if(!Game_gauge_do_time_test)
		#endif
		{
			SetFunctionMode(MENU_MODE);
		}
		else
		{
			SetFunctionMode(GAMEGAUGE_MODE);
		}

#if defined(MACINTOSH) && defined (GAMERANGER)
		if(GRCheckFileForCmd()) {
			SetFunctionMode (GAME_MODE);
			GRGetWaitingCmd();
		}
#endif
		MainLoop();

		//delete the lock file in the temp directory (as long as it belongs to us)
		ddio_DeleteLockFile(Descent3_temp_directory);

		//Save settings to registry
		SaveGameSettings();
	}
	catch(cfile_error *cfe) {
		Error(TXT_D3ERROR1,(cfe->read_write==CFE_READING)?TXT_READING:TXT_WRITING,cfe->file->name,cfe->msg);
	}
}

//	---------------------------------------------------------------------------
//	Main Loop: Choke 1 from editor Choke 2 from Game
//		If called from the editor, this is the best way to start up the game.
//		Otherwise this should be called from the main function.
//	---------------------------------------------------------------------------

//#ifdef GAMEGAUGE
extern int frames_one_second;
extern int min_one_second;
extern int max_one_second;
extern float gamegauge_start_time;
extern int gamegauge_total_frames;
extern float gamegauge_total_time;
extern short gamegauge_fpslog[GAMEGAUGE_MAX_LOG];

//#endif

void MainLoop()
{
	int exit_game = 0;

	while (!exit_game)
	{
		if (Dedicated_server && !(Function_mode==GAME_MODE || Function_mode==QUIT_MODE))
			SetFunctionMode (GAME_MODE);
					
		switch(Function_mode)
		{
			case QUIT_MODE:
				exit_game=1;
				break;
			case MENU_MODE:
				exit_game = MainMenu();
				break;
			case RESTORE_GAME_MODE:				// do special sequencing for load games.
				SetGameState(GAMESTATE_LOADGAME);
				PlayGame();
				break;

			case GAME_MODE:
				SetGameState(GAMESTATE_NEW);
				PlayGame();							// Does what is says.
				break;
			case LOADDEMO_MODE:
				SetGameState(GAMESTATE_LOADDEMO);
				PlayGame();
				break;
			case CREDITS_MODE:
				Credits_Display();
				Function_mode = MENU_MODE;
				break;
//#ifdef GAMEGAUGE
			case GAMEGAUGE_MODE:
				{
					int c;
					for(c=0;c<GAMEGAUGE_MAX_LOG;c++)
						gamegauge_fpslog[c] = 0;
					SetGameState(GAMESTATE_GAMEGAUGEDEMO);
					PlayGame();
					//exit_game = 1;
				}
				break;
//#endif
		#ifdef EDITOR
			case EDITOR_GAME_MODE:				// run level and then instead of menus, go to editor.
				QuickPlayGame();
				Function_mode = EDITOR_MODE;
				break;
			case EDITOR_MODE:
				SetScreenMode(SM_NULL);
				exit_game = 1;						// this MainLoop call should be issued from editor, so
														// this should just return to the editor
				break;
		#endif

			default:
				Int3();								// Bogus function mode
		}
	}
	//Clean up these items so we don't report them as leaked memory
	Sound_system.KillSoundLib(true);
	for(int a=0;a<MAX_PLAYERS;a++)
	{
		Players[a].inventory.Reset(false,INVRESET_ALL);
		Players[a].counter_measures.Reset(false,INVRESET_ALL);
	}
	
#ifdef GAMEGAUGE
	if(1)
#else
	if(Game_gauge_do_time_test)
#endif
	{
		char fpsfile[_MAX_PATH*2];
		CFILE *cfp;
		ddio_MakePath(fpsfile,Base_directory,"fps.txt",NULL);
		cfp = cfopen(fpsfile,"wt");
		if(cfp)
		{
			char szline[200];
			sprintf(szline,"%.2f Descent3 v%d.%d",gamegauge_total_frames/gamegauge_total_time,(int)Program_version.major,(int)Program_version.minor);
			cf_WriteString(cfp,szline);
			sprintf(szline,"%d Min",min_one_second);
			cf_WriteString(cfp,szline);
			sprintf(szline,"%d Max",max_one_second);
			cf_WriteString(cfp,szline);
			for(int b=1;((b<GAMEGAUGE_MAX_LOG)&&(gamegauge_fpslog[b]));b++)
			{
				sprintf(szline,"%d Second %d",gamegauge_fpslog[b],b);
				cf_WriteString(cfp,szline);
			}
			
			cfclose(cfp);
		}
	}
#if defined(OEM) 
	if(!Dedicated_server)
		ShowStaticScreen("oemupsell.ogf");
#elif defined(DEMO)
	if(!Dedicated_server)
		ShowStaticScreen("upsell.ogf");
#endif
	FreeMultiDLL();
	SetScreenMode(SM_NULL);
}

#if (defined(OEM) || defined(DEMO) || defined(RELEASE))
//Shows a fullscreen static bitmap
void ShowStaticScreen(char *bitmap_filename,bool timed,float delay_time)
{
	extern void ui_SetScreenMode(int w, int h);
	chunked_bitmap splash_bm;

// do splash screen on release
	int bm_handle = bm_AllocLoadFileBitmap(IGNORE_TABLE(bitmap_filename),0);

	ddio_KeyFlush();
	ddio_MouseReset();

	if (bm_handle > -1) {
		if (!bm_CreateChunkedBitmap(bm_handle, &splash_bm)) 
			Error("Failed to slice up %s.",bitmap_filename);

		bm_FreeBitmap(bm_handle);
		float start_time = timer_GetTime();
		while (1)
		{
			StartFrame();
			
			rend_DrawChunkedBitmap(&splash_bm, 0,0,255);
			rend_Flip();
			EndFrame();

			
			Descent->defer();

			if(!timed)
			{
				int key = ddio_KeyInKey();
				if (key == KEY_ENTER || key == KEY_SPACEBAR || key == KEY_ESC) 
					break;

				int x,y,dx,dy;
				if (ddio_MouseGetState(&x,&y,&dx,&dy))
					break;
			}
			else
			{
				if((timer_GetTime()-start_time)>delay_time)
				{
					break;
				}

			}
		}

		bm_DestroyChunkedBitmap(&splash_bm);
	}
	else {
		mprintf((0, "Couldn't load %s.\n",bitmap_filename));
	}

	ui_SetScreenMode(Max_window_w, Max_window_h);
}
#endif



//	---------------------------------------------------------------------------
//	Accessor functions
//	---------------------------------------------------------------------------

void SetFunctionMode(function_mode mode)
{
	Last_function_mode = Function_mode;
	Function_mode = mode;
}


function_mode GetFunctionMode()
{
	return Function_mode;
}


//	---------------------------------------------------------------------------
// The game's defer handler (For Win32, it happens during idle processing) 
//	---------------------------------------------------------------------------
extern bool Skip_render_game_frame;
void GameFrame(void);
void D3DeferHandler(bool is_active)
{
	if (is_active) {
	// perform any needed io system processing in the background
		ddio_Frame();
	}
	else {
	//JEFF - If the game is in idle loop and we are in multiplayer game
	//then process the game frame so we stay alive in the game.  
		if(Game_mode&GM_MULTI) {
			Skip_render_game_frame = true;
			GameFrame();
			Skip_render_game_frame = false;
		}
	}

	//JEFF - Commented out due to new idle processing
	nw_DoNetworkIdle();
}


//	---------------------------------------------------------------------------
//	this is called when you hit a debug break!
//	---------------------------------------------------------------------------
#ifndef RELEASE
#include "debug.h"
#include "renderer.h"
extern int rend_initted;			// from game.cpp

void D3DebugStopHandler()
{
//	close off all systems for debugging.
	if (rend_initted) {
		rend_Close();
	}
}


void D3DebugResumeHandler()
{
//	reopen all systems for gameplay.
	if (rend_initted) 
		rend_initted = rend_Init (PreferredRenderer, Descent,&Render_preferred_state);
	
	if (rend_initted!=0)
		rend_initted=1;	
}

#endif

void RenderBlankScreen(void);

char * GetCDVolume(int cd_num)
{
	char *p = NULL;

#if defined (MACINTOSH)
	char volume_labels[3][_MAX_PATH] = {"","Descent3","Descent3"};
#elif !defined (OEM)
	char volume_labels[3][_MAX_PATH] = {"","D3_1","D3_2"};
#else
	char volume_labels[3][_MAX_PATH] = {"","D3OEM_1",""};
#endif

	p = ddio_GetCDDrive("D3_DVD");

	if(!p)
		p = ddio_GetCDDrive(volume_labels[cd_num]);

	if(p)
	{
		//We've got the disk already in the drive!
		return p;
	}
	else
	{
		//Don't prompt for CD if not a release build
		#ifndef RELEASE
		//return NULL;
		#endif

		//prompt them to enter the disk...
		do
		{
			char message_txt[50];
#ifdef MACINTOSH
			strcpy(message_txt,TXT_CDPROMPT);
			message_txt[strlen(message_txt)-2] = '\0';
#else
			sprintf(message_txt,TXT_CDPROMPT,cd_num);
#endif
			//We need a background drawn!
#if defined(LINUX)
			void (*ui_cb)() = GetUICallback();
#else
			void *ui_cb = GetUICallback();
#endif
			if(ui_cb==NULL)
				SetUICallback(RenderBlankScreen);
			int res = DoMessageBox(PRODUCT_NAME,message_txt,MSGBOX_OKCANCEL,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);		
			SetUICallback((void (*)(void))ui_cb);
			//
			if(res == 0)
			{
				return NULL;
			}
			p = ddio_GetCDDrive(volume_labels[cd_num]);
			if(p && *p)
				return p;
		}while(!(p && *p));
		
		return NULL;
	}

}

typedef struct file_vols
{
	char file[_MAX_PATH];
	char localpath[_MAX_PATH*2];
	int volume;
	bool localized;
}file_vols;

extern int CD_inserted;

//Localization_GetLanguage();


char Oem_language_dirs[5][10] = 
{
	"English",
	"German",
	"Spanish",
	"Italian",
	"French"
};


file_vols file_volumes[] = 
{
	//Filename, directory it might be installed on the hard drive, CD number to look for it
	{"d3.mn3","missions",1,false},
	{"d3_2.mn3","missions",2,false},
	{"level1.mve","movies",1,true},
	{"level5.mve","movies",2,true},
	{"end.mve","movies",2,true},
	{"intro.mve","movies",1,true},
#ifdef MACINTOSH
	{"ppics.hog","missions",1,true},
	{"training.mn3","missions",1,true},
#else
	{"dolby1.mv8","movies",1,true},
#endif
	{"d3voice1.hog","missions",1,true},
	{"d3voice2.hog","missions",2,true}
};

int num_cd_files = sizeof(file_volumes)/sizeof(file_vols);

//This function figures out whether or not a file needs to be loaded off of
//CD or off of the local drive. If it needs to come from a CD, it figures out
//which CD and prompts the user to enter that CD. If they hit cancel, it 
//returns NULL.
char * GetMultiCDPath(char *file)
{
	static char filepath[_MAX_PATH*2];
	static char fullpath[_MAX_PATH*2];
	int volume = 0;
	int i;
	
	if(file==NULL)
		return NULL;
	if(*file=='\0')
		return NULL;

	// see if there is a command line override for the name of the intro movie
	int intro_movie_arg = FindArg("-intro");
	char temp_filename[256];
	
	//Clear out any old path
	//memset(filepath,0,_MAX_PATH*2);
	filepath[0] = NULL;
	
	for(i=0;i<num_cd_files;i++)
	{
		char *vol_filename;
		vol_filename = file_volumes[i].file;

		// check to see if we need to override this string (for intro movie)
		if(intro_movie_arg>0 && !stricmp(vol_filename,"intro.mve"))
		{
			// we have to override this intro movie
			char extension[16],*p;
			p = extension;
			ddio_SplitPath(GameArgs[intro_movie_arg+1],NULL,temp_filename,extension);
			while(*p) { *p = tolower(*p); p++;}
			strcat(temp_filename,extension);
			vol_filename = temp_filename;
		}

		if(strcmpi(vol_filename,file)==0)
		{
			volume = file_volumes[i].volume;			
			ddio_MakePath(fullpath,LocalD3Dir,file_volumes[i].localpath,file,NULL);
			//See if the file is in the local dir already.
			if(cfexist(fullpath))
			{
				return fullpath;
			}
		#ifdef _DEBUG
			else if (strcmpi(file_volumes[i].localpath, "movies")==0) {
			// if one specified a directory where the movies are located.
				int arg = FindArg("-moviedir");
				if (arg) {
					ddio_MakePath(fullpath, GameArgs[arg+1], file, NULL);
					return fullpath;
				}
			}
		#endif
			break;
		}
	}
	//This is a file we don't know about
	if(i==num_cd_files)
		return file;

	if(volume)
	{
		char *p = GetCDVolume(volume);
		if(p)
		{
			//If it's DVD, we need to get the proper files for the language
			if( (CD_inserted==3)&&(file_volumes[i].localized) )
			{
				ddio_MakePath(filepath,p,file_volumes[i].localpath,Oem_language_dirs[Localization_GetLanguage()],file,NULL);
			}
			else
			{
				ddio_MakePath(filepath,p,file_volumes[i].localpath,file,NULL);
			}
			//strcpy(filepath,p);
			//strcat(filepath,file);
			return filepath;
		}
		else
		{
			return NULL;
		}		
	}

	return NULL;
}
