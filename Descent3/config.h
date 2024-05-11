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

#ifndef __CONFIG_H__
#define __CONFIG_H__

//Main menu configuration functions
// ------------------------------------------------------
// ConfigForceFeedback
//	Purpose:
//	Configures your Force Feedback device on your computer
// ------------------------------------------------------
void ConfigForceFeedback(void);


// General option toggles
typedef struct tGameToggles
{
	bool show_reticle;
	bool guided_mainview;
	bool ship_noises;
}
tGameToggles;

extern tGameToggles Game_toggles;

// this list should match the list in config.cpp to work.
#ifdef MACINTOSH
	#define N_SUPPORTED_VIDRES 3
	
	#define RES_640X480		0
	#define RES_800X600		1
	#define RES_960X720		2
	#define RES_1024X768	3
#else
	#define N_SUPPORTED_VIDRES 8

	#define RES_512X384		0
	#define RES_640X480		1
	#define RES_800X600		2
	#define RES_960X720		3
	#define RES_1024X768		4
	#define RES_1280X960		5
	#define RES_1600X1200	6
#endif
// stored resolution list and desired game resolution
struct tVideoResolution
{
	ushort width;
	ushort height;
};

extern int Game_video_resolution;
extern int Game_window_res_width, Game_window_res_height;
extern bool Game_fullscreen;

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//KEEP THESE MEMBERS IN THE SAME ORDER, IF YOU ADD,REMOVE, OR CHANGE ANYTHING IN THIS STRUCT, MAKE SURE YOU
//UPDATE DetailPresetLow,DetailPresetMed,DetailPresetHigh AND DetailPresetVHi IN CONFIG.CPP
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
struct tDetailSettings 
{
	float Terrain_render_distance;	//VisibleTerrainDistance
	float Pixel_error;				//PixelErrorTolerance
	bool Specular_lighting;			//DoSpecularPass
	bool Dynamic_lighting;			//Enable_dynamic_lighting
	bool Fast_headlight_on;
	bool Mirrored_surfaces;
	bool Fog_enabled;
	bool Coronas_enabled;
	bool Procedurals_enabled;
	bool Powerup_halos;
	bool Scorches_enabled;
	bool Weapon_coronas_enabled;
	bool Bumpmapping_enabled;
	ubyte Specular_mapping_type;
	ubyte Object_complexity;		//0 = low, 1 = medium, 2=high
};

// Call this with one of the above defines to set the detail level to a predefined set (custom level is ignored)
void ConfigSetDetailLevel(int level);
// returns the current detail level that the given tDetailSettings is at
int ConfigGetDetailLevel(tDetailSettings *ds);

#define DETAIL_LEVEL_LOW			0
#define DETAIL_LEVEL_MED			1
#define DETAIL_LEVEL_HIGH			2
#define DETAIL_LEVEL_VERY_HIGH	3
#define DETAIL_LEVEL_CUSTOM		4

extern tDetailSettings Detail_settings;
extern int Default_detail_level;

// displays new options menu
extern void OptionsMenu();
#endif
