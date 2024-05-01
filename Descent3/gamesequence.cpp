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

#ifdef  USE_PROFILER
#include <profiler.h>	
#endif

#include "gamesequence.h"

#include "game.h"
#include "gameloop.h"
#include "descent.h"
#include "player.h"
#include "Mission.h"
#include "BOA.h"
#include "gameevent.h"
#include "AIMain.h"

#include "soar_helpers.h"

#include "terrain.h"
#include "hlsoundlib.h"
#include "SmallViews.h"
#include "polymodel.h"
#include "gametexture.h"
#include "hud.h"
#include "findintersection.h"
#include "menu.h"
#include "newui.h"
#include "cockpit.h"
#include "help.h"
#include "buddymenu.h"
#include "mem.h"
#include "soundload.h"
#include "robot.h"
#include "screens.h"
#include "game2dll.h"
#include "ship.h"
#include "TelCom.h"
#include "scorch.h"
#include "render.h"
#include "stringtable.h"
#include "ddio_common.h"
#include "gamesave.h"
#include "sounds.h"
#include "ambient.h"
#include "vclip.h"
#include "pilot.h"
#include "doorway.h"
#include "matcen.h"
#include "dedicated_server.h"
#include "networking.h"
#include "levelgoal.h"
#include "demofile.h"
#include "lightmap_info.h"
#include "lightmap.h"
#include "fireball.h"
#include "d3music.h"
#include "TelComAutoMap.h"
#include "aiambient.h"
#include "ObjScript.h"
#include "marker.h"
#include "gamecinematics.h"
#include "osiris_dll.h"
#include "debuggraph.h"
#include "multi_dll_mgr.h"
#include "multi_ui.h"
#include "rocknride.h"
#include "gamepath.h"
#include "vclip.h"
#include "bsp.h"
#include "vibeinterface.h"

#include "args.h"
void ResetHudMessages(void);

//	Variables
tGameState Game_state = GAMESTATE_IDLE;		// current game state.
tGameState Last_game_state = GAMESTATE_IDLE;	// previous frame game state.
int Game_interface_mode = GAME_INTERFACE;		// game interface mode (options menu?)

extern bool FirstGame;

static bool Level_started = false;
static int Level_warp_next = 0;

#ifdef E3_DEMO
extern float E3_TIME_LIMIT;
extern bool E3_enforce_level_restart;
#endif

extern float Multi_Game_time_start;

extern bool Demo_looping;
extern bool IsRestoredGame;

//	internal functions
bool StartNewGame();									//	We start a game by using the current mission in memory.
bool DoLevelIntro();									// shows movie and briefing
void StartGameFromEditor();						// set up new game/new level stuff when playing from editor
void EndLevel(int state);							//	ends the current level.
void RestartLevel();									// restarts the current level.
void RunGameMenu();									// executes a game menu.
void FreeThisLevel();								// frees any data/esc that was created for that level.
void FlushDataCache();								// Clears out all the level specific stuff from memory
void SetNextLevel();									// advances to the next level
void CheckHogfile();									// make sure we have the right hogfile			

// Data paging functions
void PageInAllData ();
void PageInLevelTexture (int);
bool PageInSound (int);
void PageInDoor (int);
void PageInWeapon (int);
void PageInGeneric (int);
void PageInShip (int);

// Data allocation arrays, for keeping track of what textures/sounds are level specific
ubyte Models_to_free[MAX_POLY_MODELS];
ubyte Textures_to_free[MAX_TEXTURES];
ubyte Sounds_to_free[MAX_TEXTURES];

#ifdef EDITOR
extern vector editor_player_pos;
extern matrix editor_player_orient;
extern int editor_player_roomnum;
#endif

extern bool mn3_Open(const char *mn3file);
extern void mn3_Close();

extern bool Game_gauge_do_time_test;
extern char Game_gauge_usefile[_MAX_PATH];

///////////////////////////////////////////////////////////////////////////////
//	Sequences game events
///////////////////////////////////////////////////////////////////////////////
bool GameSequencer()
{
	bool in_editor = (GetFunctionMode() == EDITOR_GAME_MODE) ;
	tGameState old_game_state;

// interpret current function mode.
	switch (GetFunctionMode())
	{
	case RESTORE_GAME_MODE:
	case LOADDEMO_MODE:
	case GAMEGAUGE_MODE:
		SetFunctionMode(GAME_MODE);				// need to do so sequencer thiks we're in game.
		break;
	}


//	The main game sequencer 
	while ((GetFunctionMode() == GAME_MODE) || (GetFunctionMode() == EDITOR_GAME_MODE))
	{
		old_game_state = Game_state;

		switch (Game_state) 
		{
		case GAMESTATE_NEW:
			StartNewGame();
			SetGameState(GAMESTATE_LVLSTART);
			break;

		case GAMESTATE_LVLSTART:
			CheckHogfile();		//make sure we have the right hogfile

			if (in_editor) {		//start in editor
				#ifdef EDITOR
				StartGameFromEditor();
				#endif		
			}
			else {					//start in game
				if (!DoLevelIntro() || !LoadAndStartCurrentLevel())
				{
					if(Game_mode & GM_MULTI)
						MultiLeaveGame();
					MultiDLLGameStarting=0;
					Multi_bail_ui_menu=false;
					SetFunctionMode(MENU_MODE);				//	return to main menu
					break;
				}
			}
			
			SetGameState(GAMESTATE_LVLPLAYING);
			break;

		case GAMESTATE_LVLPLAYING:
		//	must do to display main menu interfaces
			if (Last_game_state == GAMESTATE_LVLSTART) {
			// 1st frame resume sounds.
				Sound_system.ResumeSounds();
			}

			if (Game_interface_mode != GAME_INTERFACE) {
				RunGameMenu();
			}
			else {
				Descent->defer();	
				GameFrame();
			}
			break;

		case GAMESTATE_LOADDEMO:
			if(DemoPlaybackFile(Demo_fname)) {
				SetGameState(GAMESTATE_LVLPLAYING);
			}
			else {
				SetFunctionMode(MENU_MODE);
			}
			break;
		case GAMESTATE_GAMEGAUGEDEMO:
			{
				char ggdemopath[_MAX_PATH*2];
				ddio_MakePath(ggdemopath, User_directory,"demo",Game_gauge_usefile,NULL);
				if(DemoPlaybackFile(ggdemopath)) {
					SetGameState(GAMESTATE_LVLPLAYING);
				}
				else {
					SetFunctionMode(MENU_MODE);
				}
			}
			break;
		case GAMESTATE_LOADGAME:
			if(Game_mode & GM_MULTI)
			{
				DoMessageBox(TXT_ERROR, TXT_CANT_LOAD_MULTI, MSGBOX_OK);
				break;	
			}			
			D3MusicStop();						// make sure all music streams are stopped before restoring game.
			PauseGame();
			if (!LoadCurrentSaveGame()) {
				SetFunctionMode(MENU_MODE);
			}
			else {
				SetGameState(GAMESTATE_LVLPLAYING);
				SetScreenMode(SM_GAME);
				SetHUDMode(GetHUDMode());
			}
			ResumeGame();
			break;

		case GAMESTATE_LVLEND:
			SuspendControls();
			EndLevel(1);
			SetGameState(GAMESTATE_LVLNEXT);
			
			
			//This is for HEAT.NET and their tournement mode stuff
			//if(FindArg("-doonelevel"))
			//	SetGameState(GAMESTATE_LVLSTART);
			if(FindArg("-doonelevel"))
			{
				SetFunctionMode(QUIT_MODE);
			}
			
			break;

		case GAMESTATE_LVLNEXT:
			SetNextLevel();
			SetGameState(GAMESTATE_LVLSTART);
			break;

		case GAMESTATE_LVLWARP:
			SuspendControls();
			EndLevel(-1);
			SetCurrentLevel(Level_warp_next);
			SetGameState(GAMESTATE_LVLSTART);
			break;

		case GAMESTATE_LVLFAILED:
			SuspendControls();
			EndLevel(0);
			SetGameState(GAMESTATE_LVLSTART);		//restart same level
			break;
		}
		Last_game_state = old_game_state;
	}

	#ifdef EDITOR		//Ugly hack to get play position back to the editor
	if (Player_object && (Player_object->type == OBJ_PLAYER)) {
		editor_player_pos =	 Player_object->pos;
		editor_player_orient = Player_object->orient;
		editor_player_roomnum = Player_object->roomnum;
	}
	else
		editor_player_roomnum = -1;
	#endif

//	free up scripts.
	PltWriteFile(&Current_pilot);
	FreeThisLevel();

	SetGameMode(GM_NONE);

	return true;
}

//Make sure we have the correct hogfile
void CheckHogfile()
{
	char hogpath[_MAX_PATH*2];
	mprintf((0,"Checking to see if we need to open another hog off of disk or CDROM\n"));

	if(Current_mission.filename && (strcmpi(Current_mission.filename,"d3.mn3")==0) && (Current_mission.cur_level > 4) )
	{
		//close the mission hog file and open d3_2.mn3
		mn3_Close();
		char *hogp = GetMultiCDPath("d3_2.mn3");
		if(hogp)
		{
			strcpy(hogpath,hogp);
			mn3_Open(hogpath);
			mem_free(Current_mission.filename);
			Current_mission.filename = mem_strdup("d3_2.mn3");
		}
		else
		{
			SetFunctionMode(MENU_MODE);
		}
	}
	else if(Current_mission.filename && (strcmpi(Current_mission.filename,"d3_2.mn3")==0) && (Current_mission.cur_level <= 4) )
	{
		//Part 2 of the mission is d3_2.mn3
		//close the mission hog file and open d3.mn3
		mn3_Close();
		char *hogp = GetMultiCDPath("d3.mn3");
		if(hogp)
		{
			strcpy(hogpath,hogp);
			mn3_Open(hogpath);
			mem_free(Current_mission.filename);
			Current_mission.filename = mem_strdup("d3.mn3");
		}
		else
		{
			SetFunctionMode(MENU_MODE);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
//	quickly sets up a mission of 1 level, and doesn't do any
//	fancy intro stuff
bool SimpleStartLevel(char *level_name)
{
//	this initializes a mini one level mission with no frills.

	Current_mission.cur_level = 1;
	Current_mission.num_levels = 1;
	Current_mission.levels = (tLevelNode *)mem_malloc(sizeof(tLevelNode));
	memset(Current_mission.levels, 0, sizeof(tLevelNode));
	
	Current_level = NULL;
	Current_mission.levels[0].filename = mem_strdup(level_name);
	InitMissionScript();

	return true;
}


///////////////////////////////////////////////////////////////////////////////
//	We start a game by using the current mission in memory.
//	First we display the mission briefing (the intro movie for Descent 3, if not an add on)
//	Then we start level
///////////////////////////////////////////////////////////////////////////////

bool StartNewGame()
{
	SetCurrentLevel(Current_mission.cur_level);

	ResetGamemode();

	if (!(Game_mode & GM_MULTI)) {
		Player_num=0;	// Reset player num
		Players[Player_num].ship_index = FindShipName(DEFAULT_SHIP);
		ASSERT(Players[Player_num].ship_index != -1);
	}

	if (Game_mode & GM_MULTI)
		CallGameDLL (EVT_GAMECREATED,&DLLInfo);

	// Load any addon data
	mng_LoadAddonPages();

	InitPlayerNewShip(Player_num,INVRESET_ALL);
	InitPlayerNewGame (Player_num);

	InitCameraViews(1);	 //Turn off all camera views, including rear views

	Quicksave_game_slot = -1;					// reset so that quicksave key will always go to savegame menu first

	return true;
}


//Shows the intro movie and briefing for a level
bool DoLevelIntro()
{
	tLevelNode *lvl = Current_level;

	mprintf((0,"In DoLevelIntro()\n"));

	// multiplayer stuff (we skip the movie and briefings)
	if (Game_mode & GM_MULTI) {
		if (Netgame.local_role==LR_CLIENT) {
		// Clear out residual junk
			MultiFlushAllIncomingBuffers();

			if (! MultiPollForLevelInfo ()) {
				return false; 
			}
		}
		return true;
	}

#ifdef STEALTH	//DAJ just get started
	return true;
#endif
	//	if this level has a movie:
	if (lvl->flags & LVLFLAG_STARTMOVIE) {
		DoMissionMovie(Current_mission.levels[Current_mission.cur_level-1].moviename);
	}

	//	if this level has a briefing:
	if (lvl->flags & LVLFLAG_BRIEFING) {
		if(!DoMissionBriefing(Current_mission.cur_level-1)) {
			//quit exit to main menu
			return false;
		}
	}
	return true;
}

//Sets the current level for subsequent level loads, movies, or briefings
void SetCurrentLevel(int level)
{
	ASSERT(level>0 && level<=Current_mission.num_levels);

	//Set this level as the mission's current level
	Current_mission.cur_level = level;

	//Set pointer to this level
	Current_level = &Current_mission.levels[level-1];
}

void StartLevelSounds();

//#ifdef GAMEGAUGE
extern float gamegauge_start_time;
//#endif

//Get rid of any viewer objects in the level
void ClearViewerObjects()
{
	int i;
	object *objp;

	for (i=0,objp=Objects;i<=Highest_object_index;i++,objp++)
		if (objp->type == OBJ_VIEWER) {
			mprintf((0,"Deleting viewer object %d\n",i));
			ObjDelete(i);
		}
}

//If low or medium detail, delete all or some ambient objects
void DeleteAmbientObjects()
{
	int i,count;
	object *objp;

	if (!(Game_mode & GM_MULTI) && Detail_settings.Object_complexity == 2)		//high
		return;

	for (i=0,objp=Objects,count=0;i<=Highest_object_index;i++,objp++) {
		if (IS_GENERIC(objp->type) && (Object_info[objp->id].flags & OIF_AMBIENT_OBJECT)) {
			if ((Detail_settings.Object_complexity == 0) || (count & 1) || (Game_mode & GM_MULTI))
				ObjDelete(i);
			count++;
		}
	}
}

void Localization_SetLanguage(int type);
int Localization_GetLanguage(void);
void LoadLevelText(char *level_filename);

//Starts the level, which has already been loaded
void StartLevel()
{
	extern void RestoreCameraRearviews();		// gameloop.cpp

	//Init time
	Gametime = 0.0f;

	//Make sure all sounds have stopped
	Sound_system.StopAllSounds();

	//Get the data for this level
	PageInAllData();

	// TEMP HACK 
	if (rend_SupportsBumpmapping())
		Detail_settings.Bumpmapping_enabled=1;
	else
		Detail_settings.Bumpmapping_enabled=0;

	

	//Initialize a bunch of stuff for this level
	MakeBOA();
#ifndef MACINTOSH
	ComputeAABB(true);
#endif
	
	//Clear/reset objects & events
	ClearAllEvents();
	ClearRoomChanges();
	ResetMarkers();
	ResetScorches();
	ResetWaypoint();
	ResetLightGlows();
	DoorwayDeactivateAll();
	ClearViewerObjects();
	DeleteAmbientObjects();

	//AI, scripting, & Soar
	AIInitAll();

	//Set up the player object
	ResetPlayerObject(Player_num);
  	
	DSSoarInit();

	InitMatcensForLevel();

	//What is this?
	a_life.InitForLevel();

	//Start sound & music
	StartLevelSounds();
	Sound_system.PauseSounds();									// HACK!! pause sounds started up to now.
	D3MusicStart(Current_level->score);

	//Get the list of waypoints from the waypoint objects
	MakeAtuoWaypointList();

	//Test stuff
	#ifdef _DEBUG
	InitTestSystems();
	#endif

	//Set screen mode
	SetScreenMode(SM_GAME);

	//Init HUD and cockpit	
	int ship_index;

	if(Game_mode&GM_MULTI)
	{
		char ship_model[PAGENAME_LEN];
		Current_pilot.get_ship(ship_model);
		ship_index = FindShipName(ship_model);
		ASSERT(ship_index != -1);	//DAJ -1FIX
		if(Netgame.local_role==LR_SERVER)
		{
			if(PlayerIsShipAllowed(Player_num,ship_model))
			{
				Players[Player_num].ship_index=FindShipName (ship_model);
				ASSERT(Players[Player_num].ship_index != -1);	//DAJ -1FIX
			}
			else
			{
				mprintf((0,"Player %d wanted to use a ship (%s) which wasn't allowed.\n",Player_num,ship_model));
				int i;
				bool found_one=false;

				//Loop through ships, looking for one that's allowed
				for (i=0;i<MAX_SHIPS && !found_one;i++)
				{
					if(Ships[i].used && PlayerIsShipAllowed(Player_num,i))
					{
						Players[Player_num].ship_index=i;
						found_one=true;
					}
				}

				//We should have found one
				ASSERT(found_one == true);
			}
			ship_index = Players[Player_num].ship_index;
			PlayerChangeShip (Player_num,ship_index);
		}
	}
	else
	{
		ship_index = Players[Player_num].ship_index;
	}

	if (ship_index<0)
		ship_index=0;
	InitShipHUD(ship_index);							
	InitCockpit(ship_index);
	if (GetHUDMode() == HUD_COCKPIT)		// make sure cockpit is open, if hud mode is cockpit
		QuickOpenCockpit();
	ResetHUDMessages();
	ResetGameMessages();
	ResetReticle();
	ResetSmallViews();		//ResetSmallViews() must come before InitCameraViews()
	InitCameraViews(0);		//ResetSmallViews() must come before InitCameraViews()
	RestoreCameraRearviews();
	SetHUDMode(GetHUDMode());		//what does this do?

	//Init zoom
	Render_zoom=D3_DEFAULT_ZOOM;

	// flush controller system.
#ifndef MACINTOSH
	ResumeControls();
	Controller->flush();
#endif

	Level_goals.InitLevel();

	//Do multiplayer & player stuff
	if (Game_mode & GM_MULTI) {
		MultiStartNewLevel(Current_mission.cur_level);
	}
	else {	//Init player for single-player
		DeleteMultiplayerObjects();
		InitPlayerNewLevel (Player_num);
	}

	//Set flags
	Level_started = true;
	Multi_bail_ui_menu = false;

	//Clear robot keys
	Global_keys = 0;

	// Clear OSIRIS stuff
	Osiris_ResetAllTimers();

	// Initialize IGC
	Cinematic_LevelInit();

	//Bash the language to english
	int save_lang = Localization_GetLanguage();
	Localization_SetLanguage(LANGUAGE_ENGLISH);

	//Load the English text so the goal stuff will initialize correctly
	LoadLevelText(Current_mission.levels[Current_mission.cur_level-1].filename);

	//Restore the correct language
	Localization_SetLanguage(save_lang);

	//Init the level scripts
	InitLevelScript();

	//Load the localized text
	LoadLevelText(Current_mission.levels[Current_mission.cur_level-1].filename);

	Gametime = 0.0f;
	//Start the clock
	InitFrameTime();
	//#ifdef GAMEGAUGE
	gamegauge_start_time = timer_GetTime();
	//#endif
	LoadLevelProgress(LOAD_PROGRESS_DONE,0);

#ifdef MACINTOSH
	// flush controller system.
	ResumeControls();
	Controller->flush();
	#ifdef USE_OPENGL
	extern void opengl_ResetContext(void);
		opengl_ResetContext();
	#endif
#ifdef USE_PROFILER
	ProfilerSetStatus(1);
#endif
#endif
}

//Loads a level and starts everything up
bool LoadAndStartCurrentLevel()
{
	char hogpath[_MAX_PATH*2];
	//This is a bit redundant because we just did it in most cases, but we need to be sure that it always happens,
	//and this code is here for weird systems, like save/load and demo, etc.
	if(Current_mission.filename && (strcmpi(Current_mission.filename,"d3.mn3")==0) && (Current_mission.cur_level > 4) )
	{
		//close the mission hog file and open d3_2.mn3
		mn3_Close();
		char *hogp = GetMultiCDPath("d3_2.mn3");
		if(hogp)
		{
			strcpy(hogpath,hogp);
			mn3_Open(hogpath);
			mem_free(Current_mission.filename);
			Current_mission.filename = mem_strdup("d3_2.mn3");
		}
		else
		{
			SetFunctionMode(MENU_MODE);
		}
	}
	else if(Current_mission.filename && (strcmpi(Current_mission.filename,"d3_2.mn3")==0) && (Current_mission.cur_level <= 4) )
	{
		//Part 2 of the mission is d3_2.mn3
		//close the mission hog file and open d3.mn3
		mn3_Close();
		char *hogp = GetMultiCDPath("d3.mn3");
		if(hogp)
		{
			strcpy(hogpath,hogp);
			mn3_Open(hogpath);
			mem_free(Current_mission.filename);
			Current_mission.filename = mem_strdup("d3.mn3");
		}
		else
		{
			SetFunctionMode(MENU_MODE);
		}
	}
	
	//	load the level. if fails, then bail out
	//ShowProgressScreen (TXT_LOADINGLEVEL);
	if (!LoadMissionLevel(Current_mission.cur_level)) 
		return false;

	AutomapClearVisMap();

	//Now start the level
	StartLevel();

	//Done!
	return true;
}

#ifdef EDITOR
//Called to do game initialization stuff when playing from the editor
//Mostly, this will be the same stuff as StartNewLevel()
void StartGameFromEditor()
{
	//Set up the player ship
	InitPlayerNewShip(Player_num,INVRESET_ALL);

	//Start the level
	StartLevel();

	//Move the player to the editor viewer position
	ObjSetPos(Player_object,&editor_player_pos,editor_player_roomnum,&editor_player_orient,false);
}
#endif

void ComputeCenterPointOnFace(vector *vp,room *rp,int facenum);

// Start object ambient sounds
void StartObjectSounds()
{
	for(int i = 0; i < MAX_OBJECTS; i++)
	{
		object *objp = &Objects[i];

		//If generic object, check for ambient sound
		switch (objp->type) {

			case OBJ_CLUTTER:
			case OBJ_BUILDING:
			case OBJ_ROBOT:
			case OBJ_POWERUP: {
				int ambient_sound = Object_info[objp->id].sounds[GSI_AMBIENT];
				if (ambient_sound != SOUND_NONE_INDEX)
					Sound_system.Play3dSound(ambient_sound, SND_PRIORITY_LOWEST, objp);
				break;
			}

			case OBJ_SOUNDSOURCE:
				ASSERT(objp->control_type == CT_SOUNDSOURCE);
				if (objp->name)		//if has name, attach sound to object
					Sound_system.Play3dSound(objp->ctype.soundsource_info.sound_index, SND_PRIORITY_NORMAL, objp, objp->ctype.soundsource_info.volume);
				else {					//no name, so attach to 3D position & delete object
					pos_state ps;
					ps.position = &objp->pos;
					ps.roomnum = objp->roomnum;
					ps.orient = (matrix *) &Identity_matrix;
					Sound_system.Play3dSound(objp->ctype.soundsource_info.sound_index,SND_PRIORITY_NORMAL, &ps,objp->ctype.soundsource_info.volume);
					ObjDelete(i);
				}
				break;
		}
	}
}

//Go through the level and start and sounds associated with textures
void StartTextureSounds()
{
	int r,f;
	room *rp;
	face *fp;

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			for (f=0,fp=rp->faces;f<rp->num_faces;f++,fp++) {
				int sound = GameTextures[fp->tmap].sound;
				if ((sound != -1) && (sound != SOUND_NONE_INDEX)) {
					vector pos;
					pos_state ps;
					ComputeCenterPointOnFace(&pos,rp,f);
					ps.position = &pos;
					ps.roomnum = r;
					ps.orient = (matrix *) &Identity_matrix;
					Sound_system.Play3dSound(sound,SND_PRIORITY_LOWEST, &ps);
				}
			}
		}
	}
}

//Starts all the sounds on this level
void StartLevelSounds()
{
	StartObjectSounds();
	StartTextureSounds();
	StartTerrainSound();
	InitAmbientSounds();
}

// frees any data/esc that was created for that level
void FreeThisLevel()										
{
	if (!Level_started)
		return;

	bool original_controls = Control_poll_flag;

	Cinematic_Stop();

	// make sure our controls are correct (this is needed for failed missions)
	if(Control_poll_flag!=original_controls)
	{
		if(Control_poll_flag)
			SuspendControls();
		else
			ResumeControls();
	}

	Multi_bail_ui_menu = false;

	DemoAbort();

// closes hud.	
	FreeCockpit();
	CloseShipHUD();
	
//	end music sequencer's run.
	D3MusicStop();

	FreeScriptsForLevel();
	ClearAllEvents ();

	// Resets the ambient life controller
	a_life.ALReset();

	DestroyAllMatcens();
	Level_goals.CleanupAfterLevel();

	Sound_system.StopAllSounds();

// Clear out all memory that was used for this past level
	FlushDataCache ();

	InitGamePaths();	//DAJ LEAKFIX
	
	DSSoarEnd();

	// Reset the camera if need be
	if (Player_camera_objnum!=-1)
	{
		SetObjectDeadFlag (&Objects[Player_camera_objnum]);
		Player_camera_objnum=-1;
		Viewer_object=&Objects[Players[Player_num].objnum];
	}

	ResetHUDMessages();

	DestroyDefaultBSPTree();
	
	Level_started = false;
	IsRestoredGame = false;
}


extern void FreeTextureBumpmaps(int);
// Clears out all the level specific stuff from memory
void FlushDataCache ()
{
	int i;

	// This must be done before we free polymodels
	FreeAllObjects();

	#ifdef EDITOR
		if (Network_up)
			return;
	#endif

	int texfreed=0;
	int modelsfreed=0;
	int soundsfreed=0;

	for (i=0;i<MAX_TEXTURES;i++)
	{
		if (Textures_to_free[i]!=0)
		{
			if (!(GameTextures[i].flags & TF_ANIMATED))	
			{
				if (GameTextures[i].bumpmap!=-1)
					FreeTextureBumpmaps(i);

				bm_FreeBitmapData (GameTextures[i].bm_handle);
			}

			texfreed++;
		}
	}
	for (i=0;i<MAX_POLY_MODELS;i++)
	{
		if (Models_to_free[i]!=0)
		{
			FreePolymodelData (i);
			modelsfreed++;
		}
	}

	for (i=0;i<MAX_SOUNDS;i++)
	{
#ifndef MACINTOSH		//on Mac, free all sounds
		if (Sounds_to_free[i]!=0)	
#endif
		{
			soundsfreed++;
#ifndef MACINTOSH
			int index=Sounds[i].sample_index;
#else
			int index=i;
#endif
			if (SoundFiles[index].sample_16bit)
			{
				GlobalFree(SoundFiles[index].sample_16bit);
				SoundFiles[index].sample_16bit=NULL;
			}
			if (SoundFiles[index].sample_8bit)
			{
				GlobalFree(SoundFiles[index].sample_8bit);
				SoundFiles[index].sample_8bit=NULL;
			}
		}
	}

	mprintf ((0,"Freed %d textures, %d models, and %d sounds.\n",texfreed,modelsfreed,soundsfreed));

	if (!Dedicated_server)
		rend_ResetCache();

}

///////////////////////////////////////////////////////////////////////////////
//	Runs any possible end movie and stats screen, as well as loads in the next level
///////////////////////////////////////////////////////////////////////////////

//Parameter:	state -  1 = success, 0 = failure, -1 = abort
extern float Player_shields_saved_from_last_level;
extern float Player_energy_saved_from_last_level;
void EndLevel(int state)
{
	// tell IntelliVIBE
	VIBE_DoLevelEnd();

	tLevelNode *lvl = &Current_mission.levels[Current_mission.cur_level-1];

	// Tells all the clients to end the level
	if ((Game_mode & GM_MULTI) && Netgame.local_role==LR_SERVER)
	{
		MultiSendLevelEnded (state,Multi_next_level);
		CallGameDLL (EVT_CLIENT_GAMELEVELEND,&DLLInfo);
		CallMultiDLL(MT_EVT_GAME_OVER);
	}

	if (!(Game_mode & GM_MULTI)){

		//report the information to the pilot's mission data
		CurrentPilotUpdateMissionStatus();
//		Sound_system.StopAllSounds();		-- moved to below because bug was reported that sounds were playing in performance screen

		//save our shields (in case this call is due to starting a new level
		//in InitPlayerNewLevel, we'll determine if we should restore them
		//this function ALWAYS gets called right before InitPlayerNewLevel from StartLevel
		Player_shields_saved_from_last_level = Objects[Players[Player_num].objnum].shields;
		Player_energy_saved_from_last_level = Players[Player_num].energy;
	}
	else
	{
		// Stop all player sounds
		for (int i=0;i<MAX_PLAYERS;i++)
		{
			if (NetPlayers[i].flags & NPF_CONNECTED)
			{
				PlayerStopSounds(i);
				Players[i].flags &=~(PLAYER_FLAGS_THRUSTED|PLAYER_FLAGS_AFTERBURN_ON);
			}
		}
	}

	// clear screen now.
	if (!Dedicated_server)
	{
		StartFrame();
		rend_ClearScreen(GR_BLACK);
		EndFrame();
		rend_Flip();
		Sound_system.StopAllSounds();
	}

																												
//	Sequencing here.
// if this level has an endmovie
	if ((state == 1) && (lvl->flags & LVLFLAG_ENDMOVIE) && !(Game_mode & GM_MULTI)) {
		DoMissionMovie(lvl->endmovie);
	}

//	display postlevel results.	
	if ((state != -1) && (GetFunctionMode() != EDITOR_GAME_MODE))
		PostLevelResults(state == 1);

	//check for dead players
	if (Game_mode&GM_MULTI)
	{
		//in multiplayer check all players
		for(int i=0;i<MAX_PLAYERS;i++)
		{
			if((NetPlayers[i].flags & NPF_CONNECTED) && (NetPlayers[i].sequence >= NETSEQ_PLAYING))
			{
				//check to see if player is dying
				if ((Players[i].flags & PLAYER_FLAGS_DYING) || (Players[i].flags & PLAYER_FLAGS_DEAD))
				{
					mprintf((0,"Prematurely ending death for player %d\n",i));
					EndPlayerDeath (i);
				}
			}
		}

	}else
	{
		//in single player, check Player_num
		if ((Players[Player_num].flags & PLAYER_FLAGS_DYING) || (Players[Player_num].flags & PLAYER_FLAGS_DEAD))
		{
			mprintf((0,"Prematurely ending death for player\n"));
			EndPlayerDeath (Player_num);
		}
	}

//	Free any game objects/etc that needs to be done when ending a level here.
	FreeThisLevel();
	
	//This option is used for HEAT.NET. They have their dedicated server shut down after the 
	//kill goal or time limit is reached.
	if(Dedicated_server)
	{
		if(FindArg("-quitongoal"))
		{
			SetFunctionMode (QUIT_MODE);
		}
	}
}


//Advances to the next level.  If we were on the last level, end the game
void SetNextLevel()
{
	ASSERT(Current_mission.cur_level > 0);

	tLevelNode *lvl = &Current_mission.levels[Current_mission.cur_level-1];

	if (Game_mode & GM_MULTI && Multi_next_level!=-1)
	{
		SetCurrentLevel(Multi_next_level);
		Multi_next_level=-1;
	}
	else if (lvl->flags & LVLFLAG_BRANCH) {
	//	jump to brached level
		mprintf((0,"Branching...\n"));
		Current_mission.cur_level = lvl->lvlbranch0;
		SetCurrentLevel(Current_mission.cur_level);
	}
	else if ((Current_mission.cur_level == Current_mission.num_levels) || (lvl->flags & LVLFLAG_FINAL)) {
		//	in this case we are done with the mission!
			DoEndMission();

			if (!(Game_mode & GM_MULTI))		// Multiplayer loops its levels and never ends
			{
				SetFunctionMode(MENU_MODE);
			}
	} 
	else if (Current_mission.game_state_flags & MSN_STATE_SECRET_LEVEL) {
		if (lvl->flags & LVLFLAG_SPAWNSECRET) {
		// display secret level screen?
			ShowProgressScreen(TXT_ENTERSECRETLVL,NULL,true);
			Descent->delay(1.0f);
			Current_mission.game_state_flags &= (~MSN_STATE_SECRET_LEVEL);
			SetCurrentLevel(lvl->secretlvl);
		}
		else {
			Int3();		//Game says go to secret level, but there's none to go to.
			SetCurrentLevel(Current_mission.cur_level+1);
		}
	}
	else {
		//nothing special
		SetCurrentLevel(Current_mission.cur_level+1);
	}
}


void GameFrameUI()
{
	GameFrame();
	Last_game_state = Game_state;
}


void RunGameMenu()
{
	bool pause_game = false;

	if (Game_interface_mode != GAME_INTERFACE) {
		NewUIWindow_alpha = 226;
		SetUICallback(GameFrameUI);
		SuspendControls();
		RNR_UpdateGameStatus(RNRGSC_INMENU);
	
	// reset bail flag.
		Multi_bail_ui_menu = false;
		
		if (!(Game_mode & GM_MULTI))  {
			pause_game = true;
#ifndef MACINTOSH
			PauseGame();
			D3MusicResume();
#else
			StopTime();	//DAJ just the time man
			Game_paused = true;
#endif
		}

		switch(Game_interface_mode)
		{
		case GAME_OPTIONS_INTERFACE:
			{
				extern void OptionsMenu();
				ui_ShowCursor();
				OptionsMenu();
				ui_HideCursor();
			}
			break;
		case GAME_HELP_INTERFACE:
			{
				ui_ShowCursor();
				HelpDisplay(); 
				ui_HideCursor();
			}
			break;
		case GAME_TOGGLE_DEMO:
			{
				ui_ShowCursor();
				DemoToggleRecording();				
				ui_HideCursor();
			}
			break;
		case GAME_BUDDY_INTERFACE:
			{
//			#ifdef DEMO
//				DoMessageBox(TXT_ERROR, TXT_WRONGVERSION, MSGBOX_OK);
//			#else 
				ui_ShowCursor();
				BuddyDisplay(); 
				ui_HideCursor();
//			#endif
			}
			break;
		case GAME_TELCOM_CARGO:
			{
			//#ifndef DEMO
				DoWaitPopup(true,TXT_TELCOMLOAD);
				ui_ShowCursor();
				TelComShow(TS_CARGO);
				ui_HideCursor();
				DoWaitPopup(false);
			//#else
			//	DoMessageBox(TXT_ERROR, TXT_WRONGVERSION, MSGBOX_OK);
			//#endif
			}
			break;
		case GAME_TELCOM_BRIEFINGS:
			{
				DoWaitPopup(true,TXT_TELCOMLOAD);
				ui_ShowCursor();
				TelComShow(TS_MAINMENU);
				ui_HideCursor();
				DoWaitPopup(false);
			}
			break;
		case GAME_TELCOM_AUTOMAP:
			{
			//#ifndef DEMO				
				DoWaitPopup(true,TXT_TELCOMLOAD);
				ui_ShowCursor();
				TelComShow(TS_MAP);
				ui_HideCursor();
				DoWaitPopup(false);
			//#else
			//	DoMessageBox(TXT_ERROR, TXT_WRONGVERSION, MSGBOX_OK);
			//#endif
			}
			break;
		case GAME_PAUSE_INTERFACE:
			if (Game_mode & GM_MULTI)
				AddHUDMessage(TXT_NOPAUSEINMULTI);
			else {
	#ifdef MACINTOSH
				DoMessageBoxAdvanced(TXT(TXI_HLPPAUSE), TXT_PRESSOKTOCONT, TXT_OK, KEY_PAGEDOWN, NULL);
	#else
				DoMessageBoxAdvanced(TXT(TXI_HLPPAUSE), TXT_PRESSOKTOCONT, TXT_OK, KEY_PAUSE, NULL);
	#endif
				//DoMessageBox(TXT(TXI_HLPPAUSE),TXT_PRESSOKTOCONT, MSGBOX_OK);
			}
			break;
		case GAME_LEVEL_WARP:
			Level_warp_next = DisplayLevelWarpDlg(-1);
			if (Level_warp_next != -1)
				SetGameState(GAMESTATE_LVLWARP);
			break;
		case GAME_EXIT_CONFIRM:
			{
				int ret = 0;
				ui_ShowCursor();
				//Weird code for the training mission and the first time you play...
				if(Current_mission.filename && (strcmpi(Current_mission.filename,"training.mn3")==0) && (FirstGame) )
				{
					ret = DoMessageBoxAdvanced(TXT_TRAININGABORTTITLE, TXT_TRAININGABORTTEXT, TXT_SKIP, KEY_S, TXT_ABORT, KEY_A, TXT_CANCEL, KEY_ESC, NULL);
					if(ret==2)
					{
						//Cancel
						ret = 0;
					}
					else if(ret==1)
					{
						//Abort -- back to the main menu
						FirstGame = false;
					}
					else if(ret==0)
					{
						//Skip
						ret = 1;						
					}
					
				}
				else
				{
					ret = DoMessageBox(TXT_CONFIRM,TXT_CONFIRMEXIT,MSGBOX_YESNO);
				}
				if(ret){
					Demo_looping = false;
					if ((Players[Player_num].flags & PLAYER_FLAGS_DYING) || (Players[Player_num].flags & PLAYER_FLAGS_DEAD))
					{
						EndPlayerDeath(Player_num); 
						if (Game_mode & GM_MULTI)
						{
							MultiSendEndPlayerDeath ();
							MultiLeaveGame();
						}
						SetFunctionMode(MENU_MODE);
					}else{
						if (Game_mode & GM_MULTI)
							MultiLeaveGame();
						SetFunctionMode(MENU_MODE);
					}
					Sound_system.StopAllSounds();
				}
			}break;
		case GAME_DLL_INTERFACE:
			{
				DLLInfo.me_handle = DLLInfo.it_handle = OBJECT_HANDLE_NONE;
				ui_ShowCursor();
				CallGameDLL (EVT_CLIENT_SHOWUI,&DLLInfo);
				ui_HideCursor();
			}break;

		case GAME_DEBUGGRAPH_INTERFACE:
			{
				DebugGraph_DisplayOptions();
			}break;

		case GAME_SAVE_INTERFACE:
			ui_ShowCursor();
#ifdef EDITOR
			if (GetFunctionMode() == EDITOR_GAME_MODE) 
				DoMessageBox("Silly", "You can't save while playing from the editor!", MSGBOX_OK);
			else 
#endif	
			SaveGameDialog();							// link to above endif!
			ui_HideCursor();
			break;

		case GAME_LOAD_INTERFACE:
			ui_ShowCursor();
#ifdef EDITOR
			if (GetFunctionMode() == EDITOR_GAME_MODE) 
				DoMessageBox("Silly", "You can't load while playing from the editor!", MSGBOX_OK);
			else 
#endif	
			LoadGameDialog();
			ui_HideCursor();
			break;

		case GAME_POST_DEMO:
#ifdef GAMEGAUGE
			if(0)
#else
			if(!Game_gauge_do_time_test)
#endif
			{
				DemoPostPlaybackMenu();
			}
			SetFunctionMode(MENU_MODE);
			break;
		case GAME_DEMO_LOOP:
			SetFunctionMode(MENU_MODE);
			break;
		default:
			Int3();		//unhandled game interface mode
		}

		if (pause_game) { 
#ifndef MACINTOSH
			ResumeGame();
#else
			StartTime();	//DAJ
			Game_paused = false;
#endif
			pause_game = false;
		}
 
		ResumeControls();
		SetUICallback(NULL);
		Game_interface_mode = GAME_INTERFACE;
		NewUIWindow_alpha = 192;
		Clear_screen = 4;
	}
}

void PageInLevelTexture (int id)
{
	if (id==-1 || id==0)
		return;

	if (Dedicated_server)
		return;

	TouchTexture (id);	

	// Upload all these textures to the card
	if (GameTextures[id].flags & TF_ANIMATED)
	{	
		vclip *vc=&GameVClips[GameTextures[id].bm_handle];
		for (int i=0;i<vc->num_frames;i++)
		{
			if (!(GameTextures[id].flags & TF_PROCEDURAL) && !(GameTextures[id].flags & TF_SPECULAR))
				GameBitmaps[vc->frames[i]].flags|=BF_COMPRESSABLE;
			else
				GameBitmaps[vc->frames[i]].flags&=~BF_COMPRESSABLE;

			if (!Dedicated_server && GameBitmaps[vc->frames[i]].cache_slot==-1)
				rend_PreUploadTextureToCard (vc->frames[i],MAP_TYPE_BITMAP);
		}
	}
	else
	{
		if (!(GameTextures[id].flags & TF_PROCEDURAL) && !(GameTextures[id].flags & TF_SPECULAR))
			GameBitmaps[GameTextures[id].bm_handle].flags|=BF_COMPRESSABLE;
		else
			GameBitmaps[GameTextures[id].bm_handle].flags&=~BF_COMPRESSABLE;

		if (!Dedicated_server && GameBitmaps[GameTextures[id].bm_handle].cache_slot==-1)
			rend_PreUploadTextureToCard (GameTextures[id].bm_handle,MAP_TYPE_BITMAP);
	}
	
	
	if (GameTextures[id].flags & TF_DESTROYABLE && GameTextures[id].destroy_handle!=-1)
		PageInLevelTexture (GameTextures[id].destroy_handle);

	Textures_to_free[id]=1;
}

bool PageInSound (int id)
{
	if (id==-1)
		return false;

	if (Dedicated_server)
		return false;

// sometimes, id passed was 0xffff which seems like a short -1.  The if statement
// ensures that the array Sounds_to_free is dealt with properly.
	if (Sound_system.CheckAndForceSoundDataAlloc(id)) {
		Sounds_to_free[id]=1;
		return true;
	}

	return false;
}

void PageInDoor (int id)
{
	//Set sounds
	door *doorpointer=&Doors[id];

	PageInPolymodel (doorpointer->model_handle);
	Models_to_free[doorpointer->model_handle]=1;

	poly_model *pm=&Poly_models[doorpointer->model_handle];
	for (int t=0;t<pm->n_textures;t++)
		PageInLevelTexture (pm->textures[t]);
	

	if (doorpointer->open_sound!=-1 && doorpointer->open_sound!=SOUND_NONE_INDEX)
		PageInSound (doorpointer->open_sound);
	if (doorpointer->close_sound!=-1 && doorpointer->close_sound!=SOUND_NONE_INDEX)
		PageInSound (doorpointer->close_sound);
}

void PageInWeapon (int id)
{
	weapon *weaponpointer=&Weapons[id];
	
	if (id==-1)
		return;

	int i;
	
	if (!(weaponpointer->flags & (WF_IMAGE_BITMAP|WF_IMAGE_VCLIP)))
	{
		PageInPolymodel (weaponpointer->fire_image_handle);
		Models_to_free[weaponpointer->fire_image_handle]=1;

		poly_model *pm=&Poly_models[weaponpointer->fire_image_handle];
		for (int t=0;t<pm->n_textures;t++)
			PageInLevelTexture (pm->textures[t]);
	}
	
	// Load the various textures associated with this weapon
	if (weaponpointer->explode_image_handle!=-1)
	{
		PageInLevelTexture (weaponpointer->explode_image_handle);
	}

	if (weaponpointer->particle_handle!=-1)
	{
		PageInLevelTexture (weaponpointer->particle_handle);
	}

	if (weaponpointer->smoke_handle!=-1)
	{
		PageInLevelTexture(weaponpointer->smoke_handle);
	}

	if (weaponpointer->scorch_handle!=-1)
	{
		PageInLevelTexture (weaponpointer->scorch_handle);
	}

	if (weaponpointer->icon_handle!=-1)
	{
		PageInLevelTexture (weaponpointer->icon_handle);
	}

	// Try to load spawn weapons
	if (weaponpointer->spawn_handle!=-1 && weaponpointer->spawn_count>0 && weaponpointer->spawn_handle!=id)
	{
		PageInWeapon (weaponpointer->spawn_handle);
	}
	
	if (weaponpointer->alternate_spawn_handle!=-1 && weaponpointer->spawn_count>0 && weaponpointer->alternate_spawn_handle!=id)
	{
		PageInWeapon (weaponpointer->alternate_spawn_handle);
	}

	if (weaponpointer->robot_spawn_handle!=-1)
	{
		PageInGeneric (weaponpointer->robot_spawn_handle);
	}

	// Try and load the various sounds
	for (i=0;i<MAX_WEAPON_SOUNDS;i++)
	{
		if (weaponpointer->sounds[i]!=SOUND_NONE_INDEX)
		{
			PageInSound (weaponpointer->sounds[i]);
		}
	}
}
int paged_in_count = 0;
extern int need_to_page_in;
extern int need_to_page_num;
int paged_in_num = 0;

//#define PAGED_IN_CALC paged_in_count ? (float)paged_in_count/(float)need_to_page_in : 0.0f
#define PAGED_IN_CALC paged_in_num ? (float)paged_in_num/(float)need_to_page_num : 0.0f
void PageInShip (int id)
{
	int i,t;
	
	ship *shippointer=&Ships[id];

	// Page in all textures for this object

	PageInPolymodel (shippointer->model_handle);
	Models_to_free[shippointer->model_handle]=1;


	poly_model *pm=&Poly_models[shippointer->model_handle];

	for (t=0;t<pm->n_textures;t++)
	{
		PageInLevelTexture (pm->textures[t]);
		// Create bumps if neccessary
		if (rend_SupportsBumpmapping())
		{
			if (GameTextures[pm->textures[t]].bumpmap==-1) 
			{
				mprintf ((0,"Trying to make bumpmap!\n"));
				BuildTextureBumpmaps (pm->textures[t]);
			}
		}
	}
	
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	if (shippointer->med_render_handle!=-1)
	{
		PageInPolymodel (shippointer->med_render_handle);
		Models_to_free[shippointer->med_render_handle]=1;

		pm=&Poly_models[shippointer->med_render_handle];
		for (t=0;t<pm->n_textures;t++)
			PageInLevelTexture (pm->textures[t]);
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	if (shippointer->lo_render_handle!=-1)
	{
		PageInPolymodel (shippointer->lo_render_handle);
		Models_to_free[shippointer->lo_render_handle]=1;

		pm=&Poly_models[shippointer->lo_render_handle];
		for (t=0;t<pm->n_textures;t++)
			PageInLevelTexture (pm->textures[t]);
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	if (shippointer->dying_model_handle!=-1)
	{
		PageInPolymodel (shippointer->dying_model_handle);
		Models_to_free[shippointer->dying_model_handle]=1;

		pm=&Poly_models[shippointer->dying_model_handle];
		for (t=0;t<pm->n_textures;t++)
			PageInLevelTexture(pm->textures[t]);
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	// Try and load the various weapons
	int j;
	if(shippointer->static_wb) {
		for(i = 0; i < MAX_PLAYER_WEAPONS; i++)
		{
			for(j = 0; j < MAX_WB_GUNPOINTS; j++)
			{
				if (shippointer->static_wb[i].gp_weapon_index[j] != LASER_INDEX)
				{	
					PageInWeapon (shippointer->static_wb[i].gp_weapon_index[j]);
					LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
				}
			}
		}
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	// Try and load the various weapons
	for(i = 0; i < MAX_PLAYER_WEAPONS; i++)
	{
		for(j = 0; j < MAX_WB_FIRING_MASKS; j++)
		{
			if((j%5)==0)
				LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
			if (shippointer->static_wb[i].fm_fire_sound_index[j] != SOUND_NONE_INDEX)
				PageInSound (shippointer->static_wb[i].fm_fire_sound_index[j]);
		}
	}

	for(i = 0; i < MAX_PLAYER_WEAPONS; i++)
	{
		if((i%5)==0)
			LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
		if (shippointer->firing_sound[i]!=-1)
			PageInSound (shippointer->firing_sound[i]);
		
		if(shippointer->firing_release_sound[i] != -1)
			PageInSound (shippointer->firing_release_sound[i]);
		
		if (shippointer->spew_powerup[i]!=-1)
			PageInGeneric (shippointer->spew_powerup[i]);
		
	}
}

void PageInGeneric (int id)
{
	int i,t;
	
	if (id==-1)
		return;

	object_info *objinfopointer=&Object_info[id];

	// Page in all textures for this object

	PageInPolymodel (objinfopointer->render_handle);
	Models_to_free[objinfopointer->render_handle]=1;

	poly_model *pm=&Poly_models[objinfopointer->render_handle];

	for (t=0;t<pm->n_textures;t++)
	{
		PageInLevelTexture(pm->textures[t]);

		// Create bumps if neccessary
		if (objinfopointer->type==OBJ_ROBOT && rend_SupportsBumpmapping())
		{
			if (GameTextures[pm->textures[t]].bumpmap==-1)
				BuildTextureBumpmaps (pm->textures[t]);
		}
	}

	if (objinfopointer->med_render_handle!=-1)
	{
		PageInPolymodel (objinfopointer->med_render_handle);
		Models_to_free[objinfopointer->med_render_handle]=1;


		pm=&Poly_models[objinfopointer->med_render_handle];
		for (t=0;t<pm->n_textures;t++)
			PageInLevelTexture (pm->textures[t]);
	}

	if (objinfopointer->lo_render_handle!=-1)
	{
		PageInPolymodel (objinfopointer->lo_render_handle);
		Models_to_free[objinfopointer->lo_render_handle]=1;

		pm=&Poly_models[objinfopointer->lo_render_handle];
		for (t=0;t<pm->n_textures;t++)
			PageInLevelTexture (pm->textures[t]);
	}

	// Process all sounds for this object
	for (i=0;i<MAX_OBJ_SOUNDS;i++)
	{
		if (objinfopointer->sounds[i]!=SOUND_NONE_INDEX)
		{
			PageInSound (objinfopointer->sounds[i]);
		}
	}

	if(objinfopointer->ai_info) {
		for (i=0;i<MAX_AI_SOUNDS;i++)
		{
			if (objinfopointer->ai_info->sound[i]!=SOUND_NONE_INDEX)
			{
				PageInSound (objinfopointer->ai_info->sound[i]);
			}
		}
	}
	// Try and load the various wb sounds
	int j;
	if(objinfopointer->static_wb) {
		for(i = 0; i < MAX_WBS_PER_OBJ; i++)
		{
			for(j = 0; j < MAX_WB_FIRING_MASKS; j++)
			{
				if(objinfopointer->static_wb[i].fm_fire_sound_index[j]!=SOUND_NONE_INDEX)
				{
					PageInSound (objinfopointer->static_wb[i].fm_fire_sound_index[j]);
				}
			}
		}
	}
	// Try and load the various wb sounds
	if(objinfopointer->anim) {
		for(i = 0; i < NUM_MOVEMENT_CLASSES; i++)
		{
			for(j = 0; j < NUM_ANIMS_PER_CLASS; j++)
			{
				if(objinfopointer->anim[i].elem[j].anim_sound_index!=SOUND_NONE_INDEX)
				{
					PageInSound (objinfopointer->anim[i].elem[j].anim_sound_index);
				}
			}
		}
	}

	// Load the spew types
	for(i=0;i<MAX_DSPEW_TYPES;i++)
	{
		if (objinfopointer->dspew_number[i]>0 && objinfopointer->dspew[i]!=0 && objinfopointer->dspew[i]!=id)
		{
			PageInGeneric (objinfopointer->dspew[i]);
		}
	}

	// Try and load the various weapons

	// Automatically include laser
	PageInWeapon (LASER_INDEX);

	if(objinfopointer->static_wb) {
		for(i = 0; i < MAX_WBS_PER_OBJ; i++)
		{
			for(j = 0; j < MAX_WB_GUNPOINTS; j++)
			{
				if (objinfopointer->static_wb[i].gp_weapon_index[j]!=LASER_INDEX)
				{
					PageInWeapon (objinfopointer->static_wb[i].gp_weapon_index[j]);
				}
			}
		}
	}
}

extern char *Static_sound_names[];



void PageInAllData ()
{
	int i;
	paged_in_count = 0;
	paged_in_num = 0;
	memset (Textures_to_free,0,MAX_TEXTURES);
	memset (Sounds_to_free,0,MAX_SOUNDS);
	memset (Models_to_free,0,MAX_POLY_MODELS);
	
	PageInShip (Players[Player_num].ship_index);
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	/*
	$$TABLE_TEXTURE "LightFlareStare"
	$$TABLE_TEXTURE "LightFlare"
	*/
	PageInLevelTexture (FindTextureName(IGNORE_TABLE("LightFlareStar")));
	PageInLevelTexture (FindTextureName(IGNORE_TABLE("LightFlare")));

	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	
	if (PreferredRenderer==RENDERER_DIRECT3D)
	{
		if(LightmapInfo && !Dedicated_server)
		{
			if (!NoLightmaps)
			{
				for (i=0;i<MAX_LIGHTMAP_INFOS;i++)
				{
					if (LightmapInfo[i].used && LightmapInfo[i].type!=LMI_DYNAMIC)
					{
						if (!Dedicated_server && GameLightmaps[LightmapInfo[i].lm_handle].cache_slot==-1)
						{
							rend_PreUploadTextureToCard (LightmapInfo[i].lm_handle,MAP_TYPE_LIGHTMAP);
						}
					}
				}
			}
		}
	}

	// Get static fireballs
	for (i=0;i<NUM_FIREBALLS;i++)
	{
		char name[PAGENAME_LEN];
		strcpy (name,Fireballs[i].name);

		name[strlen(name)-4]=0;
		int id=FindTextureName (name);
		if (id!=-1)
			PageInLevelTexture (id);
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	// Get static sounds
	for (i=0;i<NUM_STATIC_SOUNDS;i++)
	{
		int sid=FindSoundName(Static_sound_names[i]);

		if (sid!=-1)
			PageInSound(sid);
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	
	// First get textures	
	for (i=0;i<=Highest_room_index;i++)
	{
		if((i%15)==0)
			LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
			
		if (!Rooms[i].used)
			continue;

		room *rp=&Rooms[i];
		for (int t=0;t<rp->num_faces;t++)
		{
			PageInLevelTexture(rp->faces[t].tmap);
		}
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);

	// Touch all terrain textures
	for (i=0;i<TERRAIN_TEX_WIDTH*TERRAIN_TEX_DEPTH;i++)
	{
		PageInLevelTexture (Terrain_tex_seg[i].tex_index);
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
	if (Terrain_sky.textured)
	{
		PageInLevelTexture (Terrain_sky.dome_texture);
	}
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);	
	for (i=0;i<Terrain_sky.num_satellites;i++)
		PageInLevelTexture (Terrain_sky.satellite_texture[i]);
	
	// Touch all objects
	for (i=0;i<=Highest_object_index;i++)
	{
		if((i%20)==0)
			LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA,PAGED_IN_CALC);
		object *obj=&Objects[i];
		if (obj->type==OBJ_POWERUP || obj->type==OBJ_ROBOT || obj->type==OBJ_CLUTTER || obj->type==OBJ_BUILDING)
		{
			PageInGeneric (obj->id);
			continue;
		}

		if (obj->type==OBJ_DOOR)
		{
			PageInDoor (obj->id);
			continue;
		}
	}
	LoadLevelProgress(LOAD_PROGRESS_PREPARE,0);
} 
