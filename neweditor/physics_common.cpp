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
 #ifdef NED_PHYSICS

#ifdef NED_PHYSICS
extern float Gametime;
#else
#include "game.h"
#endif
#include "vecmat.h"
#include "object.h"
#include "objinfo.h"
#include "doorway.h"
#include "findintersection.h"

ubyte CollisionResult[MAX_OBJECT_TYPES][MAX_OBJECT_TYPES];
ubyte CollisionRayResult[MAX_OBJECT_TYPES];

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

void CollideInit()	
{
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

bool AreObjectsAttached(const object *obj1, const object *obj2)
{
	const bool f_o1_a = (obj1->flags & OF_ATTACHED) != 0;
	const bool f_o2_a = (obj2->flags & OF_ATTACHED) != 0;

	if(f_o1_a || f_o2_a)
	{
		const int o1_uh = obj1->attach_ultimate_handle;
		const int o2_uh = obj2->attach_ultimate_handle;

		if((f_o1_a) && ((o1_uh == obj2->handle) || (f_o2_a && (o1_uh == o2_uh))))
			return true;

		if((f_o2_a) && (o2_uh == obj1->handle))
			return true;
	}

	return false;
}

bool ObjectsAreRelated( int o1, int o2 )
{
	if ( (o1 < 0) || (o2 < 0) )	
		return false;

	const object *obj1 = &Objects[o1];
	const object *obj2 = &Objects[o2];

	ASSERT(obj1->handle != OBJECT_HANDLE_NONE);
	ASSERT(obj2->handle != OBJECT_HANDLE_NONE);

	if(obj1->movement_type == MT_OBJ_LINKED || obj2->movement_type == MT_OBJ_LINKED)
		return true;

	if(obj1->type != OBJ_SHOCKWAVE && (obj1->mtype.phys_info.flags & PF_NO_COLLIDE))
	{
		return true;
	}

	if(obj2->type != OBJ_SHOCKWAVE && (obj2->mtype.phys_info.flags & PF_NO_COLLIDE))
	{
		return true;
	}

	if (((obj1->type == OBJ_PLAYER) && ((obj2->type == OBJ_ROBOT) && (obj2->id == GENOBJ_CHAFFCHUNK))) ||
		 ((obj2->type == OBJ_PLAYER) && ((obj1->type == OBJ_ROBOT) && (obj1->id == GENOBJ_CHAFFCHUNK))))
		return true;

	if(((obj1->type == OBJ_BUILDING) && (obj1->movement_type != MT_NONE) && (obj2->type == OBJ_POWERUP)) ||
	   ((obj2->type == OBJ_BUILDING) && (obj2->movement_type != MT_NONE) && (obj1->type == OBJ_POWERUP)))
	{
		return true;
	}

#ifndef NED_PHYSICS
	if(obj1->type == OBJ_DOOR && DoorwayGetPosition(&Rooms[obj1->roomnum]) == 1.0f && obj2->type == OBJ_ROBOT)
		return true;

	if(obj2->type == OBJ_DOOR && DoorwayGetPosition(&Rooms[obj2->roomnum]) == 1.0f && obj1->type == OBJ_ROBOT)
		return true;
#endif

	if(AreObjectsAttached(obj1, obj2))
		return true;

	if ( obj1->type != OBJ_WEAPON && obj2->type != OBJ_WEAPON )	
	{
#ifndef NED_PHYSICS
		if(((Gametime < obj1->creation_time + 3.0f) && obj1->parent_handle == obj2->handle) || 
			((Gametime < obj2->creation_time + 3.0f) && obj2->parent_handle == obj1->handle))
			return true;
		else
			return false;
#endif
	}

	if(obj1->type == OBJ_WEAPON && obj1->movement_type == MT_PHYSICS && (obj1->mtype.phys_info.flags & PF_PERSISTENT) &&
		obj1->ctype.laser_info.last_hit_handle == obj2->handle)
		return true;


	if(obj2->type == OBJ_WEAPON && obj2->movement_type == MT_PHYSICS && (obj2->mtype.phys_info.flags & PF_PERSISTENT) && 
		obj2->ctype.laser_info.last_hit_handle == obj1->handle)
		return true;

	// See if o2 is the parent of o1
	if ( obj1->type == OBJ_WEAPON && (obj1->mtype.phys_info.flags & PF_NO_COLLIDE_PARENT))
	{
		if (obj1->parent_handle == obj2->handle)
			return true;

		object *t1 = ObjGet(obj1->parent_handle);

		if(t1)
		{
			if(AreObjectsAttached(obj2, t1))
				return true;
		}
	}

	// See if o1 is the parent of o2
	if ( obj2->type == OBJ_WEAPON && (obj2->mtype.phys_info.flags & PF_NO_COLLIDE_PARENT))
	{
		if (obj2->parent_handle == obj1->handle)
			return true;

		object *t2 = ObjGet(obj2->parent_handle);

		if(t2)
		{
			if(AreObjectsAttached(obj1, t2))
				return true;
		}
	}

	// They must both be weapons
	if ( obj1->type != OBJ_WEAPON || obj2->type != OBJ_WEAPON )	
	{
		return false;
	}

	//	Here is the 09/07/94 change -- Siblings must be identical, others can hurt each other
	// See if they're siblings...
	if ( obj1->parent_handle == obj2->parent_handle)
		if ((obj1->mtype.phys_info.flags & PF_HITS_SIBLINGS)  || (obj2->mtype.phys_info.flags & PF_HITS_SIBLINGS))
			return false;		//if either is proximity, then can blow up, so say not related
		else
			return true;

   // Otherwise, it is two weapons and by default, they should not collide
	return true;
}
#endif