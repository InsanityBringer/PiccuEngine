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

#include "hud.h"
#include "gauges.h"
#include "grdefs.h"
#include "game.h"
#include "ddio.h"
#include "player.h"
#include "renderer.h"
#include "descent.h"
#include "object.h"
#include "gamefont.h"
#include "polymodel.h"
#include "cockpit.h"
#include "game2dll.h"
#include "ship.h"
#include "pilot.h"
#include "mem.h"
#include "d3music.h"
#include "demofile.h"
#include "stringtable.h"
#include "pstring.h"
#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "gamecinematics.h"
#include "CtlCfgElem.h"
#include "ctlconfig.h"

//////////////////////////////////////////////////////////////////////////////
//	constants

#define FRAMERATE_TIME_DELAY	0.25	//update every quarter second


//////////////////////////////////////////////////////////////////////////////

//	PLAYER SAVEGAME PREFERENCES START
//	determines what's displayable on the hud
#ifdef E3_DEMO 
tStatMask Hud_stat_mask = STAT_TIMER;
#else
tStatMask Hud_stat_mask = 0;
#endif

#define STAT_SCORE STAT_TIMER

//	PLAYER SAVEGAME PREFERENCES END

//	hud aspect 
float Hud_aspect_x = 1.0f;
float Hud_aspect_y = 1.0f;
bool Small_hud_flag = false;							// changes how hud items are drawn in small huds.

// used by reticle system.
static short Ret_x_off, Ret_y_off;
static char Reticle_prefix[PSFILENAME_LEN + 1];

//	Hud mode of display
static tHUDMode Hud_mode = HUD_COCKPIT;

// hud item array.
static tHUDItem HUD_array[MAX_HUD_ITEMS];

#define HUDITEMUSED(_c) (HUD_array[_c].stat ? true : false)

//	hud resource structure
struct sHUDResources  HUD_resources;

// initializes non configurable hud items.
void InitDefaultHUDItems();

//	initializes items based off their type (information, etc.)
void InitHUDItem(int new_item, tHUDItem* item);

//	iterate through entire hud item list to draw.
void RenderHUDItems(tStatMask stat_mask);

//	frees up reticle bitmaps
void FreeReticle();

//	renders the reticle
void RenderReticle();

//	renders missile reticle
void RenderMissileReticle();

//	renders zoom reticle
void RenderZoomReticle();

//	hack!

//	functions in HudDisplay.cpp 
extern void HudDisplayRouter(tHUDItem* item);

bool Hud_show_controls = false;


//////////////////////////////////////////////////////////////////////////////

//	initializes hud.
void InitHUD()
{
	int i;
	extern bool HUD_disabled;			// from gameloop.cpp

	for (i = 0; i < MAX_HUD_ITEMS; i++)
	{
		HUD_array[i].stat = 0;
		HUD_array[i].flags = 0;
		HUD_array[i].id = HUD_INVALID_ID;
	}

	//	load static hud image resources to be used by HudDisplay.cpp
	HUD_resources.arrow_bmp = -1;
	HUD_resources.goal_bmp = -1;
	HUD_resources.goal_complete_bmp = -1;
	HUD_resources.lock_bmp[0] = -1;
	HUD_resources.lock_bmp[1] = -1;
	HUD_resources.wpn_bmp = -1;
	HUD_resources.dot_bmp = -1;
	HUD_resources.antigrav_bmp[0] = -1;
	HUD_resources.antigrav_bmp[1] = -1;
	strcpy(HUD_resources.hud_inf_name, "hud.inf");

	strcpy(Reticle_prefix, "ret");
	Ret_x_off = 0;
	Ret_y_off = 0;

	for (i = 0; i < NUM_SHIELD_GAUGE_FRAMES; i++)
		HUD_resources.shield_bmp[i] = -1;
	HUD_resources.energy_bmp = -1;
	HUD_resources.afterburn_bmp = -1;
	HUD_resources.invpulse_bmp = -1;

	// this call may be unnecessary since it's called again in InitCockpit.
	//	InitReticle(-1,-1);					// initialize reticle system

	// scrollback windows
	ResetGameMessages();					// resets game message rollback screen.
	ResetHUDMessages();
	HUD_disabled = 0;
}


//	closes hud.
void CloseHUD()
{
	CloseHUDMessageConsole();			// closes down rollback console
	CloseGameMessageConsole();			// closes game journal rollback
	ResetGameMessages();					// reset game message system.
	ResetHUDMessages();

	//	free hud items.
	for (int i = 0; i < MAX_HUD_ITEMS; i++)
	{
		if (HUD_array[i].stat)
			FreeHUDItem(i);
	}
}

tHUDItem* GetHUDItem(int id)
{
	return &HUD_array[id];
}

//	initializes hud.
void InitShipHUD(int ship)
{
	//	load static hud image resources to be used by HudDisplay.cpp
	HUD_resources.arrow_bmp = bm_AllocLoadFileBitmap("hudarrow.ogf", 0);
	HUD_resources.goal_bmp = bm_AllocLoadFileBitmap("hudgoal.ogf", 0);
	HUD_resources.goal_complete_bmp = bm_AllocLoadFileBitmap("hudgoalon.ogf", 0);
	HUD_resources.lock_bmp[0] = bm_AllocLoadFileBitmap("hudlockl.ogf", 0);
	HUD_resources.lock_bmp[1] = bm_AllocLoadFileBitmap("hudlockr.ogf", 0);
	HUD_resources.wpn_bmp = bm_AllocLoadFileBitmap("hudicon.ogf", 0);
	HUD_resources.dot_bmp = bm_AllocLoadFileBitmap("huddot.ogf", 0);
	HUD_resources.antigrav_bmp[0] = bm_AllocLoadFileBitmap("hudantigravl.ogf", 0);
	HUD_resources.antigrav_bmp[1] = bm_AllocLoadFileBitmap("hudantigravr.ogf", 0);

	for (int i = 0; i < NUM_SHIELD_GAUGE_FRAMES; i++)
		HUD_resources.shield_bmp[i] = -1;
	HUD_resources.energy_bmp = -1;
	HUD_resources.afterburn_bmp = -1;
	HUD_resources.invpulse_bmp = -1;

	//	sets current hud mode static global
	ubyte hud_tmp;
	Current_pilot.get_hud_data(&hud_tmp);
	Hud_mode = (tHUDMode)hud_tmp;
}


// closes hud for current ship
void CloseShipHUD()
{
	FreeReticle();

	//	free hud resources
	bm_FreeBitmap(HUD_resources.antigrav_bmp[1]);
	bm_FreeBitmap(HUD_resources.antigrav_bmp[0]);
	bm_FreeBitmap(HUD_resources.dot_bmp);
	bm_FreeBitmap(HUD_resources.wpn_bmp);
	bm_FreeBitmap(HUD_resources.lock_bmp[1]);
	bm_FreeBitmap(HUD_resources.lock_bmp[0]);
	bm_FreeBitmap(HUD_resources.goal_complete_bmp);
	bm_FreeBitmap(HUD_resources.goal_bmp);
	bm_FreeBitmap(HUD_resources.arrow_bmp);
}


//	places an item on the hud
void AddHUDItem(tHUDItem* item)
{
	//	find free hud slot.
	int i;

	for (i = 0; i < MAX_HUD_ITEMS; i++)
	{
		if (!HUD_array[i].stat)
		{
			//	we will initialize the hud item.
			InitHUDItem(i, item);
			break;
		}
	}

	if (i == MAX_HUD_ITEMS)
	{
		mprintf((0, "Unable to add hud item (type=%d).\n", item->type));
	}
}


//	resets hud
void ResetHUD()
{
	for (int i = 0; i < MAX_HUD_ITEMS; i++)
	{
		if (HUD_array[i].stat && !(HUD_array[i].flags & HUD_FLAG_LEVEL) && !(HUD_array[i].flags & HUD_FLAG_PERSISTANT))
			FreeHUDItem(i);
	}
}

//clears out HUD level items
void ResetHUDLevelItems()
{
	for (int i = 0; i < MAX_HUD_ITEMS; i++)
	{
		if (HUD_array[i].stat && (HUD_array[i].flags & HUD_FLAG_LEVEL))
			FreeHUDItem(i);
	}
}

// from game.cpp
extern void InitGameScreen(int, int);
extern bool Dedicated_server;

//	manually sets the cockpit display.
void SetHUDMode(tHUDMode mode)
{
	static int saved_window_w = 0, saved_window_h = 0;

	if (Dedicated_server)
		return;

	if (GetScreenMode() != SM_GAME)
	{
#ifdef _DEBUG
		Int3();
#endif
		return;
	}

	//	clear the screen.
	Clear_screen = 4;

	//	free poly model of cockpit if we're not switching to a cockpit mode.
	if (Hud_mode == HUD_COCKPIT && IsValidCockpit())
	{
		if (mode == HUD_LETTERBOX || mode == HUD_OBSERVER)
			QuickCloseCockpit();
		else if (mode != HUD_COCKPIT)
			CloseCockpit();
	}

	//	do cockpit specific thing.
redo_hud_switch:
	switch (mode)
	{
		ushort cp_hud_stat, cp_hud_graphical_stat;

	case HUD_LETTERBOX:
		//	our objective here is to display a letterbox screen for the cockpit.
		//	we save the old width and height of the windows.
		saved_window_w = Game_window_w;
		saved_window_h = Game_window_h;

		Game_window_w = Max_window_w;
		Game_window_h = Max_window_h - (Max_window_h / 3);

		if (Game_window_h > Max_window_h)
			Game_window_h = Max_window_h;

		if (Game_window_w > Max_window_w)
			Game_window_w = Max_window_w;

		Game_window_x = (Max_window_w - Game_window_w) / 2;
		Game_window_y = (Max_window_h - Game_window_h) / 2;

		SetHUDState(STAT_MESSAGES | (Hud_stat_mask & STAT_FPS), 0);
		break;

	case HUD_FULLSCREEN:
		//	our objective here is to display a fullscreen box that is resizable.
		LoadHUDConfig(HUD_resources.hud_inf_name);

		InitGameScreen(saved_window_w ? saved_window_w : Game_window_w, saved_window_h ? saved_window_h : Game_window_h);
		saved_window_w = 0;
		saved_window_h = 0;

		Current_pilot.get_hud_data(NULL, &cp_hud_stat, &cp_hud_graphical_stat);

		SetHUDState(cp_hud_stat | STAT_SCORE | (Hud_stat_mask & STAT_FPS), cp_hud_graphical_stat);

		break;

	case HUD_COCKPIT:
		if (!IsValidCockpit())
		{
			mode = HUD_FULLSCREEN;
			AddHUDMessage(TXT_NOCOCKPIT);
			goto redo_hud_switch;
		}
		if (Hud_mode == HUD_COCKPIT || Hud_mode == HUD_LETTERBOX || Hud_mode == HUD_OBSERVER)
			QuickOpenCockpit();
		else
			OpenCockpit();

		InitGameScreen(saved_window_w ? saved_window_w : Game_window_w, saved_window_h ? saved_window_h : Game_window_h);

		saved_window_w = 0;
		saved_window_h = 0;

		Current_pilot.get_hud_data(NULL, &cp_hud_stat, &cp_hud_graphical_stat);

		SetHUDState(STAT_MESSAGES | STAT_SHIELDS | STAT_ENERGY | STAT_GOALS | STAT_CUSTOM | STAT_AFTERBURN | STAT_INVENTORY | STAT_CNTRMEASURE | STAT_SCORE |
			(Hud_stat_mask & STAT_WARNING) |
			(Hud_stat_mask & STAT_FPS) |
			(Hud_stat_mask & STAT_SCORE),
			(cp_hud_graphical_stat & STAT_WARNING));
		break;

	case HUD_OBSERVER:
		InitGameScreen(saved_window_w ? saved_window_w : Game_window_w, saved_window_h ? saved_window_h : Game_window_h);
		saved_window_w = 0;
		saved_window_h = 0;

		SetHUDState(STAT_MESSAGES | (Hud_stat_mask & STAT_FPS), 0);
		break;

	default:
		Int3();

	}

	// save out current hud mode 
	//JEFF: (only if going from HUD_FULLSCREEN->HUD_COCKPIT or vice-versa)
	if ((Hud_mode == HUD_FULLSCREEN && mode == HUD_COCKPIT) ||
		(Hud_mode == HUD_COCKPIT && mode == HUD_FULLSCREEN))
	{
		ubyte bmode = mode;	//DAJ MAC using enums always int
		Current_pilot.set_hud_data((ubyte*)&bmode);
		mprintf((0, "Saving new hud mode to pilot\n"));
		PltWriteFile(&Current_pilot);
	}

	Hud_mode = mode;
}


//	the current cockpit mode;
tHUDMode GetHUDMode()
{
	return Hud_mode;
}


// toggle the hud between cockput & fullscreen modes
void ToggleHUDMode()
{
	if (GetHUDMode() == HUD_COCKPIT)
		SetHUDMode(HUD_FULLSCREEN);
	else if (GetHUDMode() == HUD_FULLSCREEN)
		SetHUDMode(HUD_COCKPIT);
}

// sets the hud item state(what's visible, how it's drawn, etc)
void SetHUDState(ushort hud_mask, ushort hud_gr_mask)
{
	Hud_stat_mask = (hud_mask | hud_gr_mask);

	// now go through each hud item, check if it's hud stat mask includes the requested hud_gr_mask
	//	if it does, then set hud mask to graphical
	for (int i = 0; i < MAX_HUD_ITEMS; i++)
	{
		HUD_array[i].stat &= ~(STAT_GRAPHICAL);
		if (HUD_array[i].stat & hud_gr_mask)
			HUD_array[i].stat |= STAT_GRAPHICAL;
	}
}


//	initializes items based off their type (information, etc.)
void InitHUDItem(int new_item, tHUDItem* item)
{
	ASSERT(new_item < MAX_HUD_ITEMS);
	ushort stat = 0x0000;

	item->id = new_item;

	switch (item->type)
	{
	case HUD_ITEM_PRIMARY:
		stat = STAT_PRIMARYLOAD;
		break;

	case HUD_ITEM_SECONDARY:
		stat = STAT_SECONDARYLOAD;
		break;

	case HUD_ITEM_SHIELD:
		stat = STAT_SHIELDS;
		break;

	case HUD_ITEM_ENERGY:
		stat = STAT_ENERGY;
		break;

	case HUD_ITEM_AFTERBURNER:
		stat = STAT_AFTERBURN;
		break;

	case HUD_ITEM_INVENTORY:
		stat = STAT_INVENTORY;
		break;

	case HUD_ITEM_SHIPSTATUS:
		stat = STAT_SHIP;
		break;

	case HUD_ITEM_CNTRMEASURE:
		stat = STAT_CNTRMEASURE;
		break;

	case HUD_ITEM_CUSTOMTEXT:
		if (item->data.text)
		{
			char* text = mem_strdup(item->data.text);
			HUD_array[new_item].data.text = text;
		}
		stat = STAT_CUSTOM;
		break;

	case HUD_ITEM_CUSTOMTEXT2:		//malloc buffer to be updated later
		HUD_array[new_item].data.text = (char*)mem_malloc(item->buffer_size);
		HUD_array[new_item].data.text[0] = 0;
		HUD_array[new_item].buffer_size = item->buffer_size;
		stat = STAT_CUSTOM;
		break;

	case HUD_ITEM_CUSTOMIMAGE:
		stat = STAT_CUSTOM;
		break;

	case HUD_ITEM_WARNINGS:
		stat = STAT_WARNING;
		break;

	case HUD_ITEM_GOALS:
	case HUD_ITEM_GOALSTATES:
		stat = STAT_GOALS;
		break;

	case HUD_ITEM_SCORE:
		stat = STAT_SCORE;
		break;

	case HUD_ITEM_TIMER:
		HUD_array[new_item].data.timer_handle = item->data.timer_handle;
		stat = STAT_CUSTOM;
		break;

	default:
		Int3();
	}

	//	copy hud item structure.
	HUD_array[new_item].x = item->x;
	HUD_array[new_item].y = item->y;
	HUD_array[new_item].xa = item->xa;
	HUD_array[new_item].ya = item->ya;
	HUD_array[new_item].xb = item->xb;
	HUD_array[new_item].yb = item->yb;
	HUD_array[new_item].tx = item->tx;
	HUD_array[new_item].ty = item->ty;
	HUD_array[new_item].grscalex = item->grscalex;
	HUD_array[new_item].grscaley = item->grscaley;
	HUD_array[new_item].type = item->type;
	HUD_array[new_item].stat = item->stat | stat;
	HUD_array[new_item].flags = item->flags;
	HUD_array[new_item].alpha = item->alpha;
	HUD_array[new_item].color = item->color;
	HUD_array[new_item].tcolor = item->tcolor;
	HUD_array[new_item].saturation_count = item->saturation_count;
	HUD_array[new_item].render_fn = item->render_fn;
	HUD_array[new_item].id = item->id;
	HUD_array[new_item].dirty_rect.reset();
}

//Returns the item number if there's a customtext2 item, else -1
int FindCustomtext2HUDItem()
{
	for (int i = 0; i < MAX_HUD_ITEMS; i++)
	{
		if (HUD_array[i].stat && (HUD_array[i].type == HUD_ITEM_CUSTOMTEXT2))
			return i;
	}

	return -1;
}


//Updates the customtext2 item, if there is one
void UpdateCustomtext2HUDItem(char* text)
{
	for (int i = 0; i < MAX_HUD_ITEMS; i++)
	{
		if (HUD_array[i].stat && (HUD_array[i].type == HUD_ITEM_CUSTOMTEXT2))
		{
			ASSERT(HUD_array[i].data.text != NULL);
			strncpy(HUD_array[i].data.text, text, HUD_array[i].buffer_size);
			HUD_array[i].data.text[HUD_array[i].buffer_size - 1];
			break;
		}
	}
}


// savegame system hooks
void SGSHudState(CFILE* fp)
{
	tHUDItem* huditem;

	for (int i = 0; i < MAX_HUD_ITEMS; i++)
	{
		// save custom text2 and timer hud items
		huditem = &HUD_array[i];
		if (huditem->stat)
		{
			if (huditem->type == HUD_ITEM_CUSTOMTEXT2)
			{
				cf_WriteShort(fp, (short)huditem->stat);
				cf_WriteByte(fp, (sbyte)huditem->type);
				cf_WriteShort(fp, huditem->x);
				cf_WriteShort(fp, huditem->y);
				cf_WriteInt(fp, huditem->color);
				cf_WriteShort(fp, (short)huditem->flags);
				cf_WriteByte(fp, (sbyte)huditem->alpha);

				cf_WriteShort(fp, (short)huditem->buffer_size);
				cf_WriteString(fp, huditem->data.text);
				mprintf((0, "sg: saved customtext2 (%x,%x,bufsize=%d)\n", huditem->x, huditem->y, huditem->buffer_size));
			}
			else if (huditem->type == HUD_ITEM_TIMER)
			{
				cf_WriteShort(fp, (short)huditem->stat);
				cf_WriteByte(fp, (sbyte)huditem->type);
				cf_WriteShort(fp, huditem->x);
				cf_WriteShort(fp, huditem->y);
				cf_WriteInt(fp, huditem->color);
				cf_WriteShort(fp, (short)huditem->flags);
				cf_WriteByte(fp, (sbyte)huditem->alpha);

				cf_WriteInt(fp, huditem->data.timer_handle);
				mprintf((0, "sg: restored timer (%x,%x,timer_hndl=%d)\n", huditem->x, huditem->y, huditem->data.timer_handle));
			}
			else if (huditem->type == HUD_ITEM_CUSTOMTEXT)
			{
				// commented out because persistent hud messages are custom text, and its a mess to save the current
				// state of hud persistent messages.
				//	cf_WriteShort(fp, (short)huditem->stat);
				//	cf_WriteByte(fp, (sbyte)huditem->type);
				//	cf_WriteShort(fp, huditem->x);
				//	cf_WriteShort(fp, huditem->y);
				//	cf_WriteInt(fp, huditem->color);
				//	cf_WriteShort(fp, (short)huditem->flags);
				//	cf_WriteByte(fp, (sbyte)huditem->alpha);
				//
				//	cf_WriteString(fp, huditem->data.timer_handle);
			}
		}
	}

	cf_WriteShort(fp, 0);					// mark end of saved hud items.

}


bool LGSHudState(CFILE* fp)
{
	//	restore hud items
	ushort stat_mask = 0;

	while ((stat_mask = (ushort)cf_ReadShort(fp)) != 0)
	{
		tHUDItem huditem;
		ubyte item_type = (ubyte)cf_ReadByte(fp);

		memset(&huditem, 0, sizeof(huditem));
		huditem.type = item_type;
		huditem.stat = stat_mask;

		switch (item_type)
		{
			char* buffer;
			extern void RenderHUDTimer(tHUDItem * item);

		case HUD_ITEM_CUSTOMTEXT2:
			huditem.x = cf_ReadShort(fp);
			huditem.y = cf_ReadShort(fp);
			huditem.color = (ddgr_color)cf_ReadInt(fp);
			huditem.flags = (ushort)cf_ReadShort(fp);
			huditem.alpha = (ubyte)cf_ReadByte(fp);

			huditem.buffer_size = (int)cf_ReadShort(fp);
			huditem.render_fn = NULL; 					// use pointer to function void (*fn)(struct tHUDItem *)
			AddHUDItem(&huditem);

			buffer = (char*)mem_malloc(huditem.buffer_size);
			cf_ReadString(buffer, huditem.buffer_size, fp);
			UpdateCustomtext2HUDItem(buffer);
			mem_free(buffer);
			mprintf((0, "lg: restored customtext2 (%x,%x,bufsize=%d)\n", huditem.x, huditem.y, huditem.buffer_size));
			break;

		case HUD_ITEM_TIMER:
			huditem.x = cf_ReadShort(fp);
			huditem.y = cf_ReadShort(fp);
			huditem.color = (ddgr_color)cf_ReadInt(fp);
			huditem.flags = (ushort)cf_ReadShort(fp);
			huditem.alpha = (ubyte)cf_ReadByte(fp);

			huditem.data.timer_handle = cf_ReadInt(fp);
			huditem.render_fn = RenderHUDTimer; 					// use pointer to function void (*fn)(struct tHUDItem *)
			AddHUDItem(&huditem);
			mprintf((0, "lg: restored timer (%x,%x,timer_hndl=%d)\n", huditem.x, huditem.y, huditem.data.timer_handle));
			break;

			//case HUD_ITEM_CUSTOMTEXT:
			//	break;
		default:
			return false;
		}
	}

	return true;
}


//	frees hud items based off their type class.
void FreeHUDItem(int item)
{
	ASSERT(item < MAX_HUD_ITEMS);

	HUD_array[item].stat = 0;
	HUD_array[item].flags = 0;
	HUD_array[item].id = HUD_INVALID_ID;
	if (((HUD_array[item].type == HUD_ITEM_CUSTOMTEXT) || (HUD_array[item].type == HUD_ITEM_CUSTOMTEXT2)) && HUD_array[item].data.text)
	{
		mem_free(HUD_array[item].data.text);
		HUD_array[item].data.text = NULL;
	}
}

extern void RenderHUDScore(tHUDItem* item);

// initializes non configurable hud items.
void InitDefaultHUDItems()
{
	tHUDItem huditem;

	memset(&huditem, 0, sizeof(huditem));
	huditem.type = HUD_ITEM_SCORE;			// KEEP THIS
	huditem.stat = STAT_SCORE;					// This is bound to timer. in release builds, no timer code but just score
	huditem.x = 640;								// position on HUD in 640,480 coordinates
	huditem.y = 5;
	huditem.color = HUD_COLOR;					// you can ignore this if you dont use this value passed into your render function
	huditem.render_fn = RenderHUDScore; 	// use pointer to function void (*fn)(struct tHUDItem *)
	AddHUDItem(&huditem);
}


//	loads in hud configuration file, adds hud items.
void LoadHUDConfig(const char* filename, bool (*fn)(const char*, const char*, void*), void* ext_data)
{
	CFILE* fp;
	char srcline[128];							// One line of mission source
	bool text_pos = false;							// text position defined?

	//	start over.
	ResetHUD();
	InitDefaultHUDItems();

	//	open file
	fp = cfopen(filename, "rt");
	if (!fp)
	{
		mprintf((0, "Unable to find hud.inf file.\n"));
		return;
	}

	//	check if valid cockpit file
	cf_ReadString(srcline, sizeof(srcline), fp);

	if (strcmpi(srcline, "[hud file]") == 0)
	{
		tHUDItem hud_item;

		memset(&hud_item, 0, sizeof(hud_item));
		hud_item.alpha = HUD_ALPHA;
		hud_item.color = HUD_COLOR;
		hud_item.tcolor = HUD_COLOR;

		while (!cfeof(fp))
		{
			char command[32];						// Command read from line.
			char operand[96];						// operand read from line
			char* keyword;							// parsed keyword
			int readcount;							// read-in count

			readcount = cf_ReadString(srcline, sizeof(srcline), fp);
			if (readcount)
			{
				//	we have a line of source.  parse out primary keyword
				//	then parse out remainder.
				keyword = strtok(srcline, " \t=");
				CleanupStr(command, srcline, sizeof(command));
				CleanupStr(operand, srcline + strlen(command) + 1, sizeof(operand));

				//	skip comments
				if (command[0] == '@')
					continue;

				//	find command, 
				if (!strcmpi(command, "type"))
				{
					//	get operand.
					hud_item.type = atoi(operand);
				}
				else if (!strcmpi(command, "pos"))
				{
					//	get two numbers from line
					int ix, iy;
					sscanf(operand, "%d,%d", &ix, &iy);
					hud_item.x = ix;
					hud_item.y = iy;
				}
				else if (!strcmpi(command, "posA"))
				{
					//	get two numbers from line
					int ix, iy;
					sscanf(operand, "%d,%d", &ix, &iy);
					hud_item.xa = ix;
					hud_item.ya = iy;
				}
				else if (!strcmpi(command, "posB"))
				{
					//	get two numbers from line
					int ix, iy;
					sscanf(operand, "%d,%d", &ix, &iy);
					hud_item.xb = ix;
					hud_item.yb = iy;
				}
				else if (!strcmpi(command, "grscale"))
				{
					sscanf(operand, "%f,%f", &hud_item.grscalex, &hud_item.grscaley);
				}
				else if (!strcmpi(command, "textpos"))
				{
					//	get two numbers from line
					int ix, iy;
					sscanf(operand, "%d,%d", &ix, &iy);
					hud_item.tx = ix;
					hud_item.ty = iy;
					text_pos = true;
				}
				else if (!strcmpi(command, "alpha"))
				{
					// get alpha value.
					hud_item.alpha = atoi(operand);
				}
				else if (!strcmpi(command, "sat"))
				{
					// saturation count
					hud_item.saturation_count = atoi(operand);
				}
				else if (!strcmpi(command, "rgb"))
				{
					// saturation count
					int r, g, b;
					sscanf(operand, "%d,%d,%d", &r, &g, &b);
					hud_item.color = GR_RGB(r, g, b);
				}
				else if (!strcmpi(command, "textrgb"))
				{
					int r, g, b;
					sscanf(operand, "%d,%d,%d", &r, &g, &b);
					hud_item.tcolor = GR_RGB(r, g, b);
				}
				else if (!strcmpi(command, "special"))
				{
					hud_item.stat |= STAT_SPECIAL;
				}
				else if (!strcmpi(command, "create"))
				{
					//	create hud item.
					if (!text_pos)
					{
						hud_item.tx = hud_item.x;
						hud_item.ty = hud_item.y;
					}
					hud_item.render_fn = HudDisplayRouter;
					AddHUDItem(&hud_item);

					//	reset hud item.
					memset(&hud_item, 0, sizeof(hud_item));
					hud_item.alpha = HUD_ALPHA;
					hud_item.color = HUD_COLOR;
					hud_item.tcolor = HUD_COLOR;
					text_pos = false;
				}
				else if (!strcmpi(command, "reticleprefix"))
				{
					// copy prefix of reticle bitmaps.
					strcpy(Reticle_prefix, operand);
				}
				else if (!strcmpi(command, "reticleoffset"))
				{
					int x, y;
					sscanf(operand, "%d,%d", &x, &y);
					Ret_x_off = (short)x;
					Ret_y_off = (short)y;
				}
				else if (fn && (*fn)(command, operand, ext_data))
				{
					continue;
				}
				else
				{
					mprintf((0, "Error reading hud file.\n"));
					Int3();					// contact samir.
					break;
				}
			}
		}
	}
	else
	{
		mprintf((0, "Not a valid hud file.\n"));
	}

	// use any reticle specified.
	FreeReticle();
	InitReticle(0, 0);

	cfclose(fp);
}



//////////////////////////////////////////////////////////////////////////////
//	render cockpit and gauges frame
//		checks to see what cockpit mode we are in.
//		draws stuff accordingly.
//		render internal gauges too.

void RenderHUDFrame(float zoom)
{
	extern bool Guided_missile_smallview;			// from smallviews.cpp

	//	Start frame and 3d frame

	// [ISB] Calculate a new 4:3 window from the vertical size
	tHUDMode hudmode = GetHUDMode();

	int temp_window_w = Game_window_w, temp_window_x = Game_window_x;
	if (hudmode == HUD_FULLSCREEN || hudmode == HUD_COCKPIT)
	{
		int game_window_xmid = Game_window_x + Game_window_w / 2;

		Game_window_w = Game_window_h * 4 / 3;
		Game_window_x = game_window_xmid - Game_window_w / 2;
	}

	StartFrame(false);
	g3_StartFrame(&Viewer_object->pos, &Viewer_object->orient, zoom);

	rend_SetOverlayType(OT_NONE);

	// determine hud rendering properties.
	Hud_aspect_x = (float)Game_window_w / DEFAULT_HUD_WIDTH;
	Hud_aspect_y = (float)Game_window_h / DEFAULT_HUD_HEIGHT;
	Small_hud_flag = (((float)Game_window_h / (float)Max_window_h) <= 0.80f) ? true : false;

	bool must_render_cockpit = false;
	//	render special missile hud if available
	if (Players[Player_num].guided_obj && !Guided_missile_smallview)
	{
		if (!Cinematic_inuse)
			RenderMissileReticle();
	}
	else if (Players[Player_num].flags & PLAYER_FLAGS_ZOOMED)
	{
		if (!Cinematic_inuse)
			RenderZoomReticle();
	}
	else if (!(Players[Player_num].flags & PLAYER_FLAGS_REARVIEW))
	{
		switch (hudmode)
		{
		case HUD_FULLSCREEN:
			RenderHUDItems(Hud_stat_mask);
			must_render_cockpit = true; // needed to render animated deactivation sequence and should be dormant								
			if (Game_toggles.show_reticle)
				RenderReticle();
			break;

		case HUD_COCKPIT:
			RenderHUDItems(Hud_stat_mask);
			must_render_cockpit = true; // called when cockpit is activating and functioning.									
			if (Game_toggles.show_reticle)
				RenderReticle();
			break;

		case HUD_LETTERBOX:
			if (!Cinematic_inuse)
				RenderHUDItems(Hud_stat_mask);
			break;

		case HUD_OBSERVER:
			if (!Cinematic_inuse)
				RenderHUDItems(Hud_stat_mask);
			break;

		default:
			Int3();
		}
	}

	// Do dll stuff
	CallGameDLL(EVT_CLIENT_HUD_INTERVAL, &DLLInfo);

	//	End frame
	g3_EndFrame();
	EndFrame();

	if (hudmode == HUD_FULLSCREEN || hudmode == HUD_COCKPIT)
	{
		Game_window_w = temp_window_w;
		Game_window_x = temp_window_x;
	}

	// [ISB] extra pass to render the cockpit so it always uses correct window
	if (must_render_cockpit)
	{
		StartFrame(false);
		g3_StartFrame(&Viewer_object->pos, &Viewer_object->orient, zoom);
		RenderCockpit();
		g3_EndFrame();
		EndFrame();
	}

	rend_SetZBufferState(1);
	mprintf_at((2, 0, 0, "FPS: %f", GetFPS()));
}


// renders hud frame before any graphics are drawn
void RenderPreHUDFrame()
{
	extern void RenderHUDMsgDirtyRects();

	// render any dirty rectangles if small hud flag is set
	if (Small_hud_flag)
	{
		for (int i = 0; i < MAX_HUD_ITEMS; i++)
		{
			if ((Hud_stat_mask & HUD_array[i].stat) && (HUD_array[i].flags & HUD_FLAG_SMALL))
				HUD_array[i].dirty_rect.fill(GR_BLACK);
		}

		// messages.
		if ((Hud_stat_mask & STAT_MESSAGES))
			RenderHUDMsgDirtyRects();
	}
}



extern MsgListConsole Game_msg_con;
extern MsgListConsole HUD_msg_con;

// render auxillary hud
void RenderAuxHUDFrame()
{
	//	take care of any 'small hud' stuff
	//	render hud array list for items drawn differently on a smaller hud.
	if (Small_hud_flag)
	{
		int cur_game_win_w = Game_window_w;
		int cur_game_win_h = Game_window_h;
		int cur_game_win_x = Game_window_x;
		int cur_game_win_y = Game_window_y;
		int i;
		ushort stat_mask = Hud_stat_mask;

		// emulating hud that takes up entire screen
		Game_window_w = Max_window_w;
		Game_window_h = Max_window_h;
		Game_window_x = 0;
		Game_window_y = 0;

		// determine hud rendering properties.
		Hud_aspect_x = (float)Game_window_w / DEFAULT_HUD_WIDTH;
		Hud_aspect_y = (float)Game_window_h / DEFAULT_HUD_HEIGHT;

		for (i = 0; i < MAX_HUD_ITEMS; i++)
		{
			if ((stat_mask & HUD_array[i].stat) && (HUD_array[i].flags & HUD_FLAG_SMALL))
			{
				if (!HUD_array[i].render_fn)
					HudDisplayRouter(&HUD_array[i]);
				else
					(*HUD_array[i].render_fn)(&HUD_array[i]);
			}
		}

		// messages.
		if ((stat_mask & STAT_MESSAGES))
			RenderHUDMessages();

		grtext_Flush();

		Game_window_w = cur_game_win_w;
		Game_window_h = cur_game_win_h;
		Game_window_x = cur_game_win_x;
		Game_window_y = cur_game_win_y;
	}

	// render game and hud message consoles.
	Game_msg_con.DoInput();
	Game_msg_con.Draw();
	HUD_msg_con.DoInput();
	HUD_msg_con.Draw();
}

extern const char* cfg_binding_text(ct_type ctype, ubyte ctrl, ubyte binding);

char* GetControllerBindingText(int fidcont)
{
	static char* cont_bind_txt;
	ct_type ctype[CTLBINDS_PER_FUNC];
	ubyte cfgflags[CTLBINDS_PER_FUNC];
	ct_config_data cfgdata;
	tCfgDataParts cfgparts;
	if (-1 == fidcont)
		return NULL;
	cont_bind_txt = NULL;
	fidcont = CtlFindBinding(fidcont, false);
	if (fidcont == -1)		//DAJ -1FIX
		return NULL;

	Controller->get_controller_function(Cfg_joy_elements[fidcont].fn_id, ctype, &cfgdata, cfgflags);
	parse_config_data(&cfgparts, ctype[0], ctype[1], cfgdata);
	ubyte one_binding = cfgparts.bind_0;
	ubyte one_ctrlbind = cfgparts.ctrl_0;

	cont_bind_txt = (char*)cfg_binding_text(ctype[0], one_ctrlbind, one_binding);
	return cont_bind_txt;
}

char* GetKeyBindingText(int fidkey)
{
	static char* key_bind_txt;
	ct_type ctype[CTLBINDS_PER_FUNC];
	ubyte cfgflags[CTLBINDS_PER_FUNC];
	ct_config_data cfgdata;
	tCfgDataParts cfgparts;
	if (-1 == fidkey)
		return NULL;
	key_bind_txt = NULL;
	fidkey = CtlFindBinding(fidkey, true);
	if (fidkey == -1)		//DAJ -1FIX
		return NULL;

	Controller->get_controller_function(Cfg_key_elements[fidkey].fn_id, ctype, &cfgdata, cfgflags);
	parse_config_data(&cfgparts, ctype[0], ctype[1], cfgdata);
	ubyte one_binding = cfgparts.bind_0;
	ubyte one_ctrlbind = cfgparts.ctrl_0;

	key_bind_txt = (char*)cfg_binding_text(ctype[0], one_ctrlbind, one_binding);
	return key_bind_txt;
}

void DoEnabledControlsLine(char* controlp, char* keyp, char* label, int y, char* axis = NULL)
{
	char control_text[200] = "";

	strcpy(control_text, label);
	strcat(control_text, " : ");
	if (keyp)
		strcat(control_text, keyp);
	if (keyp && (controlp || axis))
		strcat(control_text, " / ");
	if (controlp)
		strcat(control_text, controlp);
	if (controlp && axis)
		strcat(control_text, " / ");
	if (axis)
		strcat(control_text, axis);

	if (keyp || controlp || axis)
		RenderHUDTextFlags(0, GR_GREEN, HUD_ALPHA, 0, 30, y, control_text);

}

extern bool Demo_make_movie;

#define HUD_KEYS_NEXT_LINE	hudconty+=14

// [ISB] When the vertical resolution exceeds this threshold, start scaling the font up.
constexpr int PICCU_FONT_SCALE_THRESHOLD = 1080;

//	iterate through entire hud item list to draw.
void RenderHUDItems(tStatMask stat_mask)
{
	static float framerate_timer = 0;
	static double last_fps;
	float font_aspect_x, font_aspect_y;
	int i;

	grtext_Reset();
	grtext_SetFont(HUD_FONT);

	//	for lores screens, we use different fonts, so DONT SCALE.
	font_aspect_x = (float)Game_window_w / Max_window_w;
	font_aspect_y = (float)Game_window_h / Max_window_h;

	//[ISB] If the screen height goes above 1080, start scaling it extra to compensate.
	//The max stock vertical res was 1200, so this seems like a good cutoff. 
	float extra_scale = 1.0f;
	if (Max_window_h > PICCU_FONT_SCALE_THRESHOLD)
	{
		extra_scale = (float)Max_window_h / PICCU_FONT_SCALE_THRESHOLD;
	}

	if (font_aspect_y <= 0.60f)
		grtext_SetFontScale(0.60f * extra_scale);
	else if (font_aspect_y <= 0.80f)
		grtext_SetFontScale(0.80f * extra_scale);
	else
		grtext_SetFontScale(extra_scale);

	//	do framerate calculations
	framerate_timer -= Frametime;
	while (framerate_timer < 0)
	{
		framerate_timer += FRAMERATE_TIME_DELAY;
		last_fps = GetFPS();
	}

	//	show framerate text gauge
	if (stat_mask & STAT_FPS)
		RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 10, 10, "FPS: %.8f", last_fps);

	// show music spew

#ifdef _DEBUG
	if (Music_debug_verbose)
	{
		int min, sec;
		RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 10, 60, "Music: %s", IsD3MusicOn() ? "ON" : "OFF");
		RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 10, 72, "Region: %d", D3MusicGetRegion());
		RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 10, 84, "Song: %s", Music_type_names[Game_music_info.cur_song]);
		min = (int)(Game_music_info.peace_timer / 60);
		sec = (int)((Game_music_info.peace_timer / 60.0f - (float)min) * 60.0f);
		RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 10, 96, "Peace time: %d:%.2d", min, sec);

		if (Game_music_info.cur_loop_name) {
			RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 10, 108, "Loop: %s", Game_music_info.cur_loop_name);
		}
	}

	if (Viewer_object)
	{
		RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 10, 120, "Position: %.2f %.2f %.2f", Viewer_object->pos.x, Viewer_object->pos.y, Viewer_object->pos.z);
		RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 10, 132, "Forward:  %.2f %.2f %.2f", Viewer_object->orient.fvec.x, Viewer_object->orient.fvec.y, Viewer_object->orient.fvec.z);
	}

	//	show timer
	if (1)
	{
		// timer always displayed in debug builds.
		int min, sec, y;

		min = (int)(Gametime / 60);
		sec = (int)((Gametime / 60.0f - (float)min) * 60.0f);

		y = (Game_mode & GM_MULTI) ? 10 : 40;

		RenderHUDText(HUD_COLOR, HUD_ALPHA, 0, 605, y, "%d:%.2d", min, sec);
	}
#endif

	//	render hud array list.
	for (i = 0; i < MAX_HUD_ITEMS; i++)
	{
		if ((HUD_array[i].flags & HUD_FLAG_SMALL) && Small_hud_flag) {
			continue;		// skip items renderered differently on a small hud if we are in a small hud (see RenderAUXHUDFrame)
		}
		if ((stat_mask & HUD_array[i].stat))
		{
			if (!HUD_array[i].render_fn)
				HudDisplayRouter(&HUD_array[i]);
			else
				(*HUD_array[i].render_fn)(&HUD_array[i]);
		}
	}
	if (Demo_flags == DF_RECORDING)
	{
		RenderHUDTextFlags(HUDTEXT_CENTERED, GR_BLUE, HUD_ALPHA, 0, 10, 300, TXT_RECORDINGDEMO);
	}
	else if (Demo_flags == DF_PLAYBACK)
	{
		if (!Demo_make_movie)
		{
			if (Demo_paused)
				RenderHUDTextFlags(HUDTEXT_CENTERED, GR_BLUE, HUD_ALPHA, 0, 10, 300, TXT_DEMOPAUSED);
			else
				RenderHUDTextFlags(HUDTEXT_CENTERED, GR_BLUE, HUD_ALPHA, 0, 10, 300, TXT_PLAYINGDEMO);
		}
	}

	//This is a big pain in the butt. It's for the training mission.
	if (Hud_show_controls && (Players[Player_num].controller_bitflags != 0xffffffff))
	{
		char* controlp = NULL;
		char* keyp = NULL;
		char* axis = NULL;
		player* pp = &Players[Player_num];
		int fidkey = -1, fidcont = -1;
		int hudconty = 130;

		RenderHUDTextFlags(0, GR_GREEN, HUD_ALPHA, 0, 15, hudconty, TXT_ENABLED_CONTROLS);
		HUD_KEYS_NEXT_LINE;

		if (pp->controller_bitflags & PCBF_FORWARD)
		{
			keyp = GetKeyBindingText(ctfFORWARD_THRUSTKEY);
			controlp = GetControllerBindingText(ctfFORWARD_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_FORWARD, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_REVERSE)
		{
			keyp = GetKeyBindingText(ctfREVERSE_THRUSTKEY);

			controlp = GetControllerBindingText(ctfREVERSE_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_REVERSE, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_LEFT)
		{
			keyp = GetKeyBindingText(ctfLEFT_THRUSTKEY);
			controlp = GetControllerBindingText(ctfLEFT_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_SLIDELEFT, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_RIGHT)
		{
			keyp = GetKeyBindingText(ctfRIGHT_THRUSTKEY);
			controlp = GetControllerBindingText(ctfRIGHT_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_SLIDERIGHT, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_UP)
		{
			keyp = GetKeyBindingText(ctfUP_THRUSTKEY);
			controlp = GetControllerBindingText(ctfUP_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_SLIDEUP, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_DOWN)
		{
			keyp = GetKeyBindingText(ctfDOWN_THRUSTKEY);
			controlp = GetControllerBindingText(ctfDOWN_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_SLIDEDOWN, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_PITCHUP)
		{
			axis = NULL;
			if ((pp->controller_bitflags & PCBF_PITCHUP) && (pp->controller_bitflags & PCBF_PITCHDOWN))
			{
				axis = GetControllerBindingText(ctfPITCH_DOWNAXIS);
			}
			keyp = GetKeyBindingText(ctfPITCH_UPKEY);
			controlp = GetControllerBindingText(ctfPITCH_UPBUTTON);
			if (keyp || controlp || axis)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_PITCHUP, hudconty, axis);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_PITCHDOWN)
		{
			axis = NULL;
			if ((pp->controller_bitflags & PCBF_PITCHUP) && (pp->controller_bitflags & PCBF_PITCHDOWN))
			{
				axis = GetControllerBindingText(ctfPITCH_DOWNAXIS);
			}
			keyp = GetKeyBindingText(ctfPITCH_DOWNKEY);
			controlp = GetControllerBindingText(ctfPITCH_DOWNBUTTON);
			if (keyp || controlp || axis)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_PITCHDOWN, hudconty, axis);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_HEADINGLEFT)
		{
			axis = NULL;
			if ((pp->controller_bitflags & PCBF_HEADINGLEFT) && (pp->controller_bitflags & PCBF_HEADINGRIGHT))
			{
				axis = GetControllerBindingText(ctfHEADING_RIGHTAXIS);
			}
			keyp = GetKeyBindingText(ctfHEADING_LEFTKEY);
			controlp = GetControllerBindingText(ctfHEADING_LEFTBUTTON);
			if (keyp || controlp || axis)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_HEADLEFT, hudconty, axis);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_HEADINGRIGHT)
		{
			axis = NULL;
			if ((pp->controller_bitflags & PCBF_HEADINGLEFT) && (pp->controller_bitflags & PCBF_HEADINGRIGHT))
			{
				axis = GetControllerBindingText(ctfHEADING_RIGHTAXIS);
			}
			keyp = GetKeyBindingText(ctfHEADING_RIGHTKEY);
			controlp = GetControllerBindingText(ctfHEADING_RIGHTBUTTON);
			if (keyp || controlp || axis)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_HEADRIGHT, hudconty, axis);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_BANKLEFT)
		{
			axis = NULL;
			if ((pp->controller_bitflags & PCBF_BANKLEFT) && (pp->controller_bitflags & PCBF_BANKRIGHT))
			{
				axis = GetControllerBindingText(ctfBANK_RIGHTAXIS);
			}
			keyp = GetKeyBindingText(ctfBANK_LEFTKEY);
			controlp = GetControllerBindingText(ctfBANK_LEFTBUTTON);
			if (keyp || controlp || axis)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_BANKLEFT, hudconty, axis);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_BANKRIGHT)
		{
			axis = NULL;
			if ((pp->controller_bitflags & PCBF_BANKLEFT) && (pp->controller_bitflags & PCBF_BANKRIGHT))
			{
				axis = GetControllerBindingText(ctfBANK_RIGHTAXIS);
			}
			keyp = GetKeyBindingText(ctfBANK_RIGHTKEY);
			controlp = GetControllerBindingText(ctfBANK_RIGHTBUTTON);
			if (keyp || controlp || axis)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_BANKRIGHT, hudconty, axis);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_PRIMARY)
		{
			keyp = GetKeyBindingText(ctfFIREPRIMARY_KEY);
			controlp = GetControllerBindingText(ctfFIREPRIMARY_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_PRIMWEAP, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_SECONDARY)
		{
			keyp = GetKeyBindingText(ctfFIRESECONDARY_KEY);
			controlp = GetControllerBindingText(ctfFIRESECONDARY_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_SECWEAP, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
		if (pp->controller_bitflags & PCBF_AFTERBURNER)
		{
			keyp = GetKeyBindingText(ctfAFTERBURN_KEY);
			controlp = GetControllerBindingText(ctfAFTERBURN_BUTTON);
			if (keyp || controlp)
			{
				DoEnabledControlsLine(controlp, keyp, TXT_ENABLED_CONT_AFTERBURN, hudconty);
				HUD_KEYS_NEXT_LINE;
			}
		}
	}

	//	render hud messages. if in a small hud, don't do it now but inside RenderAuxHUDFrame
	if ((stat_mask & STAT_MESSAGES) && !Small_hud_flag)
		RenderHUDMessages();
#if 0
	grtext_SetColor(GR_GREEN);
	grtext_SetAlpha(128);
	grtext_SetFont(BIG_BRIEFING_FONT);
	grtext_CenteredPrintf(0, 300, "Beta Version");
#endif
	grtext_Flush();

}



//////////////////////////////////////////////////////////////////////////////
//	RETICLE SYSTEM

#include "robotfire.h"

#define RET_IMAGE_WIDTH				32
#define RET_IMAGE_HEIGHT			32

#define MAX_RETICLE_ELEMS			18
#define RET_CPRIMARY				0
#define RET_L1PRIMARY				1
#define RET_R1PRIMARY				2
#define RET_L2PRIMARY				3
#define RET_R2PRIMARY				4
#define RET_CPRIMARY2				5
#define RET_CSECONDARY				8
#define RET_L1SECONDARY				9
#define RET_R1SECONDARY				10
#define RET_L2SECONDARY				11
#define RET_R2SECONDARY				12
#define RET_LGUNSIGHT				16
#define RET_RGUNSIGHT				17

/*
$$TABLE_GAMEFILE "ret_pr0a.ogf"
$$TABLE_GAMEFILE "ret_pr0b.ogf"
$$TABLE_GAMEFILE "ret_pr1b.ogf"
$$TABLE_GAMEFILE "ret_pr2b.ogf"
$$TABLE_GAMEFILE "ret_pr3b.ogf"
$$TABLE_GAMEFILE "ret_pr4b.ogf"
$$TABLE_GAMEFILE "ret_pr13a.ogf"
$$TABLE_GAMEFILE "ret_pr24a.ogf"
$$TABLE_GAMEFILE "ret_se0a.ogf"
$$TABLE_GAMEFILE "ret_se1a.ogf"
$$TABLE_GAMEFILE "ret_se2a.ogf"
$$TABLE_GAMEFILE "ret_se3a.ogf"
$$TABLE_GAMEFILE "ret_se4a.ogf"
$$TABLE_GAMEFILE "ret_se0b.ogf"
$$TABLE_GAMEFILE "ret_se1b.ogf"
$$TABLE_GAMEFILE "ret_se2b.ogf"
$$TABLE_GAMEFILE "ret_se3b.ogf"
$$TABLE_GAMEFILE "ret_se4b.ogf"
$$TABLE_GAMEFILE "ret_lguna.ogf"
$$TABLE_GAMEFILE "ret_rguna.ogf"
$$TABLE_GAMEFILE "ret_pr5a.ogf"
$$TABLE_GAMEFILE "ret_pr5b.ogf"

$$TABLE_GAMEFILE "phx_pr0a.ogf"
$$TABLE_GAMEFILE "phx_pr0b.ogf"
$$TABLE_GAMEFILE "phx_pr1b.ogf"
$$TABLE_GAMEFILE "phx_pr2b.ogf"
$$TABLE_GAMEFILE "phx_pr3b.ogf"
$$TABLE_GAMEFILE "phx_pr4b.ogf"
$$TABLE_GAMEFILE "phx_pr13a.ogf"
$$TABLE_GAMEFILE "phx_pr24a.ogf"
$$TABLE_GAMEFILE "phx_se0a.ogf"
$$TABLE_GAMEFILE "phx_se1a.ogf"
$$TABLE_GAMEFILE "phx_se2a.ogf"
$$TABLE_GAMEFILE "phx_se3a.ogf"
$$TABLE_GAMEFILE "phx_se4a.ogf"
$$TABLE_GAMEFILE "phx_se0b.ogf"
$$TABLE_GAMEFILE "phx_se1b.ogf"
$$TABLE_GAMEFILE "phx_se2b.ogf"
$$TABLE_GAMEFILE "phx_se3b.ogf"
$$TABLE_GAMEFILE "phx_se4b.ogf"
$$TABLE_GAMEFILE "phx_lguna.ogf"
$$TABLE_GAMEFILE "phx_rguna.ogf"
$$TABLE_GAMEFILE "phx_pr5a.ogf"
$$TABLE_GAMEFILE "phx_pr5b.ogf"

$$TABLE_GAMEFILE "mag_pr0a.ogf"
$$TABLE_GAMEFILE "mag_pr0b.ogf"
$$TABLE_GAMEFILE "mag_pr1b.ogf"
$$TABLE_GAMEFILE "mag_pr2b.ogf"
$$TABLE_GAMEFILE "mag_pr3b.ogf"
$$TABLE_GAMEFILE "mag_pr4b.ogf"
$$TABLE_GAMEFILE "mag_pr13a.ogf"
$$TABLE_GAMEFILE "mag_pr24a.ogf"
$$TABLE_GAMEFILE "mag_se0a.ogf"
$$TABLE_GAMEFILE "mag_se1a.ogf"
$$TABLE_GAMEFILE "mag_se2a.ogf"
$$TABLE_GAMEFILE "mag_se3a.ogf"
$$TABLE_GAMEFILE "mag_se4a.ogf"
$$TABLE_GAMEFILE "mag_se0b.ogf"
$$TABLE_GAMEFILE "mag_se1b.ogf"
$$TABLE_GAMEFILE "mag_se2b.ogf"
$$TABLE_GAMEFILE "mag_se3b.ogf"
$$TABLE_GAMEFILE "mag_se4b.ogf"
$$TABLE_GAMEFILE "mag_lguna.ogf"
$$TABLE_GAMEFILE "mag_rguna.ogf"
$$TABLE_GAMEFILE "mag_pr5a.ogf"
$$TABLE_GAMEFILE "mag_pr5b.ogf"
*/

//	this should map to the reticle element defines
//		column 0 = off bitmap
//		column 1 = on bitmap (if any).
//		column 2 = left, center or right aligned bitmap.
const char* Reticle_image_names[MAX_RETICLE_ELEMS][3] =
{
	{ "_pr0a.ogf", "_pr0b.ogf", "c" },
	{ "_pr13a.ogf", "_pr1b.ogf", "l" },					// gp1
	{ "_pr24a.ogf", "_pr2b.ogf", "r" },					// gp2
	{ "_pr13a.ogf", "_pr3b.ogf", "l" },					// since off version is same as gp1
	{ "_pr24a.ogf", "_pr4b.ogf", "r" },					// since off version is same as gp2
	{ "_pr5a.ogf", "_pr5b.ogf", "c"},					// center gunpoint version of gp0 (ammo based, single gp)
	{ NULL, NULL, "l"},
	{ NULL, NULL, "l"},
	{ "_se0a.ogf", "_se0b.ogf", "c" },
	{ "_se1a.ogf", "_se1b.ogf", "l" },
	{ "_se2a.ogf", "_se2b.ogf", "r" },
	{ "_se3a.ogf", "_se3b.ogf", "l" },
	{ "_se4a.ogf", "_se4b.ogf", "r" },
	{ NULL, NULL, "l"},
	{ NULL, NULL, "l"},
	{ NULL, NULL, "l"},
	{ "_lguna.ogf", NULL, "l" },
	{ "_rguna.ogf", NULL, "r" }
};



//	reticle slot element array
static struct
{
	int bmp_off;
	int bmp_on;
	char align;									//	'l', 'r', or 'c'  (clever, huh?)
} Reticle_elem_array[MAX_RETICLE_ELEMS];


const int Ret_prim_wb[MAX_WB_GUNPOINTS + 1] =
{
	RET_CPRIMARY, 								// reserved for NON mass driver
	RET_R1PRIMARY,
	RET_L1PRIMARY,
	RET_R2PRIMARY,
	RET_L2PRIMARY,
	-1,
	-1,
	-1,
	RET_CPRIMARY2								// reserved for mass driver slot weapons.
};

const int Ret_sec_wb[MAX_WB_GUNPOINTS + 1] =
{
	RET_CSECONDARY,
	RET_R1SECONDARY,
	RET_L1SECONDARY,
	RET_R2SECONDARY,
	RET_L2SECONDARY,
	-1,
	-1,
	-1,
	-1
};

#define RETMASK_FLAG_AUXPRIMARY0			(1<<8)			// matches 9th bit (bit 8) of reticle mask

static ushort Ret_prim_mask = 0;
static ushort Ret_sec_mask = 0;



//	Initializes Reticle on Hud.  Usually called when weapon changes.
void InitReticle(int primary_slots, int secondary_slots)
{
	for (int i = 0; i < MAX_RETICLE_ELEMS; i++)
	{
		char filename[PSFILENAME_LEN + 1];

		if (Reticle_image_names[i][0] && primary_slots >= 0)
		{
			sprintf(filename, "%s%s", Reticle_prefix, Reticle_image_names[i][0]);
			Reticle_elem_array[i].bmp_off = bm_AllocLoadFileBitmap(IGNORE_TABLE(filename), 0);
			if (Reticle_elem_array[i].bmp_off <= BAD_BITMAP_HANDLE)
			{
				Reticle_elem_array[i].bmp_off = -1;
				mprintf((0, "Unable to load %s reticle image.\n", filename));
			}
		}
		else
			Reticle_elem_array[i].bmp_off = -1;

		if (Reticle_image_names[i][1] && primary_slots >= 0)
		{
			sprintf(filename, "%s%s", Reticle_prefix, Reticle_image_names[i][1]);
			Reticle_elem_array[i].bmp_on = bm_AllocLoadFileBitmap(IGNORE_TABLE(filename), 0);
			if (Reticle_elem_array[i].bmp_on <= BAD_BITMAP_HANDLE)
			{
				mprintf((0, "Unable to load %s reticle image.\n", filename));
				Reticle_elem_array[i].bmp_on = -1;
			}
		}
		else
			Reticle_elem_array[i].bmp_on = -1;

		Reticle_elem_array[i].align = Reticle_image_names[i][2][0];
	}
}


//	frees up bitmaps associated with reticle.
void FreeReticle()
{
	for (int i = 0; i < MAX_RETICLE_ELEMS; i++)
	{
		if (Reticle_elem_array[i].bmp_off > BAD_BITMAP_HANDLE)
			bm_FreeBitmap(Reticle_elem_array[i].bmp_off);
		Reticle_elem_array[i].bmp_off = -1;
		if (Reticle_elem_array[i].bmp_on > BAD_BITMAP_HANDLE)
			bm_FreeBitmap(Reticle_elem_array[i].bmp_on);
		Reticle_elem_array[i].bmp_on = -1;
	}
}


//	resets reticle based off current player object.
//	and the ship's weapon configuration
void ResetReticle()
{
	player* player = &Players[Player_num];
	object* pobj = &Objects[player->objnum];

	ASSERT(player->objnum >= 0);

	// Make sure we're not resetting a non-existent object
	if (!pobj || (pobj->type != OBJ_PLAYER && pobj->type != OBJ_GHOST && pobj->type != OBJ_OBSERVER))
		return;

	poly_model* pm = &Poly_models[Objects[player->objnum].rtype.pobj_info.model_num];
	otype_wb_info* prim_wb = &Ships[player->ship_index].static_wb[player->weapon[PW_PRIMARY].index];
	otype_wb_info* sec_wb = &Ships[player->ship_index].static_wb[player->weapon[PW_SECONDARY].index];
	dynamic_wb_info* prim_dyn_wb = &pobj->dynamic_wb[player->weapon[PW_PRIMARY].index];

	int i, j;

	//	assign reticle elements to the Ret_prim_xx array.
	//	create battery mask
	Ret_prim_mask = 0;

	// iterate through all battery masks to determine which gun points occupy the primary weapon
	// on the player ship!
	for (j = 0; j < prim_wb->num_masks; j++)
	{
		for (i = 0; i < pm->poly_wb[0].num_gps; i++)
		{
			if (prim_wb->gp_fire_masks[j] & (1 << i))
				Ret_prim_mask |= (1 << i);
			else if ((prim_dyn_wb->flags & DWBF_QUAD) && (prim_wb->gp_quad_fire_mask & (1 << i)))
				Ret_prim_mask |= (1 << i);
		}
	}

	if (player->weapon[PW_PRIMARY].index == MASSDRIVER_INDEX && (Ret_prim_mask & (1 << 0)))
	{
		// special hack for mass driver weapons
		Ret_prim_mask &= (~(1 << 0));
		Ret_prim_mask |= RETMASK_FLAG_AUXPRIMARY0;
	}

	// iterate through all battery masks to determine which gun points occupy the secondary weapon
	// on the player ship!
	Ret_sec_mask = 0;

	for (j = 0; j < sec_wb->num_masks; j++)
	{
		for (i = 0; i < pm->poly_wb[0].num_gps; i++)
		{
			if (sec_wb->gp_fire_masks[j] & (1 << i))
				Ret_sec_mask |= (1 << i);
		}
	}
}


//	creates the reticle display bitmap mask to be used by the reticle renderer.
inline ushort reticle_mask(object* pobj, otype_wb_info* static_wb, int wb_index)
{
	poly_model* pm = &Poly_models[pobj->rtype.pobj_info.model_num];
	dynamic_wb_info* dyn_wb = &pobj->dynamic_wb[wb_index];
	unsigned mask = 0;
	int i;

	// determine if weapon battery is charged, if not, then return 0, indicating no 'on' weapon baterries
	if (!WBIsBatteryReady(pobj, static_wb, wb_index))
		return 0;

	if (static_wb->ammo_usage)
	{
		if (Players[pobj->id].weapon_ammo[wb_index] == 0)
			return 0;
	}
	else if (static_wb->energy_usage)
	{
		if (Players[pobj->id].energy < static_wb->energy_usage)
			return 0;
	}

	//	create battery mask
	for (i = 0; i < pm->poly_wb[0].num_gps; i++)
	{
		if (static_wb->gp_fire_masks[dyn_wb->cur_firing_mask] & (1 << i))
			mask |= (1 << i);
		else if ((dyn_wb->flags & DWBF_QUAD) && (static_wb->gp_quad_fire_mask & (1 << i)))
			mask |= (1 << i);
	}

	if (wb_index == MASSDRIVER_INDEX && (mask & (1 << 0)))
	{
		// special hack for mass driver weapons
		mask &= (~(1 << 0));
		mask |= RETMASK_FLAG_AUXPRIMARY0;
	}

	return mask;
}


inline void draw_reticle_sub(int cx, int cy, int rw, int rh, ushort on_mask, ushort gp_mask, const int* wb_elem_array)
{
	int i, x, y;
	int bmp_handle;
	char align;

	for (i = 0; i < (MAX_WB_GUNPOINTS + 1); i++)
	{
		if (gp_mask & (1 << i))
		{
			// we can definitely draw a reticle image, which one is the question now.
			ASSERT(wb_elem_array[i] > -1);
			if (on_mask & (1 << i))
				bmp_handle = Reticle_elem_array[wb_elem_array[i]].bmp_on;
			else
				bmp_handle = Reticle_elem_array[wb_elem_array[i]].bmp_off;
			align = Reticle_elem_array[wb_elem_array[i]].align;

			if (bmp_handle > -1)
			{
				// draw on image based off of alignment to cx,cy and 'align'
				switch (align)
				{
				case 'l':
					x = cx - rw;
					y = cy - rh / 2;
					break;
				case 'r':
					x = cx;
					y = cy - rh / 2;
					break;
				case 'c':
					x = cx - rw / 2;
					y = cy - rh / 2;
					break;
				default:
					x = cx;
					y = cy;
					Int3();
				}

				RenderHUDQuad(x, y, rw, rh, 0, 0, 1, 1, bmp_handle, 192);
			}
		}
	}
}


//	renders the reticle
void RenderReticle()
{
	static ushort primary_index_last_frame = 0xffff;
	static bool quad_primary_last_frame = false;
	player* player = &Players[Player_num];
	object* pobj = &Objects[player->objnum];
	ship* ship = &Ships[player->ship_index];
	player_weapon* prim_pw = &player->weapon[PW_PRIMARY];
	player_weapon* sec_pw = &player->weapon[PW_SECONDARY];
	int prim_wb_index = prim_pw->index;
	int sec_wb_index = sec_pw->index;
	otype_wb_info* prim_wb = &ship->static_wb[prim_wb_index];
	otype_wb_info* sec_wb = &ship->static_wb[sec_wb_index];
	dynamic_wb_info* prim_dyn_wb = &pobj->dynamic_wb[prim_wb_index];

	int cx = Ret_x_off + (FIXED_SCREEN_WIDTH >> 1);
	int cy = Ret_y_off + (FIXED_SCREEN_HEIGHT >> 1) + 6;
	int rw = RET_IMAGE_WIDTH;
	int rh = RET_IMAGE_HEIGHT;

	//	quad weapon check hack (any weapon states that change should be noted here.)
	if (prim_dyn_wb->flags & DWBF_QUAD)
	{
		if (!quad_primary_last_frame || primary_index_last_frame != prim_wb_index)
			ResetReticle();

		quad_primary_last_frame = true;
	}
	else
	{
		if (quad_primary_last_frame)
			ResetReticle();

		quad_primary_last_frame = false;
	}

	primary_index_last_frame = prim_wb_index;

	//	determine which primary batteries are open.
	draw_reticle_sub(cx, cy, rw, rh, reticle_mask(pobj, prim_wb, prim_wb_index), Ret_prim_mask, Ret_prim_wb);
	draw_reticle_sub(cx, cy, rw, rh, reticle_mask(pobj, sec_wb, sec_wb_index), Ret_sec_mask, Ret_sec_wb);

	if (Reticle_elem_array[RET_LGUNSIGHT].bmp_off > -1)
		RenderHUDQuad(cx - rw, cy - rh / 2, rw, rh, 0, 0, 1, 1, Reticle_elem_array[RET_LGUNSIGHT].bmp_off, 192);

	if (Reticle_elem_array[RET_RGUNSIGHT].bmp_off > -1)
		RenderHUDQuad(cx, cy - rh / 2, rw, rh, 0, 0, 1, 1, Reticle_elem_array[RET_RGUNSIGHT].bmp_off, 192);
}


//	renders missile reticle
void RenderMissileReticle()
{
	//	Crosshair reticle
	int cx = Game_window_w / 2;
	int cy = Game_window_h / 2;

	RenderHUDTextFlags(HUDTEXT_CENTERED, GR_RED, HUD_ALPHA, 0, 10, cy - 50, TXT_HUD_GUIDED);
	grtext_Flush();

	rend_SetZBufferState(0);
	rend_SetFlatColor(GR_GREEN);

	rend_DrawLine(cx - 6, cy, cx + 6, cy);
	rend_DrawLine(cx, cy - 6, cx, cy + 6);
	rend_SetZBufferState(1);
}

//	renders missile reticle
void RenderZoomReticle()
{
	//	Crosshair reticle
	int cx = Game_window_w / 2;
	int cy = Game_window_h / 2;
	int text_height = grfont_GetHeight(HUD_FONT);
	char str[255];

	RenderHUDTextFlags(HUDTEXT_CENTERED, GR_RED, HUD_ALPHA, 0, 10, cy - 50, TXT_HUD_ZOOM);

	sprintf(str, TXT_HUD_ZOOM_UNITS, Players[Player_num].zoom_distance);

	RenderHUDTextFlags(HUDTEXT_CENTERED, GR_RED, HUD_ALPHA, 0, 10, cy - 50 + text_height, str);
	grtext_Flush();

	rend_SetZBufferState(0);

	if (Players[Player_num].flags & PLAYER_FLAGS_BULLSEYE)
		rend_SetFlatColor(GR_RED);
	else
		rend_SetFlatColor(GR_GREEN);

	rend_DrawLine(cx - 8, cy, cx + 8, cy);
	rend_DrawLine(cx, cy - 8, cx, cy + 8);
}
