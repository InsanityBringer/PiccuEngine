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

#ifndef GAMETEXTURE_H
#define GAMETEXTURE_H

#ifdef NEWEDITOR /* only include tablefile header (manage stuff for NEWEDITOR) */
#include "..\neweditor\ned_TableFile.h"
#include "..\neweditor\ned_GameTexture.h"
#else

#include "manage.h"

#define TF_VOLATILE				1
#define TF_WATER				(1<<1)
#define TF_METAL				(1<<2)		// Shines like metal
#define TF_MARBLE				(1<<3)		// Shines like marble
#define TF_PLASTIC				(1<<4)		// Shines like plastic
#define TF_FORCEFIELD			(1<<5)
#define TF_ANIMATED				(1<<6)
#define TF_DESTROYABLE			(1<<7)
#define TF_EFFECT				(1<<8)
#define TF_HUD_COCKPIT			(1<<9)
#define TF_MINE					(1<<10)
#define TF_TERRAIN				(1<<11)
#define TF_OBJECT				(1<<12)
#define TF_TEXTURE_64			(1<<13)
#define TF_TMAP2				(1<<14)
#define TF_TEXTURE_32			(1<<15)
#define TF_FLY_THRU				(1<<16)
#define TF_PASS_THRU			(1<<17)
#define TF_PING_PONG			(1<<18)
#define TF_LIGHT				(1<<19)
#define TF_BREAKABLE			(1<<20)		// Breakable (as in glass)
#define TF_SATURATE				(1<<21)
#define TF_ALPHA				(1<<22)
#define TF_DONTUSE				(1<<23)		
#define TF_PROCEDURAL			(1<<24)		
#define TF_WATER_PROCEDURAL		(1<<25)
#define TF_FORCE_LIGHTMAP		(1<<26)
#define TF_SATURATE_LIGHTMAP	(1<<27)
#define TF_TEXTURE_256			(1<<28)
#define TF_LAVA					(1<<29)
#define TF_RUBBLE				(1<<30)
#define TF_SMOOTH_SPECULAR		(1<<31)

#define TF_TEXTURE_TYPES	(TF_MINE+TF_TERRAIN+TF_OBJECT+TF_EFFECT+TF_HUD_COCKPIT+TF_LIGHT)

#define TF_SPECULAR			(TF_METAL+TF_MARBLE|TF_PLASTIC)


#define NOT_TEXTURE			0
#define NORMAL_TEXTURE		1				// a normal size texture
#define SMALL_TEXTURE		2				// 1/4 of a normal texture
#define TINY_TEXTURE		3				// 1/8 of a normal texture
#define HUGE_TEXTURE		4				// Double the size of a normal texture

#define MAX_TEXTURES 3100

#define PROC_MEMORY_TYPE_NONE	0
#define PROC_MEMORY_TYPE_FIRE	1
#define PROC_MEMORY_TYPE_WATER	2

struct static_proc_element
{
	ubyte type;

	ubyte frequency;
	ubyte speed;
	ubyte color;
	ubyte size;

	ubyte x1,y1,x2,y2;
	
};

struct proc_struct
{
	short dynamic_proc_elements;	// list of dynamic procedural texture elements
	void *proc1;					// pointer for procedural page
	void *proc2;					// back page of procedural
	short procedural_bitmap;		// handle to the bitmap holding the finished procedural

	ushort *palette;
	static_proc_element *static_proc_elements;
	ushort num_static_elements;

	ubyte memory_type;

	ubyte heat;
	ubyte thickness;
	ubyte light;

	float last_evaluation_time;
	float evaluation_time;
	float osc_time;
	ubyte osc_value;

	int	last_procedural_frame;		// last frame a procedural was calculated for this texture
};

struct texture
{
	char name[PAGENAME_LEN];			// this textures name
	int flags;							// values defined above
	int bm_handle;						// handle which shows what this texture looks like
	int destroy_handle;					// handle which denotes the destroyed image
	
	int damage;
	float reflectivity;
	float r,g,b;						// colored lighting	 (0 to 100%)

	float slide_u,slide_v;				// How many times this texture slides during a second
	float alpha;						// alpha value (from 0 to 1)
	float speed;						// how fast this texture animates

	proc_struct *procedural;

	int	sound;							// The sound this texture makes
	float	sound_volume; 				// The volume for this texture's sound

	short bumpmap;						// The bumpmap for this texture, or -1 if there is none
	ubyte corona_type;					// what type of corona this thing uses
	ubyte used;							// is this texture free to be allocated?

};


extern texture GameTextures[MAX_TEXTURES];
extern int Num_textures;

// Inits the texture system, returning 1 if successful
int InitTextures();
void ShutdownTextures();

// Set aside a texture for use
int AllocTexture (void);

// Frees a texture for future use
void FreeTexture (int);

// Given current index, gets index of next texture in use
int GetNextTexture (int n);

// Given current index, gets index of prev texture in use
int GetPreviousTexture (int n);

// Searches thru all textures for a specific name, returns -1 if not found
// or index of texture with name
int FindTextureName (char *name);

// Searches thru all textures for a bitmap of a specific name, returns -1 if not found
// or index of texture with name
int FindTextureBitmapName (char *name);

// Given a texture handle, returns that textures bitmap
// If the texture is animated, returns framenum mod num_of_frames in the animation
int GetTextureBitmap (int handle,int framenum,bool force=false);

// Given a filename, loads either the bitmap or vclip found in that file.  If type
// is not NULL, sets it to 1 if file is animation, otherwise sets it to zero
int LoadTextureImage (char *filename,int *type,int texture_size,int mipped,int pageable=0,int format=0);

// Goes through and marks a texture as a tmap2 if its bitmap(s) have transparency
bool CheckIfTextureIsTmap2 (int texnum);

// Touches a texture, makes sure its in memory
void TouchTexture (int n);

// Builds the bumpmaps for the texture
void BuildTextureBumpmaps (int texhandle);

// Allocates the memory needed for static elements for a procedural texture
void AllocateStaticProceduralsForTexture (int handle,int num_elements);

// Frees the memory used by static procedural elements
void FreeStaticProceduralsForTexture (int handle);

// Frees all the procedural memory for a texture
void FreeProceduralForTexture (int n);

// Allocates memory for a procedural that is attached to a texture
int AllocateProceduralForTexture (int handle);

#endif

#endif
