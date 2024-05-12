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
#include "multi_client.h"
#include "game.h"
#include "player.h"
#include "ddio.h"
#include "pilot.h"
#include "Mission.h"
#include "stringtable.h"
#include "ship.h"

#define WEAPONS_LOAD_UPDATE_INTERVAL	2.0

float Last_weapons_load_update_time = 0;

// Setup saved moves
saved_move SavedMoves[MAX_SAVED_MOVES];
int Current_saved_move = 0;

extern int Use_file_xfer;

// Tell the server about my info, such as name, ship type, etc
void MultiSendMyInfo()
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending my info\n"));
	char pshipmodel[PAGENAME_LEN];
	Current_pilot.get_ship(pshipmodel);

	int ship_index = FindShipName(pshipmodel);
	if (ship_index < 0)
		ship_index = 0;

	size = START_DATA(MP_MY_INFO, data, &count);

	// Do callsign name
	MultiAddByte(Player_num, data, &count);
	int len = strlen(Players[Player_num].callsign) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], Players[Player_num].callsign, len);
	count += len;

	// Do ship name
	len = strlen(Ships[ship_index].name) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], Ships[ship_index].name, len);
	count += len;

	if (Game_is_master_tracker_game)
	{
		MultiAddUint(MASTER_TRACKER_SIG, data, &count);
		strcpy(Players[Player_num].tracker_id, Tracker_id);
		len = strlen(Players[Player_num].tracker_id) + 1;
		MultiAddByte(len, data, &count);
		memcpy(&data[count], Players[Player_num].tracker_id, len);
		count += len;
	}
	int ser = 0;
	//GetInternalSerializationNumber(&ser);
	MultiAddInt(ser, data, &count);

	// Send packets per second
	int pps = nw_ReccomendPPS();
	if ((Netgame.flags & NF_PERMISSABLE) && pps < 8)
		pps = 8;	// If permissable game, can't be lower than 8

	MultiAddByte(pps, data, &count);

	//pilot picture id
	ushort ppic_id;
	Current_pilot.get_multiplayer_data(NULL, NULL, NULL, &ppic_id);
	MultiAddUshort(ppic_id, data, &count);

	// Copy the guidebot name out 
	char guidebot_name[32];
	Current_pilot.get_guidebot_name(guidebot_name);
	len = strlen(guidebot_name) + 1;
	MultiAddByte(len, data, &count);
	memcpy(&data[count], guidebot_name, len);
	count += len;

	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[Player_num].reliable_socket, data, count);
}

// Ask the server to tell me about the players
void MultiSendRequestForPlayers()
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending request for players\n"));

	size = START_DATA(MP_REQUEST_PLAYERS, data, &count);
	MultiAddByte(Player_num, data, &count);
	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[Player_num].reliable_socket, data, count);
}

// Ask the server to tell me about the buildings
void MultiSendRequestForBuildings()
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending request for buildings\n"));

	size = START_DATA(MP_REQUEST_BUILDINGS, data, &count);
	MultiAddByte(Player_num, data, &count);
	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[Player_num].reliable_socket, data, count);
}

// Ask the server to tell me about the world
void MultiSendRequestForWorldStates()
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending request for world states\n"));

	size = START_DATA(MP_REQUEST_WORLD_STATES, data, &count);
	MultiAddByte(Player_num, data, &count);
	// Send the digest (checksum) for this level + our salt + the ships
	// Do level checksum	
	for (int i = 0; i < 16; i++)
	{
		MultiAddByte(NetPlayers[Player_num].digest[i], data, &count);
	}

	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[Player_num].reliable_socket, data, count);
}

// Ask the server to tell me about the objects
void MultiSendRequestForObjects()
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	mprintf((0, "Sending request for objects\n"));

	size = START_DATA(MP_REQUEST_OBJECTS, data, &count);
	MultiAddByte(Player_num, data, &count);
	END_DATA(count, data, size);

	nw_SendReliable(NetPlayers[Player_num].reliable_socket, data, count);
}



#define SERVER_DISCONNECT_TIME	8.0

// Returns 1 if the server is gone!
int ServerTimedOut()
{
	// Don't check for a non-connected player
	if (!(NetPlayers[Player_num].flags & NPF_CONNECTED))
		return 0;

	if (!nw_CheckReliableSocket(NetPlayers[Player_num].reliable_socket))
	{
		mprintf((0, "Reliable connection to the server was broken. Disconnecting.\n"));
		ShowProgressScreen(TXT_RELIABLE_OVERRUN);
		Sleep(1000);
		return 1;
	}
	if ((timer_GetTime() - Netgame.last_server_time) > SERVER_DISCONNECT_TIME)
		return 1;

	return 0;

}
#define NET_CLIENT_GAMETIME_REQ_TIMEOUT	10
#define NET_CLIENT_GAMETIME_REQ_RETRY		1
float First_gametime_req = 0;
float Last_gametime_req = 0;

// Do client stuff for this frame
void MultiDoClientFrame()
{
	int i;
	Multi_last_sent_time[Player_num][0] += Frametime;

	// Get data from the server
	MultiProcessIncoming();

	if (Netgame.flags & NF_EXIT_NOW)
	{
		// DLL says we should bail!
		Netgame.flags &= ~NF_EXIT_NOW;
		MultiLeaveGame();
		SetFunctionMode(MENU_MODE);
		return;
	}

	if (NetPlayers[Player_num].sequence == NETSEQ_LEVEL_START)
	{
		MultiSendMyInfo();
		NetPlayers[Player_num].sequence = NETSEQ_NEED_GAMETIME;
		First_gametime_req = timer_GetTime();
	}
	else if (NetPlayers[Player_num].sequence == NETSEQ_NEED_GAMETIME)
	{
		//Ask for gametime
		GetServerGameTime();
		Last_gametime_req = timer_GetTime();
		NetPlayers[Player_num].sequence = NETSEQ_WAIT_GAMETIME;
	}
	else if (NetPlayers[Player_num].sequence == NETSEQ_WAIT_GAMETIME)
	{
		if (Got_new_game_time)
		{
			NetPlayers[Player_num].sequence = NETSEQ_REQUEST_PLAYERS;
		}
		else
		{
			//Wait for server response
			if ((timer_GetTime() - First_gametime_req) > NET_CLIENT_GAMETIME_REQ_TIMEOUT)
			{
				//Giving up, we waited too long.
				mprintf((0, "Server disconnected while waiting for gametime!\n"));
				MultiLeaveGame();
				ShowProgressScreen(TXT_MLTDISCFRMSERV);
				Sleep(2000);
			}
			else if ((timer_GetTime() - Last_gametime_req) > NET_CLIENT_GAMETIME_REQ_RETRY)
			{
				NetPlayers[Player_num].sequence = NETSEQ_NEED_GAMETIME;
			}

		}

	}
	else if (NetPlayers[Player_num].sequence == NETSEQ_REQUEST_PLAYERS)
	{
		MultiSendRequestForPlayers();
		NetPlayers[Player_num].sequence = NETSEQ_PLAYERS;
	}
	else if (NetPlayers[Player_num].sequence == NETSEQ_REQUEST_BUILDINGS)
	{
		MultiSendRequestForBuildings();
		NetPlayers[Player_num].sequence = NETSEQ_BUILDINGS;
	}
	else if (NetPlayers[Player_num].sequence == NETSEQ_REQUEST_OBJECTS)
	{
		MultiSendRequestForObjects();
		NetPlayers[Player_num].sequence = NETSEQ_OBJECTS;
	}
	else if (NetPlayers[Player_num].sequence == NETSEQ_REQUEST_WORLD)
	{
		MultiSendRequestForWorldStates();
		NetPlayers[Player_num].sequence = NETSEQ_WORLD;
		NetPlayers[Player_num].custom_file_seq = 0;
	}
	else if (NetPlayers[Player_num].sequence == NETSEQ_PLAYING)
	{
		if (NetPlayers[Player_num].custom_file_seq == 0)
		{
			//Tell the server about our custom data (once only)
			if (NetPlayers[Player_num].ship_logo[0])
				PlayerSetCustomTexture(Player_num, NetPlayers[Player_num].ship_logo);
			NetPlayers[Player_num].custom_file_seq = 0xff;

			// Make sure we as the client actually want to send our custom data
			if (Use_file_xfer)
				MultiSendClientCustomData(Player_num);

		}

		bool client_file_xfering = false;
		for (i = 0; i < MAX_NET_PLAYERS; i++)
		{
			if (NetPlayers[i].file_xfer_flags != NETFILE_NONE)
				client_file_xfering = true;
		}
		//See if we have room for a transfer
		if (!client_file_xfering)
		{
			//Check to see if we need to request any files from this bozo
			for (i = 0; i < MAX_NET_PLAYERS; i++)
			{
				if (i == Player_num)
					continue;
				if ((NetPlayers[i].custom_file_seq != NETFILE_ID_NOFILE) && (NetPlayers[i].custom_file_seq != NETFILE_ID_DONE))
				{
					//See if we are already sending or receiving a file from this player
					if (NetPlayers[i].file_xfer_flags == NETFILE_NONE)
					{
						//Check to see if we have this file in our cache
						//Time to ask for the file specified in the sequence
						MultiAskForFile(NetPlayers[i].custom_file_seq, i, 0);
						break;
					}
				}
			}

		}

		Last_weapons_load_update_time += Frametime;
		if (Last_weapons_load_update_time > WEAPONS_LOAD_UPDATE_INTERVAL)
		{
			Last_weapons_load_update_time = 0;
			MultiSendWeaponsLoad();
		}


		// Figure out if we need to send our movement
		bool send_it = false;

		if (Player_fire_packet[Player_num].fired_on_this_frame != PFP_NO_FIRED)
			send_it = true;

		if (Multi_last_sent_time[Player_num][0] > (1.0 / (float)NetPlayers[Player_num].pps))
			send_it = true;


		if (send_it)
		{
			// Request damage if need be
			if (Multi_requested_damage_amount != 0)
			{
				if ((Netgame.local_role == LR_CLIENT) && (Netgame.flags & NF_PEER_PEER))
				{
					MultiSendRequestPeerDamage(&Objects[Players[Player_num].objnum], -1, Multi_requested_damage_type, Multi_requested_damage_amount);
				}
				else
				{
					MultiSendRequestDamage(Multi_requested_damage_type, Multi_requested_damage_amount);
				}
				Multi_requested_damage_amount = 0;
			}

			// Request shields if need be
			for (i = 0; i < MAX_SHIELD_REQUEST_TYPES; i++)
			{
				if (Multi_additional_shields[i] != 0)
				{
					MultiSendRequestShields(i, Multi_additional_shields[i]);
					Multi_additional_shields[i] = 0;
				}
			}

			ubyte data[MAX_GAME_DATA_SIZE], count = 0, add_count = 0;

			count = MultiStuffPosition(Player_num, data);

			// Send firing if needed
			if (Player_fire_packet[Player_num].fired_on_this_frame == PFP_FIRED)
				add_count = MultiStuffPlayerFire(Player_num, &data[count]);

			count += add_count;

			// Add in guided stuff
			if (Players[Player_num].guided_obj != NULL)
			{
				add_count = MultiStuffGuidedInfo(Player_num, &data[count]);
				count += add_count;
			}

			ASSERT(count < MAX_GAME_DATA_SIZE);

			if (Netgame.flags & NF_PEER_PEER)
			{
				for (i = 0; i < MAX_NET_PLAYERS; i++)
				{
					if (!(NetPlayers[i].flags & NPF_CONNECTED))
						continue;
					if (i == Player_num)
						continue;

					//Now get the ping time if needed
					if ((timer_GetTime() - NetPlayers[i].last_ping_time) > MULTI_PING_INTERVAL)
						MultiSendPing(i);

					nw_Send(&NetPlayers[i].addr, data, count, 0);
				}
			}
			else
				nw_Send(&Netgame.server_address, data, count, 0);

			// TODO: SEND RELIABLE WEAPON FIRE
			if (Player_fire_packet[Player_num].fired_on_this_frame == PFP_FIRED_RELIABLE)
			{
				//mprintf((0,"I NEED TO SEND RELIABLE FIRE\n"));
				count = MultiStuffPlayerFire(Player_num, data);
				nw_SendReliable(NetPlayers[Player_num].reliable_socket, data, count, true);
			}

			// Clear our data
			Player_fire_packet[Player_num].fired_on_this_frame = PFP_NO_FIRED;
			Multi_last_sent_time[Player_num][0] = 0;
			Players[Player_num].flags &= ~PLAYER_FLAGS_SEND_MOVEMENT;
		}

		if (ServerTimedOut())
		{
			mprintf((0, "Server disconnected!\n"));
			MultiLeaveGame();
			ShowProgressScreen(TXT_MLTDISCFRMSERV);
			Sleep(2000);
		}
	}
}

// Initializes some fields for a network client to play
// Client only
void MultiStartClient(char* scriptname)
{
	int i;

	for (i = 0; i < MAX_NET_PLAYERS; i++)
	{
		memset(&NetPlayers[i], 0, sizeof(netplayer));
	}

	for (i = 0; i < MAX_SAVED_MOVES; i++)
		SavedMoves[i].timestamp = 0;

	Current_saved_move = 0;

	// Temporary name fix
	Current_pilot.get_name(Players[Player_num].callsign);

	Multi_send_size[Player_num] = 0;
	Multi_last_sent_time[Player_num][0] = 0;
	Last_weapons_load_update_time = 0;

	Netgame.local_role = LR_CLIENT;
	Game_mode = GM_NETWORK;
	SetGamemodeScript(scriptname);
}

