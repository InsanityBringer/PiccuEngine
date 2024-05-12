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

#ifdef WIN32
#include <crtdbg.h>
#include "windows.h"
#endif
#include "ui.h"
#include "newui.h"
#include "game.h"
#include "gamefont.h"
#include "multi.h"
#include "multi_client.h"
#include "manage.h"
#include "Mission.h"
#include "pilot.h"
#include "ui.h"
#include "newui.h"
#include "pstypes.h"
#include "pserror.h"
#include "descent.h"
#include "game.h"
#include "room.h"
#include "object.h"
#include "terrain.h"
#include "player.h"
#include "mono.h"
#include "hud.h"
#include "Inventory.h"
#include "multi_server.h"
#include "multi_ui.h"
#include "ship.h"
#include "soundload.h"
#include "spew.h"
#include "DllWrappers.h"
#include "appdatabase.h"
#include "module.h"
#include "ship.h"
#include "localization.h"
#include "stringtable.h"
#include "dedicated_server.h"
#include "multi_save_settings.h"
#include "multi_dll_mgr.h"
#include "mission_download.h"
#include "module.h"
#include "mem.h"
#include "args.h"
//#define USE_DIRECTPLAY

#ifdef USE_DIRECTPLAY
#include "directplay.h"
#endif

void* callback = NULL;
module MultiDLLHandle = { NULL };
int SearchForLocalGamesTCP(unsigned int ask, ushort port);
int SearchForGamesPXO(unsigned int ask, ushort port);
int SearchForLocalGamesIPX(network_address* check_addr);
extern ubyte NewUIWindow_alpha;
extern void DoScreenshot();
extern void UpdateAndPackGameList(void);
extern bool Multi_Gamelist_changed;
int CheckMissionForScript(char* mission, char* script, int dedicated_server_num_teams);
void ShowNetgameInfo(network_game* game);
// The exported DLL function call prototypes
#if defined(__LINUX__)
typedef void DLLFUNCCALL(*DLLMultiCall_fp)(int eventnum);
typedef void DLLFUNCCALL(*DLLMultiScoreCall_fp)(int eventnum, void* data);
typedef void DLLFUNCCALL(*DLLMultiInit_fp)(int* api_fp);
typedef void DLLFUNCCALL(*DLLMultiClose_fp)();
#else
typedef void(DLLFUNCCALL* DLLMultiCall_fp)(int eventnum);
typedef void(DLLFUNCCALL* DLLMultiScoreCall_fp)(int eventnum, void* data);
typedef void(DLLFUNCCALL* DLLMultiInit_fp)(int* api_fp);
typedef void(DLLFUNCCALL* DLLMultiClose_fp)();
#endif
DLLMultiScoreCall_fp DLLMultiScoreCall = NULL;
DLLMultiCall_fp DLLMultiCall = NULL;
DLLMultiInit_fp DLLMultiInit = NULL;
DLLMultiClose_fp DLLMultiClose = NULL;
//dllmultiiInfo DLLMultiInfo;
//The DLL needs these too.
#define MAXTEXTITEMS			100
#define MAXNEWWINDOWS		5
#define MAXNEWGAMEWINDOWS	5
#define MAXUIBUTTONS			20
#define MAXUITEXTS			20
#define MAXEDITS				20
#define MAXLISTS				20
#define MAXHOTSPOTS			20
#define MAXCONSOLES			5
#define MAXPARENTS		5
NewUIMessageBox	messageb;
//Stuff for the splash
UITextItem ti_msg("", UICOL_TEXT_NORMAL);
UITextItem ti_tmp("", UICOL_TEXT_NORMAL);
UITextItem ti_cancel_on("", UICOL_HOTSPOT_HI);
UITextItem ti_cancel_off("", UICOL_HOTSPOT_LO);
UIHotspot uih;
UIHotspot uihcancel;
UIText ti;
extern char HelpText1[];
extern char HelpText2[];
extern char HelpText3[];
extern char HelpText4[];
char Auto_login_name[MAX_AUTO_LOGIN_STUFF_LEN];
char Auto_login_pass[MAX_AUTO_LOGIN_STUFF_LEN];
char Auto_login_addr[MAX_AUTO_LOGIN_STUFF_LEN];
char Auto_login_port[MAX_AUTO_LOGIN_STUFF_LEN];
char Multi_conn_dll_name[_MAX_PATH * 2] = "";
char PXO_hosted_lobby_name[100] = "global";
bool Supports_score_api = false;
#ifdef USE_DIRECTPLAY
extern modem_list	Modems_found[MAX_MODEMS];
extern int Num_modems_found;
#endif
#define DLL_BRIEFING_FONT			1
#define DLL_BIG_BRIEFING_FONT		2
extern unsigned short nw_ListenPort;
extern ushort PXOPort;


void GetMultiAPI(multi_api* api)
{
	//make the compiler happy
	bool (*fp_PlayerIsShipAllowed)(int, int) = PlayerIsShipAllowed;
	api->objs = (int*)Objects;
	api->rooms = (int*)Rooms;
	api->terrain = (int*)Terrain_seg;
	api->players = (int*)Players;
	api->netgame = (int*)&Netgame;
	api->netplayers = (int*)&NetPlayers;
	api->ships = (int*)Ships;
	// Fill in function pointers here.  The order here must match the order on the 
	// DLL side

	api->fp[0] = (int*)SetUITextItemText;
	api->fp[1] = (int*)NewUIWindowCreate;
	api->fp[2] = (int*)NewUIWindowDestroy;
	api->fp[3] = (int*)NewUIWindowOpen;
	api->fp[4] = (int*)NewUIWindowClose;
	api->fp[5] = (int*)TextCreate;
	api->fp[6] = (int*)EditCreate;
	api->fp[7] = (int*)ButtonCreate;
	api->fp[8] = (int*)ListCreate;
	api->fp[9] = (int*)ListRemoveAll;
	api->fp[10] = (int*)ListAddItem;
	api->fp[11] = (int*)ListRemoveItem;
	api->fp[12] = (int*)ListSelectItem;
	api->fp[13] = (int*)ListGetItem;
	api->fp[14] = (int*)ListGetSelectedIndex;
	api->fp[15] = (int*)EditSetText;
	api->fp[16] = (int*)EditGetText;
	api->fp[17] = (int*)DatabaseWrite;
	api->fp[18] = (int*)DatabaseRead;
	api->fp[19] = (int*)DoUI;
	api->fp[20] = (int*)DoMessageBox;
	api->fp[21] = (int*)DescentDefer;
	api->fp[22] = (int*)MonoPrintf;
	api->fp[23] = (int*)DestroyStringTable;
	api->fp[24] = (int*)CreateStringTable;
	api->fp[25] = (int*)NewUIGameWindowCreate;
	api->fp[26] = (int*)NewUIGameWindowDestroy;
	api->fp[27] = (int*)NewUIGameWindowOpen;
	api->fp[28] = (int*)NewUIGameWindowClose;
	api->fp[29] = (int*)SetScreenMode;
	api->fp[30] = (int*)timer_GetTime;
	api->fp[31] = (int*)TryToJoinServer;
	api->fp[32] = (int*)MultiStartClient;
	api->fp[33] = (int*)rend_GetRenderState;
	api->fp[34] = (int*)LoadMission;
	api->fp[35] = (int*)ddio_MakePath;
	api->fp[36] = (int*)ddio_FindFileStart;
	api->fp[37] = (int*)ddio_FindFileClose;
	api->fp[38] = (int*)ddio_FindNextFile;
	api->fp[39] = (int*)MultiStartServer;
	api->fp[40] = (int*)ShowProgressScreen;
	api->fp[41] = (int*)SearchForLocalGamesTCP;
	api->fp[42] = (int*)nw_GetHostAddressFromNumbers;
	api->fp[43] = (int*)NULL;//nw_GetProtocolType;
	api->fp[44] = (int*)HotSpotCreate;
	api->fp[45] = (int*)PollUI;
	api->fp[46] = (int*)GetMissionName;
	api->fp[47] = (int*)RemoveUITextItem;
	api->fp[48] = (int*)CreateNewUITextItem;
	api->fp[49] = (int*)mem_malloc_sub;
	api->fp[50] = (int*)mem_free_sub;
	api->fp[51] = (int*)CreateSplashScreen;
	api->fp[52] = (int*)CloseSplashScreen;
	api->fp[53] = (int*)UIConsoleGadgetCreate;
	api->fp[54] = (int*)UIConsoleGadgetputs;
	api->fp[55] = (int*)NewUIWindowSetFocusOnEditGadget;
	api->fp[56] = (int*)OldEditCreate;
	api->fp[57] = (int*)OldListCreate;
	api->fp[58] = (int*)OldListRemoveAll;
	api->fp[59] = (int*)OldListAddItem;
	api->fp[60] = (int*)OldListRemoveItem;
	api->fp[61] = (int*)OldListSelectItem;
	api->fp[62] = (int*)OldListGetItem;
	api->fp[63] = (int*)OldListGetSelectedIndex;
	api->fp[64] = (int*)OldEditSetText;
	api->fp[65] = (int*)OldEditGetText;
	api->fp[66] = (int*)ToggleUICallback;
	api->fp[67] = (int*)SearchForGamesPXO;
	api->fp[68] = (int*)SetOldEditBufferLen;
	api->fp[69] = (int*)NewUIWindowLoadBackgroundImage;
	api->fp[70] = (int*)DeleteUIItem;
	api->fp[71] = (int*)SearchForLocalGamesIPX;
	api->fp[72] = (int*)HotSpotSetStates;
	api->fp[73] = (int*)PlayerSetShipPermission;
	api->fp[74] = (int*)fp_PlayerIsShipAllowed;
#ifdef USE_DIRECTPLAY
	api->fp[75] = (int*)dp_InitDirectPlay;
	api->fp[76] = (int*)dp_ListDirectPlayGames;
	api->fp[77] = (int*)dp_GetModemChoices;
#else
	api->fp[75] = (int*)NULL;
	api->fp[76] = (int*)NULL;
	api->fp[77] = (int*)NULL;
#endif
	api->fp[78] = (int*)DatabaseReadInt;
	api->fp[79] = (int*)DatabaseWriteInt;
	api->fp[80] = (int*)DoScreenshot;
	api->fp[81] = (int*)IsMissionMultiPlayable;
	api->fp[82] = (int*)grtext_GetTextLineWidth;
	api->fp[83] = (int*)GadgetDestroy;
#ifdef USE_DIRECTPLAY
	api->fp[84] = (int*)dp_StartGame;
	api->fp[85] = (int*)dp_EndGame;
#else
	api->fp[84] = (int*)NULL;
	api->fp[85] = (int*)NULL;
#endif
	api->fp[86] = (int*)nw_Asyncgethostbyname;
	api->fp[87] = (int*)PrintDedicatedMessage;
	api->fp[88] = (int*)nw_ReccomendPPS;
	api->fp[89] = (int*)DoMultiAllowed;
	api->fp[90] = (int*)MultiDoConfigSave;
	api->fp[91] = (int*)MultiDoConfigLoad;
	api->fp[92] = (int*)MultiLoadSettings;
	api->fp[93] = (int*)nw_DoReceiveCallbacks;
	api->fp[94] = (int*)nw_SendWithID;
	api->fp[95] = (int*)nw_UnRegisterCallback;
	api->fp[96] = (int*)nw_RegisterCallback;
	api->fp[97] = (int*)msn_CheckGetMission;
	api->fp[98] = (int*)MultiGameOptionsMenu;
	api->fp[99] = (int*)mod_FreeModule;
	api->fp[100] = (int*)mod_GetLastError;
	api->fp[101] = (int*)mod_GetSymbol;
	api->fp[102] = (int*)mod_LoadModule;
	api->fp[103] = (int*)dCurrentPilotName;
	api->fp[104] = (int*)UpdateAndPackGameList;
	api->fp[105] = (int*)MultiLevelSelection;
	api->fp[106] = (int*)DoPlayerMouselookCheck;
	api->fp[107] = (int*)CheckMissionForScript;
	api->fp[108] = (int*)ShowNetgameInfo;
	api->fp[109] = (int*)GetRankIndex;
	api->fp[110] = (int*)CheckGetD3M;

	// Variable pointers
	api->vp[0] = (int*)&Player_num;
	api->vp[1] = (int*)Tracker_id;
	api->vp[2] = (int*)&Game_is_master_tracker_game;
	api->vp[3] = (int*)&Game_mode;
	api->vp[4] = (int*)NULL;//Current_pilot; no longer a struct
	api->vp[5] = (int*)Base_directory; //[ISB] aa, should this be working dir or user dir? Probably working. 
	api->vp[6] = (int*)&MultiDLLGameStarting;
	api->vp[7] = (int*)MTPilotinfo;
	api->vp[8] = (int*)&Num_network_games_known;
	api->vp[9] = (int*)&Network_games;
	api->vp[10] = (int*)&NewUIWindow_alpha;
	api->vp[11] = (int*)HelpText1;
	api->vp[12] = (int*)HelpText2;
	api->vp[13] = (int*)HelpText3;
	api->vp[14] = (int*)HelpText4;
	api->vp[15] = (int*)Auto_login_name;
	api->vp[16] = (int*)Auto_login_pass;
	api->vp[17] = (int*)Auto_login_addr;
	api->vp[18] = (int*)Auto_login_port;
#ifdef USE_DIRECTPLAY
	api->vp[19] = (int*)&Num_directplay_games;
	api->vp[20] = (int*)Directplay_sessions;
	api->vp[21] = (int*)&DP_active;
	api->vp[22] = (int*)&Use_DirectPlay;
	api->vp[23] = (int*)&Modems_found;
	api->vp[24] = (int*)&Num_modems_found;
#else
	static bool _dummyUse_DirectPlay = false;
	api->vp[19] = (int*)NULL;
	api->vp[20] = (int*)NULL;
	api->vp[21] = (int*)NULL;
	api->vp[22] = (int*)&_dummyUse_DirectPlay;
	api->vp[23] = (int*)NULL;
	api->vp[24] = (int*)NULL;
#endif
	api->vp[25] = (int*)&Dedicated_server;
	api->vp[26] = (int*)&TCP_active;
	api->vp[27] = (int*)&IPX_active;
	api->vp[28] = (int*)nw_ListenPort;
	api->vp[29] = (int*)&Multi_Gamelist_changed;
	api->vp[30] = (int*)PXO_hosted_lobby_name;
	api->vp[31] = (int*)&Supports_score_api;
	api->vp[32] = (int*)PXOPort;
	//Jeff: Linux dies if you try to free a DLL/so on 
	//atexit, these should be freed during game sequencing
	//anyway.
	//atexit(FreeMultiDLL);

}
// Frees the dll if its in memory
void FreeMultiDLL()
{
	if (!MultiDLLHandle.handle)
		return;
	if (DLLMultiClose)
		DLLMultiClose();
	mod_FreeModule(&MultiDLLHandle);
	//Try deleting the file now!
	if (!ddio_DeleteFile(Multi_conn_dll_name))
	{
		mprintf((0, "Couldn't delete the tmp dll"));
	}
	DLLMultiCall = NULL;
	DLLMultiInit = NULL;
	DLLMultiClose = NULL;
}
// Loads the Multi dll.  Returns 1 on success, else 0 on failure
int LoadMultiDLL(char* name)
{
	static int first = 1;
	char lib_name[_MAX_PATH * 2];
	char dll_name[_MAX_PATH * 2];
	char tmp_dll_name[_MAX_PATH * 2];
	char dll_path_name[_MAX_PATH * 2];
	MultiFlushAllIncomingBuffers();

	//Delete old dlls
	if (MultiDLLHandle.handle)
		FreeMultiDLL();

	ddio_MakePath(dll_path_name, Base_directory, "online", "*.tmp", NULL);
	ddio_DeleteFile(dll_path_name);
	//Make the hog filename
	ddio_MakePath(lib_name, Base_directory, "online", name, NULL);
	strcat(lib_name, ".d3c");
	//Make the dll filename
#if defined (WIN32)
	sprintf(dll_name, "%s.dll", name);
#else
	sprintf(dll_name, "%s.so", name);
#endif

	//Open the hog file
	if (!cf_OpenLibrary(lib_name))
	{
		ddio_MakePath(tmp_dll_name, Base_directory, "online", name, NULL);
		strcat(tmp_dll_name, ".d3c");
		Multi_conn_dll_name[0] = NULL;
		goto loaddll;
	}
	//get a temp file name
	if (!ddio_GetTempFileName(Descent3_temp_directory, "d3c", tmp_dll_name))
	{
		return 0;
	}
	//Copy the DLL
//	ddio_MakePath(dll_path_name,Base_directory,"online",tmp_dll_name,NULL);
	if (!cf_CopyFile(tmp_dll_name, dll_name))
	{
		mprintf((0, "DLL copy failed!\n"));
		return 0;
	}
	strcpy(Multi_conn_dll_name, tmp_dll_name);
loaddll:

	if (!mod_LoadModule(&MultiDLLHandle, tmp_dll_name))
	{
		int err = mod_GetLastError();
		mprintf((0, "You are missing the DLL %s!\n", name));
		return 0;
	}

	DLLMultiInit = (DLLMultiInit_fp)mod_GetSymbol(&MultiDLLHandle, "DLLMultiInit", 4);
	if (!DLLMultiInit)
	{
		int err = mod_GetLastError();
		mprintf((0, "Couldn't get a handle to the dll function DLLMultiInit!\n"));
		Int3();
		FreeMultiDLL();
		return 0;
	}
	DLLMultiCall = (DLLMultiCall_fp)mod_GetSymbol(&MultiDLLHandle, "DLLMultiCall", 4);
	if (!DLLMultiCall)
	{
		int err = mod_GetLastError();
		mprintf((0, "Couldn't get a handle to the dll function DLLMultiCall!\n"));
		Int3();
		FreeMultiDLL();
		return 0;
	}
	DLLMultiClose = (DLLMultiClose_fp)mod_GetSymbol(&MultiDLLHandle, "DLLMultiClose", 0);
	if (!DLLMultiClose)
	{
		int err = mod_GetLastError();
		mprintf((0, "Couldn't get a handle to the dll function DLLMultiClose!\n"));
		Int3();
		FreeMultiDLL();
		return 0;
	}

	if (first)
	{
		//Jeff: Linux dies if you try to free a DLL/so during atexit
		//for some reason.  The dll/so should be freed during
		//game sequencing anyway.
		//atexit (FreeMultiDLL);
		first = 0;
	}
	void* api_fp;
	api_fp = (void*)GetMultiAPI;

	if (Auto_login_name[0])
	{
		Database->write("TrackerLogin", Auto_login_name, strlen(Auto_login_name) + 1);
	}
	if (Auto_login_pass[0])
	{
		Database->write("TrackerPassword", Auto_login_pass, strlen(Auto_login_pass) + 1);
	}
	if (!Dedicated_server)
	{
		strcpy(PXO_hosted_lobby_name, "global");
	}
	DLLMultiInit((int*)api_fp);

	if (Supports_score_api)
	{
		DLLMultiScoreCall = (DLLMultiScoreCall_fp)mod_GetSymbol(&MultiDLLHandle, "DLLMultiScoreEvent", 8);

		if (!DLLMultiScoreCall)
		{
			int err = mod_GetLastError();
			mprintf((0, "Couldn't get a handle to the dll function DLLMultiScoreCall!\n"));
			Int3();
			Supports_score_api = false;
		}
	}
	return 1;
}
// The chokepoint function to call the dll function
void CallMultiDLL(int eventnum)
{
	if (MultiDLLHandle.handle && DLLMultiCall)
		DLLMultiCall(eventnum);
}
void SetUITextItemText(UITextItem* uit, char* newtext, unsigned int color)
{
	//This function is currently broken!
	strcpy(uit->m_Text, newtext);
	uit->set_color(color);
}
void* NewUIWindowCreate(int x, int y, int w, int h, int flags)
{
	NewUIWindow* newwin;
	newwin = new NewUIWindow;
	newwin->Create(x, y, w, h, flags);
	return newwin;
}
void NewUIWindowDestroy(NewUIWindow* deswin)
{
	deswin->Destroy();
}
void NewUIWindowOpen(NewUIWindow* deswin)
{
	deswin->Open();
}
void NewUIWindowClose(NewUIWindow* deswin)
{
	deswin->Close();
}
void* TextCreate(UIWindow* parentwin, UITextItem* textitem, int x, int y, int flags)
{
	UIText* newtext;
	newtext = new UIText;
	newtext->Create(parentwin, textitem, x, y, flags);
	return newtext;
}
void TextSetTitle(UIText* text, UITextItem* textitem)
{
	text->SetTitle(textitem);
}
void* EditCreate(UIWindow* parentwin, int id, int x, int y, int w, int h, int flags)
{
	NewUIEdit* newedit;
	newedit = new NewUIEdit;
	newedit->Create(parentwin, id, x, y, w, h, flags);
	return newedit;
}
void EditSetText(NewUIEdit* item, char* newtext)
{
	item->SetText(newtext);
}
void EditGetText(NewUIEdit* item, char* buff, int len)
{
	item->GetText(buff, len);
}
void* ButtonCreate(UIWindow* parentwin, int id, UITextItem* titleitem, int x, int y, int w, int h, int flags)
{
	UIButton* newbutt;
	newbutt = new UIButton;
	newbutt->Create(parentwin, id, titleitem, x, y, w, h, flags);
	return newbutt;
}
void* ListCreate(UIWindow* parentwin, int id, int x, int y, int w, int h, int flags)
{
	NewUIListBox* newlist;
	newlist = new NewUIListBox;
	newlist->Create(parentwin, id, x, y, w, h, flags);
	newlist->SetSelectedColor(UICOL_LISTBOX_HI);
	newlist->SetHiliteColor(UICOL_LISTBOX_HI);
	return newlist;
}
void ListRemoveAll(UIListBox* item)
{
	item->RemoveAll();
}
void ListAddItem(UIListBox* item, UITextItem* uitext)
{
	item->AddItem(uitext);
}
void ListRemoveItem(UIListBox* item, UITextItem* txtitem)
{
	item->RemoveItem(txtitem);
}
void ListSelectItem(UIListBox* item, UITextItem* txtitem)
{
	item->SelectItem(txtitem);
}
char* ListGetItem(UIListBox* item, int index)
{
	UITextItem* ui_item;
	ui_item = (UITextItem*)item->GetItem(index);
	if (ui_item) return (char*)ui_item->GetBuffer();
	else
	{
		mprintf((0, "No listbox item found for index %d\n", index));
		return "";
	}
}
int ListGetSelectedIndex(UIListBox* item)
{
	return item->GetSelectedIndex();
}
void ListSetSelectedIndex(UIListBox* item, int index)
{
	item->SetSelectedIndex(index);
}
void DatabaseRead(const char* label, char* entry, int* entrylen)
{
	Database->read(label, entry, entrylen);
}
void DatabaseWrite(const char* label, const char* entry, int entrylen)
{
	Database->write(label, entry, entrylen);
}
void DatabaseReadInt(const char* label, int* val)
{
	Database->read_int(label, val);
	mprintf((0, "Read int: %s:%d\n", label, *val));
}
void DatabaseWriteInt(const char* label, int val)
{
	mprintf((0, "Writing int: %s:%d\n", label, val));
	Database->write(label, val);
}
void DescentDefer(void)
{
	Descent->defer();
}
void* NewUIGameWindowCreate(int x, int y, int w, int h, int flags)
{
	NewUIGameWindow* newgamewin;
	newgamewin = new NewUIGameWindow;
	newgamewin->Create(x, y, w, h, flags);
	return newgamewin;
}
void NewUIGameWindowDestroy(NewUIGameWindow* item)
{
	item->Destroy();
}
void NewUIGameWindowOpen(NewUIGameWindow* item)
{
	item->Open();
}
void NewUIGameWindowClose(NewUIGameWindow* item)
{
	item->Close();
}
void* HotSpotCreate(UIWindow* parentwin, int id, int key, UIItem* txtitemoff, UIItem* txtitemon, int x, int y, int w, int h, int flags)
{
	UIHotspot* newhs;
	newhs = new UIHotspot;
	if (newhs)
		newhs->Create(parentwin, id, key, txtitemoff, txtitemon, x, y, w, h, flags);
	return newhs;
}
void HotSpotSetStates(UIHotspot* hs, UIItem* texton, UIItem* textoff)
{
	hs->SetStates(textoff, texton);
}
void* CheckBoxCreate(UIWindow* parent, int id, UIItem* title, int x, int y, int w, int h, int flags)
{
	UICheckBox* newcb;
	newcb = new UICheckBox;
	if (newcb)
		newcb->Create(parent, id, title, x, y, w, h, flags);
	return newcb;
}
void CheckBoxSetCheck(UICheckBox* cb, bool state)
{
	cb->SetCheck(state);
}
bool CheckBoxIsChecked(UICheckBox* cb)
{
	return cb->IsChecked();
}
float UI_LastPoll = 0l;
#define MIN_FRAMETIME	.033
int PollUI(void)
{
	//	this should poll UI_frame_result.
	int result = -1;

	ui_ShowCursor();

	//Limit this to a fixed framerate
	if (UI_LastPoll)
	{
		if ((timer_GetTime() - UI_LastPoll) < MIN_FRAMETIME)
			Sleep((timer_GetTime() - UI_LastPoll) * 1000);
	}

	UI_LastPoll = timer_GetTime();

	Descent->defer();
	DoUIFrame();
	rend_Flip();
	result = GetUIFrameResult();
	if (result != -1)
	{
		ui_HideCursor();
		ui_Flush();
		ddio_KeyFlush();
	}
	return result;
}
void* CreateNewUITextItem(char* newtext, unsigned int color, int font)
{
	UITextItem* new_text_item;
	if (font == -1)
	{
		new_text_item = new UITextItem(newtext);
	}
	else
	{
		switch (font)
		{
		case DLL_BRIEFING_FONT:
			new_text_item = new UITextItem(BRIEFING_FONT, newtext);
			break;
		case DLL_BIG_BRIEFING_FONT:
			new_text_item = new UITextItem(BIG_BRIEFING_FONT, newtext);
			break;
		default:
			new_text_item = new UITextItem(BRIEFING_FONT, newtext);
		}
	}
	if (new_text_item)
		new_text_item->set_color(color);
	return new_text_item;
}
void RemoveUITextItem(void* item)
{
	UITextItem* old_text_item = (UITextItem*)item;
	delete old_text_item;
}
void* CreateNewUIBmpItem(int handle, ubyte alpha)
{
	UIBitmapItem* new_bmp_item;
	new_bmp_item = new UIBitmapItem(handle, alpha);
	return new_bmp_item;
}
int GetUIItemWidth(void* item)
{
	UIItem* i = (UIItem*)item;
	return i->width();
}
int GetUIItemHeight(void* item)
{
	UIItem* i = (UIItem*)item;
	return i->height();
}
void RemoveUIBmpItem(void* item)
{
	UIBitmapItem* old_bmp_item = (UIBitmapItem*)item;
	delete old_bmp_item;
}
void NewUIMessageBoxCreate(int x, int y, int w, int flags)
{
	messageb.Create(x, y, w, flags);
}
void NewUIMessageBoxDestroy()
{

}
void NewUIMessageBoxOpen()
{
	messageb.Open();
}
void NewUIMessageBoxClose()
{

}
static bool splash_screen_up = false;
void CreateSplashScreen(char* msg, int usecancel)
{
	if (splash_screen_up)
		return;
	splash_screen_up = true;
	ti_cancel_on = UITextItem(TXT_CANCEL, UICOL_HOTSPOT_HI);
	ti_cancel_off = UITextItem(TXT_CANCEL, UICOL_HOTSPOT_LO);
	ti_msg = UITextItem(msg);
	messageb.Create(0, 0, 384, UIF_CENTER);
	ti.Create(&messageb, &ti_msg, 20, 50, UIF_CENTER);

	if (usecancel)
	{
		uihcancel.Create(&messageb, 99, KEY_ESC, &ti_cancel_off, &ti_cancel_on, 1, 75, 100, 15, UIF_CENTER);
	}
	messageb.Open();
}
void CloseSplashScreen(void)
{
	if (splash_screen_up)
	{
		messageb.Close();
		messageb.Destroy();
		splash_screen_up = false;
	}
	else
		return;
}
void* UIConsoleGadgetCreate(UIWindow* parentid, int id, int x, int y, int font, int cols, int rows, int flags)
{
	UIConsoleGadget* newconsole;
	newconsole = new UIConsoleGadget;
	newconsole->Create(parentid, id, x, y, BRIEFING_FONT, cols, rows, flags);
	return newconsole;
}
void UIConsoleGadgetputs(UIConsoleGadget* item, const char* str)
{
	unsigned char r = 0;
	unsigned char g = 230;
	unsigned char b = 0;
	if (*str == 1)
	{
		str++;
		r = *str;
		str++;
		g = *str;
		str++;
		b = *str;
		str++;
	}
	item->puts(GR_RGB(r, g, b), str);
}
void NewUIWindowSetFocusOnEditGadget(UIEdit* item, UIWindow* parent)
{
	item->Activate();
}
void* OldListCreate(UIWindow* parentitem, int id, int x, int y, int w, int h, int flags)
{
	UIListBox* newoldlist;
	newoldlist = new UIListBox;
	newoldlist->Create(parentitem, id, x, y, w, h, flags);
	newoldlist->SetSelectedColor(UICOL_LISTBOX_HI);
	newoldlist->SetHiliteColor(UICOL_LISTBOX_HI);
	return newoldlist;
}
void OldListRemoveAll(UIListBox* item)
{
	item->RemoveAll();
}
void OldListAddItem(UIListBox* item, UITextItem* uitext)
{
	item->AddItem(uitext);
}
void OldListRemoveItem(UIListBox* item, UITextItem* txtitem)
{
	item->RemoveItem(txtitem);
}
void OldListSelectItem(UIListBox* item, UITextItem* txtitem)
{
	item->SelectItem(txtitem);
}
char* OldListGetItem(UIListBox* item, int index)
{
	UITextItem* ti = (UITextItem*)item->GetItem(index);
	if (ti)
		return (char*)ti->GetBuffer();
	else
	{
		mprintf((0, "No listbox item found for index %d\n", index));
		return "";
	}
}
int OldListGetSelectedIndex(UIListBox* item)
{
	return item->GetSelectedIndex();
}
void* OldEditCreate(UIWindow* parentitem, int id, int x, int y, int w, int h, int flags)
{
	UIEdit* newoldedit;
	newoldedit = new UIEdit;
	newoldedit->Create(parentitem, id, x, y, w, h, flags);
	return newoldedit;
}
void OldEditSetText(UIEdit* item, char* newtext)
{
	item->SetText(newtext);
}
void OldEditGetText(UIEdit* item, char* buff, int len)
{
	item->GetText(buff, len);
}
static void (*DLLInit_old_ui_callback)() = NULL;
void ToggleUICallback(int state)
{

	if (state == 0)
	{
		if (DLLInit_old_ui_callback == NULL)
		{
			DLLInit_old_ui_callback = GetUICallback();
			SetUICallback(NULL);
		}
	}
	else
	{
		if (DLLInit_old_ui_callback != NULL)
		{
			SetUICallback(DLLInit_old_ui_callback);
			DLLInit_old_ui_callback = NULL;
		}
	}
}
void SetOldEditBufferLen(UIEdit* item, int len)
{
	item->SetBufferLen(len);
}
void NewUIWindowLoadBackgroundImage(NewUIWindow* item, const char* image_name)
{
	item->LoadBackgroundImage(image_name);
}
void DeleteUIItem(void* delitem)
{

	delete delitem;
}
void GadgetDestroy(UIGadget* item)
{
	item->Destroy();
}
void* SliderCreate(UIWindow* parent, int id, int x, int y, int flags)
{
	NewUISlider* slid;
	slid = new NewUISlider;
	slid->Create(parent, id, x, y, flags);
	return (void*)slid;
}
void SliderSetRange(UISlider* slider, int range)
{
	slider->SetRange(range);
}
int SliderGetRange(UISlider* slider)
{
	return slider->GetRange();
}
void SliderSetPos(UISlider* slider, int pos)
{
	slider->SetPos(pos);
}
int SliderGetPos(UISlider* slider)
{
	return slider->GetPos();
}
void SliderSetSelectChangeCallback(UISlider* slider, void (*fn)(int))
{
	slider->SetSelectChangeCallback(fn);
}
void SliderSetSelectChangeCallbackWData(UISlider* slider, void (*fn)(int, void*), void* ptr)
{
	slider->SetSelectChangeCallback(fn, ptr);
}
void GetUIItemPosition(UIObject* item, int* x, int* y, int* w, int* h)
{
	if (x)
		*x = item->X();
	if (y)
		*y = item->Y();
	if (w)
		*w = item->W();
	if (h)
		*h = item->H();
}
#define CDT_EVT_PLAYERKILLED			1
#define CDT_EVT_CONNECT					2
#define CDT_EVT_GAME_OVER				3
#define CDT_EVT_GAME_STARTING			4
#define CDT_EVT_FRAME					5
#define CDT_EVT_PLAYER_DIED			6
#define CDT_EVT_PLAYER_JOINS			7
#define CDT_EVT_PLAYER_LEFT			8
#define CDT_EVT_SET_SCORE_IP			9
#define CDT_EVT_SET_GAMEID				10
#define CALLSIGN_LEN	19
typedef struct player_killed
{
	char killer_name[CALLSIGN_LEN + 1];
	char killed_name[CALLSIGN_LEN + 1];
}player_killed;
typedef struct player_score_matrix
{
	int num_players;
	char name[MAX_NET_PLAYERS][CALLSIGN_LEN + 1];
	short deaths[MAX_NET_PLAYERS];
	short kills[MAX_NET_PLAYERS];
}player_score_matrix;
// The chokepoint function to call the dll function
void CallMultiScoreDLL(int eventnum, void* data)
{
	if (MultiDLLHandle.handle && DLLMultiCall)
		DLLMultiScoreCall(eventnum, data);
}
//Determine if the Score API functions should run
bool ScoreAPIShouldBail()
{
	if (!(Game_mode & GM_MULTI))
		return true;
	if (Supports_score_api)
		return false;
	return true;
}
void ScoreAPIPlayerKilled(int killer, int killed)
{
	if (ScoreAPIShouldBail())
		return;
	player_killed pk;
	strcpy(pk.killed_name, Players[killed].callsign);
	strcpy(pk.killer_name, Players[killer].callsign);

	CallMultiScoreDLL(CDT_EVT_PLAYERKILLED, &pk);
}
void ScoreAPIPlayerEntered(int playernum)
{
	if (ScoreAPIShouldBail())
		return;
	CallMultiScoreDLL(CDT_EVT_CONNECT, Players[playernum].callsign);
}
void ScoreAPIPlayerLeft(int playernum)
{
	if (ScoreAPIShouldBail())
		return;
	CallMultiScoreDLL(CDT_EVT_PLAYER_LEFT, Players[playernum].callsign);
}
void ScoreAPIPlayerJoin(int playernum)
{
	if (ScoreAPIShouldBail())
		return;
	CallMultiScoreDLL(CDT_EVT_PLAYER_JOINS, Players[playernum].callsign);
}
void ScoreAPIGameOver()
{
	player_score_matrix sm;

	if (ScoreAPIShouldBail())
		return;



	int counted = 0;
	for (int i = 0; i < MAX_NET_PLAYERS; i++)
	{
		if (Players[i].callsign[0])
		{
			strcpy(sm.name[counted], Players[i].callsign);
			sm.kills[counted] = Players[i].num_kills_level;
			sm.deaths[counted] = Players[i].num_deaths_level;
			counted++;
		}
	}
	sm.num_players = counted;
	//Create a matrix of all the players with their scores and send a pointer to it.
	CallMultiScoreDLL(CDT_EVT_GAME_OVER, &sm);
}
bool Set_score_server_ip = false;
void ScoreAPIFrameInterval()
{
	if (ScoreAPIShouldBail())
		return;
	if (!Set_score_server_ip)
	{
		Set_score_server_ip = true;
		//Look for a score server on the command line..
		int ss_arg = FindArg("-scoreserver");
		if (ss_arg)
		{
			CallMultiScoreDLL(CDT_EVT_SET_SCORE_IP, GameArgs[ss_arg + 1]);
		}
		int gi_arg = FindArg("-gameid");
		if (gi_arg)
		{
			CallMultiScoreDLL(CDT_EVT_SET_GAMEID, GameArgs[gi_arg + 1]);
		}
	}
	CallMultiScoreDLL(CDT_EVT_FRAME, NULL);
}
