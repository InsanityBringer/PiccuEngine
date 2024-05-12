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

#ifndef MMITEM_H
#define MMITEM_H
#include "newui.h"
#include "psclass.h"
#include "gamefont.h"
#if ( (!defined(OEM)) && (!defined(DEMO)) )
//#define MOVIE_MENU //[ISB]
#endif
#define MMITEM_ALPHA		192						// default alpha
#define MMITEM_SAT			0						// default saturation
#define MMITEM_FONT			MENU_FONT				// default font
#define MMITEM_COLOR		GR_RGB(255,255,255)		// default color of main menu
#define MMITEM_X			(Max_window_w*3/5)		// x position of menu text
#define MMITEM_Y			175						// y position of menu text start
#define N_MMENU_ITEMS		10						// modify this value to set the maximum main menu items avail.
#define MM_STARTMENU_TYPE	1						// start menu group (used in mmInterface::AddITem)
#define MM_ENDMENU_TYPE		2						// end menu group
// main menu sounds
#define N_MM_SOUNDS			2
#define MM_SELECT_SOUND		0
#define MM_FOCUS_SOUND		1
// main menu music
#define MM_MUSIC_REGION			0
#define NEWGAME_MUSIC_REGION	1
#define OPTIONS_MUSIC_REGION	2
#define MULTI_MUSIC_REGION		3

//	mmItem FX list.
enum tmmItemFX
{
	mmItemFXNull,
	mmItemFXNormal,									// displays item with normal alpha, color, saturation.
	mmItemFXFadeInToBright,
	mmItemFXFadeOutToNormal							// fade in and out to and from brighten
};

void MenuScene();						// display menu scene
/////////////////////////////////////////////////////////////////////
//gcc doesn't like the tQueue template in psclass.h
#ifdef __LINUX__
//	tQueue 
//		a circular queue implementation
class tmmItemQueue
{
#define tmmItemSIZE 8
	tmmItemFX m_items[tmmItemSIZE];
	short m_head, m_tail;
public:
	tmmItemQueue() {  m_head = m_tail = 0; };
	~tmmItemQueue() { };
	void send(tmmItemFX& item) {						// sends an item onto the queue
		short temp = m_tail+1;
		if (temp == tmmItemSIZE) 
			temp = 0;
		if (temp != m_head) {
			m_items[m_tail] = item;
			m_tail = temp;
		}
	};
	bool recv(tmmItemFX* item) {						// returns an item from the queue, false if no item.
		if (m_head == m_tail) 
			return false;
		*item = m_items[m_head++];
		if (m_head == tmmItemSIZE) 
			m_head = 0;
		return true;
	};
	void flush() {								// flush queue entries.
		m_head = m_tail = 0;
	};
};
#endif

//////////////////////////////////////////////////////////////////////
class mmInterface;

class mmItem : public UIGadget  
{
	char *m_text;										// text for item
	short m_alpha;										// alpha for text item
	short m_satcount;
	tmmItemFX m_curfx;								// current effect
#ifndef __LINUX__
	tQueue<tmmItemFX, 8>	m_fxqueue;				// special effects queue
#else
	tmmItemQueue m_fxqueue;
#endif
	ddgr_color m_colors[6];							// colors for menu (4 corners and 2 mid points)
	float m_process_speed;							// speed gadget is processed (state changes, etc every x seconds)
	float m_last_frametime;							// last frame's time.
	static mmInterface *m_window;					// attached window
public:
	mmItem();
	virtual ~mmItem();
public:
//	attaches a window to all mmitems.
	static void AttachInterface(mmInterface *wnd);
	void Create(int id, int key, int x, int y, const char *text, int flags, tmmItemFX init_fx=mmItemFXNormal);
	void AddEffect(tmmItemFX fx);
	void ClearEffects();
protected:
	virtual void OnMouseBtnDown(int btn);		// override: behavior when mouse button is pressed.
	virtual void OnMouseBtnUp(int btn);			// override: behavior when mouse button is released.
	virtual void OnFormat();						// override: called when resized or before drawing.
	virtual void OnDraw();							// override: behavior when gadget is being drawn.
	virtual void OnLostFocus();					// override: behavior when gadget loses input focus.
	virtual void OnGainFocus();					// override: behavior when gadget gains input focus.
	virtual void OnDestroy();						// override: behavior when gadget is destroyed.
	virtual void OnUserProcess();					// override: behavior when gadget is processed
	virtual void OnSelect();
};

//	Main Menu Interface Object
struct tCinematic;
class mmInterface: public UIWindow
{
	int m_nmenu_items;								// number of menu items available.
	mmItem m_menuitems[N_MMENU_ITEMS];			// main menu items
#ifdef MOVIE_MENU
	tCinematic *m_movie;								// menu movie handle
#endif
	tLargeBitmap m_art;								// artwork for the bitmap
public:
// type MM_STARTMENU_TYPE = start, type MM_ENDMENU_TYPE = end, (1+2) = start and end, type 0 = normal
	bool AddItem(int id, int key, const char *text, int type =0);
	void Create();
	int DoUI();											//	does user interface.
public:	
	int SoundHandles[N_MM_SOUNDS];
	void SetMusicRegion(int region);
	friend void MenuScene();						// display menu scene
protected:
	virtual void OnDestroy();						// destroys window.
private:
	void CopyrightText();							// displays the copyright text
};
#endif 
