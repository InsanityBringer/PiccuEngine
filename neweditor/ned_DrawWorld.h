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
 


#include "vecmat.h"

// bit flags for DrawWorld(,,,,,,,drawflags)
#define DRAW_ALL	1
#define DRAW_SEL	2
#define DRAW_CUR	4
#define DRAW_OBJ	8
#define DRAW_AXES	16

// color flags
#define CF_DEFAULT	1
#define CF_CURRENT	2
#define CF_MARKED	4

//Find mode values
#define FM_CLOSEST	0	//Finds the closest face
#define FM_SPECIFIC	1	//Finds the passed face
#define FM_NEXT		2	//Find the next-farthest face from the last-found face

void DrawLine(ddgr_color color,int pnum0,int pnum1);
void DrawEdges(int room_color=-1);
//Draw the objects in a room as spheres
void DrawRoomObjects(room *rp);
//Draw an object in a room as a sphere
void DrawRoomObject(room *rp, int objnum, int colorflag);
// Draw object, with orientation vectors
void DrawObject(ddgr_color color,object *obj,bool bDrawOrient);
//	Draw a room
void DrawRoom(room *rp,int room_type = -1);
//	Draw a room rotated and placed in space
void DrawRoomRotated(room *rp,vector *rotpoint,matrix *rotmat,vector *placepoint,int room_color);
// Draws a 3D perspective wireframe view
// Parameters:	prim - the structure containing the primitives to use
void DrawWorld(grViewport *vp,vector *view_target,matrix *view_orient,float view_dist,int start_room,float rad,int drawflags,prim *prim);
void DrawAllRooms(vector *view_target,float rad);

//Finds the closest room:face to the viewer at a given x,y screen position in the wireframe view
//Parameters:	vp - the viewport we're checking.  Must be the same viewport as the last wireframe view rendered
//					x,y - the screen coordinates clicked on
//					roomnum,facenum - pointers to variables to be filled in
//					ingore_previous - if this is true, ignore the previous face & find the next farther face
//Returns:		true if found a room/face
bool WireframeFindRoomFace(grViewport *vp,int x,int y,int *roomnum,int *facenum,int find_mode,bool one_room);

//Adds all the rooms that have a vertex inside of a given screen box to the selected list
//Parameters:	vp - the viewport we're checking.  Must be the same viewport as the last wireframe view rendered
//					left,top,right,bot - the screen coordinates of the box
//Returns:		the number of rooms found
int SelectRoomsInBox(grViewport *vp,int left,int top,int right,int bot);
