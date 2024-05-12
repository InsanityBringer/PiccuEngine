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

#include "ObjScript.h"

#include "object.h"
#include "objinfo.h"
#include "Mission.h"
#include "pserror.h"
#include "trigger.h"
#include "door.h"
#include "game.h"
#include "multi.h"
#include "osiris_dll.h"

#include <string.h>

void InitTriggerScript(trigger* tp);
void FreeTriggerScript(trigger* tp);

//	We assign all the scripts needed for the level right here.
void AssignScriptsForLevel()
{
	int i;

	//Initialize scripts for the objects
	for (i = 0; i <= Highest_object_index; i++)
	{
		if (Objects[i].type != OBJ_NONE && Objects[i].osiris_script == NULL)
			InitObjectScripts(&Objects[i]);
	}

	//Initialize scripts for the triggers
	for (i = 0; i < Num_triggers; i++)
	{
		InitTriggerScript(&Triggers[i]);
	}
}


//	free scripts for a level
void FreeScriptsForLevel()
{
	int i;

	//Free scripts for the objects
	for (i = 0; i <= Highest_object_index; i++)
	{
		FreeObjectScripts(&Objects[i], true);
	}

	//Free scripts for the triggers
	for (i = 0; i < Num_triggers; i++)
	{
		FreeTriggerScript(&Triggers[i], true);
	}

	FreeLevelScript();
}


//allocates and initializes the scripts for an object.
//	robots, powerups, doors, etc.
void InitObjectScripts(object* objp, bool do_evt_created)
{
	//Bind scripts to the object
	Osiris_BindScriptsToObject(objp);

	if (do_evt_created)
	{
		tOSIRISEventInfo ei;
		if (objp->control_type == CT_AI)
		{
			Osiris_CallEvent(objp, EVT_AI_INIT, &ei);
		}

		Osiris_CallEvent(objp, EVT_CREATED, &ei);
	}
}


//frees all scripts for an object.
void FreeObjectScripts(object* objp, bool level_end)
{
	tOSIRISEventInfo ei;
	ei.evt_destroy.is_dying = (level_end) ? 1 : 0;

	if (!level_end)
		Osiris_CallEvent(objp, EVT_DESTROY, &ei);

	Osiris_DetachScriptsFromObject(objp);
}


//allocates and initializes the script for a trigger
void InitTriggerScript(trigger* tp)
{
}


//frees the script for a trigger
void FreeTriggerScript(trigger* tp, bool level_end)
{
}
