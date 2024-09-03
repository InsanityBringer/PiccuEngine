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

#include "pserror.h"
#include "renderer.h"
#include "3d.h"
#include "scorch.h"
#include "room.h"
#include "config.h"
#include "object_external_struct.h" //for ROOMNUM_OUTSIDE macro
#include "psrand.h"


//Structure for storing scorch marks
struct scorch
{
	int			roomface;		//room number & face number combined into an int
	vector		pos;			//the position of the center of the scorch mark
	ubyte		handle_index;	//which mark?
	sbyte		rx,ry,rz;		//right vector
	sbyte		ux,uy,uz;		//up vector
	ubyte		size;			//floating-point size times 16
};

//How many scorch marks
#define MAX_SCORCHES		500

//Buffer of scorch marks
scorch Scorches[MAX_SCORCHES];

//Indices into scorch buffer.  Start is oldest item, end is newest item
int Scorch_start,Scorch_end;

//Bitmap handles for scorches
#define MAX_SCORCH_TEXTURES	10
int Num_scorch_textures = 0;
int Scorch_texture_handles[MAX_SCORCH_TEXTURES];

//Pack & unpack roomnum & facenum into an int
#define ROOMFACE(r,f)	((r << 16) + f)
#define RF_ROOM(rf)		(rf >> 16)
#define RF_FACE(rf)		(rf & 0xffff)

//Value used to limit the number of scorches drawn per face
#define MAX_VIS_COUNT		30

//Called when a new level is started to reset the scorch list
void ResetScorches()
{
	Scorch_start = Scorch_end = 0;
}

//Delete the specified scorch mark
void DeleteScorch(int index)
{
	int roomface = Scorches[index].roomface;
	scorch *sp;
	int i;

	//mprintf((0,"Deleting scorch %d\n",index));

	//Look through all the scorches to see if there are other scorches on the same face
	for (i=Scorch_start,sp=&Scorches[Scorch_start];;)
	{
		if ((sp->roomface == roomface) && (i != index))		//Found another scorch
			return;

		//Check for end of loop
		if (i == Scorch_end)
			break;

		//Increment & wrap
		if (++i == MAX_SCORCHES)
		{
			i = 0;
			sp = Scorches;
		}
		else
			sp++;
	}

	//If we're here, there are no other scorches on the face, so clear the flag
	Rooms[RF_ROOM(roomface)].faces[RF_FACE(roomface)].flags &= ~FF_SCORCHED;
	//mprintf((0,"Clearing scorch flag from %d:%d\n",RF_ROOM(roomface),RF_FACE(roomface)));
}

//Add a scorch mark
//Parameters:	roomnum,facenum - the face that the scorch is on
//					pos - where the scorch goes
//					texture_handle - the texture for the scorch mark
//					size - how big the scorch is
void AddScorch(int roomnum,int facenum,vector *pos,int texture_handle,float size)
{
	//Bail if on terrain.  We should probably add support for this later.
	if (ROOMNUM_OUTSIDE(roomnum))
		return;

	room *rp = &Rooms[roomnum];
	face *fp = &rp->faces[facenum];

	//Check if face is a light, and if so don't add the scorch
	if (GameTextures[fp->tmap].flags & (TF_LIGHT|TF_PROCEDURAL))
		return;

	int roomface = ROOMFACE(roomnum,facenum);

	//Count the number of scorches on this face, and bail if already at max
	int count = 0;
	int i;
	scorch *sp;
	for (i=Scorch_start,sp=&Scorches[Scorch_start];;)
	{
		float size = (float) sp->size / 16.0;

		//Increment count, and stop drawing if hit limit
		if (sp->roomface == roomface)
		{
			//Found one!
			count += __max((int) size,1);	//count large marks more
			if (count >= MAX_VIS_COUNT)
				return;
		}

		//Check for end of loop
		if (i == Scorch_end)
			break;

		//Increment & wrap
		if (++i == MAX_SCORCHES)
		{
			i = 0;
			sp = Scorches;
		}
		else
			sp++;
	}

	//Check for scorch going off the edge of the face, and bail if does
	vector *v0,*v1=&rp->verts[fp->face_verts[fp->num_verts-1]];
	for (int v=0;v<fp->num_verts;v++)
	{
		vector edgevec,checkvec,checkp;
		float dot,dist;

		v0 = v1;
		v1 = &rp->verts[fp->face_verts[v]];
		vm_GetNormalizedDir(&edgevec,v1,v0);

		checkvec = *pos - *v0;
		dot = checkvec * edgevec;
		checkp = *v0 + edgevec * dot;
		dist = vm_VectorDistance(&checkp,pos);
		if (dist < size)
			return;
	}

	//Get next item in circular list, freeing oldest item if necessary
	int new_end = Scorch_end+1;
	if (new_end == MAX_SCORCHES)
		new_end = 0;
	if (new_end == Scorch_start)
	{
		DeleteScorch(Scorch_start);
		Scorch_start++;
		if (Scorch_start == MAX_SCORCHES)
			Scorch_start = 0;
	}
	Scorch_end = new_end;
	if (Scorch_start == -1)
		Scorch_start = 0;

	//Get a pointer to our struct
	sp = &Scorches[Scorch_end];

	//Fill in the data
	sp->roomface = roomface;
	sp->pos = *pos;
	ASSERT((size >= 0.0) && (size < 15.0));
	sp->size = size * 16.0;
	
	int handle_index;

	//Get index for handle
	for (handle_index=0;handle_index<Num_scorch_textures;handle_index++)
		if (Scorch_texture_handles[handle_index] == texture_handle)
			break;
	if (handle_index == Num_scorch_textures)
	{
		//didn't find one, so add
		ASSERT(Num_scorch_textures < MAX_SCORCH_TEXTURES);
		Scorch_texture_handles[handle_index] = texture_handle;
		Num_scorch_textures++;
	}
	sp->handle_index = handle_index;

	//Calculate the vectors
	matrix m;
	vm_VectorAngleToMatrix(&m,&fp->normal,ps_rand() * 2);

	//Store the vectors as signed 8-bit values
	sp->rx = -m.rvec.x * 127;
	sp->ry = -m.rvec.y * 127;
	sp->rz = -m.rvec.z * 127;

	sp->ux = m.uvec.x * 127;
	sp->uy = m.uvec.y * 127;
	sp->uz = m.uvec.z * 127;

	//Flag this face as being scorched
	fp->flags |= FF_SCORCHED;
}

//The UVs for the blob bitmap
struct
{
	float u,v;
} scorch_uvs[4] = { {0.0,0.0}, {1.0,0.0}, {1.0,1.0}, {0.0,1.0} };

//Values used to get rid of far-away scorches
#define MAX_VIS_DISTANCE		200		//if mark farther than this, don't draw
#define FADE_START_DISTANCE	170		//when mark is this far away, start fading

//Draw the scorch(es) for a given face
void DrawScorches(int roomnum,int facenum)
{
	int i;
	scorch *sp;

	if (!Detail_settings.Scorches_enabled)
		return;

	int roomface = ROOMFACE(roomnum,facenum);

	//Set alpha, transparency, & lighting for this face
	rend_SetAlphaType(AT_LIGHTMAP_BLEND);
	rend_SetAlphaValue(255);
	rend_SetLighting(LS_NONE);
	rend_SetColorModel(CM_MONO);
	rend_SetOverlayType(OT_NONE);
	rend_SetZBias (-.5);
	rend_SetZBufferWriteMask (0);
			
	//Select texture type
	rend_SetTextureType(TT_LINEAR);

	ASSERT(Scorch_end != -1);

	//Loop through all the scorches, and draw the ones for this face
	for (i=Scorch_start,sp=&Scorches[Scorch_start];;)
	{
		if (sp->roomface == roomface)
		{
			//Found one!
			vector right,up;
			vector corners[4];
			g3Point points[4];
			g3Point *pointlist[4];
			float size = (float) sp->size / 16.0;

			//Don't draw mark if too far away, and make smaller those marks that are almost too far
			float depth = g3_CalcPointDepth(&sp->pos);
			if (depth > MAX_VIS_DISTANCE)
				goto skip_draw;
			if (depth > FADE_START_DISTANCE)
				size *= 1.0 - ((depth - FADE_START_DISTANCE) / (MAX_VIS_DISTANCE - FADE_START_DISTANCE));

			//Calculate vectors to corners
			right.x = (float) sp->rx * (size / 127.0);
			right.y = (float) sp->ry * (size / 127.0);
			right.z = (float) sp->rz * (size / 127.0);

			up.x = (float) sp->ux * (size / 127.0);
			up.y = (float) sp->uy * (size / 127.0);
			up.z = (float) sp->uz * (size / 127.0);

			//Compute four corners
			corners[0] = sp->pos - right + up;
			corners[1] = sp->pos + right + up;
			corners[2] = sp->pos + right - up;
			corners[3] = sp->pos - right - up;

			//Rotate the points.  Set uv values
			for (int p=0;p<4;p++) {
				g3_RotatePoint(&points[p],&corners[p]);
				pointlist[p] = &points[p];
				points[p].p3_uvl.u = scorch_uvs[p].u;
				points[p].p3_uvl.v = scorch_uvs[p].v;
				points[p].p3_l=1.0;
				points[p].p3_flags |= PF_UV|PF_L;
			}

			//Get the bitmap handle
		  	int bm_handle = GetTextureBitmap(Scorch_texture_handles[sp->handle_index],0);

		  	//Draw the polygon
			g3_DrawPoly(4,pointlist,bm_handle);
		}
skip_draw:;

		//Check for end of loop
		if (i == Scorch_end)
			break;

		//Increment & wrap
		if (++i == MAX_SCORCHES)
		{
			i = 0;
			sp = Scorches;
		}
		else
			sp++;
	}

	//Reset rendering states
	rend_SetZBias (0);
	rend_SetZBufferWriteMask (1);
}
