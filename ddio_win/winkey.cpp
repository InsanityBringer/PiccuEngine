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

// ----------------------------------------------------------------------------
//	Keyboard Interface
// ----------------------------------------------------------------------------
#include "DDAccess.h"

#include "pserror.h"
#include "mono.h"
#include "ddio.h"
#include "ddio_win.h"
#include "Application.h"

#include <stdlib.h>
#include <process.h>


// ----------------------------------------------------------------------------
//	Local global data
// ----------------------------------------------------------------------------
#define	DIKEY_BUFFER_SIZE			32

static struct tWinKeyData
{
	LPDIRECTINPUTDEVICE lpdikey;					// key device
	HANDLE evtnotify;									// notify event
	HHOOK winhook;										// windows hook
	unsigned long thread;							// thread id
//	osMutex keyframe_mutex;							// mutex between internal key frame and key thread.
	bool nextframe;									// used for mutexing between keyframe and thread.
	bool acquired;										// device acquired?
	bool thread_active;								// used in thread.
	bool suspended;
}
WKD = { 
	NULL,
	NULL,
	NULL,
	0xffffffff,
	false,
	false,
	false,
	true
};

volatile struct tWinKeys
{
	union {
		DWORD up_ticks;								// windows ticks when key went up last
		float up_time;
	};
	union {
		DWORD down_ticks;								// windows ticks when key went down last
		float down_time;
	};
	bool status;										// is it down?
	bool mutex_flag;									// done for mutexing between ddio_Internal and KeyThread
	ushort mutex_data;
}
WKeys[DDIO_MAX_KEYS];


static		int				DDIO_key_language = KBLANG_AMERICAN;


///////////////////////////////////////////////////////////////////////////////
// emulated keyboard functionality
bool ddio_Win_KeyInit();
void ddio_Win_KeyClose();

int ddio_KeyHandler(HWnd wnd, unsigned msg, unsigned wParam, long lParam);

LRESULT CALLBACK KeyboardProc( int code, WPARAM wParam, LPARAM lParam);

// translates scan code to foreign equivs.
ubyte xlate_scancode(ubyte scan_code);


///////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------------
//	Initialization of keyboard device.
// ----------------------------------------------------------------------------

// this version will try to initialize a direct input keyboard device.  if it fails
// it falls back to the old keyboard hook (less reliable but works.)
bool ddio_InternalKeyInit(ddio_init_info *init_info)
{
	return ddio_Win_KeyInit();
}


//	this will shutdown direct input or the windows hook, whichever was chosen.
void ddio_InternalKeyClose()
{
	if (WKD.winhook) 
	{
		ddio_Win_KeyClose();
	}
}


// handled internally if keyboard system needs additional processing per frame
void ddio_InternalKeyFrame()
{
}


//////////////////////////////////////////////////////////////////////////////
// Miscellaneous API
//////////////////////////////////////////////////////////////////////////////

                                                                              
// returns if key is up or down
bool ddio_InternalKeyState(ubyte key)
{
	return WKeys[key].status;
}


float ddio_InternalKeyDownTime(ubyte key)
{
	float down_time = 0.0f;
	if (WKeys[key].status) {
		if (WKD.winhook) {
			float timer = timer_GetTime();
			down_time = timer - WKeys[key].down_time;
			WKeys[key].down_time = timer;
		}
		else {
			DWORD curtickcount = GetTickCount();
			DWORD ticks =  curtickcount - WKeys[key].down_ticks;
			if (ticks == 0) {
				//mprintf((0, "ticks=%d\n", ticks));
			}
			WKeys[key].down_ticks = curtickcount;
			down_time = (ticks/1000.0f);
		}
	}
	else {
		if (WKD.winhook) {
			down_time = WKeys[key].up_time - WKeys[key].down_time;
			WKeys[key].down_time = WKeys[key].up_time = 0.0f;
		}
		else {
			DWORD ticks = WKeys[key].up_ticks - WKeys[key].down_ticks;
			WKeys[key].down_ticks = 0;
			WKeys[key].up_ticks = 0;
			down_time = (ticks/1000.0f);
		}
	}

	return down_time;
}


// flush a key internally
void ddio_InternalResetKey(ubyte key)
{
	WKeys[key].down_ticks = 0;
	WKeys[key].up_ticks = 0;
	WKeys[key].status = false;
	WKeys[key].mutex_flag = false;
	WKeys[key].mutex_data = 0;
}


// sets type of keyboard to emulate
// #define KBLANG_AMERICAN		0
// #define KBLANG_BRITISH		1
// #define KBLANG_FRENCH		2
// #define KBLANG_GERMAN		3

void ddio_SetKeyboardLanguage(int language)
{
	DDIO_key_language = language;
}


// translates scan code to foreign equivs.
ubyte xlate_scancode(ubyte scan_code)
{
	ubyte code = scan_code;

	if (DDIO_key_language == KBLANG_FRENCH) {
		switch (scan_code) {
			case KEY_A:	code = KEY_Q; break;
			case KEY_M:	code = KEY_COMMA; break;
			case KEY_Q:	code = KEY_A; break;
			case KEY_W:	code = KEY_Z; break;
			case KEY_Z:	code = KEY_W; break;
			case KEY_SEMICOL:	code = KEY_M; break;
			case KEY_COMMA:	code = KEY_SEMICOL; break;
		}
	}
	else if (DDIO_key_language == KBLANG_GERMAN) {
		switch (scan_code) {
			case KEY_Y:	code = KEY_Z; break;
			case KEY_Z:	code = KEY_Y; break;
		}
	}
	else if (DDIO_key_language == KBLANG_BRITISH) {
		if ( scan_code == KEY_BSLASH_UK ) {	// KEY_SLASH_UK == 0x56
			code = KEY_SLASH;				// KEY_SLASH is really the backslash, 0x2B
		}
	}

	return code;
}

void ddio_InternalKeySuspend()
{
	WKD.suspended = true;
}


void ddio_InternalKeyResume()
{
	WKD.suspended = false;
}

//	Win32 non-threaded version

bool ddio_Win_KeyInit()
{
/*	Initialize hook handlers */
	WKD.winhook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)KeyboardProc, (HINSTANCE)DInputData.app->m_hInstance, GetCurrentThreadId());
	if (!WKD.winhook) {
		return false;
	}

	mprintf((0, "Keyboard initialized.\n"));

	return true;
}


void ddio_Win_KeyClose()
{
/*	Free up message handlers */
	if (WKD.winhook) {
		UnhookWindowsHookEx(WKD.winhook);
		WKD.winhook = NULL;
	}

	mprintf((0, "Keyboard released.\n"));
}


// ----------------------------------------------------------------------------
//	non DirectInput keyboard handler
// ----------------------------------------------------------------------------
LRESULT CALLBACK KeyboardProc( int code, WPARAM wParam, LPARAM lParam)
{
	int res;

	if (code < 0) {
		return CallNextHookEx(WKD.winhook, code, wParam, lParam);
	}

	if (lParam&0x80000000) 
		res = ddio_KeyHandler(	0, WM_KEYUP, wParam, lParam);
	else
		res = ddio_KeyHandler(	0, WM_KEYDOWN, wParam, lParam);

	return (!res);
}


int ddio_KeyHandler(HWnd wnd, unsigned msg, unsigned wParam, long lParam)
{
	ubyte scan_code;
	float timer = timer_GetTime();
	
	if (!WKD.winhook) 
		return 1;

	switch ((UINT)msg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		scan_code = (unsigned char )((lParam>>16) & 0xff);
		if (lParam & 0x01000000) 
			scan_code |= 0x80;

		scan_code = xlate_scancode(scan_code);

	//	print screen is a weird case.   only accept key ups.
		if (wParam != VK_SNAPSHOT) {
			if (!WKeys[scan_code].status) {
				WKeys[scan_code].up_time = 0;
				WKeys[scan_code].down_time = timer;
			}
			else {
				WKeys[scan_code].up_time = 0;
			}
			WKeys[scan_code].status = true;
			ddio_UpdateKeyState(scan_code, true);
		}
		break;

	case WM_KEYUP:
	case WM_SYSKEYUP:
		scan_code = (unsigned char )((lParam>>16) & 0xff);
		if (lParam & 0x01000000) 
			scan_code |= 0x80;

		scan_code = xlate_scancode(scan_code);

	//	handle special keys.  print screen, we will simulate the keypress.
		if (wParam == VK_SNAPSHOT) {
			scan_code = KEY_PRINT_SCREEN;
			WKeys[scan_code].down_time = timer;
			WKeys[scan_code].status = true;
			ddio_UpdateKeyState(scan_code, true);
		}
		if (WKeys[scan_code].status) {
			WKeys[scan_code].status = false;
			WKeys[scan_code].up_time = timer;
			ddio_UpdateKeyState(scan_code, false);
		}
	
		break;
	}

	return 1;
}
