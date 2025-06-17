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
 


#include "render.h"
#include "stdafx.h"
#include "globals.h"
#include "neweditor.h"

#include <stdlib.h>
#include <string.h>

#include "descent.h"
#include "3d.h"
#include "mono.h"
#include "ned_GameTexture.h"
#include "ned_Rend.h"
#include "texture.h"
#include "vclip.h"
#include "program.h"

#include "game.h"
#include "renderobject.h"
#include "door.h"
#include "terrain.h"
#include "renderer.h"
#include "room.h"
#include "lighting.h"
#include "lightmap.h"
#include "limits.h"
#include "lightmap_info.h"
#include "viseffect.h"
#include "weapon.h"
#include "fireball.h"
#include "scorch.h"
#include "findintersection.h"
#include "special_face.h"
#include "boa.h"
#include "config.h"
#include "gameloop.h"
#include "doorway.h"
//#include "TelcomAutomap.h"
#include "postrender.h"

static int Faces_rendered=0;
extern float GetFPS();

//3d point for each vertex for use during rendering a room
ubyte Room_clips[MAX_VERTS_PER_ROOM];		// used for face culling

//default face reflectivity
float Face_reflectivity = 0.5;

//the position of the viewer - valid while a frame is being rendered
static vector Viewer_eye;
static matrix Viewer_orient;
int Viewer_roomnum;
int Flag_automap,Called_from_terrain;

// Fog zone variables
float Fog_zone_start=FLT_MAX,Fog_zone_end=FLT_MAX;

int Must_render_terrain;
int Global_buffer_index;	

bool Dont_draw_terrain = false;

#define WALL_PULSE_INCREMENT	.01

//Variables for various debugging features
#if (!defined(_DEBUG) && !defined(NEWEDITOR))

#define In_editor_mode 0
#define Outline_lightmaps 0
#define Outline_alpha 0
#define Render_floating_triggers 0
// #define Use_software_zbuffer		0
bool Use_software_zbuffer=0;
#define Render_all_external_rooms 0
#define Render_portals 0
#define Render_one_room_only 0
#define Render_inside_only 0
#define Shell_render_flag 0

#else		//ifdef _DEBUG

// If true, draw white outline for each polygon
int Render_portals=0;
ubyte Outline_mode=0;
ubyte Shell_render_flag=0;
bool Outline_alpha=0;
bool Outline_lightmaps=0;
bool Render_floating_triggers=0;
bool Use_software_zbuffer=1;
bool Lighting_on = 0;
bool Render_all_external_rooms=0;
bool In_editor_mode=1;
bool Render_one_room_only=0;
bool Render_inside_only=0;
bool Render_flat_shaded=0;

#endif	//ifdef _DEBUG

#ifndef RELEASE
int Mine_depth;
#endif

#ifndef EDITOR
#define Search_lightmaps 0

#else		//ifndef EDITOR

//Vars for find_seg_side_face()
static int Search_lightmaps=0;		// true if searching for a lightmap
int found_lightmap;

#endif	//ifndef EDITOR


bool Vsync_enabled = true;

// Prototypes
void RenderRoomObjects(room *rp);

//The current window width & height (valid while rendering)
static int Render_width,Render_height;

int   Clear_window_color=-1;
int   Clear_window=2;         // 1 = Clear whole background window, 2 = clear view portals into rest of world, 0 = no clear

float Room_light_val=0;

#define MAX_RENDER_ROOMS   100

char Rooms_visited[MAX_ROOMS+MAX_PALETTE_ROOMS];
int Facing_visited[MAX_ROOMS+MAX_PALETTE_ROOMS];

// For keeping track of portal recursion
ubyte Room_depth_list[MAX_ROOMS+MAX_PALETTE_ROOMS];
short Render_list[MAX_RENDER_ROOMS];
short External_room_list[MAX_ROOMS];
int N_external_rooms;

int Num_specular_faces_to_render=0;
int Num_fog_faces_to_render=0;

#define MAX_EXTERNAL_ROOMS	100
vector External_room_corners[MAX_EXTERNAL_ROOMS][8];
ubyte External_room_codes[MAX_EXTERNAL_ROOMS];
ubyte External_room_project_net[MAX_EXTERNAL_ROOMS];

// For sorting our textures in state limited environments
state_limited_element State_elements[MAX_STATE_ELEMENTS];

// For terrain portals
int Terrain_portal_left,Terrain_portal_right,Terrain_portal_top,Terrain_portal_bottom;

bool Render_mirror_for_room=false;

// For textured view: determines if we should render all the rooms or only those within a certain portal distance from the current room
bool NED_Render_all_rooms=true;
int NED_Portal_depth=10;

//
//  UTILITY FUNCS
//

//Determines if a face renders
//Parameters:	rp - pointer to room that contains the face
//					fp - pointer to the face in question
inline bool FaceIsRenderable(room *rp,face *fp)
{
	//Check for a floating trigger, which doesn't get rendered
	if ((fp->flags & FF_FLOATING_TRIG) && (!In_editor_mode || !Render_floating_triggers))
		return 0;

	//Check for face that's part of a portal
	if (fp->portal_num != -1)
	{
		if (rp->portals[fp->portal_num].flags & PF_RENDER_FACES)
			return 1;
		if (rp->flags & RF_FOG && !In_editor_mode)
			return 1;

		return 0;
	}
		
	//Nothing special, so face renders
	return 1;
}

//Flags for GetFaceAlpha()
#define FA_CONSTANT		1		//face has a constant alpha for the whole face
#define FA_VERTEX			2		//face has different alpha per vertex
#define FA_TRANSPARENT	4		//face has transparency (i.e. per pixel 1-bit alpha)

//Determines if a face draws with alpha blending
//Parameters:	fp - pointer to the face in question
//					bm_handle - the handle for the bitmap for this frame, or -1 if don't care about transparence
//Returns:		bitmask describing the alpha blending for the face
//					the return bits are the ATF_ flags in renderer.h
inline int GetFaceAlpha(face *fp,int bm_handle)
{
	int ret = AT_ALWAYS;
	if (GameTextures[fp->tmap].flags & TF_SATURATE)
	{
		if (fp->flags & FF_VERTEX_ALPHA)
			ret = AT_SATURATE_TEXTURE_VERTEX;
		else
			ret=AT_SATURATE_TEXTURE;
	}
	else
	{
		//Check the face's texture for an alpha value
		if (GameTextures[fp->tmap].alpha < 1.0)
			ret |= ATF_CONSTANT;

		//Someday we'll probably check the bitmap's alpha, too
		//Check for vertex alpha flag
		if (fp->flags & FF_VERTEX_ALPHA)
			ret |= ATF_VERTEX;

		//Check for transparency
		if (GameBitmaps[bm_handle].format!=BITMAP_FORMAT_4444 && GameTextures[fp->tmap].flags & TF_TMAP2)
				ret |= ATF_TEXTURE;
	}

	return ret;
}

//Determine if you should render through a portal
//Parameters:	rp - the room the portal is in
//					pp - the portal we're checking
//Returns:		true if you should render the room to which the portal connects
inline bool RenderPastPortal(room *rp,portal *pp)
{
	//If we don't render the portal's faces, then we see through it
	if (! (pp->flags & PF_RENDER_FACES))
		return 1;

	if (!UseHardware)	// Don't render alpha stuff in software
		return 0;

	//Check if the face's texture has transparency
	face *fp = &rp->faces[pp->portal_face];
  	int bm_handle = ned_GetTextureBitmap(fp->tmap,0);

	if (GetFaceAlpha(fp,bm_handle))
		return 1;	  	//Face has alpha or transparency, so we can see through it
	else
		return 0;		//Not transparent, so no render past
}


int first_terminal_room;

#define round(a)  (int (a + 0.5))

//used during rendering as count of items in render_list[]
int N_render_rooms;


#if (defined(EDITOR) || defined(NEWEDITOR))

#define CROSS_WIDTH  8.0
#define CROSS_HEIGHT 8.0
#define	CURFACE_COLOR		GR_RGB(   0, 255,   0)
#define	CUREDGE_COLOR		GR_RGB( 255, 255,   0)
#define	MARKEDFACE_COLOR	GR_RGB(   0, 255, 255)
#define	MARKEDEDGE_COLOR	GR_RGB(   0, 150, 150)
#define	PLACED_COLOR		GR_RGB( 255,   0, 255)

//Draw outline for current edge & vertex
void OutlineCurrentFace(room *rp,int facenum,int edgenum,int vertnum,ddgr_color face_color,ddgr_color edge_color)
{
	face *fp = &rp->faces[facenum];
	g3Point p0,p1;
	ubyte c0,c1;
	int v;

	for (v=0;v<fp->num_verts;v++) {

		c0 = g3_RotatePoint(&p0,&rp->verts[fp->face_verts[v]]);
		c1 = g3_RotatePoint(&p1,&rp->verts[fp->face_verts[(v+1)%fp->num_verts]]);

	   if (! (c0 & c1)) {      //both not off screen?

		   //Draw current edge in green
			g3_DrawLine((v==edgenum)?edge_color:face_color,&p0,&p1);
		}

		if ((v==vertnum) && (c0==0)) {
		   //Draw a little cross at the current vert
			g3_ProjectPoint(&p0);	  //make sure projected
			rend_SetFlatColor(edge_color);
		   rend_DrawLine(p0.p3_sx-CROSS_WIDTH,p0.p3_sy,p0.p3_sx,p0.p3_sy-CROSS_HEIGHT);
			rend_DrawLine(p0.p3_sx,p0.p3_sy-CROSS_HEIGHT,p0.p3_sx+CROSS_WIDTH,p0.p3_sy);
	      rend_DrawLine(p0.p3_sx+CROSS_WIDTH,p0.p3_sy,p0.p3_sx,p0.p3_sy+CROSS_HEIGHT);
		   rend_DrawLine(p0.p3_sx,p0.p3_sy+CROSS_HEIGHT,p0.p3_sx-CROSS_WIDTH,p0.p3_sy);
	   }
	}

	// Draw upper left cross	
	if (Outline_lightmaps && (rp->faces[facenum].flags & FF_LIGHTMAP))
	{
		ASSERT (rp->faces[facenum].lmi_handle!=BAD_LMI_INDEX);
			
		p0.p3_flags=0;
		c0 = g3_RotatePoint(&p0,&LightmapInfo[rp->faces[facenum].lmi_handle].upper_left);

		if (! c0) 
		{
			//Draw a little cross at the current vert
			g3_ProjectPoint(&p0);	  //make sure projected
			rend_SetFlatColor(GR_RGB(255,0,0));
			rend_DrawLine(p0.p3_sx-CROSS_WIDTH,p0.p3_sy,p0.p3_sx,p0.p3_sy-CROSS_HEIGHT);
			rend_DrawLine(p0.p3_sx,p0.p3_sy-CROSS_HEIGHT,p0.p3_sx+CROSS_WIDTH,p0.p3_sy);
			rend_DrawLine(p0.p3_sx+CROSS_WIDTH,p0.p3_sy,p0.p3_sx,p0.p3_sy+CROSS_HEIGHT);
			rend_DrawLine(p0.p3_sx,p0.p3_sy+CROSS_HEIGHT,p0.p3_sx-CROSS_WIDTH,p0.p3_sy);
		}
	}

}

//	Draw a room rotated and placed in space
static void DrawPlacedRoomFace(room *rp,vector *rotpoint,matrix *rotmat,vector *placepoint,int facenum,int color)
{
	face	*fp = &rp->faces[facenum];

	g3Point p0,p1;
	ubyte c0,c1;
	int v;
	for (v=0;v<fp->num_verts;v++) {
		vector tv;

		tv = (rp->verts[fp->face_verts[v]] - *rotpoint) * *rotmat + *placepoint;
		c0 = g3_RotatePoint(&p0,&tv);

		tv = (rp->verts[fp->face_verts[(v+1)%fp->num_verts]] - *rotpoint) * *rotmat + *placepoint;
		c1 = g3_RotatePoint(&p1,&tv);

	   if (! (c0 & c1))       //both not off screen?
			g3_DrawLine(color,&p0,&p1);
	}
}

#endif	//ifdef EDITOR


typedef struct clip_wnd {
	float left,top,right,bot;
} clip_wnd;

inline int clip2d(small_point *pnt,clip_wnd *wnd)
{
	int ret = 0;

	if (pnt->p3_codes & CC_BEHIND)
		return CC_BEHIND;
		
	if (pnt->p3_sx < wnd->left)
		ret |= CC_OFF_LEFT;

	if (pnt->p3_sx > wnd->right)
		ret |= CC_OFF_RIGHT;

	if (pnt->p3_sy < wnd->top)
		ret |= CC_OFF_TOP;

	if (pnt->p3_sy > wnd->bot)
		ret |= CC_OFF_BOT;

	return ret;
}



#define STEPSIZE		.01f
#define STEPSIZE_MIN	.1f

void RenderFloatingTrig(room *rp,face *fp)\
{
	if (! Render_floating_triggers)
		return;

  	vector leftvec,rightvec;
  	vector left,right;
  	vector left_step,right_step;
  	int n_steps,i,j;
	g3Point p3;
	float stepsize;

  	left = rp->verts[fp->face_verts[0]];
  	right = rp->verts[fp->face_verts[1]];

	g3_RotatePoint(&p3,&left);
	stepsize = STEPSIZE * p3.p3_z;
	if (stepsize < STEPSIZE_MIN)
		stepsize = STEPSIZE_MIN;

  	leftvec = rp->verts[fp->face_verts[3]] - rp->verts[fp->face_verts[0]];
  	rightvec = rp->verts[fp->face_verts[2]] - rp->verts[fp->face_verts[1]];

  	n_steps = (vm_GetMagnitude(&leftvec) / stepsize + 0.5) + 1;
  	left_step = leftvec / n_steps;
  	right_step = rightvec / n_steps;

  	for (i=0;i<n_steps;i++) {
  		vector p;
  		g3Point p3;
  		vector crossvec,cross_step;
  		int n_crosssteps;
		
  		crossvec = right - left;
  		n_crosssteps = (vm_GetMagnitude(&crossvec) / stepsize + 0.5) + 1;
  		cross_step = crossvec / n_steps;

  		p = left;

  		for (j=0;j<n_crosssteps;j++) {
  			if (g3_RotatePoint(&p3,&p) == 0) {		//on screen
  				g3_ProjectPoint(&p3);
				rend_SetPixel(GR_RGB(255,100,100),p3.p3_sx,p3.p3_sy);
			}
  			p += cross_step;
  		}

  		left += left_step;
  		right += right_step;
  	}
}


//Draw the specified face
//Parameters:	rp - pointer to the room the face is un
//				facenum - which face in the specified room
void RenderFace(room *rp,int facenum)
{
	int		vn,drawn=0;
	face		*fp = &rp->faces[facenum];
	g3Point	*pointlist[MAX_VERTS_PER_FACE];
	g3Point  pointbuffer[MAX_VERTS_PER_FACE];
	int		bm_handle;
	float		uchange=0,vchange=0;
	texture_type tt;
	ubyte	do_triangle_test=0;
	g3Codes face_cc;
	static first=1;
	static float lm_red[32],lm_green[32],lm_blue[32];
	bool spec_face=0;
	face_cc.cc_and=0xff;
	face_cc.cc_or=0;
	#if (defined(EDITOR) || defined(NEWEDITOR))
	if (fp->flags & FF_FLOATING_TRIG) {
		RenderFloatingTrig(rp,fp);
		return;
	}
	#endif
	// Clear triangulation flag
	fp->flags&=~FF_TRIANGULATED;
	if (rp->flags & RF_TRIANGULATE)
		do_triangle_test=1;
	// Figure out if there is any texture sliding
	if (GameTextures[fp->tmap].slide_u!=0)
	{
		int int_time=Gametime/GameTextures[fp->tmap].slide_u;
		float norm_time=Gametime-(int_time*GameTextures[fp->tmap].slide_u);
		norm_time/=GameTextures[fp->tmap].slide_u;
		uchange=norm_time;
	}
	if (GameTextures[fp->tmap].slide_v!=0)
	{
		int int_time=Gametime/GameTextures[fp->tmap].slide_v;
		float norm_time=Gametime-(int_time*GameTextures[fp->tmap].slide_v);
		norm_time/=GameTextures[fp->tmap].slide_v;
		vchange=norm_time;
	}
	
		for (vn=0;vn<fp->num_verts;vn++) 
		{
			pointbuffer[vn]=*((g3Point *)&World_point_buffer[rp->wpb_index+fp->face_verts[vn]]);
			g3Point *p = &pointbuffer[vn];
  			pointlist[vn] = p;
			p->p3_uvl.u = fp->face_uvls[vn].u;
			p->p3_uvl.v = fp->face_uvls[vn].v;
			p->p3_uvl.u2 = fp->face_uvls[vn].u2;
			p->p3_uvl.v2 = fp->face_uvls[vn].v2;
				
			p->p3_flags |= PF_UV + PF_L + PF_UV2;	//has uv and l set
			#ifndef RELEASE
				if ((fp->flags & FF_LIGHTMAP) && UseHardware)
					p->p3_uvl.l=Room_light_val;
				else
					p->p3_uvl.l=1.0;
			#else
				p->p3_uvl.l=Room_light_val;
			#endif
		
			// do texture sliding
			p->p3_uvl.u+=uchange;
			p->p3_uvl.v+=vchange;
			face_cc.cc_and&=p->p3_codes;
			face_cc.cc_or|=p->p3_codes;
		
	  	}
	// Do stupid gouraud shading for lightmap
	if (NoLightmaps)
	{
		if (first)
		{
			first=0;
			for (int i=0;i<32;i++)
			{
				lm_red[i]=(float)i/31.0;
				lm_green[i]=(float)i/31.0;
				lm_blue[i]=(float)i/31.0;
			}
		}
	
		if (fp->flags & FF_LIGHTMAP)
		{
			int lm_handle=LightmapInfo[fp->lmi_handle].lm_handle;
			ushort *data=(ushort *)lm_data(lm_handle);
			int w=lm_w(lm_handle);
			int h=lm_h(lm_handle);
			
			for (int i=0;i<fp->num_verts;i++)
			{
				float u=fp->face_uvls[i].u2*(w-1);
				float v=fp->face_uvls[i].v2*(h-1);
				g3Point *p = &pointbuffer[i];
				int int_u=u;
				int int_v=v;
				ushort texel=data[int_v*w+int_u];
				int r=(texel>>10) & 0x1f;
				int g=(texel>>5) & 0x1f;
				int b=(texel) & 0x1f;
				p->p3_r=p->p3_l*lm_red[r];
				p->p3_g=p->p3_l*lm_green[g];
				p->p3_b=p->p3_l*lm_blue[b];
				p->p3_flags|=PF_RGBA;
			}
		}
		else
		{
			for (int i=0;i<fp->num_verts;i++)
			{
				g3Point *p = &pointbuffer[i];
				p->p3_r=p->p3_l;
				p->p3_g=p->p3_l;
				p->p3_b=p->p3_l;
				p->p3_flags|=PF_RGBA;
			}
		}
	}
  	//Get bitmap handle
	if ((fp->flags & FF_DESTROYED) && (GameTextures[fp->tmap].flags & TF_DESTROYABLE)) {
		bm_handle = GetTextureBitmap(GameTextures[fp->tmap].destroy_handle,0);
		ASSERT(bm_handle != -1);
	}
	else
	  	bm_handle = GetTextureBitmap(fp->tmap,0);
	//If searching, 
	#if (defined(EDITOR) || defined(NEWEDITOR))
		ddgr_color oldcolor;
		if (TSearch_on)
		{
			rend_SetPixel (GR_RGB(16,255,16),TSearch_x,TSearch_y);
			oldcolor=rend_GetPixel (TSearch_x,TSearch_y);
		}
	#endif
	//Set alpha, transparency, & lighting for this face
	rend_SetAlphaType(GetFaceAlpha(fp,bm_handle));
	
		rend_SetAlphaValue (GameTextures[fp->tmap].alpha*255);
	if (!UseHardware)
		rend_SetLighting (Lighting_on?LS_GOURAUD:LS_NONE);
	else
		rend_SetLighting (LS_GOURAUD);
	if (!NoLightmaps)
		rend_SetColorModel (CM_MONO);
	else
		rend_SetColorModel (CM_RGB);
	// Set lighting map
	if ((fp->flags & FF_LIGHTMAP) && (!StateLimited || UseMultitexture))
	{
		if (GameTextures[fp->tmap].flags & TF_SATURATE)
			rend_SetOverlayType (OT_NONE);
		else
			rend_SetOverlayType (OT_BLEND);
		rend_SetOverlayMap (LightmapInfo[fp->lmi_handle].lm_handle);
	}
	else
		rend_SetOverlayType (OT_NONE);
	//Select texture type
	if (!UseHardware)
	{
		if (Render_flat_shaded)
			tt = TT_FLAT;
		else
		{
		tt = TT_LINEAR;										//default to linear
		for (vn=0;vn<fp->num_verts;vn++)					//select perspective if close
			if (pointlist[vn]->p3_vec.z < 35) {
				tt = TT_PERSPECTIVE;
				break;
			}
		}
		rend_SetTextureType(tt);
	}
	else
		rend_SetTextureType(TT_PERSPECTIVE);
	if (face_cc.cc_or) // Possible triangulate this face cuz its off screen somewhat
	{
		if (Room_light_val<1.0)
			do_triangle_test=1;

		if (spec_face)
			do_triangle_test=1;
	}
	if (do_triangle_test)
	{
		fp->flags|=FF_TRIANGULATED;
		g3_SetTriangulationTest (1);
	}

	//Draw the damn thing
	drawn=g3_DrawPoly(fp->num_verts,pointlist,bm_handle,MAP_TYPE_BITMAP,&face_cc);
	#if (defined(EDITOR) || defined(NEWEDITOR))
		if (TSearch_on)
		{
			if (rend_GetPixel (TSearch_x,TSearch_y)!=oldcolor)
			{
				TSearch_found_type=TSEARCH_FOUND_MINE;
				TSearch_seg=rp-Rooms;
				TSearch_face=facenum;
			}
		}
	#endif
	
	if (do_triangle_test)
		g3_SetTriangulationTest (0);
	// Mark it as rendered
	if (drawn)
		fp->renderframe = FrameCount % 256;
	//if (Render_mirror_for_room && (GameTextures[fp->tmap].flags & TF_ALPHA))
		//rend_SetZBufferWriteMask (1);
	#if (defined(EDITOR) || defined(NEWEDITOR))
	if (OUTLINE_ON(OM_MINE))		//Outline the face
	{
		rend_SetTextureType (TT_FLAT);
		rend_SetAlphaType (AT_ALWAYS);
		rend_SetFlatColor (GR_RGB(255,255,255));
				
		if (UseHardware)
		{
			rend_SetZBias (-.1f);
			for (int i=0;i<fp->num_verts;i++)
			{
			   g3_DrawSpecialLine(pointlist[i],pointlist[(i+1)%fp->num_verts]);
	
			}
			rend_SetZBias (0);
		}
		else
		{
			for (int i=0;i<fp->num_verts;i++)
			{
			   g3_DrawLine(GR_RGB(255,255,255),pointlist[i],pointlist[(i+1)%fp->num_verts]);
	
			}
		}
		if ((fp->flags & FF_HAS_TRIGGER) && (fp->num_verts > 3)) {
		   g3_DrawLine(CUREDGE_COLOR,pointlist[0],pointlist[2]);
		   g3_DrawLine(CUREDGE_COLOR,pointlist[1],pointlist[3]);
		}
		if (fp->special_handle!=BAD_SPECIAL_FACE_INDEX)
		{
			g3Point p1,p2;
			vector verts[MAX_VERTS_PER_FACE];
			vector center,end;
			for (int t=0;t<fp->num_verts;t++)
				verts[t]=rp->verts[fp->face_verts[t]];
			vm_GetCentroid (&center,verts,fp->num_verts);
			vector subvec=SpecialFaces[fp->special_handle].spec_instance[0].bright_center-center;
			vm_NormalizeVectorFast (&subvec);
			end=center+subvec;
			g3_RotatePoint (&p1,&center);
			g3_RotatePoint (&p2,&end);
			g3_DrawLine(GR_RGB(255,255,255),&p1,&p2);
			/*for (t=0;t<fp->num_verts;t++)
			{
				end=rp->verts[fp->face_verts[t]]+SpecialFaces[fp->special_handle].vertnorms[t];
				g3_RotatePoint (&p1,&rp->verts[fp->face_verts[t]]);
				g3_RotatePoint (&p2,&end);
				g3_DrawLine(GR_RGB(255,255,255),&p1,&p2);
			}*/
		}
	}
	if (Outline_lightmaps)
	{
		rend_SetTextureType (TT_FLAT);
		rend_SetAlphaType (AT_ALWAYS);
		if (fp==&Curroomp->faces[Curface] && (fp->flags & FF_LIGHTMAP))
		{
			ASSERT (fp->lmi_handle!=BAD_LMI_INDEX);
	
			lightmap_info *lmi=&LightmapInfo[fp->lmi_handle];
			ushort *src_data=(ushort *)lm_data(lmi->lm_handle);
			matrix facematrix;
			vector fvec=-lmi->normal;
			vm_VectorToMatrix(&facematrix,&fvec,NULL,NULL);
			vector rvec=facematrix.rvec*lmi->xspacing;
			vector uvec=facematrix.uvec*lmi->yspacing;
			vm_TransposeMatrix (&facematrix);
			int w=lm_w (lmi->lm_handle);
			int h=lm_h (lmi->lm_handle);
			for (int i=0;i<w*h;i++) 
			{
				int t;
				g3Point epoints[20];
				vector evec[20];
				int y=i/w;
				int x=i%w;
				evec[0]=lmi->upper_left-(y*uvec)+(x*rvec);
				g3_RotatePoint(&epoints[0],&evec[0]);
				pointlist[0] = &epoints[0];
				evec[1]=lmi->upper_left-(y*uvec)+((x+1)*rvec);
				g3_RotatePoint(&epoints[1],&evec[1]);
				pointlist[1] = &epoints[1];
				evec[2]=lmi->upper_left-((y+1)*uvec)+((x+1)*rvec);
				g3_RotatePoint(&epoints[2],&evec[2]);
				pointlist[2] = &epoints[2];
				evec[3]=lmi->upper_left-((y+1)*uvec)+(x*rvec);
				g3_RotatePoint(&epoints[3],&evec[3]);
				pointlist[3] = &epoints[3];
	
				if (!(src_data[y*w+x] & OPAQUE_FLAG))
				{
					for (t=0;t<4;t++)
						g3_DrawLine(GR_RGB(255,0,255),pointlist[t],pointlist[(t+1)%4]);
				}
				else
				{
					for (t=0;t<4;t++)
						g3_DrawLine(GR_RGB(255,255,255),pointlist[t],pointlist[(t+1)%4]);
				}
				// Draw interpolated normals
				/*if (fp->special_handle!=BAD_SPECIAL_FACE_INDEX && SpecialFaces[fp->special_handle].normal_map!=NULL)
				{
					vector norm,from,to,temp;
					g3Point p1,p2;
					vm_MakeZero (&from);
					norm.x=Normal_table[SpecialFaces[fp->special_handle].normal_map[i*3+0]];
					norm.y=Normal_table[SpecialFaces[fp->special_handle].normal_map[i*3+1]];
					norm.z=Normal_table[SpecialFaces[fp->special_handle].normal_map[i*3+2]];
					vm_MatrixMulVector (&temp,&norm,&facematrix);
					norm=temp;
					for (t=0;t<4;t++)
						from+=evec[t];
					from/=4;
					to=from+norm;
					g3_RotatePoint (&p1,&from);
					g3_RotatePoint (&p2,&to);
					g3_DrawLine(GR_RGB(255,255,255),&p1,&p2);
				}*/
				
/*
				if (Search_lightmaps)
				{
					for (t=0;t<4;t++)
						g3_ProjectPoint (&epoints[t]);
			
					if (point_in_poly(4,epoints,TSearch_x,TSearch_y))
					{
						found_lightmap=i;
						TSearch_found_type=TSEARCH_FOUND_MINE;
						TSearch_seg = ROOMNUM(rp);
  						TSearch_face = facenum;
					}
	
				}
*/
			}
		}
	}
	#endif
}



// Takes a min,max vector and makes a surrounding cube from it
void MakePointsFromMinMax (vector *corners,vector *minp,vector *maxp)
{
	corners[0].x=minp->x;
	corners[0].y=maxp->y;
	corners[0].z=minp->z;

	corners[1].x=maxp->x;
	corners[1].y=maxp->y;
	corners[1].z=minp->z;

	corners[2].x=maxp->x;
	corners[2].y=minp->y;
	corners[2].z=minp->z;

	corners[3].x=minp->x;
	corners[3].y=minp->y;
	corners[3].z=minp->z;

	corners[4].x=minp->x;
	corners[4].y=maxp->y;
	corners[4].z=maxp->z;

	corners[5].x=maxp->x;
	corners[5].y=maxp->y;
	corners[5].z=maxp->z;

	corners[6].x=maxp->x;
	corners[6].y=minp->y;
	corners[6].z=maxp->z;

	corners[7].x=minp->x;
	corners[7].y=minp->y;
	corners[7].z=maxp->z;
}


// Rotates all the points in a room
void RotateRoomPoints (room *rp,vector *world_vecs)
{
	int i;
	for (i=0;i<rp->num_verts;i++)
	{
		g3_RotatePoint((g3Point *)&World_point_buffer[rp->wpb_index+i],&world_vecs[i]);
		g3_ProjectPoint ((g3Point *)&World_point_buffer[rp->wpb_index+i]);
	}
}



static float face_depth[MAX_FACES_PER_ROOM];

//compare function for room face sort
static int room_face_sort_func(const short *a, const short *b)
{
	float az,bz;

	az = face_depth[*a];
	bz = face_depth[*b];

	if (az < bz)
		return -1;
	else if (az > bz)
		return 1;
	else
		return 0;
}

//Sorts the faces of a room before rendering.  Used for rendering in the editor.
//Parameters:	rp	- pointer to the room to be rendered
void RenderRoomSorted(room *rp)
{
	int vn,fn,i,rcount;
	int render_order[MAX_FACES_PER_ROOM];

	ASSERT(rp->num_faces <= MAX_FACES_PER_ROOM);

	//Rotate all the points
	if (rp->wpb_index==-1)
	{
		rp->wpb_index=Global_buffer_index;

			RotateRoomPoints (rp,rp->verts);
			
		Global_buffer_index+=rp->num_verts;
	}

	//Build list of visible (non-backfacing) faces, & compute average face depths
	for (fn=rcount=0;fn<rp->num_faces;fn++) {
		face		*fp = &rp->faces[fn];
	
		if ((!(fp->flags & FF_VISIBLE)) || ((fp->flags & FF_NOT_FACING)))
		{
			fp->flags &=~(FF_NOT_FACING|FF_VISIBLE);
			continue;		// this guy shouldn't be rendered
		}

		// Clear visibility flags
		fp->flags &=~(FF_VISIBLE|FF_NOT_FACING);
		
		if (! FaceIsRenderable(rp,fp))
			continue;	//skip this face

		#if (defined(EDITOR) || defined(NEWEDITOR))
		if (In_editor_mode) {
			if ((Shell_render_flag & SRF_NO_NON_SHELL) && (fp->flags & FF_NOT_SHELL))
				continue;
			if ((Shell_render_flag & SRF_NO_SHELL) && !(fp->flags & FF_NOT_SHELL))
				continue;
		}
		#endif

		face_depth[fn] = 0;

		for (vn=0;vn<fp->num_verts;vn++)
			face_depth[fn] += World_point_buffer[rp->wpb_index+fp->face_verts[vn]].p3_z;

		face_depth[fn] /= fp->num_verts;

		//initialize order list
		render_order[rcount] = fn;

		rcount++;
		
	}

	//Sort the faces
	qsort(render_order,rcount,sizeof(*render_order),(int (cdecl *)(const void*,const void*)) room_face_sort_func);

	//Render the faces
	for (i=rcount-1;i>=0;i--)
		RenderFace(rp,render_order[i]);
}




//Renders the faces in a room without worrying about sorting.  Used in the game when Z-buffering is active
void RenderRoomUnsorted(room *rp)
{
	int fn;
	int rcount=0;

	ASSERT(rp->num_faces <= MAX_FACES_PER_ROOM);

	// Rotate points in this room if need be
	if (rp->wpb_index==-1)
	{
		rp->wpb_index=Global_buffer_index;
			RotateRoomPoints (rp,rp->verts);
		Global_buffer_index+=rp->num_verts;
	}

	//Check for visible (non-backfacing) faces, & render
	for (fn=0;fn<rp->num_faces;fn++) {
		face		*fp = &rp->faces[fn];
		int fogged_portal=0;
				
		if (!(fp->flags & FF_VISIBLE) || (fp->flags & FF_NOT_FACING))
		{
/*
			if (GameTextures[fp->tmap].flags & TF_SMOOTH_SPECULAR)
			{
				if (!Render_mirror_for_room && Detail_settings.Specular_lighting && (GameTextures[fp->tmap].flags & TF_SPECULAR) &&
					((fp->special_handle!=BAD_SPECIAL_FACE_INDEX) || (rp->flags & RF_EXTERNAL)))
					{
						fp->flags|=FF_SPEC_INVISIBLE;
						UpdateSpecularFace(rp,fp);
					}
			
			}
*/
			fp->flags &=~(FF_NOT_FACING|FF_VISIBLE);
			continue;		// this guy shouldn't be rendered
		}

		// Clear visibility flags
		fp->flags &=~(FF_VISIBLE|FF_NOT_FACING);

		if (! FaceIsRenderable(rp,fp))
			continue;	//skip this face

		#ifdef EDITOR
		if (In_editor_mode) {
			if ((Shell_render_flag & SRF_NO_NON_SHELL) && (fp->flags & FF_NOT_SHELL))
				continue;
			if ((Shell_render_flag & SRF_NO_SHELL) && !(fp->flags & FF_NOT_SHELL))
				continue;
		}
		#endif

		if (Render_mirror_for_room==false &&  (fogged_portal || (GetFaceAlpha(fp,-1) & (ATF_CONSTANT+ATF_VERTEX))))
		{
			// Place alpha faces into our postrender list
			face_depth[fn] = 0;
			for (int vn=0;vn<fp->num_verts;vn++)
				face_depth[fn] += World_point_buffer[rp->wpb_index+fp->face_verts[vn]].p3_z;

			Postrender_list[Num_postrenders].type=PRT_WALL;
			Postrender_list[Num_postrenders].roomnum=rp-Rooms;
			Postrender_list[Num_postrenders].facenum=fn;
			Postrender_list[Num_postrenders++].z=face_depth[fn] /= fp->num_verts;;
		}
		else
		{
			if (!StateLimited)
			{
				RenderFace(rp,fn);

			}
			else
			{
				//setup order list
				State_elements[rcount].facenum = fn;

				if (fp->flags & FF_LIGHTMAP)
					State_elements[rcount].sort_key = (LightmapInfo[fp->lmi_handle].lm_handle*MAX_TEXTURES)+fp->tmap;
				else
					State_elements[rcount].sort_key=fp->tmap;
				rcount++;
			}
		}
	}

	if (StateLimited)
	{
		//Sort the faces
		SortStates (State_elements,rcount);
		
		//Render the faces
		for (int i=rcount-1;i>=0;i--)
			RenderFace(rp,State_elements[i].facenum);

		if (!UseMultitexture)
		{
			// Since we're state limited, we have to render lightmap faces completely separate
			// Now render lightmap faces
			rend_SetAlphaType(AT_LIGHTMAP_BLEND);
			rend_SetLighting (LS_GOURAUD);
			rend_SetColorModel (CM_MONO);
			rend_SetOverlayType (OT_NONE);
			rend_SetTextureType(TT_PERSPECTIVE);
			rend_SetWrapType (WT_CLAMP);
			rend_SetMipState (0);

/*
			for (i=rcount-1;i>=0;i--)
				RenderLightmapFace(rp,State_elements[i].facenum);
*/

			rend_SetWrapType (WT_WRAP);
			rend_SetMipState (1);
		}
	}
}



// Renders a specific room.  If pos_offset is not NULL, adds that offset to each of the
// rooms vertices
void RenderRoom(room *rp)
{
	/*#ifdef EDITOR
		if (OBJECT_OUTSIDE(Viewer_object)==0 && !(rp->flags & RF_EXTERNAL))
		{
			if ((BOA_IsVisible(Viewer_object->roomnum,rp-Rooms))==0)
				return;
		}
	#endif*/

	//Set up rendering states
	rend_SetColorModel (CM_MONO);
	rend_SetLighting (LS_GOURAUD);
	rend_SetWrapType (WT_WRAP);

	if (rp->used==0)
	{	
		Int3(); // Trying to draw a room that isn't in use!
		return;
	}

	// Figure out pulse lighting for room
//	ComputeRoomPulseLight (rp);

	#if (defined(EDITOR) || defined(NEWEDITOR))
	if (!UseHardware)
		RenderRoomSorted(rp);
	else
	#endif
		//NOTE LINK TO ABOVE ELSE
		RenderRoomUnsorted(rp);

	rp->last_render_time=Gametime;
	rp->flags &=~RF_MIRROR_VISIBLE;
}




// This is needed for small view cameras
// It clears the facing array so that it is recomputed
void ResetFacings()
{
	memset (Facing_visited,0,sizeof(int)*(Highest_room_index+1));
}

// Marks all the faces facing us as drawable
void MarkFacingFaces (int roomnum,vector *world_verts)
{
	room *rp=&Rooms[roomnum];
	face *fp;
	vector tvec;

	if (Facing_visited[roomnum]==FrameCount)
		return;

	Facing_visited[roomnum]=FrameCount;
	
	fp=&rp->faces[0];

		for (int i=0;i<rp->num_faces;i++,fp++)
		{
			tvec = Viewer_eye - world_verts[fp->face_verts[0]];
			if ((tvec * fp->normal) <= 0)
				fp->flags |=FF_NOT_FACING;
		} 
}


// Prerotates all external room points and caches them
void RotateAllExternalRooms ()
{
	// Build the external room list if needed
	if (N_external_rooms==-1)
	{
		// Set up our z wall
		float zclip=5000*Matrix_scale.z; // (Detail_settings.Terrain_render_distance)*Matrix_scale.z;
		g3_SetFarClipZ (zclip);

		N_external_rooms=0;
		
		int i;
		for (i=0;i<=Highest_room_index;i++)
		{
			if ((Rooms[i].flags & RF_EXTERNAL) && Rooms[i].used)
			{
				External_room_list[N_external_rooms++]=i;
			}
		}

		ASSERT (N_external_rooms<MAX_EXTERNAL_ROOMS);		// Get Jason if hit this

		// Rotate all the points
		vector corners[8];
		for (i=0;i<N_external_rooms;i++)
		{
			int roomnum=External_room_list[i];
			room *rp=&Rooms[roomnum];
			MakePointsFromMinMax (corners,&rp->min_xyz,&rp->max_xyz);
			
			ubyte and=0xff;
			g3Point pnt;

			External_room_codes[i]=0xff;
			External_room_project_net[i]=0;
			bool behind=0;
			bool infront=0;
			for (int t=0;t<8;t++)
			{
				g3_RotatePoint(&pnt,&corners[t]);
				External_room_codes[i]&=pnt.p3_codes;

				if (pnt.p3_codes & CC_BEHIND)
					behind=true;
				else	
					infront=true;

				pnt.p3_codes &=~CC_BEHIND;
				g3_ProjectPoint (&pnt);
				External_room_corners[i][t].x=pnt.p3_sx;
				External_room_corners[i][t].y=pnt.p3_sy;
			}

			if (infront && behind)
			{
				External_room_codes[i]=0;
				External_room_project_net[i]=1;
			}
			else
			{
				if (behind)
				{
					External_room_codes[i]=CC_BEHIND;
				}
			}
		}
	}
}


// Returns true if a line intersects another line
inline bool LineIntersectsLine (g3Point *ls,g3Point *le,float x1,float y1,float x2,float y2)
{
	float num,denom;

	num=((ls->p3_sy-y1)*(x2-x1))-((ls->p3_sx-x1)*(y2-y1));
	denom=((le->p3_sx-ls->p3_sx)*(y2-y1))-((le->p3_sy-ls->p3_sy)*(x2-x1));

	float r=num/denom;

	if (r>=0.0 && r<=1.0)	
		return true;

	num=((ls->p3_sy-y1)*(le->p3_sx-ls->p3_sx))-((ls->p3_sx-x1)*(le->p3_sy-ls->p3_sy));
	denom=((le->p3_sx-ls->p3_sx)*(y2-y1))-((le->p3_sy-ls->p3_sy)*(x2-x1));

	float s=num/denom;

	if (s>=0.0 && s<=1.0)	
		return true;

	return false;
}

// Returns true if a face intersects the passed in portal in any way
// Returns true if a face intersects the passed in portal in any way
inline bool FaceIntersectsPortal (room *rp,face *fp,clip_wnd *wnd)
{
	g3Codes cc;
	int i;
	
	cc.cc_or=0;
	cc.cc_and=0xff;

	for (i=0;i<fp->num_verts;i++)
	{
		cc.cc_or|=Room_clips[fp->face_verts[i]];
		cc.cc_and&=Room_clips[fp->face_verts[i]];
	}

	if (cc.cc_and)
		return false;		// completely outside
	if (!cc.cc_or)
		return true;		// completely inside
					
	// Now we must do a check
	for (i=0;i<fp->num_verts;i++)
	{
		g3Point *p1=(g3Point *)&World_point_buffer[rp->wpb_index+fp->face_verts[i]];
		g3Point *p2=(g3Point *)&World_point_buffer[rp->wpb_index+fp->face_verts[(i+1)%fp->num_verts]];

		if (LineIntersectsLine (p1,p2,wnd->left,wnd->top,wnd->right,wnd->top))
			return true;
		if (LineIntersectsLine (p1,p2,wnd->right,wnd->top,wnd->right,wnd->bot))
			return true;
		if (LineIntersectsLine (p1,p2,wnd->right,wnd->bot,wnd->left,wnd->bot))
			return true;
		if (LineIntersectsLine (p1,p2,wnd->left,wnd->bot,wnd->left,wnd->top))
			return true;
	}

	return false;
}

// Returns true if the external room is visible from the passed in portal
int ExternalRoomVisibleFromPortal (int index,clip_wnd *wnd)
{
	int i;
	ubyte code=0xff;
	small_point pnt;
	// This is a stupid hack to prevent really large buildings from popping in and out of view
	if (External_room_project_net[index]) 
		return 1;
	for (i=0;i<8;i++)
	{
		pnt.p3_sx=External_room_corners[index][i].x;
		pnt.p3_sy=External_room_corners[index][i].y;
		pnt.p3_codes=0;
		code&=clip2d (&pnt,wnd); 
	}
	if (code)
		return 0;		// building can't be seen from this portal
	return 1;
}

// Checks to see what faces intersect the passed in portal
void MarkFacesForRendering (int roomnum,clip_wnd *wnd)
{
	room *rp=&Rooms[roomnum];

	int i;

	MarkFacingFaces(roomnum,rp->verts);

	// Rotate all the points in this room	
	if (rp->wpb_index==-1)
	{
		rp->wpb_index=Global_buffer_index;

			RotateRoomPoints (rp,rp->verts);

		Global_buffer_index+=rp->num_verts;
	}

	if (rp->flags & RF_DOOR)
	{
		for (i=0;i<rp->num_faces;i++)
			rp->faces[i].flags|=FF_VISIBLE;
	}
	else
	{
		// If this room contains a mirror, just mark all faces as visible
		// Else go through and figure out which ones are visible from the current portal
		if (rp->mirror_face==-1) // || !Detail_settings.Mirrored_surfaces)
		{
			// Do pointer dereferencing instead of array lookup for speed reasons
			small_point *pnt=&World_point_buffer[rp->wpb_index];

			for (i=0;i<rp->num_verts;i++,pnt++)
			{	
				Room_clips[i]=clip2d (pnt,wnd); 
			}		

			face *fp=&rp->faces[0];
			for (i=0;i<rp->num_faces;i++,fp++)
			{
				if (fp->flags & (FF_NOT_FACING|FF_VISIBLE))
					continue;			// this face is a backface
	
				if (FaceIntersectsPortal(rp,fp,wnd))
					fp->flags|=FF_VISIBLE;	
			}
		}
		else
		{
			if (rp->flags & RF_MIRROR_VISIBLE)	// If this room is already mirror, just return
				return;

			small_point *pnt=&World_point_buffer[rp->wpb_index];

			for (i=0;i<rp->num_verts;i++,pnt++)
			{	
				Room_clips[i]=clip2d (pnt,wnd); 
			}		

			face *fp=&rp->faces[rp->mirror_face];

			if (FaceIntersectsPortal(rp,fp,wnd))
			{
				rp->flags|=RF_MIRROR_VISIBLE;
//				Mirror_rooms[Num_mirror_rooms++]=roomnum;
			}
					
			if (rp->flags & RF_MIRROR_VISIBLE)	// Mirror is visible, just mark all faces as visible
			{
				fp=&rp->faces[0];
				for (i=0;i<rp->num_faces;i++,fp++)
				{
					fp->flags|=FF_VISIBLE;	
				}
			}
			else
			{
				fp=&rp->faces[0];
				for (i=0;i<rp->num_faces;i++,fp++)
				{
					if (fp->flags & (FF_NOT_FACING|FF_VISIBLE))
						continue;			// this face is a backface
	
					if (FaceIntersectsPortal(rp,fp,wnd))
						fp->flags|=FF_VISIBLE;	
				}
			}
		}
	}
	
	// Mark objects for rendering
	int objnum;
	for (objnum=rp->objects;(objnum!=-1);objnum=Objects[objnum].next) 
	{
		object *obj=&Objects[objnum];
		ubyte anded=0xff;
		g3Point pnts[8];
		vector vecs[8];
		ubyte code;

		if (rp->flags & RF_DOOR)	// Render all objects in a door room
		{
			obj->flags|=OF_SAFE_TO_RENDER;
			continue;
		}

		if (rp->flags & RF_MIRROR_VISIBLE)	// Render all objects if this mirror is visible
		{
			obj->flags|=OF_SAFE_TO_RENDER;
			continue;
		}
		
		MakePointsFromMinMax (vecs,&obj->min_xyz,&obj->max_xyz);

		for (i=0;i<8;i++)
		{
			g3_RotatePoint (&pnts[i],&vecs[i]);
			g3_ProjectPoint (&pnts[i]);
			code=clip2d ((small_point *)&pnts[i],wnd);
			anded&=code;

			if (pnts[i].p3_codes & CC_BEHIND)
				anded=0;
		}
				
		if (!anded)
		{
			// Object is visible
			obj->flags|=OF_SAFE_TO_RENDER;
		}
	}
	
}


#include "3d\clipper.h"


extern int GetFreePoints();

void BuildRoomListSub(int start_room_num,clip_wnd *wnd,int depth)
{
	room *rp = &Rooms[start_room_num];
	g3Point portal_points[MAX_VERTS_PER_FACE*5];
	int i,t;

	if (!NED_Render_all_rooms && !Render_one_room_only)
	{
		if (depth>=NED_Portal_depth)
			return;
		
	}

	if (N_render_rooms>=MAX_RENDER_ROOMS)
		return;

	if (Render_portals)
	{	
		rend_SetTextureType (TT_FLAT);
		rend_SetAlphaType (AT_CONSTANT);
		rend_SetAlphaValue (255);
		rend_SetFlatColor (GR_RGB(255,255,255));
		
		rend_DrawLine (wnd->left,wnd->top,wnd->right,wnd->top);
		rend_DrawLine (wnd->right,wnd->top,wnd->right,wnd->bot);
		rend_DrawLine (wnd->right,wnd->bot,wnd->left,wnd->bot);
		rend_DrawLine (wnd->left,wnd->bot,wnd->left,wnd->top);
	}

	if (!Rooms_visited[start_room_num])
	{
		Render_list[N_render_rooms++] = start_room_num;
	}
	
#ifdef _DEBUG
	Mine_depth++;
#endif

	Rooms_visited[start_room_num] = 1;
	Room_depth_list[start_room_num]=depth;

	//If this room is a closed (non-seethrough) door, don't check any of its portals, 
	//...UNLESS this is the first room we're looking at (meaning the viewer is in this room)
	if ((rp->flags & RF_DOOR) && (DoorwayGetPosition(rp) == 0.0) && !(Doors[rp->doorway_data->doornum].flags & DF_SEETHROUGH))
		if (depth != 0)
			return;

	//Check all the portals for this room
	for (t=0;t<rp->num_portals;t++) {
		portal *pp = &rp->portals[t];
		int croom = pp->croom;

		ASSERT (croom>=0);

		// If we are an external room portalizing into another external room, then skip!
		if ((rp->flags & RF_EXTERNAL) && (Rooms[croom].flags & RF_EXTERNAL))
			continue;

		//Check if we can see through this portal, and if not, skip it
		if (! RenderPastPortal(rp,pp))
			continue;

		// If this portal has been visited, skip it
		if (Room_depth_list[croom]<Room_depth_list[start_room_num])
			continue;

		//Deal with external portals differently
		int external_door_hack=0;
		if (rp->flags & RF_EXTERNAL && Rooms[croom].flags & RF_DOOR)
			external_door_hack=1;

		//Get pointer to this portal's face
		face *fp = &rp->faces[pp->portal_face];

		//See if portal is facing toward us
		if (! external_door_hack && !(pp->flags & PF_COMBINED)) {
			vector check_v;

			check_v = Viewer_eye - rp->verts[fp->face_verts[0]];

			if (check_v * fp->normal <= 0)		//not facing us
				continue;
		}
	
		g3Codes cc;
		cc.cc_or = 0; cc.cc_and = 0xff;
		int nv = fp->num_verts;

		//Code the face points
		// If this is a combined portal, then do that 
		if (pp->flags & PF_COMBINED)
		{
			// If this isn't the portal-combine master, then skip it
			if (pp->combine_master!=t)
				continue;

			int num_points=0;
			for (i=0;i<rp->num_portals;i++)
			{
				if (rp->portals[i].flags & PF_COMBINED && rp->portals[i].combine_master==t)
				{
					face *this_fp = &rp->faces[rp->portals[i].portal_face];

					vector check_v;
					check_v = Viewer_eye - rp->verts[this_fp->face_verts[0]];

					if (check_v * this_fp->normal <= 0)		//not facing us
						continue;
					
					g3Codes combine_cc;
					combine_cc.cc_or = 0; combine_cc.cc_and = 0xff;

					ASSERT ((num_points+this_fp->num_verts)<(MAX_VERTS_PER_FACE*5));
					
					// First we must rotate and clip this polygon
					for (int k=0;k<this_fp->num_verts;k++)
					{
						ubyte c=g3_RotatePoint(&portal_points[num_points+k],&rp->verts[this_fp->face_verts[k]]);

						combine_cc.cc_or|=c;
						combine_cc.cc_and&=c;
					}

					if (combine_cc.cc_and)
					{
						continue;	// clipped away!
					}
					else if (combine_cc.cc_or)
					{
						bool clipped = 0;
						g3Point *pointlist[MAX_VERTS_PER_FACE],**pl = pointlist;
						g3Point temp_points[MAX_VERTS_PER_FACE];
	
						for (k=0;k<this_fp->num_verts;k++)
						{
							pointlist[k] = &portal_points[num_points+k];
							ASSERT (!(pointlist[k]->p3_flags & PF_TEMP_POINT));
						}

						//If portal not all on screen, must clip it
						int combine_nv=this_fp->num_verts;

						pl = g3_ClipPolygon(pl,&combine_nv,&combine_cc);

						if (combine_cc.cc_and)	
						{
							g3_FreeTempPoints(pl,combine_nv);	
						}
						else
						{
							for (k=0;k<combine_nv;k++)
							{
								temp_points[k]=*pl[k];
								temp_points[k].p3_flags&=~PF_TEMP_POINT;
							}

							g3_FreeTempPoints(pl,combine_nv);

							for (k=0;k<combine_nv;k++)
								portal_points[k+num_points]=temp_points[k];
							
							num_points+=combine_nv;
						}
												
					}
					else	// No clipping needed, face is fully onscreen
						num_points+=this_fp->num_verts;
				}
			}

			if (num_points==0)
				continue;
		
			// Now, figure out a min/max for these points
			g3Point four_points[4];

			for (i=0;i<num_points;i++)
			{
				g3_ProjectPoint (&portal_points[i]);
			}

			int left,top,right,bottom;
			clip_wnd combine_wnd;
			combine_wnd.right = combine_wnd.bot = 0.0;
			combine_wnd.left = Render_width;
			combine_wnd.top = Render_height;

			//make new clip window
			for (i=0;i<num_points;i++) 
			{
				float x = portal_points[i].p3_sx, y = portal_points[i].p3_sy;
				if (x < combine_wnd.left)
				{
					combine_wnd.left = x;
					left=i;
				}

				if (x > combine_wnd.right)
				{
					combine_wnd.right = x;
					right=i;
				}

				if (y < combine_wnd.top)
				{
					combine_wnd.top = y;
					top=i;
				}

				if (y > combine_wnd.bot)
				{
					combine_wnd.bot = y;
					bottom=i;
				}
			}

			// Now harvest these points
			four_points[0]=portal_points[left];
			four_points[1]=portal_points[top];
			four_points[2]=portal_points[right];
			four_points[3]=portal_points[bottom];

			for (i=0;i<4;i++)
			{
				portal_points[i]=four_points[i];
				portal_points[i].p3_flags&=~(PF_PROJECTED|PF_TEMP_POINT);

				ubyte c = portal_points[i].p3_codes;
				cc.cc_and &= c;
				cc.cc_or  |= c;
			}

			nv=4;
		}
		else
		{
			for (i=0;i<nv;i++) 
			{
				g3_RotatePoint (&portal_points[i],&rp->verts[fp->face_verts[i]]);
				
				ubyte c = portal_points[i].p3_codes;
				cc.cc_and &= c;
				cc.cc_or  |= c;
			}
		}

		//If points are on screen, see if they're in the clip window
		if (cc.cc_and == 0 || external_door_hack) 
		{
			bool clipped = 0;
			g3Point *pointlist[MAX_VERTS_PER_FACE],**pl = pointlist;

			for (i=0;i<nv;i++)
				pointlist[i] = &portal_points[i];

			//If portal not all on screen, must clip it
			if (cc.cc_or) 
			{
				pl = g3_ClipPolygon(pl,&nv,&cc);
				clipped = 1;
			}

			cc.cc_and = 0xff;
			for (i=0;i<nv;i++) 
			{
				g3_ProjectPoint(pl[i]);
				cc.cc_and &= clip2d((small_point *)pl[i],wnd);
			}

			// Make sure it didn't get clipped away
			if (cc.cc_and == 0 || external_door_hack) 
			{
				clip_wnd new_wnd;
				new_wnd.right = new_wnd.bot = 0.0;
				new_wnd.left = Render_width;
				new_wnd.top = Render_height;

				//make new clip window
				for (i=0;i<nv;i++) 
				{
					float x = pl[i]->p3_sx, y = pl[i]->p3_sy;
					if (x < new_wnd.left)
						new_wnd.left = x;

					if (x > new_wnd.right)
						new_wnd.right = x;

					if (y < new_wnd.top)
						new_wnd.top = y;

					if (y > new_wnd.bot)
						new_wnd.bot = y;
				}

				//Combine the two windows
				new_wnd.left = __max(wnd->left,new_wnd.left);
				new_wnd.right = __min(wnd->right,new_wnd.right);
				new_wnd.top = __max(wnd->top,new_wnd.top);
				new_wnd.bot = __min(wnd->bot,new_wnd.bot);
				if (clipped) 
				{		//Free up temp points
					g3_FreeTempPoints(pl,nv);
					clipped = 0;
				}
				if (Rooms[croom].flags & RF_EXTERNAL)
				{
					if (!Called_from_terrain)
					{
						Must_render_terrain = 1;
						RotateAllExternalRooms();
						// For this external portal, we must check to see what external
						// rooms are visible from here
						for (i=0;i<N_external_rooms;i++)
						{
							if (External_room_codes[i])
							{
								continue;
							}
							if (External_room_list[i]!=croom)
							{
								// If this portal has been visited, skip it
								if (Room_depth_list[External_room_list[i]]<Room_depth_list[start_room_num])
								{
									continue;
								}
								if (!ExternalRoomVisibleFromPortal(i,&new_wnd))
								{
									continue;
								}
							}
							MarkFacesForRendering (External_room_list[i],&new_wnd);
							BuildRoomListSub(External_room_list[i],&new_wnd,depth+1);
							Room_depth_list[External_room_list[i]]=255;
						}
						//Combine the two windows
						Terrain_portal_left = __min(new_wnd.left,Terrain_portal_left);
						Terrain_portal_right = __max(new_wnd.right,Terrain_portal_right);
						Terrain_portal_top = __min(new_wnd.top,Terrain_portal_top);
						Terrain_portal_bottom = __max(new_wnd.bot,Terrain_portal_bottom);
					}
				}
				else 
				{
					MarkFacesForRendering (croom,&new_wnd);
					BuildRoomListSub(croom,&new_wnd,depth+1);
					Room_depth_list[croom]=255;
				}
			}
			if (clipped)		//Free up temp points
				g3_FreeTempPoints(pl,nv);
		}
	}
}


//build a list of rooms to be rendered
//fills in Render_list & N_render_rooms
void BuildRoomList(int start_room_num,bool render_all)
{
	clip_wnd wnd;
	room *rp=&Rooms[start_room_num];

	//For now, render all connected rooms

	for (int i=0;i<=Highest_room_index;i++)
	{
		Rooms_visited[i] = 0;
		Room_depth_list[i]=255;
		Rooms[i].wpb_index=-1;
	}

	#if (defined(EDITOR) || defined(NEWEDITOR))
	Rooms_visited[start_room_num] = 0;		//take care of rooms in the room palette
	Room_depth_list[start_room_num]=0;
	#endif

	N_external_rooms=-1;
	N_render_rooms = 0;
	Global_buffer_index=0;

	if (!render_all)
	{

	// Mark all the faces in our start room as renderable
	for (i=0;i<rp->num_faces;i++)
		rp->faces[i].flags|=FF_VISIBLE;
	
	MarkFacingFaces (start_room_num,rp->verts);

	// Get our points rotated, and update the global point list
	rp->wpb_index=Global_buffer_index;

		RotateRoomPoints (rp,rp->verts);

	Global_buffer_index+=rp->num_verts;
	
	// Mark all objects in this room as visible
	for (int objnum=rp->objects;(objnum!=-1);objnum=Objects[objnum].next) 
		Objects[objnum].flags|=OF_SAFE_TO_RENDER;
	
	
	//Initial clip window is whole screen
	wnd.left = wnd.top = 0.0;
	wnd.right = Render_width;
	wnd.bot = Render_height;

	BuildRoomListSub(start_room_num,&wnd,0);

	}
	else
	{

	// Mark all faces in all rooms as renderable
	for (i=0; i<=Highest_room_index; i++)
	{
		rp = &Rooms[i];
		if (!rp->used)
			continue;
		for (int j=0; j<rp->num_faces; j++)
			rp->faces[j].flags |= FF_VISIBLE;

		MarkFacingFaces(i,rp->verts);

		// Get our points rotated, and update the global point list
		rp->wpb_index=Global_buffer_index;

			RotateRoomPoints (rp,rp->verts);

		Global_buffer_index+=rp->num_verts;
	
		// Mark all objects in this room as visible
		for (int objnum=rp->objects;(objnum!=-1);objnum=Objects[objnum].next) 
			Objects[objnum].flags|=OF_SAFE_TO_RENDER;
	
	
		//Initial clip window is whole screen
		wnd.left = wnd.top = 0.0;
		wnd.right = Render_width;
		wnd.bot = Render_height;

		BuildRoomListSub(i,&wnd,i);

	}

	} // end if (render_all)

	//mprintf((0,"N_render_rooms = %d ",N_render_rooms));

	#ifdef EDITOR
	//Add all external rooms to render list if that flag set
	if (Editor_view_mode==VM_MINE && In_editor_mode)
	{
		if (Render_all_external_rooms) {
			int i;
			room *rp;

			for (i=0,rp=Rooms;i<=Highest_room_index;i++,rp++) {
				if (rp->used && (rp->flags & RF_EXTERNAL))
				{
					for (int t=0;t<rp->num_faces;t++)
						rp->faces[t].flags|=FF_VISIBLE;
					MarkFacingFaces (i,rp->verts);
		
					if (!Rooms_visited[i])
						Render_list[N_render_rooms++] = i;
					Rooms_visited[i]=1;
				}
			}
		}
	}
	#endif
}


#define MAX_OBJECTS_PER_ROOM 2000

typedef struct 
{
	int	vis_effect;
	int	objnum;
	float	dist;
} obj_sort_item;

obj_sort_item obj_sort_list[MAX_OBJECTS_PER_ROOM];

//Compare function for room face sort
static int obj_sort_func(const obj_sort_item *a, const obj_sort_item *b)
{
	if (a->dist < b->dist)
		return -1;
	else if (a->dist > b->dist)
		return 1;
	else
		return 0;
}


//Render the objects and viseffects in a room.  Do a simple sort
void RenderRoomObjects(room *rp)
{
	int n_objs=0,objnum,i,visnum;
	float zdist;

	if (!Render_mirror_for_room && UseHardware)
		return;	// This function only works for mirrors now

	
	//Add objects to sort list
	for (objnum=rp->objects;(objnum!=-1) && (n_objs < MAX_OBJECTS_PER_ROOM);objnum=Objects[objnum].next) 
	{
		ASSERT (objnum!=Objects[objnum].next);

		object *obj = &Objects[objnum];
		
		if (obj->render_type == RT_NONE)
			continue;

#ifndef NEWEDITOR
		if (obj==Viewer_object && !Render_mirror_for_room)
			continue;
#endif

		float size = obj->size;

		// Special case weapons with streamers
		if (obj->type==OBJ_WEAPON && (Weapons[obj->id].flags & WF_STREAMER))
			size=Weapons[obj->id].phys_info.velocity.z;

		// Check if object is trivially rejected
		if (Render_mirror_for_room || (obj->type==OBJ_WEAPON && Weapons[obj->id].flags & WF_ELECTRICAL) || IsPointVisible (&obj->pos,size,&zdist))
		{
			obj_sort_list[n_objs].vis_effect=0;
			obj_sort_list[n_objs].objnum = objnum;
			obj_sort_list[n_objs].dist = zdist;
			n_objs++;
		}
	}

	//Add vis effects to sort list
	for (visnum=rp->vis_effects;(visnum!=-1) && (n_objs < MAX_OBJECTS_PER_ROOM);visnum=VisEffects[visnum].next) 
	{
		ASSERT (visnum!=VisEffects[visnum].next);

		if (VisEffects[visnum].type==VIS_NONE || VisEffects[visnum].flags & VF_DEAD)
			continue;

		if (IsPointVisible (&VisEffects[visnum].pos,VisEffects[visnum].size,&zdist))
		{
			obj_sort_list[n_objs].vis_effect=1;
			obj_sort_list[n_objs].objnum = visnum;
			obj_sort_list[n_objs].dist = zdist;
			n_objs++;
		}
	}

	ASSERT(objnum == -1);	//if not -1, ran out of space in render_order[]
	ASSERT(visnum == -1);	//if not -1, ran out of space in render_order[]

	//Sort the objects
	qsort(obj_sort_list,n_objs,sizeof(*obj_sort_list),(int (*)(const void*,const void*))obj_sort_func);

	#if (defined(_DEBUG) || defined(NEWEDITOR))
	bool save_polymodel_outline_mode = Polymodel_outline_mode;
	Polymodel_outline_mode = OUTLINE_ON(OM_OBJECTS);
	#endif

	//Render the objects

	for (i=n_objs-1;i>=0;i--) 
	{
		objnum = obj_sort_list[i].objnum;

		if (obj_sort_list[i].vis_effect) 
		{
			DrawVisEffect (&VisEffects[objnum]);
		}
		else 
		{
			object *objp = &Objects[objnum];

#ifndef NEWEDITOR
			if (objp==Viewer_object)
				continue;
#endif

			RenderObject(objp);
		}
	}

	#if (defined(_DEBUG) || defined(NEWEDITOR))
	Polymodel_outline_mode = save_polymodel_outline_mode;
	#endif
}


//Draws the mine, starting at a the specified room
//The rendering surface must be set up, and g3_StartFrame() must have been called
//Parameters:	viewer_roomnum - what room the viewer is in
//					flag_automap - if true, flag segments as visited when rendered
//					called_from_terrain - set if calling this routine from the terrain renderer
void RenderMine(int viewer_roomnum,int flag_automap,int called_from_terrain,bool render_all,bool outline,bool flat,prim *prim)
{
#ifdef EDITOR
	In_editor_mode = (GetFunctionMode() == EDITOR_MODE) ;
#endif

	if (outline)
	{
		Outline_mode |= OM_ON;
		Outline_mode |= OM_ALL;
	}
	else
	{
		Outline_mode &= ~OM_ON;
		Outline_mode &= ~OM_ALL;
	}

	if (flat)
		Render_flat_shaded = 1;
	else
		Render_flat_shaded = 0;

	//Get the viewer eye so functions down the line can look at it
	g3_GetViewPosition(&Viewer_eye);
	g3_GetUnscaledMatrix (&Viewer_orient);

	//set these globals so functions down the line can look at them
	Viewer_roomnum		= viewer_roomnum;
//	Flag_automap		= flag_automap;
	Called_from_terrain = called_from_terrain;

	//Assume no terrain
	Must_render_terrain = 0;
	
	//Get the width & height of the render window
	rend_GetProjectionParameters(&Render_width,&Render_height);

	if (!Called_from_terrain)
	{
		Terrain_portal_top=Render_height;
		Terrain_portal_bottom=0;
		Terrain_portal_right=0;
		Terrain_portal_left=Render_width;
	}


	//Build the list of visible rooms
	BuildRoomList(viewer_roomnum,render_all);		//fills in Render_list & N_render_segs

	//If we determined that the terrain is visible, render it
	if (!Dont_draw_terrain)
	{
	if (Must_render_terrain && !Called_from_terrain && !(In_editor_mode && Render_inside_only))
	{
	  	RenderTerrain(1,Terrain_portal_left,Terrain_portal_top,Terrain_portal_right,Terrain_portal_bottom);

		// Mark all room points to be rerotated due to terrain trashing our point list
		for (int i=0;i<=Highest_room_index;i++)
		{
			Rooms[i].wpb_index=-1;
			Global_buffer_index=0;
		}


		g3_SetFarClipZ (VisibleTerrainZ);
		
		rend_SetZValues(0,5000);
		
	}
	}

	//Render the list of rooms
	for (int nn=N_render_rooms-1;nn>=0;nn--) 
	{
		int roomnum;

		roomnum = Render_list[nn];

		#if (defined(_DEBUG) || defined(NEWEDITOR))
		if (In_editor_mode && (Render_one_room_only || !render_all) && (roomnum != viewer_roomnum))
			continue;
		#endif

		if (roomnum!=-1) 
		{

			ASSERT (Rooms_visited[roomnum]!=255);	
			RenderRoom(&Rooms[roomnum]);
			Rooms_visited[roomnum] = (char) 255;

			// Render objects for NEWEDITOR
			RenderRoomObjects (&Rooms[roomnum]);
/*
			// Stuff objects into our postrender list
			CheckToRenderMineObjects (roomnum);
*/
		}
	}

	rend_SetOverlayType(OT_NONE);	// turn off lightmap blending

	#if (defined(EDITOR) || defined(NEWEDITOR))
	if (OUTLINE_ON(OM_MINE)) {
		if (prim->roomp != NULL && prim->face != -1 && prim->edge != -1 && prim->vert != -1)
			OutlineCurrentFace(prim->roomp,prim->face,prim->edge,prim->vert,CURFACE_COLOR,CUREDGE_COLOR);
		if (Markedroomp)
			OutlineCurrentFace(Markedroomp,Markedface,Markededge,Markedvert,MARKEDFACE_COLOR,MARKEDEDGE_COLOR);
		if (Placed_room != -1)
			DrawPlacedRoomFace(&Rooms[Placed_room],&Placed_room_origin,&Placed_room_rotmat,&Placed_room_attachpoint,Placed_room_face,PLACED_COLOR);
	}
	#endif
}


#define STATE_PUSH(val) {state_stack[state_stack_counter]=val; state_stack_counter++; ASSERT (state_stack_counter<2000);}
#define STATE_POP()	{state_stack_counter--; pop_val=state_stack[state_stack_counter];}
// Sorts our texture states using the quicksort algorithm
void SortStates (state_limited_element *state_array,int cellcount)
{
	state_limited_element v,t;
	int pop_val;
	int i,j;
	int l,r;
	l=0;
	r=cellcount-1;

	ushort state_stack_counter=0;
	ushort state_stack[2000];
	

	while (1)
	{
		while (r>l)
		{
			i=l-1;
			j=r;
			v=state_array[r];
			while (1)
			{
				while (state_array[++i].sort_key < v.sort_key)
					;

				while (state_array[--j].sort_key > v.sort_key)
					;

				if (i>=j)
					break;

				t=state_array[i];
				state_array[i]=state_array[j];
				state_array[j]=t;
			}

			t=state_array[i];
			state_array[i]=state_array[r];
			state_array[r]=t;
			
			if (i-l > r-i)
			{
				STATE_PUSH (l);
				STATE_PUSH (i-1);
				l=i+1;
			}
			else
			{
				STATE_PUSH (i+1);
				STATE_PUSH (r);
				r=i-1;
			}
		}

		if (!state_stack_counter)
			break;
		STATE_POP ();
		r=pop_val;
		STATE_POP ();
		l=pop_val;
	}	
}


// Builds a list of mirror faces for each room and allocs memory accordingly
void ConsolidateMineMirrors()
{
	int i,t;
	mprintf ((0,"Consolidating mine mirrors!\n"));

	for (i=0;i<MAX_ROOMS;i++)
	{
		room *rp=&Rooms[i];
		if (!rp->used)
			continue;

		if (rp->mirror_faces_list)
		{
			mem_free(rp->mirror_faces_list);
			rp->mirror_faces_list=NULL;
			rp->num_mirror_faces=0;
		}

		if (rp->mirror_face==-1)
			continue;

		// Count the number of faces that have the same texture as the mirror face
		int num_mirror_faces=0;
		for (t=0;t<rp->num_faces;t++)
		{
			face *fp=&rp->faces[t];
			if (fp->tmap==rp->faces[rp->mirror_face].tmap)
				num_mirror_faces++;
		}

		if (num_mirror_faces==0)
		{
			// No faces found?  Weird.
			rp->mirror_face=0;
			continue;
		}

		rp->mirror_faces_list=(ushort *)mem_malloc (num_mirror_faces*sizeof(ushort));
		ASSERT (rp->mirror_faces_list);
		rp->num_mirror_faces=num_mirror_faces;

		// Now go through and fill in our list
		int count=0;
		for (t=0;t<rp->num_faces;t++)
		{
			face *fp=&rp->faces[t];
			if (fp->tmap==rp->faces[rp->mirror_face].tmap)
				rp->mirror_faces_list[count++]=t;
		}
	}
} 


/************** stubs **************/

// Render a fog layer on top of a face
void RenderFogFaces (room *rp)
{
}

void RenderSpecularFacesFlat(room *rp)
{
}

// Renders all the lights glows for this frame
void RenderLightGlows ()
{
}

// Sets up fog if this room is fogged
void SetupRoomFog (room *rp,vector *eye,matrix *orient,int viewer_room)
{
}

// from Main\renderobject.cpp

#define RO_STATIC		0
#define RO_GOURAUD	1
#define RO_LIGHTMAPS	2

ubyte RenderObjectType=RO_STATIC;
float RenderObjectStaticRedValue=1.0f;
float RenderObjectStaticGreenValue=1.0f;
float RenderObjectStaticBlueValue=1.0f;
float RenderObjectStaticScalar=1.0f;

#if (defined(EDITOR) || defined(NEWEDITOR))
//Draws the little corner brackets around the selected object
//Actually, only draws those either in front or in back of the object, based on front_flag
void DrawObjectSelectionBrackets(object *obj,bool front_flag)
{
	vector viewvec;
	poly_model *pm;
	vector mins,maxs;
	float line_len;

	// This is only until I implement ship paging -G
	if (obj->type != OBJ_PLAYER)
	{
		pm = &Poly_models[obj->rtype.pobj_info.model_num];
		mins.x = pm->mins.x; mins.y = pm->mins.y; mins.z = pm->mins.z; 
		maxs.x = pm->maxs.x; maxs.y = pm->maxs.y; maxs.z = pm->maxs.z; 
	}
	else
	{
		mins.x = mins.y = mins.z = -obj->size;
		maxs.x = maxs.y = maxs.z = obj->size;
	}

	//Get vector from object to viewer
	g3_GetViewPosition(&viewvec);
	viewvec -= obj->pos;

	//Get length of line segments we're drawing
	line_len = (maxs.x - mins.x) * 0.2f;

	//Do each corner
	for (int c=0;c<8;c++) {
		vector corner;

		//Get the corner relative to the object
		corner = (obj->orient.rvec * ((c&1) ? mins.x : maxs.x)) + 
					(obj->orient.uvec * ((c&2) ? mins.y : maxs.y)) + 
					(obj->orient.fvec * ((c&4) ? mins.z : maxs.z));

		//See if this corner is in front or in back of the object, as specified 
		if (((corner * viewvec) > 0.0) != front_flag)
			continue;

		//Get absolute position in 3-space
		corner += obj->pos;

		//Draw line for each axis at this corner
		for (int a=0;a<3;a++) {
			g3Point pp0,pp1;
			vector t;

			//Grab the x,y, or z axis, and scale by the line segment length
			t = corner + ((((vector *) &obj->orient)[a]) * ((c&(1<<a)) ? line_len : -line_len));

			//Rotate both ends of the line
			g3_RotatePoint(&pp0,&corner);
			g3_RotatePoint(&pp1,&t);

			//Draw!
		   g3_DrawLine(GR_RGB(255,255,255),&pp0,&pp1);
		}
	}
}

void DrawNumber (int num,vector pos,float size);

void DrawObjectGunpoints(object *obj)
{
	poly_model *pm;
	vector mins,maxs;
	float line_len;

	// This is only until I implement ship paging -G
	if (obj->type == OBJ_PLAYER)
		return;

	pm = &Poly_models[obj->rtype.pobj_info.model_num];
	mins.x = pm->mins.x; mins.y = pm->mins.y; mins.z = pm->mins.z; 
	maxs.x = pm->maxs.x; maxs.y = pm->maxs.y; maxs.z = pm->maxs.z; 

	//Get length of line segments we're drawing
	line_len = (maxs.x - mins.x);

	//Do each gunpoint
	vector start_vec,end_vec;
	vector normal;
	g3Point start,end;
	for (int i=0; i<pm->n_guns; i++)
	{
		WeaponCalcGun(&start_vec, &normal, obj, i);
		end_vec = start_vec + line_len*normal;
		g3_RotatePoint(&start,&start_vec);
		g3_RotatePoint(&end,&end_vec);

		//Draw!
		g3_DrawLine(GR_RGB(255,128,0),&start,&end);
		g3_DrawSphere(GR_RGB(255,128,0), &end, 0.05f*line_len);
		DrawNumber(i,end_vec,0.05f*line_len);
	}
}

#endif

// Sets the polygon render object type to static (one lightval for whole object)
void RenderObject_SetStatic (float r,float g,float b)
{
	RenderObjectType=RO_STATIC;
	RenderObjectStaticRedValue=r;
	RenderObjectStaticGreenValue=g;
	RenderObjectStaticBlueValue=b;
	RenderObjectStaticScalar=1.0;
}

// Actually draws a polygon model based on the light parameters set by the above
// functions
void RenderObject_DrawPolymodel (object *obj,float *normalized_times)
{
	int model_num;
	int use_effect=0;
	vector obj_pos=obj->pos;
	polymodel_effect pe={0};

/*
	// Lots of eye candy stuff taken out for NEWEDITOR
*/

	// Pick a lod model to use if eligible
	if (obj->lighting_render_type==LRT_STATIC || obj->lighting_render_type==LRT_GOURAUD)
	{
		if (obj->type==OBJ_POWERUP || obj->type==OBJ_ROBOT || obj->type==OBJ_CLUTTER)
		{
			g3Point pnt;
			float detail_scalar=1.0;

			g3_RotatePoint (&pnt,&obj->pos);

/*
			if (Detail_settings.Object_complexity==0)
				detail_scalar=.6f;
			else if (Detail_settings.Object_complexity==2)
				detail_scalar=1.2f;
*/

//			if (pnt.p3_z<(Object_info[obj->id].med_lod_distance*detail_scalar))
				model_num=obj->rtype.pobj_info.model_num;
/*
			else if (pnt.p3_z<(Object_info[obj->id].lo_lod_distance*detail_scalar))
			{
				if (Object_info[obj->id].med_render_handle!=-1)
					model_num=Object_info[obj->id].med_render_handle;
				else
				{
					model_num=obj->rtype.pobj_info.model_num;
				}
			}
			else
			{
				if (Object_info[obj->id].lo_render_handle!=-1)
					model_num=Object_info[obj->id].lo_render_handle;
				else
				{
					model_num=obj->rtype.pobj_info.model_num;
					if (Object_info[obj->id].med_render_handle!=-1)
						model_num=Object_info[obj->id].med_render_handle;
					else
						model_num=obj->rtype.pobj_info.model_num;

				}
			}
		}
		else if (obj->type==OBJ_MARKER)
		{
			model_num=Marker_polynum;
		}
		else if (obj->type==OBJ_PLAYER && !(Players[obj->id].flags & (PLAYER_FLAGS_DYING|PLAYER_FLAGS_DEAD)))
		{
			g3Point pnt;

			g3_RotatePoint (&pnt,&obj->pos);
			int ship_num=Players[obj->id].ship_index;
			float detail_scalar=1.0;

			if (Detail_settings.Object_complexity==0)
				detail_scalar=.6f;
			else if (Detail_settings.Object_complexity==2)
				detail_scalar=1.2f;

			if (pnt.p3_z<(Ships[ship_num].med_lod_distance * detail_scalar))
				model_num=obj->rtype.pobj_info.model_num;
			else if (pnt.p3_z<(Ships[ship_num].lo_lod_distance * detail_scalar))
			{
				if (Ships[ship_num].med_render_handle!=-1)
					model_num=Ships[ship_num].med_render_handle;
				else
				{
					model_num=obj->rtype.pobj_info.model_num;
				}
			}
			else
			{
				if (Ships[ship_num].lo_render_handle!=-1)
					model_num=Ships[ship_num].lo_render_handle;
				else
				{
					model_num=obj->rtype.pobj_info.model_num;
					if (Ships[ship_num].med_render_handle!=-1)
						model_num=Ships[ship_num].med_render_handle;
					else
						model_num=obj->rtype.pobj_info.model_num;
				}
			}
*/
		}
		else
			model_num=obj->rtype.pobj_info.model_num;
	}
	else
		model_num=obj->rtype.pobj_info.model_num;

/*	if (obj->type==OBJ_BUILDING && obj->flags & OF_USE_DESTROYED_POLYMODEL)
	{
		if (Object_info[obj->id].lo_render_handle!=-1)
			model_num=Object_info[obj->id].lo_render_handle;
	}
*/

	if (RenderObjectType==RO_STATIC)
	{
		// Draw this object with static light
		int overlay = 0;

		if((obj->type == OBJ_ROBOT || obj->type==OBJ_PLAYER) && obj->rtype.pobj_info.subobj_flags != 0xFFFFFFFF)
			overlay = 1;
		
		DrawPolygonModel (&obj_pos,&obj->orient,model_num,normalized_times,0, 
								RenderObjectStaticRedValue, RenderObjectStaticGreenValue,RenderObjectStaticBlueValue,
								obj->rtype.pobj_info.subobj_flags,use_effect, overlay);
	}
/*
	else if (RenderObjectType==RO_GOURAUD || NoLightmaps)
	{
		// Draw this object with gouraud static light
		int overlay = 0;

		if((obj->type == OBJ_ROBOT || obj->type==OBJ_PLAYER) && obj->rtype.pobj_info.subobj_flags != 0xFFFFFFFF)
			overlay = 1;

		DrawPolygonModel (&obj_pos,&obj->orient,model_num,normalized_times,0, &RenderObject_LightDirection,
								 RenderObjectStaticRedValue, RenderObjectStaticGreenValue,RenderObjectStaticBlueValue,
								 obj->rtype.pobj_info.subobj_flags,use_effect,overlay);
	}
	else if (RenderObjectType==RO_LIGHTMAPS)
	{
		int overlay = 0;

		if((obj->type == OBJ_ROBOT || obj->type==OBJ_PLAYER) && obj->rtype.pobj_info.subobj_flags != 0xFFFFFFFF)
			overlay = 1;

		// If this object is a destroyed building then do something different with it
		DrawPolygonModel (&obj_pos,&obj->orient,model_num,normalized_times,0, RenderObjectLightmapObject, obj->rtype.pobj_info.subobj_flags,use_effect, overlay);
	}
*/
	else
		Int3();	// Get Jason

}

void DrawObjectSelectionBrackets(object *obj,bool front_flag);

// Draw object, with orientation vectors
void DrawObject(ddgr_color color,object *obj,bool bDrawOrient);
#define	PLAYER_COLOR		GR_RGB(   0, 255,   0)		//a player object

// -----------------------------------------------------------------------------
//	Render an object.  Calls one of several routines based on type
void RenderObject(object *obj)
{
//	float normalized_time[MAX_SUBOBJECTS];
	bool render_it=false;

	if ( obj->type==OBJ_NONE )	{
		mprintf( (1, "ERROR!!!! Bogus obj %d in room %d is rendering!\n", OBJNUM(obj), obj->roomnum ));
		Int3();
		return;
	}

	if ( obj->type==OBJ_DUMMY )
		return;

	if (obj->flags & OF_ATTACHED)
	{
		// See if we should be rendered, because our attach parent might be invisible
		object *parent_obj=ObjGet (obj->attach_ultimate_handle);

		if (!parent_obj)
			return;

		if (parent_obj->render_type==RT_NONE && parent_obj->type != OBJ_POWERUP && (parent_obj->type==OBJ_PLAYER || parent_obj->movement_type != MT_NONE))
			return;
	}

#ifndef NEWEDITOR
	if (OBJECT_OUTSIDE(obj))
		render_it=SetupTerrainObject (obj);
	else
		render_it=SetupMineObject (obj);

	if (!render_it)
		return;
#else
	if (OBJECT_OUTSIDE(obj))
		obj->flags|=OF_SAFE_TO_RENDER;
	RenderObject_SetStatic (1.0f,1.0f,1.0f);
#endif

	if (!(obj->flags & OF_SAFE_TO_RENDER))
		return;

	// Mark this a rendered this frame
	obj->flags|=OF_RENDERED;

	// If we're not rendering from a mirror, mark this object as rendered
	if (Render_mirror_for_room==false)
		obj->flags&=~OF_SAFE_TO_RENDER;

	obj->renderframe=FrameCount % 65536;

/*
	if(obj->control_type == CT_AI &&
		(GetFunctionMode() == GAME_MODE || GetFunctionMode() == EDITOR_GAME_MODE)) 
	{
		AI_RenderedList[AI_NumRendered] = OBJNUM(obj);
		AI_NumRendered += 1;
	}
*/

	#if (defined(EDITOR) || defined(NEWEDITOR))
	ddgr_color oldcolor;
		if (TSearch_on && obj->type!=OBJ_ROOM)
		{
  	  		rend_SetPixel(GR_RGB(16,255,64),TSearch_x,TSearch_y);
			oldcolor = rend_GetPixel(TSearch_x,TSearch_y);			//will be different in 15/16-bit color
		}
	#endif

	switch (obj->render_type) {

		case RT_NONE:
			break;

		case RT_EDITOR_SPHERE:		//to render objects in editor mode
			#if (defined(EDITOR) || defined(NEWEDITOR))
			if ((GetFunctionMode() == EDITOR_MODE)) {		//only render if in editor mode

				if (!UseHardware)
				{
					g3Point sphere_point;
					g3_RotatePoint(&sphere_point,&obj->pos);
					g3_DrawSphere(obj->rtype.sphere_color,&sphere_point,obj->size);
				}
/*
				else
				{
					//Let me take this opportunity to say how much it pisses me off that 
					//the DrawColoredDisk() function takes r,g,b as floats, when the standard
					//in our graphics system is to pass color as a ddgr_color
					float r = (float) GR_COLOR_RED(obj->rtype.sphere_color) / 255.0,
							g = (float) GR_COLOR_GREEN(obj->rtype.sphere_color) / 255.0,
							b = (float) GR_COLOR_BLUE(obj->rtype.sphere_color) / 255.0;
					DrawColoredDisk (&obj->pos,r,g,b,1,1,obj->size,0);
				}
*/
			}
			#endif
			break;

		case RT_POLYOBJ:

			#if (defined(EDITOR) || defined(NEWEDITOR))
			if ((GetFunctionMode() == EDITOR_MODE) && (obj-Objects == Cur_object_index))
			{
				DrawObjectSelectionBrackets(obj,0);		//draw back brackets
				DrawObjectGunpoints(obj);
			}
			#endif

			// This is only until I implement ship paging -G
			if (obj->type == OBJ_PLAYER)
			{
				if (!UseHardware)
					DrawObject(PLAYER_COLOR,obj,true);
/*
				else
				{
					//Let me take this opportunity to say how much it pisses me off that 
					//the DrawColoredDisk() function takes r,g,b as floats, when the standard
					//in our graphics system is to pass color as a ddgr_color
					float r = (float) GR_COLOR_RED(obj->rtype.sphere_color) / 255.0,
							g = (float) GR_COLOR_GREEN(obj->rtype.sphere_color) / 255.0,
							b = (float) GR_COLOR_BLUE(obj->rtype.sphere_color) / 255.0;
					DrawColoredDisk (&obj->pos,r,g,b,1,1,obj->size,0);
				}
*/
			}
/*
			if(obj->rtype.pobj_info.anim_frame || (Poly_models[obj->rtype.pobj_info.model_num].frame_max != Poly_models[obj->rtype.pobj_info.model_num].frame_min))
			{
				SetNormalizedTimeObj(obj, normalized_time);
				RenderObject_DrawPolymodel (obj,normalized_time);
			}
			else
			{
*/
			if (obj->type != OBJ_PLAYER)
				RenderObject_DrawPolymodel (obj,NULL);
//			}

/*
			#ifdef _DEBUG
				if(Game_show_sphere)
				{	
					DrawDebugInfo(obj);
				}
			#endif
*/

/*
			// Render that powerup glow
			if (obj->type==OBJ_POWERUP)
				DrawPowerupGlowDisk (obj);
			if (obj->type==OBJ_PLAYER)
			{				
				DrawPlayerDamageDisk (obj);
				DrawPlayerRotatingBall (obj);
				DrawPlayerNameOnHud (obj);
				DrawPlayerTypingIndicator (obj);
				DrawPlayerInvulSphere (obj);
			}
			if (obj->type==OBJ_PLAYER || obj->type==OBJ_ROBOT || (obj->type == OBJ_BUILDING && obj->ai_info))
				DrawSparkyDamageLightning (obj);
*/
			break;
		case RT_FIREBALL: 
/*
			DrawFireballObject(obj); 
*/
			break;

		case RT_WEAPON:  
/*
			DrawWeaponObject(obj); 
			
			#ifdef _DEBUG
				if(Game_show_sphere)
				{	
					DrawDebugInfo(obj);
				}
			#endif
*/

			break;
		case RT_SPLINTER:
//			DrawSplinterObject (obj);
			break;

		case RT_SHARD:
//			DrawShardObject(obj);
			break;

		#ifdef _DEBUG
		case RT_LINE: 
			{
				g3Point g3p[2];
				memset(g3p, 0, 2 * sizeof(g3Point));

				//g3p[0].p3_vec = obj->pos;
				//g3p[1].p3_vec = obj->rtype.line_info.end_pos;
				
				g3_RotatePoint(&g3p[0], &obj->pos);
				g3_RotatePoint(&g3p[1], &obj->rtype.line_info.end_pos);

				g3_DrawLine(GR_RGB(255,255,255),&g3p[0],&g3p[1]);
				break;
			}
		#endif

		default: Error("Unknown render_type <%d>",obj->render_type);
 	}

	#ifdef NEWDEMO
	if ( obj->render_type != RT_NONE )
		if ( Newdemo_state == ND_STATE_RECORDING ) {
			if (!WasRecorded[obj-Objects]) {     
				newdemo_record_RenderObject(obj);
				WasRecorded[obj-Objects]=1;
			}
		}
	#endif

//??	Max_linear_depth = mld_save;

	//Mark selected objects
	#if (defined(EDITOR) || defined(NEWEDITOR))
	if ((GetFunctionMode() == EDITOR_MODE) && (obj-Objects == Cur_object_index)) {
		if (obj->render_type != RT_POLYOBJ) {
			g3Point pnt;
			g3_RotatePoint(&pnt,&obj->pos);
			g3_DrawBox(GR_RGB(255,255,255),&pnt,obj->size);
		}
		else {		//polygon model
			DrawObjectSelectionBrackets(obj,1);		//draw front brackets
		}
	}
	#endif	//ifdef EDITOR


	#if (defined(EDITOR) || defined(NEWEDITOR))
		if (TSearch_on)
		{
			if (rend_GetPixel(TSearch_x,TSearch_y) != oldcolor) 
			{
				TSearch_found_type=TSEARCH_FOUND_OBJECT;
				TSearch_seg = obj-Objects;
				mprintf ((0,"TR:objnum=%d\n",obj-Objects));
			}
		}
	#endif


}

extern float Far_clip_z;
int Point_visible_last_frame=-1;

// Given a position in 3space and a size, returns whether or not that sized point is
// visible from the current view matrix
int IsPointVisible (vector *pos,float size,float *pointz)
{
	g3Point pnt;
	ubyte ccode;
	static float last_render_fov=-1;
	static vector left_normal,right_normal,top_normal,bottom_normal,view_position;
	static matrix unscaled_matrix;
	
	g3_RotatePoint(&pnt,pos);
	ccode=g3_CodePoint (&pnt);

	if (pointz!=NULL)
		*pointz=pnt.p3_z;

	if (ccode)	// the center point is off, find out if the whole object is off
	{
		float dotp;
		
		if (pnt.p3_z<-size)
			return 0;			// behind plane!

		if (pnt.p3_z>Far_clip_z+size)
			return 0;		   // too far away!

		if (Render_FOV!=last_render_fov)
		{
			last_render_fov=Render_FOV;
			int angle_adjust=(Render_FOV/2)*(65536/360);

			vector rvec={1,0,0};
			vector lvec={-1,0,0};
			vector tvec={0,1,0};
			vector bvec={0,-1,0};

			matrix temp_mat;

			vm_AnglesToMatrix (&temp_mat,0,65536-angle_adjust,0);
			right_normal=rvec*temp_mat;

			vm_AnglesToMatrix (&temp_mat,0,angle_adjust,0);
			left_normal=lvec*temp_mat;

			vm_AnglesToMatrix (&temp_mat,65536-angle_adjust,0,0);
			bottom_normal=bvec*temp_mat;

			vm_AnglesToMatrix (&temp_mat,angle_adjust,0,0);
			top_normal=tvec*temp_mat;
		}

		// Get unscaled matrix stuff
		if (Point_visible_last_frame!=FrameCount)
		{
			Point_visible_last_frame=FrameCount;	
			g3_GetUnscaledMatrix (&unscaled_matrix);
			g3_GetViewPosition (&view_position);
		}

		vector temp_vec = *pos - view_position;
		pnt.p3_vec = temp_vec * unscaled_matrix;		


		if (ccode & CC_OFF_RIGHT)
		{
			dotp=vm_DotProduct (&right_normal,&pnt.p3_vec);
			if (dotp>size)
				return 0;
		}
		if (ccode & CC_OFF_LEFT)
		{
			dotp=vm_DotProduct (&left_normal,&pnt.p3_vec);
			if (dotp>size)
				return 0;
		}
					
		if (ccode & CC_OFF_TOP)
		{
			dotp=vm_DotProduct (&top_normal,&pnt.p3_vec);
			if (dotp>size)
				return 0;
		}

		if (ccode & CC_OFF_BOT)
		{
			dotp=vm_DotProduct (&bottom_normal,&pnt.p3_vec);
			if (dotp>size)
				return 0;
		}

		return 1;
	}

	// Center point is on, must be visible
	return 1;
}
