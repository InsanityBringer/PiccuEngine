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
 

#ifndef HTEXTURE_H
#define HTEXTURE_H

#include "pstypes.h"

#ifndef NEWEDITOR
#include "d3edit.h"
#else
#include "../neweditor/stdafx.h"
#include "../neweditor/neweditor.h"
#include "../neweditor/globals.h"
#endif

struct room;

//	function to apply a texture to a segment side.
void HTextureStretchLess(room *rp, int face, int edge);
void HTextureStretchMore(room *rp, int face, int edge);
void HTextureSetDefault(room *rp, int face=Curface);
void HTextureFlipX();
void HTextureFlipY();
void HTextureSlide(room *rp, int facenum, float right, float up);
void HTextureRotate(room *rp, int facenum, angle ang);
void HTextureNextEdge();

//Apply the specified texture to the specified room:face
void HTextureApplyToRoomFace(room *rp,int facenum,int tnum);

//Copy texture from current face to adjacent face, tiling the UV coordinates
//Parameters:	destrp,destface - room:face that the propagate is based on
//					srcrp,srcface - room:face that is changed
//Return:	1 if success, 0 if faces not adjacent
int HTexturePropagateToFace(room *destrp,int destface,room *srcrp,int srcface,bool tex=true);

//Copy texture UVs from one face to another
//Parameters:	destrp,destface - room:face that is changed
//					srcrp,srcface - room:face to copy from
//					offset - vert 0 on source is assigned to vert offset on dest
//Return:	1 if success, 0 if faces don't have the same number of verts
int HTextureCopyUVsToFace(room *destrp,int destface,room *srcrp,int srcface,int offset);

#endif