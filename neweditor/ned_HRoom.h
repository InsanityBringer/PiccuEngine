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
 #include "room.h"
#include "../editor/group.h"
#include "../editor/erooms.h"

//Placed room info
extern int Placed_room;
extern group *Placed_group;
extern int Placed_room_face;
extern int Placed_door;
extern float Placed_room_angle;
extern vector Placed_room_origin;
extern matrix Placed_room_orient;
extern vector Placed_room_attachpoint;
extern matrix Placed_room_rotmat;
extern room *Placed_baseroomp;
extern int Placed_baseface;

//Recomputes the rotation matrix for the placed room (called when the room rotated)
void ComputePlacedRoomMatrix();

//Place a room for orientation before attachment
//Parameters:	baseroomp - pointer to the room in the mine to which the new room will be attached
//					baseface - the face on baseroomp to attach to
//					placed_room - the number of the room to be attached
//					placed_room_face the face on placed_room that's attached
void PlaceRoom(room *baseroomp,int baseface,room *placed_roomp,int placed_room_face,int placed_room_door);

//Attach an already-placed room
void AttachRoom();

//Takes two faces which are going to be made into a portal and makes them match exactly
//Alternatively, checks if the two faces can be matched
//After this function the two faces will have exactly the same vertices
//Parameters:	rp0,facenum0 - one of the faces
//					rp1,facenum1 - the other face
//					check_only - if set, doesn't change anything; just checks the faces
//Returns the number of points added to the faces
//If just_checking set, returns true if faces match, else false
int MatchPortalFaces(room *rp0,int facenum0,room *rp1,int facenum1,bool check_only=0);

//Add an insert to the edge list
void AddEdgeInsert(int v0,int v1,int new_v);

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
void ClipAgainstEdge(int nv,short *vertnums,vertex *vertices,int *num_vertices,vector *v0,vector *v1,vector *normal,short *inbuf,int *inv,short *outbuf,int *onv);

//Clips a pair of faces against each other.
//Produces one polygon in each face that is the intersection of the two input faces, 
//and zero or more extra polygons, which are parts of the input faces outside the clipping faces.
//The input faces are replaced by the clipped faces, and new faces (formed by the parts of the input
//face outside the clip-against face) are added to the end of the room's facelist.
//This routine assumes that part or all of the being-clipped face is inside the clip-against face
//Parameters:	rp0,face0 - the first room:face
//					rp1,face1 - the second room:face
//Returns:		true if the clip was ok, false if there was an error
bool ClipFacePair(room *rp0,int face0,room *rp1,int face1);

//Adds a new vertex to all instances of a given edge
void AddVertToAllEdges(room *rp,int v0,int v1,int new_v);

//Adds a new point to all instances of a given edge
//First adds the point to the room (or finds it there), then calls AddVertToAllEdges()
void AddPointToAllEdges(room *rp,int v0,int v1,vector *new_v);

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
bool ClipFace(room *arp,int afacenum,room *brp,int bfacenum);

//Adds a new vertex to all instances of a given edge
void AddVertToAllEdges(room *rp,int v0,int v1,int new_v);

