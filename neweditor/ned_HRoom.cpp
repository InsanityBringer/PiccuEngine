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
#include "ned_HRoom.h"


//Placed room info
int Placed_door=-1;
int Placed_room=-1;
group *Placed_group = NULL;
int Placed_room_face;
float Placed_room_angle;
vector Placed_room_origin;
matrix Placed_room_orient;
matrix Placed_room_rotmat;
vector Placed_room_attachpoint;
room *Placed_baseroomp;
int Placed_baseface;


//Computes the orientation matrix for the placed room
void ComputePlacedRoomMatrix()
{
	room *placedroomp;
	int placedface;
	matrix srcmat;
	vector t;

	if (Placed_room != -1) {
		placedroomp = &Rooms[Placed_room];
		placedface = Placed_room_face;
	}
	else {
		ASSERT(Placed_group != NULL);
		placedroomp = &Placed_group->rooms[Placed_group->attachroom];
		placedface = Placed_group->attachface;
	}


	//Compute source and destination matrices
	t = -placedroomp->faces[placedface].normal;
	vm_VectorToMatrix(&srcmat,&t,NULL,NULL);
	vm_VectorAngleToMatrix(&Placed_room_orient,&Placed_room_orient.fvec,Placed_room_angle);

	mprintf((0,"srcmat: %f %f %f\n",vm_GetMagnitude(&srcmat.fvec),vm_GetMagnitude(&srcmat.rvec),vm_GetMagnitude(&srcmat.uvec)));
	mprintf((0,"orient: %f %f %f\n",vm_GetMagnitude(&Placed_room_orient.fvec),vm_GetMagnitude(&Placed_room_orient.rvec),vm_GetMagnitude(&Placed_room_orient.uvec)));

	//Make sure the matrices are ok
	vm_Orthogonalize(&srcmat);
	vm_Orthogonalize(&Placed_room_orient);

	//Compute matrix to rotate src -> dest
 	vm_MatrixMulTMatrix(&Placed_room_rotmat,&srcmat,&Placed_room_orient);

	mprintf((0,"rotmat: %f %f %f\n",vm_GetMagnitude(&Placed_room_rotmat.fvec),vm_GetMagnitude(&Placed_room_rotmat.rvec),vm_GetMagnitude(&Placed_room_rotmat.uvec)));

	//Make sure the matrix is ok
	vm_Orthogonalize(&Placed_room_rotmat);
}


//Place a room for orientation before attachment
//Parameters:	baseroomp - pointer to the room in the mine to which the new room will be attached
//					baseface - the face on baseroomp to attach to
//					placed_room - the number of the room to be attached
//					placed_room_face the face on placed_room that's attached
void PlaceRoom(room *baseroomp,int baseface,room *placed_roomp,int placed_room_face,int placed_room_door)
{
	//Clear the placed group if one exists
	Placed_group = NULL;

	ASSERT(baseroomp->faces[baseface].portal_num == -1);

	//Set globals
	Placed_room = ROOMNUM(placed_roomp);
	Placed_room_face = placed_room_face;
	Placed_room_orient.fvec = baseroomp->faces[baseface].normal;
	Placed_room_angle = 0;
	Placed_baseroomp = baseroomp;
	Placed_baseface = baseface;
	Placed_door = placed_room_door;

	//Compute attach points on each face
	ComputeCenterPointOnFace(&Placed_room_attachpoint,baseroomp,baseface);
	ComputeCenterPointOnFace(&Placed_room_origin,placed_roomp,placed_room_face);

	//Compute initial orientation matrix
	ComputePlacedRoomMatrix();

	//Set the flag
	State_changed = 1;
}


//Attach an already-placed room
void AttachRoom()
{
	vector basecenter,attcenter;
	room *baseroomp,*attroomp,*newroomp;
	int baseface,attface;

	ASSERT(Placed_room != -1);

	//Set some vars
	baseroomp = Placed_baseroomp;
	baseface = Placed_baseface;
	attroomp = &Rooms[Placed_room];
	attface = Placed_room_face;
	attcenter = Placed_room_origin;
	basecenter = Placed_room_attachpoint;

	//Get the new room
	newroomp = CreateNewRoom(attroomp->num_verts,attroomp->num_faces,0);
	if (newroomp == NULL) {
		OutrageMessageBox("Cannot attach room: No free rooms.");
		return;
	}

	//Rotate verts, copying into new room
	for (int i=0;i<attroomp->num_verts;i++)
		newroomp->verts[i] = ((attroomp->verts[i] - attcenter) * Placed_room_rotmat) + basecenter;

	//Copy faces to new room
	for (int i=0;i<attroomp->num_faces;i++)
		CopyFace(&newroomp->faces[i],&attroomp->faces[i]);

	//Recompute normals for the faces
	if (! ResetRoomFaceNormals(newroomp))
		Int3();	//Get Matt

	//Copy other values for this room
	newroomp->flags	= attroomp->flags;
	newroomp->num_portals = 0;

	//Check for terrain or mine room
	if (baseroomp == NULL) {				//a terrain room

		//Flag this as an external room
		newroomp->flags |= RF_EXTERNAL;

		//Delete the attach face, which should now be facing the ground
		DeleteRoomFace(newroomp,attface);
	}
	else {										//a mine room

		//Clip the connecting faces against each other
		if (! ClipFacePair(newroomp,attface,baseroomp,baseface)) {
			OutrageMessageBox("Error making portal -- faces probably don't overlap.");
			FreeRoom(newroomp);
			return;
		}
	
		//Make the two faces match exactly
		MatchPortalFaces(baseroomp,baseface,newroomp,attface);
		
		//Create the portals between the rooms
		LinkRoomsSimple(Rooms,ROOMNUM(baseroomp),baseface,ROOMNUM(newroomp),attface);

/*
		// If there is a door, place it!
		if (Placed_door!=-1)
		{
			vector room_center;
			matrix orient = ~Placed_room_rotmat;
			vector doorcenter={0,0,0};

			FreeRoom (&Rooms[Placed_room]);
		
			room_center=((doorcenter - attcenter) * Placed_room_rotmat) + basecenter;

			ObjCreate(OBJ_DOOR, Placed_door, newroomp-Rooms, &room_center, &orient);

			doorway *dp = DoorwayAdd(newroomp,Placed_door);

			Placed_door=-1;
		}
*/
	}

	//Un-place the now-attached room
	Placed_room = -1;

	//Flag the world as changed
	World_changed = 1;
}


//structure for keeping track of vertices inserted in edges (from clipping)
typedef struct {
	int	v0,v1;		//the edge that got the new vert
	int	new_v;		//the new vertex
} edge_insert;

//List of vertices inserted in edges.  Used only during clipping.
edge_insert Edge_inserts[MAX_VERTS_PER_FACE];
int Num_edge_inserts;


//Add an insert to the edge list
void AddEdgeInsert(int v0,int v1,int new_v)
{
	Edge_inserts[Num_edge_inserts].v0 = v0;
	Edge_inserts[Num_edge_inserts].v1 = v1;
	Edge_inserts[Num_edge_inserts].new_v = new_v;
	Num_edge_inserts++;
}


//Clip a polygon against one edge of another polygon
//Fills inbuf and maybe outbuf with new polygons, and writes any new verts to the vertices array
//Parameters:	nv - the number of verts in the polygon to be clipped
//					vertnums - pointer to list of vertex numbers in the polygon
//					vertices - list of vertices referred to in vertnums
//					v0,v1 - the edge we're clipping against
//					normal - the surface normal of the polygon
//					inbuf - the clipped polygon is written to this buffer
//					inv - the number of verts in inbuf is written here
//					outbuf - the new polygon created by the part of the input polygon that was clipped away
//					onv - the number of verys in outbuf
//					num_vertices - pointer to the number of verts in the vertices array
void ClipAgainstEdge(int nv,short *vertnums,vertex *vertices,int *num_vertices,vector *v0,vector *v1,vector *normal,short *inbuf,int *inv,short *outbuf,int *onv)
{
	int i,prev,next,check;
	short *ip = inbuf,*op = outbuf;
	vertex *curv,*prevv,*nextv;
	int inside_points=0,outside_points=0;		//real inside/outside points, distinct from edge points

	for (i=0,prev=nv-1,next=1;i<nv;i++) {

		curv = &vertices[vertnums[i]];

		//Find out where point lies
		check = CheckPointAgainstEdge(&curv->vec,v0,v1,normal);
		if (check == 0) {		//Current vertex is on edge

			//Add to both inside & outside lists
			*op++ = vertnums[i];
			*ip++ = vertnums[i];
		}
		else if (check == -1) {		//Current vertex is outside
			int check2;

			prevv = &vertices[vertnums[prev]];
			nextv = &vertices[vertnums[next]];

			//Clip edge w/ previous vertex
			check2 = CheckPointAgainstEdge(&prevv->vec,v0,v1,normal);
			if (check2 == 1) {		//prev inside, so clip
				ClipEdge(normal,prevv,curv,v0,v1,&vertices[*num_vertices]);
				AddEdgeInsert(vertnums[prev],vertnums[i],*num_vertices);
				*op++ = *ip++ = (*num_vertices)++;
			}

			//Add current vertex to outside polygon
			*op++ = vertnums[i];
			outside_points++;

			//Clip edge w/ next vertex
			check2 = CheckPointAgainstEdge(&nextv->vec,v0,v1,normal);
			if (check2 == 1) {		//next inside, so clip
				ClipEdge(normal,curv,nextv,v0,v1,&vertices[*num_vertices]);
				AddEdgeInsert(vertnums[i],vertnums[next],*num_vertices);
				*op++ = *ip++ = (*num_vertices)++;
			}
		}
		else {			//Current vertex is inside
			ASSERT(check == 1);

			//Add current vertex to inside polygon
			*ip++ = vertnums[i];
			inside_points++;
		}

		prev = i;
		if (++next == nv)
			next = 0;
	}

	//Set number of verts for return.  If no real inside or outside points, then don't count edge points
	*inv = inside_points ? (ip - inbuf) : 0;
	*onv = outside_points ? (op - outbuf) : 0;
}


//Takes two faces which are going to be made into a portal and makes them match exactly
//Alternatively, checks if the two faces can be matched
//After this function the two faces will have exactly the same vertices
//Parameters:	rp0,facenum0 - one of the faces
//					rp1,facenum1 - the other face
//					check_only - if set, doesn't change anything; just checks the faces
//Returns the number of points added to the faces
//If just_checking set, returns true if faces match, else false
int MatchPortalFaces(room *rp0,int facenum0,room *rp1,int facenum1,bool check_only)
{
  	face *fp0 = &rp0->faces[facenum0];
  	face *fp1 = &rp1->faces[facenum1];
  	vector *v0,*v1,*prev_v0,*prev_v1;
  	int n0,n1,i,j,prev_vn0,prev_vn1,max_nv;
	int points_added=0;
  	
  	check_faces:;
  	
  	//First, find one point in common
  	for (i=0;i<fp0->num_verts;i++) {
  		for (j=0;j<fp1->num_verts;j++)
  			if (PointsAreSame(&rp0->verts[fp0->face_verts[i]],&rp1->verts[fp1->face_verts[j]]))
  				break;
  		if (j < fp1->num_verts)
  			break;
  	}
  	if (i >= fp0->num_verts) {
		if (! check_only)
	  		Int3();	//Counldn't find common point!  This is very bad.  Get Matt.
  		return 0;	//no match
  	}
  	
  	prev_vn0 = fp0->face_verts[i];
  	prev_vn1 = fp1->face_verts[j];
  	prev_v0 = &rp0->verts[prev_vn0];
  	prev_v1 = &rp1->verts[prev_vn1];

	//Make starting points identical
	if (! check_only)
		*prev_v0 = *prev_v1;

	//Use the larger number of vert
	max_nv = __max(fp0->num_verts,fp1->num_verts);

  	//Trace through faces, adding points where needed
  	for (n0=n1=1;n0<max_nv && n1<max_nv;n0++,n1++) {
		int vn0,vn1;

recheck:;

		vn0 = fp0->face_verts[(i+n0) % fp0->num_verts];
		vn1 = fp1->face_verts[(j-n1+fp1->num_verts) % fp1->num_verts];

  		v0 = &rp0->verts[vn0];
  		v1 = &rp1->verts[vn1];
  	
  		if (PointsAreSame(v0,v1)) {		//Points are at least very close.
			if (! check_only)
				*v0 = *v1;	//Make the points identical
		}
		else {			//The points are not the same, so we check for an extra (colinear) point

  			float d0,d1;
  	
  			//One of these points should lie along the edge of the other. Find which is which
  			d0 = vm_VectorDistance(v0,prev_v0);
  			d1 = vm_VectorDistance(v1,prev_v1);
  	
  			if (d0 > d1) {		//Point 1 is presumably on the edge prev_v0 -> v0
  	
  				//Make sure the point is actually on the edge
  				if (CheckPointAgainstEdge(v1,prev_v0,v0,&fp0->normal)) {
					if (check_only)
						return 0;
  					Int3();	//point isn't on edge!  Bad!  Get Matt!
  				}
				else {
					if (! check_only) {
  	
		  				//Add the point
						AddPointToAllEdges(rp0,prev_vn0,vn0,v1);
						points_added++;
					}
					else {
						n1++;		//skip edge point & resume checking
						goto recheck;
					}
				}
  			}
  			else {				//Point 0 is presumably on the edge prev_v1 -> v1
  	
  				//Make sure the point is actually on the edge
  				if (CheckPointAgainstEdge(v0,prev_v1,v1,&fp1->normal)) {
					if (check_only)
						return 0;
  					Int3();	//point isn't on edge!  Bad!  Get Matt!
  				}
				else {
  	
					if (! check_only) {
						//Add the point
						AddPointToAllEdges(rp1,prev_vn1,vn1,v0);
						points_added++;
					}
					else {
						n0++;		//skip edge point & resume checking
						goto recheck;
					}
				}
  			}
  	
  			//Start check again
			if (! check_only)
	  			goto check_faces;
  		}
  	
		prev_vn0 = vn0;
		prev_vn1 = vn1;

  		prev_v0 = v0;
  		prev_v1 = v1;
  	}

	if (check_only)
		return 1;		//no errors found, so faces ok

	ASSERT(fp0->num_verts == fp1->num_verts);		//Get Matt!

	return points_added;
}


//Clips a pair of faces against each other.
//Produces one polygon in each face that is the intersection of the two input faces, 
//and zero or more extra polygons, which are parts of the input faces outside the clipping faces.
//The input faces are replaced by the clipped faces, and new faces (formed by the parts of the input
//face outside the clip-against face) are added to the end of the room's facelist.
//This routine assumes that part or all of the being-clipped face is inside the clip-against face
//Parameters:	rp0,face0 - the first room:face
//					rp1,face1 - the second room:face
//Returns:		true if the clip was ok, false if there was an error
bool ClipFacePair(room *rp0,int face0,room *rp1,int face1)
{
	//Clip each face aginst the other
	return (ClipFace(rp0,face0,rp1,face1) && ClipFace(rp1,face1,rp0,face0));
}


//Adds a new vertex to all instances of a given edge
void AddVertToAllEdges(room *rp,int v0,int v1,int new_v)
{
	face *fp;
	int f,v;

	for (f=0,fp=rp->faces;f<rp->num_faces;f++,fp++) {
		for (v=0;v<fp->num_verts;v++) {
			if (((fp->face_verts[v] == v0) && (fp->face_verts[(v+1)%fp->num_verts] == v1)) ||
				 ((fp->face_verts[v] == v1) && (fp->face_verts[(v+1)%fp->num_verts] == v0)))
				 AddVertToFace(rp,f,new_v,v);
		}
	}
}


//Adds a new point to all instances of a given edge
//First adds the point to the room (or finds it there), then calls AddVertToAllEdges()
void AddPointToAllEdges(room *rp,int v0,int v1,vector *new_v)
{
	int newvertnum = RoomAddVertices(rp,1);

	rp->verts[newvertnum] = *new_v;

	AddVertToAllEdges(rp,v0,v1,newvertnum);
}


//Clips a face against another.  Produces one polygon that is the intersection of the two input 
//faces, and zero or more extra polygons, which are parts of the input face outside the clipping face.
//The input face is replaced by the clipped face, and new faces (formed by the parts of the input
//face outside the clip-against face) are added to the end of the room's facelist.
//This routine assumes that part or all of the being-clipped face is inside the clip-against face
//Parameters:	arp - the room with the face that is being changed
//					afacenum - the face being changed
//					brp - the room with the face we're clipping against
//					bfacenum - the face we're clipping against
//Returns:		true if the clip was ok, false if there was an error
bool ClipFace(room *arp,int afacenum,room *brp,int bfacenum)
{
	face *afp = &arp->faces[afacenum];
	face *bfp = &brp->faces[bfacenum];
	int edgenum;
	short vbuf0[MAX_VERTS_PER_FACE],vbuf1[MAX_VERTS_PER_FACE];
	short newface_verts[MAX_VERTS_PER_FACE][MAX_VERTS_PER_FACE];
	int newface_nvs[MAX_VERTS_PER_FACE];
	vertex newverts[MAX_VERTS_PER_FACE];
	int newvertnums[MAX_VERTS_PER_FACE];
	int num_newverts;
	int num_newfaces = 0;
	short *src,*dest;
	int nv;
	int i;

	//Init some stuff
	nv = afp->num_verts;
	src = vbuf0;
	dest = vbuf1;

	Num_edge_inserts = 0;

	//copy our vertices into one buffer
	for (i=0;i<nv;i++) {
		newverts[i].vec = arp->verts[afp->face_verts[i]];
		newverts[i].uvl = afp->face_uvls[i];
		newvertnums[i] = afp->face_verts[i];
		src[i] = i;
	}
	num_newverts = nv;

	//Clip our polygon against each edge
	for (edgenum=0;edgenum<bfp->num_verts;edgenum++) {
		vector *v0,*v1;
		short *outbuf = newface_verts[num_newfaces];
		int *onv = &newface_nvs[num_newfaces];

		v0 = &brp->verts[bfp->face_verts[(bfp->num_verts-edgenum)%bfp->num_verts]];
		v1 = &brp->verts[bfp->face_verts[bfp->num_verts-edgenum-1]];

		ClipAgainstEdge(nv,src,newverts,&num_newverts,v0,v1,&afp->normal,dest,&nv,outbuf,onv);

		if (nv <= 2)			//no new face -- faces must not overlap
			return 0;

		src = dest;
		dest = (src==vbuf0) ? vbuf1 : vbuf0;

		if (newface_nvs[num_newfaces])	//is there a new face?
			num_newfaces++;					//..yes, increment counter
	}

	//Now we have the clipped face and the other new faces
	//Replace the old face, and add the new faces
	int first_new_vert,first_new_face;
	face *fp;

	//Allocate space for the new verts
	first_new_vert = RoomAddVertices(arp,num_newverts-afp->num_verts);

	//Copy new vertices into room & get real vert numbers
	for (i=0;i<num_newverts-afp->num_verts;i++) {
		arp->verts[first_new_vert+i] = newverts[afp->num_verts+i].vec;
		newvertnums[afp->num_verts+i] = first_new_vert+i;
	}

	//Replace the input face with the clipped face
	ReInitRoomFace(afp,nv);
	for (i=0;i<nv;i++) {
		afp->face_verts[i] = newvertnums[src[i]];
		afp->face_uvls[i] = newverts[src[i]].uvl;
	}
	if (! ComputeFaceNormal(arp,afacenum))
		Int3();	//Get Matt

	//Allocate space for the new faces
	first_new_face = RoomAddFaces(arp,num_newfaces);

	//Copy data for new faces (the outside faces)
	for (i=0;i<num_newfaces;i++) {
		fp = &arp->faces[first_new_face+i];
		InitRoomFace(fp,newface_nvs[i]);
		for (int j=0;j<newface_nvs[i];j++) {
			fp->face_verts[j] = newvertnums[newface_verts[i][j]];
			fp->face_uvls[j] = newverts[newface_verts[i][j]].uvl;
		}
		if (! ComputeFaceNormal(arp,first_new_face+i))
			Int3();	//Get Matt
		fp->tmap = arp->faces[afacenum].tmap;
		CopyFaceFlags(fp,&arp->faces[afacenum]);
	}

	//Add new verts to edges
	for (i=0;i<Num_edge_inserts;i++) {
		int v0,v1,new_v;
		v0 = newvertnums[Edge_inserts[i].v0];
		v1 = newvertnums[Edge_inserts[i].v1];
		new_v = newvertnums[Edge_inserts[i].new_v];

		AddVertToAllEdges(arp,v0,v1,new_v);
	}

	//Done!
	return 1;
}


