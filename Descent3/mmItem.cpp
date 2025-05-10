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

#include <string>

#include "mmItem.h"
#include "game.h"
#include "program.h"
#include "descent.h"
#include "cinematics.h"
#include "hlsoundlib.h"
#include "soundload.h"
#include "d3music.h"

#include "ddio.h"
//#include <malloc.h>
#include "mem.h"

#include <string.h>

#ifdef SDL3
#include <SDL3/SDL_version.h>
#endif

// externed from newui.cpp
extern int UI_frame_result;

static mmInterface* MM_object = NULL;
// attached window.
mmInterface* mmItem::m_window = NULL;


char* MMSoundFiles[N_MM_SOUNDS] = { "MenuBeepEnter","MenuBeepSelect" };
void PlayMenuSound(int sound_index, bool wait_till_done = false);

/*
$$TABLE_SOUND "MenuBeepEnter"
$$TABLE_SOUND "MenuBeepSelect"
*/


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

mmItem::mmItem()
{
	m_text = NULL;
}


mmItem::~mmItem()
{
	if (m_text)
		mem_free(m_text);
}


//	attaches a window to all mmitems.
void mmItem::AttachInterface(mmInterface* wnd)
{
	mmItem::m_window = wnd;
}


void mmItem::Create(int id, int key, int x, int y, const char* text, int flags, tmmItemFX init_fx)
{
	m_alpha = 0;
	m_satcount = 0;
	m_fxqueue.flush();

	m_text = text ? mem_strdup(text) : NULL;
	m_fxqueue.send(init_fx);
	m_curfx = mmItemFXNull;
	m_process_speed = 0.0f;

	//	set colors
	m_colors[0] = MMITEM_COLOR;
	m_last_frametime = UI_TIME();

	UIGadget::Create(m_window, id, x, y, 0, 0, UIF_FIT | flags);
	UIGadget::SetHotkey(key);
}


void mmItem::AddEffect(tmmItemFX fx)
{
	m_fxqueue.send(fx);
}


void mmItem::ClearEffects()
{
	m_fxqueue.flush();
}


// override: called when resized or before drawing.
void mmItem::OnFormat()
{
	if (m_Flags & UIF_FIT) 
	{
		ui_DrawSetFont(MMITEM_FONT);

		m_W = (m_text ? ui_GetTextWidth(m_text) : 0);
		m_H = (m_text ? ui_GetTextHeight(m_text) : 0);
	}

	UIGadget::OnFormat();
}


// override: behavior when gadget is destroyed.
void mmItem::OnDestroy()
{
	if (m_text) 
	{
		mem_free(m_text);
		m_text = NULL;
	}
}


void mmItem::OnSelect()
{
	PlayMenuSound(m_window->SoundHandles[MM_SELECT_SOUND], true);
	UIGadget::OnSelect();
}


// override: behavior when mouse button is pressed.
void mmItem::OnMouseBtnDown(int btn)
{
	if (btn == UILMSEBTN) 
	{
		LOCK_FOCUS(this);
	}
}


// override: behavior when mouse button is released.
void mmItem::OnMouseBtnUp(int btn)
{
	if (btn == UILMSEBTN) 
	{
		if (HasFocus()) 
		{
			if (PT_IN_GADGET(UIGadget::m_Wnd, this, UI_input.mx, UI_input.my)) 
				OnSelect();
		}
		UNLOCK_FOCUS(this);
	}
}


// override: behavior when gadget loses input focus.
void mmItem::OnLostFocus()
{
	AddEffect(mmItemFXFadeOutToNormal);
}


// override: behavior when gadget gains input focus.
void mmItem::OnGainFocus()
{
	AddEffect(mmItemFXFadeInToBright);
	PlayMenuSound(m_window->SoundHandles[MM_FOCUS_SOUND]);
}


// override: behavior when gadget is processed
void mmItem::OnUserProcess()
{
	//	if (HasFocus() && !PT_IN_GADGET(m_Wnd, this, UI_input.mx, UI_input.my)) {
	//		LostFocus();
	//	}
}


// override: behavior when gadget is being drawn.
void mmItem::OnDraw()
{
	//	if not disabled, do fancy effects.
	if (m_text) 
	{
		ui_DrawSetFont(MMITEM_FONT);
		ui_DrawSetTextType(0);
		ui_SetCharAlpha(255);
		ui_DrawString(m_colors[0], 0, 0, m_text);
		ui_DrawSetTextType(UI_TEXTTYPE_SATURATE);
		ui_SetCharAlpha((ubyte)m_alpha);
		for (int i = 0; i < m_satcount; i++)
			ui_DrawString(m_colors[0], 0, 0, m_text);
	}

	if ((UI_TIME() - m_last_frametime) >= m_process_speed) 
	{
		// process special fx.
		switch (m_curfx)
		{
		case mmItemFXNormal:
			m_alpha = MMITEM_ALPHA;
			m_satcount = MMITEM_SAT;
			m_process_speed = 0.0f;
			m_curfx = mmItemFXNull;
			break;

		case mmItemFXFadeOutToNormal:
			m_process_speed = 0.075f;
			m_satcount = (m_satcount < MMITEM_SAT) ? MMITEM_SAT : (m_satcount - 1);
			if (m_satcount == MMITEM_SAT) 
				m_curfx = mmItemFXNull;
			
			break;

		case mmItemFXFadeInToBright:
			m_process_speed = 0.075f;
			m_satcount = (m_satcount > 2) ? 2 : (m_satcount + 1);
			if (m_satcount == 2) 
				m_curfx = mmItemFXNull;
			
			break;
		}

		// grab new fx.
		if (m_curfx == mmItemFXNull) 
		{
			tmmItemFX newfx;
			if (m_fxqueue.recv(&newfx)) 
			{
				m_process_speed = 0.0f;
				m_curfx = newfx;
			}
		}

		m_last_frametime = UI_TIME();
	}
}


//////////////////////////////////////////////////////////////////////////////
//	MAIN MENU INTERFACE CODE

#include "stdlib.h"		//need this to get _MAX_PATH, which really ought to be in ddio.h

bool static_menu_background = false;

void mmInterface::Create()
{
	MM_object = this;

	UIWindow::Create(0, 0, Max_window_w, Max_window_h);
	mmItem::AttachInterface(this);

#ifndef MOVIE_MENU
#if defined(OEM)
	if (!LoadLargeBitmap("oemmenu.ogf", &m_art)) 
		Error("Unable to load main menu art oemmenu.ogf.");
#elif defined(DEMO)
	if (!LoadLargeBitmap("demomenu.ogf", &m_art))
		Error("Unable to load main menu art demomenu.ogf.");
#else
	if (!LoadLargeBitmap("mainmenu.ogf", &m_art))
		Error("Unable to load main menu art mainmenu.ogf.");
#endif
#else
	if (cfexist("mainmenuoverride.ogf"))
	{
		if (!LoadLargeBitmap("mainmenuoverride.ogf", &m_art))
			Error("Unable to load main menu art mainmenuoverride.ogf.");
		
		m_movie = NULL;
		static_menu_background = true;
	}
	else
	{
		char filename[_MAX_PATH];
		ddio_MakePath(filename, Base_directory, "movies", "mainmenu", NULL);
		m_movie = StartMovie(filename, true);
	}
#endif

	// load sounds and music
	for (int i = 0; i < N_MM_SOUNDS; i++)
		SoundHandles[i] = FindSoundName(IGNORE_TABLE(MMSoundFiles[i]));

	D3MusicStart("mainmenu.omf");
	SetMusicRegion(MM_MUSIC_REGION);

	SetUICallback(MenuScene);
	m_nmenu_items = 0;
}


//	does user interface.
int mmInterface::DoUI()
{
	UI_frame_result = -1;
	ui_ShowCursor();

	SetUICallback(MenuScene);

	while (UI_frame_result == -1)
	{
		Descent->defer();
		DoUIFrame();
		rend_Flip();
	}

	ui_HideCursor();
	ui_Flush();

	return UI_frame_result;
}


//	add item.
bool mmInterface::AddItem(int id, int key, const char* text, int type)
{
	if (m_nmenu_items == N_MMENU_ITEMS) 
	{
		Int3();										// N_MMENU_ITEMS needs to be upped! (Samir)
		return false;
	}

	m_menuitems[m_nmenu_items].Create(id, key, MMITEM_X, MMITEM_Y + (m_nmenu_items * 20), text, (type == 1) ? UIF_GROUP_START : (type == 2) ? UIF_GROUP_END : (type == 3) ? (UIF_GROUP_START + UIF_GROUP_END) : 0);
	m_nmenu_items++;

	return true;
}


// When interface is being nuked.
void mmInterface::OnDestroy()
{
	UIWindow::OnDestroy();

	D3MusicStop();
	Sound_system.StopAllSounds();


	if (static_menu_background) 
	{
		FreeLargeBitmap(&m_art);
	}
	else 
	{
#ifdef MOVIE_MENU
		if (m_movie) 
		{
			EndMovie(m_movie);
			m_movie = NULL;
		}
#else
		FreeLargeBitmap(&m_art);
#endif
	}

	SetUICallback(DEFAULT_UICALLBACK);
	MM_object = NULL;
}


// displays the copyright text
void mmInterface::CopyrightText()
{
	int i;
	std::string typestr = "";

	if (PROGRAM(beta))
		typestr += "Beta ";

	if (Program_version.version_type == DEVELOPMENT_VERSION)
		typestr += "Dev ";
	else if (Program_version.version_type == RELEASE_VERSION)
		typestr += "Ver ";

	//Build engine name str
	std::string engstr = ENGINE_NAME;
	engstr.push_back(' ');
	engstr += GIT_DESCRIPTION;

	//Get 7 character hash
	char hashbuf[8];
	strncpy(hashbuf, GIT_HASH, sizeof(hashbuf) - 1);
	hashbuf[sizeof(hashbuf) - 1] = '\0';

	int x = Max_window_w - 164, y = Max_window_h - 37;		//was -128 and -29

	// attempt to print text nicely.
	grtext_SetFont(BRIEFING_FONT);
	grtext_SetAlpha(192);
	grtext_SetColor(GR_RGB(255, 32, 32));
	grtext_Printf(x, y, "%s v%d.%d", typestr.c_str(), Program_version.major, Program_version.minor);
	grtext_Puts(x, y + 12, engstr.c_str());
	grtext_Puts(x, y + 24, hashbuf);
#ifdef SDL3
	int sdlver = SDL_GetVersion();
	grtext_SetColor(GR_RGB(32, 255, 32));
	grtext_Printf(x, y - 12, "SDL v%d.%d.%d", SDL_VERSIONNUM_MAJOR(sdlver), SDL_VERSIONNUM_MINOR(sdlver), SDL_VERSIONNUM_MICRO(sdlver));
#endif

	grtext_SetFlags(GRTEXTFLAG_SATURATE);

	for (i = 0; i < 1; i++)
	{
		grtext_Printf(x, y, "%s v%d.%d", typestr.c_str(), Program_version.major, Program_version.minor, Program_version.build);
		grtext_Puts(x, y + 12, engstr.c_str());
		grtext_Puts(x, y + 24, hashbuf);
	}

	grtext_Flush();
}


void mmInterface::SetMusicRegion(int region)
{
	if (region == -1) 
	{
		D3MusicStop();
		D3MusicStart("mainmenu.omf");
		SetMusicRegion(MM_MUSIC_REGION);
	}
	else
		D3MusicSetRegion(region, true);
}


// display menu scene
void MenuScene()
{
	extern bool NewUI_wait_dialog;				// is wait dialog up?

	if (MM_object)
	{
		StartFrame();

		if (static_menu_background)
		{
			DrawLargeBitmap(&MM_object->m_art, 0, 0, 1.0f);
		}
		else
		{
#ifdef MOVIE_MENU
			FrameMovie(MM_object->m_movie, -1, -1, true);
#else 
			DrawLargeBitmap(&MM_object->m_art, 0, 0, 1.0f);
#endif
		}

		MM_object->CopyrightText();

		EndFrame();
	}
}


#define N_MENU_SOUND_SLOTS		4

void PlayMenuSound(int sound_index, bool wait_till_done)
{
	int sound_uid;

	Sound_system.BeginSoundFrame(false);
	sound_uid = Sound_system.Play2dSound(sound_index, 1.0f);
	Sound_system.EndSoundFrame();

	if (wait_till_done)
	{
		float timer = timer_GetTime();
		while (Sound_system.IsSoundPlaying(sound_uid) && ((timer + 5.0f) > timer_GetTime()))
		{
			Sound_system.BeginSoundFrame(false);
			Descent->defer();
			Sound_system.EndSoundFrame();
		}
	}
}
