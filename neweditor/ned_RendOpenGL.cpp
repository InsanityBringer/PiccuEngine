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
 

#include "mono.h"
#include "mem.h"
#include "vecmat.h"
#include "3d.h"
#include "bitmap.h"
#include "lightmap.h"
#include "DDAccess.h"				// Device Dependent access level module
#include "pserror.h"

#include "ned_Rend.h"
#include "ned_RendOpenGL.h"

#include "module.h"

#include "glad/gl.h"	//contains the OpenGL DLL binding code
#include "glad/wgl.h"


/*
#define MAX_BITMAPS		10
#define MAX_LIGHTMAPS	10
#define BF_CHANGED		1
#define BF_BRAND_NEW	2
#define LF_CHANGED		1
#define LF_BRAND_NEW	2
typedef struct tDummyMap
{
	int flags;
}tDummyMap;
tDummyMap GameBitmaps[MAX_BITMAPS];
tDummyMap GameLightmaps[MAX_BITMAPS];
*/

// =======================================================================
// Local globals
// =======================================================================
static HWND hOpenGLWnd = NULL;
static HDC hOpenGLDC = NULL;
HGLRC ResourceContext;
bool OpenGLInitialized = false;
rendering_state OpenGL_state;
static float Alpha_multiplier=1.0;
renderer_preferred_state OpenGL_preferred_state={0,1,1.5}; 
module *OpenGLDLLHandle=NULL;
int Already_loaded=0;

#define GET_WRAP_STATE(x)	(x>>4)
#define GET_FILTER_STATE(x)	(x & 0x0f)

#define SET_WRAP_STATE(x,s) {x&=0x0F; x|=(s<<4);}
#define SET_FILTER_STATE(x,s) {x&=0xF0; x|=(s);}

//	OpenGL Stuff
#define UNSIGNED_SHORT_5_5_5_1_EXT 0x8034
#define UNSIGNED_SHORT_4_4_4_4_EXT 0x8033
static int OpenGL_sets_this_frame[10];
static int OpenGL_packed_pixels=0;
static int Cur_texture_object_num=1;
static int OpenGL_cache_initted=0;
static int OpenGL_last_bound[2];

uint *opengl_Upload_data=NULL;
uint *opengl_Translate_table=NULL;
uint *opengl_4444_translate_table=NULL;
ushort *opengl_packed_Upload_data=NULL;
ushort *opengl_packed_Translate_table=NULL;
ushort *opengl_packed_4444_translate_table=NULL;
ushort *OpenGL_bitmap_remap;
ushort *OpenGL_lightmap_remap;
ubyte *OpenGL_bitmap_states;
ubyte *OpenGL_lightmap_states;

// =======================================================================
// local prototypes
// =======================================================================
void rGL_ShutdownRenderer(void);
// Gets some specific information about this particular flavor of opengl
void rGL_GetInformation ();
// Sets default states for our renderer
void rGL_SetDefaults();
// Sets the gamma correction value
void rGL_SetGammaValue (float val);


// These structs are for drawing with OpenGL vertex arrays
// Useful for fast indexing
typedef struct
{
	float r,g,b,a;
} color_array;

typedef struct
{	
	float s,t,r,w;
} tex_array;
vector GL_verts[100];
color_array GL_colors[100];
tex_array GL_tex_coords[100];
tex_array GL_tex_coords2[100];


// =====================
// rGL_Flip
// =====================
//
//	Renders the window
void rGL_Flip(void)
{
// start OS-specific code
	SwapBuffers ((HDC)hOpenGLDC);
// end
}

// ===================
// rGL_DrawLine
// ===================
//
// draws a line
void rGL_DrawLine (int x1,int y1,int x2,int y2)
{
	sbyte atype;
	light_state ltype;
	texture_type ttype;
	
	int color=OpenGL_state.cur_color;
	
	int r=GR_COLOR_RED(color);
	int g=GR_COLOR_GREEN(color);
	int b=GR_COLOR_BLUE(color);
		
	
	atype=OpenGL_state.cur_alpha_type;					
	ltype=OpenGL_state.cur_light_state;					
	ttype=OpenGL_state.cur_texture_type;

	rend_SetAlphaType (AT_ALWAYS);
	rend_SetLighting (LS_NONE);
	rend_SetTextureType (TT_FLAT);
	
		
	glBegin (GL_LINES);
	glColor4ub (r,g,b,255);
	glVertex2i (x1+OpenGL_state.clip_x1,y1+OpenGL_state.clip_y1);
	glColor4ub (r,g,b,255);
	glVertex2i (x2+OpenGL_state.clip_x1,y2+OpenGL_state.clip_y1);
	glEnd();

	rend_SetAlphaType (atype);
	rend_SetLighting(ltype);
	rend_SetTextureType (ttype);	
}

// ===================
// rGL_GetAspectRatio
// ===================
//
// Returns the aspect ratio of the physical screen
float rGL_GetAspectRatio ()
{
	float aspect_ratio = (float)((3.0 * OpenGL_state.screen_width)/(4.0 * OpenGL_state.screen_height));	
	return aspect_ratio;
}

// =====================
// rGL_SetAlphaType
// =====================
//
// Sets the type of alpha blending you want
void rGL_SetAlphaType (sbyte atype)
{
	if (atype==OpenGL_state.cur_alpha_type)
		return;		// don't set it redundantly

	switch (atype)
	{
		case AT_ALWAYS:
			rGL_SetAlphaValue (255);
			glBlendFunc (GL_ONE,GL_ZERO);
			break;
		case AT_CONSTANT:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
		case AT_TEXTURE:
			rGL_SetAlphaValue (255);
			glBlendFunc (GL_ONE,GL_ZERO);
			break;
		case AT_CONSTANT_TEXTURE:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
		case AT_VERTEX:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
		case AT_CONSTANT_TEXTURE_VERTEX:
		case AT_CONSTANT_VERTEX:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
		case AT_TEXTURE_VERTEX:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			break;
		case AT_LIGHTMAP_BLEND:
			glBlendFunc (GL_DST_COLOR,GL_ZERO);
			break;
		case AT_SATURATE_TEXTURE:
		case AT_LIGHTMAP_BLEND_SATURATE:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE);
			
			break;
		case AT_SATURATE_VERTEX:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE);
			break;
		case AT_SATURATE_CONSTANT_VERTEX:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE);
			break;
		case AT_SATURATE_TEXTURE_VERTEX:
			glBlendFunc (GL_SRC_ALPHA,GL_ONE);
			break;
		case AT_SPECULAR:
			break;
		default:
			break;
	}
	OpenGL_state.cur_alpha_type=atype;
	rGL_SetAlphaMultiplier();
}

// ===================
// rGL_SetWrapType
// ===================
//
// Sets texture wrapping type
void rGL_SetWrapType (wrap_type val)
{
	OpenGL_state.cur_wrap_type=val;
}

// ===================
// rGL_SetZBufferWriteMask
// ===================
//
// Sets whether or not to write into the zbuffer
void rGL_SetZBufferWriteMask (int state)
{
	if (state)
	{
		glDepthMask (GL_TRUE);
	}
	else
	{
		glDepthMask (GL_FALSE);
	}
}

// ====================
// rGL_SetAlphaValue
// ====================
//
// Sets the constant alpha value
void rGL_SetAlphaValue (ubyte val)
{
	OpenGL_state.cur_alpha=val;
	rGL_SetAlphaMultiplier();
}

// ======================
// rGL_SetAlphaMultiplier
// ======================
//
// Sets the alpha multiply factor
void rGL_SetAlphaMultiplier ()
{
	Alpha_multiplier=rGL_GetAlphaMultiplier();
}

// ======================
// rGL_GetAlphaMultiplier
// ======================
//
// returns the alpha that we should use
float rGL_GetAlphaMultiplier ()
{
	switch (OpenGL_state.cur_alpha_type)
	{
		case AT_ALWAYS:
			return 1.0f;
		case AT_CONSTANT:
			return OpenGL_state.cur_alpha/255.0f;
		case AT_TEXTURE:
			return 1.0f;
		case AT_CONSTANT_TEXTURE:
			return OpenGL_state.cur_alpha/255.0f;
		case AT_VERTEX:
			return 1.0f;
		case AT_CONSTANT_TEXTURE_VERTEX:
		case AT_CONSTANT_VERTEX:
			return OpenGL_state.cur_alpha/255.0f;
		case AT_TEXTURE_VERTEX:
			return 1.0f;
		case AT_LIGHTMAP_BLEND:
		case AT_LIGHTMAP_BLEND_SATURATE:
			return OpenGL_state.cur_alpha/255.0f;
		case AT_SATURATE_TEXTURE:
			return OpenGL_state.cur_alpha/255.0f;
		case AT_SATURATE_VERTEX:
			return 1.0f;
		case AT_SATURATE_CONSTANT_VERTEX:
			return OpenGL_state.cur_alpha/255.0f;
		case AT_SATURATE_TEXTURE_VERTEX:
			return 1.0f;
		case AT_SPECULAR:
			return 1.0f;
		default:
			return 0;
	}
}

// =====================
// rGL_SetLightingState
// =====================
//
// Sets the lighting state of opengl
void rGL_SetLightingState (light_state state)
{
	if (state==OpenGL_state.cur_light_state)
		return;	// No redundant state setting

	switch (state)
	{
		case LS_NONE:
			glShadeModel (GL_SMOOTH);
			OpenGL_state.cur_light_state=LS_NONE;
			break;
		case LS_FLAT_GOURAUD:
			glShadeModel (GL_SMOOTH);
			OpenGL_state.cur_light_state=LS_FLAT_GOURAUD;
			break;
		case LS_GOURAUD:
		case LS_PHONG:
			glShadeModel (GL_SMOOTH);
			OpenGL_state.cur_light_state=LS_GOURAUD;
			break;
		default:
			break;
	}
}

// =======================
// rGL_SetColorModel
// =======================
//
// Sets the opengl color model (either rgb or mono)
void rGL_SetColorModel (color_model state)
{
	switch (state)
	{
		case CM_MONO:
			OpenGL_state.cur_color_model=CM_MONO;
			break;
		case CM_RGB:
			OpenGL_state.cur_color_model=CM_RGB;
			break;
		default:
			break;
	}
}

// =====================
// rGL_SetFiltering
// =====================
//
// Sets the state of bilinear filtering for our textures
void rGL_SetFiltering (sbyte state)
{
	OpenGL_state.cur_bilinear_state=state;
}

// ========================
// rGL_SetZBufferState
// ========================
//
// Sets the state of zbuffering to on or off
void rGL_SetZBufferState  (sbyte state)
{
	if (state==OpenGL_state.cur_zbuffer_state)
		return;	// No redundant state setting

	OpenGL_state.cur_zbuffer_state=state;


	if (state)
	{
		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);
	}
	else
		glDisable (GL_DEPTH_TEST);
}


// =======================
// rGL_SetZValues
// =======================
//
// Sets the z clip plane values
void rGL_SetZValues (float nearz,float farz)
{
	OpenGL_state.cur_near_z=nearz;
	OpenGL_state.cur_far_z=farz;

	glDepthRange (0,farz);
}

// =======================
// rGL_ClearScreen
// =======================
//
// Clears the display to a specified color
void rGL_ClearScreen (ddgr_color color)
{
	int r=(color>>16 & 0xFF);
	int g=(color>>8 & 0xFF);
	int b=(color & 0xFF);

	glClearColor ((float)r/255.0f,(float)g/255.0f,(float)b/255.0f,0);

	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

// =======================
// rGL_ClearZBuffer
// =======================
//
// Clears the zbuffer
void rGL_ClearZBuffer ()
{
	glClear (GL_DEPTH_BUFFER_BIT);
}

// =======================
// rGL_FillRect
// =======================
//
// Fills a rectangle on the display
void rGL_FillRect (ddgr_color color,int x1,int y1,int x2,int y2)
{
	int r=GR_COLOR_RED(color);
	int g=GR_COLOR_GREEN(color);
	int b=GR_COLOR_BLUE(color);
	
	glEnable (GL_SCISSOR_TEST);
	glScissor (x1+OpenGL_state.clip_x1,y1+OpenGL_state.clip_y1,x2+OpenGL_state.clip_x1,y2+OpenGL_state.clip_y1);
	glClearColor ((float)r/255.0f,(float)g/255.0f,(float)b/255.0f,0);
	glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glScissor (OpenGL_state.clip_x1,OpenGL_state.clip_y1,OpenGL_state.clip_x2,OpenGL_state.clip_y2);
	glDisable (GL_SCISSOR_TEST);
}

// ====================
// rGL_SetPixel
// ====================
//
// Sets a pixel on the display
void rGL_SetPixel (ddgr_color color,int x,int y)
{
	int r=(color>>16 & 0xFF);
	int g=(color>>8 & 0xFF);
	int b=(color & 0xFF);
	
	glColor3ub (r,g,b);
	
	glBegin (GL_POINTS);
	glVertex2i (x,y);
	glEnd();
}

// =======================
// rGL_GetPixel
// =======================
//
// Returns the pixel color at x,y
ddgr_color rGL_GetPixel (int x,int y)
{
	ddgr_color color[4];

	glReadPixels (x,(OpenGL_state.screen_height-1)-y,1,1,GL_RGBA,GL_UNSIGNED_BYTE,(GLvoid *)color);

	return color[0];
}

// =========================
// rGL_BeginFrame
// =========================
//
// Starts a rendering frame
void rGL_BeginFrame (int x1,int y1,int x2,int y2,int clear_flags)
{
	if (clear_flags & RF_CLEAR_ZBUFFER)
		glClear(GL_DEPTH_BUFFER_BIT);
		
	OpenGL_state.clip_x1=x1;
	OpenGL_state.clip_y1=y1;
	OpenGL_state.clip_x2=x2;
	OpenGL_state.clip_y2=y2;

	glScissor (OpenGL_state.clip_x1,OpenGL_state.clip_y1,OpenGL_state.clip_x2,OpenGL_state.clip_y2);
}

// ===================
// rGL_EndFrame
// ===================
//
// Ends a rendering frame
void rGL_EndFrame()
{
	
}

// ======================
// rGL_GetProjectionParameters
// ======================
//
// Fills in projection variables
void rGL_GetProjectionParameters (int *width,int *height)
{
	*width=OpenGL_state.clip_x2-OpenGL_state.clip_x1;
	*height=OpenGL_state.clip_y2-OpenGL_state.clip_y1;
}


// ======================
// rGL_DrawPolygon
// ======================
//
// Takes nv vertices and draws the polygon defined by those vertices.  Uses bitmap "handle"
// as a texture
void rGL_DrawPolygon (int handle,g3Point **p,int nv,int map_type)
{
	g3Point *pnt;
	int i,fr,fg,fb;
	float alpha;
	vector *vertp;
	color_array *colorp;
	tex_array *texp;
	
	ASSERT (nv<100);

	
	//if (OpenGL_state.cur_texture_quality==0)
	//{
	//	opengl_DrawFlatPolygon (p,nv);
	//	return;
	//}

	int x_add=OpenGL_state.clip_x1;
	int y_add=OpenGL_state.clip_y1;

	if (OpenGL_state.cur_light_state==LS_NONE)
	{
		fr=GR_COLOR_RED(OpenGL_state.cur_color);
		fg=GR_COLOR_GREEN(OpenGL_state.cur_color);
		fb=GR_COLOR_BLUE(OpenGL_state.cur_color);
	}
	
	// make sure our bitmap is ready to be drawn
	rGL_MakeBitmapCurrent (handle,map_type,0);
	rGL_MakeWrapTypeCurrent (handle,map_type,0);
	rGL_MakeFilterTypeCurrent(handle,map_type,0);
	
	alpha=Alpha_multiplier;

	vertp=&GL_verts[0];
	texp=&GL_tex_coords[0];
	colorp=&GL_colors[0];

	// Specify our coordinates
	for (i=0;i<nv;i++,vertp++,texp++,colorp++)
	{
		pnt=p[i];
			
		if (OpenGL_state.cur_alpha_type & ATF_VERTEX)
			alpha=pnt->p3_a*Alpha_multiplier;
	
		// If we have a lighting model, apply the correct lighting!
		if (OpenGL_state.cur_light_state!=LS_NONE)
		{
			// Do lighting based on intesity (MONO) or colored (RGB)
			if (OpenGL_state.cur_color_model==CM_MONO)
			{
				colorp->r=pnt->p3_l;
				colorp->g=pnt->p3_l;
				colorp->b=pnt->p3_l;
				colorp->a=alpha;
			}
			else
			{
				colorp->r=pnt->p3_r;
				colorp->g=pnt->p3_g;
				colorp->b=pnt->p3_b;
				colorp->a=alpha;
			}
		}
		else
		{
			colorp->r=1;
			colorp->g=1;
			colorp->b=1;
			colorp->a=alpha;
		}
	
		// Texture this polygon!
		float texw=1.0/(pnt->p3_z+Z_bias);
		texp->s=pnt->p3_u*texw;
		texp->t=pnt->p3_v*texw;
		texp->r=0;
		texp->w=texw;
	
		// Finally, specify a vertex
		vertp->x=pnt->p3_sx+x_add;
		vertp->y=pnt->p3_sy+y_add;

		float z=(pnt->p3_z+Z_bias)/OpenGL_state.cur_far_z;
		if (z>1)
			z=1;
		vertp->z=-z;
		
	}
	
	// And draw!
	glDrawArrays (GL_POLYGON,0,nv);
		
	// If there is a lightmap to draw, draw it as well
	if (Overlay_type!=OT_NONE)
	{
		return;	// Temp fix until I figure out whats going on
	}
}

// =====================
// rGL_SetFlatColor
// =====================
//
//	Sets the current color
void rGL_SetFlatColor (ddgr_color color)
{
	OpenGL_state.cur_color=color;
}

// ======================
// rGL_SetTextureType
// ======================
//
// Sets texture 
void rGL_SetTextureType (texture_type state)
{
	if (state==OpenGL_state.cur_texture_type)
		return;	// No redundant state setting

	switch (state)
	{
		case TT_FLAT:
			glDisable (GL_TEXTURE_2D);
			OpenGL_state.cur_texture_quality=0;
			break;
		case TT_LINEAR:
		case TT_LINEAR_SPECIAL:
		case TT_PERSPECTIVE:
		case TT_PERSPECTIVE_SPECIAL:
			glEnable (GL_TEXTURE_2D);
			OpenGL_state.cur_texture_quality=2;
			break;
		default:
			//@@@Int3();	// huh? Get Jason
			break;
	}

	OpenGL_state.cur_texture_type=state;
}

// =================
// rGL_Init
// =================
//
// Sets up our OpenGL rendering context
// Returns 1 if ok, 0 if something bad
int rGL_Init(oeApplication *app,renderer_preferred_state *pref_state)
{
	int width,height;
	int retval=1;

	mprintf ((0,"Setting up opengl mode!\n"));

	if (pref_state)
		OpenGL_preferred_state=*pref_state;

// start OS-specific code
	memset (&OpenGL_state,0,sizeof(rendering_state));

	if (app!=NULL)
	{
		hOpenGLWnd = (HWND)((oeWin32Application *)app)->m_hWnd;
	}
	
	hOpenGLDC = GetDC(hOpenGLWnd);

	RECT rect;
	GetWindowRect ((HWND)hOpenGLWnd,&rect);
	width=abs(rect.right-rect.left);
	height=abs(rect.bottom-rect.top);

	OpenGL_state.screen_width=width;
	OpenGL_state.screen_height=height;

	if (!rGL_Setup(hOpenGLDC))
	{
		rGL_Close();
		return 0;
	}

	// Save gamma values for later
	//@@@@GetDeviceGammaRamp(hOpenGLDC,(LPVOID)Saved_gamma_values);

	// Get some info
	rGL_GetInformation();

	mprintf ((0,"Setting up projection matrix\n"));
	
	glMatrixMode(GL_PROJECTION);	
	glLoadIdentity();
	glOrtho(0.0f, (GLfloat) width, (GLfloat) height,0,0,1);
	glViewport(0, 0, width, height);   
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

// end
	int i;
	rGL_InitCache();

  	//@@@@@rGL_GetDLLFunctions();

	OpenGL_packed_pixels=rGL_CheckExtension ("GL_EXT_packed_pixels");

	if (OpenGL_packed_pixels)
	{
		opengl_packed_Upload_data=(ushort *)mem_malloc (256*256*2);
		opengl_packed_Translate_table=(ushort *)mem_malloc (65536*2);
		opengl_packed_4444_translate_table=(ushort *)mem_malloc (65536*2);

		mprintf ((0,"Building packed OpenGL translate table...\n"));

		for (i=0;i<65536;i++)
		{
			int r=(i>>10) & 0x1f;
			int g=(i>>5) & 0x1f;
			int b=i & 0x1f;
			
			ushort pix;
		
			if (!(i & OPAQUE_FLAG))
				pix=0;
			else
				pix=(r<<11) | (g<<6) | (b<<1) | 1;

			opengl_packed_Translate_table[i]=pix;

			// 4444 table
			int a=(i>>12) & 0xf;
			r=(i>>8) & 0xf;
			g=(i>>4) & 0xf;
			b=i & 0xf;

			a=0xf;

			pix=(r<<12) | (g<<8) | (b<<4) | a;

			opengl_packed_4444_translate_table[i]=pix;

		}
	}
	else
	{
		opengl_Upload_data=(uint *)mem_malloc (256*256*4);
		opengl_Translate_table=(uint *)mem_malloc (65536*4);
		opengl_4444_translate_table=(uint *)mem_malloc (65536*4);

		mprintf ((0,"Building OpenGL translate table...\n"));

		for (i=0;i<65536;i++)
		{
			uint pix=GR_16_TO_COLOR(i);
			int r=GR_COLOR_RED(pix);
			int g=GR_COLOR_GREEN(pix);
			int b=GR_COLOR_BLUE(pix);
		
			if (!(i & OPAQUE_FLAG))
				pix=0;
			else
				pix=(255<<24) | (b<<16) | (g<<8) | (r);

			opengl_Translate_table[i]=pix;

			// Do 4444
			int a=(i>>12) & 0xf;
			r=(i>>8) & 0xf;
			g=(i>>4) & 0xf;
			b=i & 0xf;

			a=0xf;
		
			pix=(a<<24) | (b<<16) | (g<<8) | (r);

			opengl_4444_translate_table[i]=pix;
		}
	}

	rGL_SetDefaults();
	
	OpenGL_state.initted = 1;
	
	mprintf ((0,"OpenGL initialization at %d x %d was successful.\n",width,height));

	return retval;
}

// Releases the rendering context
void rGL_Close()
{
	uint *delete_list=(uint *)mem_malloc (Cur_texture_object_num*sizeof(int));
	ASSERT (delete_list);
	for (int i=1;i<Cur_texture_object_num;i++)
		delete_list[i]=i;

	if (Cur_texture_object_num>1)
		glDeleteTextures (Cur_texture_object_num,(const uint *)delete_list);

	mem_free (delete_list);

// start OS-specific code
  	/*if (dwglMakeCurrent)
		dwglMakeCurrent(NULL, NULL);

	if (dwglDeleteContext)
		dwglDeleteContext(ResourceContext);*/

	// Change our display back
//	if (!WindowGL)
//		ChangeDisplaySettings(NULL,0);
// end

	if (OpenGL_packed_pixels)
	{
		if (opengl_packed_Upload_data)
			mem_free (opengl_packed_Upload_data);
		if (opengl_packed_Translate_table)
			mem_free (opengl_packed_Translate_table);
		if (opengl_packed_4444_translate_table)
			mem_free (opengl_packed_4444_translate_table);
		opengl_packed_Upload_data=NULL;
		opengl_packed_Translate_table=NULL;
		opengl_packed_4444_translate_table=NULL;
	}
	else
	{
		if (opengl_Upload_data)
			mem_free (opengl_Upload_data);
		if (opengl_Translate_table)
			mem_free (opengl_Translate_table);
		if (opengl_4444_translate_table)
			mem_free (opengl_4444_translate_table);
		opengl_Upload_data=NULL;
		opengl_Translate_table=NULL;
		opengl_4444_translate_table=NULL;
	}

	if (OpenGL_cache_initted)
	{
		mem_free (OpenGL_lightmap_remap);
		mem_free (OpenGL_bitmap_remap);
		mem_free (OpenGL_lightmap_states);
		mem_free (OpenGL_bitmap_states);
		OpenGL_cache_initted=0;
	}

	// Restore gamma values
	//@@@@@SetDeviceGammaRamp(hOpenGLDC,(LPVOID)Saved_gamma_values);
	ReleaseDC(hOpenGLWnd, hOpenGLDC);

	OpenGL_state.initted = 0;
}


// Takes our 16bit format and converts it into the memory scheme that OpenGL wants
void rGL_TranslateBitmapToOpenGL (int texnum,int bm_handle,int map_type,int replace,int tn)
{
	ushort *bm_ptr;

	int w,h;
	int size;

/*$
	if (map_type==MAP_TYPE_LIGHTMAP)
	{
		if (GameLightmaps[bm_handle].flags & LF_BRAND_NEW)
			replace=0;

		bm_ptr=lm_data (bm_handle);
		GameLightmaps[bm_handle].flags &=~(LF_CHANGED|LF_BRAND_NEW);

		w=lm_w(bm_handle);
		h=lm_h(bm_handle);
		size=GameLightmaps[bm_handle].square_res;
	}
	else
*/
	{
		if (GameBitmaps[bm_handle].flags & BF_BRAND_NEW)
			replace=0;

		bm_ptr=bm_data (bm_handle,0);
		GameBitmaps[bm_handle].flags &=~(BF_CHANGED|BF_BRAND_NEW);
		w=bm_w(bm_handle,0);
		h=bm_h(bm_handle,0);
		size=w;
	}

	if (OpenGL_last_bound[tn]!=texnum)
	{
		glBindTexture (GL_TEXTURE_2D,texnum);
		OpenGL_sets_this_frame[0]++;
		OpenGL_last_bound[tn]=texnum;
	}

	int i;
	
	if (OpenGL_packed_pixels)
	{
/*$
		if (map_type==MAP_TYPE_LIGHTMAP)
		{	
			ushort *left_data=(ushort *)opengl_packed_Upload_data;
			int bm_left=0;
						
			for (int i=0;i<h;i++,left_data+=size,bm_left+=w)
			{
				ushort *dest_data=left_data;
				for (int t=0;t<w;t++)
				{
					*dest_data++=opengl_packed_Translate_table[bm_ptr[bm_left+t]];
				}
			}

			if (replace)
			{
				glTexSubImage2D (GL_TEXTURE_2D,0,0,0,size,size,GL_RGBA,UNSIGNED_SHORT_5_5_5_1_EXT,opengl_packed_Upload_data);
			}
			else
				glTexImage2D (GL_TEXTURE_2D,0,GL_RGB5_A1,size,size,0,GL_RGBA,UNSIGNED_SHORT_5_5_5_1_EXT,opengl_packed_Upload_data);
		}
		else
*/
		{
			int limit=0;

			if (bm_mipped(bm_handle))
				limit=NUM_MIP_LEVELS+3;
			else
				limit=1;

			for (int m=0;m<limit;m++)
			{
				if (m<NUM_MIP_LEVELS)
				{
					bm_ptr=bm_data (bm_handle,m);
					w=bm_w(bm_handle,m);
					h=bm_h(bm_handle,m);
				}
				else
				{
					bm_ptr=bm_data (bm_handle,NUM_MIP_LEVELS-1);
					w=bm_w(bm_handle,NUM_MIP_LEVELS-1);
					h=bm_h(bm_handle,NUM_MIP_LEVELS-1);

					w>>=m-(NUM_MIP_LEVELS-1);
					h>>=m-(NUM_MIP_LEVELS-1);

					if (w<1)
						continue;

				}
				
				if (bm_format(bm_handle)==BITMAP_FORMAT_4444)
				{
					// Do 4444

					if (bm_mipped(bm_handle))
					{
						for (i=0;i<w*h;i++)
							opengl_packed_Upload_data[i]=0xf|(opengl_packed_4444_translate_table[bm_ptr[i]]);
					}
					else
					{
						for (i=0;i<w*h;i++)
							opengl_packed_Upload_data[i]=opengl_packed_4444_translate_table[bm_ptr[i]];
					}

					if (replace)
						glTexSubImage2D (GL_TEXTURE_2D,m,0,0,w,h,GL_RGBA,UNSIGNED_SHORT_4_4_4_4_EXT,opengl_packed_Upload_data);
					else
						glTexImage2D (GL_TEXTURE_2D,m,GL_RGBA4,w,h,0,GL_RGBA,UNSIGNED_SHORT_4_4_4_4_EXT,opengl_packed_Upload_data);
				}
				else
				{
					// Do 1555
					for (i=0;i<w*h;i++)
						opengl_packed_Upload_data[i]=opengl_packed_Translate_table[bm_ptr[i]];

					if (replace)
						glTexSubImage2D (GL_TEXTURE_2D,m,0,0,w,h,GL_RGBA,UNSIGNED_SHORT_5_5_5_1_EXT,opengl_packed_Upload_data);
					else
						glTexImage2D (GL_TEXTURE_2D,m,GL_RGB5_A1,w,h,0,GL_RGBA,UNSIGNED_SHORT_5_5_5_1_EXT,opengl_packed_Upload_data);
				}
			}
		}

		
		
	}
	else
	{
/*$
		if (map_type==MAP_TYPE_LIGHTMAP)
		{	
			uint *left_data=(uint *)opengl_Upload_data;
			int bm_left=0;
		
			for (int i=0;i<h;i++,left_data+=size,bm_left+=w)
			{
				uint *dest_data=left_data;
				for (int t=0;t<w;t++)
				{
					*dest_data++=opengl_Translate_table[bm_ptr[bm_left+t]];
				}
			}

			if (replace)
				glTexSubImage2D (GL_TEXTURE_2D,0,0,0,size,size,GL_RGBA,GL_UNSIGNED_BYTE,opengl_Upload_data);
			else
				glTexImage2D (GL_TEXTURE_2D,0,GL_RGBA,size,size,0,GL_RGBA,GL_UNSIGNED_BYTE,opengl_Upload_data);
		}
		else
*/
		{
			int limit=0;

			if (bm_mipped(bm_handle))
				limit=NUM_MIP_LEVELS;
			else
				limit=1;

			for (int m=0;m<limit;m++)
			{
				bm_ptr=bm_data (bm_handle,m);
				w=bm_w(bm_handle,m);
				h=bm_h(bm_handle,m);

				if (bm_format(bm_handle)==BITMAP_FORMAT_4444)
				{
					// Do 4444

					if (bm_mipped(bm_handle))
					{
						for (i=0;i<w*h;i++)
							opengl_Upload_data[i]=(255<<24)|opengl_4444_translate_table[bm_ptr[i]];
					}
					else
					{
						for (i=0;i<w*h;i++)
							opengl_Upload_data[i]=opengl_4444_translate_table[bm_ptr[i]];
					}
				}
				else
				{
					// Do 1555

					for (i=0;i<w*h;i++)
						opengl_Upload_data[i]=opengl_Translate_table[bm_ptr[i]];
				}

				if (replace)
					glTexSubImage2D (GL_TEXTURE_2D,m,0,0,w,h,GL_RGBA,GL_UNSIGNED_BYTE,opengl_Upload_data);
				else
					glTexImage2D (GL_TEXTURE_2D,m,GL_RGBA,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,opengl_Upload_data);
				
			}


			
		}
		
		
	}


	//mprintf ((1,"Doing slow upload to opengl!\n"));

//$	if (map_type==MAP_TYPE_LIGHTMAP)
//$		GameLightmaps[bm_handle].flags&=~LF_LIMITS;

}

// Utilizes a LRU cacheing scheme to select/upload textures the opengl driver
int rGL_MakeBitmapCurrent (int handle,int map_type,int tn)
{
	int w,h;
	int texnum;

/*$
	if (map_type==MAP_TYPE_LIGHTMAP)
	{
		w=GameLightmaps[handle].square_res;
		h=GameLightmaps[handle].square_res;
	}
	else
*/
	{
		w=bm_w(handle,0);
		h=bm_h(handle,0);
	}

	if (w!=h)
	{
		mprintf ((0,"Can't use non-square textures with OpenGL!\n"));
		return 0;
	}

	// See if the bitmaps is already in the cache
/*$
	if (map_type==MAP_TYPE_LIGHTMAP)
	{
		if (OpenGL_lightmap_remap[handle]==65535)
		{
			texnum=opengl_MakeTextureObject (tn);
			SET_WRAP_STATE(OpenGL_lightmap_states[handle],1);
			SET_FILTER_STATE(OpenGL_lightmap_states[handle],0);
			OpenGL_lightmap_remap[handle]=texnum;
			rGL_TranslateBitmapToOpenGL(texnum,handle,map_type,0,tn);
		}
		else
		{
			texnum=OpenGL_lightmap_remap[handle];
			if (GameLightmaps[handle].flags & LF_CHANGED)
				rGL_TranslateBitmapToOpenGL(texnum,handle,map_type,1,tn);
		}
	}
	else
*/
	{
		if (OpenGL_bitmap_remap[handle]==65535)
		{
			texnum=rGL_MakeTextureObject (tn);
			SET_WRAP_STATE(OpenGL_bitmap_states[handle],1);
			SET_FILTER_STATE(OpenGL_bitmap_states[handle],0);
			OpenGL_bitmap_remap[handle]=texnum;
			rGL_TranslateBitmapToOpenGL(texnum,handle,map_type,0,tn);
		}
		else
		{
			texnum=OpenGL_bitmap_remap[handle];
			if (GameBitmaps[handle].flags & BF_CHANGED)
				rGL_TranslateBitmapToOpenGL(texnum,handle,map_type,1,tn);
		}
	}

	if (OpenGL_last_bound[tn]!=texnum)
	{
		glBindTexture (GL_TEXTURE_2D,texnum);
		OpenGL_last_bound[tn]=texnum;
		OpenGL_sets_this_frame[0]++;
	}

	return 1;
}


// Sets up an appropriate wrap type for the current bound texture
void rGL_MakeWrapTypeCurrent (int handle,int map_type,int tn)
{
	int uwrap;
	wrap_type dest_wrap;

	if (tn==1)
		dest_wrap=WT_CLAMP;
	else
		dest_wrap=OpenGL_state.cur_wrap_type;

/*$
	if (map_type==MAP_TYPE_LIGHTMAP)
		uwrap=GET_WRAP_STATE(OpenGL_lightmap_states[handle]);
	else
*/
		uwrap=GET_WRAP_STATE(OpenGL_bitmap_states[handle]);

	if (uwrap==dest_wrap)
		return;

	OpenGL_sets_this_frame[1]++;
		
	if (OpenGL_state.cur_wrap_type==WT_CLAMP)
	{
		glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP);

	}
	else if (OpenGL_state.cur_wrap_type==WT_WRAP_V)
	{
		glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP);
		glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	}
	else
	{
		glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	}

/*$
	if (map_type==MAP_TYPE_LIGHTMAP)
	{
		SET_WRAP_STATE (OpenGL_lightmap_states[handle],dest_wrap);
	}
	else
*/
	{
		SET_WRAP_STATE (OpenGL_bitmap_states[handle],dest_wrap);
	}

}

// Chooses the correct filter type for the currently bound texture
void rGL_MakeFilterTypeCurrent (int handle,int map_type,int tn)
{
	int magf;
	sbyte dest_state;

/*$
	if (map_type==MAP_TYPE_LIGHTMAP)
	{
		magf=GET_FILTER_STATE(OpenGL_lightmap_states[handle]);
		dest_state=1;
	}
	else
*/
	{
		magf=GET_FILTER_STATE(OpenGL_bitmap_states[handle]);
		dest_state=OpenGL_preferred_state.filtering;
		if (!OpenGL_state.cur_bilinear_state)
			dest_state=0;
	}

	if (magf==dest_state)
		return;

	OpenGL_sets_this_frame[2]++;

	if (dest_state)
	{
		if (map_type==MAP_TYPE_BITMAP && bm_mipped(handle))
		{
			glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		}
		else
		{
			glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		}
	}
	else
	{
		if (map_type==MAP_TYPE_BITMAP && bm_mipped(handle))
		{
			//glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST_MIPMAP_NEAREST);
			glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
		}
		else
		{
			glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
			glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		}
	}

/*$
	if (map_type==MAP_TYPE_LIGHTMAP)
	{
		SET_FILTER_STATE (OpenGL_lightmap_states[handle],dest_state);
	}
	else
*/
	{
		SET_FILTER_STATE (OpenGL_bitmap_states[handle],dest_state);
	}

}


// Gets some specific information about this particular flavor of opengl
void rGL_GetInformation ()
{
	mprintf ((0,"OpenGL Vendor: %s\n",glGetString(GL_VENDOR)));
	mprintf ((0,"OpenGL Renderer: %s\n",glGetString(GL_RENDERER)));
	mprintf ((0,"OpenGL Version: %s\n",glGetString(GL_VERSION)));
	mprintf ((0,"OpenGL Extensions: %s\n",glGetString (GL_EXTENSIONS)));
}

int rGL_MakeTextureObject (int tn)
{
	int num=Cur_texture_object_num;

	Cur_texture_object_num++;

	glBindTexture (GL_TEXTURE_2D,num);
	glPixelStorei (GL_UNPACK_ALIGNMENT,2);

	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);

	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameteri (GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	
	//glTexEnvf (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	return num;
}

int rGL_InitCache ()
{
	OpenGL_bitmap_remap=(ushort *)mem_malloc (MAX_BITMAPS*2);
	ASSERT (OpenGL_bitmap_remap);
//$	OpenGL_lightmap_remap=(ushort *)mem_malloc (MAX_LIGHTMAPS*2);
//$	ASSERT (OpenGL_lightmap_remap);

	OpenGL_bitmap_states=(ubyte *)mem_malloc (MAX_BITMAPS);
	ASSERT (OpenGL_bitmap_states);
//	OpenGL_lightmap_states=(ubyte *)mem_malloc (MAX_LIGHTMAPS);
//$	ASSERT (OpenGL_lightmap_states);

	Cur_texture_object_num=1;
	// Setup textures and cacheing
	for (int i=0;i<MAX_BITMAPS;i++)
	{
		OpenGL_bitmap_remap[i]=65535;
		OpenGL_bitmap_states[i]=255;
		GameBitmaps[i].flags|=BF_CHANGED|BF_BRAND_NEW;
	}
//$	for (i=0;i<MAX_LIGHTMAPS;i++)
//$	{
//$		OpenGL_lightmap_remap[i]=65535;
//$		OpenGL_lightmap_states[i]=255;
//$		GameLightmaps[i].flags|=LF_CHANGED|LF_BRAND_NEW;
//$	}

	glTexEnvf (GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	OpenGL_cache_initted=1;
	return 1;
}

// Sets default states for our renderer
void rGL_SetDefaults()
{
	mprintf ((0,"Setting states\n"));

	OpenGL_state.cur_color=0x00FFFFFF;
	OpenGL_state.cur_bilinear_state=-1;
	OpenGL_state.cur_zbuffer_state=-1;
	OpenGL_state.cur_texture_quality=-1;
	OpenGL_state.cur_light_state=LS_GOURAUD;
	OpenGL_state.cur_color_model=CM_MONO;
	OpenGL_state.cur_bilinear_state=-1;
	OpenGL_state.cur_alpha_type=AT_TEXTURE;

	// Enable some states
	glAlphaFunc (GL_GREATER,0);
	glEnable (GL_ALPHA_TEST);
	glEnable (GL_BLEND);
	glEnable (GL_DITHER);

	rGL_SetAlphaType (AT_ALWAYS);
	rGL_SetAlphaValue (255);
	rGL_SetFiltering (1);
	rGL_SetLightingState (LS_NONE);
	rGL_SetTextureType (TT_FLAT);
	rGL_SetColorModel (CM_RGB);
	rGL_SetZBufferState (1);
	rGL_SetZValues (0,3000);
	rGL_SetGammaValue (OpenGL_preferred_state.gamma);
	//$$opengl_last_bound[0]=9999999;
	//$$opengl_last_bound[1]=9999999;
	
	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_COLOR_ARRAY);
	glEnableClientState (GL_TEXTURE_COORD_ARRAY);

	glVertexPointer (3,GL_FLOAT,0,GL_verts);
	glColorPointer (4,GL_FLOAT,0,GL_colors);
	glTexCoordPointer (4,GL_FLOAT,0,GL_tex_coords);
	
	glHint (GL_PERSPECTIVE_CORRECTION_HINT,GL_FASTEST);
	glHint (GL_FOG_HINT,GL_FASTEST);
}

// start OS-specific code
// Check for OpenGL support, 
static HMODULE glDllhandle = nullptr;
static GLADapiproc opengl_GLADLoad(const char* name)
{
	if (!glDllhandle)
	{
		glDllhandle = LoadLibrary("opengl32.dll");
		if (!glDllhandle)
			Error("opengl_GLADLoad: failed to load opengl32.dll!");
	}
	void* ptr = wglGetProcAddress(name);
	//I love OpenGL btw
	if (!ptr)
	{
		ptr = GetProcAddress(glDllhandle, name);
	}
	return (GLADapiproc)ptr;
}


int rGL_Setup(HDC glhdc)
{
	// Finds an acceptable pixel format to render to
	PIXELFORMATDESCRIPTOR pfd, pfd_copy;
	int pf;
	
	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize        = sizeof(pfd);
	pfd.nVersion     = 1;
	pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;
	pfd.iPixelType   = PFD_TYPE_RGBA;

	pfd.cColorBits   = 32;
	pfd.cDepthBits   = 32;

	
	// Find the user's "best match" PFD 
	pf = ChoosePixelFormat(glhdc, &pfd);
	if (pf == 0) 
	{
		Int3();
		//FreeLibrary(opengl_dll_handle);
		return NULL;
	} 

	mprintf ((0,"Choose pixel format successful!\n"));
 
	// Try and set the new PFD
	if (SetPixelFormat(glhdc, pf, &pfd) == FALSE) 
	{
		DWORD ret=GetLastError();
		Int3();
		//FreeLibrary(opengl_dll_handle);
		return NULL;
	}

	mprintf ((0,"SetPixelFormat successful!\n"));

	// Get a copy of the newly set PFD
	if(DescribePixelFormat(glhdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd_copy)==0)
	{
		Int3();
		//FreeLibrary(opengl_dll_handle);
		return NULL;
	}

	// Check the returned PFD to see if it is hardware accelerated
/*GW
	if((pfd_copy.dwFlags & PFD_GENERIC_ACCELERATED)==0 && (pfd_copy.dwFlags & PFD_GENERIC_FORMAT)!=0) {
		Int3();
		//FreeLibrary(opengl_dll_handle);
		return NULL;
	}
*/

	// Create an OpenGL context, and make it the current context
	ResourceContext = wglCreateContext((HDC)glhdc);
	if(ResourceContext==NULL) {
		DWORD ret=GetLastError();
		//FreeLibrary(opengl_dll_handle);
		Int3();
		return NULL;   
	}

	ASSERT (ResourceContext!=NULL);
	mprintf ((0,"Making context current\n"));
	wglMakeCurrent((HDC)glhdc, ResourceContext);

	if (!Already_loaded)
	{
		if (!gladLoadGL(opengl_GLADLoad))
		{
			//rend_SetErrorMessage("Failed to load opengl dll!\n");
			Int3();
			return 0;
		}
	}

	Already_loaded=1;
	
	return 1;
	
}
// end


// returns true if the passed in extension name is supported
int rGL_CheckExtension( char *extName )
{
	char *p = (char *) glGetString(GL_EXTENSIONS);
	if(!p)
		return 0;

	char *end;
	int extNameLen;

	extNameLen = strlen(extName);
	end = p + strlen(p);
    
	while (p < end) 
	{
		int n = strcspn(p, " ");
		if ((extNameLen == n) && (strncmp(extName, p, n) == 0)) 
			return 1;
	
		p += (n + 1);
	}
	return 0;
}

// Sets the gamma correction value
void rGL_SetGammaValue (float val)
{
	return;
}

