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
 #ifndef __NED_RENDER_H_
#define __NED_RENDER_H_

#include <windows.h>
#include "renderer.h"
#include "application.h"
#include "3d.h"

// Renderer types
extern renderer_type Renderer_type;

extern bool  StateLimited;
extern bool  NoLightmaps;
extern bool  UseMultitexture;
extern bool  UseHardware;

int rend_Init (renderer_type state, oeApplication *app,renderer_preferred_state *pref_state);

// color model
void rend_SetColorModel (color_model);

// Sets a bitmap as an overlay to rendered on top of the next texture map
void rend_SetOverlayMap (int handle);

// Sets the type of overlay operation
void rend_SetOverlayType (ubyte type);

// Sets the constant alpha value
void rend_SetAlphaValue (ubyte val);

// Sets the wrap parameter
void rend_SetWrapType (wrap_type val);

// Enables/disables writes the depth buffer
void rend_SetZBufferWriteMask (int state);

// Tells the software renderer whether or not to use mipping
void rend_SetMipState (sbyte mipstate);

// Gets OpenGL ready to work in a window
int rend_InitOpenGLWindow (oeApplication *app,renderer_preferred_state *pref_state);

// Sets the state of the OpenGLWindow to on or off
void rend_SetOpenGLWindowState (int state,oeApplication *app,renderer_preferred_state *pref_state);


#endif