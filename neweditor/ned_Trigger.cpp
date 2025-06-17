/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */
 #include <string.h>
#include "ned_Trigger.h"
#include "globals.h"
#include "pserror.h"


//The maximum number of triggers that can be in the mine
#define MAX_TRIGGERS 100

//The list of triggers for the mine
trigger Triggers[MAX_TRIGGERS];


//Free up all the triggers
void FreeTriggers()
{
	int i;

	for (i = 0; i < Num_triggers; i++)
		FreeTriggerScript(&Triggers[i]);

	Num_triggers = 0;
}

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


// from main/ObjScript.cpp
//frees the script for a trigger (YES, this is really empty!)
void FreeTriggerScript(trigger *tp,bool level_end)
{
}


// from main\trigger.cpp

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

// copy of osipf_FindTriggerName
int FindTriggerName(char *name)
{
	for(int i=0;i<Num_triggers;i++){
		if(Triggers[i].name){
			if(!stricmp(name,Triggers[i].name))
				return i;
		}
	}
	return -1;
}


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
