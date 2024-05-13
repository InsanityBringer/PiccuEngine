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

#include "TelComCargo.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <algorithm>

#include "CFILE.H"
#include "pserror.h"
#include "ddio.h"
#include "bitmap.h"

#include "TelCom.h"
#include "renderer.h"
#include "game.h"
#include "mem.h"
#include "stringtable.h"
#include "Inventory.h"
#include "player.h"
#include "gametexture.h"
#include "textaux.h"
#include "TelComEfxStructs.h"
#include "TelComEffects.h"
#include "weapon.h"
#include "hlsoundlib.h"

#define TCBACK_BUTTON_X		500
#define TCBACK_BUTTON_Y		350
#define TCFORW_BUTTON_X		532
#define TCFORW_BUTTON_Y		350

#define LIT_TITLE		0
#define LIT_VALUE		1

#define LID_NONE				-1
#define LID_SHIELDS				0
#define LID_ENERGY				1
#define LID_PRIMARIES			2
#define LID_SECONDARIES			3
#define LID_COUNTERMEASURES		4
#define LID_INVENTORY			5

typedef struct tLineInfo
{
	ubyte type;
	char *name;
	int id;
};

tLineInfo StatusLines[] = 
{
	{LIT_TITLE,"Ship Status",		LID_NONE},
	{LIT_VALUE,"Shields",			LID_SHIELDS},
	{LIT_VALUE,"Energy",			LID_ENERGY},
	{LIT_TITLE,"Primaries",			LID_PRIMARIES},
	{LIT_TITLE,"Secondaries",		LID_SECONDARIES},
	{LIT_TITLE,"Counter Measures",	LID_COUNTERMEASURES},
	{LIT_TITLE,"Inventory",			LID_INVENTORY}
};

#define NUM_LINES	(sizeof(StatusLines)/sizeof(tLineInfo))
#define TITLE_X		30+TCminx
#define VALUE_X		400+TCminx
int TCminx,TCmaxx,TCminy,TCmaxy;
#define SM_FONT	BRIEFING_FONT
#define BG_FONT	BIG_BRIEFING_FONT

// Returns true if the player has this weapon (this fuction should be moved)
bool TCCPlayerHasWeapon (int weapon_index)
{
	if (Players[Player_num].weapon_flags & HAS_FLAG(weapon_index))
		return true;
	return false;
}

int TCCargoCreateLine(int id,int y,char *title,int type)
{
	//char buffer[100];
	int small_height = grfont_GetHeight(SM_FONT);
	int big_height = grfont_GetHeight(BG_FONT);

	switch(id){
	case LID_NONE:
		return y;

	case LID_SHIELDS:
		{
			grtext_SetFont(SM_FONT);

			float shields = Player_object->shields;
			shields = std::max(shields,0.f);			
			int perc = (int) ((shields/INITIAL_SHIELDS)*100.0f);
			grtext_Printf(TITLE_X,y,title);
			grtext_Printf(VALUE_X,y,"%d%c",perc,'%'); y+=small_height;
			
		}break;

	case LID_ENERGY:
		{
			grtext_SetFont(SM_FONT);

			float energy = Players[Player_num].energy;
			energy = std::max(energy,0.f);			
			int perc = (int) ((energy/INITIAL_ENERGY)*100.0f);
			grtext_Printf(TITLE_X,y,title);
			grtext_Printf(VALUE_X,y,"%d%c",perc,'%'); y+=small_height;

		}break;

	case LID_PRIMARIES:
		{
			grtext_SetFont(SM_FONT);
			int prim_start = PRIMARY_INDEX;
			int prim_end = (PRIMARY_INDEX<SECONDARY_INDEX)?SECONDARY_INDEX:MAX_STATIC_WEAPONS;
			int weap_index;
			weapon *weap;

			ASSERT( prim_start >= 0 );
			ASSERT( prim_end <= MAX_STATIC_WEAPONS );
			ASSERT( prim_start < prim_end );

			for( weap_index = prim_start; weap_index<prim_end; weap_index++ ){
				if(TCCPlayerHasWeapon (weap_index)){
					weap = &Weapons[weap_index];
					grtext_Printf(TITLE_X,y,Static_weapon_names[weap_index]); y+=small_height;
				}
			}

		}break;

	case LID_SECONDARIES:
		{
			grtext_SetFont(SM_FONT);
			int sec_start = SECONDARY_INDEX;
			int sec_end = (SECONDARY_INDEX<PRIMARY_INDEX)?PRIMARY_INDEX:MAX_STATIC_WEAPONS;
			int weap_index;
			weapon *weap;

			ASSERT( sec_start >= 0 );
			ASSERT( sec_end <= MAX_STATIC_WEAPONS );
			ASSERT( sec_start < sec_end );

			for( weap_index = sec_start; weap_index<sec_end; weap_index++ ){
				if(TCCPlayerHasWeapon (weap_index)){
					weap = &Weapons[weap_index];
					grtext_Printf(TITLE_X,y,Static_weapon_names[weap_index]); y+=small_height;
				}
			}

		}break;


	case LID_COUNTERMEASURES:
		break;

	case LID_INVENTORY:
		break;
	}

	return y;
}

void TCCargoButtonCallback(int efxnum)
{
	//tccargo_button_pressed = efxnum;
}


void TCCargoRenderCallback(void)
{
	grtext_SetFont(BRIEFING_FONT);
	grtext_SetAlpha(255);
	rend_SetOverlayType (OT_NONE);
	rend_SetLighting (LS_NONE);
	rend_SetColorModel (CM_MONO);
	rend_SetZBufferState (0);
	rend_SetAlphaType (AT_CONSTANT+AT_TEXTURE);
	rend_SetAlphaValue(255);	
	rend_SetZBufferState (0);
	//update inventory rendering	

	int y = TCminy;
	int id;
	grtext_SetAlpha(255);
	grtext_SetColor(GR_WHITE);

	for( int i = 0; i < NUM_LINES; i++ )
	{
		id = StatusLines[i].id;

		y = TCCargoCreateLine(id,y,StatusLines[i].name,StatusLines[i].type);
	}

	grtext_Flush();

	rend_SetZBufferState (1);
}

// This is the function called by TelCom
//  return true if TelCom should exit to TelCom Main Menu
//  return false if you should exit out of TelCom completly
bool TelComCargo(tTelComInfo *tcs)
{
	//int efxmap[2];
	bool done = false;

	TelcomStartScreen(0);

	TCBKGDESC backg;
	backg.color = BACKGROUND_COLOR;
	backg.caps = TCBGD_COLOR;
	backg.type = TC_BACK_STATIC;
	CreateBackgroundEffect(&backg,MONITOR_MAIN,0);
	CreateBackgroundEffect(&backg,MONITOR_TOP,0);
	
	//Setup On-Screen buttons
	/*
	TCBUTTONDESC buttdesc;
	strcpy(buttdesc.filename,BACK_BUTTON);
	strcpy(buttdesc.flash_filename,BACK_BUTTON_GLOW);
	buttdesc.sibling_id = buttdesc.parent_id = -1;
	buttdesc.internal = TCCargoButtonCallback;
	buttdesc.flash_time = 0;
	buttdesc.button_type = BUTT_INTERNAL;
	buttdesc.flasher = true;
	buttdesc.click_type = CLICKTYPE_CLICKUP;
	buttdesc.osflags = OBF_GLOW;
	buttdesc.x = TCBACK_BUTTON_X;
	buttdesc.y = TCBACK_BUTTON_Y;
	buttdesc.tab_stop = true;	//these buttons can have focus
	efxmap[0] = CreateButtonEffect(&buttdesc,MONITOR_MAIN,0);
	strcpy(buttdesc.filename,FORW_BUTTON);
	strcpy(buttdesc.flash_filename,FORW_BUTTON_GLOW);
	buttdesc.x = TCFORW_BUTTON_X;
	buttdesc.y = TCFORW_BUTTON_Y;
	efxmap[1] = CreateButtonEffect(&buttdesc,MONITOR_MAIN,0);
	*/
	
	TCminx = tcs->Monitor_coords[MONITOR_MAIN].left + 20;
	TCminy = tcs->Monitor_coords[MONITOR_MAIN].top +10;
	TCmaxy = tcs->Monitor_coords[MONITOR_MAIN].bottom - 10;
	TCmaxx = tcs->Monitor_coords[MONITOR_MAIN].right - 20;

	TCTEXTDESC textdesc;
	textdesc.type = TC_TEXT_STATIC;
	textdesc.font = BBRIEF_FONT_INDEX;
	textdesc.caps = TCTD_TEXTBOX|TCTD_COLOR|TCTD_FONT;
	textdesc.textbox.left = 75;
	textdesc.textbox.right = 380;
	textdesc.textbox.top = 2;
	textdesc.textbox.bottom = 50;
	textdesc.color = GR_RGB(255,255,255);

	CreateTextEffect(&textdesc,TXT_TCCARGO,MONITOR_TOP,0);
	TelcomEndScreen();
	TelcomRenderSetScreen(0);

	TelcomRenderSetCallback(TCCargoRenderCallback);

	while(!done)
	{
		Sound_system.BeginSoundFrame(Telcom_called_from_game);

		if(tcs->state!=TCS_POWERON || tcs->current_status!=TS_CARGO){
			//we're done with the loop
			done = true;
		}

		/*
		if(tccargo_button_pressed!=-1){
			if(tccargo_button_pressed==efxmap[0])
				curr_cargo_page = &Regular;
			else
			if(tccargo_button_pressed==efxmap[1])
				curr_cargo_page = &CounterMeasures;
			tccargo_button_pressed = -1;
		}
		*/

		//Process all waiting events for the TelCom	(we may only want to handle this once a frame!)
		TelComHandleAllEvents(&Telcom_system);

		TelcomRenderScreen();
		Descent->defer();
		if(KEY_STATE(KEY_ESC))
			Telcom_system.state = TCS_POWEROFF;

		Sound_system.EndSoundFrame();
	}

	DestroyAllScreens();
	TelcomRenderSetScreen(DUMMY_SCREEN);
	TelcomRenderSetCallback(NULL);

	return true;
}
