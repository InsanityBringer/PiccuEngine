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

#include "trigger.h"
#include "room.h"
#include "object.h"
//@$-#include "d3x.h"
#include "pserror.h"
#include "osiris_dll.h"
#include "levelgoal.h"

#ifdef LINUX
#include <stdlib.h>
#endif

#include <string.h>

//The maximum number of triggers that can be in the mine
#define MAX_TRIGGERS 100

//The list of triggers for the mine
trigger Triggers[MAX_TRIGGERS];

//The number of triggers currently in the mine
int Num_triggers=0;

void FreeTriggers();

//	initializes trigger system
void InitTriggers()
{
	int i;

	for (i = 0; i < MAX_TRIGGERS; i++)
	{
		Triggers[i].roomnum = -1;
		Triggers[i].facenum = -1;
		Triggers[i].flags = 0;

		Triggers[i].osiris_script.script_id		 = -1;
		Triggers[i].osiris_script.script_instance= NULL;
	}

	Num_triggers = 0;

	atexit(FreeTriggers);
}

//Free up all the triggers
void FreeTriggers()
{
	int i;

	for (i = 0; i < Num_triggers; i++)
		FreeTriggerScript(&Triggers[i]);

	Num_triggers = 0;
}

//Returns a pointer to the trigger attached to the given room:face
//Generates an error if the trigger cannot be found
//Paramaters:	roomnum,facenum - the room and face with the trigger
trigger *FindTrigger(int roomnum,int facenum)
{
	face *fp = &Rooms[roomnum].faces[facenum];
	trigger *tp;
	int i;

	//Go through all the triggers & look for the requested one
	for (i=0,tp=Triggers;i<Num_triggers;i++,tp++)
		if (tp->roomnum == roomnum)
				if (tp->facenum == facenum)
					return tp;

	Int3();		//Didn't find the requested trigger -- very bad!

	return NULL;
}

//Called to see if a trigger was tripped
void CheckTrigger(int roomnum,int facenum,object *objp,int event_type)
{
	room *rp = &Rooms[roomnum];
	face *fp = &rp->faces[facenum];

	ASSERT((event_type == TT_PASS_THROUGH) || (event_type == TT_COLLIDE));

	if (fp->flags & FF_HAS_TRIGGER)
	{
		trigger *tp;
		int type;

		tp = FindTrigger(roomnum,facenum);	ASSERT(tp != NULL);

		if (tp == NULL)
		{
			Int3();		//very bad!  Get Matt.
			return;
		}

		//Check for dead or disabled trigger
		if (tp->flags & (TF_DISABLED | TF_DEAD))
			return;

		//If portal trigger, make sure this is a pass-through event
		if ((fp->portal_num != -1) && (event_type != TT_PASS_THROUGH))
			return;

		//Get the activator type for the object that hit the trigger
		switch (objp->type)
		{
			case OBJ_PLAYER:	type = AF_PLAYER; break;
			case OBJ_BUILDING:
			case OBJ_ROBOT:	type = AF_ROBOT; break;
			case OBJ_CLUTTER:	type = AF_CLUTTER; break;
			case OBJ_WEAPON:
			{
				object *parent = ObjGetUltimateParent(objp);
				int parent_type = parent->type;
				if (parent_type == OBJ_PLAYER)
					type = AF_PLAYER_WEAPON;
				else if (parent_type == OBJ_ROBOT || parent_type == OBJ_BUILDING)
					type = AF_ROBOT_WEAPON;
				else
					return;		//unknown parent for weapon
				break;
			}
			default:
				return;
		}

		//Check if this object is a valid activator for this trigger
		if (tp->activator & type) 
		{
			mprintf((0,"Hit trigger %d\n",tp-Triggers));

			//Execute this trigger's script
			tOSIRISEventInfo ei;
			ei.evt_collide.it_handle = objp->handle;
			Osiris_CallTriggerEvent((tp-Triggers),EVT_COLLIDE,&ei);

			//If one-shot, kill this trigger, and mark face as destroyed
			if (tp->flags & TF_ONESHOT)
			{
				tp->flags |= TF_DEAD;
				if (fp->portal_num == -1)		//don't destroy a portal face
					fp->flags |= FF_DESTROYED;
			}

			if(tp->flags & TF_INFORM_ACTIVATE_TO_LG)
			{
				Level_goals.Inform(LIT_TRIGGER, LGF_COMP_ACTIVATE, tp - Triggers);
			}
		}
	}
}

//Enable or disable a trigger
void TriggerSetState(int trigger_num,bool enabled)
{
	trigger *tp = &Triggers[trigger_num];

	if (enabled)
		tp->flags &= ~TF_DISABLED;
	else
		tp->flags |= TF_DISABLED;
}

//Determines if a trigger is enabled or disabled
//Returns TRUE if enabled, else false
bool TriggerGetState(int trigger_num)
{
	trigger *tp = &Triggers[trigger_num];

	return ((tp->flags & TF_DISABLED) == 0);
}

#ifdef EDITOR

#include "editor\d3edit.h"

//
// EDITOR functions follow
//

//Create a new trigger
//Parameters:	roomnum,facenum - where the trigger is
//					flags - flags for this trigger
//					activator - activator mask
//					script - handle for the script for this trigger
//Returns:	trigger number of new trigger, or -1 if error
int AddTrigger(char *name,int roomnum,int facenum,int activator,const char *script)
{
	room *rp = &Rooms[roomnum];
	face *fp = &rp->faces[facenum];
	trigger *tp;

	if (Num_triggers >= MAX_TRIGGERS)
		return -1;

	if (strlen(name) > TRIG_NAME_LEN) {
		Int3();
		return -1;
	}

	tp = &Triggers[Num_triggers];

	strcpy(tp->name,name);
	tp->roomnum			= roomnum;
	tp->facenum			= facenum;
	tp->activator		= activator;
	tp->flags			= 0;

	//Flag the face
	fp->flags |= FF_HAS_TRIGGER;

	//Update count
	Num_triggers++;

	//Update flag
	World_changed = 1;

	//Everything ok
	return Num_triggers-1;
}

//Remove a trigger
//Paramters:	trig_num - the trigger to delete
void DeleteTrigger(int trig_num)
{
	trigger *tp = &Triggers[trig_num];
	room *rp = &Rooms[tp->roomnum];
	face *fp = &rp->faces[tp->facenum];

	//Clear the face flag
	fp->flags &= ~FF_HAS_TRIGGER;

	//Free the script for this trigger
	FreeTriggerScript(tp);

	//Move other triggers down in list
	for (int i=trig_num;i<Num_triggers-1;i++)
		Triggers[i] = Triggers[i+1];

	//Update count
	Num_triggers--;
}

//Remove a trigger
//Paramters:	roomnum,facenum - where the trigger is
void DeleteTrigger(int roomnum,int facenum)
{
	trigger *tp;
	
	tp = FindTrigger(roomnum,facenum);

	ASSERT(tp != NULL);

	DeleteTrigger(tp-Triggers);
}

#endif
