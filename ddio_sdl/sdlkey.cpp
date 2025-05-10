/*
* Descent 3: Piccu Engine
* Copyright (C) 2024 SaladBadger
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
#include <stdlib.h>
#include <stdint.h>
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_events.h>
#include <unordered_map>
#include <stdexcept>

#include "DDAccess.h"

#include "pserror.h"
#include "mono.h"
#include "ddio.h"


volatile struct tWinKeys
{
	float down_time;
	float up_time;
	bool status;										// is it down?
	bool mutex_flag;									// done for mutexing between ddio_Internal and KeyThread
	ushort mutex_data;
}
WKeys[DDIO_MAX_KEYS];

//This is not a particularly good way to write a keyboard handler, but it has the least friction with the Descent 3 code as-is.
//Uses SDL 3's keycodes vs scancodes to ensure some compatibility with non-qwerty keyboards, but it won't handle a whole lot beyond latin languages.
//TODO: keys like ! and " that aren't on US keyboards should probably still emit a usable character.
static std::unordered_map<SDL_Keycode, int> DDIO_SDLKeycodeToDDIOKeycode;

bool ddio_InternalKeyInit(ddio_init_info* init_info)
{
	if (DDIO_SDLKeycodeToDDIOKeycode.empty())
	{
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_RETURN, KEY_ENTER });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_ESCAPE, KEY_ESC });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_BACKSPACE, KEY_BACKSP });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_TAB, KEY_TAB });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_APOSTROPHE, KEY_RAPOSTRO });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_COMMA, KEY_COMMA });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_MINUS, KEY_MINUS });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_PERIOD, KEY_PERIOD });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_SLASH, KEY_SLASH });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_0, KEY_0 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_1, KEY_1 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_2, KEY_2 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_3, KEY_3 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_4, KEY_4 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_5, KEY_5 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_6, KEY_6 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_7, KEY_7 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_8, KEY_8 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_9, KEY_9 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_SEMICOLON, KEY_SEMICOL });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_EQUALS, KEY_EQUAL });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_LEFTBRACKET, KEY_LBRACKET });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_RIGHTBRACKET, KEY_RBRACKET });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_BACKSLASH, KEY_BACKSLASH });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_GRAVE, KEY_RAPOSTRO });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_A, KEY_A });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_B, KEY_B });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_C, KEY_C });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_D, KEY_D });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_E, KEY_E });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F, KEY_F });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_G, KEY_G });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_H, KEY_H });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_I, KEY_I });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_J, KEY_J });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_K, KEY_K });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_L, KEY_L });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_M, KEY_M });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_N, KEY_N });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_O, KEY_O });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_P, KEY_P });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_Q, KEY_Q });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_R, KEY_R });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_S, KEY_S });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_T, KEY_T });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_U, KEY_U });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_V, KEY_V });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_W, KEY_W });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_X, KEY_X });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_Y, KEY_Y });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_Z, KEY_Z });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_DELETE, KEY_DELETE });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_CAPSLOCK, KEY_CAPSLOCK });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F1, KEY_F1 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F2, KEY_F2 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F3, KEY_F3 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F4, KEY_F4 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F5, KEY_F5 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F6, KEY_F6 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F7, KEY_F7 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F8, KEY_F8 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F9, KEY_F9 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F10, KEY_F10 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F11, KEY_F11 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_F12, KEY_F12 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_PRINTSCREEN, KEY_PRINT_SCREEN });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_SCROLLLOCK, KEY_SCROLLOCK });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_PAUSE, KEY_PAUSE });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_INSERT, KEY_INSERT });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_HOME, KEY_HOME });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_PAGEUP, KEY_PAGEUP });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_END, KEY_END });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_PAGEDOWN, KEY_PAGEDOWN });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_RIGHT, KEY_RIGHT });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_LEFT, KEY_LEFT });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_DOWN, KEY_DOWN });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_UP, KEY_UP });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_NUMLOCKCLEAR, KEY_NUMLOCK });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_DIVIDE, KEY_PADDIVIDE });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_MULTIPLY, KEY_PADMULTIPLY });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_MINUS, KEY_PADMINUS });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_PLUS, KEY_PADPLUS });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_ENTER, KEY_PADENTER });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_1, KEY_PAD1 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_2, KEY_PAD2 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_3, KEY_PAD3 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_4, KEY_PAD4 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_5, KEY_PAD5 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_6, KEY_PAD6 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_7, KEY_PAD7 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_8, KEY_PAD8 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_9, KEY_PAD9 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_0, KEY_PAD0 });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_KP_PERIOD, KEY_PADPERIOD });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_LCTRL, KEY_LCTRL });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_LSHIFT, KEY_LSHIFT });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_LALT, KEY_LALT });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_RCTRL, KEY_RCTRL });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_RSHIFT, KEY_RSHIFT });
		DDIO_SDLKeycodeToDDIOKeycode.insert({ SDLK_RALT, KEY_RALT });

	}
	return true;
}

void ddio_InternalKeyClose()
{
}

// handled internally if keyboard system needs additional processing per frame
void ddio_InternalKeyFrame()
{
}

void ddio_InternalKeySuspend()
{
}

void ddio_InternalKeyResume()
{
}

// returns if key is up or down
bool ddio_InternalKeyState(ubyte key)
{
	return WKeys[key].status;
}

float ddio_InternalKeyDownTime(ubyte key)
{
	float down_time = 0.0f;
	if (WKeys[key].status) 
	{
		float timer = timer_GetTime();
		down_time = timer - WKeys[key].down_time;
		WKeys[key].down_time = timer;
	}
	else 
	{
		down_time = WKeys[key].up_time - WKeys[key].down_time;
		WKeys[key].down_time = WKeys[key].up_time = 0.0f;
	}

	return down_time;
}

// flush a key internally
void ddio_InternalResetKey(ubyte key)
{
	WKeys[key].down_time = 0;
	WKeys[key].up_time = 0;
	WKeys[key].status = false;
	WKeys[key].mutex_flag = false;
	WKeys[key].mutex_data = 0;
}

void ddio_SDLKeyEvent(SDL_Event& ev)
{
	auto it = DDIO_SDLKeycodeToDDIOKeycode.find(ev.key.key);
	if (it != DDIO_SDLKeycodeToDDIOKeycode.end())
	{
		float timer = timer_GetTime();
		//Lookup the keycode
		int DDIOKeyCode = it->second;

		//Determine the type of event
		if (ev.type == SDL_EVENT_KEY_DOWN)
		{
			if (!WKeys[DDIOKeyCode].status) 
			{
				WKeys[DDIOKeyCode].up_time = 0;
				WKeys[DDIOKeyCode].down_time = timer;
			}
			else 
			{
				WKeys[DDIOKeyCode].up_time = 0;
			}
			WKeys[DDIOKeyCode].status = true;
		}
		else if (ev.type == SDL_EVENT_KEY_UP)
		{
			WKeys[DDIOKeyCode].status = false;
			WKeys[DDIOKeyCode].up_time = timer;
		}
		ddio_UpdateKeyState(DDIOKeyCode, WKeys[DDIOKeyCode].status);
	}
}

void ddio_SetKeyboardLanguage(int language)
{
	//SDL should handle this
}
