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
// Initialization routines for Descent3/Editor

#include <stdlib.h>

#include "mono.h"
#include "application.h"
#include "gametexture.h"
#include "object.h"
#include "vecmat.h"
#include "init.h"
#include "config.h"
#include "3d.h"
#include "hlsoundlib.h"
#include "manage.h"
#include "bitmap.h"
#include "ddio.h"
#include "joystick.h"
#include "render.h"
#include "descent.h"
#include "renderer.h"
#include "vclip.h"
#include "grdefs.h"
#include "pserror.h"
#include "lighting.h"
#include "program.h"
#include "polymodel.h"
#include "door.h"
#include "terrain.h"
#include "soundload.h"
#include "ship.h"
#include "controls.h"
#include "texture.h"
#include "Mission.h"
#include "findintersection.h"
#include "appdatabase.h"
#include "AppConsole.h"
#include "room.h"
#include "game.h"
#include "render.h"
#include "gamefile.h"
#include "TelCom.h"
#include "objinfo.h"
#include "ObjScript.h"
#include "cinematics.h"
#include "lightmap_info.h"
#include "fireball.h"
#include "networking.h"
#include "args.h"
#include "pilot.h"
#include "d3serial.h"
#include "gameloop.h"
#include "trigger.h"
#include "PHYSICS.H"
#include "special_face.h"
#include "streamaudio.h"
#include "voice.h"
#include "localization.h"
#include "stringtable.h"
#include "hlsoundlib.h"
#include "player.h"
#include "ambient.h"
#include "matcen.h"
#include "dedicated_server.h"
#include "D3ForceFeedback.h"
#include "newui.h"
#include "SmallViews.h"
#include "uisys.h"
#include "rtperformance.h"
#include "d3music.h"
#include "PilotPicsAPI.h"
#include "osiris_dll.h"
//#include "gamespy.h"
#include "mem.h"
#include "multi.h"
#include "marker.h"
#include "gamecinematics.h"
#include "debuggraph.h"
#include "rocknride.h"
#include "vibeinterface.h"

//Uncomment this for all non-US versions!!
//#define LASERLOCK

//Uncomment this to allow all languages
#define ALLOW_ALL_LANG	1

#if defined(WIN32) && defined(LASERLOCK)
#include "laserlock.h"
#endif

#ifdef EDITOR
	#include "editor\HFile.h"
	#include "editor\d3edit.h"
	#include "slew.h"
	#include "gr.h"
	#define INIT_MESSAGE(c) SplashMessage c	
#else
	void IntroScreen();
	#define INIT_MESSAGE(c)	InitMessage(c)
#endif

#if defined(EDITOR)||defined(NEWEDITOR)
bool Running_editor = true;//didn't we have a variable like this somewhere
#else
bool Running_editor = false;//didn't we have a variable like this somewhere
#endif

static bool Init_in_editor = false;

// used to update load bar.
void SetInitMessageLength(char *c, float amount);	// portion of total bar to fill (0 to 1)
void UpdateInitMessage(float amount);					// amount is 0 to 1
void SetupTempDirectory(void);
void DeleteTempFiles(void);


#define TEMPBUFFERSIZE	256

//If there's a joystick, this is the stick number.  Else, this is -1
char App_ddvid_subsystem[8];

//	other info.
static chunked_bitmap Title_bitmap;
static bool Init_systems_init = false;
static bool Graphics_init = false;
static bool Title_bitmap_init = false;
ubyte Use_motion_blur = 0;

// The "root" directory of the D3 file tree
char Base_directory[_MAX_PATH];

extern int Min_allowed_frametime;

extern bool Mem_low_memory_mode;
extern bool Render_powerup_sparkles;
extern int Use_file_xfer;

extern bool Mem_superlow_memory_mode;

const float kDefaultMouselookSensitivity = 9.102f;
const float kAnglesPerDegree             = 65536.0f / 360.0f;
int CD_inserted = 0;
float Mouselook_sensitivity = kAnglesPerDegree * kDefaultMouselookSensitivity;
float Mouse_sensitivity     = 1.0f;

int IsLocalOk(void)
{
#ifdef WIN32	
#ifdef ALLOW_ALL_LANG
	return 1;
#endif
	
	switch (PRIMARYLANGID(GetSystemDefaultLangID())) 
	{ 
	case LANG_JAPANESE:
		/* Japanese */
		return 0;
#ifndef LASERLOCK
	//If no laserlock that means this is the US distribution
	case LANG_ENGLISH:
		if( (SUBLANG_ENGLISH_US!=SUBLANGID(GetSystemDefaultLangID())) && (SUBLANG_ENGLISH_CAN!=SUBLANGID(GetSystemDefaultLangID())) )
			return 0;
		else
			return 1;
		break;
	default:
		//Non-japanese or english version 
		return 0;
#endif
	}
#endif
	return 1;
}



void PreGameCdCheck()
{
	CD_inserted = 0;
	do
	{
		char *p = NULL;
#if defined (MACINTOSH)
		p = ddio_GetCDDrive("Descent3");
		if(p && *p)
		{
			CD_inserted = 1;			
			break;
		}
#elif defined  (OEM)
		p = ddio_GetCDDrive("D3OEM_1");
		if(p && *p)
		{
			CD_inserted = 1;			
			break;
		}
#else
		p = ddio_GetCDDrive("D3_DVD");
		if(p && *p)
		{
			CD_inserted = 3;			
			break;
		}
		p = ddio_GetCDDrive("D3_1");
		if(p && *p)
		{
			CD_inserted = 1;			
			break;
		}
		p = ddio_GetCDDrive("D3_2");
		if(p && *p)
		{
			CD_inserted = 2;			
			break;
		}
#endif

		if(!CD_inserted)
		{
#if defined(WIN32)
			char message_txt[50];
			sprintf(message_txt,TXT_CDPROMPT,1);
			ShowCursor(true);
			HWND dwnd = FindWindow(NULL,PRODUCT_NAME);
			if(MessageBox(dwnd,message_txt,PRODUCT_NAME,MB_OKCANCEL)==IDCANCEL)
			{
				exit(0);	
			}
			ShowCursor(false);

#elif defined(MACINTOSH)
		::ShowCursor();
		short action = ::Alert(130, NULL);
		if(action == 1)
			::ExitToShell();
#elif defined(__LINUX__)
			// ummm what should we do in Linux?
			// right now I'm just going to return this hopefully will be safe, as I think
			// this function is only really used for a copy protection check of sorts, if they
			// don't have the CD in on start, then they don't get the intro movie
			return;
#endif
		}

	}while(!CD_inserted);
}


/*
	Initializes all the subsystems that need to be initialized BEFORE application creation.
	Returns 1 if all is good, 0 if something is wrong
*/
void PreInitD3Systems()
{
// initialize error system
	bool debugging = false, console_output = false;

	#ifndef RELEASE

		debugging = (FindArg("-debug") != 0);

		#ifdef MONO
			console_output = true;
		#endif
		#ifndef MACINTOSH
			if (FindArg("-logfile")) 
				Debug_Logfile("d3.log");
		#endif

	#endif


#ifdef DAJ_DEBUG
	debugging = true;
#endif

	error_Init(debugging, console_output, PRODUCT_NAME);

	#ifndef EDITOR
	{
		int serial_error = SerialCheck();	//determine if its ok to run based on serialization
	
		if(serial_error) {
			SerialError(serial_error);
			exit(1);
		}
	}
#endif
	if(FindArg("-lowmem"))
		Mem_low_memory_mode = true;
	if(FindArg("-superlowmem"))
		Mem_low_memory_mode = Mem_superlow_memory_mode = true;
	if(FindArg("-dedicated"))
		Mem_low_memory_mode = true;
#ifndef MACINTOSH
	mem_Init();
#endif
	if(FindArg("-himem"))
	{
		Mem_low_memory_mode = false;
		Mem_superlow_memory_mode = false;
	}
	//For the client and the server, this turns off the bitmap exchange system.
	if(FindArg("-nomultibmp"))
		Use_file_xfer = 0;
	
	int iframelmtarg = FindArg("-limitframe");
	if(iframelmtarg)
	{
		Min_allowed_frametime = atoi(GameArgs[iframelmtarg+1]);
		mprintf((0,"Using %d as a minimum frametime\n",Min_allowed_frametime));
	}
	else
	{
		if(FindArg("-dedicated"))
			Min_allowed_frametime=30;
		else
			Min_allowed_frametime=0;
	}
	iframelmtarg = FindArg("-framecap");
	if(iframelmtarg)
	{
		Min_allowed_frametime = ((float)1.0/(float)atoi(GameArgs[iframelmtarg+1]))*1000;
		mprintf((0,"Using %d as a minimum frametime\n",Min_allowed_frametime));
	}
	else
	{
		// Default to a framecap of 60
		Min_allowed_frametime = (1.0/60.0)*1000;
		mprintf ((0,"Using default framecap of 60\n"));
	}

	//Mouselook sensitivity!
	int msensearg = FindArg("-mlooksens");
	if(msensearg)
	{
		Mouselook_sensitivity = kAnglesPerDegree * atof( GameArgs[msensearg+1] );
		mprintf((0,"Using mouselook sensitivity of %f\n",Mouselook_sensitivity));
	}

	//Mouse sensitivity (non-mouselook)
	msensearg = FindArg("-mousesens");
	if(msensearg)
	{
		Mouse_sensitivity = atof(GameArgs[msensearg+1]);
		mprintf((0,"Using mouse sensitivity of %f\n",Mouse_sensitivity));
	}

	grtext_Init();

	#ifndef RELEASE
	SetMessageBoxTitle(PRODUCT_NAME " Message");
	#endif
}


/*
	Save game variables to the registry
*/
void SaveGameSettings()
{
	char tempbuffer[TEMPBUFFERSIZE];
	int tempint;

	sprintf(tempbuffer,"%f",Render_preferred_state.gamma);
	Database->write("RS_gamma",tempbuffer,strlen(tempbuffer)+1);

	sprintf(tempbuffer,"%f",Sound_system.GetMasterVolume());
	Database->write("SND_mastervol",tempbuffer,strlen(tempbuffer)+1);

	sprintf(tempbuffer,"%f",D3MusicGetVolume());
	Database->write("MUS_mastervol",tempbuffer,strlen(tempbuffer)+1);

	sprintf(tempbuffer,"%f",Detail_settings.Pixel_error);
	Database->write("RS_pixelerror",tempbuffer,strlen(tempbuffer)+1);

	sprintf(tempbuffer,"%f",Detail_settings.Terrain_render_distance/((float)TERRAIN_SIZE));
	Database->write("RS_terraindist",tempbuffer,strlen(tempbuffer)+1);

	Database->write("Dynamic_Lighting",Detail_settings.Dynamic_lighting);

#ifdef _DEBUG
	Database->write("Outline_mode",Outline_mode);
	Database->write("Lighting_on",Lighting_on);
	Database->write("Render_floating_triggers",Render_floating_triggers);
#endif

	Database->write("TerrLeveling",Default_player_terrain_leveling);
	Database->write("RoomLeveling",Default_player_room_leveling);
	//Database->write("Terrain_casting",Detail_settings.Terrain_casting);
	Database->write("Specmapping",Detail_settings.Specular_lighting);
	Database->write("FastHeadlight",Detail_settings.Fast_headlight_on);
	Database->write("MirrorSurfaces",Detail_settings.Mirrored_surfaces);
	Database->write("MissileView",Missile_camera_window);
#ifndef GAMEGAUGE
	Database->write("RS_vsync",Render_preferred_state.vsync_on);
#else
	Render_preferred_state.vsync_on = 0;
#endif
	Database->write("DetailScorchMarks",Detail_settings.Scorches_enabled);
	Database->write("DetailWeaponCoronas",Detail_settings.Weapon_coronas_enabled);
	Database->write("DetailFog",Detail_settings.Fog_enabled);
	Database->write("DetailCoronas",Detail_settings.Coronas_enabled);
	Database->write("DetailProcedurals",Detail_settings.Procedurals_enabled);
	Database->write("DetailObjectComp",Detail_settings.Object_complexity);
	Database->write("DetailPowerupHalos",Detail_settings.Powerup_halos);

	Database->write("RS_resolution", Game_video_resolution);
	Database->write("RS_windowwidth", Game_window_res_width);
	Database->write("RS_windowheight", Game_window_res_height);
	Database->write("RS_fullscreen", Game_fullscreen);
	Database->write("RS_fovdesired", Render_FOV_desired);

	Database->write("RS_bitdepth",Render_preferred_bitdepth);
	Database->write("RS_bilear",Render_preferred_state.filtering);
	Database->write("RS_mipping",Render_preferred_state.mipping);
	Database->write("RS_color_model",Render_state.cur_color_model);
	Database->write("RS_light",Render_state.cur_light_state);
	Database->write("RS_texture_quality",Render_state.cur_texture_quality);

	Database->write("VoicePowerup",PlayPowerupVoice);
	Database->write("VoiceAll",PlayVoices);

	// Write out force feedback
#ifndef MACINTOSH
	Database->write("EnableJoystickFF",D3Use_force_feedback);
	Database->write("ForceFeedbackAutoCenter",D3Force_auto_center);
	ubyte force_gain;
	if(D3Force_gain<0.0f) D3Force_gain = 0.0f;
	if(D3Force_gain>1.0f) D3Force_gain = 1.0f;
	force_gain = (ubyte)((100.0f * D3Force_gain)+0.5f);
	Database->write("ForceFeedbackGain",force_gain);
#endif

#ifndef RELEASE			// never save this value out in release.
	Database->write("SoundMixer", Sound_mixer);
	Database->write("PreferredRenderer",PreferredRenderer);
#endif
	
	tempint = Sound_quality;
	Database->write("SoundQuality", tempint);

	Database->write("SoundQuantity",Sound_system.GetLLSoundQuantity());

	if(Default_pilot[0]!='\0')
		Database->write("Default_pilot",Default_pilot,strlen(Default_pilot)+1);
	else
		Database->write("Default_pilot"," ",2);
}

extern bool Game_gauge_do_time_test;
extern char Game_gauge_usefile[_MAX_PATH];

/*
	Read game variables from the registry
*/
void LoadGameSettings()
{
	char tempbuffer[TEMPBUFFERSIZE],*stoptemp;
	int templen = TEMPBUFFERSIZE;
	int tempint;

	//set defaults
#ifdef _DEBUG
	Outline_mode = OM_ALL;
	Lighting_on = true;
#endif

	int tt_arg = FindArg("-timetest");
	if(tt_arg)
	{
		Game_gauge_do_time_test = true;
		strcpy(Game_gauge_usefile,GameArgs[tt_arg+1]);
	}

	Detail_settings.Specular_lighting = false;
	Detail_settings.Dynamic_lighting = true;
	Detail_settings.Fast_headlight_on = true;
	Detail_settings.Mirrored_surfaces = true;
	Detail_settings.Scorches_enabled = true;
	Detail_settings.Weapon_coronas_enabled = true;
	Render_preferred_state.mipping = true;
	Render_preferred_state.filtering = true;
	Render_preferred_state.bit_depth = 16;
	Render_preferred_bitdepth = 16;
	Default_player_terrain_leveling = 2;
	Default_player_room_leveling = 2;
	Render_preferred_state.gamma = 1.5;
	PreferredRenderer = RENDERER_NONE;

#ifdef MACINTOSH

//DAJ render switch
#if defined(USE_OPENGL)
	PreferredRenderer = RENDERER_OPENGL;
#elif defined(USE_GLIDE)
	PreferredRenderer = RENDERER_GLIDE;
#elif defined(USE_SOFTWARE)
	PreferredRenderer = RENDERER_SOFTWARE_16BIT;
#else
	PreferredRenderer = RENDERER_NONE;
#endif		

#endif

	Sound_system.SetLLSoundQuantity(MIN_SOUNDS_MIXED+ (MAX_SOUNDS_MIXED-MIN_SOUNDS_MIXED)/2);
	D3MusicSetVolume(0.5f);
	Detail_settings.Pixel_error = 8.0;
#ifdef MACINTOSH
	Sound_system.SetMasterVolume(0.7);
	Detail_settings.Terrain_render_distance = 10.0 * TERRAIN_SIZE;	//DAJ
#else
	Sound_system.SetMasterVolume(1.0);
	Detail_settings.Terrain_render_distance = 70.0 * TERRAIN_SIZE;
	D3Use_force_feedback = true;
	D3Force_gain = 1.0f;
	D3Force_auto_center = true;
#endif
	Game_video_resolution = RES_640X480;
	PlayPowerupVoice = true;
	PlayVoices = true;
	Sound_mixer = SOUND_MIXER_SOFTWARE_16;
#ifdef MACINTOSH
	Sound_quality = SQT_LOW;
#else
	Sound_quality = SQT_NORMAL;
#endif
	Missile_camera_window = SVW_LEFT;
	Render_preferred_state.vsync_on = true;
	Detail_settings.Fog_enabled = true;
	Detail_settings.Coronas_enabled = true;
	Detail_settings.Procedurals_enabled = true;
	Detail_settings.Object_complexity = 1;
	Detail_settings.Powerup_halos = true;

	ddio_SetKeyboardLanguage(KBLANG_AMERICAN);

	if (Database->read("KeyboardType", tempbuffer, &templen)) 
	{
		if (strcmpi(tempbuffer, "French") == 0) {
			ddio_SetKeyboardLanguage(KBLANG_FRENCH);
		}
		else if (strcmpi(tempbuffer, "German") == 0) {
			ddio_SetKeyboardLanguage(KBLANG_GERMAN);
		}
	}

	templen = TEMPBUFFERSIZE;
	if(Database->read("RS_gamma",tempbuffer,&templen))
	{
		Render_preferred_state.gamma = strtod(tempbuffer,&stoptemp);
	}
	templen = TEMPBUFFERSIZE;
	if(Database->read("SND_mastervol",tempbuffer,&templen))
	{
		Sound_system.SetMasterVolume(strtod(tempbuffer,&stoptemp));
	}
	templen = TEMPBUFFERSIZE;
	if(Database->read("MUS_mastervol",tempbuffer,&templen))
	{
		D3MusicSetVolume(strtod(tempbuffer,&stoptemp));
	}
	templen = TEMPBUFFERSIZE;
	if(Database->read("RS_pixelerror",tempbuffer,&templen))
	{
		Detail_settings.Pixel_error = strtod(tempbuffer,&stoptemp);
	}
	templen = TEMPBUFFERSIZE;
	if(Database->read("RS_terraindist",tempbuffer,&templen))
	{
		Detail_settings.Terrain_render_distance = strtod(tempbuffer,&stoptemp) * TERRAIN_SIZE;
	}
	templen = TEMPBUFFERSIZE;
	Database->read_int("Dynamic_Lighting",&Detail_settings.Dynamic_lighting);
	
#ifdef _DEBUG
	Database->read_int("Outline_mode",&Outline_mode);
	Database->read("Lighting_on",&Lighting_on);
	Database->read("Render_floating_triggers",&Render_floating_triggers);
#endif

	Database->read_int("TerrLeveling",&Default_player_terrain_leveling);
	Database->read_int("RoomLeveling",&Default_player_room_leveling);
	//Database->read("Terrain_casting",&Detail_settings.Terrain_casting);
	Database->read("Specmapping",&Detail_settings.Specular_lighting);
	Database->read("RS_bitdepth",&Render_preferred_bitdepth,sizeof(Render_preferred_bitdepth));
	Database->read_int("RS_resolution", &Game_video_resolution);
	Database->read_int("RS_windowwidth", &Game_window_res_width);
	Database->read_int("RS_windowheight", &Game_window_res_height);
	int temp;
	Database->read_int("RS_fovdesired", &temp);
	if (temp < D3_DEFAULT_FOV)
		temp = D3_DEFAULT_FOV;
	Render_FOV_desired = Render_FOV = temp;
	Database->read_int("RS_bilear",&Render_preferred_state.filtering);
	Database->read_int("RS_mipping",&Render_preferred_state.mipping);
	Database->read_int("RS_color_model",&Render_state.cur_color_model);
	Database->read_int("RS_light",&Render_state.cur_light_state);
	Database->read_int("RS_texture_quality",&Render_state.cur_texture_quality);
#ifdef MACINTOSH
	if(Render_state.cur_texture_quality == 0) {
		Mem_low_memory_mode = true;
		Mem_superlow_memory_mode = true;
	} else if(Render_state.cur_texture_quality == 1) {
		Mem_low_memory_mode = true;
		Mem_superlow_memory_mode = false;
	} else if(Render_state.cur_texture_quality == 2) {
		Mem_low_memory_mode = false;
		Mem_superlow_memory_mode = false;
	}
#else
	// force feedback stuff
	Database->read("EnableJoystickFF",&D3Use_force_feedback);
	Database->read("ForceFeedbackAutoCenter",&D3Force_auto_center);
	ubyte force_gain;
	Database->read("ForceFeedbackGain",&force_gain,sizeof(force_gain));
	if(force_gain>100) force_gain = 100;
	D3Force_gain = ((float)force_gain)/100.0f;
#endif
	Database->read_int("PreferredRenderer",&PreferredRenderer);
	Database->read_int("MissileView",&Missile_camera_window);
	Database->read("FastHeadlight",&Detail_settings.Fast_headlight_on);
	Database->read("MirrorSurfaces",&Detail_settings.Mirrored_surfaces);
	Database->read_int("RS_vsync",&Render_preferred_state.vsync_on);

	if (FindArg ("-vsync"))
		Render_preferred_state.vsync_on=true;


//@@	// Base missile camera if in wrong window
//@@	if (Missile_camera_window==SVW_CENTER)
//@@		Missile_camera_window=SVW_LEFT;

	Database->read("VoicePowerup",&PlayPowerupVoice);
	Database->read("VoiceAll",&PlayVoices);

	Database->read("DetailScorchMarks",&Detail_settings.Scorches_enabled);
	Database->read("DetailWeaponCoronas",&Detail_settings.Weapon_coronas_enabled);
	Database->read("DetailFog",&Detail_settings.Fog_enabled);
	Database->read("DetailCoronas",&Detail_settings.Coronas_enabled);
	Database->read("DetailProcedurals",&Detail_settings.Procedurals_enabled);
	Database->read_int("DetailObjectComp",&tempint);
	Detail_settings.Object_complexity = tempint; 
	if(Detail_settings.Object_complexity<0 || Detail_settings.Object_complexity>2) 
		Detail_settings.Object_complexity = 1;

	Database->read("DetailPowerupHalos",&Detail_settings.Powerup_halos);

	if(Database->read_int("SoundMixer",&tempint))
		Sound_mixer = tempint;

	if(Database->read_int("SoundQuality",&tempint))
		Sound_quality = tempint;
	
	if(Database->read_int("SoundQuantity",&tempint)) {
		Sound_system.SetLLSoundQuantity(tempint);
	}

	Sound_card_name[0] = 0;
	templen = TEMPBUFFERSIZE;
	if (Database->read("SoundcardName",tempbuffer, &templen)) {
		strcpy(Sound_card_name, tempbuffer);
	}
	
	int len = _MAX_PATH;
	Database->read("Default_pilot",Default_pilot,&len);

	//If preferred renderer set to software, force it to be glide
	if ((PreferredRenderer == RENDERER_SOFTWARE_8BIT) || (PreferredRenderer == RENDERER_SOFTWARE_16BIT)) {
		Int3();	//Warning: rederer was set to Software.  Ok to ignore this.
		PreferredRenderer = RENDERER_OPENGL;
	}

	//Now that we have read in all the data, set the detail level if it is a predef setting (custom is ignored in function)

	int level;
#ifdef MACINTOSH
	#ifdef USE_OPENGL
		level = DETAIL_LEVEL_LOW;
	#else
		level = DETAIL_LEVEL_HIGH;
	#endif
#else
	level = DETAIL_LEVEL_MED;
#endif

	Database->read_int("PredefDetailSetting",&level);
	ConfigSetDetailLevel(level);

	// Motion blur
	Use_motion_blur = 0;
	if(Katmai || FindArg("-motionblur"))
	{	
		if(!FindArg("-nomotionblur"))
		{
			Use_motion_blur = 1;
		}
	}

	Render_powerup_sparkles = false;
	if(Katmai && !FindArg("-nosparkles"))
	{
		Render_powerup_sparkles = true;
	}

#ifdef GAMEGAUGE
	// Setup some default params for gamegauge
	Detail_settings.Scorches_enabled=1;
	Detail_settings.Weapon_coronas_enabled=1;
	Detail_settings.Fog_enabled=1;
	Detail_settings.Coronas_enabled=1;
	Detail_settings.Procedurals_enabled=1;
	Detail_settings.Powerup_halos=1;
	Detail_settings.Mirrored_surfaces=1;
	Detail_settings.Pixel_error = 8;
	Detail_settings.Terrain_render_distance = 120 * TERRAIN_SIZE;
	Detail_settings.Specular_lighting=1;
	Use_motion_blur = 0;

	if(1)
#else
	if(Game_gauge_do_time_test)
#endif
	{
		Detail_settings.Procedurals_enabled = 0;
	}

	// We only support OpenGL now...
	PreferredRenderer = RENDERER_OPENGL;
}


typedef struct
{
	char *wildcard;
}tTempFileInfo;
tTempFileInfo temp_file_wildcards[] = 
{
	{"d3s*.tmp"},
	{"d3m*.tmp"},
	{"d3o*.tmp"},
	{"d3c*.tmp"},
	{"d3t*.tmp"},
	{"d3i*.tmp"}
};
int num_temp_file_wildcards = sizeof(temp_file_wildcards)/sizeof(tTempFileInfo);

bool Merc_IsInstalled;
//Returns true if Mercenary is installed
bool MercInstalled()
{
	return Merc_IsInstalled;
}


/*
	I/O systems initialization
*/
void InitIOSystems(bool editor)
{
	ddio_init_info io_info;
	int dirlen = _MAX_PATH;

	//Set the base directory
	int dirarg = FindArg("-setdir");
	int exedirarg = FindArg("-useexedir");
	if(dirarg)
	{
		strcpy(Base_directory,GameArgs[dirarg+1]);
	}
	else if(exedirarg)
	{
		strcpy(Base_directory,GameArgs[0]);
		int len = strlen(Base_directory);
		for(int i=(len-1);i>=0;i--)
		{
			if(i=='\\')
			{
				Base_directory[i] = NULL;
			}
		}
		mprintf((0,"Using working directory of %s\n",Base_directory));
	}
	else
	{
		ddio_GetWorkingDir(Base_directory,sizeof(Base_directory));
	}

	ddio_SetWorkingDir(Base_directory);

	Descent->set_defer_handler(D3DeferHandler);

//	do io init stuff
	io_info.obj = Descent;
	io_info.use_lo_res_time = (bool)(FindArg("-lorestimer")!=0);
	io_info.joy_emulation = (bool)((FindArg("-alternatejoy")==0) && (FindArg("-directinput")==0));
	io_info.key_emulation = true; //(bool)(FindArg("-slowkey")!=0); WIN95: DirectInput is flaky for some keys. 
	if (!ddio_Init(&io_info)) {
		Error("I/O initialization failed.");
	} 

	if( !editor)
	{
		ddio_MouseMode(MOUSE_STANDARD_MODE);
		ddio_MouseSetCallbackFn(ShouldCaptureMouse);
	}

	int rocknride_arg = FindArg("-rocknride");
	if(rocknride_arg)
	{
		int comm_port = atoi(GameArgs[rocknride_arg+1]);
		if(!RNR_Initialize(comm_port))
		{
			mprintf((0,"Rock'n'Ride Init failed!\n"));
		}else
		{
			mprintf((0,"Rock'n'Ride Init success!\n"));
		}
	}

	rtp_Init();
	RTP_ENABLEFLAGS(RTI_FRAMETIME|RTI_RENDERFRAMETIME|RTI_MULTIFRAMETIME|RTI_MUSICFRAMETIME|RTI_AMBSOUNDFRAMETIME);
	RTP_ENABLEFLAGS(RTI_WEATHERFRAMETIME|RTI_PLAYERFRAMETIME|RTI_DOORFRAMETIME|RTI_LEVELGOALTIME|RTI_MATCENFRAMETIME);
	RTP_ENABLEFLAGS(RTI_OBJFRAMETIME|RTI_AIFRAMETIME|RTI_PROCESSKEYTIME);

//	Read in stuff from the registry
	LoadGameSettings();

	// Setup temp directory
	SetupTempDirectory();

//	Initialize file system
	INIT_MESSAGE(("Managing file system."));

	//delete any leftover temp files
	mprintf((0,"Removing any temp files left over from last execution\n"));
	DeleteTempFiles();

//	create directory system.
	if (!mng_InitTableFiles())
	{
	#ifdef EDITOR
		Error("Couldn't successfully initialize the table files.  I'm shutting down!");
	#else
		Error(TXT_INITTABLEERR);
	#endif
	}

	//Init hogfiles
	int d3_hid=-1,extra_hid=-1,merc_hid=-1,sys_hid=-1,extra13_hid=-1;
	char fullname[_MAX_PATH];
	
	#ifdef DEMO
//DAJ	d3_hid = cf_OpenLibrary("d3demo.hog");
	ddio_MakePath(fullname, LocalD3Dir, "d3demo.hog", NULL);
	#else
	ddio_MakePath(fullname, LocalD3Dir, "d3.hog", NULL);
	#endif
	d3_hid = cf_OpenLibrary(fullname);

	#ifdef __LINUX__
//DAJ	sys_hid = cf_OpenLibrary("d3-linux.hog");
		ddio_MakePath(fullname, LocalD3Dir, "d3-linux.hog", NULL);
		sys_hid = cf_OpenLibrary(fullname);
	#endif	
	#ifdef MACOSX
		ddio_MakePath(fullname, LocalD3Dir, "d3-osx.hog", NULL);
		sys_hid = cf_OpenLibrary(fullname);
	#endif	
	

	//Open this file if it's present for stuff we might add later
	ddio_MakePath(fullname, LocalD3Dir, "extra.hog", NULL);
	extra_hid = cf_OpenLibrary(fullname);

	// non-windows platforms always get merc!
	merc_hid = cf_OpenLibrary("merc.hog");
	if (merc_hid)
		Merc_IsInstalled = true;
	// Open this for extra 1.3 code (Black Pyro, etc)
	ddio_MakePath(fullname, LocalD3Dir, "extra13.hog", NULL);
	extra13_hid = cf_OpenLibrary(fullname);

	//Check to see if there is a -mission command line option
	//if there is, attempt to open that hog/mn3 so it can override such
	//things as the mainmenu movie, or loading screen
	int mission_arg = FindArg("-mission");
	if(mission_arg>0)
	{
		char path_to_mission[_MAX_PATH];
		char filename[256];

		// get the true filename
		ddio_SplitPath(GameArgs[mission_arg+1],NULL,filename,NULL);
		strcat(filename,".mn3");

		// make the full path (it is forced to be on the harddrive since it contains
		// textures and stuff).
		ddio_MakePath(path_to_mission,LocalD3Dir,"missions",filename,NULL);
		if(cfexist(path_to_mission))
		{
			cf_OpenLibrary(path_to_mission);
		}else
		{
			Int3();//mission not found
		}
	}

	// Initialize debug graph early incase any system uses it in it's init
	DebugGraph_Initialize();

	//	initialize all the OSIRIS systems
	//	extract from extra.hog first, so it's dll files are listed ahead of d3.hog's
	Osiris_InitModuleLoader();	
	if(extra13_hid!=-1)
		Osiris_ExtractScriptsFromHog(extra13_hid,false);
	if(extra_hid!=-1)
		Osiris_ExtractScriptsFromHog(extra_hid,false);
	if(merc_hid!=-1)
		Osiris_ExtractScriptsFromHog(merc_hid,false);
	if(sys_hid!=-1)
		Osiris_ExtractScriptsFromHog(sys_hid,false);
	Osiris_ExtractScriptsFromHog(d3_hid,false);	
}

extern int Num_languages;
void InitStringTable()
{

#if defined (MACINTOSH) && !defined (DAJ_DEBUG)
	if(cfopen("german.lan", "rt")) {
		Localization_SetLanguage(LANGUAGE_GERMAN);
		ddio_SetKeyboardLanguage(KBLANG_GERMAN);
	} else if(cfopen("spanish.lan", "rt")) {
		Localization_SetLanguage(LANGUAGE_SPANISH);
		ddio_SetKeyboardLanguage(KBLANG_AMERICAN);
	} else if(cfopen("italian.lan", "rt")) {
		Localization_SetLanguage(LANGUAGE_ITALIAN);
		ddio_SetKeyboardLanguage(KBLANG_AMERICAN);
	} else if(cfopen("french.lan", "rt")) {
		Localization_SetLanguage(LANGUAGE_FRENCH);
		ddio_SetKeyboardLanguage(KBLANG_FRENCH);
	} else {
		Localization_SetLanguage(LANGUAGE_ENGLISH);
		ddio_SetKeyboardLanguage(KBLANG_AMERICAN);
	}	
#else
	int language = LANGUAGE_ENGLISH;
	Database->read("LanguageType",&language,sizeof(language));

	if(language<0 || language>=Num_languages)
	{
		Int3();
		language = LANGUAGE_ENGLISH;
	}
	Localization_SetLanguage(language);
	
#endif
	int string_count = LoadStringTables();
	
	if(string_count==0) 
		Error("Couldn't find the string table.");
	else
		mprintf((0,"%d strings loaded from the string tables\n",string_count));
}


void InitGraphics(bool editor)
{
// Init our bitmaps, must be called before InitTextures
	bm_InitBitmaps();

// Init our textures
	if (!InitTextures())
		Error("Failed to initialize texture system.");

#ifdef EDITOR
	char *driver = "GDIX";
	
	if (!ddgr_Init(Descent, driver, editor ? false: true)) 
		Error("DDGR graphic system init failed.");

// Init our renderer
	grSurface::init_system();
	rend_Init (RENDERER_SOFTWARE_16BIT, Descent,NULL);
	Desktop_surf = new grSurface(0,0,0, SURFTYPE_VIDEOSCREEN, 0);
#else
	strcpy(App_ddvid_subsystem,  "GDIX");

	if (!Dedicated_server)
	{
		if (!ddvid_Init( Descent, App_ddvid_subsystem)) 
			Error("Graphics initialization failed.\n");
	}

	INIT_MESSAGE("Loading fonts.");
	LoadAllFonts();
#endif
	Graphics_init = true;
}


void InitGameSystems(bool editor)
{
//	initialize possible remote controller.
	int adr = FindArg("-rjoy");
	if (adr) 
		Controller_ip = &GameArgs[adr+1][1];

//	do other joint editor/game initialization.
	SetInitMessageLength(TXT_INITCOLLATING, 0.4f);
	TelComInit();
	InitFrameTime();

	//Check for aspect ratio override
	int t = FindArg("-aspect");
	if (t) {
		extern void g3_SetAspectRatio(float);
		float aspect = atof(GameArgs[t+1]);
		if (aspect > 0.0)
			g3_SetAspectRatio(aspect);
	}

	//Initialize force feedback effects (if we can)
	ForceInit();

	if (!editor) {
		tUIInitInfo uiinit;
		uiinit.window_font = SMALL_FONT;
		uiinit.w = 640;
		uiinit.h = 480;
		ui_Init(Descent, &uiinit);
		ui_UseCursor("StdCursor.ogf");

		NewUIInit();
		InitControls();

		atexit(CloseControls);
		atexit(ui_Close);
	}
}


//////////////////////////////////////////////////////////////////////////////
static float Init_messagebar_portion = 0.0f, Init_messagebar_offset=0.0f;
static char *Init_messagebar_text = NULL;

// portion of total bar to fill (0 to 1)
void SetInitMessageLength(char *c, float amount)
{
	Init_messagebar_text = c;
	Init_messagebar_offset += Init_messagebar_portion;
	Init_messagebar_portion = amount;
}


// amount is 0 to 1
void UpdateInitMessage(float amount)
{
	if (Init_in_editor) return;
	InitMessage(Init_messagebar_text, (amount*Init_messagebar_portion)+Init_messagebar_offset);
//	mprintf((0, "amt=%.2f, portion=%.2f offs=%.2f, prog=%.2f\n", amount, Init_messagebar_portion, Init_messagebar_offset, (amount*Init_messagebar_portion)+Init_messagebar_offset));
}


void InitMessage(char *c,float progress)
{
	int x = Game_window_w/2 - Title_bitmap.pw/2;
	int y = Game_window_h/2 - Title_bitmap.ph/2;
	int i, rx, ry, rw ,rh;

	if (Dedicated_server)
	{
		PrintDedicatedMessage("%s\n",c);
		return;
	}

	if (!Graphics_init)
		return;

	StartFrame();

	if (Title_bitmap_init) {
		rend_ClearScreen(GR_BLACK);
		rend_DrawChunkedBitmap(&Title_bitmap, x,y,255);
	}

	if (c) {
		g3Point *pntlist[4],points[4];
	// Set our four corners to cover the screen
		grtext_SetFont(MONITOR9_NEWUI_FONT);
		rw = Game_window_w;
		rh = grfont_GetHeight(MONITOR9_NEWUI_FONT) + 8;
		rx = 0;
		ry = Game_window_h - rh;
		points[0].p3_sx=rx;
		points[0].p3_sy=ry;
		points[1].p3_sx=rx+rw;
		points[1].p3_sy=ry;
		points[2].p3_sx=rx+rw;
		points[2].p3_sy=ry+rh;
		points[3].p3_sx=rx;
		points[3].p3_sy=ry+rh;

		for (i=0;i<4;i++)
		{
			points[i].p3_z=0;
			points[i].p3_flags=PF_PROJECTED;
			pntlist[i]=&points[i];
		}

		rend_SetZBufferState(0);
		rend_SetTextureType (TT_FLAT);
		rend_SetAlphaType (AT_CONSTANT);
		rend_SetLighting (LS_NONE);
		rend_SetFlatColor (GR_BLACK);
		rend_SetAlphaValue(230);
		rend_DrawPolygon2D( 0, pntlist, 4 );

		if (progress>=0)
		{
			points[0].p3_sx=rx;
			points[0].p3_sy=ry+rh-(rh/2)+2;
			points[1].p3_sx=rx+((float)rw*progress);
			points[1].p3_sy=ry+rh-(rh/2)+2;
			points[2].p3_sx=rx+((float)rw*progress);
			points[2].p3_sy=ry+rh;
			points[3].p3_sx=rx;
			points[3].p3_sy=ry+rh;

			rend_SetFlatColor (GR_RGB(255,0,0));
			rend_SetAlphaValue(230);
			rend_DrawPolygon2D( 0, pntlist, 4 );
		}
	
		grtext_SetAlpha(255);
		grtext_SetColor(NEWUI_MONITORFONT_COLOR);
//		grtext_CenteredPrintf(0, ry, c);
		grtext_Puts(0, ry, c);
		grtext_Flush();
	}

	EndFrame();
	rend_Flip();
}


//////////////////////////////////////////////////////////////////////////////
#if (defined(OEM) || defined(DEMO) || defined(RELEASE))
void ShowStaticScreen(char *bitmap_filename,bool timed=false,float delay_time=0.0f);
#endif

void IntroScreen()
{		
//#if (defined(OEM) || defined(DEMO) )
#ifdef DEMO
	#ifdef MACINTOSH
	ShowStaticScreen("graphsim.ogf",true,3.0);	
	#else
	ShowStaticScreen("tantrum.ogf",true,3.0);	
	#endif
	ShowStaticScreen("outrage.ogf",true,3.0);	
#else
	#ifdef MACINTOSH
	if(cfopen("publisher.ogf", "rb"))
		ShowStaticScreen("publisher.ogf",true,3.0);	
	
	if(cfopen("graphsim.ogf", "rb"))
		ShowStaticScreen("graphsim.ogf",true,3.0);	
	#endif
#endif
	
#ifdef DEMO
	int bm_handle = bm_AllocLoadFileBitmap ("demomenu.ogf",0);
#else
	int bm_handle = bm_AllocLoadFileBitmap ("oemmenu.ogf",0);
#endif
	mprintf((0, "Intro screen!.\n"));

	if (bm_handle > -1) {
		if (!bm_CreateChunkedBitmap(bm_handle, &Title_bitmap)) 
			Error("Failed to slice up d3.ogf!");

		bm_FreeBitmap(bm_handle);

		Title_bitmap_init = true;
		InitMessage(NULL);
	}
	else {
		mprintf((1, "Unable to find d3.tga.\n"));
	}
}

void InitDedicatedServer()
{
  	Game_mode|=GM_MULTI;
  	Netgame.local_role=LR_SERVER;

  	int ok=LoadServerConfigFile ();

  	if (!ok) {
  		PrintDedicatedMessage (TXT_SHUTTINGDOWN);
		Error("Cannot load Dedicated Server config file.");
#ifdef WIN32
		exit(0);
#endif
	}
}

/*
 	The D3 Initialization Mess
  
	This initialization sequence will occur after the application and debug systems 
	are set up.

	From here, we initialize all systems.

	Initializes all the subsystems that D3/Editor needs to run. 
	Returns 1 if all is good, 0 if something is wrong
*/
int ServerTimeout = 0;
float LastPacketReceived;

ushort Gameport = D3_DEFAULT_PORT;
ushort PXOPort = 0;
//Initialiaze everything before data load
void InitD3Systems1(bool editor)
{
#if defined (RELEASE) || defined (MACINTOSH)
	SetDebugBreakHandlers(NULL, NULL);
#else
	SetDebugBreakHandlers(D3DebugStopHandler, D3DebugResumeHandler);
#endif

	Init_in_editor = editor;

	//Gamespy command to specify the port to listen on
	int gameportarg = FindArg("+host");
	if(!gameportarg)
	{
		gameportarg = FindArg("-useport");
	}
	if(gameportarg)
	{
		Gameport = atoi(GameArgs[gameportarg+1]);
	}
	
	int pxoportarg = FindArg("-pxoport");
	if(pxoportarg)
		PXOPort = atoi(GameArgs[pxoportarg+1]);

// perform user i/o system initialization
	INIT_MESSAGE(("Initializing I/O system."));
	InitIOSystems(editor);

//	load the string table
	InitStringTable();

	if(!IsLocalOk())
	{
#ifdef WIN32
		MessageBox(NULL,"Sorry, your computer has a language not supported by this version of Descent 3.",PRODUCT_NAME,MB_OK);
#else
		printf("Sorry, your computer has a language not supported by this version of Descent 3.");
#endif
		exit(0);		
	}
	
// With 1.5 no more copy protection!
#if 0
	//CD Check goes here
//#if ( defined(RELEASE) && (!defined(DEMO)) && (!defined(GAMEGAUGE)) )
	if( (!FindArg("-dedicated")) ) 
		PreGameCdCheck();
#endif

#ifdef LASERLOCK
	//At this point the laser locked CD MUST be inserted or the
	//game and possibly the entire OS will crash because laserlock
	//will assume it's a hacked CD.
	if(!ll_Init())
		exit(0);
	if(!ll_Verify())
		exit(0);
#else
bool CheckCdForValidity(int cd);
	if( (!FindArg("-dedicated"))) 
	{
		if(!CheckCdForValidity(CD_inserted))
		{
#ifdef WIN32
			ShowCursor(true);
			MessageBox(NULL,"Invalid CDROM!",PRODUCT_NAME,MB_OK);		
			ShowCursor(false);
#endif
			exit(0);
		}
	}
#endif
	INIT_MESSAGE(("Initializing GFX"));
	InitGraphics(editor);

//	initialize data structures
	InitObjectInfo();
	InitVClips();
	InitRooms();
	
//	initialize lighting systems
	InitLightmapInfo();
	InitSpecialFaces();
	InitDynamicLighting();
	
// Initialize missions
	InitMission();

// Initializes the ship structure
	InitShips();

// Initializes the fvi structures for quicker operation
   InitFVI();
	
// Initializes the matcens
	InitMatcens();

// This function needs be called before ANY 3d stuff can get done. I mean it.
	InitMathTables();

// This function has to be done before any sound stuff is called
	InitSounds();

// Allocate memory and stuff for our terrain engine, objects, etc.
	InitTerrain();

	InitModels();
	InitDoors();
	InitGamefiles();

//	network initialization
	if(!FindArg("-nonetwork"))
	{
		nw_InitNetworking();
		nw_InitSockets(Gameport);
		
		int tcplogarg;
		tcplogarg = FindArg("-tcplog");
		if(tcplogarg)
		{
			char ipparse[50];
			char *pport;
			int port = 9999;
			strcpy(ipparse,GameArgs[tcplogarg+1]);
			pport = strchr(ipparse,':');
			if(pport)
			{
				*pport = NULL;
				pport++;
				port = atoi(pport);
			}
#if !defined(RELEASE) && !defined(MACINTOSH)
			nw_InitTCPLogging(ipparse,port);
#endif
		}
	}

	int timeoutarg = FindArg("-timeout");
	if(timeoutarg)
	{
		ServerTimeout = atoi(GameArgs[timeoutarg+1]);
		LastPacketReceived = timer_GetTime();
	}

//Init gamespy
//	gspy_Init();

// Sound initialization
	int soundres = Sound_system.InitSoundLib(Descent, Sound_mixer, Sound_quality, false);
#ifdef OEM_AUREAL
	if(!soundres)
	{
		Error("Unable to initialize Aureal audio. This version of Descent 3 requires Aureal audio");
	}
#endif

	//	Initialize Cinematics system
	InitCinematics();

	// Initialize IntelliVIBE (if available)
	VIBE_Init(Descent);
}

//Initialize rest of stuff
void InitD3Systems2(bool editor)
{
	// Initialize In-Game Cinematics system
	Cinematic_Init();

//	initialize slewing
#ifdef EDITOR
	SlewControlInit();
#endif

	int tables_loaded=0;

//	load in all data headers.
	INIT_MESSAGE((TXT_INITDATA));
	tables_loaded=mng_LoadTableFiles(! editor); 
	
	if (!tables_loaded)
		Error("Cannot load table file.");

	INIT_MESSAGE((TXT_INITCOLLATING));

//Initialize the pilot system
	PilotInit();

// Setup our object system.  By object I mean stuff in our world, not those silly C++ things.
// This call must come after InitTableFiles(), so we have a player ship for the user
	InitObjects();

// Fireball initting must come after LoadTableFiles
	SetInitMessageLength(TXT_INITCOLLATING, 0.1f);
	InitFireballs();
	UpdateInitMessage(1.0f);

// initializes triggers
	InitTriggers();

// the remaining sound system
	InitVoices();
	InitD3Music(FindArg("-nomusic") ? false : true);
	InitAmbientSoundSystem();

	InitGameSystems(editor);

	InitPlayers();

	SetInitMessageLength(TXT_INITCOLLATING, 0.5f);
	InitMarkers();
	UpdateInitMessage(1.0f);

	PPic_InitDatabase();

//	Set next logical function for game.
	if (editor)
		SetFunctionMode(EDITOR_MODE);
	else
		SetFunctionMode(MENU_MODE);

	if (Dedicated_server)
		InitDedicatedServer();

	Init_systems_init = true;

// free title screen bitmap.
	if (Title_bitmap_init) {
		bm_DestroyChunkedBitmap(&Title_bitmap);
		Title_bitmap_init = false;
	}

// initialize localized text for controller system.
	extern void Localize_ctl_bindings();
	Localize_ctl_bindings();

#ifdef OEM
	if(cfexist("bundler.mve"))
	{
		PlayMovie("bundler.mve");
	}
	
	if(cfexist("bundler.tga"))
	{
		ShowStaticScreen("bundler.tga");
	}
#endif
}

void SetupTempDirectory(void)
{
	//NOTE: No string tables are available at this point
	//--------------------------------------------------

	mprintf((0,"Setting up temp directory\n"));

	int t_arg = FindArg("-tempdir");
	if(t_arg)
	{
		strcpy(Descent3_temp_directory,GameArgs[t_arg+1]);
	}else
	{
		//initialize it to custom/cache
		ddio_MakePath(Descent3_temp_directory,Base_directory,"custom","cache",NULL);
	}

	//verify that temp directory exists
	if(!ddio_SetWorkingDir(Descent3_temp_directory))
	{
		ddio_MakePath(Descent3_temp_directory, Base_directory, "custom", NULL);
		ddio_CreateDir(Descent3_temp_directory);
		ddio_MakePath(Descent3_temp_directory, Base_directory, "custom", "cache", NULL);
		ddio_CreateDir(Descent3_temp_directory);
		if (!ddio_SetWorkingDir(Descent3_temp_directory))
		{
			Error("Unable to set temporary directory to: \"%s\"", Descent3_temp_directory);
			exit(1);
		}
	}

	char tempfilename[_MAX_PATH];

	//verify that we can write to the temp directory
	if(!ddio_GetTempFileName(Descent3_temp_directory,"d3t",tempfilename))
	{
		mprintf((0,"Unable to get temp file name\n"));
		Error("Unable to set temporary directory to: \"%s\"",Descent3_temp_directory);
		exit(1);
	}

	//try to create a file in temp directory
	CFILE *file = cfopen(tempfilename,"wb");
	if(!file)
	{
		//unable to open file for writing
		mprintf((0,"Unable to open temp file name for writing\n"));
		Error("Unable to set temporary directory to: \"%s\"",Descent3_temp_directory);
		exit(1);
	}

	cf_WriteInt(file,0x56);
	cfclose(file);

	//now open up the file and make sure it is correct
	file = cfopen(tempfilename,"rb");
	if(!file)
	{
		//unable to open file for reading
		mprintf((0,"Unable to open temp file name for reading\n"));
		ddio_DeleteFile(tempfilename);
		Error("Unable to set temporary directory to: \"%s\"",Descent3_temp_directory);
		exit(1);	
	}

	if(cf_ReadInt(file)!=0x56)
	{
		//verify failed
		mprintf((0,"Temp file verify failed\n"));
		cfclose(file);
		ddio_DeleteFile(tempfilename);
		Error("Unable to set temporary directory to: \"%s\"",Descent3_temp_directory);
		exit(1);
	}

	cfclose(file);

	//temp directory is valid!
	ddio_DeleteFile(tempfilename);

	mprintf((0,"Temp Directory Set To: \"%s\"\n",Descent3_temp_directory));

#ifndef MACINTOSH
	// Lock the directory
	int lock_res = ddio_CreateLockFile(Descent3_temp_directory);
	switch(lock_res)
	{
	case 1:
		mprintf((0,"Lock file created in temp dir\n"));
		break;
	case 2:
		mprintf((0,"Lock file created in temp dir (deleted dead lock)\n"));
		break;
	case 3:
		mprintf((0,"Lock file created in temp dir (lock already exists)\n"));
		break;
	case 0:
		mprintf((0,"Lock file NOT created in temp dir\n"));
		Error("Unable to set temporary directory to: \"%s\"\nThe directory is in use, please use -tempdir to set a different temp directory",Descent3_temp_directory);
		exit(1);		
		break;
	case -1:
		mprintf((0,"Illegal directory for Lock file\n"));
		Error("Unable to set temporary directory to: \"%s\"\nIllegal directory for lock file",Descent3_temp_directory);
		exit(1);		
		break;
	case -2:
		mprintf((0,"Illegal Lock file, unable to create\n"));
		Error("Unable to set temporary directory to: \"%s\"\nInvalid lock file located in directory",Descent3_temp_directory);
		exit(1);		
		break;
	case -3:
 		mprintf((0,"Error creating Lock file\n"));
		Error("Unable to set temporary directory to: \"%s\"\nUnable to create lock file",Descent3_temp_directory);
		exit(1);		
		break;
	}
#endif
	//restore working dir
	ddio_SetWorkingDir(Base_directory);
}

void DeleteTempFiles(void)
{
	char filename[_MAX_PATH];

	//delete the d3 temp files in the temp directory
	if(ddio_SetWorkingDir(Descent3_temp_directory))
	{
		int i;
		for(i=0;i<num_temp_file_wildcards;i++)
		{
			if(ddio_FindFileStart(temp_file_wildcards[i].wildcard,filename))
			{
				do
				{
					ddio_DeleteFile(filename);
				}
#ifdef MACINTOSH
				while(ddio_FindFileStart(temp_file_wildcards[i].wildcard,filename));
#else
				while(ddio_FindNextFile(filename));
#endif
			}
			ddio_FindFileClose();
		}
	}

	//restore directory
	ddio_SetWorkingDir(Base_directory);	
}


/*
 *	
 *	These functions handle shutting down and restarting all systems in D3
 *
 */

static int Init_old_screen_mode;
static void (*Init_old_ui_callback)() = NULL;
static bool Init_old_control_mode;
static bool Init_ui_cursor_visible;
static bool Init_was_game_paused = false;
static pilot Init_old_pilot;

void ShutdownD3()
{
	if (!Init_systems_init) 
		return;

	mprintf((0, "Shutting down D3...\n"));

	//Close forcefeedback effects
	ForceShutdown();

//	shutdown game systems
	Init_old_control_mode = Control_poll_flag;

//JEFF: only pause game if not in multi, so we can background process
	if (GetFunctionMode() == GAME_MODE) {
		if( !(Game_mode&GM_MULTI))  {
			Init_was_game_paused = Game_paused;
			if (!Init_was_game_paused) {
				PauseGame();
			}
			else {
				D3MusicPause();
			}
		}
	}
	else {
		D3MusicPause();
		Sound_system.PauseSounds();
	}

	SaveControlConfig(&Init_old_pilot);
	CloseControls();

//	shutdown cinematics.

//	shutdown screen.
	Init_ui_cursor_visible = ui_IsCursorVisible();
	Init_old_screen_mode = GetScreenMode();
	Init_old_ui_callback = GetUICallback();
	SetScreenMode(SM_NULL);
	ddvid_Close();

// shutdown IO
	ddio_Close();
}


//	This function restarts all game systems
void RestartD3()
{
	ddio_init_info io_info;

	if (!Init_systems_init) 
		return;

	mprintf((0, "Restarting D3...\n"));

// startup io
	io_info.obj = Descent;
	io_info.use_lo_res_time = (bool)(FindArg("-lorestimer")!=0);
	io_info.key_emulation = true; //(bool)(FindArg("-slowkey")!=0);
	io_info.joy_emulation = (bool)((FindArg("-alternatejoy")==0) && (FindArg("-directinput")==0));
	if (!ddio_Init(&io_info)) {
		Error("I/O initialization failed.");
	} 

	if (Dedicated_server)
	{
		ddio_MouseMode(MOUSE_STANDARD_MODE);
	}
	else
	{
		ddio_MouseMode(MOUSE_EXCLUSIVE_MODE);
	}

//	startup screen.
	ddvid_Init(Descent, App_ddvid_subsystem); 
	ddio_KeyFlush();
	SetScreenMode(Init_old_screen_mode);
	SetUICallback(Init_old_ui_callback);
	if (Init_ui_cursor_visible)
		ui_ShowCursor();
	
//	startup game systems
	InitControls();
	LoadControlConfig(&Init_old_pilot);

// resume game sounds and time as needed
	if (GetFunctionMode() == GAME_MODE) {
		if( !(Game_mode&GM_MULTI) )
		{
			if (!Init_was_game_paused) {
				ResumeGame();
			}
			else {
				D3MusicResume();
			}
		}
	}
	else {
		Sound_system.ResumeSounds();
		D3MusicResume();
	}

// resume controller if it was active before alt-tabbing out.
	if (Init_old_control_mode) {
		ResumeControls();
	}

//Restart Force Feedback
	ForceRestart();

//	startup cinematics.

//	startup sound.
//	Sound_system.ResumeSounds();
//	Sound_system.InitSoundLib(Descent, Sound_mixer, Sound_quality, false);
}

#if (defined(RELEASE) && defined(WIN32) && (!defined(LASERLOCK)) )

#include "io.h"
char * GetCDVolume(int cd_num);

#endif

unsigned int checksum = 0x2bad4b0b;
int DoADir(char *patternp, char *patternn);

//Checks the checksum of all the files on the directory
//and returns true if it matches the built in checksum
bool CheckCdForValidity(int cd)
{
	// 1.5 removed copy protection		
	return true;
#if (defined(RELEASE) && defined(WIN32) && (!defined(LASERLOCK)) )


#ifdef GAMEGAUGE
	return true;
#endif
	
	checksum = 0x2bad4b0b;
	const unsigned int altsums[5] = {0,734315313,732785576,0,0};//South American	& EU NO-LL
	const unsigned int altsums_b[5] = {0,734427847,732785576,0,0};//US 1.2 installable version
	//
#ifndef OEM
	const unsigned int validsums[5] = {0,734382075,732785576,0,0};//US D3
	//const unsigned int validsums[5] = {0,734316207,732784682,0,0};????

#else
	const unsigned int validsums[5] = {0,734315450,0,0,0};
#endif

#ifdef OEM
	if(cd!=1)
	{
		//MessageBox(NULL,"Wrong disk!","Grr...",MB_OK);
		return false;
	}
#else
	if(cd==3)
	{
		//DVD==no protection
		return true;
	}

	if( (cd!=2) && (cd!=1) )
	{
		//MessageBox(NULL,"Wrong disk!","Grr...",MB_OK);
		return false;
	}
#endif
	char *p = GetCDVolume(cd);
	if(*p)
	{
		DWORD SectorsPerCluster;		// sectors per cluster
		DWORD BytesPerSector;			// bytes per sector
		DWORD NumberOfFreeClusters;	// number of free clusters
		DWORD TotalNumberOfClusters;	// total number of clusters

		DoADir(p,"*.*");

		if(GetDiskFreeSpace(p,&SectorsPerCluster,&BytesPerSector,&NumberOfFreeClusters,&TotalNumberOfClusters))
		{
			//Check that we used enough clusters!
			int a=SectorsPerCluster*BytesPerSector*TotalNumberOfClusters;
			//Look for a non-overburned cd
			if(a<696000000)
			{
				//char dbg[300];
				//sprintf(dbg,"Disk too small (%d).",a);
				//MessageBox(NULL,dbg,"Grr...",MB_OK);
				return false;
			}
		}
		else
		{
			//char dbg[300];
			//sprintf(dbg,"Can't get disk info. (%s).",p);
			//MessageBox(NULL,dbg,"Grr...",MB_OK);
			return false;
		} 
	}
	if( (checksum == validsums[cd]) || (checksum == altsums[cd]) || (checksum == altsums_b[cd]))
		return true;
	//MessageBox(NULL,"Bad Checksum.","Grr...",MB_OK);
	return false;
#else
	return true;
#endif
}

//lame copy protection
#if (defined(RELEASE) && defined(WIN32) && (!defined(LASERLOCK)) )






void AddStringToChecksum(char *str)
{
	unsigned int localsum = 0;

	int len = strlen(str);
	localsum = len;
	for(int i=0;i<len;i++)
	{
		localsum += str[i];
	}
	checksum += localsum;
}

int DoADir(char *patternp, char *patternn)
{
	char          patternw[_MAX_PATH];
	char          npatternp[_MAX_PATH];
		int           mfiles;
	int           have_subs;
	int           nfiles;
	struct _finddata_t  fileinfo;
	
	if(patternp[strlen(patternp)-1]!='\\')
		strcat(patternp, "\\");
	strcpy(patternw, patternp);


	strcat(patternw, patternn);

	mfiles = 0;
	have_subs = 0;

	int isearchhandle = _findfirst(patternw, &fileinfo);
	if (isearchhandle)
	{
		do
		{
			//Add this file to the checksum
			
			//Always use lower case
			char lowerfile[_MAX_PATH];
			strcpy(lowerfile,fileinfo.name);
			int slen = strlen(lowerfile);
			for(int b = 0;b<slen;b++)
				lowerfile[b] = tolower(lowerfile[b]);
			
			char *n = lowerfile;
			//Don't use any strings that start with a dot or 'free' (as in 'freespacemovie.exe')
			if( (n[0] != '.') && (! ((n[0] == 'f')&&(n[1] == 'r')&&(n[2] == 'e')&&(n[3] == 'e')) ) 
				&& (! ((n[0] == 'i')&&(n[1] == ' ')&&(n[2] == 'w')&&(n[3] == 'a')) )
				
				)
				AddStringToChecksum(lowerfile);
			if (fileinfo.attrib & _A_SUBDIR)  // subdirectory
			{
				if (fileinfo.name[0] != '.')  // ignore . and ..
					have_subs = 1;				
			}			
		}
        while (!_findnext(isearchhandle,&fileinfo));
	}
	if (have_subs)
	{
		int isearchhandle2 = _findfirst(patternw, &fileinfo);
		if (isearchhandle2)
		{
			do
			{
				if (fileinfo.attrib & _A_SUBDIR)  // subdirectory
				{
					if (fileinfo.name[0] != '.')  // ignore . and ..
					{
						if(patternp[strlen(patternp)-1]!='\\')
							strcat(patternp, "\\");
						strcpy(npatternp, patternp);
						//strcat(npatternp, "\\");
						strcat(npatternp, fileinfo.name);
						nfiles = DoADir(npatternp, patternn);

						if (nfiles >= 0)
							mfiles += nfiles;
						else
						{
							mfiles -= nfiles;
							return(-mfiles); // error return
						}
					}
				}
			}while (!_findnext(isearchhandle2,&fileinfo));
		}
	}

	return mfiles;
}



#endif
