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
#include "descent.h"
#include "weapon.h"
#include "polymodel.h"
#include "room.h"
#include "object.h"
#include "terrain.h"
#include "player.h"
#include "hud.h"
#include "hlsoundlib.h"
#include "gameevent.h"
#include "polymodel.h"
#include "lighting.h"
#include "fireball.h"
#include <string.h>
#include <stdlib.h>
#include "findintersection.h"
#include "robotfire.h"
#include "AIMain.h"
#include "controls.h"
#include "damage.h"
#include "sounds.h"
#include "viseffect.h"
#include "vclip.h"
#include "SmallViews.h"
#include "hlsoundlib.h"
#include "soundload.h"
#include "stringtable.h"
#include "multi.h"
#include "game2dll.h"
#include "gameloop.h"
#include "collide.h"
#include "demofile.h"
#include "difficulty.h"
#include "D3ForceFeedback.h"
#include "config.h"
#include "ObjScript.h"
#include "doorway.h"
#include "psrand.h"
#include "BOA.h"
#include "vibeinterface.h"

bool AreObjectsAttached(const object* obj1, const object* obj2)
{
	const bool f_o1_a = (obj1->flags & OF_ATTACHED) != 0;
	const bool f_o2_a = (obj2->flags & OF_ATTACHED) != 0;

	if (f_o1_a || f_o2_a)
	{
		const int o1_uh = obj1->attach_ultimate_handle;
		const int o2_uh = obj2->attach_ultimate_handle;

		if ((f_o1_a) && ((o1_uh == obj2->handle) || (f_o2_a && (o1_uh == o2_uh))))
			return true;

		if ((f_o2_a) && (o2_uh == obj1->handle))
			return true;
	}

	return false;
}

bool ObjectsAreRelated(int o1, int o2)
{
	if ((o1 < 0) || (o2 < 0))
		return false;

	const object* obj1 = &Objects[o1];
	const object* obj2 = &Objects[o2];

	ASSERT(obj1->handle != OBJECT_HANDLE_NONE);
	ASSERT(obj2->handle != OBJECT_HANDLE_NONE);

	if (obj1->movement_type == MT_OBJ_LINKED || obj2->movement_type == MT_OBJ_LINKED)
		return true;

	if (obj1->type != OBJ_SHOCKWAVE && (obj1->mtype.phys_info.flags & PF_NO_COLLIDE))
	{
		return true;
	}

	if (obj2->type != OBJ_SHOCKWAVE && (obj2->mtype.phys_info.flags & PF_NO_COLLIDE))
	{
		return true;
	}

	if (((obj1->type == OBJ_PLAYER) && ((obj2->type == OBJ_ROBOT) && (obj2->id == GENOBJ_CHAFFCHUNK))) ||
		((obj2->type == OBJ_PLAYER) && ((obj1->type == OBJ_ROBOT) && (obj1->id == GENOBJ_CHAFFCHUNK))))
		return true;

	if (((obj1->type == OBJ_BUILDING) && (obj1->movement_type != MT_NONE) && (obj2->type == OBJ_POWERUP)) ||
		((obj2->type == OBJ_BUILDING) && (obj2->movement_type != MT_NONE) && (obj1->type == OBJ_POWERUP)))
	{
		return true;
	}

	if (obj1->type == OBJ_DOOR && DoorwayGetPosition(&Rooms[obj1->roomnum]) == 1.0f && obj2->type == OBJ_ROBOT)
		return true;

	if (obj2->type == OBJ_DOOR && DoorwayGetPosition(&Rooms[obj2->roomnum]) == 1.0f && obj1->type == OBJ_ROBOT)
		return true;

	if (AreObjectsAttached(obj1, obj2))
		return true;

	if (obj1->type != OBJ_WEAPON && obj2->type != OBJ_WEAPON)
	{
		if (((Gametime < obj1->creation_time + 3.0f) && obj1->parent_handle == obj2->handle) ||
			((Gametime < obj2->creation_time + 3.0f) && obj2->parent_handle == obj1->handle))
			return true;
		else
			return false;
	}

	if (obj1->type == OBJ_WEAPON && obj1->movement_type == MT_PHYSICS && (obj1->mtype.phys_info.flags & PF_PERSISTENT) &&
		obj1->ctype.laser_info.last_hit_handle == obj2->handle)
		return true;


	if (obj2->type == OBJ_WEAPON && obj2->movement_type == MT_PHYSICS && (obj2->mtype.phys_info.flags & PF_PERSISTENT) &&
		obj2->ctype.laser_info.last_hit_handle == obj1->handle)
		return true;

	// See if o2 is the parent of o1
	if (obj1->type == OBJ_WEAPON && (obj1->mtype.phys_info.flags & PF_NO_COLLIDE_PARENT))
	{
		if (obj1->parent_handle == obj2->handle)
			return true;

		object* t1 = ObjGet(obj1->parent_handle);

		if (t1)
		{
			if (AreObjectsAttached(obj2, t1))
				return true;
		}
	}

	// See if o1 is the parent of o2
	if (obj2->type == OBJ_WEAPON && (obj2->mtype.phys_info.flags & PF_NO_COLLIDE_PARENT))
	{
		if (obj2->parent_handle == obj1->handle)
			return true;

		object* t2 = ObjGet(obj2->parent_handle);

		if (t2)
		{
			if (AreObjectsAttached(obj1, t2))
				return true;
		}
	}

	// They must both be weapons
	if (obj1->type != OBJ_WEAPON || obj2->type != OBJ_WEAPON)
	{
		return false;
	}

	//	Here is the 09/07/94 change -- Siblings must be identical, others can hurt each other
	// See if they're siblings...
	if (obj1->parent_handle == obj2->parent_handle)
		if ((obj1->mtype.phys_info.flags & PF_HITS_SIBLINGS) || (obj2->mtype.phys_info.flags & PF_HITS_SIBLINGS))
			return false;		//if either is proximity, then can blow up, so say not related
		else
			return true;

	// Otherwise, it is two weapons and by default, they should not collide
	return true;
}

bool Enable_omega_collions = false;

// Picks out a target for an elecrical weapon to fire on
void AquireElectricalTarget(object* obj)
{
	int i;
	float closest_dist = 99999, dist;
	object* closest_obj = NULL, * hit_obj_ptr;


	if (obj->mtype.phys_info.flags & PF_HOMING)
	{
		for (i = 0; i <= Highest_object_index; i++)
		{
			hit_obj_ptr = &Objects[i];

			if ((hit_obj_ptr->render_type == RT_NONE) || !(hit_obj_ptr->type == OBJ_PLAYER || hit_obj_ptr->type == OBJ_ROBOT || (hit_obj_ptr->type == OBJ_BUILDING && hit_obj_ptr->ai_info)))
				continue;

			if (ObjectsAreRelated(OBJNUM(obj), i))
				continue;

			if (obj == hit_obj_ptr)
				continue;

			if (hit_obj_ptr->flags & OF_DEAD)
				continue;

			// The GB has an "electric shield" that works kind of like a Faraday's Cage (it also
			// make him less annoying as a player's own Omega will not track his GB)
			if (IS_GUIDEBOT(hit_obj_ptr))
				continue;

			// Make sure this target is within our view cone
			vector subvec = hit_obj_ptr->pos - obj->pos;
			vm_NormalizeVectorFast(&subvec);
			if (vm_DotProduct(&subvec, &obj->orient.fvec) < .8)
				continue;

			// The distance is actually the objects' centers minus some of the hit object's radius (I set it to 80%)
			if ((hit_obj_ptr->flags & OF_POLYGON_OBJECT) && hit_obj_ptr->type != OBJ_WEAPON && hit_obj_ptr->type != OBJ_POWERUP && hit_obj_ptr->type != OBJ_DEBRIS)
			{
				vector pos;
				pos = hit_obj_ptr->pos + hit_obj_ptr->wall_sphere_offset;

				dist = vm_VectorDistanceQuick(&pos, &obj->pos) - Poly_models[hit_obj_ptr->rtype.pobj_info.model_num].wall_size;
			}
			else
			{
				dist = vm_VectorDistanceQuick(&hit_obj_ptr->pos, &obj->pos) - hit_obj_ptr->size;
			}

			if (dist < 0.0f)
				dist = 0.0f;
			if (dist > 50)
				continue;

			if (dist <= closest_dist)
			{
				closest_dist = dist;
				closest_obj = hit_obj_ptr;
			}
		}
	}

	if (closest_obj != NULL)
	{
		float old_shield, new_shield;
		if (closest_obj->effect_info && closest_obj != Viewer_object)
		{
			closest_obj->effect_info->type_flags |= EF_DEFORM;
			closest_obj->effect_info->deform_time = 1.0;
			closest_obj->effect_info->deform_range = .03f;
		}

		obj->ctype.laser_info.track_handle = closest_obj->handle;

		vector subvec = obj->pos - closest_obj->pos;
		vm_NormalizeVectorFast(&subvec);

		old_shield = closest_obj->shields;

		vector save_pos = obj->pos;
		vector col_norm = { 0,1,0 };
		obj->pos = closest_obj->pos;

		Enable_omega_collions = true;
		collide_two_objects(obj, closest_obj, &closest_obj->pos, &col_norm);
		Enable_omega_collions = false;

		obj->pos = save_pos;
		new_shield = closest_obj->shields;

		object* parent_obj = ObjGet(obj->parent_handle);

		if (parent_obj->shields < 100)
		{
			float shields_to_add = (old_shield - new_shield) * .1;
			if (parent_obj->shields + shields_to_add > 100)
				shields_to_add = 100 - parent_obj->shields;

			if (Game_mode & GM_MULTI)
			{
				if (Netgame.local_role == LR_SERVER)
				{
					parent_obj->shields += shields_to_add;
					Multi_additional_damage[parent_obj->id] -= shields_to_add;
					Multi_additional_damage_type[parent_obj->id] = PD_ENERGY_WEAPON;


					DLLInfo.it_handle = -1;
					DLLInfo.me_handle = parent_obj->handle;
					DLLInfo.fParam = shields_to_add;
					CallGameDLL(EVT_GAMEOBJECTSHIELDSCHANGED, &DLLInfo);
				}
			}
			else
				parent_obj->shields += shields_to_add;
		}
	}
	else
	{
		obj->ctype.laser_info.track_handle = OBJECT_HANDLE_NONE;

		// Cast a ray to see what we've hit
		int fate;															// Collision type for response code
		fvi_info hit_info;												// Hit information
		fvi_query fq;														// Movement query
		vector dest = obj->pos + (obj->orient.fvec * 50.0);

		fq.p0 = &obj->pos;
		fq.startroom = obj->roomnum;
		fq.p1 = &dest;
		fq.rad = .0001f;
		//fq.thisobjnum			= Objects-obj;
		fq.thisobjnum = Objects - ObjGet(obj->parent_handle);
		fq.ignore_obj_list = NULL;
		fq.flags = FQ_CHECK_OBJS | FQ_IGNORE_POWERUPS | FQ_IGNORE_WEAPONS;

		fate = fvi_FindIntersection(&fq, &hit_info);

		vector save_pos = obj->pos;

		if (fate == HIT_WALL || fate == HIT_TERRAIN)
		{
			obj->pos = hit_info.hit_pnt;
			collide_object_with_wall(obj, 0, hit_info.hit_face_room[0], hit_info.hit_face[0], &hit_info.hit_face_pnt[0], &hit_info.hit_wallnorm[0], 0);
		}
		else if (fate == HIT_OBJECT || fate == HIT_SPHERE_2_POLY_OBJECT)
		{
			if (CollisionResult[obj->type][Objects[hit_info.hit_object[0]].type] != RESULT_NOTHING)
			{
				obj->pos = hit_info.hit_pnt;
				Enable_omega_collions = true;
				collide_two_objects(obj, &Objects[hit_info.hit_object[0]], &hit_info.hit_face_pnt[0], &hit_info.hit_wallnorm[0]);
				Enable_omega_collions = false;
			}
		}

		obj->pos = save_pos;
		obj->flags &= ~OF_DEAD;	// Bring it back from the dead in case the collide code killed it!
	}

}


// Creates an weapon and sends it speeding on its way
int CreateAndFireWeapon(vector* pos, vector* dir, object* parent, int weapon_num)
{
	int objnum;
	object* obj;
	int parentnum = parent - Objects;
	ubyte terrain = 0;


	ASSERT(Weapons[weapon_num].used);

	// This can happen in multiplayer -- not single player
	if (parent->flags & OF_DEAD)
		return-1;

	//Find out if it is o.k. to create a weapon here.
	fvi_query	fq;
	fvi_info		hit_data;
	int			fate;

	fq.p0 = &parent->pos;
	fq.startroom = parent->roomnum;
	fq.p1 = pos;
	fq.rad = 0;
	fq.thisobjnum = OBJNUM(parent);
	fq.ignore_obj_list = NULL;
	fq.flags = FQ_CHECK_OBJS | FQ_IGNORE_WEAPONS | FQ_IGNORE_POWERUPS | FQ_IGNORE_NON_LIGHTMAP_OBJECTS;					//what about trans walls???

	if (parent->type == OBJ_BUILDING)
	{
		fq.flags &= ~FQ_CHECK_OBJS;
	}

	fate = fvi_FindIntersection(&fq, &hit_data);
	if (fate != HIT_NONE) {
		mprintf((0, "Warning: Laser from parent=%d weapon point is in a wall or object, didn't fire!\n", OBJNUM(parent)));
		return -1;
	}

	objnum = ObjCreate(OBJ_WEAPON, weapon_num, hit_data.hit_room, pos, NULL, parent->handle);

	if (objnum < 0)
	{
		mprintf((1, "Can't create laser - Out of objects!\n"));
		return -1;
	}

	obj = &Objects[objnum];

	// Fill in laser-specific data
	obj->ctype.laser_info.multiplier = 1.0;
	obj->ctype.laser_info.parent_type = parent->type;

	if (parent->flags & OF_ATTACHED)
		obj->parent_handle = parent->attach_ultimate_handle;
	else
		obj->parent_handle = parent->handle;

	//	Assign parent type to highest level creator.  This propagates parent type down from
	//	the original creator through weapons which create children of their own (ie, smart missile)
	object* p = parent;
	while (p && (p->type == OBJ_WEAPON))
	{
		p = ObjGet(p->parent_handle);
		if (p)
		{
			obj->ctype.laser_info.parent_type = p->type;
			obj->parent_handle = p->handle;
		}
	}

	// Set direction
	vm_VectorToMatrix(&obj->orient, dir, &parent->orient.uvec, NULL);

	float scalar = 1.0f;
	if (!ObjGet(obj->parent_handle) || ObjGet(obj->parent_handle)->type != OBJ_PLAYER)
		scalar = (ObjGet(obj->parent_handle)->control_type == CT_AI && ((ObjGet(obj->parent_handle)->ai_info->flags & AIF_TEAM_MASK) == AIF_TEAM_REBEL)) ? 1.0f : Diff_ai_weapon_speed[DIFF_LEVEL];

	//	Set thrust 
	if (obj->mtype.phys_info.flags & PF_USES_THRUST)
	{
		vm_ScaleVector(&obj->mtype.phys_info.thrust, dir, obj->mtype.phys_info.full_thrust * scalar);
		// If this is a player, scale the thrust based on the players weapon_speed scalar
		if (parent->type == OBJ_PLAYER)
			obj->mtype.phys_info.thrust *= Players[parent->id].weapon_speed_scalar;
	}
	else
	{
		vm_MakeZero(&obj->mtype.phys_info.thrust);
		vm_MakeZero(&obj->mtype.phys_info.rotthrust);
	}
	//	vm_MakeZero(&obj->mtype.phys_info.rotvel);

		// Set the initial velocity
	vm_ScaleVector(&obj->mtype.phys_info.velocity, dir, Weapons[weapon_num].phys_info.velocity.z * scalar);

	// If this is a player, scale the velocity based on the players weapon_speed scalar
	if (parent->type == OBJ_PLAYER)
		obj->mtype.phys_info.velocity *= Players[parent->id].weapon_speed_scalar;

	// Set initial velocity to that of the firing object
	// Don't do it though if its a spawned weapon
	if ((obj->mtype.phys_info.flags & PF_USES_PARENT_VELOCITY) && parent->type != OBJ_WEAPON)
	{

		float fdot = (parent->mtype.phys_info.velocity * parent->orient.fvec);
		vector fvel;

		if (fdot > 0.0)
			fvel = parent->orient.fvec * fdot;
		else
			fvel = Zero_vector;

		vector rvel = 0.1f * parent->orient.rvec * (parent->mtype.phys_info.velocity * parent->orient.rvec);
		vector uvel = 0.1f * parent->orient.uvec * (parent->mtype.phys_info.velocity * parent->orient.uvec);

		obj->mtype.phys_info.velocity += fvel + rvel + uvel;
	}

	if (Weapons[obj->id].flags & WF_ELECTRICAL)
	{
		AquireElectricalTarget(obj);
	}
	else if (obj->movement_type == MT_PHYSICS)
	{
		if (!((obj->mtype.phys_info.flags & PF_USES_PARENT_VELOCITY) && (obj->mtype.phys_info.flags & PF_USES_THRUST)))
		{
			// For now, it only works on fixed velocity weapons
			if ((obj->mtype.phys_info.flags & PF_FIXED_VELOCITY) && !(obj->mtype.phys_info.flags & (PF_GRAVITY_MASK | PF_HOMING | PF_WIGGLE | PF_GUIDED)))
			{
				fvi_query fq;
				fvi_info	hit_info;
				vector end_pos = obj->pos + obj->mtype.phys_info.velocity * obj->lifeleft;

				fq.p0 = &obj->pos;
				fq.startroom = obj->roomnum;
				fq.p1 = &end_pos;
				fq.rad = obj->size;
				fq.thisobjnum = OBJNUM(obj);
				fq.ignore_obj_list = NULL;
				fq.flags = 0;

				fq.flags |= FQ_TRANSPOINT;

				fate = fvi_FindIntersection(&fq, &hit_info);

				if (fate != HIT_NONE && fate != HIT_OUT_OF_TERRAIN_BOUNDS)
				{
					float try_dist = vm_VectorDistance(&obj->pos, &end_pos);
					float got_dist = vm_VectorDistance(&obj->pos, &hit_info.hit_pnt);

					obj->ctype.laser_info.hit_pnt = hit_info.hit_pnt;
					obj->ctype.laser_info.hit_wall_pnt = hit_info.hit_face_pnt[0];
					obj->ctype.laser_info.hit_room = hit_info.hit_room;
					obj->ctype.laser_info.hit_pnt_room = hit_info.hit_face_room[0];
					obj->ctype.laser_info.hit_wall_normal = hit_info.hit_wallnorm[0];
					obj->ctype.laser_info.hit_face = hit_info.hit_face[0];

					obj->ctype.laser_info.hit_status = WPC_HIT_WALL;
					//					mprintf((0, "WPC: Hit r%d rw%d w%d (%f,%f,%f)\n", hit_info.hit_room, hit_info.hit_face_room[0], hit_info.hit_face[0], XYZ(&hit_info.hit_wallnorm[0])));
				}
				else
				{
					obj->ctype.laser_info.hit_status = WPC_NO_COLLISIONS;
					//					mprintf((0, "WPC: No collisions\n"));
				}
			}
		}
	}

	if (parent && (parent->type == OBJ_PLAYER || parent->type == OBJ_GHOST) && (parent->id == Player_num)) {
		//Do ForceFeeback for recoil
		DoForceForRecoil(parent, obj);
	}

	// For smoke effects
	obj->ctype.laser_info.last_smoke_pos = obj->pos - (obj->orient.fvec * (obj->size / 2));
	obj->ctype.laser_info.casts_light = true;
	return objnum;

}

// Steers a homing missile
void HomingTurnTowardObj(object* weapon, object* target)
{
	vector dir_to_target;
	vector movement = Zero_vector;

	if (target == NULL) return;

	if (target->movement_type == MT_PHYSICS || target->movement_type == MT_WALKING)
	{
		movement = 0.1f * (target->mtype.phys_info.velocity);
	}

	dir_to_target = target->pos + movement - weapon->pos;

	if (weapon->mtype.phys_info.rotdrag > 0.0f)
	{
		if (dir_to_target * weapon->orient.rvec > 0.0)
		{
			weapon->mtype.phys_info.rotthrust.y = weapon->mtype.phys_info.full_rotthrust;
		}
		else
		{
			weapon->mtype.phys_info.rotthrust.y = -weapon->mtype.phys_info.full_rotthrust;
		}

		if (dir_to_target * weapon->orient.uvec > 0.0)
		{
			weapon->mtype.phys_info.rotthrust.x = -weapon->mtype.phys_info.full_rotthrust;
		}
		else
		{
			weapon->mtype.phys_info.rotthrust.x = weapon->mtype.phys_info.full_rotthrust;
		}

		weapon->mtype.phys_info.rotthrust.z = 0.0;

		if (!ObjGet(weapon->parent_handle) || ObjGet(weapon->parent_handle)->type != OBJ_PLAYER)
			weapon->mtype.phys_info.rotthrust *= Diff_homing_strength[DIFF_LEVEL];
	}
	else
	{
		float scalar;
		vm_NormalizeVector(&dir_to_target);

		if (!ObjGet(weapon->parent_handle) || ObjGet(weapon->parent_handle)->type != OBJ_PLAYER)
			scalar = Diff_homing_strength[DIFF_LEVEL];
		else
			scalar = 1.0f;

		AITurnTowardsDir(weapon, &dir_to_target, weapon->mtype.phys_info.full_rotthrust * scalar);
		weapon->mtype.phys_info.velocity = weapon->orient.fvec * weapon->mtype.phys_info.full_thrust / weapon->mtype.phys_info.drag;
		weapon->mtype.phys_info.thrust = weapon->orient.fvec * weapon->mtype.phys_info.full_thrust;
	}
}

#define MAX_HOMING_DIST	500.0f
#define CLOSE_HOMING_DIST 15.0

#define MAX_HOMING_SOUND_TIME_DIST	250.00f
#define MAX_HOMING_SOUND_TIME         1.00f
#define MIN_HOMING_SOUND_TIME			  0.05f
#define HOMING_CHECK_VIS_INTERVAL	  0.25f
#define HOMING_FIND_LOC_INTERVAL		  0.20f

object* HomingAquireTarget(object* obj)
{
	object* track_goal = ObjGet(obj->ctype.laser_info.track_handle);
	object* weapon_parent = ObjGet(obj->parent_handle);

	// Forces its parent to itself if there is no parent
	if (weapon_parent == NULL)
	{
		weapon_parent = obj;
	}

	// Determine if the current track_goal is valid
	if (track_goal)
	{
		bool f_locked = false;

		vector to_target = track_goal->pos - obj->pos;
		float dist_to_target = vm_NormalizeVector(&to_target) - track_goal->size - obj->size;

		if (obj->type == OBJ_GHOST || obj->type == OBJ_DUMMY || (obj->type == OBJ_PLAYER && (Players[obj->id].flags & PLAYER_FLAGS_DEAD)))
		{
			f_locked = false;
		}
		else if (obj->effect_info && (obj->effect_info->type_flags & EF_CLOAKED))
		{
			f_locked = false;
		}
		else if (to_target * obj->orient.fvec > Weapons[obj->id].homing_fov)
		{
			if (track_goal == Player_object && Player_object->type == OBJ_PLAYER)
			{
				float sound_delta;
				float volume;

				if (dist_to_target > MAX_HOMING_SOUND_TIME_DIST)
				{
					sound_delta = MAX_HOMING_SOUND_TIME;
					volume = MAX_GAME_VOLUME / 2.0f;
				}
				else
				{
					float percent_near = dist_to_target / (float)MAX_HOMING_SOUND_TIME_DIST;

					sound_delta = MAX_HOMING_SOUND_TIME * percent_near;
					volume = MAX_GAME_VOLUME / 2.0f + MAX_GAME_VOLUME / 2.0 * (1.0f - percent_near);

					if (sound_delta < MIN_HOMING_SOUND_TIME)
					{
						sound_delta = MIN_HOMING_SOUND_TIME;
						volume = MAX_GAME_VOLUME;
					}
				}

				if (!(obj->type == OBJ_WEAPON && Weapons[obj->id].flags & WF_SILENT_HOMING) && !(obj->mtype.phys_info.flags & PF_GUIDED) && Players[Player_object->id].last_homing_warning_sound_time + sound_delta < Gametime)
				{
					fvi_query fq;
					fvi_info	hit_info;

					fq.p0 = &obj->pos;
					fq.startroom = obj->roomnum;
					fq.p1 = &track_goal->pos;
					fq.rad = 0.0f;
					fq.thisobjnum = OBJNUM(obj);
					fq.ignore_obj_list = NULL;
					fq.flags = /*FQ_TRANSPOINT | */FQ_CHECK_OBJS | FQ_ONLY_DOOR_OBJ | FQ_NO_RELINK;

					fvi_FindIntersection(&fq, &hit_info);

					// Only plays sound if there is a LOS between the player and the homing weapon
					if (hit_info.hit_type[0] == HIT_NONE)
					{
						Sound_system.Play2dSound(SOUND_MISSLE_LOCK, SND_PRIORITY_HIGHEST, volume);
						Players[Player_object->id].last_homing_warning_sound_time = Gametime;
					}


				}
			}

			if (dist_to_target < CLOSE_HOMING_DIST)
			{
				f_locked = true;
			}
			else if (dist_to_target < MAX_HOMING_DIST)
			{
				if (BOA_IsVisible(obj->roomnum, track_goal->roomnum))
				{
					if (Gametime > obj->ctype.laser_info.last_track_time + HOMING_CHECK_VIS_INTERVAL)
					{
						obj->ctype.laser_info.last_track_time = Gametime;
						fvi_query fq;
						fvi_info	hit_info;

						fq.p0 = &obj->pos;
						fq.startroom = obj->roomnum;
						fq.p1 = &track_goal->pos;
						fq.rad = 0.0f;
						fq.thisobjnum = OBJNUM(obj);
						fq.ignore_obj_list = NULL;
						fq.flags = /*FQ_TRANSPOINT | */FQ_CHECK_OBJS | FQ_ONLY_DOOR_OBJ | FQ_NO_RELINK;

						fvi_FindIntersection(&fq, &hit_info);
						if (hit_info.hit_type[0] == HIT_NONE)
						{
							f_locked = true;
						}
					}
					else
					{
						f_locked = true;
					}
				}
			}
		}

		if (!f_locked)
		{
			track_goal = NULL;
			obj->ctype.laser_info.track_handle = OBJECT_HANDLE_NONE;
		}
	}

	// Determine a track_goal if there is not one
	if (!track_goal)
	{
		int i;
		int best_index = -1;
		int best_dist = MAX_HOMING_DIST + 1;
		float best_dot = -1.0f;

		if (Gametime > obj->ctype.laser_info.last_track_time + HOMING_FIND_LOC_INTERVAL)
		{
			obj->ctype.laser_info.last_track_time = Gametime;

			for (i = 0; i <= Highest_object_index; i++)
			{
				if (BOA_IsVisible(obj->roomnum, Objects[i].roomnum))
				{
					if ((i != OBJNUM(weapon_parent)) && ((Objects[i].type == OBJ_BUILDING && Objects[i].ai_info) || (Objects[i].type == OBJ_ROBOT) || (Objects[i].type == OBJ_PLAYER)))
					{
						vector to_target = Objects[i].pos - obj->pos;

						if (Objects[i].effect_info && Objects[i].effect_info->type_flags & EF_CLOAKED)
							continue;

						if (weapon_parent && !AIObjEnemy(ObjGetUltimateParent(weapon_parent), ObjGetUltimateParent(&Objects[i])))
							continue;

						float dist_to_target = vm_NormalizeVector(&to_target) - obj->size - Objects[i].size;
						float cur_dot = to_target * obj->orient.fvec;

						if (cur_dot > Weapons[obj->id].homing_fov)
						{
							//Pick chaff over other objects
							if ((Objects[i].type == OBJ_ROBOT) && (Objects[i].id == GENOBJ_CHAFFCHUNK))
							{
								// Make multiplayer safe!
								if (Game_mode & GM_MULTI)
								{
									if (Netgame.local_role == LR_CLIENT)
									{
										if (!(Local_object_list[i] & 0x3))
										{
											best_index = i;
											break;
										}
									}
									else
									{
										if (!(i & 0x3))
										{
											best_index = i;
											break;
										}
									}
								}
								else
								{
									if (ps_rand() < RAND_MAX / 4)
									{		// 1/4 chance of picking chaff
										best_index = i;
										break;
									}
								}
							}
							else if (dist_to_target < CLOSE_HOMING_DIST)
							{
								best_index = i;
								break;
							}
							else if (dist_to_target < MAX_HOMING_DIST)
							{
								if (cur_dot > best_dot)
								{

									fvi_query fq;
									fvi_info	hit_info;

									fq.p0 = &obj->pos;
									fq.startroom = obj->roomnum;
									fq.p1 = &Objects[i].pos;
									fq.rad = 0.0f;
									fq.thisobjnum = OBJNUM(obj);
									fq.ignore_obj_list = NULL;
									fq.flags = FQ_TRANSPOINT | FQ_CHECK_OBJS | FQ_ONLY_DOOR_OBJ | FQ_NO_RELINK;

									fvi_FindIntersection(&fq, &hit_info);
									if (hit_info.hit_type[0] == HIT_NONE)
									{
										best_index = i;
										best_dist = dist_to_target;
										best_dot = cur_dot;
									}
								}
							}
						}
					}
				}
			}
			if (best_index >= 0)
			{
				track_goal = &Objects[best_index];
				obj->ctype.laser_info.track_handle = Objects[best_index].handle;
			}
		}
	}

	return track_goal;
}

// Does homing code
void HomingDoFrame(object* obj)
{
	obj->mtype.phys_info.rotthrust = Zero_vector;

	HomingTurnTowardObj(obj, HomingAquireTarget(obj));
}

void WeaponDoFrame(object* obj)
{
	bool draw_effects = 1;

	if (!Detail_settings.Weapon_coronas_enabled)
		draw_effects = 0;

	if (MAX_WEAPON_NOT_HIT_PARENT_TIME + obj->creation_time < Gametime)
		obj->mtype.phys_info.flags &= (~PF_NO_COLLIDE_PARENT);


	if ((obj->mtype.phys_info.flags & PF_USES_THRUST) && (obj->ctype.laser_info.thrust_left >= 0.0))
	{
		obj->ctype.laser_info.thrust_left -= Frametime;

		if ((obj->creation_time + 0.1f < Gametime) && (obj->mtype.phys_info.flags & PF_HOMING) && !(obj->mtype.phys_info.flags & PF_GUIDED))
		{
			obj->ctype.laser_info.last_track_time = Gametime - HOMING_CHECK_VIS_INTERVAL;
			HomingDoFrame(obj);
		}

		if (obj->ctype.laser_info.thrust_left <= 0.0)
		{
			obj->mtype.phys_info.flags &= (~PF_USES_THRUST);
			vm_MakeZero(&obj->mtype.phys_info.thrust);
			vm_MakeZero(&obj->mtype.phys_info.rotthrust);

			obj->mtype.phys_info.drag = 0.005f;
			obj->mtype.phys_info.rotdrag = 0.005f;

			obj->mtype.phys_info.flags |= PF_GRAVITY;
		}
	}

	if ((Weapons[obj->id].flags & WF_SMOKE) && draw_effects)
	{
		if (Weapons[obj->id].flags & WF_PLANAR_SMOKE)
		{
			vector newpos = obj->pos - (obj->orient.fvec * (obj->size / 2));
			int visnum = VisEffectCreate(VIS_FIREBALL, BILLBOARD_SMOKETRAIL_INDEX, obj->roomnum, &newpos);
			if (visnum >= 0)
			{
				vis_effect* vis = &VisEffects[visnum];
				vis->custom_handle = Weapons[obj->id].smoke_handle;
				vis->lifeleft = 1.0;
				vis->lifetime = 1.0;

				vis->end_pos = obj->ctype.laser_info.last_smoke_pos;
				vis->billboard_info.width = 4;
				vis->lighting_color = GR_RGB16(Weapons[obj->id].lighting_info.red_light1 * 255.0, Weapons[obj->id].lighting_info.green_light1 * 255.0, Weapons[obj->id].lighting_info.blue_light1 * 255.0);
				obj->ctype.laser_info.last_smoke_pos = vis->pos;
			}
		}
		else
		{
			// Create blobby smoke
			// Create extras
			vector blobpos = obj->pos - (obj->orient.fvec * obj->size);

			float delta_time = Frametime;
			vector delta_vec = obj->mtype.phys_info.velocity * delta_time;

			float mag = vm_GetMagnitudeFast(&delta_vec);
			float extras = (mag * 4) + .5;
			int int_extras = extras + 1;

			int_extras = min(int_extras, 2);

			delta_vec /= int_extras;
			delta_time /= int_extras;

			vector new_blobpos = blobpos;

			for (int i = 0; i < int_extras; i++)
			{
				int visnum = CreateFireball(&new_blobpos, SMOKE_TRAIL_INDEX, obj->roomnum, VISUAL_FIREBALL);

				if (visnum >= 0)
				{
					vis_effect* vis = &VisEffects[visnum];

					vis->custom_handle = Weapons[obj->id].smoke_handle;
					vis->lifeleft -= (i * delta_time);
					vis->creation_time -= (i * delta_time);

					if (Weapons[obj->id].flags & WF_REVERSE_SMOKE)
						vis->flags |= VF_REVERSE;
				}

				new_blobpos -= delta_vec;
			}
		}
	}

	if (Weapons[obj->id].particle_count > 0 && Weapons[obj->id].particle_handle != -1 && draw_effects)
	{
		vector pos = obj->pos - (obj->orient.fvec * obj->size);
		weapon* weap = &Weapons[obj->id];
		float particle_interval = (1.0 / (float)weap->particle_count);

		if (Gametime - obj->ctype.laser_info.last_drop_time > particle_interval)
		{
			CreateRandomParticles(1, &pos, obj->roomnum, weap->particle_handle, weap->particle_size, weap->particle_life);
			obj->ctype.laser_info.last_drop_time = Gametime;
		}

	}


	if (Weapons[obj->id].flags & WF_RING)
	{
		float norm = ((Gametime - obj->creation_time) / obj->lifetime);
		obj->size = Weapons[obj->id].size + (norm * Weapons[obj->id].size * 8);
	}

}

bool WeaponCalcGun(vector* gun_point, vector* gun_normal, object* obj, int gun_num)
{
	poly_model* pm;
	vector pnt;
	vector normal;
	matrix m;
	int mn;				//submodel number
	float normalized_time[MAX_SUBOBJECTS];
	bool f_good_gp = true;

	if (!(obj->flags & OF_POLYGON_OBJECT))
	{
		// mprintf ((0,"Object type %d is not a polyobj!\n",obj->type));

		if (gun_point)
			*gun_point = obj->pos;
		if (gun_normal)
			*gun_normal = obj->orient.fvec;

		return false;
	}

	pm = &Poly_models[obj->rtype.pobj_info.model_num];

	if (pm->n_guns == 0)
	{
		mprintf((0, "WARNING: Object with no weapons is firing.\n", gun_num));

		if (gun_point)
			*gun_point = obj->pos;
		if (gun_normal)
			*gun_normal = obj->orient.fvec;

		return false;
	}

	SetNormalizedTimeObj(obj, normalized_time);
	SetModelAnglesAndPos(pm, normalized_time);

	if (gun_num < 0 || gun_num >= pm->n_guns)
	{
		mprintf((0, "WARNING: Bashing gun num %d to 0.\n", gun_num));
		if (gun_point)
			*gun_point = obj->pos;
		if (gun_normal)
			*gun_normal = obj->orient.fvec;
		return false;
	}

	pnt = pm->gun_slots[gun_num].pnt;
	normal = pm->gun_slots[gun_num].norm;
	mn = pm->gun_slots[gun_num].parent;

	// Instance up the tree for this gun
	while (mn != -1)
	{
		vector tpnt;

		vm_AnglesToMatrix(&m, pm->submodel[mn].angs.p, pm->submodel[mn].angs.h, pm->submodel[mn].angs.b);
		vm_TransposeMatrix(&m);

		if (gun_point)
			tpnt = pnt * m;

		if (gun_normal)
			normal = normal * m;

		if (gun_point)
			pnt = tpnt + pm->submodel[mn].offset + pm->submodel[mn].mod_pos;

		mn = pm->submodel[mn].parent;
	}

	m = obj->orient;
	vm_TransposeMatrix(&m);

	if (gun_point)
		*gun_point = pnt * m;
	if (gun_normal)
		*gun_normal = normal * m;

	if (gun_point)
		*gun_point += obj->pos;

	return f_good_gp;
}

#define MIN_FIRE_SPREAD 5000
#define MAX_FIRE_SPREAD	16768  // We vary by difficulty level!!!!!!!!  Its the total fire cone (half each way)

// Given an object and a weapon, fires a shot from that object
int FireWeaponFromObject(object* obj, int weapon_num, int gun_num, bool f_force_forward, bool f_force_target)
{
	ASSERT(Weapons[weapon_num].used);



	// Mass driver is hack
	static int mass_driver_id = -2;
	static int mass_driver_ring = -2;

	if (mass_driver_id == -2)
	{
		mass_driver_id = FindWeaponName("Mass Driver");
		mass_driver_ring = FindTextureName("CloakRing.tga1");
	}

	vector	    laser_pos, laser_dir;
	int			objnum;

	if (gun_num == -1)
	{
		laser_pos = obj->pos;
		laser_dir = obj->orient.fvec;
	}
	else
	{
		// Find the initial position of the laser
		poly_model* pm = &Poly_models[obj->rtype.pobj_info.model_num];
		//		vector *pnt = &pm->gun_slots[gun_num].pnt;
		//		vector gun_point;
		//		matrix m;

		if (gun_num >= pm->n_guns)
		{
			// We don't have a gun point for this gun!
			mprintf((0, "Trying to fire from gun %d...we don't have that gun!\n", gun_num));
			laser_pos = obj->pos;
			laser_dir = obj->orient.fvec;
		}
		else
		{
			if (f_force_forward)
			{
				WeaponCalcGun(&laser_pos, NULL, obj, gun_num);
				laser_dir = obj->orient.fvec;
			}
			else if (f_force_target && obj->ai_info && obj->ai_info->vec_to_target_perceived != Zero_vector)
			{
				WeaponCalcGun(&laser_pos, NULL, obj, gun_num);
				laser_dir = obj->ai_info->vec_to_target_perceived;
			}
			else
			{
				WeaponCalcGun(&laser_pos, &laser_dir, obj, gun_num);
			}
		}
	}

	if ((obj->control_type == CT_AI) && (obj->ai_info->fire_spread || (Diff_ai_min_fire_spread[DIFF_LEVEL])))
	{
		matrix rot_mat;
		angle p, h, b;

		float diff_scale = DIFF_LEVEL / (MAX_DIFFICULTY_LEVELS - 1);
		float fs = (MAX_FIRE_SPREAD * (1.0f - diff_scale)) + (MIN_FIRE_SPREAD * diff_scale);

		short fire_spread = obj->ai_info->fire_spread * fs;

		if ((obj->control_type == CT_AI && ((obj->ai_info->flags & AIF_TEAM_MASK) != AIF_TEAM_REBEL)) && fire_spread < Diff_ai_min_fire_spread[DIFF_LEVEL] * fs)
		{
			fire_spread = Diff_ai_min_fire_spread[DIFF_LEVEL] * fs;
		}

		short half_fire_spread = fire_spread >> 1;

		if (fire_spread > 0) {
			p = (ps_rand() % fire_spread) - half_fire_spread;
			h = (ps_rand() % fire_spread) - half_fire_spread;
			b = (ps_rand() % fire_spread) - half_fire_spread;

			vm_AnglesToMatrix(&rot_mat, p, h, b);

			laser_dir = laser_dir * rot_mat;
		}
	}

	objnum = CreateAndFireWeapon(&laser_pos, &laser_dir, obj, weapon_num);
	if (Game_mode & GM_MULTI)
	{
		if (Netgame.local_role == LR_SERVER)
		{
			if (obj->control_type == CT_AI)
			{
				MultiSendRobotFireWeapon(obj->handle & HANDLE_OBJNUM_MASK, &laser_pos, &laser_dir, (unsigned short)weapon_num);
			}
		}
	}
	if (Demo_flags == DF_RECORDING)
	{
		DemoWriteWeaponFire(obj->handle & HANDLE_OBJNUM_MASK, &laser_pos, &laser_dir, (unsigned short)weapon_num, objnum, gun_num);
	}

	// bail out of their was an error creating this weapon object
	if (objnum == -1)
		return -1;

	// Record gun number
	Objects[objnum].ctype.laser_info.src_gun_num = gun_num;

	AINotify(&Objects[objnum], AIN_OBJ_FIRED, obj);

	if (obj->type == OBJ_PLAYER && obj == Player_object)
	{
		//Check if missile camera enabled, and if this weapon allows it
		if ((Missile_camera_window != -1) && (Weapons[weapon_num].flags & WF_ENABLE_CAMERA)) {

			//Check if there's already a missile camera
			object* viewer_obj = ObjGet(GetSmallViewer(Missile_camera_window));
			if ((viewer_obj == NULL) || (viewer_obj->type != OBJ_WEAPON))
				CreateSmallView(Missile_camera_window, Objects[objnum].handle, SVF_POPUP + SVF_BIGGER);
		}
	}

	//	if (Weapons[weapon_num].sounds[WSI_FIRE]!=SOUND_NONE_INDEX)
	//		Sound_system.Play3dSound(Weapons[weapon_num].sounds[WSI_FIRE], &Objects[objnum]);

	if (Weapons[weapon_num].sounds[WSI_FLYING] != SOUND_NONE_INDEX)
		Sound_system.Play3dSound(Weapons[weapon_num].sounds[WSI_FLYING], SND_PRIORITY_HIGHEST, &Objects[objnum]);

	// Do a muzzle flash from this gun
	if (Weapons[weapon_num].flags & WF_MUZZLE)
	{
		int visnum;
		vector newpos;

		if (obj == &Objects[Players[Player_num].objnum])
		{
			newpos = laser_pos + (laser_dir);
			visnum = VisEffectCreate(VIS_FIREBALL, MUZZLE_FLASH_INDEX, obj->roomnum, &newpos);
		}
		else
		{
			newpos = laser_pos + (laser_dir / 2);
			visnum = VisEffectCreate(VIS_FIREBALL, MUZZLE_FLASH_INDEX, obj->roomnum, &newpos);
		}

		// Make this guy live for a very short time
		if (visnum >= 0)
		{
			vis_effect* vis = &VisEffects[visnum];

			vis->lifetime = .08f;
			vis->lifeleft = .08f;
			vis->size = 1.0f;

			vis->movement_type = MT_PHYSICS;
			vis->velocity = obj->mtype.phys_info.velocity;

			// Make some smoke!
			visnum = VisEffectCreate(VIS_FIREBALL, MED_SMOKE_INDEX, obj->roomnum, &newpos);
			if (visnum >= 0)
			{
				vis_effect* vis = &VisEffects[visnum];

				vis->lifetime = .3f;
				vis->lifeleft = .3f;

				vis->movement_type = MT_PHYSICS;
				vis->velocity = obj->mtype.phys_info.velocity;
				vis->velocity.y += 10;
			}

		}
	}

	// Check for mass driver
	if (weapon_num == mass_driver_id)
	{
		// Create a fire trail for this weapon
		// find out where this weapon hits
		fvi_query	fq;
		fvi_info		hit_data;

		vector dest_vec = laser_pos + (laser_dir * 5000);

		fq.p0 = &laser_pos;
		fq.startroom = obj->roomnum;
		fq.p1 = &dest_vec;
		fq.rad = 0;
		fq.thisobjnum = -1;
		fq.ignore_obj_list = NULL;
		fq.flags = FQ_CHECK_OBJS | FQ_IGNORE_POWERUPS | FQ_IGNORE_WEAPONS;

		fvi_FindIntersection(&fq, &hit_data);

		float mag = vm_VectorDistanceQuick(&hit_data.hit_pnt, &obj->pos);

		int visnum;

		if (obj == NULL || obj->type != OBJ_BUILDING || stricmp(Object_info[obj->id].name, "FinalbossLITTLETIT") != 0)
			visnum = VisEffectCreate(VIS_FIREBALL, MASSDRIVER_EFFECT_INDEX, obj->roomnum, &laser_pos);
		else
			visnum = VisEffectCreate(VIS_FIREBALL, MERCBOSS_MASSDRIVER_EFFECT_INDEX, obj->roomnum, &laser_pos);

		if (visnum >= 0)
		{
			vis_effect* vis = &VisEffects[visnum];
			vis->custom_handle = mass_driver_ring;
			vis->lifeleft = 1.5;
			vis->lifetime = 1.5;

			vis->end_pos = hit_data.hit_pnt;
			vis->billboard_info.width = 1;
			if (obj == NULL || obj->type != OBJ_BUILDING || stricmp(Object_info[obj->id].name, "FinalbossLITTLETIT") != 0)
				vis->lighting_color = GR_RGB16(100, 100, 170);
			else
				vis->lighting_color = GR_RGB16(240, 10, 10);
			vis->flags |= VF_LINK_TO_VIEWER | VF_EXPAND;
			vis->size = mag;
		}
	}

	if (Game_mode & GM_MULTI)
	{
		DLLInfo.me_handle = Objects[objnum].handle;
		DLLInfo.it_handle = obj->handle;
		CallGameDLL(EVT_WEAPONFIRED, &DLLInfo);
	}


	return objnum;
}

// This define determines how wide this arc is
#define ELECTRICAL_WIDTH		.3f
#define ELECTRICAL_ALPHA		.2

#include "args.h"

// Draws a lighting like chain of electricity
void DrawElectricalWeapon(object* obj)
{
	int i, t;
	vector center_vecs[20];
	vector arc_vec;
	g3Point arc_points[100];
	vector dest_vector, src_vector, line_norm;
	float line_mag;
	g3Point* pntlist[20];
	vector dir;
	object* parent_obj = ObjGet(obj->parent_handle);
	float view_dp = 0;

	if (!parent_obj)
		return;

	if (parent_obj->type == OBJ_PLAYER)
	{
		if (Players[parent_obj->id].flags & (PLAYER_FLAGS_DYING | PLAYER_FLAGS_DEAD))
			return;
	}
	else
	{
		if (parent_obj->flags & OF_DYING)
			return;
	}

	int bm_handle = GetWeaponFireImage(obj->id, 0);
	ASSERT(bm_handle >= 0);

	WeaponCalcGun(&src_vector, &dir, parent_obj, obj->ctype.laser_info.src_gun_num);

	if (obj->ctype.laser_info.track_handle == OBJECT_HANDLE_NONE || ObjGet(obj->ctype.laser_info.track_handle) == NULL)
	{
		dest_vector = src_vector + (obj->orient.fvec * 50);
	}
	else
	{
		object* obj_to_track = ObjGet(obj->ctype.laser_info.track_handle);
		dest_vector = obj_to_track->pos;
		if (obj_to_track == Player_object)
		{
			vector subvec = src_vector - dest_vector;
			float mag = vm_GetMagnitudeFast(&subvec);

			if (mag < .5)
			{
				obj->lifeleft = 0;
				return;
			}

			subvec /= mag;

			dest_vector += subvec;	// This prevents rendering errors
		}
	}


	//Find out if it is o.k. to create a weapon here.
	fvi_query	fq;
	fvi_info		hit_data;
	int			fate;

	fq.p0 = &src_vector;
	fq.startroom = obj->roomnum;
	fq.p1 = &dest_vector;
	fq.rad = 0;
	fq.thisobjnum = -1;
	fq.ignore_obj_list = NULL;
	fq.flags = 0;					//what about trans walls???

	fate = fvi_FindIntersection(&fq, &hit_data);

	if (fate == HIT_WALL || fate == HIT_TERRAIN)
	{
		dest_vector = hit_data.hit_pnt;
		CreateRandomSparks((ps_rand() % 4), &dest_vector, obj->roomnum, COOL_SPARK_INDEX);
	}

	line_norm = dest_vector - src_vector;
	line_mag = vm_GetMagnitudeFast(&line_norm);

	if (line_mag < .5)
	{
		obj->lifeleft = 0;
		return;
	}


	line_norm /= line_mag;

	// Get view dot product for drawing blobs

	if (parent_obj != Viewer_object)
	{
		vector temp_line_norm = -line_norm;
		view_dp = Viewer_object->orient.fvec * temp_line_norm;
	}

	matrix mat;

	vm_VectorToMatrix(&mat, &line_norm, NULL, NULL);

	// Create the center points of our chain
	int num_segments = 20;
	int circle_pieces = 5;

	if (!Detail_settings.Weapon_coronas_enabled)
		num_segments = 10;

	float line_scalar = (line_mag / (float)num_segments);
	line_norm *= line_scalar;

	// Get the src and dest vectors in view space so we know how to compute z

	center_vecs[0] = src_vector;	// Set first one equal to our origin
	center_vecs[num_segments - 1] = dest_vector;
	vector from = src_vector;
	int cur_sin = FrameCount * 5000;

	for (i = 1; i < num_segments - 1; i++, from += line_norm, cur_sin += 8000)
	{
		center_vecs[i] = from;
		center_vecs[i] += (mat.uvec * FixSin(cur_sin % 65536) * .4f);
		center_vecs[i] += (mat.rvec * FixCos(cur_sin % 65536) * .4f);
	}

	// Now that we have our center points, create a tube
	for (i = 0; i < num_segments; i++)
	{
		float size = ELECTRICAL_WIDTH;

		for (t = 0; t < circle_pieces; t++)
		{
			float norm = (float)t / (float)circle_pieces;
			arc_vec = center_vecs[i];

			if (i != num_segments - 1)
			{
				arc_vec += (obj->orient.uvec * size * FixSin(norm * 65536));
				arc_vec -= (obj->orient.rvec * size * FixCos(norm * 65536));
			}

			g3_RotatePoint(&arc_points[i * circle_pieces + t], &arc_vec);
			arc_points[i * circle_pieces + t].p3_flags |= PF_UV;
		}
	}

	rend_SetAlphaType(AT_SATURATE_TEXTURE);
	rend_SetAlphaValue(ELECTRICAL_ALPHA * 255);
	rend_SetOverlayType(OT_NONE);
	rend_SetLighting(LS_NONE);
	rend_SetZBufferWriteMask(0);

	float float_time = (Gametime * 300);
	int int_time = float_time;
	int_time %= 100;
	float vchange = int_time / 100.0;

	//Check for no sat flag
	bool sat = true;
	if (FindArg("-nosatomega"))
		sat = false;

	// Form our vectors into points and draw!
	for (t = (num_segments - 1) * circle_pieces, i = num_segments - 1; i >= 1; i--, t -= circle_pieces)
	{
		rend_SetTextureType(TT_LINEAR);
		rend_SetAlphaType(sat ? AT_SATURATE_TEXTURE : AT_CONSTANT);
		rend_SetZBufferWriteMask(0);
		rend_SetLighting(LS_NONE);
		rend_SetAlphaValue(ELECTRICAL_ALPHA * 255);

		for (int k = 0; k < circle_pieces; k++)
		{
			int next = (k + 1) % circle_pieces;

			arc_points[t + k].p3_u = 0;
			arc_points[t + k].p3_v = 1;

			arc_points[t + k - circle_pieces].p3_u = 0;
			arc_points[t + k - circle_pieces].p3_v = 0;

			arc_points[t + next - circle_pieces].p3_u = 1;
			arc_points[t + next - circle_pieces].p3_v = 1;

			arc_points[t + next].p3_u = 1;
			arc_points[t + next].p3_v = 0;

			pntlist[0] = &arc_points[t + k];
			pntlist[1] = &arc_points[t + k - circle_pieces];
			pntlist[2] = &arc_points[t + next - circle_pieces];
			pntlist[3] = &arc_points[t + next];

			g3_DrawPoly(4, pntlist, bm_handle);

		}


		if (i != num_segments - 1)
		{
			vector subvec = center_vecs[i + 1] - center_vecs[i];
			subvec = center_vecs[i] + (subvec * vchange);

			if (parent_obj == Viewer_object)
			{
				DrawColoredDisk(&subvec, .2f, .5f, 1, .6f, 0, ELECTRICAL_WIDTH * 4, sat, 2);
			}
			else
			{
				DrawColoredDisk(&subvec, .2f, .5f, 1, .6f * fabs(view_dp), 0, ELECTRICAL_WIDTH * 4, sat, 2);
			}
		}
	}

	rend_SetZBufferWriteMask(1);

	// Kill it!
	obj->lifeleft = 0;

}

constexpr float FUSION_RAMP_TIME = 3.f;
constexpr float FUSION_DAMAGE_SOUND_INTERVAL = .5f;
constexpr float FUSION_FRAMETIME_AT_60FPS = 1.f / 60;

float Last_fusion_damage_time = 0;

// Does that crazy fusion effect, including damaging/shaking your ship
void DoFusionEffect(object* objp, int weapon_type)
{
	float norm;
	int move;

	ASSERT(objp->type == OBJ_PLAYER);
	ASSERT((weapon_type == PW_PRIMARY) || (weapon_type == PW_SECONDARY));

	norm = (Players[objp->id].weapon[weapon_type].firing_time / FUSION_RAMP_TIME);

	if (norm > .1)
	{
		float over = norm;

		if (over > 1.0)
			over = 1.0;

		over = (over - .1) / .9;

		move = (ps_rand() % 5) - 2;

		objp->mtype.phys_info.velocity += (objp->orient.rvec) * ((float)move * over * Frametime * 175.0);

		move = (ps_rand() % 5) - 2;

		objp->mtype.phys_info.velocity += (objp->orient.uvec) * ((float)move * over * Frametime * 175.0);

	}

	if (norm > 1.0)
	{
		norm = 1.0;
		float damagescale = Frametime / FUSION_FRAMETIME_AT_60FPS;
		float damage = (.125 * abs(move)) * damagescale;

		if ((!(Game_mode & GM_MULTI)) || Netgame.local_role == LR_SERVER)
		{
			if ((Gametime - Last_fusion_damage_time) < FUSION_DAMAGE_SOUND_INTERVAL)
			{
				ApplyDamageToPlayer(objp, objp, PD_ENERGY_WEAPON, damage, 0, 255, 0);
			}
			else
			{
				ApplyDamageToPlayer(objp, objp, PD_ENERGY_WEAPON, damage);
				Last_fusion_damage_time = Gametime;
			}
		}
		else {
			Multi_requested_damage_type = PD_ENERGY_WEAPON;
			Multi_requested_damage_amount += damage;
		}
	}

	float vals[4];
	vals[0] = 1.0;
	vals[1] = 0.0;
	vals[2] = 1.0;
	vals[3] = .25 * norm;

	CreateNewEvent(RENDER_EVENT, FUSION_EFFECT, 0, &vals, sizeof(float) * 4, DrawAlphaEvent);
}

// Do the spray effect
void DoSprayEffect(object* obj, otype_wb_info* static_wb, ubyte wb_index)
{
	vector laser_pos, laser_dir;
	int cur_m_bit;

	ASSERT(!(obj->flags & OF_DYING));

	poly_model* pm = &Poly_models[obj->rtype.pobj_info.model_num];

	// Go through each gun point
	for (cur_m_bit = 0; cur_m_bit < pm->poly_wb[0].num_gps; cur_m_bit++)
	{
		// Figure out out if this weapon fires from this gunpoint
		if (static_wb->gp_fire_masks[obj->dynamic_wb[wb_index].cur_firing_mask] & (0x01 << cur_m_bit))
		{
			int weapon_num = static_wb->gp_weapon_index[cur_m_bit];

			WeaponCalcGun(&laser_pos, &laser_dir, obj, pm->poly_wb[0].gp_index[cur_m_bit]);
			int visnum = VisEffectCreate(VIS_FIREBALL, SPRAY_INDEX, obj->roomnum, &laser_pos);

			if (visnum < 0)
				return;

			vis_effect* vis = &VisEffects[visnum];


			// Set the initial velocity
			vm_ScaleVector(&vis->velocity, &laser_dir, Weapons[weapon_num].phys_info.velocity.z);

			// Set initial velocity to that of the firing object
			if (Weapons[weapon_num].phys_info.flags & PF_USES_PARENT_VELOCITY)
			{
				float fdot = (obj->mtype.phys_info.velocity * obj->orient.fvec);
				vector fvel;

				if (fdot > 0.0)
					fvel = obj->orient.fvec * fdot;
				else
					fvel = Zero_vector;

				vector rvel = 0.1f * obj->orient.rvec * (obj->mtype.phys_info.velocity * obj->orient.rvec);
				vector uvel = 0.1f * obj->orient.uvec * (obj->mtype.phys_info.velocity * obj->orient.uvec);

				vis->velocity += fvel + rvel + uvel;
			}

			vis->movement_type = MT_PHYSICS;
			vis->custom_handle = Weapons[weapon_num].fire_image_handle;
			vis->drag = Weapons[weapon_num].phys_info.drag;
			vis->phys_flags = Weapons[weapon_num].phys_info.flags;
			vis->mass = Weapons[weapon_num].phys_info.mass;
			vis->size = Weapons[weapon_num].size;
			vis->lifetime = Weapons[weapon_num].life_time;
			vis->lifeleft = vis->lifetime;

			// Now create extras so that there are no gaps 

			if (obj != Viewer_object)
			{
				// Find hit point so we know when to stop
				float life_scalar = 1.0;
				fvi_query	fq;
				fvi_info		hit_data;
				int			fate;
				vector dest_vector = laser_pos + (vis->velocity * vis->lifetime);

				fq.p0 = &laser_pos;
				fq.startroom = obj->roomnum;
				fq.p1 = &dest_vector;
				fq.rad = vis->size;
				fq.thisobjnum = -1;
				fq.ignore_obj_list = NULL;
				fq.flags = FQ_CHECK_OBJS | FQ_IGNORE_NON_LIGHTMAP_OBJECTS;

				fate = fvi_FindIntersection(&fq, &hit_data);

				if (fate != HIT_NONE)
				{
					vector subvec = hit_data.hit_pnt - laser_pos;
					vis->lifetime = vm_GetMagnitudeFast(&subvec) / vm_GetMagnitudeFast(&vis->velocity);
					life_scalar = vis->lifetime / vis->lifeleft;
					vis->lifeleft = vis->lifetime;

				}

				float velmag = vm_GetMagnitudeFast(&vis->velocity);
				vector velnorm = vis->velocity / velmag;
				float len = Frametime * velmag;
				float fextras = len * 2;
				float fps = 1.0 / Frametime;

				int extras = fextras;

				extras = min(extras, 8);

				for (int t = 0; t < extras; t++)
				{

					vector newpos = vis->pos + (((velnorm * len) / extras) * (t + 1));

					int extranum = VisEffectCreate(VIS_FIREBALL, SPRAY_INDEX, obj->roomnum, &newpos);
					if (extranum >= 0)
					{
						vis_effect* extravis = &VisEffects[extranum];
						extravis->mass = vis->mass;
						extravis->drag = vis->drag;
						extravis->velocity = vis->velocity;
						extravis->size = vis->size;
						extravis->phys_flags = vis->phys_flags;
						extravis->movement_type = MT_PHYSICS;
						extravis->custom_handle = vis->custom_handle;
						extravis->lifetime = vis->lifetime;

						float life_adjust = ((Frametime / (float)extras) * (t + 1));

						extravis->lifeleft = Frametime * (fps / 2);
						extravis->lifeleft *= life_scalar;
						life_adjust *= life_scalar;


						if (extravis->lifeleft > 0)
							extravis->creation_time = vis->creation_time - life_adjust;
						else
						{
							VisEffectDelete(extranum);
						}
					}
				}
			}
			else
			{
				// Find hit point so we know when to stop
				fvi_query	fq;
				fvi_info		hit_data;
				int			fate;
				vector dest_vector = laser_pos + (vis->velocity * vis->lifetime);

				fq.p0 = &laser_pos;
				fq.startroom = obj->roomnum;
				fq.p1 = &dest_vector;
				fq.rad = vis->size;
				fq.thisobjnum = -1;
				fq.ignore_obj_list = NULL;
				fq.flags = FQ_CHECK_OBJS | FQ_IGNORE_NON_LIGHTMAP_OBJECTS;

				fate = fvi_FindIntersection(&fq, &hit_data);

				if (fate != HIT_NONE)
				{
					vector subvec = hit_data.hit_pnt - laser_pos;
					vis->lifetime = vm_GetMagnitudeFast(&subvec) / vm_GetMagnitudeFast(&vis->velocity);
					vis->lifeleft = vis->lifetime;
				}
			}

		}
	}

}

// Draws a streamer behind a weapon
void DrawWeaponStreamer(object* obj)
{
	float time_live = Gametime - obj->creation_time;
	float len;
	float norm_time = time_live / obj->lifetime;

	if (norm_time >= 1)
		norm_time = .99999f;		// don't go over!

	// Get length
	object* parent_obj = ObjGet(obj->parent_handle);

	if (!parent_obj)
		return;

	len = vm_VectorDistance(&parent_obj->pos, &obj->pos);

	rend_SetAlphaType(AT_VERTEX);
	rend_SetTextureType(TT_FLAT);
	rend_SetLighting(LS_GOURAUD);
	rend_SetColorModel(CM_RGB);

	vector vecs[2];
	g3Point pnts[2];
	int i;

	vecs[0] = obj->pos;
	vecs[1] = obj->pos - (obj->orient.fvec * len);

	for (i = 0; i < 2; i++)
	{
		g3_RotatePoint(&pnts[i], &vecs[i]);
		pnts[i].p3_flags |= PF_RGBA;
		pnts[i].p3_r = Weapons[obj->id].lighting_info.red_light1;
		pnts[i].p3_g = Weapons[obj->id].lighting_info.green_light1;
		pnts[i].p3_b = Weapons[obj->id].lighting_info.blue_light1;
	}

	pnts[0].p3_a = (1.0 - norm_time) * .3f;
	pnts[1].p3_a = 0;

	g3_DrawSpecialLine(&pnts[0], &pnts[1]);
}

void DrawBlobbyWeaponRing(object* obj)
{
	vector vecs[30];
	float lifenorm = (obj->lifetime - obj->lifeleft) / obj->lifetime;
	int i;

	int bm_handle;
	int num_segments = 20;

	int ring_increment = 65536 / num_segments;
	int ring_angle = 0;

	if (lifenorm >= 1.0)
		lifenorm = .999f;

	if (Weapons[obj->id].flags & WF_IMAGE_BITMAP)
		bm_handle = GetWeaponFireImage(obj->id, 0);
	else
	{
		vclip* vc = &GameVClips[Weapons[obj->id].fire_image_handle];
		int int_frame = vc->num_frames * lifenorm;
		bm_handle = vc->frames[int_frame];
	}

	rend_SetLighting(LS_NONE);
	rend_SetTextureType(TT_LINEAR);
	rend_SetAlphaType(AT_CONSTANT_TEXTURE);
	rend_SetAlphaValue((1.0 - lifenorm) * 255.0);

	// Now draw a ring of blobs
	ring_angle = 0;
	float blob_size = (obj->size / 4);

	for (i = 0; i < num_segments; i++, ring_angle += ring_increment)
	{
		float ring_sin = FixSin(ring_angle);
		float ring_cos = FixCos(ring_angle);

		vecs[i] = obj->orient.rvec * (ring_cos * obj->size);
		vecs[i] += obj->orient.uvec * (ring_sin * obj->size);
		vecs[i] += obj->pos;



		g3_DrawRotatedBitmap(&vecs[i], ring_angle, blob_size, (blob_size * bm_h(bm_handle, 0)) / bm_w(bm_handle, 0), bm_handle);
	}
}

void DrawPolygonalWeaponRing(object* obj)
{
	vector inner_vecs[30], outer_vecs[30];
	g3Point inner_points[30], outer_points[30];
	float lifenorm = (obj->lifetime - obj->lifeleft) / obj->lifetime;
	int i;
	g3Point* pntlist[4];
	float inner_size = (obj->size * .8);
	int objnum = obj - Objects;
	int odd = objnum % 2;

	int num_segments = 20;

	int ring_increment = 65536 / num_segments;
	int ring_angle = 0;

	if (lifenorm > 1.0)
		lifenorm = 1.0;

	rend_SetTextureType(TT_FLAT);
	rend_SetColorModel(CM_RGB);
	rend_SetLighting(LS_GOURAUD);
	rend_SetAlphaType(AT_SATURATE_VERTEX);

	for (i = 0; i < num_segments; i++, ring_angle += ring_increment)
	{
		float ring_sin = FixSin(ring_angle);
		float ring_cos = FixCos(ring_angle);

		inner_vecs[i] = obj->orient.rvec * (ring_cos * inner_size);
		inner_vecs[i] += obj->orient.uvec * (ring_sin * inner_size);
		inner_vecs[i] += obj->pos;

		outer_vecs[i] = obj->orient.rvec * (ring_cos * obj->size);
		outer_vecs[i] += obj->orient.uvec * (ring_sin * obj->size);
		outer_vecs[i] += obj->pos;

		g3_RotatePoint(&inner_points[i], &inner_vecs[i]);
		g3_RotatePoint(&outer_points[i], &outer_vecs[i]);

		outer_points[i].p3_flags |= PF_RGBA | PF_UV;
		inner_points[i].p3_flags |= PF_RGBA | PF_UV;

		outer_points[i].p3_a = (1.0 - lifenorm) * .3;
		inner_points[i].p3_a = (1.0 - lifenorm) * .1;

		outer_points[i].p3_r = Weapons[obj->id].lighting_info.red_light1;
		outer_points[i].p3_g = Weapons[obj->id].lighting_info.green_light1;
		outer_points[i].p3_b = Weapons[obj->id].lighting_info.blue_light1;

		inner_points[i].p3_r = Weapons[obj->id].lighting_info.red_light1;
		inner_points[i].p3_g = Weapons[obj->id].lighting_info.green_light1;
		inner_points[i].p3_b = Weapons[obj->id].lighting_info.blue_light1;
	}

	for (i = 0; i < num_segments; i++)
	{
		if (i % 2 && !odd)
			continue;
		if ((i % 2 == 0) && odd)
			continue;

		int next = (i + 1) % num_segments;

		pntlist[0] = &outer_points[i];
		pntlist[1] = &outer_points[next];
		pntlist[2] = &inner_points[next];
		pntlist[3] = &inner_points[i];

		g3_DrawPoly(4, pntlist, 0);
	}

}

// Draws a ring for the weapon
void DrawWeaponRing(object* obj)
{
	if ((Weapons[obj->id].flags & WF_IMAGE_BITMAP) || (Weapons[obj->id].flags & WF_IMAGE_VCLIP))
	{
		DrawBlobbyWeaponRing(obj);
		return;
	}
	else
		DrawPolygonalWeaponRing(obj);

}

// Draws a weapon
void DrawWeaponObject(object* obj)
{
	ASSERT(obj->type == OBJ_WEAPON || obj->type == OBJ_POWERUP);
	ASSERT(Weapons[obj->id].used > 0);


	// Don't draw if spray
	if (!(Weapons[obj->id].flags & WF_INVISIBLE))
	{
		if (Weapons[obj->id].flags & WF_ELECTRICAL)
		{
			DrawElectricalWeapon(obj);
		}
		else if ((Weapons[obj->id].flags & WF_IMAGE_BITMAP) || (Weapons[obj->id].flags & WF_IMAGE_VCLIP))
		{
			int bm_handle;
			int objnum = obj - Objects;
			float rot_temp = Weapons[obj->id].phys_info.rotvel.z / 65536.0;
			int int_game = Gametime / rot_temp;
			float diff = Gametime - (int_game * rot_temp);
			int rot_angle = diff * 65536;

			if (Weapons[obj->id].flags & WF_NO_ROTATE)
				rot_angle = (objnum * 1000) % 65536;

			bm_handle = GetWeaponFireImage(obj->id, 0);
			ASSERT(bm_handle >= 0);

			if (Weapons[obj->id].flags & WF_SATURATE)
				rend_SetAlphaType(AT_SATURATE_TEXTURE);
			else
				rend_SetAlphaType(AT_CONSTANT + AT_TEXTURE);

			rend_SetAlphaValue(Weapons[obj->id].alpha * 255);
			rend_SetOverlayType(OT_NONE);
			rend_SetLighting(LS_NONE);

			if (Weapons[obj->id].flags & WF_PLANAR)
			{
				vector norm = obj->mtype.phys_info.velocity;
				vm_NormalizeVectorFast(&norm);
				g3_DrawPlanarRotatedBitmap(&obj->pos, &norm, rot_angle, obj->size, (obj->size * bm_h(bm_handle, 0)) / bm_w(bm_handle, 0), bm_handle);
			}
			else
				g3_DrawRotatedBitmap(&obj->pos, rot_angle, obj->size, (obj->size * bm_h(bm_handle, 0)) / bm_w(bm_handle, 0), bm_handle);
		}
		else
		{

			polymodel_effect pe = { 0 };
			int use_effect = 0;

			if (obj->type == OBJ_WEAPON && !Detail_settings.Weapon_coronas_enabled)
			{
				pe.type |= PEF_NO_GLOWS;
				use_effect = 1;
			}

			if (use_effect)
				SetPolymodelEffect(&pe);

			DrawPolygonModel(&obj->pos, &obj->orient, obj->rtype.pobj_info.model_num, NULL, 0, 1.0, 1.0, 1.0, 0xFFFFFFFF, use_effect);
		}
	}

	if (Weapons[obj->id].flags & WF_STREAMER)
	{
		DrawWeaponStreamer(obj);
	}

	if (Weapons[obj->id].flags & WF_RING)
	{
		DrawWeaponRing(obj);
	}
}

// Stops any on/off weapons that are firing
void StopOnOffWeapon(object* obj)
{
	//mprintf ((0,"Stopping on/off weapon!\n"));
	obj->weapon_fire_flags &= ~WFF_ON_OFF;

	if (Demo_flags == DF_RECORDING)
	{
		DemoWriteObjWeapFireFlagChanged(OBJNUM(obj));
	}
}

// Starts an on/off weapon firing
void StartOnOffWeapon(object* obj, ubyte wb_index)
{
	//mprintf ((0,"Starting on/off weapon!\n"));
	obj->weapon_fire_flags |= WFF_ON_OFF;

	dynamic_wb_info* p_dwb = &obj->dynamic_wb[wb_index];
	p_dwb->cur_firing_mask = 0;


	if (Demo_flags == DF_RECORDING)
	{
		DemoWriteObjWeapFireFlagChanged(OBJNUM(obj));
	}
}

float Zoom_fov_time = 0;

#define ZOOM_STAY_TIME	.25
#define ZOOM_FOV_TARGET	18

void DoZoomStay()
{
	if (Zoom_fov_time > 0)
	{
		Render_FOV = ZOOM_FOV_TARGET;
		Zoom_fov_time -= Frametime;
		if (Zoom_fov_time < 0)
		{
			Zoom_fov_time = 0;
			Players[Player_num].flags &= ~PLAYER_FLAGS_ZOOMED;
			Render_FOV = Render_FOV_desired;
		}
	}
}

// Zooms in for this weapon
void DoZoomEffect(player_weapon* pw, ubyte clear)
{
	if (pw->firing_time < .5)
	{
		Players[Player_num].turn_scalar = 1.0;
		Render_FOV = Render_FOV_desired;

		if (!clear)
			DoZoomStay();
		return;
	}

	Players[Player_num].turn_scalar = .3f;

	if (pw->firing_time > 1.0)
	{
		// We are fully zoomed in

		Players[Player_num].flags |= PLAYER_FLAGS_ZOOMED;
		Render_FOV = ZOOM_FOV_TARGET;
		Zoom_fov_time = ZOOM_STAY_TIME;

		// See if we hit anything
		// Cast a ray to see what we've hit
		Players[Player_num].flags &= ~PLAYER_FLAGS_BULLSEYE;
		object* obj = &Objects[Players[Player_num].objnum];
		int fate;															// Collision type for response code
		fvi_info hit_info;												// Hit information
		fvi_query fq;														// Movement query
		vector dest = obj->pos + (obj->orient.fvec * 5000.0);

		fq.p0 = &obj->pos;
		fq.startroom = obj->roomnum;
		fq.p1 = &dest;
		fq.rad = .0001f;
		fq.thisobjnum = Objects - obj;
		fq.ignore_obj_list = NULL;
		fq.flags = FQ_CHECK_OBJS | FQ_IGNORE_POWERUPS | FQ_IGNORE_WEAPONS;

		fate = fvi_FindIntersection(&fq, &hit_info);

		Players[Player_num].zoom_distance = vm_VectorDistance(&obj->pos, &hit_info.hit_pnt);

		if (fate == HIT_SPHERE_2_POLY_OBJECT || fate == HIT_OBJECT)
		{
			object* hit_obj = &Objects[hit_info.hit_object[0]];
			if (hit_obj->type == OBJ_ROBOT || hit_obj->type == OBJ_PLAYER || (hit_obj->type == OBJ_BUILDING && hit_obj->ai_info))
				Players[Player_num].flags |= PLAYER_FLAGS_BULLSEYE;
		}
		return;
	}

	// calculate zoom effect
	float norm = (pw->firing_time - .5) * 2;

	Render_FOV = (ZOOM_FOV_TARGET * norm) + ((1.0 - norm) * Render_FOV_desired);
	mprintf((0, "%f\n", Render_FOV));

	return;
}

//Called when a player dies or switches weapons to clear out any active weapon stuff
void ClearPlayerFiring(object* objp, int weapon_type)
{
	//Check parms
	ASSERT(objp->type == OBJ_PLAYER);
	//	ASSERT(objp->id == Player_num);

		//Set up various vars
	player* player = &Players[objp->id];
	player_weapon* pw = &player->weapon[weapon_type];

	//Clear firing flags
	if (objp->weapon_fire_flags & WFF_ON_OFF)
		StopOnOffWeapon(objp);

	//Clear firing time
	pw->firing_time = 0.0;
	objp->weapon_fire_flags = 0;
	if (Demo_flags == DF_RECORDING)
	{
		DemoWriteObjWeapFireFlagChanged(OBJNUM(objp));
	}

	// Clear zoom if there is one
	if (objp->id == Player_num)
	{
		player_weapon* pw = &player->weapon[weapon_type];
		int weapon_battery_index = pw->index;
		ship* ship = &Ships[player->ship_index];

		if (ship->fire_flags[weapon_battery_index] & SFF_ZOOM)
		{
			DoZoomEffect(pw, 1);
			Players[Player_num].flags &= ~PLAYER_FLAGS_ZOOMED;
			Zoom_fov_time = 0;
		}
	}

	//Stop firing sound if one playing
	if (pw->sound_handle != -1) {
		Sound_system.StopSoundImmediate(pw->sound_handle);
		pw->sound_handle = -1;
	}
}

//Stops weapon sound & visual effects 
void StopWeapon(object* obj, player_weapon* pw, otype_wb_info* wb)
{
	// Stop any on/off weapons
	if (obj->weapon_fire_flags & WFF_ON_OFF)
		StopOnOffWeapon(obj);

	//Stop spray weapons
	if (wb->flags & WBF_SPRAY)
		obj->weapon_fire_flags &= ~WFF_SPRAY;

	//Stop firing sound if one playing
	if (pw && pw->sound_handle != -1)
	{
		Sound_system.StopSoundImmediate(pw->sound_handle);
		pw->sound_handle = -1;
	}

	//Clear firing time
	if (pw)
	{
		pw->firing_time = 0.0;
	}
	if (Demo_flags == DF_RECORDING)
	{
		DemoWriteObjWeapFireFlagChanged(OBJNUM(obj));
	}
}

// Does weapon battery stuff for the permissable network architecture
void DoPermissableWeaponMask(int weapon_battery_index)
{
	player* player = &Players[Player_num];
	object* objp = &Objects[player->objnum];
	ship* ship = &Ships[player->ship_index];
	otype_wb_info* wb = &ship->static_wb[weapon_battery_index];
	dynamic_wb_info* p_dwb = &objp->dynamic_wb[weapon_battery_index];

	player->last_fire_weapon_time = Gametime;


	p_dwb->cur_firing_mask++;
	p_dwb->last_fire_time = Gametime;

	if (p_dwb->cur_firing_mask >= wb->num_masks)
		p_dwb->cur_firing_mask = 0;
}

// Fires a weapon from our player.  Won't fire if ammo/energy requirements aren't met.
// Parameters:	weapon_type - either PW_PRIMARY or PW_SECONDARY
void FireWeaponFromPlayer(object* objp, int weapon_type, int down_count, bool down_state, float down_time)
{
	//Check parms
	ASSERT(objp->type == OBJ_PLAYER);
	//	ASSERT(objp->id == Player_num);
	ASSERT((weapon_type == PW_PRIMARY) || (weapon_type == PW_SECONDARY));

	//Set up various vars
	player* player = &Players[objp->id];
	player_weapon* pw = &player->weapon[weapon_type];
	int weapon_battery_index = pw->index;
	ship* ship = &Ships[player->ship_index];
	otype_wb_info* wb = &ship->static_wb[weapon_battery_index];
	int fire_on_release = (ship->fire_flags[weapon_battery_index] & (SFF_FUSION | SFF_ZOOM));
	bool can_fire_now = WBIsBatteryReady(objp, wb, weapon_battery_index);
	dynamic_wb_info* p_dwb = &objp->dynamic_wb[weapon_battery_index];
	float ammo_scalar = 1.0;

	if (p_dwb->flags & DWBF_QUAD)
		ammo_scalar = 2.0;

	//Don't allow down_time to be zero if we have a down count
	if (down_count && (down_time == 0.0))
		down_time = 1.0f / 100.0f;

	// Check to see if we can release a guided
	if (down_count && down_time != 0.0 && Players[objp->id].guided_obj != NULL && weapon_type == PW_SECONDARY && can_fire_now)
	{
		ReleaseGuidedMissile(objp->id);
		p_dwb->last_fire_time = Gametime;
		return;
	}

	// Check to see if we can release a user timeout weapon
	if (down_count && down_time != 0.0 && Players[objp->id].user_timeout_obj != NULL && weapon_type == PW_SECONDARY && can_fire_now)
	{
		if (Game_mode & GM_MULTI)
			MultiSendReleaseTimeoutMissile();

		ReleaseUserTimeoutMissile(objp->id);
		p_dwb->last_fire_time = Gametime;
		return;
	}

	//Check for weapon stopped firing
	if ((down_count == 0) && (down_time == 0))
	{
		if (ship->fire_flags[weapon_battery_index] & SFF_ZOOM)
			DoZoomEffect(pw, 0);

		//Abort if we weren't firing last frame either
		if (pw->firing_time == 0.0)
			return;

		//Check for fire-on-release weapon
		if (fire_on_release)
		{
			if (can_fire_now)
			{
				float scalar = 1.0;

				//Ready?
				if (ship->fire_flags[weapon_battery_index] & SFF_FUSION)
				{
					scalar = 1.0 + ((pw->firing_time / FUSION_RAMP_TIME) * 3.0);

				}

				if ((Game_mode & GM_MULTI) && (Netgame.flags & NF_PERMISSABLE))
				{
					MultiSendRequestToFire(weapon_battery_index, p_dwb->cur_firing_mask, scalar);
					DoPermissableWeaponMask(weapon_battery_index);
				}
				else
					WBFireBattery(objp, wb, 0, weapon_battery_index, scalar);

				player->energy -= wb->energy_usage * ammo_scalar;
				if (player->energy < 0)
					player->energy = 0;

				if (player->weapon_ammo[weapon_battery_index] < (wb->ammo_usage * ammo_scalar))
					player->weapon_ammo[weapon_battery_index] = 0;
				else
					player->weapon_ammo[weapon_battery_index] -= (wb->ammo_usage * ammo_scalar);

				player->num_discharges_level++;

				if (ship->fire_flags[weapon_battery_index] & SFF_ZOOM)
				{
					AddToShakeMagnitude(ship->phys_info.mass * 2);
				}
			}
		}

		//Play cut-off sound if one (but not if in permissable game)
		if (!(Game_mode & GM_MULTI) || !(Netgame.flags & NF_PERMISSABLE))
		{
			int cutoff_sound = ship->firing_release_sound[weapon_battery_index];
			if (cutoff_sound != -1)
				Sound_system.Play2dSound(cutoff_sound, SND_PRIORITY_HIGHEST);
		}

		//Stop the weapon sound & visual effects
		if (weapon_type == PW_PRIMARY)
			StopWeapon(objp, pw, wb);

		//We're done
		return;
	}

	//Check for adequate energy
	if ((wb->energy_usage * ammo_scalar) && (player->energy <= 0)) {
		AddHUDMessage(TXT_WPNNONRG);
		if (weapon_type == PW_PRIMARY)
			StopWeapon(objp, pw, wb);
		AutoSelectWeapon(weapon_type);
		return;
	}

	//Check for adequate ammo
	if ((wb->ammo_usage * ammo_scalar) && (player->weapon_ammo[weapon_battery_index] <= 0)) {
		if (weapon_type == PW_PRIMARY)
			AddHUDMessage(TXT_WPNNOAMMO);
		else
			AddHUDMessage(TXT_WPNNOPROJ);

		if (weapon_type == PW_PRIMARY)
			StopWeapon(objp, pw, wb);
		AutoSelectWeapon(weapon_type);
		return;
	}

	//Check for adequate ammo for secondaries...
	if (weapon_battery_index >= 10 && (wb->ammo_usage * ammo_scalar > player->weapon_ammo[weapon_battery_index]))
	{
		if (weapon_type == PW_PRIMARY)
			AddHUDMessage(TXT_WPNNOAMMO);
		else
			AddHUDMessage(TXT_WPNNOPROJ);

		if (weapon_type == PW_PRIMARY)
			StopWeapon(objp, pw, wb);
		AutoSelectWeapon(weapon_type);
		return;
	}

	if (!can_fire_now)
		return;

	//If continous sound weapon, start the sound if wasn't firing last frame
	int firing_sound = ship->firing_sound[weapon_battery_index];

	if (can_fire_now && (firing_sound != -1) && (pw->firing_time == 0.0))
		pw->sound_handle = Sound_system.Play2dSound(firing_sound, SND_PRIORITY_HIGHEST);

	//Set spray flag if spray weapon
	if (wb->flags & WBF_SPRAY)
		objp->weapon_fire_flags |= WFF_SPRAY;

	if (ship->fire_flags[weapon_battery_index] & SFF_ZOOM)
		DoZoomEffect(pw, 0);

	//Do fusion effect if fusion weapon
	if (can_fire_now && ship->fire_flags[weapon_battery_index] & SFF_FUSION)
		DoFusionEffect(objp, weapon_type);

	//Start any on/off weapons
	if ((wb->flags & WBF_ON_OFF) && !(objp->weapon_fire_flags & WFF_ON_OFF))
		StartOnOffWeapon(objp, weapon_battery_index);

	//If not a fire-on-release weapon, then fire if ready
	if (!fire_on_release && can_fire_now) {

		if ((Game_mode & GM_MULTI) && (Netgame.flags & NF_PERMISSABLE))
		{
			MultiSendRequestToFire(weapon_battery_index, p_dwb->cur_firing_mask);
			DoPermissableWeaponMask(weapon_battery_index);
		}
		else
			WBFireBattery(objp, wb, 0, weapon_battery_index);

		//If on/off weapon, scale energy by framerate based on "reference" rate of 20 fps.
		if (wb->flags & WBF_ON_OFF)
			player->energy -= (wb->energy_usage * ammo_scalar * Frametime / (1.0 / 20.0));
		else
			player->energy -= (wb->energy_usage * ammo_scalar);
		if (player->energy < 0)
			player->energy = 0;

		if (player->weapon_ammo[weapon_battery_index] < (wb->ammo_usage * ammo_scalar))
			player->weapon_ammo[weapon_battery_index] = 0;
		else
			player->weapon_ammo[weapon_battery_index] -= (wb->ammo_usage * ammo_scalar);

		player->num_discharges_level++;
	}


	//Update the firing time
	pw->firing_time += down_time;
	if (Demo_flags == DF_RECORDING)
	{
		DemoWriteObjWeapFireFlagChanged(OBJNUM(objp));
	}

	// tell IntelliVIBE that we fired a weapon
	VIBE_WeaponFire(weapon_type);
}

// Fires a flare from our player. 
// It might make sense to combine this with FireWeaponFromPlayer(), or maybe not
void FireFlareFromPlayer(object* objp)
{
	//Check parms
	ASSERT(objp->type == OBJ_PLAYER);
	//	ASSERT(objp->id == Player_num || objp->control_type == CT_SOAR);

		//Set up various vars
	int weapon_battery_index = FLARE_INDEX;
	ship* ship = &Ships[Players[objp->id].ship_index];
	otype_wb_info* wb = &ship->static_wb[weapon_battery_index];
	player* player = &Players[objp->id];
	dynamic_wb_info* p_dwb = &objp->dynamic_wb[weapon_battery_index];

	if (player->energy < wb->energy_usage) {
		AddHUDMessage(TXT_WPNFLARENONRG);
		return;
	}

	if (WBIsBatteryReady(Player_object, wb, weapon_battery_index)) {
		if ((Game_mode & GM_MULTI) && (Netgame.flags & NF_PERMISSABLE))
		{
			MultiSendRequestToFire(weapon_battery_index, p_dwb->cur_firing_mask);
			DoPermissableWeaponMask(weapon_battery_index);
		}
		else
			WBFireBattery(Player_object, wb, 0, weapon_battery_index);

		player->energy -= wb->energy_usage;
		if (player->energy < 0)
			player->energy = 0;

		//Samir & I think that firing a flare should not count as discharge, even though
		//it would allow people push up their kill effiency by killing things with flares.  -MT, 10/29/97
		//Players[Player_num].num_discharges_level++;
	}
}

// Creates a gravity field that sucks objects into it
int CreateGravityField(vector* pos, int roomnum, float size, float time, int parent_handle);


// Plays the animation that accompanies a weapon death
void DoWeaponExploded(object* obj, vector* norm, vector* collision_point, object* hit_object)
{
	weapon* w = &Weapons[obj->id];
	light_info* li = &w->lighting_info;
	vector col_point, normal;

	if (w->flags & WF_ELECTRICAL)
		return;

	MakeShockwave(obj, obj->parent_handle);

	if (collision_point == NULL || !(Weapons[obj->id].flags & WF_PLANAR_BLAST))
		col_point = obj->pos;
	else
		col_point = *collision_point;

	if (norm == NULL)
		normal = obj->orient.fvec;
	else
		normal = *norm;


	if (Weapons[obj->id].flags & WF_GRAVITY_FIELD)
		CreateGravityField(&obj->pos, obj->roomnum, Weapons[obj->id].gravity_size, Weapons[obj->id].gravity_time, obj->parent_handle);


	if (hit_object == Viewer_object && !(Weapons[obj->id].flags & WF_MATTER_WEAPON))
		return;	// Don't draw if its viewer who is getting hit

	if (Weapons[obj->id].flags & WF_PLANAR_BLAST)
	{
		if (Weapons[obj->id].flags & WF_BLAST_RING)
		{
			// Create a planar ring
			int visnum = VisEffectCreate(VIS_FIREBALL, BLAST_RING_INDEX, obj->roomnum, &col_point);
			if (visnum >= 0)
			{
				vis_effect* vis = &VisEffects[visnum];
				vis->size = Weapons[obj->id].explode_size;
				vis->lifetime = Weapons[obj->id].explode_time;
				vis->lifeleft = Weapons[obj->id].explode_time;
				vis->custom_handle = GetTextureBitmap(Weapons[obj->id].explode_image_handle, 0);
				vis->flags |= VF_PLANAR;
				vis->end_pos = normal;
				vis->lighting_color = OPAQUE_FLAG | GR_RGB16(li->red_light2 * 255, li->green_light2 * 255, li->blue_light2 * 255);
			}
		}
		else
		{
			// Create a planar explosion
			int visnum;

			if (Weapons[obj->id].explode_image_handle > 0)
			{
				visnum = VisEffectCreate(VIS_FIREBALL, CUSTOM_EXPLOSION_INDEX, obj->roomnum, &col_point);
				if (visnum >= 0)
				{
					vis_effect* vis = &VisEffects[visnum];
					vis->size = Weapons[obj->id].explode_size;
					vis->lifetime = Weapons[obj->id].explode_time;
					vis->lifeleft = Weapons[obj->id].explode_time;
					vis->custom_handle = Weapons[obj->id].explode_image_handle;
					vis->lighting_color = OPAQUE_FLAG | GR_RGB16(li->red_light2 * 255, li->green_light2 * 255, li->blue_light2 * 255);
				}
			}
			else
				visnum = VisEffectCreate(VIS_FIREBALL, GetRandomSmallExplosion(), obj->roomnum, &col_point);

			if (visnum >= 0)
			{
				VisEffects[visnum].flags |= VF_PLANAR;
				VisEffects[visnum].end_pos = normal;

				if (Weapons[obj->id].flags & WF_EXPAND)
					VisEffects[visnum].flags |= VF_EXPAND;

			}
		}
	}
	else
	{
		if (Weapons[obj->id].flags & WF_BLAST_RING)
		{
			// Create a non-planar ring
			int visnum = VisEffectCreate(VIS_FIREBALL, BLAST_RING_INDEX, obj->roomnum, &col_point);
			if (visnum >= 0)
			{
				vis_effect* vis = &VisEffects[visnum];
				vis->size = Weapons[obj->id].explode_size;
				vis->lifetime = Weapons[obj->id].explode_time;
				vis->lifeleft = Weapons[obj->id].explode_time;
				vis->custom_handle = GetTextureBitmap(Weapons[obj->id].explode_image_handle, 0);
				vis->lighting_color = OPAQUE_FLAG | GR_RGB16(li->red_light2 * 255, li->green_light2 * 255, li->blue_light2 * 255);
			}
		}
		else
		{
			// Create a normal "always facing you" explosion
			if (Weapons[obj->id].explode_image_handle > 0)
			{
				int visnum = VisEffectCreate(VIS_FIREBALL, CUSTOM_EXPLOSION_INDEX, obj->roomnum, &col_point);
				if (visnum >= 0)
				{
					vis_effect* vis = &VisEffects[visnum];
					vis->size = Weapons[obj->id].explode_size;
					vis->lifetime = Weapons[obj->id].explode_time;
					vis->lifeleft = Weapons[obj->id].explode_time;
					vis->custom_handle = Weapons[obj->id].explode_image_handle;
					vis->lighting_color = OPAQUE_FLAG | GR_RGB16(li->red_light2 * 255, li->green_light2 * 255, li->blue_light2 * 255);

					if (Weapons[obj->id].flags & WF_EXPAND)
						VisEffects[visnum].flags |= VF_EXPAND;

				}
			}
			else
				VisEffectCreate(VIS_FIREBALL, GetRandomSmallExplosion(), obj->roomnum, &col_point);
		}
	}

}

// Creates chidren from a dying weapon
void CreateTimeoutSpawnFromWeapon(object* obj)
{
	int n = obj->id;


	ASSERT(Weapons[n].spawn_count > 0 && Weapons[n].spawn_handle >= 0);

	int num = Weapons[n].spawn_count;
	int spawn_index = Weapons[n].spawn_handle;
	vector pos;
	matrix orient;

	// See if we should spawn the alternate
	if (Weapons[n].alternate_spawn_handle >= 0 && Weapons[n].alternate_chance > 0)
	{
		if ((ps_rand() % 100) < Weapons[n].alternate_chance)
			spawn_index = Weapons[n].alternate_spawn_handle;
	}

	for (int i = 0; i < num; i++)
	{
		if (i == 0)	// Make first one fire from the center of the ring
		{
			pos = obj->pos + (obj->orient.fvec * obj->size / 2);

			CreateAndFireWeapon(&pos, &obj->orient.fvec, obj, spawn_index);
		}
		else
		{
			matrix temp_mat;
			pos = obj->pos + (obj->orient.fvec * obj->size / 2);

			float norm = ((float)(i - 1)) / ((float)num - 1);

			float mysin = FixSin(norm * 65536);
			float mycos = FixCos(norm * 65536);

			int x_twist = mycos * 4096;
			int y_twist = mysin * 4096;

			vm_AnglesToMatrix(&temp_mat, x_twist, y_twist, 0);
			vm_TransposeMatrix(&temp_mat);

			orient = obj->orient * temp_mat;

			CreateAndFireWeapon(&pos, &orient.fvec, obj, spawn_index);

		}
	}
}

// Creates a robot as a countermeasure
void CreateRobotSpawnFromWeapon(object* obj)
{
	int index = obj->id;
	weapon* wp = &Weapons[index];

	if ((Game_mode & GM_MULTI) && Netgame.local_role != LR_SERVER)
		return;		// clients do not create robots without the servers permission

	if (wp->robot_spawn_handle == -1)
	{
		mprintf((0, "Trying to create an invalid robot spawn!\n"));
		return;
	}
	object* parent_obj = ObjGetUltimateParent(obj);

	int objnum = ObjCreate(OBJ_ROBOT, wp->robot_spawn_handle, obj->roomnum, &obj->pos, &obj->orient, parent_obj->handle);

	if (objnum < 0)
		return;

	if ((Game_mode & GM_MULTI) && Netgame.local_role == LR_SERVER)
	{
		MultiSendObject(&Objects[objnum], 0);
	}

	// Create the scripts for it
	InitObjectScripts(&Objects[objnum]);
}

// Returns a object to home on for spawned weapons
object* FindSpawnHomingTarget(object* obj)
{
	int i;
	float best_dist = MAX_HOMING_DIST / 2;
	int best_index = -1;
	object* weapon_parent = ObjGet(obj->parent_handle);

	for (i = 0; i <= Highest_object_index; i++)
	{
		if ((i != OBJNUM(weapon_parent)) && ((Objects[i].type == OBJ_ROBOT) || (Objects[i].type == OBJ_PLAYER) || (Objects[i].type == OBJ_BUILDING && Objects[i].ai_info)))
		{
			if (BOA_IsVisible(obj->roomnum, Objects[i].roomnum))
			{
				vector to_target = Objects[i].pos - obj->pos;

				if (Objects[i].effect_info && Objects[i].effect_info->type_flags & EF_CLOAKED)
					continue;

				if (ObjGet(obj->parent_handle) && !AIObjEnemy(ObjGetUltimateParent(ObjGet(obj->parent_handle)), ObjGetUltimateParent(&Objects[i])))
					continue;


				float dist_to_target = vm_NormalizeVector(&to_target) - obj->size - Objects[i].size;

				if (dist_to_target < best_dist)
				{
					fvi_query fq;
					fvi_info	hit_info;

					fq.p0 = &obj->pos;
					fq.startroom = obj->roomnum;
					fq.p1 = &Objects[i].pos;
					fq.rad = 0.0f;
					fq.thisobjnum = OBJNUM(obj);
					fq.ignore_obj_list = NULL;
					fq.flags = FQ_TRANSPOINT | FQ_CHECK_OBJS | FQ_ONLY_DOOR_OBJ | FQ_NO_RELINK;

					fvi_FindIntersection(&fq, &hit_info);
					if (hit_info.hit_type[0] == HIT_NONE)
					{
						best_index = i;
						best_dist = dist_to_target;
					}
				}
			}
		}
	}
	if (best_index >= 0)
		return &Objects[best_index];

	return NULL;
}

// Creates chidren from a dying weapon
void CreateImpactSpawnFromWeapon(object* obj, vector* normal)
{
	int n = obj->id;
	object* home_target = NULL;

	ASSERT(Weapons[n].spawn_count > 0 && Weapons[n].spawn_handle >= 0);

	int num = Weapons[n].spawn_count;
	int spawn_index = Weapons[n].spawn_handle;

	// See if we should spawn the alternate
	if (Weapons[n].alternate_spawn_handle >= 0 && Weapons[n].alternate_chance > 0)
	{
		if ((ps_rand() % 100) < Weapons[n].alternate_chance)
			spawn_index = Weapons[n].alternate_spawn_handle;
	}

	if (Game_mode & GM_MULTI)
		ps_srand(num);

	if (Weapons[n].flags & WF_HOMING_SPLIT)
	{
		home_target = FindSpawnHomingTarget(obj);
	}

	for (int i = 0; i < num; i++)
	{
		if (home_target != NULL)
		{
			vector subvec = home_target->pos - obj->pos;
			float mag = vm_GetMagnitudeFast(&subvec);
			subvec /= mag;

			int objnum = CreateAndFireWeapon(&obj->pos, &subvec, obj, spawn_index);

			// Stagger the weapons a bit
			if (objnum > -1)
			{
				object* created_obj = &Objects[objnum];

				float scalar = .2 + ((float)(ps_rand() % 1000) / 1000.0);

				created_obj->mtype.phys_info.velocity *= scalar;
			}
		}
		else
		{
			matrix temp_mat;
			matrix rot_mat;
			vector new_norm;

			float norm = ((float)(ps_rand() % 1000)) / 1000.0;
			int x_twist = 16384 - (norm * 32768);
			if (x_twist < 0)
				x_twist = 65536 + x_twist;

			norm = ((float)(ps_rand() % 1000)) / 1000.0;
			int y_twist = 16384 - (norm * 32768);
			if (y_twist < 0)
				y_twist = 65536 + y_twist;


			vm_VectorToMatrix(&rot_mat, normal, NULL, NULL);
			vm_TransposeMatrix(&rot_mat);
			vm_AnglesToMatrix(&temp_mat, x_twist, y_twist, 0);

			new_norm = temp_mat.fvec * rot_mat;

			CreateAndFireWeapon(&obj->pos, &new_norm, obj, spawn_index);
		}
	}

}

// Does whatever to be done when a weapon times out
void TimeoutWeapon(object* obj)
{
	int n = obj->id;

	if ((Weapons[n].flags & WF_SPAWNS_TIMEOUT) && Weapons[n].spawn_count > 0 && Weapons[n].spawn_handle >= 0)
	{
		DoWeaponExploded(obj, &obj->orient.fvec);
		CreateTimeoutSpawnFromWeapon(obj);
	}

	if ((Weapons[n].flags & WF_MATTER_WEAPON) && !(Weapons[n].flags & WF_SPAWNS_TIMEOUT) && (Weapons[n].spawn_count <= 0))
	{
		vector temp = Zero_vector;

		if (Weapons[obj->id].sounds[WSI_IMPACT_WALL] != SOUND_NONE_INDEX)
			Sound_system.Play3dSound(Weapons[obj->id].sounds[WSI_IMPACT_WALL], SND_PRIORITY_HIGH, obj);

		temp.y = 1.0;

		DoWeaponExploded(obj, &temp);
	}

	if (Weapons[n].flags & WF_TIMEOUT_WALL)
	{
		if (Weapons[obj->id].sounds[WSI_IMPACT_WALL] != SOUND_NONE_INDEX)
			Sound_system.Play3dSound(Weapons[obj->id].sounds[WSI_IMPACT_WALL], SND_PRIORITY_HIGH, obj);

		DoWeaponExploded(obj);
	}

	if ((Weapons[obj->id].flags & WF_SPAWNS_ROBOT) && (Weapons[obj->id].flags & WF_COUNTERMEASURE) && Weapons[obj->id].robot_spawn_handle >= 0)
		CreateRobotSpawnFromWeapon(obj);
}


// Given a parent object and a weapon id, creates that countermeasure
void CreateCountermeasureFromObject(object* parent, int weapon_id)
{
	ASSERT(parent != NULL);
	ASSERT(Weapons[weapon_id].used);

	if (Game_mode & GM_MULTI)
		MultiSendRequestCountermeasure(parent - Objects, weapon_id);
	else
		FireWeaponFromObject(parent, weapon_id, 5);
}


// Releases the guided missile of a passed in player
void ReleaseGuidedMissile(int slot)
{
	ASSERT(Players[slot].guided_obj);
	if (!Players[slot].guided_obj)
		return;

	if ((Game_mode & GM_MULTI) && (slot == Player_num))
	{
		MultiSendMissileRelease(slot, true);
	}

	Players[slot].guided_obj->mtype.phys_info.flags &= ~PF_GUIDED;
	Players[slot].guided_obj = NULL;
}


// Releases the timeout missile of a passed in player
void ReleaseUserTimeoutMissile(int slot)
{
	ASSERT(Players[slot].user_timeout_obj);
	if (!Players[slot].user_timeout_obj)
		return;

	if ((Game_mode & GM_MULTI) && (slot == Player_num))
	{
		MultiSendMissileRelease(slot, false);
	}

	TimeoutWeapon(Players[slot].user_timeout_obj);
	SetObjectDeadFlag(Players[slot].user_timeout_obj);

	Players[slot].user_timeout_obj = NULL;
}
