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

#include "args.h"
#include "multi.h"
#include "multi_server.h"
#include "player.h"
#include "game.h"
#include "mono.h"
#include "ddio.h"
#include "hud.h"
#include "pilot.h"
#include "BOA.h"
#include "LoadLevel.h"
#include "Mission.h"
#include "game2dll.h"
#include "stringtable.h"

//#define USE_DIRECTPLAY

#ifdef USE_DIRECTPLAY
#include "directplay.h"
#endif

#include "dedicated_server.h"
#include "damage.h"
//#include "gamespy.h"
#include "multi_world_state.h"
#include "ObjScript.h"
#include "marker.h"
#include "findintersection.h"
#include "weapon.h"
#include "weather.h"
#include "doorway.h"
#include "object_lighting.h"
#include "ship.h"
#include "pstring.h"
#include "audiotaunts.h"

#include "ui.h"
#include "newui.h"
#include "multi_dll_mgr.h"
#include "spew.h"
#include "psrand.h"
#include "polymodel.h"
#include "init.h"
#include "../md5/md5.h"
#include "gamespy.h"

void MultiProcessShipChecksum(MD5* md5, int ship_index);

int GameDLLGetConnectingPlayersTeam(int slot);

int Moved_robots[MAX_NET_PLAYERS][MAX_CHANGED_OBJECTS];
ushort Num_moved_robots[MAX_NET_PLAYERS];

int Changed_anim[MAX_CHANGED_OBJECTS][MAX_NET_PLAYERS];
ushort Num_changed_anim[MAX_NET_PLAYERS];

int Changed_wb_anim[MAX_CHANGED_OBJECTS][MAX_NET_PLAYERS];
ushort Num_changed_wb_anim[MAX_NET_PLAYERS];

int Changed_turret[MAX_CHANGED_OBJECTS][MAX_NET_PLAYERS];
ushort Num_changed_turret[MAX_NET_PLAYERS];

float last_sent_bytes[MAX_NET_PLAYERS];
float Multi_last_send_visible[MAX_NET_PLAYERS];
uint Multi_visible_players[MAX_NET_PLAYERS];
extern int Buddy_handle[MAX_PLAYERS];
extern char Multi_message_of_the_day[];

int Join_response_strings[] = { TXI_MLTOK,TXI_MLTNOTSERV,TXI_MLTDORK,TXI_MLTNOTENOUGHSTR,TXI_MLTGAMEFULL };

//d3_net_game_data D3_tracker_info;
float LastTrackerDataUpdate;

int MTReadingPilot = -1;
int MTWritingPilot = -1;

int Player_count = 0;	// The total number of players playing the game

extern int Use_file_xfer;

unsigned int Secret_net_id = 0;
// Stars a multiplayer server.  If playing is non-zero, then the server is a player in the
// game
// Server only
void MultiStartServer(int playing, char* scriptname, int dedicated_server_num_teams)
{
	// Clear out some stuff
	for (int i = 0; i < MAX_NET_PLAYERS; i++)
	{
		memset(&NetPlayers[i], 0, sizeof(netplayer));
		Multi_send_size[i] = 0;

		for (int t = 0; t < MAX_NET_PLAYERS; t++)
			Multi_last_sent_time[i][t] = 0;

		Player_fire_packet[i].fired_on_this_frame = PFP_NO_FIRED;

		Multi_reliable_send_size[i] = 0;
		Multi_reliable_last_send_time[i] = 0;

		Multi_reliable_urgent[i] = 0;
		Multi_last_send_visible[i] = 0;
		Multi_visible_players[i] = 0xFFFFFFFF;

		Num_moved_robots[i] = 0;
		Num_changed_anim[i] = 0;
		Num_changed_turret[i] = 0;
		Num_changed_wb_anim[i] = 0;
		last_sent_bytes[i] = timer_GetTime();
		NetPlayers[i].file_xfer_flags = NETFILE_NONE;
		NetPlayers[i].ship_logo[0] = 0;
		NetPlayers[i].voice_taunt1[0] = 0;
		NetPlayers[i].voice_taunt2[0] = 0;
		NetPlayers[i].voice_taunt3[0] = 0;
		NetPlayers[i].voice_taunt4[0] = 0;
		NetPlayers[i].pilot_pic_id = PPIC_INVALID_ID;
		NetPlayers[i].custom_file_seq = NETFILE_ID_NOFILE;
		PlayerSetCustomTexture(i, NULL);

		NetPlayers[i].ping_time = 0;
		NetPlayers[i].last_ping_time = 0;
	}
	NetPlayers[Player_num].custom_file_seq = 0xff;
	Current_pilot.get_multiplayer_data(NetPlayers[Player_num].ship_logo, NetPlayers[Player_num].voice_taunt1, NetPlayers[Player_num].voice_taunt2, NULL, NetPlayers[Player_num].voice_taunt3, NetPlayers[Player_num].voice_taunt4);

	//ship_logo
	if (NetPlayers[Player_num].ship_logo[0])
		PlayerSetCustomTexture(Player_num, NetPlayers[Player_num].ship_logo);

	Player_num = 0;
	NetPlayers[Player_num].flags = NPF_SERVER | NPF_CONNECTED;

	// Strip trailing .d3m if there is one
	int sl = strlen(scriptname);
	if (scriptname[sl - 4] == '.' && scriptname[sl - 3] == 'd' && scriptname[sl - 2] == '3' && scriptname[sl - 1] == 'm')
		scriptname[sl - 4] = 0;

	NetPlayers[Player_num].sequence = NETSEQ_PREGAME;

	Netgame.local_role = LR_SERVER;
	Netgame.server_sequence = 0;
	Netgame.server_version = MULTI_VERSION;

	Game_mode = GM_NETWORK;

	// Setup audio taunt delay time
	int audiotauntdelayarg = FindArg("-audiotauntdelay");
	if (audiotauntdelayarg > 0)
	{
		float t;
		char adt_str[64], * p;
		strcpy(adt_str, GameArgs[audiotauntdelayarg + 1]);
		p = adt_str;
		while (*p && (*p == ' ' || *p == '\t')) p++;
		t = atof(p);
		if (t == 0)
			t = 5.0f;

		mprintf((0, "MULTI: Setting audio taunt delay time to %.2f seconds\n", t));
		taunt_SetDelayTime(t);
	}
	else
	{
		taunt_SetDelayTime(5.0f);
	}

	// Temporary name fix
	Current_pilot.get_name(Players[Player_num].callsign);

	// setup the server's pilot id
	Current_pilot.get_multiplayer_data(NULL, NULL, NULL, &NetPlayers[Player_num].pilot_pic_id);

	// Choose ship
	char pshipmodel[PAGENAME_LEN];
	Current_pilot.get_ship(pshipmodel);

	Players[Player_num].ship_index = FindShipName(pshipmodel);
	if (Players[Player_num].ship_index < 0)
		Players[Player_num].ship_index = 0;

	SetGamemodeScript(scriptname, dedicated_server_num_teams);

	MultiFlushAllIncomingBuffers();
#ifdef USE_DIRECTPLAY
	dp_StartGame(Netgame.name);
#endif

	gspy_StartGame(Netgame.name);
	//We use this to identify clients....
	ps_srand(timer_GetTime());
	Secret_net_id = ps_rand() * ps_rand();
	LastPacketReceived = timer_GetTime();

	// At this point we have a level checksum, so add the ships into the mix
	for (int i = 0; i < MAX_SHIPS; i++)
		if (Ships[i].used)
			MultiGetShipChecksum(i);

}


//Checks if the selected mission and script are compatible
//Return values:
//-1 Not compatible!
//>=0 Number of teams supported for this mod & level

#define SCRIPTBADFORMISSION -1

int CheckMissionForScript(char* mission, char* script, int dedicated_server_num_teams)
{
	char mod_keys[MAX_KEYWORDLEN];
	if (!GetDLLRequirements(script, mod_keys, MAX_KEYWORDLEN))
	{
		return SCRIPTBADFORMISSION;
	}
	int teams = MissionGetKeywords(mission, mod_keys);
	//Bad match!
	if (teams == -1)
	{
		if (!Dedicated_server)
		{
			DoMessageBox(script, TXT_MSNNOTCOMPATIBLE, MSGBOX_OK);
			return SCRIPTBADFORMISSION;
		}
		else
		{
			PrintDedicatedMessage(TXT_MSNNOTCOMPATIBLE);
			PrintDedicatedMessage("\n");
		}
	}



	//See how many teams this dll will allow.
	int desired_teams = 1;
	int max_teams, min_teams;

	GetDLLNumTeamInfo(script, &min_teams, &max_teams);

	if (min_teams > teams)
	{
		mprintf((0, "This multiplayer game requires more teams than the mission supports!\n"));
		return SCRIPTBADFORMISSION;
	}
	//Use whatever is smaller, the dll's max teams, or what the missions supports
	if (max_teams > teams)
		max_teams = teams;

	if (Dedicated_server)
	{
		teams = min_teams;

		if (min_teams != max_teams)
		{
			//they can set the number of teams for this game
			//make sure it's valid
			if (dedicated_server_num_teams >= min_teams && dedicated_server_num_teams <= max_teams)
			{
				teams = dedicated_server_num_teams;
			}
			else
			{
				//invalid num teams
				PrintDedicatedMessage(TXT_INVALIDNUMTEAMS, teams);
				PrintDedicatedMessage("\n");
			}
		}

	}
	else
	{
		if (min_teams != max_teams)
		{
			char a_num_teams[5];
			char team_count_msg[300];
			sprintf(a_num_teams, "%d", min_teams);
			sprintf(team_count_msg, "%s %s (%d - %d)", TXT_TEAMCOUNTPROMPT, script, min_teams, max_teams);
		retry_team_count:
			int res = DoEditDialog(team_count_msg, a_num_teams, 4);
			if (res)
			{
				desired_teams = atoi(a_num_teams);
				if ((desired_teams > max_teams) || (desired_teams < min_teams))
				{
					DoMessageBox(script, TXT_INVALIDTEAMCOUNT, MSGBOX_OK);
					goto retry_team_count;
				}
				else
				{
					teams = desired_teams;
				}
			}
			else
			{
				return SCRIPTBADFORMISSION;
			}
		}
	}

	return teams;
}



// multi_check_listen() calls low level routines to see if we have a connection from a client we
// should accept.
void MultiCheckListen()
{
	int i;
	network_address addr;
	SOCKET sock = INVALID_SOCKET;

	// call routine which calls select to see if we need to check for a connect from a client
	// by passing addr, we are telling check_for_listen to do the accept and return who it was from in
	// addr.  The
	sock = nw_CheckListenSocket(&addr);
	if (sock != INVALID_SOCKET)
	{
		// the connection was accepted.  Find a free slot for this new player
		for (i = 0; i < MAX_NET_PLAYERS; i++)
		{
			if (i == Player_num)
				continue;

			if (!(NetPlayers[i].flags & NPF_CONNECTED))
			{
				// This slot is free!
				MultiSendConnectionAccepted(i, sock, &addr);
				break;
			}
		}

		// if we didn't find a player, close the socket
		if (i == MAX_NET_PLAYERS)
		{
			mprintf((0, "Got accept on my listen socket, but no free slots.  Closing socket.\n"));
			nw_CloseSocket(&sock);
		}
	}
}

// Tells the client that we're done sending players
void MultiSendDonePlayers(int slot)
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending done players\n"));

	size = START_DATA(MP_DONE_PLAYERS, data, &count);
	int num = MultiCountPlayers();
	MultiAddByte(num, data, &count);
	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);
}

// Tells the client that we're done sending buildings
void MultiSendDoneBuildings(int slot)
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending done buildings\n"));

	size = START_DATA(MP_DONE_BUILDINGS, data, &count);
	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);
}

// Tells the client that we're done sending objects
void MultiSendDoneObjects(int slot)
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending done objects\n"));

	// Take the level checksum and clone it.
	MD5* playermd5 = Level_md5->Clone();
	// Generate a random salt value and send it to the client.
	int salt = ps_rand();
	// process the salt through the md5
	playermd5->MD5Update(salt);
	// process the ships through the md5
	for (int i = 0; i < MAX_SHIPS; i++)
		if (Ships[i].used)
			MultiProcessShipChecksum(playermd5, i);
	// save the digest value in the netplayer slot
	playermd5->MD5Final(NetPlayers[slot].digest);
	MD5::Destroy(playermd5);

	size = START_DATA(MP_DONE_OBJECTS, data, &count);
	MultiAddInt(salt, data, &count);
	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);
}

// Tells the client that we're done sending the world states
void MultiSendDoneWorldStates(int slot)
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending done world states\n"));

	size = START_DATA(MP_DONE_WORLD_STATES, data, &count);
	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);
}

// Clears all the player markers belonging to a particular slot
void MultiClearPlayerMarkers(int slot)
{
	int m1 = slot * 2;
	int m2 = (slot * 2) + 1;

	for (int i = 0; i <= Highest_object_index; i++)
	{
		if (Objects[i].type == OBJ_MARKER && (Objects[i].id == m1 || Objects[i].id == m2))
		{
			SetObjectDeadFlag(&Objects[i], true, false);
		}
	}
}

// Tells our clients that a player disconnected
void MultiSendPlayerDisconnect(int slot)
{
	int size, count = 0;
	ubyte data[MAX_GAME_DATA_SIZE];

	size = START_DATA(MP_DISCONNECT, data, &count);

	// Send player whose disconnected
	MultiAddByte(slot, data, &count);

	END_DATA(count, data, size);

	if (Game_is_master_tracker_game)
	{
		NetPlayers[slot].flags |= NPF_MT_WRITING_PILOT;
	}
	for (int i = 0; i < MAX_NET_PLAYERS; i++)
	{
		if (i == Player_num || i == slot)
			continue;

		if (NetPlayers[i].flags & NPF_CONNECTED)
			nw_SendReliable(NetPlayers[i].reliable_socket, data, count);
	}
}

#define DISCONNECT_TIME		10
// Disconnects all players that haven't been heard from in a while
// Server only
void MultiDisconnectDeadPlayers()
{
	for (int i = 0; i < MAX_NET_PLAYERS; i++)
	{
		if (i == Player_num)
			continue;

		if (NetPlayers[i].flags & NPF_CONNECTED)
		{
			float cur_time = timer_GetTime();

			if (!nw_CheckReliableSocket(NetPlayers[i].reliable_socket))
			{
				mprintf((0, "Disconnecting player %d because the reliable socket closed.\n", i));
				AddHUDMessage("%s (%s)", TXT_RELIABLE_OVERRUN, Players[i].callsign);
				MultiDisconnectPlayer(i);
			}
			else if (NetPlayers[i].sequence > NETSEQ_LEVEL_START && NetPlayers[i].sequence != NETSEQ_LEVEL_END)
			{
				if (cur_time - NetPlayers[i].last_packet_time > DISCONNECT_TIME)
				{
					mprintf((0, "8sec disconnecting player %d.  Last packet time=%f Sequence=%d\n", i, cur_time - NetPlayers[i].last_packet_time, NetPlayers[i].sequence));
					MultiDisconnectPlayer(i);
				}
			}
			else	// If not playing (ie joining, give them longer)
			{
				if (cur_time - NetPlayers[i].last_packet_time > (DISCONNECT_TIME * 15))
				{
					mprintf((0, "Too long...disconnecting player %d.  Last packet time=%f Sequence=%d\n", i, cur_time - NetPlayers[i].last_packet_time, NetPlayers[i].sequence));
					MultiDisconnectPlayer(i);
				}
			}
		}
	}
}

// Disconnect a player for whatever reason
// Server only
void MultiDisconnectPlayer(int slot)
{
	ASSERT(Netgame.local_role == LR_SERVER);

	DLLInfo.me_handle = DLLInfo.it_handle = Objects[Players[slot].objnum].handle;
	CallGameDLL(EVT_GAMEPLAYERDISCONNECT, &DLLInfo);

	if (NetPlayers[slot].flags & NPF_CONNECTED)
	{
		mprintf((0, "Disconnecting player %d (%s)...\n", slot, Players[slot].callsign));
		if (NetPlayers[slot].file_xfer_flags != NETFILE_NONE)
		{
			MultiCancelFile(slot, NetPlayers[slot].custom_file_seq, NetPlayers[slot].file_xfer_who);
		}
		nw_CloseSocket(&NetPlayers[slot].reliable_socket);

		if (NetPlayers[slot].sequence == NETSEQ_PLAYING)
		{
			PlayerSpewInventory(&Objects[Players[slot].objnum], true, true);
			MultiMakePlayerGhost(slot);
			AddHUDMessage(TXT_MLTDISCONNECTED, Players[slot].callsign);
		}


		NetPlayers[slot].flags &= ~NPF_CONNECTED;
		NetPlayers[slot].secret_net_id = 0;
		MultiClearGuidebot(slot);
		MultiClearPlayerMarkers(slot);
		MultiSendPlayerDisconnect(slot);
		Players[slot].flags = 0;

	}
	else
		mprintf((0, "Trying to disconnect a non-existant player!\n"));
}


// Sends existing players to a joining player 
// Sends to "slot" and describes player "which"
// Server only
void MultiSendPlayer(int slot, int which)
{
	ASSERT(Netgame.local_role == LR_SERVER);

	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	int size_offset;

	mprintf((0, "Sending MP_PLAYER packet to player %d!\n", slot));

	size_offset = START_DATA(MP_PLAYER, data, &count);

	MultiAddByte(which, data, &count);

	// Add name
	int len = strlen(Players[which].callsign) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], Players[which].callsign, len);
	count += len;

	// Add name
	len = strlen(Ships[Players[which].ship_index].name) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], Ships[Players[which].ship_index].name, len);
	count += len;

	// Add flags, shields, inventory
	MultiAddInt(Players[which].flags, data, &count);
	MultiAddInt(Players[which].weapon_flags, data, &count);
	MultiAddInt(NetPlayers[which].flags, data, &count);
	MultiAddFloat(Objects[Players[which].objnum].shields, data, &count);

	// Add position, roomnum, and terrain flag

	// Just for safety sake add the object number
	MultiAddShort(Players[which].objnum, data, &count);

	// If this is a team game, get the first start position
	if (slot == which)
	{
		int myteam = GameDLLGetConnectingPlayersTeam(slot);

		ubyte temp_team = (myteam == -1) ? 255 : myteam;
		MultiAddByte(temp_team, data, &count);

		if (myteam >= 0 && myteam <= 4)
			Players[slot].team = myteam;

		if (temp_team == 255)
			Players[slot].start_index = slot;
		else
			Players[slot].start_index = PlayerGetRandomStartPosition(slot);

		MultiAddShort(Players[slot].start_index, data, &count);
	}
	else
	{
		ubyte temp_team = (Players[which].team == -1) ? 255 : Players[which].team;
		MultiAddByte(temp_team, data, &count);
	}

	// Tell them if this player is currently observing
	if (Objects[Players[which].objnum].type == OBJ_OBSERVER)
		MultiAddByte(1, data, &count);
	else
		MultiAddByte(0, data, &count);


	//send the player's address
	memcpy(data + count, &NetPlayers[which].addr, sizeof(network_address));
	count += sizeof(network_address);

	// PPS
	MultiAddByte(NetPlayers[which].pps, data, &count);

	//pilot picture id
	MultiAddUshort(NetPlayers[which].pilot_pic_id, data, &count);

	// Ranking
	MultiAddFloat(Players[which].rank, data, &count);

	// Mastertracker id
	if (Game_is_master_tracker_game)
	{
		int len = strlen(Players[which].tracker_id) + 1;
		MultiAddByte(len, data, &count);
		memcpy(&data[count], Players[which].tracker_id, len);
		count += len;
	}

	END_DATA(count, data, size_offset);

	// Send it out
	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);

}

// Sends blownup buildings to a joining player 
// Server only
void MultiSendBuildings(int slot)
{
	ASSERT(Netgame.local_role == LR_SERVER);

	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	int size_offset;

	mprintf((0, "Sending MP_BUILDING packet to player %d!\n", slot));

	size_offset = START_DATA(MP_BUILDING, data, &count);

	MultiAddByte(Multi_num_buildings_changed, data, &count);

	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if (Multi_building_states[i] != 0)
		{
			MultiAddShort((ushort)i, data, &count);
		}
	}

	ASSERT(count < MAX_GAME_DATA_SIZE);

	END_DATA(count, data, size_offset);

	// Send it out
	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);
}

// Returns true if we should send an update about this object
bool MultiIsValidMovedObject(object* obj)
{
	bool good = false;

	if (obj->type == OBJ_ROBOT || obj->type == OBJ_CLUTTER || obj->type == OBJ_BUILDING)
		good = true;

	if (good == true)
	{
		if (obj->flags & (OF_DEAD))
			good = false;
		if (!(obj->flags & OF_CLIENT_KNOWS))
			good = false;

		// Still going?  Good!
		if (good == true)
		{
			if (obj->movement_type == MT_WALKING || obj->movement_type == MT_PHYSICS)
				return true;
		}
	}

	// Didn't pass all the tests else it wouldn't make it this fare
	return false;
}

void MultiSendJoinDemoObjects(int slot)
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	int size_offset, i;

	int num_demo_objects = 0;
	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (Objects[i].type != OBJ_NONE && Objects[i].flags & OF_CLIENTDEMOOBJECT)
		{
			num_demo_objects++;
		}
	}

	ASSERT((num_demo_objects * sizeof(ushort) + sizeof(ushort)) < MAX_GAME_DATA_SIZE);

	mprintf((0, "Sending DemoJoinObjects (%d)\n", num_demo_objects));

	count = 0;
	size_offset = START_DATA(MP_SEND_DEMO_OBJECT_FLAGS, data, &count);

	MultiAddUshort(num_demo_objects, data, &count);

	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (Objects[i].type != OBJ_NONE && Objects[i].flags & OF_CLIENTDEMOOBJECT)
		{
			MultiAddUshort(i, data, &count);
		}
	}

	END_DATA(count, data, size_offset);
	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);
}

int StuffObjectIntoPacket(object* obj, ubyte* data)
{
	uint index;
	int count = 0;
	bool obj_is_dummy = false;

	MultiAddUshort(obj - Objects, data, &count);
	MultiAddByte(obj->type, data, &count);

	//	Send old object type if it's a dummy
	if (obj->type == OBJ_DUMMY)
	{
		obj_is_dummy = true;
		MultiAddByte(obj->dummy_type, data, &count);
		ObjUnGhostObject(obj - Objects);
	}


	if (obj->type != OBJ_CAMERA && obj->type != OBJ_DOOR)
	{
		// Stuff the checksum 
		index = MultiGetMatchChecksum(obj->type, obj->id);
		MultiAddUint(index, data, &count);
	}

	if (obj->type != OBJ_POWERUP && obj->type != OBJ_DOOR)
	{
		multi_orientation mat;
		MultiMakeMatrix(&mat, &obj->orient);
		MultiMatrixMakeEndianFriendly(&mat);
		memcpy(&data[count], &mat, sizeof(multi_orientation));
		count += sizeof(multi_orientation);
	}

	// Send position
	if (obj->type != OBJ_DOOR)
	{
		//memcpy (&data[count],&obj->pos,sizeof(vector));
		//count+=sizeof(vector);
		MultiAddVector(obj->pos, data, &count);

		// Send room number and terrain flag
		MultiAddUshort(CELLNUM(obj->roomnum), data, &count);

		if (OBJECT_OUTSIDE(obj))
			MultiAddByte(1, data, &count);
		else
			MultiAddByte(0, data, &count);

		if (obj->flags & OF_USES_LIFELEFT)
		{
			ASSERT(obj->lifeleft < 255);
			MultiAddUbyte(obj->lifeleft, data, &count);
		}
		else
			MultiAddUbyte(255, data, &count);
	}

	if (obj->type == OBJ_MARKER)
	{
		// Add marker message to the end of this
		ubyte len = strlen(MarkerMessages[obj->id]) + 1;
		MultiAddByte(len, data, &count);
		memcpy(data + count, MarkerMessages[obj->id], len);
		count += len;
	}

	ASSERT(count < MAX_GAME_DATA_SIZE);

	// we need to reghost the object if what originally a ghost
	if (obj_is_dummy)
	{
		ObjGhostObject(obj - Objects);
	}

	return count;
}

// Sends objects to a joining player 
// Server only
#define MAX_OBJECTS_PER_PACKET	50
void MultiSendJoinObjects(int slot)
{
	ASSERT(Netgame.local_role == LR_SERVER);

	ubyte data[MAX_GAME_DATA_SIZE];
	ushort outgoing_objects[MAX_OBJECTS];
	int count = 0;
	int size_offset;
	int i;

	mprintf((0, "Sending MP_JOIN_OBJECTS packet to player %d!\n", slot));

	ushort total_objects = 0;
	last_sent_bytes[slot] = timer_GetTime();

	Num_moved_robots[slot] = 0;
	Num_changed_anim[slot] = 0;
	Num_changed_turret[slot] = 0;
	Num_changed_wb_anim[slot] = 0;

	// Count how many powerups/robots we have to send and make a list
	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (Objects[i].type == OBJ_ROBOT || Objects[i].type == OBJ_POWERUP || Objects[i].type == OBJ_MARKER || Objects[i].type == OBJ_CAMERA || Objects[i].type == OBJ_CLUTTER || Objects[i].type == OBJ_BUILDING || Objects[i].type == OBJ_DUMMY || Objects[i].type == OBJ_DOOR)
		{
			outgoing_objects[total_objects] = i;
			total_objects++;

			if (MultiIsValidMovedObject(&Objects[i]))
			{
				if (Num_changed_anim[slot] < MAX_CHANGED_OBJECTS)
				{
					Changed_anim[Num_changed_anim[slot]][slot] = Objects[i].handle;
					Num_changed_anim[slot]++;
				}

				object* obj = &Objects[i];
				polyobj_info* p_info = &obj->rtype.pobj_info;
				if (p_info->multi_turret_info.num_turrets)
				{
					if (Num_changed_turret[slot] < MAX_CHANGED_OBJECTS)
					{
						Changed_turret[Num_changed_turret[slot]][slot] = Objects[i].handle;
						Num_changed_turret[slot]++;
					}
				}
			}
		}

	}

	// Only send up to n objects in a packet lest we overflow outgoing packet size

	int cur_object = 0;
	int objects_to_go = total_objects;

	while (objects_to_go > 0)
	{
		int num_objects_this_packet = min(MAX_OBJECTS_PER_PACKET, objects_to_go);
		int overflow = 0;

		count = 0;
		size_offset = START_DATA(MP_JOIN_OBJECTS, data, &count);
		int num_objs_offset = count;
		MultiAddByte(0, data, &count);

		// Actually add the objects to our outgoing buffer
		for (i = 0; i < num_objects_this_packet && !overflow; i++)
		{
			// Add objnum and id
			object* obj = &Objects[outgoing_objects[i + cur_object]];
			ubyte obj_data[MAX_GAME_DATA_SIZE];

			int num_bytes = StuffObjectIntoPacket(obj, obj_data);

			// If their is an overflow, send it out!
			if ((num_bytes + count) >= MAX_GAME_DATA_SIZE)
			{
				num_objects_this_packet = i;
				data[num_objs_offset] = num_objects_this_packet;
				END_DATA(count, data, size_offset);
				nw_SendReliable(NetPlayers[slot].reliable_socket, data, count, false);
				overflow = 1;
			}
			else
			{
				memcpy(&data[count], obj_data, num_bytes);
				count += num_bytes;
			}
		}

		if (!overflow)
		{
			data[num_objs_offset] = num_objects_this_packet;
			END_DATA(count, data, size_offset);
			nw_SendReliable(NetPlayers[slot].reliable_socket, data, count, false);
		}

		cur_object += num_objects_this_packet;
		objects_to_go -= num_objects_this_packet;
	}

	MultiSendJoinDemoObjects(slot);
}

void MultiStoreWorldPacket(int slot, ubyte* big_data, int* big_count, ubyte* cur_data, int* cur_count, int* size_offset)
{
	if (*big_count + *cur_count >= (MAX_GAME_DATA_SIZE - 3))
	{
		mprintf((0, "Starting another world packet!\n"));
		MultiAddByte(WS_END, big_data, big_count);
		END_DATA(*big_count, big_data, *size_offset);

		// Send it out
		nw_SendReliable(NetPlayers[slot].reliable_socket, big_data, *big_count);

		// Restart another packet
		*big_count = 0;
		*size_offset = START_DATA(MP_WORLD_STATES, big_data, big_count);
	}

	memcpy(&big_data[*big_count], cur_data, *cur_count);
	*big_count += *cur_count;
	*cur_count = 0;
	ASSERT(*big_count < (MAX_GAME_DATA_SIZE - 3));
}

doorway* GetDoorwayFromObject(int door_obj_handle);
// Function that sends all the changed world states to an incoming player
void MultiSendWorldStates(int slot)
{
	int i;
	ASSERT(Netgame.local_role == LR_SERVER);

	ubyte data[MAX_GAME_DATA_SIZE];
	ubyte cur_data[MAX_GAME_DATA_SIZE];
	int cur_count;
	int count = 0;
	int size_offset;

	mprintf((0, "Sending MP_WORLD_STATES packet to player %d!\n", slot));

	size_offset = START_DATA(MP_WORLD_STATES, data, &count);

	// Send MOTD
	if (Dedicated_server && Multi_message_of_the_day[0] != 0)
	{
		cur_count = 0;
		MultiAddByte(WS_MOTD, cur_data, &cur_count);
		MultiAddString(Multi_message_of_the_day, cur_data, &cur_count);
		MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
	}


	// Send weather
	cur_count = 0;
	MultiAddByte(WS_WEATHER, cur_data, &cur_count);
	MultiAddByte(Weather.flags, cur_data, &cur_count);
	MultiAddFloat(Weather.rain_intensity_scalar, cur_data, &cur_count);
	MultiAddFloat(Weather.snow_intensity_scalar, cur_data, &cur_count);
	MultiAddFloat(Weather.lightning_interval_time, cur_data, &cur_count);
	MultiAddShort(Weather.lightning_rand_value, cur_data, &cur_count);
	MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);



	// Send broke glass
	if (Num_broke_glass > 0)
	{
		for (i = 0; i < Num_broke_glass; i++)
		{
			cur_count = 0;
			MultiAddByte(WS_ROOM_GLASS, cur_data, &cur_count);
			MultiAddUshort(Broke_glass_rooms[i], cur_data, &cur_count);
			MultiAddUshort(Broke_glass_faces[i], cur_data, &cur_count);
			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}
	}

	// Send waypoint
	cur_count = 0;
	MultiAddByte(WS_WAYPOINT, cur_data, &cur_count);
	MultiAddByte(Current_waypoint, cur_data, &cur_count);
	MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);

	// Send player bit-fields
	cur_count = 0;
	MultiAddByte(WS_PLAYERBF, cur_data, &cur_count);
	MultiAddUint(Players_typing, cur_data, &cur_count);
	MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);

	// Do room stuff
	for (i = 0; i <= Highest_room_index; i++)
	{
		if (!Rooms[i].used)
			continue;

		if (Rooms[i].room_change_flags & RCF_WIND)
		{
			cur_count = 0;

			MultiAddByte(WS_ROOM_WIND, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);

			MultiAddFloat(Rooms[i].wind.x, cur_data, &cur_count);
			MultiAddFloat(Rooms[i].wind.y, cur_data, &cur_count);
			MultiAddFloat(Rooms[i].wind.z, cur_data, &cur_count);

			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}
		if (Rooms[i].room_change_flags & RCF_FOG)
		{
			cur_count = 0;

			MultiAddByte(WS_ROOM_FOG, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);

			MultiAddFloat(Rooms[i].fog_depth, cur_data, &cur_count);
			MultiAddUbyte(Rooms[i].fog_r * 255.0, cur_data, &cur_count);
			MultiAddUbyte(Rooms[i].fog_g * 255.0, cur_data, &cur_count);
			MultiAddUbyte(Rooms[i].fog_b * 255.0, cur_data, &cur_count);

			if (Rooms[i].flags & RF_FOG)
				MultiAddUbyte(1, cur_data, &cur_count);
			else
				MultiAddUbyte(0, cur_data, &cur_count);


			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}

		if (Rooms[i].room_change_flags & RCF_LIGHTING)
		{
			cur_count = 0;

			MultiAddByte(WS_ROOM_LIGHTING, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);
			MultiAddUbyte(Rooms[i].pulse_time, cur_data, &cur_count);
			MultiAddUbyte(Rooms[i].pulse_offset, cur_data, &cur_count);

			if (Rooms[i].flags & RF_FLICKER)
				MultiAddUbyte(1, cur_data, &cur_count);
			else
				MultiAddUbyte(0, cur_data, &cur_count);

			if (Rooms[i].flags & RF_STROBE)
				MultiAddUbyte(1, cur_data, &cur_count);
			else
				MultiAddUbyte(0, cur_data, &cur_count);


			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}

		if (Rooms[i].room_change_flags & RCF_TEXTURE)
		{
			for (int t = 0; t < Rooms[i].num_faces; t++)
			{
				if (Rooms[i].faces[t].flags & FF_TEXTURE_CHANGED)
				{
					cur_count = 0;
					MultiAddByte(WS_ROOM_TEXTURE, cur_data, &cur_count);
					MultiAddShort(i, cur_data, &cur_count);
					MultiAddShort(t, cur_data, &cur_count);
					MultiAddString(GameTextures[Rooms[i].faces[t].tmap].name, cur_data, &cur_count);
					MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
				}
			}
		}

		if (Rooms[i].room_change_flags & RCF_GOALSPECIAL_FLAGS)
		{
			int mask = (RF_SPECIAL1 | RF_SPECIAL2 | RF_SPECIAL3 | RF_SPECIAL4 | RF_SPECIAL5 | RF_SPECIAL6 | RF_GOAL1 | RF_GOAL2 | RF_GOAL3 | RF_GOAL4);
			int change_mask = (Rooms[i].flags & mask);
			MultiAddByte(WS_ROOM_GOALSPECFLAG, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);
			MultiAddInt(change_mask, cur_data, &cur_count);
			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}

		if (Rooms[i].room_change_flags & RCF_DAMAGE)
		{
			cur_count = 0;
			MultiAddByte(WS_ROOM_DAMAGE, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);

			MultiAddFloat(Rooms[i].damage, cur_data, &cur_count);
			MultiAddUbyte(Rooms[i].damage_type, cur_data, &cur_count);
			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}
		if (Rooms[i].room_change_flags & RCF_REFUEL)
		{
			cur_count = 0;
			MultiAddByte(WS_ROOM_REFUEL, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);

			if (Rooms[i].flags & RF_FUELCEN)
				MultiAddUbyte(1, cur_data, &cur_count);
			else
				MultiAddUbyte(0, cur_data, &cur_count);

			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}

		if (Rooms[i].room_change_flags & RCF_PORTAL_RENDER)
		{
			for (int t = 0; t < Rooms[i].num_portals; t++)
			{
				if (Rooms[i].portals[t].flags & PF_CHANGED)
				{
					cur_count = 0;
					MultiAddByte(WS_ROOM_PORTAL_RENDER, cur_data, &cur_count);
					MultiAddShort(i, cur_data, &cur_count);
					MultiAddShort(t, cur_data, &cur_count);
					MultiAddByte(Rooms[i].portals[t].flags, cur_data, &cur_count);
					MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
				}
			}
		}

		if (Rooms[i].room_change_flags & RCF_PORTAL_BLOCK)
		{
			for (int t = 0; t < Rooms[i].num_portals; t++)
			{
				if (Rooms[i].portals[t].flags & PF_CHANGED)
				{
					cur_count = 0;
					MultiAddByte(WS_ROOM_PORTAL_BLOCK, cur_data, &cur_count);
					MultiAddShort(i, cur_data, &cur_count);
					MultiAddShort(t, cur_data, &cur_count);
					MultiAddByte(Rooms[i].portals[t].flags, cur_data, &cur_count);
					MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
				}
			}
		}
	}

	// Do object stuff
	for (i = 0; i <= Highest_object_index; i++)
	{
		object* obj = &Objects[i];

		if ((obj->type == OBJ_NONE) || (obj->flags & OF_DEAD))
			continue;

		if (obj->type == OBJ_DOOR)
		{
			doorway* dp;
			dp = GetDoorwayFromObject(Objects[i].handle);
			cur_count = 0;
			MultiAddByte(WS_DOOR, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);
			MultiAddByte(dp->state, cur_data, &cur_count);
			MultiAddByte((dp->flags & DF_LOCKED) != 0, cur_data, &cur_count);
			MultiAddFloat(dp->position, cur_data, &cur_count);
			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}

		if (obj->change_flags & OCF_LIGHT_DISTANCE)
		{
			cur_count = 0;
			MultiAddByte(WS_LIGHT_DISTANCE, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);

			light_info* li = ObjGetLightInfo(obj);
			if (li)
				MultiAddFloat(li->light_distance, cur_data, &cur_count);
			else
				MultiAddFloat(0, cur_data, &cur_count);

			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}

		if (obj->change_flags & OCF_LIGHT_COLOR)
		{
			cur_count = 0;
			MultiAddByte(WS_LIGHT_COLOR, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);

			light_info* li = ObjGetLightInfo(obj);
			if (li)
			{
				MultiAddFloat(li->red_light1, cur_data, &cur_count);
				MultiAddFloat(li->green_light1, cur_data, &cur_count);
				MultiAddFloat(li->blue_light1, cur_data, &cur_count);
			}
			else
			{
				MultiAddFloat(0, cur_data, &cur_count);
				MultiAddFloat(0, cur_data, &cur_count);
				MultiAddFloat(0, cur_data, &cur_count);
			}

			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}

		if (obj->change_flags & OCF_NO_RENDER)
		{
			cur_count = 0;
			MultiAddByte(WS_NO_RENDER, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);

			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}


		if (obj->change_flags & OCF_PHYS_FLAGS)
		{
			cur_count = 0;
			MultiAddByte(WS_OBJECT_PHYS_FLAGS, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);
			MultiAddInt(Objects[i].mtype.phys_info.flags, cur_data, &cur_count);

			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}


		// Check to see if we have an attached object
		if (obj->attach_children != NULL)
		{
			cur_count = 0;

			poly_model* pm = &Poly_models[obj->rtype.pobj_info.model_num];
			MultiAddByte(WS_OBJECT_ATTACH, cur_data, &cur_count);
			MultiAddUshort(i, cur_data, &cur_count);

			MultiAddByte(pm->n_attach, cur_data, &cur_count);

			for (int t = 0; t < pm->n_attach; t++)
			{
				object* child = ObjGet(obj->attach_children[t]);
				if (!child)
					MultiAddUshort(65535, cur_data, &cur_count);
				else
				{
					ASSERT(child->flags & OF_ATTACHED);
					MultiAddUshort(OBJNUM(child), cur_data, &cur_count);
					MultiAddByte(child->attach_index, cur_data, &cur_count);
					MultiAddByte(child->attach_type, cur_data, &cur_count);

					if (child->attach_type == AT_RAD)
						MultiAddFloat(child->attach_dist / child->size, cur_data, &cur_count);
				}
			}

			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}
	}

	// Do spew stuff
	int spewcount = 0;
	for (i = 0; i < MAX_SPEW_EFFECTS; i++)
	{

		if (SpewEffects[i].inuse)
		{
			spewcount++;
			cur_count = 0;
			spewinfo* sp = &SpewEffects[i];

			MultiAddByte(WS_SPEW, cur_data, &cur_count);
			MultiAddShort(i, cur_data, &cur_count);

			if (sp->use_gunpoint)
			{
				MultiAddByte(1, cur_data, &cur_count);
				MultiAddUshort(sp->gp.obj_handle & HANDLE_OBJNUM_MASK, cur_data, &cur_count);
				MultiAddByte(sp->gp.gunpoint, cur_data, &cur_count);
			}
			else
			{
				MultiAddByte(0, cur_data, &cur_count);
				//memcpy (&cur_data[cur_count],&sp->pt.normal,sizeof(vector));
				//cur_count+=sizeof(vector);
				MultiAddVector(sp->pt.normal, cur_data, &cur_count);
				//memcpy (&cur_data[cur_count],&sp->pt.origin,sizeof(vector));
				//cur_count+=sizeof(vector);
				MultiAddVector(sp->pt.origin, cur_data, &cur_count);

				MultiAddInt(sp->pt.room_num, cur_data, &cur_count);
			}

			MultiAddByte(sp->random, cur_data, &cur_count);
			MultiAddByte(sp->real_obj, cur_data, &cur_count);
			MultiAddByte(sp->effect_type, cur_data, &cur_count);
			MultiAddByte(sp->phys_info, cur_data, &cur_count);
			MultiAddFloat(sp->drag, cur_data, &cur_count);
			MultiAddFloat(sp->mass, cur_data, &cur_count);
			MultiAddFloat(sp->time_int, cur_data, &cur_count);
			MultiAddFloat(sp->longevity, cur_data, &cur_count);
			MultiAddFloat(sp->lifetime, cur_data, &cur_count);
			MultiAddFloat(sp->size, cur_data, &cur_count);
			MultiAddFloat(sp->speed, cur_data, &cur_count);

			MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
		}
	}
	mprintf((0, "Send %d spew events\n", spewcount));

	//Send buddybot handles if needed
	if (Netgame.flags & NF_ALLOWGUIDEBOT)
	{
		for (int i = 0; i < MAX_PLAYERS; i++)
		{
			if (Buddy_handle[i] != OBJECT_HANDLE_NONE)
			{
				MultiAddByte(WS_BUDDYBOTS, cur_data, &cur_count);
				MultiAddByte(i, cur_data, &cur_count);
				MultiAddUshort(Buddy_handle[i] & HANDLE_OBJNUM_MASK, cur_data, &cur_count);
				MultiStoreWorldPacket(slot, data, &count, cur_data, &cur_count, &size_offset);
			}
		}
	}


	MultiAddByte(WS_END, data, &count);

	ASSERT(count < MAX_GAME_DATA_SIZE);

	END_DATA(count, data, size_offset);

	// Send it out
	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count, false);
}


// Sends this reliable packet to everyone except the server and the named slot
void MultiSendReliablyToAllExcept(int except, ubyte* data, int size, int seq_threshold, bool urgent)
{
	for (int i = 0; i < MAX_NET_PLAYERS; i++)
	{
		if (!(NetPlayers[i].flags & NPF_CONNECTED))
			continue;

		// Don't send to these clowns
		if (i == except || i == Player_num)
			continue;

		if (NetPlayers[i].sequence >= seq_threshold && NetPlayers[i].sequence != NETSEQ_LEVEL_END)
			nw_SendReliable(NetPlayers[i].reliable_socket, data, size, urgent);
	}
}

// Sends this nonreliable packet to everyone except the server and the named slot
void MultiSendToAllExcept(int except, ubyte* data, int size, int seq_threshold)
{

	ASSERT(Netgame.local_role == LR_SERVER);
	for (int i = 0; i < MAX_NET_PLAYERS; i++)
	{
		if (!(NetPlayers[i].flags & NPF_CONNECTED))
			continue;

		// Don't send to these clowns
		if (i == except || i == Player_num)
			continue;

		if (seq_threshold == -1 || (NetPlayers[i].sequence >= seq_threshold && NetPlayers[i].sequence != NETSEQ_LEVEL_END))
			nw_Send(&NetPlayers[i].addr, data, size, 0);

	}
}

// Flushes all receive sockets so that there is no data coming from them
void MultiFlushAllIncomingBuffers()
{
	ubyte* data;
	network_address from_addr;
	int size;

	data = &(Multi_receive_buffer[0]);

	// get all incoming data and throw it away
	while ((size = nw_Receive(data, &from_addr)) > 0)
		;


	if (Netgame.local_role == LR_SERVER)
	{
		// Flush reliable sockets
		for (int i = 0; i < MAX_NET_PLAYERS; i++)
		{
			if (Player_num == i)
				continue;

			if ((NetPlayers[i].flags & NPF_CONNECTED) && (NetPlayers[i].reliable_socket != INVALID_SOCKET)
				&& (NetPlayers[i].reliable_socket != 0))
			{
				while ((size = nw_ReceiveReliable(NetPlayers[i].reliable_socket, data, MAX_RECEIVE_SIZE)) > 0)
					;

			}
		}
	}

}

// Checks to see if its any powerups need repositioning on the client machine
void MultiCheckToRepositionPowerups()
{
	int i;
	static int invis_id = -2;

	if (invis_id == -2)
	{
		invis_id = FindObjectIDName("InvisiblePowerup");
	}

	int changed = 0;

	for (i = 0; i <= Highest_object_index; i++)
	{
		object* obj = &Objects[i];
		if (obj->type != OBJ_POWERUP)
			continue;

		if (obj->id == invis_id)
			continue;

		if (obj->flags & OF_DEAD)
			continue;

		if (obj->flags & OF_STOPPED_THIS_FRAME)
		{
			// Send out our positional change
			int count = 0;
			ubyte data[MAX_GAME_DATA_SIZE];
			int size = START_DATA(MP_POWERUP_REPOSITION, data, &count);
			changed++;

			MultiAddUshort(i, data, &count);
			//memcpy (&data[count],&obj->pos,sizeof(vector));
			//count+=sizeof(vector);
			MultiAddVector(obj->pos, data, &count);

			// Send room number and terrain flag
			MultiAddUshort(CELLNUM(obj->roomnum), data, &count);

			if (OBJECT_OUTSIDE(obj))
				MultiAddByte(1, data, &count);
			else
				MultiAddByte(0, data, &count);

			END_DATA(count, data, size);
			MultiSendReliablyToAllExcept(Player_num, data, count, NETSEQ_OBJECTS, false);
		}
	}

	if (changed > 0)
	{
		mprintf((0, "Stopped=%d\n", changed));
	}
}


// Checks to see if its time to respawn any powerups 
void MultiCheckToRespawnPowerups()
{
	int i, t;

	for (i = 0; i < Num_powerup_timer; i++)
	{
		if (Powerup_timer[i].respawn_time < Gametime)
		{
			// This powerup needs to be respawned, so find a vacant spot
			int num_cand = 0;
			int candidates[MAX_RESPAWNS];

			for (t = 0; t < Num_powerup_respawn; t++)
			{
				if (Powerup_respawn[t].used == 0)
				{
					if (Netgame.flags & NF_RANDOMIZE_RESPAWN)
					{
						// We found a spot!
						candidates[num_cand++] = t;
					}
					else
					{
						if (Powerup_timer[i].id == Powerup_respawn[t].original_id)
							candidates[num_cand++] = t;
					}
				}
			}

			if (num_cand == 0)
			{
				mprintf((0, "Couldn't find a good spot to respawn!!!\n"));
				return;
			}

			t = candidates[ps_rand() % num_cand];

			mprintf((0, "Respawning powerup with id of %d.\n", Powerup_timer[i].id));
			Powerup_respawn[t].used = 1;
			int objnum = ObjCreate(OBJ_POWERUP, Powerup_timer[i].id, Powerup_respawn[t].roomnum, &Powerup_respawn[t].pos, NULL);
			if (objnum > -1)
				MultiSendObject(&Objects[objnum], 1);

			InitObjectScripts(&Objects[objnum]);
			Powerup_respawn[t].objnum = objnum;

			// Now erase this powerup from our timer info
			for (t = i; t < Num_powerup_timer - 1; t++)
				Powerup_timer[t] = Powerup_timer[t + 1];

			Num_powerup_timer--;
			i--;
		}
	}
}

extern int Multi_occluded;
// Sends out positional updates based on clients pps
void MultiSendPositionalUpdates(int to_slot)
{
	ubyte data[MAX_GAME_DATA_SIZE];

	// Figure out if we should send positional updates to players
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		ubyte send_position = 0, timer_popped = 0;

		if ((Netgame.flags & NF_PEER_PEER) && i != Player_num)
			continue;		// Don't send out other positions if in peer mode

		//Don't send out position of the server in dedicated mode.
		if (Dedicated_server && (i == Player_num))
			continue;

		//Don't send out position of dead players (ghosts) or observers
		if (Objects[Players[i].objnum].type != OBJ_PLAYER)
			continue;

		if (i != Player_num)
			Multi_last_sent_time[to_slot][i] += Frametime;

		if (i == to_slot)
			continue;

		if (!(NetPlayers[i].flags & NPF_CONNECTED))
		{
			Multi_visible_players[i] &= ~(1 << to_slot);
			Multi_visible_players[to_slot] &= ~(1 << i);
			continue;
		}

		if (NetPlayers[i].sequence != NETSEQ_PLAYING)
		{
			Multi_visible_players[i] &= ~(1 << to_slot);
			Multi_visible_players[to_slot] &= ~(1 << i);
			continue;
		}

		// Find out the player priority of this player in relation to the to_slot player
		float pps_check_time = NetPlayers[to_slot].pps;

		/*if (pps_check_time>=8)
		{
			vector subvec=Objects[Players[i].objnum].pos-Objects[Players[to_slot].objnum].pos;
			float mag=vm_GetMagnitudeFast (&subvec);
			subvec/=mag;

			float dp=vm_DotProduct (&subvec,&Objects[Players[to_slot].objnum].orient.fvec);
			dp=fabs(dp);

			pps_check_time=((float)pps_check_time/2.0)+((dp*((float)pps_check_time/2.0))+(dp*.9));
			pps_check_time=max (pps_check_time,6.0);
			pps_check_time=min (pps_check_time,20.0);

			int pps=pps_check_time;
			pps_check_time=pps;
		}*/

		// Set the timer as popped if it actually did				
		if (Multi_last_sent_time[to_slot][i] > (1.0 / pps_check_time))
			timer_popped = 1;


		//if (i==Player_num)	// Always send server position
		//	send_position=1;

		int num_src_to_check = 1;
		int srcs[10];
		srcs[0] = Players[to_slot].objnum;

		// Check for guideds
		if (Players[to_slot].guided_obj != NULL)
			srcs[num_src_to_check++] = Players[to_slot].guided_obj - Objects;

		if (Players[to_slot].small_dll_obj != -1)
		{
			int objnum = Players[to_slot].small_dll_obj;
			if (Objects[objnum].flags & OF_DEAD || Objects[objnum].type == OBJ_NONE)
			{
				Players[to_slot].small_dll_obj = -1;
			}
			else
			{
				srcs[num_src_to_check] = objnum;
				num_src_to_check++;
			}
		}

		// See if there are any small views we need to send for
		// Do left view
		if (Players[to_slot].small_left_obj != -1)
		{
			int objnum = Players[to_slot].small_left_obj;
			if (Objects[objnum].flags & OF_DEAD || Objects[objnum].type == OBJ_NONE || Objects[objnum].type == OBJ_WEAPON)
			{
				Players[to_slot].small_left_obj = -1;
			}
			else
			{
				srcs[num_src_to_check] = objnum;
				num_src_to_check++;
			}
		}

		// Do right view
		if (Players[to_slot].small_right_obj != -1)
		{
			int objnum = Players[to_slot].small_right_obj;
			if ((Objects[objnum].flags & OF_DEAD) || (Objects[objnum].type == OBJ_NONE) || (Objects[objnum].type == OBJ_WEAPON))
			{
				Players[to_slot].small_right_obj = -1;
			}
			else
			{
				srcs[num_src_to_check] = objnum;
				num_src_to_check++;
			}
		}

		for (int t = 0; t < num_src_to_check; t++)
		{
			int room_a = Objects[srcs[t]].roomnum;
			int room_b = Objects[Players[i].objnum].roomnum;

			if (BOA_IsVisible(room_a, room_b))
				send_position = 1;

			if (Player_fire_packet[i].fired_on_this_frame != PFP_NO_FIRED)
			{
				timer_popped = 1;

				if (Player_fire_packet[i].wb_index >= SECONDARY_INDEX)
				{
					send_position = 1;
				}
				else
				{
					if (BOA_IsVisible(Player_fire_packet[i].dest_roomnum, Objects[srcs[t]].roomnum))
						send_position = 1;

				}
			}
		}

		//Always send the server position, because we always say that the server is visible.  This
		//should fix some or all ghost ship problems.
		if (i == Player_num)
		{
			send_position = 1;
		}

		if (timer_popped)
		{
			if (i != Player_num)
				Multi_last_sent_time[to_slot][i] = 0;


			if (send_position)
			{
				Multi_visible_players[to_slot] |= (1 << i);

				int count = MultiStuffPosition(i, data);
				NetPlayers[to_slot].total_bytes_sent += count;

				int add_count = 0;
				// Send firing if needed
				if (Player_fire_packet[i].fired_on_this_frame == PFP_FIRED)
					add_count = MultiStuffPlayerFire(i, &data[count]);
				count += add_count;
				// Add in guided stuff
				if (Players[i].guided_obj != NULL)
				{
					add_count = MultiStuffGuidedInfo(i, &data[count]);
					count += add_count;
				}

				ASSERT(count < MAX_GAME_DATA_SIZE);

				nw_Send(&NetPlayers[to_slot].addr, data, count, 0);

				// TODO: SEND RELIABLE WEAPON FIRE HERE
				if (Player_fire_packet[i].fired_on_this_frame == PFP_FIRED_RELIABLE)
				{
					//mprintf((0,"NEED TO SEND RELIABLE FIRE FOR %d\n",i));
					count = MultiStuffPlayerFire(i, data);
					nw_SendReliable(NetPlayers[to_slot].reliable_socket, data, count, true);
				}
			}
			else
			{
				Multi_visible_players[to_slot] &= ~(1 << i);
				Multi_occluded++;
			}
		}
	}

}

// Figures out which robots have moved since the last time this player slot was updated
void MultiUpdateRobotMovedList(int slot)
{
	bool skip_this_obj = false;
	//check for moved robots
	for (int a = 0; a <= Highest_object_index; a++)
	{
		object* obj = &Objects[a];
		if (obj->type == OBJ_NONE)
			continue;

		if (MultiIsValidMovedObject(obj))
		{
			if ((obj->flags & OF_MOVED_THIS_FRAME))
			{
				skip_this_obj = false;
				//mprintf ((0,"Object %d (type=%d) moved this frame!\n",a,obj->type));
				//Check to see if this robot is on the list already
				for (int b = 0; b < Num_moved_robots[slot] && !skip_this_obj; b++)
				{
					if (Moved_robots[slot][b] == obj->handle)
					{
						skip_this_obj = true;
					}
				}

				if (!skip_this_obj)
				{
					Moved_robots[slot][Num_moved_robots[slot]] = obj->handle;
					ASSERT(Objects[a].flags & OF_CLIENT_KNOWS);
					Num_moved_robots[slot]++;
				}
			}
		}
	}
}

// Sets up our data structures so that the nonvisible robots will be sent when they are needed
void MultiSetupNonVisRobots(int slot, object* obj)
{
	obj->generic_nonvis_flags |= (1 << slot);
}


// Sends out a list of generics that this client can't see
void MultiSendGenericNonVis(int slot, ushort* objarray, int num)
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	int size_offset;

	size_offset = START_DATA(MP_GENERIC_NONVIS, data, &count);

	MultiAddShort(num, data, &count);

	for (int i = 0; i < num; i++)
	{
		MultiAddUshort(objarray[i], data, &count);
	}

	ASSERT(count < MAX_GAME_DATA_SIZE);

	END_DATA(count, data, size_offset);

	// Send it out
	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);

}

// Does the bookkeeping that needs to be done for non visible generic objects and a player
void MultiDoNonVisGenericForSlot(int slot)
{
	// Keep track of the ones we need to send about
	ushort send_out[MAX_OBJECTS];
	int num_to_send = 0;
	int i;


	// Deal with non-vis objects
	for (i = 0; i <= Highest_object_index; i++)
	{
		int objnum = i;
		object* obj = &Objects[i];
		if (obj->type == OBJ_NONE)
			continue;

		if (obj->generic_nonvis_flags & (1 << slot))
		{
			if (MultiIsValidMovedObject(obj))
			{
				if (!(obj->generic_sent_nonvis & (1 << slot)))
				{
					ASSERT(obj->flags & OF_CLIENT_KNOWS);
					send_out[num_to_send++] = objnum;
					obj->generic_sent_nonvis |= (1 << slot);
				}

				bool skip_this_obj = false;
				int b;

				//For movement, Check to see if this robot is on the list already
				for (b = 0; b < Num_moved_robots[slot] && !skip_this_obj; b++)
				{
					if (Moved_robots[slot][b] == obj->handle)
					{
						skip_this_obj = true;
					}
				}

				if (!skip_this_obj)
				{
					if (Num_moved_robots[slot] < MAX_CHANGED_OBJECTS)
					{
						Moved_robots[slot][Num_moved_robots[slot]] = obj->handle;
						ASSERT(Objects[i].flags & OF_CLIENT_KNOWS);
						Num_moved_robots[slot]++;
					}
				}

				// Now do changed anim
				skip_this_obj = false;
				//Check to see if this robot is on the list already
				for (b = 0; b < Num_changed_anim[slot]; b++)
				{
					if (Changed_anim[b][slot] == Objects[objnum].handle)
					{
						skip_this_obj = true;
						break;
					}
				}
				if (skip_this_obj == false)
				{
					if (Num_changed_anim[slot] < MAX_CHANGED_OBJECTS)
					{
						Changed_anim[Num_changed_anim[slot]][slot] = Objects[objnum].handle;
						ASSERT(Objects[i].flags & OF_CLIENT_KNOWS);
						Num_changed_anim[slot]++;
					}

				}

				// Now do changed turrets
				skip_this_obj = false;
				//Check to see if this robot is on the list already
				for (b = 0; b < Num_changed_turret[slot]; b++)
				{
					if (Changed_turret[b][slot] == Objects[objnum].handle)
					{
						skip_this_obj = true;
						break;
					}
				}
				if (skip_this_obj == false)
				{
					if (Num_changed_turret[slot] < MAX_CHANGED_OBJECTS)
					{
						Changed_turret[Num_changed_turret[slot]][slot] = Objects[objnum].handle;
						ASSERT(Objects[i].flags & OF_CLIENT_KNOWS);
						Num_changed_turret[slot]++;
					}
				}

				// Finally, do WB anim
				skip_this_obj = false;
				//Check to see if this robot is on the list already
				for (b = 0; b < Num_changed_wb_anim[slot]; b++)
				{
					if (Changed_wb_anim[b][slot] == Objects[objnum].handle)
					{
						skip_this_obj = true;
						break;
					}
				}
				if (skip_this_obj == false)
				{
					if (Num_changed_wb_anim[slot] < MAX_CHANGED_OBJECTS)
					{
						Changed_wb_anim[Num_changed_wb_anim[slot]][slot] = Objects[objnum].handle;
						ASSERT(Objects[i].flags & OF_CLIENT_KNOWS);
						Num_changed_wb_anim[slot]++;
					}
				}

			}

			// Clear our flag
			obj->generic_nonvis_flags &= ~(1 << slot);
		}
	}

	// Now send out what we need to
	if (num_to_send > 0)
	{
		int num_packets = num_to_send / 100;
		int num_left = num_to_send % 100;

		for (i = 0; i < num_packets; i++)
			MultiSendGenericNonVis(slot, &send_out[i * 100], 100);

		// Send out leftovers
		if (num_left > 0)
			MultiSendGenericNonVis(slot, &send_out[num_packets * 100], num_left);
	}


}

// Returns true or false based on whether or not the player can see this generic object in a multiplayer game
// This takes into account markers, dll objects, and other stuff
bool MultiIsGenericVisibleToPlayer(int test_objnum, int to_slot)
{
	int srcs[10], num_src_to_check = 1;

	srcs[0] = Players[to_slot].objnum;

	if (Players[to_slot].guided_obj != NULL)
		srcs[num_src_to_check++] = Players[to_slot].guided_obj - Objects;

	if (Players[to_slot].small_dll_obj != -1)
	{
		int objnum = Players[to_slot].small_dll_obj;
		if (Objects[objnum].flags & OF_DEAD || Objects[objnum].type == OBJ_NONE)
		{
			Players[to_slot].small_dll_obj = -1;
		}
		else
		{
			srcs[num_src_to_check] = objnum;
			num_src_to_check++;
		}
	}
	// See if there are any small views we need to send for
	// Do left view
	if (Players[to_slot].small_left_obj != -1)
	{
		int objnum = Players[to_slot].small_left_obj;
		if (Objects[objnum].flags & OF_DEAD || Objects[objnum].type == OBJ_NONE || Objects[objnum].type == OBJ_WEAPON)
		{
			Players[to_slot].small_left_obj = -1;
		}
		else
		{
			srcs[num_src_to_check] = objnum;
			num_src_to_check++;
		}
	}

	// Do right view
	if (Players[to_slot].small_right_obj != -1)
	{
		int objnum = Players[to_slot].small_right_obj;
		if (Objects[objnum].flags & OF_DEAD || Objects[objnum].type == OBJ_NONE || Objects[objnum].type == OBJ_WEAPON)
		{
			Players[to_slot].small_right_obj = -1;
		}
		else
		{
			srcs[num_src_to_check] = objnum;
			num_src_to_check++;
		}
	}

	for (int t = 0; t < num_src_to_check; t++)
	{
		if (BOA_IsVisible(Objects[test_objnum].roomnum, Objects[srcs[t]].roomnum))
			return true;
	}

	return false;

}




// Does robot stuff for a particular client
void MultiDoServerRobotFrame(int slot)
{
	int rcount = 0;
	int m = 0;
	ubyte rdata[MAX_GAME_DATA_SIZE];

	//send robot information for any robots that have moved.
	for (m = 0; m < Num_moved_robots[slot]; m++)
	{
		int send_position = 0;
		int objnum = Moved_robots[slot][m] & HANDLE_OBJNUM_MASK;

		if (Moved_robots[slot][m] != Objects[objnum].handle)
		{
			mprintf((0, "Caught handle objnum problem!\n"));
			continue;
		}

		if (MultiIsGenericVisibleToPlayer(objnum, slot))
			send_position = 1;

		if (send_position)
		{
			rcount = MultiStuffRobotPosition(objnum, rdata);
			if (rcount > 0)
			{
				if (Multi_send_size[slot] + rcount >= MAX_GAME_DATA_SIZE)
					MultiSendFullPacket(slot, 0);
				memcpy(&Multi_send_buffer[slot][Multi_send_size[slot]], rdata, rcount);
				Multi_send_size[slot] += rcount;
			}

			Objects[objnum].generic_sent_nonvis &= ~(1 << slot);
		}
		else
			MultiSetupNonVisRobots(slot, &Objects[objnum]);
	}

	//clear the robot movement flag for this player
	Num_moved_robots[slot] = 0;


	//Now do anim changed stuff
	for (m = 0; m < Num_changed_anim[slot]; m++)
	{
		int send_position = 0;
		int objnum = Changed_anim[m][slot] & HANDLE_OBJNUM_MASK;

		object* obj = &Objects[objnum];

		if (Changed_anim[m][slot] != obj->handle)
		{
			mprintf((0, "Caught anim handle objnum problem!\n"));
			continue;
		}

		if (MultiIsGenericVisibleToPlayer(objnum, slot))
			send_position = 1;

		if (send_position)
		{
			rcount = MultiStuffObjAnimUpdate(objnum, rdata);
			if (Multi_send_size[slot] + rcount >= MAX_GAME_DATA_SIZE)
				MultiSendFullPacket(slot, 0);
			memcpy(&Multi_send_buffer[slot][Multi_send_size[slot]], rdata, rcount);
			Multi_send_size[slot] += rcount;
			Objects[objnum].generic_sent_nonvis &= ~(1 << slot);
		}
		else
			MultiSetupNonVisRobots(slot, &Objects[objnum]);
	}
	// Clear changed animation for this player
	Num_changed_anim[slot] = 0;

	//Now send any changed turrets
	for (m = 0; m < Num_changed_turret[slot]; m++)
	{
		int send_position = 0;
		int objnum = Changed_turret[m][slot] & HANDLE_OBJNUM_MASK;

		object* obj = &Objects[objnum];

		if (Changed_turret[m][slot] != obj->handle)
		{
			mprintf((0, "Caught turret handle objnum problem!\n"));
			continue;
		}

		if (MultiIsGenericVisibleToPlayer(objnum, slot))
			send_position = 1;

		if (send_position)
		{
			rcount = MultiStuffTurretUpdate(objnum, rdata);
			if (Multi_send_size[slot] + rcount >= MAX_GAME_DATA_SIZE)
				MultiSendFullPacket(slot, 0);
			memcpy(&Multi_send_buffer[slot][Multi_send_size[slot]], rdata, rcount);
			Multi_send_size[slot] += rcount;
			Objects[objnum].generic_sent_nonvis &= ~(1 << slot);
		}
		else
			MultiSetupNonVisRobots(slot, &Objects[objnum]);
	}
	// Clear changed turret flag for this player
	Num_changed_turret[slot] = 0;

	//Next is weapon battery animation states
	for (m = 0; m < Num_changed_wb_anim[slot]; m++)
	{
		int send_position = 0;
		int objnum = Changed_wb_anim[m][slot] & HANDLE_OBJNUM_MASK;

		object* obj = &Objects[objnum];

		if (Changed_wb_anim[m][slot] != obj->handle)
		{
			mprintf((0, "Caught wb anim handle objnum problem!\n"));
			continue;
		}


		if (MultiIsGenericVisibleToPlayer(objnum, slot))
			send_position = 1;

		if (send_position)
		{
			rcount = MultiStuffObjWBAnimUpdate(objnum, rdata);
			if (Multi_send_size[slot] + rcount >= MAX_GAME_DATA_SIZE)
				MultiSendFullPacket(slot, 0);
			memcpy(&Multi_send_buffer[slot][Multi_send_size[slot]], rdata, rcount);
			Multi_send_size[slot] += rcount;
			Objects[objnum].generic_sent_nonvis &= ~(1 << slot);
		}
		else
			MultiSetupNonVisRobots(slot, &Objects[objnum]);
	}

	Num_changed_wb_anim[slot] = 0;

	MultiSendFullPacket(slot, 0);

	MultiDoNonVisGenericForSlot(slot);


}

// Shoots a straight vector from all the firing players and finds out what room 
// they hit.  This is useful for vis stuff

void MultiGetDestFireRooms()
{
	for (int i = 0; i < MAX_PLAYERS; i++)
	{
		if (!(NetPlayers[i].flags & NPF_CONNECTED))
			continue;
		if (NetPlayers[i].flags != NETSEQ_PLAYING)
			continue;

		if (Player_fire_packet[i].fired_on_this_frame == PFP_NO_FIRED)
			continue;

		// Ok, so now we know the player has fired, so just cast a ray and see where it hits
		fvi_query	fq;
		fvi_info		hit_data;
		int			fate;

		vector dest_vector = Objects[Players[i].objnum].pos + (Objects[Players[i].objnum].orient.fvec * 50000);

		fq.p0 = &Objects[Players[i].objnum].pos;
		fq.startroom = Objects[Players[i].objnum].roomnum;
		fq.p1 = &dest_vector;
		fq.rad = .05f;
		fq.thisobjnum = -1;
		fq.ignore_obj_list = NULL;
		fq.flags = FQ_IGNORE_NON_LIGHTMAP_OBJECTS | FQ_CHECK_OBJS;

		fate = fvi_FindIntersection(&fq, &hit_data);

		if (hit_data.hit_room < 0)
			Player_fire_packet[i].dest_roomnum = hit_data.hit_room;
		else
		{
			if (hit_data.hit_room > Highest_room_index)
				Player_fire_packet[i].dest_roomnum = Highest_room_index + 1;
			else
			{
				if (Rooms[hit_data.hit_room].flags & RF_EXTERNAL)
				{
					int roomnum = GetTerrainRoomFromPos(&hit_data.hit_pnt);
					Player_fire_packet[i].dest_roomnum = roomnum;
				}
				else
					Player_fire_packet[i].dest_roomnum = hit_data.hit_room;
			}
		}
	}
}


int Multi_occluded = 0;
#define MT_DATA_UPDATE_INTERVAL	300	//300 seconds
// Does whatever the server needs to do for this frame
void MultiDoServerFrame()
{
	int i;

	if (Netgame.flags & NF_EXIT_NOW)
	{
		// DLL says we should bail!
		Netgame.flags &= ~NF_EXIT_NOW;
		MultiLeaveGame();
		SetFunctionMode(MENU_MODE);
		return;
	}

	if (NetPlayers[Player_num].sequence == NETSEQ_PREGAME)
	{
		MultiFlushAllIncomingBuffers();
		NetPlayers[Player_num].sequence = NETSEQ_PLAYING;

		Players[Player_num].time_in_game = timer_GetTime();

		// Pick a start slot for me
		Players[Player_num].start_index = PlayerGetRandomStartPosition(Player_num);
		PlayerMoveToStartPos(Player_num, Players[Player_num].start_index);

		//Update the tracker if it's a master tracker game
		CallMultiDLL(MT_EVT_FIRST_FRAME);
		if (Game_is_master_tracker_game)
		{
			NetPlayers[Player_num].flags |= NPF_MT_READING_PILOT;
			strcpy(MTPilotinfo[Player_num].tracker_id, Players[Player_num].tracker_id);
			strcpy(MTPilotinfo[Player_num].pilot_name, Players[Player_num].callsign);
			NetPlayers[Player_num].flags &= ~NPF_WROTE_RANK;
		}

		if (Dedicated_server)	// Make this player an observer
			PlayerSwitchToObserver(Player_num, OBSERVER_MODE_ROAM);

		for (int i = 0; i < MAX_PLAYERS; i++)
			Multi_visible_players[i] = 0xFFFFFFFF;

	}

	if (Game_is_master_tracker_game)
	{
		CallMultiDLL(MT_EVT_FRAME);
	}

	// Check for new connections
	MultiCheckListen();
	MultiDisconnectDeadPlayers();
	MultiCheckToRespawnPowerups();
	MultiCheckToRepositionPowerups();

	// get the other net players data
	MultiProcessIncoming();

	// Find destination room if they fired
	MultiGetDestFireRooms();

	Player_count = 1;

	// Send out data
	for (i = 0; i < MAX_NET_PLAYERS; i++)
	{

		// Check to see if we should send rank to everyone
		if (Game_is_master_tracker_game)
		{
			if (NetPlayers[i].flags & NPF_CONNECTED)
			{
				if (Players[i].rank != -1)
				{
					if (!(NetPlayers[i].flags & NPF_WROTE_RANK))
					{
						MultiSendInitialRank(i);
						NetPlayers[i].flags |= NPF_WROTE_RANK;
					}
				}
			}
		}

		if (i == Player_num) {

			//START: code to fix clients not hearing server omega damage
			//Note the code here to keep us from sending the damage every frame.  Since NetPlayers[].pps doesn't
			//seem to be defined for the server/player, I've hard-coded it to 12 fps.
			Multi_last_sent_time[i][Player_num] += Frametime;
			float last_client_update = Multi_last_sent_time[i][Player_num];

			// Send other info if the timer has popped
			if (last_client_update > (1.0 / 12.0))		//was: (float)NetPlayers[i].pps
			{
				// Check to see if there is additional damage or shields to be added to this guy
				if (Multi_additional_damage[i] != 0)
				{
					if (!(Players[i].flags & (PLAYER_FLAGS_DYING | PLAYER_FLAGS_DEAD)))
						MultiSendAdditionalDamage(i, Multi_additional_damage_type[i], Multi_additional_damage[i]);
					Multi_additional_damage[i] = 0;
					Multi_additional_damage_type[i] = PD_NONE;
				}

				Multi_last_sent_time[i][Player_num] = 0;
			}
			//END: code to fix clients not hearing server omega damage

			continue;
		}

		if (NetPlayers[i].flags & NPF_CONNECTED)
		{
			// Check to see if this guy as any special requests
			if (NetPlayers[i].sequence == NETSEQ_REQUEST_PLAYERS)
			{
				for (int t = 0; t < MAX_NET_PLAYERS; t++)
				{
					if (NetPlayers[t].flags & NPF_CONNECTED)
					{
						MultiSendPlayer(i, t);
					}
				}

				NetPlayers[i].sequence = NETSEQ_PLAYERS;
				MultiSendDonePlayers(i);
			}

			if (NetPlayers[i].sequence == NETSEQ_REQUEST_BUILDINGS)
			{
				MultiSendBuildings(i);
				NetPlayers[i].sequence = NETSEQ_BUILDINGS;
				MultiSendDoneBuildings(i);
			}
			if (NetPlayers[i].sequence == NETSEQ_REQUEST_OBJECTS)
			{
				MultiSendJoinObjects(i);
				NetPlayers[i].sequence = NETSEQ_OBJECTS;
				MultiSendDoneObjects(i);
			}
			if (NetPlayers[i].sequence == NETSEQ_REQUEST_WORLD)
			{
				MultiSendWorldStates(i);
				NetPlayers[i].sequence = NETSEQ_WORLD;
				MultiSendDoneWorldStates(i);
			}
			else if (NetPlayers[i].sequence == NETSEQ_PLAYING)
			{
				Player_count++;

				if ((!(Netgame.flags & NF_PEER_PEER)) && ((timer_GetTime() - NetPlayers[i].last_ping_time) > MULTI_PING_INTERVAL))
				{
					MultiSendPing(i);
				}
				//If we actually want to send custom data out..
				if (Use_file_xfer)
				{
					//Check to see if we need to request any files from this bozo
					if ((NetPlayers[i].custom_file_seq != NETFILE_ID_NOFILE) && (NetPlayers[i].custom_file_seq != NETFILE_ID_DONE))
					{
						//See if we are already sending or receiving a file from this player
						if (NetPlayers[i].file_xfer_flags == NETFILE_NONE)
						{
							//Check to see if we have this file in our cache
							//Time to ask for the file specified in the sequence
							MultiAskForFile(NetPlayers[i].custom_file_seq, i, i);
						}
					}
				}


				//Send the client how much data we have sent him so far.
				if ((timer_GetTime() - last_sent_bytes[i]) > 5)
				{
					MultiSendBytesSent(i);
					last_sent_bytes[i] = timer_GetTime();
				}

				if ((timer_GetTime() - Multi_last_send_visible[i]) > .5)
				{
					Multi_last_send_visible[i] = timer_GetTime();
					MultiSendVisiblePlayers(i);
				}

				// Figure out which robots moved this frame
				MultiUpdateRobotMovedList(i);

				Multi_last_sent_time[i][Player_num] += Frametime;
				float last_client_update = Multi_last_sent_time[i][Player_num];

				// Send out all players movement
				MultiSendPositionalUpdates(i);

				// Send other info if the timer has popped
				if (last_client_update > (1.0 / (float)NetPlayers[i].pps))
				{
					// Send out robot updates if needed
					MultiDoServerRobotFrame(i);

					// Check to see if there is additional damage or shields to be added to this guy
					if (Multi_additional_damage[i] != 0)
					{
						if (!(Players[i].flags & (PLAYER_FLAGS_DYING | PLAYER_FLAGS_DEAD)))
							MultiSendAdditionalDamage(i, Multi_additional_damage_type[i], Multi_additional_damage[i]);
						Multi_additional_damage[i] = 0;
						Multi_additional_damage_type[i] = PD_NONE;
					}

					Multi_last_sent_time[i][Player_num] = 0;
				}
			}
		}
	}

	for (i = 0; i < MAX_NET_PLAYERS; i++)
		Player_fire_packet[i].fired_on_this_frame = PFP_NO_FIRED;

	mprintf_at((2, 5, 0, "Occ=%d  ", Multi_occluded));

	// If this is a dedicated server then we should get possible input
	if (Dedicated_server)
		DoDedicatedServerFrame();

}


// Tells clients to execute dlls on their machines
void MultiSendClientExecuteDLL(int eventnum, int me_objnum, int it_objnum, int to, dllinfo* info)
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	int size_offset;

	ASSERT(to >= -1 && to < MAX_PLAYERS);

	if (to == Player_num)
		return;
	//mprintf ((0,"Sending MP_EXECUTE_DLL packet to slot %d! type=%d id=%d\n",to,Objects[me_objnum].type,Objects[me_objnum].id));

	size_offset = START_DATA(MP_EXECUTE_DLL, data, &count, 1);

	MultiAddShort(eventnum, data, &count);
	MultiAddShort(me_objnum, data, &count);
	MultiAddShort(it_objnum, data, &count);

	if (info) {
		MultiAddByte(1, data, &count);
		MultiAddFloat(info->fParam, data, &count);
		MultiAddInt(info->iParam, data, &count);

		switch (eventnum) {
		case EVT_CLIENT_GAMEWALLCOLLIDE:
		case EVT_GAMEWALLCOLLIDE:
			MultiAddFloat(info->collide_info.point.x, data, &count);
			MultiAddFloat(info->collide_info.point.y, data, &count);
			MultiAddFloat(info->collide_info.point.z, data, &count);
			MultiAddFloat(info->collide_info.normal.x, data, &count);
			MultiAddFloat(info->collide_info.normal.y, data, &count);
			MultiAddFloat(info->collide_info.normal.z, data, &count);
			MultiAddFloat(info->collide_info.hitspeed, data, &count);
			MultiAddFloat(info->collide_info.hit_dot, data, &count);
			MultiAddInt(info->collide_info.hitseg, data, &count);
			MultiAddInt(info->collide_info.hitwall, data, &count);
			break;
		case EVT_CLIENT_GAMECOLLIDE:
		case EVT_GAMECOLLIDE:
			MultiAddFloat(info->collide_info.point.x, data, &count);
			MultiAddFloat(info->collide_info.point.y, data, &count);
			MultiAddFloat(info->collide_info.point.z, data, &count);
			MultiAddFloat(info->collide_info.normal.x, data, &count);
			MultiAddFloat(info->collide_info.normal.y, data, &count);
			MultiAddFloat(info->collide_info.normal.z, data, &count);
			break;
		case EVT_GAMEOBJCHANGESEG:
		case EVT_CLIENT_GAMEOBJCHANGESEG:
		case EVT_GAMEPLAYERCHANGESEG:
		case EVT_CLIENT_GAMEPLAYERCHANGESEG:
			MultiAddInt(info->newseg, data, &count);
			MultiAddInt(info->oldseg, data, &count);
			break;
		}
	}
	else {
		MultiAddByte(0, data, &count);
	}

	END_DATA(count, data, size_offset);

	// Send it out
	if (to == -1)
		MultiSendReliablyToAllExcept(Player_num, data, count, NETSEQ_OBJECTS, false);
	else
		nw_SendReliable(NetPlayers[to].reliable_socket, data, count);
}

// Resets the settings that a server uses
void MultiResetSettings()
{
	strcpy(Netgame.scriptname, "Anarchy.d3m");
	strcpy(Netgame.connection_name, "Direct TCP~IP");
	strcpy(Netgame.mission, "fury.mn3");
	strcpy(Netgame.name, "Generic D3 Game");

	Netgame.packets_per_second = 30;
	Netgame.respawn_time = 60;
	Netgame.flags = NF_RANDOMIZE_RESPAWN;
	Netgame.max_players = 8;
	Netgame.difficulty = 2;

	// Clear MOTD
	Multi_message_of_the_day[0] = 0;


}

#define LEVEL_SPAN		10.0f

#define KILL_BASE			0.52f
#define DEATH_BASE		0.50f

#define KILL_SCALAR		(KILL_BASE/LEVEL_SPAN)
#define DEATH_SCALAR		(DEATH_BASE/LEVEL_SPAN)


int GetRankLevel(int rank)
{
	if (rank < 600)	return 1;
	if (rank < 900)	return 2;
	if (rank < 1200)	return 3;
	if (rank < 1500)	return 4;
	if (rank < 1800)	return 5;
	if (rank < 2100)	return 6;
	if (rank < 2400)	return 7;
	if (rank < 2600)	return 8;
	if (rank < 3000)	return 9;
	return 10;
}

// Given two objects, calculates rank for them after a kill
float CalculateNewRanking(object* obj_a, object* obj_b, int won)
{
	ASSERT(obj_a->type == OBJ_PLAYER && obj_b->type == OBJ_PLAYER);

	float rank_a = Players[obj_a->id].rank;
	float rank_b = Players[obj_b->id].rank;

	int level_a = GetRankLevel((int)rank_a);
	int level_b = GetRankLevel((int)rank_b);

	if ((obj_a == obj_b) && won) return rank_a;

	if (won)
	{
		rank_a += (KILL_SCALAR * float(level_b - level_a) + KILL_BASE);
	}
	else
	{
		rank_a -= (DEATH_SCALAR * float(level_a - level_b) + DEATH_BASE);
	}
	float new_rank = rank_a;

	if (new_rank < 0)
		new_rank = 0;

	return new_rank;
}


/*
// Given two objects, calculates rank for them after a kill
float CalculateNewRanking (object *obj_a,object *obj_b,int won)
{
	ASSERT (obj_a->type==OBJ_PLAYER && obj_b->type==OBJ_PLAYER);

	float rank_a=Players[obj_a->id].rank;
	float rank_b=Players[obj_b->id].rank;

	float k_factor;
	float g_factor=400;
	float scalar;
	int num_players=Player_count;

	if (num_players>8)
		num_players=8;
	if (num_players<2)
		num_players=2;

	scalar=1.0-(((num_players-2)/6.0)*.5);
	g_factor=400*(1.0+(((num_players-2)/6.0)*.4));

	k_factor=16;
	k_factor*=scalar;

	float delta_r=rank_b-rank_a;
	float expected_wins = 1.0/((pow(10.0,delta_r / g_factor) + 1));
	float new_rank = (k_factor * (won - (expected_wins)));

	if (won && new_rank<.005f)
		new_rank=.005f;


	// This prevents players of vastly different rank from hurting each the better rank
	if ((rank_a-rank_b)>400 && !won)
		new_rank=0;

	new_rank=Players[obj_a->id].rank+new_rank;

	if (new_rank<0)
		new_rank=0;

	mprintf ((0,"Player %s rating is now %f\n",Players[obj_a->id].callsign,new_rank));

	return new_rank;
}
*/

// Returns a ranking index based on the player rating
// If rankbuf is non-null, fills in the string corresponding to that rank
// Returns -1 if not a pxo game (ie no rankings in this game)
int GetRankIndex(int pnum, char* rankbuf)
{
	if (!Game_is_master_tracker_game)
		return -1;

	float ranking = Players[pnum].rank;
	int val = 0;

	if (ranking >= 0 && ranking < 600)
		val = 0;
	else if (ranking >= 600 && ranking < 900)
		val = 1;
	else if (ranking >= 900 && ranking < 1200)
		val = 2;
	else if (ranking >= 1200 && ranking < 1500)
		val = 3;
	else if (ranking >= 1500 && ranking < 1800)
		val = 4;
	else if (ranking >= 1800 && ranking < 2100)
		val = 5;
	else if (ranking >= 2100 && ranking < 2400)
		val = 6;
	else if (ranking >= 2400 && ranking < 2600)
		val = 7;
	else if (ranking >= 2600 && ranking < 3000)
		val = 8;
	else if (ranking >= 3000)
		val = 9;

	if (rankbuf)
		sprintf(rankbuf, "%s", TXT(TXT_MULTI_RANKS + val));

	return val;

}

// Tells the clients to change rank for a player
void MultiSendChangeRank(int pnum, char* str, ubyte goodnews)
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending change rank!\n"));

	size = START_DATA(MP_CHANGE_RANK, data, &count);
	MultiAddByte(pnum, data, &count);
	MultiAddString(str, data, &count);
	MultiAddByte(goodnews, data, &count);
	MultiAddFloat(Players[pnum].rank, data, &count);
	END_DATA(count, data, size);

	MultiSendReliablyToAllExcept(Player_num, data, count, NETSEQ_PLAYING, false);
	MultiDoChangeRank(data);
}

// Does PXO stuff and sends out messages demoting/promoting the player
void ChangeRankIndex(int old_index, int pnum)
{
	char str[255];
	int new_index = GetRankIndex(pnum);
	ubyte goodnews = 1;

	if (old_index < new_index)	// promoted
	{
		sprintf(str, "%s %s %s %s!", Players[pnum].callsign, TXT_HAS_BEEN, TXT_PROMOTED, TXT(TXT_MULTI_RANKS + new_index));
		goodnews = 1;
	}
	else
	{
		sprintf(str, "%s %s %s %s!", Players[pnum].callsign, TXT_HAS_BEEN, TXT_DEMOTED, TXT(TXT_MULTI_RANKS + new_index));
		goodnews = 0;
	}

	MultiSendChangeRank(pnum, str, goodnews);


}

// Given a killer and killed player, calculates their new rankings
void GetNewRankings(object* killed, object* killer)
{
	float killed_rank;
	float killer_rank;

	//#ifdef OEM
	//	return;
	//#endif

#ifdef DEMO
	return;
#endif

	if (!killer || !killed)
		return;

	if (!(Netgame.flags & NF_TRACK_RANK))
		return;

	if (killed->type != OBJ_PLAYER || killer->type != OBJ_PLAYER)
		return;

	int old_killed_index = GetRankIndex(killed->id);
	int old_killer_index = GetRankIndex(killer->id);

	if (killed == killer)	// Suicide, always subract .5 points
	{
		Players[killer->id].rank -= .5;
		if (Players[killer->id].rank < 0)
		{
			Players[killer->id].rank = 0;
		}

		if (GetRankIndex(killer->id) != old_killer_index)
			ChangeRankIndex(old_killer_index, killer->id);
	}
	else
	{
		killed_rank = CalculateNewRanking(killed, killer, 0);
		killer_rank = CalculateNewRanking(killer, killed, 1);

		Players[killer->id].rank = killer_rank;
		Players[killed->id].rank = killed_rank;

		if (GetRankIndex(killer->id) != old_killer_index)
			ChangeRankIndex(old_killer_index, killer->id);

		if (GetRankIndex(killed->id) != old_killed_index)
			ChangeRankIndex(old_killed_index, killed->id);
	}
}
