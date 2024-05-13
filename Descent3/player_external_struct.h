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

#ifndef _PLAYER_EXTERNAL_STRUCT_H_
#define _PLAYER_EXTERNAL_STRUCT_H_

#include "pstypes.h"
#include "vecmat_external.h"
#include "object_external_struct.h"
#include "multi_external.h"

#define MAX_PLAYERS MAX_NET_PLAYERS
#define MAX_TEAMS		4

class Inventory;

#define CALLSIGN_LEN	19
//	THIS CONSTANT MUST NEVER BE OVER 32!!!
#define MAX_PLAYER_WEAPONS	MAX_WBS_PER_OBJ

#define TRACKER_ID_LEN 10	//if this is changed, make sure it's changed in pilottrack.h


//Info on player weapon firing.  
//There is one of these each for the primary & secondary weapons
struct player_weapon
{
	int	index;					//the index of the current primary or secondary weapon
	float firing_time; 			//how long the current weapon has been firing
	int	sound_handle;			//the handle for the sound the firing weapon is making
};

//The structure for a player.  Most of this data will be for multiplayer
struct player
{
	// positional data for player starts
	int start_index;
	vector	start_pos;							//where the player starts
	int		start_roomnum;
	matrix	start_orient;

	int		startpos_flags;					// these flags apply to the start position (used for teams)

	int		ship_index;							// the index into the Ships array that this player is flying

	// Who am I data
	char		callsign[CALLSIGN_LEN+1];		// The callsign of this player, for net purposes.

	// Game data
	uint		flags;								// Powerup flags, see above...
	int		score;								// The player's current score
	float		damage_magnitude;					// for shield effects
	float		edrain_magnitude;					// for energy drain effects
	float		invul_magnitude;					// for invulnerability effects
	float		energy;								// Amount of energy remaining.
	ubyte		lives;								// Lives remaining, 0 = game over.
	sbyte		level;								// Current level player is playing. (must be signed for secret levels)
	sbyte		starting_level;					// What level the player started on.
	ubyte		keys;									// Which keys the player has
	short	 	killer_objnum;						// Who killed me.... (-1 if no one)
	float		invulnerable_time;				// Time left invulnerable
	float		last_hit_wall_sound_time;		// Last time we played a hit wall sound
	float		last_homing_warning_sound_time;  // Obvious  :)  
	float		last_thrust_time;						// Last time the player thrusted
	float		last_afterburner_time;			// Last time the player used the afterburner
	short		objnum;								// The object number of this player
	sbyte		team;									// The team number this guy is on

	//The current auto-waypoint for this player
	int		current_auto_waypoint_room;	//the most recent auto-waypoint, or -1

	// Statistics...
	//	int		score;								// Current score.
	float		time_level;							// Level time played
	float		time_total;							// Game time played (high word = seconds)

	int		num_hits_level;					// used for accuracy calculation.
	int		num_discharges_level;

	short		num_kills_level;					// Number of kills this level
	short		friendly_kills_level;			// Number of friendly kills this level
 	short		num_kills_total;					// Number of kills total

	// Player weapon info
	uint		weapon_flags;							// Mask of currently owned weapons
	ushort	weapon_ammo[MAX_PLAYER_WEAPONS];	// Ammo for each weapon

	//Weapons
	player_weapon weapon[2];					// Info on the player weapons
	ubyte		laser_level;						// Current level of the laser.
	
	// lighting
	float light_dist;
	float r,g,b;

	float ballspeed;
	ubyte num_balls;
	float ball_r[3],ball_g[3],ball_b[3];

	// Room tracking
	int oldroom;
	
	// Inventory
	Inventory inventory;

	// CounterMeasures Inventory
	Inventory counter_measures;

	// Last time the player fired a weapon
	float last_fire_weapon_time;

	// Afterburner stuff
	float afterburner_mag;			// How big the thrust is for the afterburner
	float thrust_mag;
	int afterburner_sound_handle;
	float afterburn_time_left;

	int thruster_sound_handle;
	int thruster_sound_state;

	// For small views and external cameras
	int small_left_obj,small_right_obj,small_dll_obj;


	// Multiplayer stuff
	ubyte multiplayer_flags;
	ubyte last_multiplayer_flags;
	float last_guided_time;

	char tracker_id[TRACKER_ID_LEN];
	int kills;
	int deaths;
	int suicides;
	float rank;
	float lateral_thrust;		//total lateral movement over the whole game
	float rotational_thrust;	//total rotational movement over the whole game
	unsigned int time_in_game; //seconds in game
	object *guided_obj,*user_timeout_obj;

	float zoom_distance;


	// Scalar values
	float movement_scalar;
	float damage_scalar;
	float armor_scalar;
	float turn_scalar;
	float weapon_recharge_scalar;
	float weapon_speed_scalar;

	// Observer stuff
	int piggy_objnum;
	int piggy_sig;

	// Custom texture stuff
	int custom_texture_handle;

	// Ship permissions (1 bit per ship)
	int ship_permissions;

	// For invul hit effect
	vector invul_vector;

	//Used to disable various controller input from scripting
	unsigned int controller_bitflags;

	// Marker stuff
	short num_markers;

	short		num_deaths_level;					// Number of kills this level
	short		num_deaths_total;					// Number of kills total
};

#endif
