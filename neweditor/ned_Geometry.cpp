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

#include "../editor/ERooms.h"
#include "room_external.h"
#include "vecmat.h"
#include "globals.h"
#include "ned_Geometry.h"
#include "../editor/HRoom.h"
#include "../editor/HTexture.h"

void GetFaceVertFromVert(int *face, int *vert, room *rp, int srcvert)
{
	int i,j;

	for (i=0; i<rp->num_faces; i++)
		for (j=0; j<rp->faces[i].num_verts; j++)
			if (rp->faces[i].face_verts[j] == srcvert)
			{
				*face = i;
				*vert = j;
				return;
			}
}


int InsertVertex(room *rp, vector pos)
{
	int vertnum = -1;

	if ( !(rp->num_verts > MAX_VERTS_PER_ROOM) )
	{
		vertnum = RoomAddVertices(rp,1);
		rp->verts[vertnum] = pos;
	}

	return vertnum;
}


int InsertVertex(room *rp, float x, float y, float z)
{
	vector vec = {x,y,z};

	return InsertVertex(rp,vec);
}


int VertInFace(room *rp, int facenum, int vertnum);


bool RotateVerts(room *rp, short *list, int num_verts, angle ang_x, angle ang_y, angle ang_z, vector offset)
{
	matrix mat;
	vector v1,v2;
	int i;
	int num_faces;
	bool bConcave = false;
//	int *face_list = (int *) mem_malloc(rp->num_faces * sizeof(int));
//	ASSERT (face_list);
	short face_list[MAX_FACES_PER_ROOM]; // TODO : tried using mem_malloc and I get unhandled exceptions on HeapAlloc
	memset (face_list,0,MAX_FACES_PER_ROOM*sizeof(short)); // rp->num_faces

	// Record which faces contain these verts
	num_faces = GetFacesFromVerts(rp,face_list,list,num_verts);

	// Rotate the verts
	vm_AnglesToMatrix(&mat,ang_x,ang_y,ang_z);
	for (i=0; i<num_verts; i++)
	{
		v1 = rp->verts[list[i]] - offset;
		vm_MatrixMulVector(&v2,&v1,&mat);
		rp->verts[list[i]] = v2 + offset;
	}

/*
	// Test whether a concave face results
	for (i=0; i<num_faces; i++)
	{
		fp = &rp->faces[face_list[i]];
		int vert = CheckFaceConcavity(fp->num_verts,fp->face_verts,&fp->normal,rp->verts);
		if (vert != -1)
		{
//			mprintf(0,"Room %3d face %3d is concave at vertex %d",ROOMNUM(rp),i,vert);
			bConcave = true;
		}
	}

	if (bConcave)
	{
		int answer = OutrageMessageBox(MB_YESNOCANCEL,"One or more faces would become concave as a result of this move.\nDo you want to split faces to resolve concavities?");
		if (answer == IDYES)
			FixConcaveFaces(rp,face_list,num_faces);
		else
		{
			// Rotate the verts back to their original positions
			vm_AnglesToMatrix(&mat,-ang_x,-ang_y,-ang_z);
			for (i=0; i<num_verts; i++)
			{
				v1 = rp->verts[list[i]] - offset;
				vm_MatrixMulVector(&v2,&v1,&mat);
				rp->verts[list[i]] = v2 + offset;
			}
//			mem_free (face_list);
			return false;
		}
	}
*/

	// Recompute the normals for the affected faces
	int bad_normals = 0;
	for (i=0; i<num_faces; i++)
	{
		if (! ComputeFaceNormal(rp,face_list[i]))
			bad_normals++;
	}

	if (bad_normals > 0) {
		mprintf((1,"Warning: Room %d has %d bad or low-precision normals\n",ROOMNUM(rp),bad_normals));
		Int3();	// Hmm, not good!
	}

//	mem_free (face_list);

	return true;
}


bool MoveVerts(room *rp, short *list, int num_verts, vector vec)
{
	vector *pvert;
	int i;
	int num_faces;
	bool bConcave = false;
//	int *face_list = (int *) mem_malloc(rp->num_faces * sizeof(int));
//	ASSERT (face_list);
	short face_list[MAX_FACES_PER_ROOM]; // TODO : tried using mem_malloc and I get unhandled exceptions on HeapAlloc
	memset (face_list,0,MAX_FACES_PER_ROOM*sizeof(short)); // rp->num_faces

	// Record which faces contain these verts
	num_faces = GetFacesFromVerts(rp,face_list,list,num_verts);

	// Move the verts
	for (i=0; i<num_verts; i++)
	{
		pvert = &rp->verts[list[i]];
		*pvert += vec;
	}

/*
	// Test whether a concave face results
	for (i=0; i<num_faces; i++)
	{
		fp = &rp->faces[face_list[i]];
		int vert = CheckFaceConcavity(fp->num_verts,fp->face_verts,&fp->normal,rp->verts);
		if (vert != -1)
		{
//			mprintf(0,"Room %3d face %3d is concave at vertex %d",ROOMNUM(rp),i,vert);
			bConcave = true;
		}
	}

	if (bConcave)
	{
		int answer = OutrageMessageBox(MB_YESNOCANCEL,"One or more faces would become concave as a result of this move.\nDo you want to split faces to resolve concavities?");
		if (answer == IDYES)
			FixConcaveFaces(rp,face_list,num_faces);
		else
		{
			// Move the verts back to their original positions
			for (i=0; i<num_verts; i++)
			{
				pvert = &rp->verts[list[i]];
				*pvert -= vec;
			}
//			mem_free (face_list);
			return false;
		}
	}
*/

	// Recompute the normals for the affected faces
	int bad_normals = 0;
	for (i=0; i<num_faces; i++)
	{
		if (! ComputeFaceNormal(rp,face_list[i]))
			bad_normals++;
	}

	if (bad_normals > 0) {
		mprintf((1,"Warning: Room %d has %d bad or low-precision normals\n",ROOMNUM(rp),bad_normals));
		Int3();	// Hmm, not good!
	}

//	mem_free (face_list);

	return true;
}


bool MoveVerts(room *rp, short *list, int num_verts, float x, float y, float z)
{
	vector vec = {x,y,z};

	return MoveVerts(rp,list,num_verts,vec);
}


int InsertFace(room *rp, short *vert_list, int num_verts)
{
	int facenum = -1;
	int i;

	if ( rp->num_faces > (MAX_FACES_PER_ROOM - 1) )
		OutrageMessageBox("You cannot add any more faces to this room.");
	else
	{
		if ( num_verts > 2 )
		{
			if ( num_verts < MAX_VERTS_PER_FACE + 1 )
			{
				// TODO : Make sure that all verts lie in the same plane
				int texnum = Editor_state.GetCurrentTexture();
				// Add the face to the room
				facenum = RoomAddFaces(rp,1);
				// Set initial values
				face *fp = &rp->faces[facenum];
				InitRoomFace(fp,num_verts);
				for (i=0; i<num_verts; i++)
					fp->face_verts[i] = vert_list[i];
				// Calculate the normal
				ComputeFaceNormal(rp,facenum);
				// Assign UVs
				AssignDefaultUVsToRoomFace(rp,facenum);
				// Apply texture
				HTextureApplyToRoomFace(rp,facenum,texnum);
			}
			else
				OutrageMessageBox("You have too many vertices marked to create a face.");
		}
		else
			OutrageMessageBox("You must mark at least three vertices before inserting a face.");
	}

	return facenum;
}


bool ExtrudeFace(room *rp, int facenum, vector vec, float dist, BOOL delete_base_face, int inward)
{
	if ( rp->num_verts > (MAX_VERTS_PER_ROOM - 1) )
	{
		OutrageMessageBox("You cannot add any more vertices to this room.");
		return false;
	}
	else
	{
		if ( rp->num_faces > (MAX_FACES_PER_ROOM - 1) )
		{
			OutrageMessageBox("You cannot add any more faces to this room.");
			return false;
		}
		else
		{
			int baseface = facenum;
			int num_new_verts = rp->faces[baseface].num_verts;
			int vertnum = RoomAddVertices(rp,num_new_verts);
			int texnum = Editor_state.GetCurrentTexture();
			int i,j;

			for (i=0; i<num_new_verts; i++)
				rp->verts[vertnum+i] = rp->verts[rp->faces[baseface].face_verts[i]] + (dist ? dist : 10)*vec;

			// Add num_new_verts faces to the room
			facenum = RoomAddFaces(rp,num_new_verts);
			// Set initial values for each face
			for (i=0; i<num_new_verts; i++)
			{
				face *fp = &rp->faces[facenum+i];
				InitRoomFace(fp,4);
				(i == num_new_verts-1) ? (j = 0) : (j = i + 1);
				fp->face_verts[0] = rp->faces[baseface].face_verts[i];
				fp->face_verts[1] = rp->faces[baseface].face_verts[j];
				fp->face_verts[2] = rp->num_verts-num_new_verts+j;
				fp->face_verts[3] = rp->num_verts-num_new_verts+i;
				// Calculate the normal
				ComputeFaceNormal(rp,facenum+i);
				if (!dist)
				{
					rp->verts[fp->face_verts[2]] = rp->verts[fp->face_verts[1]];
					rp->verts[fp->face_verts[3]] = rp->verts[fp->face_verts[0]];
				}
/*
				if ( !ok || (fp->face_verts[3] - fp->face_verts[0]) || (fp->face_verts[2] - fp->face_verts[1]) );
				{
					// Had an error; we've got to compute the normal ourselves (this is a hack for faces extruded a distance of 0)
					fp->normal = 
				}
*/
				// Assign UVs
				AssignDefaultUVsToRoomFace(rp,facenum+i);
				// Apply texture
				HTextureApplyToRoomFace(rp,facenum+i,texnum);
				// Flip face if necessary
				if (inward)
					FlipFace(rp,facenum+i);
			}

			// Add the cap face
			facenum = RoomAddFaces(rp,1);
			// Set initial values for it
			face *fp = &rp->faces[facenum];
			InitRoomFace(fp,num_new_verts);
			for (i=0; i<num_new_verts; i++)
				fp->face_verts[i] = rp->num_verts-num_new_verts+i;
			// Calculate the normal
			ComputeFaceNormal(rp,facenum);
			// Assign UVs
			AssignDefaultUVsToRoomFace(rp,facenum);
			// Apply texture
			HTextureApplyToRoomFace(rp,facenum,texnum);
			// Flip face if necessary
			if (inward)
				FlipFace(rp,facenum);

			// Delete the base face
			if (delete_base_face)
				DeleteRoomFace(rp,baseface);
		}
	}

	return true;
}


bool LatheVerts(room *rp, short *list, int num_verts, int axis, int num_sides, BOOL bCaps, int inward, vector offset)
{
	int i,j,k,m;
	int vertnum,facenum;
	int old_num_verts = rp->num_verts;
	vector v1,v2;
	int texnum = Editor_state.GetCurrentTexture();

	// For each side, copy the marked verts
	for (i=1; i<num_sides; i++)
	{
		double a = (double)i/(double)num_sides;
		angle ang = (angle)(65535 * a);
		matrix mat;
		vertnum = RoomAddVertices(rp,num_verts);
		for (j=0; j<num_verts; j++)
		{
			switch (axis)
			{
			case X_AXIS:
				vm_AnglesToMatrix(&mat,ang,0,0);
				break;

			case Y_AXIS:
				vm_AnglesToMatrix(&mat,0,ang,0);
				break;

			case Z_AXIS:
				vm_AnglesToMatrix(&mat,0,0,ang);
				break;

			default:
				return false;
			}
			v1 = rp->verts[list[j]] - offset;
			vm_MatrixMulVector(&v2,&v1,&mat);
			rp->verts[vertnum+j] = v2 + offset;
		}
	}

	// Create new faces for each part of the lathe
	facenum = RoomAddFaces(rp,num_sides*(num_verts-1));
	face *fp;

	m = 0;
	for (i=0; i<num_sides; i++)
	{
		(i < num_sides-1) ? (k = i + 1) : (k = 0);
		for (j=0; j<=num_verts-2; j++)
		{
			fp = &rp->faces[facenum + m];
			InitRoomFace(fp,4);
			if (i > 0)
			{
				if (k > 0)
				{
					fp->face_verts[0] = old_num_verts + ((i - 1) * num_verts) + j;
					fp->face_verts[1] = old_num_verts + ((i - 1) * num_verts) + j + 1;
					fp->face_verts[2] = old_num_verts + ((k - 1) * num_verts) + j + 1;
					fp->face_verts[3] = old_num_verts + ((k - 1) * num_verts) + j;
				}
				else
				{
					fp->face_verts[0] = old_num_verts + ((i - 1) * num_verts) + j;
					fp->face_verts[1] = old_num_verts + ((i - 1) * num_verts) + j + 1;
					fp->face_verts[2] = list[j + 1];
					fp->face_verts[3] = list[j];
				}
			}
			else
			{
				fp->face_verts[0] = list[j];
				fp->face_verts[1] = list[j + 1];
				fp->face_verts[2] = old_num_verts + ((k - 1) * num_verts) + j + 1;
				fp->face_verts[3] = old_num_verts + ((k - 1) * num_verts) + j;
			}
			// Calculate the normal
			ComputeFaceNormal(rp,facenum+m);
			// Assign UVs
			AssignDefaultUVsToRoomFace(rp,facenum+m);
			// Apply texture
			HTextureApplyToRoomFace(rp,facenum+m,texnum);
/*
			if (j == 0)
				HTextureApplyToRoomFace(rp,facenum,texnum);
			else
				HTexturePropagateToFace(rp,facenum+m,rp,facenum+m-1);
*/
			// Flip face if necessary
			if (!inward)
				FlipFace(rp,facenum+m);
			m++;
		}
	}

	// Create end caps if necessary
	switch (bCaps)
	{
	case FALSE:
		break;

	case TRUE:
		facenum = RoomAddFaces(rp,2);

		fp = &rp->faces[facenum];
		InitRoomFace(fp,num_sides);
		fp->num_verts = num_sides;
		fp->face_verts[0] = list[0];
		for (i=1; i<num_sides; i++)
			fp->face_verts[i] = old_num_verts + ((i - 1) * num_verts);
		// Calculate the normal
		ComputeFaceNormal(rp,facenum);
		// Assign UVs
		AssignDefaultUVsToRoomFace(rp,facenum);
		// Apply texture
		HTextureApplyToRoomFace(rp,facenum,texnum);
		// Flip face if necessary
		if (!inward)
			FlipFace(rp,facenum);

		fp = &rp->faces[facenum+1];
		InitRoomFace(fp,num_sides);
		fp->num_verts = num_sides;
		fp->face_verts[0] = list[num_verts - 1];
		for (i=1; i<num_sides; i++)
			fp->face_verts[i] = old_num_verts + num_verts - 1 + ((i - 1) * num_verts);
		// Calculate the normal
		ComputeFaceNormal(rp,facenum+1);
		// Assign UVs
		AssignDefaultUVsToRoomFace(rp,facenum+1);
		// Apply texture
		HTextureApplyToRoomFace(rp,facenum+1,texnum);
		// Flip face if necessary
		if (inward)
			FlipFace(rp,facenum+1);
		break;
	}

	return true;
}


bool BendVerts(room *rp, short *list, int num_verts, int axis, float degrees, float dist, vector offset)
{
	matrix mat;
	vector v1,v2;
	double rad,bend_angle,factor;
	angle ang;
	int i;

	rad = degrees*PI/180;
	dist ? (bend_angle = 1/dist*rad) : (bend_angle = rad);

	factor = 65535*bend_angle/(2*PI);

	switch (axis)
	{
	case X_AXIS:
		for (i=0; i<num_verts; i++)
		{
			ang = (angle)(factor*(rp->verts[list[i]].y-offset.y));
			vm_AnglesToMatrix(&mat,ang,0,0);
			v1.x = rp->verts[list[i]].x-offset.x;
			v1.y = 0; // -offset.y;
			v1.z = rp->verts[list[i]].z-offset.z;
			vm_MatrixMulVector(&v2,&v1,&mat);
			rp->verts[list[i]] = v2+offset;
		}
		break;

	case Y_AXIS:
		for (i=0; i<num_verts; i++)
		{
			ang = (angle)(factor*(rp->verts[list[i]].z-offset.z));
			vm_AnglesToMatrix(&mat,0,ang,0);
			v1.x = rp->verts[list[i]].x-offset.x;
			v1.y = rp->verts[list[i]].y-offset.y;
			v1.z = 0; // -offset.z;
			vm_MatrixMulVector(&v2,&v1,&mat);
			rp->verts[list[i]] = v2+offset;
		}
		break;

	case Z_AXIS:
		for (i=0; i<num_verts; i++)
		{
			ang = (angle)(factor*(rp->verts[list[i]].y-offset.y));
			vm_AnglesToMatrix(&mat,0,0,ang);
			v1.x = rp->verts[list[i]].x-offset.x;
			v1.y = 0;
			v1.z = rp->verts[list[i]].z-offset.z;
			vm_MatrixMulVector(&v2,&v1,&mat);
			rp->verts[list[i]] = v2+offset;
		}
		break;

	default:
		return false;
	}

	return true;
}


int DeleteUnusedRoomVerts(room *rp);
int RemoveDuplicatePoints(room *rp);
int RemoveDuplicateFacePoints(room *rp);

// Removes all the duplicate and unused points in a room
int RemoveDuplicateAndUnusedPointsInRoom(room *rp)
{
	int n_unused=0,n_duplicate=0,n_duplicate_face=0;

	if (rp->used) {
		n_unused += DeleteUnusedRoomVerts(rp);
		n_duplicate += RemoveDuplicatePoints(rp);
		n_duplicate_face += RemoveDuplicateFacePoints(rp);
	}

	OutrageMessageBox(
			"  %d Unused points deleted\n"
			"  %d Duplicate points deleted\n"
			"  %d Duplicate face points deleted\n",
			n_unused,n_duplicate,n_duplicate_face
		);

	return (n_unused + n_duplicate + n_duplicate_face);
}


// Record which faces contain verts in list
int GetFacesFromVerts(room *rp, short *face_list, short *vert_list, int num_verts)
{
	int i,j,k;
	int num_faces = 0;
	bool inlist;

	for (i=0; i<num_verts; i++)
	{
		for (j=0; j<rp->num_faces; j++)
		{
			if (VertInFace(rp,j,vert_list[i]) != -1)
			{
				inlist = false;
				// Don't count a face twice
				for (k=0; k<num_faces; k++)
				{
					if (j == face_list[k])
						inlist = true;
				}
				if (!inlist)
					face_list[num_faces++] = j;
				ASSERT(num_faces<=rp->num_faces);
			}
		}
	}

	return num_faces;
}

float SnapSingleVertToFace(room *rp,short *list,int num_verts,vector *v0,vector *normal,int snap_vert)
{
	vector *vp;
	float d;

	ASSERT (snap_vert != -1);

	vp = &rp->verts[snap_vert];
	//Calculate new point
	d = vm_DistToPlane(vp,normal,v0);

	// Save old vert position
	vector oldvec = *vp;

	*vp -= *normal * d;

	// Move all the verts by the snap vector
	vector snap_vec = *vp-oldvec;
	MoveVerts(rp,list,num_verts,snap_vec);

	return vm_GetMagnitudeFast(&snap_vec);
}

void SnapVertsToFace(room *rp,short *list,int num_verts,vector *v0,vector *normal)
{
	vector *vp;
	float d;

	for (int i=0; i<num_verts; i++)
	{
		vp = &rp->verts[list[i]];
		//Calculate new point
		d = vm_DistToPlane(vp,normal,v0);
		*vp -= *normal * d;
	}
}

float SnapSingleVertToEdge(room *rp,short *list,int num_verts,vector *v0,vector *v1,int snap_vert)
{
	vector *vp;
	vector edge_vec,edge_normal;
	float d;

	ASSERT (snap_vert != -1);

	vp = &rp->verts[snap_vert];
	//Calculate new point
	edge_normal = edge_vec = *v1 - *v0;
	vm_NormalizeVector(&edge_normal);
	d = vm_DistToPlane(vp,&edge_normal,v0);

	// Save old vert position
	vector oldvec = *vp;

	*vp = *v0 + edge_normal * d;

	// Move all the verts by the snap vector
	vector snap_vec = *vp-oldvec;
	MoveVerts(rp,list,num_verts,snap_vec);

	return vm_GetMagnitudeFast(&snap_vec);
}

void SnapVertsToEdge(room *rp,short *list,int num_verts,vector *v0,vector *v1)
{
	vector *vp;
	vector edge_vec,edge_normal;
	float d;

	for (int i=0; i<num_verts; i++)
	{
		vp = &rp->verts[list[i]];
		//Calculate new point
		edge_normal = edge_vec = *v1 - *v0;
		vm_NormalizeVector(&edge_normal);
		d = vm_DistToPlane(vp,&edge_normal,v0);
		*vp = *v0 + edge_normal * d;
	}
}

float SnapSingleVertToVert(room *rp,short *list,int num_verts,int vertnum,int snap_vert)
{
	vector *vp;

	ASSERT (snap_vert != -1);

	vp = &rp->verts[snap_vert];
	// Save old vert position
	vector oldvec = *vp;

	*vp = rp->verts[vertnum];

	// Move all the verts by the snap vector
	vector snap_vec = *vp-oldvec;
	MoveVerts(rp,list,num_verts,snap_vec);

	return vm_GetMagnitudeFast(&snap_vec);
}

void SnapVertsToVert(room *rp,short *list,int num_verts,int vertnum)
{
	vector *vp;

	for (int i=0; i<num_verts; i++)
	{
		vp = &rp->verts[list[i]];
		*vp = rp->verts[vertnum];
	}
}

void MirrorRoom(room *rp,int axis)
{
	vector center,objpos;
	object *objp;
	int i;

	ComputeRoomCenter(&center,rp);
	int roomnum = ROOMNUM(rp);

	switch (axis)
	{
	case X_AXIS:
		// Mirror vertex positions
		for (i=0; i<rp->num_verts; i++)
			rp->verts[i].x = center.x + (center.x - rp->verts[i].x);
		// Flip faces
		for (i=0; i<rp->num_faces; i++)
			FlipFace(rp,i);
		// Mirror object positions
		for (i=rp->objects; i!=-1; i=Objects[i].next)
		{
			objp = &Objects[i];
			ASSERT(objp->roomnum == roomnum);
			objpos = objp->pos;
			objpos.x = center.x + (center.x - objpos.x);
			ObjSetPos(objp,&objpos,roomnum,&objp->orient,false);
		}
		break;
	case Y_AXIS:
		// Mirror vertex positions
		for (i=0; i<rp->num_verts; i++)
			rp->verts[i].y = center.y + (center.y - rp->verts[i].y);
		// Flip faces
		for (i=0; i<rp->num_faces; i++)
			FlipFace(rp,i);
		// Mirror object positions
		for (i=rp->objects; i!=-1; i=Objects[i].next)
		{
			objp = &Objects[i];
			ASSERT(objp->roomnum == roomnum);
			objpos = objp->pos;
			objpos.y = center.y + (center.y - objpos.y);
			ObjSetPos(objp,&objpos,roomnum,&objp->orient,false);
		}
		break;
	case Z_AXIS:
		// Mirror vertex positions
		for (i=0; i<rp->num_verts; i++)
			rp->verts[i].z = center.z + (center.z - rp->verts[i].z);
		// Flip faces
		for (i=0; i<rp->num_faces; i++)
			FlipFace(rp,i);
		// Mirror object positions
		for (i=rp->objects; i!=-1; i=Objects[i].next)
		{
			objp = &Objects[i];
			ASSERT(objp->roomnum == roomnum);
			objpos = objp->pos;
			objpos.z = center.z + (center.z - objpos.z);
			ObjSetPos(objp,&objpos,roomnum,&objp->orient,false);
		}
		break;
	default:
		Int3();
	}
}


// Ensures that the face and portal elements of the prim struct agree
void MatchFacePortalPair(prim *prim)
{
	if (prim->face != -1)
	{
		ASSERT(prim->portal == prim->roomp->faces[prim->face].portal_num);
		prim->portal = prim->roomp->faces[prim->face].portal_num;
	}
	else
	{
		ASSERT(prim->portal == -1);
		prim->portal = -1;
	}

	if (prim->portal != -1)
	{
		ASSERT(prim->face == prim->roomp->portals[prim->portal].portal_face);
		prim->face = prim->roomp->portals[prim->portal].portal_face;
	}
	else
	{
		if (prim->face != -1)
		{
			ASSERT(prim->roomp->faces[prim->face].portal_num == -1);
			prim->roomp->faces[prim->face].portal_num = -1;
		}
	}
}


// Based on FindSharedEdge in Main\editor\Erooms.cpp

//Finds a shared edge, if one exists, between two faces in the same room, ignoring the direction of the normal
//Parameters:	fp0,fp1 - pointers to the two faces
//					vn0,vn1 - filled in with the vertex numbers of the edge.  These vert numbers are 
//									relative to their own faces.  The shared edge is verts <vn0,vn0+1> on
//									face 0, and <vn1+1,vn1> on face 1
//Returns:		true if a shared edge was found, else false
bool ned_FindSharedEdge(face *fp0,face *fp1,int *vn0,int *vn1)
{
	int i,j,a0,b0,a1,b1;

  	//Go through each edge in first face
  	for (i=0;i<fp0->num_verts;i++) {

		//Get edge verts - <a0,b0> is edge on first face
  		a0 = fp0->face_verts[i];
  		b0 = fp0->face_verts[(i+1)%fp0->num_verts];

  		//Check against second face
  		for (j=0;j<fp1->num_verts;j++) {

			//Get edge verts - <a1,b1> is edge on second face
  			a1 = fp1->face_verts[j];
  			b1 = fp1->face_verts[(j+1)%fp1->num_verts];

  			//@@if ((a0==a1) && (b0==b1))
			//@@	Int3();							//If you hit this, you probably have a duplicate or overlapping face
			
			if ((a0==b1) && (b0==a1) || (a0==a1) && (b0==b1)) { 	//found match!
				*vn0 = i;
				*vn1 = j;
				return 1;
			}
  		}
  	}

	//Didn't find an edge, so return error
	return 0;
}


float ComputeFaceBoundingCircle(vector *center,room *rp,int facenum)
{
	ASSERT(facenum != -1);

	face *fp = &rp->faces[facenum];

	vector face_min,face_max;

	// Get min/max coordinates of face from the verts
	for (int i=0; i<fp->num_verts; i++)
	{
		if (i == 0 || rp->verts[fp->face_verts[i]].x < face_min.x)
			face_min.x = rp->verts[fp->face_verts[i]].x;

		if (i == 0 || rp->verts[fp->face_verts[i]].y < face_min.y)
			face_min.y = rp->verts[fp->face_verts[i]].y;

		if (i == 0 || rp->verts[fp->face_verts[i]].z < face_min.z)
			face_min.z = rp->verts[fp->face_verts[i]].z;

		if (i == 0 || rp->verts[fp->face_verts[i]].x > face_max.x)
			face_max.x = rp->verts[fp->face_verts[i]].x;

		if (i == 0 || rp->verts[fp->face_verts[i]].y > face_max.y)
			face_max.y = rp->verts[fp->face_verts[i]].y;

		if (i == 0 || rp->verts[fp->face_verts[i]].z > face_max.z)
			face_max.z = rp->verts[fp->face_verts[i]].z;
	}

	*center = (face_max+face_min)/2;

	vector temp = face_max-face_min;
	return vm_GetMagnitudeFast(&temp)/2;
}


void DeleteRoomVert(room *rp,int vertnum);

// based on DeleteUnusedRoomVerts

//Check the vertices in a list & remove unused ones
//Parameters:	rp - the room to check
//				list - the list of vert indices to check
//				num_verts - how many in the list
//Returns: the number of vertices removed
int DeleteUnusedVertsInList(room *rp,short *list,int num_verts)
{
	bool vert_used[MAX_VERTS_PER_ROOM];
	int f,v,j;
	face *fp;
	int n_deleted=0;
	CRoomFrm *wnd = theApp.FindRoomWnd(rp);
	prim *prim = theApp.AcquirePrim();

	//Init all the flags to unused
	for (v=0;v<rp->num_verts;v++)
		vert_used[v] = 0;

	//Go through all the faces & flag the used verts
	for (f=0,fp=rp->faces;f<rp->num_faces;f++,fp++)
		for (v=0;v<fp->num_verts;v++)
			vert_used[fp->face_verts[v]] = 1;

	//Delete the verts in our list that are not in the used list
	for (v=0;v<num_verts;v++)
	{
		int vertnum = list[v-n_deleted];
		if (! vert_used[vertnum]) {
			DeleteRoomVert(rp,vertnum);

			if (wnd != NULL)
			{
				// Adjust current vert if necessary
				if (prim->vert >= rp->num_verts)
					prim->vert = vertnum - 1;
				if (!rp->num_verts && prim->vert == -1)
					wnd->m_Current_nif_vert = -1;
				// Shift vert markings down by one
				int nv = rp->num_verts;
				for (j=vertnum; j<nv; j++)
					wnd->m_Vert_marks[j] = wnd->m_Vert_marks[j+1];
				// Now zero out the unused element in the array (IMPORTANT)
				wnd->m_Vert_marks[j] = 0;
				for (j=v-n_deleted; j<num_verts-(n_deleted+1); j++)
				{
					list[j] = list[j+1];
					list[j]--;
				}
				// Now zero out the unused element in the array (IMPORTANT)
				list[j] = 0;
			}

			n_deleted++;
		}
	}

	//Done
	return n_deleted;
}

