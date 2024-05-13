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

#ifdef _DEBUG

#ifdef EDITOR
#include "editor\mainfrm.h"
#include "editor\d3edit.h"
#endif

#include <stdlib.h>
#include "descent.h"
#include "slew.h"
#include "vecmat.h"
#include "ddio.h"
#include "object.h"
#include "mono.h"
#include "game.h"
#include "joystick.h"
#include "findintersection.h"
#include "room.h"

//variables for slew system

#define ROT_SPEED		(1.0 / 8.0)		//rate of rotation while key held down
#define VEL_SPEED		(110.0)			//rate of acceleration while key held down
#define JOY_NULL		32

float Slew_key_speed = 1.0f;
int Joystick_active = -1;

// -------------------------------------------------------------------

#ifdef EDITOR
void SlewControlInit()
{
	Joystick_active = -1;

#ifdef EDITOR
	if (!D3EditState.joy_slewing)
		return;
#endif

	if (joy_IsValid(JOYSTICK_1)) {
		tJoyPos joystate;
		tJoystick joyid = JOYSTICK_1;

		Joystick_active = (int)joyid;
		joy_GetPos((tJoystick)Joystick_active, &joystate);		//get all the stick values

		if ((abs(joystate.x) > 32) || (abs(joystate.y) > 32))
			EditorMessageBox("Warning: Your joystick is not centered.  You should either center it now or recalibrate.");
	}
}
#endif

int SlewStop(object* obj)
{
	if (!obj)
		return 0;

	vm_MakeZero(&obj->mtype.phys_info.velocity);

	return 1;
}

// Resets object's orientation
void SlewResetOrient(object* obj)
{
	if (!obj)
		return;

	ObjSetOrient(obj, &Identity_matrix);
}

// Moves the object for one frame
int SlewFrame(object* obj, int movement_limitations)
{
	static short old_joy_x = 0, old_joy_y = 0;	//position last time around
	int ret_flags = 0;
	vector svel, movement;				//scaled velocity (per this frame)
	matrix rotmat, new_pm;
	angvec rotang;
	vector rottime;
	vector new_pos;
	int new_room;
	fvi_query fq;
	fvi_info	hit_info;
	int fate;

	float key_timex1 = 0, key_timex0 = 0;
	float key_timey1 = 0, key_timey0 = 0;
	float key_timez1 = 0, key_timez0 = 0;
	float key_timep1 = 0, key_timep0 = 0;
	float key_timeh1 = 0, key_timeh0 = 0;
	float key_timeb1 = 0, key_timeb0 = 0;

	if (!obj)
		return 0;

	//	check keyboard for slewing.
	key_timex1 = ddio_KeyDownTime(KEY_PAD9);
	key_timex0 = ddio_KeyDownTime(KEY_PAD7);
	key_timey1 = ddio_KeyDownTime(KEY_PADMINUS);
	key_timey0 = ddio_KeyDownTime(KEY_PADPLUS);
	key_timez1 = ddio_KeyDownTime(KEY_PAD8);
	key_timez0 = ddio_KeyDownTime(KEY_PAD2);
	key_timep1 = ddio_KeyDownTime(KEY_LBRACKET);
	key_timep0 = ddio_KeyDownTime(KEY_RBRACKET);
	key_timeh1 = ddio_KeyDownTime(KEY_PAD6);
	key_timeh0 = ddio_KeyDownTime(KEY_PAD4);
	key_timeb1 = ddio_KeyDownTime(KEY_PAD1);
	key_timeb0 = ddio_KeyDownTime(KEY_PAD3);

	if (!key_timep1)
		key_timep1 = ddio_KeyDownTime(KEY_PAGEDOWN);
	if (!key_timep0)
		key_timep0 = ddio_KeyDownTime(KEY_PAGEUP);

	if (key_timex1 || key_timex0 || key_timey1 || key_timey0 || key_timez1 || key_timez0)
		ret_flags |= SLEW_KEY;
	if (key_timep1 || key_timep0 || key_timeh1 || key_timeh0 || key_timeb1 || key_timeb0)
		ret_flags |= SLEW_KEY;

	if (key_timez0 || key_timez1) {
		mprintf_at((1, 0, 0, "Timez0: %.2f  ", key_timez0));
		mprintf_at((1, 1, 0, "Timez1: %.2f  ", key_timez1));
	}

	//	adjust physics info of object accordingly to keyboard input.
	obj->mtype.phys_info.velocity.x += VEL_SPEED * (key_timex1 - key_timex0) * Slew_key_speed;
	obj->mtype.phys_info.velocity.y += VEL_SPEED * (key_timey1 - key_timey0) * Slew_key_speed;
	obj->mtype.phys_info.velocity.z += VEL_SPEED * (key_timez1 - key_timez0) * Slew_key_speed;

	//mprintf((0,"<%x %x %x> ",obj->mtype.phys_info.velocity.x,obj->mtype.phys_info.velocity.y,obj->mtype.phys_info.velocity.z));

	rottime.x = key_timep1 - key_timep0;
	rottime.y = key_timeh1 - key_timeh0;
	rottime.z = key_timeb1 - key_timeb0;
	rotang.p = (short)(65536.0 * rottime.x * ROT_SPEED * Slew_key_speed);
	rotang.h = (short)(65536.0 * rottime.y * ROT_SPEED * Slew_key_speed);
	rotang.b = (short)(65536.0 * rottime.z * ROT_SPEED * Slew_key_speed);

	// joystick movement
#ifdef EDITOR
	if (Joystick_active != -1)
	{
		int joy_x, joy_y, btns;
		tJoyPos joystate;
		bool joyx_moved = false;
		bool joyy_moved = false;

		joy_GetPos((tJoystick)Joystick_active, &joystate);		//get all the stick values

		joy_x = joystate.x;
		joy_y = joystate.y;
		btns = joystate.buttons;
		mprintf_at((2, 1, 0, "JoyX: %d   ", joy_x));
		mprintf_at((2, 2, 0, "JoyY: %d   ", joy_y));

		if (abs(joy_x) < JOY_NULL) joy_x = 0;
		if (abs(joy_y) < JOY_NULL) joy_y = 0;

		joyx_moved = (abs(joy_x - old_joy_x) > JOY_NULL);
		joyy_moved = (abs(joy_y - old_joy_y) > JOY_NULL);

		//@@		if (joyx_moved)
		//@@			mprintf((1,"SLEW: Joy X moved\n"));
		//@@		if (joyy_moved)
		//@@			mprintf((1,"SLEW: Joy Y moved\n"));

		if (btns)
		{
			if (!rotang.p)
				rotang.p = -joy_y * 256 * Frametime;
		}
		else
		{
			if (joyy_moved)
				obj->mtype.phys_info.velocity.z = (float)-joy_y / 4.0;
		}

		if (!rotang.h)
			rotang.h = joy_x * 256 * Frametime;

		if (joyx_moved)
			old_joy_x = joy_x;
		if (joyy_moved)
			old_joy_y = joy_y;
	}
#endif

	vm_AnglesToMatrix(&rotmat, rotang.p, rotang.h, rotang.b);

	new_pm = obj->orient * rotmat;
	vm_Orthogonalize(&new_pm);

	ObjSetOrient(obj, &new_pm);

	vm_TransposeMatrix(&new_pm);		//make those columns rows

	svel = obj->mtype.phys_info.velocity * Frametime;
	movement = svel * new_pm;

	if (movement_limitations & 1)
		movement.x = 0;
	if (movement_limitations & 2)
		movement.y = 0;
	if (movement_limitations & 4)
		movement.z = 0;

	new_pos = obj->pos + movement;

	//Did the object position change?
	if ((movement.x != 0.0f) || (movement.y != 0.0f) || (movement.z != 0.0f))
		ret_flags |= SLEW_MOVE;

	if (ret_flags & SLEW_MOVE)
	{
		//Get the new room
		bool outside_mine = ((obj->flags & OF_OUTSIDE_MINE) != 0);

		mprintf((1, "SLEW: Moved\n"));

#ifdef EDITOR
		if (Editor_view_mode == VM_ROOM)
		{
			//Room number is bogus in room view, so don't update it
			new_room = obj->roomnum;
		}
		else
#endif
			//NOTE LINK TO ABOVE IF
			if (outside_mine) //starting outside the mine?
			{
				//See if we've moved back into a room
				new_room = FindPointRoom(&new_pos);

				if (new_room != -1) //back in the mine
				{
					outside_mine = 0;
					mprintf((0, "SLEW: Re-entered mine at room %d\n", new_room));
				}
				else							//not back in the mine
					new_room = obj->roomnum;
			}
			else
			{
				bool was_outside = (ROOMNUM_OUTSIDE(obj->roomnum) != 0);

				//Limit new position to terrain bounds if outside
				if (was_outside) {
					if (new_pos.x < 1.0)
						new_pos.x = 1.0;
					if (new_pos.x > TERRAIN_WIDTH * TERRAIN_SIZE - 1.0)
						new_pos.x = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;
					if (new_pos.z < 1.0)
						new_pos.z = 1.0;
					if (new_pos.z > TERRAIN_DEPTH * TERRAIN_SIZE - 1.0)
						new_pos.z = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;
				}

				//Call FVI up get updated room number
				fq.p0 = &obj->pos;
				fq.startroom = obj->roomnum;
				fq.p1 = &new_pos;
				fq.rad = 0;
				fq.thisobjnum = OBJNUM(obj);
				fq.ignore_obj_list = NULL;
				fq.flags = FQ_IGNORE_RENDER_THROUGH_PORTALS;
				fate = fvi_FindIntersection(&fq, &hit_info);

				//If bad room, don't move
				if ((fate == HIT_OUT_OF_TERRAIN_BOUNDS) || (hit_info.hit_room == -1))
				{
					new_room = obj->roomnum;
					new_pos = obj->pos;
				}
				else
					new_room = hit_info.hit_room;

				//The object hit a wall, and maybe went outside the mine.
				if (fate == HIT_WALL)
				{
					int t;

					mprintf((0, "SLEW: hit wall\n"));

					//Check if we're in a room
					t = FindPointRoom(&new_pos);

					if (t != -1)
					{
						//We're in a room
						new_room = t;
						mprintf((0, "SLEW: still in mine in room %d\n", new_room));
					}
					else
					{
						//Not in a room.  Set a special flag
						outside_mine = 1;
						mprintf((0, "SLEW: left mine from room %d\n", new_room));
					}
				}

				if (new_room != obj->roomnum)
				{
					//if we've changed rooms, say so
					if (ROOMNUM_OUTSIDE(new_room))
						if (was_outside)
							mprintf((0, "SLEW: Moved to cell %d, BOA TR %d\n", CELLNUM(new_room), TERRAIN_REGION(new_room)));
						else
							mprintf((0, "SLEW: Moved outside to cell %d\n", CELLNUM(new_room)));
					else
						if (was_outside)
							mprintf((0, "SLEW: Moved inside to room %d\n", new_room));
						else
							mprintf((0, "SLEW: Moved into room %d\n", new_room));
				}
			}

		//Now we have the new room, so update the object position
		ObjSetPos(obj, &new_pos, new_room, NULL, false);

		//Set outside-mine flag if we're outside
		if (outside_mine)
			obj->flags |= OF_OUTSIDE_MINE;
	}

	//Set flag if rotation changed
	if ((rotang.p != 0) || (rotang.h != 0) || (rotang.b != 0))
		ret_flags |= SLEW_ROTATE;

	return ret_flags;
}

#endif
