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

#ifndef PSERROR_H
#define PSERROR_H
#include <assert.h>
#include "debug.h"
#include "mono.h"

//	initializes error handler.
bool error_Init(bool debugger, bool mono_debug, const char *app_title);

//	exits the application and prints out a standard error message
[[noreturn]] void Error(char *fmt,...);

//	prints out an assertion error
void AssertionFailed(char *expstr, char *file, int line);

//Brings up an error message for an int3
void Int3MessageBox(char *file,int line);

//	Message box functions 
#define MBOX_OK					1
#define MBOX_YESNO				2
#define MBOX_YESNOCANCEL		3
#define MBOX_ABORTRETRYIGNORE	4
//	prints out a standard OS messagebox
void OutrageMessageBox(char *str, ...);
int OutrageMessageBox (int type, char *str, ...);

// Sets the title for future OutrageMessageBox() dialogs
void SetMessageBoxTitle(char *title);

//Write a block of text to the system clipboard
void DumpTextToClipboard(char *text);

//////////////////////////////////////////////////////////////////////////////
//	development debugging functions
//	adds a function to be called when a debug break occurs.
//	undefine any ASSERT macro previously defined.
#ifdef ASSERT
#undef ASSERT
#endif
//	this callback is invoked when a DEBUG_BREAK macro is used.
//		arguments 
//			style = 1 if ASSERT
//					= 0 if Int3 debugger break.
extern void (*DebugBreak_callback_stop)();
extern void (*DebugBreak_callback_resume)();
//	set DEBUG_BREAK callback
inline void SetDebugBreakHandlers(void (*stop)(), void (*resume)()) { 
	DebugBreak_callback_stop = stop; 
	DebugBreak_callback_resume = resume;
}

// DEBUG_BREAK()
// Calls the debug_break() macro surrounded by calls to the debug callbacks (to turn off & on graphics)
// ASSERT()
// Like the standard C assert(), but if the condition failed and debugging on,
// does a DEBUG_BREAK().  If debugging on, brings up a dialog.
// Int3()
// Does a DEBUG_BREAK() if debugging is turned on.  Also does an mprintf().
//	Define the macros
#ifndef RELEASE
#if defined(MACOSX)
	#include <sys/malloc.h>
#else
	#include <malloc.h>
#endif
#if defined (WIN32)
	#define DEBUG_BREAK() \
		do { \
			if (DebugBreak_callback_stop)  \
				(*DebugBreak_callback_stop)(); \
			debug_break(); \
			if (DebugBreak_callback_resume) \
				(*DebugBreak_callback_resume)(); \
		} while(0)
			
	#define ASSERT(x) \
		do { \
			if (!(unsigned)(x)) { \
				mprintf((0, "Assertion failed (%s) in %s line %d.\n", #x, __FILE__, __LINE__)); \
				if (Debug_break)	\
					DEBUG_BREAK(); \
				else	\
					AssertionFailed(#x, __FILE__, __LINE__); \
			} \
		} while(0)
			
	#define Int3() do { \
		mprintf((0, "Int3 at %s line %d.\n", __FILE__, __LINE__));	\
		if (Debug_break)	\
			DEBUG_BREAK(); \
		else	\
			Int3MessageBox(__FILE__,__LINE__); \
	} while (0)
		
	#define HEAPCHECK() do { if (_heapchk() != _HEAPOK) Int3(); } while(0) 
#elif defined (LINUX)
	//For some reason Linux doesn't like the \ continuation character, so I have to uglify this
	#define DEBUG_BREAK() do{ if(DebugBreak_callback_stop) (*DebugBreak_callback_stop)(); debug_break(); if(DebugBreak_callback_resume) (*DebugBreak_callback_resume)(); }while(0)
	#define ASSERT(x) do{ if(!(unsigned long long)(x)){mprintf((0,"Assertion failed (%s) in %s line %d.\n", #x, __FILE__, __LINE__)); if(Debug_break) DEBUG_BREAK(); else AssertionFailed(#x,__FILE__,__LINE__);}}while(0)
	#define Int3() do{ mprintf((0, "Int3 at %s line %d.\n",__FILE__,__LINE__)); if(Debug_break) DEBUG_BREAK(); else Int3MessageBox(__FILE__,__LINE__);}while(0)
	#define HEAPCHECK()
#endif
#else
	#define DEBUG_BREAK() do {} while (0)
	#define ASSERT(x) do {} while (0)
	#define Int3() do {} while (0)
	#define HEAPCHECK() do {} while (0)
#endif
#endif
