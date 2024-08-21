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

#ifndef TEXTURE_H
#define TEXTURE_H

#include "3d.h"
#include "pstypes.h"
#include "renderer.h"

#define MAX_TEXTURE_SHADES 32

struct texs_texpoint
{
	int screenx,screeny;
	float flscreenx,flscreeny;

	fix ucoord,vcoord;
	float flucoord,flvcoord,flzcoord;

	fix monolight;
	fix redlight,greenlight,bluelight;
};

// For the following enum table: LIN=Linear, PER=Perspective, NON=No Transparency, LIT = Shaded, 
//								 COLORED=Colored lighting, TRANS=Transparency ALPHA=Alpha renderer

#define MAX_RENDER_TYPES 15

extern void (*Texture_functions[])(g3Point *,int);

// Our shade tables
extern ubyte TexShadeTable8[MAX_TEXTURE_SHADES][256];
extern ulong TexShadeTable16[MAX_TEXTURE_SHADES][256];
extern ubyte TexRevShadeTable8[MAX_TEXTURE_SHADES][256];
extern ulong TexRevShadeTable16[MAX_TEXTURE_SHADES][256];

int tex_Init();

// These are the different routines for drawing the various types of effects on a texture
void tex_PerUnlitTexturedQuad (g3Point *,int);
void tex_PerLitTexturedQuad (g3Point *,int);
void tex_UnlitTexturedQuad (g3Point *,int);
void tex_LitTexturedQuad (g3Point *,int);
void tex_ColoredTexturedQuad (g3Point *,int);
void tex_PerColoredTexturedQuad (g3Point *,int);
void tex_UnlitTransQuad (g3Point *,int);
void tex_LitTransQuad(g3Point *,int);
void tex_PerUnlitTransQuad (g3Point *,int);
void tex_PerLitTransQuad(g3Point *,int);
void tex_LinFlatShade (g3Point *,int);
void tex_PerFlatShade (g3Point *,int);
void tex_LitTexturedTriangle (g3Point *,int);
void tex_FlatPolyTriangle (g3Point *,int);
void tex_SetFlatColor (ddgr_color color);
void tex_UnlitTexturedQuad16 (g3Point *p,int nv);
void tex_LitTexturedQuad16 (g3Point *p,int nv);
void tex_PerUnlitTexturedQuad16 (g3Point *p,int nv);
void tex_PerLitTexturedQuad16 (g3Point *p,int nv);
void tex_LitTexturedTriangle16 (g3Point *p,int nv);
void tex_LinFlatShade16 (g3Point *p,int nv);
void tex_PerFlatShade16 (g3Point *p,int nv);
void tex_AlphaTexturedQuad(g3Point *p,int nv);

// These functions get the extremes of a polygon
void tex_GetVertexOrdering (texs_texpoint *t, int nv, int *vlt, int *vlb, int *vrt, int *vrb,int *bottom_y_ind);
int tex_PrevIndex(int val,int modulus);
int tex_NextIndex(int val,int modulus);

// Builds the 1/n table
void tex_BuildRecipTable();

// Draws a solid color polygon.  "color" is a color in 5-6-5 format.
void tex_DrawFlatPolygon (g3Point **pv,int nv);

// Given a handle to a texture and nv point vertices, does a fan algorithm to draw them all
// Uses the function set by tex_SetRenderType
void tex_DrawPointList(int handle,g3Point **p,int nv);

// Tells the software renderer whether or not to use mipping
void tex_SetMipState (int);

// Sets the fog state to TRUE or FALSE
void tex_SetFogState (int on);

// Sets the a lighting state for the software renderer
void tex_SetLighting (light_state);

// Sets a color model (see renderer.h for valid models)
void tex_SetColorModel (color_model);

// Sets a texturing type (see renderer.h)
void tex_SetTextureType (texture_type);

// Sets the near and far plane of fog
void tex_SetFogBorders (float fog_near,float fog_far);

// Sets the alpha type
void tex_SetAlphaType (sbyte);

// ZBuffer variables
void tex_SetZBufferState (int state);
void tex_ClearZBuffer ();
void tex_StartFrame(int x1,int y1,int x2,int y2);
void tex_EndFrame();

// Sets the software renderer to render to a particular place
void tex_SetSoftwareParameters (float,int,int,int,ubyte *);

// Fills in projection variables
void tex_GetProjectionParameters (int *width,int *height);

// Returns the aspect ratio of the physical screen
float tex_GetAspectRatio ();

//	software line renderer.
void tex_DrawLine(int x1, int y1, int x2, int y2);

//	fill rect supported.
void tex_FillRect(ddgr_color color, int x1, int y1, int x2, int y2);

//	set pixel
void tex_SetPixel(ddgr_color color, int x, int y);

//	get pixel
ddgr_color tex_GetPixel(int x, int y);

//	fillcircle
void tex_FillCircle(ddgr_color col, int x, int y, int r);

//	draw circle
void tex_DrawCircle(int x, int y, int r);

//	lfb stuff
void tex_GetLFBLock(renderer_lfb *lfb);

void tex_ReleaseLFBLock(renderer_lfb *lfb);

void tex_GetRenderState (rendering_state *rs);


#endif
