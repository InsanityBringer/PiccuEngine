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

#include "weapon.h"
#include "pstypes.h"
#include "pserror.h"
#include "object.h"
#include "3d.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "bitmap.h"
#include "vclip.h"
#include "game.h"
#include "polymodel.h"
#include "player.h"
#include "hud.h"
#include "hlsoundlib.h"
#include "soundload.h"
#include "objinfo.h"
#include "gametexture.h"
#include "ship.h"
#include "gauges.h"
#include "sounds.h"
#include "stringtable.h"
#include "Macros.h"
#include "CFILE.H"
#include "AIMain.h"

//#include "samirlog.h"
#define LOGFILE(_s)

weapon Weapons[MAX_WEAPONS];
int Num_weapons = 0;

const char* Static_weapon_names[] =
{
	//	Primary weapons
		"Laser",
		"Vauss",
		"Microwave",
		"Plasma",
		"Fusion",
		"Super Laser",
		"Mass Driver",
		"Napalm",
		"EMD Gun",
		"Omega",
		//	Secondary weapons
			"Concussion",
			"Homing",
			"Impact Mortar",
			"Smart",
			"Mega",
			"Frag",
			"Guided",
			"Napalm Rocket",
			"Cyclone",
			"Black Shark",
			// The flares
				"Yellow Flare",
};

int	Static_weapon_names_msg[] =
{
	//	Primary weapons
		TXI_WPN_LASER,
		TXI_WPN_VAUSS,
		TXI_WPN_MICROWAVE,
		TXI_WPN_PLASMA,
		TXI_WPN_FUSION,
		TXI_WPN_SUPLASER,
		TXI_WPN_MASSDRIVER,
		TXI_WPN_NAPALM,
		TXI_WPN_EMDGUN,
		TXI_WPN_OMEGA,
		//	Secondary weapons
			TXI_WPN_CONCUSSION,
			TXI_WPN_HOMING,
			TXI_WPN_IMPACT,
			TXI_WPN_SMART,
			TXI_WPN_MEGA,
			TXI_WPN_FRAG,
			TXI_WPN_GUIDED,
			TXI_WPN_NAPALMR,
			TXI_WPN_CYCLONE,
			TXI_WPN_BLACKSHARK,
			// The flares
				TXI_WPN_YELL_FLARE,
};

int Static_weapon_ckpt_names[][2] =
{
	//	Primary weapons
		{TXI_WPNC_LASER_1,TXI_WPNC_LASER_2},
		{TXI_WPNC_VAUSS_1,TXI_WPNC_VAUSS_2},
		{TXI_WPNC_MICRO_1,TXI_WPNC_MICRO_2},
		{TXI_WPNC_PLASMA_1,TXI_WPNC_PLASMA_2},
		{TXI_WPNC_FUSION_1,TXI_WPNC_FUSION_2},
		{TXI_WPNC_SUPLAS_1,TXI_WPNC_SUPLAS_2},
		{TXI_WPNC_MASSD_1,TXI_WPNC_MASSD_2},
		{TXI_WPNC_NAPALM_1,TXI_WPNC_NAPALM_2},
		{TXI_WPNC_EMD_1,TXI_WPNC_EMD_2},
		{TXI_WPNC_OMEGA_1,TXI_WPNC_OMEGA_2},
		//	Secondary weapons
			{TXI_WPNC_CONC_1,TXI_WPNC_CONC_2},
			{TXI_WPNC_HOMING_1,TXI_WPNC_HOMING_2},
			{TXI_WPNC_IMPACT_1,TXI_WPNC_IMPACT_2},
			{TXI_WPNC_SMART_1,TXI_WPNC_SMART_2},
			{TXI_WPNC_MEGA_1,TXI_WPNC_MEGA_2},
			{TXI_WPNC_FRAG_1,TXI_WPNC_FRAG_2},
			{TXI_WPNC_GUID_1,TXI_WPNC_GUID_2},
			{TXI_WPNC_NAPALMR_1,TXI_WPNC_NAPALMR_2},
			{TXI_WPNC_CYCLONE_1,TXI_WPNC_CYCLONE_2},
			{TXI_WPNC_BLKSHRK_1,TXI_WPNC_BLKSHRK_2},
			// The flares
				{TXI_WPNC_YELFLARE_1,TXI_WPNC_YELFLARE_2},
};

// Sets all weapons to unused
void InitWeapons()
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		Weapons[i].used = 0;
		Weapons[i].name[0] = 0;
	}
	Num_weapons = 0;
}

// Allocs a weapon for use, returns -1 if error, else index on success
int AllocWeapon()
{
	for (int i = MAX_STATIC_WEAPONS; i < MAX_WEAPONS; i++)
	{
		if (Weapons[i].used == 0)
		{
			memset(&Weapons[i], 0, sizeof(weapon));
			for (int t = 0; t < MAX_WEAPON_SOUNDS; t++)
				Weapons[i].sounds[t] = SOUND_NONE_INDEX;
			Weapons[i].alpha = 1.0;
			Weapons[i].spawn_handle = -1;
			Weapons[i].alternate_spawn_handle = -1;
			Weapons[i].icon_handle = -1;
			Weapons[i].scorch_handle = -1;
			Weapons[i].gravity_size = 1;
			Weapons[i].gravity_time = 1;
			Weapons[i].explode_size = 1;
			Weapons[i].explode_time = 1;
			Weapons[i].particle_count = 0;
			Weapons[i].particle_size = 1;
			Weapons[i].particle_life = 0;
			Weapons[i].particle_handle = -1;
			Weapons[i].robot_spawn_handle = -1;
			Weapons[i].alternate_chance = 0;
			Weapons[i].terrain_damage_size = 0;
			Weapons[i].terrain_damage_depth = 0;
			Weapons[i].used = 1;
			Weapons[i].recoil_force = 0.0f;
			Num_weapons++;
			return i;
		}
	}

	Int3();		 // No weapons free!
	return -1;
}

// Frees weapon index n and all associated images
void FreeWeapon(int n)
{
	ASSERT(Weapons[n].used > 0);

	Weapons[n].used = 0;
	Weapons[n].name[0] = 0;
	Num_weapons--;
}

// Gets next weapon from n that has actually been alloced
int GetNextWeapon(int n)
{
	int i;

	ASSERT(n >= 0 && n < MAX_WEAPONS);

	if (Num_weapons == 0)
		return -1;

	for (i = n + 1; i < MAX_WEAPONS; i++)
		if (Weapons[i].used)
			return i;
	for (i = 0; i < n; i++)
		if (Weapons[i].used)
			return i;

	// this is the only one

	return n;
}

// Gets previous weapon from n that has actually been alloced
int GetPrevWeapon(int n)
{
	int i;

	ASSERT(n >= 0 && n < MAX_WEAPONS);

	if (Num_weapons == 0)
		return -1;

	for (i = n - 1; i >= 0; i--)
	{
		if (Weapons[i].used)
			return i;
	}
	for (i = MAX_WEAPONS - 1; i > n; i--)
	{
		if (Weapons[i].used)
			return i;
	}

	// this is the only one
	return n;

}
// Searches thru all weapons for a specific name, returns -1 if not found
// or index of weapon with name
int FindWeaponName(char* name)
{
	int i;

	ASSERT(name != NULL);

	for (i = 0; i < MAX_WEAPONS; i++)
		if (Weapons[i].used && !stricmp(name, Weapons[i].name))
			return i;

	return -1;
}

// Given a weapon handle, returns that weapons bitmap
// If the weapon is animated, returns framenum mod num_of_frames in the animation
// Also figures in gametime
int GetWeaponHudImage(int handle, int framenum)
{
	if (Weapons[handle].flags & WF_HUD_ANIMATED)
	{
		float cur_frametime;
		int int_frame;

		vclip* vc = &GameVClips[Weapons[handle].hud_image_handle];
		ASSERT(vc->used);

		cur_frametime = Gametime / vc->frame_time;
		int_frame = cur_frametime;
		int_frame += framenum;

		return (vc->frames[int_frame % vc->num_frames]);
	}
	else
		return (Weapons[handle].hud_image_handle);
}

// Given a filename, loads either the bitmap or vclip found in that file.  If type
// is not NULL, sets it to 1 if file is animation, otherwise sets it to zero
int LoadWeaponHudImage(char* filename, int* type)
{
	int anim = 0, bm_handle;
	char extension[10];

	int len = strlen(filename);

	if (len < 4)
		return -1;

	strncpy(extension, &filename[len - 3], 5);

	if ((!strnicmp("oaf", extension, 3)) || (!strnicmp("ifl", extension, 3)) || (!strnicmp("abm", extension, 3)))
		anim = 1;

	if (type != NULL)
		*type = anim;

	if (anim)
		bm_handle = AllocLoadVClip(IGNORE_TABLE(filename), NOT_TEXTURE, 0);
	else
		bm_handle = bm_AllocLoadFileBitmap(IGNORE_TABLE(filename), 0);

	return bm_handle;
}


// Given a weapon handle, returns that weapons discharge bitmap 
int GetWeaponFireImage(int handle, int frame)
{
	if (Weapons[handle].flags & WF_IMAGE_VCLIP)
	{
		float cur_frametime;
		int int_frame;

		PageInVClip(Weapons[handle].fire_image_handle);

		vclip* vc = &GameVClips[Weapons[handle].fire_image_handle];
		ASSERT(vc->used >= 1);

		cur_frametime = Gametime / vc->frame_time;
		int_frame = cur_frametime;
		int_frame += frame;
		return (vc->frames[int_frame % vc->num_frames]);
	}
	else
		return (Weapons[handle].fire_image_handle);
}

// Given a filename, loads either the bitmap or model found in that file.  If type
// is not NULL, sets it to 1 if file is model, otherwise sets it to zero
int LoadWeaponFireImage(char* filename, int* type, int* anim, int pageable)
{
	int model = 0, bm_handle;
	int is_vclip = 0;
	char extension[10];

	int len = strlen(filename);

	if (len < 4)
		return -1;

	strncpy(extension, &filename[len - 3], 5);

	if ((!strnicmp("pof", extension, 3)) || (!strnicmp("oof", extension, 3)))
		model = 1;

	if (type != NULL)
		*type = model;
	if (anim != NULL)
		*anim = is_vclip;

	if (model)
		bm_handle = LoadPolyModel(IGNORE_TABLE(filename), 0);
	else
	{
		bm_handle = LoadTextureImage(IGNORE_TABLE(filename), anim, NOT_TEXTURE, 0);
	}

	return bm_handle;
}

// Given a weapon name, assigns that weapon to a specific index into
// the Weapons array.  Returns -1 if the named weapon is not found, 0 if the weapon
// is already in its place, or 1 if successfully moved
int MatchWeaponToIndex(char* name, int dest_index)
{
	ASSERT(dest_index >= 0 && dest_index < MAX_WEAPONS);

	int cur_index = FindWeaponName(name);
	int new_index;

	if (cur_index == -1)
	{
		// The weapon we are trying to find is not loaded or doesn't exist
		Int3();		// Get Jason or Matt
		return -1;
	}

	if (cur_index == dest_index)
		return 0;		// hurray, we're already matched up!

	if (Weapons[dest_index].used)
	{
		// This weapon is currently in use.  Copy the present data
		// into a new weapon and then assign our desired weapon to that index

		new_index = AllocWeapon();
		if (new_index >= 0) 	//DAJ -1FIX
			memcpy(&Weapons[new_index], &Weapons[dest_index], sizeof(weapon));

		// Now copy our new info over and free the old one
		memcpy(&Weapons[dest_index], &Weapons[cur_index], sizeof(weapon));
		FreeWeapon(cur_index);
	}
	else
	{
		// This slot is unused, so just take it

		Weapons[dest_index].used = 1;
		Num_weapons++;

		memcpy(&Weapons[dest_index], &Weapons[cur_index], sizeof(weapon));
		FreeWeapon(cur_index);
		return 0;
	}

	return new_index;		// we made it!
}

// Moves a weapon from a given index into a new one (above MAX_STATIC_WEAPONS)
// returns new index
int MoveWeaponFromIndex(int index)
{
	ASSERT(index >= 0 && index < MAX_STATIC_WEAPONS);
	ASSERT(Weapons[index].used);

	int new_index = AllocWeapon();
	memcpy(&Weapons[new_index], &Weapons[index], sizeof(weapon));
	FreeWeapon(index);

	return new_index;
}

// This is a very confusing function.  It takes all the weapons that we have loaded 
// and remaps then into their proper places (if they are static). 
void RemapWeapons()
{
	int limit = sizeof(Static_weapon_names) / sizeof(void*);
	int i;

	for (i = 0; i < MAX_STATIC_WEAPONS; i++)
	{
		if (Weapons[i].used)
		{
			int match = -1;
			for (int t = 0; t < limit; t++)
				if (!stricmp(Weapons[i].name, Static_weapon_names[t]))
				{
					match = t;
					break;

				}

			if (match == -1)		// this weapon is not supposed to be in the static area
			{
				int new_index;

				new_index = MoveWeaponFromIndex(i);
				RemapAllWeaponObjects(i, new_index);
			}
			else	// this weapon is a static weapon, make sure its in its place
			{
				if (i != match)		// its not where it belongs, move it
				{
					int new_index;

					new_index = MatchWeaponToIndex(Weapons[i].name, match);

					if (new_index != 0)
						RemapAllWeaponObjects(match, new_index);
					RemapAllWeaponObjects(i, match);
				}
			}
		}
	}

	for (i = MAX_STATIC_WEAPONS; i < MAX_WEAPONS; i++)
	{
		if (Weapons[i].used)
		{
			int match = -1;
			for (int t = 0; t < limit; t++)
				if (!stricmp(Weapons[i].name, Static_weapon_names[t]))
				{
					match = t;
					break;

				}

			if (match != -1)		// this is a static weapon that isn't supposed to be in the free area
			{

				int new_index;
				new_index = MatchWeaponToIndex(Weapons[i].name, match);
				if (new_index != 0)
					RemapAllWeaponObjects(match, new_index);
				RemapAllWeaponObjects(i, match);

			}
		}
	}

}

// goes thru every entity that could possible have a weapon index (ie objects, weapons, etc)
// and changes the old index to the new index
void RemapAllWeaponObjects(int old_index, int new_index)
{
	int i, j, k;

	// Remaps flying weapons
	for (i = 0; i < MAX_OBJECTS; i++)
	{

		if (Objects[i].type == OBJ_WEAPON)
		{
			if (Objects[i].id == old_index)
			{
				Objects[i].id = new_index;
			}
		}
	}

	// Remaps weapons contained by generic objects
	for (i = 0; i < MAX_OBJECT_IDS; i++)
	{
		if (Object_info[i].static_wb)
		{
			for (j = 0; j < MAX_WBS_PER_OBJ; j++)
			{
				for (k = 0; k < MAX_WB_GUNPOINTS; k++)
				{
					if (Object_info[i].static_wb[j].gp_weapon_index[k] == old_index)
					{
						Object_info[i].static_wb[j].gp_weapon_index[k] = new_index;
					}
				}
			}
		}
	}

	// Remaps weapons contained by Ships
	for (i = 0; i < MAX_SHIPS; i++)
	{
		for (j = 0; j < MAX_WBS_PER_OBJ; j++)
		{
			for (k = 0; k < MAX_WB_FIRING_MASKS; k++)
			{
				if (Ships[i].static_wb[j].gp_weapon_index[k] == old_index)
				{
					Ships[i].static_wb[j].gp_weapon_index[k] = new_index;
				}
			}
		}
	}

	// Do spawns
	for (i = 0; i < MAX_WEAPONS; i++)
	{
		if (!Weapons[i].used)
			continue;

		if (Weapons[i].spawn_handle == old_index)
			Weapons[i].spawn_handle = new_index;
		if (Weapons[i].alternate_spawn_handle == old_index)
			Weapons[i].alternate_spawn_handle = new_index;
	}

}

bool IsWeaponSecondary(int index)
{
	if (index < SECONDARY_INDEX) return false;
	else return true;
}

///////////////////////////////////////////////////////////////////////////
//	Weapon acquirement

//	unconditionally adds a weapon and ammo to a player.
int AddWeaponToPlayer(int slot, int weap_index, int ammo)
{
	bool select_new = false;

	LOGFILE((_logfp, "Adding weapon(%d,ammo=%d) to player.\n", weap_index, ammo));

	Players[slot].weapon_flags |= HAS_FLAG(weap_index);

	ship* ship = &Ships[Players[slot].ship_index];
	otype_wb_info* wb = &ship->static_wb[weap_index];
	ASSERT(wb != NULL);

	//if secondary or primary that uses ammo, then use the ammo
	if ((weap_index >= SECONDARY_INDEX) || wb->ammo_usage)
	{
		//figure out much ammo to add
		int added = min(ship->max_ammo[weap_index] - Players[slot].weapon_ammo[weap_index], ammo);

		//now add it
		Players[slot].weapon_ammo[weap_index] += (ushort)added;
	}

	if (slot == Player_num)
	{
		if (weap_index < SECONDARY_INDEX)
			select_new = AutoSelectWeapon(PW_PRIMARY, weap_index);
		else
			select_new = AutoSelectWeapon(PW_SECONDARY, weap_index);

		// this should be done in multisafe code now.
		//if (!select_new) {
		//	AddHUDMessage ("%s!",TXT(Static_weapon_names_msg[weap_index]));
		//}
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////
//	Weapon selection 

//	Note that we allocate five keys per category of weapon (primary, secondary)
//	We essentially select the weapon in slot passed.

#define  NUM_PRIMARY_SLOTS			5
#define	NUM_SECONDARY_SLOTS		5

//	This is NOT a mask of weapons available to the player.  This is a mask of what CLASS of 
//	weapon this slot is currently in.  The code below checks this mask to see if it should
//	select the higher class weapon in that slot when switching to that slot.
static ushort Weapon_slot_mask = 0;

void SelectPrimaryWeapon(int slot);
void SelectSecondaryWeapon(int slot);
void SetPrimaryWeapon(int index, int slot);
void SetSecondaryWeapon(int index, int slot);

inline bool is_weapon_available(unsigned player_weapon_flags, int new_weapon, ushort ammo = 0xffff)
{
	return ((player_weapon_flags & HAS_FLAG(new_weapon)) && ammo > 0) ? true : false;
}

// used for sequencing
void ResetWeaponSelectStates(ushort new_state)
{
	Weapon_slot_mask = new_state;
}

void SaveWeaponSelectStates(CFILE* fp)
{
	cf_WriteShort(fp, Weapon_slot_mask);
}

void LoadWeaponSelectStates(CFILE* fp)
{
	ushort state = (ushort)cf_ReadShort(fp);
	ResetWeaponSelectStates(state);
}

void SelectWeapon(int slot)
{
	if (Player_object->type != OBJ_PLAYER)
		return;		// This can happen when a player is dead and tries to select a weapon

	if (slot < NUM_PRIMARY_SLOTS)
		SelectPrimaryWeapon(slot);
	else
		SelectSecondaryWeapon(slot);
}

//	slot ranges 0-4
void SelectPrimaryWeapon(int slot)
{
	// get slot of currently selected weapon
	int oldslot;
	int nw_low, nw_high;
	unsigned avail_flags;
	player* plr = &Players[Player_num];

	avail_flags = plr->weapon_flags;
	oldslot = plr->weapon[PW_PRIMARY].index % NUM_PRIMARY_SLOTS;

	//	do selection.  if we are selecting the same slot of weapon, then we select to the next
	//	level of weapon.  when going from highest level, go to lowest
	if (oldslot == slot)
	{
		ushort nw_low = (plr->weapon[PW_PRIMARY].index + NUM_PRIMARY_SLOTS) % MAX_PRIMARY_WEAPONS;

		if (is_weapon_available(avail_flags, nw_low))
		{
			// toggle class of weapon in specified slot (save for selection)
			SetPrimaryWeapon(nw_low, slot);
		}
		else
		{
			AddHUDMessage(TXT_WPNNOTAVAIL);
			Sound_system.Play2dSound(SOUND_DO_NOT_HAVE_IT);

			ain_hear hear;
			hear.f_directly_player = true;
			hear.hostile_level = 0.1f;
			hear.curiosity_level = 0.5f;
			hear.max_dist = AI_SOUND_SHORT_DIST;
			AINotify(&Objects[Players[Player_num].objnum], AIN_HEAR_NOISE, (void*)&hear);
		}

		return;
	}
	else
	{
		//	we are selecting a new weapon slot.
		nw_low = slot % NUM_PRIMARY_SLOTS;
		nw_high = nw_low + NUM_PRIMARY_SLOTS;

		if (Game_mode & GM_MULTI)
		{
			int lowpriority = GetPrimaryPriority(nw_low);
			int highpriority = GetPrimaryPriority(nw_high);

			if (lowpriority > highpriority)
			{
				int heh = nw_low;
				nw_low = nw_high;
				nw_high = heh;
			}
		}

		if (Weapon_slot_mask & (1 << slot) || (Game_mode & GM_MULTI))
		{
			// we think we have a higher class of weapon.
			if (is_weapon_available(avail_flags, nw_high))
			{
				// if this slot had the higher class of weapon then check if we still have the higher class weapon.
				// toggle class of weapon in specified slot (save for selection)
				SetPrimaryWeapon(nw_high, slot);
			}
			else if (is_weapon_available(avail_flags, nw_low))
			{
				// check if we have the lower class.			
				// toggle class of weapon in specified slot (save for selection)
				SetPrimaryWeapon(nw_low, slot);
			}
			else
			{
				AddHUDMessage(TXT_WPNNOTAVAIL);
				Sound_system.Play2dSound(SOUND_DO_NOT_HAVE_IT);

				ain_hear hear;
				hear.f_directly_player = true;
				hear.hostile_level = 0.1f;
				hear.curiosity_level = 0.5f;
				hear.max_dist = AI_SOUND_SHORT_DIST;
				AINotify(&Objects[Players[Player_num].objnum], AIN_HEAR_NOISE, (void*)&hear);
			}
		}
		else
		{
			//	if this is a lower class of weapon, make sure this slot is flagged as having the lower version
			if (is_weapon_available(avail_flags, nw_low))
			{
				// if this slot had the higher class of weapon then check if we still have the higher class weapon.
				// toggle class of weapon in specified slot (save for selection)
				SetPrimaryWeapon(nw_low, slot);
			}
			else if (is_weapon_available(avail_flags, nw_high))
			{
				// check if we have the lower class.			
				// toggle class of weapon in specified slot (save for selection)
				SetPrimaryWeapon(nw_high, slot);
			}
			else
			{
				AddHUDMessage(TXT_WPNNOTAVAIL);
				Sound_system.Play2dSound(SOUND_DO_NOT_HAVE_IT);

				ain_hear hear;
				hear.f_directly_player = true;
				hear.hostile_level = 0.1f;
				hear.curiosity_level = 0.5f;
				hear.max_dist = AI_SOUND_SHORT_DIST;
				AINotify(&Objects[Players[Player_num].objnum], AIN_HEAR_NOISE, (void*)&hear);
			}
		}
	}
}

// slot ranges 5-9
void SelectSecondaryWeapon(int slot)
{
	// get slot of currently selected weapon
	int oldslot;
	int nw_low, nw_high;
	unsigned avail_flags;
	player* plr = &Players[Player_num];

	avail_flags = plr->weapon_flags;
	oldslot = (plr->weapon[PW_SECONDARY].index % NUM_SECONDARY_SLOTS) + NUM_PRIMARY_SLOTS;

	//	do selection.  if we are selecting the same slot of weapon, then we select to the next
	//	level of weapon.  when going from highest level, go to lowest
	if (oldslot == slot)
	{
		nw_low = SECONDARY_INDEX + ((plr->weapon[PW_SECONDARY].index + NUM_SECONDARY_SLOTS) % MAX_SECONDARY_WEAPONS);

		if (is_weapon_available(avail_flags, nw_low, plr->weapon_ammo[nw_low]))
		{
			// toggle class of weapon in specified slot (save for selection)
			SetSecondaryWeapon(nw_low, slot);
		}
		else
		{
			AddHUDMessage(TXT_WPNNOTAVAIL);
			Sound_system.Play2dSound(SOUND_DO_NOT_HAVE_IT);

			ain_hear hear;
			hear.f_directly_player = true;
			hear.hostile_level = 0.1f;
			hear.curiosity_level = 0.5f;
			hear.max_dist = AI_SOUND_SHORT_DIST;
			AINotify(&Objects[Players[Player_num].objnum], AIN_HEAR_NOISE, (void*)&hear);
		}
		return;
	}

	//	we are selecting a new weapon slot.
	nw_low = (slot % NUM_SECONDARY_SLOTS) + SECONDARY_INDEX;
	nw_high = nw_low + NUM_SECONDARY_SLOTS;

	if (Game_mode & GM_MULTI)
	{
		int lowpriority = GetSecondaryPriority(nw_low);
		int highpriority = GetSecondaryPriority(nw_high);

		if (lowpriority > highpriority)
		{
			int heh = nw_low;
			nw_low = nw_high;
			nw_high = heh;
		}
	}

	if (Weapon_slot_mask & (1 << slot) || (Game_mode & GM_MULTI))
	{
		// we think we have a higher class of weapon.
		if (is_weapon_available(avail_flags, nw_high, plr->weapon_ammo[nw_high]))
		{
			// if this slot had the higher class of weapon then check if we still have the higher class weapon.
			SetSecondaryWeapon(nw_high, slot);
		}
		else if (is_weapon_available(avail_flags, nw_low, plr->weapon_ammo[nw_low]))
		{
			SetSecondaryWeapon(nw_low, slot);
		}
		else
		{
			AddHUDMessage(TXT_WPNNOTAVAIL);
			Sound_system.Play2dSound(SOUND_DO_NOT_HAVE_IT);

			ain_hear hear;
			hear.f_directly_player = true;
			hear.hostile_level = 0.1f;
			hear.curiosity_level = 0.5f;
			hear.max_dist = AI_SOUND_SHORT_DIST;
			AINotify(&Objects[Players[Player_num].objnum], AIN_HEAR_NOISE, (void*)&hear);
		}
	}
	else
	{
		//	if this is a lower class of weapon, make sure this slot is flagged as having the lower version
		if (is_weapon_available(avail_flags, nw_low, plr->weapon_ammo[nw_low]))
		{
			// if this slot had the higher class of weapon then check if we still have the higher class weapon.
			SetSecondaryWeapon(nw_low, slot);
		}
		else if (is_weapon_available(avail_flags, nw_high, plr->weapon_ammo[nw_high]))
		{
			// check if we have the lower class.			
			SetSecondaryWeapon(nw_high, slot);
		}
		else
		{
			AddHUDMessage(TXT_WPNNOTAVAIL);
			Sound_system.Play2dSound(SOUND_DO_NOT_HAVE_IT);

			ain_hear hear;
			hear.f_directly_player = true;
			hear.hostile_level = 0.1f;
			hear.curiosity_level = 0.5f;
			hear.max_dist = AI_SOUND_SHORT_DIST;
			AINotify(&Objects[Players[Player_num].objnum], AIN_HEAR_NOISE, (void*)&hear);
		}
	}
}

void SetPrimaryWeapon(int index, int slot)
{
	dynamic_wb_info* p_dwb = &Player_object->dynamic_wb[index];

	if (index < NUM_PRIMARY_SLOTS)
		Weapon_slot_mask &= ~(1 << slot);
	else
		Weapon_slot_mask |= (1 << slot);

	if (index != Players[Player_num].weapon[PW_PRIMARY].index)
	{
		ClearPlayerFiring(Player_object, PW_PRIMARY);
		Players[Player_num].weapon[PW_PRIMARY].index = index;

		AddHUDMessage(TXT_WPNSELECT, TXT(Static_weapon_names_msg[index]));
		//	SetGaugeModified(GGF_PRIMARYLOAD, true);
		Sound_system.Play2dSound(SOUND_CHANGE_PRIMARY);

		ain_hear hear;
		hear.f_directly_player = true;
		hear.hostile_level = 0.2f;
		hear.curiosity_level = 0.5f;
		hear.max_dist = AI_SOUND_SHORT_DIST;
		AINotify(&Objects[Players[Player_num].objnum], AIN_HEAR_NOISE, (void*)&hear);

		// Stop any firing activity
		p_dwb->last_fire_time = Gametime;
		//	resets reticle to current weapon.
		ResetReticle();
	}

}


void SetSecondaryWeapon(int index, int slot)
{
	dynamic_wb_info* p_dwb = &Player_object->dynamic_wb[index];

	if (index < (NUM_SECONDARY_SLOTS + SECONDARY_INDEX))
		Weapon_slot_mask &= ~(1 << slot);
	else
		Weapon_slot_mask |= (1 << slot);

	//Stop any firing activity
	ClearPlayerFiring(Player_object, PW_SECONDARY);

	if (index != Players[Player_num].weapon[PW_SECONDARY].index)
	{
		Players[Player_num].weapon[PW_SECONDARY].index = index;

		AddHUDMessage(TXT_WPNSELECT, TXT(Static_weapon_names_msg[index]));
		//	SetGaugeModified(GGF_SECONDARYLOAD, true);
		Sound_system.Play2dSound(SOUND_CHANGE_SECONDARY);

		ain_hear hear;
		hear.f_directly_player = true;
		hear.hostile_level = 0.1f;
		hear.curiosity_level = 0.3f;
		hear.max_dist = AI_SOUND_SHORT_DIST;
		AINotify(&Objects[Players[Player_num].objnum], AIN_HEAR_NOISE, (void*)&hear);

		//Stop any firing activity
		ClearPlayerFiring(Player_object, PW_SECONDARY);
		p_dwb->last_fire_time = Gametime;
		//	resets reticle to current weapon.
		ResetReticle();
	}
}

///////////////////////////////////////////////////////////////////////////
//	Weapon AUTO selection 

const ushort	SELLIST_START = 0x7ffe,
SELLIST_END = 0x7fff;

static ushort PrimaryWpnSelectList[] =
{
	SELLIST_START,
	LASER_INDEX,
	VAUSS_INDEX,
	MICROWAVE_INDEX,
	PLASMA_INDEX,
	FUSION_INDEX,
	SUPER_LASER_INDEX,
	MASSDRIVER_INDEX,
	NAPALM_INDEX,
	EMD_INDEX,
	OMEGA_INDEX,
	SELLIST_END
};

static ushort SecondaryWpnSelectList[] =
{
	SELLIST_START,
	CONCUSSION_INDEX,
	HOMING_INDEX,
	IMPACTMORTAR_INDEX,
	SMART_INDEX,
	MEGA_INDEX,
	FRAG_INDEX,
	GUIDED_INDEX + WPNSEL_SKIP,
	NAPALMROCKET_INDEX,
	CYCLONE_INDEX,
	BLACKSHARK_INDEX,
	SELLIST_END
};


ushort GetAutoSelectPrimaryWpnIdx(int slot)
{
	int i = -1;

	while (PrimaryWpnSelectList[i + 1] != SELLIST_END)
	{
		if (slot == i)
			return PrimaryWpnSelectList[i + 1];
		i++;
	}

	return WPNSEL_INVALID;
}

ushort GetAutoSelectSecondaryWpnIdx(int slot)
{
	int i = -1;

	while (SecondaryWpnSelectList[i + 1] != SELLIST_END)
	{
		if (slot == i)
			return SecondaryWpnSelectList[i + 1];
		i++;
	}

	return WPNSEL_INVALID;
}

void SetAutoSelectPrimaryWpnIdx(int slot, ushort idx)
{
	if (slot < 0 || slot >= MAX_PRIMARY_WEAPONS)
		Int3();

	PrimaryWpnSelectList[slot + 1] = idx;
}

void SetAutoSelectSecondaryWpnIdx(int slot, ushort idx)
{
	if (slot < 0 || slot >= MAX_SECONDARY_WEAPONS)
		Int3();

	SecondaryWpnSelectList[slot + 1] = idx;
}

const ushort IWPNSEL_SKIP = (~WPNSEL_SKIP);

#define WPNINDEX(_index) (sel_list[(_index)]&IWPNSEL_SKIP)

int GetPrimaryPriority(int num)
{
	for (int i = 0; i < 10; i++)
	{
		int item = PrimaryWpnSelectList[i + 1] & IWPNSEL_SKIP;
		if (item == num)
		{
			if (PrimaryWpnSelectList[i + 1] & WPNSEL_SKIP)
				return i - 10;
			return i;
		}
	}
	return -100; //corrupt pilot file?
}

int GetSecondaryPriority(int num)
{
	for (int i = 0; i < 10; i++)
	{
		int item = SecondaryWpnSelectList[i + 1] & IWPNSEL_SKIP;
		if (item == num)
		{
			if (SecondaryWpnSelectList[i + 1] & WPNSEL_SKIP)
				return i - 10;
			return i;
		}
	}
	return -100; //corrupt pilot file?
}

// automatically switches weapon up to next level in autoselect order to this value. and type.
int SwitchPlayerWeapon(int weapon_type)
{
	player* plr;
	ship* ship;
	int new_index;
	void (*setwpnfunc)(int, int);			// Call either primary or secondary set weapon function
	ushort* sel_list;
	int plr_wpn_index;

	plr = &Players[Player_num];
	ship = &Ships[plr->ship_index];
	plr_wpn_index = plr->weapon[weapon_type].index;

	setwpnfunc = (weapon_type == PW_SECONDARY) ? SetSecondaryWeapon : SetPrimaryWeapon;
	sel_list = (weapon_type == PW_SECONDARY) ? SecondaryWpnSelectList : PrimaryWpnSelectList;

	new_index = 0;
	while (WPNINDEX(new_index) != plr_wpn_index && WPNINDEX(new_index) != SELLIST_END)
		new_index++;

	if (WPNINDEX(new_index) == SELLIST_END)
	{
		Int3();								// every weapon should be in list that's in the game. bad
		new_index = 1;						// 1st item after SELLIST_START
	}

	// select weapon if we can. find a weapon we can select, if we go back to start, return.
	//	THIS CODE IS SIMILAR BUT NOT THE SAME AS THE AUTO SELECT CODE.
	int old_index = new_index;
	new_index++;

	while (old_index != new_index)
	{
		if (WPNINDEX(new_index) == SELLIST_END)
		{
			new_index = 0;
		}
		else if (WPNINDEX(new_index) == SELLIST_START)
		{
			new_index++;
		}
		else
		{
			ushort wpn_index = WPNINDEX(new_index);
			otype_wb_info* wb = &ship->static_wb[wpn_index];
			int slot = (weapon_type == PW_SECONDARY) ? (((wpn_index - SECONDARY_INDEX) % NUM_SECONDARY_SLOTS) + NUM_PRIMARY_SLOTS) : (wpn_index % NUM_PRIMARY_SLOTS);

			//mprintf((0, "wpn_index = %d\n", wpn_index));

			if ((Players[Player_num].weapon_flags & HAS_FLAG(wpn_index)) && !(sel_list[new_index] & WPNSEL_SKIP))
			{
				if (wpn_index >= SECONDARY_INDEX && wb->ammo_usage && (wb->ammo_usage <= plr->weapon_ammo[wpn_index]))
				{
					//	we've found a weapon to select to that uses ammo!
					(*setwpnfunc)(wpn_index, slot);
					LOGFILE((_logfp, "ammo wpn: switch to new index %d\n", wpn_index));
					break;
				}
				else if (wpn_index < SECONDARY_INDEX && wb->ammo_usage && plr->weapon_ammo[wpn_index])
				{
					//	we've found a weapon to select to that uses ammo!
					(*setwpnfunc)(wpn_index, slot);
					LOGFILE((_logfp, "ammo wpn: switch to new index %d\n", wpn_index));
					break;
				}
				else if (!wb->ammo_usage && (plr->energy >= wb->energy_usage))
				{
					//	we've found an energy weapon to select to!
					(*setwpnfunc)(wpn_index, slot);
					LOGFILE((_logfp, "energy wpn:switch to new index %d\n", wpn_index));
					break;
				}
			}

			new_index++;
			//mprintf((0, "new_index = %d\n", new_index));
		}
	}

	return sel_list[new_index] & (~WPNSEL_SKIP);
}


//	Auto selects a weapon, usually the next best weapon.
//	step_up = true if we're going to next higher in order.
//	step_up = false if we're going to next lower in order.
bool AutoSelectWeapon(int weapon_type, int new_wpn)
{
	player* plr;
	ship* ship;
	ushort* sel_list;							// a weapon selection list
	int weapon_index;							// the current weapon index
	int list_index;							// index into a selection list
	int list_initial;							// initial index in list.
	int slot;
	bool sel_new_wpn = false;

	void (*setwpnfunc)(int, int);			// Call either primary or secondary set weapon function

	LOGFILE((_logfp, "Entering AutoSelect\n"));

	ASSERT((weapon_type == PW_PRIMARY) || (weapon_type == PW_SECONDARY));
	plr = &Players[Player_num];
	weapon_index = plr->weapon[weapon_type].index;
	ship = &Ships[plr->ship_index];

	//	choose primary or secondary list and select function
	sel_list = (weapon_type == PW_SECONDARY) ? SecondaryWpnSelectList : PrimaryWpnSelectList;
	setwpnfunc = (weapon_type == PW_SECONDARY) ? SetSecondaryWeapon : SetPrimaryWeapon;

	list_index = 0;
	list_initial = 0;
	while (WPNINDEX(list_index) != SELLIST_END)
	{
		if (WPNINDEX(list_index) == weapon_index)
			list_initial = list_index;
		list_index++;
	}

	list_index--;

	//	this code takes care of selecting a GIVEN new weapon over the existing.
	if (new_wpn > -1)
	{
		while (WPNINDEX(list_index) != SELLIST_START && WPNINDEX(list_index) != new_wpn)
			list_index--;
		if (!(sel_list[list_index] & WPNSEL_SKIP))
		{
			if (list_initial >= list_index)
			{
				ushort index = WPNINDEX(list_initial);
				otype_wb_info* wb = &ship->static_wb[index];
				if (index >= SECONDARY_INDEX && wb->ammo_usage && (wb->ammo_usage <= plr->weapon_ammo[index]))
				{
					LOGFILE((_logfp, "keep current ammo weapon...(ind=%d)\n", list_index));
					return sel_new_wpn;	// the current weapon supercedes the new weapon, (or is the same) so return
				}
				else if (index < SECONDARY_INDEX && wb->ammo_usage && plr->weapon_ammo[index])
				{
					LOGFILE((_logfp, "keep current ammo weapon...(ind=%d)\n", list_index));
					return sel_new_wpn;	// the current weapon supercedes the new weapon, (or is the same) so return
				}
				else if (!wb->ammo_usage && (plr->energy >= wb->energy_usage))
				{
					LOGFILE((_logfp, "keep current energy weapon...(ind=%d)\n", list_index));
					return sel_new_wpn;	// the current weapon supercedes the new weapon, (or is the same) so return
				}
				LOGFILE((_logfp, "tried to keep current weapon, but no ammo!...(ind=%d)\n", list_index));
			}
		}
		else
		{
			LOGFILE((_logfp, "tried to autoselect skipped weapon, will not do...(ind=%d)\n", list_index));
			return sel_new_wpn;
		}
	}

	while (1)
	{
		ushort index = sel_list[list_index];
		otype_wb_info* wb = &ship->static_wb[index];

		if (index == SELLIST_START)
		{
			break;
		}
		else
		{
			//	we have a real weapon coming up, lets see if we have it, and if we do, then do we have
			//	ammo
			if (!(index & WPNSEL_SKIP))
			{
				if (plr->weapon_flags & HAS_FLAG(index))
				{
					slot = (weapon_type == PW_SECONDARY) ? (((index - SECONDARY_INDEX) % NUM_SECONDARY_SLOTS) + NUM_PRIMARY_SLOTS) : (index % NUM_PRIMARY_SLOTS);
					if (new_wpn == -1)
					{
						//	if no new weapon, then select to next best weapon that can be used
						if (index >= SECONDARY_INDEX && wb->ammo_usage && (wb->ammo_usage <= plr->weapon_ammo[index]))
						{
							//	we've found a weapon to select to that uses ammo!
							LOGFILE((_logfp, "ammo wpn: auto select to new index %d\n", index));
							(*setwpnfunc)(index, slot);
							break;
						}
						else if (index <= SECONDARY_INDEX && wb->ammo_usage && plr->weapon_ammo[index])
						{
							//	we've found a weapon to select to that uses ammo!
							LOGFILE((_logfp, "ammo wpn: auto select to new index %d\n", index));
							(*setwpnfunc)(index, slot);
							break;
						}
						else if (!wb->ammo_usage && (plr->energy >= wb->energy_usage)) {
							//	we've found an energy weapon to select to!
							LOGFILE((_logfp, "energy wpn: auto select to new index %d\n", index));
							(*setwpnfunc)(index, slot);
							break;
						}
					}
					else
					{
						if (index < SECONDARY_INDEX || !wb->ammo_usage || (index >= SECONDARY_INDEX && wb->ammo_usage && (wb->ammo_usage <= plr->weapon_ammo[index])))
						{
							//	if new weapon, then always select that weapon (already assumes that new weapon is better.)
							LOGFILE((_logfp, "auto select to new index with new weapon!! %d\n", index));
							(*setwpnfunc)(index, slot);
							sel_new_wpn = true;
							break;
						}
					}
				}
			}
		}

		list_index--;
	}

	return sel_new_wpn;
}

// Draws an alpha blended polygon over the entire 3d rendering scene
// The r,g,b floats specify the color
void DrawAlphaBlendedScreen(float r, float g, float b, float alpha)
{
	g3Point* pntlist[4], points[4];
	ddgr_color	color;
	int i;

	color = GR_RGB(r * 255, g * 255, b * 255);

	// Set our four corners to cover the screen
	points[0].p3_sx = 0;
	points[0].p3_sy = 0;
	points[1].p3_sx = Game_window_w;
	points[1].p3_sy = 0;
	points[2].p3_sx = Game_window_w;
	points[2].p3_sy = Game_window_h;
	points[3].p3_sx = 0;
	points[3].p3_sy = Game_window_h;

	for (i = 0; i < 4; i++)
	{
		points[i].p3_z = 0;
		points[i].p3_flags = PF_PROJECTED;
		pntlist[i] = &points[i];
	}

	rend_SetZBufferState(0);
	rend_SetTextureType(TT_FLAT);
	rend_SetAlphaType(AT_CONSTANT);
	rend_SetAlphaValue(alpha * 255);
	rend_SetLighting(LS_NONE);
	rend_SetFlatColor(color);

	rend_DrawPolygon2D(0, pntlist, 4);
	rend_SetZBufferState(1);
}

// Retreives the weapon in the weapon array based off of an 'index' from 0-19 (non-mapped 
//	primary and secondaries)
weapon* GetWeaponFromIndex(int player, int index)
{
	ship* ship = &Ships[Players[player].ship_index];
	otype_wb_info* wb = &ship->static_wb[index];
	object* pobj = &Objects[Players[player].objnum];
	poly_model* pm = &Poly_models[pobj->rtype.pobj_info.model_num];
	dynamic_wb_info* dyn_wb = &pobj->dynamic_wb[index];
	int cur_m_bit;

	for (cur_m_bit = 0; cur_m_bit < pm->poly_wb[0].num_gps; cur_m_bit++)
	{
		if (wb->gp_fire_masks[dyn_wb->cur_firing_mask] & (0x01 << cur_m_bit))
		{
			return &Weapons[wb->gp_weapon_index[cur_m_bit]];
		}
	}

	return NULL;
}
