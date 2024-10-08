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

#ifndef __ENDIAN_H___
#define __ENDIAN_H___

extern "C"
{
// Endian_IsLittleEndian
//
//	Returns true if the machine is Little Endian (i.e. 80x86)
//	Returns false if the machine is Big Endian (i.e. Macintosh)
bool Endian_IsLittleEndian(void);

// Swaps (if needed) a short value (2 bytes) (assumes incoming value is in little endian format)
short Endian_SwapShort(short value);

// Swaps (if needed) an int value (4 bytes) (assumes incoming value is in little endian format)
int Endian_SwapInt(int value);

// Swaps (if needed) a float value (4 bytes) (assumes incoming value is in little endian format)
float Endian_SwapFloat(float value);
}
#endif
