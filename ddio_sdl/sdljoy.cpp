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
#include <SDL3/SDL_joystick.h>

#include "sdl/SDLDDIO.h"
#include "joystick.h"

bool joy_Init(bool emulation)
{
}

void joy_Close()
{
}

void joy_GetJoyInfo(tJoystick joy, tJoyInfo* info)
{
	memset(info, 0, sizeof(*info));
	strcpy(info->name, "bogus joystick");
}

void joy_GetPos(tJoystick joy, tJoyPos* pos)
{
	memset(pos, 0, sizeof(*pos));
}

void joy_GetRawPos(tJoystick joy, tJoyPos* pos)
{
	memset(pos, 0, sizeof(*pos));
}

bool joy_IsValid(tJoystick joy)
{
	return false;
}

void ddio_InternalJoyFrame()
{
}

void ddio_SDLJoyEvent(SDL_Event& ev)
{
}
