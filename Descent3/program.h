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

#ifndef PROGRAM_H
#define PROGRAM_H

#include "pstypes.h"

#include "buildno.h"

#define D3_MAJORVER					0x1		// DESCENT 3 VERSION NUMBER
#define D3_MINORVER					0x5
#define D3_BUILD					0x0

#define DEVELOPMENT_VERSION			0x1		// without editor: with debug, no beta
#define RELEASE_VERSION				0x2		// final release candidate: no debug, beta, editor

#define BETA_VERSION				0x1000	// beta modifier.
#define DEMO_VERSION				0x2000	// same as release, but its the demo.


typedef struct t_program_version 
{
	int version_type;
	ubyte major, minor, build;
	bool debug: 1;						// are we in debug mode
	bool beta:1;						// are we in beta testing mode
	bool release: 1;					// are we a final release candidate
	bool editor: 1;						// editor code?
	bool windowed: 1;					// runs in a window?
	bool demo:1;						// demo?
} program_version;

//	Program state available to everyone
extern program_version Program_version;

//	Useful macros
#define PROGRAM(_c) Program_version._c

//	functions
void ProgramVersion(int version_type, ubyte major, ubyte minor, ubyte build);

#endif
