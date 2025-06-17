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
 


#ifndef NED_TEXTURE_H_
#define NED_TEXTURE_H_

typedef struct
{
	bool used;
	char name[PAGENAME_LEN];			// this textures name

	int flags;							// values defined in ned_GameTexture.h
	char image_filename[PAGENAME_LEN];
	int bm_handle;						// handle which shows what this texture looks like
	int destroy_handle;					// handle which denotes the destroyed image

	float r,g,b;	//lighting

	float slide_u,slide_v;			// How many times this texture slides during a second
	float alpha;						// alpha value (from 0 to 1)
	float speed;	//vclip speed
	float reflectivity;

	int ref_count;
	int table_file_id;
	int table_stack[MAX_LOADED_TABLE_FILES];
}ned_texture_info;

#define MAX_TEXTURES	2600
extern ned_texture_info	GameTextures[MAX_TEXTURES];
extern int Num_textures;

#define TF_VOLATILE				1
#define TF_WATER					(1<<1)
#define TF_METAL					(1<<2)		// Shines like metal
#define TF_MARBLE					(1<<3)		// Shines like marble
#define TF_PLASTIC				(1<<4)		// Shines like plastic
#define TF_FORCEFIELD			(1<<5)
#define TF_ANIMATED				(1<<6)
#define TF_DESTROYABLE			(1<<7)
#define TF_EFFECT					(1<<8)
#define TF_HUD_COCKPIT			(1<<9)
#define TF_MINE					(1<<10)
#define TF_TERRAIN				(1<<11)
#define TF_OBJECT					(1<<12)
#define TF_TEXTURE_64			(1<<13)
#define TF_TMAP2					(1<<14)
#define TF_TEXTURE_32			(1<<15)
#define TF_FLY_THRU				(1<<16)
#define TF_PASS_THRU				(1<<17)
#define TF_PING_PONG				(1<<18)
#define TF_LIGHT					(1<<19)
// #define TF_UNUSED6				(1<<20) // Gwar - added TF_BREAKABLE cause its required by BOA.cpp...what is this TF_UNUSED6?
#define TF_BREAKABLE				(1<<20)		// Breakable (as in glass)
#define TF_SATURATE				(1<<21)
#define TF_ALPHA					(1<<22)
#define TF_DONTUSE				(1<<23)		
#define TF_PROCEDURAL			(1<<24)		
#define TF_WATER_PROCEDURAL	(1<<25)
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
#define TINY_TEXTURE			3				// 1/8 of a normal texture
#define HUGE_TEXTURE			4				// Double the size of a normal texture

#define PROC_MEMORY_TYPE_NONE		0
#define PROC_MEMORY_TYPE_FIRE		1
#define PROC_MEMORY_TYPE_WATER	2

// ====================
// ned_FindTexture
// ====================
//
// given the name of a texture it will return it's id...-1 if it doesn't exist
// it will only search textures loaded from tablefiles
int ned_FindTexture(char *name);

// ====================
// ned_FindTextureBitmap
// ====================
//
// given the name of a bitmap of a texture it will return it's id...-1 if it doesn't exist
// it will only search textures loaded from tablefiles
int ned_FindTextureBitmap(char *name);

// =========================
// ned_FreeAllTextures
// =========================
//
// Frees all textures from memory
void ned_FreeAllTextures ();

// ==================
// ned_FreeTexture
// ==================
//
// given a texture slot it free's it and makes it available
// if force_unload is true, and this slot was loaded from a table file, then
// it will unload all instances (based on it's table stack) from memory, else
// it will just pop off the current instance and load back in the popped version
void ned_FreeTexture(int slot,bool force_unload=false);

// =====================
// ned_AllocTexture
// =====================
//
//	Searches for an available texture ID, and returns it, -1 if none
//  if name is not NULL, then it is being allocated from a table file, and thus, the tablefile
//	parameter must also be specified.  If there is already a texture by that name (loaded from 
//  a table file), than it will be pushed onto the stack.
int ned_AllocTexture(char *name=NULL,int tablefile=-1);

// ======================
// ned_InitTextures
// ======================
// 
// Initializes the Texture system
int ned_InitTextures ();

// ======================
// ned_GetPreviousTexture
// ======================
//
// Given current index, gets index of prev texture in use
int ned_GetPreviousTexture (int n);

// ==========================
// ned_GetNextTexture
// ==========================
//
// Given current index, gets index of next texture in use
int ned_GetNextTexture (int n);

// ======================
// ned_GetTextureBitmap
// ======================
//
// Given a texture handle, returns that textures bitmap
// If the texture is animated, returns framenum mod num_of_frames in the animation
// Force is to force the evaluation of a procedural
// Also figures in gametime
int ned_GetTextureBitmap (int handle,int framenum,bool force=false);

// wrapper to make game code happy
int GetTextureBitmap (int handle,int framenum,bool force=false);

// ========================
// ned_MarkTextureInUse
// ========================
//
// Handles memory management for a texture.  Call this, passing true when you need to use a texture
// when the texture is no longer needed, call this again, passing false.
void ned_MarkTextureInUse(int slot,bool inuse);

#include "room_external.h"

void SwitchTexture(room *rp, int tex1, int tex2);

#endif

