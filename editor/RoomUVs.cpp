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
 

#include "RoomUVs.h"

#ifndef NEWEDITOR
#include "d3edit.h"
#else
#include "..\neweditor\globals.h"
#endif

#include "pserror.h"

//returns the magnatude of the 2d vector <a,b>
static float zhypot(float a,float b)
{
	return sqrt(a*a + b*b);
}

//	Given u,v coordinates at two vertices, assign u,v coordinates to the other vertices on a face.
//	va, vb = face-relative vertex indices corresponding to uva, uvb.  Ie, they are always in 0..num_verts_in_face
void AssignUVsToFace(room *rp, int facenum, roomUVL *uva, roomUVL *uvb, int va, int vb)
{
	face		*fp = &rp->faces[facenum];
	int		nv = fp->num_verts;
	int		vlo,vhi;
	vector	fvec,rvec,tvec;
	roomUVL	ruvmag,fuvmag,uvlo,uvhi;
	float		fmag,mag01;
	int		i;
	
	float saveu2[MAX_VERTS_PER_FACE],savev2[MAX_VERTS_PER_FACE];
	for (i=0;i<fp->num_verts;i++)
	{
		saveu2[i]=fp->face_uvls[i].u2;
		savev2[i]=fp->face_uvls[i].v2;
	}

	ASSERT( (va<nv) && (vb<nv) );
	ASSERT((abs(va - vb) == 1) || (abs(va - vb) == nv-1));		// make sure the verticies specify an edge

	// We want vlo precedes vhi, ie vlo < vhi, or vlo = num_verts, vhi = 0
	if (va == ((vb + 1) % nv)) {		// va = vb + 1
		vlo = vb;
		vhi = va;
		uvlo = *uvb;
		uvhi = *uva;
	} else {
		vlo = va;
		vhi = vb;
		uvlo = *uva;
		uvhi = *uvb;
	}

	ASSERT(((vlo+1) % nv) == vhi);	// If we are on an edge, then uvhi is one more than uvlo (mod num_verts)
	fp->face_uvls[vlo] = uvlo;
	fp->face_uvls[vhi] = uvhi;

	// Now we have vlo precedes vhi, compute vertices ((vhi+1) % nv) and ((vhi+2) % nv)

	// Assign u,v scale to a unit length right vector.
	fmag = zhypot(uvhi.v - uvlo.v,uvhi.u - uvlo.u);
	if (fmag < 0.001) {
		//mprintf((0,"Warning: fmag = %7.3f, using approximate u,v values\n",f2fl(fmag)));
		ruvmag.u = 256.0;
		ruvmag.v = 256.0;
		fuvmag.u = 256.0;
		fuvmag.v = 256.0;
	} else {
		ruvmag.u = uvhi.v - uvlo.v;
		ruvmag.v = uvlo.u - uvhi.u;

		fuvmag.u = uvhi.u - uvlo.u;
		fuvmag.v = uvhi.v - uvlo.v;
	}

	//Get pointers to our verts
	vector *vv0 = &rp->verts[fp->face_verts[vlo]],
			 *vv1 = &rp->verts[fp->face_verts[vhi]];

	//Get forward vector from our edge
	fvec = *vv1 - *vv0;
	mag01 = vm_NormalizeVector(&fvec);

	//Check for bad vector
	if (mag01 < 0.001 ) {
		OutrageMessageBox("U, V bogosity in room #%i, probably on face #%i.  CLEAN UP YOUR MESS!", ROOMNUM(rp), facenum);
		return;
	}

	//Get right vector from the cross product of the forward vec with the surface normal
	rvec = fvec ^ fp->normal;

	//Normalize uv values
	ruvmag.u /= mag01;
	ruvmag.v /= mag01;
	fuvmag.u /= mag01;
	fuvmag.v /= mag01;

	//Compute UVs for each point
	for (i=1;i<nv-1;i++) {
  		int fv = (vhi+i)%nv;					//vert index in face
  		int rv = fp->face_verts[fv];		//vert index in room

		//Get the vector for this edge
  		tvec = rp->verts[rv] - *vv0;

		//Project the current edge onto our forward & right vectors
		float rproj = tvec * rvec,
				fproj = tvec * fvec;

		//Compute and assign UV values
  		fp->face_uvls[fv].u = uvlo.u + (ruvmag.u * rproj) + (fuvmag.u * fproj);
  		fp->face_uvls[fv].v = uvlo.v + (ruvmag.v * rproj) + (fuvmag.v * fproj);
  	}

}

// Stretches the UVS of a face 
void StretchRoomUVs(room *rp, int facenum, int edge)
{
	roomUVL	uv0,uv1;
	int		v0, v1;
	face *fp=&rp->faces[facenum];

	int i;
	float saveu2[MAX_VERTS_PER_FACE],savev2[MAX_VERTS_PER_FACE];
	for (i=0;i<fp->num_verts;i++)
	{
		saveu2[i]=fp->face_uvls[i].u2;
		savev2[i]=fp->face_uvls[i].v2;
	}

	v0 = edge;
	v1 = (v0 + 1) % rp->faces[facenum].num_verts;

	uv0 = rp->faces[facenum].face_uvls[v0];		//copy uv AND l
	uv1 = rp->faces[facenum].face_uvls[v1];

	AssignUVsToFace(rp, facenum, &uv0, &uv1, v0, v1);

	for (i=0;i<fp->num_verts;i++)
	{
		fp->face_uvls[i].u2=saveu2[i];
		fp->face_uvls[i].v2=savev2[i];
	}

}

//Scale all the UV values in a face from the center point (as defined by averaging the u & v values)
void ScaleFaceUVs(room *rp,int facenum,float scale)
{
	face *fp=&rp->faces[facenum];

	int i;
	float midu=0,midv=0;

	for (i=0;i<fp->num_verts;i++) {
		midu += fp->face_uvls[i].u;
		midv += fp->face_uvls[i].v;
	}

	midu /= fp->num_verts;
	midv /= fp->num_verts;

	for (i=0;i<fp->num_verts;i++)
	{
		fp->face_uvls[i].u = midu + (fp->face_uvls[i].u - midu) * scale;
		fp->face_uvls[i].v = midv + (fp->face_uvls[i].v - midv) * scale;
	}

	World_changed=1;
}
