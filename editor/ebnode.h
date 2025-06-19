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
 

#ifndef EBNODE_H_
#define EBNODE_H_

#include "bnode.h"
#include "room.h"
#include "mem.h"
#include "vecmat.h"
#include "3d.h"
#include "../neweditor/RendHandle.h"

#define EBDRAW_NONE                0
#define EBDRAW_ROOM                1
#define EBDRAW_ROOM_AND_NEXT_ROOMS 2
#define EBDRAW_LEVEL					  3

extern char EBN_draw_type;

extern void EBNode_MakeDefaultIntraRoomNodes(int roomnum);
extern void EBNode_MakeDefaultInterRoomEdges(int roomnum);
extern void EBNode_MakeFirstPass(void);
extern void EBNode_Draw(char draw_type, RendHandle& handle,vector *viewer_eye,matrix *viewer_orient,float zoom);
extern void EBNode_Move(bool f_offset, int roomnum, int pnt, vector *pos);
extern int EBNode_AddNode(int roomnum, vector *pnt, bool f_from_editor, bool f_check_for_close_nodes);
// Note the 2 last parameters are for internal use ONLY never set them
extern void EBNode_AddEdge(int spnt, int sroom, int epnt, int eroom, bool f_make_reverse = true, float computed_max_rad = -1.0f);
extern void EBNode_RemoveEdge(int spnt, int sroom, int epnt, int eroom, bool f_remove_reverse = true);
int EBNode_InsertNodeOnEdge(int spnt, int sroom, int epnt, int eroom);
void EBNode_RemoveNode(int roomnum, int pnt);
void EBNode_ClearLevel();
bool EBNode_VerifyGraph();
void EBNode_AutoEdgeNode(int spnt, int sroom);

#endif