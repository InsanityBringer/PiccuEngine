/*
* Descent 3: Piccu Engine
* Copyright (C) 2024 Parallax Software
* Copyright (C) 2024 SaladBadger
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
#pragma once

#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#include <algorithm>
#include <glad/gl.h>
#include "wglext.h"
#define DD_ACCESS_RING //need direct access to some stuff
#include "application.h"
#include "3d.h"
#include "renderer.h"
#include "vecmat.h"
#include "bitmap.h"
#include "lightmap.h"
#include "mem.h"
#include "mono.h"
#include "pserror.h"
#include "gl_shared.h"
#include "IRenderer.h"

struct compat_color_array
{
	float r, g, b, a;
};

struct compat_tex_array
{
	float s, t, r, w;
};

constexpr int NUM_GL1_FBOS = 2;
class GLCompatibilityRenderer : public IRenderer
{
	//MAIN
	oeApplication* ParentApplication = nullptr;

	bool OpenGL_multitexture_state = false;
	bool OpenGL_packed_pixels = false;
	bool Fast_test_render = false;

	Framebuffer framebuffers[NUM_GL1_FBOS];
	int framebuffer_current_draw = 0;

	unsigned int framebuffer_blit_x = 0, framebuffer_blit_y = 0, framebuffer_blit_w = 0, framebuffer_blit_h = 0;

	ShaderProgram blitshader;
	GLint blitshader_gamma = -1;

	int OpenGL_last_frame_polys_drawn = 0;
	int OpenGL_last_frame_verts_processed = 0;
	int OpenGL_last_uploaded = 0;

	//DRAW
	vector GL_verts[100];
	compat_color_array GL_colors[100];
	compat_tex_array GL_tex_coords[100];
	compat_tex_array GL_tex_coords2[100];

	float OpenGL_Alpha_factor = 1.0f;
	float Alpha_multiplier = 1.0f;

	int OpenGL_polys_drawn = 0;
	int OpenGL_verts_processed = 0;

	int Overlay_map = -1;
	int Bump_map = 0;
	int Bumpmap_ready = 0;
	ubyte Overlay_type = OT_NONE;

	bool OpenGL_blending_on = true;

	GLuint drawbuffer = 0;
	//The next committed vertex is where to start writing vertex data to the buffer
	GLuint nextcommittedvertex = 0;
	ShaderProgram drawshaders[8];
	int lastdrawshader = -1;

	GLuint drawvao = 0;
	void* drawbuffermap = 0;

	//IMAGE
	ubyte opengl_Framebuffer_ready = 0;
	chunked_bitmap opengl_Chunked_bitmap;

	ushort* OpenGL_bitmap_remap = nullptr;
	ushort* OpenGL_lightmap_remap = nullptr;
	ubyte* OpenGL_bitmap_states = nullptr;
	ubyte* OpenGL_lightmap_states = nullptr;

	unsigned int opengl_last_upload_res = 0;
	uint* opengl_Upload_data = nullptr;
	uint* opengl_Translate_table = nullptr;
	uint* opengl_4444_translate_table = nullptr;

	ushort* opengl_packed_Upload_data = nullptr;
	ushort* opengl_packed_Translate_table = nullptr;
	ushort* opengl_packed_4444_translate_table = nullptr;

	//Texture list
	GLuint texture_name_list[10000];
	int Cur_texture_object_num = 1;
	int Last_texel_unit_set = -1;

	int OpenGL_last_bound[2];
	int OpenGL_sets_this_frame[10];
	int OpenGL_uploads = 0;

	bool OpenGL_cache_initted = false;

	//SHADER
	GLuint commonbuffername = 0;
	GLuint legacycommonbuffername = 0;
	GLuint fogbuffername = 0;
	GLuint specularbuffername = 0;
	GLuint terrainfogbuffername = 0;
	int terrainfogcounter = 0;

	ShaderProgram* lastshaderprog = nullptr;

	//FRAMEBUFFER
	GLuint fbVAOName = 0;
	GLuint fbVBOName = 0;

	//INIT
	renderer_preferred_state OpenGL_preferred_state = { false, true, false, 32, 1.0 };
	rendering_state OpenGL_state;

	bool OpenGL_debugging_enabled = false;
	bool OpenGL_buffer_storage_enabled = false;

#if defined(WIN32)
	//	Moved from DDGR library
	HWND hOpenGLWnd = nullptr;
	HDC hOpenGLDC = nullptr;
	HGLRC ResourceContext = nullptr;
#endif

	// The font characteristics
	float rend_FontRed[4], rend_FontBlue[4], rend_FontGreen[4], rend_FontAlpha[4];

private:
	//DRAW
	void SetDrawDefaults();
	void DrawMultitexturePolygon3D(int handle, g3Point** p, int nv, int map_type);
	void DrawFlatPolygon3D(g3Point** p, int nv);

	// Turns on/off multitexture blending
	void SetMultitextureBlendMode(bool state);

	//INIT
	// Sets default states for our renderer
	void SetDefaults();
	int Setup(HDC glhdc);
	void GetInformation();

	//IMAGES
	void InitImages();
	void FreeImages();
	int MakeTextureObject(int tn);
	int MakeBitmapCurrent(int handle, int map_type, int tn);
	void MakeWrapTypeCurrent(int handle, int map_type, int tn);
	void MakeFilterTypeCurrent(int handle, int map_type, int tn);
	int InitCache();
	void FreeCache();
	void FreeUploadBuffers();
	void SetUploadBufferSize(int width, int height);
	void TranslateBitmapToOpenGL(int texnum, int bm_handle, int map_type, int replace, int tn);
	void ChangeChunkedBitmap(int bm_handle, chunked_bitmap* chunk);

	bool CheckExtension(char* extName);
	void UpdateFramebuffer();
	void CloseFramebuffer();
	void SetViewport();
	void UpdateWindow();
	void SetAlwaysAlpha(bool state);
	float GetAlphaMultiplier();

public:
	GLCompatibilityRenderer();

	//INITIALIZATION

	// Init our renderer, pass the application object also.
	int Init(oeApplication* app, renderer_preferred_state* pref_state) override;
	// de-init the renderer
	void Close() override;

	//STATE

	// Tells the software renderer whether or not to use mipping
	void SetMipState(sbyte) override;

	// Sets the fog state to TRUE or FALSE
	void SetFogState(sbyte on) override;

	// Sets the near and far plane of fog
	void SetFogBorders(float fog_near, float fog_far) override;

	// Sets the color for fill based primitives;
	void SetFlatColor(ddgr_color color) override;

	void SetTextureType(texture_type) override;

	// Sets the state of bilinear filtering for our textures
	void SetFiltering(sbyte state) override;

	// Sets the state of zbuffering to on or off
	void SetZBufferState(sbyte state) override;

	// Sets the near and far planes for z buffer
	void SetZValues(float nearz, float farz) override;

	// Sets a bitmap as an overlay to rendered on top of the next texture map
	void SetOverlayMap(int handle) override;

	// Sets the type of overlay operation
	void SetOverlayType(ubyte type) override;

	// Sets the color of fog
	void SetFogColor(ddgr_color fogcolor) override;

	// sets the alpha type
	void SetAlphaType(sbyte alphatype) override;

	// Sets the constant alpha value
	void SetAlphaValue(ubyte val) override;

	// Sets the overall alpha scale factor (all alpha values are scaled by this value)
	// usefull for motion blur effect
	void SetAlphaFactor(float val) override;

	// Returns the current Alpha factor
	float GetAlphaFactor() override;

	// Sets the wrap parameter
	void SetWrapType(wrap_type val) override;


	// Sets some global preferences for the renderer
	// Returns -1 if it had to use the default resolution/bitdepth
	int SetPreferredState(renderer_preferred_state* pref_state) override;

	// Sets the hardware bias level for coplanar polygons
	// This helps reduce z buffer artifaces
	void SetCoplanarPolygonOffset(float factor) override;

	void SetCullFace(bool state) override;

	// color model
	void SetColorModel(color_model) override;

	void SetLighting(light_state) override;

	// Adds a bias to each coordinates z value.  This is useful for making 2d bitmaps
	// get drawn without being clipped by the zbuffer
	void SetZBias(float z_bias) override;

	// Enables/disables writes the depth buffer
	void SetZBufferWriteMask(int state) override;

	// Gets a bumpmap ready for drawing, or turns off bumpmapping
	void SetBumpmapReadyState(int state, int map) override;

	void SetGammaValue(float val) override;

	//INFORMATION

	// returns rendering statistics for the frame
	void GetStatistics(tRendererStats* stats) override;

	// Fills in some variables so the 3d math routines know how to project
	void GetProjectionParameters(int* width, int* height) override;
	void GetProjectionScreenParameters(int& screenLX, int& screenTY, int& screenW, int& screenH) override;

	// Returns the aspect ratio of the physical screen
	float GetAspectRatio() override;

	// Fills in the passed in pointer with the current rendering state
	void GetRenderState(rendering_state* rstate) override;

	// Fills in the passed in pointer with the current rendering state
	// Uses legacy structure for compatibiltity with current DLLs. 
	void DLLGetRenderState(DLLrendering_state* rstate) override;

	// Returns 1 if there is mid video memory, 2 if there is low vid memory, or 0 if there is large vid memory
	int LowVidMem() override;

	// Returns 1 if the renderer supports bumpmapping
	int SupportsBumpmapping() override;

	//IMAGES

	// Preuploads a bitmap to the card
	void PreUploadTextureToCard(int, int) override;
	void FreePreUploadedTexture(int, int) override;

	// Clears the texture cache
	void ResetCache() override;

	//DRAWING

	// Tells the renderer we're starting a frame.  Clear flags tells the renderer
	// what buffer (if any) to clear
	void StartFrame(int x1, int y1, int x2, int y2, int clear_flags = RF_CLEAR_ZBUFFER) override;

	// Tells the renderer the frame is over
	void EndFrame() override;

	// Clears the display to a specified color
	void ClearScreen(ddgr_color color) override;

	// Clears the zbuffer
	void ClearZBuffer() override;

	// Given a handle to a bitmap and nv point vertices, draws a 3D polygon
	void DrawPolygon3D(int handle, g3Point** p, int nv, int map_type = MAP_TYPE_BITMAP) override;

	// Given a handle to a bitmap and nv point vertices, draws a 2D polygon
	void DrawPolygon2D(int handle, g3Point** p, int nv) override;

	// Draws a scaled 2d bitmap to our buffer
	// NOTE: scripts are expecting the old prototype that has a zvalue (which is ignored) before color
	void DrawScaledBitmap(int x1, int y1, int x2, int y2, int bm, float u0, float v0, float u1, float v1, int color = -1, float* alphas = nullptr) override;

	void DrawScaledBitmapWithZ(int x1, int y1, int x2, int y2, int bm, float u0, float v0, float u1, float v1, float zval, int color, float* alphas = nullptr) override;

	//	given a chunked bitmap, renders it.
	void DrawChunkedBitmap(chunked_bitmap* chunk, int x, int y, ubyte alpha) override;

	//	given a chunked bitmap, renders it.scaled
	void DrawScaledChunkedBitmap(chunked_bitmap* chunk, int x, int y, int neww, int newh, ubyte alpha) override;

	// Draws a simple bitmap at the specified x,y location
	void DrawSimpleBitmap(int bm_handle, int x, int y) override;

	// Fills a rectangle on the display
	void FillRect(ddgr_color color, int x1, int y1, int x2, int y2) override;

	// Sets a pixel on the display
	void SetPixel(ddgr_color color, int x, int y) override;

	// Sets a pixel on the display
	ddgr_color GetPixel(int x, int y) override;

	// Sets the argb characteristics of the font characters.  color1 is the upper left and proceeds clockwise
	void SetCharacterParameters(ddgr_color color1, ddgr_color color2, ddgr_color color3, ddgr_color color4) override;

	// Sets up a font character to draw.  We draw our fonts as pieces of textures
	void DrawFontCharacter(int bm_handle, int x1, int y1, int x2, int y2, float u, float v, float w, float h) override;

	// Draws a line
	void DrawLine(int x1, int y1, int x2, int y2) override;

	//	Draws spheres
	void FillCircle(ddgr_color col, int x, int y, int rad) override;

	//	draws circles
	void DrawCircle(int x, int y, int rad) override;

	// Flips the surface
	void Flip() override;

	// Draws a line using the states of the renderer
	void DrawSpecialLine(g3Point* p0, g3Point* p1) override;

	//OTHER TRANSFERS

	// Takes a bitmap and blits it to the screen using linear frame buffer stuff
	// X and Y are the destination X,Y.
	void CopyBitmapToFramebuffer(int bm_handle, int x, int y) override;

	// Gets a renderer ready for a framebuffer copy, or stops a framebuffer copy
	void SetFrameBufferCopyState(bool state) override;

	// Takes a screenshot of the current frame and puts it into the handle passed
	void Screenshot(int bm_handle) override;

	//NEW STATE

	void UpdateCommon(float* projection, float* modelview, int depth = 0) override;
	void SetCommonDepth(int depth) override;

	//Gets a handle to a shader by name
	uint32_t GetPipelineByName(const char* name) override;

	//Given a handle from rend_GetShaderByName, binds that particular pipeline object
	void BindPipeline(uint32_t handle) override;

	//Updates specular components
	void UpdateSpecular(SpecularBlock* specularstate) override;

	//Updates brightness/fog components
	void UpdateFogBrightness(RoomBlock* roomstate, int numrooms) override;
	void SetCurrentRoomNum(int roomblocknum) override;
	void UpdateTerrainFog(float color[4], float start, float end) override;

	//These are temporary, used to test shader code.
	//Use the test shader.
	void UseShaderTest() override;

	//Revert to non-shader rendering
	void EndShaderTest() override;

	void BindBitmap(int handle) override;
	void BindLightmap(int handle) override;

	void RestoreLegacy() override;

	void GetScreenSize(int& screen_width, int& screen_height) override;

	void ClearBoundTextures() override;
};

#define GL_DEBUG

#define GET_WRAP_STATE(x)	((x>>2) & 0x03)
#define GET_MIP_STATE(x)	((x>>1) & 0x01);
#define GET_FILTER_STATE(x)	(x & 0x01)

#define SET_WRAP_STATE(x,s) {x&=0xF3; x|=(s<<2);}
#define SET_MIP_STATE(x,s) {x&=0xFD; x|=(s<<1);}
#define SET_FILTER_STATE(x,s) {x&=0xFE; x|=(s);}
