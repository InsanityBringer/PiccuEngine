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

#ifndef AISTRUCT_H_
#define AISTRUCT_H_

#include "pstypes.h"
#include "vecmat.h"
#include "aistruct_external.h"
#include "room.h"

//-------------------------------------------------
// GLOBAL Info and Constants for AI
//-------------------------------------------------

#define AI_INVALID_INDEX			-1

// AI system state.

#define AISTAT_ENABLED		1
#define AISTAT_DISABLED		2

extern char AI_Status;

// Last know player info -- might make it a [MAX_PLAYERS]

extern vector ai_lkplayer_pos;
extern vector ai_lkplayer_velocity;
extern float ai_lkplayer_time;

#define MAX_AI_INIT_CLASSES 3
extern char *Ai_class_strings[MAX_AI_INIT_CLASSES];

#define MAX_AI_INIT_TYPES 9
extern char *Ai_type_strings[MAX_AI_INIT_TYPES];

#define MAX_AI_INIT_MOVEMENT_TYPES 4
extern char *Ai_movement_type_strings[MAX_AI_INIT_MOVEMENT_TYPES];

#define MAX_AI_INIT_MOVEMENT_SUBTYPES 7
extern char *Ai_movement_subtype_flying_strings[MAX_AI_INIT_MOVEMENT_SUBTYPES];
extern char *Ai_movement_subtype_walking_strings[MAX_AI_INIT_MOVEMENT_SUBTYPES];

extern float AI_last_time_room_noise_alert_time[MAX_ROOMS+8];

//-------------------------------------------------
// AI Class, Type, and Action indices
//-------------------------------------------------

// AI Class -- make sure to update MAX_AI_CLASSES if you add a new class

#define AIC_STATIC					0
#define AIC_PURE_PATH				1
#define AIC_AIS_FULL				2
//#define AIC_DYNAMIC				3
//#define AIC_AIS_MOVEMENT			4
//#define AIC_AIS_FIRING			5

// AI Action -- What is it doing?

#define AID_SLEEPING				0
#define AID_ATTACKING_OBJ			1
#define AID_WANDERING				2	
#define AID_NOT_THINKING			3
#define AID_CHASE_OBJECT			4
#define AID_RUN_FROM_OBJECT			5
#define AID_HIDE					6
#define AID_FOLLOW_PATH 			7
#define AID_OPEN_DOOR				8

//-------------------------------------------------
// Notification structures
//-------------------------------------------------

struct ain_see
{
	bool f_use_fov;
	float max_dist;
};

struct ain_hear
{
	bool f_directly_player;
	float max_dist;
	float hostile_level; // 0 - 1
	float curiosity_level;
};

//-------------------------------------------------
// Path Structures
//-------------------------------------------------

#define MAX_JOINED_PATHS 5

// Allow for linked paths

#define AIP_STATIC		0
#define AIP_DYNAMIC		1
#define AIP_MOVE_LIST	2

struct ai_path_info
{
	ushort cur_path;
	ushort cur_node;

	ushort num_paths;

	int goal_uid;  // which goal generated this path
	int goal_index;

	// Used by all paths
	ubyte path_id[MAX_JOINED_PATHS];
	ubyte path_type[MAX_JOINED_PATHS];

	// Used for static game paths
	ushort path_start_node[MAX_JOINED_PATHS];
	ushort path_end_node[MAX_JOINED_PATHS];
	ushort path_flags[MAX_JOINED_PATHS];
};

// Used for predefined move lists (off of normal static paths)
struct ai_move_path
{
	vector pos;
	matrix orient;

	short path_id;
};

struct path_information
{
	int path_id;
	int start_node;
	int next_node;
	int end_node;
};

// Goal Ender Structure

struct goal_enabler
{
	char enabler_type;

	union 
	{
		float float_value;
		float time;
		char movement_type;
		int flags;					// Flags that enable/disable this goal
		float dist;
		int awareness;
	};

	float percent_enable;
	float check_interval;
	float last_check_time;
	
	char bool_next_enabler_op;

};

//-------------------------------------------------
// Goal Structures
//-------------------------------------------------

// MAX of 32 goal types unless the bitfield is made wider.

// I wonder if goals can be classified.  If so, we could reserve X goal for class a, and Y goal slots for class b
// plus it would make sure the our slots do not fill up in bad or degenerate ways.

struct gi_fire
{
	short cur_wb;			// for ranged attack
	ubyte cur_mask;		// for ranged attack
	ubyte melee_number;  // this could be union'ed but it makes this struct word aligned
};

struct g_steer
{
	float min_dist;
	float max_dist;
	float max_strength;
};

struct g_floats
{
	float fp1;
	float fp2;
	float fp3;
};

struct g_wander_extra
{
	int avoid_handle;
	char min_rooms;
	char max_rooms;
	char flags;
	char mine_index; // Used for if this object accidently goes on to the terrain
};


#define GAF_ALIGNED								0x01
#define GAF_SPHERE								0x02
#define GAF_TEMP_CLEAR_AUTOLEVEL				0x04
#define GAF_TEMP_CLEAR_ROBOT_COLLISIONS	0x08
#define GAF_TEMP_POINT_COLLIDE_WALLS		0x10

struct g_attach
{
	float rad;
	short flags;
	char parent_ap;
	char child_ap;
};

struct g_static_path
{
	short start_node;
	short end_node;
	short cur_node;
};

struct goal_info
{
	union
	{
		int	handle;
		int   roomnum;
		int	f_actions;
		int	id;				// Type of CUSTOM -- Id determines which one it was
		                     // Also used as the Path ID for static path followers
	};
	
	union
	{
		float time;
		vector vec;
		vector pos;
		g_floats fs;  // goal floats or a vector
	};

	union
	{
		g_steer dist_info;
		g_attach attach_info;
		g_wander_extra wander_extra_info;
		g_static_path static_path_info;
		void *scripted_data_ptr;
	};

};


// Goal structure
struct goal 
{
	unsigned int type;
	char subtype;
	ubyte activation_level;
	float creation_time;
	
	float min_influence;
	union
	{
		float influence;
		float max_influence;
	};

	float ramp_influence_dists[4];  // Sorted by distance

	goal_info g_info;

	char num_enablers;
	goal_enabler enabler[MAX_ENABLERS_PER_GOAL];

	float circle_distance;  
	int status_reg;

	float start_time;
	float next_path_time;  // used of goals with paths associated with them

	float dist_to_goal;

	vector vec_to_target;
	float next_check_see_target_time;
	vector last_see_target_pos;
	float last_see_target_time;
	float next_target_update_time;

	int flags;
	int guid;		// Designer assigned

	int goal_uid;  // used by the AI system for paths

	vector set_fvec;
	vector set_uvec;

	bool used;
};

#define OBJGOAL(x) (((goal *)x)->type&(AIG_GET_AWAY_FROM_OBJ|AIG_HIDE_FROM_OBJ|AIG_GET_TO_OBJ|AIG_ATTACH_TO_OBJ|AIG_FIRE_AT_OBJ|AIG_MELEE_TARGET|AIG_GUARD_OBJ|AIG_DODGE_OBJ|AIG_MOVE_AROUND_OBJ|AIG_MOVE_RELATIVE_OBJ_VEC|AIG_MOVE_RELATIVE_OBJ|AIG_GET_AROUND_OBJ|AIG_AVOID_OBJ|AIG_COHESION_OBJ|AIG_ALIGN_OBJ|AIG_PLACE_OBJ_ON_OBJ))
#define COMPLETEATOBJ(x) (((goal *)x)->type&(AIG_GET_TO_OBJ))
#define POSGOAL(x) (((goal *)x)->type&(AIG_WANDER_AROUND|AIG_GUARD_AREA|AIG_GET_TO_POS|AIG_GET_AROUND_POS))
#define TARGETONLYGOAL(x) (((goal *)x)->type&(AIG_MELEE_TARGET))
#define COMPLETEATPOS(x) (((goal *)x)->type&(AIG_WANDER_AROUND|AIG_GET_TO_POS))

struct notify
{
	union
	{
		int obj_handle;
		int goal_num;
	};

	union
	{
		vector pos;
		int movement_type;
		int anim_type;
		int attack_num;
		int enabler_num;
	};

	float time;
};

struct weapon_hit_info
{
	int parent_handle;
	int weapon_handle;
	int hit_face;
	int hit_subobject;
	float hit_damage;
	vector hit_pnt;
};

//-------------------------------------------------
// AI awareness scale
//-------------------------------------------------


//-------------------------------------------------
// AI Sounds
//-------------------------------------------------

#define MAX_AI_SOUNDS		5


#define AI_MEM_DEPTH 5

struct ai_mem
{
	// Computed at end of memory frame
	float shields;
	short num_enemies;
	short num_friends;

	// Incremented during the memory frame
	short num_times_hit;
	short num_enemy_shots_fired;
	short num_hit_enemy;
	short num_enemy_shots_dodged;
};

//-------------------------------------------------
// AI tracking information
//-------------------------------------------------

//-------------------------------------------------
// AI framework per robot
//-------------------------------------------------

struct ai_frame 
{
	char ai_class;							// Static, DLL, Soar, Flock, and other will be here -- chrishack
	char ai_type;							// Used for some coded types

	ai_path_info path;

	float max_velocity;
	float max_delta_velocity;
	float max_turn_rate;
	float max_delta_turn_rate;

	float attack_vel_percent;
	float flee_vel_percent;
	float dodge_vel_percent;

	float circle_distance;  
	float dodge_percent;

	float melee_damage[2];
	float melee_latency[2];

	int  sound[MAX_AI_SOUNDS];					// AI sounds,
	float last_sound_time[MAX_AI_SOUNDS];
	short last_played_sound_index;

	char movement_type;
	char movement_subtype;
	
	char animation_type;
	char next_animation_type;
	
	char next_movement;	// For queueing actions  :)
	char current_wb_firing;
	char last_special_wb_firing;

	goal goals[MAX_GOALS];	

	//Standard memory
	int target_handle;
	float next_target_update_time;
	
	float dist_to_target_actual;
	float dist_to_target_perceived;
	vector vec_to_target_actual;
	vector vec_to_target_perceived;

	float next_check_see_target_time;
	vector last_see_target_pos;
	float last_see_target_time;
	float last_hear_target_time;

	//int rand_val;
	//float next_rand_time;

	float weapon_speed;

	float next_melee_time;
	float last_render_time;  // Last time I was rendered -- BAD IN MULTIPLAYER -- chrisnote
	float next_flinch_time;  // Next valid time to flinch

	int status_reg;

	int flags;
	int notify_flags;					// Agent is only notified of some event types

	// Normalized movement and facing information
	vector movement_dir;
	vector rot_thrust_vector;

	float fov;

	int anim_sound_handle;  // Goes with Animation sounds which can loop -- not for AI sounds

	float avoid_friends_distance;

	float frustration;
	float curiousity;
	float	life_preservation;
	float agression;

	// Current emotional levels
	float cur_frustration;
	float cur_curiousity;
	float	cur_life_preservation;
	float cur_agression;

	// X Second memory
	float mem_time_till_next_update;
	ai_mem memory[AI_MEM_DEPTH];

	float fire_spread;
	float	night_vision;
	float	fog_vision;
	float	lead_accuracy;
	float	lead_varience;
	float	fight_team;
	float fight_same;
	float	hearing;
	float	roaming;
	float leadership;
	float coop_same;
	float coop_team;

	float biased_flight_importance;
	float biased_flight_min;
	float biased_flight_max;

	vector last_dodge_dir;
	float dodge_till_time;

	float awareness;

	matrix saved_orient;

};

//Etern'ed functions that depend of aistruct stuff

#include "object_external.h"
#include "room.h"


#define MAX_DYNAMIC_PATHS 50
#define MAX_NODES 50

class ai_dynamic_path
{
	public:
	ai_dynamic_path(){num_nodes = 0; use_count = 0; owner_handle = OBJECT_HANDLE_NONE;};

	vector pos[MAX_NODES];
	int roomnum[MAX_NODES];

	short num_nodes;
	short use_count;

	int owner_handle;
};

extern ai_dynamic_path AIDynamicPath[MAX_DYNAMIC_PATHS];
extern int AIAltPath[MAX_ROOMS];
extern int AIAltPathNumNodes;


#endif
