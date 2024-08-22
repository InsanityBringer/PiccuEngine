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

#include "ui.h"
#include "newui.h"
#include "grdefs.h"
#include "player.h"
#include "game.h"
#include "pilot.h"
#include "module.h"
#include "ddio_common.h"

#include "inetgetfile.h"

#ifdef __STATIC_NETWORK_CLIENTS
#define DLLMultiCall         DLLMultiCall_LAN
#define DLLMultiInit         DLLMultiInit_LAN
#define DLLMultiClose        DLLMultiClose_LAN
#define MainMultiplayerMenu  MainMultiplayerMenu_LAN
#define AutoLoginAndJoinGame AutoLoginAndJoinGame_LAN
#endif

#define TXT_DLL_SAVESETTINGS	TXT(27)
#define TXT_DLL_LOADSETTINGS	TXT(28)

///////////////////////////////////////////////
//localization header
#include "lanstrings.h"


#define TXT_GEN_MPLYROPTIONS	TXT_LC_MPLYROPTIONS
#define TXT_GEN_TIMELIMIT		TXT_LC_TIMELIMIT
#define TXT_GEN_KILLGOAL		TXT_LC_KILLGOAL
#define TXT_GEN_PPS				TXT_LC_PPS
#define TXT_GEN_RESPAWNRATE		TXT_LC_RESPAWNRATE
#define TXT_GEN_MAXPLAYERS		TXT_LC_MAXPLAYERS
#define TXT_GEN_PREVMENU		TXT_LC_PREVMENU
#define TXT_GEN_CANCEL			TXT_LC_CANCEL
#define TXT_GEN_CFGALLOWEDSHIP	TXT_LC_CFGALLOWEDSHIP
#define TXT_GEN_USEROTVEL		TXT_LC_USEROTVEL
#define TXT_GEN_USEROTVEL		TXT_LC_USEROTVEL
#define TXT_GEN_USESMOOTHING	TXT_LC_USESMOOTHING
#define TXT_GEN_CLIENTSERVER	TXT_LC_CLIENTSERVER
#define TXT_GEN_PEERPEER		TXT_LC_PEERPEER
#define TXT_GEN_ACC_WEAP_COLL	TXT_LC_ACC_WEAP_COLL
#define TXT_GEN_BRIGHT_PLAYERS	TXT_LC_BRIGHT_PLAYERS

#define MULTI_USE_ALL_OPTIONS	1

#include "lanclient.h"
#include "DLLUiItems.h"

using namespace lanclient;

//int DLLUIClass_CurrID = 0xD0;

#define MAX_NET_GAMES	100
#define JEFF_RED		GR_RGB(255,40,40)
#define JEFF_BLUE		GR_RGB(40,40,255)
#define JEFF_GREEN	GR_RGB(40,255,40)
#define NETPOLLINTERVAL	10.0

extern int MTAVersionCheck(unsigned int oldver, char *URL);
/////////////////////////////
// Defines


#ifdef MACINTOSH
#pragma export on
#endif

// These next two function prototypes MUST appear in the extern "C" block if called
// from a CPP file.
extern "C"
{
	DLLEXPORT void DLLFUNCCALL DLLMultiInit (int *api_func);
	DLLEXPORT void DLLFUNCCALL DLLMultiCall (int eventnum);
	DLLEXPORT void DLLFUNCCALL DLLMultiClose ();
}

static bool All_ok = true;
// Initializes the game function pointers
void DLLFUNCCALL DLLMultiInit (int *api_func)
{
	Use_netgame_flags	= 1;
	#include "mdllinit.h"
	DLLCreateStringTable("lanclient.str",&StringTable,&StringTableSize);
	DLLmprintf((0,"%d strings loaded from string table\n",StringTableSize));
	if(!StringTableSize){
		All_ok = false;
		return;
	}
	*DLLUse_DirectPlay = false;
}

// Called when the DLL is shutdown
void DLLFUNCCALL DLLMultiClose ()
{
	DLLDestroyStringTable(StringTable,StringTableSize);	
}


// The main entry point where the game calls the dll
void DLLFUNCCALL DLLMultiCall (int eventnum)
{
			
	switch(eventnum)
	{
	case MT_EVT_GET_HELP:
		strcpy(DLLHelpText1,TXT_LC_HELP1);
		strcpy(DLLHelpText2,TXT_LC_HELP2);
		strcpy(DLLHelpText3,TXT_LC_HELP3);
		strcpy(DLLHelpText4,TXT_LC_HELP4);
		break;
	case MT_AUTO_START:
		if(!All_ok)
			*DLLMultiGameStarting = 0;
		else
			*DLLMultiGameStarting = 1;
		break;
	case MT_RETURN_TO_GAME_LIST:
	case MT_EVT_LOGIN:
		if(!DLLTCP_active)
		{
			DLLDoMessageBox(TXT_LC_ERROR,TXT_LC_NO_TCPIP,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
			*DLLMultiGameStarting = 0;
			break;
		}
		if(!All_ok)
			*DLLMultiGameStarting = 0;
		else
			*DLLMultiGameStarting = MainMultiplayerMenu ();
		break;
	case MT_EVT_FRAME:
		
		break;
	case MT_EVT_FIRST_FRAME:
		
		break;
	case MT_EVT_GAME_OVER:
		break;
	case MT_AUTO_LOGIN:
		if(!DLLTCP_active)
		{
			DLLDoMessageBox(TXT_LC_ERROR,TXT_LC_NO_TCPIP,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
			*DLLMultiGameStarting = 0;
			break;
		}
		if(All_ok)
			AutoLoginAndJoinGame();
		break;
	}
}

#ifdef MACINTOSH
#pragma export off
#endif
namespace lanclient {
#define GET_INFO_ID	50
// The first multiplayer menu that the user will see...all multiplayer stuff is
// reached from this menu
// Returns true if we're starting a multiplayer game
int MainMultiplayerMenu ()
{

	char selgame[200] = "";
	void * join_LC_text  = DLLCreateNewUITextItem(TXT_LC_RETURNMAIN,GR_BLACK);//return_menu
	void * list_head_txt  = DLLCreateNewUITextItem(TXT_LC_GAMELISTHDR,UICOL_TEXT_NORMAL);
	void * exit_on_text   = DLLCreateNewUITextItem(TXT_LC_EXIT,UICOL_HOTSPOT_HI);
	void * exit_off_text  = DLLCreateNewUITextItem(TXT_LC_EXIT,UICOL_HOTSPOT_LO);
	void * join_on_text   = DLLCreateNewUITextItem(TXT_LC_JOINSEL,UICOL_HOTSPOT_HI);
	void * join_off_text  = DLLCreateNewUITextItem(TXT_LC_JOINSEL,UICOL_HOTSPOT_LO);
	void * start_on_text  = DLLCreateNewUITextItem(TXT_LC_STARTNEW,UICOL_HOTSPOT_HI);
	void * start_off_text = DLLCreateNewUITextItem(TXT_LC_STARTNEW,UICOL_HOTSPOT_LO);
	void * srch_on_text   = DLLCreateNewUITextItem(TXT_LC_SRCHADDR,UICOL_HOTSPOT_HI);
	void * srch_off_text  = DLLCreateNewUITextItem(TXT_LC_SRCHADDR,UICOL_HOTSPOT_LO);
	void * scan_on_text   = DLLCreateNewUITextItem(TXT_LC_SCANLOCAL,UICOL_HOTSPOT_HI);
	void * scan_off_text  = DLLCreateNewUITextItem(TXT_LC_SCANLOCAL,UICOL_HOTSPOT_LO);
	void * game_hdr_text  = DLLCreateNewUITextItem(TXT_LC_GAMEHEADER,UICOL_WINDOW_TITLE,DLL_BIG_BRIEFING_FONT);
		
	int exit_menu=0;
	void * net_game_txt_items[MAX_NET_GAMES];
	int a;
	for(a=0;a<MAX_NET_GAMES;a++) net_game_txt_items[a] = NULL;
	int ret=0;
	ubyte oldalpha = *DLLNewUIWindow_alpha;
	int cury = 40;

	DLLSetScreenMode(SM_MENU);
	*DLLNewUIWindow_alpha = 255;

	void * main_wnd = DLLNewUIWindowCreate(0,0,640,480,UIF_PROCESS_ALL);
	
	void * info_on_text =  DLLCreateNewUITextItem("",UICOL_HOTSPOT_HI);
	void * info_hs = DLLHotSpotCreate(main_wnd,GET_INFO_ID,KEY_I,info_on_text,info_on_text,1,1,1,1,0);

	void * screen_header = DLLTextCreate(main_wnd,game_hdr_text,45,cury,UIF_CENTER); cury+=35;
	void * start_hs = DLLHotSpotCreate(main_wnd,7,KEY_S,start_off_text,start_on_text,320,cury,150,15,UIF_CENTER);cury+=25;
	void * srch_hs = DLLHotSpotCreate(main_wnd,9,KEY_A,srch_off_text,srch_on_text,320,cury,250,15,UIF_CENTER);cury+=25;
	void * edit_box = DLLEditCreate(main_wnd,9 ,10,cury,300,15,UIF_CENTER); cury+=35;
	void * scan_hs = DLLHotSpotCreate(main_wnd,8,KEY_L,scan_off_text,scan_on_text,320,cury,200,15,UIF_CENTER);cury+=50;
	void * list_header = DLLTextCreate(main_wnd,list_head_txt,45,cury,0); cury+=13;
	void * main_list = DLLListCreate(main_wnd,UID_OK,10,cury,600,170,UIF_CENTER|UILB_NOSORT);cury+=200;
	void * join_hs = DLLHotSpotCreate(main_wnd,UID_OK,KEY_ENTER,join_off_text,join_on_text,100,cury,130,15,0);
	void * exit_hs = DLLHotSpotCreate(main_wnd,UID_CANCEL,KEY_ESC,exit_off_text,exit_on_text,400,cury,70,15,0);
	char szdip[30] = "";
	int diplen = 29;
	DLLDatabaseRead("DirectIP",szdip,&diplen);
	DLLEditSetText(edit_box,szdip);	
	
	DLLNewUIWindowLoadBackgroundImage(main_wnd,"multimain.ogf");
	DLLNewUIWindowOpen(main_wnd);
	*DLLNum_network_games_known = 0;
	int lastgamesfound = 0;
	int itemp;
	int looklocal = 1;
	void * selti = NULL;
	
	float lastpoll = DLLtimer_GetTime();
	DLLSearchForLocalGamesTCP(0xffffffffl,htons(DEFAULT_GAME_PORT));
	// Menu loop
	while (!exit_menu) 
	{
		int res;
		int selno;
		DLLUpdateAndPackGameList();

		if(((itemp = DLLSearchForLocalGamesTCP(0,0))!=0) || (*DLLMulti_Gamelist_changed) )
		{	
			//if(itemp != lastgamesfound)
			if(*DLLMulti_Gamelist_changed)
			{
				//We found a new game!
				*DLLMulti_Gamelist_changed = false;
				selti = NULL;
				lastgamesfound = itemp;
				//Get the currently selected item
				//char * psel = DLLListGetItem(main_list,DLLListGetSelectedIndex(main_list))				;
				//selno = DLLListGetSelectedIndex(main_list);
				//strcpy(selgame,DLLNetwork_games[selno].name);
				DLLmprintf((0,"Selected item = %s\n",selgame));
				selti = NULL;
				DLLListRemoveAll(main_list);
				for(int k=0;k<*DLLNum_network_games_known;k++)
				{
					char fmtline[200];
					char server_mode[20];
					
					if(DLLNetwork_games[k].flags & NF_PEER_PEER)
					{
						strcpy(server_mode,"PP");
					}
					else if(DLLNetwork_games[k].flags & NF_PERMISSABLE)
					{
						strcpy(server_mode,"PS");
					}
					else
					{
						strcpy(server_mode,"CS");
					}
					
					if(DLLNetwork_games[k].flags & NF_ALLOW_MLOOK)
					{
						strcat(server_mode,"-ML");
					}

					DLLmprintf ((0,"Found game: %s\n",DLLNetwork_games[k].name));
					sprintf(fmtline,"%.20s\t\x02\x02b%s %.10s\x02\x45%.15s\x02\x63%d\x02\x6d%d/%d\x02\x7e%.3f",DLLNetwork_games[k].name,server_mode,DLLNetwork_games[k].scriptname,DLLNetwork_games[k].mission_name,
						DLLNetwork_games[k].level_num,DLLNetwork_games[k].curr_num_players,DLLNetwork_games[k].max_num_players,
						DLLNetwork_games[k].server_response_time);
		
					if(DLLNetwork_games[k].dedicated_server)
					{
						net_game_txt_items[k] = DLLCreateNewUITextItem(fmtline,GR_WHITE);
					}
					else
					{
						net_game_txt_items[k] = DLLCreateNewUITextItem(fmtline,GR_LIGHTGRAY);
					}

					selgame[20] = NULL;
					if(strncmp(selgame,DLLNetwork_games[k].name,19)==0)
					{
						selti = net_game_txt_items[k];
						DLLmprintf((0,"Found previously selected game in list, reselecting...\n"));

					}
					DLLListAddItem(main_list,net_game_txt_items[k]);
				}
				if(selti)
				{
					DLLListSelectItem(main_list,selti);
				}
			}
		}
		if((looklocal)&&((DLLtimer_GetTime() - lastpoll)>NETPOLLINTERVAL))
		{
			DLLSearchForLocalGamesTCP(0xffffffffl,htons(DEFAULT_GAME_PORT));
			//*DLLNum_network_games_known = 0;
			//DLLListRemoveAll(main_list);
			lastgamesfound = 0;
			lastpoll = DLLtimer_GetTime();
			selno = DLLListGetSelectedIndex(main_list);
			if(selno>=0)
				strcpy(selgame,DLLNetwork_games[selno].name);
			else
				selgame[0]=NULL;
		}
		res = DLLPollUI();

		if(res==-1)
		{
			continue;

		}
	// handle all UI results.
		switch(res)
		{

		case UID_CANCEL:
			DLLNewUIWindowClose(main_wnd);
			exit_menu = 1;
			break;
		case UID_OK:
			//Double click on listbox, or join selected hit.
			if(*DLLNum_network_games_known)
			{
				//Get the appropriate game address
				int gameno;
				gameno = DLLListGetSelectedIndex(main_list);
				DLLmprintf((0,"Selected item is %d\n",gameno));
				network_address s_address;
				memcpy (&s_address,&DLLNetwork_games[gameno].addr,sizeof(network_address));
				s_address.connection_type = NP_TCP;
				//s_address.port=&DLLNetwork_games[gameno].addr.port;//DEFAULT_GAME_PORT;
				*DLLGame_is_master_tracker_game = 0;
				DLLMultiStartClient (NULL);			
				if(DLLDoPlayerMouselookCheck(DLLNetwork_games[gameno].flags))
				{
					if(DLLmsn_CheckGetMission(&s_address,DLLNetwork_games[gameno].mission))
					{
						if ((DLLTryToJoinServer (&s_address)))
						{
							DLLmprintf ((0,"Menu: Game joined!\n"));
							DLLNewUIWindowClose(main_wnd);
							exit_menu=1;
							ret=1;
						}
						else
						{
							DLLNewUIWindowClose(main_wnd);
							DLLNewUIWindowOpen(main_wnd);
						}
					}
				}
			}
			else
			{
				DLLDoMessageBox(TXT_LC_ERROR,TXT_LC_NO_GAMES,MSGBOX_OK,UICOL_WINDOW_TITLE,UICOL_TEXT_NORMAL);
			}
			break;
		case 7:
			//Start a new game
			// Start a netgame
			DLLNewUIWindowClose(main_wnd);
			*DLLGame_is_master_tracker_game = 0;
			if (StartMultiplayerGameMenu())
			{
				exit_menu=1;
				ret=1;
			}
			else 
			{
				DLLNewUIWindowOpen(main_wnd);
			}
			break;
		case 8:
			//Scan for local games
			*DLLMulti_Gamelist_changed = true;
			*DLLNum_network_games_known = 0;
			looklocal = 1;
			lastgamesfound = 0;
			selno = DLLListGetSelectedIndex(main_list);
			if(selno>=0)
				strcpy(selgame,DLLNetwork_games[selno].name);
			else
				selgame[0]=NULL;

			DLLSearchForLocalGamesTCP(0xffffffffl,htons(DEFAULT_GAME_PORT));
			DLLListRemoveAll(main_list);
			for(a=0;a<MAX_NET_GAMES;a++) if(net_game_txt_items[a]) DLLRemoveUITextItem(net_game_txt_items[a]);
			for(a=0;a<MAX_NET_GAMES;a++) net_game_txt_items[a] = NULL;
			break;
		case 9:
			{
				//Scan a specific IP
				unsigned short iport = DEFAULT_GAME_PORT;
				looklocal = 0;
				unsigned int iaddr;
				lastgamesfound = 0;
				DLLEditGetText(edit_box,szdip,25);
				//Make this IP the default
				DLLDatabaseWrite("DirectIP",szdip,strlen(szdip)+1);
				
				char *pport = strchr(szdip,':');
				if(pport)
				{
					*pport = NULL;
					pport++;
					iport = atoi(pport);
				}

				iaddr = inet_addr(szdip);
				DLLmprintf((0,"Local inet_addr %x\n",iaddr));
				if(iaddr && (INADDR_NONE!=iaddr))
					DLLSearchForLocalGamesTCP(iaddr,htons(iport));
				else
					DLLmprintf((0,"Invalid IP for local search\n"));
				DLLListRemoveAll(main_list);
				for(a=0;a<MAX_NET_GAMES;a++) if(net_game_txt_items[a]) DLLRemoveUITextItem(net_game_txt_items[a]);
				for(a=0;a<MAX_NET_GAMES;a++) net_game_txt_items[a] = NULL;
			}
			break;
		case GET_INFO_ID:
			{
				if(*DLLNum_network_games_known)
				{
					int gameno;
					gameno = DLLListGetSelectedIndex(main_list);
					DLLShowNetgameInfo(&DLLNetwork_games[gameno]);
				}
			}
			break;
		}

	}
	DLLNewUIWindowDestroy(main_wnd);
	
	*DLLNewUIWindow_alpha = oldalpha;
	for(a=0;a<MAX_NET_GAMES;a++) if(net_game_txt_items[a]) DLLRemoveUITextItem(net_game_txt_items[a]);
	//Cleanup
	DLLDeleteUIItem(screen_header);
	DLLDeleteUIItem(main_wnd);
	DLLDeleteUIItem(main_list);
	DLLDeleteUIItem(list_header);
	DLLDeleteUIItem(exit_hs);
	DLLDeleteUIItem(join_hs);
	DLLDeleteUIItem(start_hs);
	DLLDeleteUIItem(srch_hs);
	DLLDeleteUIItem(scan_hs);
	DLLDeleteUIItem(edit_box);

	DLLDeleteUIItem(info_hs);	
	DLLRemoveUITextItem(info_on_text);
	DLLRemoveUITextItem(game_hdr_text);
	DLLRemoveUITextItem(join_LC_text);
	DLLRemoveUITextItem(list_head_txt);
	DLLRemoveUITextItem(exit_on_text);
	DLLRemoveUITextItem(exit_off_text);
	DLLRemoveUITextItem(join_on_text);
	DLLRemoveUITextItem(join_off_text);
	DLLRemoveUITextItem(start_on_text);
	DLLRemoveUITextItem(start_off_text);
	DLLRemoveUITextItem(srch_on_text);
	DLLRemoveUITextItem(srch_off_text);
	DLLRemoveUITextItem(scan_on_text);
	DLLRemoveUITextItem(scan_off_text);
	return ret;

}



void AutoLoginAndJoinGame(void)
{
	unsigned short port;
	unsigned long iaddr;
	
	*DLLMultiGameStarting = 0;
	
	if(!*DLLAuto_login_addr)
	{
		DLLmprintf((0,"Can't autostart because no IP address was specified!!\n"));
		return;
	}
	if(*DLLAuto_login_port)
	{
		port = atoi(DLLAuto_login_port);
	}
	else 
	{
		port = DEFAULT_GAME_PORT;
	}
		
	//Now actually connect to the server!

	network_address s_address;
	iaddr = inet_addr(DLLAuto_login_addr);
	memcpy (&s_address.address,&iaddr,sizeof(unsigned long));
	s_address.port=port;
	s_address.connection_type = NP_TCP;
	*DLLGame_is_master_tracker_game = 0;
	DLLMultiStartClient (NULL);			

	if ((DLLTryToJoinServer (&s_address)))
	{
		DLLmprintf ((0,"Menu: Game joined!\n"));
		*DLLMultiGameStarting = 1;		
	}
}

}
