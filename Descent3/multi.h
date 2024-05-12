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

#ifndef MULTI_H
#define MULTI_H

#include "pstypes.h"
#include "vecmat_external.h"
#include "object_external_struct.h"
#include "object_external.h"
#include "player_external.h"


#if defined(__LINUX__)
#include "linux/linux_fix.h"
#endif


#include "multi_external.h"	//defines and structs are in here

extern bool Multi_bail_ui_menu;
#if defined(DEMO)
#define MULTI_VERSION	7
#elif defined(OEM)
#define MULTI_VERSION	5
#else
//#define MULTI_VERSION	4
//Patch 1.1!
//#define MULTI_VERSION	6
//Patch 1.3
#define MULTI_VERSION	10
#endif

#define MULTI_PING_INTERVAL	3

// Multiplayer packets
#define MP_CONNECTION_ACCEPTED					1	// Server says we can join
#define MP_OBJECT								2	// Object packet from the server
#define MP_PLAYER								3	// Name packet from the server
#define MP_ALL_SET								4	// Client is ready to play!
#define MP_PLAYER_POS							5	// Player position packet
#define MP_REQUEST_PLAYERS						6	// Clients wants to know about players
#define MP_MY_INFO								7	// Stats for local client
#define MP_DONE_PLAYERS							8	// Done requesting players
#define MP_ENTERING_GAME						9  // I'm entering the game
#define MP_DISCONNECT							10	// A player has disconnected
#define MP_PLAYER_FIRE							11	// A player is firing
#define MP_ASK_TO_JOIN							12	// Asking if its ok to join
#define MP_JOIN_RESPONSE						13	// Answering MP_ASK_TO_JOIN
#define MP_SERVER_QUIT							14 // Server is quitting
#define MP_LEAVE_GAME							15 // A client is leaving the game
#define MP_BLOWUP_BUILDING						16 // A building is blowing up
#define MP_DONE_BUILDINGS						17	// Server is done sending buildings
#define MP_REQUEST_BUILDINGS					18 // Requesting buildings
#define MP_BUILDING								19 // Server is telling us about buildings
#define MP_DONE_OBJECTS							20	// Server is done sending buildings
#define MP_REQUEST_OBJECTS						21 // Requesting buildings
#define MP_JOIN_OBJECTS							22 // Server is telling us about buildings
#define MP_PLAYER_DEAD							23 // Server says someone is dead!
#define MP_PLAYER_RENEW							24	// A player is coming back from the dead!
#define MP_PLAYER_ENTERED_GAME					25 // A player is entering the game
#define MP_DAMAGE_PLAYER						26 // A player should take damage
#define MP_MESSAGE_FROM_SERVER					27	// A text message from the server
#define MP_END_PLAYER_DEATH						28	// A player is done dying
#define MP_RENEW_PLAYER							29	// Renew a player (a new ship!)
#define MP_GET_GAME_INFO						30	// Someone is asking about our game
#define MP_GAME_INFO							31 // Server is telling us about its game
//@@#define MP_EXECUTE_SCRIPT					32	// Server says to execute a script
#define MP_MESSAGE_TO_SERVER					33 // A message from the client to the server
#define MP_SPECIAL_PACKET						34 // a special data packet for scripts
#define MP_EXECUTE_DLL							35 // Server says to execute a dll
#define MP_REMOVE_OBJECT						36 // Server says to remove an object
#define MP_GUIDED_INFO							37 // Guided missile info from a player
#define MP_LEVEL_ENDED							38	// Server says the level has ended
#define MP_READY_FOR_LEVEL						39 // Client wants info about the level
#define MP_LEVEL_INFO							40 // Server is telling client about the level
#define MP_GET_PXO_GAME_INFO					41	// Same as MP_GET_GAME_INFO but for PXO games only
#define MP_POWERUP_REPOSITION					42	// The server is telling the client to move a powerup
#define MP_GET_GAMETIME							43	// Client equesting the server's gametime
#define MP_HERE_IS_GAMETIME						44 // Server's gametime response
#define MP_ROBOT_POS							45	// Robot position and orientation
#define MP_ROBOT_FIRE							46	// A robot is firing
#define MP_ROBOT_DAMAGE							47 // Apply damage to robot
#define MP_ROBOT_EXPLODE						48 // Blow up robot
#define MP_ON_OFF								49 // a player is starting or stopping an on/off weapon
#define MP_ADDITIONAL_DAMAGE					50 // Server is telling us to add or subtract shields
#define MP_ANIM_UPDATE							51	// Server is sending an animation update packet
#define MP_REQUEST_COUNTERMEASURE				52	// Client is asking the server to create a countermeasure
#define MP_CREATE_COUNTERMEASURE				53 // Server is telling us to create a countermeasure
#define MP_PLAY_3D_SOUND_FROM_OBJ				54 // Server sending a 3d sound based on an obj position
#define MP_PLAY_3D_SOUND_FROM_POS				55 // Server sending a 3d sound based on an arbitrary position
#define MP_ROBOT_FIRE_SOUND						56 // The packet type which makes the firing sound from the robot
//@@#define MP_EXECUTE_SCRIPT_WITH_PARMS		57 // Execute Script with parameters
#define MP_TURRET_UPDATE						58 // Update on turret info from the server
#define MP_CLIENT_USE_INVENTORY_ITEM			59	//	Client is telling the server that he want's to use an item from his inventory
#define MP_REMOVE_INVENTORY_ITEM				60	// Server is telling the clients to remove an item from a player's inventory
#define MP_SERVER_SENT_COUNT					61	//	The server is telling the client how much data he has sent
#define MP_CLIENT_SET_PPS						62	//	The client is telling the server what pps to use
#define MP_GREETINGS_FROM_CLIENT				63	//	The client is identifying himself so the server knows his ip
#define MP_REQUEST_TO_OBSERVE					64	// A player is requesting to enter observe mode
#define MP_OBSERVER_CHANGE						65 // The server is telling us about an observer change
#define MP_VISIBLE_PLAYERS						66 // Server is telling us what players are visible
#define MP_FILE_REQ								67	// Request a sound or bmp for a particular player slot
#define MP_FILE_DENIED							68	// The server isn't going to send you a file you asked for (no soup for you)
#define MP_FILE_DATA							69	// Data chunk, which is part of a file xfer
#define MP_FILE_ACK								70	// When you receive a chunk of data, reply with this ACK and the sender will send the next chunk
#define MP_SERVER_ECHO_REQ						71 // Special packet the server sends to an echo server (on the mastertracker) which responds with his IP address and port. It makes sure servers behind firewalls/NAT/proxy work
#define MP_SERVER_ECHO_RSP						72 // Response from Echo server with our real IP and port
#define MP_FILE_CANCEL							73	// Cancel an existing file transfer (client or server/Sender or receiver)
#define MP_INVISIBLE_PLAYER						74	// Just a packeting test
#define MP_PLAYER_CUSTOM_DATA					75	//	This player has custom data. Here are the file names
#define MP_ABORT_JOIN_SERVER					76	// Stop trying to join a server because the level is ending or server quiting
#define MP_ENERGY_SHIELD_CONV_ON_OFF			77	// Client wants to do an energy to shield conversion
#define MP_GHOST_OBJECT							78	//Tells clients to ghost or unghost an object
#define MP_PLAYER_PING							79	// Ask for a ping
#define MP_PLAYER_PONG							80	// Response to a ping.
#define MP_PLAYER_LAG_INFO						81	// Tell clients in a client/server game what the ping of another player is.
#define MP_REQUEST_DAMAGE						82	// We're asking the server to damage us
#define MP_REQUEST_SHIELDS						83 // We're asking the server to give us shields
#define MP_REQUEST_WORLD_STATES					84	// Clients wants to know about the world state
#define MP_DONE_WORLD_STATES					85 // Server says we're done with world states
#define MP_WORLD_STATES							86 // Information about the world
#define MP_ATTACH_OBJ							87	// Attach an object with the attach system
#define MP_UNATTACH_OBJ							88	// UnAttach an object with the attach system
#define MP_AIWEAP_FLAGS							89	// Allows robots to have continous and spray weapons
#define MP_ATTACH_RAD_OBJ						90 // Attach by rad
#define MP_TIMEOUT_WEAPON						91	// Timeout weapon
#define MP_WEAPONS_LOAD							92	// Client is telling server about weapons load
#define MP_REQUEST_PEER_DAMAGE					93 // Client is asking another player to damage him
#define MP_MSAFE_FUNCTION						94	// Msafe function
#define MP_MSAFE_POWERUP						95 // Multisafe powerup
#define MP_ASK_FOR_URL							96	//Ask for a url list from the server
#define MP_CUR_MSN_URLS							97	//Response from the server listing the URLs for the current mission
#define MP_REQUEST_TO_FIRE						98	//Client is requesting to fire
#define MP_PERMISSION_TO_FIRE					99	//Permissable firing packet
#define MP_CONNECT_BAIL							100 // Server says to bail on connecting
#define MP_CLIENT_PLAY_TAUNT					101	// Client is requesting to play audio taunt
#define MP_SERVER_PLAY_TAUNT					102	// Server is telling clients to play a player's audio taunt
#define MP_REQUEST_MARKER						103	// Client is asking for a marker to be dropped
#define MP_REQUEST_TYPE_ICON					104	// Client is telling server that he is typing a message
#define MP_SEND_TYPE_ICON						105	// Server is telling clients that a player is typing a message (to display icon on hud)
#define MP_REQUEST_TO_MOVE						106 // Client is requesting to move
#define MP_ADJUST_POSITION						107 // Server is telling me my new position
#define MP_GENERIC_NONVIS						108	// Server says I can't see this generic object
#define MP_SEND_DEMO_OBJECT_FLAGS				109	// Server is sending what join objects have the OF_CLIENTDEMOOBJECT set
#define MP_GUIDEBOTMENU_REQUEST					110	//Client is requesting either the guidebot menu, or one of the items on the menu
#define MP_GUIDEBOTMENU_DATA					111	//Server is sending the guidebot text to display in the menu
#define MP_BREAK_GLASS							112 // Server is telling us to break some glass
#define MP_THIEF_STEAL							113	// Server is telling a client that the thief stole an item from him
#define MP_REQUEST_PLAYERLIST					114	// Client is requesting a list of players for this game
#define MP_PLAYERLIST_DATA						115	// Server is sending a list of players in this game
#define MP_SERVER_AUDIOTAUNT_TIME				116
#define MP_CHANGE_RANK							117	// Server says a player has changed rank
#define MP_BASHPLAYER_SHIP						118	// Server says we need to use a different ship
#define MP_HEARTBEAT							119 // A blank packet that says we're still alive (for loading levels)
#define MP_INITIAL_RANK							120 // Server is telling me about rank
#define MP_MISSILE_RELEASE						121 // Informing about a guided missile being released from guided mode
#define MP_STRIP_PLAYER							122 // Strips player of all weapons (but laser) and reduces energy to 0
#define MP_REJECTED_CHECKSUM					123 // The server rejected the client checksum. This lets the client know.

// Shield request defines
#define MAX_SHIELD_REQUEST_TYPES	1
#define SHIELD_REQUEST_ENERGY_TO_SHIELD	0


#define SERVER_PLAYER	128

#define MASTER_TRACKER_SIG	0xf429b142

// values for fired_on_this_frame
#define PFP_NO_FIRED		0	// the player didn't fire at all this frame
#define PFP_FIRED			1	// the player fired this frame and the info should be packed in a player pos flag
#define PFP_FIRED_RELIABLE	2	// used for secondaries of a non-peer to peer game
struct player_fire_packet
{
	ubyte fired_on_this_frame;
	ubyte wb_index;
	ubyte fire_mask;
	ubyte damage_scalar;
	ubyte reliable;
	int dest_roomnum;
};

extern netgame_info Netgame;

extern ushort Local_object_list[];
extern ushort Server_object_list[];
extern ushort Server_spew_list[];

#define MAX_RECEIVE_SIZE	4096
#define MAX_NETWORK_GAMES	100

// The size of the packet_number and guaranteed fields of game_packet_data:
#define HEADER_INFO_SIZE	5

#define NETFILE_NONE					0		// No file transfer in progress
#define NETFILE_SENDING				1		// Sending a file to someone
#define NETFILE_RECEIVING			2		// Receiving a file from someone
#define NETFILE_ASKING				3		// Waiting for a response as to if we can get a file

#define NETFILE_ID_NOFILE			0		// No file at all (for sequencing)
#define NETFILE_ID_SHIP_TEX		1		// Custom ship texture
#define NETFILE_ID_VOICE_TAUNT1	2		// Voice taunt #1
#define NETFILE_ID_VOICE_TAUNT2	3		// Voice taunt #2
#define NETFILE_ID_VOICE_TAUNT3	4		// Voice taunt #3
#define NETFILE_ID_VOICE_TAUNT4	5		// Voice taunt #4
#define NETFILE_ID_DONE				99		// Done transferring files

//This sets the last file we look for in the sequence above
#define NETFILE_ID_LAST_FILE		NETFILE_ID_VOICE_TAUNT4

// A semi-compressed orientation matrix for multiplayer games
struct multi_orientation
{
	short multi_matrix[9];
};

inline void MultiMatrixMakeEndianFriendly(multi_orientation *mmat)
{
	for(int i=0;i<9;i++)
	{
		mmat->multi_matrix[i] = INTEL_SHORT(mmat->multi_matrix[i]);
	}
}

// For firing players
extern player_fire_packet Player_fire_packet[MAX_NET_PLAYERS];

// For powerup respawning
#define MAX_RESPAWNS		300
#define RESPAWN_TIME		60		// seconds until a powerup respawns

#define MULTI_SEND_MESSAGE_ALL			-1
#define MULTI_SEND_MESSAGE_RED_TEAM		-2
#define MULTI_SEND_MESSAGE_BLUE_TEAM	-3
#define MULTI_SEND_MESSAGE_GREEN_TEAM	-4
#define MULTI_SEND_MESSAGE_YELLOW_TEAM	-5

struct powerup_respawn
{
	vector pos;
	int objnum;
	int roomnum;
	ubyte used;
	short original_id;
};

struct powerup_timer
{
	int id;
	float respawn_time;
};

extern powerup_timer Powerup_timer[];
extern powerup_respawn Powerup_respawn[];
extern network_game Network_games[];
extern netplayer NetPlayers[MAX_NET_PLAYERS];
extern ubyte Multi_receive_buffer[MAX_RECEIVE_SIZE];
extern int Ok_to_join;
extern int Num_powerup_respawn;
extern int Num_powerup_timer;
extern int Multi_next_level;

// Heartbeat flag
extern bool Got_heartbeat;

// This is for breakable glass
#define MAX_BROKE_GLASS	100
extern ushort Broke_glass_rooms[],Broke_glass_faces[];
extern int Num_broke_glass;

// For keeping track of damage and shields
extern float Multi_additional_damage[];
extern int Multi_requested_damage_type;
extern float Multi_requested_damage_amount;
extern float Multi_additional_shields[];

extern ubyte Multi_send_buffer[MAX_NET_PLAYERS][MAX_GAME_DATA_SIZE];
extern int Multi_send_size[MAX_NET_PLAYERS];
extern float Multi_last_sent_time[MAX_NET_PLAYERS][MAX_NET_PLAYERS];
extern int Multi_additional_damage_type[MAX_NET_PLAYERS];

extern ubyte Multi_reliable_urgent[MAX_NET_PLAYERS];
extern ubyte Multi_reliable_send_buffer[MAX_NET_PLAYERS][MAX_GAME_DATA_SIZE];
extern int Multi_reliable_send_size[MAX_NET_PLAYERS];
extern float Multi_reliable_last_send_time[MAX_NET_PLAYERS];
extern ubyte Multi_reliable_sent_position[MAX_NET_PLAYERS];
extern uint Multi_visible_players[];

extern int Got_level_info;
extern int Got_new_game_time;
// For keeping track of buildings that have changed
extern ubyte Multi_building_states[];
extern ushort Multi_num_buildings_changed;

extern bool Multi_logo_state;

// For searching out netgames
extern int Num_network_games_known;

// Is this a master tracker game?
extern int Game_is_master_tracker_game;

#define TRACKER_ID_LEN	10	//Don't change this!
extern char Tracker_id[TRACKER_ID_LEN];

extern ushort Turrett_position_counter[MAX_OBJECTS];


#define LOGIN_LEN 33
#define REAL_NAME_LEN 66
#define PASSWORD_LEN 17
#define EMAIL_LEN 100
#define TRACKER_ID_LEN 10
#define PILOT_NAME_LEN	20

#if defined(WIN32)
#pragma pack(push,pxo)
#endif
#pragma pack(1)	//Single byte alignment!
struct vmt_descent3_struct 
{
	char tracker_id[TRACKER_ID_LEN];
	char pilot_name[PILOT_NAME_LEN];
	int rank;

	int kills;
	int deaths;
	int suicides;
	int online_time;
	int games_played;
	unsigned int security;
	unsigned char virgin_pilot;	//This pilot was just created if TRUE
	unsigned int lateral_thrust;
	unsigned int rotational_thrust;
	unsigned int sliding_pct;	//Percentage of the time you were sliding
	unsigned long checksum;			//This value needs to be equal to whatever the checksum is once the packet is decoded
	unsigned long pad;			//just to provide room for out 4 byte encryption boundry only needed on the client side for now
};

#define DESCENT3_BLOCK_SIZE (sizeof(vmt_descent3_struct)-4)
#if defined(WIN32)
#pragma pack(pop,pxo)
#else
#pragma pack()
#endif

extern vmt_descent3_struct MTPilotinfo[MAX_NET_PLAYERS];

//Display a menu based on what the server just told us about
void MultiDoGuidebotMenuData(ubyte *data);

//The user either wants a menu to display or they took an action off of the menu
void MultiDoGuidebotMenuRequest(ubyte *data,int slot);




void MultiProcessBigData(ubyte *buf, int len,network_address *from_addr);

// Points the server_address variable at a new location
void MultiSetServerAddress (network_address *addr);

// Does multiplayer stuff for one frame
void MultiDoFrame();

// Returns 1 on success, 0 on fail
int TryToJoinServer (network_address *addr);

// The server says we can join! 
void MultiDoConnectionAccepted(ubyte *data);

// Polls for a connection message so we can finally join this game
void MultiPollForConnectionAccepted ();

// Gets a new connection set up
void MultiSendConnectionAccepted (int slotnum,SOCKET sock,network_address *addr);

// Starts a new multiplayer game
void StartNewMultiplayerGame ();

// Clears all connections 
// Server and Client
void MultiCloseGame ();

// The server sends to everyone that the player is dead
void MultiSendPlayerDead (int slot,ubyte fate);

// Called when you want to leave a netgame
void MultiLeaveGame ();


// MultiProcessIncoming reads incoming data off the unreliable and reliable ports and sends
// the data to process_big_data
void MultiProcessIncoming();

// Starts a packet of data
int START_DATA (int type,ubyte *data,int *count,ubyte reliable=0);

// End a pakcet of data
void END_DATA (int count,ubyte *data,int offset);

// Skip the header stuff
void SKIP_HEADER (ubyte *data,int *count);

// Starts a level for multiplayer
bool MultiStartNewLevel (int level);

// Returns the number of players in a game
int MultiCountPlayers ();

// Puts player "slot" position info into the passed in buffer
// Returns the number of bytes used
int MultiStuffPosition (int slot,ubyte *data);

// Sends a full packet out the the server
// Resets the send_size variable
// If slot = -1, sends out to the server
void MultiSendFullPacket (int slot,int flags);

void MultiSendFullReliablePacket (int slot,int flags);

// Makes the passed in player a ghost
void MultiMakePlayerGhost (int slot);


// Makes the passed in player real (non-ghost)
void MultiMakePlayerReal (int slot);

// Sends a fire packet from a player
void MultiSendFirePlayerWB (int playernum,ubyte wb_index,ubyte fire_mask,ubyte reliable=0,float scalar=1.0);

void MultiMakeMatrix (multi_orientation *dest,matrix *src);

// Extracts a matrix from an abbreviated matrix
void MultiExtractMatrix (matrix *dest,multi_orientation *src);

void MultiSendBlowupBuilding (int,int,float);

// Return index of powerup that has matching table entry
int MultiMatchPowerup (int unique_id);

// Return index of robot that has matching table entry
int MultiMatchRobot (int unique_id);

// Builds the tables for uniquely identifying powerups and robots
void MultiBuildMatchTables();

// Return index of generic that has matching table entry
int MultiMatchWeapon (uint unique_id);

// Tell my clients about damage done to a player
void MultiSendDamagePlayer (int,int,int type,float amount);

// Send a message!
void MultiSendMessageFromServer (int,char *,int to=MULTI_SEND_MESSAGE_ALL);

// Tells the server that I'm done dying
void MultiSendEndPlayerDeath ();

// Returns the unique id of a given object type/id
uint MultiGetMatchChecksum (int type,int id);

// Return index of generic that has matching table entry
int MultiMatchGeneric (uint unique_id);

// Sends a message from client to server
void MultiSendMessageToServer (int, char *,int to=MULTI_SEND_MESSAGE_ALL);

// Sends an object from the server to the client
void MultiSendObject (object *obj,ubyte announce,ubyte demo_record=false);

// Paints all the goal rooms in a level with their colors
void MultiPaintGoalRooms (int *texcolors=NULL);

// Sends a special script packet to a player
void MultiSendSpecialPacket (int slot,ubyte *outdata,int size);

// Flushes all receive sockets so that there is no data coming from them
void MultiFlushAllIncomingBuffers ();

// Tells all clients to remove a specified object
void MultiSendRemoveObject (object *obj,ubyte playsound);

// Sends the special script packet to the server
void MultiClientSendSpecialPacket (ubyte *outdata,int size);

// Sends info out on my guided missile into a slot
// returns number of bytes in packet
int MultiStuffGuidedInfo (ubyte,ubyte *);

// Polls for a connection message so we can finally join this game
// Client only
int MultiPollForLevelInfo ();

// Server is telling us about the level
void MultiDoLevelInfo (ubyte *data);

// Server is telling the client about the level currently playing
// Server only
void MultiSendLevelInfo (int slot);

// Clients says he's ready for level info
// so send it to him
void MultiDoReadyForLevel (ubyte *data);

// Client is telling the server that he is ready for a level 
// Client only
void MultiSendReadyForLevel ();

// Tells all the clients to end the level
void MultiSendLevelEnded (int success,int next_level);

// Some DLL is telling us to end the level!
void MultiEndLevel ();

//Request the server's gametime
void GetServerGameTime();

//Send robot info
int MultiStuffRobotPosition (unsigned short objectnum,ubyte *data);

//Handle robot position
void MultiDoRobotPos (ubyte *data);

//Handle robot (or any AI created) weapon fire
int MultiSendRobotFireWeapon (unsigned short objectnum,vector *pos,vector *dir,unsigned short weaponnum);

//Send robot damage
void MultiSendKillObject (object *hit_obj,object *killer,float damage,int death_flags,float delay,short seed);

//handle robot damage
void MultiDoRobotExplode(ubyte *data);

// Peer to peer request for damage
void MultiSendRequestPeerDamage (object *,int,int,float);

// Tell all the clients about this players rank
void MultiSendInitialRank (int pnum);

// Tells the other players that a slot is starting/stopping its on/off weapon
void MultiSendOnOff (object *obj,ubyte on,ubyte wb_index,ubyte fire_mask);

// Tells all the clients to apply damage to a player
void MultiSendAdditionalDamage (int slot,int type,float amount);

// We're asking the server to create a countermeasure for us
void MultiSendRequestCountermeasure (short objnum,int weapon_index);

// Tell the client that an object took damage
void MultiSendDamageObject (object *hit_obj,object *killer,float damage,int weaponid);

// Handle message from server that robot/object took damage
void MultiDoRobotDamage(ubyte *data);

// Add an object to the list of objects that need an animation update next player packet interval
void MultiAddObjAnimUpdate(int objnum);

// Stuff an animation update into a packet
int MultiStuffObjAnimUpdate(unsigned short objnum, ubyte *data);

// Handle an animation update
void MultiDoObjAnimUpdate(ubyte *data);

// Play a 3d sound that the server told us about
void MultiDoPlay3dSound(ubyte *data);

// Tell the clients to play a 3d sound
void MultiPlay3dSound(short soundidx,ushort objnum,int priority);

// Tell the client to play a sound because a robot fired
void MultiSendRobotFireSound(short soundidx,ushort objnum);

// Play the robot sound that the server told us about
void MultiDoRobotFireSound(ubyte *data);

// Add a turret to the list of stuff to be updated
void MultiAddObjTurretUpdate(int objnum);

// Stuff turret data into a packet
int MultiStuffTurretUpdate(unsigned short objnum, ubyte *data);

// Handle a turret update from the server
void MultiDoTurretUpdate(ubyte *data);

// Handle a client use inventory item packet
void MultiDoClientInventoryUseItem(int slot,ubyte *data);

// Send a request to use an inventory item to the server
void MultiSendClientInventoryUseItem(int type,int id);

// Handle a remove item from inventory
void MultiDoClientInventoryRemoveItem(int slot,ubyte *data);

// Tell the clients to remove an item from a player's inventory
void MultiSendInventoryRemoveItem(int slot,int type,int id);

void MultiAddObjWBAnimUpdate(int objnum);

int MultiStuffObjWBAnimUpdate(unsigned short objnum, ubyte *data);

void MultiDoObjWBAnimUpdate(ubyte *data);

void MultiDoBytesSent(ubyte *data);

void MultiSendBytesSent(int slot);

void MultiSendPPSSet(int pps);

void MultiDoPPSSet(ubyte *data,int slot);

void MultiSendGreetings(unsigned int id);

void MultiDoGreetings (ubyte *data,network_address *addr);

// We're asking to enter observer mode
void MultiSendRequestToObserve (int mode,int on,int objnum);

// Server is telling us about players that we can see
void MultiDoVisiblePlayers (ubyte *data);

// Sends all the visible players to another player
void MultiSendVisiblePlayers (int to_slot);

void MultiDoFileReq(ubyte *data);

void MultiDoFileDenied(ubyte *data);

void MultiDoFileData(ubyte *data);

void MultiDoFileAck(ubyte *data);

//	Tells clients that a particular player's custom data is here and ready for downloading
void MultiSendClientCustomData(int slot,int whoto = -1);

void MultiCancelFile(int playernum,int filenum,int file_who);

void MultiAskForFile(ushort file_id,ushort file_who,ushort who);

void DoNextPlayerFile(int playernum);

// We're asking the server to damage us
void MultiSendRequestDamage (int type,float amount);

// Asks the server for shields based on frametime "amount" x the type of shields requested
void MultiSendRequestShields (int type,float amount);

// Tells the clients to ghost or unghost an object
void MultiSendGhostObject( object *obj, bool ghost);
void MultiDoGhostObject (ubyte *data);

// Sends this nonreliable packet to everyone except the server and the named slot
void MultiSendToAllExcept (int except,ubyte *data,int size,int seq_threshold);

// Tells the server about the weapons we're carrying
void MultiSendWeaponsLoad ();

// Tell clients to break some glass
void MultiSendBreakGlass (room *rp,int facenum);

// Sends a heartbeat to the server
void MultiSendHeartbeat ();

//Ping functions to find the players latency
void MultiDoPong (ubyte *data);
void MultiDoPing (ubyte *data,network_address *addr);
void MultiSendPing( int slot);
void MultiDoLagInfo(ubyte *data);

// Stuffs a players firing information into a packet
int MultiStuffPlayerFire (int slot,ubyte *data);

// Stuffs request to move into a packet
int MultiStuffRequestToMove (ubyte *data);

// Stuff info for a guided missile 
int MultiStuffGuidedInfo (int slot,ubyte *data);

// Tell everyone I'm timingout my timeout weapon
void MultiSendReleaseTimeoutMissile ();

// We're asking the server for permission to fire!
void MultiSendRequestToFire (int,int,float scalar=1.0);

// Client is asking the server to play an audio taunt
void MultiSendRequestPlayTaunt(int index);

// Client is asking for a marker
void MultiSendRequestForMarker (char *message);

// Client is telling the server that he is [not] typing a hud message
void MultiSendRequestTypeIcon(bool typing_message);

// Sets whether or not the server answsers to a connection request
extern bool Multi_accept_state;
void MultiSetAcceptState (bool state);

void MultiSendAiWeaponFlags(object *obj, int flags, int wb_index);
void MultiDoAiWeaponFlags(ubyte *data);
void MultiSendAttach(object *parent, char parent_ap, object *child, char child_ap, bool f_aligned);
void MultiDoAttach(ubyte * data);
void MultiSendAttachRad(object *parent, char parent_ap, object *child, float rad);
void MultiDoAttachRad(ubyte * data);
void MultiSendUnattach(object *child);
void MultiDoUnattach(ubyte * data);

void MultiDoJoinDemoObjects (ubyte *data);

// Rank stuff
void MultiDoChangeRank (ubyte *data);

// Sets whether or not we want the logos to be displayed on ships
void MultiSetLogoState (bool state);

void MultiSendThiefSteal(int player,int item);
void MultiDoThiefSteal(ubyte * data);

void MultiSetAudioTauntTime(float time,int to_who=-1);
void MultiDoAudioTauntTime(ubyte *data);

// Server only function to clear a Guidebot for a disconnected player
void MultiClearGuidebot(int slot);

// Guided missile release
void MultiDoMissileRelease(int from_slot,ubyte *data);
void MultiSendMissileRelease(int slot,bool is_guided);

//Server telling a client what ship to switch to
void MultiBashPlayerShip(int slot,char *ship);

// Strips a player bare of weapons
void MultiSendStripPlayer(int slot);

inline void MultiGetTypeID(ubyte *data,int *count,int *type,int *id)
{
	*id = -1;
	*type = MultiGetByte(data,count);

	uint hash_value = MultiGetUint(data,count);

	if((*type)==OBJ_WEAPON)
		*id = MultiMatchWeapon(hash_value);
	else
		*id = MultiMatchGeneric(hash_value);
}

inline void MultiAddTypeID(int type,int id,ubyte *data,int *count)
{
	uint hash_value;
	hash_value = MultiGetMatchChecksum(type,id);

	MultiAddByte(type,data,count);
	MultiAddUint(hash_value,data,count);
}

int MultiGetShipChecksum (int ship_index);


#endif

