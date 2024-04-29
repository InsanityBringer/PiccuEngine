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

#include <stdlib.h>
#include <stdio.h>
#include <algorithm>

#include "collide.h"
#include "PHYSICS.H"
#include "pserror.h"
#include "mono.h"
#include "object.h"
#include "player.h"
#include "hlsoundlib.h"
#include "weapon.h"
#include "damage.h"
#include "fireball.h"
#include "sounds.h"
#include "AIMain.h"
#include "multi.h"
#include "game.h"
#include "soundload.h"
#include "game2dll.h"
#include "scorch.h"
#include "ddio.h"
#include "vecmat.h"
#include "trigger.h"
#include "lighting.h"
#include "hud.h"
#include "D3ForceFeedback.h"
#include "demofile.h"
#include "osiris_dll.h"
#include "marker.h"
#include "hud.h"
#include "levelgoal.h"
#include "psrand.h"

#define PLAYER_ROTATION_BY_FORCE_SCALAR 0.12f
#define NONPLAYER_ROTATION_BY_FORCE_SCALAR 1.0f

ubyte CollisionResult[MAX_OBJECT_TYPES][MAX_OBJECT_TYPES];
ubyte CollisionRayResult[MAX_OBJECT_TYPES];

bool IsOKToApplyForce(object *objp)
{
	if(Game_mode & GM_MULTI)
	{
		if(objp->type == OBJ_PLAYER)
		{
			if(objp != Player_object)
				return false;
		}
		else
		{
			if(objp->type!=OBJ_WEAPON && objp->type!=OBJ_POWERUP && Netgame.local_role != LR_SERVER)
				return false;
		}
	}

	if (objp->mtype.phys_info.mass == 0.0)
		return false;

	if (objp->movement_type != MT_PHYSICS && objp->movement_type != MT_WALKING)
		return false;

	if(objp->mtype.phys_info.flags & PF_PERSISTENT)
		return false;

	if (objp->mtype.phys_info.flags & PF_LOCK_MASK)  // Not done!
		return false;

	return true;
}

//	-----------------------------------------------------------------------------
void bump_this_object(object *objp, object *other_objp, vector *force, vector *collision_pnt, int damage_flag)
{
//	float force_mag;

	if(objp->type == OBJ_PLAYER)
	{
		if((Game_mode & GM_MULTI) && (objp != Player_object))
			return;

		phys_apply_force(objp,force);
	}
	else
	{
		if((Game_mode & GM_MULTI) && (objp->type != OBJ_PLAYER && objp->type!=OBJ_POWERUP) && (Netgame.local_role != LR_SERVER))
			return;

		phys_apply_force(objp,force);
		phys_apply_rot(objp,force);
	}

//	if (! (objp->mtype.phys_info.flags & PF_PERSISTENT))
//		if (objp->type == OBJ_PLAYER) {
//			vector force2;
//			force2 = forcex/4;

//			phys_apply_force(objp,&force2);
			
//			if (damage_flag && ((other_objp->type != OBJ_ROBOT) || !Robot_info[other_objp->id].companion)) {
//				force_mag = vm_vec_mag_quick(&force2);
//				apply_force_damage(objp, force_mag, other_objp);
//			}
//		} else if ((objp->type == OBJ_ROBOT) || (objp->type == OBJ_CLUTTER) || (objp->type == OBJ_CNTRLCEN)) {
//			if (!Robot_info[objp->id].boss_flag) {
//				vector force2;
//				force2.x = force->x/(4 + Difficulty_level);
//				force2.y = force->y/(4 + Difficulty_level);
//				force2.z = force->z/(4 + Difficulty_level);

//				phys_apply_force(objp, force);
//				phys_apply_rot(objp, &force2);
//				if (damage_flag) {
//					force_mag = vm_vec_mag_quick(force);
//					apply_force_damage(objp, force_mag, other_objp);
//				}
//			}
//		}
}

/*
  
	 void bump_one_object(object *obj0, vector *hit_dir, float damage)
{
	vector	hit_vec;

	hit_vec = *hit_dir;
	vm_vec_scale(&hit_vec, damage);

	phys_apply_force(obj0,&hit_vec);

}
*/

//#define DAMAGE_SCALE 		128	//	Was 32 before 8:55 am on Thursday, September 15, changed by MK, walls were hurting me more than robots!
//#define DAMAGE_THRESHOLD 	(F1_0/3)
//#define WALL_LOUDNESS_SCALE (20)

//float force_force = 50.0;

typedef struct v{
	float i,j;
} vec2d;

#define cross(v0,v1) (((v0)->i * (v1)->j) - ((v0)->j * (v1)->i))

//finds the uv coords of the given point on the given seg & side
//fills in u & v. if l is non-NULL fills it in also
void FindHitpointUV(float *u,float *v,vector *point,room *rp,int facenum)
{
	face *fp = &rp->faces[facenum];
	int ii,jj;
	vec2d pnt[3],checkp,vec0,vec1;
	float *t;
	float k0,k1;
	int i;

	//1. find what plane to project this wall onto to make it a 2d case

	GetIJ(&fp->normal,&ii,&jj);

	//2. compute u,v of intersection point

	//Copy face points into 2d verts array
	for (i=0;i<3;i++) {
		t = &rp->verts[fp->face_verts[i]].x;
		pnt[i].i = t[ii];
		pnt[i].j = t[jj];
	}
	t = &point->x;
	checkp.i = t[ii];
	checkp.j = t[jj];

	//vec from 1 -> 0
	vec0.i = pnt[0].i - pnt[1].i;
	vec0.j = pnt[0].j - pnt[1].j;

	//vec from 1 -> 2
	vec1.i = pnt[2].i - pnt[1].i;
	vec1.j = pnt[2].j - pnt[1].j;

	k1 = -((cross(&checkp,&vec0) + cross(&vec0,&pnt[1])) / cross(&vec0,&vec1));
	if (std::abs(vec0.i) > std::abs(vec0.j))
		k0 = ((-k1 * vec1.i) + checkp.i - pnt[1].i) / vec0.i;
	else
		k0 = ((-k1 * vec1.j) + checkp.j - pnt[1].j) / vec0.j;

	//mprintf(0," k0,k1  = %x,%x\n",k0,k1);

	*u = fp->face_uvls[1].u + (k0 * (fp->face_uvls[0].u - fp->face_uvls[1].u)) + (k1 * (fp->face_uvls[2].u - fp->face_uvls[1].u));
	*v = fp->face_uvls[1].v + (k0 * (fp->face_uvls[0].v - fp->face_uvls[1].v)) + (k1 * (fp->face_uvls[2].v - fp->face_uvls[1].v));

	//mprintf(0," u,v    = %x,%x\n",*u,*v);
}

// Creates some effects where a weapon has collided with a wall
void DoWallEffects (object *weapon,int surface_tmap)
{
	texture *texp = &GameTextures[surface_tmap];

	if (texp->flags & (TF_VOLATILE+TF_LAVA+TF_WATER))
	{
		// Create some lava steam
		if ((texp->flags & TF_WATER) || ((ps_rand()%4)==0))
		{
			int visnum=VisEffectCreate (VIS_FIREBALL,MED_SMOKE_INDEX,weapon->roomnum,&weapon->pos);
			if (visnum>=0)
			{
				vis_effect *vis=&VisEffects[visnum];
	
				vis->lifetime=.9f;
				vis->lifeleft=.9f;

				vis->movement_type=MT_PHYSICS;
				vis->size=3.0;
				vm_MakeZero (&vis->velocity);
				vis->velocity.y=10;
			}	
		}
	}
	else if (texp->flags & TF_RUBBLE)
	{
		if ((ps_rand()%4)==0)
		{
			int num_rubble=(ps_rand()%3)+1;
			int bm_handle=GetTextureBitmap(texp-GameTextures,0);
			ushort *data=bm_data(bm_handle,0);
			
			ushort color=data[(bm_w(bm_handle,0)*(bm_h(bm_handle,0)/2))+(bm_w(bm_handle,0)/2)];

			for (int i=0;i<num_rubble;i++)
			{
				int visnum;
	
				visnum=VisEffectCreate (VIS_FIREBALL,RUBBLE1_INDEX+(ps_rand()%2),weapon->roomnum,&weapon->pos);
		
				if (visnum>=0)
				{
					vis_effect *vis=&VisEffects[visnum];
			
					vis->movement_type=MT_PHYSICS;
					vis->mass=100;
					vis->drag=.1f;

					vis->phys_flags|=PF_GRAVITY|PF_NO_COLLIDE;
				
					if ((ps_rand()%3)==0)
					{
						vis->velocity.x=(ps_rand()%100)-50;
						vis->velocity.y=-((ps_rand()%200)-30);
						vis->velocity.z=(ps_rand()%100)-50;
					}
					else
					{
						vis->velocity.x=(ps_rand()%300)-50;
						vis->velocity.y=-((ps_rand()%200)-30);
						vis->velocity.z=(ps_rand()%300)-50;
					}

					vm_NormalizeVectorFast (&vis->velocity);
					vis->velocity*=4+(ps_rand()%20);
					vis->size=.5+(((ps_rand()%11)-5)*.05);
					vis->flags|=VF_USES_LIFELEFT;
					float lifetime=1.0+(((ps_rand()%11)-5)*.1);
					vis->lifeleft=lifetime;
					vis->lifetime=lifetime;
					vis->lighting_color=color;
				}
			}
		}
	}
}

#define FORCEFIELD_DAMAGE 5.0f

void DeformTerrain (vector *pos,int depth,float size);

//Check for lava, volatile, or water surface.  If contact, make special sound & kill the weapon
void check_for_special_surface(object *weapon,int surface_tmap,vector *surface_normal,float hit_dot)
{
	bool f_forcefield,f_volatile,f_lava,f_water;

	f_forcefield = (GameTextures[surface_tmap].flags & TF_FORCEFIELD) != 0;
	f_volatile = (GameTextures[surface_tmap].flags & TF_VOLATILE) != 0;
	f_lava = (GameTextures[surface_tmap].flags & TF_LAVA) != 0;
	f_water = (GameTextures[surface_tmap].flags & TF_WATER) != 0;

	//Kill the weapon if the surface is volatile, lava, or water
	if (f_volatile || f_lava || f_water) {
		int snd;
		ain_hear hear;
		hear.f_directly_player = false;

		if (f_water)
		{
			snd = SOUND_WEAPON_HIT_WATER;
			hear.hostile_level = 0.2f;
			hear.curiosity_level = 0.3f;
		}
		else if (f_volatile || f_lava)
		{
			snd = SOUND_WEAPON_HIT_LAVA;
			hear.hostile_level = 0.1f;
			hear.curiosity_level = 0.5f;
		}
		else
			Int3();

		hear.max_dist = Sounds[snd].max_distance;
		Sound_system.Play3dSound(snd, SND_PRIORITY_NORMAL, weapon);
		AINotify(weapon, AIN_HEAR_NOISE, (void *)&hear);

		if (! f_water)
			DoWeaponExploded (weapon, surface_normal);

		SetObjectDeadFlag (weapon);
	}
}

//Process a collision between a weapon and a wall
//Returns true if the weapon hits the wall, and false if should keep going though the wall (for breakable glass)
bool collide_weapon_and_wall( object * weapon, fix hitspeed, int hitseg, int hitwall, vector * hitpnt, vector *wall_normal, float hit_dot)
{
	bool f_forcefield;
	bool f_volatile,f_lava,f_water;
	//mprintf((0, "Weapon hit wall, how nice.\n"));

	//#ifndef RELEASE
	if ((stricmp(Weapons[weapon->id].name,"Yellow flare") == 0) &&
				(weapon->parent_handle == Player_object->handle) &&
				(KEY_STATE(KEY_LAPOSTRO)))
		if (ROOMNUM_OUTSIDE(hitseg))
			AddHUDMessage("Terrain cell %d",CELLNUM(hitseg));
		else
			AddHUDMessage("Room %d face %d",hitseg,hitwall);
	//#endif

	//Check if forcefield
	int tmap;
	if (!ROOMNUM_OUTSIDE(hitseg))	 //Make sure we've hit a wall, and not terrain
	{
		tmap = Rooms[hitseg].faces[hitwall].tmap;
	}
	else
	{
		tmap = Terrain_tex_seg[Terrain_seg[CELLNUM(hitseg)].texseg_index].tex_index;
   }
	f_forcefield = (GameTextures[tmap].flags & TF_FORCEFIELD) != 0;
	f_volatile = (GameTextures[tmap].flags & TF_VOLATILE) != 0;
	f_lava = (GameTextures[tmap].flags & TF_LAVA) != 0;
	f_water = (GameTextures[tmap].flags & TF_WATER) != 0;


	if(f_forcefield && !(Weapons[weapon->id].flags & WF_MATTER_WEAPON))
	{
		ain_hear hear;
		hear.f_directly_player = false;
		hear.hostile_level = 0.9f;
		hear.curiosity_level = 0.1f;

		if(sound_override_force_field == -1)
			hear.max_dist = Sounds[SOUND_FORCEFIELD_BOUNCE].max_distance;
		else
			hear.max_dist = Sounds[sound_override_force_field].max_distance;
		
		AINotify(weapon, AIN_HEAR_NOISE, (void *)&hear);

		if(sound_override_force_field == -1)
			Sound_system.Play3dSound(SOUND_FORCEFIELD_BOUNCE,SND_PRIORITY_HIGH,weapon);
		else
			Sound_system.Play3dSound(sound_override_force_field,SND_PRIORITY_HIGH,weapon);

		return true;
	}

	//Check for destroyable or breakable face
	if (! ROOMNUM_OUTSIDE(hitseg)) //First, make sure we've hit a wall, and not terrain
	{		
		room *rp = &Rooms[hitseg];
		face *fp = &rp->faces[hitwall];

		//Check for a trigger on this wall
		CheckTrigger(hitseg,hitwall,weapon,TT_COLLIDE);

		// Check for a destroyable face
		if ((GameTextures[fp->tmap].flags & TF_DESTROYABLE) && !(fp->flags & FF_DESTROYED))
		{
			int visnum=CreateFireball (hitpnt,SHATTER_INDEX+(ps_rand()%2),hitseg,VISUAL_FIREBALL);

			if (visnum>=0)
			{
				// Alter the size of this explosion based on the face size
				vector verts[MAX_VERTS_PER_FACE];
				vector center;
				for (int t=0;t<fp->num_verts;t++)
					verts[t]=rp->verts[fp->face_verts[t]];

				float size=sqrt(vm_GetCentroid (&center,verts,fp->num_verts));
				VisEffects[visnum].size=(size/2);
			}

			CreateRandomSparks (20,hitpnt,hitseg);
	
			if(sound_override_glass_breaking == -1)
				Sound_system.Play3dSound(SOUND_BREAKING_GLASS,SND_PRIORITY_HIGH,weapon);
			else
				Sound_system.Play3dSound(sound_override_glass_breaking,SND_PRIORITY_HIGH,weapon);
			
			fp->flags|=FF_DESTROYED;
		}

		//Check for a breakable face: If the texture is breakable and it's on a portal 
		//and hit with a matter weapon, break it
		if ((fp->portal_num != -1) && (GameTextures[fp->tmap].flags & TF_BREAKABLE) && (Weapons[weapon->id].flags & WF_MATTER_WEAPON)) {
		
			//Do the breaking glass stuff
			if (!((Game_mode & GM_MULTI) && Netgame.local_role==LR_CLIENT))
				BreakGlassFace(rp,hitwall,hitpnt,&weapon->mtype.phys_info.velocity);

			ain_hear hear;
			hear.f_directly_player = false;
			hear.hostile_level = 0.9f;
			hear.curiosity_level = 1.0f;
			if(sound_override_glass_breaking == -1)
				hear.max_dist = Sounds[SOUND_BREAKING_GLASS].max_distance;
			else
				hear.max_dist = Sounds[sound_override_glass_breaking].max_distance;
			AINotify(weapon, AIN_HEAR_NOISE, (void *)&hear);

			return false;
		}
	}

	//Add a scorch
	if ((Weapons[weapon->id].scorch_handle != -1) && !f_water && !f_volatile && !f_lava)
		AddScorch(hitseg,hitwall,hitpnt,Weapons[weapon->id].scorch_handle,Weapons[weapon->id].scorch_size);

	if (ROOMNUM_OUTSIDE(hitseg))
	{
		if (Weapons[weapon->id].terrain_damage_size>0 && Weapons[weapon->id].terrain_damage_depth>0)
		{
			DeformTerrain (&weapon->pos,Weapons[weapon->id].terrain_damage_depth,Weapons[weapon->id].terrain_damage_size);
		}
	}

	//Do special smoke, etc.
	DoWallEffects (weapon,tmap);

	//look for lava, volatile, water
	check_for_special_surface(weapon,tmap,wall_normal,hit_dot);

	//If dead, we're done
	if (weapon->flags & OF_DEAD)
		return true;

	//If done bouncing, kill the weapon
	if ((weapon->mtype.phys_info.num_bounces <= 0) && !(weapon->mtype.phys_info.flags & PF_STICK) && (hit_dot > weapon->mtype.phys_info.hit_die_dot))
	{
		int snd;
		ain_hear hear;
		hear.f_directly_player = false;

		snd = Weapons[weapon->id].sounds[WSI_IMPACT_WALL];
		hear.hostile_level = 0.9f;
		hear.curiosity_level = 0.1f;

		if (snd != SOUND_NONE_INDEX)
		{
			hear.max_dist = Sounds[snd].max_distance;
			Sound_system.Play3dSound(snd, SND_PRIORITY_NORMAL, weapon);
			AINotify(weapon, AIN_HEAR_NOISE, (void *)&hear);
		}

		// Check to see if we should spawn
		if ((Weapons[weapon->id].flags & WF_SPAWNS_IMPACT) && Weapons[weapon->id].spawn_count>0 && Weapons[weapon->id].spawn_handle>=0)
			CreateImpactSpawnFromWeapon (weapon, wall_normal);

		if ((Weapons[weapon->id].flags & WF_SPAWNS_ROBOT) && (Weapons[weapon->id].flags & WF_COUNTERMEASURE) && Weapons[weapon->id].robot_spawn_handle>=0)
			CreateRobotSpawnFromWeapon (weapon);

		DoWeaponExploded (weapon, wall_normal);

		SetObjectDeadFlag (weapon);
	}
	else {	//weapon is bouncing

		if (Weapons[weapon->id].sounds[WSI_BOUNCE]!=SOUND_NONE_INDEX)
		{
			Sound_system.Play3dSound(Weapons[weapon->id].sounds[WSI_BOUNCE], SND_PRIORITY_HIGH, weapon);

			if(Weapons[weapon->id].sounds[WSI_BOUNCE] > -1)
			{
				ain_hear hear;
				hear.f_directly_player = false;
				hear.hostile_level = 0.20f;
				hear.curiosity_level = 0.6f;
				hear.max_dist = Sounds[Weapons[weapon->id].sounds[WSI_BOUNCE]].max_distance;
				AINotify(weapon, AIN_HEAR_NOISE, (void *)&hear);
			}
		}
	}

	return true;
}

// Prints out a marker hud message if needed
void collide_player_and_marker( object * playerobj, object * marker_obj, vector *collision_point, vector *collision_normal, bool f_reverse_normal, fvi_info *hit_info ) 
{
	if (playerobj->id==Player_num)
	{
		char str[100];
		sprintf (str,"Marker: %s",MarkerMessages[marker_obj->id]);
		AddHUDMessage (str);
	}
}

#define MIN_WALL_HIT_SOUND_VEL 40.0f
#define MAX_WALL_HIT_SOUND_VEL 120.0f
#define MIN_PLAYER_WALL_SOUND_TIME .1f
#define WALL_DAMAGE 0.5f
#define MIN_WALL_HIT_DAMAGE_SHIELDS	5
#define MIN_WALL_DAMAGE_SPEED			65.0

#define VOLATILE_DAMAGE	 7.0f		//damage per hit

void collide_player_and_wall( object * playerobj, float hitspeed, int hitseg, int hitwall, vector * hitpt, vector *wall_normal, float hit_dot)
{
	float volume = MAX_GAME_VOLUME;
	bool f_volatile,f_lava;
	bool f_forcefield;
	int tmap;

   if(playerobj->flags & OF_DYING)
		playerobj->ctype.dying_info.delay_time *= 0.9f;

	//Check for a trigger on this wall
	if (! ROOMNUM_OUTSIDE(hitseg))	 //Make sure we've hit a wall, and not terrain
	{
		CheckTrigger(hitseg,hitwall,playerobj,TT_COLLIDE);
		tmap = Rooms[hitseg].faces[hitwall].tmap;
	}
	else
	{
		tmap = Terrain_tex_seg[Terrain_seg[CELLNUM(hitseg)].texseg_index].tex_index;
	}
	f_lava = (GameTextures[tmap].flags & TF_LAVA) != 0;
	f_volatile = (GameTextures[tmap].flags & TF_VOLATILE) != 0;
	f_forcefield = (GameTextures[tmap].flags & TF_FORCEFIELD) != 0;
	
	if(f_lava)
	{
		if(!((Game_mode & GM_MULTI) && (Netgame.local_role==LR_CLIENT)))
		{
			int id = FindWeaponName("NapalmBlob");
			if(id >= 0)
				SetNapalmDamageEffect(playerobj,NULL,id);
			else
				Int3();
		}
	}

	//If volatile, make the sound & apply damage
	if (f_volatile)
	{
		if(playerobj == Player_object)
		{
			if ((!(Game_mode & GM_MULTI)) || Netgame.local_role==LR_SERVER)
				ApplyDamageToPlayer (playerobj, playerobj, PD_VOLATILE_HISS, VOLATILE_DAMAGE);
			else	{
				Multi_requested_damage_type = PD_VOLATILE_HISS;
				Multi_requested_damage_amount+=(VOLATILE_DAMAGE);
			}
		}
	}

	if(f_forcefield)
	{
		if(playerobj == Player_object && sound_override_force_field == -1)
		{
			if ((!(Game_mode & GM_MULTI)) || Netgame.local_role==LR_SERVER)
				ApplyDamageToPlayer (playerobj, playerobj, PD_ENERGY_WEAPON, FORCEFIELD_DAMAGE);
			else	{
				Multi_requested_damage_type = PD_ENERGY_WEAPON;
				Multi_requested_damage_amount+=(FORCEFIELD_DAMAGE);
			}
		}

		if(sound_override_force_field == -1)
			Sound_system.Play3dSound(SOUND_FORCEFIELD_BOUNCE, SND_PRIORITY_HIGH, playerobj, MAX_GAME_VOLUME);
		else
			Sound_system.Play3dSound(sound_override_force_field, SND_PRIORITY_HIGH, playerobj, MAX_GAME_VOLUME);

	}

	if(f_forcefield || f_lava || f_volatile)
	{
		ain_hear hear;
		hear.f_directly_player = true;
		hear.hostile_level = 0.90f;
		hear.curiosity_level = 0.6f;
		if(sound_override_force_field == -1)
			hear.max_dist = Sounds[SOUND_FORCEFIELD_BOUNCE].max_distance;
		else
			hear.max_dist = Sounds[sound_override_force_field].max_distance;
		AINotify(playerobj, AIN_HEAR_NOISE, (void *)&hear);
	}

	// Do sound stuff when a player hits a wall
	if(!f_forcefield && !f_volatile && !f_lava && hitspeed > MIN_WALL_HIT_SOUND_VEL && Players[playerobj->id].last_hit_wall_sound_time + MIN_PLAYER_WALL_SOUND_TIME < Gametime)
	{
		if(hitspeed < MAX_WALL_HIT_SOUND_VEL)
		{
			volume = MAX_GAME_VOLUME * (hitspeed - MIN_WALL_HIT_SOUND_VEL)/(MAX_WALL_HIT_SOUND_VEL - MIN_WALL_HIT_SOUND_VEL);
		}

		if(hitspeed > MIN_WALL_DAMAGE_SPEED)
		{
			if(playerobj == Player_object && playerobj->shields >= MIN_WALL_HIT_DAMAGE_SHIELDS)
			{
				if ((!(Game_mode & GM_MULTI)) || Netgame.local_role==LR_SERVER)
					ApplyDamageToPlayer (playerobj, playerobj, PD_WALL_HIT, WALL_DAMAGE);
				else {
					Multi_requested_damage_type = PD_WALL_HIT;
					Multi_requested_damage_amount+=(WALL_DAMAGE);
				}
			}
		}


		Sound_system.Play3dSound(SOUND_PLAYER_HIT_WALL, SND_PRIORITY_NORMAL, playerobj, volume);
		if(Demo_flags==DF_RECORDING)
			DemoWrite3DSound(SOUND_PLAYER_HIT_WALL,OBJNUM(playerobj),1,volume);

		ain_hear hear;
		hear.f_directly_player = true;
		hear.hostile_level = 0.10f;
		hear.curiosity_level = 0.5f;
		hear.max_dist = Sounds[SOUND_PLAYER_HIT_WALL].max_distance * volume;
		AINotify(playerobj, AIN_HEAR_NOISE, (void *)&hear);

		Players[playerobj->id].last_hit_wall_sound_time = Gametime;
	}

	if (Players[playerobj->id].flags & PLAYER_FLAGS_DYING) 
		StartPlayerExplosion(playerobj->id);

	//Do ForceFeedback 
	//----------------
	if(playerobj->id == Player_num && ForceIsEnabled() )
	{
		DoForceForWall(playerobj,hitspeed,hitseg,hitwall,wall_normal);
	}

	return;
}

void collide_generic_and_wall( object * genericobj, float hitspeed, int hitseg, int hitwall, vector * hitpt, vector *wall_normal, float hit_dot)
{
	bool f_volatile,f_lava;
	bool f_forcefield;
	float volume = MAX_GAME_VOLUME;

	//Check for a trigger on this wall
	int tmap;
	if (! ROOMNUM_OUTSIDE(hitseg))	 //Make sure we've hit a wall, and not terrain
	{
		CheckTrigger(hitseg,hitwall,genericobj,TT_COLLIDE);
		tmap = Rooms[hitseg].faces[hitwall].tmap;
	}
	else
	{
		tmap = Terrain_tex_seg[Terrain_seg[CELLNUM(hitseg)].texseg_index].tex_index;
	}
	f_forcefield = (GameTextures[tmap].flags & TF_FORCEFIELD) != 0;
	f_volatile = (GameTextures[tmap].flags & TF_VOLATILE) != 0;
	f_lava = (GameTextures[tmap].flags & TF_LAVA) != 0;
	
	if(IS_GUIDEBOT(genericobj))
	{
		f_forcefield = false;
		f_volatile = false;
		f_lava = false;
	}

	if(f_lava)
	{
		if(!((Game_mode & GM_MULTI) && (Netgame.local_role==LR_CLIENT)))
		{
			int id = FindWeaponName("NapalmBlob");
			if(id >= 0)
				SetNapalmDamageEffect(genericobj,NULL,id);
			else
				Int3();
		}
	}

	//If volatile, make the sound & apply damage
	if (f_volatile && !((Game_mode & GM_MULTI) && (Netgame.local_role==LR_CLIENT))) {
		ApplyDamageToGeneric(genericobj, genericobj, GD_VOLATILE_HISS, VOLATILE_DAMAGE);
		Sound_system.Play3dSound(SOUND_VOLATILE_HISS, SND_PRIORITY_HIGHEST, genericobj, MAX_GAME_VOLUME);
	}

	if(f_forcefield && !((Game_mode & GM_MULTI) && (Netgame.local_role==LR_CLIENT)))
	{
		if(sound_override_force_field == -1)
		{
			Sound_system.Play3dSound(SOUND_FORCEFIELD_BOUNCE, SND_PRIORITY_LOW, genericobj, MAX_GAME_VOLUME);
			ApplyDamageToGeneric(genericobj, genericobj, GD_ENERGY, FORCEFIELD_DAMAGE);
		}
		else
		{
			Sound_system.Play3dSound(sound_override_force_field, SND_PRIORITY_LOW, genericobj, MAX_GAME_VOLUME);
		}
	}

	if(genericobj->control_type != CT_AI)
	{
		if(!f_forcefield && !f_volatile && !f_lava && hitspeed > MIN_WALL_HIT_SOUND_VEL)
		{
			if(hitspeed < MAX_WALL_HIT_SOUND_VEL)
			{
				volume = MAX_GAME_VOLUME * (hitspeed - MIN_WALL_HIT_SOUND_VEL)/(MAX_WALL_HIT_SOUND_VEL - MIN_WALL_HIT_SOUND_VEL);
			}

			if(hitspeed > MIN_WALL_DAMAGE_SPEED)
			{
				if ((!(Game_mode & GM_MULTI)) || Netgame.local_role==LR_SERVER)
				{
					ApplyDamageToGeneric(genericobj, genericobj, GD_PHYSICS, WALL_DAMAGE);
				}
			}


			Sound_system.Play3dSound(SOUND_PLAYER_HIT_WALL, SND_PRIORITY_LOW, genericobj, volume);
			if(Demo_flags==DF_RECORDING)
				DemoWrite3DSound(SOUND_PLAYER_HIT_WALL,OBJNUM(genericobj),1,volume);

		}
	}

	return;
}


float	Last_volatile_scrape_sound_time = 0;

//this gets called when an object is scraping along the wall
void scrape_object_on_wall(object *obj, int hitseg, int hitwall, vector * hitpt, vector *wall_normal )
{
/*	switch (obj->type) {

		case OBJ_PLAYER:

			if (obj->id==Player_num) {
				int type;

				//mprintf((0, "Scraped segment #%3i, side #%i\n", hitseg, hitside));

				if ((type=check_volatile_wall(obj,hitseg,hitside,hitpt))!=0) {
					vector	hit_dir, rand_vec;

					if ((GameTime > Last_volatile_scrape_sound_time + F1_0/4) || (GameTime < Last_volatile_scrape_sound_time)) {
						int sound = (type==1)?SOUND_VOLATILE_WALL_HISS:SOUND_SHIP_IN_WATER;

						Last_volatile_scrape_sound_time = GameTime;

						digi_link_sound_to_pos( sound, hitseg, 0, hitpt, 0, F1_0 );
						if (Game_mode & GM_MULTI)
							multi_send_play_sound(sound, F1_0);
					}

					#ifdef COMPACT_SEGS
						get_side_normal(&Segments[hitseg], higside, 0, &hit_dir );	
					#else
						hit_dir = Segments[hitseg].sides[hitside].normals[0];
					#endif
			
					make_random_vector(&rand_vec);
					vm_vec_scale_add2(&hit_dir, &rand_vec, F1_0/8);
					vm_vec_normalize_quick(&hit_dir);
					bump_one_object(obj, &hit_dir, F1_0*8);
				}

				//@@} else {
				//@@	//what scrape sound
				//@@	//PLAY_SOUND( SOUND_PLAYER_SCRAPE_WALL );
				//@@}
		
			}

			break;

		//these two kinds of objects below shouldn't really slide, so
		//if this scrape routine gets called (which it might if the
		//object (such as a fusion blob) was created already poking
		//through the wall) call the collide routine.

		case OBJ_WEAPON:
			collide_weapon_and_wall(obj,0,hitseg,hitside,hitpt); 
			break;

		case OBJ_DEBRIS:		
			collide_debris_and_wall(obj,0,hitseg,hitside,hitpt); 
			break;
	}
*/
}
/*
void apply_damage_to_player(object *playerobj, object *killer, float damage)
{
	if (Player_is_dead)
		return;

	if (Players[Player_num].flags & PLAYER_FLAGS_INVULNERABLE)
		return;

	if (Endlevel_sequence)
		return;

	//for the player, the 'real' shields are maintained in the Players[]
	//array.  The shields value in the player's object are, I think, not
	//used anywhere.  This routine, however, sets the objects shields to
	//be a mirror of the value in the Player structure. 

	if (playerobj->id == Player_num) {		//is this the local player?

		//	MK: 08/14/95: This code can never be reached.  See the return about 12 lines up.
// -- 		if (Players[Player_num].flags & PLAYER_FLAGS_INVULNERABLE) {
// -- 
// -- 			//invincible, so just do blue flash
// -- 
// -- 			PALETTE_FLASH_ADD(0,0,f2i(damage)*4);	//flash blue
// -- 
// -- 		} 
// -- 		else {		//take damage, do red flash

			Players[Player_num].shields -= damage;

			PALETTE_FLASH_ADD(f2i(damage)*4,-f2i(damage/2),-f2i(damage/2));	//flash red

// -- 		}

		if (Players[Player_num].shields < 0)	{

  			Players[Player_num].killer_objnum = killer-Objects;
			
//			if ( killer && (killer->type == OBJ_PLAYER))
//				Players[Player_num].killer_objnum = killer-Objects;

			playerobj->flags |= OF_DEAD;

			if (Buddy_objnum != -1)
				if ((killer->type == OBJ_ROBOT) && (Robot_info[killer->id].companion))
					Buddy_sorry_time = GameTime;
		}
// -- removed, 09/06/95, MK --  else if (Players[Player_num].shields < LOSE_WEAPON_THRESHOLD) {
// -- removed, 09/06/95, MK -- 			int	randnum = ps_rand();
// -- removed, 09/06/95, MK -- 
// -- removed, 09/06/95, MK -- 			if (floatmul(Players[Player_num].shields, randnum) < damage/4) {
// -- removed, 09/06/95, MK -- 				if (ps_rand() > 20000) {
// -- removed, 09/06/95, MK -- 					destroy_secondary_weapon(Secondary_weapon);
// -- removed, 09/06/95, MK -- 				} else if (Primary_weapon == 0) {
// -- removed, 09/06/95, MK -- 					if (Players[Player_num].flags & PLAYER_FLAGS_QUAD_LASERS)
// -- removed, 09/06/95, MK -- 						destroy_primary_weapon(MAX_PRIMARY_WEAPONS);	//	This means to destroy quad laser.
// -- removed, 09/06/95, MK -- 					else if (Players[Player_num].laser_level > 0)
// -- removed, 09/06/95, MK -- 						destroy_primary_weapon(Primary_weapon);
// -- removed, 09/06/95, MK -- 				} else
// -- removed, 09/06/95, MK -- 					destroy_primary_weapon(Primary_weapon);
// -- removed, 09/06/95, MK -- 			} else
// -- removed, 09/06/95, MK -- 				; // mprintf((0, "%8x > %8x, so don't lose weapon.\n", floatmul(Players[Player_num].shields, randnum), damage/4));
// -- removed, 09/06/95, MK -- 		}

		playerobj->shields = Players[Player_num].shields;		//mirror

	}
}
*/

void CollideAnglesToMatrix(matrix *m, float p, float h, float b)
{
	float sinp,cosp,sinb,cosb,sinh,cosh;

   sinp = sin(p); cosp = cos(p);
   sinb = sin(b); cosb = cos(b);
   sinh = sin(h); cosh = cos(h);

	m->rvec.x = cosb*cosh;
	m->rvec.y = cosb*sinp*sinh - cosp*sinb;
	m->rvec.z = cosb*cosp*sinh + sinb*sinp;

	m->uvec.x = cosh*sinb;
	m->uvec.y = sinb*sinp*sinh + cosp*cosb;
	m->uvec.z = sinb*cosp*sinh - cosb*sinp;

	m->fvec.x = -sinh;
	m->fvec.y = sinp*cosh;
	m->fvec.z = cosp*cosh;
}

vector *CollideExtractAnglesFromMatrix(vector *a,matrix *m)
{
	float sinh,cosh,sinp,sinb;

	sinh = -m->fvec.x;
	if(sinh < -1.0f) sinh = -1.0f;
	else if(sinh > 1.0f) sinh = 1.0f;
	a->y = asin(sinh);

	cosh = cos(a->y);
	ASSERT(cosh != 0.0);

	sinp = m->fvec.y/cosh;
	if(sinp < -1.0f) sinp = -1.0f;
	else if(sinp > 1.0f) sinp = 1.0f;

	sinb = m->uvec.x/cosh;
	if(sinb < -1.0f) sinb = -1.0f;
	else if(sinb > 1.0f) sinb = 1.0f;

	a->x = asin(sinp);
	a->z = asin(sinb);

	return a;
}

void ConvertEulerToAxisAmount(vector *e, vector *n, float *w)
{
	float rotspeed = vm_GetMagnitude(e);
	matrix rotmat;
	vector e_n;  
	float scale = rotspeed/.0001f;

	// If there isn't a rotation, return something valid
	if(rotspeed == 0.0f || scale == 0.0f)
	{
		*n = Zero_vector;
		n->y = 1.0f;
		*w = 0.0f;

		return;
	}

	e_n = *e / scale;

//	vector f;
	CollideAnglesToMatrix(&rotmat, e_n.x, e_n.y, e_n.z);

//	mprintf((0, "F %f, %f, %f\n", XYZ(&rotmat.fvec)));
//	mprintf((0, "R %f, %f, %f\n", XYZ(&rotmat.rvec)));
//	mprintf((0, "U %f, %f, %f\n", XYZ(&rotmat.uvec)));
	
//	CollideExtractAnglesFromMatrix(&f, &rotmat);

//	mprintf((0, "Before %f, %f, %f\n", XYZ(&e_n)));
//	mprintf((0, "After  %f, %f, %f\n", XYZ(&f)));

	// This is from Graphics Gems 1 p.467  I am converting from a angle vector 
	// to the normal of that rotation (you can also get the angle about that normal, but
	// we don't need it)
	n->x = rotmat.uvec.z - rotmat.fvec.y;
	n->y = rotmat.fvec.x - rotmat.rvec.z;
	n->z = rotmat.rvec.y - rotmat.uvec.x;
	
	if(*n != Zero_vector)
	{
		vm_NormalizeVector(n);
		
		float ct = (rotmat.rvec.x + rotmat.uvec.y + rotmat.fvec.z - 1.0f)/2.0f;
		if(ct < -1.0f) ct = -1.0f;
		else if(ct > 1.0f) ct = 1.0f;

		float v = acos(ct);
		float z = sin(v);

//		if(v < 0.0)
//			*w = -rotspeed;
//		else
		*w = rotspeed * ((2.0f * PI)/(65535.0f));

		if(z >= 0.0f)
			*n *= -1.0f;
	}
	else
	{
		*w = 0.0f;
	}
}

void ConvertAxisAmountToEuler(vector *n, float *w, vector *e)
{
	float s;
	float c;
	float t;

	float scale = *w/.0001f;
	float w_n = .0001f;
	vector s_result;

	if(*w == 0.0f)
	{
		*e = Zero_vector;
		return;
	}

	s = sin(.0001);
	c = cos(.0001);
	t = 1.0f - c;

	matrix rotmat;
	const float sx = s * n->x;
	const float sy = s * n->y;
	const float sz = s * n->z;
	const float txy = t * n->x * n->y;
	const float txz = t * n->x * n->z;
	const float tyz = t * n->y * n->z;
	const float txx = t * n->x * n->x;
	const float tyy = t * n->y * n->y;
	const float tzz = t * n->z * n->z;

	rotmat.rvec.x = txx + c;
	rotmat.rvec.y = txy - sz;
	rotmat.rvec.z = txz + sy;
	rotmat.uvec.x = txy + sz;
	rotmat.uvec.y = tyy + c;
	rotmat.uvec.z = tyz - sx;
	rotmat.fvec.x = txz - sy;
	rotmat.fvec.y = tyz + sx;
	rotmat.fvec.z = tzz + c;

	CollideExtractAnglesFromMatrix(&s_result, &rotmat);

	e->x = (s_result.x) * scale * (65535.0f/(2.0*PI));
	e->y = (s_result.y) * scale * (65535.0f/(2.0*PI));
	e->z = (s_result.z) * scale * (65535.0f/(2.0*PI));
}

void bump_obj_against_fixed(object *obj, vector *collision_point, vector *collision_normal)
{
	ASSERT(_finite(obj->mtype.phys_info.rotvel.x) != 0);
	ASSERT(_finite(obj->mtype.phys_info.rotvel.y) != 0);
	ASSERT(_finite(obj->mtype.phys_info.rotvel.z) != 0);
	ASSERT(_finite(obj->mtype.phys_info.velocity.x) != 0);
	ASSERT(_finite(obj->mtype.phys_info.velocity.y) != 0);
	ASSERT(_finite(obj->mtype.phys_info.velocity.z) != 0);

	if(!IsOKToApplyForce(obj))
		return;

	vector r1 = *collision_point - obj->pos;
	vector w1;
	vector n1;
	float temp1;

	float j;

	matrix o_t1 = obj->orient;
	vm_TransposeMatrix(&o_t1);

	vector cmp1 = obj->mtype.phys_info.rotvel * o_t1;

	ConvertEulerToAxisAmount(&cmp1, &n1, &temp1);
	
	n1 *= temp1;

	if(temp1 != 0.0f)
	{
		vm_CrossProduct(&w1, &n1, &r1);
	}
	else
	{
		w1 = Zero_vector;
	}

	vector p1 = obj->mtype.phys_info.velocity + w1;
	float v_rel;

	float m1 = obj->mtype.phys_info.mass;

	ASSERT(m1 != 0.0f);
	if(m1 <= 0.0f) m1 = 0.00000001f;

	v_rel = *collision_normal * (p1);

	float e = obj->mtype.phys_info.coeff_restitution;
	
	vector c1;
	vector cc1;
	float cv1;

//	matrix i1;
//	matrix i2;
	float i1 = (2.0f/5.0f)*m1*obj->size*obj->size;

	if(i1 < .0000001)
		i1 = .0000001f;

	vm_CrossProduct(&c1, &r1, collision_normal);

	c1 = c1/i1;
	
	vm_CrossProduct(&cc1, &c1, &r1);

	cv1 = (*collision_normal)*c1;

	j = (-(1.0f + e))*v_rel;
	j /= (1/m1 + cv1);

	obj->mtype.phys_info.velocity += ((j*(*collision_normal))/m1);
	
	vector jcn = j * (*collision_normal);

	vm_CrossProduct(&c1, &r1, &jcn);

	n1 = (c1)/i1;

	temp1 = vm_NormalizeVector(&n1);

	vector txx1;

	ConvertAxisAmountToEuler(&n1, &temp1, &txx1);

	obj->mtype.phys_info.rotvel += (txx1*obj->orient);
	
	ASSERT(_finite(obj->mtype.phys_info.rotvel.x) != 0);
	ASSERT(_finite(obj->mtype.phys_info.rotvel.y) != 0);
	ASSERT(_finite(obj->mtype.phys_info.rotvel.z) != 0);
	ASSERT(_finite(obj->mtype.phys_info.velocity.x) != 0);
	ASSERT(_finite(obj->mtype.phys_info.velocity.y) != 0);
	ASSERT(_finite(obj->mtype.phys_info.velocity.z) != 0);

	//hack rotvel
/*	vector v = obj->mtype.phys_info.velocity;
	vector c = *collision_normal;

	if(v != Zero_vector)
	{
		vm_NormalizeVector(&v);

		if(v != c)
		{
			float rad = vm_VectorDistance(&obj->pos, collision_point);
			float rotvel = vm_GetMagnitude(&obj->mtype.phys_info.velocity)*((2*PI*rad));
			vector r;
			vector e;

			vm_CrossProduct(&r, &v, &c);
			vm_NormalizeVector(&r);

			ConvertAxisAmountToEuler(&r, &rotvel, &e);

			matrix rrr = obj->orient;
//			vm_TransposeMatrix(&rrr);
			obj->mtype.phys_info.rotvel = rrr * r;
		}
		else
		{
			obj->mtype.phys_info.rotvel = Zero_vector;
		}
	}
	else
	{
		obj->mtype.phys_info.rotvel = Zero_vector;
	}*/
}

void bump_two_objects(object *object0, object *object1, vector *collision_point, vector *collision_normal, int damage_flag) 
{
//	vector force;	//dv, 
	object *t = NULL;
	object *other = NULL;

	// Determine if a moving object hits a non-moving object
	if((object0->movement_type != MT_PHYSICS && object0->movement_type != MT_WALKING) || 
		(object0->movement_type == MT_PHYSICS && object0->mtype.phys_info.velocity == Zero_vector && (object0->mtype.phys_info.flags & PF_LOCK_MASK) && (object0->mtype.phys_info.flags & PF_POINT_COLLIDE_WALLS)))
	{
		t = object1;
		other = object0;
		*collision_normal *= -1.0f;
	}
	if((object1->movement_type != MT_PHYSICS && object1->movement_type != MT_WALKING) || 
		(object1->movement_type == MT_PHYSICS && object1->mtype.phys_info.velocity == Zero_vector && (object1->mtype.phys_info.flags & PF_LOCK_MASK) && (object1->mtype.phys_info.flags & PF_POINT_COLLIDE_WALLS)))
	{
		t = object0;
		other = object1;
	}

	// If we hit a non-moving object...
	if (t) 
	{
		// chrishack -- walker hack
		if(t->movement_type != MT_PHYSICS && t->movement_type != MT_WALKING)
		{
			t->mtype.phys_info.velocity = Zero_vector;
			return;
		}

		if(t->mtype.phys_info.flags & PF_PERSISTENT)
			return;

		vector moved_v;
		float wall_part;

		float luke_test;

		if(t->type == OBJ_PLAYER)
		{
			luke_test = vm_GetMagnitude(&t->mtype.phys_info.velocity);
		}

		if (!(t->flags & OF_DEAD))	
		{
//			bump_obj_against_fixed(t, collision_point, collision_normal);
//		}	
///*
			// Find hit speed	
			moved_v = t->pos - t->last_pos; 
			wall_part = *collision_normal * t->mtype.phys_info.velocity;

			if (t->mtype.phys_info.flags & PF_BOUNCE) 
			{ 
				wall_part *= 2.0;	//Subtract out wall part twice to achieve bounce

							// New bounceness code
				if ((t->mtype.phys_info.flags & PF_BOUNCE) && (t->mtype.phys_info.num_bounces != PHYSICS_UNLIMITED_BOUNCE)) 
				{
					if(t->mtype.phys_info.num_bounces == 0) 
					{
						ASSERT (t->type!=OBJ_PLAYER);
						if (t->flags & OF_DYING) {
							ASSERT((t->control_type == CT_DYING) || (t->control_type == CT_DYING_AND_AI));
							DestroyObject(t,50.0,t->ctype.dying_info.death_flags);
						}
						else
							SetObjectDeadFlag (t);
					}
				}
				t->mtype.phys_info.num_bounces--;

				t->mtype.phys_info.velocity += *collision_normal * (wall_part * -1.0f);
				
				if(t->mtype.phys_info.coeff_restitution != 1.0f) 
					t->mtype.phys_info.velocity -= (t->mtype.phys_info.velocity * (1.0f - t->mtype.phys_info.coeff_restitution));
			}
			else
			{
				float wall_force;

				wall_force = t->mtype.phys_info.thrust * *collision_normal;
				t->mtype.phys_info.thrust += *collision_normal * (wall_force * -1.001); // 1.001 so that we are not quite tangential

				// Update velocity from wall hit.  
				t->mtype.phys_info.velocity += *collision_normal * (wall_part * -1.001); // 1.001 so that we are not quite tangential
				if(t->type == OBJ_PLAYER)
				{
					float real_vel;

					real_vel = vm_NormalizeVector(&t->mtype.phys_info.velocity);
					t->mtype.phys_info.velocity *= ((real_vel + luke_test)/2.0f);
					//obj->mtype.phys_info.velocity *= (luke_test);
				}
			}
		
			// Weapons should face their new heading.  This is so missiles are pointing in the correct direct.
			if (t->type == OBJ_WEAPON && (t->mtype.phys_info.flags & (PF_BOUNCE | PF_GRAVITY | PF_WIND )))
				vm_VectorToMatrix(&t->orient, &t->mtype.phys_info.velocity, &t->orient.uvec, NULL);
		}

		// Return it to the original direction
		if(object0->movement_type != MT_PHYSICS && object0->movement_type != MT_WALKING)
		{
			*collision_normal *= -1.0f;
		}

		//Do ForceFeedback stuff for non-moving objects
		//-----------------------------------------
		if(ForceIsEnabled()){

			if(object0->type==OBJ_PLAYER && object0->id==Player_num){
			}
		}
//*/		
		return;
	}

//	force = object0->mtype.phys_info.velocity - object1->mtype.phys_info.velocity;
//	force *= 2*(object0->mtype.phys_info.mass * object1->mtype.phys_info.mass)/(object0->mtype.phys_info.mass + object1->mtype.phys_info.mass);

//	if(!(object1->mtype.phys_info.flags & PF_PERSISTENT))
//		bump_this_object(object1, object0, &force, collision_point, damage_flag);
//	
//	force = -force;
//
//	if(!(object0->mtype.phys_info.flags & PF_PERSISTENT))
//		bump_this_object(object0, object1, &force, collision_point, damage_flag);


//	vector r_vel = object0->mtype.phys_info.velocity - object1->mtype.phys_info.velocity;
//Add this back

	ASSERT(_finite(object1->mtype.phys_info.rotvel.x) != 0);
	ASSERT(_finite(object1->mtype.phys_info.rotvel.y) != 0);
	ASSERT(_finite(object1->mtype.phys_info.rotvel.z) != 0);
	ASSERT(_finite(object0->mtype.phys_info.rotvel.x) != 0);
	ASSERT(_finite(object0->mtype.phys_info.rotvel.y) != 0);
	ASSERT(_finite(object0->mtype.phys_info.rotvel.z) != 0);
	ASSERT(_finite(object1->mtype.phys_info.velocity.x) != 0);
	ASSERT(_finite(object1->mtype.phys_info.velocity.y) != 0);
	ASSERT(_finite(object1->mtype.phys_info.velocity.z) != 0);
	ASSERT(_finite(object0->mtype.phys_info.velocity.x) != 0);
	ASSERT(_finite(object0->mtype.phys_info.velocity.y) != 0);
	ASSERT(_finite(object0->mtype.phys_info.velocity.z) != 0);

	vector r1 = *collision_point - object0->pos;
	vector r2 = *collision_point - object1->pos;
	vector w1;
	vector w2;
	vector n1;
	vector n2;
	float temp1;
	float temp2;

	float j;

	matrix o_t1 = object0->orient;
	matrix o_t2 = object1->orient;

	vm_TransposeMatrix(&o_t1);
	vm_TransposeMatrix(&o_t2);

	vector cmp1 = object0->mtype.phys_info.rotvel * o_t1;
	vector cmp2 = object1->mtype.phys_info.rotvel * o_t2;

	ConvertEulerToAxisAmount(&cmp1, &n1, &temp1);
	ConvertEulerToAxisAmount(&cmp2, &n2, &temp2);
	
	n1 *= temp1;
	n2 *= temp2;

	if(temp1 != 0.0f)
	{
		vm_CrossProduct(&w1, &n1, &r1);
	}
	else
	{
		w1 = Zero_vector;
	}

	if(temp2 != 0.0f)
	{
		vm_CrossProduct(&w2, &n2, &r2);
	}
	else
	{
		w2 = Zero_vector;
	}

	vector p1 = object0->mtype.phys_info.velocity + w1;
	vector p2 = object1->mtype.phys_info.velocity + w2;
	float v_rel;

	float m1 = object0->mtype.phys_info.mass;
	float m2 = object1->mtype.phys_info.mass;

	bool f_force_1 = IsOKToApplyForce(object0);
	bool f_force_2 = IsOKToApplyForce(object1);

	ASSERT(m1 != 0.0f && m2 != 0.0f);
	if(m1 <= 0.0f) m1 = 0.00000001f;
	if(m2 <= 0.0f) m2 = 0.00000001f;

	v_rel = *collision_normal * (p1 - p2);

	float e;
	
	if(object0->type == OBJ_PLAYER && object1->type == OBJ_PLAYER)
		e = 0.015f;
	else if((object0->type == OBJ_WEAPON || object1->type == OBJ_WEAPON) && (object0->type != OBJ_PLAYER && object1->type != OBJ_PLAYER))
		e = 1.0f;
	else if((object0->type == OBJ_CLUTTER && object1->type == OBJ_PLAYER) || (object0->type == OBJ_PLAYER && object1->type == OBJ_CLUTTER))
		e = 0.5f;
	else
		e = 0.1f;

	vector c1;
	vector c2;
	vector cc1;
	vector cc2;
	float cv1;
	float cv2;

//	matrix i1;
//	matrix i2;
	float i1 = (2.0f/5.0f)*m1*object0->size*object0->size;
	float i2 = (2.0f/5.0f)*m2*object1->size*object1->size;

	if(i1 < .0000001)
		i1 = .0000001f;
	if(i2 < .0000001)
		i2 = .0000001f;

	vm_CrossProduct(&c1, &r1, collision_normal);
	vm_CrossProduct(&c2, &r2, collision_normal);

	c1 = c1/i1;
	c2 = c2/i2;
	
	vm_CrossProduct(&cc1, &c1, &r1);
	vm_CrossProduct(&cc2, &c2, &r2);

	cv1 = (*collision_normal)*c1;
	cv2 = (*collision_normal)*c2;

	j = (-(1.0f + e))*v_rel;
	j /= (1/m1 + 1/m2 + cv1 + cv2);

	if(f_force_1)
		object0->mtype.phys_info.velocity += ((j*(*collision_normal))/m1);
	
	if(f_force_2)
		object1->mtype.phys_info.velocity -= ((j*(*collision_normal))/m2);

	vector jcn = j * (*collision_normal);

	vm_CrossProduct(&c1, &r1, &jcn);
	vm_CrossProduct(&c2, &r2, &jcn);

	n1 = (c1)/i1;
	n2 = (c2)/i2;

	temp1 = vm_NormalizeVector(&n1);
	temp2 = vm_NormalizeVector(&n2);

	vector txx1;
	vector txx2;

	ConvertAxisAmountToEuler(&n1, &temp1, &txx1);
	ConvertAxisAmountToEuler(&n2, &temp2, &txx2);

	float rotscale1, rotscale2;

	if(object0->type == OBJ_PLAYER)
		rotscale1 = PLAYER_ROTATION_BY_FORCE_SCALAR;
	else
		rotscale1 = NONPLAYER_ROTATION_BY_FORCE_SCALAR;

	if(object1->type == OBJ_PLAYER)
		rotscale2 = PLAYER_ROTATION_BY_FORCE_SCALAR;
	else
		rotscale2 = NONPLAYER_ROTATION_BY_FORCE_SCALAR;

	if(f_force_1)
		object0->mtype.phys_info.rotvel += (txx1*object0->orient) * rotscale1;
	
	if(f_force_2)
		object1->mtype.phys_info.rotvel += (txx2*object1->orient) * rotscale2;

	ASSERT(_finite(object1->mtype.phys_info.rotvel.x) != 0);
	ASSERT(_finite(object1->mtype.phys_info.rotvel.y) != 0);
	ASSERT(_finite(object1->mtype.phys_info.rotvel.z) != 0);
	ASSERT(_finite(object0->mtype.phys_info.rotvel.x) != 0);
	ASSERT(_finite(object0->mtype.phys_info.rotvel.y) != 0);
	ASSERT(_finite(object0->mtype.phys_info.rotvel.z) != 0);
	ASSERT(_finite(object1->mtype.phys_info.velocity.x) != 0);
	ASSERT(_finite(object1->mtype.phys_info.velocity.y) != 0);
	ASSERT(_finite(object1->mtype.phys_info.velocity.z) != 0);
	ASSERT(_finite(object0->mtype.phys_info.velocity.x) != 0);
	ASSERT(_finite(object0->mtype.phys_info.velocity.y) != 0);
	ASSERT(_finite(object0->mtype.phys_info.velocity.z) != 0);

	//Do ForceFeedback stuff for moving objects
	//-----------------------------------------
	if(ForceIsEnabled()){

		if(object0->type==OBJ_PLAYER && object0->id==Player_num){
			//v is the force vector
			vector v;
			v = -1.0 * v_rel * (*collision_normal);

			//Was it weapon->player collide
			switch(object1->type){
			case OBJ_WEAPON:
				//Do force effect for player/weapon collision
				DoForceForWeapon(object0,object1,&v);
				break;
			default:
				break;
			}
		}
	}

	// Catch things on fire if possible
	if (object0->effect_info && object1->effect_info)
	{
		// One of these objects must be burning
		if ((object0->effect_info->type_flags & EF_NAPALMED) || (object1->effect_info->type_flags & EF_NAPALMED))
		{
			// Both cannot be burning
			if (!(object0->effect_info->type_flags & EF_NAPALMED) || !(object1->effect_info->type_flags & EF_NAPALMED))
			{
				object *src_obj,*dest_obj;
				if (object0->effect_info->type_flags & EF_NAPALMED)
				{
					src_obj=object0;
					dest_obj=object1;
				}
				else
				{
					src_obj=object1;
					dest_obj=object0;
				}

				dest_obj->effect_info->type_flags|=EF_NAPALMED;

				dest_obj->effect_info->damage_time=std::max(1.0,src_obj->effect_info->damage_time/3.0);
				dest_obj->effect_info->damage_per_second=src_obj->effect_info->damage_per_second;

				// We need this cap (as the gb burns forever
				if(dest_obj->effect_info->damage_time > 10.0f)
				{
					dest_obj->effect_info->damage_time = 10.0f;
				}

				dest_obj->effect_info->last_damage_time=0;

				dest_obj->effect_info->damage_handle=src_obj->handle;
		
				if (dest_obj->effect_info->sound_handle == SOUND_NONE_INDEX)
					dest_obj->effect_info->sound_handle = Sound_system.Play3dSound(SOUND_PLAYER_BURNING, SND_PRIORITY_HIGHEST, dest_obj);
			}
		}
	}
}

void collide_player_and_player( object * p1, object * p2, vector *collision_point, vector *collision_normal, bool f_reverse_normal, fvi_info *hit_info ) 
{
	if(f_reverse_normal)
		*collision_normal *= -1.0f;

	vector rvel = p1->mtype.phys_info.velocity - p2->mtype.phys_info.velocity;
	float speed = vm_NormalizeVector(&rvel);
	float scalar;

	if(speed <= MIN_WALL_HIT_SOUND_VEL)
	{
		scalar = 0.0f;
	}
	else if(speed >= MAX_WALL_HIT_SOUND_VEL)
	{
		scalar = 1.0f;
	}
	else 
	{
		scalar = (speed - MIN_WALL_HIT_SOUND_VEL)/(MAX_WALL_HIT_SOUND_VEL - MIN_WALL_HIT_SOUND_VEL);
	}

	if(scalar > .01)
	{
		pos_state cur_pos;
		cur_pos.position = collision_point;
		cur_pos.orient = &p1->orient;
		cur_pos.roomnum = p1->roomnum;

		Sound_system.Play3dSound(SOUND_PLAYER_HIT_WALL, SND_PRIORITY_HIGHEST, &cur_pos, MAX_GAME_VOLUME * scalar);		

	}

	bump_two_objects(p1, p2, collision_point, collision_normal, 1);
}

#include "polymodel.h"

void collide_generic_and_player( object * robotobj, object * playerobj, vector *collision_point, vector *collision_normal, bool f_reverse_normal, fvi_info *hit_info ) 
{ 
	if(f_reverse_normal)
		*collision_normal *= -1.0f;

	vector rvel;
	if(robotobj->movement_type == MT_PHYSICS || robotobj->movement_type == MT_WALKING)
	{
		rvel = robotobj->mtype.phys_info.velocity;
	}
	else
	{
		rvel = Zero_vector;
	}
	rvel -= playerobj->mtype.phys_info.velocity;
	float speed = vm_NormalizeVector(&rvel);
	float scalar;

	if(speed <= MIN_WALL_HIT_SOUND_VEL)
	{
		scalar = 0.0f;
	}
	else if(speed >= MAX_WALL_HIT_SOUND_VEL)
	{
		scalar = 1.0f;
	}
	else 
	{
		scalar = (speed - MIN_WALL_HIT_SOUND_VEL)/(MAX_WALL_HIT_SOUND_VEL - MIN_WALL_HIT_SOUND_VEL);
	}

	//Check for lava surface on an object
	if ((robotobj->type == OBJ_BUILDING) && hit_info) {
		poly_model *pm = GetPolymodelPointer(robotobj->rtype.pobj_info.model_num);
		int tmap = pm->textures[pm->submodel[hit_info->hit_subobject[0]].faces[hit_info->hit_face[0]].texnum];

		if (GameTextures[tmap].flags & TF_LAVA) {
			if(!((Game_mode & GM_MULTI) && (Netgame.local_role==LR_CLIENT)))
			{
				int id = FindWeaponName("NapalmBlob");
				if(id >= 0)
					SetNapalmDamageEffect(playerobj,NULL,id);
				else
					Int3();
			}
			scalar = 0;		//don't make collide sound
		}
	}

	if(scalar > .01 || (robotobj->mtype.phys_info.flags & PF_LOCK_MASK))
	{
		pos_state cur_pos;
		cur_pos.position = collision_point;
		cur_pos.orient = &playerobj->orient;
		cur_pos.roomnum = playerobj->roomnum;

		Sound_system.Play3dSound(SOUND_PLAYER_HIT_WALL, SND_PRIORITY_HIGHEST, &cur_pos, MAX_GAME_VOLUME * scalar);		

		ain_hear hear;
		hear.f_directly_player = true;
		hear.hostile_level = 0.10f;
		hear.curiosity_level = 0.5f;
		hear.max_dist = Sounds[SOUND_PLAYER_HIT_WALL].max_distance * scalar;
		AINotify(playerobj, AIN_HEAR_NOISE, (void *)&hear);

		if((scalar > .25f && (robotobj->movement_type == MT_WALKING || robotobj->movement_type == MT_PHYSICS)) || ((robotobj->mtype.phys_info.flags & PF_LOCK_MASK) && (robotobj->mtype.phys_info.flags & PF_POINT_COLLIDE_WALLS)))
		{
			if(!(IS_GUIDEBOT(robotobj)))
			{
				if(robotobj->shields <= 1.0f)
				{
					ApplyDamageToGeneric(robotobj, playerobj, GD_PHYSICS, 2.0f);
				}
				else
				{
					ApplyDamageToGeneric(robotobj, playerobj, GD_PHYSICS, 5.0f * Frametime * scalar);
				}
				if(scalar < 1.0f && (robotobj->mtype.phys_info.flags & PF_LOCK_MASK) && (robotobj->mtype.phys_info.flags & PF_POINT_COLLIDE_WALLS))
					scalar = 1.0f;

				ApplyDamageToPlayer(playerobj, playerobj, PD_WALL_HIT, 2.0f * Frametime * scalar);
			}
		}
	}

	//mprintf((0, "We hit a robot\n"));
	if(robotobj->control_type == CT_AI)
	{
		AINotify(robotobj, AIN_BUMPED_OBJ, (void *)playerobj);
	}

/*	if((GameTextures[Rooms[hitseg].faces[hitwall].tmap].flags & TF_VOLATILE) && !((Game_mode & GM_MULTI) && (Netgame.local_role==LR_CLIENT)))
	{
		int id = FindWeaponName("NapalmBlob");
		if(id >= 0)
		{
			int objnum = ObjCreate(OBJ_WEAPON, id, playerobj->roomnum, &playerobj->pos, NULL, playerobj->handle);
				
			if(objnum >= 0)
			{
				object *weapon = &Objects[objnum];
				float damage_to_apply = Weapons[weapon->id].damage;

				// Factor in multiplier
				damage_to_apply *= weapon->ctype.laser_info.multiplier;

				ApplyDamageToPlayer(playerobj, weapon, 0);

				SetObjectDeadFlag(objnum);
			}
		}
	}
*/
	bump_two_objects(robotobj, playerobj, collision_point, collision_normal, 1);
}

void MakeWeaponStick(object *weapon, object *parent, fvi_info *hit_info)
{
	weapon->mtype.obj_link_info.parent_handle = parent->handle;
	weapon->mtype.obj_link_info.fvec = hit_info->hit_subobj_fvec;
	weapon->mtype.obj_link_info.uvec = hit_info->hit_subobj_uvec;
	weapon->mtype.obj_link_info.pos = hit_info->hit_subobj_pos;
	weapon->mtype.obj_link_info.sobj_index = hit_info->hit_subobject[0]; 
	weapon->movement_type = MT_OBJ_LINKED;
}

void collide_generic_and_weapon( object * robotobj, object * weapon, vector *collision_point, vector *collision_normal, bool f_reverse_normal, fvi_info *hit_info ) 
{ 
	object *parent_obj;
	float damage_to_apply;
	ubyte electrical= (Weapons[weapon->id].flags & WF_ELECTRICAL)?1:0;
	bool f_stick = ((weapon->mtype.phys_info.flags & PF_STICK) != 0);
	bool f_energy = ((Weapons[weapon->id].flags & WF_MATTER_WEAPON) == 0);
	int damage_type;

	//Check for lava & volatile surfaces on an object
	if ((robotobj->type == OBJ_BUILDING) && hit_info) {
		poly_model *pm = GetPolymodelPointer(robotobj->rtype.pobj_info.model_num);
		int tmap = pm->textures[pm->submodel[hit_info->hit_subobject[0]].faces[hit_info->hit_face[0]].texnum];
		vector *normal = &hit_info->hit_wallnorm[0];

		DoWallEffects (weapon,tmap);

		check_for_special_surface(weapon,tmap,normal,1.0);

		//If object died, stop processing
		if (weapon->flags & OF_DEAD)
			return;
	}

	if(Weapons[weapon->id].flags & WF_NAPALM)
		damage_type = GD_FIRE;
	else if(Weapons[weapon->id].flags & WF_MATTER_WEAPON)
		damage_type = GD_MATTER;
	else if(Weapons[weapon->id].flags & WF_ELECTRICAL)
		damage_type = GD_ELECTRIC;
	else
		damage_type = GD_ENERGY;

	if(f_reverse_normal)
		*collision_normal *= -1.0f;

   if(robotobj->flags & OF_DYING)
		robotobj->ctype.dying_info.delay_time *= 0.975f;

	if (Weapons[weapon->id].sounds[WSI_IMPACT_WALL]!=SOUND_NONE_INDEX)
	{
		Sound_system.Play3dSound(Weapons[weapon->id].sounds[WSI_IMPACT_WALL], SND_PRIORITY_HIGH, weapon);

		ain_hear hear;
		hear.f_directly_player = false;
		if(robotobj->control_type == CT_AI)
		{
			hear.hostile_level = 1.0f;
			hear.curiosity_level = 0.1f;
		}
		else if(robotobj->type == OBJ_DOOR)
		{
			hear.hostile_level = 0.5f;
			hear.curiosity_level = 1.0f;
		}
		else
		{
			hear.hostile_level = .9f;
			hear.curiosity_level = 0.5f;
		}

		hear.max_dist = Sounds[Weapons[weapon->id].sounds[WSI_IMPACT_WALL]].max_distance;
		AINotify(weapon, AIN_HEAR_NOISE, (void *)&hear);
	}

	// Do weapon explosion stuff
	DoWeaponExploded (weapon,collision_normal,collision_point);

	// Check to see if we should spawn
	if (robotobj->type==OBJ_DOOR && !(Game_mode & GM_MULTI))
	{
		if ((Weapons[weapon->id].flags & WF_SPAWNS_IMPACT) && Weapons[weapon->id].spawn_count>0 && Weapons[weapon->id].spawn_handle>=0)
			CreateImpactSpawnFromWeapon (weapon,collision_normal);
	}

	if ((Weapons[weapon->id].flags & WF_SPAWNS_ROBOT) && (Weapons[weapon->id].flags & WF_COUNTERMEASURE) && Weapons[weapon->id].robot_spawn_handle>=0)
		CreateRobotSpawnFromWeapon (weapon);

	parent_obj = ObjGet(weapon->parent_handle);

	if((parent_obj) && (parent_obj->control_type == CT_AI))
	{
		AINotify(parent_obj, AIN_WHIT_OBJECT, (void *)robotobj);
	}

	if(robotobj->control_type == CT_AI)
	{
		AINotify(robotobj, AIN_HIT_BY_WEAPON, (void *)weapon);
	}

	if (electrical)
	{
		damage_to_apply=Weapons[weapon->id].generic_damage*Frametime;
	}
	else
		damage_to_apply=Weapons[weapon->id].generic_damage;

	// Factor in multiplier
	damage_to_apply*=weapon->ctype.laser_info.multiplier;

	if (ApplyDamageToGeneric(robotobj, weapon, damage_type, damage_to_apply)) 
	{
		if (Weapons[weapon->id].sounds[WSI_IMPACT_ROBOT]!=SOUND_NONE_INDEX)
		{
			Sound_system.Play3dSound(Weapons[weapon->id].sounds[WSI_IMPACT_ROBOT], SND_PRIORITY_HIGHEST, weapon);
		}
		if (!electrical)
		{
			light_info *li=&Weapons[weapon->id].lighting_info;
			ushort color=GR_RGB16(li->red_light2*255,li->green_light2*255,li->blue_light2*255);
			CreateRandomLineSparks (3+ps_rand()%6,&weapon->pos,weapon->roomnum,color);
		}
	}
#if 0	
	if(Demo_flags == DF_RECORDING)
	{
		DemoWriteCollideGenericWeapon( robotobj, weapon, collision_point, collision_normal, f_reverse_normal, hit_info );	
	}
	else if(Demo_flags == DF_PLAYBACK)
	{
		//During playback we don't need the code below (actually it won't work)
		return;
	}
#endif

	if (!electrical)
	{

		bump_two_objects(robotobj, weapon, collision_point, collision_normal, 0);

		if(!f_stick || (hit_info == NULL))
		{
			if((robotobj->lighting_render_type == LRT_LIGHTMAPS) || !(weapon->mtype.phys_info.flags & PF_PERSISTENT))
				SetObjectDeadFlag (weapon);
		}
		else
		{
			MakeWeaponStick(weapon, robotobj, hit_info);
		}
	}
	
	
}

void collide_player_and_weapon( object * playerobj, object * weapon, vector *collision_point, vector *collision_normal, bool f_reverse_normal, fvi_info *hit_info ) 
{ 
	object *parent_obj;
	float damage_to_apply;
	ubyte electrical=Weapons[weapon->id].flags & WF_ELECTRICAL?1:0;
	bool f_stick = ((weapon->mtype.phys_info.flags & PF_STICK) != 0);
	
	if(f_reverse_normal)
		*collision_normal *= -1.0f;

	parent_obj = ObjGet(weapon->parent_handle);

	if((parent_obj) && (parent_obj->control_type == CT_AI))
	{
		AINotify(parent_obj, AIN_WHIT_OBJECT, (void *)playerobj);
	}

	// Do weapon explosion stuff
	DoWeaponExploded (weapon,collision_normal,collision_point,playerobj);
	

	// Check to see if we should spawn
	/*if ((Weapons[weapon->id].flags & WF_SPAWNS_IMPACT) && Weapons[weapon->id].spawn_count>0 && Weapons[weapon->id].spawn_handle>=0)
		CreateImpactSpawnFromWeapon (weapon,collision_normal);*/

	if ((Weapons[weapon->id].flags & WF_SPAWNS_ROBOT) && (Weapons[weapon->id].flags & WF_COUNTERMEASURE) && Weapons[weapon->id].robot_spawn_handle>=0)
		CreateRobotSpawnFromWeapon (weapon);

	if (electrical)
		damage_to_apply=Weapons[weapon->id].player_damage*Frametime;
	else
		damage_to_apply=Weapons[weapon->id].player_damage;

	// Factor in multiplier
	damage_to_apply*=weapon->ctype.laser_info.multiplier;

	int damage_type = electrical ? PD_ENERGY_WEAPON : PD_MATTER_WEAPON;

	if (ApplyDamageToPlayer(playerobj, weapon, damage_type, damage_to_apply)) 
	{
	// we were damaged!
		if (Weapons[weapon->id].sounds[WSI_IMPACT_ROBOT]!=SOUND_NONE_INDEX)
			Sound_system.Play3dSound(Weapons[weapon->id].sounds[WSI_IMPACT_ROBOT], SND_PRIORITY_NORMAL, weapon);
	}
#if 0		
	if(Demo_flags == DF_RECORDING)
	{
		DemoWriteCollidePlayerWeapon( playerobj, weapon, collision_point, collision_normal, f_reverse_normal, hit_info );	
	}
	else if(Demo_flags == DF_PLAYBACK)
	{
		//During playback we don't need the code below (actually it won't work)
		return;
	}
#endif

	if (!electrical)
	{
		bump_two_objects(playerobj, weapon, collision_point, collision_normal, 0);

		if(!f_stick || (hit_info == NULL))
		{
			if(!(weapon->mtype.phys_info.flags & PF_PERSISTENT))
				SetObjectDeadFlag (weapon);
		}
		else
		{
			MakeWeaponStick(weapon, playerobj, hit_info);
		}
	}
}

#define COLLISION_OF(a,b) (((a)<<8) + (b))

#define DO_COLLISION(type1,type2,collision_function) case COLLISION_OF( (type1), (type2) ): (collision_function)( (A), (B), collision_point, collision_normal, false, hit_info); break; case COLLISION_OF( (type2), (type1) ): (collision_function)( (B), (A), collision_point, collision_normal, true, hit_info); break;

#define DO_SAME_COLLISION(type1,type2,collision_function) case COLLISION_OF( (type1), (type1) ): (collision_function)( (A), (B), collision_point, collision_normal, false, hit_info); break;

#define NO_COLLISION(type1,type2) case COLLISION_OF( (type1), (type2) ): case COLLISION_OF( (type2), (type1) ):	break;

void check_lg_inform(object *A, object *B)
{
	if(A->flags & (OF_INFORM_PLAYER_COLLIDE_TO_LG | OF_INFORM_PLAYER_WEAPON_COLLIDE_TO_LG))
	{
		if(B->type == OBJ_PLAYER)
		{
			bool f_pwc = (A->flags & OF_INFORM_PLAYER_WEAPON_COLLIDE_TO_LG) != 0;
			int type;

			if(f_pwc)
			{
				type = OF_INFORM_PLAYER_WEAPON_COLLIDE_TO_LG;
			}
			else
			{
				type = OF_INFORM_PLAYER_COLLIDE_TO_LG;
			}

			Level_goals.Inform(LIT_OBJECT, type, A->handle);
		}
	}

	if(A->flags & OF_INFORM_PLAYER_WEAPON_COLLIDE_TO_LG)
	{
		if(B->type == OBJ_WEAPON)
		{
			object *parent = ObjGetUltimateParent(B);

			if(parent && parent->type == OBJ_PLAYER)
			{
				Level_goals.Inform(LIT_OBJECT, OF_INFORM_PLAYER_WEAPON_COLLIDE_TO_LG, A->handle);
			}
		}
	}
}

void collide_two_objects( object * A, object * B, vector *collision_point, vector *collision_normal, fvi_info *hit_info)
{
	int collision_type;	
	int a_num=A-Objects;
	int b_num=B-Objects;
	ubyte a_good=0,b_good=0;
	ubyte a_hittime_good=1,b_hittime_good=1;

	//Only do omega particle collisions if specifically allowed
	extern bool Enable_omega_collions;
	if (((A->type == OBJ_WEAPON) && (A->id == OMEGA_INDEX)) || ((B->type == OBJ_WEAPON) && (B->id == OMEGA_INDEX)))
		if (! Enable_omega_collions)
			return;

	ASSERT(CollisionResult[A->type][B->type] != RESULT_NOTHING);

	collision_type = COLLISION_OF(A->type,B->type);

	check_lg_inform(A, B);
	check_lg_inform(B, A);

	//mprintf( (0, "Object %d of type %d collided with object %d of type %d\n", A-Objects,A->type, B-Objects, B->type ));

//	COLLISION SCRIPT HOOK 
	if (Game_mode & GM_MULTI)
	{
		ASSERT (!(A->flags & OF_DEAD));
		ASSERT (!(B->flags & OF_DEAD));

		// Make sure we have good scripts to call
		if (A->type==OBJ_PLAYER || A->type==OBJ_ROBOT || A->type==OBJ_POWERUP || A->type==OBJ_WEAPON || A->type==OBJ_BUILDING || A->type==OBJ_DOOR || A->type==OBJ_CLUTTER)
			a_good=1;
		if (B->type==OBJ_PLAYER || B->type==OBJ_ROBOT || B->type==OBJ_POWERUP || B->type==OBJ_WEAPON || B->type==OBJ_BUILDING || B->type==OBJ_DOOR || B->type==OBJ_CLUTTER)
			b_good=1;

		if (A->type==OBJ_PLAYER && (Players[A->id].flags & (PLAYER_FLAGS_DYING|PLAYER_FLAGS_DEAD)))
			a_good=0;
		if (B->type==OBJ_PLAYER && (Players[B->id].flags & (PLAYER_FLAGS_DYING|PLAYER_FLAGS_DEAD)))
			b_good=0;

		if (A->type==OBJ_POWERUP && A->effect_info->last_object_hit==b_num && (Gametime-A->effect_info->last_object_hit_time)<1)
		{
			a_good = 0;
			a_hittime_good=0;
		}

		if (B->type==OBJ_POWERUP && B->effect_info->last_object_hit==a_num && (Gametime-B->effect_info->last_object_hit_time)<1)
		{
			b_good = 0;
			b_hittime_good=0;
		}

		if (a_good && b_good)
		{
			DLLInfo.me_handle=A->handle;
			DLLInfo.it_handle=B->handle;
			DLLInfo.collide_info.point.x = collision_point->x;
			DLLInfo.collide_info.point.y = collision_point->y;
			DLLInfo.collide_info.point.z = collision_point->z;
			DLLInfo.collide_info.normal.x = collision_normal->x;
			DLLInfo.collide_info.normal.y = collision_normal->y;
			DLLInfo.collide_info.normal.z = collision_normal->z;
			CallGameDLL (EVT_GAMECOLLIDE,&DLLInfo);

			DLLInfo.me_handle=B->handle;
			DLLInfo.it_handle=A->handle;
			CallGameDLL (EVT_GAMECOLLIDE,&DLLInfo);

			// Update static variables
			if (A->type==OBJ_POWERUP)
			{
				A->effect_info->last_object_hit=b_num;
				A->effect_info->last_object_hit_time=Gametime;
			}
			if (B->type==OBJ_POWERUP)
			{
				B->effect_info->last_object_hit=a_num;
				B->effect_info->last_object_hit_time=Gametime;
			}
		}
	}
	
	// Call script only if its ok to
	int ok_to_call_script=1;
	if (A->type==OBJ_PLAYER && (Players[A->id].flags & (PLAYER_FLAGS_DYING|PLAYER_FLAGS_DEAD)))
		ok_to_call_script=0;
	if (B->type==OBJ_PLAYER && (Players[B->id].flags & (PLAYER_FLAGS_DYING|PLAYER_FLAGS_DEAD)))
		ok_to_call_script=0;

	// Check to see if we should call the script
	if ((Game_mode & GM_MULTI))
	{
		if ((A->type==OBJ_POWERUP || B->type==OBJ_POWERUP) && (a_hittime_good==0 || b_hittime_good==0))
			ok_to_call_script=0;
	}
			
	if (ok_to_call_script)
	{

		if (!(Game_mode & GM_MULTI))
		{
			ASSERT (!(A->flags & OF_DEAD));
			ASSERT (!(B->flags & OF_DEAD));
		}
		tOSIRISEventInfo ei;
		ei.evt_collide.it_handle = B->handle;
		Osiris_CallEvent(A,EVT_COLLIDE,&ei);

		ei.evt_collide.it_handle = A->handle;
		Osiris_CallEvent(B,EVT_COLLIDE,&ei);
	}

	switch( collision_type )	
	{
		DO_COLLISION( OBJ_PLAYER,		OBJ_WEAPON, collide_player_and_weapon )
		DO_COLLISION( OBJ_PLAYER,		OBJ_MARKER, collide_player_and_marker )

		DO_COLLISION( OBJ_ROBOT,		OBJ_PLAYER, collide_generic_and_player )
		DO_COLLISION( OBJ_BUILDING,	OBJ_PLAYER, collide_generic_and_player )
		DO_COLLISION( OBJ_DOOR,			OBJ_PLAYER, collide_generic_and_player )
		DO_COLLISION( OBJ_ROOM,			OBJ_VIEWER, collide_generic_and_player )
		DO_COLLISION( OBJ_ROOM,			OBJ_PLAYER, collide_generic_and_player )
		DO_COLLISION( OBJ_CLUTTER,		OBJ_PLAYER, collide_generic_and_player )

		DO_COLLISION( OBJ_ROBOT,		OBJ_WEAPON, collide_generic_and_weapon )
		DO_COLLISION( OBJ_CLUTTER,		OBJ_WEAPON, collide_generic_and_weapon )
		DO_COLLISION( OBJ_BUILDING,	OBJ_WEAPON, collide_generic_and_weapon )
		DO_COLLISION( OBJ_ROOM,			OBJ_WEAPON, collide_generic_and_weapon )
		DO_COLLISION( OBJ_DOOR,			OBJ_WEAPON, collide_generic_and_weapon )

		DO_SAME_COLLISION(OBJ_PLAYER, OBJ_PLAYER, collide_player_and_player  )

		//Handled by the script, so no code
		NO_COLLISION( OBJ_PLAYER, OBJ_POWERUP )
		NO_COLLISION (OBJ_POWERUP, OBJ_WEAPON)

		default:
			bump_two_objects(A, B, collision_point, collision_normal, 1);
	}

	if((A->type == OBJ_PLAYER && B->type == OBJ_POWERUP && (B->flags & OF_DEAD)) || (B->type == OBJ_PLAYER && A->type == OBJ_POWERUP && (A->flags & OF_DEAD)))
	{
		ain_hear hear;
		hear.f_directly_player = true;
		hear.hostile_level = 0.05f;
		hear.curiosity_level = 1.0f;

		hear.max_dist = Sounds[SOUND_POWERUP_PICKUP].max_distance;
		if(A->type == OBJ_PLAYER)
			AINotify(A, AIN_HEAR_NOISE, (void *)&hear);
		else
			AINotify(B, AIN_HEAR_NOISE, (void *)&hear);
	}

}

#define ENABLE_COLLISION_SPHERE_SPHERE(type1,type2)	CollisionResult[type1][type2] = RESULT_CHECK_SPHERE_SPHERE;	CollisionResult[type2][type1] = RESULT_CHECK_SPHERE_SPHERE;

#define ENABLE_COLLISION_SPHERE_POLY(type1,type2) CollisionResult[type1][type2] = RESULT_CHECK_SPHERE_POLY;	CollisionResult[type2][type1] = RESULT_CHECK_POLY_SPHERE;

#define ENABLE_COLLISION_POLY_SPHERE(type1,type2) CollisionResult[type1][type2] = RESULT_CHECK_POLY_SPHERE;	CollisionResult[type2][type1] = RESULT_CHECK_SPHERE_POLY;

#define ENABLE_COLLISION_BBOX_POLY(type1,type2) CollisionResult[type1][type2] = RESULT_CHECK_BBOX_POLY;	CollisionResult[type2][type1] = RESULT_CHECK_POLY_BBOX;

#define ENABLE_COLLISION_POLY_BBOX(type1,type2) CollisionResult[type1][type2] = RESULT_CHECK_POLY_BBOX;	CollisionResult[type2][type1] = RESULT_CHECK_BBOX_POLY;

#define ENABLE_COLLISION_BBOX_BBOX(type1,type2)	CollisionResult[type1][type2] = RESULT_CHECK_BBOX_BBOX;	CollisionResult[type2][type1] = RESULT_CHECK_BBOX_BBOX;

#define ENABLE_COLLISION_BBOX_SPHERE(type1,type2) CollisionResult[type1][type2] = RESULT_CHECK_BBOX_SPHERE;	CollisionResult[type2][type1] = RESULT_CHECK_SPHERE_BBOX;

#define ENABLE_COLLISION_SPHERE_BBOX(type1,type2) CollisionResult[type1][type2] = RESULT_CHECK_SPHERE_BBOX;	CollisionResult[type2][type1] = RESULT_CHECK_BBOX_SPHERE;

#define ENABLE_COLLISION_SPHERE_ROOM(type1,type2) CollisionResult[type1][type2] = RESULT_CHECK_SPHERE_ROOM;	CollisionResult[type2][type1] = RESULT_CHECK_SPHERE_ROOM;

#define ENABLE_COLLISION_BBOX_ROOM(type1,type2) CollisionResult[type1][type2] = RESULT_CHECK_BBOX_ROOM;	CollisionResult[type2][type1] = RESULT_CHECK_BBOX_ROOM;

#define DISABLE_COLLISION(type1,type2) CollisionResult[type1][type2] = RESULT_NOTHING;	CollisionResult[type2][type1] = RESULT_NOTHING;

void CollideInit()	{
	int i, j;

	for (i=0; i < MAX_OBJECT_TYPES; i++ )
		for (j=0; j < MAX_OBJECT_TYPES; j++ )
		{
			CollisionResult[i][j] = RESULT_NOTHING;
		}

	for (i=0; i < MAX_OBJECT_TYPES; i++ )
	{
		CollisionRayResult[i] = RESULT_NOTHING;
	}
	CollisionRayResult[OBJ_ROBOT] = RESULT_CHECK_SPHERE_POLY;
	CollisionRayResult[OBJ_PLAYER] = RESULT_CHECK_SPHERE_POLY;
	CollisionRayResult[OBJ_WEAPON] = RESULT_CHECK_SPHERE_POLY;
	CollisionRayResult[OBJ_POWERUP] = RESULT_CHECK_SPHERE_POLY;
	CollisionRayResult[OBJ_CLUTTER] = RESULT_CHECK_SPHERE_POLY;
	CollisionRayResult[OBJ_BUILDING] = RESULT_CHECK_SPHERE_POLY;
	CollisionRayResult[OBJ_DOOR] = RESULT_CHECK_SPHERE_POLY;
	CollisionRayResult[OBJ_ROOM] = RESULT_CHECK_SPHERE_POLY;
	
	for(i=0; i < MAX_OBJECT_TYPES; i++)
	{
		ENABLE_COLLISION_SPHERE_ROOM( i, OBJ_ROOM )
	}

	ENABLE_COLLISION_POLY_SPHERE( OBJ_WALL, OBJ_ROBOT )
	ENABLE_COLLISION_POLY_SPHERE( OBJ_WALL, OBJ_WEAPON )
	ENABLE_COLLISION_POLY_SPHERE( OBJ_WALL, OBJ_PLAYER  )
	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_ROBOT, OBJ_ROBOT )
//	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_BUILDING, OBJ_BUILDING )
	ENABLE_COLLISION_POLY_SPHERE( OBJ_PLAYER, OBJ_FIREBALL )
	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_PLAYER, OBJ_PLAYER )
	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_PLAYER, OBJ_MARKER )
	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_MARKER, OBJ_PLAYER )
	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_WEAPON, OBJ_WEAPON )
	ENABLE_COLLISION_POLY_SPHERE( OBJ_ROBOT, OBJ_PLAYER )
//	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_ROBOT, OBJ_PLAYER )
	ENABLE_COLLISION_POLY_SPHERE( OBJ_ROBOT, OBJ_WEAPON )

	ENABLE_COLLISION_POLY_SPHERE( OBJ_PLAYER, OBJ_WEAPON )
	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_PLAYER, OBJ_POWERUP )
	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_POWERUP, OBJ_WALL )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_WEAPON, OBJ_CLUTTER )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_PLAYER, OBJ_CLUTTER )
	ENABLE_COLLISION_SPHERE_SPHERE( OBJ_CLUTTER, OBJ_CLUTTER )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_ROBOT, OBJ_CLUTTER )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_PLAYER, OBJ_BUILDING )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_ROBOT, OBJ_BUILDING )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_WEAPON, OBJ_BUILDING )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_CLUTTER, OBJ_BUILDING )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_CLUTTER, OBJ_DOOR )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_BUILDING, OBJ_DOOR )

	ENABLE_COLLISION_SPHERE_ROOM( OBJ_PLAYER, OBJ_ROOM )
	ENABLE_COLLISION_SPHERE_ROOM( OBJ_ROBOT, OBJ_ROOM )
	ENABLE_COLLISION_SPHERE_ROOM( OBJ_WEAPON, OBJ_ROOM )
	ENABLE_COLLISION_SPHERE_ROOM( OBJ_VIEWER, OBJ_ROOM )

	ENABLE_COLLISION_SPHERE_POLY( OBJ_PLAYER, OBJ_DOOR )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_ROBOT, OBJ_DOOR )
	ENABLE_COLLISION_SPHERE_POLY( OBJ_WEAPON, OBJ_DOOR )
	DISABLE_COLLISION( OBJ_POWERUP, OBJ_POWERUP )
}

//Process a collision between an object and a wall
//Returns true if the object hits the wall, and false if should keep going though the wall (for breakable glass)
bool collide_object_with_wall( object * A, float hitspeed, int hitseg, int hitwall, vector * hitpt, vector *wall_normal, float hit_dot )
{
	ubyte do_event = 0;
	bool ret = true;

	switch( A->type )	{
	case OBJ_NONE:
		Error( "A object of type NONE hit a wall!\n");
		break;
	case OBJ_PLAYER:		collide_player_and_wall(A,hitspeed,hitseg,hitwall,hitpt, wall_normal, hit_dot); do_event = 1; break;
	case OBJ_WEAPON:		ret = collide_weapon_and_wall(A,hitspeed,hitseg,hitwall,hitpt, wall_normal, hit_dot); do_event = 1; break;
	case OBJ_DEBRIS:		break; // chrishack -- collide_debris_and_wall(A,hitspeed,hitseg,hitwall,hitpt); break;
	case OBJ_FIREBALL:	break;		//collide_fireball_and_wall(A,hitspeed,hitseg,hitwall,hitpt); 
	case OBJ_CLUTTER:		
	case OBJ_BUILDING:
	case OBJ_ROBOT:		collide_generic_and_wall(A,hitspeed,hitseg,hitwall,hitpt, wall_normal, hit_dot); do_event = 1; break;
	case OBJ_VIEWER:		break;		//collide_camera_and_wall(A,hitspeed,hitseg,hitwall,hitpt); 
	case OBJ_POWERUP:		break;		//collide_powerup_and_wall(A,hitspeed,hitseg,hitwall,hitpt); 
	case OBJ_GHOST:		break;	//do nothing
	case OBJ_OBSERVER:	break;	// do nothing
	case OBJ_SPLINTER:	break;
	case OBJ_MARKER:		break;
	case OBJ_SHARD:
		if(sound_override_glass_breaking == -1)
			Sound_system.Play3dSound(SOUND_BREAKING_GLASS,SND_PRIORITY_NORMAL,A,MAX_GAME_VOLUME/10);
		else
			Sound_system.Play3dSound(sound_override_glass_breaking,SND_PRIORITY_NORMAL,A,MAX_GAME_VOLUME/10);
		break;
		

	default:
		mprintf((0, "Unhandled collision of object type %d and wall\n", A->type));
//		Error( "Unhandled object type hit wall in collide.c\n" );
	}

	if(do_event && (Game_mode&GM_MULTI))
	{
		//call multiplayer event
		DLLInfo.me_handle=A->handle;
		DLLInfo.it_handle=OBJECT_HANDLE_NONE;
		DLLInfo.collide_info.point.x = hitpt->x;
		DLLInfo.collide_info.point.y = hitpt->y;
		DLLInfo.collide_info.point.z = hitpt->z;
		DLLInfo.collide_info.normal.x = wall_normal->x;
		DLLInfo.collide_info.normal.y = wall_normal->y;
		DLLInfo.collide_info.normal.z = wall_normal->z;
		DLLInfo.collide_info.hitspeed = hitspeed;
		DLLInfo.collide_info.hit_dot = hit_dot;
		DLLInfo.collide_info.hitseg = hitseg;
		DLLInfo.collide_info.hitwall = hitwall;		
		CallGameDLL (EVT_GAMEWALLCOLLIDE,&DLLInfo);
	}

	return ret;
}


