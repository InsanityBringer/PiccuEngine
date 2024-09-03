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

#ifndef DDACCESS_H
#define DDACCESS_H

//	allows access to DD system information, such as OS specific info.
#define DD_ACCESS_RING

//	Win32 region of DD_ACCESS
//		Include the standard windows header since API calls are available in DD_ACCESS_RING
//		Define any additional messages too.

#if defined(WIN32)

//	Headers
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

//	Messages
#define WM_SYNCMOUSEACQUIRE		(WM_USER+0)

#endif		// WIN32

#endif
