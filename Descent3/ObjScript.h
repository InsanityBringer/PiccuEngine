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

#ifndef OBJSCRIPT_H
#define OBJSCRIPT_H

#include "pstypes.h"
#include "d3x_op.h"
#include "vecmat.h"

struct object;
struct trigger;


//	assigns scripts for a level.
void AssignScriptsForLevel();

//	free scripts for a level
void FreeScriptsForLevel();

//allocates and initializes the scripts for an object.
void InitObjectScripts(object *objp,bool do_evt_created=true);

//frees all scripts for an object.
void FreeObjectScripts(object *objp,bool level_end=false);

//allocates and initializes the scripts for a trigger.
void InitTriggerScript(trigger *tp);

//frees all scripts for an trigger.
void FreeTriggerScript(trigger *tp,bool level_end=false);

#endif
