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

#ifndef _PLAYER_H
#define _PLAYER_H

#include "pstypes.h"
#include "Inventory.h"

#include "robotfirestruct.h"
#include "object_external_struct.h"
#include "player_external_struct.h"
#include "player_external.h"

#define MAX_WAYPOINTS	25

#define INITIAL_LIVES			3			//start off with 3 lives

//For energy to shields conversion
#define CONVERTER_RATE  20.0f			//10 units per second xfer rate
#define CONVERTER_SCALE  2.0f			//2 units energy -> 1 unit shields
#define CONVERTER_SOUND_DELAY 0.5f		//play every half second

// How long afterburner lasts before it has to be recharged
#ifdef E3_DEMO
#define AFTERBURN_TIME	5000.0
#else
#define AFTERBURN_TIME	5.0
#endif

// Player start position flags
#define PSPF_RED		1
#define PSPF_BLUE		2
#define PSPF_GREEN		4
#define PSPF_YELLOW		8


struct player_pos_suppress
{
	int room;
	float expire_time;
	int ignored_pos;
	bool active;
};


#define PLAYER_POS_HACK_TIME	10

extern player_pos_suppress Player_pos_fix[MAX_PLAYERS];

struct team
{
	char name[CALLSIGN_LEN+1];
	int score;
};

//[ISB] hack ahoy:
//This is some aux data I needed for Piccu features but aren't important enough to warrant changing the player structure over
struct player_extra
{
	float last_pain_time;
};

extern int Player_num;
extern int Default_ship_permission;

//the object which is the person playing the game
extern object *Player_object;

extern int Num_teams,Team_game;
extern player Players[];
extern player_extra PlayersExtra[];
extern team Teams[];
extern float HudNameTan;
extern int Current_waypoint;

extern bool Player_has_camera;
extern int Player_camera_objnum;
extern uint Players_typing;	//information about which players are typing messages (to display an icon)

// How long a player must be dead before he can respawn
#define DEATH_RESPAWN_TIME	3.0f
extern float Total_time_dead;

//Stuff for the new score info on the HUD
extern int Score_added;					//the recently-added amount
extern float Score_added_timer;		//how long the added value will be displayed
#define SCORE_ADDED_TIME 2.0		//how long new score info stays on the screen

// Sets up players array
void InitPlayers();

//Look for player objects & set player starts
extern void FindPlayerStarts();

// Resets all the properties a player ship to the default values
// Pass in what kind of reset the inventory should do INVRESET_
void InitPlayerNewShip (int slot,int inven_reset);

//	makes the player invulnerable
void MakePlayerInvulnerable(int slot,float time,bool play_sound_and_message=false);

//	makes the player invulnerable
void MakePlayerVulnerable(int slot);

//	Performs the player death sequence.
void InitiatePlayerDeath(object *playerobj,bool melee=false,int fate=-1);

//	Forces end to player death
void EndPlayerDeath(int slot);

//	Detaches a subobject from the player ship
void DeadPlayerShipHit(object *obj, vector *hitpt, float magnitude);

//	Do actions for the player each frame
void DoPlayerFrame();

//	Force player to explode (this should be called to intercept the death sequencer's control of explosions
void StartPlayerExplosion(int slot);

// Resets the player object in a mine to stop moving
void ResetPlayerObject(int slot, bool f_reset_pos = true);

// Makes the player into an AI controlled physics object
void PlayerSetControlToAI(int slot,float velocity=75.0f);

// Resets a player's control type back to it's default setting
void ResetPlayerControlType (int slot);

void InitPlayerNewGame (int slot);

// Called from a single player game to get rid of all multiplayer ships
void DeleteMultiplayerObjects ();

// Sets the maximum number of teams in a game
void SetMaxTeams (int num);

// Returns the goal room of the passed in team
int GetGoalRoomForTeam (int teamnum);

// Returns the goal room of the passed in player
int GetGoalRoomForPlayer (int slot);

// Moves a player to a specified start position
void PlayerMoveToStartPos (int slot,int start_slot);

// Returns a random player starting position
int PlayerGetRandomStartPosition (int slot);

// Increases the team score by an amount, returning the new total
int IncTeamScore (int,int);

// Resets the scores for all the teams
void ResetTeamScores ();

// Sets the lighting that a player will cast
void PlayerSetLighting (int slot,float dist,float r,float g,float b);

// Sets a wacky rotating ball around the player ship
void PlayerSetRotatingBall (int slot,int num,float speed,float *r,float *g,float *b);

// Gets the position of a given ball in world coords
void PlayerGetBallPosition (vector *dest,int slot,int num);

// Spews the inventory of the passed in player object
void PlayerSpewInventory (object *obj,bool spew_energy_and_shield=true,bool spew_nonspewable=false);

// Changes the ship a particular player is flying
void PlayerChangeShip (int slot,int ship_index);

// Called when a player is entering a new level
void InitPlayerNewLevel (int slot);

// Sets the FOV range at which the hud names will come on 
void PlayerSetHUDNameFOV (int fov);

// Switches a player object to observer mode
void PlayerSwitchToObserver (int slot,int observer_mode,int piggy_objnum=0);

// Stops a player from observing
void PlayerStopObserving (int slot);

// Sets the players custom texture.  If filename=NULL then sets to no texture
// Returns 1 if filename was successfully loaded, else 0
int PlayerSetCustomTexture (int slot,char *filename);

// Chooses the style of death a player is going to use
int PlayerChooseDeathFate (int slot,float damage,bool melee);

//	Sets/Clears a permission for a ship on a given player
//	if pnum is -1 then all players will be set, else player is the player number
//	returns true on success
bool PlayerSetShipPermission(int pnum,char *ship_name,bool allowed);

//	Resets the ship permissions for a given player
//	pass false for set_default if you don't want the default ship allowed
bool PlayerResetShipPermissions(int pnum,bool set_default);

//	Returns true if the given ship is allowed to be chosen for a pnum
bool PlayerIsShipAllowed(int pnum,char *ship_name);
bool PlayerIsShipAllowed(int pnum,int ship_index);

// Performs the energy->shields tranfer (for the powerup) given the playernum, call this while player
// is holding down the e->s key.
void DoEnergyToShields(int pnum);

// Stop sounds for this player
void PlayerStopSounds (int slot);

// Sets the start position that the player will respawn at
void PlayerAddWaypoint (int index);

//Resets the waypoints (for new level)
void ResetWaypoint();

//Find all the waypoint objects and add them to the list of waypoints
void MakeAtuoWaypointList();

//Sets the auto-waypoint in the object's room to be current
void SetAutoWaypoint(object *objp);

//Returns the team (0 to 3) of the given player
inline int PlayerGetTeam(int pnum)
{
	if(Players[pnum].team==-1){
		//special "no-team" for Dedicated server
		return 0;	//fake a red team
	}

	return Players[pnum].team;
}

//Add the player's score
void PlayerScoreAdd(int playernum,int points);

// steals an item from the given player
void ThiefStealItem(int player_object_handle,int item);
// returns a stolen item to a player
void ThiefReturnItem(int player_object_handle,int item);
// returns true if a player has the specified item to be stolen
bool ThiefPlayerHasItem(int player_object_handle,int item);
#endif
