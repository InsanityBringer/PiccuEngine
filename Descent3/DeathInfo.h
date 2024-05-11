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

#ifndef _DEATHINFO_H
#define _DEATHINFO_H

//Get the death delay type 
#define DEATH_DELAY(f) (f & DF_DELAY_MASK)

//Get the explosion size
#define DEATH_EXPL_SIZE(f) (f & DF_EXPL_SIZE_MASK)

//Include the actual flags
#include "deathinfo_external.h"

#endif
