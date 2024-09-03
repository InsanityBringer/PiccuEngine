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

#ifndef __LNXVIDEOMODE_H__
#define __LNXVIDEOMODE_H__

#include "linux/linux_fix.h"
//#include "linux/dyna_xwin.h"
//#include <X11/extensions/xf86vmode.h>
#include <SDL.h>

#define MODE_OK	    0
#define MODE_HSYNC  1		/* hsync out of range */
#define MODE_VSYNC  2		/* vsync out of range */
#define MODE_BAD    255		/* unspecified reason */

class CLnxVideoModes
{
public:
	CLnxVideoModes();
	~CLnxVideoModes();

	bool Init(void);  //Display *dpy,int screen);
	void RestoreVideoMode(void);

	//bool QueryExtension(Display *dpy);
	//	void GetModeLine(Display *dpy,int screen,int *dotclock,XF86VidModeModeLine *modeline);

	bool SwitchResolution(int width,int height);
	void Lock(bool lock);

    SDL_Rect **getModes(void) { return m_ModeInfoList; }
    Uint32 getSDLFlags(void) { return(sdlflags); }

private:
//	bool SwitchToMode(XF86VidModeModeInfo *mode);
	bool LoadLibraries(void);
	
	bool m_Inited;
	bool m_VideoResolutionChanged;
	int m_NumVideoModes;
//	XF86VidModeModeInfo **m_ModeInfoList;
    SDL_Rect **m_ModeInfoList;
    Uint32 sdlflags;
//	Display *m_Display;
//	int m_Screen;
};

extern CLnxVideoModes LinuxVideoMode;

#endif


