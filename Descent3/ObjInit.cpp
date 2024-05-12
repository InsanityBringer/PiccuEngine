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

#include "objinit.h"
#include "player.h"
#include "ship.h"
#include "pserror.h"
#include "PHYSICS.H"
#include "weapon.h"
#include "AIMain.h"
#include "fireball.h"
#include "objinfo.h"
#include "Mission.h"
#include "robotfire.h"
#include "door.h"
#include "vclip.h"
#include "polymodel.h"
#include "robot.h"
#include "sounds.h"
#include "mem.h"
#include "marker.h"
//#include <malloc.h>
#include <stdlib.h>
#include "psrand.h"


//Allocate and initialize an effect_info struct for an object
void ObjCreateEffectInfo(object* objp)
{
	if (objp->effect_info)
		mem_free(objp->effect_info);

	objp->effect_info = (effect_info_s*)mem_malloc(sizeof(effect_info_s));
	memset(objp->effect_info, 0, sizeof(effect_info_s));
	ASSERT(objp->effect_info);
	objp->effect_info->sound_handle = SOUND_NONE_INDEX;
}


void ObjSetRenderPolyobj(object* objp, int model_num, int dying_model_num = -1);
#define NUM_PLAYER_ATTACH_POINTS 3
//Initialize the polygon object information for an object
void ObjSetRenderPolyobj(object* objp, int model_num, int dying_model_num)
{
	objp->render_type = RT_POLYOBJ;
	objp->flags |= OF_POLYGON_OBJECT;
	polyobj_info* p_info = &objp->rtype.pobj_info;
	p_info->model_num = model_num;
	p_info->dying_model_num = dying_model_num;
	p_info->anim_frame = 0.0f;
	p_info->tmap_override = -1;
	p_info->subobj_flags = 0xFFFFFFFF;
	p_info->multi_turret_info.num_turrets = 0;
	p_info->multi_turret_info.keyframes = NULL;
	p_info->multi_turret_info.last_keyframes = NULL;
	//Initialize attach slots
	if (model_num != -1)
	{
		poly_model* pm = GetPolymodelPointer(model_num);		//get also pages in
		if (objp->attach_children != NULL)
		{
			mem_free(objp->attach_children);
			objp->attach_children = NULL;
		}

		if ((objp->attach_children == NULL) && pm->n_attach)
		{
			objp->attach_children = (int*)mem_malloc(sizeof(int) * pm->n_attach);

			if (objp->type == OBJ_PLAYER)
				ASSERT(pm->n_attach >= NUM_PLAYER_ATTACH_POINTS);

			for (int i = 0; i < pm->n_attach; i++)
				objp->attach_children[i] = OBJECT_HANDLE_NONE;
		}
	}
}


//Initialize a player object
//Returns 1 if ok, 0 if error
int ObjInitPlayer(object* objp)
{
	ship* ship;
	int ret = 1;
	ASSERT(objp->type == OBJ_PLAYER);
	objp->shields = INITIAL_SHIELDS;
	ship = &Ships[Players[objp->id].ship_index];
	if (!ship->used)
	{
		//this ship doesn't exist
		int new_ship;
		Int3();
		ret = 0;
		new_ship = GetNextShip(0);
		ASSERT(new_ship != -1);
		Players[objp->id].ship_index = new_ship;
		ship = &Ships[Players[objp->id].ship_index];
	}

	//Set up render info
	ObjSetRenderPolyobj(objp, ship->model_handle, ship->dying_model_handle);
	ASSERT(Poly_models[ship->model_handle].n_attach >= 3);
	objp->lighting_render_type = LRT_GOURAUD;
	objp->lm_object.used = 0;

	// Page in those models
	PageInPolymodel(ship->model_handle, OBJ_PLAYER, &ship->size);
	if (ship->dying_model_handle >= 0)
		PageInPolymodel(ship->dying_model_handle);
	if (ship->lo_render_handle >= 0)
		PageInPolymodel(ship->lo_render_handle);
	if (ship->med_render_handle >= 0)
		PageInPolymodel(ship->med_render_handle);

	//Set size
	objp->size = ship->size;

	//Set up control info.  The player flies, other players do nothing
	// MUST be after paging in polymodel cause SetObjectControlType does some
	// stuff with the object's polymodel.
	if (objp->id == Player_num)
	{
		SetObjectControlType(objp, CT_FLYING);
		objp->movement_type = MT_PHYSICS;
		objp->mtype.phys_info = ship->phys_info;
		Player_object = objp;
	}
	else
	{
		SetObjectControlType(objp, CT_NONE);
		objp->movement_type = MT_NONE;
		objp->mtype.phys_info = ship->phys_info;
		objp->mtype.phys_info.flags = PF_FIXED_VELOCITY;
	}

	//Set up physics info
	//These are always set for a player
	objp->mtype.phys_info.num_bounces = PHYSICS_UNLIMITED_BOUNCE;
	if (objp->dynamic_wb == NULL)
		objp->dynamic_wb = (dynamic_wb_info*)mem_malloc(sizeof(dynamic_wb_info) * MAX_WBS_PER_OBJ);
	
	WBClearInfo(objp);
	// Set a few misc things
	Players[objp->id].team = objp->id % 2;
	ObjCreateEffectInfo(objp);
	objp->effect_info->type_flags = EF_VOLUME_LIT;

	return ret;
}


//Initialize a generic object (robot, powerup, building, etc.)
//Returns 1 if ok, 0 if error
int ObjInitGeneric(object* objp, bool reinit)
{
	object_info* obj_info;
	int ret = 1;
	float r_val = (float)ps_rand() / (float)RAND_MAX;
	if ((objp->id < 0) || (objp->id >= MAX_OBJECT_IDS))
	{
		Int3();
		return 0;
	}
	obj_info = &Object_info[objp->id];
	//Deal with deleted type
	if (obj_info->type == OBJ_NONE)
	{
		int i;
		for (i = 0, obj_info = Object_info; i < MAX_OBJECT_IDS; i++, obj_info)
		{
			//find other object of same type
			if (Object_info[i].type == objp->type)
				break;
		}
		ASSERT(i < MAX_OBJECT_IDS);		//There should (in real life) always be at least one of each type
#ifdef EDITOR
		if (GetFunctionMode() == EDITOR_MODE)
			OutrageMessageBox("Object %d (\"%s\") had ID %d which no longer exists.  Changing to %d, \"%s\".", OBJNUM(objp), objp->name ? objp->name : "<no name>", objp->id, i, obj_info->name);
#endif
	}
	if (obj_info->type != objp->type)
	{
#ifdef EDITOR
		if (GetFunctionMode() == EDITOR_MODE)
			OutrageMessageBox("Object %d (\"%s\"), type name \"%s\", changed from type %s to %s", OBJNUM(objp), objp->name ? objp->name : "<no name>", obj_info->name, Object_type_names[objp->type], Object_type_names[obj_info->type]);
#endif
		objp->type = obj_info->type;
	}

	//Set size & shields
	objp->shields = obj_info->hit_points;
	//Set impact stuff for non-weapons
	objp->impact_size = obj_info->impact_size;
	objp->impact_time = obj_info->impact_time;
	objp->impact_player_damage = obj_info->damage;
	objp->impact_generic_damage = obj_info->damage;
	objp->impact_force = obj_info->damage * 50.0;

	//Set flags
	if (obj_info->flags & OIF_DESTROYABLE)
		objp->flags |= OF_DESTROYABLE;
	if (obj_info->flags & OIF_AI_SCRIPTED_DEATH)
		objp->flags |= OF_AI_DO_DEATH;
	if (obj_info->flags & OIF_DO_CEILING_CHECK)
		objp->flags |= OF_FORCE_CEILING_CHECK;

	//Set up movement info
	if (obj_info->flags & OIF_USES_PHYSICS)
	{
		objp->movement_type = MT_PHYSICS;
		// Setup some physics things
		objp->mtype.phys_info = obj_info->phys_info;	// Set the initial velocity
		//Warn about initial velocity & rotvel
		if (obj_info->phys_info.velocity.z > 0.0)
			Int3();	//Warning: This object has an initial velocity.  This is not supported. 
		//If your object does not need an initial velocity, set it to zero on 
		//the page.  If you do need an initial velocity, someone will have to
		//add code to deal with it, perhaps here or perhaps at level start.
	}
	else
		objp->movement_type = MT_NONE;

	//Set up render info
	ObjSetRenderPolyobj(objp, obj_info->render_handle);
	PageInPolymodel(obj_info->render_handle, objp->type, &obj_info->size);
	objp->size = obj_info->size;

	//Page in low-res models
	if (obj_info->lo_render_handle != -1)
		PageInPolymodel(obj_info->lo_render_handle);
	if (obj_info->med_render_handle != -1)
		PageInPolymodel(obj_info->med_render_handle);

	//Set anim frame
	if (Object_info[objp->id].anim)
	{
		objp->rtype.pobj_info.anim_frame = ((1.0f - r_val) * (float)Object_info[objp->id].anim[MC_STANDING].elem[AS_ALERT].from) +
			(r_val * (float)Object_info[objp->id].anim[MC_STANDING].elem[AS_ALERT].to);
	}
	else
	{
		objp->rtype.pobj_info.anim_frame = 0.0f;
	}

	//Set up control info (must be after wb stuff)
	if (obj_info->ai_info && obj_info->flags & OIF_CONTROL_AI) //DAJ 12/18/99 added check for ai_info 
	{
		SetObjectControlType(objp, CT_AI);
		AIInit(objp, obj_info->ai_info->ai_class, obj_info->ai_info->ai_type, obj_info->ai_info->movement_type);
	}
	else
		SetObjectControlType(objp, CT_NONE);

	if (objp->control_type == CT_AI)
	{
		poly_model* pm = &Poly_models[objp->rtype.pobj_info.model_num];
		int num_wbs = pm->num_wbs;

		if ((objp->dynamic_wb != NULL) && ((unsigned int)num_wbs != mem_size(objp->dynamic_wb) / sizeof(dynamic_wb_info)))
		{
			mem_free(objp->dynamic_wb);
			objp->dynamic_wb = NULL;
		}
		if ((objp->dynamic_wb == NULL) && num_wbs)
		{
			objp->dynamic_wb = (dynamic_wb_info*)mem_malloc(sizeof(dynamic_wb_info) * num_wbs);
		}

		//Setup the weapon batteries (must be after polymodel stuff)
		WBClearInfo(objp);
	}

	objp->lighting_render_type = obj_info->lighting_info.lighting_render_type;
	if (!reinit)
		objp->lm_object.used = 0;

	// Allocate effect memory
	ObjCreateEffectInfo(objp);

	if (objp->type != OBJ_POWERUP)
		objp->effect_info->type_flags = EF_VOLUME_LIT;
	if (objp->movement_type == MT_PHYSICS)
	{
		if (objp->mtype.phys_info.rotdrag < 60.0f)
			objp->mtype.phys_info.rotdrag = 60.0;   // CHRISHACK - MAKES SURE ROBOTS DONT SPIN FOREVER
	}

	//Set ammo amounts for powerups
	if (objp->type == OBJ_POWERUP)
	{
		if (objp->control_type == CT_NONE)
		{
			//some powerups have AI; don't mess with them
			SetObjectControlType(objp, CT_POWERUP);
			objp->ctype.powerup_info.count = obj_info->ammo_count;
		}
	}
	// Clear wiggle flag to supersede those pesky designer errors!
	objp->mtype.phys_info.flags &= ~PF_WIGGLE;
	return ret;
}


int ObjInitDebris(object* objp)
{
	ASSERT(objp->type == OBJ_DEBRIS);
	objp->movement_type = MT_PHYSICS;
	SetObjectControlType(objp, CT_DEBRIS);
	objp->lifeleft = DEBRIS_LIFE;
	objp->flags |= OF_USES_LIFELEFT;
	objp->ctype.debris_info.death_flags = 0;

	//Set up render info
	ObjSetRenderPolyobj(objp, -1);
	objp->lighting_render_type = LRT_GOURAUD;
	objp->lm_object.used = 0;

	// Allocate effect memory
	ObjCreateEffectInfo(objp);

	objp->effect_info->type_flags = EF_VOLUME_LIT;

	return 1;
}


int ObjInitShard(object* objp)
{
	ASSERT(objp->type == OBJ_SHARD);
	objp->movement_type = MT_PHYSICS;
	SetObjectControlType(objp, CT_NONE);
	objp->lifeleft = DEBRIS_LIFE * 5;
	objp->flags |= OF_USES_LIFELEFT;

	//Set physics info
	objp->mtype.phys_info.flags = PF_GRAVITY + PF_BOUNCE;
	objp->mtype.phys_info.mass = 1.0;
	objp->mtype.phys_info.drag = 0.0001f;
	objp->mtype.phys_info.coeff_restitution = 0.3f;
	objp->mtype.phys_info.num_bounces = 0;

	//Set up render info
	objp->render_type = RT_SHARD;
	return 1;
}


#ifdef _DEBUG
int ObjInitLine(object* objp)
{
	ASSERT(objp->type == OBJ_DEBUG_LINE);
	objp->movement_type = MT_NONE;
	SetObjectControlType(objp, CT_NONE);
	objp->render_type = RT_LINE;
	return 1;
}
#endif


//Initialize a camera
//Returns 1 if ok, 0 if error
int ObjInitCamera(object* objp)
{
	//Set size & shields
	objp->size = 0.5;
	objp->shields = 0;
	//Set up various info
	SetObjectControlType(objp, CT_NONE);
	objp->movement_type = MT_NONE;
	objp->lm_object.used = 0;
#ifdef EDITOR
	objp->render_type = RT_EDITOR_SPHERE;		//draw cameras as spheres in the editor
	objp->rtype.sphere_color = GR_RGB(255, 255, 0);
#else
	objp->render_type = RT_NONE;
#endif
	return 1;
}


//Initialize a sound object
//Returns 1 if ok, 0 if error
int ObjInitSoundSource(object* objp)
{
	//Set size & shields
	objp->size = 0.5;
	objp->shields = 0;
	//Set up various info
	SetObjectControlType(objp, CT_SOUNDSOURCE);
	objp->movement_type = MT_NONE;
	objp->lm_object.used = 0;
#ifdef EDITOR
	objp->render_type = RT_EDITOR_SPHERE;		//draw cameras as spheres in the editor
	objp->rtype.sphere_color = GR_RGB(0, 255, 0);
#else
	objp->render_type = RT_NONE;
#endif
	return 1;
}


//Initialize a waypoint object
//Returns 1 if ok, 0 if error
int ObjInitWaypoint(object* objp)
{
	//Set size & shields
	objp->size = 0.5;
	objp->shields = 0;
	//Set up various info
	SetObjectControlType(objp, CT_NONE);
	objp->movement_type = MT_NONE;
	objp->lm_object.used = 0;
#ifdef EDITOR
	objp->render_type = RT_EDITOR_SPHERE;		//draw cameras as spheres in the editor
	objp->rtype.sphere_color = GR_RGB(255, 130, 33);
#else
	objp->render_type = RT_NONE;
#endif
	return 1;
}


//Initialize an editor viewer object
//Returns 1 if ok, 0 if error
int ObjInitViewer(object* objp)
{
	//Set size & shields
	objp->size = 5.0;
	objp->shields = 0;
	//Set up various info
	SetObjectControlType(objp, CT_NONE);
	objp->movement_type = MT_NONE;
	objp->render_type = RT_NONE;
	objp->lm_object.used = 0;
	return 1;
}


//Initialize a weapon
//Returns 1 if ok, 0 if error
int ObjInitWeapon(object* objp)
{
	weapon* weap;
	int ret = 1;
	ASSERT(objp->type == OBJ_WEAPON);

	if (objp->id < 0 || objp->id >= MAX_WEAPONS)
		return 0;

	weap = &Weapons[objp->id];
	ASSERT(weap->used > 0);
	//Set up shields
	objp->shields = 0;
	//Set up control info
	SetObjectControlType(objp, CT_WEAPON);
	// Not tracking anything
	objp->ctype.laser_info.track_handle = OBJECT_HANDLE_NONE;
	//Set up physics info
	objp->movement_type = MT_PHYSICS;
	objp->mtype.phys_info.mass = weap->phys_info.mass;
	objp->mtype.phys_info.drag = weap->phys_info.drag;
	objp->mtype.phys_info.rotdrag = weap->phys_info.rotdrag;
	objp->mtype.phys_info.flags = weap->phys_info.flags;

	objp->mtype.phys_info.full_rotthrust = weap->phys_info.full_rotthrust;
	objp->mtype.phys_info.full_thrust = weap->phys_info.full_thrust;
	objp->mtype.phys_info.rotvel = weap->phys_info.rotvel;
	//objp->mtype.phys_info.rot_thrust = weap->phys_info.full_rot_thrust;
	objp->mtype.phys_info.wiggle_amplitude = weap->phys_info.wiggle_amplitude;
	objp->mtype.phys_info.wiggles_per_sec = weap->phys_info.wiggles_per_sec;
	objp->mtype.phys_info.num_bounces = weap->phys_info.num_bounces;
	objp->mtype.phys_info.coeff_restitution = weap->phys_info.coeff_restitution;
	objp->lifeleft = weap->life_time;
	objp->lifetime = weap->life_time;
	objp->ctype.laser_info.thrust_left = weap->thrust_time;
	objp->ctype.laser_info.last_drop_time = 0;
	objp->mtype.phys_info.hit_die_dot = weap->phys_info.hit_die_dot;
	//Set impact stuff
	objp->impact_size = weap->impact_size;
	objp->impact_time = weap->impact_time;
	objp->impact_player_damage = weap->impact_player_damage;
	objp->impact_generic_damage = weap->impact_generic_damage;
	objp->impact_force = weap->impact_force;
	// Set up a few flags
	objp->mtype.phys_info.flags |= PF_NO_COLLIDE_PARENT;
	objp->flags |= OF_USES_LIFELEFT;
	// Set up rendering info
	objp->render_type = RT_WEAPON;

	if (!((weap->flags & WF_IMAGE_BITMAP) || (weap->flags & WF_IMAGE_VCLIP)))
	{
		objp->rtype.pobj_info.model_num = weap->fire_image_handle;
		objp->rtype.pobj_info.dying_model_num = -1;
		PageInPolymodel(weap->fire_image_handle, OBJ_WEAPON, &weap->size);

		objp->rtype.pobj_info.subobj_flags = 0xFFFFFFFF;
		objp->flags |= OF_POLYGON_OBJECT;
	}
	else if (weap->flags & WF_IMAGE_VCLIP)
	{
		PageInVClip(weap->fire_image_handle);
	}

	objp->size = weap->size;
	objp->ctype.laser_info.hit_status = WPC_NOT_USED;
	if (weap->flags & WF_CUSTOM_SIZE)
	{
		objp->size = weap->custom_size;
	}
	objp->lighting_render_type = LRT_STATIC;
	objp->lm_object.used = 0;

	return ret;
}


int ObjInitFireball(object* objp)
{
	int ret = 1;
	ASSERT(objp->type == OBJ_FIREBALL);
	//Set size & shields
	objp->shields = 0;
	objp->size = 0;
	//Set up control info
	SetObjectControlType(objp, CT_EXPLOSION);
	//Set up physics info
	objp->movement_type = MT_NONE;
	objp->mtype.phys_info.num_bounces = PHYSICS_UNLIMITED_BOUNCE;

	//Set up render info
	objp->render_type = RT_FIREBALL;
	objp->size = Fireballs[objp->id].size;
	objp->flags |= OF_USES_LIFELEFT;
	objp->lifeleft = Fireballs[objp->id].total_life;
	objp->lighting_render_type = LRT_STATIC;
	objp->lm_object.used = 0;
	return ret;
}


int ObjInitMarker(object* objp)
{
	int ret = 1;
	static int first = 1;
	static int polynum = -1;
	ASSERT(objp->type == OBJ_MARKER);
	//Set size & shields
	objp->shields = 100;
	objp->size = 2.0;
	//Set up control info
	SetObjectControlType(objp, CT_NONE);
	objp->movement_type = MT_NONE;
	ObjSetRenderPolyobj(objp, Marker_polynum);
	PageInPolymodel(Marker_polynum, OBJ_MARKER, &objp->size);
	objp->lm_object.used = 0;
	objp->lighting_render_type = LRT_STATIC;
	return ret;
}


int ObjInitDoor(object* objp, bool reinit)
{
	//Set up movement info
	objp->movement_type = MT_NONE;
	SetObjectControlType(objp, CT_NONE);
	//Set up render info
	ObjSetRenderPolyobj(objp, GetDoorImage(objp->id));
	PageInPolymodel(objp->rtype.pobj_info.model_num);
	ComputeDefaultSize(OBJ_DOOR, objp->rtype.pobj_info.model_num, &objp->size);
	objp->lighting_render_type = LRT_LIGHTMAPS;
	//Set shields
	if (Doors[objp->id].flags & DF_BLASTABLE)
	{
		objp->flags |= OF_DESTROYABLE;
		objp->shields = Doors[objp->id].hit_points;
	}
	if (!reinit)
		objp->lm_object.used = 0;

	return 1;
}


int ObjInitRoom(object* objp)
{
	//Set up movement info
	objp->movement_type = MT_NONE;
	SetObjectControlType(objp, CT_NONE);
	//Set up render info
	objp->render_type = RT_ROOM;		//rendering handled as special case
	//I have no idea about this stuff
	objp->lighting_render_type = LRT_STATIC;
	objp->lm_object.used = 0;
	objp->flags |= OF_POLYGON_OBJECT;

	return 1;
}


//Set all the type-specific info for this object
//Returns 1 if ok, 0 if error
int ObjInitTypeSpecific(object* objp, bool reinitializing)
{
	switch (objp->type)
	{
	case OBJ_CLUTTER:
	case OBJ_BUILDING:
	case OBJ_ROBOT:
	case OBJ_POWERUP:		return ObjInitGeneric(objp, reinitializing);		break;
	case OBJ_SHOCKWAVE:		return 1;								break;
	case OBJ_PLAYER:		return ObjInitPlayer(objp);		break;
	case OBJ_CAMERA:		return ObjInitCamera(objp);		break;
	case OBJ_MARKER:		return ObjInitMarker(objp);		break;
	case OBJ_VIEWER:		return ObjInitViewer(objp);		break;
	case OBJ_WEAPON:		return ObjInitWeapon(objp);		break;
	case OBJ_FIREBALL:		return ObjInitFireball(objp);		break;
	case OBJ_DOOR:			return ObjInitDoor(objp, reinitializing);			break;
	case OBJ_ROOM:			return ObjInitRoom(objp);			break;
	case OBJ_DEBRIS:		return ObjInitDebris(objp);		break;
	case OBJ_SHARD:			return ObjInitShard(objp);			break;
	case OBJ_SOUNDSOURCE:	return ObjInitSoundSource(objp);	break;
	case OBJ_WAYPOINT:		return ObjInitWaypoint(objp);		break;
	case OBJ_SPLINTER:		return 1;						break;
#ifdef _DEBUG
	case OBJ_DEBUG_LINE:	return ObjInitLine(objp);			break;
#endif		
		break;
	default:
		Int3();
		return 0;
	}
}


//Initializes a new object.  All fields not passed in set to defaults.
//Returns 1 if ok, 0 if error
int ObjInit(object* objp, int type, int id, int handle, vector* pos, int roomnum, float creation_time, int parent_handle)
{
	//Zero out object structure to keep weird bugs from happening in uninitialized fields.
	//I hate doing this because it seems sloppy, but it's probably better to do it
	memset(objp, 0, sizeof(object));

	//Set the stuff that's passed in
	objp->type = type;
	objp->id = id;
	objp->handle = handle;
	objp->pos = objp->last_pos = *pos;
	objp->parent_handle = parent_handle;
	objp->creation_time = creation_time;
	objp->osiris_script = NULL;

	//Initialize some general stuff
	objp->roomnum = roomnum;
	objp->orient = Identity_matrix;
	objp->next = objp->prev = -1;
	objp->dummy_type = OBJ_NONE;
	objp->flags = 0;
	objp->size = 0;
	objp->change_flags = 0;
	objp->generic_nonvis_flags = 0;
	objp->generic_sent_nonvis = 0;
	objp->name = NULL;
	objp->custom_default_script_name = NULL;
	objp->custom_default_module_name = NULL;
	objp->contains_type = -1;
	objp->lifeleft = 0;
	objp->effect_info = NULL;
	objp->ai_info = NULL;
	objp->dynamic_wb = NULL;
	objp->attach_children = NULL;
	//Now initialize the type-specific data

	int res = ObjInitTypeSpecific(objp, 0);
	objp->roomnum = -1; //[ISB] Make sure it appears unlinked
	return res;
}

//Re-copies data to each object from the appropriate page for that object type.
//Called after an object page has changed.
void ObjReInitAll()
{
	int objnum;
	object* objp;
	for (objnum = 0, objp = Objects; objnum <= Highest_object_index; objnum++, objp++)
		if (objp->type != OBJ_NONE)
			ObjInitTypeSpecific(objp, 1);
}
