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

#ifndef __VISEFFECT_EXTERNAL_H_
#define __VISEFFECT_EXTERNAL_H_


#include "pstypes.h"
#include "pserror.h"
#include "vecmat.h"

#define MAX_VIS_EFFECTS	5000

// types
#define VIS_NONE			0
#define VIS_FIREBALL		1

// Flags
#define VF_USES_LIFELEFT		1
#define VF_WINDSHIELD_EFFECT	2
#define VF_DEAD					4
#define VF_PLANAR					8
#define VF_REVERSE				16
#define VF_EXPAND					32
#define VF_ATTACHED				64
#define VF_NO_Z_ADJUST			128
#define VF_LINK_TO_VIEWER		256	// Always link into the room that the viewer is in

extern ushort max_vis_effects;

struct object;

struct vis_attach_info
{
	int obj_handle;
	int dest_objhandle;

	ushort modelnum;
	ushort vertnum;
	ushort end_vertnum;

	ubyte subnum,subnum2;	
};

struct axis_billboard_info
{
	ubyte width;
	ubyte height;
	ubyte texture;			
};

struct vis_effect
{
	vector pos;
	vector end_pos;	
	vector velocity;
	float mass;
	float drag;
	float size;
	float lifeleft;
	float lifetime;
	float creation_time;

	int roomnum;
	
	int phys_flags;
	
	short custom_handle;
	ushort lighting_color;

	ushort flags;

	short next;
	short prev;

	vis_attach_info attach_info;
	axis_billboard_info billboard_info;

	ubyte movement_type;
	ubyte type,id;
};

#endif
