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

#include "help.h"
#include "mono.h"
#include "renderer.h"
#include "render.h"
#include "ddio.h"
#include "descent.h"
#include "game.h"
#include "CFILE.H"
#include "application.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "newui.h"
#include "grtext.h"
#include "gamefont.h"
#include "stringtable.h"

#define TITLETEXT	TXT_HELP
int HelpText[] = { TXI_ESC			,	TXI_HLPQUIT,
#ifndef DEMO	//Do Not include in the PC Gamer Demo
					TXI_HLPALTF2	,	TXI_HLPSAVEGAME,
					TXI_HLPALTF3	,	TXI_HLPLOADGAME,
#endif
					TXI_F2			,	TXI_HLPCONFIG,
					TXI_F3			,	TXI_HLPCOCKPIT,
					TXI_HLPF4		,	TXI_HLPGUIDEBOT,
					TXI_F5			,	TXI_TOGGLEDEMO,
					TXI_F6			,	TXI_MLTMENU,
					TXI_F8			,	TXI_HLP_MULTIMSG,
					TXI_F9			,	TXI_HLP_QUIKSAVE,
					TXI_F12			,	TXI_DROPSMARKER,
					TXI_SHFTTAB		,	TXI_TCMM,
	#ifdef MACINTOSH
					TXI_PAGE_DOWN	,	TXI_HLPPAUSEDESC,
	#else
					TXI_HLPPAUSE	,	TXI_HLPPAUSEDESC,
	#endif
					TXI_PLUSMINUS	,	TXI_HLPSCRNSIZE,
					TXI_HLPPRNTSCRN	,	TXI_HLPTAKESCRNSHT,
					TXI_HLP1_5		,	TXI_HLPSELPRIM,
					TXI_HLP6_0		,	TXI_HLPSELSECN,
					TXI_SF1   		,	TXI_HLPREARLEFT,
					TXI_SF2  		,	TXI_HLPREARRIGHT,
					TXI_SHFTF8		,	TXI_DISPLAYGAMEMSGCONSOLE,
					TXI_SHFTF9		,	TXI_DISPLAYHUDMSGCONSOLE,
					0 };


#define HELP_X_KEY_POS	60
#define HELP_X_DESC_POS	160
#define HELP_Y		32


#define IDH_QUIT	UID_CANCEL
#define WND_HELP_W	448
#define WND_HELP_H	384
#define WND_HELP_X	(Game_window_w - WND_HELP_W)/2
#define WND_HELP_Y  (Game_window_h - WND_HELP_H)/2

void HelpDisplay(void)
{
	newuiTiledWindow help_wnd;
	newuiSheet* sheet;
	int strs_to_print = 0;
	int index, res;
	bool exit_menu = false;

	help_wnd.Create(TITLETEXT, 0, 0, WND_HELP_W, WND_HELP_H);
	sheet = help_wnd.GetSheet();

	// add commands
	help_wnd.AddAcceleratorKey(KEY_F1, UID_CANCEL);

	strs_to_print = 0;

	//find out how many strings to print out
	while (HelpText[strs_to_print] > 0) strs_to_print++;

	sheet->NewGroup(NULL, 30, 10);
	for (index = 0; index < strs_to_print; index += 2)
	{
		if (index < strs_to_print)
		{
			sheet->AddText(TXT(HelpText[index]));
		}
	}


	sheet->NewGroup(NULL, 130, 10);
	for (index = 1; index < strs_to_print; index += 2)
	{
		if (index < strs_to_print)
		{
			sheet->AddText(TXT(HelpText[index]));
		}
	}

	sheet->NewGroup(NULL, WND_HELP_W - 160, WND_HELP_H - 96, NEWUI_ALIGN_HORIZ);
	sheet->AddButton(TXT_OK, UID_CANCEL);

	//	quit_hot.Create(&help_wnd, UID_CANCEL, 0, &UITextItem(TXT_PRESSESCRET, UICOL_HOTSPOT_LO,UIALPHA_HOTSPOT_LO), 
	//									&UITextItem(TXT_PRESSESCRET,UICOL_HOTSPOT_HI,UIALPHA_HOTSPOT_HI), 
	//									HELP_X_KEY_POS, WND_HELP_H - OKCANCEL_YOFFSET, 0,0,UIF_FIT|UIF_CENTER);
	help_wnd.Open();

	while (!exit_menu)
	{
		res = help_wnd.DoUI();

		// handle all UI results.
		switch (res)
		{
		case UID_CANCEL:
		case NEWUIRES_FORCEQUIT:
			exit_menu = true;
			break;
		}
	}

	help_wnd.Close();
	help_wnd.Destroy();
}
