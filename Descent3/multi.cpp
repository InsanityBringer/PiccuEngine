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

#include <algorithm>
#include "pstypes.h"
#include "pserror.h"
#include "player.h"
#include "game.h"
#include "multi.h"
#include "multi_client.h"
#include "multi_server.h"
#include "ddio.h"
#include "hud.h"
#include "robotfire.h"
#include "ship.h"
#include "descent.h"
#include "damage.h"
#include "gamesequence.h"
#include "objinfo.h"
#include "gametexture.h"
#include "room.h"
#include "game2dll.h"
#include "viseffect.h"
#include "hlsoundlib.h"
#include "soundload.h"
#include "fireball.h"
#include "Mission.h"
#include "LoadLevel.h"
#include "gamecinematics.h"
#include "init.h"

#include "sounds.h"
#include "weapon.h"
#include "stringtable.h"

#include "dedicated_server.h"
#include "demofile.h"
#include "args.h"

#include "ui.h"
#include "newui.h"
#include "multi_dll_mgr.h"
#include "BOA.h"
#include "attach.h"
#include "mission_download.h"
#include "multi_world_state.h"
#include "ObjScript.h"
#include "audiotaunts.h"
#include "marker.h"
#include "weather.h"
#include "doorway.h"
#include "object_lighting.h"
#include "spew.h"
#include "PHYSICS.H"
#include "SmallViews.h"
#include "demofile.h"
#include "debuggraph.h"
#include "levelgoal.h"
#include "osiris_share.h"
#include "cockpit.h"
#include "hud.h"
#include "gamespy.h"


#include <string.h>
#include <memory.h>
#include <stdlib.h>

#include "psrand.h"

#include "../md5/md5.h"
void MultiProcessShipChecksum (MD5 *md5, int ship_index);

player_pos_suppress Player_pos_fix[MAX_PLAYERS];

// Define this if you want to have secondaries be sent as reliable packets 
//(not recommended - talk to Jason)
//#define RELIABLE_SECONDARIES

//If this is true, PXO games won't save the kills, deaths, etc.
bool Multi_no_stats_saved = false;

unsigned int Netgame_curr_handle = 1;

ushort Local_object_list[MAX_OBJECTS];
ushort Server_object_list[MAX_OBJECTS];
ushort Server_spew_list[MAX_SPEW_EFFECTS];

#ifndef RELEASE
int Multi_packet_tracking[255];
#endif

// This is for clearing lightmapped objects on the client/server
int Num_client_lm_objects,Num_server_lm_objects;
ushort Client_lightmap_list[MAX_OBJECTS],Server_lightmap_list[MAX_OBJECTS];

// This is for breakable glass
ushort Broke_glass_rooms[MAX_BROKE_GLASS],Broke_glass_faces[MAX_BROKE_GLASS];
int Num_broke_glass=0;

// This is for getting out a menu if in multiplayer 
bool Multi_bail_ui_menu=false;

uint Multi_generic_match_table[MAX_OBJECT_IDS];
uint Multi_weapon_match_table[MAX_WEAPONS];
ubyte Multi_receive_buffer[MAX_RECEIVE_SIZE];

ubyte Multi_send_buffer[MAX_NET_PLAYERS][MAX_GAME_DATA_SIZE];
int Multi_send_size[MAX_NET_PLAYERS];

player_fire_packet Player_fire_packet[MAX_NET_PLAYERS];
float Multi_last_sent_time[MAX_NET_PLAYERS][MAX_NET_PLAYERS];

ubyte Multi_reliable_send_buffer[MAX_NET_PLAYERS][MAX_GAME_DATA_SIZE];
int Multi_reliable_send_size[MAX_NET_PLAYERS];
float Multi_reliable_last_send_time[MAX_NET_PLAYERS];
ubyte Multi_reliable_sent_position[MAX_NET_PLAYERS];
ubyte Multi_reliable_urgent[MAX_NET_PLAYERS];

// For keeping track of buildings that have changed
ubyte Multi_building_states[MAX_OBJECTS];
ushort Multi_num_buildings_changed=0;

// For keeping track of powerup respawn points
powerup_respawn Powerup_respawn[MAX_RESPAWNS];
powerup_timer Powerup_timer[MAX_RESPAWNS];

// For keeping track of damage and shields
int Multi_additional_damage_type[MAX_PLAYERS];
float Multi_additional_damage[MAX_PLAYERS];
int Multi_requested_damage_type=PD_NONE;
float Multi_requested_damage_amount=0;
float Multi_additional_shields[MAX_SHIELD_REQUEST_TYPES];

int Num_powerup_respawn=0;
int Num_powerup_timer=0;

// For level sequencing 
int Multi_next_level=-1;

netplayer NetPlayers[MAX_NET_PLAYERS];
netgame_info Netgame;

int Num_network_games_known=0;
network_game Network_games[MAX_NETWORK_GAMES];

int Game_is_master_tracker_game=0;

char Tracker_id[TRACKER_ID_LEN];

vmt_descent3_struct MTPilotinfo[MAX_NET_PLAYERS];


short Multi_kills[MAX_NET_PLAYERS];
short Multi_deaths[MAX_NET_PLAYERS];

int Got_new_game_time = 0;

#define MAX_COOP_TURRETS 400
float turret_holder[MAX_COOP_TURRETS];

#define DATA_CHUNK_SIZE	450

int Bandwidth_throttle = 0;

int Use_file_xfer = 1;

int Reliable_count=0;
int Last_reliable_count=0;

float Time_last_taunt_request = 0.0f;

// Display logos or not?
bool Multi_logo_state=true;

bool Multi_Expect_demo_object_flags = false;

// Heartbeat flag
bool Got_heartbeat=false;

// Message of the day
char Multi_message_of_the_day[HUD_MESSAGE_LENGTH*2]={0};

//Local function prototypes
void SendDataChunk(int playernum);
void DenyFile(int playernum,int filenum,int file_who);
void MultiDoFileCancelled(ubyte *data);
void MultiDoCustomPlayerData(ubyte *data);
char * GetFileNameFromPlayerAndID(short playernum,short id);

// Multiplayer position flags
#define MPF_AFTERBURNER	1			// Afterburner is on
#define MPF_OUTSIDE		2			// Player is outside
#define MPF_DEAD			4			// Player is dead
#define MPF_FIRED			8			// Player fired 
#define MPF_SPRAY			16			// Player is using a spray weapon
#define MPF_ON_OFF		32			// Player is using an on/off weapon
#define MPF_HEADLIGHT	64			// Headlight is on for this player
#define MPF_THRUSTED		128		// Player thrusted this frame


#define MULTI_ASSERT(x,m) { if ((x)==0) { BailOnMultiplayer((m)); return; } }
#define MULTI_ASSERT_NOMESSAGE(x) { if ((x)==0) { BailOnMultiplayer(NULL); return; } }
										
// Gets us out of a multiplayer game in a hurry...useful for when things are screwed
void BailOnMultiplayer (char *message)
{
	Int3();	// Get Jason

	// Show message
	if (message==NULL)
		ShowProgressScreen (TXT_MULTI_DATACORRUPT);
	else
		ShowProgressScreen (message);

	MultiLeaveGame();
	Sleep (2000);
}

// Adds the trunctuated position data to an outgoing packet
void MultiAddPositionData (vector *pos,ubyte *data,int *count)
{
	MultiAddUshort ((pos->x*16.0),data,count);
	MultiAddUshort ((pos->z*16.0),data,count);
	MultiAddFloat (pos->y,data,count);
}
void MultiExtractPositionData (vector *vec,ubyte *data,int *count)
{
	vec->x=((float)MultiGetUshort (data,count))/16.0;
	vec->z=((float)MultiGetUshort (data,count))/16.0;
	vec->y=MultiGetFloat (data,count);
}

void MultiSendBadChecksum(int slot)
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int size;
	int count=0;
	size=START_DATA (MP_REJECTED_CHECKSUM,data,&count);
	END_DATA (count,data,size);
	nw_SendReliable (NetPlayers[slot].reliable_socket,data,count);
}

// Puts player "slot" position info into the passed in buffer
// Returns the number of bytes used
int MultiStuffPosition (int slot,ubyte *data)
{
	int size;
	int count=0;
	ubyte flags=0;
	
	object *obj=&Objects[Players[slot].objnum];

	size=START_DATA (MP_PLAYER_POS,data,&count);
	MultiAddByte (slot,data,&count);

	if (slot==Player_num)
	{
		// tell the reciever to expect fire information, only if
		// we fired this frame and it isn't going to be a seperate packet
		// (like secondaries in a non-peer to peer, are sent reliably)
		if (Player_fire_packet[slot].fired_on_this_frame==PFP_FIRED)
			flags|=MPF_FIRED;
	}

	// Send timestamp
	if (slot==Player_num)
		MultiAddFloat (Gametime,data,&count);
	else
		MultiAddFloat (NetPlayers[slot].packet_time,data,&count);
		
	// Do position
	MultiAddPositionData (&obj->pos,data,&count);
	
	// Do orientation
	angvec angs;
	vm_ExtractAnglesFromMatrix (&angs,&obj->orient);

	MultiAddShort (angs.p,data,&count);
	MultiAddShort (angs.h,data,&count);
	MultiAddShort (angs.b,data,&count);

	// Do roomnumber and terrain flag
	
	MultiAddShort (CELLNUM (obj->roomnum),data,&count);

	
	// Fill flags
	if (OBJECT_OUTSIDE(obj))
		flags |=MPF_OUTSIDE;
	if (Players[slot].flags & PLAYER_FLAGS_AFTERBURN_ON)
		flags |=MPF_AFTERBURNER;
	if (Players[slot].flags & PLAYER_FLAGS_THRUSTED)
		flags |=MPF_THRUSTED;
	if (Objects[Players[slot].objnum].weapon_fire_flags & WFF_SPRAY)
		flags |=MPF_SPRAY;
	if (Objects[Players[slot].objnum].weapon_fire_flags & WFF_ON_OFF)
		flags |=MPF_ON_OFF;
	if (Players[slot].flags & PLAYER_FLAGS_DEAD)// || Players[slot].flags & PLAYER_FLAGS_DYING)
		flags |=MPF_DEAD;
	if (Players[slot].flags & PLAYER_FLAGS_HEADLIGHT)
		flags |=MPF_HEADLIGHT;
	
	// Do velocity
	vector *vel=&obj->mtype.phys_info.velocity;
	vector *rotvel=&obj->mtype.phys_info.rotvel;

//	mprintf ((0,"Outvel x=%f y=%f z=%f\n",vel->x,vel->y,vel->z));
	
	MultiAddShort ((vel->x*128.0),data,&count);
	MultiAddShort ((vel->y*128.0),data,&count);
	MultiAddShort ((vel->z*128.0),data,&count);

	if (Netgame.flags & NF_SENDROTVEL)
	{
		MultiAddShort (rotvel->x/4,data,&count);
		MultiAddShort (rotvel->y/4,data,&count);
		MultiAddShort (rotvel->z/4,data,&count);

		MultiAddShort (obj->mtype.phys_info.turnroll,data,&count);
	}

	int weapon_indices=((Players[slot].weapon[PW_SECONDARY].index-10)<<4)|Players[slot].weapon[PW_PRIMARY].index;
	MultiAddByte (weapon_indices,data,&count);

	MultiAddUbyte (Players[slot].energy,data,&count);
	MultiAddByte (flags,data,&count);
	
		
	END_DATA (count,data,size);

	return count;
}

void DoNextPlayerFile(int playernum)
{
	NetPlayers[playernum].file_xfer_flags = NETFILE_NONE;
	NetPlayers[playernum].custom_file_seq++;
	if(NetPlayers[playernum].custom_file_seq > NETFILE_ID_LAST_FILE)
	{
		NetPlayers[playernum].custom_file_seq = NETFILE_ID_DONE;
		mprintf((0,"Setting %s's custom ship logo to %s\n",Players[playernum].callsign,NetPlayers[playernum].ship_logo));
		PlayerSetCustomTexture(playernum,NetPlayers[playernum].ship_logo);
		//If we are the server, we need to tell everyone about this guy's custom data
		if((Netgame.local_role==LR_SERVER)&&(Use_file_xfer))
		{
			for (int i=0;i<MAX_PLAYERS;i++)
			{
				//Send custom ship info if we have it
				if((NetPlayers[i].custom_file_seq==NETFILE_ID_DONE)||(i==Player_num))
				{
					MultiSendClientCustomData(i,playernum);
				}
			}
			MultiSendClientCustomData(playernum);
		}
	}

}

// Puts player "slot" position info into the passed in buffer
// Returns the number of bytes used
int MultiStuffRobotPosition (unsigned short objectnum,ubyte *data)
{
	int size;
	int count=0;
	//@@multi_orientation mat;
	
	//mprintf((0,"!"));//KBTEST
	object *obj=&Objects[objectnum];

	
	ASSERT (obj->flags & OF_CLIENT_KNOWS);

	size=START_DATA (MP_ROBOT_POS,data,&count);
	MultiAddUshort (objectnum,data,&count);

	// Do position
	MultiAddPositionData (&obj->pos,data,&count);
	
	// Do orientation
	
	// Do orientation
	angvec angs;
	vm_ExtractAnglesFromMatrix (&angs,&obj->orient);

	MultiAddShort (angs.p,data,&count);
	MultiAddShort (angs.h,data,&count);
	MultiAddShort (angs.b,data,&count);

	// Do roomnumber and terrain flag
	
	MultiAddShort (CELLNUM (obj->roomnum),data,&count);

	if (OBJECT_OUTSIDE(obj))
		MultiAddByte (1,data,&count);
	else
		MultiAddByte (0,data,&count);
	
	vector *vel=&obj->mtype.phys_info.velocity;
	vector *rotvel=&obj->mtype.phys_info.rotvel;

	MultiAddShort ((vel->x*128.0),data,&count);
	MultiAddShort ((vel->y*128.0),data,&count);
	MultiAddShort ((vel->z*128.0),data,&count);

	END_DATA (count,data,size);
	return count;
}

// Puts player "slot" position info into the passed in buffer
// Returns the number of bytes used
int MultiSendRobotFireWeapon (unsigned short objectnum,vector *pos,vector *dir,unsigned short weaponnum)
{
	int size;
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	//mprintf((0,"Sending Robot %d fired.\n",objectnum));
	// Check to make sure we're the server
	if (Netgame.local_role!=LR_SERVER)
	{	
		BailOnMultiplayer(NULL);
		return 0;
	}
	
	//mprintf((0,"@"));//KBTEST
	object *obj=&Objects[objectnum];
	ASSERT (obj->flags & OF_CLIENT_KNOWS);

	size=START_DATA (MP_ROBOT_FIRE,data,&count);
	MultiAddUshort (objectnum,data,&count);
	
	// Do position
	//memcpy (&data[count],pos,sizeof(vector));
	//count+=sizeof(vector);
	MultiAddVector(*pos,data,&count);

	// Do orientation
	//memcpy (&data[count],dir,sizeof(vector));
	//count+=sizeof(vector);
	MultiAddVector(*dir,data,&count);

	uint index=MultiGetMatchChecksum (OBJ_WEAPON,weaponnum);
	MultiAddUint (index,data,&count);
	
	END_DATA (count,data,size);
	//Add it to the outgoing queue. The MultiSendRobotFireSound() function will actually send it
	for (int i=0;i<MAX_NET_PLAYERS;i++)
	{	
		if (i==Player_num)	
			continue;

		if ((NetPlayers[i].flags & NPF_CONNECTED)&&(NetPlayers[i].sequence==NETSEQ_PLAYING))
		{
			if (Multi_send_size[i]+count>=MAX_GAME_DATA_SIZE)
				MultiSendFullPacket (i,0);

			memcpy (&Multi_send_buffer[i][Multi_send_size[i]],data,count);
			Multi_send_size[i]+=count;
			//Don't send because we are waiting for the robot fire sound to send
		}
	}
	return count;
}

void MultiDoRobotFire(ubyte *data)
{
	int count = 0;
	unsigned short obj_num;
	vector weapon_pos;
	vector weapon_dir;
	unsigned int weapon_num;
	uint uniqueid;
	
	if (Netgame.local_role==LR_SERVER)
	{
		Int3();	// Get Jason, a server got the multi do fire packet
		return;
	}

	//mprintf((0,"*"));
	SKIP_HEADER (data,&count);
	int serv_objnum=MultiGetUshort (data,&count);
	obj_num = Server_object_list[serv_objnum];

	if (obj_num==65535 || !(Objects[obj_num].flags & OF_SERVER_OBJECT))
	{
		//Int3(); // Get Jason, invalid robot firing!
		return;
	}

	//memcpy (&weapon_pos,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	weapon_pos = MultiGetVector(data,&count);
	//memcpy (&weapon_dir,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	weapon_dir = MultiGetVector(data,&count);

	weapon_num = MultiGetUint(data,&count);

	if(Objects[obj_num].type==255)
		return;
	uniqueid = MultiMatchWeapon(weapon_num);

	CreateAndFireWeapon( &weapon_pos, &weapon_dir, &Objects[obj_num], uniqueid);

}

// Does a cool vis effect to announce a player or powerup
void MultiAnnounceEffect(object *obj,float size,float time)
{
	int visnum=VisEffectCreate (VIS_FIREBALL,BLAST_RING_INDEX,obj->roomnum,&obj->pos);
	if (visnum>=0)
	{
		vector norm={0,1,0};
		vis_effect *vis=&VisEffects[visnum];
		vis->size=size;
		vis->lifetime=time;
		vis->lifeleft=time;
		vis->custom_handle=Fireballs[BLUE_BLAST_RING_INDEX].bm_handle;
		vis->flags|=VF_PLANAR|VF_REVERSE;
		vis->end_pos=norm;
	}
}

// MultiProcessIncoming reads incoming data off the unreliable and reliable ports and sends
// the data to process_big_data
void MultiProcessIncoming()
{
	int size, i;
	ubyte *data;
	network_address from_addr;	

	data = &(Multi_receive_buffer[0]);

	// get the other net players data
	while( (size = nw_Receive(data, &from_addr))>0 )	
	{
		MultiProcessBigData(data, size, &from_addr);
		if(ServerTimeout)
		{
			LastPacketReceived = timer_GetTime();
		}
	} // end while
	
	// read reliable sockets for data
	data = &(Multi_receive_buffer[0]);
	if(Netgame.local_role==LR_SERVER)
	{
		for (i=0;i<MAX_NET_PLAYERS;i++) 
		{
			if (Player_num==i)
				continue;

			if((NetPlayers[i].flags & NPF_CONNECTED) && (NetPlayers[i].reliable_socket != INVALID_SOCKET) 
			&& (NetPlayers[i].reliable_socket != 0))
			{

				while( (size = nw_ReceiveReliable(NetPlayers[i].reliable_socket, data, MAX_RECEIVE_SIZE)) > 0)
				{
					MultiProcessBigData(data,size,&NetPlayers[i].addr);
				}
			}
		}
	} 
	else 
	{
		// if I'm not the master of the game, read reliable data from my connection with the server
		if((NetPlayers[Player_num].reliable_socket != INVALID_SOCKET) && (NetPlayers[Player_num].reliable_socket != 0))
		{
			while( (size = nw_ReceiveReliable(NetPlayers[Player_num].reliable_socket,data, MAX_RECEIVE_SIZE)) > 0)
			{				
				MultiProcessBigData(data,size,&Netgame.server_address);
			}
		}
	}
}

// Starts a packet of data
int START_DATA (int type,ubyte *data,int *count,ubyte reliable)
{
	int size_offset;

	MultiAddByte (type,data,count);

	// Do size
	size_offset=*count;
	MultiAddShort (0,data,count);

/*
	#ifndef RELEASE

	if (Netgame.local_role==LR_SERVER)
	{
		MultiAddByte (reliable,data,count);
		if (reliable)
		{	
			MultiAddInt (Reliable_count,data,count);
			Reliable_count++;
		}
	}
	else
		MultiAddByte (0,data,count);

	#endif
*/
	return size_offset;
}


// End a pakcet of data
void END_DATA (int count,ubyte *data,int offset)
{
	MultiAddShort (count,data,&offset);
}

// Skips the header stuff at the beginning of a packet
void SKIP_HEADER (ubyte *data,int *count)
{
	MultiGetByte (data,count);
	MultiGetShort (data,count);
}

// called once a frame on client and server.  NOTE: only servers (game masters) check for
// listen connections.
void MultiDoFrame()
{
	if (!(Game_mode & GM_MULTI))
		return;

	ScoreAPIFrameInterval();

	static int debug_id = -2;
	static float last_game_time = 99999999.9f;

	CallGameDLL (EVT_GAME_INTERVAL,&DLLInfo);

	if (Netgame.local_role==LR_SERVER)
		MultiDoServerFrame ();
	else
	{
		if(debug_id==-2)
		{
			debug_id = DebugGraph_Add(0.0f,10000.0f,"Ping",DGF_MULTIPLAYER);
		}
		if(debug_id>=0)
		{			
			if(last_game_time>Gametime)
			{
				//reset last_game_time, we must have started a new level
				last_game_time = Gametime;
			}

			float next_time = last_game_time + 0.3f;	//update every 1/3 second
			if(Gametime>=next_time)
			{
				last_game_time = Gametime;
				if(NetPlayers[Player_num].flags&NPF_CONNECTED && NetPlayers[Player_num].sequence==NETSEQ_PLAYING)
				{				
					DebugGraph_Update(debug_id,NetPlayers[Player_num].ping_time*1000.0f);
				}
			}
		}

		MultiDoClientFrame();
		// Verify object list
	}
}

// Returns to the slot number of the player that the passed in address belongs to
int MultiMatchPlayerToAddress (network_address *from_addr)
{
	int i;
	for (i=0;i<MAX_NET_PLAYERS;i++)
	{
		if (NetPlayers[i].flags & NPF_CONNECTED)
		{
			if (!(memcmp (from_addr,&NetPlayers[i].addr,sizeof(network_address))))
				return i;
		}
		
	}
	//mprintf ((0,"Got a packet from unconnected player?!\n"));
	return -1;
}

// Gets info about a player
// Server only
extern int Buddy_handle[MAX_PLAYERS];
bool AINotify(object *obj, ubyte notify_type, void *info);
void MultiDoMyInfo (ubyte *data)
{
	int count=0;
	char ship_name[PAGENAME_LEN];
	int length;

	// Skip header stuff
	SKIP_HEADER (data,&count);
	

	ubyte slot=MultiGetByte (data,&count);
	if (!(NetPlayers[slot].flags & NPF_CONNECTED))
	{
		mprintf ((1,"ERROR!  We got a MY_INFO packet from a disconnected player!\n"));
		Int3();	// Get Jason
		return;
	}

	// Copy the name out 
	ubyte len=MultiGetByte (data,&count);
	if (len > CALLSIGN_LEN)
		length = CALLSIGN_LEN;
	else
		length = len;

	memcpy (Players[slot].callsign,&data[count],length);
	count+=len;

	// Copy the ship name out 
	len=MultiGetByte (data,&count);
	memcpy (ship_name,&data[count],len);
	count+=len;

	int ship_index=FindShipName (ship_name);
	if (ship_index<0)
		ship_index=0;

	if(!PlayerIsShipAllowed(slot,ship_name))
	{
		bool found_one=false;
		int i;

		//Loop through ships, looking for one that's allowed
		for (i=0;i<MAX_SHIPS && !found_one;i++)
		{
			if(Ships[i].used && PlayerIsShipAllowed(Player_num,i))
			{
				Players[Player_num].ship_index=i;
				MultiBashPlayerShip(slot,Ships[i].name);
				found_one=true;
			}
		}

		//We should have found one
		ASSERT(found_one == true);
	}

	PlayerChangeShip (slot,ship_index);
		
	Players[slot].time_in_game = timer_GetTime();
	if(Game_is_master_tracker_game)	
	{
		unsigned int mt_sig;

		Players[slot].kills = 0;
		Players[slot].deaths = 0;
		Players[slot].suicides = 0;
		Players[slot].lateral_thrust = 0;
		Players[slot].rotational_thrust = 0;
				
		mt_sig = MultiGetUint(data,&count);

		if(mt_sig != MASTER_TRACKER_SIG)
		{
			NetPlayers[slot].flags &= ~NPF_CONNECTED;
			mprintf ((0,"Invalid master tracker signature! Someone tried to join a master tracker game through the local net screen!\n"));
		}
		NetPlayers[slot].flags |= NPF_MT_READING_PILOT;
		len=MultiGetByte (data,&count);
		memcpy (Players[slot].tracker_id,&data[count],len);
		count+=len;
		strcpy(MTPilotinfo[slot].tracker_id,Players[slot].tracker_id);
		strcpy(MTPilotinfo[slot].pilot_name,Players[slot].callsign);
	}
	
	Multi_deaths[slot] = 0;
	Multi_kills[slot] = 0;
	//Request pilot info from the mastertracker
	int ser = 0;
	ser = MultiGetInt(data,&count);
	if(ser)
	{
		mprintf((0,"Serialized user joining game: %d\n",ser));
	}

	// Get packets per second for this player
	NetPlayers[slot].pps=MultiGetByte (data,&count);

	if (NetPlayers[slot].pps<2)
		NetPlayers[slot].pps=2;
	if (NetPlayers[slot].pps>Netgame.packets_per_second)
		NetPlayers[slot].pps=Netgame.packets_per_second;

	//pilot picture id
	NetPlayers[slot].pilot_pic_id = MultiGetUshort(data,&count);

	// Copy the guidebot name out 
	char guidebot_name[32];
	memset(guidebot_name,0,32);
	len=MultiGetByte (data,&count);
	ASSERT(len<32);
	memcpy (guidebot_name,&data[count],len);
	count+=len;

	//Set the guidebot name
	if(Netgame.flags & NF_ALLOWGUIDEBOT)
	{
		gb_com command;
		command.action = COM_DO_ACTION;
		command.index = 44;//GBC_RENAME_SILENT
		command.ptr = (void *) guidebot_name;
		object *b_obj = ObjGet(Buddy_handle[slot]);
		ASSERT(b_obj);
		AINotify(b_obj, AIN_USER_DEFINED, (void *)&command);
	}

	NetPlayers[slot].sequence=NETSEQ_NEED_GAMETIME;	

	mprintf ((0,"Got a myinfo packet from %s len=%d!\n",Players[slot].callsign,len));
}

// Tell a client about the players connected
// Server only
void MultiDoRequestPlayers (ubyte *data)
{
	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	if (!(NetPlayers[slot].flags & NPF_CONNECTED))
	{
		mprintf ((1,"ERROR!  We got a MY_INFO packet from a disconnected player!\n"));
		Int3();	// Get Jason
		return;
	}

	NetPlayers[slot].sequence=NETSEQ_REQUEST_PLAYERS;
}

// Tell a client about the buildings
// Server only
void MultiDoRequestBuildings (ubyte *data)
{
	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	if (!(NetPlayers[slot].flags & NPF_CONNECTED))
	{
		mprintf ((1,"ERROR!  We got a request buildings packet from a disconnected player!\n"));
		Int3();	// Get Jason
		return;
	}

	NetPlayers[slot].sequence=NETSEQ_REQUEST_BUILDINGS;
}


// Tell a client about the objects
// Server only
void MultiDoRequestObjects (ubyte *data)
{
	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	if (!(NetPlayers[slot].flags & NPF_CONNECTED))
	{
		mprintf ((1,"ERROR!  We got a request object packet from a disconnected player!\n"));
		Int3();	// Get Jason
		return;
	}

	NetPlayers[slot].sequence=NETSEQ_REQUEST_OBJECTS;
}

// Tell a client about the objects
// Server only
void MultiDoRequestWorldStates (ubyte *data)
{
	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);
	ubyte slot=MultiGetByte (data,&count);

	ubyte rxdigest[16];
	for(int i=0;i<16;i++)
	{
		rxdigest[i] = MultiGetByte (data,&count);
	}

	char dbgmsg[1024];
	char expectedstr[50] = "";
	char gotstr[50] = "";
	for(int j=0;j<16;j++)
	{
		char bstr[10];
		sprintf(bstr,"%.2x",rxdigest[j] & 0xff);
		strcat(gotstr,bstr);
		sprintf(bstr,"%.2x",NetPlayers[slot].digest[j] & 0xff);
		strcat(expectedstr,bstr);
	}
	sprintf(dbgmsg,"Expected %s for the digest, got %s for player %d\n",expectedstr,gotstr,slot);

	for(int i=0;i<16;i++)
	{
		// We got the digest from the player previously. Now see if it matches.
		if(rxdigest[i]!=NetPlayers[slot].digest[i])
		{
			// send 'bad checksum message.'
			MultiSendBadChecksum(slot);
			
			// Bad checksum, kick the player out.
			NetPlayers[slot].sequence=NETSEQ_PREGAME;
			NetPlayers[slot].flags |= NPF_CONNECTED;
			return;
		}
	}
	if (!(NetPlayers[slot].flags & NPF_CONNECTED))
	{
		mprintf ((1,"ERROR!  We got a request world states packet from a disconnected player!\n"));
		Int3();	// Get Jason
		return;
	}

	NetPlayers[slot].sequence=NETSEQ_REQUEST_WORLD;
}

// The server is telling me about a player in the game
// Client only
void MultiDoPlayer (ubyte *data)
{
	int count=0;
	char ship_name[PAGENAME_LEN];
	
	mprintf ((0,"Got player packet!\n"));

	// Skip header stuff
	SKIP_HEADER (data,&count);

	// Get slotnumber
	ubyte slot=MultiGetByte (data,&count);

	// Reset some local stuff for this player
	if (slot!=Player_num)
	{
		InitPlayerNewShip (slot,INVRESET_ALL);
		InitPlayerNewGame (slot);
		PlayerStopSounds(slot);
		ResetPlayerObject (slot);
		InitPlayerNewLevel (slot);
	}

	// Get name
	ubyte len=MultiGetByte (data,&count);
	memcpy (Players[slot].callsign,&data[count],len);
	count+=len;

	// Get ship name
	len=MultiGetByte (data,&count);
	memcpy (ship_name,&data[count],len);
	count+=len;

	int ship_index=FindShipName (ship_name);
	if (ship_index<0)
	{
		ship_index=0;	// Should we bail out here?
	}

	PlayerChangeShip (slot,ship_index);

	// Get various flags
	if (slot!=Player_num)
	{
		Players[slot].flags=MultiGetInt (data,&count);
		Players[slot].weapon_flags=MultiGetInt (data,&count);
	}
	else
	{
		// Skip my own data
		MultiGetInt (data,&count);
		MultiGetInt (data,&count);
	}

	NetPlayers[slot].flags=MultiGetInt (data,&count);
	float shields=MultiGetFloat (data,&count);
	
	// Get object number, just to verify
	int objnum=MultiGetShort (data,&count);

	// Make sure we have the right object number
	if (objnum!=Players[slot].objnum)
	{
		BailOnMultiplayer (NULL);
		return;
	}

	// Get team
	ubyte temp_team = MultiGetByte (data,&count);
	Players[slot].team = (temp_team==255)?-1:temp_team;

	// Get team start
	if (slot==Player_num)
	{
		Players[slot].start_index=MultiGetShort (data,&count);
		PlayerMoveToStartPos (slot,Players[slot].start_index);
	}

	// Get observer mode
	ubyte observing=MultiGetByte (data,&count);
	
	//Put in address
	memcpy(&NetPlayers[slot].addr,data+count,sizeof(network_address));
#ifdef MACINTOSH //DAJ if the addr is comming from a PC swap it
	if(NetPlayers[slot].addr.connection_type > NP_DIRECTPLAY) {
		NetPlayers[slot].addr.port = SWAPSHORT(NetPlayers[slot].addr.port);
		NetPlayers[slot].addr.connection_type = (network_protocol)SWAPINT(NetPlayers[slot].addr.connection_type);
	}
#endif
	char dbg_output[50];
	nw_GetNumbersFromHostAddress(&NetPlayers[slot].addr,dbg_output);
	mprintf((0,"Got player address of: %s\n", dbg_output));
	
	count += sizeof(network_address);
	
	// PPS
	NetPlayers[slot].pps=MultiGetByte (data,&count);

	//pilot picture id
	NetPlayers[slot].pilot_pic_id = MultiGetUshort(data,&count);

	// Get ranking
	Players[slot].rank=MultiGetFloat (data,&count);

	// Get tracker id
	if(Game_is_master_tracker_game)	
	{
		int len=MultiGetByte (data,&count);
		memcpy (Players[slot].tracker_id,&data[count],len);
		count+=len;
	}

	object *obj=&Objects[objnum];
	obj->id=slot;

	MultiMakePlayerReal (slot);
		
	if (Player_num!=slot)
	{
		NetPlayers[slot].sequence=NETSEQ_PLAYING;
		NetPlayers[slot].flags |= NPF_CONNECTED;
	}

	if (slot!=Player_num)
	{
		obj->shields=shields;
		if (Players[slot].flags & PLAYER_FLAGS_DEAD)
		{
			mprintf ((0,"Incoming %d player is dead!\n",slot));
			MultiMakePlayerGhost (slot);
		}
		else if (Players[slot].flags & PLAYER_FLAGS_DYING)
		{
			mprintf ((0,"Incoming %d player is dying!\n",slot));
			int save_flags=Players[slot].flags;
			Players[slot].flags &=~(PLAYER_FLAGS_DEAD|PLAYER_FLAGS_DYING);
			InitiatePlayerDeath (&Objects[Players[slot].objnum],false,0);
			Players[slot].flags=save_flags;
		}
	}

	obj->flags|=OF_SERVER_OBJECT;

	if (observing)
	{
		obj->render_type=RT_NONE;
		obj->type=OBJ_OBSERVER;
	}

	if (slot==0)
	{
		memcpy(&NetPlayers[slot].addr,&Netgame.server_address,sizeof(network_address));
		
	}

}

// Tells all our clients about a new player entering the game
void MultiSendNewPlayer (int slot)
{
	
}

void MultiDoPlayerEnteredGame (ubyte *data)
{
	int count=0;
	char ship_name[PAGENAME_LEN];
	int length;
	
	mprintf ((0,"Got player entered game packet!\n"));

	// Skip header stuff
	SKIP_HEADER (data,&count);

	// Get slot number for this player
	ubyte slot=MultiGetByte (data,&count);

	if(slot > MAX_PLAYERS)
		return;
		
	ScoreAPIPlayerJoin(slot);
	// get name
	ubyte len=MultiGetByte (data,&count);

	if (len > CALLSIGN_LEN)
		length = CALLSIGN_LEN;
	else
		length = len;
		
	memcpy (Players[slot].callsign,&data[count],length);
	count+=len;

	// get ship name
	len=MultiGetByte (data,&count);
	memcpy (ship_name,&data[count],len);
	count+=len;

	int ship_index=FindShipName (ship_name);
	if (ship_index<0)
		ship_index=0;

	// Get various flags
	Players[slot].flags=MultiGetInt (data,&count);
	NetPlayers[slot].flags=MultiGetInt (data,&count);

	// Verify object number
	int objnum=MultiGetShort (data,&count);

	MULTI_ASSERT (objnum==Players[slot].objnum,NULL);
	MULTI_ASSERT (Objects[objnum].id==slot,NULL);

	//Get the player's network address
	memcpy(&NetPlayers[slot].addr,data+count,sizeof(network_address));
#ifdef MACINTOSH //DAJ if the addr is comming from a PC swap it
	if(NetPlayers[slot].addr.connection_type > NP_DIRECTPLAY) {
		NetPlayers[slot].addr.port = SWAPSHORT(NetPlayers[slot].addr.port);
		NetPlayers[slot].addr.connection_type = (network_protocol)SWAPINT(NetPlayers[slot].addr.connection_type);
	}
#endif
	count += sizeof(network_address);

	// Get PPS
	NetPlayers[slot].pps=MultiGetByte (data,&count);
	
	//pilot picture id
	NetPlayers[slot].pilot_pic_id = MultiGetUshort(data,&count);

	Players[slot].start_index=MultiGetShort (data,&count);

	// Get ranking
	Players[slot].rank=MultiGetFloat (data,&count);

	// Get tracker id
	if(Game_is_master_tracker_game)	
	{
		int len=MultiGetByte (data,&count);
		memcpy (Players[slot].tracker_id,&data[count],len);
		count+=len;

		NetPlayers[slot].flags&=~NPF_WROTE_RANK;
	}
			
	if (Player_num!=slot && NetPlayers[Player_num].sequence==NETSEQ_PLAYING)
	{
		NetPlayers[slot].sequence=NETSEQ_PLAYING;
		NetPlayers[slot].flags |= NPF_CONNECTED;
	}

	InitPlayerNewShip (slot,INVRESET_ALL);
	InitPlayerNewGame (slot);
	PlayerStopSounds(slot);
	ResetPlayerObject (slot);
	InitPlayerNewLevel (slot);
	PlayerChangeShip (slot,ship_index);
	PlayerMoveToStartPos (slot,Players[slot].start_index);
	MultiMakePlayerReal (slot);
	Objects[objnum].flags|=OF_SERVER_OBJECT;

	MultiAnnounceEffect (&Objects[objnum],Objects[objnum].size*5,1.0);
	NetPlayers[slot].total_bytes_sent = 0;
	NetPlayers[slot].total_bytes_rcvd = 0;
	NetPlayers[slot].sequence=NETSEQ_PLAYING;
}

// Sends a new player to existing players 
// Sends to "slot" and describes player "which"
// Server only
void MultiSendPlayerEnteredGame (int which)
{
	MULTI_ASSERT (Netgame.local_role==LR_SERVER,NULL);

	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;

	size_offset=START_DATA(MP_PLAYER_ENTERED_GAME,data,&count,1);

	MultiAddByte (which,data,&count);
	
	// Add name
	int len=strlen(Players[which].callsign)+1;
	MultiAddByte (len,data,&count);
	memcpy (&data[count],Players[which].callsign,len);
	count+=len;

	// Add ship name
	len=strlen(Ships[Players[which].ship_index].name)+1;
	MultiAddByte (len,data,&count);
	memcpy (&data[count],Ships[Players[which].ship_index].name,len);
	count+=len;

	// Add flags
	MultiAddInt (Players[which].flags,data,&count);
	MultiAddInt (NetPlayers[which].flags,data,&count);
	
	// Just for safety sake send the object number
	MultiAddShort (Players[which].objnum,data,&count);

	//Add the player's network address.
	memcpy(data+count,&NetPlayers[which].addr,sizeof(network_address));
	count += sizeof(network_address);

	// Add pps
	MultiAddByte (NetPlayers[which].pps,data,&count);

	//pilot picture id
	MultiAddUshort (NetPlayers[which].pilot_pic_id,data,&count);


	// Add start position
	MultiAddShort (Players[which].start_index,data,&count);

	// Send ranking
	MultiAddFloat (Players[which].rank,data,&count);

	// Send tracker id
	if(Game_is_master_tracker_game)
	{
		int len = strlen(Players[which].tracker_id)+1;
		MultiAddByte (len,data,&count);
		memcpy (&data[count],Players[which].tracker_id,len);
		count+=len;
	}


	END_DATA (count,data,size_offset);

	// Send it out

	MultiSendReliablyToAllExcept (which,data,count,NETSEQ_REQUEST_BUILDINGS);
	MultiDoPlayerEnteredGame (data);	
}


// Client is saying that he's entering the game
// Server only
void MultiDoEnteringGame (ubyte *data)
{
	int count=0;
	
	// Skip header stuff
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);

	if (!(NetPlayers[slot].flags & NPF_CONNECTED))
	{
		Int3(); // Get Jason
		return;
	}

	mprintf ((0,"Client %d (%s) entering game.\n",slot,Players[slot].callsign));

	MultiSendPlayerEnteredGame (slot);

	//send the player the audio taunt delay time
	MultiSetAudioTauntTime(taunt_DelayTime(),slot);

	Level_goals.MultiSendChangedGoals(slot);

	//add his guidebot (if it is a guidebot game)
	//NOTE: It is possible that DMFC may InitPlayerNewShip after this
	//so it is placed as a precaution in their also
	if(Netgame.flags&NF_ALLOWGUIDEBOT)
	{
		if((!Players[slot].inventory.CheckItem(OBJ_ROBOT, ROBOT_GUIDEBOT)) && (ObjGet(Buddy_handle[slot])->type != OBJ_ROBOT))
			Players[slot].inventory.Add(OBJ_ROBOT, ROBOT_GUIDEBOT);
	}

	DLLInfo.me_handle=Objects[Players[slot].objnum].handle;
	DLLInfo.it_handle=Objects[Players[slot].objnum].handle;
	CallGameDLL (EVT_GAMEPLAYERENTERSGAME,&DLLInfo);

	// Clear vis info
	for (int i=0;i<MAX_OBJECTS;i++)
	{
		Objects[i].generic_nonvis_flags&=~(1<<slot);
		Objects[i].generic_sent_nonvis&=~(1<<slot);
	}

}

// Tell the server I'm entering the game
void MultiSendEnteringGame ()
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;

	mprintf ((0,"Sending entering game\n"));

	size=START_DATA (MP_ENTERING_GAME,data,&count);
	MultiAddByte (Player_num,data,&count);
	END_DATA (count,data,size);

	nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count);
}

// Tell my clients about damage done to a player
// Server only
void MultiSendDamagePlayer (int slot,int weapon_id,int type,float amount)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	MULTI_ASSERT (Netgame.local_role==LR_SERVER,NULL);
	
	size_offset=START_DATA(MP_DAMAGE_PLAYER,data,&count,1);
	MultiAddByte (slot,data,&count);
	MultiAddByte (weapon_id,data,&count);
	MultiAddByte (type,data,&count);
	MultiAddFloat (amount,data,&count);
	END_DATA (count,data,size_offset);

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYING,true);
}

// The server says to damage a player, so do it!
void MultiDoDamagePlayer (ubyte *data)
{	

	if (Netgame.local_role!=LR_CLIENT)
	{
		Int3();
		return;
	}
	
	int count=0;

	SKIP_HEADER (data,&count);
	ubyte slot=MultiGetByte (data,&count);
	ubyte weapon_id=MultiGetByte (data,&count);
	ubyte type = MultiGetByte (data,&count);
	float amount=MultiGetFloat (data,&count);

	ApplyDamageToPlayer (&Objects[Players[slot].objnum],NULL,type,amount,1,weapon_id);
}


// Makes the passed in player a ghost
void MultiMakePlayerGhost (int slot)
{
	object *obj=&Objects[Players[slot].objnum];


	MULTI_ASSERT (obj->id==slot,NULL);	// Get Jason
	
	obj->type=OBJ_GHOST;
	obj->movement_type=MT_NONE;
	obj->render_type=RT_NONE;
	obj->mtype.phys_info.flags|=PF_NO_COLLIDE;
	SetObjectControlType(obj, CT_NONE);

	if(Demo_flags==DF_RECORDING)
	{
		DemoWritePlayerTypeChange(slot);
	}

	// Stop sounds for this player
	PlayerStopSounds (slot);

}

// Makes the passed in player real (non-ghost)
void MultiMakePlayerReal (int slot)
{
	object *obj=&Objects[Players[slot].objnum];
	MULTI_ASSERT (obj->id==slot,NULL);	// Get Jason

	obj->type=OBJ_PLAYER;

	if(Demo_flags==DF_RECORDING)
	{
		DemoWritePlayerTypeChange(slot);
	}

	obj->render_type=RT_POLYOBJ;
	obj->mtype.phys_info.flags&=~PF_NO_COLLIDE;
	
	if (slot==Player_num)
	{
		SetObjectControlType(obj, CT_FLYING);
		obj->movement_type=MT_PHYSICS;
		Viewer_object=Player_object=obj;
	}
	else
	{
		obj->movement_type=MT_PHYSICS;
		obj->mtype.phys_info.flags=PF_FIXED_VELOCITY;
		SetObjectControlType(obj, CT_NONE);
	}

	
}

// Server is telling us that its done sending players
void MultiDoDonePlayers (ubyte *data)
{
	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);

	ubyte num=MultiGetByte (data,&count);
	if (MultiCountPlayers() !=num)
	{
		mprintf ((1,"ERROR!  We don't have the correct number of players!"));
		Int3();	// Get Jason
		return;
	}

	NetPlayers[Player_num].sequence=NETSEQ_REQUEST_BUILDINGS;
}


// Server is telling us that its done sending buildings
void MultiDoDoneBuildings (ubyte *data)
{
	MULTI_ASSERT (Netgame.local_role==LR_CLIENT,NULL);

	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);
	NetPlayers[Player_num].sequence=NETSEQ_REQUEST_OBJECTS;
}


// Server is telling us that its done sending objects
void MultiDoDoneObjects (ubyte *data)
{
	MULTI_ASSERT (Netgame.local_role==LR_CLIENT,NULL);

	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);
	// Get the salt.
	MD5 *playermd5 = Level_md5->Clone();
	// Get the salt value from the server.
	int salt = MultiGetInt (data,&count);
	// process the salt through the md5
	playermd5->MD5Update(salt);
	// process the ships through the md5
	for (int i=0;i<MAX_SHIPS;i++)
		if (Ships[i].used)
			MultiProcessShipChecksum(playermd5,i);
	// save the digest value to send to the server
	playermd5->MD5Final(NetPlayers[Player_num].digest);
	MD5::Destroy(playermd5);

	NetPlayers[Player_num].sequence=NETSEQ_REQUEST_WORLD;

	// Clear out all the lightmapped objects that weren't sent
	for (int i=0;i<Num_client_lm_objects;i++)
	{
		int objnum=Client_lightmap_list[i];
		int found=0;
		for (int t=0;t<Num_server_lm_objects && !found;t++)
		{
			if (Server_lightmap_list[t]==objnum)
				found=1;
		}

		// If its not in the server list, delete it!
		if (!found)
		{
			Objects[objnum].flags|=OF_SERVER_SAYS_DELETE;
			ObjDelete (objnum);
		}
	}
}


// Server is telling us that its done sending objects
void MultiDoDoneWorldStates (ubyte *data)
{
	MULTI_ASSERT (Netgame.local_role==LR_CLIENT,NULL);

	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);
	NetPlayers[Player_num].sequence=NETSEQ_PLAYING;
	NetPlayers[Player_num].total_bytes_sent = 0;
	NetPlayers[Player_num].total_bytes_rcvd = 0;
	PlayerMoveToStartPos (Player_num,Players[Player_num].start_index);

	MultiSendEnteringGame ();
}

// Makes an abbrievated version of a matrix
void MultiMakeMatrix (multi_orientation *dest,matrix *src)
{
	dest->multi_matrix[0]=(src->rvec.x*32767.0);
	dest->multi_matrix[1]=(src->rvec.y*32767.0);
	dest->multi_matrix[2]=(src->rvec.z*32767.0);
	
	dest->multi_matrix[3]=(src->uvec.x*32767.0);
	dest->multi_matrix[4]=(src->uvec.y*32767.0);
	dest->multi_matrix[5]=(src->uvec.z*32767.0);

	dest->multi_matrix[6]=(src->fvec.x*32767.0);
	dest->multi_matrix[7]=(src->fvec.y*32767.0);
	dest->multi_matrix[8]=(src->fvec.z*32767.0);
}

// Extracts a matrix from an abbreviated matrix
void MultiExtractMatrix (matrix *dest,multi_orientation *src)
{
	dest->rvec.x=(float)src->multi_matrix[0]/32767.0;
	dest->rvec.y=(float)src->multi_matrix[1]/32767.0;
	dest->rvec.z=(float)src->multi_matrix[2]/32767.0;

	dest->uvec.x=(float)src->multi_matrix[3]/32767.0;
	dest->uvec.y=(float)src->multi_matrix[4]/32767.0;
	dest->uvec.z=(float)src->multi_matrix[5]/32767.0;

	dest->fvec.x=(float)src->multi_matrix[6]/32767.0;
	dest->fvec.y=(float)src->multi_matrix[7]/32767.0;
	dest->fvec.z=(float)src->multi_matrix[8]/32767.0;
}

void MultiDoPlayerPos (ubyte *data)
{
	int use_smoothing=(Netgame.flags & NF_USE_SMOOTHING);
	int count=0; 
	
	// Skip header stuff
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	

	// Make sure its not out of order
	float packet_time=MultiGetFloat (data,&count);
	if (packet_time<NetPlayers[slot].packet_time)
		return;
	NetPlayers[slot].packet_time=packet_time;

			
	object *obj=&Objects[Players[slot].objnum];

	vector pos;
	matrix orient;
	ushort short_roomnum;
	int roomnum;

	// Get position
	MultiExtractPositionData (&pos,data,&count);
	
	// Get orientation
	ushort p=MultiGetShort (data,&count);
	ushort h=MultiGetShort (data,&count);
	ushort b=MultiGetShort (data,&count);

	vm_AnglesToMatrix (&orient,p,h,b);

	// Get room and terrain flag
	short_roomnum=MultiGetUshort (data,&count);

	roomnum = short_roomnum;

	vector vel={0,0,0},rotvel;
	angle turnroll;

//	float dist=vm_VectorDistance (&pos,&obj->pos);
	
		// Get velocity
	vel.x=((float)MultiGetShort (data,&count))/128.0;
	vel.y=((float)MultiGetShort (data,&count))/128.0;
	vel.z=((float)MultiGetShort (data,&count))/128.0;

	//mprintf ((0,"INCOMING x=%f y=%f z=%f dist=%f\n",vel.x,vel.y,vel.z,dist));
	
	// Get rotational velocity
	if (Netgame.flags & NF_SENDROTVEL)
	{
		rotvel.x=MultiGetShort (data,&count)*4;
		rotvel.y=MultiGetShort (data,&count)*4;
		rotvel.z=MultiGetShort (data,&count)*4;

		turnroll=MultiGetShort (data,&count);

		obj->mtype.phys_info.rotvel=rotvel;
		obj->mtype.phys_info.turnroll=turnroll;
	}

//	obj->mtype.phys_info.velocity=vel;

// Get weapon states
	ubyte windex=MultiGetByte (data,&count);
	Players[slot].weapon[PW_PRIMARY].index=windex & 0x0F;
	Players[slot].weapon[PW_SECONDARY].index=(windex >> 4) +10;

	// Get energy
	Players[slot].energy=MultiGetUbyte (data,&count);
	// Get flags
	ubyte flags=MultiGetByte (data,&count);

	// Do special stuff for non-visible objects
	bool visible=true;
	if (Objects[Players[slot].objnum].render_type==RT_NONE)
		visible=false;

	if (Netgame.local_role==LR_SERVER)
	{
		// Stop cheating with on/off weapons (can't fire things you shouldn't have!)
		player *playp=&Players[slot];
		if (!(playp->weapon_flags & (1<<Players[slot].weapon[PW_PRIMARY].index)))
			flags&=~(MPF_SPRAY|MPF_ON_OFF);

		if(Player_pos_fix[slot].active) {
			if(Player_pos_fix[slot].expire_time <= Gametime) {
				Player_pos_fix[slot].active = false;	
				//PrintDedicatedMessage("Discarded %d position updates from %s timed out though.\n",Player_pos_fix[slot].ignored_pos,Players[slot].callsign);
			}
		}

	}
	
	if ((flags & MPF_SPRAY) && visible)
		Objects[Players[slot].objnum].weapon_fire_flags|=WFF_SPRAY;
	else
		Objects[Players[slot].objnum].weapon_fire_flags&=~WFF_SPRAY;

	if ((flags & MPF_ON_OFF) && visible)
	{
		Objects[Players[slot].objnum].weapon_fire_flags|=WFF_ON_OFF;
	}
	else
		Objects[Players[slot].objnum].weapon_fire_flags&=~WFF_ON_OFF;

	if(Demo_flags == DF_RECORDING)
	{
		DemoWriteObjWeapFireFlagChanged(Players[slot].objnum);
	}
	
	if ((flags & MPF_AFTERBURNER) && !(flags & MPF_DEAD) && visible)
	{
		Players[slot].flags |=PLAYER_FLAGS_AFTERBURN_ON;
	}
	else
	{
		Players[slot].flags &=~PLAYER_FLAGS_AFTERBURN_ON;
	}

	if ((flags & MPF_THRUSTED) && !(flags & MPF_DEAD) && visible)
	{
		Players[slot].flags |=PLAYER_FLAGS_THRUSTED;
	}
	else
	{
		Players[slot].flags &=~PLAYER_FLAGS_THRUSTED;
	}


	if ((flags & MPF_HEADLIGHT) && !(flags & MPF_DEAD))
		Players[slot].flags |=PLAYER_FLAGS_HEADLIGHT;
	else
		Players[slot].flags &=~PLAYER_FLAGS_HEADLIGHT;

	if (flags & MPF_OUTSIDE)
		roomnum = MAKE_ROOMNUM(roomnum);
	
	if ((flags & MPF_DEAD))// || (Players[slot].flags & PLAYER_FLAGS_DYING) || (Players[slot].flags & PLAYER_FLAGS_DEAD))
		return;		// If dying, don't do positional updates

	if (! ROOMNUM_OUTSIDE(roomnum))
	{
		// Deal with late packets from last level

		if (roomnum>Highest_room_index || roomnum<0)
			return;

		if (Rooms[roomnum].used==0)
			return;

		if (Rooms[roomnum].flags & RF_EXTERNAL)
			return;
	}

	if (Netgame.local_role==LR_CLIENT && use_smoothing && !(flags & MPF_FIRED))
	{
		// Check to see if we need to correct this ship due to error
		if (vm_VectorDistance(&pos,&obj->pos)<8)
		{
			int pps_player_num;
			if (Netgame.local_role==LR_SERVER)
			{
				pps_player_num=slot;				
			}
			else
			{
				pps_player_num=Player_num;
			}

			vector dest_pos=pos+(vel*(1.0/NetPlayers[pps_player_num].pps));
			vel=(dest_pos-obj->pos)/(1.0/NetPlayers[pps_player_num].pps);

			pos=obj->pos;
		}
	}
	
	obj->mtype.phys_info.velocity=vel;

	if( (Netgame.local_role==LR_SERVER) && (Player_pos_fix[slot].active) && (roomnum!=Player_pos_fix[slot].room) ) {
		//ignoring this position update....what should we do????
		Player_pos_fix[slot].ignored_pos++;
	}
	else {
		if(Player_pos_fix[slot].active) {
			Player_pos_fix[slot].active = false;
			//print some info to the server console...
			//PrintDedicatedMessage("Discarded %d position updates from %s\n",Player_pos_fix[slot].ignored_pos,Players[slot].callsign);
		}
		ObjSetPos (obj,&pos,roomnum,&orient,true);
	}
	
	if (Netgame.local_role==LR_SERVER)
	{
		Players[slot].oldroom=roomnum;
	}
}

void MultiDoRobotPos (ubyte *data)
{
	int count=0;
	//@@multi_orientation multi_mat;
	// Skip header stuff
	SKIP_HEADER (data,&count);
	
	ushort server_objnum=MultiGetUshort (data,&count);
	ushort objectnum = Server_object_list[server_objnum];
	if(objectnum==65535 || !(Objects[objectnum].flags & OF_SERVER_OBJECT))
	{
		mprintf((0,"Bad robotposition object number!\n"));
		return;
	}
	object *obj=&Objects[objectnum];

	vector pos;
	matrix orient;
	ushort short_roomnum;
	int roomnum;
	
	// Get position
	MultiExtractPositionData (&pos,data,&count);
			
	// Get orientation
	ushort p=MultiGetShort (data,&count);
	ushort h=MultiGetShort (data,&count);
	ushort b=MultiGetShort (data,&count);

	vm_AnglesToMatrix (&orient,p,h,b);


	// Get room and terrain flag
	short_roomnum=MultiGetUshort (data,&count);
	ubyte terrain=MultiGetByte (data,&count);

	roomnum = short_roomnum;
	if (terrain)
		roomnum = MAKE_ROOMNUM(roomnum);
	vector vel={0,0,0};
	
	// Get velocity
	vel.x=((float)MultiGetShort (data,&count))/128.0;
	vel.y=((float)MultiGetShort (data,&count))/128.0;
	vel.z=((float)MultiGetShort (data,&count))/128.0;
	obj->mtype.phys_info.velocity=vel;

	obj->mtype.phys_info.flags &=~PF_NO_COLLIDE;
	obj->render_type=RT_POLYOBJ;
			
	if(!(obj->flags & (OF_DEAD)) && obj->type!=OBJ_NONE)
	{
		ObjSetPos (obj,&pos,roomnum,&orient,false);


	}

}


// Stuffs a players firing information into a packet
int MultiStuffPlayerFire (int slot,ubyte *data)
{
	int size_offset;
	int count=0;
	
	size_offset=START_DATA (MP_PLAYER_FIRE,data,&count);
	MultiAddByte (slot,data,&count);
	MultiAddByte (Player_fire_packet[slot].wb_index,data,&count);
	MultiAddByte (Player_fire_packet[slot].fire_mask,data,&count);
	MultiAddByte (Player_fire_packet[slot].damage_scalar,data,&count);
	MultiAddByte (Player_fire_packet[slot].reliable,data,&count);
	
	END_DATA (count,data,size_offset);

	return count;
}

// Returns true if there is enough ammo to allow this player to fire, else false
bool MultiEnoughAmmoToFire (int slot,int wb_index)
{
	ship *ship = &Ships[Players[slot].ship_index];
	otype_wb_info *wb = &ship->static_wb[wb_index];
	
	if (wb->ammo_usage>0 && Players[slot].weapon_ammo[wb_index]<=0)
		return false;

	return true;
}

void MultiSubtractAmmoToFire (int slot,int wb_index)
{
	ship *ship = &Ships[Players[slot].ship_index];
	otype_wb_info *wb = &ship->static_wb[wb_index];
	Players[slot].weapon_ammo[wb_index]-=wb->ammo_usage;
	if (Players[slot].weapon_ammo[wb_index]<0)
		Players[slot].weapon_ammo[wb_index]=0;
}


#define MPFF_QUADED	128
// Sends a fire packet from a player
void MultiSendFirePlayerWB (int playernum,ubyte wb_index,ubyte fire_mask,ubyte reliable,float scalar)
{
	// Send quaded info if needed
	ubyte index_to_send=wb_index;

	if (Objects[Players[playernum].objnum].dynamic_wb[wb_index].flags & DWBF_QUAD)
		index_to_send|=MPFF_QUADED;

	ubyte damage_scalar=(scalar*64.0);
	
	Player_fire_packet[playernum].fire_mask=fire_mask;
	Player_fire_packet[playernum].damage_scalar=damage_scalar;
	Player_fire_packet[playernum].wb_index=index_to_send;
	Player_fire_packet[playernum].reliable=reliable;

	// How should we send this information?
	// secondary fire in a non-peer to peer game should get sent reliably
	// except for a flage
	#ifdef RELIABLE_SECONDARIES
		if( (!(Netgame.flags&NF_PEER_PEER)) && (wb_index>=SECONDARY_INDEX) && (wb_index!=FLARE_INDEX))
		{
			Player_fire_packet[playernum].fired_on_this_frame=PFP_FIRED_RELIABLE;
		}
		else
	#endif
	{
		// send like normal fire
		Player_fire_packet[playernum].fired_on_this_frame=PFP_FIRED;
	}
}

// Does player firing
void MultiDoFirePlayerWB (ubyte *data)
{
	int count=0,ok_to_fire=1;
		
	// Skip header stuff
	SKIP_HEADER (data,&count);

	int pnum=MultiGetByte (data,&count);
	Player_fire_packet[pnum].wb_index=MultiGetByte (data,&count);
	Player_fire_packet[pnum].fire_mask=MultiGetByte (data,&count);
	Player_fire_packet[pnum].damage_scalar=MultiGetByte (data,&count);
	Player_fire_packet[pnum].reliable=MultiGetByte (data,&count);

	// Strip out quad data
	int wb_index=Player_fire_packet[pnum].wb_index;
	int quaded=wb_index & MPFF_QUADED;
	wb_index&=~MPFF_QUADED;

	// Fire!
	float scalar=(float)Player_fire_packet[pnum].damage_scalar/64.0;
	int ship_index=Players[pnum].ship_index;
	Objects[Players[pnum].objnum].dynamic_wb[wb_index].cur_firing_mask=Player_fire_packet[pnum].fire_mask;

	if (quaded)
		Objects[Players[pnum].objnum].dynamic_wb[wb_index].flags |=DWBF_QUAD;
	else
		Objects[Players[pnum].objnum].dynamic_wb[wb_index].flags &=~DWBF_QUAD;

	if (Netgame.local_role==LR_SERVER)
	{
		// Check to see if this player is firing a weapon he doesn't have
		player *playp=&Players[pnum];
		if (!(playp->weapon_flags & (1<<wb_index)))
		{
			mprintf ((0,"Can't fire client weapon...flags=%d wb_index=%d\n",playp->weapon_flags,wb_index));
			ok_to_fire=0;
		}

		if (!MultiEnoughAmmoToFire(pnum,wb_index))
			ok_to_fire=0;
	}
	
	if (ok_to_fire)
	{
		WBFireBattery(&Objects[Players[pnum].objnum], &Ships[ship_index].static_wb[wb_index], 0, wb_index,scalar);
		// Take off ammo
		MultiSubtractAmmoToFire (pnum,wb_index);
		
	}
			

	// Play cutoff sound if there is one
	int cutoff_sound = Ships[ship_index].firing_release_sound[wb_index];
	if (cutoff_sound != -1)
		Sound_system.Play3dSound(cutoff_sound,&Objects[Players[pnum].objnum]);

	if ( (Netgame.local_role==LR_SERVER && ok_to_fire) && ( (!(Netgame.flags & NF_PEER_PEER)) || ((Netgame.flags & NF_PEER_PEER) && Player_fire_packet[pnum].reliable) ) )
	{
		#ifdef RELIABLE_SECONDARIES
		// How should we send this information?
		// secondary fire in a non-peer to peer game should get sent reliably
			if( (!(Netgame.flags&NF_PEER_PEER)) && (wb_index>=SECONDARY_INDEX) && (wb_index!=FLARE_INDEX) )
			{
				Player_fire_packet[pnum].fired_on_this_frame=PFP_FIRED_RELIABLE;
			}
			else
		#endif
		{
			// send like normal fire
			Player_fire_packet[pnum].fired_on_this_frame=PFP_FIRED;
		}
	}
}

// Tell everyone I'm quitting
void MultiSendLeaveGame ()
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;

	mprintf ((0,"Sending leave game!\n"));

	if (Netgame.local_role==LR_SERVER)
	{
		size_offset=START_DATA(MP_SERVER_QUIT,data,&count,1);
		
		ShowProgressScreen (TXT_MLTSHUTDOWN);
	
	}
	else
	{
		size_offset=START_DATA(MP_LEAVE_GAME,data,&count);
		MultiAddByte (Player_num,data,&count);
	}
	
	END_DATA(count,data,size_offset);

	if (Netgame.local_role==LR_SERVER)
	{
		MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_REQUEST_BUILDINGS);
	}
	else
	{	
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count);
	}
	
	
}

// Called whenever I want to leave the game
void MultiLeaveGame ()
{	
	mprintf ((0,"I'm leaving the netgame!\n"));

	CallGameDLL (EVT_GAME_DISCONNECTED,&DLLInfo);

	if ((Players[Player_num].flags & PLAYER_FLAGS_DYING) || (Players[Player_num].flags & PLAYER_FLAGS_DEAD))
		EndPlayerDeath (Player_num);

	//Cancel all incoming file transfers

	Multi_bail_ui_menu=true;
	MultiSendLeaveGame ();
	
	CallMultiDLL(MT_EVT_GAME_OVER);
	MultiCloseGame ();
	
	SetFunctionMode(MENU_MODE); 
	if(Netgame.local_role==LR_SERVER)
	{
		gspy_EndGame();
	}
	
	
	ScoreAPIGameOver();
	NetPlayers[Player_num].flags &=~NPF_CONNECTED;

}

// Releases a missile that belongs to a player
void MultiDoReleaseTimeoutMissile (ubyte *data)
{
	int count=0;

	SKIP_HEADER (data,&count);

	int pnum=MultiGetByte (data,&count);

	if (Players[pnum].user_timeout_obj!=NULL)
	{
		ReleaseUserTimeoutMissile (pnum);
	}

	if (Netgame.local_role==LR_SERVER)
		MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYING);
}

// Tell everyone I'm timingout my timeout weapon
void MultiSendReleaseTimeoutMissile ()
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;

	ASSERT (Players[Player_num].user_timeout_obj!=NULL);

	mprintf ((0,"Sending timeout weapon packet!\n"));

	size_offset=START_DATA(MP_TIMEOUT_WEAPON,data,&count);
	MultiAddByte (Player_num,data,&count);
	
	END_DATA(count,data,size_offset);

	if (Netgame.local_role==LR_SERVER)
		MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_REQUEST_BUILDINGS);
	else
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count);
	
}

// Tells all the clients who are trying to join to piss off until the next level
void MultiSendConnectBail()
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;
	bool wait_to_send=false;
	int i;

	size_offset=START_DATA(MP_CONNECT_BAIL,data,&count);
	END_DATA(count,data,size_offset);

	// Tell them to stop trying to join
	for (i=1;i<MAX_PLAYERS;i++)
	{
		if (NetPlayers[i].flags & NPF_CONNECTED && NetPlayers[i].sequence<NETSEQ_OBJECTS)
		{
			nw_SendReliable (NetPlayers[i].reliable_socket,data,count,true);
			wait_to_send=true;
		}

	
	}

	// Make sure these clients get this
	if (wait_to_send)
	{
		for(int t=0;t<10;t++)
		{
			nw_DoNetworkIdle();	
			Sleep(100);		
		}
	}

	// Now disconnect them 
	for (i=1;i<MAX_PLAYERS;i++)
	{
		if (NetPlayers[i].flags & NPF_CONNECTED && NetPlayers[i].sequence<NETSEQ_OBJECTS)
		{
			nw_CloseSocket (&NetPlayers[i].reliable_socket);
			NetPlayers[i].flags&=~NPF_CONNECTED;
		}
	}

		
}
// Server is telling us to bail on our connection
void MultiDoConnectBail ()
{
	if (Netgame.local_role!=LR_CLIENT)
		return;

	ShowProgressScreen (TXT_MULTI_SERVERCHANGEA,TXT_MULTI_SERVERCHANGEB);
	Sleep (3000);
	MultiLeaveGame();
	SetFunctionMode(MENU_MODE);
}


// Some DLL is telling us to end the level!
void MultiEndLevel ()
{
	MULTI_ASSERT (Netgame.local_role==LR_SERVER,NULL);
	SetGameState(GAMESTATE_LVLEND);
	PrintDedicatedMessage (TXT_DS_ENDINGLEVEL);
	Multi_bail_ui_menu=true;
}

// server is telling us the level has ended
void MultiDoLevelEnded (ubyte *data)
{
	int count=0;
	SKIP_HEADER (data,&count);

	int success=MultiGetByte (data,&count);
	Multi_next_level=MultiGetByte (data,&count);

	// Make sure we're not being fooled
	if (Multi_next_level<1 || Multi_next_level>Current_mission.num_levels)
		Multi_next_level=-1;

	mprintf ((0,"Doing level ended!  Next level=%d\n",Multi_next_level));

	if (success)
		SetGameState(GAMESTATE_LVLEND);
	else
		SetGameState(GAMESTATE_LVLFAILED);


	NetPlayers[Player_num].sequence=NETSEQ_LEVEL_END;

	Multi_bail_ui_menu=true;

	CallGameDLL (EVT_CLIENT_GAMELEVELEND,&DLLInfo);
	CallMultiDLL(MT_EVT_GAME_OVER);
	ScoreAPIGameOver();
}


// Tells all the clients to end the level
void MultiSendLevelEnded (int success,int next_level)
{
	MULTI_ASSERT (Netgame.local_role==LR_SERVER,NULL);

	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;

	mprintf ((0,"Sending level ended!\n"));

	size_offset=START_DATA (MP_LEVEL_ENDED,data,&count,1);
	MultiAddByte (success,data,&count);
	MultiAddByte (next_level,data,&count);
	END_DATA (count,data,size_offset);

	// First send out to all the people who are playing
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_OBJECTS,true);

	// Now send a connect bail on all the ones who are connecting
	MultiSendConnectBail();
	
	NetPlayers[Player_num].sequence=NETSEQ_LEVEL_END;

	if (success)
		SetGameState(GAMESTATE_LVLEND);
	else
		SetGameState(GAMESTATE_LVLFAILED);

	if (Netgame.local_role==LR_SERVER)
	{
		for (int i=0;i<MAX_PLAYERS;i++)
		{
			if (NetPlayers[i].flags & NPF_CONNECTED && NetPlayers[i].sequence==NETSEQ_PLAYING)
				NetPlayers[i].sequence=NETSEQ_WAITING_FOR_LEVEL;
		}
	}
	//Do this so it gets sent off now.
	for(int t=0;t<10;t++)
	{
		nw_DoNetworkIdle();	
		Sleep(100);		
	}
}

extern void MultiClearPlayerMarkers(int slot);
// Do leave game stuff
void MultiDoLeaveGame (ubyte *data)
{
	int count=0;

	mprintf ((0,"In MultiDoLeaveGame!\n"));

	// Skip header stuff
	SKIP_HEADER (data,&count);
	
	ubyte slot=MultiGetByte (data,&count);

	
	if (NetPlayers[slot].flags & NPF_CONNECTED)
	{	
		if (NetPlayers[slot].sequence==NETSEQ_PLAYING)
		{
			AddHUDMessage (TXT_MLTLEAVEGAME,Players[slot].callsign);
			MultiMakePlayerGhost (slot);
			Players[slot].flags=0;
		}
		NetPlayers[slot].flags &=~NPF_CONNECTED;
		NetPlayers[slot].secret_net_id = 0;
	}
	else
		mprintf ((0,"Got a leave game from a non-connected player!\n"));

	// Now clear any files this guy might be receiving
	if(NetPlayers[slot].file_xfer_flags != NETFILE_NONE) 
	{
		if(NetPlayers[slot].file_xfer_cfile)
		{
			char delfile[_MAX_PATH*2];
			strcpy(delfile,NetPlayers[slot].file_xfer_cfile->name);
			cfclose(NetPlayers[slot].file_xfer_cfile);
			NetPlayers[slot].file_xfer_cfile = NULL;				
			if(NetPlayers[slot].file_xfer_flags == NETFILE_RECEIVING) 
				ddio_DeleteFile(delfile);
		}
		NetPlayers[slot].file_xfer_flags = NETFILE_NONE;
		NetPlayers[slot].custom_file_seq++;
		NetPlayers[slot].custom_file_seq = NETFILE_ID_NOFILE;
	}

	// If this is the server, do any cleanup on this slot
	if (Netgame.local_role==LR_SERVER)
	{
		MultiClearGuidebot(slot);
		MultiClearPlayerMarkers(slot);
		DLLInfo.me_handle=DLLInfo.it_handle=Objects[Players[slot].objnum].handle;
		CallGameDLL(EVT_GAMEPLAYERDISCONNECT,&DLLInfo);	

		if (NetPlayers[slot].sequence==NETSEQ_PLAYING)
			PlayerSpewInventory (&Objects[Players[slot].objnum],true,true);
		MultiSendReliablyToAllExcept (slot,data,count,NETSEQ_REQUEST_BUILDINGS);
		if(NetPlayers[slot].file_xfer_flags != NETFILE_NONE)
		{
			MultiCancelFile(slot,NetPlayers[slot].custom_file_seq,NetPlayers[slot].file_xfer_who);
		}
		nw_CloseSocket (&NetPlayers[slot].reliable_socket);
		if(Game_is_master_tracker_game)
		{
			NetPlayers[slot].flags  |= NPF_MT_WRITING_PILOT;
		}
	}
	ScoreAPIPlayerLeft(slot);
}

void MultiDoServerQuit (ubyte *data)
{
	ShowProgressScreen (TXT_MLTSERVERQUIT);
	//Abort all file transfers
	//ddio_DeleteFile(NetPlayers[filewho].file_xfer_cfile->name);
	for (int i=0;i<MAX_NET_PLAYERS;i++)
	{
		if(NetPlayers[i].file_xfer_flags != NETFILE_NONE) 
		{
			if(NetPlayers[i].file_xfer_cfile)
			{
				char delfile[_MAX_PATH*2];
				strcpy(delfile,NetPlayers[i].file_xfer_cfile->name);
				cfclose(NetPlayers[i].file_xfer_cfile);
				NetPlayers[i].file_xfer_cfile = NULL;				
				if(NetPlayers[i].file_xfer_flags == NETFILE_RECEIVING) 
					ddio_DeleteFile(delfile);
			}
			NetPlayers[i].file_xfer_flags = NETFILE_NONE;
			NetPlayers[i].custom_file_seq++;
			NetPlayers[i].custom_file_seq = NETFILE_ID_NOFILE;
		}
	}
	mprintf ((0,"Server quitting!\n"));
	MultiLeaveGame();
	Sleep(2000);
}

void MultiDoDisconnect (ubyte *data)
{
	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);
	
	ubyte slot=MultiGetByte (data,&count);

	if (NetPlayers[slot].flags & NPF_CONNECTED)
	{	
		if(NetPlayers[slot].file_xfer_flags != NETFILE_NONE) 
		{
			if(NetPlayers[slot].file_xfer_cfile)
			{
				char delfile[_MAX_PATH*2];
				strcpy(delfile,NetPlayers[slot].file_xfer_cfile->name);
				cfclose(NetPlayers[slot].file_xfer_cfile);
				NetPlayers[slot].file_xfer_cfile = NULL;				
				if(NetPlayers[slot].file_xfer_flags == NETFILE_RECEIVING) 
					ddio_DeleteFile(delfile);
			}
			NetPlayers[slot].file_xfer_flags = NETFILE_NONE;
			NetPlayers[slot].custom_file_seq++;
			NetPlayers[slot].custom_file_seq = NETFILE_ID_NOFILE;
		}
		if (NetPlayers[slot].sequence==NETSEQ_PLAYING)
		{
			AddHUDMessage (TXT_MLTDISCONNECT,Players[slot].callsign);
			MultiMakePlayerGhost (slot);
		}
		NetPlayers[slot].flags &=~NPF_CONNECTED;
		NetPlayers[slot].secret_net_id = 0;
	}
	else
		mprintf ((0,"Got a disconnect from a non-connected player!\n"));

	ScoreAPIPlayerLeft(slot);	
}


void MultiDoServerRejectedChecksum(ubyte *data)
{
	int count=0;

	MULTI_ASSERT (Netgame.local_role==LR_CLIENT,NULL);

	SKIP_HEADER (data,&count);
	ShowProgressScreen (TXT_MLTLEVELNOMATCH, "Remove any ship modifications if you have any loaded");
	MultiLeaveGame();
	Sleep (2000);
}

// Lets us know if the server says its ok to join
void MultiDoJoinResponse (ubyte *data)
{
	int count=0;

	MULTI_ASSERT (Netgame.local_role==LR_CLIENT,NULL);

	SKIP_HEADER (data,&count);

	Ok_to_join=MultiGetByte(data,&count);

}

// Returns an index of a unconnected slot
// Returns -1 if there are none
int MultiFindFreeSlot ()
{
	if (Netgame.local_role!=LR_SERVER)
	{
		return -1;
	}
	
	for (int i=0;i<MAX_PLAYERS;i++)
	{
		if ((NetPlayers[i].flags & (NPF_CONNECTED|NPF_MT_READING_PILOT|NPF_MT_WRITING_PILOT))==0)
			return i;
		//(!(NetPlayers[i].flags & NPF_CONNECTED))
	}

	return -1;
}


// Someone is asking to join our game
// Tell them if its ok
void MultiDoAskToJoin (ubyte *data,network_address *from_addr)
{
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int count=0;
	int incount=0;
	int size;
			
	size=START_DATA(MP_JOIN_RESPONSE,outdata,&count);
	
	if (Netgame.local_role==LR_SERVER && Multi_accept_state)
	{
		// First see if this guy is banned
		if (GameDLLIsAddressBanned(from_addr,NULL))
		{
			MultiAddByte (JOIN_ANSWER_REJECTED,outdata,&count);
			
		}
		else
		{
			if (MultiCountPlayers()<Netgame.max_players)
			{
				int slot=MultiFindFreeSlot();
				MULTI_ASSERT (slot>0,"FindFreeSlot returned -1!");

				if (Players[slot].start_roomnum!=-1)
				{
					mprintf ((0,"Sending OK to join!\n"));
					MultiAddByte (JOIN_ANSWER_OK,outdata,&count);
				}
				else
				{	
					MultiAddByte (JOIN_ANSWER_NO_ROOM,outdata,&count);
				}
			}
			else
			{
				MultiAddByte (JOIN_ANSWER_FULL,outdata,&count);
			}
		}
	}
	else
		MultiAddByte (JOIN_ANSWER_NOT_SERVER,outdata,&count);

	END_DATA (count,outdata,size);

	nw_Send(from_addr,outdata,count,0);
}

// Someone is asking about our game
void MultiDoGetGameInfo (ubyte *data,network_address *from_addr)
{
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;
	network_address my_addr;
	float ping_time;

	//mprintf ((0,"Got a request for knowledge about my game!\n"));

	//Get the time and stuff it back in the packet
	SKIP_HEADER (data,&count);
	ping_time=MultiGetFloat(data,&count);
	int version=MultiGetInt (data,&count);

	if (version!=MULTI_VERSION)
		return;		// Versions don't match, so just return


	count=0;

	if(Game_is_master_tracker_game)
		return;

	if (!Multi_accept_state)
		return;
	

	if ((Netgame.local_role==LR_SERVER)&&(Game_mode & GM_MULTI))
	{
		size_offset=START_DATA(MP_GAME_INFO,outdata,&count);
		// Copies my address into the passed argument
		nw_GetMyAddress (&my_addr);
		memcpy (&outdata[count],&my_addr,sizeof(network_address));
		count+=sizeof(network_address);
		
		int len=strlen (Netgame.name)+1;
		MultiAddByte (len,outdata,&count);
		memcpy (&outdata[count],Netgame.name,len);
		count+=len;

		len=strlen (Netgame.mission)+1;

		MultiAddByte (len,outdata,&count);
		memcpy (&outdata[count],Netgame.mission,len);
		count+=len;

		len=strlen (Netgame.mission_name)+1;

		MultiAddByte (len,outdata,&count);
		memcpy (&outdata[count],Netgame.mission_name,len);
		count+=len;

		len=strlen (Netgame.scriptname)+1;

		MultiAddByte (len,outdata,&count);
		memcpy (&outdata[count],Netgame.scriptname,len);
		count+=len;

		MultiAddShort(Current_mission.cur_level,outdata,&count);
		unsigned short icurrplayers = 0;
		for(int i=0;i<MAX_NET_PLAYERS;i++)
		{
			if(Dedicated_server)
			{
				if(i==Player_num)
					continue;
			}
			if( (NetPlayers[i].flags & NPF_CONNECTED) && (Players[i].callsign[0]) )
				icurrplayers++;
		}
		//Adjust the players count because the server isn't a player
		
		int maxplayers = Netgame.max_players;
		if(Dedicated_server)
		{
			//icurrplayers--;
			maxplayers--;
		}

		
		MultiAddShort(icurrplayers,outdata,&count);
		MultiAddShort(maxplayers,outdata,&count);
		MultiAddFloat(ping_time,outdata,&count);		

		MultiAddInt(Netgame.flags,outdata,&count);
		MultiAddByte(Dedicated_server?1:0,outdata,&count);	
		MultiAddByte(Netgame.difficulty,outdata,&count);	

		END_DATA (count,outdata,size_offset);
		nw_Send(from_addr,outdata,count,0);

	}
}

// Someone is asking about our PXO game
void MultiDoGetPXOGameInfo (ubyte *data,network_address *from_addr)
{
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;
	float ping_time;
	network_address my_addr;

	//mprintf ((0,"Got a request for knowledge about my game!\n"));

	if(!Game_is_master_tracker_game)
	{
		return;
	}
	//Get the time and stuff it back in the packet
	SKIP_HEADER (data,&count);
	ping_time=MultiGetFloat(data,&count);
	int version=MultiGetInt (data,&count);

	if (version!=MULTI_VERSION)
		return;		// Versions don't match, so just return


	count=0;

	if ((Netgame.local_role==LR_SERVER)&&(Game_mode & GM_MULTI))
	{
		size_offset=START_DATA(MP_GAME_INFO,outdata,&count);
		// Copies my address into the passed argument
		nw_GetMyAddress (&my_addr);
		memcpy (&outdata[count],&my_addr,sizeof(network_address));
		count+=sizeof(network_address);
		
		int len=strlen (Netgame.name)+1;
		MultiAddByte (len,outdata,&count);
		memcpy (&outdata[count],Netgame.name,len);
		count+=len;

		len=strlen (Netgame.mission)+1;

		MultiAddByte (len,outdata,&count);
		memcpy (&outdata[count],Netgame.mission,len);
		count+=len;

		len=strlen (Netgame.mission_name)+1;

		MultiAddByte (len,outdata,&count);
		memcpy (&outdata[count],Netgame.mission_name,len);
		count+=len;

		len=strlen (Netgame.scriptname)+1;

		MultiAddByte (len,outdata,&count);
		memcpy (&outdata[count],Netgame.scriptname,len);
		count+=len;

		MultiAddShort(Current_mission.cur_level,outdata,&count);
		unsigned short icurrplayers = 0;
		int i=0;
		
		for(i=0;i<MAX_NET_PLAYERS;i++)
		{
			if(Dedicated_server)
			{
				if(i==Player_num)
					continue;
			}
			if(NetPlayers[i].flags & NPF_CONNECTED)
				icurrplayers++;
		}

		//Adjust the players count because the server isn't a player
		
		int maxplayers = Netgame.max_players;
		if(Dedicated_server)
		{
			maxplayers--;
		}

		
		MultiAddShort(icurrplayers,outdata,&count);
		MultiAddShort(maxplayers,outdata,&count);
		MultiAddFloat(ping_time,outdata,&count);		

		MultiAddInt(Netgame.flags,outdata,&count);		
		MultiAddByte(Dedicated_server?1:0,outdata,&count);	
		MultiAddByte(Netgame.difficulty,outdata,&count);	

		END_DATA (count,outdata,size_offset);
		nw_Send(from_addr,outdata,count,0);

	}
}


bool Multi_Gamelist_changed = false;

// A server is telling us about a game we've requested
void MultiDoGameInfo (ubyte *data,network_address *from_addr)
{
	int count=0;
	
	char name[NETGAME_NAME_LEN];
	char mission[MSN_NAMELEN];
	char mission_name[MISSION_NAME_LEN];
	char scriptname[NETGAME_SCRIPT_LEN];
	int level;
	int maxplayers;
	int currplayers;
	network_address incoming_addr;
	int len,mission_len,script_len,mission_name_len;
	float ping_time;
	int i;
	int fixed_len;

	if (Num_network_games_known>=MAX_NETWORK_GAMES)
		return;

	//mprintf ((0,"Got game info packet!!\n"));

	SKIP_HEADER (data,&count);
	
	//memcpy (&incoming_addr,&data[count],sizeof(network_address));
	//Get the network_address from the packet header, not the packet data
	memcpy (&incoming_addr,from_addr,sizeof(network_address));
	count+=sizeof (network_address);

	

	len=MultiGetByte (data,&count);
	
	fixed_len = std::min(NETGAME_NAME_LEN,len);

	memcpy (name,&data[count],fixed_len);
	name[fixed_len-1] = 0;
	count+=len;
		
	mission_len=MultiGetByte (data,&count);

	fixed_len = std::min(MSN_NAMELEN,mission_len);

	memcpy (mission,&data[count],fixed_len);
	mission[fixed_len-1] = 0;
	count+=mission_len;

	mission_name_len=MultiGetByte (data,&count);

	fixed_len = std::min(MISSION_NAME_LEN,mission_name_len);

	memcpy (mission_name,&data[count],fixed_len);
	mission_name[fixed_len-1] = 0;
	count+=mission_name_len;
	
	script_len=MultiGetByte (data,&count);

	fixed_len = std::min(NETGAME_SCRIPT_LEN,script_len);

	memcpy (scriptname,&data[count],fixed_len);
	scriptname[fixed_len-1] = 0;
	count+=script_len;

	level = MultiGetShort(data,&count);
	currplayers = MultiGetShort(data,&count);
	maxplayers = MultiGetShort(data,&count);

	ping_time =  MultiGetFloat(data,&count);

	unsigned int flags = MultiGetInt(data,&count);
	bool dedicated = MultiGetByte (data,&count)?true:false;

	int diff = MultiGetByte (data,&count);

	// Now go through and see if this is game we don't already have in our list
	for (i=0;i<Num_network_games_known;i++)
	{
		if ((!memcmp (&incoming_addr,&Network_games[i].addr,sizeof(network_address) )))
		{
			Multi_Gamelist_changed = true;
			Network_games[i].server_response_time = timer_GetTime()-ping_time;
			Network_games[i].last_update = timer_GetTime();
			Network_games[i].level_num = level;
			Network_games[i].curr_num_players = currplayers;
			return;
		}
		
	}

	if (i==Num_network_games_known)
	{
		Multi_Gamelist_changed = true;
		// We found a new one!
		int n=Num_network_games_known;
		Network_games[n].last_update = timer_GetTime();
		memcpy (&Network_games[n].addr,&incoming_addr,sizeof(network_address));
		strcpy (Network_games[n].name,name);
		strcpy (Network_games[n].mission,mission);
		strcpy (Network_games[n].mission_name,mission_name);
		strcpy (Network_games[n].scriptname,scriptname);
		Network_games[n].level_num = level;
		Network_games[n].curr_num_players = currplayers;
		Network_games[n].max_num_players = maxplayers;
		Network_games[n].server_response_time = timer_GetTime()-ping_time;
		Network_games[n].flags = flags;
		Network_games[n].dedicated_server = dedicated;
		Network_games[n].difficulty = diff;
		//This handle is used to make the game list update nicely
		Network_games[n].handle = Netgame_curr_handle++;
	//	mprintf ((0,"Got new game called %s!\n",name));

		Num_network_games_known++;
		
	}
	else
		Int3();	// How'd we get here?
}

// Tells our clients that building exploded
// Server only
void MultiSendBlowupBuilding (int hit_objnum,int killer_objnum,float damage)
{
	MULTI_ASSERT (Netgame.local_role==LR_SERVER,"Client in SendBlowupBuilding!");

	int count=0;
	int size_offset=0;
	ubyte data[MAX_GAME_DATA_SIZE];

	ushort short_damage=damage;

	size_offset=START_DATA (MP_BLOWUP_BUILDING,data,&count,1);
	MultiAddShort (hit_objnum,data,&count);
	MultiAddShort (killer_objnum,data,&count);
	MultiAddShort (damage,data,&count);

	END_DATA (count,data,size_offset);

	if (!Multi_building_states[hit_objnum])
	{
		Multi_building_states[hit_objnum]=1;
		Multi_num_buildings_changed++;
	}

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_REQUEST_OBJECTS);
}

// Blowup a building because the server told us so
void MultiDoBlowupBuilding (ubyte *data)
{
	int count=0;

	SKIP_HEADER (data,&count);

	ushort hit_objnum=MultiGetUshort (data,&count);
	ushort killer_objnum=MultiGetUshort (data,&count);
	ushort damage=MultiGetUshort (data,&count);
	ushort seed=MultiGetUshort (data,&count);

	if (Objects[hit_objnum].type!=OBJ_BUILDING)
	{
		Int3();	// Get Jason, trying to blowup a non-building
		return;
	}

	ps_srand(seed);
	KillObject(&Objects[hit_objnum],&Objects[killer_objnum],damage);

	if (!Multi_building_states[hit_objnum])
	{
		Multi_building_states[hit_objnum]=1;
		Multi_num_buildings_changed++;
	}
	
}

// Server is telling us about buildings to get rid of
void MultiDoBuilding (ubyte *data)
{
	int count=0;

	SKIP_HEADER (data,&count);
	ubyte num=MultiGetByte (data,&count);
	for (int i=0;i<num;i++)
	{
		ushort objnum=MultiGetUshort (data,&count);
		if (Objects[objnum].type!=OBJ_BUILDING)
		{
			mprintf ((0,"Error! Server says objnum %d is a building and its not!\n",objnum));
		}
		else
		{		
			Multi_building_states[objnum]=1;

			object *obj=&Objects[objnum];


			if (Object_info[obj->id].lo_render_handle!=-1)
			{
				obj->flags|=OF_USE_DESTROYED_POLYMODEL;
				obj->lighting_render_type=LRT_GOURAUD;
	
				//make parent object only draw center part
				obj->rtype.pobj_info.subobj_flags=1;
			}
			else
				ObjDelete (objnum);	
		}
	}

	Multi_num_buildings_changed=num;
}

doorway *GetDoorwayFromObject(int door_obj_handle);
// Server is telling us the world state
void MultiDoWorldStates (ubyte *data)
{
	int count=0;
	ubyte world_type;

	SKIP_HEADER (data,&count);

	mprintf ((0,"Got a world state packet!\n"));

	while ((world_type=MultiGetByte (data,&count))!=WS_END)
	{
		switch (world_type)
		{
			case WS_ROOM_WIND:
			{
				// Room wind
				
				short roomnum=MultiGetShort (data,&count);
				Rooms[roomnum].wind.x=MultiGetFloat (data,&count);
				Rooms[roomnum].wind.y=MultiGetFloat (data,&count);
				Rooms[roomnum].wind.z=MultiGetFloat (data,&count);

				mprintf ((0,"Got room wind packet! Room=%d wind=%f %f %f\n",roomnum,Rooms[roomnum].wind.x,Rooms[roomnum].wind.y,Rooms[roomnum].wind.z));
				break;
			
			}
			case WS_ROOM_FOG:
			{
				// Room wind
				short roomnum=MultiGetShort (data,&count);
				Rooms[roomnum].fog_depth=MultiGetFloat (data,&count);
				Rooms[roomnum].fog_r=((float)MultiGetUbyte (data,&count))/255.0;
				Rooms[roomnum].fog_g=((float)MultiGetUbyte (data,&count))/255.0;
				Rooms[roomnum].fog_b=((float)MultiGetUbyte (data,&count))/255.0;
				ubyte state=MultiGetUbyte (data,&count);
				if (state)
					Rooms[roomnum].flags|=RF_FOG;
				else
					Rooms[roomnum].flags&=~RF_FOG;

				break;
			}

			case WS_ROOM_LIGHTING:
			{
				// Room lighting
				ubyte state;
				short roomnum=MultiGetShort (data,&count);
				Rooms[roomnum].pulse_time=MultiGetUbyte (data,&count);
				Rooms[roomnum].pulse_offset=MultiGetUbyte (data,&count);
				state=MultiGetUbyte (data,&count);
				if (state)
					Rooms[roomnum].flags|=RF_FLICKER;
				else
					Rooms[roomnum].flags&=~RF_FLICKER;

				state=MultiGetUbyte (data,&count);
				if (state)
					Rooms[roomnum].flags|=RF_STROBE;
				else
					Rooms[roomnum].flags&=~RF_STROBE;


				break;
			}
			case WS_ROOM_REFUEL:
			{
				// Room fueling
				short roomnum=MultiGetShort (data,&count);
				ubyte state=MultiGetUbyte (data,&count);
				if (state)
					Rooms[roomnum].flags|=RF_FUELCEN;
				else
					Rooms[roomnum].flags&=~RF_FUELCEN;

				break;
			}
			case WS_ROOM_TEXTURE:
			{
				short roomnum=MultiGetShort (data,&count);
				short facenum=MultiGetShort (data,&count);
				char str[255];
				MultiGetString (str,data,&count);
				ChangeRoomFaceTexture (roomnum,facenum,FindTextureName(IGNORE_TABLE(str)));
				break;
			}
			case WS_ROOM_GLASS:
			{
				short roomnum=MultiGetShort (data,&count);
				short facenum=MultiGetShort (data,&count);
				BreakGlassFace (&Rooms[roomnum],facenum);
				break;
			}
			case WS_ROOM_PORTAL_RENDER:
			{
				short roomnum=MultiGetShort (data,&count);
				short portalnum=MultiGetShort (data,&count);
				ubyte flags=MultiGetByte (data,&count);
				Rooms[roomnum].portals[portalnum].flags=flags;
				break;
			}
			case WS_ROOM_PORTAL_BLOCK:
			{
				short roomnum=MultiGetShort (data,&count);
				short portalnum=MultiGetShort (data,&count);
				ubyte flags=MultiGetByte (data,&count);
				Rooms[roomnum].portals[portalnum].flags=flags;
				break;
			}
			case WS_ROOM_DAMAGE:
			{
				// Room wind
				short roomnum=MultiGetShort (data,&count);
				Rooms[roomnum].damage=MultiGetFloat (data,&count);
				Rooms[roomnum].damage_type=MultiGetUbyte (data,&count);
				break;
			}
			case WS_ROOM_GOALSPECFLAG:
			{
				//goals & special flags
				short roomnum=MultiGetShort (data,&count);
				int mask = (RF_SPECIAL1|RF_SPECIAL2|RF_SPECIAL3|RF_SPECIAL4|RF_SPECIAL5|RF_SPECIAL6|RF_GOAL1|RF_GOAL2|RF_GOAL3|RF_GOAL4);
				int change_mask = MultiGetInt (data,&count);

				Rooms[roomnum].flags &= ~mask;
				Rooms[roomnum].flags |= change_mask;

				break;
			}
			case WS_WAYPOINT:
			{
				Current_waypoint=MultiGetByte (data,&count);
				break;
			}
			case WS_BUDDYBOTS:
			{
				int b_index = MultiGetByte(data,&count);
				ushort serv_objnum = MultiGetUshort(data,&count);
				int local_objnum = Server_object_list[serv_objnum];

				ASSERT(Objects[local_objnum].type!=OBJ_NONE);

				Buddy_handle[b_index] = Objects[local_objnum].handle;

				//now add the guide to that player's inventory (if we need to)
				if((!Players[b_index].inventory.CheckItem(OBJ_ROBOT, ROBOT_GUIDEBOT)) && (ObjGet(Buddy_handle[b_index])->type != OBJ_ROBOT))
					Players[b_index].inventory.Add(OBJ_ROBOT, ROBOT_GUIDEBOT);
				break;
			}
			case WS_BUDDYBOTUPDATE:
			{
				int b_index = MultiGetByte(data,&count);
				ushort serv_objnum = MultiGetUshort(data,&count);
				int local_objnum = Server_object_list[serv_objnum];

				ASSERT(Objects[local_objnum].type!=OBJ_NONE);

				Buddy_handle[b_index] = Objects[local_objnum].handle;				

				break;
			}
			case WS_PLAYERBF:
			{
				Players_typing = MultiGetUint (data,&count);
				break;
			}
			case WS_MOTD:
			{
				MultiGetString (Multi_message_of_the_day,data,&count);
				AddPersistentHUDMessage(GR_RGB(255,255,255),HUD_MSG_PERSISTENT_CENTER,Game_window_y + (Game_window_h/2) - 20,10,HPF_FADEOUT+HPF_FREESPACE_DRAW,SOUND_GOAL_COMPLETE,Multi_message_of_the_day);
				break;
			}
			case WS_WEATHER:
			{
				Weather.flags=MultiGetByte (data,&count);
				Weather.rain_intensity_scalar=MultiGetFloat (data,&count);
				Weather.snow_intensity_scalar=MultiGetFloat (data,&count);
				Weather.lightning_interval_time=MultiGetFloat (data,&count);
				Weather.lightning_rand_value=MultiGetShort (data,&count);
				break;
			}
			case WS_DOOR:
			{
				doorway *dp;
				int objnum=MultiGetShort(data,&count);
				objnum=Server_object_list[objnum];
				ASSERT (Objects[objnum].type==OBJ_DOOR);
				dp = GetDoorwayFromObject(Objects[objnum].handle);

				int state=MultiGetByte(data,&count);
				ubyte locked=MultiGetByte (data,&count);
				if (locked)
					dp->flags|=DF_LOCKED;
				else
					dp->flags&=~DF_LOCKED;

				if (state== DOORWAY_OPENING || state == DOORWAY_WAITING || dp->state == DOORWAY_OPENING_AUTO)
					DoorwayActivate (Objects[objnum].handle);

				DoorwaySetPosition (Objects[objnum].handle,MultiGetFloat (data,&count));
				
				break;
			}
			case WS_LIGHT_DISTANCE:
			{
				int objnum=MultiGetShort(data,&count);
				float val=MultiGetFloat (data,&count);
				objnum=Server_object_list[objnum];

				if (objnum==65535)
					break;

				light_info *li = ObjGetLightInfo(&Objects[objnum]);
				if (li)
					li->light_distance=val;

				break;
			}
			case WS_LIGHT_COLOR:
			{
				int objnum=MultiGetShort(data,&count);
				objnum=Server_object_list[objnum];

				if (objnum==65535)
					break;

				light_info *li = ObjGetLightInfo(&Objects[objnum]);
				if (li)
				{
					li->red_light1=MultiGetFloat (data,&count);
					li->green_light1=MultiGetFloat (data,&count);
					li->blue_light1=MultiGetFloat (data,&count);
				}
				else
				{
					MultiGetFloat (data,&count);
					MultiGetFloat (data,&count);
					MultiGetFloat (data,&count);
				}

				break;
			}
			case WS_LEVELGOAL:
			{
				char name[256];
				int goal_index = MultiGetUshort(data,&count);
				int priority = MultiGetInt(data,&count);

				ubyte len=MultiGetByte (data,&count);
				memcpy (name,&data[count],len);
				count+=len;

				int flags = MultiGetInt(data,&count);
				int temp = 0xFFFFFFFF;

				mprintf((0,"Recieved Level Goal World State: %d\n",goal_index));
				Level_goals.GoalSetName(goal_index,name);				
				Level_goals.GoalStatus(goal_index,LO_CLEAR_SPECIFIED,&temp);
				Level_goals.GoalStatus(goal_index,LO_SET_SPECIFIED,&flags,false);
				Level_goals.GoalPriority (goal_index,LO_SET_SPECIFIED,&priority);

				break;
			}
			case WS_SPEW:
			{
				spewinfo spew;
				mprintf ((0,"Got a spew packet!\n"));

				ushort spewnum=MultiGetShort (data,&count);

				if (MultiGetByte (data,&count))
					spew.use_gunpoint=true;
				else
					spew.use_gunpoint=false;

				if (spew.use_gunpoint)
				{
					int objnum=MultiGetUshort (data,&count);
					objnum=Server_object_list[objnum];
					ASSERT(objnum!=65535);
					ASSERT (Objects[objnum].flags & OF_SERVER_OBJECT);

					spew.gp.obj_handle=Objects[objnum].handle;
					spew.gp.gunpoint=MultiGetByte (data,&count);
				}
				else
				{
					//memcpy (&spew.pt.normal,&data[count],sizeof(vector));
					//count+=sizeof(vector);
					spew.pt.normal = MultiGetVector(data,&count);
					//memcpy (&spew.pt.origin,&data[count],sizeof(vector));
					//count+=sizeof(vector);
					spew.pt.origin = MultiGetVector(data,&count);

					spew.pt.room_num=MultiGetInt (data,&count);
				}

				spew.random=MultiGetByte (data,&count);
				if (MultiGetByte (data,&count))
					spew.real_obj=true;
				else
					spew.real_obj=false;

				spew.effect_type=MultiGetByte (data,&count);
				spew.phys_info=MultiGetByte (data,&count);
				spew.drag=MultiGetFloat (data,&count);
				spew.mass=MultiGetFloat (data,&count);

				spew.time_int=MultiGetFloat (data,&count);
				spew.longevity=MultiGetFloat (data,&count);
				spew.lifetime=MultiGetFloat (data,&count);
				spew.size=MultiGetFloat (data,&count);
				spew.speed=MultiGetFloat (data,&count);
				
				ushort local_spewnum = SpewCreate(&spew);
				ASSERT(local_spewnum != -1);	//DAJ -1FIX
				local_spewnum&=0xFF; // Adjust for handle
				Server_spew_list[spewnum]=local_spewnum;
				mprintf ((0,"Got spew of type %d. Server=%d local=%d\n",spew.effect_type,spewnum,local_spewnum));

				break;
			}
			case WS_NO_RENDER:	
			{
				int objnum=MultiGetShort(data,&count);
				objnum=Server_object_list[objnum];

				if (objnum==65535)
					break;

				Objects[objnum].render_type=RT_NONE;
				break;
			}

			case WS_OBJECT_PHYS_FLAGS:	
			{
				int objnum=MultiGetShort(data,&count);
				int flags=MultiGetInt (data,&count);
				objnum=Server_object_list[objnum];

				if (objnum==65535)
					break;
				Objects[objnum].mtype.phys_info.flags=flags;
				break;
			}
						
			case WS_OBJECT_ATTACH:
			{
				int parent_objnum=MultiGetUshort (data,&count);
				int n_attach=MultiGetByte (data,&count);

				for (int i=0;i<n_attach;i++)
				{
					int child_objnum=MultiGetUshort (data,&count);
					if (child_objnum!=65535)
					{
						float rad;
						
						int attach_index=MultiGetByte (data,&count);
						int attach_type=MultiGetByte (data,&count);

						if (attach_type==AT_RAD)
							rad=MultiGetFloat (data,&count);
						
						child_objnum=Server_object_list[child_objnum];
						int local_objnum=Server_object_list[parent_objnum];

						if (child_objnum!=65535 && local_objnum!=65535)
						{
							if (attach_type==AT_RAD)
							{	
								AttachObject(&Objects[local_objnum],i,&Objects[child_objnum], rad);
							}
							else
							{
								bool aligned=(attach_type==AT_ALIGNED)?true:false;
								AttachObject (&Objects[local_objnum],i,&Objects[child_objnum],attach_index,aligned);
							}
							
						}
					
					}
				}
			}
			break;

			default:
				Int3(); // Unrecognized world state type, get Jason!
				break;
		}
	}

}

void MultiDoJoinDemoObjects (ubyte *data)
{
	int count=0;

	SKIP_HEADER (data,&count);

	int num_demo_objects = MultiGetUshort(data,&count);
	int objnum;
	int local_objnum;

	Multi_Expect_demo_object_flags = true;
	mprintf((0,"Processing DoJoinDemoObjects...\n"));

	for( int i = 0; i < num_demo_objects; i++ )
	{
		objnum = MultiGetUshort(data,&count);

		ASSERT(objnum>=0 && objnum<MAX_OBJECTS);

		local_objnum = Server_object_list[objnum];

		ASSERT(local_objnum>=0 && local_objnum<MAX_OBJECTS);
		ASSERT(Objects[local_objnum].type!=OBJ_NONE);
		
		Objects[local_objnum].flags|=OF_CLIENTDEMOOBJECT;
	}
}

// Server is telling us about objects in the game
void MultiDoJoinObjects (ubyte *data)
{
	int count=0;

	SKIP_HEADER (data,&count);
	ubyte num_objects=MultiGetByte (data,&count);

	mprintf ((0,"Got join object packet. Num objects=%d\n",num_objects));
	
	for (int i=0;i<num_objects;i++)
	{
		bool obj_is_dummy = false;

		// Extract info about this object
		ushort server_objnum=MultiGetUshort (data,&count);
		ubyte type=MultiGetByte (data,&count);
	

		//@@mprintf ((0,"Got join objnum %d from server. Type=%d\n",server_objnum,type));

		if (type==OBJ_DUMMY)
		{
			obj_is_dummy = true;
			type = MultiGetByte (data,&count);
		}

		uint checksum;
		matrix orient;
		ubyte name_len=0;
		ubyte num_persist_vars=0;

		vm_MakeIdentity (&orient);

		if (type!=OBJ_CAMERA && type!=OBJ_DOOR)	
		{
			// Get checksum
			checksum=MultiGetUint (data,&count);
		}

		if (type!=OBJ_POWERUP && type!=OBJ_DOOR)
		{
			multi_orientation multi_mat;

			// Get orientation
			memcpy (&multi_mat,&data[count],sizeof(multi_orientation));
			MultiMatrixMakeEndianFriendly(&multi_mat);
			MultiExtractMatrix (&orient,&multi_mat);
			count+=sizeof(multi_orientation);
		}

		int id;

		if (type!=OBJ_CAMERA && type!=OBJ_MARKER && type!=OBJ_DOOR)
		{
			id=MultiMatchGeneric (checksum);

			if (id==-1)
			{
				mprintf ((0,"Server data doesn't match client data!\n"));
				ASSERT (1);
				//Error ("Server data doesn't match client data!");
				MultiMatchGeneric (checksum);
			}
			else
			{
				MULTI_ASSERT (Object_info[id].type==type,"DoJoinObjects type mismatch!");	// Get Jason!
			}
		}
		else if (type==OBJ_MARKER)
		{
			id=checksum;
		}
		else	// camera
			id=0;

		vector pos;
		ushort short_roomnum;
		ubyte terrain=0;
		ubyte lifeleft=255;
		int roomnum;


		if (type!=OBJ_DOOR)
		{
			//memcpy (&pos,&data[count],sizeof(vector));
			//count+=sizeof(vector);
			pos = MultiGetVector(data,&count);

			short_roomnum=MultiGetUshort (data,&count);
			terrain=MultiGetByte (data,&count);
			lifeleft=MultiGetUbyte (data,&count);

			roomnum = short_roomnum;
		}

		if (type==OBJ_MARKER)
		{
			// Get message if marker
			ubyte len=MultiGetByte (data,&count);
			memcpy (MarkerMessages[id],&data[count],len);
			count+=len;
		}	

		if (terrain)
			roomnum = MAKE_ROOMNUM(roomnum);

		int local_objnum;
		
		// Don't recreate lightmap objects
		if(type!=OBJ_DOOR && (type==OBJ_CAMERA || type==OBJ_MARKER || Object_info[id].lighting_info.lighting_render_type!=LRT_LIGHTMAPS))
		{
			local_objnum = ObjCreate (type,id,roomnum,&pos,&orient);
		}
		else
		{

			ASSERT (type==Objects[server_objnum].type);
			if (type!=OBJ_DOOR)
				ObjSetPos (&Objects[server_objnum],&pos,roomnum,&orient,false);
			local_objnum=server_objnum;

			if (type==OBJ_DOOR || (type!=OBJ_CAMERA && Object_info[id].lighting_info.lighting_render_type==LRT_LIGHTMAPS))
			{
				Server_lightmap_list[Num_server_lm_objects]=server_objnum;
				Num_server_lm_objects++;
			}
		}

		if (local_objnum<0)
		{
			Error ("Ran out of objects!");
			return;
		}

		if (lifeleft!=255)
		{
			Objects[local_objnum].flags|=OF_USES_LIFELEFT;
			Objects[local_objnum].lifeleft=lifeleft;
		}

		// Put it in our lists
		Local_object_list[local_objnum]=server_objnum;
		Server_object_list[server_objnum]=local_objnum;
		
//??		InitObjectScripts (&Objects[local_objnum],false);

		// Ghost the object if it is a dummy
		if( obj_is_dummy )
		{
			ObjGhostObject(local_objnum);
		}

		Objects[local_objnum].flags|=OF_SERVER_OBJECT;
	}

}

// Starts a death sequence of a player
void MultiDoPlayerDead (ubyte *data)
{
	int count=0;

	SKIP_HEADER (data,&count);
	int slot=MultiGetByte (data,&count);
	int fate=MultiGetByte (data,&count);

	Players[slot].damage_magnitude=0;
	Players[slot].edrain_magnitude=0;

	InitiatePlayerDeath (&Objects[Players[slot].objnum],false,fate);
	if (Objects[Players[slot].objnum].effect_info->sound_handle != SOUND_NONE_INDEX) 
	{
		Sound_system.StopSoundLooping(Objects[Players[slot].objnum].effect_info->sound_handle);
		Objects[Players[slot].objnum].effect_info->sound_handle = SOUND_NONE_INDEX;
	}
}

// The server sends to everyone that the player is dead
void MultiSendPlayerDead (int slot,ubyte fate)
{
	MULTI_ASSERT (Netgame.local_role==LR_SERVER,"Client in SendPlayerDead!");
	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;
	
	size_offset=START_DATA(MP_PLAYER_DEAD,data,&count,1);
	MultiAddByte (slot,data,&count);
	MultiAddByte (fate,data,&count);
	END_DATA (count,data,size_offset);

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYERS);
	MultiDoPlayerDead (data);
	if(Game_is_master_tracker_game)
	{
		mprintf ((0,"Adding kill and death to player stats. Killer = %d Killee = %d\n",Objects[Players[slot].killer_objnum].id,slot));
		if(Objects[Players[slot].killer_objnum].id==slot)
		{
			Players[slot].suicides++;
		}
		else
		{
			Players[Objects[Players[slot].killer_objnum].id].kills++;
			Players[slot].deaths++;
		}
	}
	if(Objects[Players[slot].killer_objnum].id==slot)
	{
		Multi_deaths[slot]++;
	}
	else
	{
		Multi_kills[Objects[Players[slot].killer_objnum].id]++;
		Multi_deaths[slot]++;
	}
	ScoreAPIPlayerKilled(Objects[Players[slot].killer_objnum].id,slot);

}

// A player is coming back from the dead...restore his ship!
void MultiDoRenewPlayer (ubyte *data)
{
	int count=0;
	bool add_guidebot = false;

	SKIP_HEADER (data,&count);
	ubyte slot=MultiGetByte (data,&count);
	int start_slot=MultiGetShort (data,&count);

	if(MultiGetByte(data,&count))
	{
		add_guidebot = true;
	}	
	
	EndPlayerDeath (slot);

	PlayerMoveToStartPos (slot,start_slot);
	MultiAnnounceEffect (&Objects[Players[slot].objnum],Objects[Players[slot].objnum].size*5,1.0);

	// Make him invul for 2 seconds
	MakePlayerInvulnerable(slot,2.0);

	//add guidebot if needed
	if(add_guidebot)
	{
		mprintf((0,"MULTI: Adding guidebot to respawned player (%s)\n",Players[slot].callsign));
		if(!Players[slot].inventory.CheckItem(OBJ_ROBOT, ROBOT_GUIDEBOT))
			Players[slot].inventory.Add(OBJ_ROBOT, ROBOT_GUIDEBOT);
	}

	//Update player position in demo
	if(Demo_flags==DF_RECORDING)
	{
		DemoWriteChangedObj(&Objects[Players[slot].objnum]);
	}
}

// Tell everyone that a player is coming back from the dead
void MultiSendRenewPlayer (int slot)
{
	MULTI_ASSERT (Netgame.local_role==LR_SERVER,"Client in SendRenewPlayer!");

	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;
	int size_offset;

	size_offset=START_DATA(MP_RENEW_PLAYER,data,&count,1);
	MultiAddByte (slot,data,&count);

	// Get random start slot
	int start_slot=PlayerGetRandomStartPosition (slot);
	MultiAddShort (start_slot,data,&count);

	Player_pos_fix[slot].active = true;
	Player_pos_fix[slot].expire_time = Gametime + PLAYER_POS_HACK_TIME;
	Player_pos_fix[slot].room = Players[start_slot].start_roomnum;
	Player_pos_fix[slot].ignored_pos = 0;

	//check to see if they need to add a guidebot to the player's inventory
	if(Netgame.flags&NF_ALLOWGUIDEBOT)
	{
		if((!Players[slot].inventory.CheckItem(OBJ_ROBOT, ROBOT_GUIDEBOT)) && (ObjGet(Buddy_handle[slot])->type != OBJ_ROBOT))
		{
			MultiAddByte(1,data,&count);
		}else
		{
			MultiAddByte(0,data,&count);
		}
	}else
	{
		MultiAddByte (0,data,&count);
	}

	END_DATA (count,data,size_offset);

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYERS,false);

	MultiDoRenewPlayer (data);
}

// This player says he's done dying
void MultiDoEndPlayerDeath (ubyte *data)
{
	MULTI_ASSERT (Netgame.local_role==LR_SERVER,"Client in DoEndPlayerDeath!");

	int count=0;

	SKIP_HEADER (data,&count);
	ubyte slot=MultiGetByte (data,&count);

	if (Players[slot].flags & PLAYER_FLAGS_DEAD || Players[slot].flags & PLAYER_FLAGS_DYING)
		MultiSendRenewPlayer (slot);
}
void GetServerGameTime()
{
	MULTI_ASSERT (Netgame.local_role!=LR_SERVER,"Server in GetServerGameTime!");
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int count=0;
	int size;
	mprintf((0,"Requesting gametime from server\n"));
	Got_new_game_time=0;
	size=START_DATA(MP_GET_GAMETIME,outdata,&count);
	MultiAddFloat (timer_GetTime(),outdata,&count);
	END_DATA (count,outdata,size);
	nw_Send(&Netgame.server_address,outdata,count,0);
}
void MultiDoGameTimeReq(ubyte *data,network_address *from_addr)
{
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int count=0;
	int incount=0;
	int size;

	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	mprintf((0,"Processing request for gametime\n"));
	SKIP_HEADER (data,&incount);
	float client_time=MultiGetFloat (data,&incount);
	
	size=START_DATA(MP_HERE_IS_GAMETIME,outdata,&count);
	MultiAddFloat (client_time,outdata,&count);
	MultiAddFloat (Gametime,outdata,&count);

	END_DATA (count,outdata,size);

	nw_Send(from_addr,outdata,count,0);

}

void MultiDoSetGameTime(ubyte *data)
{
	float req_time;
	float server_latency;
	float server_game_time;
	int count=0;
	mprintf((0,"Processing and setting new for gametime\n"));
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role!=LR_SERVER);
	//Now get the latency. We calculate this by comparing the current timer to the time we got back
	//Which was the time that we sent the request.
	SKIP_HEADER (data,&count);
	req_time = MultiGetFloat(data,&count);	
	server_game_time = MultiGetFloat(data,&count);	

	//Half of the ping time is the latency
	server_latency = (timer_GetTime()-req_time)/2;
	mprintf((0,"Server Latency = %f\n",server_latency));
	
	Gametime = (server_game_time+server_latency);
	mprintf((0,"New gametime = %f\n",Gametime));
	Got_new_game_time = 1;
}

// Tell the server that I'm done dying
void MultiSendEndPlayerDeath ()
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	mprintf ((0,"Sending end player death packet!\n"));

	size_offset=START_DATA(MP_END_PLAYER_DEATH,data,&count);

	MultiAddByte (Player_num,data,&count);
	END_DATA (count,data,size_offset);

	if (Netgame.local_role==LR_CLIENT)
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
	else
		MultiDoEndPlayerDeath (data);
}

// Prints out a message we got from the server
void MultiDoMessageFromServer (ubyte *data)
{
	static int sound_id = -2;
	int count=0;
	char message[255];

	SKIP_HEADER (data,&count);


	ddgr_color color=MultiGetInt (data,&count);
	ubyte len=MultiGetByte (data,&count);

	memcpy (message,&data[count],len);
	count+=len;

	//Play the player hud message sound
	if(sound_id==-2){
		sound_id = FindSoundName("Hudmessage");
	}

	if(sound_id>-1){
		Sound_system.Play2dSound(sound_id);
	}

	AddColoredHUDMessage (color,"%s",message);	// added "%s" because message could have % signs, formatting issue.
}

// Sends a message from the server to the client
void MultiSendMessageFromServer (int color,char *message,int to)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	
	size_offset=START_DATA (MP_MESSAGE_FROM_SERVER,data,&count,1);
	MultiAddInt (color,data,&count);

	ubyte len=strlen(message)+1;

	MultiAddByte (len,data,&count);

	memcpy (&data[count],message,len);

	count+=len;

	END_DATA (count,data,size_offset);

	if (to==MULTI_SEND_MESSAGE_ALL)
	{
		MultiSendReliablyToAllExcept (Player_num,data,count,false);
		MultiDoMessageFromServer (data);
	}
	else
	{
		int team = -1;

		switch(to){
		case MULTI_SEND_MESSAGE_RED_TEAM:		team = 0;	break;
		case MULTI_SEND_MESSAGE_BLUE_TEAM:		team = 1;	break;
		case MULTI_SEND_MESSAGE_GREEN_TEAM:		team = 2;	break;
		case MULTI_SEND_MESSAGE_YELLOW_TEAM:	team = 3;	break;
		}

		if(team==-1){
			//send it off to one person
			if(to>=0 && to<MAX_PLAYERS){
				if (NetPlayers[to].flags & NPF_CONNECTED && NetPlayers[to].sequence==NETSEQ_PLAYING && to!=Player_num) {
					nw_SendReliable (NetPlayers[to].reliable_socket,data,count,false);					
				}
			}
			if(to==Player_num)
				MultiDoMessageFromServer (data);
		}else{
			//send it off only to teammates
			for(int p=0;p<MAX_PLAYERS;p++){
				if (NetPlayers[p].flags & NPF_CONNECTED && 
					NetPlayers[p].sequence==NETSEQ_PLAYING && 
					Players[p].team==team && 
					p!=Player_num){

					nw_SendReliable (NetPlayers[p].reliable_socket,data,count,false);
				}
			}
			if(team==Players[Player_num].team)
				MultiDoMessageFromServer (data);
		}	
	}

}

// Prints out a message we got from the server
void MultiDoMessageToServer (ubyte *data)
{
	int count=0;
	char message[255];

	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	int towho = (sbyte)MultiGetByte (data,&count);
	ubyte len=MultiGetByte (data,&count);

	memcpy (message,&data[count],len);
	count+=len;

	MultiSendMessageFromServer (GR_RGB(0,128,255),message,towho);
}

// Sends a message from the server to the client
void MultiSendMessageToServer (int color,char *message,int to_who)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	
	size_offset=START_DATA (MP_MESSAGE_TO_SERVER,data,&count);
	MultiAddByte (Player_num,data,&count);
	MultiAddByte (to_who,data,&count);

	ubyte len=strlen(message)+1;

	MultiAddByte (len,data,&count);

	memcpy (&data[count],message,len);

	count+=len;

	END_DATA (count,data,size_offset);
	
	nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
}


// Executes a dll that the server says to
void MultiDoExecuteDLL (ubyte *data)
{
	int count=0;
	float fParam = 0;
	int iParam = 0;
	
	SKIP_HEADER (data,&count);
		
	ushort eventnum=MultiGetShort (data,&count);
	short me_objnum=MultiGetShort (data,&count);
	short it_objnum=MultiGetShort (data,&count);

	if(MultiGetByte(data,&count)){
		//we need to extract out parameters
		fParam = MultiGetFloat (data,&count);
		iParam = MultiGetInt (data,&count);

		switch(eventnum)
		{
		case EVT_CLIENT_GAMEWALLCOLLIDE:
		case EVT_GAMEWALLCOLLIDE:
			DLLInfo.collide_info.point.x = MultiGetFloat (data,&count);
			DLLInfo.collide_info.point.y = MultiGetFloat (data,&count);
			DLLInfo.collide_info.point.z = MultiGetFloat (data,&count);
			DLLInfo.collide_info.normal.x = MultiGetFloat (data,&count);
			DLLInfo.collide_info.normal.y = MultiGetFloat (data,&count);
			DLLInfo.collide_info.normal.z = MultiGetFloat (data,&count);
			DLLInfo.collide_info.hitspeed = MultiGetFloat (data,&count);
			DLLInfo.collide_info.hit_dot = MultiGetFloat (data,&count);
			DLLInfo.collide_info.hitseg = MultiGetInt (data,&count);
			DLLInfo.collide_info.hitwall = MultiGetInt (data,&count);
			break;
		case EVT_CLIENT_GAMECOLLIDE:
		case EVT_GAMECOLLIDE:
			DLLInfo.collide_info.point.x = MultiGetFloat (data,&count);
			DLLInfo.collide_info.point.y = MultiGetFloat (data,&count);
			DLLInfo.collide_info.point.z = MultiGetFloat (data,&count);
			DLLInfo.collide_info.normal.x = MultiGetFloat (data,&count);
			DLLInfo.collide_info.normal.y = MultiGetFloat (data,&count);
			DLLInfo.collide_info.normal.z = MultiGetFloat (data,&count);
			break;
		case EVT_GAMEOBJCHANGESEG:
		case EVT_CLIENT_GAMEOBJCHANGESEG:
		case EVT_GAMEPLAYERCHANGESEG:
		case EVT_CLIENT_GAMEPLAYERCHANGESEG:
			DLLInfo.newseg = MultiGetInt (data,&count);
			DLLInfo.oldseg = MultiGetInt (data,&count);
			break;
		}
	}

	
	short local_me_objnum;
	short local_it_objnum;

	if (me_objnum==-1)
		local_me_objnum=-1;
	else
	{
		local_me_objnum=Server_object_list[me_objnum];
		if (local_me_objnum==65535)
		{
			mprintf ((0,"Invalid object in DoExecuteDLL!\n"));
			Int3();
			return;
		}
	}

	if (it_objnum==-1)
		local_it_objnum=-1;
	else
	{
		local_it_objnum=Server_object_list[it_objnum];
		if (local_it_objnum==65535)
		{
			mprintf ((0,"Invalid object in DoExecuteDLL!\n"));
			Int3();
			return;
		}
	}

	//mprintf ((0,"Received MP_EXECUTE_DLL packet! type=%d id=%d\n",Objects[local_me_objnum].type,Objects[local_me_objnum].id));

	int mehandle;
	int ithandle;

	if (local_me_objnum==-1)
		mehandle=OBJECT_HANDLE_NONE;
	else
		mehandle=Objects[local_me_objnum].handle;

	if (local_it_objnum==-1)
		ithandle=OBJECT_HANDLE_NONE;
	else
		ithandle=Objects[local_it_objnum].handle;

	DLLInfo.me_handle=mehandle;
	DLLInfo.it_handle=ithandle;
	DLLInfo.fParam = fParam;
	DLLInfo.iParam = iParam;

	CallGameDLL (eventnum,&DLLInfo);
}




// Server is telling us to create an object
void MultiDoObject (ubyte *data)
{
	int count=0;
	int parent_handle;
	bool self_parent=false;

	SKIP_HEADER (data,&count);
	
	// Extract info about this object
	ubyte announce=MultiGetByte (data,&count);
	int server_objnum=MultiGetUshort (data,&count);
	int parent_objnum = MultiGetUshort (data,&count);

	if (parent_objnum==server_objnum)
		self_parent=true;


	//mprintf((0,"MultiDoObject() got a new object. Server's objnum = %d Parent objnum = %d\n",server_objnum,parent_objnum));
	if (self_parent || parent_objnum==65535)
		parent_handle=OBJECT_HANDLE_NONE;
	else
	{
		int local_parent=Server_object_list[parent_objnum];

		if (local_parent==65535)// || !(Objects[local_parent].flags2 & OF2_SERVER_OBJECT))
		{
			MULTI_ASSERT (1,"Invalid parent for object!");
			return;
		}

		parent_handle=Objects[local_parent].handle;
	
	}
		
	ubyte type=MultiGetByte (data,&count);
	uint checksum=MultiGetUint (data,&count);
	ubyte dummy_type = OBJ_NONE;
	if(type==OBJ_DUMMY) //we need to get the original type
		dummy_type = MultiGetByte (data,&count);

	/*char name[255];
	ubyte len=MultiGetByte (data,&count);
	memcpy (name,&data[count],len);*/

	int id;
	if (type==OBJ_WEAPON)
		id=MultiMatchWeapon (checksum);
	else if (type==OBJ_MARKER)
		id=checksum;
	else
		id=MultiMatchGeneric (checksum);
		

	if (id==-1)
	{
		mprintf ((0,"Server data doesn't match!\n"));
		MULTI_ASSERT (id!=-1,"Server data doesn't match!");

		if (type==OBJ_WEAPON)
			MultiMatchWeapon (checksum);
		else if (type!=OBJ_MARKER)
			MultiMatchGeneric (checksum);

		return;
	}
	else
	{
		if (type!=OBJ_WEAPON && type!=OBJ_MARKER)
		{
			if (Object_info[id].type!=type)
			{
				BailOnMultiplayer (TXT_MULTI_CORRUPTOBJ);
				return;
			}
		}
	}

	vector pos,vel;
	matrix orient,*orientp=NULL;

	// Extract position
	//memcpy (&pos,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	pos = MultiGetVector(data,&count);

	// Extract velocity
	//memcpy (&vel,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	vel = MultiGetVector(data,&count);

	if (type!=OBJ_POWERUP && type!=OBJ_DUMMY)
	{
		// Extract orientation
		ushort p=MultiGetShort (data,&count);
		ushort h=MultiGetShort (data,&count);
		ushort b=MultiGetShort (data,&count);

		vm_AnglesToMatrix (&orient,p,h,b);
		orientp=&orient;
	}

	// Get movement type
	ubyte mtype=MultiGetByte (data,&count);

	// Get room/cell stuff
	ushort short_roomnum=MultiGetUshort (data,&count);
	ubyte terrain=MultiGetByte (data,&count);
	ubyte lifeleft=MultiGetUbyte (data,&count);
	int roomnum = short_roomnum;

	if (terrain)
		roomnum = MAKE_ROOMNUM(roomnum);

	int local_objnum;
	
	if(type!=OBJ_DUMMY)
		local_objnum = ObjCreate (type,id,roomnum,&pos,orientp,parent_handle);
	else{
		MULTI_ASSERT_NOMESSAGE (dummy_type!=OBJ_NONE && dummy_type!=OBJ_DUMMY);
		local_objnum = ObjCreate (dummy_type,id,roomnum,&pos,orientp,parent_handle);
		if(local_objnum>=0)
			ObjGhostObject(local_objnum);
	}

	if (local_objnum<0)
	{
		mprintf ((0,"Ran out of objects!\n"));
		Int3();	// Get Jason
		return;
	}

	object *obj=&Objects[local_objnum];

	if (obj->type!=OBJ_WEAPON)
		obj->flags|=OF_SERVER_OBJECT;
	obj->movement_type=mtype;
	obj->mtype.phys_info.velocity=vel;
	if (lifeleft!=255)
	{
		obj->flags|=OF_USES_LIFELEFT;
		obj->lifeleft=lifeleft;
	}
	DemoWriteObjLifeLeft(obj);
	
	obj->render_type = MultiGetByte (data,&count);

	if (obj->type==OBJ_MARKER)
	{
		// Get message if marker
		ubyte len=MultiGetByte (data,&count);
		memcpy (MarkerMessages[obj->id],&data[count],len);
		count+=len;
	}
	bool demo_record = false;	

	if(Multi_Expect_demo_object_flags)
	{
		//if Multi_Expect_demo_object_flags is false that means that
		//OEM 1.0 is the server and so it won't be packing this data at
		//the end
		demo_record = (MultiGetByte (data,&count)==0xBE)?true:false;
	}

	// Put it in our lists
	if (obj->type!=OBJ_WEAPON)
	{
		Local_object_list[local_objnum]=server_objnum;
		Server_object_list[server_objnum]=local_objnum;
	}
	
	if (announce)	// Do effect to announce
	{
		Sound_system.Play3dSound(SOUND_WALL_FADE, obj);
		CreateRandomSparks (20,&obj->pos,obj->roomnum);

		MultiAnnounceEffect (obj,obj->size*3,.7f);
	}

	// Create the scripts for it
	InitObjectScripts (&Objects[local_objnum]);

	if (type==OBJ_MARKER)
	{
		if (parent_handle==Objects[Players[Player_num].objnum].handle)
			Players[Player_num].num_markers++;
	}

	if(demo_record)
	{
		obj->flags |= OF_CLIENTDEMOOBJECT;

		if( Demo_flags==DF_RECORDING)
		{
			mprintf((0,"Recording object created on server\n"));
			//DemoWriteObjCreate(obj->type,obj->id,obj->roomnum,&obj->pos,&obj->orient,obj->parent_handle,obj);
		}
	}
	
//	mprintf((0,"MultiDoObject() Local objnum = %d\n",Server_object_list[server_objnum]));
}

// Sends an object from the server to the client
void MultiSendObject (object *obj,ubyte announce,ubyte demo_record)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	if (obj->type!=OBJ_WEAPON)
		obj->flags|=OF_CLIENT_KNOWS;

	if(demo_record)
		obj->flags|=OF_CLIENTDEMOOBJECT;

//	mprintf ((0,"Telling clients to CREATE object %d type=%d\n",obj-Objects,obj->type));

	object *parent_obj=ObjGetUltimateParent(obj);
			
	size_offset=START_DATA(MP_OBJECT,data,&count,1);
	
	uint index=MultiGetMatchChecksum (obj->type,obj->id);
	
	// Send server object number		
	MultiAddByte (announce,data,&count);
	MultiAddUshort (obj-Objects,data,&count);

	// Add parent
	if (obj->parent_handle==OBJECT_HANDLE_NONE)
		MultiAddUshort (65535,data,&count);  
	else
		MultiAddUshort ((obj->parent_handle & HANDLE_OBJNUM_MASK),data,&count);  // Add parent

	MultiAddByte (obj->type,data,&count);
	MultiAddUint (index,data,&count);
	//If it's a ghost object (type==OBJ_DUMMY) send it's old type
	if(obj->type==OBJ_DUMMY)
		MultiAddByte (obj->dummy_type,data,&count);

	// Send position
	//memcpy (&data[count],&obj->pos,sizeof(vector));
	//count+=sizeof(vector);
	MultiAddVector(obj->pos,data,&count);

	// Send velocity
	//memcpy (&data[count],&obj->mtype.phys_info.velocity,sizeof(vector));
	//count+=sizeof(vector);
	MultiAddVector(obj->mtype.phys_info.velocity,data,&count);

	if (obj->type!=OBJ_POWERUP && obj->type!=OBJ_DUMMY)
	{
		// Send over orientation
		angvec angs;
		vm_ExtractAnglesFromMatrix (&angs,&obj->orient);

		MultiAddShort (angs.p,data,&count);
		MultiAddShort (angs.h,data,&count);
		MultiAddShort (angs.b,data,&count);
	}

	// Send movement type
	MultiAddByte (obj->movement_type,data,&count);

	// Send room number and terrain flag
	MultiAddUshort (CELLNUM (obj->roomnum),data,&count);

	if (OBJECT_OUTSIDE(obj))
		MultiAddByte (1,data,&count);
	else
		MultiAddByte (0,data,&count);

	if (obj->flags & OF_USES_LIFELEFT)
	{
		MULTI_ASSERT_NOMESSAGE (obj->lifeleft<255);
		MultiAddUbyte (obj->lifeleft,data,&count);
	}
	else
		MultiAddByte (255,data,&count);

	MultiAddByte (obj->render_type,data,&count);

	if (obj->type==OBJ_MARKER)
	{
		// Add marker message to the end of this
		ubyte len = strlen(MarkerMessages[obj->id])+1;
		MultiAddByte (len,data,&count);
		memcpy (data+count,MarkerMessages[obj->id],len);
		count+=len;
	}

	MultiAddByte ((demo_record)?0xBE:0,data,&count);

	END_DATA (count,data,size_offset);

	if (announce)	// Do effect to announce
	{
		Sound_system.Play3dSound(SOUND_WALL_FADE, obj);
		CreateRandomSparks (20,&obj->pos,obj->roomnum);

		MultiAnnounceEffect (obj,obj->size*3,.7f);
	}
//	mprintf((0,"Sending object %d to clients.\n",OBJNUM(obj)));
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_OBJECTS,true);
}

void MultiDoGuidedInfo (ubyte *data)
{
	int count=0;

	// Skip header stuff
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	ubyte release=MultiGetByte (data,&count);
		
	if (Players[slot].guided_obj==NULL)
		return;

	object *obj=Players[slot].guided_obj;

	vector pos;
	matrix orient;
	ushort short_roomnum;
	int roomnum;

	MultiExtractPositionData (&pos,data,&count);
	
	// Get orientation
	ushort p=MultiGetShort (data,&count);
	ushort h=MultiGetShort (data,&count);
	ushort b=MultiGetShort (data,&count);

	vm_AnglesToMatrix (&orient,p,h,b);

	// Get room and terrain flag
	short_roomnum=MultiGetUshort (data,&count);
	ubyte terrain=MultiGetByte (data,&count);

	roomnum = short_roomnum;
	if (terrain)
		roomnum = MAKE_ROOMNUM(roomnum);

	// Get velocity
	vector vel;
	vel.x=((float)MultiGetShort (data,&count))/128.0;
	vel.y=((float)MultiGetShort (data,&count))/128.0;
	vel.z=((float)MultiGetShort (data,&count))/128.0;
	
	obj->mtype.phys_info.velocity=vel;
		
	ObjSetPos (obj,&pos,roomnum,&orient,false);

	if (release)
		ReleaseGuidedMissile (slot);
}

// Stuff info for a guided missile 
int MultiStuffGuidedInfo (int slot,ubyte *data)
{
	int count=0;
	int size_offset;
	object *obj=Players[slot].guided_obj;

	if (Players[slot].guided_obj==NULL)
		return 0;
		
	size_offset=START_DATA(MP_GUIDED_INFO,data,&count);
		
	MultiAddByte (slot,data,&count);
	MultiAddByte (0,data,&count);

		// Do position
	MultiAddPositionData (&obj->pos,data,&count);
	
	// Do orientation
	angvec angs;
	vm_ExtractAnglesFromMatrix (&angs,&obj->orient);

	MultiAddShort (angs.p,data,&count);
	MultiAddShort (angs.h,data,&count);
	MultiAddShort (angs.b,data,&count);
	
	// Do roomnumber and terrain flag
	
	MultiAddShort (CELLNUM (obj->roomnum),data,&count);

	if (OBJECT_OUTSIDE(obj))
		MultiAddByte (1,data,&count);
	else
		MultiAddByte (0,data,&count);
		
	// Do velocity
	vector *vel=&obj->mtype.phys_info.velocity;
	vector *rotvel=&obj->mtype.phys_info.rotvel;

	MultiAddShort (vel->x*128.0,data,&count);
	MultiAddShort (vel->y*128.0,data,&count);
	MultiAddShort (vel->z*128.0,data,&count);

	END_DATA (count,data,size_offset);

	return count;
}

// Guided missile release
void MultiDoMissileRelease(int slot,ubyte *data)
{
	// if we are the server, we'll have to forward this packet to the clients
	int count=0;
	unsigned char pnum_release;
	
	SKIP_HEADER (data,&count);

	bool is_guided = false;
	pnum_release = MultiGetUbyte(data,&count);
	if(pnum_release&0x80)
	{
		pnum_release &= ~0x80;
		is_guided = true;
	}

	if(Netgame.local_role==LR_SERVER)
	{
		// verify that slot and player match up
		if(pnum_release!=slot)
		{
			mprintf((0,"%s Release: Packet pnum does not match real pnum\n",(is_guided)?"Guided":"Timeout"));
			return;
		}

		// relay the packet to the clients
		MultiSendMissileRelease(pnum_release,is_guided);

		// actually perform the guided release
		if(is_guided)
			ReleaseGuidedMissile (pnum_release);
		else
			ReleaseUserTimeoutMissile (pnum_release);
	}else
	{
		// make sure we got this packet from the server
		if(slot!=0)
		{
			mprintf((0,"%s Release: got packet from non-server!?!\n",(is_guided)?"Guided":"Timeout"));
			return;
		}

		// actually perform the guided release
		if(is_guided)
			ReleaseGuidedMissile (pnum_release);
		else
			ReleaseUserTimeoutMissile (pnum_release);
	}
}

void MultiSendMissileRelease(int slot,bool is_guided)
{
	ASSERT(slot>=0 && slot<MAX_PLAYERS);

	if (!(NetPlayers[slot].flags & NPF_CONNECTED) || NetPlayers[slot].sequence!=NETSEQ_PLAYING)
		return;	// hey this player isn't in the game
	
	if(is_guided && Players[slot].guided_obj==NULL)
		return;	// no guided missile for this player
	if(!is_guided && Players[slot].user_timeout_obj==NULL)
		return;	// no user timeout object

	// everything looks ok, fire off this packet
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	unsigned char byte_to_send;
	byte_to_send = slot;
	if(is_guided)
		byte_to_send |= 0x80;
	
	size_offset=START_DATA(MP_MISSILE_RELEASE,data,&count,1);		
	MultiAddUbyte(byte_to_send,data,&count);
	END_DATA (count,data,size_offset);
	
	if(Netgame.local_role==LR_SERVER)
	{
		int i;

		// tell all the clients (except the player who is releasing)		
		for(i=0;i<MAX_NET_PLAYERS;i++)
		{	
			if(i==Player_num)	
				continue;
			if(i==slot)
				continue;

			if ((NetPlayers[i].flags & NPF_CONNECTED)&&(NetPlayers[i].sequence==NETSEQ_PLAYING))
			{
				nw_SendReliable (NetPlayers[i].reliable_socket,data,count,true);
			}
		}
	}else
	{
		// tell the server
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,true);
	}
}

// Calls the scripts packet extractor code
void MultiDoSpecialPacket (ubyte *data)
{
	int count=0;
	ubyte indata[MAX_GAME_DATA_SIZE];
	
	SKIP_HEADER (data,&count);
	int size=MultiGetInt (data,&count);

	MULTI_ASSERT_NOMESSAGE (size<MAX_GAME_DATA_SIZE);
	memcpy (indata,&data[count],size);

	DLLInfo.special_data=indata;
	CallGameDLL (EVT_CLIENT_GAMESPECIALPACKET,&DLLInfo);
}

// Sends the special script packet to a player
void MultiSendSpecialPacket (int slot,ubyte *outdata,int size)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	
	size_offset=START_DATA(MP_SPECIAL_PACKET,data,&count,1);
		
	MultiAddInt (size,data,&count);

	// Send special packet data
	memcpy (&data[count],outdata,size);
	count+=size;

	END_DATA (count,data,size_offset);
	
	nw_SendReliable (NetPlayers[slot].reliable_socket,data,count,true);
}

// Sends the special script packet to the server
void MultiClientSendSpecialPacket (ubyte *outdata,int size)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role!=LR_SERVER);
		
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	
	size_offset=START_DATA(MP_SPECIAL_PACKET,data,&count);
		
	MultiAddInt (size,data,&count);

	// Send special packet data
	memcpy (&data[count],outdata,size);
	count+=size;

	END_DATA (count,data,size_offset);
	
	nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
}

// Server is telling us to remove an object
void MultiDoRemoveObject (ubyte *data)
{
	int count=0;
	
	SKIP_HEADER (data,&count);

	ushort server_objnum=MultiGetUshort (data,&count);
	ubyte type=MultiGetByte (data,&count);
	
	ubyte sound=MultiGetByte (data,&count);

	// Get name to help track down bug
	/*#ifndef RELEASE
		char name[255];
		ubyte len=MultiGetByte (data,&count);
		memcpy (name,&data[count],len);
		count+=len;
	#endif*/

	//mprintf ((0,"Server says to remove object %d\n",server_objnum));

	int local_objnum=Server_object_list[server_objnum];
	if (local_objnum==65535)
	{
		mprintf ((0,"Client/Server object lists don't match. Something is wrong!\n"));
		//Error ("Bad object 1");
		return;

	}

	if (!(Objects[local_objnum].flags & OF_SERVER_OBJECT))
	{
		mprintf ((0,"Client/Server object lists don't match. Something is wrong!\n"));
		ASSERT (1);
		Objects[local_objnum].flags|=OF_SERVER_OBJECT;
	}
/*	if (Objects[local_objnum].type!=type)
	{
		mprintf ((0,"Client/Server object types don't match. Something is wrong!\n"));
		//mprintf ((0,"Object to be removed was type %d, I thought it was %d name: %s\n",type,Objects[local_objnum].type,name));
		Error ("Bad object 2");
		ASSERT (1);
		return;
	}*/

	if (sound)
		Sound_system.Play3dSound(SOUND_POWERUP_PICKUP, &Objects[local_objnum]);
	
	if(Objects[local_objnum].flags&OF_CLIENTDEMOOBJECT)
	{
		if(Demo_flags==DF_RECORDING)
		{
			mprintf((0,"Recording object deleted on server\n"));
			DemoWriteSetObjDead(&Objects[local_objnum]);
		}
	}

	Objects[local_objnum].flags|=OF_SERVER_SAYS_DELETE;	
	SetObjectDeadFlag (&Objects[local_objnum]);

	// Clear our lists
	int this_server_objnum=Local_object_list[local_objnum];
	if (this_server_objnum!=server_objnum)
		ASSERT (1); //Error ("Client object doesn't match remove object!");
		
	Server_object_list[server_objnum]=65535;
	Local_object_list[local_objnum]=65535;

	if (Objects[local_objnum].type==OBJ_MARKER)
	{
		if (Objects[local_objnum].parent_handle==Objects[Players[Player_num].objnum].handle)
			Players[Player_num].num_markers--;
	}
}

// Tells all clients to remove a specified object
void MultiSendRemoveObject (object *obj,ubyte playsound)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	size_offset=START_DATA(MP_REMOVE_OBJECT,data,&count,1);

	//mprintf ((0,"Telling clients to remove object %d type=%d name=%s\n",obj-Objects,obj->type,Object_info[obj->id].name));

	ASSERT (obj->flags & OF_CLIENT_KNOWS);
		
	MultiAddUshort (obj-Objects,data,&count);
	MultiAddByte (obj->type,data,&count);
	MultiAddByte (playsound,data,&count);

	/*#ifndef RELEASE
		ubyte len = strlen(Object_info[obj->id].name)+1;
		MultiAddByte (len,data,&count);
		memcpy (data+count,Object_info[obj->id].name,len);
		count+=len;
	#endif*/
	END_DATA (count,data,size_offset);

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_OBJECTS,false);
}

// Repositions a powerup to be where it should be
void MultiDoPowerupReposition (ubyte *data)
{
	int count=0;
		
	SKIP_HEADER (data,&count);

	int server_objnum=MultiGetUshort (data,&count);
	
	int local_objnum=Server_object_list[server_objnum];
	if (local_objnum==65535)
	{
		return;	// Powerup hasn't been born yet
	}
	MULTI_ASSERT_NOMESSAGE (Objects[local_objnum].type==OBJ_POWERUP);
	object *obj=&Objects[local_objnum];
	vector pos;

	//memcpy (&pos,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	pos = MultiGetVector(data,&count);

	ushort short_roomnum=MultiGetUshort (data,&count);
	ubyte terrain=MultiGetByte (data,&count);

	int roomnum = short_roomnum;
	if (terrain)
		roomnum = MAKE_ROOMNUM(roomnum);

	obj->mtype.phys_info.velocity=Zero_vector;

	ObjSetPos (obj,&pos,roomnum,&obj->orient,true);
}


// Client is telling us about weapons and energy he has
void MultiDoWeaponsLoad (ubyte *data)
{
	int count=0;
		
	SKIP_HEADER (data,&count);

	int slot=MultiGetByte(data,&count);
	
	for (int i=0;i<MAX_SECONDARY_WEAPONS;i++)
	{
		int num=MultiGetUbyte (data,&count);

		if (num<Players[slot].weapon_ammo[MAX_PRIMARY_WEAPONS+i])
			Players[slot].weapon_ammo[MAX_PRIMARY_WEAPONS+i]=num;
	}

	// Now get small views
	int objnum=MultiGetShort (data,&count);
	if (objnum==-1)
		Players[slot].small_left_obj=-1;
	else
		Players[slot].small_left_obj=objnum;

	objnum=MultiGetShort (data,&count);

	if (objnum==-1)
		Players[slot].small_right_obj=-1;
	else
		Players[slot].small_right_obj=objnum;

	Players[slot].small_dll_obj=MultiGetShort (data,&count);


}

// Tells the server about the weapons we're carrying
// Plus about the viewers we're looking at 
void MultiSendWeaponsLoad ()
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	
	int size_offset=START_DATA(MP_WEAPONS_LOAD,data,&count);
		
	MultiAddByte (Player_num,data,&count);

	for (int i=0;i<(MAX_SECONDARY_WEAPONS);i++)
	{
		int num=Players[Player_num].weapon_ammo[MAX_PRIMARY_WEAPONS+i];
		num= std::min(num,255);
		
		MultiAddUbyte (num,data,&count);
	}

	// Add small view objnums in case we're looking through someone elses eyes
	object *obj=ObjGet (GetSmallViewer(SVW_LEFT));
	if (!obj || obj->type==OBJ_WEAPON)
		MultiAddShort (-1,data,&count);
	else
		MultiAddShort (Local_object_list[GetSmallViewer(SVW_LEFT) & HANDLE_OBJNUM_MASK],data,&count);

	obj=ObjGet (GetSmallViewer(SVW_RIGHT));
	if (!obj || obj->type==OBJ_WEAPON)
		MultiAddShort (-1,data,&count);
	else
		MultiAddShort (Local_object_list[GetSmallViewer(SVW_RIGHT) & HANDLE_OBJNUM_MASK],data,&count);

	if (Players[Player_num].small_dll_obj==-1)
		MultiAddShort (-1,data,&count);
	else
		MultiAddShort (Local_object_list[Players[Player_num].small_dll_obj],data,&count);

	
	END_DATA (count,data,size_offset);

	nw_Send(&Netgame.server_address,data,count,0);
}

// Tells the other players that a slot is starting/stopping its on/off weapon
void MultiSendOnOff (object *obj,ubyte on,ubyte wb_index,ubyte fire_mask)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	int slot = obj->id;

	size_offset=START_DATA(MP_ON_OFF,data,&count);
		
	MultiAddByte (slot,data,&count);
	MultiAddByte (on,data,&count);
	MultiAddByte (wb_index,data,&count);
	MultiAddByte (fire_mask,data,&count);
	
	END_DATA (count,data,size_offset);

	if (Netgame.local_role==LR_SERVER)
		MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYING,false);
}

// Server is telling us to start/stop and on/off weapon
void MultiDoOnOff (ubyte *data)
{
	int count=0;
		
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	ubyte on=MultiGetByte (data,&count);
	ubyte wb_index=MultiGetByte (data,&count);
	ubyte fire_mask=MultiGetByte (data,&count);

	if (on)
	{
		Objects[Players[slot].objnum].weapon_fire_flags|=WFF_ON_OFF;
				
		// Clear out firing mask
		dynamic_wb_info *p_dwb = &Objects[Players[slot].objnum].dynamic_wb[wb_index];
		p_dwb->cur_firing_mask=0;

		Players[slot].weapon[PW_PRIMARY].index = wb_index;
	}
	else
		Objects[Players[slot].objnum].weapon_fire_flags&=~WFF_ON_OFF;

  
	if (Netgame.local_role==LR_SERVER)
		MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYING,false);
	if(Demo_flags == DF_RECORDING)
	{
		DemoWriteObjWeapFireFlagChanged(Players[slot].objnum);
	}
}

// Tells all the clients to apply damage to a player
void MultiSendAdditionalDamage (int slot,int type,float amount)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	size_offset=START_DATA(MP_ADDITIONAL_DAMAGE,data,&count,1);
		
	MultiAddByte (slot,data,&count);
	MultiAddByte (type,data,&count);
	MultiAddFloat (amount,data,&count);
	
	END_DATA (count,data,size_offset);

	mprintf ((0,"Sending additional damage packet of type=%d amount=%f!\n",type,amount));

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYING,false);
}

// Server is telling us to apply damage to a player
void MultiDoAdditionalDamage (ubyte *data)
{
	int count=0;

	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_CLIENT);

	//mprintf ((0,"Got additional damage packet!\n"));
		
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	ubyte type = MultiGetByte (data,&count);
	float amount=MultiGetFloat (data,&count);

	if (amount<0)	// add to shields
	{
		Objects[Players[slot].objnum].shields += (-amount);
		if (Objects[Players[slot].objnum].shields > MAX_SHIELDS)
			Objects[Players[slot].objnum].shields = MAX_SHIELDS;
	}
	else	// take damage
	{
		ApplyDamageToPlayer (&Objects[Players[slot].objnum],NULL,type,amount,1);
	}
}

// Asks the server for shields based on frametime "amount" x the type of shields requested
void MultiSendRequestShields (int type,float amount)
{
	if (Netgame.local_role!=LR_CLIENT)
		return;

	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	size_offset=START_DATA(MP_REQUEST_SHIELDS,data,&count);

	MultiAddByte (Player_num,data,&count);
	MultiAddByte (type,data,&count);
	MultiAddFloat (amount,data,&count);
		
	END_DATA (count,data,size_offset);

	nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
}

// Someone wants us to give them shields
void MultiDoRequestShields (ubyte *data)
{
	if (Netgame.local_role!=LR_SERVER)
		return;

	int count=0;

	SKIP_HEADER (data,&count);

	ubyte pnum=MultiGetByte (data,&count);
	ubyte type=MultiGetByte (data,&count);
	float amount=MultiGetFloat (data,&count);

	if (type==SHIELD_REQUEST_ENERGY_TO_SHIELD)
	{
		// This guys wants to use the energy to shield converter

		float amount_to_give;

		amount_to_give=(amount*CONVERTER_RATE)/CONVERTER_SCALE;

		if (Objects[Players[pnum].objnum].shields+amount_to_give>INITIAL_SHIELDS)
			amount_to_give=INITIAL_SHIELDS-Objects[Players[pnum].objnum].shields;

		if (amount_to_give>0)
		{
			Multi_additional_damage[pnum]=-amount_to_give;
			Multi_additional_damage_type[pnum]=PD_NONE;
		}

	}
}


// We're asking the server to damage us
void MultiSendRequestDamage (int type,float amount)
{
	if (Netgame.local_role!=LR_CLIENT)
		return;

	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	size_offset=START_DATA(MP_REQUEST_DAMAGE,data,&count);

	MultiAddByte (Player_num,data,&count);
	MultiAddByte (type,data,&count);
	MultiAddFloat (amount,data,&count);
		
	END_DATA (count,data,size_offset);
	
	nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
}

// Someone wants us to damage them
void MultiDoRequestDamage (ubyte *data)
{
	if (Netgame.local_role!=LR_SERVER)
		return;

	int count=0;

	SKIP_HEADER (data,&count);

	ubyte pnum=MultiGetByte (data,&count);
	ubyte type = MultiGetByte (data,&count);
	float amount=MultiGetFloat (data,&count);

	ApplyDamageToPlayer (&Objects[Players[pnum].objnum],&Objects[Players[pnum].objnum],type,amount);
}

// Server is telling us to create a countermeasure
void MultiDoRequestCountermeasure (ubyte *data)
{
	int count=0;

	mprintf ((0,"Got request for countermeasure!\n"));
			
	SKIP_HEADER (data,&count);

	ushort parent_objnum=MultiGetShort (data,&count);

	if (Netgame.local_role!=LR_SERVER)
	{
		parent_objnum=Server_object_list[parent_objnum];
		MULTI_ASSERT_NOMESSAGE (parent_objnum!=65535);
	}

	uint checksum=MultiGetUint (data,&count);
	
	int id=MultiMatchWeapon (checksum);

	if (id==-1)
	{
		mprintf ((0,"Server data doesn't match!\n"));
		Int3();
		MultiMatchWeapon (checksum);

		return;
	}

	int objnum=FireWeaponFromObject (&Objects[parent_objnum],id,5);

	if (objnum>=0)
	{
		if (Netgame.local_role==LR_SERVER)
			MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYERS,false);
	}

}

// We're asking the server to create a countermeasure for us
void MultiSendRequestCountermeasure (short objnum,int weapon_index)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	size_offset=START_DATA(MP_REQUEST_COUNTERMEASURE,data,&count);

	MultiAddShort (objnum,data,&count);

	uint index=MultiGetMatchChecksum (OBJ_WEAPON,weapon_index);
	MultiAddUint (index,data,&count);
		
	END_DATA (count,data,size_offset);

	if (Netgame.local_role==LR_CLIENT)
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
	else
		MultiDoRequestCountermeasure (data);
		
	mprintf ((0,"Sending out request for countermeasure!\n"));
}


// Server is telling us about a player who is changing his observer mode
void MultiDoObserverChange (ubyte *data)
{
	int count=0;
	int objnum=-1;

	SKIP_HEADER (data,&count);

	int slot=MultiGetByte (data,&count);
	ubyte mode=MultiGetByte (data,&count);
	ubyte on=MultiGetByte (data,&count);
	int restart_slot=MultiGetShort (data,&count);
	if (mode==OBSERVER_MODE_PIGGYBACK)
	{		
		if (Netgame.local_role==LR_SERVER)
			objnum=MultiGetInt (data,&count);
		else
		{
			objnum=Server_object_list [MultiGetInt(data,&count)];
			MULTI_ASSERT_NOMESSAGE (objnum!=65535);
		}
	}

	if (on)
		PlayerSwitchToObserver (slot,mode,objnum);
	else
	{
		Player_pos_fix[slot].active = true;
		Player_pos_fix[slot].expire_time = Gametime + PLAYER_POS_HACK_TIME;
		Player_pos_fix[slot].room = Players[restart_slot].start_roomnum;
		Player_pos_fix[slot].ignored_pos = 0;
		PlayerStopObserving (slot);		
		PlayerMoveToStartPos (slot,restart_slot);
	}
}

// Someone is asking us for permission to enter observer mode
void MultiDoRequestToObserve (ubyte *data)
{
	int count=0;
	int objnum;

	mprintf ((0,"Got request to observe!\n"));

	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
			
	SKIP_HEADER (data,&count);

	ubyte slot=MultiGetByte (data,&count);
	ubyte mode=MultiGetByte (data,&count);
	ubyte on=MultiGetByte (data,&count);

	if (mode==OBSERVER_MODE_PIGGYBACK)
		objnum=MultiGetInt (data,&count);
	
	if ((Objects[Players[slot].objnum].type==OBJ_PLAYER && on) || (Objects[Players[slot].objnum].type==OBJ_OBSERVER && !on))
	{
		int newcount=0;
		ubyte newdata[MAX_GAME_DATA_SIZE];
		int size_offset;

		size_offset=START_DATA(MP_OBSERVER_CHANGE,newdata,&newcount);

		MultiAddByte (slot,newdata,&newcount);
		MultiAddByte (mode,newdata,&newcount);
		MultiAddByte (on,newdata,&newcount);

		if (!on)
		{
			int start_slot=PlayerGetRandomStartPosition (slot);
			MultiAddShort (start_slot,newdata,&newcount);
		}
		else
			MultiAddShort (0,newdata,&newcount);

		if (mode==OBSERVER_MODE_PIGGYBACK)
		{
			MultiAddInt (objnum,newdata,&newcount);
		}

		END_DATA (newcount,newdata,size_offset);

		MultiSendReliablyToAllExcept (Player_num,newdata,newcount,NETSEQ_PLAYERS,false);
		MultiDoObserverChange (newdata);
	}
}

// We're asking to enter observer mode
void MultiSendRequestToObserve (int mode,int on,int objnum)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	// Check to see if the player has enough shields to enter
	if (on && Objects[Players[Player_num].objnum].shields<(INITIAL_SHIELDS/2))
	{
		AddHUDMessage (TXT_NOTENOUGHSHIELDSFOROBS);
		return;
	}

	size_offset=START_DATA(MP_REQUEST_TO_OBSERVE,data,&count);

	MultiAddByte (Player_num,data,&count);
	MultiAddByte (mode,data,&count);
	MultiAddByte (on,data,&count);

	if (mode==OBSERVER_MODE_PIGGYBACK)
	{
		if (Netgame.local_role==LR_SERVER)
			MultiAddInt (objnum,data,&count);
		else
			MultiAddInt (Local_object_list[objnum],data,&count);
	}
		
	END_DATA (count,data,size_offset);

	if (Netgame.local_role==LR_SERVER)
	{
		MultiDoRequestToObserve(data);
		return;
	}
	else
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
}

// Server is telling us about players that we can see
void MultiDoVisiblePlayers (ubyte *data)
{
	int count=0;
	int i;
	
	if (Netgame.local_role==LR_SERVER)
		return;
				
	SKIP_HEADER (data,&count);

	int bitmask=MultiGetUint (data,&count);

	MULTI_ASSERT_NOMESSAGE (MAX_NET_PLAYERS<=32);

	for (i=0;i<MAX_NET_PLAYERS;i++)
	{
		int hide_it=0;
		
		if (i==Player_num)
			continue;

		if (!(NetPlayers[i].flags & NPF_CONNECTED) || NetPlayers[i].sequence!=NETSEQ_PLAYING)
			hide_it=1;
			
		if (!(bitmask & (1<<i)) || hide_it)
		{
			if (Objects[Players[i].objnum].type==OBJ_PLAYER || hide_it)
			{
				// Kill all the sounds, render types, etc.
				Objects[Players[i].objnum].render_type=RT_NONE;
				Objects[Players[i].objnum].movement_type=MT_NONE;
				Objects[Players[i].objnum].mtype.phys_info.flags|=PF_NO_COLLIDE;
				Objects[Players[i].objnum].weapon_fire_flags&=~(WFF_ON_OFF|WFF_SPRAY);
				Players[i].flags&=~(PLAYER_FLAGS_AFTERBURN_ON|PLAYER_FLAGS_THRUSTED);
				PlayerStopSounds(i);
			}
		}
		else
		{
			if (Objects[Players[i].objnum].type==OBJ_PLAYER)
			{
				Objects[Players[i].objnum].render_type=RT_POLYOBJ;
				Objects[Players[i].objnum].movement_type=MT_PHYSICS;
				Objects[Players[i].objnum].mtype.phys_info.flags&=~PF_NO_COLLIDE;
			}
		}
	}
	
}

// Sends all the visible players to another player
void MultiSendVisiblePlayers (int to_slot)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	size_offset=START_DATA(MP_VISIBLE_PLAYERS,data,&count);

	// Always mark server as visible
	Multi_visible_players[to_slot]|=(1<<Player_num);

	MultiAddUint (Multi_visible_players[to_slot],data,&count);
	
	END_DATA (count,data,size_offset);

	nw_Send(&NetPlayers[to_slot].addr,data,count,0);
}

void MultiDoRequestPeerDamage (ubyte *data,network_address *from_addr)
{
	int count=0;

	if (Netgame.local_role!=LR_SERVER)
		return;
				
	SKIP_HEADER (data,&count);

	int pnum=MultiGetByte (data,&count);
	ushort killer_objnum=MultiGetUshort (data,&count);
	ubyte weapon_id=MultiGetUbyte (data,&count);
	ubyte damage_type=MultiGetUbyte (data,&count);
	float amount=MultiGetFloat (data,&count);

	if (killer_objnum==65535 || Objects[killer_objnum].type==OBJ_NONE)
		ApplyDamageToPlayer (&Objects[Players[pnum].objnum],&Objects[Players[pnum].objnum],damage_type,amount,1,weapon_id);
	else
		ApplyDamageToPlayer (&Objects[Players[pnum].objnum],&Objects[killer_objnum],damage_type,amount,1,weapon_id);
		
}

// Tell the server to damage us
void MultiSendRequestPeerDamage (object *killer,int weapon_id,int damage_type,float amount)
{
	int size;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;

	if (Netgame.local_role==LR_SERVER)
		return;

	mprintf ((0,"Sending request peer damage of %f\n",amount));
	
	size=START_DATA (MP_REQUEST_PEER_DAMAGE,data,&count);
	MultiAddByte (Player_num,data,&count);

	// Add killer info
	if (killer==NULL || Local_object_list[killer-Objects]==65535 || killer==&Objects[Players[Player_num].objnum])
		MultiAddUshort(65535,data,&count);
	else
		MultiAddUshort(Local_object_list[killer-Objects],data,&count);

	MultiAddUbyte (weapon_id,data,&count);
	MultiAddUbyte (damage_type,data,&count);
	MultiAddFloat (amount,data,&count);

	END_DATA (count,data,size);

	nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count);
}

// Starts a new multiplayer game
void StartNewMultiplayerGame ()
{
	SetGameState(GAMESTATE_LVLSTART);
}


// Deletes all the objects in the level except for players
// Makes all the players ghosts
void MultiMassageAllObjects (int kill_powerups,int kill_robots)
{
	int i;
	Num_powerup_respawn=0;
	Num_powerup_timer=0;
	Num_client_lm_objects=0;
	Num_server_lm_objects=0;
	mprintf ((0,"Massaging all objects!\n"));

	for (i=0;i<=Highest_object_index;i++)
	{
		// If client, delete all robots and powerups
		if (Netgame.local_role==LR_CLIENT)
		{
			if (IS_GENERIC(Objects[i].type) || Objects[i].type==OBJ_DOOR)
			{
				if (IS_GENERIC(Objects[i].type))
				{
					if(Object_info[Objects[i].id].lighting_info.lighting_render_type!=LRT_LIGHTMAPS)
					{
						Objects[i].flags|=OF_SERVER_SAYS_DELETE;
						ObjDelete(i);
					}
					else
					{
						// Add it to our lightmap list
						Client_lightmap_list[Num_client_lm_objects]=i;
						Num_client_lm_objects++;
					}
				}
				else
				{
					// Add this door to our lightmap list
					Client_lightmap_list[Num_client_lm_objects]=i;
					Num_client_lm_objects++;
				}
			}
		}

		// If Server, then only delete the types we're supposed to
		if (Netgame.local_role==LR_SERVER)
		{
			if ((Objects[i].type==OBJ_ROBOT || (Objects[i].type==OBJ_BUILDING && Objects[i].ai_info)) && kill_robots)
			{
				ObjDelete(i);
			}
			else if (Objects[i].type==OBJ_POWERUP)
			{
				if ((kill_powerups)||(!(Object_info[Objects[i].id].multi_allowed)))
					ObjDelete (i);
				else
				{
					if (Num_powerup_respawn<MAX_RESPAWNS)
					{
						Powerup_respawn[Num_powerup_respawn].pos=Objects[i].pos;
						Powerup_respawn[Num_powerup_respawn].roomnum=Objects[i].roomnum;
						Powerup_respawn[Num_powerup_respawn].objnum=i;
						Powerup_respawn[Num_powerup_respawn].used=1;
						Powerup_respawn[Num_powerup_respawn].original_id=Objects[i].id;
						Num_powerup_respawn++;
					}
				}
			}
			Objects[i].flags|=OF_CLIENT_KNOWS;
		}

		if (Objects[i].type==OBJ_PLAYER)
		{
			if (Netgame.local_role==LR_CLIENT || (Netgame.local_role==LR_SERVER && Objects[i].id!=Player_num))
			{
				MultiMakePlayerGhost (Objects[i].id);
			}
		}
	}
}

int MultiGetShipChecksum (int ship_index);
//Starts multiplayer-specific stuff for a new level
//Anything not specific to multiplayer should be in StartNewLevel()
bool MultiStartNewLevel (int level)
{
	int i;
	Time_last_taunt_request = 0;

	// Figure the ships into the checksum
	for (int i=0;i<MAX_SHIPS;i++)
		if (Ships[i].used)
			MultiGetShipChecksum(i);

	Reliable_count=0;
	Last_reliable_count=-1;
	Multi_next_level=-1;

	#ifndef RELEASE
	// Clear our packet tracking
	for (i=0;i<255;i++)
		Multi_packet_tracking[i]=0;
	
	#endif


	memset(Player_pos_fix,0,sizeof(Player_pos_fix));

	memset (Multi_building_states,0,MAX_OBJECTS);
	Multi_num_buildings_changed=0;

	memset (Multi_additional_damage,0,MAX_PLAYERS*4);
	memset (Multi_additional_shields,0,MAX_SHIELD_REQUEST_TYPES*4);
	Multi_requested_damage_amount=0;

	for (i=0;i<MAX_OBJECTS;i++)
	{
		Server_object_list[i]=65535;
		Local_object_list[i]=65535;
		Objects[i].generic_nonvis_flags=0;
	}

	for (i=0;i<MAX_SPEW_EFFECTS;i++)	
		Server_spew_list[i]=65535;

	// SCRIPT POINT!!!
	MultiMassageAllObjects (0,(Netgame.flags & NF_USE_ROBOTS)?0:1);
	MultiBuildMatchTables();
	Num_broke_glass=0;

	// Fill in player object numbers
	for (i=0;i<MAX_PLAYERS;i++)
	{
		if (Players[i].start_roomnum!=-1)
		{
			int objnum=Players[i].objnum;

			Server_object_list[objnum]=objnum;
			Local_object_list[objnum]=objnum;
			Objects[objnum].flags|=(OF_SERVER_OBJECT|OF_CLIENT_KNOWS);
		}

		InitPlayerNewLevel (i);


		// Inititialize all other players besides myself
		if (Netgame.local_role==LR_CLIENT && i!=Player_num && !(NetPlayers[i].flags & NPF_SERVER))
			NetPlayers[i].flags&=~NPF_CONNECTED;
							
	}

	if (Netgame.local_role==LR_CLIENT)
		NetPlayers[Player_num].sequence=NETSEQ_LEVEL_START;
	else
	{
		NetPlayers[Player_num].sequence=NETSEQ_PREGAME;
	}

	ResetTeamScores ();

	InitPlayerNewShip (Player_num,INVRESET_ALL);

	if(Netgame.local_role==LR_SERVER && Netgame.flags&NF_ALLOWGUIDEBOT)
	{
		//add the guidebot for the server here
		ASSERT(Buddy_handle[Player_num]!=OBJECT_HANDLE_NONE);
		if((!Players[Player_num].inventory.CheckItem(OBJ_ROBOT, ROBOT_GUIDEBOT)) && (ObjGet(Buddy_handle[Player_num])->type != OBJ_ROBOT))
			Players[Player_num].inventory.Add(OBJ_ROBOT, ROBOT_GUIDEBOT);
	}
	
	CallGameDLL (EVT_GAMELEVELSTART,&DLLInfo);
		
	return true;	
}

// Returns the number of players in a game
int MultiCountPlayers ()
{
	int i;
	int count=0;

	for (i=0;i<MAX_NET_PLAYERS;i++)
	{	
		if (NetPlayers[i].flags & NPF_CONNECTED)
			count++;
	}

	return count;
}

// Sends a full packet out the the server
// Resets the send_size variable
// Server only
void MultiSendFullPacket (int slot,int flags)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);

	if (Multi_send_size[slot]<1)
		return;
	
	
	MULTI_ASSERT_NOMESSAGE (NetPlayers[slot].flags & NPF_CONNECTED);
	nw_Send (&NetPlayers[slot].addr,Multi_send_buffer[slot],Multi_send_size[slot],flags);
	Multi_send_size[slot]=0;
}

// Sends a full packet out the the server
// Resets the send_size variable
// Client and Server
void MultiSendFullReliablePacket (int slot,int flags)
{
	if (slot==SERVER_PLAYER)
		MULTI_ASSERT_NOMESSAGE (Netgame.local_role!=LR_SERVER);

	// We're sending to the server
	if (slot==SERVER_PLAYER)
	{
		//mprintf ((0,"Sending full packet of size %d to slot%d!\n",Multi_cur_send_size,slot));
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,Multi_reliable_send_buffer[Player_num],Multi_reliable_send_size[Player_num],false);
		Multi_reliable_send_size[Player_num]=0;
		Multi_reliable_sent_position[Player_num]=0;
		Multi_reliable_last_send_time[Player_num]=0;
		Multi_reliable_urgent[Player_num]=0;
	}
	else // We are the server and we're sending to "slot"
	{
		MULTI_ASSERT_NOMESSAGE (NetPlayers[slot].flags & NPF_CONNECTED);
		nw_SendReliable (NetPlayers[slot].reliable_socket,Multi_reliable_send_buffer[slot],Multi_reliable_send_size[slot],true);
		Multi_reliable_send_size[slot]=0;
		Multi_reliable_sent_position[slot]=0;
		Multi_reliable_last_send_time[slot]=0;
		Multi_reliable_urgent[slot]=0;
	}
}


// Given a string, returns a unique integer for that string
uint MultiGetUniqueIDFromString (char *plainstring)
{
	int i,t,len;
	uint ret;
	ubyte cryptstring[PAGENAME_LEN];
	uint vals[4];
  
	len=strlen (plainstring); 

	if (len<4)
	{
		for (i=len;i<4;i++)
			plainstring[i]=('g'+i);

		len=4;
	}
	   
	for (i=0;i<4;i++)
	{
		cryptstring[i]=0; 
		ubyte backbyte;

		if (i==0)
			backbyte=0x9a;
		else
			backbyte=plainstring[i-1];

		for (t=0;t<len;t++)
		{
			ubyte a=plainstring[t];
			ubyte b=plainstring[i%(t+1)];

			a*=a;
			b*=b*b;
			
			cryptstring[i]+=a ^ b ^ (backbyte+t);
		}

		vals[i]=cryptstring[i];
	}
	
	ret=(vals[0]<<24)|(vals[1]<<16)|(vals[2]<<8)|(vals[3]);

	if (ret==0)
	{
		Int3(); // Get Jason, no match at all
		return 0;
	}
	
	return ret;
}


// Returns the unique id of a given object type/id
uint MultiGetMatchChecksum (int type,int id)
{
	switch (type)
	{
		case OBJ_POWERUP:
		case OBJ_ROBOT:
		case OBJ_CLUTTER:
		case OBJ_BUILDING:
			return Multi_generic_match_table[id];
			break;
		case OBJ_WEAPON:
			return Multi_weapon_match_table[id];
			break;
		case OBJ_MARKER:
			return id;
			break;
		default:
			Int3();	// Get Jason
	}

	Error ("Invalid type/id combo in MultiGetMatchChecksum");
	return -1;
}

// Return index of generic that has matching table entry
int MultiMatchGeneric (uint unique_id)
{
	for (int i=0;i<MAX_OBJECT_IDS;i++)
		if (Multi_generic_match_table[i]==unique_id)
			return i;

	return -1;
}

// Return index of generic that has matching table entry
int MultiMatchWeapon (uint unique_id)
{
	for (int i=0;i<MAX_WEAPONS;i++)
		if (Multi_weapon_match_table[i]==unique_id)
			return i;

	return -1;
}


// Builds the tables for uniquely identifying powerups and robots
void MultiBuildMatchTables()
{
	int i;
	
	mprintf ((0,"Building match tables for multiplayer.\n"));

	memset (Multi_generic_match_table,0,MAX_OBJECT_IDS*sizeof(int));
	memset (Multi_weapon_match_table,0,MAX_WEAPONS*sizeof(int));
	
	// Build generic tables
	for (i=0;i<MAX_OBJECT_IDS;i++)
	{
		if (Object_info[i].type!=OBJ_NONE)
		{
			uint val=MultiGetUniqueIDFromString (Object_info[i].name);
			val+=Object_info[i].type;

			// See if there is a hash collision.  If so, increment the value and retry
			while ((MultiMatchGeneric(val))!=-1)
			{

				mprintf ((0,"Match collision!\n"));
				Int3();
				val++;
			}

			Multi_generic_match_table[i]=val;
		}
		else
			Multi_generic_match_table[i]=0;
	}

	// Build weapon tables
	for (i=0;i<MAX_WEAPONS;i++)
	{
		if (Weapons[i].used)
		{
			uint val=MultiGetUniqueIDFromString (Weapons[i].name);

			// See if there is a hash collision.  If so, increment the value and retry
			while ((MultiMatchWeapon(val))!=-1)
			{
				mprintf ((0,"Match collision!\n"));
				Int3();
				val++;
			}

			Multi_weapon_match_table[i]=val;
		}
		else
			Multi_weapon_match_table[i]=0;
	}
}

// Paints all the goal rooms in a level with their colors
void MultiPaintGoalRooms (int *texcolors)
{
	int i;
	/*
	$$TABLE_TEXTURE "Redbase"
	$$TABLE_TEXTURE "Bluebase"
	$$TABLE_TEXTURE "Greenbase"
	$$TABLE_TEXTURE "Yellowbase"
	*/

	char *tnames[]={"RedBase","BlueBase","GreenBase","YellowBase"};

	for (i=0;i<MAX_TEAMS;i++)
	{
		int roomnum=GetGoalRoomForTeam(i);
		if (roomnum!=-1)
		{	
			Rooms[roomnum].room_change_flags |= RCF_TEXTURE;

			int tex;
			if (texcolors==NULL)
			{
				tex=FindTextureName(IGNORE_TABLE(tnames[i]));
				if (tex==-1)
					tex=0;
			}
			else
			{
				tex=texcolors[i];
				if (tex==-1)
					continue;
			}

			int goalfaces=0;
			int t;
			
			for (t=0;t<Rooms[roomnum].num_faces && !goalfaces;t++)
			{
				if (Rooms[roomnum].faces[t].flags & FF_GOALFACE)
				{
					goalfaces=1;
				}
			}

			for (t=0;t<Rooms[roomnum].num_faces;t++)
			{
				texture *texp=&GameTextures[Rooms[roomnum].faces[t].tmap];
				if ((!(texp->flags & TF_ALPHA)) && texp->r<.01 && texp->g<.01 && texp->b<.01)
				{
					if (!goalfaces || (goalfaces && Rooms[roomnum].faces[t].flags & FF_GOALFACE))
					{
						Rooms[roomnum].faces[t].tmap=tex;
						Rooms[roomnum].faces[t].flags |= FF_TEXTURE_CHANGED;

					}

				}
			}
		}
	}
}

void MultiSendKillObject (object *hit_obj,object *killer,float damage,int death_flags,float delay,short seed)
{
	int size;
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	ushort hit_objnum,killer_objnum;
	
	//mprintf((0,"MultiSendExplodeObject Hit obj:%d Killer Obj: %d\n",OBJNUM(hit_obj),OBJNUM(killer)));
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);

	ASSERT (hit_obj->flags & OF_CLIENT_KNOWS);
	
	size=START_DATA (MP_ROBOT_EXPLODE,data,&count,1);
	//Get the dying objnum
	hit_objnum = OBJNUM(hit_obj);
	//Get the killer objnum

	if (killer!=NULL)
		killer_objnum = OBJNUM(killer);
	else
		killer_objnum=65535;

	MultiAddUshort (hit_objnum,data,&count);
	MultiAddUshort (killer_objnum,data,&count);		
	MultiAddFloat (damage,data,&count);
	MultiAddUint( death_flags,data,&count);
	MultiAddFloat (delay,data,&count);
	MultiAddUshort (seed,data,&count);

	END_DATA (count,data,size);
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_OBJECTS,false);
}

void MultiDoRobotExplode(ubyte *data)
{
	int count = 0;
	float damage,delay;
	ushort hit_objnum,killer_objnum,seed;
	int death_flags;
	
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	
	SKIP_HEADER (data,&count);
	hit_objnum = Server_object_list[MultiGetUshort (data,&count)];

	if (hit_objnum==65535)
	{
		mprintf((0,"Bad hit_objnum(%d) MultiDoRobotExplode\n",hit_objnum));
		ASSERT (1);
		return;
	}
	
	killer_objnum = MultiGetUshort (data,&count);

	if (killer_objnum!=65535)
	{
		killer_objnum = Server_object_list[killer_objnum];

		if (killer_objnum==65535)
		{	
			mprintf((0,"Bad killer_objnum(%d) in MultiDoRobotExplode()\n",killer_objnum));
		}
	}

	damage = MultiGetFloat (data,&count);
	death_flags = MultiGetInt(data,&count);
	delay = MultiGetFloat(data,&count);
	seed = MultiGetUshort (data,&count);
	if(killer_objnum!=65535)
	{ 
		if ((Objects[killer_objnum].type!=OBJ_NONE) && (Objects[hit_objnum].type!=OBJ_NONE))
		{
			Objects[hit_objnum].flags|=OF_SERVER_SAYS_DELETE;	
			ps_srand(seed);
			KillObject(&Objects[hit_objnum],&Objects[killer_objnum],damage,death_flags,delay);	
		}
	}
	else
	{
		if ((Objects[hit_objnum].type!=OBJ_NONE))
		{
			Objects[hit_objnum].flags|=OF_SERVER_SAYS_DELETE;	
			ps_srand(seed);
			KillObject(&Objects[hit_objnum],NULL,damage,death_flags,delay);	
		}
		
	}
}

void MultiSendDamageObject (object *hit_obj,object *killer,float damage,int weaponid)
{
	int size;
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	ushort hit_objnum,killer_objnum;
	
	//mprintf((0,"MultiSendDamageObject Hit obj:%d Killer Obj: %d\n",OBJNUM(hit_obj),OBJNUM(killer)));
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
	ASSERT (hit_obj->flags & OF_CLIENT_KNOWS);
	//mprintf((0,"$"));//KBTEST
	size=START_DATA (MP_ROBOT_DAMAGE,data,&count,1);
	//Get the dying objnum
	hit_objnum = OBJNUM(hit_obj);
	//Get the killer objnum
	killer_objnum = OBJNUM(killer);
	MultiAddUshort (hit_objnum,data,&count);
	MultiAddUshort (killer_objnum,data,&count);		
	MultiAddFloat (damage,data,&count);	
	MultiAddInt (weaponid,data,&count);	

	END_DATA (count,data,size);
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYING,false);
}

void MultiDoRobotDamage(ubyte *data)
{
	int count = 0;
	float damage;
	int weaponid;
	ushort hit_objnum,killer_objnum;
	
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	SKIP_HEADER (data,&count);
	hit_objnum = Server_object_list[MultiGetUshort (data,&count)];
	killer_objnum = Server_object_list[MultiGetUshort (data,&count)];

	if (hit_objnum==65535 || killer_objnum==65535 || !(Objects[hit_objnum].flags & OF_SERVER_OBJECT) || !(Objects[killer_objnum].flags & OF_SERVER_OBJECT))
	{
		mprintf((0,"Bad hit_objnum(%d) or killer_objnum(%d) in MultiDoRobotDamage()\n",hit_objnum,killer_objnum));
		//Int3(); // Get Jason, bad object number
		return;
	}

	damage = MultiGetFloat (data,&count);
	weaponid = MultiGetInt (data,&count);
	if((!((Objects[hit_objnum].flags&OF_DYING)||(Objects[hit_objnum].flags&OF_DEAD)))&&Objects[hit_objnum].type!=255)
		if((!((Objects[killer_objnum].flags&OF_DYING)||(Objects[killer_objnum].flags&OF_DEAD)))&&Objects[killer_objnum].type!=255)
		{
			// The damage type doesn't matter on the client side (only server side) -- so, this one is bogus
			ApplyDamageToGeneric(&Objects[hit_objnum],&Objects[killer_objnum],GD_PHYSICS,damage,1,weaponid);
		}
}

void MultiAddObjAnimUpdate(int objnum)
{
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
	ASSERT(Objects[objnum].flags & OF_CLIENT_KNOWS);
	//mprintf((0,"D`"));
	for (int i=0;i<MAX_NET_PLAYERS;i++)
	{	
		if (i==Player_num)	
			continue;
		bool skip_this_obj=false;
		if ((NetPlayers[i].flags & NPF_CONNECTED)&&(NetPlayers[i].sequence==NETSEQ_PLAYING))
		{
			skip_this_obj=false;
			//Check to see if this robot is on the list already
			for(int b=0;b<Num_changed_anim[i];b++)
			{
				if(Changed_anim[b][i]==Objects[objnum].handle)
				{
					skip_this_obj=true;
					break;
				}
			}
			if(skip_this_obj==false)
			{
				if (Num_changed_anim[i]<MAX_CHANGED_OBJECTS)
				{
					Changed_anim[Num_changed_anim[i]][i] = Objects[objnum].handle;
					Num_changed_anim[i]++;
				}
			}
		}
	}
}

int MultiStuffObjAnimUpdate(unsigned short objnum, ubyte *data)
{
	custom_anim multi_anim_info;
	int count=0;
	int size=0;

	if (Netgame.local_role!=LR_SERVER)
	{
		BailOnMultiplayer (NULL);
		return 0;
	}
	
	//mprintf((0,"MultiStuffObjAnimUpdate Sending object %d\n",objnum));
	if(ObjGetAnimUpdate(objnum, &multi_anim_info)) // Checks if obj is still alive and all
	{
		ASSERT(Objects[objnum].flags & OF_CLIENT_KNOWS);
		size=START_DATA (MP_ANIM_UPDATE,data,&count);
		MultiAddUshort(objnum,data,&count);
		MultiAddFloat (multi_anim_info.server_time,data,&count);
		MultiAddUshort(multi_anim_info.server_anim_frame,data,&count);
		MultiAddByte(multi_anim_info.anim_start_frame,data,&count);
		MultiAddByte(multi_anim_info.anim_end_frame,data,&count);
		MultiAddFloat (multi_anim_info.anim_time,data,&count);
		MultiAddFloat (multi_anim_info.max_speed,data,&count);
		MultiAddByte(multi_anim_info.flags,data,&count);
		MultiAddShort(multi_anim_info.anim_sound_index,data,&count);
		END_DATA (count,data,size);
		return count;
	}
	return 0;
}

void MultiDoObjAnimUpdate(ubyte *data)
{
	custom_anim multi_anim_info;
	int objnum;
	int count = 0;
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	//mprintf((0,"R`"));
	SKIP_HEADER (data,&count);
	int serverobjnum = MultiGetUshort (data,&count);
	objnum = Server_object_list[serverobjnum];
	multi_anim_info.server_time = MultiGetFloat (data,&count);
	multi_anim_info.server_anim_frame = MultiGetUshort (data,&count);
	multi_anim_info.anim_start_frame = MultiGetByte (data,&count);
	multi_anim_info.anim_end_frame = MultiGetByte (data,&count);
	multi_anim_info.anim_time = MultiGetFloat (data,&count);
	multi_anim_info.max_speed = MultiGetFloat (data,&count);
	multi_anim_info.flags = MultiGetByte (data,&count);
	multi_anim_info.anim_sound_index = MultiGetShort (data,&count);

	if (objnum==65535)
	{
		mprintf((0,"bad objnum in MultiDoObjAnimUpdate() server=%d\n",serverobjnum));
		//Int3(); // Get Jason, bad object number
		return;
	}

	if(Objects[objnum].type!=255)
	{
		ObjSetAnimUpdate(objnum, &multi_anim_info);
	}
}

void MultiDoPlay3dSound(ubyte *data)
{
	int objnum;
	short soundidx;
	int count=0;
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	
	SKIP_HEADER (data,&count);
	int serverobjnum = MultiGetUshort (data,&count);
	objnum = Server_object_list[serverobjnum];
		
	soundidx = MultiGetShort (data,&count);
	ubyte priority=MultiGetByte (data,&count);

	if (objnum==65535 || !(Objects[objnum].flags & OF_SERVER_OBJECT))
	{
		mprintf((0,"Bad server objnum(%d) in MultiDoPlay3dSound().\n",serverobjnum));
		//Int3(); // Get Jason, bad object number
		return;
	}


	Sound_system.Play3dSound(soundidx,&Objects[objnum],priority);
}

void MultiPlay3dSound(short soundidx,ushort objnum,int priority)
{
	ubyte data[MAX_GAME_DATA_SIZE];
	int count=0;
	int size=0;
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
	ASSERT (Objects[objnum].flags & OF_CLIENT_KNOWS);
	//mprintf((0,"^"));//KBTEST
	size=START_DATA (MP_PLAY_3D_SOUND_FROM_OBJ,data,&count);
	MultiAddUshort(objnum,data,&count);
	MultiAddShort(soundidx,data,&count);
	MultiAddByte (priority,data,&count);
	END_DATA (count,data,size);
	
		//Add it to the outgoing queue. The MultiSendRobotFireSound() function will actually send it
	for (int i=0;i<MAX_NET_PLAYERS;i++)
	{	
		if (i==Player_num)	
			continue;

		if ((NetPlayers[i].flags & NPF_CONNECTED)&&(NetPlayers[i].sequence==NETSEQ_PLAYING))
		{
			if (Multi_send_size[i]+count>=MAX_GAME_DATA_SIZE)
				MultiSendFullPacket (i,0);

			memcpy (&Multi_send_buffer[i][Multi_send_size[i]],data,count);
			Multi_send_size[i]+=count;
			MultiSendFullPacket (i,0);
		}
	}
}
	
void MultiSendRobotFireSound(short soundidx,ushort objnum)
{
	int size;
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	//mprintf((0,"&"));//KBTEST
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
	//mprintf((0,"Sending Robot %d fire sound.\n",objnum));
	object *obj=&Objects[objnum];

	ASSERT (obj->flags & OF_CLIENT_KNOWS);
	
	size=START_DATA (MP_ROBOT_FIRE_SOUND,data,&count);
	MultiAddUshort (objnum,data,&count);

	MultiAddShort (soundidx,data,&count);
	
	END_DATA (count,data,size);
	
	//Add it to the outgoing queue. The MultiSendRobotFireSound() function will actually send it
	for (int i=0;i<MAX_NET_PLAYERS;i++)
	{	
		if (i==Player_num)	
			continue;

		if ((NetPlayers[i].flags & NPF_CONNECTED)&&(NetPlayers[i].sequence==NETSEQ_PLAYING))
		{
			if (Multi_send_size[i]+count>=MAX_GAME_DATA_SIZE)
				MultiSendFullPacket (i,0);

			memcpy (&Multi_send_buffer[i][Multi_send_size[i]],data,count);
			Multi_send_size[i]+=count;
			MultiSendFullPacket (i,0);
		}
	}
	
}

void MultiDoRobotFireSound(ubyte *data)
{
	int objnum;
	short soundidx;
	int count=0;
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	SKIP_HEADER (data,&count);
	
	int serverobjnum = MultiGetUshort (data,&count);
	objnum = Server_object_list[serverobjnum];
	soundidx = MultiGetShort (data,&count);

	if (objnum==65535 || !(Objects[objnum].flags & OF_SERVER_OBJECT))
	{
		mprintf((0,"Bad server objnum(%d) in MultiDoRobotFireSound().Objnum = %d\n",serverobjnum,objnum));
		//Int3(); // Get Jason, bad object number
		return;
	}
	Sound_system.Play3dSound(soundidx,&Objects[objnum]);
}

void MultiAddObjTurretUpdate(int objnum)
{

	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
	ASSERT (Objects[objnum].flags & OF_CLIENT_KNOWS);

	
	for (int i=0;i<MAX_NET_PLAYERS;i++)
	{	
		if (i==Player_num)	
			continue;
		bool skip_this_obj=false;
		if ((NetPlayers[i].flags & NPF_CONNECTED)&&(NetPlayers[i].sequence==NETSEQ_PLAYING))
		{
			skip_this_obj=false;
			//Check to see if this robot is on the list already
			for(int b=0;b<Num_changed_turret[i];b++)
			{
				if(Changed_turret[b][i]==Objects[objnum].handle)
				{
					skip_this_obj=true;
					break;
				}
			}
			if(skip_this_obj==false)
			{
				if (Num_changed_turret[i]<MAX_CHANGED_OBJECTS)
				{
					Changed_turret[Num_changed_turret[i]][i] = Objects[objnum].handle;
					Num_changed_turret[i]++;
				}
			}
		}
	}
}

int MultiStuffTurretUpdate(unsigned short objnum, ubyte *data)
{
	int count=0;
	int size=0;
	multi_turret multi_turret_info;
	//mprintf((0,"MultiStuffTurretUpdate Sending object %d\n",objnum));
	if (Netgame.local_role!=LR_SERVER)
	{
		BailOnMultiplayer (NULL);
		return 0;
	}	

	object *obj = &Objects[objnum];
	polyobj_info *p_info = &obj->rtype.pobj_info;
	if((objnum >= 0) && (obj->type != OBJ_NONE) && (obj->type != OBJ_WEAPON) &&
		(obj->flags & OF_POLYGON_OBJECT) && (p_info->multi_turret_info.num_turrets))
	{	
		int i;
		ASSERT (Objects[objnum].flags & OF_CLIENT_KNOWS);
		size=START_DATA (MP_TURRET_UPDATE,data,&count);
		MultiAddUshort (objnum,data,&count);

		multi_turret_info.keyframes = (float *)&turret_holder;
		ObjGetTurretUpdate(objnum,&multi_turret_info);

		
		MultiAddFloat (multi_turret_info.time,data,&count);
		MultiAddUshort(multi_turret_info.num_turrets,data,&count);
		
		for(i = 0; i < multi_turret_info.num_turrets; i++)
		{
			if(MAX_COOP_TURRETS>i)
				MultiAddFloat (multi_turret_info.keyframes[i],data,&count);			
		}
		END_DATA (count,data,size);
		
	}

	return count;
}

void MultiDoTurretUpdate(ubyte *data)
{
	
	multi_turret multi_turret_info;
	int objnum;
	ushort num_turrets;
	float turr_time;
	int count = 0;
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	
	SKIP_HEADER (data,&count);
	
	int serverobjnum = MultiGetUshort (data,&count);
	objnum = Server_object_list[serverobjnum];

	if (objnum==65535 || !(Objects[objnum].flags & OF_SERVER_OBJECT))
	{
		mprintf((0,"Bad server objnum(%d) in MultiDoTurretUpdate().\n",serverobjnum));
		//Int3(); // Get Jason , bad object number
		return;
	}

	turr_time = MultiGetFloat (data,&count);
	num_turrets = MultiGetUshort (data,&count);
	multi_turret_info.keyframes = (float *)&turret_holder;
	multi_turret_info.num_turrets = num_turrets;
	for(int i = 0; i < num_turrets; i++)
	{
		if(MAX_COOP_TURRETS>i)
			multi_turret_info.keyframes[i] = MultiGetFloat (data,&count);
	}
	
	if(Objects[objnum].type!=255)
	{
		ObjSetTurretUpdate(objnum, &multi_turret_info);
	}
	
}

void MultiSendClientInventoryUseItem(int type,int id)
{
	if(id==-1)
	{
		mprintf((0,"Asking server to use objnum %d\n",OBJNUM(ObjGet(type))));
	}else
	{
		mprintf((0,"Asking server to use T=%d ID=%d\n",type,id));
	}

	int size=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	//Client sends this only
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);

	if(id!=-1)
	{
		//type/id use
		size=START_DATA (MP_CLIENT_USE_INVENTORY_ITEM,data,&count,1);
		MultiAddUshort(type,data,&count);
		MultiAddUint (MultiGetMatchChecksum(type,id),data,&count);
		END_DATA (count,data,size);
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);

	}else
	{
		//object use
		int local_objhandle = type;
		object *local_obj = ObjGet(local_objhandle);
		ASSERT(local_obj);
		ASSERT(local_obj->type!=OBJ_NONE);

		if(local_obj)
		{
			int server_objnum = Local_object_list[OBJNUM(local_obj)];
			ASSERT (server_objnum!=65535);

			mprintf((0,"Inven Use: Requesting to use server objnum %d.  T=%d i=%d\n",server_objnum,local_obj->type,local_obj->id));

			size=START_DATA (MP_CLIENT_USE_INVENTORY_ITEM,data,&count,1);
			MultiAddUshort(server_objnum,data,&count);
			MultiAddUint (0xFFFFFFFF,data,&count);
			END_DATA (count,data,size);
			nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
		}
	}

}

void MultiDoClientInventoryUseItem(int slot,ubyte *data)
{
	int count=0;
	int type;
	int id;
	unsigned int hash;

	//Only the server receives this
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
	SKIP_HEADER (data,&count);
	
	type = MultiGetUshort (data,&count);
	hash = MultiGetUint(data,&count);

	bool is_counter_measure = false;

	if(hash==0xFFFFFFFF)
	{
		//object use
		int server_objnum;
		server_objnum = type;
		

		mprintf((0,"Inven: use request objnum %d\n",server_objnum));

		//the objnum coming in better be in our object list
		ASSERT( server_objnum>=0 && server_objnum<MAX_OBJECTS);
		ASSERT( Objects[server_objnum].type!=OBJ_NONE );

		mprintf((0,"Client is asking me to use objnum %d\n",server_objnum));

		//convert type to objhandle
		type = Objects[server_objnum].handle;
		id = -1;
	}else
	{
		//type/id use
		if(type==OBJ_WEAPON)
		{
			id = MultiMatchWeapon(hash);
			is_counter_measure = true;
		}else
		{
			id = MultiMatchGeneric(hash);
		}

		mprintf((0,"Client is asking me to use T=%d ID=%d\n",type,id));
	}

	if(is_counter_measure)
	{
		if(Players[slot].counter_measures.FindPos(type,id)){
			mprintf((0,"I found it, so I'm going to use it\n"));
			if(Players[slot].counter_measures.UsePos(&Objects[Players[slot].objnum])){
				//Tell clients to remove this item
				MultiSendInventoryRemoveItem(slot,type,id);
			}
		}else
		{
			mprintf((0,"But I couldn't find it in my copy of his inventory\n"));
		}
	}
	else
	{
		if(Players[slot].inventory.FindPos(type,id)){
			if(id==-1)
			{
				mprintf((0,"I found it, so I'm going to use it, type=%d id=%d\n",type,id));
			}
			else
			{
				mprintf((0,"I found it, so I'm going to use it, handle=0x%x id=%d\n",type,id));
			}

			if(Players[slot].inventory.UsePos(&Objects[Players[slot].objnum])){
				//Tell clients to remove this item
				MultiSendInventoryRemoveItem(slot,type,id);
			}
		}else
		{
			mprintf((0,"But I couldn't find it in my copy of his inventory\n"));
		}
	}
}

// Handle a remove item from inventory
void MultiDoClientInventoryRemoveItem(int slot,ubyte *data)
{
	int count=0;
	int type;
	int id;
	unsigned int hash;
	int pnum;

	//Only the server receives this
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	SKIP_HEADER (data,&count);
	
	type = MultiGetUshort (data,&count);
	hash = MultiGetUint(data,&count);
	pnum = MultiGetByte(data,&count);

	if(hash==0xFFFFFFFF)
	{
		//object given
		id = -1;

		//convert server objnum to a local objnum
		type = Server_object_list[type];

		//the objnum coming in better be in our object list
		ASSERT( type>=0 && type<MAX_OBJECTS);
		ASSERT( Objects[type].type!=OBJ_NONE );

		mprintf((0,"Server is telling me to remove objnum %d\n",type));

		//convert type to objhandle
		type = Objects[type].handle;

		Players[pnum].inventory.Remove(type,id);

	}else
	{		

		if(type==OBJ_WEAPON)
		{
			id=MultiMatchWeapon(hash);		
			mprintf((0,"Server is telling me to remove T=%d ID=%d from %d\n",type,id,pnum));
			Players[pnum].counter_measures.Remove(type,id);
		}
		else
		{
			id=MultiMatchGeneric(hash);
			mprintf((0,"Server is telling me to remove T=%d ID=%d from %d\n",type,id,pnum));
			Players[pnum].inventory.Remove(type,id);
		}
	}
}

// Tell the clients to remove an item from a player's inventory
void MultiSendInventoryRemoveItem(int slot,int type,int id)
{
	if(id==-1)
	{
		mprintf((0,"Telling clients to remove objnum %d from %d\n",OBJNUM(ObjGet(type)),slot));
	}else
	{
		mprintf((0,"Telling clients to remove inven item T=%d ID=%d from %d\n",type,id,slot));
	}

	int size=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	if(id==-1)
	{
		object *o = ObjGet(type);
		ASSERT(o);
		if(o)
		{
			int objnum = OBJNUM(o);

			MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
			size=START_DATA (MP_REMOVE_INVENTORY_ITEM,data,&count,1);
			MultiAddUshort(objnum,data,&count);
			MultiAddUint(0xFFFFFFFF,data,&count);
			MultiAddByte (slot,data,&count);
			END_DATA(count,data,size);
			MultiSendReliablyToAllExcept(Player_num,data,count,false);
		}

	}else
	{
		//type/id
		MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
		size=START_DATA (MP_REMOVE_INVENTORY_ITEM,data,&count,1);
		MultiAddUshort((ushort)type,data,&count);
		MultiAddUint (MultiGetMatchChecksum(type,id),data,&count);
		MultiAddByte (slot,data,&count);
		END_DATA(count,data,size);
		MultiSendReliablyToAllExcept(Player_num,data,count,false);
	}	
}

void MultiSetAudioTauntTime(float time,int to_who)
{
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);

	taunt_SetDelayTime(time);
	time = taunt_DelayTime();	//make sure it is valid

	int size=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;

	size=START_DATA (MP_SERVER_AUDIOTAUNT_TIME,data,&count,1);
	MultiAddFloat (time,data,&count);
	END_DATA(count,data,size);

	if(to_who==-1)
	{
		MultiSendReliablyToAllExcept(Player_num,data,count);
	}else
	{
		MULTI_ASSERT_NOMESSAGE(to_who>=0 && to_who<MAX_NET_PLAYERS);
		MULTI_ASSERT_NOMESSAGE((NetPlayers[to_who].flags & NPF_CONNECTED)&&(NetPlayers[to_who].sequence==NETSEQ_PLAYING));
		nw_SendReliable (NetPlayers[to_who].reliable_socket,data,count);
	}
}

void MultiDoAudioTauntTime(ubyte *data)
{
	int count = 0;
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);

	SKIP_HEADER (data,&count);

	float time = MultiGetFloat(data,&count);
	taunt_SetDelayTime(time);
}



void MultiAddObjWBAnimUpdate(int objnum)
{
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
	ASSERT (Objects[objnum].flags & OF_CLIENT_KNOWS);
	//mprintf((0,"D`"));
	for (int i=0;i<MAX_NET_PLAYERS;i++)
	{	
		if (i==Player_num)	
			continue;
		bool skip_this_obj=false;
		if ((NetPlayers[i].flags & NPF_CONNECTED)&&(NetPlayers[i].sequence==NETSEQ_PLAYING))
		{
			skip_this_obj=false;
			//Check to see if this robot is on the list already
			for(int b=0;b<Num_changed_wb_anim[i];b++)
			{
				if(Changed_wb_anim[b][i]==Objects[objnum].handle)
				{
					skip_this_obj=true;
					break;
				}
			}
			if(skip_this_obj==false)
			{
				if (Num_changed_wb_anim[i]<MAX_CHANGED_OBJECTS)
				{
					Changed_wb_anim[Num_changed_wb_anim[i]][i] = Objects[objnum].handle;
					Num_changed_wb_anim[i]++;
				}
			}
		}
	}
}

int MultiStuffObjWBAnimUpdate(unsigned short objnum, ubyte *data)
{
	//multi_anim multi_anim_info;
	int count=0;
	int size=0;

	if (Netgame.local_role!=LR_SERVER)
	{
		BailOnMultiplayer (NULL);
		return 0;
	}


	ASSERT (Objects[objnum].flags & OF_CLIENT_KNOWS);
	//mprintf((0,"%"));//KBTEST
	/*
	if(ObjGetAnimUpdate(objnum, &multi_anim_info)) // Checks if obj is still alive and all
	{
		size=START_DATA (MP_ANIM_UPDATE,data,&count);
		MultiAddUshort(objnum,data,&count);
		MultiAddFloat (multi_anim_info.server_time,data,&count);
		MultiAddUshort(multi_anim_info.server_anim_frame,data,&count);
		MultiAddByte(multi_anim_info.anim_start_frame,data,&count);
		MultiAddByte(multi_anim_info.anim_end_frame,data,&count);
		MultiAddFloat (multi_anim_info.anim_time,data,&count);
		MultiAddFloat (multi_anim_info.max_speed,data,&count);
		MultiAddByte(multi_anim_info.flags,data,&count);
		MultiAddShort(multi_anim_info.anim_sound_index,data,&count);
		END_DATA (count,data,size);
		return count;
	}
	*/
	return 0;
}

void MultiDoObjWBAnimUpdate(ubyte *data)
{
	//multi_anim multi_anim_info;
	//int objnum;
	int count = 0;
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	//mprintf((0,"R`"));
	SKIP_HEADER (data,&count);
	/*
	objnum = Server_object_list[MultiGetUshort (data,&count)];
	multi_anim_info.server_time = MultiGetFloat (data,&count);
	multi_anim_info.server_anim_frame = MultiGetUshort (data,&count);
	multi_anim_info.anim_start_frame = MultiGetByte (data,&count);
	multi_anim_info.anim_end_frame = MultiGetByte (data,&count);
	multi_anim_info.anim_time = MultiGetFloat (data,&count);
	multi_anim_info.max_speed = MultiGetFloat (data,&count);
	multi_anim_info.flags = MultiGetByte (data,&count);
	multi_anim_info.anim_sound_index = MultiGetShort (data,&count);
	if(Objects[objnum].type!=255)
	{
		ObjSetWBAnimUpdate(objnum, &multi_anim_info);
	}
	*/
}

void MultiSendBytesSent(int slot)
{
	int size=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	//Server sends this only
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role==LR_SERVER);
	size=START_DATA (MP_SERVER_SENT_COUNT,data,&count,1);
	MultiAddUint (NetPlayers[slot].total_bytes_sent,data,&count);
	END_DATA(count,data,size);
	NetPlayers[slot].total_bytes_sent = 0;
	NetPlayers[slot].total_bytes_rcvd = 0;
	nw_SendReliable (NetPlayers[slot].reliable_socket,data,count,false);
}

#define PPS_MAX	10
#define PPS_MIN	2
#define PING_FALLOFF	3.0

#define MIN(a,b) ((a<b)?a:b)

void MultiDoBytesSent(ubyte *data)
{
	int count = 0;
	unsigned int server_sent;
	float drop_ratio;
	SKIP_HEADER (data,&count);
	server_sent = MultiGetUint (data,&count);
	int dropped = server_sent-NetPlayers[Player_num].total_bytes_rcvd;
	if(dropped>0) 
	{
		drop_ratio = (float)NetPlayers[Player_num].total_bytes_rcvd/(float)server_sent;
		float loss = 1-drop_ratio;
		int pct_loss = loss*100;
		NetPlayers[Player_num].percent_loss=pct_loss;

		if(pct_loss>15) 
		{
			if(Bandwidth_throttle >= 0)
				Bandwidth_throttle++;
			else
				Bandwidth_throttle = 0;
		}
		else if(pct_loss<5)
		{
			if(Bandwidth_throttle <= 0)
				Bandwidth_throttle--;
			else
				Bandwidth_throttle = 0;
		}
		if(NetPlayers[Player_num].ping_time>=PING_FALLOFF)
		{
			Bandwidth_throttle--;
			Bandwidth_throttle--;
		}
		//mprintf_at((2,1,0,"Packet Loss: %d%%  ",pct_loss));
	}
	else
	{
		//mprintf_at((2,1,0,"Packet Loss: 0%%  "));
		NetPlayers[Player_num].percent_loss=0;
		Bandwidth_throttle --;
	}
	if(Bandwidth_throttle>=3)
	{
		//Time to lower the pps setting!
		if(NetPlayers[Player_num].pps>PPS_MIN)
		{
			NetPlayers[Player_num].pps--;
			MultiSendPPSSet(NetPlayers[Player_num].pps);
			Bandwidth_throttle = 0;
		}
	}
	else if(Bandwidth_throttle<= -2)
	{
		//Time to up the pps setting!
		if(NetPlayers[Player_num].pps<MIN(nw_ReccomendPPS(),Netgame.packets_per_second))
		{
			NetPlayers[Player_num].pps++;
			MultiSendPPSSet(NetPlayers[Player_num].pps);
			Bandwidth_throttle = 0;
		}
	}
	NetPlayers[Player_num].total_bytes_rcvd = 0;
}

void MultiSendPPSSet(int pps)
{
	int size=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	//Client sends this only
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	size=START_DATA (MP_CLIENT_SET_PPS,data,&count,1);
	MultiAddByte (pps,data,&count);
	mprintf((0,"Telling the server we want a PPS of %d\n",pps));
	END_DATA(count,data,size);
	nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
}

void MultiDoPPSSet(ubyte *data,int slot)
{
	int count = 0;
	SKIP_HEADER (data,&count);
	NetPlayers[slot].pps = MultiGetByte (data,&count);
	mprintf((0,"%s changed his PPS to %d\n",Players[slot].callsign,NetPlayers[slot].pps));
}

void MultiSendGreetings(unsigned int id)
{
	int size=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	//Client sends this only
	MULTI_ASSERT_NOMESSAGE(Netgame.local_role!=LR_SERVER);
	size=START_DATA (MP_GREETINGS_FROM_CLIENT,data,&count);
	MultiAddUint (id,data,&count);
	mprintf((0,"Saying hello to the server\n"));
	END_DATA(count,data,size);
	nw_Send(&Netgame.server_address,data,count,0);
}

void MultiDoGreetings (ubyte *data,network_address *addr)
{
	int count = 0;
	SKIP_HEADER (data,&count);
	unsigned int id = MultiGetUint(data,&count);
	int i;
//	mprintf((0,"Got a greeting packet with an id of %d\n"));
	for (i = 0; i < MAX_NET_PLAYERS; i++ ) 
	{
		//mprintf((0,"Netplayer[%d] has an id of %d\n",i,NetPlayers[i].secret_net_id));
		if(NetPlayers[i].secret_net_id==id)
		{
			//Get the address
			memcpy(&NetPlayers[i].addr,addr,sizeof(network_address));
			return;
		}
	}
}


// Ask for a file from the client if you are the server
// Or, if you are the server, ask a client
// file_id = NETFILE_ID_???? type
//	file_who = If you are the client who's file you want from the server
//	who = player number of who you are asking for the file
void MultiAskForFile(ushort file_id,ushort file_who,ushort who)
{
	int size=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int count = 0;
	//@@mprintf((0,"Asking player %d for a file (%d) from player %d\n",who,file_id,file_who));
	if(NetPlayers[who].file_xfer_flags != NETFILE_NONE)
	{
		mprintf((0,"File already in progress, can't start another one!\n"));
		Int3();
		return;//Make sure a file isn't in progress
	}
	if(who==Player_num)
	{
		mprintf((0,"Can't send a file to myself!\n"));
		return;
	}
	//Check to see if this file exists already
	char *p = GetFileNameFromPlayerAndID(file_who,file_id);	
	if(*p)
	{
		if(CF_ON_DISK==cfexist(p))
		{
			char szcrc[_MAX_PATH];
			char path[_MAX_PATH];
			char ext[_MAX_PATH];
			char file[_MAX_PATH];

			ddio_SplitPath(p, path, file, ext);			
			sprintf(szcrc,"_%.8x",cf_GetfileCRC(p));
			//See if the the CRC is already in the filename
			if(strnicmp(szcrc,file+(strlen(file)-9),9)!=0)
			{
				mprintf((0,"Bad CRC on file %s! It must be corrupt! File will not be used, and is being deleted!\n",p));
				ddio_DeleteFile(p);
			}
			else
			{
				//Hey hey, we already have this file
				mprintf((0,"Using existing file: %s\n",p));
				DoNextPlayerFile(file_who);
				return;
			}
		}
	}
	else
	{
		// No file
		mprintf((0,"No custom file %d for player %d\n",file_id,file_who));
		DoNextPlayerFile(file_who);
		return;
	}

	size=START_DATA (MP_FILE_REQ,data,&count);
	MultiAddUshort (file_id,data,&count);
	MultiAddUshort (file_who,data,&count);
	MultiAddUshort (Player_num,data,&count);
	END_DATA(count,data,size);
	if(Netgame.local_role==LR_SERVER)
	{
		//If we are a server, send a request to the client asking for a file of theirs
		mprintf((0,"Asking client %d for a file.\n",who));
		nw_SendReliable (NetPlayers[who].reliable_socket,data,count);
		NetPlayers[who].file_xfer_flags = NETFILE_ASKING;
		NetPlayers[who].file_xfer_pos = 0;
		NetPlayers[who].file_xfer_cfile = NULL;
		NetPlayers[who].file_xfer_who = file_who;
	}
	else
	{
		//If we are a client, ask the server for a file
		mprintf((0,"Asking server for a file.\n"));
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count);
		NetPlayers[file_who].file_xfer_flags = NETFILE_ASKING;
		NetPlayers[file_who].file_xfer_pos = 0;
		NetPlayers[file_who].file_xfer_cfile = NULL;
		NetPlayers[file_who].file_xfer_who = file_who;
	}
}

void MultiDoFileReq(ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);
	ushort filenum = MultiGetUshort (data,&count);
	ushort filewho = MultiGetUshort (data,&count);
	ushort playernum = MultiGetUshort (data,&count);	
	//If we are the server, someone want's a file. Either start sending it to them, or deny the request
	if(NetPlayers[playernum].file_xfer_flags == NETFILE_NONE)
	{
		//FIXME!! Figure out what file to open
		CFILE * cfp;
		//char filename[_MAX_PATH];
		char filewithpath[_MAX_PATH*2];
		strcpy(filewithpath,GetFileNameFromPlayerAndID(filewho,filenum));
		//ddio_MakePath(filewithpath,LocalD3Dir,"custom","cache",filename,NULL);
		if(filewithpath[0]==0)
		{
			mprintf((0,"Got a file request for a file that doesn't exist (%s).\n",filewithpath));
			DenyFile(playernum,filenum,NetPlayers[playernum].file_xfer_who);
		}
		else
		{
			cfp = cfopen(filewithpath,"rb");
			if(!cfp)
			{
				mprintf((0,"Couldn't open a file (%s) for transfer.\n",filewithpath));
				DenyFile(playernum,filenum,NetPlayers[playernum].file_xfer_who);
				return;
				// We couldn't create the file, so cancel the attempt to transfer it.
			}
			mprintf((0,"Sending first data chunk!\n"));
			NetPlayers[playernum].file_xfer_who = filewho;
			NetPlayers[playernum].file_xfer_cfile = cfp;
			NetPlayers[playernum].file_xfer_pos = 0;
			NetPlayers[playernum].file_xfer_flags = NETFILE_SENDING;
			NetPlayers[playernum].file_xfer_total_len = cfp->size;
			NetPlayers[playernum].file_xfer_id = filenum;
			mprintf((0,"File size = %d\n",cfp->size));
			SendDataChunk(playernum);
		}
	}
	else
	{
		mprintf((0,"Got a file request while one was already in progress!\n"));
		DenyFile(playernum,filenum,NetPlayers[playernum].file_xfer_who);
	}	
}

void DenyFile(int playernum,int filenum,int file_who)
{
	int size=0;
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int count = 0;
	size=START_DATA (MP_FILE_DENIED,outdata,&count);
	MultiAddUshort (filenum,outdata,&count);
	MultiAddUshort (Player_num,outdata,&count);
	MultiAddUshort (file_who,outdata,&count);
	END_DATA(count,outdata,size);
	NetPlayers[playernum].file_xfer_cfile = NULL;
	if(Netgame.local_role==LR_SERVER)
	{
		nw_SendReliable (NetPlayers[playernum].reliable_socket,outdata,count);
	}
	else
	{
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,outdata,count);
	}
}

void MultiCancelFile(int playernum,int filenum,int file_who)
{
	int size=0;
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int count = 0;
	// Send message to our peer and abort the xfer
	if(NetPlayers[playernum].file_xfer_cfile)
	{
		cfclose(NetPlayers[playernum].file_xfer_cfile);

	}

	size=START_DATA (MP_FILE_CANCEL,outdata,&count);
	MultiAddUshort (playernum,outdata,&count);
	MultiAddUshort (filenum,outdata,&count);
	MultiAddUshort (file_who,outdata,&count);
	END_DATA(count,outdata,size);
	NetPlayers[playernum].file_xfer_cfile = NULL;
	NetPlayers[playernum].file_xfer_pos = 0;
	NetPlayers[playernum].file_xfer_flags = NETFILE_NONE;
	if(Netgame.local_role==LR_SERVER)
	{
		nw_SendReliable (NetPlayers[playernum].reliable_socket,outdata,count);
	}
	else
	{
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,outdata,count);
	}

}
void MultiDoFileDenied(ubyte *data)
{
	//We asked for a file, but the request was denied for some reason
	int count = 0;
	SKIP_HEADER (data,&count);
	ushort filenum = MultiGetUshort (data,&count);
	ushort playernum = MultiGetUshort (data,&count);
	ushort filewho = MultiGetUshort (data,&count);
	mprintf((0,"Got a file denied packet from %d\n",playernum));

	DoNextPlayerFile(filewho);
}

void MultiDoFileData(ubyte *data)
{
	//File data. We asked for it, now the server is sending it to us.
	unsigned int total_len;	// Length of the entire file
	unsigned int curr_len;  // Length of file sent so far
	ushort	file_id; 	// Defines which file this is
	ushort playernum;	// Who is sending us the file
	ushort	data_len;	// between 1-450 bytes
	//ushort	file_who;
	int count = 0;
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int outcount = 0;
	int size;

	SKIP_HEADER (data,&count);
	total_len = MultiGetUint(data,&count);
	curr_len = MultiGetUint(data,&count); //Length of the file after before this data chunk
	file_id = MultiGetUshort (data,&count);
	playernum = MultiGetUshort (data,&count);
	//file_who = MultiGetUshort (data,&count);
	data_len = MultiGetUshort (data,&count);
	//mprintf((0,"Got a data packet from %d\n",playernum));
	if((NetPlayers[playernum].file_xfer_flags == NETFILE_RECEIVING)||(NetPlayers[playernum].file_xfer_flags == NETFILE_ASKING))
	{
		
		//Find out if this is the first packet of this file. if so, create and open the file
		if(NetPlayers[playernum].file_xfer_pos == 0)
		{
			mprintf((0,"Creating file...\n"));
			CFILE * cfp;
			//char filename[_MAX_PATH];
			char filewithpath[_MAX_PATH*2];
			strcpy(filewithpath,GetFileNameFromPlayerAndID(playernum,file_id));
			//ddio_MakePath(filewithpath,LocalD3Dir,"custom","cache",filename,NULL);
			
			cfp = cfopen(filewithpath,"wb");
			if(!cfp)
			{
				mprintf((0,"Can't create file %s\n",filewithpath));
				// We couldn't create the file, so cancel the attempt to transfer it.
				MultiCancelFile(playernum,file_id,NetPlayers[playernum].file_xfer_who);
				return;
			}
			NetPlayers[playernum].file_xfer_cfile = cfp;
			NetPlayers[playernum].file_xfer_flags = NETFILE_RECEIVING;			
		}

		MULTI_ASSERT_NOMESSAGE(NetPlayers[playernum].file_xfer_cfile);
		//Write the data to the file
		int data_wrote = cf_WriteBytes(data+count,data_len,NetPlayers[playernum].file_xfer_cfile);
		if(data_wrote!=data_len)
		{
			mprintf((0,"cf_WriteBytes() should have written %d bytes, but only wrote %d!\n",data_len,data_wrote));
			Int3();
		}
			
		NetPlayers[playernum].file_xfer_pos+=data_len;
		//See if this is the last data chunk
		if(NetPlayers[playernum].file_xfer_pos>=total_len)
		{
			//We got the whole thing baby!
			cfclose(NetPlayers[playernum].file_xfer_cfile);
			NetPlayers[playernum].file_xfer_cfile = NULL;
			NetPlayers[playernum].file_xfer_pos = 0;
			DoNextPlayerFile(playernum);		
			mprintf((0,"Finished downloading file!\n"));
			return; // Don't ack the last packet
		}
		//Ack the sender
		size=START_DATA (MP_FILE_ACK,outdata,&outcount);
		MultiAddUshort (Player_num,outdata,&outcount);
		MultiAddUint (NetPlayers[playernum].file_xfer_pos,outdata,&outcount);
		END_DATA(outcount,outdata,size);
		if(Netgame.local_role==LR_SERVER)
		{
			nw_SendReliable (NetPlayers[playernum].reliable_socket,outdata,outcount,true);
		}
		else
		{
			nw_SendReliable (NetPlayers[Player_num].reliable_socket,outdata,outcount,true);
		}
	}
	else
	{
		mprintf((0,"Received file transfer data from someone who we aren't expecting data from!\n"));
	}
}

void MultiDoFileAck(ubyte *data)
{
	//If we are transferring a file, and someone ACK's us, simply send them the next bit of data they are waiting for
	ubyte	playernum;	// Who is acking us
	uint	len_recvd;	// Total number of bytes received so far
	int count = 0;
	
	SKIP_HEADER (data,&count);
	playernum = MultiGetUshort (data,&count);
	len_recvd = MultiGetUint(data,&count);
	//mprintf((0,"Got a ACK packet from %d\n",playernum));	
	if(NetPlayers[playernum].file_xfer_flags == NETFILE_SENDING)
	{
		if(NetPlayers[playernum].file_xfer_pos == len_recvd)
		{
			SendDataChunk(playernum);
		}
		else
		{
			Int3();//This shouldn't happen because the reliable network code should handle it
		}
	}
	else
	{
		mprintf((0,"Received an ACK from someone we weren't sending a file to!\n"));
	}
}


void SendDataChunk(int playernum)
{
	int outcount = 0;
	static ubyte outdata[MAX_PACKET_SIZE];
	static ubyte readbuf[MAX_PACKET_SIZE];
	int size;
	int dataread;
	int done=0;
	//mprintf((0,"Sending a data chunk to %d.\n",playernum));
	//Read the next chunk of the file and send it!
	if((DATA_CHUNK_SIZE+NetPlayers[playernum].file_xfer_pos)>(unsigned int)NetPlayers[playernum].file_xfer_cfile->size)
	{
		dataread = NetPlayers[playernum].file_xfer_cfile->size-NetPlayers[playernum].file_xfer_pos;
		//This is the end of the file 
		mprintf((0,"End of file detected!\n"));
		done = 1;
	}
	else
	{
		dataread = DATA_CHUNK_SIZE;
	}

	try
	{
		cf_ReadBytes(readbuf,dataread,NetPlayers[playernum].file_xfer_cfile);
	}
	catch(cfile_error *cfe)
	{
		int t=cfe->read_write;	//this is is here to fix compile warning
		//Woops, can't read the file, better error out!
		MultiCancelFile(playernum,NetPlayers[playernum].file_xfer_id,NetPlayers[playernum].file_xfer_who);
		Int3();
		return;
	}

	if(done)
	{
		cfclose(NetPlayers[playernum].file_xfer_cfile);
		NetPlayers[playernum].file_xfer_cfile = NULL;
		NetPlayers[playernum].file_xfer_pos = 0;
		NetPlayers[playernum].file_xfer_flags = NETFILE_NONE;
	}
	//Send them the next chunk
	size=START_DATA (MP_FILE_DATA,outdata,&outcount);
	MultiAddUint (NetPlayers[playernum].file_xfer_total_len,outdata,&outcount);
	MultiAddUint (NetPlayers[playernum].file_xfer_pos,outdata,&outcount);
	MultiAddUshort (NetPlayers[playernum].file_xfer_id,outdata,&outcount);
	if(Netgame.local_role==LR_SERVER)
		MultiAddUshort (NetPlayers[playernum].file_xfer_who,outdata,&outcount);
	else
		MultiAddUshort (Player_num,outdata,&outcount);
	//MultiAddUshort (NetPlayers[playernum].file_xfer_who,outdata,&outcount);	
	MultiAddUshort (dataread,outdata,&outcount);
	memcpy(outdata+outcount,readbuf,dataread);
	outcount += dataread;
	END_DATA(outcount,outdata,size);
	if(Netgame.local_role==LR_SERVER)
	{
		nw_SendReliable (NetPlayers[playernum].reliable_socket,outdata,outcount,true);
	}
	else
	{
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,outdata,outcount,true);
	}
	NetPlayers[playernum].file_xfer_pos += dataread;
}

void MultiDoFileCancelled(ubyte *data)
{
	unsigned short	playernum;	// Who is telling us the file is cancelled
	unsigned short filewho;		// Who's file is being cancelled
	int count = 0;
	
	SKIP_HEADER (data,&count);
	playernum = MultiGetUshort (data,&count);
	filewho = MultiGetUshort (data,&count);
	mprintf((0,"Got a cancelled packet from %d for %d\n",playernum,filewho));		
	if(NetPlayers[filewho].file_xfer_cfile)
	{
		char delfile[_MAX_PATH*2];
		strcpy(delfile,NetPlayers[filewho].file_xfer_cfile->name);
		cfclose(NetPlayers[filewho].file_xfer_cfile);
		NetPlayers[filewho].file_xfer_cfile = NULL;				
		ddio_DeleteFile(delfile);
	}
	NetPlayers[filewho].file_xfer_cfile = NULL;
	NetPlayers[filewho].file_xfer_pos = 0;
	DoNextPlayerFile(filewho);
}

//ddio_splitpath
void MultiSendClientCustomData(int slot,int whoto)
{
	ubyte data[MAX_GAME_DATA_SIZE];
	char csum_filename[_MAX_PATH];
	char path[_MAX_PATH];
	char ext[_MAX_PATH];
	char file[_MAX_PATH];
	int count=0;
	int size_offset;
	
	//@@mprintf((0,"Sending player %d's data to %d.\n",slot,whoto));

	size_offset=START_DATA(MP_PLAYER_CUSTOM_DATA,data,&count);
	//Who we are talking about
	MultiAddShort (slot,data,&count);

	//Send custom logo filename (all 0's if none)
	csum_filename[0]=NULL;
	if(NetPlayers[slot].ship_logo[0])
	{
		if(slot==Player_num)
		{
			char szcrc[_MAX_PATH];
			ddio_SplitPath(NetPlayers[slot].ship_logo, path, file, ext);
			sprintf(szcrc,"_%.8x",cf_GetfileCRC(NetPlayers[slot].ship_logo));
			//See if the the CRC is already in the filename
			if(strnicmp(szcrc,file+(strlen(file)-9),9)==0)
			{
				sprintf(csum_filename,"%s%s",file,ext);
			}
			else
			{
				sprintf(csum_filename,"%s_%.8x%s",file,cf_GetfileCRC(NetPlayers[slot].ship_logo),ext);
			}
		}
		else
		{
			strcpy(csum_filename,NetPlayers[slot].ship_logo);
		}
	}
	unsigned short logo_len = strlen(csum_filename)+1;
	MultiAddUshort (logo_len,data,&count);
	memcpy (data+count,csum_filename,logo_len);
	count+=logo_len;

	for(int t=0;t<4;t++)
	{
		//Send custom voice taunts (all 0's if none)
		csum_filename[0]='\0';
		char *filename;

		switch(t)
		{
		case 0:
			filename = NetPlayers[slot].voice_taunt1;
			break;
		case 1:
			filename = NetPlayers[slot].voice_taunt2;
			break;
		case 2:
			filename = NetPlayers[slot].voice_taunt3;
			break;
		case 3:
			filename = NetPlayers[slot].voice_taunt4;
			break;
		}

		if(filename[0])
		{
			if(slot==Player_num)
			{
				char szcrc[_MAX_PATH];
				ddio_SplitPath(filename, path, file, ext);
				sprintf(szcrc,"_%.8x",cf_GetfileCRC(filename));
				//See if the the CRC is already in the filename
				if(strnicmp(szcrc,file+(strlen(file)-9),9)==0)
				{
					sprintf(csum_filename,"%s%s",file,ext);
				}
				else
				{
					sprintf(csum_filename,"%s_%.8x%s",file,cf_GetfileCRC(filename),ext);
				}
			}
			else
			{
				strcpy(csum_filename,filename);
			}
		}
		unsigned short vt_len = strlen(csum_filename)+1;
		MultiAddUshort (vt_len,data,&count);
		memcpy (data+count,csum_filename,vt_len);
		count+=vt_len;
	}
		
	END_DATA (count,data,size_offset);

	if(Netgame.local_role==LR_SERVER)
	{
		if(whoto==-1)
		{
			//Send to all clients
			MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYING);
		}
		else
		{
			nw_SendReliable (NetPlayers[whoto].reliable_socket,data,count,true);
		}
	}
	else
	{
		//Send to the server
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,true);
	}
}



void MultiDoCustomPlayerData(ubyte *data)
{
	unsigned short	playernum;	// Who has data we are interested in
	int count = 0;
	
	SKIP_HEADER (data,&count);
	playernum = MultiGetUshort (data,&count);
	if(playernum==Player_num)
		return;
	mprintf((0,"Got custom data in MultiDoCustomPlayerData()\n"));
	NetPlayers[playernum].custom_file_seq = NETFILE_ID_SHIP_TEX;//First in the sequence of files we will request
	short logo_len = MultiGetUshort (data,&count);
	memcpy (NetPlayers[playernum].ship_logo,data+count,logo_len);
	count+=logo_len;
	mprintf((0,"%s uses custom ship logo %s\n",Players[playernum].callsign,NetPlayers[playernum].ship_logo));

	for(int t=0;t<4;t++)
	{
		char *filename;
		short vt_len;

		switch(t)
		{
		case 0:
			filename = NetPlayers[playernum].voice_taunt1;
			break;
		case 1:
			filename = NetPlayers[playernum].voice_taunt2;
			break;
		case 2:
			filename = NetPlayers[playernum].voice_taunt3;
			break;
		case 3:
			filename = NetPlayers[playernum].voice_taunt4;
			break;
		}

		vt_len = MultiGetUshort (data,&count);
		memcpy (filename,data+count,vt_len);
		count+=vt_len;
	}			
}


char * GetFileNameFromPlayerAndID(short playernum,short id)
{
	static char rval[_MAX_PATH*2];

	rval[0]=NULL;

	if(playernum>=MAX_NET_PLAYERS)
	{
		mprintf((0,"Invalid playernum (%d) passed to GetFileNameFromPlayerAndID()\n",playernum));
		return rval;
	}
	if(id>NETFILE_ID_LAST_FILE)
	{
		mprintf((0,"Invalid file id (%d) passed to GetFileNameFromPlayerAndID()\n",id));
		return rval;
	}
	switch(id)
	{

	case NETFILE_ID_NOFILE:
		break;
	case NETFILE_ID_SHIP_TEX:
		if(NetPlayers[playernum].ship_logo[0])
			ddio_MakePath(rval,User_directory,"custom","graphics",NetPlayers[playernum].ship_logo,NULL);
		break;
	case NETFILE_ID_VOICE_TAUNT1:
		if(NetPlayers[playernum].voice_taunt1[0])
			ddio_MakePath(rval, User_directory,"custom","sounds",NetPlayers[playernum].voice_taunt1,NULL);
		break;
	case NETFILE_ID_VOICE_TAUNT2:
		if(NetPlayers[playernum].voice_taunt2[0])
			ddio_MakePath(rval, User_directory,"custom","sounds",NetPlayers[playernum].voice_taunt2,NULL);
		break;
	case NETFILE_ID_VOICE_TAUNT3:
		if(NetPlayers[playernum].voice_taunt3[0])
			ddio_MakePath(rval, User_directory,"custom","sounds",NetPlayers[playernum].voice_taunt3,NULL);
		break;
	case NETFILE_ID_VOICE_TAUNT4:
		if(NetPlayers[playernum].voice_taunt4[0])
			ddio_MakePath(rval, User_directory,"custom","sounds",NetPlayers[playernum].voice_taunt4,NULL);
		break;
	default:
		mprintf((0,"Unknown id (%d) passed to GetFileNameFromPlayerAndID()\n",id));
		Int3();
		break;
	}
	if(*rval)
	{
		CFILE *cfp;
		cfp = cfopen(rval,"rb");
		if(!cfp)
		{
			mprintf((0,"Multiplayer file xfer File does not exist, not using file %d for player %d!\n",id,playernum));
			//rval[0] = NULL;
		}
		else if(32768<cfilelength(cfp))
		{
			mprintf((0,"Multiplayer file xfer File to long, not using file %d for player %d!\n",id,playernum));
			rval[0] = NULL;
		}
		if(cfp)
			cfclose(cfp);
	}
	
	
	return rval;

}


// Tells the clients to ghost or unghost an object
void MultiSendGhostObject( object *obj, bool ghost)
{
	MULTI_ASSERT_NOMESSAGE( obj );

	int count=0;
	ushort objnum;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	size_offset=START_DATA(MP_GHOST_OBJECT,data,&count,1);

	objnum = obj-Objects;

	MultiAddUshort (objnum,data,&count);
	MultiAddByte ((ghost)?1:0,data,&count);

	END_DATA (count,data,size_offset);

	if (Netgame.local_role==LR_SERVER)
		MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_OBJECTS,false);
}

void MultiDoGhostObject (ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	ushort server_objnum,objnum;
	bool ghost;

	server_objnum = MultiGetUshort(data,&count);
	ghost = (MultiGetByte(data,&count))?true:false;

	objnum = Server_object_list[server_objnum];

	if (objnum==65535)
	{
		Int3(); // Get Jason, bad object number
		return;
	}

	MULTI_ASSERT_NOMESSAGE (objnum!=-1);
	MULTI_ASSERT_NOMESSAGE ( Objects[objnum].type!=OBJ_NONE );

	if(ghost){
		ObjGhostObject(objnum);
	}else{
		ObjUnGhostObject(objnum);
	}
}

// Sends a ping request to a player
void MultiSendPing( int slot)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	if(slot==Player_num)
		return;
	NetPlayers[slot].last_ping_time = timer_GetTime();
	size_offset=START_DATA(MP_PLAYER_PING,data,&count);

	MultiAddByte (Player_num,data,&count);
	MultiAddFloat (NetPlayers[slot].last_ping_time,data,&count);
	END_DATA (count,data,size_offset);
	nw_Send (&NetPlayers[slot].addr,data,count,0);
}

void MultiDoPing (ubyte *data,network_address *addr)
{
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int outcount = 0;
	int count = 0;
	int size_offset;

	SKIP_HEADER (data,&count);
	ubyte slot = MultiGetByte (data,&count);
	float pingtime = MultiGetFloat (data,&count); 
	
	// Now send a response
	size_offset=START_DATA(MP_PLAYER_PONG,outdata,&outcount);
	MultiAddByte (Player_num,outdata,&outcount);
	MultiAddFloat (pingtime,outdata,&outcount);
	END_DATA (outcount,outdata,size_offset);

	//mprintf((0, "got ping packet, timer of %f\n", pingtime));
	
	nw_Send (addr,outdata,outcount,0);
}

void MultiDoPong (ubyte *data)
{
	ubyte outdata[MAX_GAME_DATA_SIZE];
	int outcount = 0;
	int count = 0;
	int size_offset;

	SKIP_HEADER (data,&count);
	ubyte slot = MultiGetByte (data,&count);
	float pingtime = MultiGetFloat (data,&count); 
	NetPlayers[slot].ping_time = timer_GetTime() - pingtime;
	//mprintf((0, "pong calculated ping: %f sec, retrieved time %f\n", NetPlayers[slot].ping_time, pingtime));

	//If we are the server and this is a client server game, send the lag info to other clients
	if((Netgame.local_role==LR_SERVER)&&(!(Netgame.flags & NF_PEER_PEER)))
	{
		size_offset=START_DATA(MP_PLAYER_LAG_INFO,outdata,&outcount);
		MultiAddByte (slot,outdata,&outcount);
		MultiAddFloat (NetPlayers[slot].ping_time,outdata,&outcount);
		END_DATA (outcount,outdata,size_offset);
		MultiSendToAllExcept (Player_num,outdata,outcount,0);
	}
}

void MultiDoLagInfo(ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);
	ubyte slot = MultiGetByte (data,&count);
	NetPlayers[slot].ping_time = MultiGetFloat (data,&count); 
	//mprintf((0, "%d's lag info net ping: %f ms\n", slot, NetPlayers[slot].ping_time));
}

// the server is telling us to play an audio taunt
void MultiDoPlayTaunt(ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);
	int pnum = MultiGetByte(data,&count);
	int index = MultiGetByte(data,&count);
	if(!Dedicated_server)
		taunt_PlayPlayerTaunt(pnum,index);
}

// tell the clients to play an audio taunt
void MultiSendPlayTaunt(int pnum,int index)
{
	if (Netgame.local_role!=LR_SERVER)
		return;

	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_SERVER_PLAY_TAUNT,data,&count);
	MultiAddByte(pnum,data,&count);
	MultiAddByte(index,data,&count);
	END_DATA (count,data,size_offset);
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYERS,false);
	MultiDoPlayTaunt(data);
}

// process a request by a client to play an audio taunt
void MultiDoRequestPlayTaunt(ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	int pnum = MultiGetByte(data,&count);
	int index = MultiGetByte(data,&count);

	MultiSendPlayTaunt(pnum,index);
}

// Client is asking the server to play an audio taunt
void MultiSendRequestPlayTaunt(int index)
{
	
	if (!(Game_mode & GM_MULTI))
		return;
	

	if(index<0 || index>3)
		return;

	if(!taunt_AreEnabled())
		return;

	//make sure an audio file exists there
	char audio_file[_MAX_PATH];

	switch(index)
	{
	case 0:
		ddio_MakePath(audio_file,LocalCustomSoundsDir,NetPlayers[Player_num].voice_taunt1,NULL);
		break;
	case 1:
		ddio_MakePath(audio_file,LocalCustomSoundsDir,NetPlayers[Player_num].voice_taunt2,NULL);
		break;
	case 2:
		ddio_MakePath(audio_file,LocalCustomSoundsDir,NetPlayers[Player_num].voice_taunt3,NULL);
		break;
	case 3:
		ddio_MakePath(audio_file,LocalCustomSoundsDir,NetPlayers[Player_num].voice_taunt4,NULL);
		break;
	}

	if(!cfexist(audio_file)){
		mprintf((0,"Ignoring request to play audio taunt...it does not exist\n"));
		return;
	}

	float t = Gametime;
	if( (t - Time_last_taunt_request) < taunt_DelayTime() ){
		//too soon since last request
		return;
	}
	Time_last_taunt_request = t;

	int count = 0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset = START_DATA(MP_CLIENT_PLAY_TAUNT,data,&count);

	MultiAddByte(Player_num,data,&count);
	MultiAddByte(index,data,&count);

	END_DATA(count,data,size_offset);

	if (Netgame.local_role==LR_SERVER)
	{
		MultiDoRequestPlayTaunt (data);
	}
	else
	{
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count);
	}
}

// the server is telling us that about a player's message-type state (is [not] typing a message)
void MultiDoTypeIcon(ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);
	int pnum = MultiGetByte(data,&count);
	bool typing = (MultiGetByte(data,&count))?true:false;

	ASSERT( pnum >= 0 && pnum<MAX_PLAYERS);

	uint bit = 0x01;
	bit = bit << pnum;

	if(typing)
	{
		mprintf((0,"%s is typing\n",Players[pnum].callsign));
		Players_typing |= bit;
	}else
	{
		mprintf((0,"%s is done typing\n",Players[pnum].callsign));
		Players_typing &= ~bit;
	}
}

// tell the clients that a player is [not] typing a message
void MultiSendTypeIcon(int pnum,bool typing_message)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_SEND_TYPE_ICON,data,&count);
	MultiAddByte(pnum,data,&count);
	MultiAddByte((typing_message)?1:0,data,&count);
	END_DATA (count,data,size_offset);
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYERS,false);
	MultiDoTypeIcon(data);
}

// process a request by a client that he is [not] typing a message
void MultiDoRequestTypeIcon(ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	int pnum = MultiGetByte(data,&count);
	bool typing = (MultiGetByte(data,&count))?true:false;

	MultiSendTypeIcon(pnum,typing);
}

// Client is telling the server that he is [not] typing a hud message
void MultiSendRequestTypeIcon(bool typing_message)
{
	uint bit = (0x01<<Player_num);

	if(typing_message && (bit&Players_typing) )
		return;	//already typing no need to request

	int count = 0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset = START_DATA(MP_REQUEST_TYPE_ICON,data,&count);

	MultiAddByte(Player_num,data,&count);
	MultiAddByte((typing_message)?1:0,data,&count);

	END_DATA(count,data,size_offset);

	if (Netgame.local_role==LR_SERVER)
	{
		MultiDoRequestTypeIcon (data);
	}
	else
	{
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
	}
}

void MultiSendAiWeaponFlags(object *obj, int flags, int wb_index) 
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_AIWEAP_FLAGS,data,&count);
	MultiAddUshort (OBJNUM(obj),data,&count);
	MultiAddInt(flags,data,&count);
	MultiAddByte(wb_index,data,&count);	
	END_DATA (count,data,size_offset);
	MultiSendToAllExcept (Player_num,data,count,NETSEQ_OBJECTS);
}

void MultiDoAiWeaponFlags(ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	int flags;
	int wb_index;
	short obj_num = Server_object_list[MultiGetUshort(data,&count)];
	flags = MultiGetInt(data,&count);
	wb_index = MultiGetByte(data,&count);
	if(obj_num==65535)
	{
		mprintf((0,"Client/Server object lists don't match! (Server num %d)\n",obj_num));
		Int3();
		return;
	}

	ASSERT(Objects[obj_num].ai_info);
	ASSERT(Objects[obj_num].type != OBJ_NONE);

	Objects[obj_num].ai_info->last_special_wb_firing = wb_index;
	Objects[obj_num].weapon_fire_flags = flags;
	if(Demo_flags == DF_RECORDING)
	{
		DemoWriteObjWeapFireFlagChanged(obj_num);
	}
}


void MultiSendAttach(object *parent, char parent_ap, object *child, char child_ap, bool f_aligned)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_ATTACH_OBJ,data,&count);
	MultiAddUshort (OBJNUM(parent),data,&count);
	MultiAddByte (parent_ap,data,&count);
	MultiAddUshort (OBJNUM(child),data,&count);
	MultiAddByte (child_ap,data,&count);
	MultiAddByte (f_aligned?1:0,data,&count);
	END_DATA (count,data,size_offset);
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_WORLD,false);

}

void MultiDoAttach(ubyte * data)
{

	object *parent;
	char parent_ap;
	object *child;
	char child_ap;
	bool f_aligned;

	int count = 0;
	SKIP_HEADER (data,&count);

	ushort parent_num = Server_object_list[MultiGetUshort(data,&count)];
	parent_ap = MultiGetByte(data,&count);
	parent = &Objects[parent_num];
	ushort child_num = Server_object_list[MultiGetUshort(data,&count)];
	child_ap = MultiGetByte(data,&count);
	child = &Objects[child_num];
	f_aligned =  MultiGetByte(data,&count)?true:false;

	if(parent_num==65535)
	{
		mprintf((0,"Client/Server object lists don't match! (Server num %d)\n",parent_num));
		Int3();
		return;
	}
	if(child_num==65535)
	{
		mprintf((0,"Client/Server object lists don't match! (Server num %d)\n",child_num));
		Int3();
		return;
	}

	ASSERT(child >= Objects && parent >= Objects);
	ASSERT(child->type != OBJ_NONE && parent->type != OBJ_NONE);

	AttachObject(parent, parent_ap, child, child_ap, f_aligned);
}

void MultiSendAttachRad(object *parent, char parent_ap, object *child, float rad)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_ATTACH_RAD_OBJ,data,&count);
	MultiAddUshort (OBJNUM(parent),data,&count);
	MultiAddByte (parent_ap,data,&count);
	MultiAddUshort (OBJNUM(child),data,&count);
	MultiAddFloat (rad,data,&count);
	END_DATA (count,data,size_offset);
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_WORLD,false);

}

void MultiDoAttachRad(ubyte * data)
{

	object *parent;
	char parent_ap;
	object *child;
	float rad;

	int count = 0;
	SKIP_HEADER (data,&count);

	ushort parent_num = Server_object_list[MultiGetUshort(data,&count)];
	parent_ap = MultiGetByte(data,&count);
	parent = &Objects[parent_num];
	ushort child_num = Server_object_list[MultiGetUshort(data,&count)];
	rad = MultiGetFloat(data,&count);
	child = &Objects[child_num];

	if(parent_num==65535)
	{
		mprintf((0,"Client/Server object lists don't match! (Server num %d)\n",parent_num));
		Int3();
		return;
	}
	if(child_num==65535)
	{
		mprintf((0,"Client/Server object lists don't match! (Server num %d)\n",child_num));
		Int3();
		return;
	}

	ASSERT(child >= Objects && parent >= Objects);
	ASSERT(child->type != OBJ_NONE && parent->type != OBJ_NONE);

	AttachObject(parent, parent_ap, child, rad);
}

void MultiSendUnattach(object *child)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;

	ASSERT (Objects[OBJNUM(child)].flags & OF_CLIENT_KNOWS);

	size_offset=START_DATA(MP_UNATTACH_OBJ,data,&count);
	MultiAddUshort (OBJNUM(child),data,&count);
	END_DATA (count,data,size_offset);
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_OBJECTS,false);
}

void MultiDoUnattach(ubyte * data)
{
	object *child;

	int count = 0;
	SKIP_HEADER (data,&count);
	int server_objnum = MultiGetUshort(data,&count);
	ASSERT(server_objnum>=0 && server_objnum<MAX_OBJECTS);
	ushort child_num = Server_object_list[server_objnum];
	child = &Objects[child_num];
	if(child_num==65535)
	{
		mprintf((0,"Client/Server object lists don't match! (Server num %d)\n",child_num));
		Int3();
		return;
	}

	UnattachFromParent(child);
}

void MultiSendThiefSteal(int player,int item)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	MULTI_ASSERT_NOMESSAGE (player>=0 && player<MAX_PLAYERS);

	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_THIEF_STEAL,data,&count);
	MultiAddInt (item,data,&count);
	MultiAddInt (player,data,&count);
	END_DATA (count,data,size_offset);

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_OBJECTS,false);
}

void MultiDoThiefSteal(ubyte * data)
{
	int count = 0;
	SKIP_HEADER (data,&count);
	int item = MultiGetInt(data,&count);
	int player = MultiGetInt(data,&count);
	ThiefStealItem(Objects[Players[player].objnum].handle,item);		
}


// Sets whether or not we want the logos to be displayed on ships
void MultiSetLogoState (bool state)
{
	Multi_logo_state=state;
	mprintf ((0,"Setting multi_logo_state to %d\n",state));
}

void MultiDoPermissionToFire (ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	int pnum=MultiGetByte (data,&count);
	Player_fire_packet[pnum].wb_index=MultiGetByte (data,&count);
	Player_fire_packet[pnum].fire_mask=MultiGetByte (data,&count);
	Player_fire_packet[pnum].damage_scalar=MultiGetByte (data,&count);

	// Get positional info
	vector pos;
	matrix orient;
	
	//memcpy (&pos,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	pos = MultiGetVector(data,&count);
	
	// Get orientation
	ushort p=MultiGetShort (data,&count);
	ushort h=MultiGetShort (data,&count);
	ushort b=MultiGetShort (data,&count);

	vm_AnglesToMatrix (&orient,p,h,b);

	// Get room and terrain flag
	ushort short_roomnum=MultiGetUshort (data,&count);
	ubyte outside=MultiGetByte (data,&count);
	int roomnum;

	if (outside)
		roomnum=MAKE_ROOMNUM(short_roomnum);
	else
		roomnum=short_roomnum;

	// Strip out quad data
	int wb_index=Player_fire_packet[pnum].wb_index;
	int quaded=wb_index & MPFF_QUADED;
	wb_index&=~MPFF_QUADED;

	// Fire!
	float scalar=(float)Player_fire_packet[pnum].damage_scalar/64.0;
	int ship_index=Players[pnum].ship_index;
	object *obj=&Objects[Players[pnum].objnum];
	
	int save_mask=obj->dynamic_wb[wb_index].cur_firing_mask;
	int save_flags=obj->dynamic_wb[wb_index].flags;
	int save_roomnum=obj->roomnum;
	matrix save_orient=obj->orient;
	vector save_pos=obj->pos;
	
	obj->dynamic_wb[wb_index].cur_firing_mask=Player_fire_packet[pnum].fire_mask;

	if (quaded)
		obj->dynamic_wb[wb_index].flags |=DWBF_QUAD;
	else
		obj->dynamic_wb[wb_index].flags &=~DWBF_QUAD;

	// Move the player into the temp position just for this firing
	ObjSetPos (obj,&pos,roomnum,&orient,false);
	
	WBFireBattery(obj, &Ships[ship_index].static_wb[wb_index], 0, wb_index,scalar);

	obj->dynamic_wb[wb_index].cur_firing_mask=save_mask;
	obj->dynamic_wb[wb_index].flags=save_flags;

	ObjSetPos (obj,&save_pos,save_roomnum,&save_orient,false);

	// Play cutoff sound if there is one
	int cutoff_sound = Ships[ship_index].firing_release_sound[wb_index];
	if (cutoff_sound != -1)
		Sound_system.Play3dSound(cutoff_sound,&Objects[Players[pnum].objnum]);
}

void MultiSendPermissionToFire (int pnum)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);

	object *obj=&Objects[Players[pnum].objnum];

	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_PERMISSION_TO_FIRE,data,&count);
	MultiAddByte (pnum,data,&count);
	MultiAddUbyte (Player_fire_packet[pnum].wb_index,data,&count);
	MultiAddUbyte (Player_fire_packet[pnum].fire_mask,data,&count);
	MultiAddUbyte (Player_fire_packet[pnum].damage_scalar,data,&count);

	// Do position
	//memcpy (&data[count],&obj->pos,sizeof(vector));
	//count+=sizeof(vector);
	MultiAddVector(obj->pos,data,&count);

	// Do orientation
	angvec angs;
	vm_ExtractAnglesFromMatrix (&angs,&obj->orient);

	MultiAddShort (angs.p,data,&count);
	MultiAddShort (angs.h,data,&count);
	MultiAddShort (angs.b,data,&count);

	// Do roomnumber and terrain flag
	MultiAddShort (CELLNUM (obj->roomnum),data,&count);
	
	// Fill flags
	if (OBJECT_OUTSIDE(obj))
		MultiAddByte (1,data,&count);
	else
		MultiAddByte (0,data,&count);

	END_DATA (count,data,size_offset);

	#ifdef RELIABLE_SECONDARIES
		if( (Player_fire_packet[pnum].wb_index>=SECONDARY_INDEX) && (Player_fire_packet[pnum].wb_index!=FLARE_INDEX) )
		{
			// send reliably
			//mprintf((0,"PCS: SENDING RELIABLE FIRE FOR %d\n",pnum));
			MultiSendReliablyToAllExcept(Player_num,data,count,NETSEQ_PLAYING,true);
		}
		else
	#endif
	{
		MultiSendToAllExcept (Player_num,data,count,NETSEQ_PLAYING);
	}
	MultiDoPermissionToFire (data);

}

// A client is asking for permission to fire
void MultiDoRequestToFire (ubyte *data)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);

	int count = 0;
	SKIP_HEADER (data,&count);

	int pnum=MultiGetByte (data,&count);
	Player_fire_packet[pnum].wb_index=MultiGetUbyte (data,&count);
	Player_fire_packet[pnum].fire_mask=MultiGetUbyte (data,&count);
	Player_fire_packet[pnum].damage_scalar=MultiGetUbyte (data,&count);

	// Check to see if this player is firing a weapon he doesn't have
	player *playp=&Players[pnum];
	if ((playp->weapon_flags & (1<<Player_fire_packet[pnum].wb_index)))
	{
		if (MultiEnoughAmmoToFire (pnum,Player_fire_packet[pnum].wb_index))
		{
			MultiSendPermissionToFire (pnum);
			// Take off ammo
			MultiSubtractAmmoToFire (pnum,Player_fire_packet[pnum].wb_index);

		}			
	}
}


// We're asking the servers permission to fire
void MultiSendRequestToFire (int wb_index,int fire_mask,float scalar)
{
	// Send quaded info if needed
	ubyte index_to_send=wb_index;

	if (Objects[Players[Player_num].objnum].dynamic_wb[wb_index].flags & DWBF_QUAD)
		index_to_send|=MPFF_QUADED;

	ubyte damage_scalar=(scalar*64.0);

	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_REQUEST_TO_FIRE,data,&count);
	
	MultiAddByte (Player_num,data,&count);
	MultiAddUbyte (index_to_send,data,&count);
	MultiAddUbyte (fire_mask,data,&count);
	MultiAddUbyte (damage_scalar,data,&count);

	END_DATA (count,data,size_offset);
	
	if (Netgame.local_role==LR_SERVER)
	{
		MultiDoRequestToFire (data);
	}
	else
	{
		#ifdef RELIABLE_SECONDARIES
			// should we send reliably? (secondaries)
			if( (wb_index>=SECONDARY_INDEX) && (wb_index!=FLARE_INDEX) )
			{
				// send reliably
				//mprintf((0,"PCS: Sending fire request reliably\n"));
				nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,true);
			}
			else
		#endif
		{
			nw_Send(&Netgame.server_address,data,count,0);
		}
	}
}

// Server is processing a request for a marker
void MultiDoRequestMarker (ubyte *data)
{	
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);

	int count = 0;
	char message[100];
	SKIP_HEADER (data,&count);

	
	int pnum=MultiGetByte (data,&count);
	ubyte len=MultiGetByte (data,&count);
	memcpy (message,&data[count],len);
	count+=len;

	int limit=2;
	int cur_marker_num;
	
	object *pobj=&Objects[Players[pnum].objnum];

	if (Players[pnum].num_markers>=limit)
	{
		// Delete the oldest marker
		int found=-1;
		float low_time=999999999.0f;
		for (int i=0;i<=Highest_object_index;i++)
		{
			if (Objects[i].type==OBJ_MARKER && Objects[i].parent_handle==pobj->handle && Objects[i].creation_time<low_time)
			{
				found=i;
				low_time=Objects[i].creation_time;
			}
		}

		ASSERT (found!=-1);

		SetObjectDeadFlag(&Objects[found],true,false);

		cur_marker_num=Objects[found].id;
	}
	else
	{
		cur_marker_num=pnum*2+Players[pnum].num_markers;
		Players[pnum].num_markers++;
	}

	int objnum=ObjCreate (OBJ_MARKER,cur_marker_num,pobj->roomnum,&pobj->pos,&pobj->orient,pobj->handle);

	if (objnum>=0)
	{
		strcpy (MarkerMessages[cur_marker_num],message);
		MultiSendObject (&Objects[objnum],0);
	}
}

// Client is asking for a marker
void MultiSendRequestForMarker (char *message)
{
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_REQUEST_MARKER,data,&count);
	
	MultiAddByte (Player_num,data,&count);

	ubyte len = strlen(message)+1;
	MultiAddByte (len,data,&count);
	memcpy (data+count,message,len);
	count+=len;
	
	END_DATA (count,data,size_offset);
	
	if (Netgame.local_role==LR_SERVER)
		MultiDoRequestMarker (data);
	else
		nw_SendReliable (NetPlayers[Player_num].reliable_socket,data,count,false);
}

// The server is telling me to adjust my position
void MultiDoAdjustPosition (ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	float timestamp=MultiGetFloat (data,&count);

	vector newpos,newvel;

	//memcpy (&newpos,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	newpos = MultiGetVector(data,&count);
	//memcpy (&newvel,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	newvel = MultiGetVector(data,&count);

	int roomnum=MultiGetInt (data,&count);

	object *obj=&Objects[Players[Player_num].objnum];

	ObjSetPos (obj,&newpos,roomnum,&obj->orient,true);
	obj->mtype.phys_info.velocity=newvel;

	float save_frametime=Frametime;
	int start_move=-1,end_move=-1;

	// Now go through and adjust our position based on our saved moves
	for (int i=0;i<MAX_SAVED_MOVES;i++)
	{
		int next=i+1;
		int prev=i-1;

		if (next==MAX_SAVED_MOVES)
			next=0;
		if (prev==-1)
			prev=MAX_SAVED_MOVES-1;

		if (start_move==-1 && SavedMoves[prev].timestamp<timestamp && SavedMoves[i].timestamp>=timestamp)
			start_move=i;
		if (end_move==-1 && SavedMoves[i].timestamp>=timestamp && SavedMoves[next].timestamp<timestamp)
			end_move=i;
	}

	ASSERT (start_move!=-1);	// HUH?  We don't have a copy of this move in our list
	ASSERT (end_move!=-1);
	
	int cur_move=start_move;	
	int done=0;

	vector save_thrust,save_rotthrust;
	save_thrust=obj->mtype.phys_info.thrust;
	save_rotthrust=obj->mtype.phys_info.rotthrust;

	while (!done)
	{
		int next=cur_move+1;
		int next_end=end_move+1;

		if (next==MAX_SAVED_MOVES)	
			next=0;

		if (next_end==MAX_SAVED_MOVES)	
			next_end=0;

		float delta;

		if (next==next_end)
			delta=0.1f;
		else
			delta=SavedMoves[next].timestamp-SavedMoves[cur_move].timestamp;

		ASSERT (delta>=0);

		Frametime=delta;
		obj->mtype.phys_info.thrust=SavedMoves[cur_move].thrust;
		obj->mtype.phys_info.rotthrust=SavedMoves[cur_move].rotthrust;


		do_physics_sim(obj);
					
		if (cur_move==end_move)
			done=1;	
		cur_move=next;
	}

	obj->mtype.phys_info.rotthrust=save_rotthrust;
	obj->mtype.phys_info.thrust=save_thrust;
	Frametime=save_frametime;


}

// Server is telling the client to update his position
void MultiSendAdjustPosition (int slot,float timestamp)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);

	int size_offset;
	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
		
	object *obj=&Objects[Players[slot].objnum];

	size_offset=START_DATA (MP_ADJUST_POSITION,data,&count);
	MultiAddFloat (timestamp,data,&count);

	MultiAddFloat (obj->pos.x,data,&count);
	MultiAddFloat (obj->pos.y,data,&count);
	MultiAddFloat (obj->pos.z,data,&count);

	MultiAddFloat (obj->mtype.phys_info.velocity.x,data,&count);
	MultiAddFloat (obj->mtype.phys_info.velocity.y,data,&count);
	MultiAddFloat (obj->mtype.phys_info.velocity.z,data,&count);

	MultiAddInt (obj->roomnum,data,&count);

	END_DATA (count,data,size_offset);
	
	nw_Send (&NetPlayers[slot].addr,data,count,0);
}


float Last_update_time[MAX_PLAYERS];
// Client is asking permission to move
void MultiDoRequestToMove (ubyte *data)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);

	int count = 0;
	SKIP_HEADER (data,&count);

	vector thrust,rotthrust,pos;
	matrix orient;

	int slot=MultiGetByte (data,&count);
	object *obj=&Objects[Players[slot].objnum];

	float timestamp=MultiGetFloat (data,&count);
	
	if (timestamp<NetPlayers[slot].packet_time)
		return;		// Got out of order packet

	if (Players[slot].flags & (PLAYER_FLAGS_DEAD|PLAYER_FLAGS_DYING))
		return;

	// Get pos
	//memcpy (&pos,&data[count],sizeof(vector));
	//count+=sizeof(vector);	
	pos = MultiGetVector(data,&count);
	
	// Get thrust
	//memcpy (&thrust,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	thrust = MultiGetVector(data,&count);

	// Get rotational thrust
	//memcpy (&rotthrust,&data[count],sizeof(vector));
	//count+=sizeof(vector);
	rotthrust = MultiGetVector(data,&count);

	// Get orientation
	ushort p=MultiGetShort (data,&count);
	ushort h=MultiGetShort (data,&count);
	ushort b=MultiGetShort (data,&count);

	vm_AnglesToMatrix (&orient,p,h,b);

	float delta_time=Gametime-NetPlayers[slot].packet_time;
	NetPlayers[slot].packet_time=Gametime;
	
	obj->mtype.phys_info.thrust=thrust;
	obj->mtype.phys_info.rotthrust=rotthrust;

	// Perform autonomous movement
	float save_frametime=Frametime;
	
	obj->movement_type=MT_PHYSICS;
	obj->mtype.phys_info.flags |=PF_USES_THRUST;
	obj->mtype.phys_info.flags &=~PF_FIXED_VELOCITY;

	obj->mtype.phys_info.thrust=thrust;
	obj->mtype.phys_info.rotthrust=rotthrust;
	Frametime=delta_time;
	
	do_physics_sim(obj);

	obj->mtype.phys_info.thrust=Zero_vector;
	obj->mtype.phys_info.rotthrust=Zero_vector;
	obj->orient=orient;


	bool client_error=0;
	if (vm_VectorDistance (&pos,&obj->pos)>5)
		client_error=true;
	
	if (Gametime-Last_update_time[slot]>.15)		
		client_error=true;
	
	// If there is too much client error then adjust
	if (client_error)
	{
		mprintf ((0,"Correcting, deltatime=%f dist=%f\n",delta_time,vm_VectorDistance (&pos,&obj->pos)));
		MultiSendAdjustPosition (slot,timestamp);
		Last_update_time[slot]=Gametime;
	}

	Frametime=save_frametime;

}

// Sets up a packet so that we can request to move from server
int MultiStuffRequestToMove (ubyte *data)
{
	int size_offset;
	int count=0;
		
	object *obj=&Objects[Players[Player_num].objnum];

	size_offset=START_DATA (MP_REQUEST_TO_MOVE,data,&count);
	MultiAddByte (Player_num,data,&count);

	// Add timestamp
	MultiAddFloat (Gametime,data,&count);
	
	MultiAddFloat (obj->pos.x,data,&count);
	MultiAddFloat (obj->pos.y,data,&count);
	MultiAddFloat (obj->pos.z,data,&count);
	
	// Do acceleration
	MultiAddFloat (obj->mtype.phys_info.thrust.x,data,&count);
	MultiAddFloat (obj->mtype.phys_info.thrust.y,data,&count);
	MultiAddFloat (obj->mtype.phys_info.thrust.z,data,&count);

	// Do rotational acceleration
	MultiAddFloat (obj->mtype.phys_info.rotthrust.x,data,&count);
	MultiAddFloat (obj->mtype.phys_info.rotthrust.y,data,&count);
	MultiAddFloat (obj->mtype.phys_info.rotthrust.z,data,&count);
		
	// Do orientation
	angvec angs;
	vm_ExtractAnglesFromMatrix (&angs,&obj->orient);

	MultiAddShort (angs.p,data,&count);
	MultiAddShort (angs.h,data,&count);
	MultiAddShort (angs.b,data,&count);

	END_DATA (count,data,size_offset);

	return count;
}

// Server is giving us a list of objects that aren't visible
void MultiDoGenericNonVis (ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	int num=MultiGetShort (data,&count);
	for (int i=0;i<num;i++)
	{
		ushort objnum=MultiGetUshort (data,&count);

		objnum=Server_object_list[objnum];
		if (objnum==65535 || !(Objects[objnum].flags & OF_SERVER_OBJECT))
		{
			Int3(); // Get Jason, invalid object received!
			continue;
		}

		ASSERT (IS_GENERIC (Objects[objnum].type));

		Objects[objnum].render_type=RT_NONE;
		Objects[objnum].mtype.phys_info.flags|=PF_NO_COLLIDE;
	}
	
}


// Tell clients to break some glass
void MultiSendBreakGlass (room *rp,int facenum)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);

	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_BREAK_GLASS,data,&count);
	
	MultiAddUshort (rp-Rooms,data,&count);
	MultiAddUshort (facenum,data,&count);

	END_DATA (count,data,size_offset);

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYING,false);
}

// Server is telling us to break some glass
void MultiDoBreakGlass (ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	int roomnum=MultiGetUshort (data,&count);
	int facenum=MultiGetUshort (data,&count);

	BreakGlassFace (&Rooms[roomnum],facenum);
}

// Server says a player has changed rank!
void MultiDoChangeRank (ubyte *data)
{
	int count = 0;
	char str[255];
	SKIP_HEADER (data,&count);

	int pnum=MultiGetByte (data,&count);
	MultiGetString (str,data,&count);
	int goodnews=MultiGetByte (data,&count);
	float newrank=MultiGetFloat (data,&count);

	AddBlinkingHUDMessage (str);

	// Play a sound
	if (goodnews)
	{
		int soundnum=FindSoundName ("CTFScore1");
		if (soundnum>0)
			Sound_system.Play2dSound (soundnum);
	}
	else
	{
		int soundnum=FindSoundName ("CTFLostFlag1");
		if (soundnum>0)
			Sound_system.Play2dSound (soundnum);
	}

	Players[pnum].rank=newrank;
}

// Server is telling me to strip bare!
void MultiDoStripPlayer(int slot,ubyte *data)
{
	if(slot!=0)
	{
		//server didnt send this to us?!
		Int3();
		return;
	}

	int count = 0;
	SKIP_HEADER (data,&count);
	slot = MultiGetByte(data,&count);
	if(Game_mode&GM_MULTI && Netgame.local_role!=LR_SERVER && (slot!=Player_num) )
	{
		Int3();
		return;
	}

	mprintf((0,"I'm stripping %d bare! (of weapons)\n",slot));

	object *pobj = &Objects[Players[slot].objnum];

	// take quad fire
	static int quad_id = -2;
	if(pobj->dynamic_wb[LASER_INDEX].flags&DWBF_QUAD ||
		pobj->dynamic_wb[SUPER_LASER_INDEX].flags&DWBF_QUAD)
	{
		pobj->dynamic_wb[LASER_INDEX].flags &= ~DWBF_QUAD;
		pobj->dynamic_wb[SUPER_LASER_INDEX].flags &= ~DWBF_QUAD;
		
		if(quad_id==-2)
			quad_id = FindObjectIDName("QuadLaser");

		if(quad_id>-1)
		{
			Players[slot].inventory.Remove(OBJ_POWERUP,quad_id);
		}
	}

	int i;

	// remove weapons
	for (i=0; i<MAX_PLAYER_WEAPONS; i++) 
	{
		Players[slot].weapon_ammo[i] = 0;
		Players[slot].weapon_flags &= ~HAS_FLAG(i);
	}

	// reset lasers back on
	Players[slot].weapon_flags = HAS_FLAG(LASER_INDEX) + HAS_FLAG(FLARE_INDEX) + HAS_FLAG(CONCUSSION_INDEX);
	Players[slot].weapon[PW_PRIMARY].index = LASER_INDEX;
	Players[slot].weapon[PW_PRIMARY].firing_time = 0;
	Players[slot].weapon[PW_PRIMARY].sound_handle = -1;
	Players[slot].weapon[PW_SECONDARY].index = CONCUSSION_INDEX;
	Players[slot].weapon[PW_SECONDARY].firing_time = 0;
	Players[slot].weapon[PW_SECONDARY].sound_handle = -1;

	// set energy to 0
	Players[slot].energy = 0;	

	// remove counter measures
	Players[slot].counter_measures.Reset(true,INVRESET_DEATHSPEW);
}

// Strips a player bare of weapons
void MultiSendStripPlayer(int slot)
{
	if(Game_mode&GM_MULTI)
	{
		MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);
	}else
	{
		if(slot==-1)
		{
			// it's not a multiplayer game, bash to just us
			slot = Player_num;
		}
	}

	int count=0,save_count;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_STRIP_PLAYER,data,&count);
	save_count = count;
	MultiAddByte(slot,data,&count);
	END_DATA (count,data,size_offset);

	if(slot!=-1)
	{
		MultiDoStripPlayer(Player_num,data);
		if(slot!=Player_num)
			nw_SendReliable(NetPlayers[slot].reliable_socket,data,count,false);
	}else
	{
		// strip all the players
		int i;
		for(i=0;i<MAX_PLAYERS;i++)
		{
			if((NetPlayers[i].flags&NPF_CONNECTED)&&(NetPlayers[i].sequence!=NETSEQ_PLAYING))
			{
				MultiAddByte(i,data,&save_count);//change the value in the packet
				MultiDoStripPlayer(Player_num,data);
				if(i!=Player_num)
					nw_SendReliable(NetPlayers[i].reliable_socket,data,count,false);
			}
		}
	}
}


// Server is telling me about a player rank
void MultiDoInitialRank (ubyte *data)
{
	int count = 0;
	SKIP_HEADER (data,&count);

	int pnum=MultiGetByte (data,&count);
	float rank=MultiGetFloat (data,&count);

	mprintf ((0,"Got initial rank for player %d (%f)\n",pnum,rank));

	Players[pnum].rank=rank;
}

// Tell all the clients about this players rank
void MultiSendInitialRank (int pnum)
{
	MULTI_ASSERT_NOMESSAGE (Netgame.local_role==LR_SERVER);

	int count=0;
	ubyte data[MAX_GAME_DATA_SIZE];
	int size_offset;
	size_offset=START_DATA(MP_INITIAL_RANK,data,&count);
	MultiAddByte (pnum,data,&count);
	MultiAddFloat (Players[pnum].rank,data,&count);

	mprintf ((0,"Sending initial rank for player %d\n",pnum));

	END_DATA (count,data,size_offset);

	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_PLAYERS,false);

}

// Server only function to clear a Guidebot for a disconnected player
void MultiClearGuidebot(int slot)
{
	if(!(Game_mode&GM_MULTI))
		return;
	if(Netgame.local_role!=LR_SERVER)
		return;
	if(!(Netgame.flags&NF_ALLOWGUIDEBOT))
		return;

	//Kill off the old buddy bot
	object *buddy = ObjGet(Buddy_handle[slot]);
	if(buddy)
	{
		SetObjectDeadFlag(buddy,true,false);
	}
	Buddy_handle[slot] = OBJECT_HANDLE_NONE;

	// Recreate a buddy bot
	int parent_handle;
	if(Players[slot].objnum < 0 || Players[slot].objnum > Highest_object_index || Objects[Players[slot].objnum].type == OBJ_NONE)
	{
		parent_handle = OBJECT_HANDLE_NONE;
	}else
	{
		parent_handle = Objects[Players[slot].objnum].handle;
	}

	//BLACKPYRO
	int objnum = ObjCreate(OBJ_ROBOT, ROBOT_GUIDEBOT, Player_object->roomnum, &Player_object->pos, NULL, parent_handle);

	ASSERT(objnum!=-1);
	if(objnum==-1)
		return;

	// tell the client about the new buddy
	MultiSendObject(&Objects[objnum],0);

	InitObjectScripts(&Objects[objnum],true);
	
	Buddy_handle[slot] = Objects[objnum].handle;
	ObjGhostObject(objnum);

	MultiSendGhostObject(&Objects[objnum],true);

	// now update the buddy handle list of the clients
	int count=0;
	int size_offset;
	ubyte data[MAX_GAME_DATA_SIZE];

	mprintf ((0,"Sending Buddy_handle update to clients for Buddy#%d\n",slot));

	size_offset=START_DATA(MP_WORLD_STATES,data,&count);
	
	MultiAddByte (WS_BUDDYBOTUPDATE,data,&count);
	MultiAddByte (slot,data,&count);
	MultiAddUshort (Buddy_handle[slot]&HANDLE_OBJNUM_MASK,data,&count);
	
	MultiAddByte (WS_END,data,&count);
	END_DATA (count,data,size_offset);

	// Send it out
	MultiSendReliablyToAllExcept (Player_num,data,count,NETSEQ_OBJECTS,false);
}

void RequestPlayerList(network_address *addr)
{
	ubyte outdata[10];
	int count=0;
	int size;

	size=START_DATA(MP_REQUEST_PLAYERLIST,outdata,&count);
	END_DATA (count,outdata,size);
	nw_Send(addr,outdata,count,0);
}

// a client is requesting a list of players... so give it to em!
void DoReqPlayerList(network_address *addr)
{
	ubyte outdata[((CALLSIGN_LEN+1)*MAX_NET_PLAYERS)+1];
	int count=0;
	int size;

	memset(outdata,0,sizeof(outdata));
	size=START_DATA(MP_PLAYERLIST_DATA,outdata,&count);
		
	unsigned short icurrplayers = 0;
	int i=0;
	if(Dedicated_server)
	{
		//Skip the server player
		i=1;
	}

	for(;i<MAX_NET_PLAYERS;i++)
	{
		if( (NetPlayers[i].flags & NPF_CONNECTED) && (Players[i].callsign[0]) )
		{
			memcpy(outdata+count,Players[i].callsign,strlen(Players[i].callsign)+1);
			count += strlen(Players[i].callsign)+1;
		}
	}
	//Add one last 0 to signal the end of the player list
	count++;
	outdata[count] = 0;

	END_DATA (count,outdata,size);

	nw_Send(addr,outdata,count,0);

}

char *Multi_recieved_player_list = NULL;
bool Multi_got_player_list = false;

void DoPlayerListData(ubyte *data,int len)
{
	int count = 0;
	if(Multi_recieved_player_list==NULL)
	{
		mprintf((0,"Received a player list packet when we weren't expecting one. Ignoring.\n"));
		return;
	}
	SKIP_HEADER (data,&count);
	memcpy(Multi_recieved_player_list,data+count,len);
	Multi_got_player_list = true;


}

//Tell a player what ship they are supposed to switch to 
//if the one they chose isn't allowed.
void MultiBashPlayerShip(int slot,char *ship)
{
	ubyte outdata[100];
	int count=0;
	int size;

	size=START_DATA(MP_BASHPLAYER_SHIP,outdata,&count);
	strcpy((char *)outdata+count,ship);	
	count+=strlen(ship)+1;
	END_DATA (count,outdata,size);
	nw_SendReliable (NetPlayers[slot].reliable_socket,outdata,count,true);

}

int ObjInitTypeSpecific(object *objp,bool reinitializing);

void MultiDoBashPlayerShip(ubyte *data)
{
	int count = 0;
	
	SKIP_HEADER (data,&count);
	int ship_index=FindShipName ((char *)data+count);
	if (ship_index<0)
		ship_index=0;

	//If told to switch to the Black Pyro, make sure it's allowed
	if(!stricmp(Ships[ship_index].name,"Black Pyro"))
	{
		extern bool MercInstalled();
		if (! MercInstalled())
		{
			BailOnMultiplayer("Exiting: Game requires Black Pyro");
		}
	}

	AddHUDMessage("%s not allowed",Ships[Players[Player_num].ship_index].name);
	AddHUDMessage("Switching to %s",Ships[ship_index].name);

	PlayerChangeShip (Player_num,ship_index);
	mprintf((0,"Server told us to switch ships to the %s\n",(char *)data+count));

	FreeCockpit();
	CloseShipHUD();
	
	//ObjInitTypeSpecific(Player_object,0);		//reset physics, model, etc.
	InitShipHUD(Players[Player_num].ship_index);
	InitCockpit(Players[Player_num].ship_index);

	if (GetHUDMode() == HUD_COCKPIT) 
		SetHUDMode(HUD_COCKPIT);
	else if (GetHUDMode() == HUD_FULLSCREEN) 
		SetHUDMode(HUD_FULLSCREEN);
}


// If its been x seconds since we've sent a heartbeat, send another!

#define HEARTBEAT_INTERVAL	10.0f
void MultiSendHeartbeat ()
{
	static float last_heartbeat=timer_GetTime();

	float time_scalar=1;

	if (Netgame.local_role==LR_SERVER)
		time_scalar=.5; // Server sends out twice as fast

	if ((timer_GetTime()-last_heartbeat)>(HEARTBEAT_INTERVAL * time_scalar))
	{
		ubyte outdata[100];
		int count=0;
		int size;

		size=START_DATA(MP_HEARTBEAT,outdata,&count);
		END_DATA (count,outdata,size);

		if (Netgame.local_role==LR_SERVER)
		{

			mprintf ((0,"Server sending heartbeat.\n"));
			MultiSendToAllExcept (Player_num,outdata,count,-1);
		}
		else
		{
			mprintf ((0,"Client sending heartbeat.\n"));
			nw_Send(&Netgame.server_address,outdata,count,0);
		}

		

		// Reset interval
		last_heartbeat=timer_GetTime();
	}
}

//-----------------------------------------------
void MultiDoMSafeFunction(ubyte *data);
void MultiDoMSafePowerup (ubyte *data);

// This allows us to specify under what sequences certain packets are accepted
#define ACCEPT_CONDITION(s,e) {if (sequence<s || sequence>e) return;}
//#define ACCEPT_CONDITION(s,e)


// Takes the individual packet types and passes their data to the appropriate routines
void MultiProcessData (ubyte *data,int len,int slot,network_address *from_addr)
{
	ubyte type=data[0];
	len=len;
	int sequence=-1;

	// HEY!!!!!  These packets are the only ones that are accepted by non-connected machines
	// (ie people on the netgame join screen)
	// This will go away once I fill out all the ACCEPT_CONDITION types
	if (!(NetPlayers[Player_num].flags & NPF_CONNECTED))
	{
		if (type!=MP_CONNECTION_ACCEPTED && type!=MP_JOIN_RESPONSE && type!=MP_GAME_INFO && type!=MP_PLAYERLIST_DATA && type!=MP_ASK_FOR_URL && type!=MP_CUR_MSN_URLS)
			return;
	}
	else
	{
		// Setup our sequences for accept conditions
		if (Netgame.local_role==LR_SERVER)
		{
			if (NetPlayers[Player_num].sequence!=NETSEQ_PLAYING)
				return;
			if (slot==-1)
				sequence=-1;
			else
				sequence=NetPlayers[slot].sequence;
		}
		else
		{
			sequence=NetPlayers[Player_num].sequence;
		}
	}

	#ifndef RELEASE
		Multi_packet_tracking[type]++;
	#endif

	switch (type)
	{
		case MP_CONNECTION_ACCEPTED:
			ACCEPT_CONDITION (-1,-1);
			MultiDoConnectionAccepted(data);
			break;
		case MP_REQUEST_PLAYERS:
			MultiDoRequestPlayers (data);
			break;
		case MP_REQUEST_BUILDINGS:
			MultiDoRequestBuildings (data);
			break;
		case MP_REQUEST_OBJECTS:
			MultiDoRequestObjects (data);
			break;
		case MP_REQUEST_WORLD_STATES:
			MultiDoRequestWorldStates (data);
			break;
		case MP_PLAYER_POS:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			NetPlayers[Player_num].total_bytes_rcvd += len;
			MultiDoPlayerPos(data);
			break;
		case MP_DONE_PLAYERS:
			MultiDoDonePlayers (data);
			break;
		case MP_DONE_BUILDINGS:
			MultiDoDoneBuildings (data);
			break;
		case MP_DONE_OBJECTS:
			MultiDoDoneObjects (data);
			break;
		case MP_DONE_WORLD_STATES:
			MultiDoDoneWorldStates (data);
			break;
		case MP_ENTERING_GAME:
			MultiDoEnteringGame(data);
			break;
		case MP_PLAYER:
			MultiDoPlayer (data);
			break;
		case MP_BUILDING:
			MultiDoBuilding (data);
			break;
		case MP_WORLD_STATES:
			MultiDoWorldStates (data);
			break;
		case MP_JOIN_OBJECTS:
			MultiDoJoinObjects (data);
			break;
		case MP_SEND_DEMO_OBJECT_FLAGS:
			MultiDoJoinDemoObjects (data);
			break;
		case MP_MY_INFO:
			ACCEPT_CONDITION (NETSEQ_LEVEL_START,NETSEQ_LEVEL_START);
			MultiDoMyInfo (data);
			break;
		case MP_DISCONNECT:
			ACCEPT_CONDITION (-1,255);
			MultiDoDisconnect (data);
			break;
		case MP_PLAYER_FIRE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoFirePlayerWB (data);
			break;
		case MP_ASK_TO_JOIN:
			ACCEPT_CONDITION (-1,-1);
			MultiDoAskToJoin (data,from_addr);
			break;
		case MP_JOIN_RESPONSE:
			ACCEPT_CONDITION (-1,-1);
			MultiDoJoinResponse (data);
			break;
		case MP_SERVER_QUIT:
			ACCEPT_CONDITION (NETSEQ_WAITING_FOR_LEVEL,255);
			MultiDoServerQuit(data);
			break;
		case MP_LEAVE_GAME:
			MultiDoLeaveGame (data);
			break;
		case MP_BLOWUP_BUILDING:
			ACCEPT_CONDITION (NETSEQ_REQUEST_OBJECTS,255);
			MultiDoBlowupBuilding (data);
			break;
		case MP_PLAYER_DEAD:
			ACCEPT_CONDITION (NETSEQ_PLAYERS,NETSEQ_PLAYING);
			MultiDoPlayerDead (data);
			break;
		case MP_PLAYER_ENTERED_GAME:
			ACCEPT_CONDITION (NETSEQ_REQUEST_BUILDINGS,255);
			MultiDoPlayerEnteredGame (data);
			break;
		case MP_DAMAGE_PLAYER:
			ACCEPT_CONDITION (NETSEQ_PLAYERS,NETSEQ_PLAYING);
			MultiDoDamagePlayer (data);
			break;
		case MP_MESSAGE_FROM_SERVER:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoMessageFromServer (data);
			break;
		case MP_MESSAGE_TO_SERVER:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoMessageToServer (data);
			break;
		case MP_END_PLAYER_DEATH:
			MultiDoEndPlayerDeath (data);
			break;
		case MP_RENEW_PLAYER:
			ACCEPT_CONDITION (NETSEQ_PLAYERS,NETSEQ_PLAYING);
			MultiDoRenewPlayer (data);
			break;
		case MP_GET_GAME_INFO:
			ACCEPT_CONDITION (-1,-1);
			MultiDoGetGameInfo (data,from_addr);
			break;
		case MP_GET_PXO_GAME_INFO:
			ACCEPT_CONDITION (-1,-1);
			MultiDoGetPXOGameInfo (data,from_addr);
			break;
		case MP_GAME_INFO:
			ACCEPT_CONDITION (-1,-1);
			MultiDoGameInfo (data,from_addr);
			break;
		case MP_OBJECT:
			ACCEPT_CONDITION (NETSEQ_OBJECTS,NETSEQ_PLAYING);
			MultiDoObject (data);
			break;
		case MP_SPECIAL_PACKET:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoSpecialPacket (data);
			break;
		case MP_EXECUTE_DLL:
			MultiDoExecuteDLL (data);
			break;
		case MP_REMOVE_OBJECT:
			MultiDoRemoveObject (data);
			break;
		case MP_GUIDED_INFO:
			MultiDoGuidedInfo(data);
			break;
		case MP_LEVEL_INFO:
			ACCEPT_CONDITION (NETSEQ_WAITING_FOR_LEVEL,NETSEQ_WAITING_FOR_LEVEL);
			MultiDoLevelInfo(data);
			break;
		case MP_READY_FOR_LEVEL:
			ACCEPT_CONDITION (NETSEQ_WAITING_FOR_LEVEL,NETSEQ_WAITING_FOR_LEVEL);
			MultiDoReadyForLevel(data);
			break;
		case MP_LEVEL_ENDED:
			MultiDoLevelEnded (data);
			break;
		case MP_GET_GAMETIME:
			ACCEPT_CONDITION (NETSEQ_NEED_GAMETIME,NETSEQ_NEED_GAMETIME);
			MultiDoGameTimeReq(data,from_addr);
			break;
		case MP_HERE_IS_GAMETIME:
			ACCEPT_CONDITION (NETSEQ_NEED_GAMETIME,NETSEQ_WAIT_GAMETIME);
			MultiDoSetGameTime(data);
			break;
		case MP_POWERUP_REPOSITION:
			ACCEPT_CONDITION (NETSEQ_OBJECTS,NETSEQ_PLAYING);
			MultiDoPowerupReposition (data);
			break;
		case MP_ROBOT_POS:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRobotPos(data);
			break;
		case MP_ROBOT_FIRE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRobotFire(data);
			break;
		case MP_ON_OFF:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoOnOff (data);
			break;
		case MP_ROBOT_DAMAGE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRobotDamage(data);
			break;
		case MP_ROBOT_EXPLODE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRobotExplode(data);
			break;
		case MP_ADDITIONAL_DAMAGE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoAdditionalDamage (data);
			break;
		case MP_ANIM_UPDATE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoObjAnimUpdate(data);
			break;
		case MP_PLAY_3D_SOUND_FROM_OBJ:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoPlay3dSound(data);
			break;
		case MP_ROBOT_FIRE_SOUND:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRobotFireSound(data);
			break;
		case MP_TURRET_UPDATE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoTurretUpdate(data);
			break;
		case MP_CLIENT_USE_INVENTORY_ITEM:
			MultiDoClientInventoryUseItem(slot,data);
			break;
		case MP_REMOVE_INVENTORY_ITEM:
			MultiDoClientInventoryRemoveItem(slot,data);
			break;
		case MP_REQUEST_COUNTERMEASURE:
			MultiDoRequestCountermeasure (data);
			break;
		case MP_SERVER_SENT_COUNT:
			MultiDoBytesSent(data);
			break;
		case MP_CLIENT_SET_PPS:
			MultiDoPPSSet(data,slot);
			break;
		case MP_GREETINGS_FROM_CLIENT:
			MultiDoGreetings(data,from_addr);
			break;
		case MP_REQUEST_TO_OBSERVE:
			MultiDoRequestToObserve (data);
			break;
		case MP_OBSERVER_CHANGE:
			MultiDoObserverChange (data);
			break;
		case MP_VISIBLE_PLAYERS:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoVisiblePlayers (data);
			break;
		case MP_FILE_REQ:
			MultiDoFileReq(data);
			break;
		case MP_FILE_DENIED:
			MultiDoFileDenied(data);
			break;
		case MP_FILE_DATA:
			MultiDoFileData(data);
			break;
		case MP_FILE_ACK:
			MultiDoFileAck(data);
			break;
		case MP_FILE_CANCEL:
			MultiDoFileCancelled(data);
			break;
		case MP_PLAYER_CUSTOM_DATA:
			MultiDoCustomPlayerData(data);
			break;
		case MP_GHOST_OBJECT:
			MultiDoGhostObject(data);
			break;
		case MP_PLAYER_PING:
			MultiDoPing(data,from_addr);
			break;
		case MP_PLAYER_PONG:
			MultiDoPong(data);
			break;
		case MP_PLAYER_LAG_INFO:
			MultiDoLagInfo(data);
			break;
		case MP_REQUEST_SHIELDS:
			MultiDoRequestShields (data);
			break;
		case MP_REQUEST_DAMAGE:
			MultiDoRequestDamage (data);
			break;
		case MP_ATTACH_OBJ:
			MultiDoAttach(data);
			break;
		case MP_UNATTACH_OBJ:
			MultiDoUnattach(data);
			break;
		case MP_AIWEAP_FLAGS:
			MultiDoAiWeaponFlags(data);
			break;
		case MP_ATTACH_RAD_OBJ:
			MultiDoAttachRad(data);
			break;
		case MP_TIMEOUT_WEAPON:
			MultiDoReleaseTimeoutMissile (data);
			break;
		case MP_WEAPONS_LOAD:
			MultiDoWeaponsLoad (data);
			break;
		case MP_REQUEST_PEER_DAMAGE:
			MultiDoRequestPeerDamage (data,from_addr);
			break;
		case MP_MSAFE_FUNCTION:
			MultiDoMSafeFunction (data);
			break;
		case MP_MSAFE_POWERUP:
			MultiDoMSafePowerup (data);
			break;
		case MP_ASK_FOR_URL:
			msn_DoAskForURL(data,from_addr);
			break;
		case MP_CUR_MSN_URLS:
			msn_DoCurrMsnURLs(data,from_addr);
			break;
		case MP_REQUEST_TO_FIRE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRequestToFire (data);
			break;
		case MP_PERMISSION_TO_FIRE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoPermissionToFire(data);
			break;
		case MP_CONNECT_BAIL:
			MultiDoConnectBail();
			break;
		case MP_CLIENT_PLAY_TAUNT:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRequestPlayTaunt(data);
			break;
		case MP_SERVER_PLAY_TAUNT:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoPlayTaunt(data);
			break;
		case MP_REQUEST_MARKER:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRequestMarker(data);
			break;
		case MP_REQUEST_TYPE_ICON:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRequestTypeIcon(data);
			break;
		case MP_SEND_TYPE_ICON:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoTypeIcon(data);
			break;
		case MP_REQUEST_TO_MOVE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoRequestToMove (data);
			break;
		case MP_ADJUST_POSITION:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoAdjustPosition (data);
			break;
		case MP_GENERIC_NONVIS:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoGenericNonVis (data);
			break;
		case MP_GUIDEBOTMENU_REQUEST:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoGuidebotMenuRequest(data,slot);
			break;
		case MP_GUIDEBOTMENU_DATA:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoGuidebotMenuData(data);
			break;
		case MP_BREAK_GLASS:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoBreakGlass (data);
			break;
		case MP_THIEF_STEAL:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoThiefSteal(data);
			break;
		case MP_REQUEST_PLAYERLIST:
			ACCEPT_CONDITION (-1,-1);
			DoReqPlayerList(from_addr);
			break;
		case MP_PLAYERLIST_DATA:
			ACCEPT_CONDITION (-1,-1);
			DoPlayerListData(data,len);
			break;
		case MP_SERVER_AUDIOTAUNT_TIME:
			MultiDoAudioTauntTime(data);
			break;
		case MP_CHANGE_RANK:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoChangeRank (data);
			break;
		case MP_BASHPLAYER_SHIP:
			ACCEPT_CONDITION (NETSEQ_LEVEL_START,NETSEQ_PLAYING);
			MultiDoBashPlayerShip(data);			
			break;
		case MP_HEARTBEAT:
			if (Netgame.local_role==LR_CLIENT)
			{
				Got_heartbeat=true;
				mprintf ((0,"Got heartbeat from server.\n"));
			}
			else
				mprintf ((0,"Got heartbeat from slot %d.\n",slot));
			break;
		case MP_INITIAL_RANK:
			MultiDoInitialRank(data);			
			break;
		case MP_MISSILE_RELEASE:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoMissileRelease(slot,data);
			break;
		case MP_STRIP_PLAYER:
			ACCEPT_CONDITION (NETSEQ_PLAYING,NETSEQ_PLAYING);
			MultiDoStripPlayer(slot,data);
			break;
		case MP_REJECTED_CHECKSUM:
			//[ISB] You'll have a positive netseq when this packet comes so always handle it. 
			MultiDoServerRejectedChecksum(data);
			break;
		default:
			mprintf ((0,"Invalid packet type %d!\n",type));
			Int3();	// Invalid packet type!!!!
			break;
	}
}



// Takes a bunch of messages, check them for validity,
// and pass them to multi_process_data. 
void MultiProcessBigData(ubyte *buf, int len,network_address *from_addr)
{
	int type, bytes_processed = 0;
	short sub_len;
	int slot=0;
	int last_type=-1,last_len=-1;

	// Update timer stuff so we don't disconnect due to timeout
	if (Netgame.local_role==LR_CLIENT)
	{
		Netgame.last_server_time=timer_GetTime();
	}
	else
	{
		slot=MultiMatchPlayerToAddress (from_addr);
		if (slot>=0)
		{
			//This is to prevent clients who have disconnected on the client side
			//but not the server side from being credited with a recent packet 
			//just because they are looking for a game
			if( (buf[0]!=MP_GET_PXO_GAME_INFO) && (buf[0]!=MP_GET_GAME_INFO) )
			{
				NetPlayers[slot].last_packet_time=timer_GetTime ();
			}
			else
			{
				//Someone who we think is still connected is looking for game info!
				//We aren't going to process this packet because otherwise they
				//might try to join which wouldn't work and they would be credited
				//with sending a packet and the server wouldn't disconnect them.
				mprintf((0,"Ignoring game info request from a currently connected user (%s)\n",Players[slot].callsign));
				return;
			}
		}
	}

	while( bytes_processed < len )  
	{
		type = buf[bytes_processed];
		sub_len= INTEL_SHORT((*(short *)(buf+bytes_processed+1)));

		if (sub_len<3 || type==0 || (len-bytes_processed)<2)
		{
			mprintf ((0,"Got a corrupted packet!\n"));
			Int3();
			return;  // just throw the rest out
		}
		
		if ( (bytes_processed+sub_len) > len )  
		{
			mprintf( (1, "multi_process_bigdata: packet type %d too short (%d>%d)!\n", type, (bytes_processed+sub_len), len ));
			Int3();
			return;
		}

		MultiProcessData (&buf[bytes_processed],sub_len,slot,from_addr);
		bytes_processed += sub_len;
		last_type=type;
		last_len=sub_len;
	}
}

