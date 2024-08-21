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

#ifndef APP_H
#define APP_H
#include "pstypes.h"
/*	Basic Application data types */
/*	Application Object
		This object entails initialization and cleanup of all operating system 
		elements, as well as data that libraries may need to initialize their 
		systems.  Look under the specific header file for a platform for information
		about what's needed for the target machine.
*/
/*	Application flags */
const int	OEAPP_WINDOWED = 1,			// App will run in a window. May not be supported.
			OEAPP_FULLSCREEN = 2,		// App will run in fullscreen.  May not be supported.
			OEAPP_TOPMOST = 4,			// App will be on the topmost display.  May not be supported.
			OEAPP_CONSOLE = 8;			// App will run in a console style window.  
class oeApplication
{
protected:
	bool m_AppActive;
public:
	oeApplication() { m_AppActive = true; };
	virtual ~oeApplication() {};
//	initializes the object
	virtual void init() = 0;
//	Function to retrieve information from object through a platform defined structure.
	virtual void get_info(void *buffer) = 0;
//	Function to get the flags
	virtual int flags(void) const = 0;
//	defer returns some flags.   essentially this function defers program control to OS.
	virtual unsigned defer() = 0;
//	suspends application for a certain amout of time...
	virtual void delay(float secs) = 0 ;
//	set a function to run when deferring to OS.
	virtual void set_defer_handler(void (*func)(bool)) = 0;
//  changes the flags and applies changes to the window.
	virtual void set_flags(int newflags) = 0;
//  moves the window
	virtual void set_sizepos(int x, int y, int w, int h) = 0;
public:
//	checks if the application is active
	bool active() const { return m_AppActive;	};
	void activate() { m_AppActive = true; };
	void deactivate() { m_AppActive = false; };
};

#if defined(WIN32)
#include "win\Win32App.h"
#elif defined(__LINUX__)
#include "linux/lnxapp.h"
#endif
#endif
