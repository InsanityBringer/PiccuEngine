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

#ifndef RENDER_H
#define RENDER_H

#include "3d.h"

//Variables for debug/test
#if (defined(_DEBUG) || defined(NEWEDITOR))

#define SRF_NO_SHELL		1	//don't render the shell
#define SRF_NO_NON_SHELL	2	//don't render the non-shell

extern int Render_portals;
extern bool Lighting_on;						// If true, draw w/ normal lighting, else draw full brightness
extern ubyte Outline_mode;						// Controls outline drawing.  See constants below
extern ubyte Shell_render_flag;
extern bool Render_floating_triggers;		// If true, render the floating triggers
extern bool Outline_lightmaps;
extern bool Use_software_zbuffer;
extern bool Render_all_external_rooms;		// If true, draw all the outside rooms
extern bool Render_one_room_only;
extern bool Render_inside_only;

#else
#define Lighting_on 1
#define Outline_mode 0
#endif
extern short use_opengl_1555_format;				//DAJ

#ifndef RELEASE
extern int Mine_depth;
#endif
//Macro for checking Outline mode
#define OUTLINE_ON(flag)  ((Outline_mode & (flag + OM_ON)) == (flag + OM_ON))

//Constants for outline mode
#define OM_ON			1
#define OM_MINE			2
#define OM_TERRAIN		4
#define OM_OBJECTS		8
#define OM_SKY			16
#define OM_ALL			(OM_MINE + OM_TERRAIN + OM_OBJECTS + OM_SKY)

extern float Fog_zone_start,Fog_zone_end;
extern bool DoBumpmaps;
extern bool Render_mirror_for_room;
extern bool Vsync_enabled;

extern float Room_light_val;
extern int Room_fog_plane_check;
extern float Room_fog_distance;
extern float Room_fog_eye_distance;
extern vector Room_fog_plane,Room_fog_portal_vert;

struct face;

struct fog_portal_data
{
	short roomnum;
	float close_dist;
	face *close_face;
};

extern fog_portal_data Fog_portal_data[];

extern int Num_fogged_rooms_this_frame;

// Sets fogzone start and end points
void SetFogZoneStart(float z);
void SetFogZoneEnd (float z);

struct room;

// For sorting our textures in state limited environments
struct state_limited_element
{
	int  facenum;
	int sort_key;
};

#define MAX_STATE_ELEMENTS 8000
extern state_limited_element State_elements[MAX_STATE_ELEMENTS];

extern g3Point SolidFogPoints[],AlphaFogPoints[];

// Takes a face and adds the appropriate vertices for drawing in the fog zone
// Returns number of points in new polygon
// New polygon points are in FogPoints array
int FogBlendFace (g3Point **src,int nv,int *num_solid,int *num_alpha);

//Draws the mine, starting at a the specified room
//The rendering surface must be set up, and g3_StartFrame() must have been called
//Parameters:	viewer_roomnum - what room the viewer is in
//					flag_automap - if true, flag segments as visited when rendered
//					called_from_terrain - set if calling this routine from the terrain renderer
void RenderMine(int viewer_roomnum,int flag_automap=0,int called_from_terrain=0);

//Finds what room & face is visible at a given screen x & y
//Everything must be set up just like for RenderMineRoom(), and presumably is the same as 
//for the last frame rendered (though it doesn't have to be)
//Parameters:	x,y - the screen coordinates
//					start_roomnum - where to start rendering
//					roomnum,facenum - these are filled in with the found values
//					if room<0, then an object was found, and the object number is -room-1
//Returns:		1 if found a room, else 0
int FindRoomFace(short x,short y,int start_roomnum,int *roomnum,int *facenum);

//finds what room,face,lumel is visible at a given screen x & y
//Everything must be set up just like for RenderMineRoom(), and presumably is the same as 
//for the last frame rendered (though it doesn't have to be)
//Parameters:	x,y - the screen coordinates
//					start_roomnum - where to start rendering
//					roomnum,facenum,lumel_num - these are filled in with the found values
//Returns:		1 if found a room, else 0
int FindLightmapFace(short x,short y,int start_roomnum,int *roomnum,int *facenum,int *lumel_num);

// This is needed for small view cameras
// It clears the facing array so that it is recomputed
void ResetFacings();

// Renders all the lights glows for this frame
void RenderLightGlows ();

// Called before a frame starts to render - sets all of our light glows to decreasing
void PreUpdateAllLightGlows ();

// Called after a frame has been rendered - slowly morphs our light glows into nothing
void PostUpdateAllLightGlows ();

// Resets our light glows to zero
void ResetLightGlows ();

// Gets the dynamic light value for this position
void GetRoomDynamicScalar (vector *pos,room *rp,float *r,float *g,float *b);

// Sorts our texture states using the quicksort algorithm
void SortStates (state_limited_element *state_array,int cellcount);

// Sets up fog if this room is fogged
void SetupRoomFog (room *rp,vector *eye,matrix *orient,int viewer_room);

//Draw the specified face
//Parameters:	rp - pointer to the room the face is un
//				facenum - which face in the specified room
void RenderFace(room *rp,int facenum);

// Renders a specular face
void RenderSpecularFacesFlat(room *rp);

// Renders fog faces for a room
void RenderFogFaces(room *rp);

// Builds a list of mirror faces for each room and allocs memory accordingly
void ConsolidateMineMirrors();

extern int Num_specular_faces_to_render,Num_fog_faces_to_render;

#endif
