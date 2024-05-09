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

#ifndef _OBJINFO_H
#define _OBJINFO_H

#include "object.h"
#include "manage.h"
#include "DeathInfo.h"

#ifdef NEWEDITOR
#include "..\neweditor\ned_Object.h"
#include "..\neweditor\ned_Tablefile.h"
#endif

//max sizes for inventory information in the objinfo
#define	MAX_INVEN_DESC_SIZE	180
#define	MAX_INVEN_ICON_SIZE	30

//How many object ids in the array
#define MAX_OBJECT_IDS	910

//How many different sounds each object can make
#define MAX_OBJ_SOUNDS 2

//Object info flags
#define OIF_CONTROL_AI							0x01	//this object uses AI
#define OIF_USES_PHYSICS						0x02	//this object uses physics
#define OIF_DESTROYABLE							0x04	//this object can be destroyed
#define OIF_INVEN_SELECTABLE					0x08	//this object can be selected in the inventory
#define OIF_INVEN_NONUSEABLE					0x10	//this object can not be used by pressing ENTER during the game
#define OIF_INVEN_TYPE_MISSION				0x20	//this object is for Mission objectives
#define OIF_INVEN_NOREMOVE						0x40	//this object should NOT be removed from the inventory when used
#define OIF_INVEN_VISWHENUSED					0x80	//this object will not have it contol type, movement type and render types
#define OIF_AI_SCRIPTED_DEATH					0x100
#define OIF_DO_CEILING_CHECK					0x200
#define OIF_IGNORE_FORCEFIELDS_AND_GLASS	0x400
#define OIF_NO_DIFF_SCALE_DAMAGE				0x800
#define OIF_NO_DIFF_SCALE_MOVE				0x1000
#define OIF_AMBIENT_OBJECT						(1<<13)	//this object is just for show, & can be removed to improve performance

// This next numbers define the distance at which LOD popping occurs in models
#define DEFAULT_MED_LOD_DISTANCE		75.0f
#define DEFAULT_LO_LOD_DISTANCE		120.0f


//The number of ids of each type in the list
extern int Num_object_ids[];

extern char *Movement_class_names[];
extern char *Anim_state_names[];

// These defines must correspond to the Static_object_names array
#define GENOBJ_GUIDEBOT			0		//NOTE: This must match ROBOT_GUIDEBOT
#define GENOBJ_CHAFFCHUNK		1
#define GENOBJ_GUIDEBOTRED		2		//NOTE: This must match ROBOT_GUIDEBOTRED

#define IS_GUIDEBOT(x) (((object *)x)->type==OBJ_ROBOT&&((((object *)x)->id==ROBOT_GUIDEBOTRED)||(((object *)x)->id==ROBOT_GUIDEBOT)))

//Animation constants
#define NUM_MOVEMENT_CLASSES	5
#define NUM_ANIMS_PER_CLASS	24

//Info for an animation state
struct anim_entry
{
	short from,to;
	float spc;
	int anim_sound_index;
	ubyte used;
};

struct anim_elem
{
	anim_entry elem[NUM_ANIMS_PER_CLASS];
};

#define MAX_DSPEW_TYPES 2
#define DSF_ONLY_IF_PLAYER_HAS_OBJ_1 1
#define DSF_ONLY_IF_NO_1             2

//How many different deaths each object can have
#define MAX_DEATH_TYPES 4

//Death info for an object type
struct death_info
{
	uint	flags;		//death flags
	float	delay_min;	//if delay, min amount
	float	delay_max;	//if delay, max amount
};

//AI info for this object
//This is the subset of ai_frame data that the user can edit for an object type
struct t_ai_info
{
	char ai_class;
	char ai_type;

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

	int  sound[MAX_AI_SOUNDS];

	char movement_type;
	char movement_subtype;
	
	int flags;
	int notify_flags;

	float fov;

	float avoid_friends_distance;

	float frustration;
	float curiousity;
	float	life_preservation;
	float agression;

	float fire_spread;
	float	night_vision;
	float	fog_vision;
	float	lead_accuracy;
	float	lead_varience;
	float	fight_team;
	float fight_same;
	float	hearing;
	float	roaming;

	float biased_flight_importance;
	float biased_flight_min;
	float biased_flight_max;
};

#ifndef NEWEDITOR

//Info for robots, powerups, debris, etc.
struct object_info
{
	char	name[PAGENAME_LEN];	//the name on the page

	int	type;						//what type of object this is
	float	size;						//size
	int	flags;					//misc flags.  See above.

//	int	render_type;			//set RT_ defines in object.h
	int	render_handle;			//handle for bitmap/polygon model(hi-res)
	int	med_render_handle;	//handle for med res version of this object
	int	lo_render_handle;		//handle for lo res version of this object   

	float med_lod_distance;		// The distance at which the med-res model takes over
	float lo_lod_distance;		// The distance at which the lo-res model takes over


	int	score;					//how many points you get for killing/picking up

	int	hit_points;				//if destroyable, the hit points
	float damage;
	float impact_size;
	float impact_time;

	int	ammo_count;				//if a powerup, how much ammo it has

	char	*description;						//used for inventory 
	char	icon_name[MAX_INVEN_ICON_SIZE];		//used for inventory

	short	sounds[MAX_OBJ_SOUNDS];		//list of sound handles
	short	dspew[MAX_DSPEW_TYPES];
	float dspew_percent[MAX_DSPEW_TYPES];
	short dspew_number[MAX_DSPEW_TYPES];
	unsigned char f_dspew;

	//Valid for physics objects only
	physics_info	phys_info;					//the physics data for this obj type

	// Valid for lighting of objects
	light_info lighting_info;

	// for multiplayer respawning
	float respawn_scalar;

	bool	multi_allowed;

	// OSIRIS information
	char module_name[MAX_MODULENAME_LEN];
	char script_name_override[PAGENAME_LEN];

	//Death information
	death_info death_types[MAX_DEATH_TYPES];		//the ways this object can die
	ubyte death_probabilities[MAX_DEATH_TYPES];		//how likely each death is, from 0-100 (percent)

	//Valid for AI objects only
	t_ai_info			*ai_info;						//the AI info for this obj type

	// Valid for polygon models with weapons
	otype_wb_info *static_wb;
//	otype_wb_info static_wb[MAX_WBS_PER_OBJ];

	//Valid for polygon model objects only
	anim_elem		*anim;	// which anim states are active
//	anim_elem		anim[NUM_MOVEMENT_CLASSES];	// which anim states are active

};

//The big array of object info
extern object_info Object_info[];

#endif

// Sets all objects to unused
void InitObjectInfo ();

// Frees up all objects (any memory allocated, and resets them) automatic atexit 
void FreeObjectInfo(void);

// Allocs a object for use, returns -1 if error, else index on success
//int AllocObjectID(int type, int flags);
int AllocObjectID(int type, bool f_anim, bool f_weapons, bool f_ai);

// Frees object index n
void FreeObjectID(int n);

//Find an object with the given type.  Returns -1 if none found.
int GetObjectID(int type);

// Gets next object from n of the same type as n
int GetNextObjectID(int n);

// Gets previous object from n that has the same type
int GetPrevObjectID(int n);

// Searches thru all object ids for a specific name
// Returns the found id, or -1 if not found
int FindObjectIDName(char *name);

// Given an object handle, returns an index to that object's model
int GetObjectImage(int handle);

// Given an object, renders the representation of this object
void DrawObject(object *obj);

// Moves an object from a given index into a new one (above MAX_STATIC_IDS)
// returns new index
int MoveObjectFromIndex (int index);

//Remap all the static object ids (objects that must have a specific index) into their assigned slots.
void RemapStaticIDs();

void ChangeOldModelsForObjects (int old_handle,int new_handle);

#endif