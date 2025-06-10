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

#include <string.h>
#include <algorithm>
#include <SDL3/SDL_joystick.h>

#include "sdl/SDLDDIO.h"
#include "joystick.h"
#include "mono.h"

struct SDLJoyCaps
{
	int numaxes;
	int numhats;
	tJoyInfo d3info;
};

static int numjoysticksSDL;
static SDL_Joystick* joysticksSDL[MAX_JOYSTICKS];
static SDLJoyCaps stickinfoSDL[MAX_JOYSTICKS];

void joy_GetSDLStickCaps(int num, SDL_Joystick* stick, SDLJoyCaps& info)
{
	//Determine axis flags.
	//It is a little disappointing how rigid Descent 3's joystick library is..
	int numaxes = std::min(6, SDL_GetNumJoystickAxes(stick));
	int flag = 1;
	for (int i = 0; i < numaxes; i++)
	{
		info.d3info.axes_mask |= flag;
		flag <<= 1;
	}
	info.numaxes = numaxes;

	//Determine hat flags.
	//Seriously it supports only 6 axes but 4 hats? I want that monster stick
	int numhats = std::min(JOYPOV_NUM, SDL_GetNumJoystickHats(stick));
	flag = JOYFLAG_POVVALID;
	for (int i = 0; i < numhats; i++)
	{
		info.d3info.axes_mask |= flag;
		flag <<= 1;
	}
	info.numhats = numhats;

	//Determine buttons.
	//Buttons cap out at 32 due to the use of a 32-bit integer for passing button flags.
	int numbuttons = std::min(32, SDL_GetNumJoystickButtons(stick));
	info.d3info.num_btns = numbuttons;

	//Determine name.
	//Is the name shown anywhere? Why do I keep on copying utf-8 strings into ascii ones?
	const char* name = SDL_GetJoystickName(stick);
	if (name)
	{
		strncpy(info.d3info.name, name, sizeof(info.d3info.name));
	}
	else
	{
		char buf[128];
		sprintf(buf, "SDL Joystick %d", num);
		strcpy(info.d3info.name, buf);
	}

	//SDL always gives 16 bit signed readings for axes
	info.d3info.minr = info.d3info.minu = info.d3info.minv = info.d3info.minx = info.d3info.miny = info.d3info.minz = -32768;
	info.d3info.maxr = info.d3info.maxu = info.d3info.maxv = info.d3info.maxx = info.d3info.maxy = info.d3info.maxz = 32767;
}

bool joy_Init(bool emulation)
{
	int numsticks;
	SDL_JoystickID* cursticks = SDL_GetJoysticks(&numsticks);
	for (int i = 0; i < numsticks; i++)
	{
		SDL_Joystick* stick = SDL_OpenJoystick(cursticks[i]);
		if (stick)
		{
			joysticksSDL[numjoysticksSDL] = stick;
			if (++numjoysticksSDL == MAX_JOYSTICKS)
				break;

			joy_GetSDLStickCaps(numjoysticksSDL, stick, stickinfoSDL[i]);
		}
		else
		{
			mprintf((1, "joy_Init: SDL_OpenJoystick failed with %s\n", SDL_GetError()));
		}
	}

	SDL_free(cursticks);
	return true;
}

void joy_Close()
{
	for (int i = 0; i < numjoysticksSDL; i++)
	{
		SDL_CloseJoystick(joysticksSDL[i]);
	}

	numjoysticksSDL = 0;
}

void joy_GetJoyInfo(tJoystick joy, tJoyInfo* info)
{
	*info = stickinfoSDL[joy].d3info;
}

int joy_HatBitsToValue(int bits)
{
	//Assumption: only physically possible combinations of hat inputs are possible here.
	//The nature of Descent 3's hat code would preclude any sort of impossibility though, like down+left and up being registered at the same time
	if (bits == SDL_HAT_UP)
		return JOYPOV_UP;

	else if (bits == SDL_HAT_RIGHT)
		return JOYPOV_RIGHT;

	else if (bits == SDL_HAT_DOWN)
		return JOYPOV_DOWN;

	else if (bits == SDL_HAT_LEFT)
		return JOYPOV_LEFT;

	else if (bits == SDL_HAT_RIGHTUP)
		return JOYPOV_UP + 0x20;

	else if (bits == SDL_HAT_RIGHTDOWN)
		return JOYPOV_RIGHT + 0x20;

	else if (bits == SDL_HAT_LEFTDOWN)
		return JOYPOV_DOWN + 0x20;

	else if (bits == SDL_HAT_LEFTUP)
		return JOYPOV_LEFT + 0x20;

	return JOYPOV_CENTER;
}

void joy_GetPos(tJoystick joy, tJoyPos* pos)
{
	SDL_Joystick* stick = joysticksSDL[joy];
	SDLJoyCaps& info = stickinfoSDL[joy];

	//Read axes
	if (info.d3info.axes_mask & JOYFLAG_XVALID)
	{
		Sint16 axis = SDL_GetJoystickAxis(stick, 0);
		pos->x = axis;
	}
	if (info.d3info.axes_mask & JOYFLAG_YVALID)
	{
		Sint16 axis = SDL_GetJoystickAxis(stick, 1);
		pos->y = axis;
	}
	if (info.d3info.axes_mask & JOYFLAG_ZVALID)
	{
		Sint16 axis = SDL_GetJoystickAxis(stick, 2);
		pos->z = axis;
	}
	if (info.d3info.axes_mask & JOYFLAG_RVALID)
	{
		Sint16 axis = SDL_GetJoystickAxis(stick, 3);
		pos->r = axis;
	}
	if (info.d3info.axes_mask & JOYFLAG_UVALID)
	{
		Sint16 axis = SDL_GetJoystickAxis(stick, 4);
		pos->u = axis;
	}
	if (info.d3info.axes_mask & JOYFLAG_VVALID)
	{
		Sint16 axis = SDL_GetJoystickAxis(stick, 5);
		pos->v = axis;
	}

	for (int i = 0; i < info.numhats; i++)
	{
		pos->pov[i] = joy_HatBitsToValue(SDL_GetJoystickHat(stick, i));
	}

	pos->buttons = 0;
	for (int i = 0; i < info.d3info.num_btns; i++)
	{
		if (SDL_GetJoystickButton(stick, i))
		{
			pos->buttons |= (1u << i);
		}
	}
}

void joy_GetRawPos(tJoystick joy, tJoyPos* pos)
{
	//I don't really have a means to return uncalibrated stick readings?
	joy_GetPos(joy, pos);
}

bool joy_IsValid(tJoystick joy)
{
	if (joy < numjoysticksSDL)
		return true;

	return false;
}

void ddio_InternalJoyFrame()
{
}

void ddio_SDLJoyEvent(SDL_Event& ev)
{
	//Hmm, should I read sticks here?
	//Or query the state in joy_GetPos?
	//Do it here if I ever write an event system for D3's input. 
}

//just kinda shoving the force feedback null impl here
#include "forcefeedback.h"
bool ddForce_found;	//a Force Feedback device was found
bool ddForce_enabled;	//Force Feedback is ready and can be used

// -------------------------------------------------------------------
//	ddio_ff_AttachForce
//	Purpose:
//		Attaches variables initialized in the general ddio system to
//	the force feedback system.
// -------------------------------------------------------------------
void ddio_ff_AttachForce(void)
{
}

// -------------------------------------------------------------------
//	ddio_ff_DetachForce
//	Purpose:
//		Detaches variables used by the force-feedback system from the
//	ddio system
// -------------------------------------------------------------------
void ddio_ff_DetachForce(void)
{
}

// -------------------------------------------------------------------
// ddio_ff_Init
// Purpose:
//    Initialize force feedback if available.
// -------------------------------------------------------------------
int ddio_ff_Init(void)
{
	ddForce_found = ddForce_enabled = false;
	return 0;

}

// -------------------------------------------------------------------
// ddio_ffjoy_Init
// Purpose:
//    Creates and acquires all joysticks
//
// Input:
//    None
//
// Return:
//    # of sticks acquired
//
// Description:
//
// -------------------------------------------------------------------
int ddio_ffjoy_Init(void)
{
	ddForce_found = ddForce_enabled = false;
	return 0;
}

// -------------------------------------------------------------------
// ddio_ff_Acquire
// Purpose:
//    Acquires a direct input device for use.
//
// Input:
//    The device to acquire (use kDI_MaxJoy to acquire all available 
//    joysticks).
//
// Return:
//    # of devices acquired.
//
// Description:
//    Call this to gain access to a device after the device has been
//    created & after regaining the focus after losing it.
//
// -------------------------------------------------------------------
int ddio_ff_Acquire(tDevice)
{
	return 0;
}

// -------------------------------------------------------------------
// ddio_ff_Unacquire
// Purpose:
//    Unacquires a direct input device
//
// Input:
//    The device to unacquire (use kDI_MaxJoy to unacquire all available 
//    joysticks).
//
// Return:
//    # of devices unacquired.
//
// Description:
//    Call this to lose access to a device after the device has been
//    aquired
//
// -------------------------------------------------------------------
int ddio_ff_Unacquire(tDevice)
{
	return 0;
}

// -------------------------------------------------------------------
// ddio_ff_SetCoopLevel
// -------------------------------------------------------------------
int ddio_ff_SetCoopLevel(tDevice, int)
{
	return 0;
}

// -------------------------------------------------------------------
// ddio_ffjoy_Query
// Purpose:
//    Besides checking what buttons/axis are available, this function
//    also checks for force feedback support.
// -------------------------------------------------------------------
int ddio_ffjoy_Query(int, int*, int*)
{
	return 0;
}



/*
========================================================================
				Force Feedback Effect Functions
========================================================================


*/
// -------------------------------------------------------------------
//	ddio_ff_GetInfo
//	Purpose:
//		Returns information about the current state of the low-level
//	Force Feedback system.
// -------------------------------------------------------------------
void ddio_ff_GetInfo(bool* ff_found, bool* ff_enabled)
{
	if (ff_found)
		*ff_found = false;
	if (ff_enabled)
		*ff_enabled = false;
}

// -------------------------------------------------------------------
// ddio_ffb_Pause
// Purpose:
//    Pause the FFB output on the given device.  Use ddio_ffb_Continue to
//    continue where you left off.
// -------------------------------------------------------------------
void ddio_ffb_Pause(tDevice)
{
}

// -------------------------------------------------------------------
// ddio_ffb_Continue
// Purpose:
//    Unpause the FFB output on the given device.  Complimentary to
//    ddio_ffb_Pause.
// -------------------------------------------------------------------
void ddio_ffb_Continue(tDevice)
{
}

// -------------------------------------------------------------------
// ddio_ffb_Enable
// Purpose:
//    Must be called after initialization in order to activate the 
//    device.
//    Use ddio_ffb_Pause & ddio_ffb_Continue if you want disable forces
//    temporarily and resume later.
// -------------------------------------------------------------------
void ddio_ffb_Enable(tDevice)
{
}

// -------------------------------------------------------------------
// ddio_ffb_Disable
// Purpose:
//    Turns off FFB, but effects still play on processor.
// -------------------------------------------------------------------
void ddio_ffb_Disable(tDevice)
{
}

// -------------------------------------------------------------------
// ddio_ffb_effectCreate
// Purpose:
//    Create a single effect for future playback.
//    Effect is given a logical ID
// -------------------------------------------------------------------
int ddio_ffb_effectCreate(tDevice, tFFB_Effect*)
{
	return 0;
}

// -------------------------------------------------------------------
// ddio_ffb_DestroyAll
//	Purpose:
//		Destroys all created effects
// -------------------------------------------------------------------
void ddio_ffb_DestroyAll(void)
{
}

// -------------------------------------------------------------------
// ddio_ffb_effectPlay
// Purpose:
//    Play an effect that was previously created.
// -------------------------------------------------------------------
void ddio_ffb_effectPlay(short)
{

}

// -------------------------------------------------------------------
// ddio_ffb_effectStop
// Purpose:
//    Stop a single effect.
// -------------------------------------------------------------------
void ddio_ffb_effectStop(short)
{
}

// -------------------------------------------------------------------
// ddio_ffb_effectStopAll
// Purpose:
//    Stops all forces on the given device.
// -------------------------------------------------------------------
void ddio_ffb_effectStopAll(tDevice)
{
}

// -------------------------------------------------------------------
// ddio_ffb_effectUnload
// Purpose:
//    Unload a single effect...  Necessary to make room for other
//    effects.
// -------------------------------------------------------------------
void ddio_ffb_effectUnload(short)
{
}

// -------------------------------------------------------------------
// ddio_ffb_effectModify
// Purpose:
//    Modifies a single effect, only if the given parameters are
//    different from what's currently loaded.
// -------------------------------------------------------------------
void ddio_ffb_effectModify(short, int*, unsigned int*, unsigned int*, unsigned int*, tEffInfo*, tEffEnvelope*)
{
}

// -------------------------------------------------------------------
// ddio_ffb_GetEffectData
// Purpose:
//    Retrieves affect data for the given parameters, pass NULL for those you don't want
// -------------------------------------------------------------------
void ddio_ffb_GetEffectData(short, int*, unsigned int*, unsigned int*, unsigned int*, tEffInfo*, tEffEnvelope*)
{
}

// -------------------------------------------------------------------
// ddio_ffjoy_EnableAutoCenter
// Purpose:
//	Disables/Enables the autocentering of the joystick
// -------------------------------------------------------------------
void ddio_ffjoy_EnableAutoCenter(tDevice, bool)
{
}

// -------------------------------------------------------------------
// ddio_ffjoy_SetGain
// Purpose:
//	Sets the gain for joystick, pass a value of 0-1
// -------------------------------------------------------------------
void ddio_ffjoy_SetGain(tDevice, float)
{
}

// -------------------------------------------------------------------
// ddio_ffjoy_IsAutoCentered
// Purpose:
//	Returns true if the joystick is set for autocentering
// -------------------------------------------------------------------
bool ddio_ffjoy_IsAutoCentered(tDevice)
{
	return true;
}

// -------------------------------------------------------------------
// ddio_ffjoy_SupportAutoCenter
// Purpose:
//	Returns true if the FF joystick supports auto centering
// -------------------------------------------------------------------
bool ddio_ffjoy_SupportAutoCenter(tDevice)
{
	return false;
}

//	Given a filename resource, this loads the file and creates a resource
//	for it.  It returns a handle to that resource.
//	If it returns NULL, then it couldn't load the project.
//	Make sure device is aquired before calling.
FORCEPROJECT ddio_ForceLoadProject(char* filename, tDevice dev)
{
	return NULL;
}

//	Unloads a FORCEPROJECT file
void ddio_ForceUnloadProject(FORCEPROJECT prj)
{
}

//	Given a handle to a resource, and the name of the effect to load
//	it will load that effect.  Returns the effect ID, or -1 if it couldn't
//	be created
int ddio_CreateForceFromProject(FORCEPROJECT project, char* forcename)
{
	return -1;
}
