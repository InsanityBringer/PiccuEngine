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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "pstring.h"
#include "grdefs.h"
#include "hud.h"
#include "game.h"
#include "ddio.h"
#include "gamefont.h"
#include "newui.h"
#include "multi.h"
#include "player.h"
#include "game2dll.h"
#include "stringtable.h"
#include "dedicated_server.h"
#include "AppConsole.h"
#include "demofile.h"
#include "mem.h"
#include "textaux.h"
#include "3d.h"
#include "marker.h"
#include "controls.h"
#include "Mission.h"
#include "sounds.h"
#include "hlsoundlib.h"
#include "args.h"

#define HUD_MESSAGE_NORMAL		0
#define HUD_MESSAGE_BLINKING	1

#define HUD_MESSAGE_NONE		0
#define HUD_MESSAGE_GENERAL		1
#define HUD_MESSAGE_TEAM		2


// How long the message stays up for (in seconds)
#define HUD_MESSAGE_TIME		5
// How long the message scrolls for	(in seconds)
#define HUD_SCROLL_TIME			.5

char HudInputMessage[MAX_HUD_INPUT_LEN];
int Doing_input_message=HUD_MESSAGE_NONE;
int HudInputMessageLen=0;
static tDirtyRect HUD_inmsg_dirty_rect;

char HUD_messages[MAX_HUD_MESSAGES][HUD_MESSAGE_LENGTH];
int HUD_message_type[MAX_HUD_MESSAGES];
ddgr_color HUD_message_color[MAX_HUD_MESSAGES];
int Num_hud_messages=0;
int Hud_scroll_offset=0;
static tDirtyRect HUD_msg_dirty_rect;

float Hud_timer=0.0f;

static ubyte Hud_persistent_msg_id = HUD_INVALID_ID;
static float Hud_persistent_msg_timer = 0.0f;
static int Hud_persistent_msg_flags = 0;
static int Hud_persistent_msg_current_len;
static float Hud_persistent_msg_char_timer;
static float Hud_persistent_msg_id2 = HUD_INVALID_ID;
static int Hud_persistent_msg_sound_handle = SOUND_NONE_INDEX;
static tDirtyRect HUD_persist_dirty_rect[2];

tHUDItem *GetHUDItem(int id);

// game message console
MsgListConsole Game_msg_con;
static tMsgList Game_msg_list;
static bool Game_msg_con_vis = false;

// hud message console
MsgListConsole HUD_msg_con;
static tMsgList HUD_msg_list;
static bool HUD_msg_con_vis = false;
static bool Hud_messages_paused = false;

#define GAME_MSGCON_W		416
#define GAME_MSGCON_H		256
#define GAME_MSGCON_X		((Max_window_w-GAME_MSGCON_W)/2)
#define GAME_MSGCON_Y		((Max_window_h-GAME_MSGCON_H)/2)


// This function takes a HUD messages, ensures it isn't too long, and if it is, it correctly fixes it
void CorrectHudMessage(char *str);

///////////////////////////////////////////////////////////////////////////////
//	Functions


//	prints a string onto the debug consle
void AddMessageToRollback(char *msg)
{
	HUD_msg_list.add(msg);
}


// Adds a single line to the hud message list
//Returns true if message added, or false if message not (because the previous message was the same)
bool AddLineToHUDMessages (char *temp_message,ddgr_color color=-1);
bool AddLineToHUDMessages (char *temp_message,ddgr_color color)
{
	if (Num_hud_messages>0 && !strcmp (temp_message,HUD_messages[Num_hud_messages-1]))
		return 0;	// this is the same message as before, don't print it twice!

	if (Dedicated_server)
	{
		PrintDedicatedMessage ("*%s\n",temp_message);
		return 1;
	}

	AddMessageToRollback (temp_message);


	if (color==-1)
		color=GR_RGB(0,255,0);

	if (Num_hud_messages==MAX_HUD_MESSAGES)
	{
		// Get rid of top message

		for (int i=1;i<MAX_HUD_MESSAGES;i++)
		{
			strcpy (HUD_messages[i-1],HUD_messages[i]);
			HUD_message_type[i-1]=HUD_message_type[i];
			HUD_message_color[i-1]=HUD_message_color[i];
		}

		Num_hud_messages--;

		Hud_scroll_offset=0;

		if(!Hud_messages_paused)
			Hud_timer=Gametime;
	}

	if (Num_hud_messages==0 && !Hud_messages_paused)
		Hud_timer=Gametime;

	CorrectHudMessage(temp_message);

	strcpy (HUD_messages[Num_hud_messages],temp_message);
	HUD_message_type[Num_hud_messages]=HUD_MESSAGE_NORMAL;

	HUD_message_color[Num_hud_messages]=color;

	Num_hud_messages++;

	return 1;
}

//Adds line, splitting if too long
//Returns true if line added
bool AddMultipleLinesToHUDMessages (char *temp_message,ddgr_color color=-1);
bool AddMultipleLinesToHUDMessages (char *temp_message,ddgr_color color)
{
	char ourstr[HUD_MESSAGE_LENGTH*2];
	//char word[HUD_MESSAGE_LENGTH*2];
	char nextword[HUD_MESSAGE_LENGTH*2];
	static char thisline[HUD_MESSAGE_LENGTH*2] = "";
	thisline[0] = NULL;
	strcpy(ourstr,temp_message);
	char* p = strtok(ourstr," ");
	bool added = 0;
	while(p)
	{
		strcat(thisline,p);
		strcat(thisline," ");
		//strcpy(word,p);

		p = strtok(NULL," ");
		if(p)
		{
			strcpy(nextword,p);
			if((grtext_GetTextLineWidth(thisline)+grtext_GetTextLineWidth(nextword))>(Game_window_w*.7))
			{
				if(thisline[strlen(thisline)-1] == ' ')
					thisline[strlen(thisline)-1] = NULL;
				added |= AddLineToHUDMessages (thisline,color);

				//Scan for color information in the string we just added
				char *c=NULL,*c2;
				c2 = strchr(thisline,'\1');
				while (c2) 
				{
					c = c2;
					c2 = strchr(c2+4,'\1');
				}
				if (c) 
				{
					//found one
					for (int i=0;i<4;i++)
						thisline[i] = c[i];
					thisline[4] = 0;
				}
				else
					thisline[0] = NULL;
			}
			p = nextword;
		}
	}
	if(thisline[strlen(thisline)-1] == ' ')
		thisline[strlen(thisline)-1] = NULL;
	if(thisline[0])
		added |= AddLineToHUDMessages (thisline,color);

	return added;
}

// adds a colored hud message to the list
//Returns true if message added, or false if message not (because the previous message was the same)
bool AddColoredHUDMessage (ddgr_color color,char *format,...)
{
	va_list args;
	char *message = NULL;
	char *last_message=NULL;
	char temp_message[HUD_MESSAGE_LENGTH*2];

	va_start(args, format );
	Pvsprintf(temp_message,HUD_MESSAGE_LENGTH*2,format,args);
	va_end(args);

	if(Demo_flags==DF_RECORDING)
		DemoWriteHudMessage(color,false,temp_message);
	
	if(Dedicated_server)
		return AddLineToHUDMessages (temp_message,color);

	return AddMultipleLinesToHUDMessages (temp_message,color);
}

// Adds a HUD message (similar to AddColoredHUDMessage), however can be filtered out by
// a "-playermessages" command line.
bool AddFilteredColoredHUDMessage (ddgr_color color,char *format,...)
{
	static signed char checked_command_line = -1;

	if(checked_command_line==-1)
	{
		if(FindArg("-playermessages")!=0)
			checked_command_line = 1;
		else
			checked_command_line = 0;
	}

	if(Game_mode&GM_MULTI && checked_command_line)
		return false;	//filter this message

	va_list args;
	char *message = NULL;
	char *last_message=NULL;
	char temp_message[HUD_MESSAGE_LENGTH*2];

	va_start(args, format );
	Pvsprintf(temp_message,HUD_MESSAGE_LENGTH*2,format,args);
	va_end(args);

	if(Demo_flags==DF_RECORDING)
		DemoWriteHudMessage(color,false,temp_message);
	
	if(Dedicated_server)
		return AddLineToHUDMessages (temp_message,color);

	return AddMultipleLinesToHUDMessages (temp_message,color);
}


// Adds a HUD message (similar to AddHUDMessage), however can be filtered out by
// a "-playermessages" command line.
bool AddFilteredHUDMessage (char *format, ... )
{
	static signed char checked_command_line = -1;

	if(checked_command_line==-1)
	{
		if(FindArg("-playermessages")!=0)
			checked_command_line = 1;
		else
			checked_command_line = 0;
	}

	if(Game_mode&GM_MULTI && checked_command_line)
		return false;	//filter this message

	va_list args;
	char *message = NULL;
	char *last_message=NULL;
	char temp_message[HUD_MESSAGE_LENGTH*2];

	va_start(args, format );
	Pvsprintf(temp_message,HUD_MESSAGE_LENGTH*2,format,args);
	va_end(args);

	if(Demo_flags==DF_RECORDING)
		DemoWriteHudMessage(0,false,temp_message);

	if(Dedicated_server)
		return AddLineToHUDMessages (temp_message);

	return AddMultipleLinesToHUDMessages (temp_message);
}


// Adds a message to the HUD message list.  If the list is already full, punt the
// top one and move the others up 
//Returns true if message added, or false if message not (because the previous message was the same)
bool AddHUDMessage (char *format, ... )
{
	va_list args;
	char *message = NULL;
	char *last_message=NULL;
	char temp_message[HUD_MESSAGE_LENGTH*2];

	va_start(args, format );
	Pvsprintf(temp_message,HUD_MESSAGE_LENGTH*2,format,args);
	va_end(args);

	if(Demo_flags==DF_RECORDING)
		DemoWriteHudMessage(0,false,temp_message);

	if(Dedicated_server)
		return AddLineToHUDMessages (temp_message);

	return AddMultipleLinesToHUDMessages (temp_message);
}



// Adds a blinking message to the HUD message list.  If the list is already full, punt the
// top one and move the others up 
//Returns true if message added, or false if message not (because the previous message was the same)
bool AddBlinkingHUDMessage (char *format, ... )
{
	va_list args;
	char *message = NULL;
	char *last_message=NULL;
	char temp_message[HUD_MESSAGE_LENGTH*2];

	va_start(args, format );
	Pvsprintf(temp_message,HUD_MESSAGE_LENGTH*2,format,args);
	va_end(args);

	if(Demo_flags==DF_RECORDING)
		DemoWriteHudMessage(0,true,temp_message);

	CorrectHudMessage(temp_message);

	if (Num_hud_messages>0 && !strcmp (temp_message,HUD_messages[Num_hud_messages-1]))
		return 0;	// this is the same message as before, don't print it twice!

	if (Num_hud_messages==MAX_HUD_MESSAGES)
	{
		// Get rid of top message

		for (int i=1;i<MAX_HUD_MESSAGES;i++)
		{
			strcpy (HUD_messages[i-1],HUD_messages[i]);
			HUD_message_type[i-1]=HUD_message_type[i];
		}

		Num_hud_messages--;

		Hud_scroll_offset=0;

		if(!Hud_messages_paused)
			Hud_timer=Gametime;
	}

	if (Num_hud_messages==0 && !Hud_messages_paused)
		Hud_timer=Gametime;
		
	strcpy (HUD_messages[Num_hud_messages],temp_message);
	HUD_message_type[Num_hud_messages]=HUD_MESSAGE_BLINKING;

	Num_hud_messages++;

	return 1;
}

// This function takes a HUD messages, ensures it isn't too long, and if it is, it correctly fixes it
void CorrectHudMessage(char *str)
{
	//if (strlen(str)<=(HUD_MESSAGE_LENGTH-1))
	if(grtext_GetTextLineWidth(str)<=Game_window_w)
		return;

	mprintf ((0,"Message '%s' is too long!\n",str));
	//the line is too long, we need to shorten it, but when we do, we need to make sure that it isn't in the middle
	//of a color (0x01 0xFF 0xFF 0xFF)
	str[HUD_MESSAGE_LENGTH-1] = '\0';

	//go back the last 3 characters and look for an 0x01, if we find one, than make it an '\0' to cut the string
	for(int i=(HUD_MESSAGE_LENGTH-2); i>=(HUD_MESSAGE_LENGTH-4); i-- )
	{
		if(str[i]==0x01)
		{
			//we found a color start
			str[i] = '\0';
		}
	}	
}

//Called when the player hit the Multiplayer message key. 
//Puts the player in input mode, or if already inputting resets the hud input message.
void StartHUDInputMessage()
{
	HudInputMessage[0]=0;
	Doing_input_message=HUD_MESSAGE_GENERAL;
	HudInputMessageLen=0;
	SuspendControls();
	MultiSendRequestTypeIcon(true);
}

//Called when the player hits the Multiplayer team-message key.
//Puts the player in input mode, or if already inputting resets the hud input message.
void StartTeamHUDInputMessage()
{
	//make sure that HUD_MESSAGE_TEAM is only set for team games
	if(!Team_game ){
		StartHUDInputMessage();
		return;
	}

	HudInputMessage[0]=0;
	Doing_input_message=HUD_MESSAGE_TEAM;
	HudInputMessageLen=0;
	SuspendControls();
}


//	Given a hud message, it will determine who should get this message.
//	"name: message" = the player with the callsign name should get the message
//	"team: message" = the Player_num's team should get the message
//	"0-32: message" = the player with player num of 0-32 should get the message
//	all other messages are for everyone else
//
//	returns the starting position of the real message (past the :)
//	destination will receive one of the following values:
//		MULTI_SEND_MESSAGE_ALL			=	everyone should get this message
//		MULTI_SEND_MESSAGE_RED_TEAM		= only red team (0) should get this message
//		MULTI_SEND_MESSAGE_BLUE_TEAM	= only blue team (1) should get this message
//		MULTI_SEND_MESSAGE_GREEN_TEAM	= only green team (2) should get this message
//		MULTI_SEND_MESSAGE_YELLOW_TEAM	= only yellow team (3) should get this message
//		0-32							= player num of the player to get the message
char *GetMessageDestination(char *message,int *destination)
{
	int to_who = MULTI_SEND_MESSAGE_ALL;
	char *ret = message;

	//see if there is a colon in the string, and match that up with a name
	char *colon_pos = NULL;
	colon_pos = strchr(message,':');
	if(colon_pos>message)
	{
		//this message might be for one person in particular
		char buffer[256];
		memcpy(buffer,message,(colon_pos-message));
		buffer[colon_pos-message] = '\0';

		//see if buffer is a number, in which case just send to that pnum
		char *p = buffer;
		bool is_num = true;
		while( *p ) { if(!isdigit(*p)) {is_num=false;break;} p++; }

		if(is_num)
		{
			to_who = atoi(buffer);
			if(to_who<0 || to_who>=MAX_PLAYERS) to_who = MULTI_SEND_MESSAGE_ALL;
		}
		else
		{
			int possible_match = -1;
			int name_len = strlen(buffer);
			int call_len;

			//it's not a number, see if it matches any of the player names
			for(int pn=0;pn<MAX_PLAYERS;pn++)
			{
				if(	NetPlayers[pn].flags & NPF_CONNECTED && 
					NetPlayers[pn].sequence==NETSEQ_PLAYING)
				{
					call_len = strlen(Players[pn].callsign);

					if(call_len>name_len)
					{
						//try to match a partial name
						if(!strnicmp(Players[pn].callsign,buffer,name_len))
						{
							//possible match
							possible_match = pn;
						}
					}
					else if(call_len==name_len)
					{
						if(!stricmp(Players[pn].callsign,buffer) )
						{
							//match!
							to_who = pn;
							break;
						}
					}
				}//end if
			}//end for

			if(to_who<0 && possible_match!=-1)
				to_who = possible_match;
		}
		
		if(to_who==MULTI_SEND_MESSAGE_ALL)
		{
			//see if it is for the team
			if(!stricmp(buffer,"team"))
			{
				switch(Players[Player_num].team)
				{
				case 0:	to_who = MULTI_SEND_MESSAGE_RED_TEAM;		break;
				case 1: to_who = MULTI_SEND_MESSAGE_BLUE_TEAM;		break;
				case 2: to_who = MULTI_SEND_MESSAGE_GREEN_TEAM;		break;
				case 3: to_who = MULTI_SEND_MESSAGE_YELLOW_TEAM;	break;
				}
			}
		}

		ret = colon_pos + 1;
	}

	*destination = to_who;
	return ret;
}


// Sends off the input message the player was typing
void SendOffHUDInputMessage ()
{
	if(Doing_input_message==HUD_MESSAGE_NONE)
		return;

	if (Game_mode & GM_MULTI)
	{
		if (HudInputMessage[0]=='$')	// special command
		{
			DLLInfo.input_string=HudInputMessage;
			CallGameDLL (EVT_CLIENT_INPUT_STRING,&DLLInfo);
		}
		else
		{
			char str[255];
			sprintf (str,TXT_HUDSAY,Players[Player_num].callsign,HudInputMessage);
			
			switch(Doing_input_message)
			{
			case HUD_MESSAGE_GENERAL:
				{
					int to_who;

					char *colon_pos = GetMessageDestination(HudInputMessage,&to_who);

					if(to_who!=MULTI_SEND_MESSAGE_ALL)
					{
						if(to_who<0)
						{
							//to a team
							sprintf (str,"[%s]: %s",Players[Player_num].callsign,colon_pos);
						}
						else
						{
							//to a player
							//cut out what's after the colon
							sprintf (str,"<%s>:%s",Players[Player_num].callsign,colon_pos);
							AddHUDMessage("Sent private message to %s",Players[to_who].callsign);
						}
					}

					if (Netgame.local_role==LR_SERVER)
						MultiSendMessageFromServer (GR_RGB(0,128,255),str,to_who);
					else
						MultiSendMessageToServer (0,str,to_who);
				}break;
			case HUD_MESSAGE_TEAM:
				{
					int team = MULTI_SEND_MESSAGE_ALL;
					switch(Players[Player_num].team)
					{
					case 0:	team = MULTI_SEND_MESSAGE_RED_TEAM;	break;
					case 1: team = MULTI_SEND_MESSAGE_BLUE_TEAM; break;
					case 2:	team = MULTI_SEND_MESSAGE_GREEN_TEAM; break;
					case 3: team = MULTI_SEND_MESSAGE_YELLOW_TEAM; break;
					}

					if(team!=MULTI_SEND_MESSAGE_ALL)
						sprintf (str,"[%s]: %s",Players[Player_num].callsign,HudInputMessage);

					if (Netgame.local_role==LR_SERVER)
						MultiSendMessageFromServer (GR_RGB(0,128,255),str,team);
					else
						MultiSendMessageToServer (0,str,team);
				}break;
			}
		}
	}
	HudInputMessage[0]=0;
	HudInputMessageLen=0;

	ddio_KeyFlush();
	ResumeControls();
	Doing_input_message=HUD_MESSAGE_NONE;
	MultiSendRequestTypeIcon(false);
}

// Breaks up the input message so that if we overflow it starts on the next line nicely
// Returns the number of characters in the next line
int BreakupHUDInputMessage (char *str)
{
	int cur=0;
	int last_space=-1;
	int len=strlen (HudInputMessage);

	for (int i=0;i<len;i++)
	{
		if (HudInputMessage[i]==' ')
			last_space=i;
	}

	if (last_space==-1)
		return 0;

	int leftover_len=len-last_space;
	int leftover_size = leftover_len-1;
	if (leftover_len>0)
	{
		int to_who;
		char *colon_pos = GetMessageDestination(HudInputMessage,&to_who);

		if(to_who!=MULTI_SEND_MESSAGE_ALL)
		{
			char to_who_text[64];
			int to_who_size;

			to_who_size = colon_pos-HudInputMessage - 1;
			strncpy(to_who_text,HudInputMessage,to_who_size);
			to_who_text[to_who_size] = '\0';

			sprintf (str,"%s:%s",to_who_text,&HudInputMessage[last_space+1]);
			leftover_size = strlen(str);
		}
		else
			strcpy (str,&HudInputMessage[last_space+1]);
	}
	
	HudInputMessage[last_space]=0;

	return leftover_size;
}

// Handles all incoming keys for an inputted hud message
void DoHUDInputMessageKey (int key)
{
	if (key==KEY_F8)
	{
		StartHUDInputMessage();
		return;
	}

	if (Doing_input_message==HUD_MESSAGE_NONE)
		return;

//	ddio_KeyFlush();			// took this out. screwed up shift-keys.   don't need it anyway.

	if (key==KEY_ESC)
	{
		Doing_input_message=HUD_MESSAGE_NONE;
		MultiSendRequestTypeIcon(false);
		Marker_message=0;
		HudInputMessage[0]=0;
		HudInputMessageLen=0;
		ResumeControls();
	}
	else if (key==KEY_BACKSP || key==KEY_DELETE)
	{
		if (HudInputMessageLen>0)
			HudInputMessageLen--;

		HudInputMessage[HudInputMessageLen]=0;
	}
	else if (key==KEY_ENTER)
	{
		if (Marker_message)
		{
			DropMarker(HudInputMessage);
			Doing_input_message=HUD_MESSAGE_NONE;
			MultiSendRequestTypeIcon(false);
			Marker_message=0;
			HudInputMessage[0]=0;
			HudInputMessageLen=0;
			ResumeControls();
		}
		else
			SendOffHUDInputMessage();
	}
	else
	{
		int ascii=ddio_KeyToAscii (key);
		if (isdigit(ascii) || isalpha(ascii) || ispunct(ascii) || isspace (ascii))
		{
			if (Marker_message)
			{	
				if (HudInputMessageLen>=(MAX_MARKER_MESSAGE_LENGTH-2))
					return;
			}

			HudInputMessage[HudInputMessageLen++]=ascii;
			HudInputMessage[HudInputMessageLen]=0;

			if (!Marker_message && HudInputMessageLen>=(MAX_HUD_INPUT_LEN-1))
			{
				// Break up our outgoing message onto the next line if need be
				char str[MAX_HUD_INPUT_LEN];
				int str_len;
				int old_style = Doing_input_message;

 				str_len=BreakupHUDInputMessage (str);
				SendOffHUDInputMessage ();

				switch(old_style)
				{
				case HUD_MESSAGE_TEAM:
					StartTeamHUDInputMessage();
					break;
				case HUD_MESSAGE_GENERAL:
					StartHUDInputMessage();
					break;
				}

				if (str_len>0)
				{
					strcpy (HudInputMessage,str);
					HudInputMessageLen=str_len;
				}
			}
		}
	}
}

// Simply renders the hud input message if there is one
void RenderHUDInputMessage ()
{
	char message[255];
	int y,x,lw;

	ddgr_color text_color=HUD_COLOR;
	grtext_SetColor(text_color);

	if(Doing_input_message==HUD_MESSAGE_TEAM){
		sprintf (message,TXT_HUD_TEAMSAY,HudInputMessage);
	}
	else
	{
		if (Marker_message)
			sprintf (message,TXT_HUD_MARKER,HudInputMessage);
		else
			sprintf (message,TXT_MESSAGE,HudInputMessage);
	}

	grtext_SetAlpha(HUD_ALPHA);
	grtext_SetFlags(0);

//	RenderHUDTextNoFormatFlags(HUDTEXT_CENTERED, HUD_COLOR, HUD_ALPHA, 1, 0, y, message);
	lw = grtext_GetTextLineWidth(message);
	x = (Game_window_w-lw)/2;
	if (Small_hud_flag) 
	{
		y = Max_window_h - (int)(40*((float)Max_window_h/(float)DEFAULT_HUD_HEIGHT));
		HUD_inmsg_dirty_rect.set(x,y,x+lw,y+grfont_GetHeight(HUD_FONT));
	}
	else
		y = (Game_window_h*3/5)+Game_window_y;

	grtext_Puts(x, y, message);	
}

int testcolor=200;


float Hud_messages_paused_saved_Gametime;

void HUDPauseMessages(void)
{
	if(Hud_messages_paused)
		return;

	Hud_messages_paused = true;
	Hud_messages_paused_saved_Gametime = Gametime;
}

void HUDUnpauseMessages(void)
{
	if(!Hud_messages_paused)
		return;

	Hud_messages_paused = false;
	float time_paused = Gametime - Hud_messages_paused_saved_Gametime;

	Hud_timer += time_paused;
}


// Renders all the messages we have in the message list.  If HUD_MESSAGE_TIME has
// elapsed, then punt the oldest message and move the others up one
void RenderScrollingHUDMessages ()
{
	if(Hud_messages_paused)
		return;

	int shade;
	int text_height;
	short l,t,r,b;
	int i;

	// Check for wraps
	if (Gametime<Hud_timer)
		Hud_timer=Gametime;

	grtext_SetFont(HUD_FONT);
	text_height=grfont_GetHeight(HUD_FONT);

	text_height+= (3*Hud_aspect_y);		// add this for a spacer

	if (Doing_input_message>HUD_MESSAGE_NONE)
		RenderHUDInputMessage ();

	if (Num_hud_messages==0)
		return;
		
	l = (Small_hud_flag) ? (Max_window_w/2) : (Game_window_w/2);
	r = l;
	t = 0;
	b = 0;
	for (i=0;i<Num_hud_messages;i++)
	{
		int x;

		if (HUD_message_type[i]==HUD_MESSAGE_BLINKING)
		{
			float blink_time=Gametime;
			int iblink=blink_time;
			float frac_blink=blink_time-iblink;

			float answer=frac_blink/.5;

			iblink=answer;

			if (iblink==1 || iblink==3)
				continue;
		}

		if (i==0 && Hud_scroll_offset!=0)
		{
			int scolor=text_height+Hud_scroll_offset;
			shade=(255*scolor)/text_height;
		}
		else
			shade=255;

		char *message=HUD_messages[i];
		int text_width=grtext_GetTextLineWidth(message);
		int vp_width=Game_window_w;
		int y=(i*text_height)+Hud_scroll_offset;
	
		if (HUD_message_type[i]==HUD_MESSAGE_BLINKING)
		{
			ddgr_color text_color;
			text_color=(GR_RGB(shade,0,0));
			grtext_SetColor(text_color);
		}
		else
		{
			ddgr_color text_color;
			text_color=HUD_message_color[i];
			grtext_SetColor(text_color);
		}

		grtext_SetAlpha(HUD_ALPHA);
		grtext_SetFlags(0);

		if (Small_hud_flag) 
		{
			x = (Max_window_w-text_width)/2;
			if (x < l) l = x;
			if ((x+text_width) > r) r = x+text_width;
			b += text_height;
		}
		else
			x = (Game_window_w-text_width)/2;
		
		grtext_Puts(x, y, message);
	}
	if (Small_hud_flag && (i>0)) 
		HUD_msg_dirty_rect.set(l, t,r, b);

	// Now see if the topmost message is old

	float new_time=Gametime;

	if ((new_time-Hud_timer)>HUD_MESSAGE_TIME)
	{
		for (int i=1;i<MAX_HUD_MESSAGES;i++)
		{
			strcpy (HUD_messages[i-1],HUD_messages[i]);
			HUD_message_type[i-1]=HUD_message_type[i];
			HUD_message_color[i-1]=HUD_message_color[i];
		}

		Num_hud_messages--;
		Hud_scroll_offset=0;
		Hud_timer=new_time;

	}
	else if ((new_time-Hud_timer)>(HUD_MESSAGE_TIME-HUD_SCROLL_TIME))
	{
		// scroll

		float cur_scroll_time=HUD_MESSAGE_TIME-HUD_SCROLL_TIME;		
		float cur_hud_time=(new_time-Hud_timer)-cur_scroll_time;	
		float fscroll;

		cur_hud_time/=HUD_SCROLL_TIME;
		// cur_hud_time is now normalized from 0 to 1 
		// 0 represents not scrolled, 1 represents a total scroll

		ASSERT (cur_hud_time>=0 && cur_hud_time<=1);

		fscroll=text_height*cur_hud_time;

		Hud_scroll_offset=-fscroll;
	}
}

#define FADEOUT_TIME		0.5f		//fades out over this amount of time
#define CHAR_DELAY		0.03f		//delay between character

// Renders all the messages we have in the message list.  If HUD_MESSAGE_TIME has
// elapsed, then punt the oldest message and move the others up one
void RenderHUDMessages ()
{
	if(Hud_messages_paused)
		return;

	//Render & update the scrolling message list
	RenderScrollingHUDMessages();

	// do persistent hud message system
	if (Hud_persistent_msg_id != HUD_INVALID_ID) 
	{
		if (Hud_persistent_msg_timer != HUD_MSG_PERSISTENT_INFINITE) 
		{
			tHUDItem *item = GetHUDItem(Hud_persistent_msg_id);

			//Update time
			Hud_persistent_msg_timer -= Frametime;

			//Do fadeout
			if (Hud_persistent_msg_flags & HPF_FADEOUT) 
			{
				if (Hud_persistent_msg_timer < FADEOUT_TIME) 
					item->alpha = HUD_ALPHA * Hud_persistent_msg_timer / FADEOUT_TIME;
			}

			if (item && Small_hud_flag) 
			{
				grtext_SetFont(HUD_FONT);
				short l = item->x*Max_window_w/DEFAULT_HUD_WIDTH;
				short t = item->y*Max_window_h/DEFAULT_HUD_HEIGHT;
				short r = l + grtext_GetTextLineWidth(item->data.text)+10;
				short b = t + grtext_GetTextHeight(item->data.text);
				item->dirty_rect.set(l,t,r,b);
			}

			//Do FreeSpace printing effect
			if (Hud_persistent_msg_id2 != HUD_INVALID_ID) 
			{
				Hud_persistent_msg_char_timer -= Frametime;
				while (Hud_persistent_msg_char_timer < 0) 
				{
					tHUDItem *item = GetHUDItem(Hud_persistent_msg_id);
					tHUDItem *item2 = GetHUDItem(Hud_persistent_msg_id2);

					item->data.text[Hud_persistent_msg_current_len] = item2->data.text[0];
					Hud_persistent_msg_current_len++;

					//Check for more chars
					if (! item->data.text[Hud_persistent_msg_current_len]) 
					{
						FreeHUDItem(Hud_persistent_msg_id2);
						Hud_persistent_msg_id2 = HUD_INVALID_ID;
						Sound_system.StopSoundLooping(Hud_persistent_msg_sound_handle);
						Hud_persistent_msg_sound_handle = SOUND_NONE_INDEX;
						break;	//stop printing chars
					}
					else 
					{
						//dp next char
						item2->data.text[0] = item->data.text[Hud_persistent_msg_current_len];
						item->data.text[Hud_persistent_msg_current_len] = 0;
						item2->x = item->x + RenderHUDGetTextLineWidth(item->data.text);
						Hud_persistent_msg_char_timer += CHAR_DELAY;
					}
				}
			}

			//If time out, clear message
			if (Hud_persistent_msg_timer <= 0.0f) 
			{
				void ClearPersistentHUDMessage();
				ClearPersistentHUDMessage();
			}
		}
	}
}


void RenderHUDMsgDirtyRects()
{
// clean hud message dirty rects
	HUD_msg_dirty_rect.fill(GR_BLACK);

// clean input message dirty rects
	HUD_inmsg_dirty_rect.fill(GR_BLACK);

// clean remnant persistant messages
	HUD_persist_dirty_rect[0].fill(GR_BLACK);
	HUD_persist_dirty_rect[1].fill(GR_BLACK);
}


// reset hud messages.
void ResetHUDMessages()
{
	Hud_timer = 0.0f;
	Num_hud_messages = 0;
	Hud_scroll_offset = 0;
	ResetPersistentHUDMessage();
	HUDUnpauseMessages();

	HUD_inmsg_dirty_rect.reset();
	HUD_msg_dirty_rect.reset();
	HUD_persist_dirty_rect[0].reset();
	HUD_persist_dirty_rect[1].reset();

	void ResetHUDLevelItems();
	ResetHUDLevelItems();
}

struct phud_message
{
	char		message[HUD_MESSAGE_LENGTH*2];
	ddgr_color	color;
	int			x,y;
	float		time;
	int			flags;
	int			sound_index;
};

#define PHUD_QUEUE_SIZE	3

phud_message PHUD_message_queue[PHUD_QUEUE_SIZE];

int Num_queued_PHUD_messages = 0;

//adds a message to the persistant message queue
void QueuePersistentHUDMessage(ddgr_color color,int x, int y, float time, int flags, int sound_index, char *message)
{
	if (Num_queued_PHUD_messages == PHUD_QUEUE_SIZE)
		return;

	phud_message *pp = &PHUD_message_queue[Num_queued_PHUD_messages++];

	pp->color = color;
	pp->x = x;
	pp->y = y;
	pp->time = time;
	pp->flags = flags;
	pp->sound_index = sound_index;
	strcpy(pp->message,message);
}

//Start playing a persistent HUD message
void StartPersistentHUDMessage(ddgr_color color,int x, int y, float time, int flags, int sound_index, char *message)
{
	if(Demo_flags==DF_RECORDING)
	{
		DemoWritePersistantHUDMessage(color,x,y,time,flags,sound_index,message);
	}

// set x and y if special formatting.
	grtext_SetFont(HUD_FONT);
	if (x == HUD_MSG_PERSISTENT_CENTER)
		x = Game_window_x + (Game_window_w - grtext_GetLineWidth(message))/2;
	if (y == HUD_MSG_PERSISTENT_CENTER)
		y = Game_window_y + (Game_window_h - grfont_GetHeight(HUD_FONT))/2;

// add custom text item
	tHUDItem huditem;

	memset(&huditem,0, sizeof(huditem));

	huditem.alpha = HUD_ALPHA;
	huditem.color = color;
	huditem.type = HUD_ITEM_CUSTOMTEXT;
	huditem.x = x*DEFAULT_HUD_WIDTH/Max_window_w;
	huditem.y = y*DEFAULT_HUD_HEIGHT/Max_window_h;
	huditem.data.text = message;
	huditem.stat = STAT_MESSAGES;
	huditem.flags = HUD_FLAG_PERSISTANT | HUD_FLAG_SMALL;	// when hud is shrunk, this will draw outside of normal view

	AddHUDItem(&huditem);

	Hud_persistent_msg_id = huditem.id;
	Hud_persistent_msg_timer = time;
	Hud_persistent_msg_flags = flags;

	//Set up for FreeSpace-style effect
	if (flags & HPF_FREESPACE_DRAW) 
	{
		//Create additional hud item for leading character
		huditem.alpha = 255;
		huditem.data.text = "a";		//single-character string
		huditem.saturation_count = 2;
		AddHUDItem(&huditem);
		Hud_persistent_msg_id2 = huditem.id;

		tHUDItem *item = GetHUDItem(Hud_persistent_msg_id);
		tHUDItem *item2 = GetHUDItem(Hud_persistent_msg_id2);

		item2->data.text[0] = item->data.text[0];
		item->data.text[0] = 0;

		Hud_persistent_msg_current_len = 0;
		Hud_persistent_msg_char_timer = CHAR_DELAY;

		if (sound_index != SOUND_NONE_INDEX)
			Hud_persistent_msg_sound_handle = Sound_system.Play2dSound(sound_index);
	}
}

//	adds a persistent hud message that is timed, or infinite until removed
// for infinite, time = -1.0f
// for centering on an axis, set either x or y to -1.
void AddPersistentHUDMessage(ddgr_color color,int x, int y, float time, int flags, int sound_index, const char *fmt, ...)
{
	va_list args;
	char temp_message[HUD_MESSAGE_LENGTH*2];

// start new message
	va_start(args, fmt );
	Pvsprintf(temp_message,HUD_MESSAGE_LENGTH*2,fmt,args);
	va_end(args);

	if (Hud_persistent_msg_id != HUD_INVALID_ID)		//already one active, so queue the new one
		QueuePersistentHUDMessage(color,x,y,time,flags,sound_index,temp_message);
	else
		StartPersistentHUDMessage(color,x,y,time,flags,sound_index,temp_message);
}

//Clears the current message, and plays the next in the queue
void ClearPersistentHUDMessage()
{
// free persistent message currently visible
	if (Hud_persistent_msg_id != HUD_INVALID_ID) 
	{
		tHUDItem *item = GetHUDItem(Hud_persistent_msg_id);
		if (item) 
		{
			tDirtyRect*r = &item->dirty_rect;
			HUD_persist_dirty_rect[0].set(r->r[0].l, r->r[0].t, r->r[0].r, r->r[0].b);
		}
		FreeHUDItem(Hud_persistent_msg_id);
		Hud_persistent_msg_id = HUD_INVALID_ID;
	}
	if (Hud_persistent_msg_id2 != HUD_INVALID_ID) 
	{
		tHUDItem *item = GetHUDItem(Hud_persistent_msg_id2);
		if (item) 
		{
			tDirtyRect*r = &item->dirty_rect;
			HUD_persist_dirty_rect[1].set(r->r[0].l, r->r[0].t, r->r[0].r, r->r[0].b);
		}
		FreeHUDItem(Hud_persistent_msg_id2);
		Hud_persistent_msg_id2 = HUD_INVALID_ID;
	}

	if (Hud_persistent_msg_sound_handle != SOUND_NONE_INDEX) 
	{
		Sound_system.StopSoundLooping(Hud_persistent_msg_sound_handle);
		Hud_persistent_msg_sound_handle = SOUND_NONE_INDEX;
	}

	//Check for queued message
	if (Num_queued_PHUD_messages) 
	{
		phud_message *pp = &PHUD_message_queue[0];
		StartPersistentHUDMessage(pp->color,pp->x,pp->y,pp->time,pp->flags,pp->sound_index,pp->message);
		*pp = PHUD_message_queue[1];
		Num_queued_PHUD_messages--;
	}
}

//Clears all the persistent hud messages
void ResetPersistentHUDMessage()
{
	Num_queued_PHUD_messages = 0;
	ClearPersistentHUDMessage();
}


/////////////////////////////////////////////////////////////////////////
// HUD Message System
/////////////////////////////////////////////////////////////////////////


// opens a hud rollback console.
void OpenHUDMessageConsole()
{
	if (Game_msg_con_vis)
		CloseGameMessageConsole();

	HUD_msg_con_vis = HUD_msg_con.Open(TXT_HUDMSGPOPUP_TITLE, GAME_MSGCON_X, GAME_MSGCON_Y, GAME_MSGCON_W, GAME_MSGCON_H);
}


// closes hud rollback console
void CloseHUDMessageConsole()
{
	if (HUD_msg_con_vis) 
	{
		HUD_msg_con.Close();
		HUD_msg_con_vis = false;
	}
}


//	toggles hud rollback console
void ToggleHUDMessageConsole()
{
	if (HUD_msg_con_vis) CloseHUDMessageConsole();
	else OpenHUDMessageConsole();
}


/////////////////////////////////////////////////////////////////////////
// Game Message System
/////////////////////////////////////////////////////////////////////////
// resets game message list to no messages
void ResetGameMessages()
{
	Game_msg_list.reset();
	Game_msg_con.AttachMsgList(&Game_msg_list);
	HUD_msg_list.reset();
	HUD_msg_con.AttachMsgList(&HUD_msg_list);
}


void AddGameMessage(const char *msg)
{
	int secs = (int)Gametime;

	Game_msg_list.add(msg, (ubyte)Current_mission.cur_level, (secs/3600), (secs/60), secs % 60);
}


void SGSGameMessages(CFILE *fp)
{
	int i=0;
	const char *msg;
	
	cf_WriteShort(fp, (short)Game_msg_list.m_nmsg);

	while ((msg =Game_msg_list.get(i++)) != NULL) 
	{
		cf_WriteString(fp, msg);
	}
}


void LGSGameMessages(CFILE *fp)
{
	int n;
	char buf[512];								// some level designers can be nuts.

	n = (int)cf_ReadShort(fp);

	Game_msg_list.reset();
	while(n)
	{
		n--;
		cf_ReadString(buf, sizeof(buf), fp);
		buf[511]=0;
		Game_msg_list.add(buf);
	}
}


// toggles game message console
void ToggleGameMessageConsole()
{
	if (Game_msg_con_vis) CloseGameMessageConsole();
	else OpenGameMessageConsole();
}


// game message console
void OpenGameMessageConsole()
{
	if (HUD_msg_con_vis)
		CloseHUDMessageConsole();
	
	Game_msg_con_vis = Game_msg_con.Open(TXT_HUD_GAMEMESSAGES, GAME_MSGCON_X, GAME_MSGCON_Y, GAME_MSGCON_W, GAME_MSGCON_H);
}


void CloseGameMessageConsole()
{
	Game_msg_con.Close();
	Game_msg_con_vis = false;
}


//////////////////////////////////////////////////////////////////////////////

#define MSGL_BORDER_THICKNESS		4

tMsgList::tMsgList()
{
	m_nmsg = 0;
	m_limit = 64;
	m_msg = NULL;
}


bool tMsgList::add(const char *msg, ubyte lvl, ubyte hr, ubyte min, ubyte sec)
{
	char buf[2048];

	if(!m_limit)
		m_limit = 64;
		
	if (m_msg == NULL)
	{
		m_msg = (char **)mem_malloc(sizeof(char*)*m_limit);
		m_nmsg = 0;
	}
	
	if (m_nmsg == m_limit && m_nmsg > 0) 
	{
		mem_free(m_msg[0]);
		for (int i = 1; i < m_nmsg; i++)
			m_msg[i-1] = m_msg[i];
		m_nmsg--;
	}

	if (lvl > 0)
	{
		sprintf(buf, "[%d.%d.%d.%d] %s", lvl,hr,min,sec,msg);
		m_msg[m_nmsg++] = mem_strdup(buf);
	}
	else
		m_msg[m_nmsg++] = mem_strdup(msg);

	return true;
}


void tMsgList::reset()
{
	if (m_msg) 
	{
		while (m_nmsg)
		{
			m_nmsg--;
			if (m_msg[m_nmsg])
			{
				mem_free(m_msg[m_nmsg]);
			}
		}
		mem_free(m_msg);
		m_msg = NULL;
	}
	m_nmsg = 0;
}


const char *tMsgList::get(int i)
{
	if (m_msg) 
	{
		if (i < m_nmsg) 
		{
			return m_msg[i];
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////////

MsgListConsole::MsgListConsole()
{
	m_opened = false;
	m_list = NULL;
	m_buffer = NULL;
	m_bufline = 0;
	m_conlines = NULL;
	n_conlines = 0;
}


MsgListConsole::~MsgListConsole()
{
}


// registers dimensions of box
bool MsgListConsole::Open(const char *title, int x, int y, int w, int h)
{
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;

	if (!m_list) return false;

//	determine text buffer dimensions
	m_buflen = 2048;

redo_copy:
	m_buffer = (char *)mem_malloc(m_buflen);
	if (m_buffer) 
	{
		m_buffer[0] = 0;
		m_opened = true;
	}
	else 
	{
		m_opened = false;
		return false;
	}

//	generate text buffer
	int i, len, j;
	char *bufptr = m_buffer;

	for (i = 0; i < m_list->m_nmsg; i++)
	{
		const char *msg = m_list->m_msg[i];
		if ((strlen(m_buffer)+strlen(msg)+2) >= (unsigned)(m_buflen)) 
		{
		// reallocate buffer
			mem_free(m_buffer);
			m_buflen = m_buflen * 2;
			goto redo_copy;
		}

		textaux_WordWrap(msg, bufptr, m_w - MSGL_BORDER_THICKNESS*2, HUD_FONT);
		bufptr += strlen(bufptr);

	// append newline character if there are more messages.
		if (i < (m_list->m_nmsg-1)) 
		{
			*bufptr = '\n';
			bufptr++;
		}
	}

// allocate console line pointers
	int c = 0;
	int y2 = grfont_GetHeight(HUD_FONT) + MSGL_BORDER_THICKNESS + 3;
	while (y2 < (m_h-MSGL_BORDER_THICKNESS-grfont_GetHeight(HUD_FONT)))
	{
		y2 += (grfont_GetHeight(HUD_FONT) + 1);
		c++;
	}

	ASSERT(c > 0);
	m_conlines = (char **)mem_malloc(sizeof(char*)*c);
	n_conlines = c;
	memset(m_conlines, 0, sizeof(char*)*c);

// get pointer to first visible line.
	c = 0;
	m_bufline = 0;
	len = strlen(m_buffer);
	bufptr = m_buffer;
	for (i = 0; i <=len; i++)
	{
		if (m_buffer[i] == '\n' || m_buffer[i] == 0) 
		{
			if (c == n_conlines) 
			{
			// scroll pointers up, store new one.
				for (j = 1; j < n_conlines; j++)
					m_conlines[j-1] = m_conlines[j];
				c--;
			}
			m_conlines[c] = bufptr;
			bufptr = &m_buffer[i+1];
			m_bufline++;
			c++;
		}
	}
	
	m_numlines = m_bufline;
	m_keydowntime = 0.0f;

	strcpy(m_title, title);
	m_curmsgs = m_list->m_nmsg;

	SuspendControls();

	return true;
}


void MsgListConsole::Close()
{
	if (m_opened) 
	{
		if (m_conlines) mem_free(m_conlines);
		if (m_buffer) mem_free(m_buffer);
		m_opened = false;
		ResumeControls();
	}
}


void MsgListConsole::AttachMsgList(tMsgList *msglist)
{
	m_list = msglist;
	m_bufline = 0;
}


void MsgListConsole::Draw()
{
	g3Point *pntlist[4],points[4];
	int y, i;

	if (!m_opened || !m_list) return;

// change in buffer, do 'slow' update 
	if (m_curmsgs != m_list->m_nmsg) 
	{
		char buf[32];
		strcpy(buf,m_title);
		this->Close();
		this->Open(buf, m_x,m_y,m_w,m_h);
	}

// draw background window.
	points[0].p3_sx=m_x;
	points[0].p3_sy=m_y;
	points[1].p3_sx=m_x+m_w;
	points[1].p3_sy=m_y;
	points[2].p3_sx=m_x+m_w;
	points[2].p3_sy=m_y+m_h;
	points[3].p3_sx=m_x;
	points[3].p3_sy=m_y+m_h;

	for (i=0;i<4;i++)
	{
		points[i].p3_z=0;
		points[i].p3_flags=PF_PROJECTED;
		pntlist[i]=&points[i];
	}

	rend_SetZBufferState(0);
	rend_SetTextureType (TT_FLAT);
	rend_SetAlphaType (AT_CONSTANT);
	rend_SetLighting (LS_NONE);
	rend_SetFlatColor (GR_BLACK);
	rend_SetAlphaValue(192);
	rend_DrawPolygon2D( 0, pntlist, 4 );

// set up text rendering.
	grtext_Reset();
	grtext_SetParameters(m_x+MSGL_BORDER_THICKNESS,m_y+MSGL_BORDER_THICKNESS-1,m_x+m_w - MSGL_BORDER_THICKNESS,m_y+m_h-MSGL_BORDER_THICKNESS);
	grtext_SetFont(HUD_FONT);
	grtext_SetColor(GR_RGB(0,255,0));
	grtext_Puts(m_x+MSGL_BORDER_THICKNESS+4, m_y+MSGL_BORDER_THICKNESS-1, m_title);

	y = m_y + grfont_GetHeight(HUD_FONT) + MSGL_BORDER_THICKNESS + 1;
	rend_SetFlatColor(GR_RGB(0,255,0));
	rend_DrawLine(m_x+MSGL_BORDER_THICKNESS,y,m_x+m_w-MSGL_BORDER_THICKNESS,y);

//	draw all lines until bottom
	y += 2;
	for (i = 0; i < n_conlines; i++)
	{
		char *sptr = NULL;
		if (i < (n_conlines-1))
			sptr = m_conlines[i+1];
		if (sptr)
			sptr[-1] = 0;
		grtext_Puts(m_x+MSGL_BORDER_THICKNESS,y,m_conlines[i]);
		if (sptr)
			sptr[-1] = '\n';
		y += (grfont_GetHeight(HUD_FONT) + 1);
		if (!sptr)
			break;
	}

	grtext_Flush();
}


void MsgListConsole::DoInput()
{
	if (m_opened) 
	{
		int i, offset_count = 0; // = ddio_KeyDownCount(KEY_DOWN) - ddio_KeyDownCount(KEY_UP);
		float key_time = ddio_KeyDownTime(KEY_DOWN) - ddio_KeyDownTime(KEY_UP);
		char *lineptr;

		if (m_numlines <= n_conlines)
			offset_count = 0;							// no need to offset if no need to scroll

		if (key_time == 0.0f)
			m_keydowntime = 0.0f;
		else
			m_keydowntime += key_time;
		if (m_keydowntime < -0.1f) 
		{
			offset_count = -1;
			m_keydowntime = 0.0f;
		}
		else if (m_keydowntime > 0.1f) 
		{
			offset_count = 1;
			m_keydowntime = 0.0f;
		}

	//	scroll offset_count amount.
		if (offset_count < 0) 
		{
		//	look back from m_conlines[0]
			if ((m_bufline + offset_count - n_conlines) < 0) offset_count = -(m_bufline-n_conlines);

			m_bufline = m_bufline + offset_count;

			if (offset_count < 0)
			{
				mprintf((0, "bufline=%d\n", m_bufline));
			}

			while (offset_count < 0)
			{
				offset_count++;
				for (i = n_conlines-1; i > 0; i--)
					m_conlines[i] = m_conlines[i-1];

			// go back until hit previous newline of beginning of buffer.
				for (lineptr = m_conlines[0]-1; lineptr != m_buffer && lineptr[-1] != '\n'; lineptr--);
				m_conlines[0] = lineptr;
			}
		}
		else if (offset_count > 0) 
		{
		// look forward from m_conlines[n_conlines-1]
			if ((m_bufline + offset_count) > m_numlines) offset_count = m_numlines - m_bufline;

	 		m_bufline = m_bufline + offset_count;

			if (offset_count > 0) 
			{
				mprintf((0, "bufline=%d\n", m_bufline));
			}

			while (offset_count > 0)
			{
				offset_count--;
				for (i = 1; i  < n_conlines; i++)
					m_conlines[i-1] = m_conlines[i];

			//	go forward until we hit a newline or end of buffer (if end, then error!)
				for (lineptr = m_conlines[n_conlines-1]; lineptr[0] != '\n'; lineptr++);
				{
					if (lineptr[0] == 0) 
					{
						Int3();
						offset_count = 0;
						lineptr = m_conlines[n_conlines-1];
						break;
					}
				}
				if (lineptr[0]) lineptr++;
				m_conlines[n_conlines-1] = lineptr;
			}
		}

	}
}
