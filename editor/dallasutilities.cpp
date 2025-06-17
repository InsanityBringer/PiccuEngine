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
 

#include "../neweditor/StdAfx.h"

#include <string.h>
#include <ctype.h>

#include "pserror.h"
#include "cfile.h"
#include "mem.h"
#include "mono.h"
#include "psglob.h"
#include "manage.h"
#include "gamefile.h"

#include "DallasUtilities.h"


// Removes any whitespace padding from the end of a string
void RemoveTrailingWhitespace(char *s)
{
	int last_char_pos;

	last_char_pos=strlen(s)-1;
	while(last_char_pos>=0 && isspace(s[last_char_pos])) {
		s[last_char_pos]='\0';
		last_char_pos--;
	}
}

// Returns a pointer to the first non-whitespace char in given string
char *SkipInitialWhitespace(char *s)
{
	while((*s)!='\0' && isspace(*s)) 
		s++;

	return(s);
}


class tFindInManage
{
public:
	tFindInManage() {searching = false;}

	bool searching;
	bool in_Gamefiles;
	int curr_index;
	char glob_string[PAGENAME_LEN];
};
tFindInManage FindInManageData;



bool FindManageFirst(char *buffer,char *wildcard)
{
#ifndef NEWEDITOR
	if(FindInManageData.searching)
		FindManageClose();

	FindInManageData.searching = true;
	strcpy(FindInManageData.glob_string,wildcard);
	FindInManageData.curr_index = 0;
	FindInManageData.in_Gamefiles = true;

	return FindManageNext(buffer);
#else
	return false;
#endif
}

bool FindManageNext(char *buffer)
{
#ifndef NEWEDITOR
	if(!FindInManageData.searching)
		return false;

	if(FindInManageData.in_Gamefiles)
	{

		for(;FindInManageData.curr_index<MAX_GAMEFILES;FindInManageData.curr_index++)
		{
			if(Gamefiles[FindInManageData.curr_index].used && 
				PSGlobMatch(FindInManageData.glob_string,Gamefiles[FindInManageData.curr_index].name,false,false))
			{
				//match
				strcpy(buffer,Gamefiles[FindInManageData.curr_index].name);
				FindInManageData.curr_index++;
				return true;
			}
		}

		FindInManageData.curr_index = 0;
		FindInManageData.in_Gamefiles = false;
	}

	for(;FindInManageData.curr_index<MAX_TRACKLOCKS;FindInManageData.curr_index++)
	{
		if(GlobalTrackLocks[FindInManageData.curr_index].used && GlobalTrackLocks[FindInManageData.curr_index].pagetype==PAGETYPE_GAMEFILE && 
			PSGlobMatch(FindInManageData.glob_string,GlobalTrackLocks[FindInManageData.curr_index].name,false,false))
		{
			//match
			strcpy(buffer,GlobalTrackLocks[FindInManageData.curr_index].name);
			FindInManageData.curr_index++;
			return true;
		}
	}

	return false;
#else
	return false;
#endif
}

void FindManageClose(void)
{
#ifndef NEWEDITOR
	FindInManageData.searching = false;
#endif
}

bool GamefileExists(char *name)
{
	bool found;
	char buffer[PAGENAME_LEN+1];

	found=FindManageFirst(buffer,name);
	FindManageClose();

	return(found);
}

