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

#include "multi.h"
#include "multi_server.h"
#include "player.h"
#include "game.h"
#include "ddio.h"
#include "Mission.h"
#include "stringtable.h"
#include "pilot.h"
#include "ship.h"
#include "args.h"

#include "ui.h"
#include "newui.h"
#include "multi_dll_mgr.h"

#include "LoadLevel.h"
//#define USE_DIRECTPLAY

#ifdef USE_DIRECTPLAY
#include "directplay.h"
#endif

SOCKET ReliableSocket;
int Ok_to_join = -1;

int Got_level_info = 0;

// Whether or not the server will answer
bool Multi_accept_state = true;

// Points the server_address variable at a new location
void MultiSetServerAddress(network_address* addr)
{
	memcpy(&Netgame.server_address, addr, sizeof(network_address));
}


// How often to inquire about joining a game
#define ASK_POLL_TIME	2

// Returns true if the server says we can join
int AskToJoin(network_address* addr)
{
	int count = 0;
	int size;
	int tries = 0;
	ubyte data[MAX_GAME_DATA_SIZE];
	float start_time;
	network_address from_addr;

	size = START_DATA(MP_ASK_TO_JOIN, data, &count);
	END_DATA(count, data, size);

	Ok_to_join = -1;

	while (tries < 5 && Ok_to_join == -1)
	{
		// Send of our question and then wait!
		ASSERT(size > 0);
		nw_Send(addr, data, count, 0);
		tries++;

		start_time = timer_GetTime();
		while ((timer_GetTime() - start_time < ASK_POLL_TIME) && Ok_to_join == -1)
		{
			int packsize;
			while (((packsize = nw_Receive(Multi_receive_buffer, &from_addr)) > 0) && Ok_to_join == -1)
			{
				MultiProcessBigData(Multi_receive_buffer, packsize, &from_addr);
			}
		}
	}

	if (tries >= 5 || Ok_to_join != JOIN_ANSWER_OK)
		return 0;

	// Ok to join!
	return 1;
}

// Trys to join this client with the server address passed in
// Returns 1 on success, 0 on fail
// Client only
// For DirectPlay games, use the addr pointer as a pointer to the DPSESSIONDESC2
int TryToJoinServer(network_address* addr)
{
	SOCKET sock;

	ShowProgressScreen(TXT_MULTI_CONNECTING);
	//Store the server's address in the netplayer struct
	memcpy(&NetPlayers[0].addr, addr, sizeof(network_address));

	for (int i = 0; i < MAX_NET_PLAYERS; i++)
	{
		NetPlayers[i].file_xfer_flags = NETFILE_NONE;
		NetPlayers[i].ship_logo[0] = 0;
		NetPlayers[i].voice_taunt1[0] = 0;
		NetPlayers[i].voice_taunt2[0] = 0;
		NetPlayers[i].voice_taunt3[0] = 0;
		NetPlayers[i].voice_taunt4[0] = 0;
		NetPlayers[i].custom_file_seq = NETFILE_ID_NOFILE;
		//Set all custom textures to NULL
		PlayerSetCustomTexture(i, NULL);
	}
	Current_pilot.get_multiplayer_data(NetPlayers[Player_num].ship_logo, NetPlayers[Player_num].voice_taunt1, NetPlayers[Player_num].voice_taunt2, NULL, NetPlayers[Player_num].voice_taunt3, NetPlayers[Player_num].voice_taunt4);
	if (NetPlayers[Player_num].ship_logo[0])
		PlayerSetCustomTexture(Player_num, NetPlayers[Player_num].ship_logo);

	bool connected = false;
#ifdef USE_DIRECTPLAY
	if (Use_DirectPlay)
	{
		LPDPSESSIONDESC2 session = (LPDPSESSIONDESC2)addr;
		DPID server_id = DPID_SERVERPLAYER;
		Netgame.server_address.connection_type = NP_DIRECTPLAY;
		memcpy(&Netgame.server_address.address, &server_id, sizeof(DPID));
		if (dp_DirectPlayJoinGame(session))
		{
			connected = true;
			NetPlayers[Player_num].reliable_socket = DPID_SERVERPLAYER;
		}
		else
		{
			mprintf((0, "Unable to join DirectPlay session!\n"));
		}
	}
	else
#endif
	{
		if (!AskToJoin(addr))
		{
			if (Ok_to_join != -1)
			{
				char str[255];
				sprintf(str, TXT(Join_response_strings[Ok_to_join]));
				ShowProgressScreen(str);
				Sleep(2000);
			}
			else
			{
				ShowProgressScreen(TXT_MLTNORESPONSE);
				Sleep(2000);
			}
			return 0;
		}
		else
			mprintf((0, "Server says its ok to join!\n"));

		nw_ConnectToServer(&sock, addr);
		if (sock != INVALID_SOCKET && sock != 0)
		{
			connected = true;
			NetPlayers[Player_num].reliable_socket = sock;
		}
	}
	if (connected)
	{
#ifdef USE_DIRECTPLAY
		if (!Use_DirectPlay)
#endif
		{
			MultiSetServerAddress(addr);
		}
		mprintf((0, "Client mode set! Polling...\n"));

		MultiPollForConnectionAccepted();

		if (NetPlayers[Player_num].flags & NPF_CONNECTED)
			return 1;
		else
		{
			mprintf((0, "Couldn't join game for some reason!\n"));
#ifdef USE_DIRECTPLAY
			if (!Use_DirectPlay)
#endif
			{
				nw_CloseSocket(&sock);
			}
		}
	}
	else
	{
		mprintf((0, "nw_ConnectToServer says it can't find a good socket!\n"));
	}

	return 0;
}

// The server says we can join! 
// Client only
void MultiDoConnectionAccepted(ubyte* data)
{
	int count = 0;
	ushort server_version;

	// Skip header
	SKIP_HEADER(data, &count);

	server_version = MultiGetShort(data, &count);

	if (server_version != MULTI_VERSION)
	{
		mprintf((0, "Client and server code versions don't match.  Do an update!\n"));
		return;
	}
	else
	{
		// Versions match, get info about the game and then connect!
		mprintf((0, "Client/server versions match.\n"));

		// Check if we have the mission needed
		ubyte len = MultiGetByte(data, &count);
		memcpy(Netgame.mission, &data[count], len);
		count += len;

		if (!LoadMission(Netgame.mission))
		{
			mprintf((0, "We don't have this mission: %s!\n", Netgame.mission));
		}
		else
			mprintf((0, "Using mission %s...\n", Netgame.mission));

		len = MultiGetByte(data, &count);
		memcpy(Netgame.name, &data[count], len);
		count += len;

		// Do script name stuff
		len = MultiGetByte(data, &count);
		memcpy(Netgame.scriptname, &data[count], len);
		count += len;

		SetGamemodeScript(Netgame.scriptname);

		// Get my player num and copy my name over
		ubyte player_num = MultiGetByte(data, &count);
		char name[CALLSIGN_LEN];
		netplayer tempplayer;
		strcpy(name, Players[Player_num].callsign);
		memcpy(&tempplayer, &NetPlayers[Player_num], sizeof(netplayer));

		Player_num = player_num;
		mprintf((0, "Server tells me that my player num is %d!\n", Player_num));
		strcpy(Players[Player_num].callsign, name);
		memcpy(&NetPlayers[Player_num], &tempplayer, sizeof(netplayer));
		NetPlayers[Player_num].flags = NPF_CONNECTED;	// Hurray! We're connected

		// Get packets per second
		Netgame.packets_per_second = MultiGetByte(data, &count);
		mprintf((0, "Server is sending %d packets per second\n", Netgame.packets_per_second));

		//Get the secret code we will use to identify ourselves to the server
		unsigned int secret_code = MultiGetUint(data, &count);

		//Get the peer-peer flag
		int flags = MultiGetInt(data, &count);
		if (flags & NF_PEER_PEER)
			mprintf((0, "Using Peer/Peer model\n"));
		else
			mprintf((0, "Using Client/Server model\n"));

		Netgame.flags = flags;

		// Do Client smoothing hack
		if (FindArg("-usesmoothing"))
			Netgame.flags |= NF_USE_SMOOTHING;

		MultiSendGreetings(secret_code);
	}
}



#define JOIN_POLL_TIME	10	//Wait 4 seconds before bailing
// Polls for a connection message so we can finally join this game
// Client only
void MultiPollForConnectionAccepted()
{
	float start_time;
	int connected = 0;
	ubyte data[MAX_RECEIVE_SIZE];

	start_time = timer_GetTime();

	while ((timer_GetTime() - start_time < JOIN_POLL_TIME) && !connected)
	{
		int size;
		while ((size = nw_ReceiveReliable(NetPlayers[Player_num].reliable_socket, data, MAX_RECEIVE_SIZE)) > 0)
		{
			MultiProcessBigData(data, size, &Netgame.server_address);

			if (NetPlayers[Player_num].flags & NPF_CONNECTED)
			{
				connected = 1;
			}
		}
	}

	if (!connected)
	{
		mprintf((0, "Couldn't get a connection_accepted packet for some reason!\n"));
	}

}

// Gets a new connection to a new client set up
// This does not mean the client will enter the game, just that the server
// is successfully talking to the client
// Server only
void MultiSendConnectionAccepted(int slotnum, SOCKET sock, network_address* addr)
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	int size_offset;

	mprintf((0, "Sending connection accepted packet to slot %d!\n", slotnum));

	NetPlayers[slotnum].reliable_socket = sock;
	memcpy(&NetPlayers[slotnum].addr, addr, sizeof(network_address));
	NetPlayers[slotnum].flags = NPF_CONNECTED;
	NetPlayers[slotnum].sequence = NETSEQ_WAITING_FOR_LEVEL;
	Players[slotnum].flags = 0;
	Players[slotnum].rank = -1;

	// Adjust port
	NetPlayers[slotnum].addr.port--;


	NetPlayers[slotnum].last_packet_time = timer_GetTime();

	// Tell the new guy about the game in progress
	size_offset = START_DATA(MP_CONNECTION_ACCEPTED, data, &count);

	// Send server version
	MultiAddShort(Netgame.server_version, data, &count);

	// Do mission name
	int len = strlen(Netgame.mission) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], Netgame.mission, len);
	count += len;

	mprintf((0, "Sending netgame mission %s with length of %d!\n", Netgame.mission, len));

	len = strlen(Netgame.name) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], Netgame.name, len);
	count += len;

	// Do script name
	len = strlen(Netgame.scriptname) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], Netgame.scriptname, len);
	count += len;

	// Set Player number
	MultiAddByte(slotnum, data, &count);

	// Send packets per second
	MultiAddByte(Netgame.packets_per_second, data, &count);

	//Send secret code which the client will use to send a non reliable packet
	NetPlayers[slotnum].secret_net_id = Secret_net_id++;
	MultiAddUint(NetPlayers[slotnum].secret_net_id, data, &count);


	// Send netplayer flags (obsoletes the above line of code, left for a while for compatability)
	MultiAddInt(Netgame.flags, data, &count);

	END_DATA(count, data, size_offset);

	// Send it back to the player
	nw_SendReliable(sock, data, count);

	ASSERT(count < MAX_GAME_DATA_SIZE);
}

#define LEVEL_POLL_TIME	60
// Polls for a connection message so we can finally join this game
// Returns true if we got the level info
// Client only

int MultiPollForLevelInfo()
{
	float start_time, ask_time, initial_start_time;
	int connected = 0;
	ubyte data[MAX_RECEIVE_SIZE];
	network_address from_addr;

	ShowProgressScreen(TXT_MLTWAITSERVER);

	NetPlayers[Player_num].sequence = NETSEQ_WAITING_FOR_LEVEL;
	Got_level_info = 0;
	Got_heartbeat = false;

	MultiSendReadyForLevel();

	start_time = timer_GetTime();
	ask_time = timer_GetTime();
	initial_start_time = timer_GetTime();


	// Go through this waiting period until the server times out or we get level info
	while ((timer_GetTime() - start_time < LEVEL_POLL_TIME) && !Got_level_info && (timer_GetTime() - initial_start_time < (LEVEL_POLL_TIME * 5)))
	{
		if (timer_GetTime() - ask_time > 5.0)
		{
			ask_time = timer_GetTime();
			MultiSendReadyForLevel();
		}

		int size;
		while ((size = nw_ReceiveReliable(NetPlayers[Player_num].reliable_socket, data, MAX_RECEIVE_SIZE)) > 0)
		{
			MultiProcessBigData(data, size, &Netgame.server_address);
		}

		while ((size = nw_Receive(data, &from_addr)) > 0)
		{
			if (data[0] == MP_HEARTBEAT)
			{
				mprintf((0, "Got a heart beat from the server.\n"));
				Got_heartbeat = true;
			}
		}

		if (Got_heartbeat)
		{
			// Reset timer because we got a heartbeat
			start_time = timer_GetTime();
			Got_heartbeat = false;
		}
	}

	if (!Got_level_info)
	{
		ShowProgressScreen(TXT_MLTNOLEVELINFO);
		MultiLeaveGame();
		Sleep(2000);
	}
	else if (Got_level_info < 0)
	{
		ShowProgressScreen(TXT(Join_response_strings[-Got_level_info]));
		MultiLeaveGame();
		Sleep(2000);
	}
	else
	{
		Got_level_info = 0;
		return 1;
	}

	return 0;
}

#include "polymodel.h"

// Returns a unique value for this ship
int MultiGetShipChecksum(int ship_index)
{
	ship* s = &Ships[ship_index];

	int num = 0;

	AppendToLevelChecksum(s->phys_info.full_thrust);
	AppendToLevelChecksum(s->phys_info.full_rotthrust);

	AppendToLevelChecksum(s->phys_info.mass);
	AppendToLevelChecksum(s->phys_info.drag);
	AppendToLevelChecksum(s->phys_info.rotdrag);

	AppendToLevelChecksum(s->phys_info.flags);
	poly_model* pm = GetPolymodelPointer(s->model_handle);
	AppendToLevelChecksum(pm->rad);

	for (int w = 0; w < MAX_PLAYER_WEAPONS; w++)
	{
		for (int i = 0; i < MAX_WB_FIRING_MASKS; i++)
		{
			AppendToLevelChecksum(s->static_wb[w].gp_fire_wait[i]);
			AppendToLevelChecksum(s->static_wb[w].gp_fire_masks[i]);
		}

		AppendToLevelChecksum(s->static_wb[w].energy_usage);
		AppendToLevelChecksum(s->static_wb[w].ammo_usage);
		AppendToLevelChecksum(s->fire_flags[w]);
	}
	return num;
}


// Returns a unique value for this ship
void MultiProcessShipChecksum(MD5* md5, int ship_index)
{
	ship* s = &Ships[ship_index];

	md5->MD5Update(s->phys_info.full_thrust);
	md5->MD5Update(s->phys_info.full_rotthrust);

	md5->MD5Update(s->phys_info.mass);
	md5->MD5Update(s->phys_info.drag);
	md5->MD5Update(s->phys_info.rotdrag);

	md5->MD5Update(s->phys_info.flags);
	poly_model* pm = GetPolymodelPointer(s->model_handle);
	md5->MD5Update(pm->rad);

	for (int w = 0; w < MAX_PLAYER_WEAPONS; w++)
	{
		for (int i = 0; i < MAX_WB_FIRING_MASKS; i++)
		{
			md5->MD5Update(s->static_wb[w].gp_fire_wait[i]);
			md5->MD5Update(s->static_wb[w].gp_fire_masks[i]);
		}

		md5->MD5Update(s->static_wb[w].energy_usage);
		md5->MD5Update(s->static_wb[w].ammo_usage);
		md5->MD5Update(s->fire_flags[w]);
	}
}

// Server is telling us about the level
void MultiDoLevelInfo(ubyte* data)
{
	int count = 0;
	char pshipmodel[PAGENAME_LEN];
	Current_pilot.get_ship(pshipmodel);

	int ship_index = FindShipName(pshipmodel);
	if (ship_index < 0)
		ship_index = 0;

	// Skip header stuff
	SKIP_HEADER(data, &count);

	int join_response = MultiGetByte(data, &count);

	// Get level number
	Current_mission.cur_level = MultiGetByte(data, &count);

	Netgame.difficulty = MultiGetByte(data, &count);

	if ((Netgame.difficulty > 4) || (Netgame.difficulty < 0))
		Netgame.difficulty = 2;

	if (join_response != JOIN_ANSWER_OK)
		Got_level_info = -join_response;
	else
		Got_level_info = 1;
}

// Server is telling the client about the level currently playing
// Server only
void MultiSendLevelInfo(int slot)
{
	ASSERT(Netgame.local_role == LR_SERVER);
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	int size_offset;

	size_offset = START_DATA(MP_LEVEL_INFO, data, &count);

	if (Netgame.local_role == LR_SERVER)
	{
		if (Players[slot].start_roomnum != -1)
			MultiAddByte(JOIN_ANSWER_OK, data, &count);
		else
			MultiAddByte(JOIN_ANSWER_NO_ROOM, data, &count);
	}
	else
		MultiAddByte(JOIN_ANSWER_NOT_SERVER, data, &count);

	// Do level number (of the mission)
	mprintf((0, "Sending current mission level %d!\n", Current_mission.cur_level));
	MultiAddByte(Current_mission.cur_level, data, &count);

	//Send the difficulty
	MultiAddByte(Netgame.difficulty, data, &count);

	END_DATA(count, data, size_offset);

	// Send it out
	nw_SendReliable(NetPlayers[slot].reliable_socket, data, count);
}

// Clients says he's ready for level info
// so send it to him
void MultiDoReadyForLevel(ubyte* data)
{
	int count = 0;
	ubyte slot;
	char ship_name[PAGENAME_LEN];

	// Skip header stuff
	SKIP_HEADER(data, &count);

	// Get slot number
	slot = MultiGetByte(data, &count);

	// Copy the ship name out 
	int len = MultiGetByte(data, &count);
	memcpy(ship_name, &data[count], len);
	count += len;

	int ship_index = FindShipName(ship_name);
	if (ship_index < 0)
		ship_index = 0;

	PlayerChangeShip(slot, ship_index);

	MultiSendLevelInfo(slot);
	NetPlayers[slot].sequence = NETSEQ_LEVEL_START;
}

// Client is telling the server that he is ready for a level 
// Client only
void MultiSendReadyForLevel()
{
	ASSERT(Netgame.local_role != LR_SERVER);

	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	int size_offset;

	mprintf((0, "Sending ready for level!\n"));

	char pshipmodel[PAGENAME_LEN];
	Current_pilot.get_ship(pshipmodel);

	int ship_index = FindShipName(pshipmodel);
	if (ship_index < 0)
		ship_index = 0;

	size_offset = START_DATA(MP_READY_FOR_LEVEL, data, &count);

	MultiAddByte(Player_num, data, &count);

	// Send ship for anti-cheating purposes (CHEAP and EASILY HACKABLE!)
	int len = strlen(Ships[ship_index].name) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], Ships[ship_index].name, len);
	count += len;

	END_DATA(count, data, size_offset);

	// Send it out
	nw_SendReliable(NetPlayers[Player_num].reliable_socket, data, count);
}


extern int Multi_packet_tracking[];
// Clears all connections 
// Server and Client
void MultiCloseGame()
{
	mprintf((0, "Multi close game!\n"));

	if (!(Game_mode & GM_NETWORK))
	{
		mprintf((0, "Not network game!\n"));
		return;
	}

#if !(defined (RELEASE) || defined(MACINTOSH))
	// Write our packet tracking
	CFILE* outfile;
	outfile = cfopen("PacketTracking", "wt");
	if (!outfile)
	{
		mprintf((0, "Couldn't open packet tracking file!\n"));
	}
	else
	{
		for (int i = 0; i < 255; i++)
		{
			cfprintf(outfile, "Packet %d = %d\n", i, Multi_packet_tracking[i]);
		}
		cfclose(outfile);
	}
#endif

	Sleep(1000);	// Sleep for one second

	if (Netgame.local_role == LR_SERVER)
	{
		int i;

		for (i = 0; i < MAX_NET_PLAYERS; i++)
		{
			if (i == Player_num)
				continue;

			if (NetPlayers[i].flags & NPF_CONNECTED)
			{
				MultiDisconnectPlayer(i);
			}
		}

		for (i = 0; i < 100; i++)
		{
			nw_DoNetworkIdle();
			Sleep(10);
		}

		NetPlayers[Player_num].flags &= ~NPF_CONNECTED;
		MultiFlushAllIncomingBuffers();
	}
	else
	{
		if (NetPlayers[Player_num].flags & NPF_CONNECTED)
		{
			MultiFlushAllIncomingBuffers();
			nw_CloseSocket(&NetPlayers[Player_num].reliable_socket);
			NetPlayers[Player_num].flags &= ~NPF_CONNECTED;
		}
	}


	Game_mode &= ~GM_MULTI;
	if (Player_num != 0 && Player_object)
		SetObjectControlType(Player_object, CT_NONE);
	Player_num = 0;
	Player_object = Viewer_object = &Objects[Players[0].objnum];

}


#define GET_GAME_INFO_TIME	2
// Fills in the passed in buffers with the info of the games that are on this subnet
int SearchForLocalGamesTCP(unsigned int ask, ushort port)
{
	int count = 0;
	int size;
	int tries = 0;
	ubyte data[MAX_GAME_DATA_SIZE];
	network_address check_addr, from_addr;
	check_addr.connection_type = NP_TCP;
	if (ask)
	{
		memcpy(&check_addr.address, &ask, 4);
		check_addr.port = htons(port);

		size = START_DATA(MP_GET_GAME_INFO, data, &count);
		MultiAddFloat(timer_GetTime(), data, &count);
		MultiAddInt(MULTI_VERSION, data, &count);
		END_DATA(count, data, size);

		nw_Send(&check_addr, data, count, 0);

		//Num_network_games_known=0;
	}

	int packsize;
	while (((packsize = nw_Receive(Multi_receive_buffer, &from_addr)) > 0))
	{
		MultiProcessBigData(Multi_receive_buffer, packsize, &from_addr);
	}
	return Num_network_games_known;

}

// Fills in the passed in buffers with the info of the games that are on this subnet
int SearchForGamesPXO(unsigned int ask, ushort port)
{
	int count = 0;
	int size;
	int tries = 0;
	ubyte data[MAX_GAME_DATA_SIZE];
	network_address check_addr, from_addr;
	check_addr.connection_type = NP_TCP;
	if (ask)
	{
		memcpy(&check_addr.address, &ask, 4);
		check_addr.port = htons(port);

		size = START_DATA(MP_GET_PXO_GAME_INFO, data, &count);
		MultiAddFloat(timer_GetTime(), data, &count);
		MultiAddInt(MULTI_VERSION, data, &count);
		END_DATA(count, data, size);

		nw_Send(&check_addr, data, count, 0);

		//Num_network_games_known=0;
	}

	int packsize;
	while (((packsize = nw_Receive(Multi_receive_buffer, &from_addr)) > 0))
	{
		MultiProcessBigData(Multi_receive_buffer, packsize, &from_addr);
	}
	return Num_network_games_known;

}

#define MULTI_SERVER_TIMEOUT_TIME	40
extern bool Multi_Gamelist_changed;

void UpdateAndPackGameList(void)
{
	float curtime = timer_GetTime();
	for (int i = 0; i < Num_network_games_known; i++)
	{
		//Network_games[i]
		if ((curtime - Network_games[i].last_update) > MULTI_SERVER_TIMEOUT_TIME)
		{
			memcpy(&Network_games[i], &Network_games[Num_network_games_known], sizeof(network_game));
			Num_network_games_known--;
			i = 0;
			Multi_Gamelist_changed = true;
		}
	}

}

int SearchForLocalGamesIPX(network_address* check_addr)
{
	int count = 0;
	int size;
	int tries = 0;
	ubyte data[MAX_GAME_DATA_SIZE];
	network_address from_addr;

	if (check_addr)
	{
		if (check_addr->connection_type != NP_IPX)
			return 0;

		size = START_DATA(MP_GET_GAME_INFO, data, &count);
		MultiAddFloat(timer_GetTime(), data, &count);
		MultiAddInt(MULTI_VERSION, data, &count);
		END_DATA(count, data, size);

		nw_Send(check_addr, data, count, 0);

		//Num_network_games_known=0;
	}

	int packsize;
	while (((packsize = nw_Receive(Multi_receive_buffer, &from_addr)) > 0))
	{
		MultiProcessBigData(Multi_receive_buffer, packsize, &from_addr);
	}
	return Num_network_games_known;

}

// Sets whether or not the server answsers to a connection request
void MultiSetAcceptState(bool state)
{
	mprintf((0, "Setting multi_accept_state to %s.\n", state ? "true" : "false"));
	Multi_accept_state = state;
}
