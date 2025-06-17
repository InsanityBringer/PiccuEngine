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
 

#ifndef OBJECT_LIGHTING_H
#define OBJECT_LIGHTING_H

#include "object.h"

// Casts light from an object onto the rooms or terrain
void DoObjectLight(object *obj);

// Frees all the memory associated with lightmap objects
void ClearAllObjectLightmaps (int terrain);

// Frees all the memory associated with this objects lightmap
void ClearObjectLightmaps (object *obj);

// Sets up the memory to be used by an object for lightmaps
void SetupObjectLightmapMemory (object *obj);

//	makes the an object cloaked
void MakeObjectInvisible(object *obj,float time,float fade_time=1.0,bool no_hud_message=false);

//	makes the player visbile
void MakeObjectVisible(object *obj);

//Returns a pointer to the lighting info for the specified object
light_info *ObjGetLightInfo(object *objp);

//Sets an object to have local lighting info
void ObjSetLocalLighting(object *objp);

#endif