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

#ifndef GAMEFONT_H
#define GAMEFONT_H

#include "grtext.h"

extern int Game_fonts[];				// D3 font handles

//Font defines

#define NUM_FONTS						6	//How many fonts we have

//Indices for the font handles
#define SMALL_FONT_INDEX				0	//Basic small font, used everywhere. Originally designed for briefing
#define BIG_FONT_INDEX					1	//Basic big Font, used for titles.
#define HUD_FONT_INDEX					2	//Dropshadowed font so can be seen over variable background
#define MENU_FONT_INDEX					3	//Main menu font, also used for some multiplayer messages
#define SMALL_UI_FONT_INDEX				4	//For the new UI
#define BIG_UI_FONT_INDEX				5	//For the new UI

//Handles to the fonts
#define SMALL_FONT						Game_fonts[SMALL_FONT_INDEX]
#define BIG_FONT						Game_fonts[BIG_FONT_INDEX]
#define HUD_FONT						Game_fonts[HUD_FONT_INDEX]
#define MENU_FONT						Game_fonts[MENU_FONT_INDEX]
#define SMALL_UI_FONT					Game_fonts[SMALL_UI_FONT_INDEX]
#define BIG_UI_FONT						Game_fonts[BIG_UI_FONT_INDEX]

//These are equivalencies for the base fonts
#define BRIEFING_FONT					SMALL_FONT
#define BIG_BRIEFING_FONT				BIG_FONT
#define BRIEF_FONT_INDEX				SMALL_FONT_INDEX
#define BBRIEF_FONT_INDEX				BIG_FONT_INDEX
#define MONITOR9_NEWUI_FONT				SMALL_UI_FONT
#define MONITOR15_NEWUI_FONT			BIG_UI_FONT
#define GADGET9_NEWUI_FONT				SMALL_UI_FONT
#define GADGET15_NEWUI_FONT				BIG_UI_FONT

//	loads all game fonts.
void LoadAllFonts();

//Set the HUD font resolution based on the render width
void SelectHUDFont(int rend_with);

#endif
