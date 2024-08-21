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

#ifndef DEBUG_H
#define DEBUG_H

#include "pstypes.h"

//	---------------------------------------------------------------------------
//	Debug system is a member of the 'platform' library.
//	---------------------------------------------------------------------------
//	---------------------------------------------------------------------------
//	Constants
//	---------------------------------------------------------------------------
#ifndef IDOK
#define IDOK					 1
#endif
#ifndef IDCANCEL
#define IDCANCEL            2
#endif
#ifndef IDABORT
#define IDABORT             3
#endif
#ifndef IDRETRY
#define IDRETRY             4
#endif
#ifndef IDIGNORE
#define IDIGNORE            5
#endif
#ifndef IDYES
#define IDYES               6
#endif
#ifndef IDNO
#define IDNO                7
#endif
//#define DEBUG_LEVEL 0		//DAJ

#if defined(WIN32) || defined(__LINUX__)
static const int OSMBOX_OK = 1;
static const int OSMBOX_YESNO = 2;
static const int OSMBOX_YESNOCANCEL = 3;
static const int OSMBOX_ABORTRETRYIGNORE = 4;
static const int OSMBOX_OKCANCEL = 5;
#else
#define OSMBOX_OK				1
#define OSMBOX_YESNO			2
#define OSMBOX_YESNOCANCEL		3
#define OSMBOX_ABORTRETRYIGNORE	4
#define OSMBOX_OKCANCEL			5
#endif

//	---------------------------------------------------------------------------
//	Functions
//	---------------------------------------------------------------------------

extern bool Debug_break;
//	if we are running under a debugger, then pass true

bool Debug_Init(bool debugger, bool mono_debug);

//Does a messagebox with a stack dump
//Messagebox shows topstring, then stack dump, then bottomstring
//Return types are the same as the Windows return values
int Debug_ErrorBox(int type,const char *topstring, const char *title,const char *bottomstring);

// displays an message box 
// Returns the same values as the Win32 MessageBox() function
int Debug_MessageBox(int type, const char *title, const char *str);	

//	these functions deal with debug spew support
bool Debug_Logfile(const char *filename);
void Debug_LogWrite(const char *str);
bool Debug_ConsoleInit();
void Debug_ConsoleOpen(int n, int row, int col, int width, int height, char * title );
void Debug_ConsoleClose(int n);
void Debug_ConsolePrintf( int n, char * format, ... );
void Debug_ConsolePrintf( int n, int row, int col, char * format, ... );
void Debug_ConsoleRedirectMessages(int virtual_window, int physical_window);

//	DEBUGGING MACROS
//Break into the debugger, if this feature was enabled in Debug_init()
#if !defined(RELEASE)
#if defined(WIN32)
void debug_break();
#elif defined(__LINUX__)
	void ddio_InternalKeyClose();
	//#define debug_break() do{__asm__ __volatile__ ( "int $3" );}while(0)
#ifndef MACOSXPPC
	#define debug_break() do{ ddio_InternalKeyClose(); __asm__ __volatile__ ( "int $3" ); }while(0)
#else
        #define debug_break() do{ ddio_InternalKeyClose(); /*nop*/ }while(0)
#endif
#elif defined(MACINTOSH)
	extern void SuspendControls();
	extern void ResumeControls();
	#define debug_break()  \
		do {  \
			if (Debug_break) \
				SuspendControls(); \
				Debugger();  \
				ResumeControls(); \
		} while (0)
#else
	#define debug_break() 
#endif
#else
	#define debug_break()
#endif
#if defined (WIN32)
// We forward declare PEXCEPTION_POINTERS so that the function
// prototype doesn't needlessly require windows.h.
typedef struct _EXCEPTION_POINTERS EXCEPTION_POINTERS, *PEXCEPTION_POINTERS;
int __cdecl RecordExceptionInfo(PEXCEPTION_POINTERS data, const char *Message);
#endif
#endif
