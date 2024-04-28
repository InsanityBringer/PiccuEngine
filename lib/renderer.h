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

#ifndef RENDERER_H
#define RENDERER_H

#include "pstypes.h"
#include "grdefs.h"

//Declare this here so we don't need to include 3d.h
struct g3Point;
struct chunked_bitmap;

//	for rend_Init prototype
class oeApplication;

#define TEXTURE_WIDTH	128
#define TEXTURE_HEIGHT	128
#define TEXTURE_BPP		2

#define FLAT_SHADE_COLOR	0x7C01
// If an incoming texture has the above color in it, change that color to this color
#define REPLACEMENT_COLOR	0x07C0

extern int Triangles_drawn;

// Is this hardware or software rendered?
enum renderer_type
{
	RENDERER_SOFTWARE_8BIT,
	RENDERER_SOFTWARE_16BIT,
	RENDERER_OPENGL,
	RENDERER_DIRECT3D,
	RENDERER_GLIDE,
	RENDERER_NONE,
};

extern renderer_type Renderer_type;

// renderer clear flags
#define RF_CLEAR_ZBUFFER	1
#define RF_CLEAR_COLOR		2

// Overlay texture settings
#define OT_NONE			0			// No overlay
#define OT_BLEND			1			// Draw a lightmap texture afterwards
#define OT_REPLACE		2			// Draw a tmap2 style texture afterwards
#define OT_FLAT_BLEND	3			// Draw a gouraud shaded polygon afterwards
#define OT_BLEND_VERTEX	4			// Like OT_BLEND, but take constant alpha into account
#define OT_BUMPMAP		5			// Draw a saturated bumpmap afterwards
#define OT_BLEND_SATURATE	6		// Add a lightmap in


extern ubyte Overlay_type;
extern int Overlay_map;
extern int Bumpmap_ready,Bump_map;
extern float Z_bias;
extern bool UseHardware;
extern bool StateLimited;
extern bool NoLightmaps;
extern bool UseMultitexture;
extern bool UseWBuffer;
extern bool UseMipmap;	//DAJ
extern bool ATIRagePro;	//DAJ
extern bool Formac;	//DAJ

// various state setting functions
//------------------------------------

// Sets our renderer
void rend_SetRendererType (renderer_type state);

#define MAP_TYPE_BITMAP			0
#define MAP_TYPE_LIGHTMAP		1
#define MAP_TYPE_BUMPMAP		2
#define MAP_TYPE_UNKNOWN		3

// lighting state
enum light_state
{
	LS_NONE,				// no lighting, fully lit rendering
	LS_GOURAUD,			// Gouraud shading
	LS_PHONG,			// Phong shading
	LS_FLAT_GOURAUD	 // Take color from flat color
};

void rend_SetLighting(light_state);

enum color_model
{
	CM_MONO,		// monochromatic (intensity) model - default
	CM_RGB,			// RGB model
};

// color model
void rend_SetColorModel (color_model);

enum texture_type
{
	TT_FLAT,					// solid color
	TT_LINEAR,					// textured linearly
	TT_PERSPECTIVE,				// texture perspectively
	TT_LINEAR_SPECIAL,			// A textured polygon drawn as a flat color
	TT_PERSPECTIVE_SPECIAL,			// A textured polygon drawn as a flat color
};

// Alpha type flags - used to decide what type of alpha blending to use
#define ATF_CONSTANT		1		// Take constant alpha into account
#define ATF_TEXTURE		2		// Take texture alpha into account
#define ATF_VERTEX		4		// Take vertex alpha into account

// Alpha types
#define	AT_ALWAYS				0				// Alpha is always 255 (1.0)					
#define	AT_CONSTANT				1				// constant alpha across the whole image
#define	AT_TEXTURE				2				// Only uses texture alpha
#define	AT_CONSTANT_TEXTURE  3				// Use texture * constant alpha
#define	AT_VERTEX				4				// Use vertex alpha only
#define	AT_CONSTANT_VERTEX	5				// Use vertex * constant alpha
#define	AT_TEXTURE_VERTEX		6				// Use texture * vertex alpha
#define	AT_CONSTANT_TEXTURE_VERTEX	7		// Use all three (texture constant vertex)
#define	AT_LIGHTMAP_BLEND		8				// dest*src colors
#define  AT_SATURATE_TEXTURE	9				// Saturate up to white when blending
#define  AT_FLAT_BLEND			10				// Like lightmap blend, but uses gouraud shaded flat polygon
#define  AT_ANTIALIAS			11				// Draws an antialiased polygon
#define  AT_SATURATE_VERTEX	12				// Saturation with vertices
#define  AT_SATURATE_CONSTANT_VERTEX	13  // Constant*vertex saturation
#define	AT_SATURATE_TEXTURE_VERTEX		14	// Texture * vertex saturation
#define	AT_LIGHTMAP_BLEND_VERTEX		15	//	Like AT_LIGHTMAP_BLEND, but take vertex alpha into account
#define	AT_LIGHTMAP_BLEND_CONSTANT		16	// Like AT_LIGHTMAP_BLEND, but take constant alpha into account
#define	AT_SPECULAR							32	
#define	AT_LIGHTMAP_BLEND_SATURATE		33	// Light lightmap blend, but add instead of multiply

#define	LFB_LOCK_READ		0
#define	LFB_LOCK_WRITE		1

enum wrap_type
{
	WT_WRAP,			// Texture repeats
	WT_CLAMP,		// Texture clamps
	WT_WRAP_V		// Texture wraps in v
};

struct rendering_state
{
	sbyte initted;			

	sbyte cur_bilinear_state;
	sbyte cur_zbuffer_state;
	sbyte cur_fog_state;
	sbyte cur_mip_state;

	texture_type cur_texture_type;
	color_model	cur_color_model;
	light_state cur_light_state;
	sbyte			cur_alpha_type;

	wrap_type	cur_wrap_type;

	float cur_fog_start,cur_fog_end;
	float cur_near_z,cur_far_z;
	float gamma_value;

	int			cur_alpha;
	ddgr_color	cur_color;
	ddgr_color	cur_fog_color;
	
	sbyte cur_texture_quality;		// 0-none, 1-linear, 2-perspective

	int clip_x1,clip_x2,clip_y1,clip_y2;
	int screen_width,screen_height;
	int view_width, view_height;

};

struct renderer_preferred_state
{
	ubyte mipping;
	ubyte filtering;
	float gamma;
	ubyte bit_depth;
	int width, height;
	ubyte vsync_on;
	bool fullscreen; //Informs the window system that fullscreen should be used. 
	int window_width, window_height; //Size of the game window, may != width/height. 
};

struct renderer_lfb
{
	int type;
	ushort* data;
	int bytes_per_row;
};

struct tRendererStats
{
	int poly_count;
	int vert_count;
	int texture_uploads;
};

// returns rendering statistics for the frame
void rend_GetStatistics(tRendererStats *stats);

void rend_SetTextureType (texture_type);

// Given a handle to a bitmap and nv point vertices, draws a 3D polygon
void rend_DrawPolygon3D(int handle,g3Point **p,int nv,int map_type=MAP_TYPE_BITMAP);

// Given a handle to a bitmap and nv point vertices, draws a 2D polygon
void rend_DrawPolygon2D(int handle,g3Point **p,int nv);

// Tells the software renderer whether or not to use mipping
void rend_SetMipState (sbyte);

// Sets the fog state to TRUE or FALSE
void rend_SetFogState (sbyte on);

// Sets the near and far plane of fog
void rend_SetFogBorders (float fog_near,float fog_far);

// Sets the color for fill based primitives;
void rend_SetFlatColor (ddgr_color color);

// Tells the renderer we're starting a frame.  Clear flags tells the renderer
// what buffer (if any) to clear
void rend_StartFrame(int x1,int y1,int x2,int y2,int clear_flags=RF_CLEAR_ZBUFFER);

// Tells the renderer the frame is over
void rend_EndFrame();

// Init our renderer, pass the application object also.
int rend_Init (renderer_type state, oeApplication *app,renderer_preferred_state *pref_state);

// de-init the renderer
void rend_Close ();

// Draws a scaled 2d bitmap to our buffer
// NOTE: scripts are expecting the old prototype that has a zvalue (which is ignored) before color
void rend_DrawScaledBitmap (int x1,int y1,int x2,int y2,int bm,float u0,float v0,float u1,float v1,int color=-1,float *alphas=NULL);

void rend_DrawScaledBitmapWithZ(int x1, int y1, int x2, int y2,
	int bm, float u0, float v0, float u1, float v1, float zval, int color, float* alphas = nullptr);

// Sets the state of bilinear filtering for our textures
void rend_SetFiltering (sbyte state);

// Sets the state of zbuffering to on or off
void rend_SetZBufferState  (sbyte state);

// Sets the near and far planes for z buffer
void rend_SetZValues (float nearz,float farz);

// Sets a bitmap as an overlay to rendered on top of the next texture map
void rend_SetOverlayMap (int handle);


// Sets the type of overlay operation
void rend_SetOverlayType (ubyte type);

// Clears the display to a specified color
void rend_ClearScreen (ddgr_color color);

// Fills a rectangle on the display
void rend_FillRect (ddgr_color color,int x1,int y1,int x2,int y2);

// Sets a pixel on the display
void rend_SetPixel (ddgr_color color,int x,int y);

// Sets a pixel on the display
ddgr_color rend_GetPixel (int x,int y);

// Sets up a font character to draw.  We draw our fonts as pieces of textures
void rend_DrawFontCharacter (int bm_handle,int x1,int y1,int x2,int y2,float u,float v,float w,float h);

// Draws a line
void rend_DrawLine (int x1,int y1,int x2,int y2);

//	Draws spheres
void rend_FillCircle(ddgr_color col, int x, int y, int rad);

//	draws circles
void rend_DrawCircle(int x, int y, int rad);

// Flips the surface
void rend_Flip();

// Sets the argb characteristics of the font characters.  color1 is the upper left and proceeds clockwise
void rend_SetCharacterParameters (ddgr_color color1,ddgr_color color2,ddgr_color color3,ddgr_color color4);

// Sets the color of fog
void rend_SetFogColor (ddgr_color fogcolor);

// sets the alpha type
void rend_SetAlphaType (sbyte);

// Sets the constant alpha value
void rend_SetAlphaValue (ubyte val);

// Sets the overall alpha scale factor (all alpha values are scaled by this value)
// usefull for motion blur effect
void rend_SetAlphaFactor(float val);

// Returns the current Alpha factor
float rend_GetAlphaFactor(void);

// Sets the wrap parameter
void rend_SetWrapType (wrap_type val);

// Takes a screenshot of the current frame and puts it into the handle passed
void rend_Screenshot (int bm_handle);

// Adds a bias to each coordinates z value.  This is useful for making 2d bitmaps
// get drawn without being clipped by the zbuffer
void rend_SetZBias (float z_bias);

// Enables/disables writes the depth buffer
void rend_SetZBufferWriteMask (int state);

// Sets where the software renderer should write to
void rend_SetSoftwareParameters(float aspect,int width,int height,int pitch,ubyte *framebuffer);

// Fills in some variables so the 3d math routines know how to project
void rend_GetProjectionParameters(int *width,int *height);
void rend_GetProjectionScreenParameters( int &screenLX, int &screenTY, int &screenW, int &screenH );

// Returns the aspect ratio of the physical screen
float rend_GetAspectRatio();

// Gets a pointer to a linear frame buffer
void rend_GetLFBLock (renderer_lfb *lfb);

// Releases an lfb lock
void rend_ReleaseLFBLock (renderer_lfb *lfb);

// Given a source x,y and width,height, draws any sized bitmap into the renderer lfb
void rend_DrawLFBBitmap (int sx,int sy,int w,int h,int dx,int dy,ushort *data,int rowsize);

//	given a chunked bitmap, renders it.
void rend_DrawChunkedBitmap(chunked_bitmap *chunk, int x, int y, ubyte alpha);

//	given a chunked bitmap, renders it.scaled
void rend_DrawScaledChunkedBitmap(chunked_bitmap *chunk, int x, int y, int neww, int newh, ubyte alpha);

// Draws a line using the states of the renderer
void rend_DrawSpecialLine (g3Point *p0,g3Point *p1);

// Sets some global preferences for the renderer
// Returns -1 if it had to use the default resolution/bitdepth
int rend_SetPreferredState (renderer_preferred_state *pref_state);

// Sets the gamma value 
void rend_SetGammaValue (float val);

// Fills in the passed in pointer with the current rendering state
void rend_GetRenderState (rendering_state *rstate);

// Draws a simple bitmap at the specified x,y location
void rend_DrawSimpleBitmap (int bm_handle,int x,int y);

// Changes the resolution of the renderer
void rend_SetResolution (int width,int height);

// Shuts down OpenGL in a window
void rend_CloseOpenGLWindow ();

// Sets the state of the OpenGLWindow to on or off
void rend_SetOpenGLWindowState (int state,oeApplication *app,renderer_preferred_state *pref_state);

// Sets the hardware bias level for coplanar polygons
// This helps reduce z buffer artifaces
void rend_SetCoplanarPolygonOffset (float factor);

// Gets the error message string
char *rend_GetErrorMessage ();

// Sets the error message string
void rend_SetErrorMessage (char *str);

// Preuploads a bitmap to the card
void rend_PreUploadTextureToCard (int,int);
void rend_FreePreUploadedTexture (int,int);

// Returns 1 if there is mid video memory, 2 if there is low vid memory, or 0 if there is large vid memory
int rend_LowVidMem ();

// Returns 1 if the renderer supports bumpmapping
int rend_SupportsBumpmapping ();

// Gets a bumpmap ready for drawing, or turns off bumpmapping
void rend_SetBumpmapReadyState (int state,int map);

// Clears the zbuffer
void rend_ClearZBuffer ();

// Clears the texture cache
void rend_ResetCache ();

// Takes a bitmap and blits it to the screen using linear frame buffer stuff
// X and Y are the destination X,Y.
void rend_CopyBitmapToFramebuffer (int bm_handle,int x,int y);

// Gets a renderer ready for a framebuffer copy, or stops a framebuffer copy
void rend_SetFrameBufferCopyState (bool state);

#if defined(DD_ACCESS_RING) 
#if defined(WIN32)
// returns the direct draw object 
void *rend_RetrieveDirectDrawObj(void **frontsurf, void **backsurf);
#endif
#endif

///////////////////////////////////////////////////////////////
#include "../renderer/RendererConfig.h"
#ifdef USE_SOFTWARE_TNL
#include "IMeshBuilder.h"

RZ::Renderer::IMeshBuilder *rend_CreateMeshBuilder(void);
#endif

#endif
