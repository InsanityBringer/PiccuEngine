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
 
#include "stdafx.h"
#include <string.h>
#include "globals.h"
#include "ned_Tablefile.h"
#include "ned_GameTexture.h"
#include "mono.h"
#include "vclip.h"

ned_texture_info	GameTextures[MAX_TEXTURES];
int Num_textures					= 0;

// =======================
// ned_FreeTextureData
// =======================
//
// Given a GameTextures slot this function frees any memory that may 
// need to be freed before a texture is destroyed
// DO NOT TOUCH TABLE STACK DATA
void ned_FreeTextureData(int slot);

// ===========================
// ned_InitializeTextureData
// ===========================
//
// Given a GameTextures slot this function initializes the data inside it
// to default information
// DO NOT TOUCH TABLE STACK DATA
void ned_InitializeTextureData(int slot);

// ====================
// ned_FindTexture
// ====================
//
// given the name of a texture it will return it's id...-1 if it doesn't exist
// it will only search textures loaded from tablefiles
int ned_FindTexture(char *name)
{
	ASSERT(name);
	if(!name)
		return -1;

	int i;
	for(i=0;i<MAX_TEXTURES;i++)
	{
		if(GameTextures[i].used && GameTextures[i].table_file_id!=-1)
		{
			//see if the name matches
			if(!strnicmp(name,GameTextures[i].name,PAGENAME_LEN-1))
			{
				//match
				return i;
			}
		}
	}
	return -1;
}

// ====================
// ned_FindTextureBitmap
// ====================
//
// given the name of a bitmap of a texture it will return it's id...-1 if it doesn't exist
// it will only search textures loaded from tablefiles
int ned_FindTextureBitmap(char *name)
{
	ASSERT(name);
	if(!name)
		return -1;

	int i;
	for(i=0;i<MAX_TEXTURES;i++)
	{
		if(GameTextures[i].used && GameTextures[i].table_file_id!=-1)
		{
			//see if the name matches
			if(!strnicmp(name,GameTextures[i].image_filename,PAGENAME_LEN-1))
			{
				//match
				return i;
			}
		}
	}
	return -1;
}

// =====================
// ned_AllocTexture
// =====================
//
//	Searches for an available texture ID, and returns it, -1 if none
//  if name is not NULL, then it is being allocated from a table file, and thus, the tablefile
//	parameter must also be specified.  If there is already a texture by that name (loaded from 
//  a table file), than it will be pushed onto the stack.
int ned_AllocTexture(char *name,int tablefile)
{
	if(name)
	{
		//this is being allocated for a tablefile load
		ASSERT(tablefile!=-1);
		if(tablefile==-1)
			return -1;

		//check to see if it's already in memory
		int old_id = ned_FindTexture(name);
		if(old_id!=-1)
		{
			//this item is already in memory!
			ned_FreeTextureData(old_id);
			ned_InitializeTextureData(old_id);			

			if(GameTextures[old_id].table_file_id==tablefile)
			{
				//we're just re-reading it

			}else
			{
				//push it onto the stack
				ntbl_PushTableStack(GameTextures[old_id].table_stack,GameTextures[old_id].table_file_id);
				ntbl_IncrementTableRef(tablefile);

				GameTextures[old_id].table_file_id = tablefile;
			}
			return old_id;
		}
	}

	int i,index = -1;
	for(i=0;i<MAX_TEXTURES;i++)
	{
		if(!GameTextures[i].used)
		{
			index = i;
			memset(&GameTextures[i],0,sizeof(ned_texture_info));
			GameTextures[i].bm_handle = -1;		// we don't have a bitmap loaded
			GameTextures[i].table_file_id = -1;	//we don't belong to any table file right now
			for(int j=0;j<MAX_LOADED_TABLE_FILES;j++) GameTextures[i].table_stack[j] = -1;
			break;
		}
	}

	if(index!=-1)
	{
		GameTextures[index].used = true;
		Num_textures++;

		if(name)
		{
			ASSERT(strlen(name)<PAGENAME_LEN);

			//table file load, give it a name and mark it's tablefile
			strncpy(GameTextures[index].name,name,PAGENAME_LEN-1);
			GameTextures[index].name[PAGENAME_LEN-1] = '\0';
			GameTextures[index].table_file_id = tablefile;

			ntbl_IncrementTableRef(tablefile);

		}else
		{
			//not from tablefile
			GameTextures[index].name[0] = '\0';
			GameTextures[index].table_file_id = -1;
		}

		ned_InitializeTextureData(index);
	}
	
	return index;
}


// ==================
// ned_FreeTexture
// ==================
//
// given a texture slot it free's it and makes it available
// if force_unload is true, and this slot was loaded from a table file, then
// it will unload all instances (based on it's table stack) from memory, else
// it will just pop off the current instance and load back in the popped version
void ned_FreeTexture(int slot,bool force_unload)
{
	ASSERT(slot>=0 && slot<MAX_TEXTURES);
	if(slot<0 || slot>=MAX_TEXTURES)
		return;

	ASSERT(GameTextures[slot].used);
	if(!GameTextures[slot].used)
		return;

	ned_FreeTextureData(slot);

	/////////////////////////////////////////
	if(GameTextures[slot].table_file_id==-1)
	{
		GameTextures[slot].used = false;
		Num_textures--;
		return;
	}

	//it has table file references, decrement
	ntbl_DecrementTableRef(GameTextures[slot].table_file_id);

	if(force_unload)
	{
		GameTextures[slot].used = false;
		Num_textures--;

		//go through it's stack, decrement references
		int tid;
		tid = ntbl_PopTableStack(GameTextures[slot].table_stack);
		while(tid!=-1)
		{
			ntbl_DecrementTableRef(tid);
			tid = ntbl_PopTableStack(GameTextures[slot].table_stack);
		}
	}else
	{
		//see if we have anything on the stack
		GameTextures[slot].table_file_id = ntbl_PopTableStack(GameTextures[slot].table_stack);
		if(GameTextures[slot].table_file_id==-1)
		{
			//nothing on the stack, its a dead one
			GameTextures[slot].used = false;
			Num_textures--;
		}else
		{
			//reload the item
			ned_InitializeTextureData(slot);
			if(!ntbl_OverlayPage(PAGETYPE_TEXTURE,slot))
			{
				Int3();

				GameTextures[slot].used = false;
				Num_textures--;
			}			
		}
	}
}

// =========================
// ned_FreeAllTextures
// =========================
//
// Frees all textures from memory
void ned_FreeAllTextures ()
{
	for (int i=0;i<MAX_TEXTURES;i++)
	{
		if (GameTextures[i].used)
			ned_FreeTexture (i,true);
	}
}

// ======================
// ned_InitTextures
// ======================
// 
// Initializes the Texture system
int ned_InitTextures ()
{
	// Initializes the texture system

	int i,j,tex;
	
	mprintf ((0,"Initializing texture system.\n"));

	memset(GameTextures,0,sizeof(ned_texture_info)*MAX_TEXTURES);
	for(i=0;i<MAX_TEXTURES;i++)
	for(j=0;j<MAX_LOADED_TABLE_FILES;j++)
		GameTextures[i].table_stack[j] = -1;
	
	tex=ned_AllocTexture();
	GameTextures[tex].bm_handle=BAD_BITMAP_HANDLE;
	strcpy (GameTextures[tex].name,"SAMPLE TEXTURE");
	GameTextures[tex].ref_count = 1;	

	// Initialize procedural tables and such
	//@@InitProcedurals ();

	atexit (ned_FreeAllTextures);

	return 1;
}

// ===========================
// ned_InitializeTextureData
// ===========================
//
// Given a GameTextures slot this function initializes the data inside it
// to default information
// DO NOT TOUCH TABLE STACK DATA
void ned_InitializeTextureData(int slot)
{
	GameTextures[slot].bm_handle = -1;
	GameTextures[slot].flags = 0;		//default to no type
	GameTextures[slot].ref_count = 0;
	GameTextures[slot].r = 1.0f;
	GameTextures[slot].g = 1.0f;
	GameTextures[slot].b = 1.0f;
	GameTextures[slot].speed= 1.0;

	//@@GameTextures[slot].corona_type=0;
	//@@GameTextures[slot].bumpmap=-1;
	//@@GameTextures[slot].procedural=NULL;
	GameTextures[slot].alpha = 1.0;	
	//@@GameTextures[slot].reflectivity=.6f;
	GameTextures[slot].destroy_handle=-1;
}

// =======================
// ned_FreeTextureData
// =======================
//
// Given a GameTextures slot this function frees any memory that may 
// need to be freed before a texture is destroyed
// DO NOT TOUCH TABLE STACK DATA
void ned_FreeTextureData(int slot)
{
	if(GameTextures[slot].flags&TF_ANIMATED)
	{
		//free the vclip
		if(GameTextures[slot].bm_handle>=0)
		{
			FreeVClip (GameTextures[slot].bm_handle);
		}
	}else
	{
		//free bitmap
		if(GameTextures[slot].bm_handle>BAD_BITMAP_HANDLE)
		{
			bm_FreeBitmap(GameTextures[slot].bm_handle);
		}
	}
	GameTextures[slot].bm_handle = -1;
	GameTextures[slot].ref_count = 0;
}

// ==========================
// ned_GetNextTexture
// ==========================
//
// Given current index, gets index of next texture in use
int ned_GetNextTexture (int n)
{
	int i;

	if (Num_textures==0)
		return -1;

	for (i=n+1;i<MAX_TEXTURES;i++)
	{
		if (GameTextures[i].used)
			return i;
	}
	for (i=0;i<n;i++)
	{
		if (GameTextures[i].used)
			return i;
	}

	return n;
}

// ======================
// ned_GetPreviousTexture
// ======================
//
// Given current index, gets index of prev texture in use
int ned_GetPreviousTexture (int n)
{
	int i;

	if (Num_textures==0)
		return -1;

	for (i=n-1;i>=0;i--)
	{
		if (GameTextures[i].used)
			return i;
	}
	for (i=MAX_TEXTURES-1;i>n;i--)
	{
		if (GameTextures[i].used)
			return i;
	}

	return n;
}

// ======================
// ned_GetTextureBitmap
// ======================
//
// Given a texture handle, returns that textures bitmap
// If the texture is animated, returns framenum mod num_of_frames in the animation
// Force is to force the evaluation of a procedural
// Also figures in gametime
int ned_GetTextureBitmap (int handle,int framenum,bool force)
{
	int src_bitmap;

	if(handle<0 || handle>=MAX_TEXTURES)
		return BAD_BITMAP_HANDLE;

	if (! GameTextures[handle].used)
		return BAD_BITMAP_HANDLE;

	if (GameTextures[handle].flags & TF_ANIMATED)
	{
		float cur_frametime;
		int int_frame;
		
		PageInVClip (GameTextures[handle].bm_handle);
		vclip *vc=&GameVClips[GameTextures[handle].bm_handle];
		ASSERT (vc->used>=1);

		if (GameTextures[handle].flags & TF_PING_PONG)
		{
			// Ping pong this texture

			float frametime=GameTextures[handle].speed/vc->num_frames;
			cur_frametime=Gametime/frametime;
			int_frame=cur_frametime;
			int_frame+=framenum;

			int_frame%=(vc->num_frames*2);
			if (int_frame>=vc->num_frames)
				int_frame=(vc->num_frames-1)-(int_frame%vc->num_frames);
			else
				int_frame%=vc->num_frames;
			src_bitmap=vc->frames[int_frame];
		}
		else
		{
			float frametime=GameTextures[handle].speed/vc->num_frames;
			cur_frametime=Gametime/frametime;
			int_frame=cur_frametime;
			int_frame+=framenum;
			src_bitmap=vc->frames[int_frame % vc->num_frames];
		}
	}
//	else if (GameTextures[handle].flags & TF_PROCEDURAL)
//		src_bitmap=GameTextures[handle].procedural->procedural_bitmap;
	else
		src_bitmap=GameTextures[handle].bm_handle;

//	if (GameTextures[handle].flags & TF_PROCEDURAL)	// Do a procedural
//	{
//		return BAD_BITMAP_HANDLE;
		/*
		int do_eval=1;
		
		if (GameTextures[handle].procedural==NULL)
			AllocateProceduralForTexture (handle);

		if (GameTextures[handle].procedural->last_procedural_frame==FrameCount)
			do_eval=0;
		if (timer_GetTime()<GameTextures[handle].procedural->last_evaluation_time+GameTextures[handle].procedural->evaluation_time)
			do_eval=0;

		if (!force && !Detail_settings.Procedurals_enabled)
		{
			if (timer_GetTime()<GameTextures[handle].procedural->last_evaluation_time+10.0)
				do_eval=0;
		}
		
		if (do_eval)
		{
			EvaluateProcedural (handle);
			GameTextures[handle].procedural->last_procedural_frame=FrameCount;
			GameTextures[handle].procedural->last_evaluation_time=timer_GetTime();
			src_bitmap=GameTextures[handle].procedural->procedural_bitmap;
			GameBitmaps[src_bitmap].flags|=BF_CHANGED;
		}
		else
			src_bitmap=GameTextures[handle].procedural->procedural_bitmap;
		*/
//	}

	if(src_bitmap==-1)
	{
		//you need to ned_MarkTextureInUse
		Int3();
	}

	return src_bitmap;

}

// ========================
// ned_MarkTextureInUse
// ========================
//
// Handles memory management for a texture.  Call this, passing true when you need to use a texture
// when the texture is no longer needed, call this again, passing false.
void ned_MarkTextureInUse(int slot,bool inuse)
{
	ASSERT(slot>=0 && slot<MAX_TEXTURES);
	if(slot<0 || slot>=MAX_TEXTURES)
		return;

	ASSERT(GameTextures[slot].used);
	if(!GameTextures[slot].used)
		return;

	if(inuse)
	{
		ASSERT(GameTextures[slot].ref_count>=0);

		if(GameTextures[slot].ref_count==0)
		{
			int texture_size;
			int mipped;
			mipped = 1;

			if(GameTextures[slot].flags&TF_TEXTURE_64)
			{
				texture_size = SMALL_TEXTURE;
			}else if(GameTextures[slot].flags&TF_TEXTURE_32)
			{
				texture_size = TINY_TEXTURE;
			}else if(GameTextures[slot].flags&TF_TEXTURE_256)
			{
				mipped = 0;
				texture_size = HUGE_TEXTURE;
			}else
			{
				texture_size = NORMAL_TEXTURE;
			}

			//load in the texture bitmap
			if(GameTextures[slot].flags&TF_ANIMATED)
			{
				GameTextures[slot].bm_handle = AllocLoadVClip (GameTextures[slot].image_filename,texture_size,mipped,1);

				if(GameTextures[slot].bm_handle>=0)
				{
					PageInVClip (GameTextures[slot].bm_handle);
				}
			}else
			{
//				if(!(GameTextures[slot].flags&TF_PROCEDURAL))
//				{
					//regular bitmap
					GameTextures[slot].bm_handle = bm_AllocLoadFileBitmap(GameTextures[slot].image_filename,0);
//				}else
//				{
					//procedural
//					GameTextures[slot].bm_handle = -1;
//				}
			}
			
			if(GameTextures[slot].bm_handle<0)
				GameTextures[slot].bm_handle = BAD_BITMAP_HANDLE;
		}
		GameTextures[slot].ref_count++;
	}else
	{
		ASSERT(GameTextures[slot].ref_count>0);
		GameTextures[slot].ref_count--;
		if(GameTextures[slot].ref_count==0)
		{
			//unload the texture bitmap...no longer needed
			if(GameTextures[slot].flags&TF_ANIMATED)
			{
				//animated bitmap
				if(GameTextures[slot].bm_handle>=0)
				{
					FreeVClip(GameTextures[slot].bm_handle);					
				}
			}else
			{
//				if(!(GameTextures[slot].flags&TF_PROCEDURAL))
//				{
					//regular bitmap
					if(GameTextures[slot].bm_handle>BAD_BITMAP_HANDLE)
					{
						bm_FreeBitmap(GameTextures[slot].bm_handle);				
					}
//				}else
//				{
					//procedural
//				}
			}

			GameTextures[slot].bm_handle = -1;			
		}
	}
}


void SwitchTexture(room *rp, int tex1, int tex2)
{
	// Adjust texture usage
	if ( ROOMNUM(rp) < MAX_ROOMS )
	{
		LevelTexDecrementTexture(tex1);
		LevelTexIncrementTexture(tex2);
	}
	else
	{
		ned_MarkTextureInUse(tex1,false);
		ned_MarkTextureInUse(tex2,true);
	}
}
