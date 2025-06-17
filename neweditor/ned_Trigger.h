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
 


#ifndef __NED_TRIGGER_H_
#define __NED_TRIGGER_H_

#include "trigger.h"

//Free up all the triggers
void FreeTriggers();

//	initializes trigger system
void InitTriggers();

// from main/ObjScript.cpp
//frees the script for a trigger (YES, this is really empty!)
void FreeTriggerScript(trigger *tp,bool level_end);

// from main\trigger.cpp

//Returns a pointer to the trigger attached to the given room:face
//Generates an error if the trigger cannot be found
//Paramaters:	roomnum,facenum - the room and face with the trigger
trigger *FindTrigger(int roomnum,int facenum);

// copy of osipf_FindTriggerName
int FindTriggerName(char *name);

//Create a new trigger
//Parameters:	roomnum,facenum - where the trigger is
//					flags - flags for this trigger
//					activator - activator mask
//					script - handle for the script for this trigger
//Returns:	trigger number of new trigger, or -1 if error
int AddTrigger(char *name,int roomnum,int facenum,int activator,const char *script);

//Remove a trigger
//Paramters:	trig_num - the trigger to delete
void DeleteTrigger(int trig_num);

//Remove a trigger
//Paramters:	roomnum,facenum - where the trigger is
void DeleteTrigger(int roomnum,int facenum);










#endif