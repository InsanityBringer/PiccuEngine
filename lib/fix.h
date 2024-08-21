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

#ifndef _FIX_H
#define _FIX_H

#include "math.h"

// Disable the "possible loss of data" warning
#pragma warning (disable:4244)

//Angles are unsigned shorts
typedef unsigned short angle;

//The basic fixed-point type
typedef long fix;

#define PI 3.141592654
#define PIOVER2 1.570796327 //DAJ

//Constants for converted between fix and float
#define FLOAT_SCALER		65536.0
#define FIX_SHIFT			16

//1.0 in fixed-point
#define F1_0				(1 << FIX_SHIFT)

//Generate the data for the trig tables.  Must be called before trig functions
void InitMathTables ();

//Returns the sine of the given angle.  Linearly interpolates between two entries in a 256-entry table
float FixSin(angle a);

//Returns the cosine of the given angle.  Linearly interpolates between two entries in a 256-entry table
float FixCos(angle a);

//Returns the sine of the given angle, but does no interpolation
float FixSinFast(angle a);

//Returns the cosine of the given angle, but does no interpolation
float FixCosFast(angle a);

#define Round(x)   ((int) (x + 0.5))

fix FloatToFixFast(float num);

//Conversion macros
#define FloatToFix(num) ((fix)((num)*FLOAT_SCALER))
#define IntToFix(num) ((num) << FIX_SHIFT)
#define ShortToFix(num) (((long) (num)) << FIX_SHIFT)
#define FixToFloat(num) (((float) (num)) / FLOAT_SCALER)
#define FixToInt(num) ((num) >> FIX_SHIFT)
#define FixToShort(num) ((short) ((num) >> FIX_SHIFT))

//Fixed-point math functions in inline ASM form
#if defined(WIN32)
	#include "win\fixwin32.h"
#endif

// use this instead of:
// for:  (int)floor(x+0.5f) use FloatRound(x)
//       (int)ceil(x-0.5f)  use FloatRound(x)
//       (int)floor(x-0.5f) use FloatRound(x-1.0f)
//       (int)floor(x)      use FloatRound(x-0.5f)
// for values in the range -2048 to 2048
int FloatRound( float x );

angle FixAtan2(float cos,float sin);
angle FixAsin(float v);
angle FixAcos(float v);

// Does a ceiling operation on a fixed number
fix FixCeil (fix num);

// Floors a fixed number 
fix FixFloor (fix num);

#endif
