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
 

#ifndef __OGL_RENDER_SYS_H_
#define __OGL_RENDER_SYS_H_

#include "ned_Rend.h"

// Return codes for rGL_InitializeRenderer
#define RGL_ERR_NONE			0	//no error
#define RGL_ERR_DLLNOTFOUND		1	//the OpenGL DLL was not found

// Closes down opengl
void rGL_Close();

// Starts up opengl
int rGL_Init (oeApplication *app,renderer_preferred_state *pref_state);

int rGL_InitCache ();

int rGL_Setup(HDC glhdc);

int rGL_CheckExtension( char *extName );

int rGL_MakeTextureObject (int tn);
// Takes our 16bit format and converts it into the memory scheme that OpenGL wants
void rGL_TranslateBitmapToOpenGL (int texnum,int bm_handle,int map_type,int replace,int tn);
// Utilizes a LRU cacheing scheme to select/upload textures the opengl driver
int rGL_MakeBitmapCurrent (int handle,int map_type,int tn);
// Sets up an appropriate wrap type for the current bound texture
void rGL_MakeWrapTypeCurrent (int handle,int map_type,int tn);
// Chooses the correct filter type for the currently bound texture
void rGL_MakeFilterTypeCurrent (int handle,int map_type,int tn);

// =====================
// rGL_Flip
// =====================
//
//	Renders the window
void rGL_Flip(void);

// ===================
// rGL_DrawLine
// ===================
//
// draws a line
void rGL_DrawLine (int x1,int y1,int x2,int y2);

// ===================
// rGL_GetAspectRatio
// ===================
//
// Returns the aspect ratio of the physical screen
float rGL_GetAspectRatio ();

// =====================
// rGL_SetAlphaType
// =====================
//
// Sets the type of alpha blending you want
void rGL_SetAlphaType (sbyte atype);

// ===================
// rGL_SetWrapType
// ===================
//
// Sets texture wrapping type
void rGL_SetWrapType (wrap_type val);

// ===================
// rGL_SetZBufferWriteMask
// ===================
//
// Sets whether or not to write into the zbuffer
void rGL_SetZBufferWriteMask (int state);

// ===================
// rGL_SetZBufferWriteMask
// ===================
//
// Sets whether or not to write into the zbuffer
void rGL_SetZBufferWriteMask (int state);

// ====================
// rGL_SetAlphaValue
// ====================
//
// Sets the constant alpha value
void rGL_SetAlphaValue (ubyte val);

// ======================
// rGL_SetAlphaMultiplier
// ======================
//
// Sets the alpha multiply factor
void rGL_SetAlphaMultiplier ();

// ======================
// rGL_GetAlphaMultiplier
// ======================
//
// returns the alpha that we should use
float rGL_GetAlphaMultiplier ();

// =====================
// rGL_SetLightingState
// =====================
//
// Sets the lighting state of opengl
void rGL_SetLightingState (light_state state);

// =======================
// rGL_SetColorModel
// =======================
//
// Sets the opengl color model (either rgb or mono)
void rGL_SetColorModel (color_model state);

// =====================
// rGL_SetFiltering
// =====================
//
// Sets the state of bilinear filtering for our textures
void rGL_SetFiltering (sbyte state);

// ========================
// rGL_SetZBufferState
// ========================
//
// Sets the state of zbuffering to on or off
void rGL_SetZBufferState  (sbyte state);

// =======================
// rGL_SetZValues
// =======================
//
// Sets the z clip plane values
void rGL_SetZValues (float nearz,float farz);

// =======================
// rGL_ClearScreen
// =======================
//
// Clears the display to a specified color
void rGL_ClearScreen (ddgr_color color);

// =======================
// rGL_ClearZBuffer
// =======================
//
// Clears the zbuffer
void rGL_ClearZBuffer ();

// =======================
// rGL_FillRect
// =======================
//
// Fills a rectangle on the display
void rGL_FillRect (ddgr_color color,int x1,int y1,int x2,int y2);

// ====================
// rGL_SetPixel
// ====================
//
// Sets a pixel on the display
void rGL_SetPixel (ddgr_color color,int x,int y);

// =======================
// rGL_GetPixel
// =======================
//
// Returns the pixel color at x,y
ddgr_color rGL_GetPixel (int x,int y);

// =========================
// rGL_BeginFrame
// =========================
//
// Starts a rendering frame
void rGL_BeginFrame (int x1,int y1,int x2,int y2,int clear_flags);

// ===================
// rGL_EndFrame
// ===================
//
// Ends a rendering frame
void rGL_EndFrame();

// ======================
// rGL_GetProjectionParameters
// ======================
//
// Fills in projection variables
void rGL_GetProjectionParameters (int *width,int *height);

// ======================
// rGL_DrawPolygon
// ======================
//
// Takes nv vertices and draws the polygon defined by those vertices.  Uses bitmap "handle"
// as a texture
void rGL_DrawPolygon (int handle,g3Point **p,int nv,int map_type);

// =====================
// rGL_SetFlatColor
// =====================
//
//	Sets the current color
void rGL_SetFlatColor (ddgr_color color);

// ======================
// rGL_SetTextureType
// ======================
//
// Sets texture 
void rGL_SetTextureType (texture_type state);

#endif
