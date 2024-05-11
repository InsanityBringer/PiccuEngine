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

#ifndef FIREBALL_H

#define FIREBALL_H

#include "fireball_external.h"
#include "object.h"
#include "vecmat.h"
#include "manage.h"
#include "grdefs.h"
#include "DeathInfo.h"

#define PARTICLE_LIFE .5
#define DEBRIS_LIFE	2.0

#define FF_MOVES	1

#define VISUAL_FIREBALL		0		// This fireball is for looks only
#define REAL_FIREBALL		1		// This fireball can actually burn things

// The alpha for a fireball 
#define FIREBALL_ALPHA	.9f
// Smoke alpha
#define SMOKE_ALPHA	.3f

#define MAX_FIREBALL_SIZE		80.0

// The lifetime of the blast ring
#define DAMAGE_RING_TIME	1.5f

#define NUM_FIREBALLS	52

struct fireball
{
	char name[PAGENAME_LEN];	// The filename of this animation

	ubyte type;					// type of fireball, see above
	ubyte tex_size;				// What size texture to use for this animation
	float total_life;			// How long this animation should last (in seconds)
	float size;					// How big this explosion is
	short bm_handle;			// The handle to the vlip
};

extern fireball Fireballs[];

// Initalizes the explosion system
void InitFireballs();

// Given an object, renders the representation of this fireball
void DrawFireballObject (object *obj);

// Creates a fireball
// Returns object number on success, else -1 on error
// If vis_effect is non-zero, then this is a visual effect only
int CreateFireball(vector *pos,int fireball_num,int roomnum,int realtype=VISUAL_FIREBALL);

//Creates a fireball vis effect for the specified object
//The explosion size is twice the object size times size_scale
//The fireball type will be randomly selected based on the object size times size_scale
//Returns the visnum of the fireball
int CreateObjectFireball(object *objp,float size_scale=1.0);

// Control code for debris
void DoDebrisFrame(object *obj);

//Process a dying object for one frame
void DoDyingFrame(object *objp);

// A quick way to see where a weapon hits.  Weapons make debris.
void CreateWeaponDebris(object *obj);

// Creates a concussive blast (physics based -- no visuals)
void MakeShockwave(object *explode_obj_ptr, int parent_handle);

void DoConcussiveForce(object *explode_obj_ptr, int parent_handle,float player_scalar=1);

// Control code for explosions
void DoExplosionFrame(object *obj);

//Destroy an object immediately
void DestroyObject(object *objp,float explosion_mag,int death_flags);

// Creates a debris piece that goes off in a given direction, with a given magnitude
object *CreateSubobjectDebrisDirected(object *parent, int subobj_num,vector *dir,float explosion_mag,int death_flags=DF_DEBRIS_SMOKES);

//	Creates nifty splinters that shoot out from the body, I figure.
void CreateSplintersFromBody (object *obj,float explosion_mag,float lifetime);

// Creates a blast ring to be drawn
int CreateBlastRing (vector *pos,int index,float lifetime,float max_size,int roomnum,int force_blue=0);

// Creates a standard blast ring for an object
int CreateObjectBlastRing(object *objp);

// Creates a smolding smoke to be drawn
int CreateSmolderingObject (vector *pos,int index,float lifetime,float max_size,int roomnum);

// Draws a blast ring
void DrawBlastRingObject (object *obj);

// Draws a colored alpha disk...useful for cool lighting effects
void DrawColoredDisk (vector *pos,float r,float g,float b,float inner_alpha,float outer_alpha,float size,ubyte saturate=0,ubyte lod=1);

// Draws a colored alpha ring...useful for cool lighting effects
void DrawColoredRing (vector *pos,float r,float g,float b,float inner_alpha,float outer_alpha,float size,float inner_ring_ratio,ubyte saturate=0,ubyte lod=1);

// Creates a blast ring from an event
void DoBlastRingEvent (int eventnum,void *data);

// Makes a fireball have a custom vclip
int CreateCustomFireballObject (vector *pos,int fireball_num,int tex_handle,int roomnum);

// Creates an explosion
void DoExplosionEvent (int eventnum,void *data);

// An event handler that simply draws an alpha blended poly on top of the screen
// Takes a 4 element array of floats int r,g,b,a format
void DrawAlphaEvent (int eventnum,void *data);

// Returns a random medium sized explosion
int GetRandomMediumExplosion ();

// Returns a random small explosion
int GetRandomSmallExplosion ();

// Returns a random small explosion
int GetRandomBillowingExplosion ();

// Draws a sphere with the appropriate texture.  If texture=-1, then uses rgb as colors
void DrawSphere (vector *pos,float r,float g,float b,float alpha,float size,int texture,ubyte saturate=1);

// Creates end points that simulate lightning
void CreateLightningRodPositions (vector *src,vector *dest,vector *world_vecs,int num_segments,float rand_mag,bool do_flat);

// Draws a glowing cone of light using a bitmap
void DrawColoredGlow (vector *pos,float r,float g,float b,float size);

void CreateElectricalBolts(object *objp,int num_bolts);

//Play the explosion sound for this object
void PlayObjectExplosionSound(object *objp);

#endif
