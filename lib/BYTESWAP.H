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

#ifndef _BYTESWAP_H
#define _BYTESWAP_H

#include "pstypes.h"

#define SWAPSHORT(x)	(short)(0xFFFF & ( ((x) << 8) | (((ushort)(x)) >> 8) ))
#define SWAPINT(x)		(int)( ((x) << 24) | (((ulong)(x)) >> 24) | (((x) & 0x0000ff00) << 8) | (((x) & 0x00ff0000) >> 8) )

//Stupid function to trick the compiler into letting me byteswap a float
inline float SWAPFLOAT(float x)
{
	int i = SWAPINT(*((int *) &(x)));
	return *((float *) &(i));
}


//Default is little endian, so change for Macintosh
#if (MACOSX && MACOSXPPC)
   #define OUTRAGE_BIG_ENDIAN
#endif

#if (defined __LINUX__) && (!defined(MACOSX))
    #include <endian.h>

    #if BYTE_ORDER == BIG_ENDIAN
	    #define OUTRAGE_BIG_ENDIAN
    #endif
#endif

#ifndef OUTRAGE_BIG_ENDIAN


	#define INTEL_INT(x)			x
	#define INTEL_SHORT(x)		x
	#define INTEL_FLOAT(x)		x
	#define MOTOROLA_INT(x)		SWAPINT(x)
	#define MOTOROLA_SHORT(x)	SWAPSHORT(x)
	#define MOTOROLA_FLOAT(x)	SWAPFLOAT(x)
#else
	#define INTEL_INT(x)			SWAPINT(x)
	#define INTEL_SHORT(x)		SWAPSHORT(x)
	#define INTEL_FLOAT(x)		SWAPFLOAT(x)
	#define MOTOROLA_INT(x)		x
	#define MOTOROLA_SHORT(x)	x	
	#define MOTOROLA_FLOAT(x)	x
#endif

#endif
