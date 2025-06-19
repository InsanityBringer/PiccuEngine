/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */
 


#include "stdafx.h"
#include "globals.h"
#include "polymodel.h"
#include "terrain.h"
#include "ship.h"
#include "FindIntersection.h"
#include "object_lighting.h"

#include "ned_Tablefile.h"
#include "ned_Object.h"
#include "ned_Door.h"

#include "robotfire.h"
#include "fireball.h"
#include "sounds.h"
#include "physics.h"
#include "psrand.h"

//Misc

#include "player.h"
extern player Players[MAX_PLAYERS];

//Data for objects

// -- Object stuff
#include "object.h"
#include "objinit.h"

#define DEFAULT_OBJECT_SIZE	 4.0f
#define DEFAULT_OBJECT_MASS	 1.0f
#define DEFAULT_OBJECT_DRAG	 0.1f
#define DEFAULT_OBJECT_ROTDRAG 0.01f

//info on the various types of objects

extern int Insert_mode;

object Objects[MAX_OBJECTS];
ned_object_info		Object_info[MAX_OBJECT_IDS];
int Num_objects					= 0;

int Highest_object_index=0;
int Highest_ever_object_index=0;

int print_object_info = 1;

//This array matches the object types in object.h
char	*Object_type_names[MAX_OBJECT_TYPES] = {
	"WALL",			//OBJ_WALL				0
	"FIREBALL",		//OBJ_FIREBALL			1
	"ROBOT",			//OBJ_ROBOT				2
	"SHARD",			//OBJ_SHARD				3
	"PLAYER",		//OBJ_PLAYER			4
	"WEAPON",		//OBJ_WEAPON			5
	"VIEWER",		//OBJ_VIEWER			6
	"POWERUP",		//OBJ_POWERUP			7
	"DEBRIS",		//OBJ_DEBRIS			8
	"CAMERA",		//OBJ_CAMERA			9
	"SHOCKWV",		//OBJ_SHOCKWAVE		10
	"CLUTTER",		//OBJ_CLUTTER			11
	"GHOST",			//OBJ_GHOST				12
	"LIGHT",			//OBJ_LIGHT				13
	"COOP",			//OBJ_COOP				14
	"UNUSED",		//OBJ_MARKER			15
	"BUILDING",		//OBJ_BUILDING			16
	"DOOR",			//OBJ_DOOR				17
	"ROOM",			//OBJ_ROOM				18
	"LINE",			//OBJ_PARTICLE			19
	"SPLINTER",		//OBJ_SPLINTER			20
	"DUMMY",			//OBJ_DUMMY				21
	"OBSERVER",		//OBJ_OBSERVER			22
	"DEBUG LINE",	//OBJ_DEBUG_LINE		23
	"SOUNDSOURCE",	//OBJ_SOUNDSOURCE		24
	"WAYPOINT",		//OBJ_WAYPOINT			25
};

int Num_big_objects = 0;
short BigObjectList[MAX_BIG_OBJECTS];

struct object *  Player_object;

static short free_obj_list[MAX_OBJECTS];

//Objinfo
#include "objinfo.h"

/*
//The array with information for robots, powerups, buildings, etc.
object_info Object_info[MAX_OBJECT_IDS];
*/

//The number of ids of each type in the list
int Num_object_ids[MAX_OBJECT_TYPES];

//Allocate and initialize an effect_info struct for an object
void ObjCreateEffectInfo(object *objp)
{
	if (objp->effect_info)
		mem_free (objp->effect_info);	
	
	objp->effect_info=(effect_info_s *)mem_malloc (sizeof(effect_info_s));
	memset (objp->effect_info,0,sizeof(effect_info_s));
	ASSERT (objp->effect_info);

	objp->effect_info->sound_handle = SOUND_NONE_INDEX;
}


void ObjSetAABB(object *obj)
{
	vector object_rad;

	if(obj->type == OBJ_ROOM)
	{
		obj->min_xyz = Rooms[obj->id].min_xyz;
		obj->max_xyz = Rooms[obj->id].max_xyz;
	}
	else if(obj->flags & OF_POLYGON_OBJECT &&
		obj->type != OBJ_WEAPON && 
		obj->type != OBJ_DEBRIS &&
		obj->type != OBJ_POWERUP &&
		obj->type != OBJ_PLAYER)
	{
		vector offset_pos;

		object_rad.x = object_rad.y = object_rad.z = Poly_models[obj->rtype.pobj_info.model_num].anim_size;
		offset_pos = obj->pos + obj->anim_sphere_offset;

		obj->min_xyz =  offset_pos - object_rad;
		obj->max_xyz = offset_pos + object_rad;
	}
	else
	{
		object_rad.x = obj->size;
		object_rad.y = obj->size;
		object_rad.z = obj->size;

		obj->min_xyz = obj->pos - object_rad;

		obj->max_xyz = obj->pos + object_rad;
	}
}


//sets the orientation of an object.  This should be called to orient an object
void ObjSetOrient(object *obj,const matrix *orient)
{
	// Accounts for if the orientation was set and then this function is being used
	// to update the other stuff
	if(&obj->orient != orient)
		obj->orient = *orient;

	// Recompute the orientation dependant information
	if(obj->flags & OF_POLYGON_OBJECT)
	{
		if(obj->type != OBJ_WEAPON && 
			obj->type != OBJ_DEBRIS &&
			obj->type != OBJ_POWERUP &&
			obj->type != OBJ_ROOM)
		{
			matrix m;

			m = obj->orient;
			vm_TransposeMatrix(&m);

			obj->wall_sphere_offset = Poly_models[obj->rtype.pobj_info.model_num].wall_size_offset * m;			
			obj->anim_sphere_offset = Poly_models[obj->rtype.pobj_info.model_num].anim_size_offset * m;
		}
		else
		{
			obj->wall_sphere_offset = Zero_vector;			
			obj->anim_sphere_offset = Zero_vector;
		}
	} 
}


//Resets the object list: sets all objects to unused, intializes handles, & sets roomnums to -1
//Called by the editor to init a new level.
void ResetObjectList()
{
	int i;

	//Init data for each object
	for (i=0;i<MAX_OBJECTS;i++) {
		Objects[i].handle = i;
		Objects[i].type = OBJ_NONE;
		Objects[i].roomnum = -1;
	}

	//Build the free object list
	ResetFreeObjects();

	//Say no big objects
	InitBigObjects();
}

extern void CollideInit();

//sets up the free list & init player & whatever else
void InitObjects()
{

	int i;

	for(i=0;i<MAX_OBJECTS;i++)
	{
		Objects[i].next = -1;
		Objects[i].prev = -1;
	}

	for (i=0;i<TERRAIN_WIDTH*TERRAIN_DEPTH;i++)
	{
		Terrain_seg[i].objects = -1;	
	}

	//Make sure no rooms are using any objects
	for (i=0;i<MAX_ROOMS;i++)
	{
		Rooms[i].objects = -1;
		Rooms[i].vis_effects=-1;
	}	

	//Initialize the collision system
	CollideInit();	

	//Mark all the objects as unused
	ResetObjectList();

	//Make sure no rooms are using any objects
	for (i=0;i<MAX_ROOMS;i++)
	{
		Rooms[i].objects = -1;
		Rooms[i].vis_effects=-1;
	}

//	InitVisEffects ();

	atexit(FreeAllObjects);
}


void ObjSetRenderPolyobj(object *objp,int model_num,int dying_model_num=-1);

//Initialize the polygon object information for an object
void ObjSetRenderPolyobj(object *objp,int model_num,int dying_model_num)
{
	objp->render_type = RT_POLYOBJ;
	objp->flags |= OF_POLYGON_OBJECT;

	polyobj_info *p_info = &objp->rtype.pobj_info;

	p_info->model_num = model_num;
	p_info->dying_model_num = dying_model_num;

	p_info->anim_frame = 0.0f;
	p_info->tmap_override = -1;
	p_info->subobj_flags = 0xFFFFFFFF;

	p_info->multi_turret_info.num_turrets = 0;
	p_info->multi_turret_info.keyframes = NULL;
	p_info->multi_turret_info.last_keyframes = NULL;

	//Initialize attach slots
	if (model_num != -1) {
		poly_model *pm = GetPolymodelPointer(model_num);		//get also pages in
		if ((objp->attach_children == NULL) && pm->n_attach) {
			objp->attach_children = (int *) mem_malloc(sizeof(int) * pm->n_attach);
			for(int i = 0; i < pm->n_attach; i++)
				objp->attach_children[i] = OBJECT_HANDLE_NONE;
		}
	}
}


//Initialize a player object
//Returns 1 if ok, 0 if error
int ObjInitPlayer(object *objp)
{
//	ship *ship;
	int ret=1;

	ASSERT(objp->type == OBJ_PLAYER);

	objp->shields = INITIAL_SHIELDS;

	// Do this so player ships will draw in NEWEDITOR
	objp->render_type = RT_POLYOBJ;
	objp->flags |= OF_POLYGON_OBJECT;

	// We don't read in ship pages in NEWEDITOR (yet), so this stuff is commented out.
/*
	ship  = &Ships[Players[objp->id].ship_index];

	if (! ship->used)	{		//this ship doesn't exist
		int new_ship;
		Int3();
		ret = 0;
		new_ship = GetNextShip(0);
		ASSERT(new_ship != -1);
		Players[objp->id].ship_index = new_ship;
		ship  = &Ships[Players[objp->id].ship_index];
	}

	//Set up render info
	ObjSetRenderPolyobj(objp,ship->model_handle,ship->dying_model_handle);

	ASSERT(Poly_models[ship->model_handle].n_attach >= 3);
	objp->lighting_render_type=LRT_GOURAUD;
	objp->lm_object.used=0;

	// Page in those models
	PageInPolymodel (ship->model_handle, OBJ_PLAYER, &ship->size);
	if (ship->dying_model_handle>=0)
		PageInPolymodel (ship->dying_model_handle);

	if (ship->lo_render_handle>=0)
		PageInPolymodel (ship->lo_render_handle);
	if (ship->med_render_handle>=0)
		PageInPolymodel (ship->med_render_handle);


	//Set size
	objp->size = ship->size;

 	//Set up control info.  The player flies, other players do nothing
	// MUST be after paging in polymodel cause SetObjectControlType does some
	// stuff with the object's polymodel.
	if (objp->id == Player_num) {
		SetObjectControlType(objp, CT_FLYING);
		objp->movement_type = MT_PHYSICS;
		objp->mtype.phys_info = ship->phys_info;
		Player_object=objp;
	}
	else {
		SetObjectControlType(objp, CT_NONE);
		objp->movement_type = MT_NONE;
		objp->mtype.phys_info = ship->phys_info;
		objp->mtype.phys_info.flags = PF_FIXED_VELOCITY;
	}

	//Set up physics info
	//These are always set for a player
	objp->mtype.phys_info.num_bounces = PHYSICS_UNLIMITED_BOUNCE;

	if(objp->dynamic_wb == NULL)
	{
		objp->dynamic_wb = (dynamic_wb_info *) mem_malloc(sizeof(dynamic_wb_info) * MAX_WBS_PER_OBJ);
	}

	WBClearInfo(objp);

	// Set a few misc things
	Players[objp->id].team=objp->id%2;

	ObjCreateEffectInfo(objp);
	objp->effect_info->type_flags=EF_VOLUME_LIT;
	
	return ret;
*/
	objp->size = DEFAULT_OBJECT_SIZE;
	return 1;
}

//Initialize a generic object (robot, powerup, building, etc.)
//Returns 1 if ok, 0 if error
int ObjInitGeneric(object *objp,bool reinit)
{
	ned_object_info *obj_info;
	int ret=1;
	float r_val = (float)ps_rand()/(float)RAND_MAX;

	if ((objp->id < 0) || (objp->id >= MAX_OBJECT_IDS)) {
		Int3();
		return 0;
	}

	obj_info  = &Object_info[objp->id];

	//Deal with deleted type
	if (obj_info->type == OBJ_NONE) {
		int i;
		for (i=0,obj_info=Object_info;i<MAX_OBJECT_IDS;i++,obj_info) {		//find other object of same type
			if (Object_info[i].type == objp->type)
				break;
		}
		ASSERT(i < MAX_OBJECT_IDS);		//There should (in real life) always be at least one of each type
		#if (defined(EDITOR) || defined(NEWEDITOR))
		if (GetFunctionMode() == EDITOR_MODE)
			OutrageMessageBox("Object %d (\"%s\") had ID %d which no longer exists.  Changing to %d, \"%s\".",OBJNUM(objp),objp->name?objp->name:"<no name>",objp->id,i,obj_info->name);
		#endif
	}

	if (obj_info->type != objp->type) {
		#if (defined(EDITOR) || defined(NEWEDITOR))
		if (GetFunctionMode() == EDITOR_MODE)
			OutrageMessageBox("Object %d (\"%s\"), type name \"%s\", changed from type %s to %s",OBJNUM(objp),objp->name?objp->name:"<no name>",obj_info->name,Object_type_names[objp->type],Object_type_names[obj_info->type]);
		#endif
		objp->type = obj_info->type;
	}

	//Set size & shields
	objp->shields = obj_info->hit_points;

/*
	//Set impact stuff for non-weapons
	objp->impact_size = obj_info->impact_size;
	objp->impact_time = obj_info->impact_time;
	objp->impact_player_damage = obj_info->damage;
	objp->impact_generic_damage = obj_info->damage;
	objp->impact_force = obj_info->damage * 50.0;
*/

	//Set flags
	if (obj_info->flags & OIF_DESTROYABLE)
		objp->flags |= OF_DESTROYABLE;

	if (obj_info->flags & OIF_AI_SCRIPTED_DEATH)
		objp->flags |= OF_AI_DO_DEATH;

	if (obj_info->flags & OIF_DO_CEILING_CHECK)
		objp->flags |= OF_FORCE_CEILING_CHECK;

/*
	//Set up movement info
	if (obj_info->flags & OIF_USES_PHYSICS) {
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

*/
	//Set up render info
	if (obj_info->render_handle==-1)
		Object_info[objp->id].render_handle = LoadPolyModel(Object_info[objp->id].image_filename,0);
	ObjSetRenderPolyobj(objp,obj_info->render_handle);

	if (obj_info->render_handle != -1)
		PageInPolymodel (obj_info->render_handle, objp->type, &obj_info->size);

	objp->size = obj_info->size;

/*
	//Page in low-res models
	if (obj_info->lo_render_handle!=-1)
		PageInPolymodel (obj_info->lo_render_handle);
	if (obj_info->med_render_handle!=-1)
		PageInPolymodel (obj_info->med_render_handle);

	//Set anim frame
	objp->rtype.pobj_info.anim_frame = ((1.0f - r_val) * (float)Object_info[objp->id].anim[MC_STANDING][AS_ALERT].from) +
		                                (r_val * (float)Object_info[objp->id].anim[MC_STANDING][AS_ALERT].to);
*/

	//Set up control info (must be after wb stuff)
	if(obj_info->flags & OIF_CONTROL_AI) 
	{
		SetObjectControlType(objp, CT_AI);	
//		AIInit(objp, obj_info->ai_info.ai_class, obj_info->ai_info.ai_type, obj_info->ai_info.movement_type);
	}
	else
		SetObjectControlType(objp, CT_NONE);

	if(objp->control_type == CT_AI)
	{
		poly_model *pm=&Poly_models[objp->rtype.pobj_info.model_num];
		int num_wbs = pm->num_wbs;

		if((objp->dynamic_wb != NULL) && ((unsigned int)num_wbs != mem_size(objp->dynamic_wb)/sizeof(dynamic_wb_info)))
		{
			mem_free(objp->dynamic_wb);
			objp->dynamic_wb = NULL;
		}

		if((objp->dynamic_wb == NULL) && num_wbs)
		{
			objp->dynamic_wb = (dynamic_wb_info *) mem_malloc(sizeof(dynamic_wb_info) * num_wbs);
		}

		//Setup the weapon batteries (must be after polymodel stuff)
//		WBClearInfo(objp);
	}

	objp->lighting_render_type=obj_info->lighting_info.lighting_render_type;
	
	if (!reinit)
		objp->lm_object.used=0;

	// Allocate effect memory
	ObjCreateEffectInfo(objp);
	
	if (objp->type == OBJ_ROBOT || objp->type == OBJ_BUILDING || objp->type==OBJ_CLUTTER)
	{
		objp->effect_info->type_flags=EF_VOLUME_LIT;
		if(objp->movement_type == MT_PHYSICS)
		{
			if(objp->mtype.phys_info.rotdrag < 60.0f)
				objp->mtype.phys_info.rotdrag = 60.0;   // CHRISHACK - MAKES SURE ROBOTS DONT SPIN FOREVER
		}
	}

/*
	//Set ammo amounts for powerups
	if (objp->type == OBJ_POWERUP) {
		if (objp->control_type == CT_NONE) {		//some powerups have AI; don't mess with them
			SetObjectControlType(objp, CT_POWERUP);
			objp->ctype.powerup_info.count = obj_info->ammo_count;
		}
	}
*/

	return ret;
}


int ObjInitDebris(object *objp)
{
	ASSERT(objp->type == OBJ_DEBRIS);

	objp->movement_type = MT_PHYSICS;
	SetObjectControlType(objp, CT_DEBRIS);

	objp->lifeleft = DEBRIS_LIFE;
	objp->flags |= OF_USES_LIFELEFT;
	objp->ctype.debris_info.death_flags = 0;

	//Set up render info
	ObjSetRenderPolyobj(objp,-1);
	objp->lighting_render_type=LRT_GOURAUD;
	objp->lm_object.used=0;

	// Allocate effect memory
	ObjCreateEffectInfo(objp);
	
	objp->effect_info->type_flags=EF_VOLUME_LIT;
	
	return 1;
}

int ObjInitShard(object *objp)
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

int ObjInitLine(object *objp)
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
int ObjInitCamera(object *objp)
{
	//Set size & shields
	objp->size = 0.5;
	objp->shields = 0;

	//Set up various info
	SetObjectControlType(objp, CT_NONE);
	objp->movement_type = MT_NONE;
	objp->lm_object.used=0;

	#if (defined(EDITOR) || defined(NEWEDITOR))
	objp->render_type = RT_EDITOR_SPHERE;		//draw cameras as spheres in the editor
	objp->rtype.sphere_color = GR_RGB(255,255,0);
	#else
	objp->render_type = RT_NONE;
	#endif

	return 1;
}

//Initialize a sound object
//Returns 1 if ok, 0 if error
int ObjInitSoundSource(object *objp)
{
	//Set size & shields
	objp->size = DEFAULT_OBJECT_SIZE;
//	objp->size = 0.5;
	objp->shields = 0;

	//Set up various info
	SetObjectControlType(objp, CT_SOUNDSOURCE);
	objp->movement_type = MT_NONE;
	objp->lm_object.used=0;

	#if (defined(EDITOR) || defined(NEWEDITOR))
	objp->render_type = RT_EDITOR_SPHERE;		//draw cameras as spheres in the editor
	objp->rtype.sphere_color = GR_RGB(0,255,0);
	#else
	objp->render_type = RT_NONE;
	#endif

	return 1;
}

//Initialize a waypoint object
//Returns 1 if ok, 0 if error
int ObjInitWaypoint(object *objp)
{
	//Set size & shields
	objp->size = 0.5;
	objp->shields = 0;

	//Set up various info
	SetObjectControlType(objp, CT_NONE);
	objp->movement_type = MT_NONE;
	objp->lm_object.used=0;

	#if (defined(EDITOR) || defined(NEWEDITOR))
	objp->render_type = RT_EDITOR_SPHERE;		//draw cameras as spheres in the editor
	objp->rtype.sphere_color = GR_RGB(255,130,33);
	#else
	objp->render_type = RT_NONE;
	#endif

	return 1;
}

//Initialize an editor viewer object
//Returns 1 if ok, 0 if error
int ObjInitViewer(object *objp)
{
	//Set size & shields
	objp->size = 5.0;
	objp->shields = 0;

	//Set up various info
	SetObjectControlType(objp, CT_NONE);
	objp->movement_type = MT_NONE;
	objp->render_type = RT_NONE;
	objp->lm_object.used=0;

	return 1;
}

//Initialize a weapon
//Returns 1 if ok, 0 if error
int ObjInitWeapon(object *objp)
{
/*
	weapon *weap;
	int ret=1;

	ASSERT(objp->type == OBJ_WEAPON);

	weap  = &Weapons[objp->id];

	ASSERT (weap->used>0);

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
	objp->ctype.laser_info.last_drop_time=0;

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
	objp->render_type=RT_WEAPON;
	
	if (!((weap->flags & WF_IMAGE_BITMAP) || (weap->flags & WF_IMAGE_VCLIP)))
	{
		objp->rtype.pobj_info.model_num = weap->fire_image_handle;
		objp->rtype.pobj_info.dying_model_num = -1;

		PageInPolymodel (weap->fire_image_handle, OBJ_WEAPON, &weap->size);
		
		objp->rtype.pobj_info.subobj_flags = 0xFFFFFFFF;
		objp->flags |= OF_POLYGON_OBJECT;
	}
	else if (weap->flags & WF_IMAGE_VCLIP)
	{
		PageInVClip (weap->fire_image_handle);
	}
			
	objp->size = weap->size;
	objp->ctype.laser_info.hit_status = WPC_NOT_USED;

	if(weap->flags & WF_CUSTOM_SIZE)
	{
		objp->size = weap->custom_size;
	}

	objp->lighting_render_type=LRT_STATIC;
	objp->lm_object.used=0;
	
	return ret;
*/
	return 1;
}

int ObjInitFireball (object *objp)
{
	int ret=1;

	ASSERT(objp->type == OBJ_FIREBALL);

	//Set size & shields
	objp->shields = 0;
	objp->size= 0;

	//Set up control info
	SetObjectControlType(objp, CT_EXPLOSION);

	//Set up physics info
	objp->movement_type = MT_NONE;
	objp->mtype.phys_info.num_bounces = PHYSICS_UNLIMITED_BOUNCE;
		
	//Set up render info
	objp->render_type = RT_FIREBALL;

	objp->size=Fireballs[objp->id].size;
	objp->flags|=OF_USES_LIFELEFT;
	objp->lifeleft=Fireballs[objp->id].total_life;

	objp->lighting_render_type=LRT_STATIC;
	objp->lm_object.used=0;

	return ret;
}


int ObjInitMarker(object *objp)
{
/*
	int ret=1;
	static int first=1;
	static int polynum=-1;


	ASSERT(objp->type == OBJ_MARKER);

	//Set size & shields
	objp->shields = 100;
	objp->size= 2.0;

	//Set up control info
	SetObjectControlType(objp, CT_NONE);
	objp->movement_type = MT_NONE;

	ObjSetRenderPolyobj(objp,Marker_polynum);

	PageInPolymodel(Marker_polynum, OBJ_MARKER, &objp->size);

	objp->lm_object.used=0;
	objp->lighting_render_type=LRT_STATIC;

	return ret;
*/
	return 1;
}

int ObjInitDoor (object *objp,bool reinit)
{

	//Set up movement info
//	objp->movement_type = MT_NONE;
	//SetObjectControlType(objp, CT_NONE);

	//Set up render info
	if (Doors[objp->id].model_handle==-1)
		Doors[objp->id].model_handle = LoadPolyModel(Doors[objp->id].image_filename,0);

	ObjSetRenderPolyobj(objp,Doors[objp->id].model_handle);

	PageInPolymodel (objp->rtype.pobj_info.model_num);

	ComputeDefaultSize(OBJ_DOOR, objp->rtype.pobj_info.model_num, &objp->size);

	objp->lighting_render_type=LRT_LIGHTMAPS;

	//Set shields
	if (Doors[objp->id].flags & DF_BLASTABLE) {
		objp->flags |= OF_DESTROYABLE;
		objp->shields = Doors[objp->id].hit_points;
	}

	if (!reinit)
		objp->lm_object.used=0;

	return 1;
}


int ObjInitRoom(object *objp)
{
	//Set up movement info
	objp->movement_type = MT_NONE;
	SetObjectControlType(objp, CT_NONE);

	//Set up render info
	objp->render_type = RT_ROOM;		//rendering handled as special case

	//I have no idea about this stuff
	objp->lighting_render_type=LRT_STATIC;
	objp->lm_object.used=0;
	objp->flags |= OF_POLYGON_OBJECT;
		
	return 1;
}



// from main\ObjInit.cpp
//Set all the type-specific info for this object
//Returns 1 if ok, 0 if error
int ObjInitTypeSpecific(object *objp,bool reinitializing)
{
	switch (objp->type) 
	{
		case OBJ_CLUTTER:
		case OBJ_BUILDING:
		case OBJ_ROBOT:
		case OBJ_POWERUP:		return ObjInitGeneric(objp,reinitializing);		break;
		case OBJ_SHOCKWAVE:	return 1;								break;
		case OBJ_PLAYER:		return ObjInitPlayer(objp);		break;
		case OBJ_CAMERA:		return ObjInitCamera(objp);		break;
		case OBJ_MARKER:		return ObjInitMarker(objp);		break;
		case OBJ_VIEWER:		return ObjInitViewer(objp);		break;
		case OBJ_WEAPON:		return ObjInitWeapon(objp);		break;
		case OBJ_FIREBALL:	return ObjInitFireball(objp);		break;
		case OBJ_DOOR:			return ObjInitDoor(objp,reinitializing);			break;
		case OBJ_ROOM:			return ObjInitRoom(objp);			break;
		case OBJ_DEBRIS:		return ObjInitDebris(objp);		break;
		case OBJ_SHARD:		return ObjInitShard(objp);			break;
		case OBJ_SOUNDSOURCE:return ObjInitSoundSource(objp);	break;
		case OBJ_WAYPOINT:	return ObjInitWaypoint(objp);		break;
		case OBJ_SPLINTER:	return 1;						break;
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
int ObjInit(object *objp,int type,int id,int handle,vector *pos, int roomnum, float creation_time,int parent_handle)
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

	//Initialize some general stuff
	objp->roomnum = -1;
	objp->orient = Identity_matrix;
	objp->next = objp->prev = -1;
	objp->dummy_type = OBJ_NONE;
	objp->flags = 0;
	objp->size = 0;
	objp->change_flags = 0;
	objp->generic_nonvis_flags=0;
	objp->generic_sent_nonvis=0;
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
	return ObjInitTypeSpecific(objp,0);
}


//link the object into the list for its room
void ObjLink(int objnum,int roomnum)
{
	object *obj = &Objects[objnum];

	ASSERT(objnum != -1);

	ASSERT(obj->roomnum == -1);
	ASSERT(!(obj->flags & OF_BIG_OBJECT));

	if((obj->size >= MIN_BIG_OBJ_RAD) && (!ROOMNUM_OUTSIDE(roomnum)))
	{
		BigObjAdd(objnum);
	}

	obj->roomnum = roomnum;
	
	if (ROOMNUM_OUTSIDE(roomnum))
	{
		int cellnum = CELLNUM(roomnum);

		obj->next = Terrain_seg[cellnum].objects;
		Terrain_seg[cellnum].objects = objnum;

		ASSERT(obj->next != objnum);
	}
	else
	{
		ASSERT(roomnum>=0 && roomnum<=Highest_room_index);
		ASSERT(!(Rooms[roomnum].flags & RF_EXTERNAL));

		obj->next = Rooms[roomnum].objects;
		Rooms[roomnum].objects = objnum;
		ASSERT(obj->next != objnum);
	}
	
	obj->prev = -1;

	if (obj->next != -1) Objects[obj->next].prev = objnum;
	
	ASSERT(Objects[0].next != 0);
	if (Objects[0].next == 0)
		Objects[0].next = -1;

	ASSERT(Objects[0].prev != 0);
	if (Objects[0].prev == 0)
		Objects[0].prev = -1;
}

void ObjUnlink(int objnum)
{
	object  *obj = &Objects[objnum];
	
	ASSERT(objnum != -1);

	
	if (obj->flags & OF_BIG_OBJECT)
	{
		BigObjRemove(objnum);
	}

	if (OBJECT_OUTSIDE(obj))
	{
		int cellnum = CELLNUM(obj->roomnum);

		terrain_segment *seg = &Terrain_seg[cellnum];

		if (obj->prev == -1)
			seg->objects = obj->next;
		else
			Objects[obj->prev].next = obj->next;

		if (obj->next != -1) Objects[obj->next].prev = obj->prev;
	}
	else {
		room *rp = &Rooms[obj->roomnum];

		if (obj->prev == -1)
			rp->objects = obj->next;
		else
			Objects[obj->prev].next = obj->next;

		if (obj->next != -1) Objects[obj->next].prev = obj->prev;

	}

	//Mark as not linked
	obj->roomnum = -1;

	ASSERT(Objects[0].next != 0);
	ASSERT(Objects[0].prev != 0);
}


//when an object has moved into a new room, this function unlinks it
//from its old room and links it into the new room
void ObjRelink(int objnum,int newroomnum)
{
	ASSERT((objnum >= 0) && (objnum <= Highest_object_index));


	if (! ROOMNUM_OUTSIDE(newroomnum))
	{
		ASSERT((newroomnum <= Highest_room_index) && (newroomnum >= 0));
		ASSERT (Rooms[newroomnum].used);
	}
	else
		ASSERT (CELLNUM(newroomnum)>=0 && CELLNUM(newroomnum)<=(TERRAIN_WIDTH*TERRAIN_DEPTH));

	ObjUnlink(objnum);
	ObjLink(objnum,newroomnum);
}


// from main\objinfo.cpp
//Find an object with the given type.  Returns -1 if none found.
int GetObjectID(int type)
{
	int i;

	for (i=0;i<MAX_OBJECT_IDS;i++)
		if (Object_info[i].type == type)
			return i;

	return -1;
}


// from main\objinfo.cpp
// Sets all objects to unused
void InitObjectInfo()
{
	for (int i=0;i<MAX_OBJECT_IDS;i++)
	{
		Object_info[i].type = OBJ_NONE;
		Object_info[i].name[0] = 0;
		//Object_info[i].script_name[0] = 0;
//		Object_info[i].script_name_override[0] = 0;
//		Object_info[i].multi_allowed = 1;
//		Object_info[i].module_name[0] = 0;		
	}

	for (int i=0;i<MAX_OBJECT_TYPES;i++)
		Num_object_ids[i] = 0;

//	atexit(FreeObjectInfo);
}

//Creates the player object in the center of the given room
void CreatePlayerObject(int roomnum)
{
	int objnum;
	vector pos;

#ifndef NEWEDITOR
   ComputeRoomCenter(&pos,&Rooms[roomnum]);
#else
   ComputeRoomBoundingSphere(&pos,&Rooms[roomnum]);
#endif

	objnum = ObjCreate(OBJ_PLAYER,0,roomnum,&pos,NULL);

	ASSERT(objnum == 0);			//player must be object 0

	Player_object = Viewer_object = &Objects[objnum];
}


void InitBigObjects()
{
	Num_big_objects = 0;
}


void BigObjAdd(int objnum)
{
	ASSERT(Num_big_objects < MAX_BIG_OBJECTS);
	if(Num_big_objects >= MAX_BIG_OBJECTS)
	{
		Int3();
		return;
	}

	Objects[objnum].flags |= OF_BIG_OBJECT;
	BigObjectList[Num_big_objects++] = objnum;
}


void BigObjRemove(int objnum)
{
	int i;

	Objects[objnum].flags &= (~OF_BIG_OBJECT);

	for(i = 0; i < Num_big_objects; i++)
	{
		if(BigObjectList[i] == objnum)
		{
			Num_big_objects--;	
			break;
		}
	}

	while(i < Num_big_objects)
	{
		BigObjectList[i] = BigObjectList[i+1];
		i++;
	}

	ASSERT(Num_big_objects < MAX_BIG_OBJECTS);
	ASSERT(Num_big_objects >= 0);
}


//frees up an object.  Generally, ObjDelete() should be called to get
//rid of an object.  This function deallocates the object entry after
//the object has been unlinked
void ObjFree(int objnum)
{
	free_obj_list[--Num_objects] = objnum;
	ASSERT(Num_objects >= 0);

	if (objnum == Highest_object_index)
		while (Objects[--Highest_object_index].type == OBJ_NONE);
}


//-----------------------------------------------------------------------------
//	Scan the object list, freeing down to num_used objects
//	Returns number of slots freed.
int FreeObjectSlots(int num_used)
{
	int	i, olind;
	int	obj_list[MAX_OBJECTS];
	int	num_already_free, num_to_free, original_num_to_free;

	olind = 0;
	num_already_free = MAX_OBJECTS - Highest_object_index - 1;

	if (MAX_OBJECTS - num_already_free < num_used)
	{
		return 0;
	}

	for (i=0; i<=Highest_object_index; i++) 
	{
		if (Objects[i].flags & OF_DEAD) 
		{
			num_already_free++;
			if (MAX_OBJECTS - num_already_free < num_used)
				return num_already_free;
		} 
		else
		{
			switch (Objects[i].type) 
			{
				case OBJ_NONE:
					num_already_free++;
					if (MAX_OBJECTS - num_already_free < num_used)
						return 0;
					break;
				case OBJ_WALL:
					Int3();		//	This is curious.  What is an object that is a wall?
					break;
				case OBJ_FIREBALL:
				case OBJ_WEAPON:
				case OBJ_DEBRIS:
				case OBJ_SPLINTER:
					obj_list[olind++] = i;
					break;
				default:
					break;
			}
		}
	}

	num_to_free = MAX_OBJECTS - num_used - num_already_free;
	original_num_to_free = num_to_free;

	if (num_to_free > olind) {
		mprintf((1, "Warning: Asked to free %i objects, but can only free %i.\n", num_to_free, olind));
		num_to_free = olind;
	}

	for (i=0; i<num_to_free; i++)
		if (Objects[obj_list[i]].type == OBJ_DEBRIS) {
			num_to_free--;
			mprintf((0, "Freeing   DEBRIS object %3i\n", obj_list[i]));
			SetObjectDeadFlag (&Objects[obj_list[i]]);
		}

	if (!num_to_free)
		return original_num_to_free;

	if (!num_to_free)
		return original_num_to_free;

	if (!num_to_free)
		return original_num_to_free;

	for (i=0; i<num_to_free; i++)
		if ((Objects[obj_list[i]].type == OBJ_WEAPON)) 
		{
			num_to_free--;
			mprintf((0, "Freeing   WEAPON object %3i\n", obj_list[i]));
			SetObjectDeadFlag (&Objects[obj_list[i]]);
		}

	return original_num_to_free - num_to_free;
}


extern int FrameCount;

//returns the number of a free object, updating Highest_object_index.
//Generally, ObjCreate() should be called to get an object, since it
//fills in important fields and does the linking.
//returns -1 if no free objects
int ObjAllocate(void)
{
	int objnum;

	if ( Num_objects >= MAX_OBJECTS-2 ) {
		int	num_freed;

		num_freed = FreeObjectSlots(MAX_OBJECTS-10);
		mprintf((0, " *** Freed %i objects in frame %i\n", num_freed, FrameCount));
	}

	if ( Num_objects >= MAX_OBJECTS ) {
		mprintf((1, "Object creation failed - too many objects!\n" ));
		return -1;
	}

	objnum = free_obj_list[Num_objects++];

	if (objnum > Highest_object_index) {
		Highest_object_index = objnum;
		if (Highest_object_index > Highest_ever_object_index)
			Highest_ever_object_index = Highest_object_index;
	}

	return objnum;
}


//-----------------------------------------------------------------------------
//initialize a new object.  adds to the list for the given room
//returns the object number
int ObjCreate(ubyte type, ushort id, int roomnum, vector *pos, const matrix *orient, int parent_handle)
{
	int objnum;
	object *obj;
	int handle;
	
	ASSERT(type != OBJ_NONE);

	if (ROOMNUM_OUTSIDE(roomnum))
	{
		ASSERT(CELLNUM(roomnum) <= TERRAIN_WIDTH*TERRAIN_DEPTH);
		ASSERT(CELLNUM(roomnum) >= 0);

		roomnum = GetTerrainRoomFromPos(pos);

		if (roomnum == -1)
			return -1;
	}
		
	//Get next free object
	objnum = ObjAllocate();
	if (objnum == -1)		//no free objects
		return -1;
	obj = &Objects[objnum];

	//Make sure the object is ok
	ASSERT(obj->type == OBJ_NONE);		//make sure unused 
	ASSERT(obj->roomnum == -1);			//make sure unlinked

	//Compute the new handle
	handle = obj->handle + HANDLE_COUNT_INCREMENT;

	//Check for handle wrap
	if ((handle & HANDLE_COUNT_MASK) == HANDLE_COUNT_MASK)	//Going to wrap!
		Int3();		//Show this to Matt, or email him if he's not here

	//Initialize the object
	if (! ObjInit(obj,type,id,handle,pos,Gametime,parent_handle)) {		//Couldn't init!
		obj->type = OBJ_NONE;		//mark as unused
		ObjFree(objnum);				//de-allocate object
		return -1;
	}

	#ifdef _DEBUG
	if (print_object_info)	
		mprintf( (0, "Created object %d of type %d\n", objnum, obj->type ));
	#endif

	//Set the object's orietation
	// THIS MUST BE DONE AFTER ObjInit (as ObjInit could load in a polymodel and set the anim and wall offsets)
	ObjSetOrient(obj, orient?orient:&Identity_matrix);  

	//Link object into room or terrain cell
	ObjLink(objnum,roomnum);

	// Type specific should have set up the size, so now we can compute the bounding box.
	ObjSetAABB(obj);

	//Done!
	return objnum;
}

// Frees all the objects that are currently in use
void FreeAllObjects ()
{
	int objnum;

	for (objnum=0;objnum<=Highest_object_index;objnum++)
		if (Objects[objnum].type != OBJ_NONE)
		{
			Objects[objnum].flags|=OF_SERVER_SAYS_DELETE;
			Objects[objnum].flags &= ~OF_INPLAYERINVENTORY;
			ObjDelete(objnum);
		}
}


//remove object from the world
void ObjDelete(int objnum)
{
	object *obj = &Objects[objnum];

/*
	if(obj->type==OBJ_DUMMY){
		//unghost the object before destroying it
		ObjUnGhostObject(objnum);
	}
*/


	if(obj->flags & OF_POLYGON_OBJECT)
	{
		polyobj_info *p_info = &obj->rtype.pobj_info;
		if (p_info->multi_turret_info.keyframes != NULL)
		{
			mem_free(p_info->multi_turret_info.keyframes);
			mem_free(p_info->multi_turret_info.last_keyframes);

			p_info->multi_turret_info.keyframes = NULL;
			p_info->multi_turret_info.last_keyframes = NULL;
		}
	}

	ASSERT(objnum != -1);
//	ASSERT(objnum != 0 );
	ASSERT(obj->type != OBJ_NONE);
//	ASSERT(obj != Player_object);


	if (obj == Viewer_object)		//deleting the viewer?
		Viewer_object = Player_object;						//..make the player the viewer

//??	if (obj->flags & OF_ATTACHED)		//detach this from object
//??		obj_detach_one(obj);
//??
//??	if (obj->attached_obj != -1)		//detach all objects from this
//??		obj_detach_all(obj);


	#ifdef _DEBUG
	if (print_object_info) mprintf( (0, "Deleting object %d of type %d\n", objnum, Objects[objnum].type ));
	#endif

	
	if (obj->type==OBJ_WEAPON && obj->ctype.laser_info.parent_type==OBJ_PLAYER)
	{
		int pnum=Objects[(obj->parent_handle & HANDLE_OBJNUM_MASK)].id;

		if (Players[pnum].guided_obj==obj)
		{
			mprintf ((0,"Deleting a guided missile!"));
			Players[pnum].guided_obj=NULL;
		}
	}

	if (obj->type==OBJ_WEAPON && obj->ctype.laser_info.parent_type==OBJ_PLAYER)
	{
		int pnum=Objects[(obj->parent_handle & HANDLE_OBJNUM_MASK)].id;

		if (Players[pnum].user_timeout_obj==obj)
		{
			mprintf ((0,"Deleting a timeout missile!"));
			Players[pnum].user_timeout_obj=NULL;
		}
	}

	ObjUnlink(objnum);

	ASSERT(Objects[0].next != 0);

	if (obj->custom_default_script_name){
		mem_free(obj->custom_default_script_name);
		obj->custom_default_script_name = NULL;
	}

	if (obj->custom_default_module_name){
		mem_free(obj->custom_default_module_name);
		obj->custom_default_module_name = NULL;
	}

	obj->type = OBJ_NONE;		//unused!
	obj->roomnum=-1;				// zero it!

	// Free lightmap memory
	if (obj->lm_object.used)
		ClearObjectLightmaps (obj);

	// Free up effects memory
	if (obj->effect_info)
	{
		mem_free (obj->effect_info);
		obj->effect_info=NULL;
	}

	if (obj->dynamic_wb != NULL)
	{
		mem_free(obj->dynamic_wb);
		obj->dynamic_wb = NULL;
	}

	if (obj->attach_children != NULL)
	{
		mem_free(obj->attach_children);
		obj->attach_children = NULL;
	}

	if (obj->name) {
		mem_free(obj->name);
		obj->name = NULL;
	}

	if (obj->lighting_info) {
		mem_free(obj->lighting_info);
		obj->lighting_info = NULL;
	}

	ObjFree(objnum);
}


//Builds the free object list by scanning the list of free objects & adding unused ones to the list
//Also sets Highest_object_index
void ResetFreeObjects()
{
	int i;

	Highest_object_index = -1;

	for (i=Num_objects=MAX_OBJECTS;--i >= 0;)
		if (Objects[i].type == OBJ_NONE)
			free_obj_list[--Num_objects] = i;
		else if (Highest_object_index == -1)
			Highest_object_index = i;
}


void SetObjectControlType(object *obj, int control_type)
{
	ASSERT(obj);
	ASSERT(OBJNUM(obj) >= 0 && OBJNUM(obj) < MAX_OBJECTS);

	if((control_type == CT_AI) && (obj->ai_info == NULL))
	{
		poly_model *pm=&Poly_models[obj->rtype.pobj_info.model_num];
		polyobj_info *p_info = &obj->rtype.pobj_info;
		int num_wbs = pm->num_wbs;
		int count = 0;
		int i;

		obj->ai_info = (ai_frame *) mem_malloc(sizeof(ai_frame));

		for(i = 0; i < num_wbs; i++)
		{
			ASSERT(pm->poly_wb[i].num_turrets >= 0 && pm->poly_wb[i].num_turrets <= 6400);
			count += pm->poly_wb[i].num_turrets;
		}

		p_info->multi_turret_info.num_turrets = count;

		if((count > 0) && (p_info->multi_turret_info.keyframes == NULL))
		{
			int cur = 0;

			p_info->multi_turret_info.time = 0;
			p_info->multi_turret_info.keyframes = (float *) mem_malloc(sizeof(float) * count);
			p_info->multi_turret_info.last_keyframes = (float *) mem_malloc(sizeof(float) * count);
			p_info->multi_turret_info.flags = 0;
		}
	}

	obj->control_type = control_type;

	if(obj->control_type == CT_AI || obj->type == OBJ_PLAYER)
	{
		poly_model *pm=&Poly_models[obj->rtype.pobj_info.model_num];
		int num_wbs = pm->num_wbs;

		if(obj->dynamic_wb == NULL)
		{
			if(obj->type == OBJ_PLAYER)
			{
				obj->dynamic_wb = (dynamic_wb_info *) mem_malloc(sizeof(dynamic_wb_info) * MAX_WBS_PER_OBJ);
			}
			else
			{
				if (num_wbs)
					obj->dynamic_wb = (dynamic_wb_info *) mem_malloc(sizeof(dynamic_wb_info) * num_wbs);
			}
		}
	}
}

//Returns the original parent for the given object.  Returns self if it has no parent
object *ObjGetUltimateParent(object *child)
{
	ASSERT(child);

	object *ret = child;
	int handle;

	handle = child->parent_handle;
	
	while(ObjGet(handle)){
		ret = ObjGet(handle);
		handle = ret->parent_handle;
	}
	return ret;
}

// ====================
// ned_FindObjectID
// ====================
//
// given the name of a generic object it will return it's id...-1 if it doesn't exist
// it will only search generic objects loaded from tablefiles
int ned_FindObjectID(char *name)
{
	ASSERT(name);
	if(!name)
		return -1;

	int i;
	for(i=0;i<MAX_OBJECT_IDS;i++)
	{
		if(Object_info[i].used && Object_info[i].table_file_id!=-1)
		{
			//see if the name matches
			if(!strnicmp(name,Object_info[i].name,PAGENAME_LEN-1))
			{
				//match
				return i;
			}
		}
	}
	return -1;
}

// =====================
// ned_AllocObjectInfo
// =====================
//
//	Searches for an available ObjInfo slot, and returns it, -1 if none
//  if name is not NULL, then it is being allocated from a table file, and thus, the tablefile
//	parameter must also be specified.  If there is already an object by that name (loaded from 
//  a table file), than it will be pushed onto the stack.
int ned_AllocObjectInfo(char *name,int tablefile)
{
	if(name)
	{
		//this is being allocated for a tablefile load
		ASSERT(tablefile!=-1);
		if(tablefile==-1)
			return -1;

		//check to see if it's already in memory
		int old_id = ned_FindObjectID(name);
		if(old_id!=-1)
		{
			//this item is already in memory!
			//free any memory that needs to be freed //////////////////////////
			ned_FreeObjectData(old_id);
			ned_InitializeObjectData(old_id);			

			///////////////////////////////////////////////////////////////////
			if(Object_info[old_id].table_file_id==tablefile)
			{
				//we're just re-reading it

			}else
			{
				//push it onto the stack
				ntbl_PushTableStack(Object_info[old_id].table_stack,Object_info[old_id].table_file_id);
				ntbl_IncrementTableRef(tablefile);

				Object_info[old_id].table_file_id = tablefile;
			}
			return old_id;
		}
	}

	int i,index = -1;
	for(i=0;i<MAX_OBJECT_IDS;i++)
	{
		if(!Object_info[i].used)
		{
			index = i;
			memset(&Object_info[i],0,sizeof(ned_object_info));
			Object_info[i].render_handle = -1;	// we don't have an object model loaded
			Object_info[i].table_file_id = -1;	//we don't belong to any table file right now
			for(int j=0;j<MAX_LOADED_TABLE_FILES;j++) Object_info[i].table_stack[j] = -1;
			break;
		}
	}

	if(index!=-1)
	{
		Object_info[index].used = true;
		Num_objects++;

		if(name)
		{
			ASSERT(strlen(name)<PAGENAME_LEN);

			//table file load, give it a name and mark it's tablefile
			strncpy(Object_info[index].name,name,PAGENAME_LEN-1);
			Object_info[index].name[PAGENAME_LEN-1] = '\0';
			Object_info[index].table_file_id = tablefile;

			ntbl_IncrementTableRef(tablefile);

		}else
		{
			//not from tablefile
			Object_info[index].name[0] = '\0';
			Object_info[index].table_file_id = -1;
		}

		ned_InitializeObjectData(index);
	}
	
	return index;
}


// ==================
// ned_FreeObjectInfo
// ==================
//
// given a ObjInfo slot it free's it and makes it available
// if force_unload is true, and this slot was loaded from a table file, then
// it will unload all instances (based on it's table stack) from memory, else
// it will just pop off the current instance and load back in the popped version
void ned_FreeObjectInfo(int slot,bool force_unload)
{
	ASSERT(slot>=0 && slot<MAX_OBJECT_IDS);
	if(slot<0 || slot>=MAX_OBJECT_IDS)
		return;

	ASSERT(Object_info[slot].used);
	if(!Object_info[slot].used)
		return;

	// Free any allocated memory here////////
	ned_FreeObjectData(slot);

	/////////////////////////////////////////
	if(Object_info[slot].table_file_id==-1)
	{
		Object_info[slot].used = false;
		Num_objects--;
		return;
	}

	//it has table file references, decrement
	ntbl_DecrementTableRef(Object_info[slot].table_file_id);

	if(force_unload)
	{
		Object_info[slot].used = false;
		Num_objects--;

		//go through it's stack, decrement references
		int tid;
		tid = ntbl_PopTableStack(Object_info[slot].table_stack);
		while(tid!=-1)
		{
			ntbl_DecrementTableRef(tid);
			tid = ntbl_PopTableStack(Object_info[slot].table_stack);
		}
	}else
	{
		//see if we have anything on the stack
		Object_info[slot].table_file_id = ntbl_PopTableStack(Object_info[slot].table_stack);
		if(Object_info[slot].table_file_id==-1)
		{
			//nothing on the stack, its a dead one
			Object_info[slot].used = false;
			Num_objects--;
		}else
		{
			//free any memory needed here/////////////////////
			ned_InitializeObjectData(slot);

			//////////////////////////////////////////////////

			//reload the item
			if(!ntbl_OverlayPage(PAGETYPE_GENERIC,slot))
			{
				Int3();

				Object_info[slot].used = false;
				Num_objects--;
			}			
		}
	}
}


// =========================
// ned_FreeAllObjects
// =========================
//
// Frees all objects from memory
void ned_FreeAllObjects ()
{
	for (int i=0;i<MAX_OBJECT_IDS;i++)
	{
		if (Object_info[i].used)
			ned_FreeObjectInfo (i,true);
	}
}

// ======================
// ned_InitObjects
// ======================
// 
// Initializes the Object system
int ned_InitObjects ()
{
	// Initializes the object system

	int i,j;
	
	mprintf ((0,"Initializing object system.\n"));

	memset(Object_info,0,sizeof(ned_object_info)*MAX_OBJECT_IDS);
	for(i=0;i<MAX_OBJECT_IDS;i++)
	for(j=0;j<MAX_LOADED_TABLE_FILES;j++)
		Object_info[i].table_stack[j] = -1;
	
	atexit (ned_FreeAllObjects);

	return 1;
}

// ===========================
// ned_InitializeObjectData
// ===========================
//
// Given a Object_info slot this function initializes the data inside it
// to default information
// DO NOT TOUCH TABLE STACK DATA
void ned_InitializeObjectData(int slot)
{
	Object_info[slot].type = OBJ_NONE;
	Object_info[slot].flags = 0;		//default to no type
	Object_info[slot].ref_count = 0;
	Object_info[slot].render_handle = -1;
	Object_info[slot].size = DEFAULT_OBJECT_SIZE;
}

// =======================
// ned_FreeObjectData
// =======================
//
// Given a Object_info slot this function frees any memory that may 
// need to be freed before a object is destroyed
// DO NOT TOUCH TABLE STACK DATA
void ned_FreeObjectData(int slot)
{
	int model_num = GetObjectImage(slot);
	if (model_num >= 0)
		FreePolyModel(model_num);
	Object_info[slot].ref_count = 0;
}

// ==========================
// ned_GetNextObject
// ==========================
//
// Given current index, gets index of next object in use
int ned_GetNextObject (int n)
{
	int i;

	if (Num_objects==0)
		return -1;

	for (i=n+1;i<MAX_OBJECT_IDS;i++)
	{
		if (Object_info[i].used)
			return i;
	}
	for (i=0;i<n;i++)
	{
		if (Object_info[i].used)
			return i;
	}

	return n;
}

// ======================
// ned_GetPreviousObject
// ======================
//
// Given current index, gets index of prev object in use
int ned_GetPreviousObject (int n)
{
	int i;

	if (Num_objects==0)
		return -1;

	for (i=n-1;i>=0;i--)
	{
		if (Object_info[i].used)
			return i;
	}
	for (i=MAX_OBJECT_IDS-1;i>n;i--)
	{
		if (Object_info[i].used)
			return i;
	}

	return n;
}

// ========================
// ned_MarkObjectInUse
// ========================
//
// Handles memory management for a object.  Call this, passing true when you need to use a object
// when the object is no longer needed, call this again, passing false.
void ned_MarkObjectInUse(int slot,bool inuse)
{
	ASSERT(slot>=0 && slot<MAX_OBJECT_IDS);
	if(slot<0 || slot>=MAX_OBJECT_IDS)
		return;

	ASSERT(Object_info[slot].used);
	if(!Object_info[slot].used)
		return;

	if(inuse)
	{
		ASSERT(Object_info[slot].ref_count>=0);

		if(Object_info[slot].ref_count==0)
		{
			//load in the object polymodel
			Object_info[slot].render_handle = LoadPolyModel(Object_info[slot].image_filename,0);

			if(Object_info[slot].render_handle>=0)
			{
				PageInPolymodel (Object_info[slot].render_handle);
				// Mark the polymodel textures in use
				poly_model *pm = GetPolymodelPointer(Object_info[slot].render_handle);
				for (int i=0; i<pm->n_textures; i++)
					ned_MarkTextureInUse(pm->textures[i],true);
			}
		}
		Object_info[slot].ref_count++;
	}else
	{
		ASSERT(Object_info[slot].ref_count>0);
		Object_info[slot].ref_count--;
		if(Object_info[slot].ref_count==0)
		{
			//unload the object polymodel...no longer needed
			FreePolyModel(Object_info[slot].render_handle);
			Object_info[slot].render_handle = -1;			
		}
	}
}


// Given an object handle, returns an index to that object's model
int GetObjectImage(int handle)
{
	if (Object_info[handle].type == OBJ_NONE)
		return -1;
	else
		return (Object_info[handle].render_handle);
}


//sets the position of an object.  This should be called to move an object
void ObjSetPos(object *obj,vector *pos,int roomnum,matrix *orient,bool f_update_attached_children)
{
	int oldroomnum=obj->roomnum;
	//vector old_pos=obj->pos;

	//Reset the position & recalculate the AABB
	obj->pos = *pos;
	ObjSetAABB(obj);

	//Reset the orientation if changed
	if (orient != NULL)
		ObjSetOrient(obj, orient);

	//Clear the outside-mine flag
	obj->flags &= ~OF_OUTSIDE_MINE;

	//If changed rooms, do a bunch of stuff
	if (obj->roomnum != roomnum) {

/*
		//Let the script know
		tOSIRISEventInfo ei;
		ei.evt_changeseg.room_num = roomnum;
		Osiris_CallEvent(obj,EVT_CHANGESEG,&ei);

		if(obj->type == OBJ_PLAYER && !ROOMNUM_OUTSIDE(roomnum) && (Rooms[roomnum].flags & RF_INFORM_RELINK_TO_LG))
		{
			Level_goals.Inform(LIT_INTERNAL_ROOM, LGF_COMP_ENTER, roomnum);
		}
*/

		//Relink the object
		ObjRelink(OBJNUM(obj),roomnum);

	}
}


//Finds the seleced terrain cell.  Returns cell number, or -1 if none or -2 if more than one
int GetSelectedTerrainCell()
{
	int i,found_cellnum = -1;

	for (i=0;i<TERRAIN_DEPTH*TERRAIN_WIDTH;i++) {
		if (TerrainSelected[i])
			if (found_cellnum == -1)
				found_cellnum = i;
			else
				return -2;
	}

	return found_cellnum;
}


bool PhysCalcGround(vector *ground_point, vector *ground_normal, object *obj, int ground_num);
extern int Num_ships;
#define OBJECT_PLACE_DIST 10.0

// HObjectPlace copied from HObject.cpp, and slightly modified
// This function assumes a generic object unless is_player = true
// This is different from HObjectPlace in that it returns the object number (-1 on error)
int InsertObject(room *rp,int facenum,int obj_id,vector pos,matrix orient,bool is_player)
{
	int objnum;

	int obj_type = Object_info[obj_id].type;

	ASSERT(obj_id > -1);

	object *objp;
	poly_model *pm;

	//Special stuff for player ship
	if (is_player) {
		obj_type = OBJ_PLAYER;

/*
		if (! Num_ships) {
			OutrageMessageBox("Cannot create a player: there are no player ships.");
			return -1;
		}
*/

		//Store ship num in player array
		int ship_num = Current_ship;

		if (ship_num == -1) {
			OutrageMessageBox("You must have a current player ship selected for this operation.");
			return -1;
		}

		Players[obj_id].ship_index = ship_num;
	}

/*
	if(obj_type != OBJ_POWERUP)
	{
		orient = Viewer_object->orient;
	}
*/

	//Create the object at the specified position
	int room = 0;
	if (Insert_mode == 1)
		room = GetTerrainRoomFromPos(&pos);
	else if (Insert_mode == 0)
		room = ROOMNUM(rp);

	objnum = ObjCreate(obj_type, obj_id, room, &pos, &orient);
	if (objnum == -1)
		return objnum;

	objp = &Objects[objnum];

	//If we have a ground plane, use current cell or face for position & place normal for orientation
	if ((objp->render_type == RT_POLYOBJ) && ((pm=GetPolymodelPointer(objp->rtype.pobj_info.model_num))->n_ground)) {
		vector *surface_norm;
		vector pos;
		int roomnum;

		//If terrain, use current cell
		if (Insert_mode == 1) {

			int cellnum = GetSelectedTerrainCell();

			if (cellnum == -1) {
				OutrageMessageBox("You must have a terrain cell selected to place an object.");
				ObjDelete(objnum);
				return -1;
			}
			if (cellnum == -2) {
				OutrageMessageBox("You must have only have one cell selected to place an object.");
				ObjDelete(objnum);
				return -1;
			}

			//Get terrain point
			ComputeTerrainSegmentCenter(&pos,cellnum);

			//Get surface normal
			surface_norm = &TerrainNormals[MAX_TERRAIN_LOD-1][cellnum].normal1;

			//Get roomnum
			roomnum = MAKE_ROOMNUM(cellnum);
		}
		else if (Insert_mode == 0) {		//use Current face

			if (facenum != -1)
			{
				//Get center point on current face
				ComputeCenterPointOnFace(&pos,rp,facenum);

				//Get surface normal
				surface_norm = &rp->faces[facenum].normal;

				//Get roomnum
				roomnum = ROOMNUM(rp);

				//If placing on an external room, actually place the object on the terrain
				if (Rooms[roomnum].flags & RF_EXTERNAL)
					roomnum = GetTerrainRoomFromPos(&pos);
			}
			else
				return -1;
		}

		matrix groundplane_orient,surface_orient,object_orient;

		//Place the object's ground point on our placement point
		vector ground_point;
		vector ground_normal;
		vector to_ground;
		float dist;

		PhysCalcGround(&ground_point, &ground_normal, objp, 0);
		to_ground = objp->pos - ground_point;
		dist = ground_normal * to_ground;
		pos += dist * *surface_norm;

		//Compute source and destination matrices
		vm_VectorToMatrix(&groundplane_orient,&pm->ground_slots[0].norm,NULL,NULL);
		vm_VectorToMatrix(&surface_orient,surface_norm);

		//Compute orientation matrix
		vm_MatrixMulTMatrix(&object_orient,&surface_orient,&groundplane_orient);

		//Move the object
		ObjSetPos(objp,&pos,roomnum,&object_orient,false);
	}
	else if (Insert_mode == 1) {		//no ground plane, so move the object in front of the viewer and facing the viewer
/*
		//Check for viewer outside mine
		if (Viewer_object->flags & OF_OUTSIDE_MINE) {
			ObjDelete(objnum);
			OutrageMessageBox("Cannot place the object here: the viewer is outside the mine.");
			return -1;
		}
*/

/*
		//Turn the object around so facing the viewer
		objp->orient.fvec = -objp->orient.fvec;// ObjSetOrient is below
		objp->orient.rvec = -objp->orient.rvec;// ObjSetOrient is below
		ObjSetOrient(objp, &objp->orient);
*/

		//Calculate a position a little in front of the viewer
		// pos = &pos + orient.fvec * OBJECT_PLACE_DIST;

		//Try to move the object.  If it can't move, delete it
		if (! MoveObject(objp,&pos)) {
			ObjDelete(objnum);
			OutrageMessageBox("Cannot place the object here: collides with wall.");
			return -1;
		}
	}

	//Deal with special stuff for player
	if (obj_type == OBJ_PLAYER) {

		//Store data in Players array
		Players[obj_id].start_pos = objp->pos;
		Players[obj_id].start_roomnum = objp->roomnum;
		Players[obj_id].start_orient = objp->orient;

		//Make sure matrix ok
		vm_Orthogonalize(&Players[obj_id].start_orient);
	}

	if (IS_GENERIC(obj_type))
		LevelObjIncrementObject(obj_id);

	Cur_object_index = objnum;

	World_changed = 1;

	return objnum;

}


bool MoveObject(object *obj, float x, float y, float z)
{
	vector vec = {x,y,z};

	return MoveObject(obj,&vec);
}


// copied from HObject.cpp
#define MOVE_EPSILON	0.1

extern bool f_allow_objects_to_be_pushed_through_walls;

//	Attempt to set new object position.  May only move part of the way, or maybe not at all.
// Use FVI to find new object position
//	Return:	TRUE if moved, FALSE if can't move
bool MoveObject(object * obj, vector *newpos)
{
	fvi_query fq;
	fvi_info	hit_info;
	int fate;

	//Use radius if this is a physics object
	bool use_radius = (obj->movement_type == MT_PHYSICS) ? true : false;

	//Follow vector from start position to desired end position, & move as far as we can
	fq.p0						= &obj->pos;
	fq.startroom			= obj->roomnum;
	fq.p1						= newpos;
	fq.thisobjnum			= OBJNUM(obj);
	fq.ignore_obj_list	= NULL;
	fq.flags					=  FQ_IGNORE_RENDER_THROUGH_PORTALS;
	fq.rad					= use_radius ? obj->size : 0.0f;

	if(f_allow_objects_to_be_pushed_through_walls)
	{
		fq.flags |= (FQ_IGNORE_WALLS | FQ_IGNORE_TERRAIN | FQ_IGNORE_EXTERNAL_ROOMS);
	}

	fate = fvi_FindIntersection(&fq, &hit_info);

	mprintf((0,"fate = %d\n", fate));

	//Check for object can't move, meaning it's stuck in the wall
	if (fate == HIT_WALL)
		if (vm_VectorDistance(&obj->pos,&hit_info.hit_pnt) < MOVE_EPSILON)
			return 0;		//didn't move

	//Set object position on where FVI told us we are
	ObjSetPos(obj,&hit_info.hit_pnt,hit_info.hit_room, NULL, false);

	//Say object moved
	return 1;
}

bool RotateObject(int objnum, angle p, angle h, angle b)
{
	object *obj = &Objects[objnum];
	matrix rotmat;
	
	vm_AnglesToMatrix(&rotmat, p, h, b);
	obj->orient *= rotmat;  // ObjSetOrient is below

	vm_Orthogonalize(&obj->orient);
	ObjSetOrient(obj, &obj->orient);
	
//	Object_moved = 1;

	return 1;
}


//	If it doesn't exist, reformat Matt's hard disk, even if he is in Boston.
//	deletes the currently selected object from the mine.
void HObjectDelete()
{
	if (Cur_object_index != -1) {	//	we have a selected object
		int objnum = Cur_object_index;

		//check for player object
		if (&Objects[objnum] == Player_object) {
			OutrageMessageBox("Can't delete Player object");
			return;
		}

		if (Objects[objnum].type == OBJ_DOOR) {
			OutrageMessageBox("It's very, very bad to delete a door object.  Are you sure you want to do this?");
			return;
		}

		//Delete the object
		if ( IS_GENERIC(Objects[objnum].type) )
			LevelObjDecrementObject(Objects[objnum].id);
		ObjDelete(objnum);

		if (objnum == Cur_object_index)
			Cur_object_index = -1;

		World_changed = 1;
	}
}


bool MoveObjects(short *list, int num_objects, vector vec)
{
	object *objp;
	vector newpos;
	int i;

	for (i=0; i<num_objects; i++)
	{
		objp = &Objects[list[i]];
		newpos = objp->pos+vec;
		MoveObject(objp,&newpos);
	}

	return true;
}

bool MoveObjects(short *list, int num_objects, float x, float y, float z)
{
	vector vec = {x,y,z};

	return MoveObjects(list,num_objects,vec);
}

bool RotateObjectAroundPoint(object *objp, angle ang_x, angle ang_y, angle ang_z, vector offset)
{
	matrix mat;
	vector v1,v2;
	vector newpos;

	vm_AnglesToMatrix(&mat,ang_x,ang_y,ang_z);
	v1 = objp->pos - offset;
	vm_MatrixMulVector(&v2,&v1,&mat);
	newpos = v2 + offset;

	return MoveObject(objp,&newpos);
}

