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

#ifndef _MONO_H
#define _MONO_H
#include "debug.h"
void nw_InitTCPLogging(char *ip,unsigned short port);
void nw_TCPPrintf(int n, char * format, ... );
#if (!defined(RELEASE)) && defined(MONO)
	extern bool Debug_print_block;
	// Prints a formatted string to the debug window
	#define mprintf(args) Debug_ConsolePrintf args
	// Prints a formatted string on window n at row, col.
	#define mprintf_at(args) Debug_ConsolePrintf args
	#define DebugBlockPrint(args) do { if(Debug_print_block)mprintf_at((1,5,51,args)); } while (0)
#else		//ifdef _DEBUG
	#define mprintf(args)	do {} while (0)
	#define mprintf_at(args) do {} while (0)
	#define DebugBlockPrint(args) do {} while (0)
#endif	//ifdef _DEBUG
#endif
