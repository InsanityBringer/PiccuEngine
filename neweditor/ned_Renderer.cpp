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
 
#include "stdafx.h"
#include "ddaccess.h"
#include "application.h"
#include "ned_Rend.h"
#include "texture.h"
#include "ned_RendOpenGL.h"
//#include "..\3d\globvars.h"
//#include "..\3d\clipper.h"
#include "globals.h"

void rend_CloseOpenGLWindow();

int OpenGL_window_initted=0;

// The font characteristics
static float rend_FontRed[4],rend_FontBlue[4],rend_FontGreen[4],rend_FontAlpha[4];

extern vector Clip_plane_point;

ubyte Renderer_close_flag=0,Renderer_initted=0;
renderer_type Renderer_type = RENDERER_SOFTWARE_16BIT;
//renderer_type Renderer_type = RENDERER_OPENGL;

bool  StateLimited;
bool  NoLightmaps;
bool  UseMultitexture;
bool  UseHardware;

float Z_bias=0.0;

// Init our renderer
int rend_Init (renderer_type state, oeApplication *app,renderer_preferred_state *pref_state)
{
	int retval=0;
	rend_SetRendererType (state);
	if (!Renderer_initted)
	{
		if (!Renderer_close_flag)
		{
			atexit (rend_Close);
			Renderer_close_flag=1;
		}
		Renderer_initted=1;
	}
	if (OpenGL_window_initted)
	{
		rend_CloseOpenGLWindow ();
		OpenGL_window_initted=0;
	}
	mprintf ((0,"Renderer init is set to %d\n",Renderer_initted));
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			NoLightmaps=false;
			UseHardware=1;
			StateLimited=1;
			UseMultitexture=0;
			//retval=rGL_Init (app,pref_state);
			retval = 0;
			break;
		default:
//			rend_SetErrorMessage ("No renderer set.");
			retval=0;
			break;
	}
//	if (retval!=0)
//		rend_SetInitOptions();
	return retval;
}

void rend_Close ()
{
	mprintf ((0,"CLOSE:Renderer init is set to %d\n",Renderer_initted));
	if (!Renderer_initted)
		return;
	if (OpenGL_window_initted)
	{
		if (Renderer_type!=RENDERER_OPENGL)
			rend_CloseOpenGLWindow ();
		OpenGL_window_initted=0;
	}
	switch (Renderer_type)
	{
		case RENDERER_SOFTWARE_16BIT:
		case RENDERER_SOFTWARE_8BIT:
			break;
		case RENDERER_OPENGL:
			//rGL_Close();
			break;
	}
	Renderer_initted=0;
}

// Sets a pixel on the display
void rend_SetPixel (ddgr_color color,int x,int y)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_SetPixel(color,x,y);
		break;
	}	
}


// Sets a pixel on the display
ddgr_color rend_GetPixel (int x,int y)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//return rGL_GetPixel(x,y);
		break;
	}
	
	return GR_RGB(0,0,0);
}


// Flips the screen
void rend_Flip ()
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_Flip();
		break;
	}

}


// Given nv points, draws that polygon according to the various state variables
// Handle is a bitmap handle
void rend_DrawPolygon (int handle,g3Point **p,int nv,int map_type)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_DrawPolygon(handle,p,nv,map_type);
		break;
	}	
}


// Sets up a font character to draw.  We draw our fonts as pieces of textures
void rend_DrawFontCharacter (int bm_handle,int x1,int y1,int x2,int y2,float u,float v,float w,float h)
{
	g3Point *ptr_pnts[4];
	g3Point pnts[4];

	for (int i=0;i<4;i++)
	{
		pnts[i].p3_l=1;
		pnts[i].p3_r=rend_FontRed[i];
		pnts[i].p3_g=rend_FontGreen[i];
		pnts[i].p3_b=rend_FontBlue[i];
		pnts[i].p3_z=1;	// Make REALLY close!
		pnts[i].p3_flags=PF_PROJECTED;
	}

	pnts[0].p3_sx=x1;
	pnts[0].p3_sy=y1;
	pnts[0].p3_u=u;
	pnts[0].p3_v=v;

	pnts[1].p3_sx=x2;
	pnts[1].p3_sy=y1;
	pnts[1].p3_u=u+w;
	pnts[1].p3_v=v;

	pnts[2].p3_sx=x2;
	pnts[2].p3_sy=y2;
	pnts[2].p3_u=u+w;
	pnts[2].p3_v=v+h;

	pnts[3].p3_sx=x1;
	pnts[3].p3_sy=y2;
	pnts[3].p3_u=u;
	pnts[3].p3_v=v+h;

	ptr_pnts[0]=&pnts[0];
	ptr_pnts[1]=&pnts[1];
	ptr_pnts[2]=&pnts[2];
	ptr_pnts[3]=&pnts[3];

	rend_DrawPolygon (bm_handle,ptr_pnts,4, MAP_TYPE_BITMAP);
}


// Sets the argb characteristics of the font characters.  color1 is the upper left and proceeds clockwise
void rend_SetCharacterParameters (ddgr_color color1,ddgr_color color2,ddgr_color color3,ddgr_color color4)
{
	rend_FontRed[0]=(float)(GR_COLOR_RED(color1)/255.0);
	rend_FontRed[1]=(float)(GR_COLOR_RED(color2)/255.0);
	rend_FontRed[2]=(float)(GR_COLOR_RED(color3)/255.0);
	rend_FontRed[3]=(float)(GR_COLOR_RED(color4)/255.0);

	rend_FontGreen[0]=(float)(GR_COLOR_GREEN(color1)/255.0);
	rend_FontGreen[1]=(float)(GR_COLOR_GREEN(color2)/255.0);
	rend_FontGreen[2]=(float)(GR_COLOR_GREEN(color3)/255.0);
	rend_FontGreen[3]=(float)(GR_COLOR_GREEN(color4)/255.0);

	rend_FontBlue[0]=(float)(GR_COLOR_BLUE(color1)/255.0);
	rend_FontBlue[1]=(float)(GR_COLOR_BLUE(color2)/255.0);
	rend_FontBlue[2]=(float)(GR_COLOR_BLUE(color3)/255.0);
	rend_FontBlue[3]=(float)(GR_COLOR_BLUE(color4)/255.0);

	rend_FontAlpha[0]=(color1>>24)/255.0;
	rend_FontAlpha[1]=(color2>>24)/255.0;
	rend_FontAlpha[2]=(color3>>24)/255.0;
	rend_FontAlpha[3]=(color4>>24)/255.0;
}


// Draws a line
void rend_DrawLine (int x1,int y1,int x2,int y2)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_DrawLine(x1,y1,x2,y2);
		break;
	}
}

//	draws circles
void rend_DrawCircle(int x, int y, int rad)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//Int3();
		break;
	}
}

void rend_SetFlatColor (ddgr_color color)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_SetFlatColor(color);
		break;
	}	
}


void rend_SetRendererType (renderer_type state)
{
	Renderer_type=state;
	mprintf ((0,"RendererType is set to %d.\n",state));
}

void rend_StartFrame (int x1,int y1,int x2,int y2,int clear_flags)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_BeginFrame (x1,y1,x2,y2,clear_flags);
		break;
	}	
}


void rend_EndFrame ()
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		///rGL_EndFrame();
		break;
	}	
}

void rend_ClearScreen (ddgr_color color)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_ClearScreen (color);
		break;
	}
}


void rend_SetSoftwareParameters(float aspect,int width,int height,int pitch,ubyte *framebuffer)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		break;
	}	
}


void rend_FillRect (ddgr_color color,int x1,int y1,int x2,int y2)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_FillRect(color,x1,y1,x2,y2);
		break;
	}	
}


// Returns the aspect ratio of the physical screen
float rend_GetAspectRatio ()
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//return rGL_GetAspectRatio ();
		break;
	}

	return 1.0f;	
}


// Returns the aspect ratio of the physical screen
void rend_GetProjectionParameters (int *width,int *height)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_GetProjectionParameters(width,height);
		break;
	}	
}


// Draws a line using the states of the renderer
void rend_DrawSpecialLine (g3Point *p0,g3Point *p1)
{
}


//	Draws spheres
void rend_FillCircle(ddgr_color col, int x, int y, int rad)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		break;
	}	
}


void rend_SetTextureType (texture_type state)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_SetTextureType(state);
		break;
	}	
}


void rend_DrawScaledBitmap (int x1,int y1,int x2,int y2,
					      int bm,float u0,float v0,float u1,float v1,float zval,int color,float *alphas)
{
			g3Point *ptr_pnts[4];
			g3Point pnts[4];
			float r,g,b;

			if (color!=-1)
			{
				r=GR_COLOR_RED(color)/255.0;
				g=GR_COLOR_GREEN(color)/255.0;
				b=GR_COLOR_BLUE(color)/255.0;
			}

			for (int i=0;i<4;i++)
			{
				if (color==-1)
					pnts[i].p3_l=1.0;
				else
				{
					pnts[i].p3_r=r;
					pnts[i].p3_g=g;
					pnts[i].p3_b=b;
				}

				if (alphas)
				{
					pnts[i].p3_a=alphas[i];
				}
					
				pnts[i].p3_z=zval;	
				pnts[i].p3_flags=PF_PROJECTED;
			}

			

			pnts[0].p3_sx=x1;
			pnts[0].p3_sy=y1;
			pnts[0].p3_u=u0;
			pnts[0].p3_v=v0;

			pnts[1].p3_sx=x2;
			pnts[1].p3_sy=y1;
			pnts[1].p3_u=u1;
			pnts[1].p3_v=v0;

			pnts[2].p3_sx=x2;
			pnts[2].p3_sy=y2;
			pnts[2].p3_u=u1;
			pnts[2].p3_v=v1;

			pnts[3].p3_sx=x1;
			pnts[3].p3_sy=y2;
			pnts[3].p3_u=u0;
			pnts[3].p3_v=v1;

			ptr_pnts[0]=&pnts[0];
			ptr_pnts[1]=&pnts[1];
			ptr_pnts[2]=&pnts[2];
			ptr_pnts[3]=&pnts[3];

			rend_SetTextureType (TT_LINEAR);
			rend_DrawPolygon (bm,ptr_pnts,4,MAP_TYPE_BITMAP);
}


void rend_SetLighting(light_state state)
{
	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		//rGL_SetLightingState (state);
		break;
	}	
}

void rend_SetColorModel (color_model state)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			//rGL_SetColorModel (state);
			break;
	}
}

void rend_SetAlphaType (sbyte atype)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			//rGL_SetAlphaType (atype);
			break;
	}
}


void rend_SetZBias (float z_bias)
{
	Z_bias=z_bias;
}

int WindowGL=0;
// Gets OpenGL ready to work in a window
int rend_InitOpenGLWindow (oeApplication *app,renderer_preferred_state *pref_state)
{
	//WindowGL=1;
	//return rGL_Init (app,pref_state);
	Int3();
	return 0; //please don't call this legacy code
}

// Shuts down OpenGL in a window
void rend_CloseOpenGLWindow ()
{
	//rGL_Close();
	//WindowGL=0;
	//OpenGL_window_initted=0;
	//mprintf ((1,"SHUTTING DOWN WINDOWED OPENGL!"));
}

// Sets the state of the OpenGLWindow to on or off
static renderer_type Save_rend;
static bool Save_state_limit;
void rend_SetOpenGLWindowState (int state,oeApplication *app,renderer_preferred_state *pref_state)
{
	/*if (state)
	{
		if (!OpenGL_window_initted)
		{
			if (rend_InitOpenGLWindow (app,pref_state))
				OpenGL_window_initted=1;
			else
				return;
		}
		UseHardware=1;
		Save_rend=Renderer_type;
		Save_state_limit=StateLimited;
		Renderer_type=RENDERER_OPENGL;
		StateLimited=1;
		NoLightmaps=false;
	}
	else
	{
		if (OpenGL_window_initted)
		{
			UseHardware=0;
			Renderer_type=RENDERER_SOFTWARE_16BIT;
			StateLimited=Save_state_limit;
		}
	}*/
}


// Sets a bitmap as a overlay map to rendered on top of the next texture map
// a -1 value indicates no overlay map
void rend_SetOverlayMap (int handle)
{
	Overlay_map=handle;
}

void rend_SetOverlayType(ubyte type)
{
	Overlay_type=type;
}


// Sets the alpha value for constant alpha
void rend_SetAlphaValue (ubyte val)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			//rGL_SetAlphaValue (val);
			break;
	}
}

// Sets the texture wrapping type
void rend_SetWrapType (wrap_type val)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			//rGL_SetWrapType (val);
			break;
	}
}

// Sets the state of bilinear filtering for our textures
void rend_SetFiltering (sbyte state)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			//rGL_SetFiltering (state);
			break;
	}
}

// Sets the state of zbuffering to on or off
void rend_SetZBufferState  (sbyte state)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			//rGL_SetZBufferState (state);
			break;
	}
}

// Sets the near and far planes for z buffer
void rend_SetZValues (float nearz,float farz)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			//rGL_SetZValues (nearz,farz);
			break;
	}
}

// Enables/disables writes the depth buffer
void rend_SetZBufferWriteMask (int state)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			//rGL_SetZBufferWriteMask (state);
			break;
	}
}

// Tells the software renderer whether or not to use mipping
void rend_SetMipState (sbyte mipstate)
{
	switch (Renderer_type)
	{
		case RENDERER_OPENGL:
			break;
	}
}

/************** stubs **************/

// Sets the fog state to TRUE or FALSE
void rend_SetFogState (sbyte on)
{
}

//Creates a new context with shared resources from the main context
RendHandle rend_NewContext(HWND hwnd)
{
	RendHandle blarg;
	blarg.handle = 1;
	return blarg;
}

//Makes the specified handle current
void rend_MakeCurrent(RendHandle& handle)
{
}

//Deletes the context. All contexts will be deleted when rend is shut down. 
void rend_DestroyContext(RendHandle& handle)
{
}

