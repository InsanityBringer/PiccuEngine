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

#include "Briefing.h"
#include "BriefingParse.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "game.h"
#include "mem.h"
#include "hlsoundlib.h"
#include "streamaudio.h"
#include "pserror.h"
#include "ddio.h"
#include "descent.h"
#include "TelCom.h"
#include "TelComEffects.h"
#include "Mission.h"

struct tBriefingTag
{
	char* name;
	int id;
	int length;
};

#define TAG_LEVELNUM	0
#define TAG_PLEVELNUM	1
#define TAG_NLEVELNUM	2

tBriefingTag HotTags[] = 
{
	{"#LEVELNUM#",TAG_LEVELNUM,-1},
	{"#PLEVELNUM#",TAG_PLEVELNUM,-1},
	{"#NLEVELNUM#",TAG_NLEVELNUM,-1},
};
int NumHotTags = sizeof(HotTags)/sizeof(tBriefingTag);

int osb_xoff = 0,osb_yoff = 0;
int current_screen = -1;
tTelComInfo *pb_tcs = NULL;
bool gottitle = false;
bool pbfirst_call;
char pbtitle[100];
bool ok_to_parse_screen = false;
int skipped_screens;

bool IsMissionMaskOK(uint set,uint unset)
{
	uint Gamemissionmask = Current_mission.game_state_flags;

	uint fGamemissionmask;
	fGamemissionmask = Gamemissionmask^0xFFFFFFFF;

	if(!( ((set&Gamemissionmask)==set) && ((unset&fGamemissionmask)==unset) )){
		return false;
	}

	return true;
}

void ReplaceHotTag(char *string,int tag)
{
	if(tag<0 || tag>=NumHotTags)
	{
		*string = '\0';
		return;
	}

	switch(tag)
	{
	case TAG_LEVELNUM:
		sprintf(string,"%d",Current_mission.cur_level);
		break;
	case TAG_PLEVELNUM:
		sprintf(string,"%d",Current_mission.cur_level-1);
		break;
	case TAG_NLEVELNUM:
		sprintf(string,"%d",Current_mission.cur_level+1);
		break;
	}
}

//returns true if there were hot tags and it had to allocate memory for dest (so it needs to be freed)
#define MEMORY_BLOCK	50
bool ParseForHotTags(char *src,char **dest)
{
	bool ret = false;
	char *curr_ptr = src;
	char *dest_ptr;
	*dest = NULL;

	bool done = false;
	int length = strlen(src)+1;
	int curr_size = length;
	int curr_index = 0;

	dest_ptr = *dest = (char *)mem_malloc(length);
	if(!dest_ptr)
		return false;
	char replacement[256];

	while(!done)
	{
		if( *curr_ptr != '#' )
		{
			if(*curr_ptr=='\0')
			{
				done = true;
			}
			else
			{
				*dest_ptr = *curr_ptr;
				dest_ptr++;
				curr_ptr++;
				curr_index++;
				length--;

				if(length<1)
				{
					//we need to allocate some more memory
					char *new_mem = (char *)mem_malloc(curr_size+MEMORY_BLOCK);
					if(!new_mem)
					{
						//out of memory
						mem_free((*dest));
						*dest = NULL;
						return false;
					}

					memcpy(new_mem,(*dest),curr_index);
					mem_free(*dest);
					*dest = new_mem;
					dest_ptr = &new_mem[curr_index];
					curr_size += MEMORY_BLOCK;
					length += MEMORY_BLOCK;
				}
			}
		}else
		{
			int is_hot_tag = -1;

			//see if it's a hot tag
			for(int i=0;i<NumHotTags;i++)
			{
				if(HotTags[i].length==-1)
					HotTags[i].length = strlen(HotTags[i].name);

				if(!strnicmp(curr_ptr,HotTags[i].name,HotTags[i].length))
				{
					//this is a hot tag!!!!!
					mprintf((0,"Found Embedded HotTag: %s\n",HotTags[i].name));
					is_hot_tag = i;
					ret = true;
					break;
				}
			}

			if(is_hot_tag==-1)
			{
				//it wasn't a hot tag continue on
				*dest_ptr = *curr_ptr;
				dest_ptr++;
				curr_ptr++;
				curr_index++;
				length--;

				if(length<1)
				{
					//we need to allocate some more memory
					char *new_mem = (char *)mem_malloc(curr_size+MEMORY_BLOCK);
					if(!new_mem)
					{
						//out of memory
						mem_free((*dest));
						*dest = NULL;
						return false;
					}

					memcpy(new_mem,(*dest),curr_index);
					mem_free(*dest);
					*dest = new_mem;
					dest_ptr = &new_mem[curr_index];
					curr_size += MEMORY_BLOCK;
					length += MEMORY_BLOCK;
				}
			}else
			{
				//we need to replace the hot tag
				ReplaceHotTag(replacement,is_hot_tag);
				curr_ptr += HotTags[is_hot_tag].length;//skip past hot tag

				int dest_left = strlen(replacement);
				char *r_ptr = replacement;

				while(dest_left > 0 )
				{
					*dest_ptr = *r_ptr;
					dest_ptr++;
					curr_index++;
					length--;

					if(length<1)
					{
						//we need to allocate some more memory
						char *new_mem = (char *)mem_malloc(curr_size+MEMORY_BLOCK);
						if(!new_mem)
						{
							//out of memory
							mem_free((*dest));
							*dest = NULL;
							return false;
						}

						memcpy(new_mem,(*dest),curr_index);
						mem_free(*dest);
						*dest = new_mem;
						dest_ptr = &new_mem[curr_index];
						curr_size += MEMORY_BLOCK;
						length += MEMORY_BLOCK;
					}

					dest_left--;
					r_ptr++;
				}				
			}
		}
	}
	*dest_ptr = '\0';

	if(!ret)
	{
		mem_free((*dest));
		*dest = NULL;
	}

	return ret;
}

bool PlayBriefing(tTelComInfo *tcs)
{
	TelcomRenderSetScreen(0);
	bool done = false;

	while(!done){
		Sound_system.BeginSoundFrame(Telcom_called_from_game);

		if(tcs->state==TCS_POWEROFF || tcs->current_status!=TS_MISSION){
			//we're done with the loop
			done = true;
		}
		//Process all waiting events for the TelCom	(we may only want to handle this once a frame!)
		TelComHandleAllEvents(tcs);

		TelcomRenderScreen();
		Descent->defer();
		if(KEY_STATE(KEY_ESC))
			tcs->state = TCS_POWEROFF;

		Sound_system.EndSoundFrame();
	}
	return true;
}

void PBAddTextEffect(LPTCTEXTDESC desc,char *text,char *description,int id)
{	
	if(IsMissionMaskOK(desc->mission_mask_set,desc->mission_mask_unset) && ok_to_parse_screen)
	{
		char *new_text;
		bool new_stuff;

		new_stuff = ParseForHotTags(text,&new_text);

		if(!new_stuff)
			new_text = text;

		CreateTextEffect(desc,new_text,MONITOR_MAIN,current_screen,id);

		if(new_stuff)
		{
			mem_free(new_text);
		}
	}
}

void PBAddBmpEffect(LPTCBMPDESC desc,char *description)
{
	if(IsMissionMaskOK(desc->mission_mask_set,desc->mission_mask_unset) && ok_to_parse_screen)
		CreateBitmapEffect(desc,MONITOR_MAIN,current_screen);
}

void PBAddMovieEffect(LPTCMOVIEDESC desc,char *description)
{
	if(IsMissionMaskOK(desc->mission_mask_set,desc->mission_mask_unset) && ok_to_parse_screen)
		CreateMovieEffect(desc,MONITOR_MAIN,current_screen);
}

void PBAddBkgEffect(LPTCBKGDESC desc,char *description)
{
	if(IsMissionMaskOK(desc->mission_mask_set,desc->mission_mask_unset) && ok_to_parse_screen)
	{
		mprintf((0,"PB: Add Bkg\n"));
	}
}

void PBAddPolyEffect(LPTCPOLYDESC desc,char *description)
{
	if(IsMissionMaskOK(desc->mission_mask_set,desc->mission_mask_unset) && ok_to_parse_screen)
		CreatePolyModelEffect(desc,MONITOR_MAIN,current_screen);
}

void PBAddSoundEffect(LPTCSNDDESC desc,char *description)
{
	if(IsMissionMaskOK(desc->mission_mask_set,desc->mission_mask_unset) && ok_to_parse_screen)
		CreateSoundEffect(desc,MONITOR_MAIN,current_screen);
}

void PBAddButtonEffect(LPTCBUTTONDESC desc,char *description,int id)
{
	if(IsMissionMaskOK(desc->mission_mask_set,desc->mission_mask_unset) && ok_to_parse_screen){
		desc->x += osb_xoff;
		desc->y += osb_yoff;
		desc->tab_stop = false;	//no briefing buttons can have focus
		CreateButtonEffect(desc,MONITOR_MAIN,current_screen,id);
	}
}

void PBStartScreen(int screen_num,char *description,char *layout,uint mask_set,uint mask_unset)
{	
	if(!IsMissionMaskOK(mask_set,mask_unset)){
		ok_to_parse_screen = false;
		skipped_screens++;
		return;
	}
	int real_screen_num = screen_num - skipped_screens;
	ok_to_parse_screen = true;

	TelcomStartScreen(real_screen_num);
	current_screen = real_screen_num;

	TCBKGDESC backg;
	backg.color = BACKGROUND_COLOR;
	backg.caps = TCBGD_COLOR;
	backg.type = TC_BACK_STATIC;
	CreateBackgroundEffect(&backg,MONITOR_TOP,current_screen);
	if(gottitle){
		TCTEXTDESC textd;
		textd.color = GR_RGB(200,200,200);
		textd.type = TC_TEXT_STATIC;
		textd.font = BBRIEF_FONT_INDEX;
		textd.textbox.left = 4;
		textd.textbox.right = 300;
		textd.textbox.top = 2;
		textd.textbox.bottom = 80;
		textd.caps = TCTD_COLOR|TCTD_FONT|TCTD_TEXTBOX;
		CreateTextEffect(&textd,pbtitle,MONITOR_TOP,current_screen);
	}

	if(layout){
		TCBMPDESC bmpd;
		bmpd.caps = TCBD_XY;
		bmpd.type = TC_BMP_STATIC;
		bmpd.flags = 0;
		bmpd.x = bmpd.y = 0;
		strcpy(bmpd.filename,layout);
		CreateBitmapEffect(&bmpd,MONITOR_MAIN,current_screen);
	}
}

void PBEndScreen()
{
	if(ok_to_parse_screen){
		TelcomEndScreen();
		current_screen = -1;
	}
}

bool PBLoopCallback()
{
	if(!pbfirst_call){
		Sound_system.EndSoundFrame();
	}else
		pbfirst_call = false;

	bool ret = false;
	Sound_system.BeginSoundFrame(Telcom_called_from_game);

	if(pb_tcs->state==TCS_POWEROFF || pb_tcs->current_status!=TS_MISSION){
		//we're done with the loop
		ret = true;
	}

	//Process all waiting events for the TelCom	(we may only want to handle this once a frame!)
	TelComHandleAllEvents(pb_tcs);
	TelcomRenderScreen();

	Descent->defer();
	if(KEY_STATE(KEY_ESC)){
		pb_tcs->state = TCS_POWEROFF;
		ret = true;
	}
	
	return ret;
}

void PBSetTitle(char *title)
{
	gottitle = true;
	strcpy(pbtitle,title);
}

void PBSetStatic(float amount)
{
	if(amount>0)
		TelcomEnableStatic(amount);
}

void PBSetGlitch(float amount)
{
	if(amount>0)
		TelcomEnableGlitch(amount);
}

void PBAddVoice(char *filename,int flags,char *description)
{
}

bool ParseBriefing(char *filename,tTelComInfo *tcs)
{
	if(!cfexist(filename)){
		tcs->state = TCS_POWEROFF;
		return false;
	}

	osb_xoff = tcs->Monitor_coords[MONITOR_MAIN].left;
	osb_yoff = tcs->Monitor_coords[MONITOR_MAIN].top;
	current_screen = -1;
	pb_tcs = tcs;
	gottitle = false;
	pbtitle[0] = '\0';
	pbfirst_call = true;

	CBriefParse engine;
	tBriefParseCallbacks pb_callbacks;

	pb_callbacks.AddTextEffect		= PBAddTextEffect;
	pb_callbacks.AddBmpEffect		= PBAddBmpEffect;
	pb_callbacks.AddMovieEffect		= PBAddMovieEffect;
	pb_callbacks.AddBkgEffect		= PBAddBkgEffect;
	pb_callbacks.AddPolyEffect		= PBAddPolyEffect;
	pb_callbacks.AddSoundEffect		= PBAddSoundEffect;
	pb_callbacks.AddButtonEffect	= PBAddButtonEffect;
	pb_callbacks.StartScreen		= PBStartScreen;
	pb_callbacks.EndScreen			= PBEndScreen;
	pb_callbacks.LoopCallback		= PBLoopCallback;
	pb_callbacks.SetTitle			= PBSetTitle;
	pb_callbacks.SetStatic			= PBSetStatic;
	pb_callbacks.SetGlitch			= PBSetGlitch;
	pb_callbacks.AddVoice			= PBAddVoice;

	engine.SetCallbacks(&pb_callbacks);

	DestroyAllScreens();
	TelcomRenderSetScreen(DUMMY_SCREEN);

	skipped_screens = 0;
	int ret = engine.ParseBriefing(filename);
	Sound_system.EndSoundFrame();

	if(ret==PBERR_NOERR){
		PlayBriefing(tcs);
	}

	DestroyAllScreens(true);
	TelcomCreateDummyScreen();
	TelcomRenderSetScreen(DUMMY_SCREEN);

	TelcomDisableStatic();
	TelcomDisableGlitch();

	return false;
}
