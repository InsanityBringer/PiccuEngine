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

#ifndef VIS_EFFECT_H
#define VIS_EFFECT_H

#include "viseffect_external.h"

extern vis_effect *VisEffects;
extern int Highest_vis_effect_index;

// Returns the next free viseffect
int VisEffectAllocate ();

// Frees up a viseffect for use
int VisEffectFree (int visnum);

int VisEffectInitType (vis_effect *vis);

//initialize a new viseffect.  adds to the list for the given room
//returns the object number
int VisEffectCreate(ubyte type,ubyte id,int roomnum,vector *pos);

//link the viseffect  into the list for its room
// Does nothing for effects over terrain
void VisEffectLink(int visnum,int roomnum);

// Unlinks a viseffect from a room
// Does nothing for terrain
void VisEffectUnlink(int visnum);

//when an effect has moved into a new room, this function unlinks it
//from its old room and links it into the new room
void VisEffectRelink(int visnum,int newroomnum);

// Frees all the vis effects that are currently in use
void FreeAllVisEffects ();

// Goes through our array and clears the slots out
void InitVisEffects();

//remove viseffect from the world
void VisEffectDelete(int visnum);

// Kills all the effects that are dead
void VisEffectDeleteDead();

// Moves our visuals
void VisEffectMoveAll ();

// Renders a vis effect
void DrawVisEffect (vis_effect *vis);

// Creates a some sparks that go in random directions
void CreateRandomSparks (int num_sparks,vector *pos,int roomnum,int which_index=-1,float force_scalar=1);

// Creates a some line sparks that go in random directions
void CreateRandomLineSparks (int num_sparks,vector *pos,int roomnum,ushort color=0,float force_scalar=1);

// Creates vis effects but has the caller set their parameters
//initialize a new viseffect.  adds to the list for the given room
//returns the vis number
int VisEffectCreateControlled(ubyte type,object *parent,ubyte id,int roomnum,vector *pos,float lifetime,vector *velocity,int phys_flags=0,float size=0,float mass=0.0f,float drag=0.0f,bool isreal=0);

// Creates a some particles that go in random directions
void CreateRandomParticles (int num_sparks,vector *pos,int roomnum,int bm_handle,float size,float life);

// Attaches viseffects that move with an object
void AttachRandomNapalmEffectsToObject (object *obj);

#endif
