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

#include "ConfigItem.h"
#include "player.h"
#include "config.h"
#include "ddio.h"
#include "newui.h"
#include "3d.h"
#include "polymodel.h"
#include "application.h"
#include "descent.h"
#include "mono.h"
#include "Mission.h"
#include "ddio.h"
#include "gamefont.h"
#include "multi_ui.h"
#include "cinematics.h"
#include "hlsoundlib.h"
#include "terrain.h"
#include "CFILE.H"
#include "mem.h"
#include "lighting.h"
#include "PHYSICS.H"
#include "pilot.h"
#include "hud.h"
#include "bitmap.h"
#include "game.h"
#include "render.h"
#include "stringtable.h"
#include "SmallViews.h"
#include "D3ForceFeedback.h"
#include "descent.h"
#include "appdatabase.h"
#include "hlsoundlib.h"
#include "soundload.h"
#include "sounds.h"
#include "ctlconfig.h"
#include "d3music.h"
#include "gameloop.h"

#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define STAT_SCORE STAT_TIMER

struct newVideoResolution
{
	int width;
	int height;
	const char* name;
} New_video_res_list[] = {
	{512,384, "512x384"},
	{640,480, "640x480"},
	{800,600, "800x600"},
	{960,720, "960x720"},
	{1024,768, "1024x768"},
	{1280,960, "1280x960"},
	{1600,1200, "1600x1200"},
	//16:9
	{1280,720, "1280x720"},
	{1366,768, "1366x768"},
	{1368,768, "1368x768"},
	{1680,1050, "1680x1050"},
	{1920,1080, "1920x1080"},
	{2560,1440, "2560x1440"},
	{3840,2160, "3840x2160"},
	//16:10
	{1280,800, "1280x800"},
	{1920,1200, "1920x1200"},
	{2560,1600, "2560x1600"},
	//Ultrawide
	{2560,1080, "2560x1080"},
	{2880,1200, "2800x1200"},
	{3440,1440, "3440x1440"},
	{3840,1600, "3840x1600"}
};

#define NUM_RESOLUTION sizeof(New_video_res_list) / sizeof(New_video_res_list[0])

int Game_video_resolution = 1;
int Game_window_res_width = 640, Game_window_res_height = 480;
bool Game_fullscreen = false;
float Render_FOV_desired = 72;

tDetailSettings Detail_settings;
int Default_detail_level = DETAIL_LEVEL_MED;

// toggles specified in general settings.
tGameToggles Game_toggles = 
{
	true,
	false,
	true
};

#define IDV_VCONFIG			12	//video config
#define IDV_GCONFIG			13	//general config
#define IDV_SCONFIG			14	//audio config
#define IDV_DCONFIG			15	//detail level config
#define IDV_HCONFIG			16 //hud config
#define IDV_CCONFIG			17 //controller config
#define IDV_QUIT				0xff

#define UID_GAMMASLIDER		0x1000


/////////////////////////////////////////////////////////////////////////////
// Defines
#define IDV_OK			1
#define IDV_CANCEL	2

//video defines
#define VID_D3D			0
#define VID_GLIDE		1
#define VID_OPENGL		2

//small view settings
#define SMVS_NONE		0
#define SMVS_MISSLE	1

//these are the default settings for each detail level
#define DL_LOW_TERRAIN_DISTANCE		(MINIMUM_RENDER_DIST*TERRAIN_SIZE)
#define DL_LOW_PIXEL_ERROR			25.0
#define DL_LOW_SPECULAR_LIGHT		false
#define DL_LOW_DYNAMIC_LIGHTING		false
#define DL_LOW_FAST_HEADLIGHT		true
#define DL_LOW_MIRRORED_SURFACES	false
#define DL_LOW_FOG_ENABLED			false
#define DL_LOW_CORONAS_ENABLES		false
#define DL_LOW_PROCEDURALS			false
#define DL_LOW_POWERUP_HALOS		false
#define DL_LOW_SCORCH_MARKS			false
#define DL_LOW_WEAPON_CORONAS		false
#define DL_LOW_SPEC_MAPPING_TYPE	1
#define DL_LOW_OBJECT_COMPLEXITY	0

#define DL_MED_TERRAIN_DISTANCE		(90*TERRAIN_SIZE)
#define DL_MED_PIXEL_ERROR			18.0
#define DL_MED_SPECULAR_LIGHT		false
#define DL_MED_DYNAMIC_LIGHTING		false
#define DL_MED_FAST_HEADLIGHT		true
#define DL_MED_MIRRORED_SURFACES	false
#define DL_MED_FOG_ENABLED			true
#define DL_MED_CORONAS_ENABLES		true
#define DL_MED_PROCEDURALS			false
#define DL_MED_POWERUP_HALOS		true
#define DL_MED_SCORCH_MARKS			true
#define DL_MED_WEAPON_CORONAS		false
#define DL_MED_SPEC_MAPPING_TYPE	1
#define DL_MED_OBJECT_COMPLEXITY	1

#define DL_HIGH_TERRAIN_DISTANCE	(100*TERRAIN_SIZE)
#define DL_HIGH_PIXEL_ERROR			12.0
#define DL_HIGH_SPECULAR_LIGHT		false
#define DL_HIGH_DYNAMIC_LIGHTING	true
#define DL_HIGH_FAST_HEADLIGHT		true
#define DL_HIGH_MIRRORED_SURFACES	true
#define DL_HIGH_FOG_ENABLED			true
#define DL_HIGH_CORONAS_ENABLES		true
#define DL_HIGH_PROCEDURALS			true
#define DL_HIGH_POWERUP_HALOS		true
#define DL_HIGH_SCORCH_MARKS		true
#define DL_HIGH_WEAPON_CORONAS		true
#define DL_HIGH_SPEC_MAPPING_TYPE	1
#define DL_HIGH_OBJECT_COMPLEXITY	2

#define DL_VHI_TERRAIN_DISTANCE		(120.0*TERRAIN_SIZE)
#define DL_VHI_PIXEL_ERROR			10.0
#define DL_VHI_SPECULAR_LIGHT		true
#define DL_VHI_DYNAMIC_LIGHTING		true
#define DL_VHI_FAST_HEADLIGHT		true
#define DL_VHI_MIRRORED_SURFACES	true
#define DL_VHI_FOG_ENABLED			true
#define DL_VHI_CORONAS_ENABLES		true
#define DL_VHI_PROCEDURALS			true
#define DL_VHI_POWERUP_HALOS		true
#define DL_VHI_SCORCH_MARKS			true
#define DL_VHI_WEAPON_CORONAS		true
#define DL_VHI_SPEC_MAPPING_TYPE	1
#define DL_VHI_OBJECT_COMPLEXITY	2

#define MINIMUM_TERRAIN_DETAIL		4
#define MAXIMUM_TERRAIN_DETAIL		28
#define MINIMUM_RENDER_DIST			80
#define MAXIMUM_RENDER_DIST			200

tDetailSettings DetailPresetLow =
{
	DL_LOW_TERRAIN_DISTANCE,
	DL_LOW_PIXEL_ERROR,
	DL_LOW_SPECULAR_LIGHT,
	DL_LOW_DYNAMIC_LIGHTING,
	DL_LOW_FAST_HEADLIGHT,
	DL_LOW_MIRRORED_SURFACES,
	DL_LOW_FOG_ENABLED,
	DL_LOW_CORONAS_ENABLES,
	DL_LOW_PROCEDURALS,
	DL_LOW_POWERUP_HALOS,
	DL_LOW_SCORCH_MARKS,
	DL_LOW_WEAPON_CORONAS,
	DL_LOW_SPEC_MAPPING_TYPE,
	DL_LOW_OBJECT_COMPLEXITY
};
tDetailSettings DetailPresetMed =
{
	DL_MED_TERRAIN_DISTANCE,
	DL_MED_PIXEL_ERROR,
	DL_MED_SPECULAR_LIGHT,
	DL_MED_DYNAMIC_LIGHTING,
	DL_MED_FAST_HEADLIGHT,
	DL_MED_MIRRORED_SURFACES,
	DL_MED_FOG_ENABLED,
	DL_MED_CORONAS_ENABLES,
	DL_MED_PROCEDURALS,
	DL_MED_POWERUP_HALOS,
	DL_MED_SCORCH_MARKS,
	DL_MED_WEAPON_CORONAS,
	DL_MED_SPEC_MAPPING_TYPE,
	DL_MED_OBJECT_COMPLEXITY
};
tDetailSettings DetailPresetHigh =
{
	DL_HIGH_TERRAIN_DISTANCE,
	DL_HIGH_PIXEL_ERROR,
	DL_HIGH_SPECULAR_LIGHT,
	DL_HIGH_DYNAMIC_LIGHTING,
	DL_HIGH_FAST_HEADLIGHT,
	DL_HIGH_MIRRORED_SURFACES,
	DL_HIGH_FOG_ENABLED,
	DL_HIGH_CORONAS_ENABLES,
	DL_HIGH_PROCEDURALS,
	DL_HIGH_POWERUP_HALOS,
	DL_HIGH_SCORCH_MARKS,
	DL_HIGH_WEAPON_CORONAS,
	DL_HIGH_SPEC_MAPPING_TYPE,
	DL_HIGH_OBJECT_COMPLEXITY
};
tDetailSettings DetailPresetVHi =
{
	DL_VHI_TERRAIN_DISTANCE,
	DL_VHI_PIXEL_ERROR,
	DL_VHI_SPECULAR_LIGHT,
	DL_VHI_DYNAMIC_LIGHTING,
	DL_VHI_FAST_HEADLIGHT,
	DL_VHI_MIRRORED_SURFACES,
	DL_VHI_FOG_ENABLED,
	DL_VHI_CORONAS_ENABLES,
	DL_VHI_PROCEDURALS,
	DL_VHI_POWERUP_HALOS,
	DL_VHI_SCORCH_MARKS,
	DL_VHI_WEAPON_CORONAS,
	DL_VHI_SPEC_MAPPING_TYPE,
	DL_VHI_OBJECT_COMPLEXITY
};

void ConfigSetDetailLevel(int level)
{
	switch (level)
	{
	case DETAIL_LEVEL_LOW:
		memcpy(&Detail_settings, &DetailPresetLow, sizeof(tDetailSettings));
		break;
	case DETAIL_LEVEL_MED:
		memcpy(&Detail_settings, &DetailPresetMed, sizeof(tDetailSettings));
		break;
	case DETAIL_LEVEL_HIGH:
		memcpy(&Detail_settings, &DetailPresetHigh, sizeof(tDetailSettings));
		break;
	case DETAIL_LEVEL_VERY_HIGH:
		memcpy(&Detail_settings, &DetailPresetVHi, sizeof(tDetailSettings));
		break;
	};

	Default_detail_level = level;
}


//////////////////////////////////////////////////////////////////////////////
// Gamma settings

//gamma configuration, window placement
#define	GAMMA_MENU_H	320
#define	GAMMA_MENU_W	416

#define GAMMA_SLICES			32
#define GAMMA_SLICE_WIDTH	256
#define GAMMA_SLICE_HEIGHT 128
#define GAMMA_SLICE_X		((GAMMA_MENU_W - GAMMA_SLICE_WIDTH)/2);
#define GAMMA_SLICE_Y		64
#define GAMMA_SLIDER_UNITS	100

#define IDV_GAMMAAPPLY	5
#define IDV_AUTOGAMMA	6

void gamma_callback(newuiTiledWindow* wnd, void* data)
{
	int bm_handle = *((int*)data);

	g3Point pnts[4], * pntlist[4];
	rend_SetColorModel(CM_RGB);
	rend_SetAlphaType(AT_ALWAYS);
	rend_SetTextureType(TT_LINEAR);
	rend_SetLighting(LS_NONE);
	rend_SetOverlayType(OT_NONE);
	rend_SetWrapType(WT_WRAP);
	rend_SetZBufferState(0);

	// First draw checkboard
	int startx = GAMMA_SLICE_X;

	for (int i = 0; i < 4; i++)
	{
		pntlist[i] = &pnts[i];
		pnts[i].p3_z = 0;
		pnts[i].p3_r = 1;
		pnts[i].p3_g = 1;
		pnts[i].p3_b = 1;
		pnts[i].p3_flags = PF_PROJECTED;
	}

	pnts[0].p3_sx = startx;
	pnts[0].p3_sy = GAMMA_SLICE_Y;
	pnts[0].p3_u = 0;
	pnts[0].p3_v = 0;

	pnts[1].p3_sx = startx + GAMMA_SLICE_WIDTH;
	pnts[1].p3_sy = GAMMA_SLICE_Y;
	pnts[1].p3_u = 2;
	pnts[1].p3_v = 0;

	pnts[2].p3_sx = startx + GAMMA_SLICE_WIDTH;
	pnts[2].p3_sy = GAMMA_SLICE_Y + GAMMA_SLICE_HEIGHT;
	pnts[2].p3_u = 2;
	pnts[2].p3_v = 1;

	pnts[3].p3_sx = startx;
	pnts[3].p3_sy = GAMMA_SLICE_Y + GAMMA_SLICE_HEIGHT;
	pnts[3].p3_u = 0;
	pnts[3].p3_v = 1;

	rend_DrawPolygon2D(bm_handle, pntlist, 4);

	// Now draw grey in the center
	int int_val = 8;
	rend_SetFlatColor(GR_RGB(int_val, int_val, int_val));
	rend_SetTextureType(TT_FLAT);

	pnts[0].p3_sx += 64;
	pnts[0].p3_sy += 32;

	pnts[1].p3_sx -= 64;
	pnts[1].p3_sy += 32;

	pnts[2].p3_sx -= 64;
	pnts[2].p3_sy -= 32;

	pnts[3].p3_sx += 64;
	pnts[3].p3_sy -= 32;

	rend_DrawPolygon2D(0, pntlist, 4);

	rend_SetWrapType(WT_CLAMP);
	rend_SetZBufferState(1);
}


void config_gamma()
{
	newuiTiledWindow gamma_wnd;
	newuiSheet* sheet;
	tSliderSettings slider_set;
	short* gamma_slider;
	short curpos;
	int res, gamma_bitmap = -1;
	float init_gamma;

	// Make gamma bitmap
	gamma_bitmap = bm_AllocBitmap(128, 128, 0);
	if (gamma_bitmap <= 0) {
		gamma_bitmap = 0;
	}
	else {
		ushort* dest_data = (ushort*)bm_data(gamma_bitmap, 0);
		int addval = 0;
		int i, t;

		// This loop makes the checkerboard
		for (i = 0; i < 128; i++)
		{
			addval = (i & 1) ? 1 : 0;

			for (t = 0; t < 128; t++)
			{
				if (((t + addval) % 2) == 0) {
					dest_data[i * 128 + t] = OPAQUE_FLAG | (1 << 10) | (1 << 5) | (1);
				}
				else {
					dest_data[i * 128 + t] = OPAQUE_FLAG;
				}
			}
		}
	}

	// create ui.
	gamma_wnd.Create(TXT_AUTO_GAMMA, 0, 0, GAMMA_MENU_W, GAMMA_MENU_H);
	gamma_wnd.SetData(&gamma_bitmap);
	gamma_wnd.SetOnDrawCB(gamma_callback);

	sheet = gamma_wnd.GetSheet();

	// ok, cancel buttons.
	sheet->NewGroup(NULL, 130, 224, NEWUI_ALIGN_HORIZ);
	sheet->AddButton(TXT_APPLY, IDV_GAMMAAPPLY);
	sheet->AddText(" ");
	sheet->AddButton(TXT_OK, UID_OK);
	sheet->AddButton(TXT_CANCEL, UID_CANCEL);

	sheet->NewGroup(NULL, 0, 145);
	sheet->AddText(TXT_GAMMAADJUSTA);
	sheet->AddText(TXT_GAMMAADJUSTB);

	// add slider!
	sheet->NewGroup(NULL, 0, 175);
	slider_set.min_val.f = 1.0f;
	slider_set.max_val.f = 3.0f;
	slider_set.type = SLIDER_UNITS_FLOAT;

	init_gamma = Render_preferred_state.gamma;
	curpos = CALC_SLIDER_POS_FLOAT(init_gamma, &slider_set, GAMMA_SLIDER_UNITS);
	gamma_slider = sheet->AddSlider(TXT_GAMMA, 100, curpos, &slider_set, UID_GAMMASLIDER);
	sheet->SetInitialFocusedGadget(UID_GAMMASLIDER);

	// do ui.
	gamma_wnd.Open();

	do
	{
		res = gamma_wnd.DoUI();
		if (res == NEWUIRES_FORCEQUIT) {
			break;
		}
		if (res == IDV_GAMMAAPPLY) {
			//get & set the gamma
			Render_preferred_state.gamma = CALC_SLIDER_FLOAT_VALUE(*gamma_slider, slider_set.min_val.f, slider_set.max_val.f, GAMMA_SLIDER_UNITS);
			rend_SetPreferredState(&Render_preferred_state);
		}
	} while (res != UID_OK && res != UID_CANCEL);

	// handle results.
	if (res == UID_OK) {
		Render_preferred_state.gamma = CALC_SLIDER_FLOAT_VALUE(*gamma_slider, slider_set.min_val.f, slider_set.max_val.f, GAMMA_SLIDER_UNITS);
	}
	else {
		Render_preferred_state.gamma = init_gamma;
	}

	rend_SetPreferredState(&Render_preferred_state);

	// cleanup
	gamma_wnd.Close();
	gamma_wnd.Destroy();

	if (gamma_bitmap != 0) {
		bm_FreeBitmap(gamma_bitmap);
	}
}

//////////////////////////////////////////////////////////////////
// VIDEO MENU
//
#define RESBUFFER_SIZE 50
#define IDV_CHANGEWINDOW 10
#define UID_RESOLUTION 110
struct video_menu
{
	newuiSheet* sheet;

	bool* filtering;									// settings
	bool* mipmapping;
	bool* vsync;

	int* resolution;									// all resolutions
	short* fov;
	char* buffer;
	bool* fullscreen;
	int* antialiasing;

	int window_width, window_height;

	// sets the menu up.
	newuiSheet* setup(newuiMenu* menu)
	{
		sheet = menu->AddOption(IDV_VCONFIG, TXT_OPTVIDEO, NEWUIMENU_MEDIUM);

		sheet->NewGroup(TXT_RESOLUTION, 0, 0);
		buffer = sheet->AddChangeableText(RESBUFFER_SIZE);
		snprintf(buffer, RESBUFFER_SIZE, "%d x %d", Game_window_res_width, Game_window_res_height);
		sheet->AddLongButton("Change...", IDV_CHANGEWINDOW);
		fullscreen = sheet->AddLongCheckBox("Fullscreen", Game_fullscreen);
		tSliderSettings settings = {};
		settings.min_val.f = D3_DEFAULT_FOV;
		settings.max_val.f = 90.f;
		settings.type = SLIDER_UNITS_FLOAT;
		fov = sheet->AddSlider("FOV", settings.max_val.f - settings.min_val.f, Render_FOV_desired - D3_DEFAULT_FOV, &settings);

		window_width = Game_window_res_width;
		window_height = Game_window_res_height;

		// video settings
		sheet->NewGroup(TXT_TOGGLES, 0, 120);
		filtering = sheet->AddLongCheckBox(TXT_BILINEAR, (Render_preferred_state.filtering != 0));
		mipmapping = sheet->AddLongCheckBox(TXT_MIPMAPPING, (Render_preferred_state.mipping != 0));

		sheet->NewGroup(TXT_MONITOR, 0, 180);
		vsync = sheet->AddLongCheckBox(TXT_CFG_VSYNCENABLED, (Render_preferred_state.vsync_on != 0));

		sheet->AddText("");
		sheet->AddLongButton(TXT_AUTO_GAMMA, IDV_AUTOGAMMA);

		sheet->NewGroup("Antialiasing", 184, 0);
		int iTemp = Render_preferred_state.antialised ? 1 : 0;
		antialiasing = sheet->AddFirstRadioButton(TXT_OFF);
		sheet->AddRadioButton(TXT_ON);
		*antialiasing = iTemp;

		return sheet;
	};

	// retreive values from property sheet here.
	void finish()
	{
		if (filtering)
			Render_preferred_state.filtering = (*filtering) ? 1 : 0;
		if (mipmapping)
			Render_preferred_state.mipping = (*mipmapping) ? 1 : 0;
		if (vsync)
			Render_preferred_state.vsync_on = (*vsync) ? 1 : 0;
		if (antialiasing)
			Render_preferred_state.antialised = !!*antialiasing;

		//Hopefully this doesn't do anything cursed..
		rend_SetPreferredState(&Render_preferred_state);

		Render_FOV_desired = fov[0] + D3_DEFAULT_FOV;
		if (Render_FOV != Render_FOV_desired)
			Render_FOV = Render_FOV_desired; //this may cause discontinuities if FOV is changed while zoomed. hmm. 

		if (window_width != Game_window_res_width ||
			window_height != Game_window_res_height ||
			*fullscreen != Game_fullscreen)
		{
			Game_fullscreen = *fullscreen;
			Game_window_res_width = window_width;
			Game_window_res_height = window_height;

			SetScreenMode(GetScreenMode(), true);
			Current_pilot.set_hud_data(NULL, NULL, NULL, &window_width, &window_height);
		}

		sheet = NULL;
	};

	// process
	void process(int res)
	{
		switch (res)
		{
		case IDV_CHANGEWINDOW:
		{
			newuiTiledWindow menu;
			newuiSheet* select_sheet;
			newuiListBox* resolution_list;

			menu.Create("Resolution", 0, 0, 300, 384);
			select_sheet = menu.GetSheet();
			select_sheet->NewGroup(NULL, 10, 0);
			resolution_list = select_sheet->AddListBox(208, 256, UID_RESOLUTION, UILB_NOSORT);
			select_sheet->NewGroup(NULL, 100, 280, NEWUI_ALIGN_HORIZ);
			select_sheet->AddButton(TXT_OK, UID_OK);
			select_sheet->AddButton(TXT_CANCEL, UID_CANCEL);

			for (int i = 0; i < NUM_RESOLUTION; i++)
			{
				resolution_list->AddItem(New_video_res_list[i].name);
			}

			menu.Open();

			//I think this needs to be done after the sheet is realized.
			for (int i = 0; i < NUM_RESOLUTION; i++)
			{
				if (Game_window_res_width == New_video_res_list[i].width &&
					Game_window_res_height == New_video_res_list[i].height)
				{
					resolution_list->SetCurrentIndex(i);
					break;
				}
			}

			int res;
			do
			{
				res = menu.DoUI();
			} while (res != UID_OK && res != UID_CANCEL);

			if (res == UID_OK)
			{
				int newindex = resolution_list->GetCurrentIndex();
				if (newindex < NUM_RESOLUTION) //for custom items
				{
					newVideoResolution& res = New_video_res_list[resolution_list->GetCurrentIndex()];
					window_width = res.width;
					window_height = res.height;
				}

				snprintf(buffer, RESBUFFER_SIZE, "%d x %d", window_width, window_height);

			}

			menu.Close();
			menu.Destroy();

		}
		break;
		case IDV_AUTOGAMMA:
			config_gamma();
			break;
		}
	};
};

//////////////////////////////////////////////////////////////////
// SOUND MENU
//
struct sound_menu
{
	newuiSheet* sheet;
	newuiMenu* parent_menu;
	int ls_sound_id, sound_id;

	short* fxvolume, * musicvolume;			// volume sliders
	short* fxquantity;							// sound fx quantity limit
	int* fxquality;								// sfx quality low/high

	short old_fxquantity;

	// sets the menu up.
	newuiSheet* setup(newuiMenu* menu)
	{
		tSliderSettings slider_set;

		sheet = menu->AddOption(IDV_SCONFIG, TXT_OPTSOUND, NEWUIMENU_MEDIUM);
		parent_menu = menu;
		ls_sound_id = -1;
		sound_id = FindSoundName("Menu Slider Click");
		ASSERT(sound_id != -1);	//DAJ -1FIX

		// volume sliders
		sheet->NewGroup(NULL, 0, 0);

		slider_set.type = SLIDER_UNITS_PERCENT;
		fxvolume = sheet->AddSlider(TXT_SOUNDVOL, 10, (short)(Sound_system.GetMasterVolume() * 10), &slider_set);

		slider_set.type = SLIDER_UNITS_PERCENT;
		musicvolume = sheet->AddSlider(TXT_SNDMUSVOL, 10, (short)(D3MusicGetVolume() * 10), &slider_set);

		// sound fx quality radio list.
		if (GetFunctionMode() != GAME_MODE && GetFunctionMode() != EDITOR_GAME_MODE) {
			sheet->NewGroup(TXT_SNDQUALITY, 0, 95);
#ifdef MACINTOSH
			fxquality = sheet->AddFirstRadioButton(TXT_LOW);
			sheet->AddRadioButton(TXT_CFG_MEDIUM);
			sheet->AddRadioButton(TXT_CFG_HIGH);
			*fxquality = Sound_system.GetSoundQuality();
#else
			fxquality = sheet->AddFirstRadioButton(TXT_SNDNORMAL);
			sheet->AddRadioButton(TXT_SNDHIGH);
			*fxquality = Sound_system.GetSoundQuality() == SQT_HIGH ? 1 : 0;
#endif
			slider_set.min_val.i = MIN_SOUNDS_MIXED;
			slider_set.max_val.i = MAX_SOUNDS_MIXED;
			slider_set.type = SLIDER_UNITS_INT;
			fxquantity = sheet->AddSlider(TXT_SNDCFG_SFXQUANTITY, (slider_set.max_val.i - slider_set.min_val.i),
				Sound_system.GetLLSoundQuantity() - MIN_SOUNDS_MIXED, &slider_set);
			old_fxquantity = (Sound_system.GetLLSoundQuantity() - MIN_SOUNDS_MIXED);

		}
		else
		{
			fxquality = NULL;
			fxquantity = NULL;
		}
		return sheet;
	};

	// retreive values from property sheet here.
	void finish()
	{
		Sound_system.SetMasterVolume((*fxvolume) / 10.0f);
		D3MusicSetVolume((*musicvolume) / 10.0f);

		if (fxquantity)
		{
			mprintf((1, "oldquant %d newquant %d\n", old_fxquantity, *fxquantity));
			if (old_fxquantity != (*fxquantity))
			{
				Sound_system.SetLLSoundQuantity((*fxquantity) + MIN_SOUNDS_MIXED);
			}
		}

		if (fxquality)
		{
			Sound_system.SetSoundQuality((*fxquality == 1) ? SQT_HIGH : SQT_NORMAL);
		}
	};

	// process output and do stuff accordintly
	void process(int res)
	{
		if (sheet->HasChanged(fxvolume))
		{
			Sound_system.SetMasterVolume((*fxvolume) / 10.0f);
			Sound_system.BeginSoundFrame(false);
			Sound_system.StopSoundImmediate(ls_sound_id);
			ls_sound_id = Sound_system.Play2dSound(sound_id);
			Sound_system.EndSoundFrame();
		}
		if (sheet->HasChanged(musicvolume))
		{
			D3MusicSetVolume((*musicvolume) / 10.0f);
		}
	};
};

//////////////////////////////////////////////////////////////////
// GENERAL SETTINGS (TOGGLES) MENU
//

#define UID_SHORTCUT_JOYSETTINGS			0x1000
#define UID_SHORTCUT_KEYSETTINGS			0x1001
#define UID_SHORTCUT_FORCEFEED				0x1002

struct toggles_menu
{
	newuiSheet* sheet;
	newuiMenu* parent_menu;

	int* terrain_autolevel;				// auto leveling radios
	int* mine_autolevel;
	int* missile_view;					// missile view radio
	bool* joy_enabled, * mse_enabled;
	bool* reticle_toggle, * guided_toggle;
	bool* shipsnd_toggle;

	// sets the menu up.
	newuiSheet* setup(newuiMenu* menu)
	{
		sheet = menu->AddOption(IDV_GCONFIG, TXT_OPTGENERAL, NEWUIMENU_MEDIUM);
		parent_menu = menu;
		bool ff_found = false;

		sheet->NewGroup(TXT_TERRAUTOLEV, 0, 0);
		terrain_autolevel = sheet->AddFirstRadioButton(TXT_NONE);
		sheet->AddRadioButton(TXT_CFG_MEDIUM);
		sheet->AddRadioButton(TXT_CFG_HIGH);
		*terrain_autolevel = Default_player_terrain_leveling;

		sheet->NewGroup(TXT_MINEAUTOLEV, 0, 70);
		mine_autolevel = sheet->AddFirstRadioButton(TXT_NONE);
		sheet->AddRadioButton(TXT_CFG_MEDIUM);
		sheet->AddRadioButton(TXT_CFG_HIGH);
		*mine_autolevel = Default_player_room_leveling;

		sheet->NewGroup(TXT_MISSILEVIEW, 0, 140);
		missile_view = sheet->AddFirstRadioButton(TXT_NONE);
		sheet->AddRadioButton(TXT_LEFT);
		sheet->AddRadioButton(TXT_RIGHT);

		*missile_view = (Missile_camera_window == SVW_LEFT) ? 1 : (Missile_camera_window == SVW_RIGHT) ? 2 : 0;

		sheet->NewGroup(TXT_CONTROL_TOGGLES, 110, 0);
		joy_enabled = sheet->AddLongCheckBox(TXT_JOYENABLED);
		mse_enabled = sheet->AddLongCheckBox(TXT_CFG_MOUSEENABLED);
		*joy_enabled = CHECK_FLAG(Current_pilot.read_controller, READF_JOY) ? true : false;
		*mse_enabled = CHECK_FLAG(Current_pilot.read_controller, READF_MOUSE) ? true : false;

		sheet->NewGroup(TXT_TOGGLES, 110, 70);
		reticle_toggle = sheet->AddLongCheckBox(TXT_TOG_SHOWRETICLE);
		guided_toggle = sheet->AddLongCheckBox(TXT_TOG_GUIDEDMISSILE);
		shipsnd_toggle = sheet->AddLongCheckBox(TXT_TOG_SHIPSOUNDS);
		*reticle_toggle = Game_toggles.show_reticle;
		*guided_toggle = Game_toggles.guided_mainview;
		*shipsnd_toggle = Game_toggles.ship_noises;

		ddio_ff_GetInfo(&ff_found, NULL);
		sheet->NewGroup(TXT_ADJUSTCONTROLSETTINGS, 110, 140);
		sheet->AddLongButton(TXT_JOYSENSBTN, UID_SHORTCUT_JOYSETTINGS);
		sheet->AddLongButton(TXT_KEYRAMPING, UID_SHORTCUT_KEYSETTINGS);

		if (ff_found)
		{
			sheet->AddLongButton(TXT_CFG_CONFIGFORCEFEEDBACK, UID_SHORTCUT_FORCEFEED);
		}
		return sheet;
	};

	// retreive values from property sheet here.
	void finish()
	{
		int iTemp;

		Default_player_terrain_leveling = *terrain_autolevel;
		Default_player_room_leveling = *mine_autolevel;

		iTemp = (*missile_view);
		Missile_camera_window = (iTemp == 1) ? SVW_LEFT : (iTemp == 2) ? SVW_RIGHT : -1;
#ifndef MACINTOSH
		Current_pilot.read_controller = (*joy_enabled) ? READF_JOY : 0;
		Current_pilot.read_controller |= (*mse_enabled) ? READF_MOUSE : 0;
#endif
		Game_toggles.show_reticle = (*reticle_toggle);
		Game_toggles.guided_mainview = (*guided_toggle);
		Game_toggles.ship_noises = (*shipsnd_toggle);
	};

	// process
	void process(int res)
	{
		switch (res)
		{
		case UID_SHORTCUT_JOYSETTINGS:
		case UID_SHORTCUT_FORCEFEED:
			LoadControlConfig();
			joystick_settings_dialog();
			SaveControlConfig();
			break;
		case UID_SHORTCUT_KEYSETTINGS:
			LoadControlConfig();
			key_settings_dialog();
			SaveControlConfig();
			break;
		}
	};
};

//////////////////////////////////////////////////////////////////
//  HUD CONFIG MENU
//
struct hud_menu
{
	newuiSheet* sheet;
	newuiMenu* parent_menu;

	int* ship_status;
	int* shield_energy;
	int* weapon_loads;
	int* afterburner;
	int* inventory;
	int* warnings;
	int* countermeasures;
	//	int *goals;

	int add_hud_option(const char* title, int** ptr, int y, int sel, bool graphical)
	{
		const int y_line = 29;

		sheet->NewGroup(title, 0, y, NEWUI_ALIGN_HORIZ);
		*ptr = sheet->AddFirstRadioButton(TXT_OFF);
		if (graphical)
		{
			sheet->AddRadioButton(TXT_TEXT);
			sheet->AddRadioButton(TXT_GRAPHICAL);
		}
		else
		{
			sheet->AddRadioButton(TXT_ON);
		}
		*(*ptr) = sel;
		return y + y_line;
	};

	// sets the menu up.
	newuiSheet* setup(newuiMenu* menu)
	{
		int y, sel;
		ushort stat, grstat;

		sheet = menu->AddOption(IDV_HCONFIG, TXT_OPTHUD, NEWUIMENU_MEDIUM);
		parent_menu = menu;

		Current_pilot.get_hud_data(NULL, &stat, &grstat);

		sel = (stat & STAT_SHIP) ? 1 : (grstat & STAT_SHIP) ? 2 : 0;
		y = add_hud_option(TXT_HUDSHIPSTATUS, &ship_status, 0, sel, true);

		sel = (stat & (STAT_SHIELDS + STAT_ENERGY)) ? 1 : (grstat & (STAT_SHIELDS + STAT_ENERGY)) ? 2 : 0;
		y = add_hud_option(TXT_HUDSHIELDENERGY, &shield_energy, y, sel, true);

		sel = (stat & (STAT_PRIMARYLOAD + STAT_SECONDARYLOAD)) ? 1 : (grstat & (STAT_PRIMARYLOAD + STAT_SECONDARYLOAD)) ? 2 : 0;
		y = add_hud_option(TXT_HUDWEAPONS, &weapon_loads, y, sel, true);

		sel = (stat & STAT_AFTERBURN) ? 1 : (grstat & STAT_AFTERBURN) ? 2 : 0;
		y = add_hud_option(TXT_HUDAFTERBURN, &afterburner, y, sel, true);

		sel = (stat & STAT_WARNING) ? 1 : (grstat & STAT_WARNING) ? 2 : 0;
		y = add_hud_option(TXT_HUDWARNINGS, &warnings, y, sel, true);

		sel = (stat & STAT_CNTRMEASURE) ? 1 : (grstat & STAT_CNTRMEASURE) ? 2 : 0;
		y = add_hud_option(TXT_HUDCNTRMEASURE, &countermeasures, y, sel, true);

		sel = (stat & STAT_INVENTORY) ? 1 : 0;
		y = add_hud_option(TXT_HUDINVENTORY, &inventory, y, sel, false);

		return sheet;
	};

	// retreive values from property sheet here.
	void finish()
	{
		int sel;
		ushort hud_new_stat = STAT_MESSAGES + STAT_CUSTOM;
		ushort hud_new_grstat = 0;

		sel = *ship_status;
		if (sel == 1) hud_new_stat |= STAT_SHIP;
		else if (sel == 2) hud_new_grstat |= STAT_SHIP;
		sel = *shield_energy;
		if (sel == 1) hud_new_stat |= (STAT_SHIELDS + STAT_ENERGY);
		else if (sel == 2) hud_new_grstat |= (STAT_SHIELDS + STAT_ENERGY);
		sel = *weapon_loads;
		if (sel == 1) hud_new_stat |= (STAT_PRIMARYLOAD + STAT_SECONDARYLOAD);
		else if (sel == 2) hud_new_grstat |= (STAT_PRIMARYLOAD + STAT_SECONDARYLOAD);
		sel = *afterburner;
		if (sel == 1) hud_new_stat |= STAT_AFTERBURN;
		else if (sel == 2) hud_new_grstat |= STAT_AFTERBURN;
		sel = *warnings;
		if (sel == 1) hud_new_stat |= STAT_WARNING;
		else if (sel == 2) hud_new_grstat |= STAT_WARNING;
		sel = *countermeasures;
		if (sel == 1) hud_new_stat |= STAT_CNTRMEASURE;
		else if (sel == 2) hud_new_grstat |= STAT_CNTRMEASURE;
		//		sel = *goals;
		//		if (sel==1) hud_new_stat |= STAT_GOALS;
		sel = *inventory;
		if (sel == 1) hud_new_stat |= STAT_INVENTORY;

		Current_pilot.set_hud_data(NULL, &hud_new_stat, &hud_new_grstat);

		// modify current hud stats if in game.
		if ((GetFunctionMode() == EDITOR_GAME_MODE || GetFunctionMode() == GAME_MODE) && GetHUDMode() == HUD_FULLSCREEN)
		{
			SetHUDState(hud_new_stat | STAT_SCORE | (Hud_stat_mask & STAT_FPS), hud_new_grstat);
		}
	};
};

#ifdef MACINTOSH
#pragma mark details --
#endif


//////////////////////////////////////////////////////////////////
// DETAILS MENU
//
struct details_menu
{
	newuiSheet* sheet;
	newuiMenu* parent_menu;

	int* detail_level;									// detail level radio
	int* objcomp;											// object complexity radio
	bool* specmap, * headlight, * mirror,				// check boxes
		* dynamic, * fog, * coronas, * procedurals,
		* powerup_halo, * scorches, * weapon_coronas;
	short* pixel_err,										// 0-27 (1-28)
		* rend_dist;											// 0-120 (80-200)

	int* texture_quality;

	// sets the menu up.
	newuiSheet* setup(newuiMenu* menu)
	{
		int iTemp;
		sheet = menu->AddOption(IDV_DCONFIG, TXT_OPTDETAIL, NEWUIMENU_MEDIUM);
		parent_menu = menu;

		// detail level radio
		Database->read_int("PredefDetailSetting", &Default_detail_level);
		iTemp = Default_detail_level;
		sheet->NewGroup(TXT_CFG_PRESETDETAILS, 0, 0);
		detail_level = sheet->AddFirstRadioButton(TXT_LOW);
		sheet->AddRadioButton(TXT_CFG_MEDIUM);
		sheet->AddRadioButton(TXT_CFG_HIGH);
		sheet->AddRadioButton(TXT_CFG_VERYHIGH);
		sheet->AddRadioButton(TXT_CFG_CUSTOM);
		*detail_level = iTemp;

		// toggles
		sheet->NewGroup(TXT_TOGGLES, 0, 87);
		specmap = sheet->AddLongCheckBox(TXT_SPECMAPPING, Detail_settings.Specular_lighting);
		headlight = sheet->AddLongCheckBox(TXT_FASTHEADLIGHT, Detail_settings.Fast_headlight_on);
		mirror = sheet->AddLongCheckBox(TXT_MIRRORSURF, Detail_settings.Mirrored_surfaces);
		dynamic = sheet->AddLongCheckBox(TXT_DYNLIGHTING, Detail_settings.Dynamic_lighting);
		fog = sheet->AddLongCheckBox(TXT_CFG_ENABLEFOG, Detail_settings.Fog_enabled);
		coronas = sheet->AddLongCheckBox(TXT_CFG_ENABLELIGHTCORONA, Detail_settings.Coronas_enabled);
		procedurals = sheet->AddLongCheckBox(TXT_CFG_PROCEDURALS, Detail_settings.Procedurals_enabled);
		powerup_halo = sheet->AddLongCheckBox(TXT_CFG_POWERUPHALOS, Detail_settings.Powerup_halos);
		scorches = sheet->AddLongCheckBox(TXT_CFG_SCORCHMARKS, Detail_settings.Scorches_enabled);
		weapon_coronas = sheet->AddLongCheckBox(TXT_CFG_WEAPONEFFECTS, Detail_settings.Weapon_coronas_enabled);

		// sliders
		tSliderSettings slider_set;
		sheet->NewGroup(TXT_GEOMETRY, 90, 0);
		iTemp = MAXIMUM_TERRAIN_DETAIL - Detail_settings.Pixel_error - MINIMUM_TERRAIN_DETAIL;
		if (iTemp < 0) iTemp = 0;
		slider_set.min_val.i = MINIMUM_TERRAIN_DETAIL;
		slider_set.max_val.i = MAXIMUM_TERRAIN_DETAIL;
		slider_set.type = SLIDER_UNITS_INT;
		pixel_err = sheet->AddSlider(TXT_TERRDETAIL, MAXIMUM_TERRAIN_DETAIL - MINIMUM_TERRAIN_DETAIL, iTemp, &slider_set);

		slider_set.min_val.i = MINIMUM_RENDER_DIST / 2;
		slider_set.max_val.i = MAXIMUM_RENDER_DIST / 2;
		slider_set.type = SLIDER_UNITS_INT;
		iTemp = (int)(Detail_settings.Terrain_render_distance / ((float)TERRAIN_SIZE)) - MINIMUM_RENDER_DIST;
		if (iTemp < 0) iTemp = 0;
		rend_dist = sheet->AddSlider(TXT_RENDDIST, (MAXIMUM_RENDER_DIST - MINIMUM_RENDER_DIST) / 2, iTemp / 2, &slider_set);

		// object complexity radio
		sheet->NewGroup(TXT_CFG_OBJECTCOMPLEXITY, 174, 87);
		objcomp = sheet->AddFirstRadioButton(TXT_LOW);
		sheet->AddRadioButton(TXT_CFG_MEDIUM);
		sheet->AddRadioButton(TXT_CFG_HIGH);
		*objcomp = Detail_settings.Object_complexity;

		return sheet;
	};

	// retreive values from property sheet here.
	void finish()
	{
		Detail_settings.Coronas_enabled = *coronas;
		Detail_settings.Dynamic_lighting = *dynamic;
		Detail_settings.Fast_headlight_on = *headlight;
		Detail_settings.Fog_enabled = *fog;
		Detail_settings.Mirrored_surfaces = *mirror;
		Detail_settings.Object_complexity = *objcomp;
		Detail_settings.Pixel_error = MAXIMUM_TERRAIN_DETAIL - ((*pixel_err) + MINIMUM_TERRAIN_DETAIL);
		Detail_settings.Powerup_halos = *powerup_halo;
		Detail_settings.Procedurals_enabled = *procedurals;
		Detail_settings.Scorches_enabled = *scorches;
		Detail_settings.Specular_lighting = *specmap;
		Detail_settings.Terrain_render_distance = (((*rend_dist) * 2) + MINIMUM_RENDER_DIST) * ((float)TERRAIN_SIZE);
		Detail_settings.Weapon_coronas_enabled = *weapon_coronas;

		Default_detail_level = *detail_level;
		Database->write("PredefDetailSetting", Default_detail_level);

		sheet = NULL;
	};

	// process output and do stuff accordintly
	void process(int res)
	{
		// check here if the detail level currently set should be custom
		bool changed = sheet->HasChanged(specmap) ||
			sheet->HasChanged(headlight) ||
			sheet->HasChanged(mirror) ||
			sheet->HasChanged(dynamic) ||
			sheet->HasChanged(fog) ||
			sheet->HasChanged(coronas) ||
			sheet->HasChanged(procedurals) ||
			sheet->HasChanged(powerup_halo) ||
			sheet->HasChanged(scorches) ||
			sheet->HasChanged(weapon_coronas) ||
			sheet->HasChanged(objcomp) ||
			sheet->HasChanged(pixel_err) ||
			sheet->HasChanged(rend_dist);

		if (changed)
		{
			// enable custom radio button
			*detail_level = DETAIL_LEVEL_CUSTOM;
		}
		else
		{
			// check if any preset detail has been selected.
			if (sheet->HasChanged(detail_level))
			{
				set_preset_details(*detail_level);
			}
		}

	};

	//	sets detail presets
	void set_preset_details(int setting);
};


//	sets detail presets
void details_menu::set_preset_details(int setting)
{
	tDetailSettings ds;

	switch (setting)
	{
	case DETAIL_LEVEL_LOW:			memcpy(&ds, &DetailPresetLow, sizeof(tDetailSettings)); break;
	case DETAIL_LEVEL_MED:			memcpy(&ds, &DetailPresetMed, sizeof(tDetailSettings)); break;
	case DETAIL_LEVEL_HIGH:			memcpy(&ds, &DetailPresetHigh, sizeof(tDetailSettings)); break;
	case DETAIL_LEVEL_VERY_HIGH:	memcpy(&ds, &DetailPresetVHi, sizeof(tDetailSettings)); break;
	default:
		return;
	};

	//now go through all the config items and set to the new values
	int iTemp = MAXIMUM_TERRAIN_DETAIL - ds.Pixel_error - MINIMUM_TERRAIN_DETAIL;
	if (iTemp < 0)
		iTemp = 0;
	*pixel_err = (short)(iTemp);
	iTemp = (int)((ds.Terrain_render_distance / ((float)TERRAIN_SIZE)) - MINIMUM_RENDER_DIST);
	if (iTemp < 0) iTemp = 0;
	iTemp = iTemp / 2;
	*rend_dist = (short)(iTemp);
	*objcomp = ds.Object_complexity;
	*specmap = ds.Specular_lighting;
	*headlight = ds.Fast_headlight_on;
	*mirror = ds.Mirrored_surfaces;
	*dynamic = ds.Dynamic_lighting;
	*fog = ds.Fog_enabled;
	*coronas = ds.Coronas_enabled;
	*procedurals = ds.Procedurals_enabled;
	*powerup_halo = ds.Powerup_halos;
	*scorches = ds.Scorches_enabled;
	*weapon_coronas = ds.Weapon_coronas_enabled;
}

//////////////////////////////////////////////////////////////////
//	new Options menu
//

// externed from init.cpp
extern void SaveGameSettings();

void OptionsMenu()
{
	newuiMenu menu;
	video_menu video;
	details_menu details;
	sound_menu sound;
	toggles_menu toggles;
	hud_menu hud;

	int res = -1, state = 0;								// state = 0, options menu, 1 = controller config, 2 = quitting.

	while (state != 2)
	{
		if (state == 1) 
		{
			// enter controller config menu
			mprintf((0, "CONTROLLER CONFIG MENU HERE!\n"));
			CtlConfig(CTLCONFIG_KEYBOARD);
			state = 0;									// goto options menu.
		}
		else 
		{
			// open menu

			DoWaitMessage(true);

			menu.Create();
			menu.Open();

			video.setup(&menu);						// setup video menu IDV_VCONFIG
			details.setup(&menu);					// setup details menu.  IDV_DCONFIG
			sound.setup(&menu);						// setup sound menu. IDV_SCONFIG
			toggles.setup(&menu);				 	// setup general menu. IDV_GCONFIG
			hud.setup(&menu);							// setup hud menu. IDV_HCONFIG

			menu.AddSimpleOption(IDV_CCONFIG, TXT_OPTCONFIG);
			menu.AddSimpleOption(UID_CANCEL, TXT_DONE);

			menu.SetCurrentOption(IDV_VCONFIG);

			DoWaitMessage(false);

			// run menu
			do
			{
				res = menu.DoUI();

				// immediate checking of any option ids.
				if (res == UID_CANCEL || res == NEWUIRES_FORCEQUIT) {
					state = 2;	break;				// next pass will quit
				}
				else if (res == IDV_CCONFIG) {
					state = 1;	break;				// next pass will enter controller menu
				}

				// handle any processing needed by current option.
				switch (menu.GetCurrentOption())
				{
				case IDV_DCONFIG:
					details.process(res);
					break;
				case IDV_SCONFIG:
					sound.process(res);
					break;
				case IDV_VCONFIG:
					video.process(res);
					break;
				case IDV_GCONFIG:
					toggles.process(res);
					break;
				}
			} while (1);

			// get settings
			hud.finish();
			toggles.finish();
			sound.finish();
			details.finish();
			video.finish();

			// write them out and close.
			PltWriteFile(&Current_pilot);

			menu.Close();
			menu.Destroy();
		}
	}

	SaveGameSettings();
}
