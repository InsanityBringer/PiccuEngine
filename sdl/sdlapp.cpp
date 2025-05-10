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

#include <SDL3/SDL_timer.h>

#include "application.h"
#include "mono.h"
#include "networking.h"
#include "pserror.h"
#include "sdl/SDLDDIO.h"

void SDLApplication::do_events()
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_EVENT_KEY_DOWN:
		case SDL_EVENT_KEY_UP:
			ddio_SDLKeyEvent(ev);
			break;
		case SDL_EVENT_MOUSE_MOTION:
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
		case SDL_EVENT_MOUSE_BUTTON_UP:
		case SDL_EVENT_MOUSE_WHEEL:
			ddio_SDLMouseEvent(ev);
			break;
		}
	}

	if (m_deferhandler)
		m_deferhandler(active());
}

void SDLApplication::change_window()
{
	if (m_flags & OEAPP_FULLSCREEN)
	{
		SDL_SetWindowFullscreen(m_window, true);
	}
	else
	{
		SDL_SetWindowFullscreen(m_window, false);
		//TODO: Should allow selecting a display number
		SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		SDL_SetWindowSize(m_window, m_winrect.w, m_winrect.h);
	}
	SDL_SyncWindow(m_window);
}

SDLApplication::SDLApplication(const char* name, unsigned flags) : oeApplication(), m_title(name)
{
	m_flags = flags;
	m_window = nullptr;
	m_deferhandler = nullptr;
	m_AppActive = false;
}

SDLApplication::~SDLApplication()
{
	if (m_window)
		SDL_DestroyWindow(m_window);

	m_deferhandler = nullptr;
	do_events(); //Pump events to ensure the window is closed
}

void SDLApplication::init()
{
	SDL_WindowFlags winflags = SDL_WINDOW_OPENGL;
	if (m_flags & OEAPP_FULLSCREEN)
	{
		winflags |= SDL_WINDOW_FULLSCREEN;
	}

	m_window = SDL_CreateWindow(m_title.c_str(), 640, 480, winflags);
	if (!m_window)
	{
		Error("Failed to create SDL Window! Error: %s", SDL_GetError());
	}

	//The mouse system needs the application object to work
	ddio_SDLMouseLinkApp(this);
}

void SDLApplication::get_info(void* buffer)
{
}

int SDLApplication::flags(void) const
{
	return m_flags;
}

unsigned SDLApplication::defer()
{
	//I don't know if I need to do the whole "run the defer handler 6 times if the window is active" thing let's find out the hard way. 
	do_events();
	return 0;
}

void SDLApplication::delay(float secs)
{
	//this seems like it's playing with fire for large secs values
	uint64_t ns = ((double)secs * 1000000000);

	uint64_t goal = SDL_GetTicksNS() + ns;

	while (SDL_GetTicksNS() < goal)
	{
		do_events();
	}
}

void SDLApplication::set_defer_handler(void(*func)(bool))
{
	m_deferhandler = func;
}

void SDLApplication::set_flags(int newflags)
{
	uint32_t oldflags = m_flags;
	m_flags = newflags;
	if (oldflags != newflags)
	{
		change_window();
	}
}

void SDLApplication::set_sizepos(int x, int y, int w, int h)
{
	m_winrect.x = x;
	m_winrect.y = y;
	m_winrect.w = w;
	m_winrect.h = h;

	if ((m_flags & OEAPP_FULLSCREEN) != 0)
	{
		//TODO: Should allow selecting a display number
		SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
		SDL_SetWindowSize(m_window, m_winrect.w, m_winrect.h);
		SDL_SyncWindow(m_window);
	}
}
