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

	DLLGetMultiAPI=(GetMultiAPI_fp)api_func;
	DLLGetMultiAPI (&API);
	DLLSetUITextItemText = (SetUITextItemText_fp)API.fp[0];
	DLLNewUIWindowCreate = (NewUIWindowCreate_fp)API.fp[1];
	DLLNewUIWindowDestroy = (NewUIWindowDestroy_fp)API.fp[2];
	DLLNewUIWindowOpen = (NewUIWindowOpen_fp)API.fp[3];
	DLLNewUIWindowClose = (NewUIWindowClose_fp)API.fp[4];
	DLLTextCreate = (TextCreate_fp)API.fp[5];
	DLLEditCreate = (EditCreate_fp)API.fp[6];
	DLLButtonCreate = (ButtonCreate_fp)API.fp[7];
	DLLListCreate = (ListCreate_fp)API.fp[8];
	DLLListRemoveAll = (ListRemoveAll_fp)API.fp[9];
	DLLListAddItem = (ListAddItem_fp)API.fp[10];
	DLLListRemoveItem = (ListRemoveItem_fp)API.fp[11];
	DLLListSelectItem = (ListSelectItem_fp)API.fp[12];
	DLLListGetItem = (ListGetItem_fp)API.fp[13];
	DLLListGetSelectedIndex = (ListGetSelectedIndex_fp)API.fp[14];
	DLLEditSetText = (EditSetText_fp)API.fp[15];
	DLLEditGetText = (EditGetText_fp)API.fp[16];
	DLLDatabaseWrite = (DatabaseWrite_fp)API.fp[17];
	DLLDatabaseRead = (DatabaseRead_fp)API.fp[18];
	DLLDoUI = (DoUI_fp)API.fp[19];
	DLLDoMessageBoxFP = (DoMessageBox_fp)API.fp[20];
	DLLDescentDefer = (DescentDefer_fp)API.fp[21];
	DLLDebug_ConsolePrintf=(Debug_ConsolePrintf_fp)API.fp[22];
	DLLDestroyStringTable = (DestroyStringTable_fp)API.fp[23];
	DLLCreateStringTable = (CreateStringTable_fp)API.fp[24];
	DLLNewUIGameWindowCreate = (NewUIGameWindowCreate_fp)API.fp[25];
	DLLNewUIGameWindowDestroy = (NewUIGameWindowDestroy_fp)API.fp[26];
	DLLNewUIGameWindowOpen = (NewUIGameWindowOpen_fp)API.fp[27];
	DLLNewUIGameWindowClose = (NewUIGameWindowClose_fp)API.fp[28];
	DLLSetScreenMode = (SetScreenMode_fp)API.fp[29];
	DLLtimer_GetTime = (timer_GetTime_fp)API.fp[30];
	DLLTryToJoinServer = (TryToJoinServer_fp)API.fp[31];
	DLLMultiStartClient = (MultiStartClient_fp)API.fp[32];
	DLLrend_GetRenderState = (rend_GetRenderState_fp)API.fp[33];
	DLLLoadMission = (LoadMission_fp)API.fp[34];
	DLLddio_MakePath = (ddio_MakePath_fp)API.fp[35];
	DLLddio_FindFileStart = (ddio_FindFileStart_fp)API.fp[36];
	DLLddio_FindFileClose = (ddio_FindFileClose_fp)API.fp[37];
	DLLddio_FindNextFile = (ddio_FindNextFile_fp)API.fp[38];
	DLLMultiStartServerFP = (MultiStartServer_fp)API.fp[39];
	DLLShowProgressScreen = (ShowProgressScreen_fp)API.fp[40];
	DLLSearchForLocalGamesTCP = (SearchForLocalGamesTCP_fp)API.fp[41];
	DLLnw_GetHostAddressFromNumbers = (nw_GetHostAddressFromNumbers_fp)API.fp[42];
	DLLnw_GetProtocolType = (nw_GetProtocolType_fp)API.fp[43];
	DLLHotSpotCreate = (HotSpotCreate_fp )API.fp[44];
	DLLPollUI = (PollUI_fp)API.fp[45];
	DLLGetMissionName = (GetMissionName_fp)API.fp[46];
	DLLRemoveUITextItem = (RemoveUITextItem_fp)API.fp[47];
	DLLCreateNewUITextItemFP = (CreateNewUITextItem_fp)API.fp[48];
	DLLmem_malloc = (mem_malloc_fp)API.fp[49];
	DLLmem_free =(mem_free_fp)API.fp[50];
	DLLCreateSplashScreen = (CreateSplashScreen_fp)API.fp[51];
	DLLCloseSplashScreen = (CloseSplashScreen_fp)API.fp[52];
	DLLUIConsoleGadgetCreate = (UIConsoleGadgetCreate_fp)API.fp[53];
	DLLUIConsoleGadgetputs = (UIConsoleGadgetputs_fp)API.fp[54];
	DLLNewUIWindowSetFocusOnEditGadget = (NewUIWindowSetFocusOnEditGadget_fp)API.fp[55];
	DLLOldEditCreate = (OldEditCreate_fp)API.fp[56];
	DLLOldListCreate = (OldListCreate_fp)API.fp[57];
	DLLOldListRemoveAll = (OldListRemoveAll_fp)API.fp[58];
	DLLOldListAddItem = (OldListAddItem_fp)API.fp[59];
	DLLOldListRemoveItem = (OldListRemoveItem_fp)API.fp[60];
	DLLOldListSelectItem = (OldListSelectItem_fp)API.fp[61];
	DLLOldListGetItem = (OldListGetItem_fp)API.fp[62];
	DLLOldListGetSelectedIndex = (OldListGetSelectedIndex_fp)API.fp[63];
	DLLOldEditSetText = (OldEditSetText_fp)API.fp[64];
	DLLOldEditGetText = (OldEditGetText_fp)API.fp[65];
	DLLToggleUICallback = (ToggleUICallback_fp)API.fp[66];
	DLLSearchForGamesPXO = (SearchForGamesPXO_fp)API.fp[67];
	DLLSetOldEditBufferLen = (SetOldEditBufferLen_fp)API.fp[68];
	DLLNewUIWindowLoadBackgroundImage = (NewUIWindowLoadBackgroundImage_fp)API.fp[69];
	DLLDeleteUIItem = (DeleteUIItem_fp)API.fp[70];
	DLLSearchForLocalGamesIPX = (SearchForLocalGamesIPX_fp)API.fp[71];
	DLLHotSpotSetStates = (HotSpotSetStates_fp)API.fp[72];
	DLLPlayerSetShipPermission = (PlayerSetShipPermission_fp)API.fp[73];
	DLLPlayerIsShipAllowed = (PlayerIsShipAllowed_fp)API.fp[74];
#ifdef WIN32
	DLLdp_InitDirectPlay = (dp_InitDirectPlay_fp)API.fp[75];
	DLLdp_ListDirectPlayGames = (dp_ListDirectPlayGames_fp)API.fp[76];
	DLLdp_GetModemChoices = (dp_GetModemChoices_fp)API.fp[77];
#endif
	DLLDatabaseReadInt = (DatabaseReadInt_fp)API.fp[78];
	DLLDatabaseWriteInt = (DatabaseWriteInt_fp)API.fp[79];
	DLLDoScreenshot = (DoScreenshot_fp)API.fp[80];
	DLLIsMissionMultiPlayable = (IsMissionMultiPlayable_fp)API.fp[81];
	DLLgrtext_GetTextLineWidth = (grtext_GetTextLineWidth_fp)API.fp[82];
	DLLGadgetDestroy = (GadgetDestroy_fp)API.fp[83];
#ifdef WIN32
	DLLdp_StartGame = (dp_StartGame_fp)API.fp[84];
	DLLdp_EndGame = (dp_EndGame_fp)API.fp[85];
#endif
	DLLnw_Asyncgethostbyname = (nw_Asyncgethostbyname_fp)API.fp[86];
	DLLPrintDedicatedMessage = (DedicatedServerPrintf_fp)API.fp[87];
	DLLnw_ReccomendPPS = (nw_ReccomendPPS_fp)API.fp[88];
	DLLDoMultiAllowed = (DoMultiAllowed_fp)API.fp[89];
	DLLMultiDoConfigSave = (MultiDoConfigSave_fp)API.fp[90];
	DLLMultiDoConfigLoad = (MultiDoConfigLoad_fp)API.fp[91];
	DLLMultiLoadSettings = (MultiLoadSettings_fp)API.fp[92];
	DLLnw_DoReceiveCallbacks = (nw_DoReceiveCallbacks_fp)API.fp[93];
	DLLnw_SendWithID = (nw_SendWithID_fp)API.fp[94];
	DLLnw_UnRegisterCallback = (nw_UnRegisterCallback_fp)API.fp[95];
	DLLnw_RegisterCallback = (nw_RegisterCallback_fp)API.fp[96];
	DLLmsn_CheckGetMission = (msn_CheckGetMission_fp)API.fp[97];
	DLLMultiGameOptionsMenu = (MultiGameOptionsMenu_fp)API.fp[98];
	DLLmod_FreeModule = (mod_FreeModule_fp)API.fp[99];
	DLLmod_GetLastError = (mod_GetLastError_fp)API.fp[100];
	DLLmod_GetSymbol = (mod_GetSymbol_fp)API.fp[101];
	DLLmod_LoadModule = (mod_LoadModule_fp)API.fp[102];
	CurrentPilotName = (CurrentPilotName_fp)API.fp[103];
	DLLUpdateAndPackGameList = (UpdateAndPackGameList_fp)API.fp[104];
	DLLMultiLevelSelection = (MultiLevelSelection_fp)API.fp[105];
	DLLDoPlayerMouselookCheck = (DoPlayerMouselookCheck_fp)API.fp[106];
	DLLCheckMissionForScript = (CheckMissionForScript_fp)API.fp[107];
	DLLShowNetgameInfo = (ShowNetgameInfo_fp)API.fp[108];
	//API.fp[109]; // Not used
	DLLCheckGetD3M = (CheckGetD3M_fp)API.fp[110];
	DLLddio_GetTempFileName = (ddio_GetTempFileName_fp)API.fp[111];
	DLLSearchForTrackedGame = (SearchForTrackedGame_fp)API.fp[112];

	DLLMPlayers = (player *)API.players;
	DLLNetgame = (netgame_info *)API.netgame;
	DLLMNetPlayers=(netplayer *)API.netplayers;
	DLLShips = (ship *)API.ships;

	DLLPlayer_num = *API.vp[0];
	DLLTracker_id = (char *)API.vp[1];
	DLLGame_is_master_tracker_game = API.vp[2];
	DLLGame_mode = *API.vp[3];
	//DLLCurrent_pilot = (pilot *)API.vp[4];
	DLLLocalD3Dir = (char *)(pilot *)API.vp[5];
	DLLMultiGameStarting = (int *)API.vp[6];
	DLLMTPilotinfo = (vmt_descent3_struct *)API.vp[7];
	DLLNum_network_games_known = API.vp[8];
	DLLNetwork_games = (network_game *)API.vp[9];
	DLLNewUIWindow_alpha = (ubyte *)API.vp[10];
	DLLHelpText1 = (char *)API.vp[11];
	DLLHelpText2 = (char *)API.vp[12];
	DLLHelpText3 = (char *)API.vp[13];
	DLLHelpText4 = (char *)API.vp[14];
	DLLAuto_login_name = (char *)API.vp[15];
	DLLAuto_login_pass = (char *)API.vp[16];
	DLLAuto_login_addr = (char *)API.vp[17];
	DLLAuto_login_port = (char *)API.vp[18];
	#ifdef WIN32
	DLLNum_directplay_games = (int *)API.vp[19];
	DLLDirectplay_sessions = (LPDPSESSIONDESC2)API.vp[20];
	DLLDP_active = (bool *)API.vp[21];
	DLLUse_DirectPlay = (bool *)API.vp[22]; 
	DLLModems_found = (modem_list *)API.vp[23]; 
	DLLNum_modems_found = (int *)API.vp[24];
	#else
	DLLUse_DirectPlay = (bool *)API.vp[22];
	#endif
	DLLDedicated_server = (bool *)API.vp[25];
	DLLTCP_active = (BOOL)*API.vp[26];
	DLLIPX_active = (BOOL)*API.vp[27];
        DLLnw_ListenPort = (unsigned short)((int)API.vp[28]&0xffff);
	DLLMulti_Gamelist_changed = (bool *)API.vp[29];
	DLLPXO_hosted_lobby_name = (char *)API.vp[30];
	DLLSupports_score_api = (bool *)API.vp[31];
	//DLLPXOPort = (bool *)API.vp[32];	// only defined for mtclient because I'm lazy...
	DLLDescent3_temp_directory = (char*)API.vp[33];

	*DLLSupports_score_api = false;
