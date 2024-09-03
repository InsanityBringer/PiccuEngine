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

#include "psrand.h"

static long ps_holdrand = 1L;

//These are adapted from the C runtime lib. Pretty simple.

void ps_srand(unsigned int seed)
{
	ps_holdrand = (long)seed;
}

int ps_rand(void)
{
	return(((ps_holdrand = ps_holdrand * 214013L + 2531011L) >> 16) & 0x7fff);
}
