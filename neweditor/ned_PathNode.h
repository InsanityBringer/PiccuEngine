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
 #ifndef _NED_PATHNODE_H
#define _NED_PATHNODE_H

#include "vecmat.h"

void ned_InsertPath(int roomnum,vector pos,matrix orient);
void ned_InsertNode(int roomnum,vector pos,matrix orient);
void ned_DeletePath(bool force = false);
void ned_DeleteNode();
bool ned_MoveNode(int path,int node,float x,float y,float z);
bool ned_MoveBNode(int room,int node,float x,float y,float z);
void ned_DeleteBNode();
void ned_DeleteEdge();
void ned_InsertBNode(int roomnum,vector pos);
void ned_InsertEdge();

#endif 
