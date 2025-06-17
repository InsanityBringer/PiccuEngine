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
 

#ifndef _HROOM_H
#define _HROOM_H

#include "room.h"

//Make the Marked room/face the current room/face
void SetMarkedRoom();

// Select next face on current room
void SelectNextFace();

// Select previous face on current room
void SelectPrevFace();

// Adds a room at the current room/face.  The room is created by extuding out from the current face
void AddRoom();

//Recomputes the rotation matrix for the placed room (called when the room rotated)
void ComputePlacedRoomMatrix();

//Place a room for orientation before attachment
//Parameters:	baseroomp - pointer to the room in the mine to which the new room will be attached
//					baseface - the face on baseroomp to attach to
//					placed_room - the number of the room to be attached
//					placed_room_face the face on placed_room that's attached
void PlaceRoom(room *baseroomp,int baseface,int placed_room,int placed_room_face,int placed_room_door);

//Lined up a placed room.  Moves the placed room so the closest vert to basevert lines up exactly,
//and the edge from basevert to basevert+1 lines up with the corresponding edge on the placed room.
void SnapRoom(int basevert);

//Attach an already-placed room
void AttachRoom();

//Creates a new bridge room connecting two rooms
//Parameters:	attroomp,attface - one end of the new room
//					baseroom,baseface - the other end of the new room
//The new room is created by extruding from attroom/attface
void BuildBridge(room *attroomp,int attface,room *baseroomp,int baseface);

//Connects two rooms by changing the shape of one room to attach to the other
//Pretty similar to BuildBride(), but doesn't create a new room
//Parameters:	attroomp,attface - one end of the new room
//					baseroom,baseface - the other end of the new room
void JoinRooms(room *attroomp,int attface,room *baseroomp,int baseface);

//Connects two rooms if they already match up exactly
//Parameters:	attroomp,attface - one end of the new room
//					baseroom,baseface - the other end of the new room
void JoinRoomsExact(room *attroomp,int attface,room *baseroomp,int baseface);

//Combine two faces, if they can be combined
//Parameters:	rp - the room the faces are in
//					face0,face1 - the two faces
//Returns:		true if the faces were combined, else false
//Note: The UV coordinates of the new face are derrived from face0
bool CombineFaces(room *rp,int face0,int face1);

//Deletes the given room from the mine
void DeleteRoomFromMine(room *rp);

//Places a room a short distance from the specified room & face
//The new room is not attached to anything
//Parameters:	baseroomp,baseface - the new room is dropped off of this face
//					droproom_num - the room to be dropped
void DropRoom(room *baseroomp,int baseface,int droproom_num);

//Place a room on the terrain for orientation before attachment
//Parameters:	cellnum - the cell where the room is being placed
//					placed_room - the number of the room to be attached
//					placed_room_face the face on placed_room that's attached
void PlaceExternalRoom(int cellnum,int placed_room,int placed_room_face,bool align_to_terrain);

//Find all the adjacent faces between two rooms and join them
void JoinAllAdjacentFaces(room *rp0,room *rp1);

//Propagate a texture from one face to all the faces in the room
//If matching_faces_only is set, only propagate if the face has the same texture
void PropagateToAllFaces(room *rp,int facenum,bool matching_faces_only);

//Splits a face unto triangles, by fanning
//Parameters:	rp,facenum - the face to triangulate
//					vertnum - the vert that is the base of the fan
void TriangulateFace(room *rp,int facenum, int vertnum);

//Deletes the connection between two rooms.  Deletes both portals.
void DeletePortalPair(room *rp,int portalnum);

//Flips a face
void FlipFace(room *rp,int facenum);

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

//Takes two faces which are going to be made into a portal and makes them match exactly
//Alternatively, checks if the two faces can be matched
//After this function the two faces will have exactly the same vertices
//Parameters:	rp0,facenum0 - one of the faces
//					rp1,facenum1 - the other face
//					check_only - if set, doesn't change anything; just checks the faces
//Returns the number of points added to the faces
//If just_checking set, returns true if faces match, else false
int MatchPortalFaces(room *rp0,int facenum0,room *rp1,int facenum1,bool check_only=0);

//Attempt to fix the cracks in a level
void FixCracks();

//Moves a vertex to lie on a specified edge
void SnapPointToEdge(room *rp,int vertnum,vector *v0,vector *v1);

//Moves a vertex to lie on a specified plane
void SnapPointToFace(room *rp,int vertnum,vector *v0,vector *normal);

//If this is not equal to -1, there's a snap that can maybe be undone
extern int Snap_roomnum;

//Moves a vertex to be conincident with another vertex
void SnapPointToPoint(room *rp,int vertnum,room *snapto_rp,int snapto_vertnum);

//Undoes the most recently-performed snap
void UndoSnap();

//Connects two rooms in a pleasing way
void BuildSmoothBridge(room *rp0,int facenum0,room *rp1,int facenum1);

//Functions for building a new face
void AddNewFaceVert();
void StartNewFace();
void EndNewFace();

//Combine the two rooms. base_rp stays, att_rp goes away
//Returns true if combine sucessful, false if can't join
bool CombineRooms(room *base_rp,room *att_rp);

//Creates a an external room and links the specified faces to it
//Parameters:	rp - the room to connect to the new room
//					nfaces - how many faces connect to the new room (this becomes the number of portals)
//					facenums - the list of faces to connect
void LinkToExternalRoom(room *rp,int nfaces,int *facenums);

//Deletes a point from a face
//Parameters:	rp,facenum - the face we're deleting the point from
//					vertindex - the vertex to delete
void DeletePointFromFace(room *rp,int facenum,int vertindex);

//Incorporates the geometry from the specified object into the specified room.  Deletes the object.
//Returns true if worked, false if some error
bool MergeObjectIntoRoom(room *rp,int objnum);

//Delete all the faces connected to the specified face
void DeleteAllConnectedFaces(room *rp,int facenum);

#endif	//ifndef _HROOM_H
