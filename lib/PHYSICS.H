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

#ifndef _PHYSICS_H
#define _PHYSICS_H

#include "vecmat.h"
#include "findintersection.h"
#include "object.h"
#include "viseffect.h"

extern int Physics_normal_counter;
extern int Physics_normal_looping_counter;
extern int Physics_walking_counter;
extern int Physics_walking_looping_counter;
extern int Physics_vis_counter;

// The current strength of the world's gravity
extern float Gravity_strength;

#define PHYSICS_UNLIMITED_BOUNCE -1

#ifdef _DEBUG
	extern int Physics_player_verbose;
#endif

extern ubyte Default_player_terrain_leveling;
extern ubyte Default_player_room_leveling;

bool PhysicsDoSimRot(object *obj, float frame_time, matrix *orient, vector *rotforce, vector *rotvel, angle *turn_roll);
void PhysicsDoSimLinear(const object& obj, const vector& pos, const vector& force, vector& velocity, vector& movementVec, vector& movementPos, float simTime, int count);

extern int Physics_NumLinked;
extern int PhysicsLinkList[MAX_OBJECTS];

//Simulate a physics object for this frame
void do_physics_sim(object *obj);

// Quick sim for vis stuff
void do_vis_physics_sim(vis_effect *vis);

//Simulate a physics object for this frame
void do_walking_sim(object *obj);

//Applies an instantaneous force on an object, resulting in an instantaneous
//change in velocity.
void phys_apply_force(object *obj,vector *force_vec,short weapon_index=-1);
void phys_apply_rot(object *obj,vector *force_vec);

//this routine will set the thrust for an object to a value that will
//(hopefully) maintain the object's current velocity
void set_thrust_from_velocity(object *obj);

// Determines the point and normal of the ground point
bool PhysCalcGround(vector *ground_point, vector *ground_normal, object *obj, int ground_num);

// Linked objects arbitrary linking for things like weapons (unlike attached objects)
void DoPhysLinkedFrame(object *obj); 

#endif
