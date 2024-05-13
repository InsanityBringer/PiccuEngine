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

#include <stdlib.h>
#include <string.h>
#include "pstypes.h"
#include "pserror.h"
#include "bitmap.h"
#include "vclip.h"
#include "CFILE.H" 
#include "mono.h"
#include "ddio.h"
#include "gametexture.h"
#include "texture.h"
#include <string.h>
#include "ctype.h"
#include "mem.h"
#include "game.h"

vclip GameVClips[MAX_VCLIPS];
int Num_vclips = 0;

#define DEFAULT_FRAMETIME	.07f

#define VCLIP_VERSION	1

// Frees all the memory used by vclips
void FreeAllVClips()
{
	mprintf((0, "Freeing all vclips!\n"));

	for (int i = 0; i < MAX_VCLIPS; i++)
	{
		if (GameVClips[i].used > 0)
		{
			GameVClips[i].used = 1;
			FreeVClip(i);
		}
	}
}

// Simply sets all vclips to unused
void InitVClips()
{
	for (int i = 0; i < MAX_VCLIPS; i++)
		GameVClips[i].used = 0;

	atexit(FreeAllVClips);
}

// Allocs a vclip for use
// Returns -1 on error
int AllocVClip()
{
	int i;

	for (i = 0; i < MAX_VCLIPS; i++)
	{
		if (GameVClips[i].used == 0)
		{
			memset(&GameVClips[i], 0, sizeof(vclip));
			GameVClips[i].frames = (short*)mem_malloc(VCLIP_MAX_FRAMES * sizeof(short));
			GameVClips[i].allocated_frames = VCLIP_MAX_FRAMES;
			ASSERT(GameVClips[i].frames);
			GameVClips[i].frame_time = DEFAULT_FRAMETIME;
			GameVClips[i].flags = VCF_NOT_RESIDENT;
			GameVClips[i].used = 1;
			Num_vclips++;
			return i;
		}
	}

	Int3(); // Ran out of vclips.  What the hell are you doing?  Get Jason
	return -1;
}

void FreeVClip(int num)
{
	ASSERT(GameVClips[num].used > 0);

	GameVClips[num].used--;
	if (GameVClips[num].used > 0)
		return;					// other things are using this vclip

	if (!(GameVClips[num].flags & VCF_NOT_RESIDENT))
	{
		for (int i = 0; i < GameVClips[num].num_frames; i++)
			bm_FreeBitmap(GameVClips[num].frames[i]);
	}

	mem_free(GameVClips[num].frames);

	Num_vclips--;
	ASSERT(Num_vclips >= 0);
}

// Frees up the bitmaps used by a vclip
void FreeVClipResidency(int num)
{
	ASSERT(GameVClips[num].used > 0);

	mprintf((0, "Freeing vclip residency!\n"));

	if (!(GameVClips[num].flags & VCF_NOT_RESIDENT))
	{
		for (int i = 0; i < GameVClips[num].num_frames; i++)
			bm_FreeBitmap(GameVClips[num].frames[i]);
	}

	GameVClips[num].flags |= VCF_NOT_RESIDENT;
}

// Saves a given video clip to a file
// Returns 1 if everything ok, 0 otherwise
// "num" is index into GameVClip array
int SaveVClip(char* filename, int num)
{
	CFILE* outfile;
	vclip* vc = &GameVClips[num];

	ASSERT(vc->used);
	ASSERT(filename != NULL);

	PageInVClip(num);

	outfile = (CFILE*)cfopen(filename, "wb");
	if (!outfile)
	{
		mprintf((0, "Couldn't save vclip %s!\n", filename));
		return 0;
	}

	// write out the header for this vclip
	cf_WriteByte(outfile, 127);
	cf_WriteByte(outfile, (sbyte)VCLIP_VERSION);

	cf_WriteByte(outfile, vc->num_frames);
	//cf_WriteFloat (outfile,vc->play_time);
	cf_WriteFloat(outfile, vc->frame_time);
	//cf_WriteInt (outfile,vc->flags);
	//cf_WriteFloat (outfile,vc->light_value);

	// Now save each frame of this vclip
	for (int i = 0; i < vc->num_frames; i++)
	{
		if (bm_SaveBitmap(outfile, vc->frames[i]) != 1)
		{
			mprintf((0, "Couldn't save frame %d of vclip %s!\n", i, filename));
			Int3();
			cfclose(outfile);
			return 0;
		}
	}

	cfclose(outfile);
	return 1;
}

extern int Low_vidmem;
// Pages in a vclip if it needs to be
void PageInVClip(int vcnum)
{
	ASSERT(GameVClips[vcnum].used);
	if (!(GameVClips[vcnum].flags & VCF_NOT_RESIDENT))
		return;

	int mipped = 0;
	int texture_size = GameVClips[vcnum].target_size;
	vclip* vc = &GameVClips[vcnum];
	if (vc->flags & VCF_WANTS_MIPPED)
		mipped = 1;


	CFILE* infile = (CFILE*)cfopen(vc->name, "rb");
	if (!infile)
	{
		// due to a bug in some 3rd party tablefile editors, full paths might
		// have been used when they shouldn't have been
		char* end_ptr, * start_ptr;
		start_ptr = vc->name;
		end_ptr = start_ptr + strlen(start_ptr) - 1;
		while ((end_ptr >= start_ptr) && (*end_ptr != '\\')) end_ptr--;
		if (end_ptr < start_ptr)
		{
			mprintf((0, "Couldn't load vclip %s!\n", vc->name));
			return;
		}

		ASSERT(*end_ptr == '\\');
		end_ptr++;

		infile = (CFILE*)cfopen(end_ptr, "rb");
		if (!infile)
		{
			mprintf((0, "Couldn't load vclip %s!\n", vc->name));
			return;
		}
	}

	mprintf((0, "Paging in vclip %s!\n", vc->name));

	ubyte start_val = cf_ReadByte(infile);
	int version = 0;
	if (start_val != 127)
	{
		version = 0;
		vc->num_frames = start_val;
		cf_ReadFloat(infile);
		vc->frame_time = cf_ReadFloat(infile);
		cf_ReadInt(infile);
		cf_ReadFloat(infile);
		vc->frame_time = DEFAULT_FRAMETIME;
	}
	else
	{
		version = cf_ReadByte(infile);
		vc->num_frames = cf_ReadByte(infile);
		vc->frame_time = cf_ReadFloat(infile);
		vc->frame_time = DEFAULT_FRAMETIME;
	}

	//[ISB] Ensure enough frames are actually allocated
	//the 50 frame max was never documented and isn't enforced anywhere. Dynamic allocation was already used? Why?
	if (vc->num_frames > vc->allocated_frames)
	{
		int old_frames = vc->allocated_frames;
		mem_free(vc->frames);
		vc->frames = (short*)mem_malloc(sizeof(vc->frames[0]) * vc->num_frames);
		vc->allocated_frames = vc->num_frames;
		mprintf((0, "Vclip exceeded frame limit! Was %d, now %d\n", old_frames, vc->allocated_frames));
	}

	for (int i = 0; i < vc->num_frames; i++)
	{
		int n = bm_AllocLoadBitmap(infile, mipped);

		ASSERT(n > 0);

		int w, h;

		if (texture_size == NORMAL_TEXTURE)
		{
			w = TEXTURE_WIDTH;
			h = TEXTURE_HEIGHT;

#ifndef EDITOR
			if (Mem_low_memory_mode || Low_vidmem)
			{
				w = TEXTURE_WIDTH / 2;
				h = TEXTURE_HEIGHT / 2;
			}
#endif
		}
		else if (texture_size == SMALL_TEXTURE)
		{
			// Make small textures a quarter of the size of normal textures
			w = TEXTURE_WIDTH / 2;
			h = TEXTURE_HEIGHT / 2;

#ifndef EDITOR
			if (Mem_low_memory_mode || Low_vidmem)
			{
				w = TEXTURE_WIDTH / 4;
				h = TEXTURE_HEIGHT / 4;
			}
#endif
		}
		else if (texture_size == TINY_TEXTURE)
		{
			// Make these tinys an eigth of the size of normal textures
			w = TEXTURE_WIDTH / 4;
			h = TEXTURE_HEIGHT / 4;
		}
		else if (texture_size == HUGE_TEXTURE)
		{
			// Make these tinys an eigth of the size of normal textures
			w = TEXTURE_WIDTH * 2;
			h = TEXTURE_HEIGHT * 2;
		}
		else
		{
			w = bm_w(n, 0);
			h = bm_h(n, 0);
		}

		// If differing size, resize!
		if (w != bm_w(n, 0) || h != bm_h(n, 0))
		{
			int dest_bm;

			dest_bm = bm_AllocBitmap(w, h, mipped * ((w * h * 2) / 3));
			ASSERT(dest_bm >= 0);
			if (mipped)
				GameBitmaps[dest_bm].flags |= BF_MIPMAPPED;
			GameBitmaps[dest_bm].format = GameBitmaps[n].format;

			bm_ScaleBitmapToBitmap(dest_bm, n);
			strcpy(GameBitmaps[dest_bm].name, GameBitmaps[n].name);
			bm_FreeBitmap(n);

			n = dest_bm;
		}

		ASSERT(n >= 0);
		vc->frames[i] = n;		// assign frame to bitmap
	}

	cfclose(infile);

	vc->flags &= ~VCF_NOT_RESIDENT;
}

// Allocs and loads a vclip from the file named "filename"
// Returns -1 on error, index into GameVClip array on success
int AllocLoadVClip(char* filename, int texture_size, int mipped, int pageable, int format)
{
	char name[PAGENAME_LEN];

	ASSERT(filename != NULL);

	int i = strlen(filename);

	if (filename[i - 4] == '.' && filename[i - 3] == 'i' && filename[i - 2] == 'f' && filename[i - 1] == 'l')
		return AllocLoadIFLVClip(IGNORE_TABLE(filename), texture_size, mipped, format);

	if (filename[i - 4] == '.' && filename[i - 3] == 'a' && filename[i - 2] == 'b' && filename[i - 1] == 'm')
		Int3();	// Get Jason
	//return AllocLoadIFFAnimClip(filename,);

	ChangeVClipName(filename, name);

	// Check to see if this vclip already exists in memory
	if ((i = FindVClipName(IGNORE_TABLE(name))) != -1)
	{
		GameVClips[i].used++;
		return i;
	}

	//mprintf ((0,"Loading OAF vclip %s\n",name));

	int vcnum = AllocVClip();

	ASSERT(vcnum >= 0);
	strncpy(GameVClips[vcnum].name, name, PAGENAME_LEN);

	if (mipped)
		GameVClips[vcnum].flags |= VCF_WANTS_MIPPED;

	GameVClips[vcnum].target_size = texture_size;


	if (pageable == 1)
		return vcnum;


	PageInVClip(vcnum);

	if (GameVClips[vcnum].num_frames < 1)
	{
		FreeVClip(vcnum);
		return -1;
	}

	return vcnum;
}

// Allocs and loads a vclip from a 3DS ILS file
// Returns -1 on error, else index into GameVClips on success
// Argument texture means that this vclip is an animated texture and
// needs to have an 8bit version 
int AllocLoadIFLVClip(char* filename, int texture_size, int mipped, int format)
{
	CFILE* infile;
	char name[PAGENAME_LEN];
	unsigned int i, done = 0;

	ASSERT(filename != NULL);

	ChangeVClipName(filename, name);

	// Check to see if this vclip already exists in memory
	if ((i = FindVClipName(IGNORE_TABLE(name))) != -1)
	{
		GameVClips[i].used++;
		return i;
	}

	infile = (CFILE*)cfopen(filename, "rt");
	if (!infile)
	{
		mprintf((0, "Couldn't load IFL vclip %s!\n", filename));
		return -1;
	}

	mprintf((0, "Loading IFL vclip %s\n", name));

	int vcnum = AllocVClip();

	ASSERT(vcnum >= 0);

	vclip* vc = &GameVClips[vcnum];

	while (!done)
	{
		char curline[200];

		if (cfeof(infile))
		{
			done = 1;
			continue;
		}

		// Read a line and parse it
		cf_ReadString(curline, 200, infile);

		if (curline[0] == ';' || curline[1] == ';' || curline[0] == ' ' || curline[1] == ' ')
			continue;
		if (!(isalnum(curline[0])))
			continue;

		else if (curline[0] == '$')
		{
			char new_command[50];

			int i;
			for (i = 0; curline[i + 1] != '=' && i < 50; i++)
			{
				new_command[i] = curline[i + 1];
			}
			if (i == 50)
			{
				Int3();	// bad command in IFL!
				return -1;
			}

			i++;	// advance to data

			// parse data
			if (!stricmp(new_command, "TIME"))
			{
				// Set play time
				float play_time = atof(&curline[i]);
				ASSERT(play_time >= 0);
			}
		}
		else
		{
			int lastslash = -1;
			char bmname[200];


			for (i = 0; i < strlen(curline); i++)
				if (curline[i] == '\\')
					lastslash = i;

			if (lastslash == -1)
			{
				for (i = 0; i < strlen(filename); i++)
					if (filename[i] == '\\')
						lastslash = i;

				ASSERT(lastslash != -1);

				strcpy(bmname, filename);
				bmname[lastslash + 1] = 0;
				strcat(bmname, curline);
			}
			else
				strcpy(bmname, curline);

			// Try and load this file

			int bm = bm_AllocLoadFileBitmap(IGNORE_TABLE(bmname), 0, format);
			if (bm < 1)
			{
				Error("Error loading frame %d of ILS file %s!\n", vc->num_frames, filename);
				cfclose(infile);
				return -1;
			}

			int w, h;

			if (texture_size == NORMAL_TEXTURE)
			{
				w = TEXTURE_WIDTH;
				h = TEXTURE_HEIGHT;
			}
			else if (texture_size == SMALL_TEXTURE)
			{
				// Make small textures a quarter of the size of normal textures
				w = TEXTURE_WIDTH / 2;
				h = TEXTURE_HEIGHT / 2;
			}
			else if (texture_size == TINY_TEXTURE)
			{
				// Make these tinys an eigth of the size of normal textures
				w = TEXTURE_WIDTH / 4;
				h = TEXTURE_HEIGHT / 4;
			}
			else
			{
				w = bm_w(bm, 0);
				h = bm_h(bm, 0);
			}

			// If differing size, resize!
			if (w != bm_w(bm, 0) || h != bm_h(bm, 0))
			{
				int dest_bm;

				dest_bm = bm_AllocBitmap(w, h, mipped * ((w * h) / 3));
				ASSERT(dest_bm >= 0);
				if (mipped)
					GameBitmaps[dest_bm].flags |= BF_MIPMAPPED;
				GameBitmaps[dest_bm].format = GameBitmaps[bm].format;

				bm_ScaleBitmapToBitmap(dest_bm, bm);
				strcpy(GameBitmaps[dest_bm].name, GameBitmaps[bm].name);
				bm_FreeBitmap(bm);

				bm = dest_bm;
			}

			vc->frames[vc->num_frames] = bm;
			vc->num_frames++;
		}
	}

	cfclose(infile);


	if (vc->num_frames == 0)
	{
		mprintf((0, "vclip had no valid bitmap names!\n"));
		FreeVClip(vcnum);
		return -1;
	}

	strcpy(vc->name, name);
	return vcnum;
}

// gets the filename from a path, plus appends our .oaf extension
void ChangeVClipName(char* src, char* dest)
{
	int limit;
	char path[256], ext[256], filename[256];

	limit = PAGENAME_LEN - 5;

	ddio_SplitPath(src, path, filename, ext);

	// Make sure we don't go over our name length limit
	strncpy(dest, filename, limit);

	strcat(dest, ".oaf");
}

// Searches thru all vclips for a specific name, returns -1 if not found
// or index of vclip with name
int FindVClipName(char* name)
{
	int i;

	for (i = 0; i < MAX_VCLIPS; i++)
		if (GameVClips[i].used && !stricmp(GameVClips[i].name, name))
			return i;

	return -1;
}

// Returns frame "frame" of vclip "vclip".  Will mod the frame so that there
// is no overflow
int GetVClipBitmap(int v, int frame)
{
	ASSERT(GameVClips[v].used > 0);
	ASSERT(v >= 0 && v < MAX_VCLIPS);
	ASSERT(frame >= 0);

	vclip* vc = &GameVClips[v];

	int bm = vc->frames[frame % vc->num_frames];

	return bm;
}

// Loads an animation from an IFF ANIM file
int AllocLoadIFFAnimClip(char* filename, int texture)
{
	/*	char name[PAGENAME_LEN];
		int i;

		ASSERT (filename!=NULL);

		ChangeVClipName (filename,name);

		if ((i=FindVClipName(name))!=-1)
		{
			GameVClips[i].used++;
			return i;
		}

		mprintf ((0,"Loading IFF vclip %s\n",name));

		int vcnum=AllocVClip ();

		ASSERT (vcnum>=0);

		vclip *vc=&GameVClips[vcnum];

		vc->num_frames=bm_AllocLoadIFFAnim (filename,vc->frames,0);
		if (vc->num_frames==-1)
		{
			mprintf ((0,"Couldn't load vclip named %d!\n",name));
			FreeVClip (vcnum);
			return -1;
		}

		strcpy (vc->name,name);

		return vcnum;*/

	return -1;
}
