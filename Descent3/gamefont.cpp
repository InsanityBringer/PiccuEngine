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
#include "gamefont.h"
#include "game.h"
#include "grtext.h"
#include "pserror.h"
#include "stringtable.h"

int Game_fonts[NUM_FONTS] = { 0,0,0,0,0,0, };

//HUD font sizes
#define HUD_FONT_LOWRES		0
#define HUD_FONT_HIGHRES	1

char* HUD_font_files[] = { "lohud.fnt","hihud.fnt" };

#define HUD_FONT_FILE HUD_font_files[HUD_font_resolution]

static int HUD_font_resolution = HUD_FONT_LOWRES;
static bool Hud_font_template_init = false;
tFontTemplate Hud_font_template;								// retain hud font template

void FreeAuxFontData()
{
	if (Hud_font_template_init)
	{
		Hud_font_template_init = false;
		grfont_FreeTemplate(&Hud_font_template);
	}
}

//Loads the font and returns the handle.  Exits with an error if the font can't be found.
int LoadFont(char* font_name)
{
	int handle = grfont_Load(font_name);
	if (handle == -1)
		Error(TXT_ERRNOFONT, font_name);

	mprintf((0, "Font <%s> height = %d\n", font_name, grfont_GetHeight(handle)));

	return handle;
}

//	loads all game fonts.
void LoadAllFonts()
{
	grfont_Reset();

	atexit(FreeAuxFontData);

	if (Hud_font_template_init)
		grfont_FreeTemplate(&Hud_font_template);

	//Load the fonts (except HUD)
	SMALL_FONT = LoadFont("briefing.fnt");
	BIG_FONT = LoadFont("bbriefing.fnt");
	MENU_FONT = LoadFont("newmenu.fnt");
	SMALL_UI_FONT = LoadFont("smallui.fnt");
	BIG_UI_FONT = LoadFont("largeui.fnt");

	//Load the HUD font
	HUD_font_resolution = HUD_FONT_LOWRES;
	HUD_FONT = LoadFont(HUD_FONT_FILE);
	if (!grfont_LoadTemplate(HUD_FONT_FILE, &Hud_font_template))
		Error(TXT_ERRNOFONT, HUD_FONT_FILE);
	Hud_font_template_init = true;
}

//If screen width greater than this, use the superhires fonts
#define SUPERHIRES_THRESHOLD_W		1024

//Set the HUD font resolution based on the render width
void SelectHUDFont(int rend_width)
{
	int fontres = (rend_width >= SUPERHIRES_THRESHOLD_W) ? HUD_FONT_HIGHRES : HUD_FONT_LOWRES;

	if (fontres == HUD_font_resolution)
		return;

	HUD_font_resolution = fontres;

	grfont_Free(HUD_FONT);

	HUD_FONT = LoadFont(HUD_FONT_FILE);
}
