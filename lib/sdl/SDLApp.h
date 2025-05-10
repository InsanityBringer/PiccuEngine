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

#pragma once

#include <stdint.h>
#include <string>
#include <SDL3/SDL_video.h>

class SDLApplication : public oeApplication
{
protected:
	bool m_AppActive;
	SDL_Window* m_window;
	uint32_t m_flags;
	std::string m_title;
	void (*m_deferhandler)(bool);

	SDL_Rect m_winrect = {};
private:
	//Runs all events in the SDL event queue. 
	void do_events();
	//Applies flag changes to the SDL window
	void change_window();
public:
	//Initializes the SDL state 
	SDLApplication(const char* name, unsigned flags);
	virtual ~SDLApplication();
	//	initializes the object
	void init() override;
	//	Function to retrieve information from object through a platform defined structure.
	void get_info(void* buffer) override;
	//	Function to get the flags
	int flags(void) const override;
	//	defer returns some flags.   essentially this function defers program control to OS.
	unsigned defer() override;
	//	suspends application for a certain amout of time...
	void delay(float secs) override;
	//	set a function to run when deferring to OS.
	void set_defer_handler(void (*func)(bool)) override;
	//  changes the flags and applies changes to the window.
	void set_flags(int newflags) override;
	//  moves the window
	void set_sizepos(int x, int y, int w, int h) override;

	SDL_Window* GetWindow()
	{
		return m_window;
	}
};
