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

#ifndef WEAPON_H
#define WEAPON_H

#include "pstypes.h"
#include "manage.h"
#include "object.h"
#include "objinfo.h"
#include "weapon_external.h"

#define MAX_PRIMARY_WEAPONS	10
#define MAX_SECONDARY_WEAPONS	10

#define MAX_WEAPON_NOT_HIT_PARENT_TIME	3.0f

// This can be changed safely
#define MAX_WEAPONS	200

//	THIS CONSTANT SHOULD EQUAL THE NUMBER OF WEAPONS SELECTABLE!!!
#define MAX_STATIC_WEAPONS	21

#define DEFAULT_WEAPON_SIZE	1.0  // Default size of a weapon -- used for bitmap weapons

#define HAS_FLAG(a)	(1<<(a))

// Weapon flags
#define WF_HUD_ANIMATED		(1<<0)
#define WF_IMAGE_BITMAP		(1<<1)	// whether or not the firing image is a bitmap or model
#define WF_SMOKE			(1<<2)	// Weapon drops smoke as it moves
#define WF_MATTER_WEAPON	(1<<3)	// This a matter weapon, as opposed to an energy weapon
#define WF_ELECTRICAL		(1<<4)	// This weapons fires as an electrical storm
#define WF_IMAGE_VCLIP		(1<<5)	// This weapon fire image is a vclip
#define WF_SPRAY			(1<<6)	// This weapon is a spray, like a flamethrower
#define WF_STREAMER			(1<<7)	// This weapon has a streamer effect attached
#define WF_INVISIBLE		(1<<8)	// This weapon is invisible
#define WF_RING				(1<<9)	// This weapon is drawn ring style
#define WF_SATURATE			(1<<10)	// Saturate this bitmap weapon
#define WF_BLAST_RING		(1<<11)	// Creates a blast ring upon explosion
#define WF_PLANAR_BLAST		(1<<12)	// Blast bitmap takes on the walls plane
#define WF_PLANAR			(1<<13)	// This weapon doesn't always face you
#define WF_ENABLE_CAMERA	(1<<14)	// This weapon can be used for missile camera
#define WF_SPAWNS_IMPACT	(1<<15)	// This weapon spawns others on impact
#define WF_SPAWNS_TIMEOUT	(1<<16)	// This weapon spawns others when it times out
#define WF_EXPAND			(1<<17)	// This weapon expands when exploding
#define WF_MUZZLE			(1<<18)	// This weapon produces a muzzle flash when fired
#define WF_MICROWAVE		(1<<19)	// This weapon makes a microwave effect on the victim
#define WF_NAPALM			(1<<20)	// This weapon does a napalm effect to objects it touches
#define WF_REVERSE_SMOKE	(1<<21)	// The smoke trail gets smaller as it ages
#define WF_GRAVITY_FIELD	(1<<22)	// This weapon has a gravity field 
#define WF_COUNTERMEASURE	(1<<23)	// This weapon is a countermeasure
#define WF_SPAWNS_ROBOT		(1<<24)	// This weapon spawns a robot upon death
#define WF_FREEZE			(1<<25)	// This weapon slows a ship/object down
#define WF_TIMEOUT_WALL		(1<<26)	// This weapon times out like a wall hit
#define WF_PLANAR_SMOKE		(1<<27)	// This weapon has a planar smoke trail instead of a blob
#define WF_SILENT_HOMING	(1<<28)	// This weapon does not give a homing lock sound
#define WF_HOMING_SPLIT		(1<<29)	// This weapon homes when it splits
#define WF_NO_ROTATE		(1<<30)	// This weapon does not rotate as a bitmap
#define WF_CUSTOM_SIZE		(1<<31)  // This weapon uses a custom size

#define MAX_LASER_LEVEL			4
#define MAX_SUPER_LASER_LEVEL	6

#define	PRIMARY_INDEX			0
#define	SECONDARY_INDEX			10

// These defines must correspond to the Static_weapons_names array

#define MAX_WEAPON_SOUNDS		7
#define WSI_FIRE				0 // chrishack -- removed!  Change const when we add a new slot
								  // grep for occurances of WSI_FIRE and fix (or remove) old code
#define WSI_IMPACT_WALL			1
#define WSI_FLYING				2
#define WSI_IMPACT_ROBOT		3
#define WSI_BOUNCE				4

struct weapon
{
	char name[PAGENAME_LEN];
	float player_damage;				// how much damage a full impact causes a player
	float generic_damage;				// how much damage a full impact causes a robot
	float alpha;						// What alpha to draw this weapon with
	short sounds[MAX_WEAPON_SOUNDS];	// sounds for various things
	short hud_image_handle;				//  a handle to a bitmap or vclip for the hud display
	short fire_image_handle;				//  model or bitmap.  Shown when you fire this weapon
	short explode_image_handle;			//  exploding vclip
	short smoke_handle;						//  smoke trail handle to texture
	short spawn_handle;						// weapon handle that gets spawned
	short alternate_spawn_handle;		// weapon handle that gets spawned (sometimes)
	short robot_spawn_handle;			// robot that gets spawned as a countermeasure
	short particle_handle;					// particle handle to texture
	short icon_handle;
	short scorch_handle;					// handle for scorch bitmap, or -1 for none
	ubyte spawn_count;					// how many of spawn handle gets created
	ubyte alternate_chance;				// how often the alternate spawn weapon gets chosen (0 to 100)

	unsigned short used;
	
	ubyte particle_count;
	ubyte terrain_damage_depth;

	float terrain_damage_size;

	float scorch_size; 					// how big the scorch mark is
	
	int flags;							//  see above
	
	float size;
	float life_time;
	float thrust_time;
	float impact_size;
	float impact_time;
	float impact_player_damage,impact_generic_damage;
	float impact_force;
	float explode_size;
	float explode_time;
	float particle_size;
	float particle_life;
	float gravity_size;
	float gravity_time;
	float custom_size;
	float homing_fov;
	float recoil_force;

	light_info	lighting_info;
	physics_info phys_info;
	
};

struct fusion_effect
{
	float total_time;
};

extern float Primary_ramp_time,Secondary_ramp_time;

extern int Num_weapons;
extern weapon Weapons[MAX_WEAPONS];
extern const char *Static_weapon_names[];
extern int Static_weapon_names_msg[];
extern int Static_weapon_ckpt_names[][2];

// Sets all weapons to unused
void InitWeapons ();

// Allocs a weapon for use, returns -1 if error, else index on success
int AllocWeapon ();

// Frees weapon index n
void FreeWeapon (int n);

// Gets next weapon from n that has actually been alloced
int GetNextWeapon (int n);

// Gets previous weapon from n that has actually been alloced
int GetPrevWeapon (int n);

// Searches thru all weapons for a specific name, returns -1 if not found
// or index of weapon with name
int FindWeaponName (char *name);

// Given a filename, loads either the model or vclip found in that file.  If type
// is not NULL, sets it to 1 if file is model, otherwise sets it to zero
int LoadWeaponHudImage (char *filename,int *type);

// Given a weapon handle, returns that weapons image for framenum
int GetWeaponHudImage (int handle,int framenum);

// Given a filename, loads either the model or vclip found in that file.  If type
// is not NULL, sets it to 1 if file is model, otherwise sets it to zero
int LoadWeaponFireImage (char *filename,int *type,int *anim,int pageable=1);

// Given a weapon handle, returns that weapons firing bitmap/model
int GetWeaponFireImage (int handle,int frame);


// Given a weapon name, assigns that weapon to a specific index into
// the Weapons array.  Returns -1 if the named weapon is not found, 0 if the weapon
// is already in its place, or 1 if successfully moved
int MatchWeaponToIndex (char *name,int dest_index);

// Moves a weapon from a given index into a new one (above MAX_STATIC_POWERUPS)
// returns new index
int MoveWeaponFromIndex (int index);

// This is a very confusing function.  It takes all the weapons that we have loaded 
// and remaps then into their proper places (if they are static). 
void RemapWeapons ();

// goes thru every entity that could possible have a weapon index (ie objects, weapons, etc)
// and changes the old index to the new index
void RemapAllWeaponObjects (int old_index,int new_index);

// Creates a weapon
// Returns the objnum of the weapon object
int CreateWeaponObject(int weapon_num,int segnum,vector *position,int flags);

// Creates an weapon and sends it speeding on its way
// returns the objnum of the weapon
int CreateAndFireWeapon (vector *pos,vector *dir,object *parent,int weapon_num);

// Given an object and a weapon, fires a shot from that object
// returns the object number of the weapon
int FireWeaponFromObject (object *obj,int weapon_num,int gun_num=-1,bool f_force_forward = false,bool f_force_target = false);


// Draws a weapon
void DrawWeaponObject (object *obj);

//	unconditionally adds a weapon and ammo to a player.
int AddWeaponToPlayer(int slot,int weap_index, int ammo);

//Called when a player dies or switches weapons to clear out any active weapon stuff
void ClearPlayerFiring(object *objp,int weapon_type);

// Fires a weapon from our player.  Won't fire if ammo/energy requirements aren't met.
// Parameters:	weapon_type - either PW_PRIMARY or PW_SECONDARY
void FireWeaponFromPlayer(object *objp,int weapon_type,int down_count,bool down_state,float down_time);

// Fires a flare from our player. 
// It might make sense to combine this with FireWeaponFromPlayer(), or maybe not
void FireFlareFromPlayer(object *objp);

// Does per frame weapon code
void WeaponDoFrame(object *obj);

// Returns the position and the normal of a gun point
bool WeaponCalcGun(vector *gun_point, vector *gun_normal, object *obj, int gun_num);

// Checks for relation between weapons and other objects
bool ObjectsAreRelated( int o1, int o2 );

// A quick way to see where a weapon hits.  Weapons make debris.
void CreateWeaponDebris(object *obj);

// Selects a weapon
void SelectWeapon(int slot);

// automatically switches primary weapon to this value. and type.
int SwitchPlayerWeapon(int weapon_type);

//	Auto selects a weapon, usually the next best weapon.
// weapon_type is either PW_PRIMARY or PW_SECONDARY
//	if new_wpn != -1, then we will see if the current weapon is inferior, to new weapon.  If
//	it isn't, then the new weapon is selected.
// returns true if selecting  new weapon, otherwise false.
bool AutoSelectWeapon(int weapon_type, int new_wpn=-1);

//	is a weapon secondary or primary?
bool IsWeaponSecondary(int index);

// used for sequencing
//	resets memory for what slots have high priority weapons (laser->super_laser, for instance) when user selects
void ResetWeaponSelectStates(ushort new_state=0);

//	save and load weapon state information
void SaveWeaponSelectStates(CFILE *fp);
void LoadWeaponSelectStates(CFILE *fp);

// Draws an alpha blended polygon over the entire 3d rendering scene
// The r,g,b floats specify the color
void DrawAlphaBlendedScreen (float r,float g,float b,float alpha);

// Does the weapon spray effect for an object
void DoSprayEffect (object *obj,otype_wb_info *static_wb,ubyte wb_index);

// Plays the animation that accompanies a weapon death
void DoWeaponExploded (object *,vector *norm=NULL,vector *collision_point=NULL,object *hit_object=NULL);

void TimeoutWeapon (object *);

// Creates chidren from a dying weapon
void CreateImpactSpawnFromWeapon (object *obj,vector *norm);

// Releases the guided missile of a passed in player
void ReleaseGuidedMissile (int slot);

// Releases the user timeout of a passed in player
void ReleaseUserTimeoutMissile (int slot);

// Retreives the weapon in the weapon array based off of an 'index' from 0-19 (non-mapped 
//	primary and secondaries) (we need a ship too.)
weapon *GetWeaponFromIndex(int player, int index);


// Creates a robot as a countermeasure
void CreateRobotSpawnFromWeapon (object *obj);

// Given a parent object and a weapon id, creates that countermeasure
void CreateCountermeasureFromObject (object *parent,int weapon_id);


//////////////////////////////////////////////////////////////////////////////
const ushort	WPNSEL_SKIP = 0x8000,
					WPNSEL_INVALID = 0xffff;


const ushort DefaultPrimarySelectList[MAX_PRIMARY_WEAPONS] = 
{
	LASER_INDEX,
	VAUSS_INDEX,
	MICROWAVE_INDEX,
	PLASMA_INDEX,
	FUSION_INDEX,
	SUPER_LASER_INDEX, 
	MASSDRIVER_INDEX,		 
	NAPALM_INDEX,		 
	EMD_INDEX,		 
	OMEGA_INDEX
};


const ushort DefaultSecondarySelectList[MAX_SECONDARY_WEAPONS] = 
{
	CONCUSSION_INDEX,
	HOMING_INDEX,			
	IMPACTMORTAR_INDEX,
	SMART_INDEX,
	MEGA_INDEX,
	FRAG_INDEX,
	GUIDED_INDEX+WPNSEL_SKIP,
	NAPALMROCKET_INDEX,
	CYCLONE_INDEX,
	BLACKSHARK_INDEX
};


// weapon auto selection info.
ushort GetAutoSelectPrimaryWpnIdx(int slot);
ushort GetAutoSelectSecondaryWpnIdx(int slot);
void SetAutoSelectPrimaryWpnIdx(int slot, ushort idx);
void SetAutoSelectSecondaryWpnIdx(int slot, ushort idx);

//[ISB] Given a primary or secondary weapon index (not slot), determines its priority. Higher values are higher priority. 
//Disabled weapons will receive a priority lower than any non-disabled weapons, but will still maintain relative order
//to other disabled weapons. 
int GetPrimaryPriority(int num);
int GetSecondaryPriority(int num);


#endif
