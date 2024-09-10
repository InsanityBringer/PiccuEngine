/*
* Piccu Engine
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

#include "renderer.h"

//Base renderer interface. 
//Renderer.cpp will call into this abstractly. 
class IRenderer
{
public:
	//INITIALIZATION

	// Init our renderer, pass the application object also.
	//virtual int Init(renderer_type state, oeApplication* app, renderer_preferred_state* pref_state) = 0;
	virtual int Init(oeApplication* app, renderer_preferred_state* pref_state) = 0;
	// de-init the renderer
	virtual void Close() = 0;

	//STATE

	// Tells the software renderer whether or not to use mipping
	virtual void SetMipState(sbyte) = 0;

	// Sets the fog state to TRUE or FALSE
	virtual void SetFogState(sbyte on) = 0;

	// Sets the near and far plane of fog
	virtual void SetFogBorders(float fog_near, float fog_far) = 0;

	// Sets the color for fill based primitives;
	virtual void SetFlatColor(ddgr_color color) = 0;

	virtual void SetTextureType(texture_type) = 0;

	// Sets the state of bilinear filtering for our textures
	virtual void SetFiltering(sbyte state) = 0;

	// Sets the state of zbuffering to on or off
	virtual void SetZBufferState(sbyte state) = 0;

	// Sets the near and far planes for z buffer
	virtual void SetZValues(float nearz, float farz) = 0;

	// Sets a bitmap as an overlay to rendered on top of the next texture map
	virtual void SetOverlayMap(int handle) = 0;

	// Sets the type of overlay operation
	virtual void SetOverlayType(ubyte type) = 0;

	// Sets the color of fog
	virtual void SetFogColor(ddgr_color fogcolor) = 0;

	// sets the alpha type
	virtual void SetAlphaType(sbyte alphatype) = 0;

	// Sets the constant alpha value
	virtual void SetAlphaValue(ubyte val) = 0;

	// Sets the overall alpha scale factor (all alpha values are scaled by this value)
	// usefull for motion blur effect
	virtual void SetAlphaFactor(float val) = 0;

	// Returns the current Alpha factor
	virtual float GetAlphaFactor() = 0;

	// Sets the wrap parameter
	virtual void SetWrapType(wrap_type val) = 0;

	// Enables/disables writes the depth buffer
	virtual void SetZBufferWriteMask(int state) = 0;

	// Sets some global preferences for the renderer
	// Returns -1 if it had to use the default resolution/bitdepth
	virtual int SetPreferredState(renderer_preferred_state* pref_state) = 0;

	// Sets the hardware bias level for coplanar polygons
	// This helps reduce z buffer artifaces
	virtual void SetCoplanarPolygonOffset(float factor) = 0;

	virtual void SetCullFace(bool state) = 0;

	// color model
	virtual void SetColorModel(color_model) = 0;

	virtual void SetLighting(light_state) = 0;

	// Adds a bias to each coordinates z value.  This is useful for making 2d bitmaps
	// get drawn without being clipped by the zbuffer
	virtual void SetZBias(float z_bias) = 0;

	// Gets a bumpmap ready for drawing, or turns off bumpmapping
	virtual void SetBumpmapReadyState(int state, int map) = 0;

	virtual void SetGammaValue(float val) = 0;


	//INFORMATION

	// returns rendering statistics for the frame
	virtual void GetStatistics(tRendererStats* stats) = 0;

	// Fills in some variables so the 3d math routines know how to project
	virtual void GetProjectionParameters(int* width, int* height) = 0;
	virtual void GetProjectionScreenParameters(int& screenLX, int& screenTY, int& screenW, int& screenH) = 0;

	// Returns the aspect ratio of the physical screen
	virtual float GetAspectRatio() = 0;

	// Fills in the passed in pointer with the current rendering state
	virtual void GetRenderState(rendering_state* rstate) = 0;

	// Fills in the passed in pointer with the current rendering state
	// Uses legacy structure for compatibiltity with current DLLs. 
	virtual void DLLGetRenderState(DLLrendering_state* rstate) = 0;

	// Returns 1 if there is mid video memory, 2 if there is low vid memory, or 0 if there is large vid memory
	virtual int LowVidMem() = 0;

	// Returns 1 if the renderer supports bumpmapping
	virtual int SupportsBumpmapping() = 0;

	//IMAGES
	
	// Preuploads a bitmap to the card
	virtual void PreUploadTextureToCard(int, int) = 0;
	virtual void FreePreUploadedTexture(int, int) = 0;

	// Clears the texture cache
	virtual void ResetCache() = 0;

	//DRAWING

	// Tells the renderer we're starting a frame.  Clear flags tells the renderer
	// what buffer (if any) to clear
	virtual void StartFrame(int x1, int y1, int x2, int y2, int clear_flags = RF_CLEAR_ZBUFFER) = 0;

	// Tells the renderer the frame is over
	virtual void EndFrame() = 0;

	// Clears the display to a specified color
	virtual void ClearScreen(ddgr_color color) = 0;

	// Clears the zbuffer
	virtual void ClearZBuffer() = 0;

	// Given a handle to a bitmap and nv point vertices, draws a 3D polygon
	virtual void DrawPolygon3D(int handle, g3Point** p, int nv, int map_type = MAP_TYPE_BITMAP) = 0;

	// Given a handle to a bitmap and nv point vertices, draws a 2D polygon
	virtual void DrawPolygon2D(int handle, g3Point** p, int nv) = 0;

	// Draws a scaled 2d bitmap to our buffer
	// NOTE: scripts are expecting the old prototype that has a zvalue (which is ignored) before color
	virtual void DrawScaledBitmap(int x1, int y1, int x2, int y2, int bm, float u0, float v0, float u1, float v1, int color = -1, float* alphas = nullptr) = 0;

	virtual void DrawScaledBitmapWithZ(int x1, int y1, int x2, int y2, int bm, float u0, float v0, float u1, float v1, float zval, int color, float* alphas = nullptr) = 0;

	//	given a chunked bitmap, renders it.
	virtual void DrawChunkedBitmap(chunked_bitmap* chunk, int x, int y, ubyte alpha) = 0;

	//	given a chunked bitmap, renders it.scaled
	virtual void DrawScaledChunkedBitmap(chunked_bitmap* chunk, int x, int y, int neww, int newh, ubyte alpha) = 0;

	// Draws a simple bitmap at the specified x,y location
	virtual void DrawSimpleBitmap(int bm_handle, int x, int y) = 0;

	// Fills a rectangle on the display
	virtual void FillRect(ddgr_color color, int x1, int y1, int x2, int y2) = 0;

	// Sets a pixel on the display
	virtual void SetPixel(ddgr_color color, int x, int y) = 0;

	// Sets a pixel on the display
	virtual ddgr_color GetPixel(int x, int y) = 0;

	// Sets the argb characteristics of the font characters.  color1 is the upper left and proceeds clockwise
	virtual void SetCharacterParameters(ddgr_color color1, ddgr_color color2, ddgr_color color3, ddgr_color color4) = 0;

	// Sets up a font character to draw.  We draw our fonts as pieces of textures
	virtual void DrawFontCharacter(int bm_handle, int x1, int y1, int x2, int y2, float u, float v, float w, float h) = 0;

	// Draws a line
	virtual void DrawLine(int x1, int y1, int x2, int y2) = 0;

	//	Draws spheres
	virtual void FillCircle(ddgr_color col, int x, int y, int rad) = 0;

	//	draws circles
	virtual void DrawCircle(int x, int y, int rad) = 0;

	// Flips the surface
	virtual void Flip() = 0;

	// Draws a line using the states of the renderer
	virtual void DrawSpecialLine(g3Point* p0, g3Point* p1) = 0;

	//OTHER TRANSFERS

	// Takes a bitmap and blits it to the screen using linear frame buffer stuff
	// X and Y are the destination X,Y.
	virtual void CopyBitmapToFramebuffer(int bm_handle, int x, int y) = 0;

	// Gets a renderer ready for a framebuffer copy, or stops a framebuffer copy
	virtual void SetFrameBufferCopyState(bool state) = 0;

	// Takes a screenshot of the current frame and puts it into the handle passed
	virtual void Screenshot(int bm_handle) = 0;

	//NEW STATE

	virtual void UpdateCommon(float* projection, float* modelview, int depth = 0) = 0;
	virtual void SetCommonDepth(int depth) = 0;

	//Gets a handle to a shader by name
	virtual uint32_t GetPipelineByName(const char* name) = 0;

	//Given a handle from rend_GetShaderByName, binds that particular pipeline object
	virtual void BindPipeline(uint32_t handle) = 0;

	//Updates specular components
	virtual void UpdateSpecular(SpecularBlock* specularstate) = 0;

	//Updates brightness/fog components
	virtual void UpdateFogBrightness(RoomBlock* roomstate, int numrooms) = 0;
	virtual void SetCurrentRoomNum(int roomblocknum) = 0;
	virtual void UpdateTerrainFog(float color[4], float start, float end) = 0;

	//These are temporary, used to test shader code.
	//Use the test shader.
	virtual void UseShaderTest() = 0;

	//Revert to non-shader rendering
	virtual void EndShaderTest() = 0;

	virtual void BindBitmap(int handle) = 0;
	virtual void BindLightmap(int handle) = 0;

	virtual void RestoreLegacy() = 0;

	virtual void GetScreenSize(int& screen_width, int& screen_height) = 0;
};
