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

#ifdef NEWEDITOR //include first to get rid of ugly warning message about macro redfinitions
#include "..\neweditor\globals.h"
#endif

#include <stdlib.h>
#include <algorithm>
#include "object.h"
#include "viseffect.h"
#include "render.h"
#include "renderobject.h"
#include "room.h"
#include "postrender.h"
#include "config.h"
#include "terrain.h"
#include "renderer.h"

postrender_struct Postrender_list[MAX_POSTRENDERS];
int Num_postrenders = 0;

static vector Viewer_eye;
static matrix Viewer_orient;
static int Viewer_roomnum;


// Resets out postrender list for a new frame
void ResetPostrenderList()
{
	Num_postrenders = 0;
}

//Compare function for room face sort
static int Postrender_sort_func(const postrender_struct* a, const postrender_struct* b)
{
	if (a->z < b->z)
		return -1;
	else if (a->z > b->z)
		return 1;
	else
		return 0;
}

#define STATE_PUSH(val) {state_stack[state_stack_counter]=val; state_stack_counter++; ASSERT (state_stack_counter<2000);}
#define STATE_POP()	{state_stack_counter--; pop_val=state_stack[state_stack_counter];}



// Sorts our texture states using the quicksort algorithm
void SortPostrenders()
{
	if (Num_postrenders > 0)
		std::sort(&Postrender_list[0], &Postrender_list[Num_postrenders - 1]);
	/*postrender_struct v, t;
	int pop_val;
	int i, j;
	int l, r;
	l = 0;
	r = Num_postrenders - 1;

	ushort state_stack_counter = 0;
	ushort state_stack[MAX_POSTRENDERS];


	while (1)
	{
		while (r > l)
		{
			i = l - 1;
			j = r;
			v = Postrender_list[r];
			while (1)
			{
				while (Postrender_list[++i].z < v.z)
					;

				while (Postrender_list[--j].z > v.z)
					;

				if (i >= j)
					break;

				t = Postrender_list[i];
				Postrender_list[i] = Postrender_list[j];
				Postrender_list[j] = t;
			}

			t = Postrender_list[i];
			Postrender_list[i] = Postrender_list[r];
			Postrender_list[r] = t;

			if (i - l > r - i)
			{
				STATE_PUSH(l);
				STATE_PUSH(i - 1);
				l = i + 1;
			}
			else
			{
				STATE_PUSH(i + 1);
				STATE_PUSH(r);
				r = i - 1;
			}
		}

		if (!state_stack_counter)
			break;
		STATE_POP();
		r = pop_val;
		STATE_POP();
		l = pop_val;
	}*/
}


void SetupPostrenderRoom(room* rp)
{
	// Setup faces if this is a fogged room
	if (rp->flags & RF_FOG)
		SetupRoomFog(rp, &Viewer_eye, &Viewer_orient, Viewer_roomnum);
}


// Rotates a face, and then renders it
void DrawPostrenderFace(int roomnum, int facenum, bool change_z)
{
	int i;

	// Always draw as non state limited
	bool save_state = StateLimited;
	StateLimited = false;

	ASSERT(roomnum >= 0 && roomnum < (MAX_ROOMS + MAX_PALETTE_ROOMS));
	ASSERT(Rooms[roomnum].used);

	room* rp = &Rooms[roomnum];

	ASSERT(facenum >= 0 && facenum < rp->num_faces);

	face* fp = &rp->faces[facenum];

	// Rotate points
	rp->wpb_index = 0;
	for (i = 0; i < fp->num_verts; i++)
	{
		g3_RotatePoint(&World_point_buffer[fp->face_verts[i]], &rp->verts[fp->face_verts[i]]);
		g3_ProjectPoint(&World_point_buffer[fp->face_verts[i]]);
	}

	SetupPostrenderRoom(rp);

	// Render!
	if (change_z)
		rend_SetZBufferWriteMask(0);
	RenderFace(rp, facenum);

	// Render any effects for this face
	if (Num_specular_faces_to_render > 0)
	{
		RenderSpecularFacesFlat(rp);
		Num_specular_faces_to_render = 0;
	}

	if (Num_fog_faces_to_render > 0)
	{
		RenderFogFaces(rp);
		Num_fog_faces_to_render = 0;
	}

	// Restore statelimited setting
	StateLimited = save_state;

	if (change_z)
		rend_SetZBufferWriteMask(1);
}

// Renders all the objects/viseffects/walls we have in our postrender list
void PostRender(int roomnum)
{
	g3_GetViewPosition(&Viewer_eye);
	g3_GetUnscaledMatrix(&Viewer_orient);

	Viewer_roomnum = roomnum;

	int i, index;
	// Sort the objects
	SortPostrenders();
	//qsort(Postrender_list,Num_postrenders,sizeof(*Postrender_list),(int (cdecl *)(const void*,const void*))Postrender_sort_func);

	for (i = Num_postrenders - 1; i >= 0; i--)
	{

		if (Postrender_list[i].type == PRT_VISEFFECT)
		{
			index = Postrender_list[i].visnum;
			DrawVisEffect(&VisEffects[index]);
		}
		else if (Postrender_list[i].type == PRT_OBJECT)
		{
			object* objp = &Objects[Postrender_list[i].objnum];

			if (!OBJECT_OUTSIDE(&Objects[Postrender_list[i].objnum]))
				SetupPostrenderRoom(&Rooms[Objects[Postrender_list[i].objnum].roomnum]);
			RenderObject(objp);
		}
		else
		{
			// Do room face
			DrawPostrenderFace(Postrender_list[i].roomnum, Postrender_list[i].facenum);
		}
	}
	Num_postrenders = 0;
	rend_SetFogState(0);
	RenderLightGlows();
}
