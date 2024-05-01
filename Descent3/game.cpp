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

#include "game.h"
#include "ddvid.h"
#include "ddio.h"
#include "pserror.h"
#include "program.h"
#include "descent.h"
#include "object.h"
#include "trigger.h"
#include "player.h"
#include "slew.h"
#include "controls.h"
#include "renderer.h"
#include "doorway.h"
#include "hud.h"
#include "multi.h"
#include "gamefont.h"
#include "newui.h"
#include "gamesequence.h"
#include "cinematics.h"
#include "SmallViews.h"
#include "Mission.h"
#include "CFILE.H"
#include "gameloop.h"
#include "cockpit.h"
#include "game2dll.h"
#include "config.h"
#include "stringtable.h"
#include "ship.h"
#include "pilot.h"
#include "args.h"
#include "gamepath.h"
#include "AIGoal.h"
#include "aipath.h"
#include "dedicated_server.h"
#include "objinfo.h"
#include <string.h>
#include "osiris_share.h"
#include "demofile.h"

///////////////////////////////////////////////////////////////////////////////
//	Variables

//Vars for game 3D window
int Game_window_x,Game_window_y,Game_window_w,Game_window_h;
int Max_window_w,Max_window_h;

// The game mode we're in (ie multiplayer vs. single, etc)
int Game_mode=0;

int sound_override_force_field = -1;
int sound_override_glass_breaking = -1;

int   force_field_bounce_texture[MAX_FORCE_FIELD_BOUNCE_TEXTURES] = {-1, -1, -1};
float force_field_bounce_multiplier[MAX_FORCE_FIELD_BOUNCE_TEXTURES] = {1.0f, 1.0f, 1.0f};

bool Level_powerups_ignore_wind = false;

//what renderer?
renderer_type PreferredRenderer=RENDERER_OPENGL;

// Rendering options
rendering_state Render_state;
renderer_preferred_state Render_preferred_state;
int Render_preferred_bitdepth;

// How hard is this game?
int Difficulty_level=0;

#ifdef _DEBUG
int Game_show_sphere = 0;
int Game_show_portal_vis_pnts = 0;
int Game_update_attach = 1;
int Game_do_walking_sim = 1;
int Game_do_vis_sim = 1;
int Game_do_flying_sim = 1;
int Game_do_ai_movement = 1;
int Game_do_ai_vis = 1;
#endif

//	How much of the mine has been explored?
int Num_rooms_explored = 0;

//	Save and restores per level
int Num_player_saves = 0;
int Num_player_restores = 0;

//Missile camera
int Missile_camera_window = SVW_LEFT;		//will default to -1 when interface is in

//	contains all relevent information for gamemode pertaining to d3x system.
gamemode Gamemode_info;


///////////////////////////////////////////////////////////////////////////////
//	Game setup
///////////////////////////////////////////////////////////////////////////////

bool InitGameScript();
void CloseGameScript();

float GetFPS ()
{
	if (Frametime == 0.0f) { Frametime = 0.1f; }
	return 1.0 / Frametime;
}


///////////////////////////////////////////////////////////////////////////////
//	Game Execution
///////////////////////////////////////////////////////////////////////////////


//Setup the game screen
void InitGameScreen(int w, int h)
{
	if (w > Max_window_w) w = Max_window_w;
	if (h > Max_window_h) h = Max_window_h;

	Game_window_w = w;
	Game_window_h = h;

	Game_window_x = (Max_window_w - Game_window_w) / 2;
	Game_window_y = (Max_window_h - Game_window_h) / 2;
}

//Setup whatever needs to be setup for game mode
bool InitGame()
{
	#ifdef _DEBUG
	//Put player in flying mode
	SlewStop(Player_object);
	#endif

	InitHUD();

	if(!InitGameScript())						// initializes game script
		return false;
	
	Frametime=0.1f;
	Skip_render_game_frame = false;

// reset controllers.
	Controller->mask_controllers((Current_pilot.read_controller&READF_JOY)?true:false, 
		(Current_pilot.read_controller&READF_MOUSE)?true:false);

	return true;
}

#ifdef EDITOR
bool Game_being_played_from_quick_play = false;
void QuickPlayGame()
{
	if(InitGame()){
		Game_being_played_from_quick_play = true;
		QuickStartMission();

		//	Run the game (note, if this call returns false, we couldn't play a level. Display an error maybe?
		GameSequencer();
		QuickEndMission();		
	}else{

	}
	Game_being_played_from_quick_play = false;

//	Close down some game stuff
//	close down any systems not needed outside game.
	CloseGameScript();
	CloseHUD();
	DoorwayDeactivateAll();		// deactivate doorways
	ui_RemoveAllWindows();			// remove any ui windows left open.
	SuspendControls();
}
#endif

void PlayGame()
{
//	Initialize misc game 
	if(InitGame()){
		//	Run the game (note, if this call returns false, we couldn't play a level. Display an error maybe?
		GameSequencer();
	}else{
		SetFunctionMode(MENU_MODE);

		//if they were going into a multiplayer game than we need to handle cleaning all that up
		if(Game_mode&GM_MULTI){
			SetGameMode(GM_NORMAL);
			for(int i=0;i<MAX_PLAYERS;i++){
				NetPlayers[i].flags &= ~NPF_CONNECTED;
			}
		}
	}

	//	Close down some game stuff
	//	close down any systems not needed outside game.

	// we must pop the pages before closing the mission, since we need to have the mn3 hog file open
	mng_ClearAddonTables();

	ResetMission();
	CloseGameScript();	
	CloseHUD();
	//DoorwayDeactivateAll();		// deactivate doorways
	ui_RemoveAllWindows();			// remove any ui windows left open.
	SuspendControls();
}


///////////////////////////////////////////////////////////////////////////////
// Sets the game mode.  this will reinitialize the game mode script.
///////////////////////////////////////////////////////////////////////////////

void SetGamemodeScript(const char *scrfilename,int num_teams)
{
	if (scrfilename!=NULL)
		strcpy(Gamemode_info.scriptname, scrfilename);
	else
		Gamemode_info.scriptname[0]=0;
	Gamemode_info.requested_num_teams = num_teams;
}

void RenderBlankScreen(void);
bool InitGameScript()
{
//	initialize gamemode script here.
	if (Gamemode_info.scriptname[0]) {
//@@		char d3xname[255];
		char dllname[255];

		sprintf (dllname,"%s",Gamemode_info.scriptname);
		
		if(!LoadGameDLL (dllname,Gamemode_info.requested_num_teams)){
			if(!Dedicated_server){
				void (*old_callback)();
				old_callback = GetUICallback();
				SetUICallback(RenderBlankScreen);
				ShowProgressScreen (TXT_LOADMODULEERR);
				DoMessageBox(TXT_ERROR,TXT_INITMODULEERR,MSGBOX_OK);
				SetUICallback(old_callback);
			}
			return false;
		}	
	}
	return true;
}


void CloseGameScript()
{
//	free any gamemode info.
	FreeGameDLL();
}



//	call this to set the game mode
void SetGameMode(int mode)
{
//	do any gamemode specific code here.

	Game_mode = mode;
}


///////////////////////////////////////////////////////////////////////////////
//	Sets screen mode
///////////////////////////////////////////////////////////////////////////////

static int Screen_mode = SM_NULL;

int GetScreenMode() 
{
	return Screen_mode;
}

//	use to sync to debug break handlers
int rend_initted = 0;
int Low_vidmem=0;

void SetScreenMode(int sm, bool force_res_change)
{
	static int old_sm = SM_NULL;
	static int rend_width=0,rend_height=0;
	rendering_state rs;

	if( sm == SM_CINEMATIC )
	{
		// force cinematic to menu
		sm = SM_MENU;
	}

	if (Dedicated_server)
		return;

	if (old_sm == sm && !force_res_change) 
		return;

//	close down any systems previously opened and that must be closed (like software->hardware, etc.)
//	make sure renderer is initialized
//	also set any preferred renderer states.
	if (sm == SM_NULL) {			// || (sm == SM_CINEMATIC && Renderer_type == RENDERER_OPENGL)) {
		if (rend_initted) {
			rend_Close();
			rend_initted = 0;
		}
	}
	else if (sm == SM_CINEMATIC) {// && (Renderer_type == RENDERER_OPENGL || Renderer_type == RENDERER_DIRECT3D)) {
		if (rend_initted) {
			rend_Close();
			rend_initted = 0;
		}
	}
//#ifdef RELEASE
//	else if (sm == SM_CINEMATIC && (Renderer_type == RENDERER_GLIDE) ) {
//		if (rend_initted) {
//			rend_Close();
//			rend_initted = 0;
//		}
//	}
//#endif
	else {
		int scr_width, scr_height, scr_bitdepth;

		if (sm == SM_GAME) 
		{
			//scr_width = Video_res_list[Game_video_resolution].width;
			//scr_height = Video_res_list[Game_video_resolution].height;
			scr_width = Game_window_res_width;
			scr_height = Game_window_res_height;
			scr_bitdepth = Render_preferred_bitdepth;
#ifdef MACINTOSH
			SwitchDSpContex(Game_video_resolution);
#endif
		}
		else 
		{
			scr_width = FIXED_SCREEN_WIDTH;
			scr_height = FIXED_SCREEN_HEIGHT;
			scr_bitdepth=16;
#ifdef MACINTOSH
			SwitchDSpContex(0);
#endif
		}

		if (!rend_initted) {

			Render_preferred_state.width=scr_width;
			Render_preferred_state.height=scr_height;
			Render_preferred_state.bit_depth=scr_bitdepth;
			Render_preferred_state.window_width = Game_window_res_width;
			Render_preferred_state.window_height = Game_window_res_height;
			Render_preferred_state.fullscreen = Game_fullscreen;

			rend_initted = rend_Init (PreferredRenderer, Descent,&Render_preferred_state);
			rend_width = rend_height = 0;
		}
		else {

			//If bitdepth changed but not initting, switch bitdepth
			if (Render_preferred_state.bit_depth != scr_bitdepth) {
				Render_preferred_state.bit_depth = scr_bitdepth;
				rend_SetPreferredState(&Render_preferred_state);
			}
		}

		if (!rend_initted) {
			Error(rend_GetErrorMessage());
		}
		else {
			int t=FindArg ("-ForceStateLimited");
			if (t) {
				StateLimited = (atoi((const char *)GameArgs[t+1]) != 0);
			}
	

			if (rend_initted==-1)
			{
			// We're using the default, so change some values for the menus
				rend_initted=1;
				mprintf ((0,"Changing menu settings to default!\n"));
				Game_video_resolution = RES_640X480;
				Render_preferred_state.bit_depth=16;
				scr_width=640;
				scr_height=480;
			}
		
			if (rend_width != scr_width || rend_height != scr_height
				|| Game_window_res_width != Render_preferred_state.window_width
				|| Game_window_res_height != Render_preferred_state.window_height
				|| Game_fullscreen != Render_preferred_state.fullscreen) 
			{
				Render_preferred_state.width=scr_width;
				Render_preferred_state.height=scr_height;
				Render_preferred_state.bit_depth=scr_bitdepth;
				Render_preferred_state.window_width = Game_window_res_width;
				Render_preferred_state.window_height = Game_window_res_height;
				Render_preferred_state.fullscreen = Game_fullscreen;

				mprintf ((0,"Setting rend_width=%d height=%d\n",scr_width,scr_height));
				int retval=rend_SetPreferredState (&Render_preferred_state);
				
				if (retval==-1)
				{
					// We're using the default, so change some values for the menus
					rend_initted=1;
					mprintf ((0,"Changing menu settings to default!\n"));
					Game_video_resolution = RES_640X480;
					Render_preferred_state.bit_depth=16;
					scr_width=640;
					scr_height=480;
					Render_preferred_state.width=scr_width;
					Render_preferred_state.height=scr_height;
				}
			}
		}
	}

	if (rend_initted) {
	// Get the amount of video memory
		Low_vidmem=rend_LowVidMem ();

		if (FindArg ("-hividmem"))
			Low_vidmem=0;
		
	//	get current render width and height.
		rend_GetRenderState (&rs);
		rend_width=rs.screen_width;
		rend_height=rs.screen_height;

	//	sets up the screen resolution for the system
		if (!UseHardware) {
			ddvid_SetVideoMode(rend_width,rend_height,BPP_16, true);
		}
		else {
			if (PreferredRenderer==RENDERER_OPENGL) {
				ddvid_SetVideoMode(rend_width,rend_height,BPP_16, false);
			}
		}

	//	chose font.
		SelectHUDFont(rend_width);

	//Setup the screen
		Max_window_w = rend_width;
		Max_window_h = rend_height;

		InitGameScreen(Max_window_w, Max_window_h);

	//	initialize ui system again
		ui_SetScreenMode(Max_window_w, Max_window_h);

		mprintf ((0,"rend_width=%d height=%d\n",Max_window_w,Max_window_h));
	}

//	assign current screen mode
	Screen_mode = sm;
	old_sm = sm;

//	Adjust mouse mapping to current screen 
//	do screen mode stuff
	switch (sm)
	{
		case SM_GAME:
		{
			ui_HideCursor();
			SetUICallback(NULL);
			int gw,gh;
			Current_pilot.get_hud_data(NULL,NULL,NULL,&gw,&gh);
			if (force_res_change) {
				gw = Max_window_w;
				gh = Max_window_h;
			}
			InitGameScreen(gw, gh);
			// need to do this since the pilot w,h could change.
			Current_pilot.set_hud_data(NULL,NULL,NULL,&Game_window_w,&Game_window_h);
			break;
		}
					
		case SM_MENU:
		{
		//	sets up the menu screen
			SetUICallback(DEFAULT_UICALLBACK);
			ui_ShowCursor();
			break;
		}

		case SM_CINEMATIC:
		{
			SetMovieProperties(0,0, FIXED_SCREEN_WIDTH, FIXED_SCREEN_HEIGHT, (rend_initted) ? Renderer_type : RENDERER_NONE);
			break;
		}

		case SM_NULL:
		{
		//	cleans up
			return;
		}
	}

	mprintf ((0,"NEW rend_width=%d height=%d\n",Max_window_w,Max_window_h));

//	mark res change as false.

#ifdef EDITOR
	extern unsigned hGameWnd;
//	HACK!!! In editor, to get things working fine, reassert window handle attached to game screen
//	is the topmost window, since in the editor, if we're fullscreen the parent window is still
//	the editor window, the screen would belong to the editor window.
	tWin32AppInfo appinfo;
	Descent->get_info(&appinfo);
	ddvid_SetVideoHandle(hGameWnd);
#endif
}	


//	These functions are called to start and end a rendering frame
typedef struct tFrameStackFrame
{
	int x1,x2,y1,y2;
	bool clear;
	tFrameStackFrame *next;
	tFrameStackFrame *prev;
}tFrameStackFrame;
tFrameStackFrame *FrameStackRoot = NULL;
tFrameStackFrame *FrameStackPtr = NULL;
tFrameStackFrame FrameStack[8];
int FrameStackDepth = 0;

void FramePush(int x1,int y1,int x2,int y2,bool clear)
{
	tFrameStackFrame *curr = FrameStackPtr;

	if(!curr){
		ASSERT( !FrameStackRoot );

		//we need to allocate for the root
//		curr = FrameStackRoot = FrameStackPtr = (tFrameStackFrame *)mem_malloc(sizeof(tFrameStackFrame));
		curr = FrameStackRoot =  FrameStackPtr = &FrameStack[0];
		if(!curr){
			Error("Out of memory\n");
		}

		curr->prev = NULL;
		curr->next = NULL;
	}else{
		//add on to the end of the list
		curr->next =  FrameStackPtr = &FrameStack[FrameStackDepth];
//		curr->next = FrameStackPtr = (tFrameStackFrame *)mem_malloc(sizeof(tFrameStackFrame));
		if(!curr->next){
			Error("Out of memory\n");
		}
		curr->next->prev = curr;	//setup previous frame
		curr = curr->next;
		curr->next = NULL;
	}

	//at this point curr should be a valid frame, with prev and next set
	curr->x1 = x1;
	curr->x2 = x2;
	curr->y1 = y1;
	curr->y2 = y2;
	curr->clear = clear;
	FrameStackDepth++;
	//DAJ
	if(FrameStackDepth > 7) {
		mprintf((2, "FrameStack Overflow\n"));
		Int3();
	}
}

void FramePop(int *x1,int *y1,int *x2,int *y2,bool *clear)
{
	if(!FrameStackRoot || !FrameStackPtr){
		mprintf((0,"StartFrame/EndFrame mismatch\n"));
		Int3();
		*clear = true;
		*x1 = Game_window_x;
		*y1 = Game_window_y;
		*x2 = *x1 + Game_window_w;
		*y2 = *y1 + Game_window_h;
		return;
	}

	tFrameStackFrame *frame = FrameStackPtr;

	*x1 = FrameStackPtr->x1;
	*x2 = FrameStackPtr->x2;
	*y1 = FrameStackPtr->y1;
	*y2 = FrameStackPtr->y2;
	*clear = FrameStackPtr->clear;

	if(frame==FrameStackRoot){
		//we're popping off the root
//DAJ		mem_free(FrameStackRoot);
		FrameStackRoot = NULL;
		FrameStackPtr = NULL;
	}else{
		//we're just going back a frame, but still have a stack
		FrameStackPtr = FrameStackPtr->prev;	//pop back a frame
		FrameStackPtr->next = NULL;
//DAJ		mem_free(frame);
	}
	FrameStackDepth--;
}

//peeks at the current frame
// returns false if there is no current frame
bool FramePeek(int *x1,int *y1,int *x2,int *y2,bool *clear)
{
	if(!FrameStackPtr)
		return false;

	*x1 = FrameStackPtr->x1;
	*x2 = FrameStackPtr->x2;
	*y1 = FrameStackPtr->y1;
	*y2 = FrameStackPtr->y2;
	*clear = FrameStackPtr->clear;
	return true;
}

void StartFrame(bool clear)
{
	StartFrame(Game_window_x, Game_window_y, Game_window_x+Game_window_w,Game_window_y+Game_window_h, clear);
}

constexpr float ASPECT_4_3 = (4.f / 3.f);

void StartFrame(int x, int y, int x2, int y2, bool clear,bool push_on_stack)
{
	static float last_fov=-1;
	//if (last_fov!=Render_FOV)
	{
		float num=(Render_FOV / 2);
		num=(3.14*(float)num/180.0);
		Render_zoom=tan(num);

		last_fov=Render_FOV;
	}

//	for software renderers perform frame buffer lock.
	if (Renderer_type == RENDERER_SOFTWARE_16BIT) {
		int w, h, color_depth, pitch;
		ubyte *data;

		ddvid_GetVideoProperties(&w, &h, &color_depth);
		ddvid_LockFrameBuffer(&data, &pitch);
		rend_SetSoftwareParameters(ddvid_GetAspectRatio(), w, h, pitch, data);
	}

	if(push_on_stack)
	{
		//push this frame onto the stack
		FramePush(x,y,x2,y2,clear);
	}

	rend_StartFrame (x,y,x2,y2);
	if (Renderer_type == RENDERER_SOFTWARE_16BIT && clear) { 
		rend_FillRect(GR_RGB(0,0,0), x,y,x2,y2);
	}
	grtext_SetParameters(0,0,(x2-x),(y2-y));	
}

// retrives the settings of the last call to StartFrame
bool GetFrameParameters(int *x1,int *y1,int *x2,int *y2)
{
	return false;
	/*
	if(!Frame_inside)
		return false;
	*x1 = Frame_x1;
	*y1 = Frame_y1;
	*x2 = Frame_x2;
	*y2 = Frame_y2;
	return true;
	*/
}


void EndFrame()
{
	//@@Frame_inside = false;
	rend_EndFrame();

//	for software renderers perform unlock on frame buffer.
	if (Renderer_type == RENDERER_SOFTWARE_16BIT) {
		ddvid_UnlockFrameBuffer();
	}

	//pop off frame
	int x1,x2,y1,y2;
	bool clear;
	
	FramePop(&x1,&y1,&x2,&y2,&clear);	//pop off frame just ending

	//see if there is a frame on the stack...if so, restore it's settings
	if(FramePeek(&x1,&y1,&x2,&y2,&clear))
	{
		//restore this frame
		StartFrame(x1,y1,x2,y2,clear,false);
	}
}

// Does a screenshot and tells the bitmap lib to save out the picture as a tga
void DoScreenshot ()
{
	int bm_handle;
	int count;
	char str[255],filename[255];
	CFILE *infile;
	int done=0;
	int width=640,height=480;

	if (UseHardware)
	{
		rendering_state rs;
		rend_GetRenderState (&rs);
		width=rs.screen_width;
		height=rs.screen_height;
	}


	bm_handle=bm_AllocBitmap (width,height,0);
	if (bm_handle<0)
	{
		AddHUDMessage(TXT_ERRSCRNSHT);
		return;
	}

	StopTime();

	// Tell our renderer lib to take a screen shot
	rend_Screenshot (bm_handle);

	// Find a valid filename
	count=1;
	while (!done)
	{
		sprintf (str,"Screenshot%.3d.tga",count);
		ddio_MakePath (filename,User_directory,str,NULL);
		infile=(CFILE *)cfopen (filename,"rb");
		if (infile==NULL)
		{
			done=1;
			continue;
		}
		else
			cfclose (infile);

		count++;
		if(count>999)
			break;
	}
	
	strcpy (GameBitmaps[bm_handle].name,str);

	// Now save it
	bm_SaveBitmapTGA (filename,bm_handle);
	if(Demo_flags != DF_PLAYBACK)
	{
		AddHUDMessage (TXT_SCRNSHT,filename);
	}

	// Free memory			
	bm_FreeBitmap (bm_handle);

	StartTime();
}

#include "TelCom.h"

bool ShouldCaptureMouse()
{
	if (Dedicated_server)
		return false;

	if (!Descent->active())
		return false; //Never grab when the app isn't active

	if (GetFunctionMode() != GAME_MODE)
		return false; //Never grab outside of game

	//This needs to be higher priority than Telcom overall
	if (TelComGetSystem() == TS_MAP)
		return true; //Always grab when the map is open, since it uses normal flight controls.

	if (TelComIsActive())
		return false; //Never grab while the Telcom system is up otherwise. 

	if (ui_IsCursorVisible())
		return false; //If a UI is visible, don't capture the mouse

	return true;
}

