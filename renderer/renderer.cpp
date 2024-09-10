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

#include "renderer.h"
#include "IRenderer.h"
#include "pserror.h"
#include "gl_local.h"

//TODO: Remove
renderer_type Renderer_type = RENDERER_OPENGL;

backend_type BackendType = BACKEND_GL3;
static IRenderer* renderer_inst;

bool Renderer_initted;
bool Renderer_close_flag;
bool UseHardware = true;
bool StateLimited;
bool NoLightmaps;
bool UseMultitexture;

float Z_bias;

// Init our renderer
int rend_Init(renderer_type state, oeApplication* app, renderer_preferred_state* pref_state)
{
#ifndef DEDICATED_ONLY
	int retval = 0;
	rend_SetRendererType(state);
	if (!Renderer_initted)
	{
		if (!Renderer_close_flag)
		{
			atexit(rend_Close);
			Renderer_close_flag = 1;
		}
	}
	else
	{
		return 1; //[ISB] don't double dip on renderer initialization. This happens after an int3
	}

	mprintf((0, "Renderer init is set to %d\n", Renderer_initted));

	switch (BackendType)
	{
	case BACKEND_GL3:
		renderer_inst = new GL3Renderer();
		break;
	default:
		Error("Unsupported backend");
		break;
	}

	retval = renderer_inst->Init(app, pref_state);
	if (retval == 0)
		rend_Close(); //Having renderer_inst->Init clean up would cause reentrancy problems, I suspect

	Renderer_initted = 1;

	return retval;
#else
	return 0;
#endif
}

void rend_Close()
{
	mprintf((0, "CLOSE:Renderer init is set to %d\n", Renderer_initted));
	if (!Renderer_initted)
		return;

	renderer_inst->Close();

	Renderer_initted = false;
}

void rend_SetRendererType(renderer_type state)
{
	Renderer_type = state;
	mprintf((0, "RendererType is set to %d.\n", state));
}

void rend_GetStatistics(tRendererStats* stats)
{
	renderer_inst->GetStatistics(stats);
}

void rend_SetTextureType(texture_type tt)
{
	renderer_inst->SetTextureType(tt);
}

void rend_DrawPolygon3D(int handle, g3Point** p, int nv, int map_type)
{
	renderer_inst->DrawPolygon3D(handle, p, nv, map_type);
}

void rend_DrawPolygon2D(int handle, g3Point** p, int nv)
{
	renderer_inst->DrawPolygon2D(handle, p, nv);
}

void rend_SetMipState(sbyte state)
{
	renderer_inst->SetMipState(state);
}

// Sets the fog state to TRUE or FALSE
void rend_SetFogState(sbyte on)
{
	renderer_inst->SetFogState(on);
}

// Sets the near and far plane of fog
void rend_SetFogBorders(float fog_near, float fog_far)
{
	renderer_inst->SetFogBorders(fog_near, fog_far);
}

// Sets the color for fill based primitives;
void rend_SetFlatColor(ddgr_color color)
{
	renderer_inst->SetFlatColor(color);
}

// Tells the renderer we're starting a frame.  Clear flags tells the renderer
// what buffer (if any) to clear
void rend_StartFrame(int x1, int y1, int x2, int y2, int clear_flags)
{
	renderer_inst->StartFrame(x1, y1, x2, y2, clear_flags);
}

// Tells the renderer the frame is over
void rend_EndFrame()
{
	renderer_inst->EndFrame();
}

// Draws a scaled 2d bitmap to our buffer
// NOTE: scripts are expecting the old prototype that has a zvalue (which is ignored) before color
void rend_DrawScaledBitmap(int x1, int y1, int x2, int y2, int bm, float u0, float v0, float u1, float v1, int color, float* alphas)
{
	renderer_inst->DrawScaledBitmap(x1, y1, x2, y2, bm, u0, v0, u1, v1, color, alphas);
}

void rend_DrawScaledBitmapWithZ(int x1, int y1, int x2, int y2, int bm, float u0, float v0, float u1, float v1, float zval, int color, float* alphas)
{
	renderer_inst->DrawScaledBitmapWithZ(x1, y1, x2, y2, bm, u0, v0, u1, v1, zval, color, alphas);
}

// Sets the state of bilinear filtering for our textures
void rend_SetFiltering(sbyte state)
{
	renderer_inst->SetFiltering(state);
}

// Sets the state of zbuffering to on or off
void rend_SetZBufferState(sbyte state)
{
	renderer_inst->SetZBufferState(state);
}

// Sets the near and far planes for z buffer
void rend_SetZValues(float nearz, float farz)
{
	renderer_inst->SetZValues(nearz, farz);
}

// Sets a bitmap as an overlay to rendered on top of the next texture map
void rend_SetOverlayMap(int handle)
{
	renderer_inst->SetOverlayMap(handle);
}

// Sets the type of overlay operation
void rend_SetOverlayType(ubyte type)
{
	renderer_inst->SetOverlayType(type);
}

// Clears the display to a specified color
void rend_ClearScreen(ddgr_color color)
{
	renderer_inst->ClearScreen(color);
}

// Fills a rectangle on the display
void rend_FillRect(ddgr_color color, int x1, int y1, int x2, int y2)
{
	renderer_inst->FillRect(color, x1, y1, x2, y2);
}

// Sets a pixel on the display
void rend_SetPixel(ddgr_color color, int x, int y)
{
	renderer_inst->SetPixel(color, x, y);
}

// Sets a pixel on the display
ddgr_color rend_GetPixel(int x, int y)
{
	return renderer_inst->GetPixel(x, y);
}

// Sets up a font character to draw.  We draw our fonts as pieces of textures
void rend_DrawFontCharacter(int bm_handle, int x1, int y1, int x2, int y2, float u, float v, float w, float h)
{
	renderer_inst->DrawFontCharacter(bm_handle, x1, y1, x2, y2, u, v, w, h);
}

// Draws a line
void rend_DrawLine(int x1, int y1, int x2, int y2)
{
	renderer_inst->DrawLine(x1, y1, x2, y2);
}

//	Draws spheres
void rend_FillCircle(ddgr_color col, int x, int y, int rad)
{
	renderer_inst->FillCircle(col, x, y, rad);
}

//	draws circles
void rend_DrawCircle(int x, int y, int rad)
{
	renderer_inst->DrawCircle(x, y, rad);
}

// Flips the surface
void rend_Flip()
{
	renderer_inst->Flip();
}

// Sets the argb characteristics of the font characters.  color1 is the upper left and proceeds clockwise
void rend_SetCharacterParameters(ddgr_color color1, ddgr_color color2, ddgr_color color3, ddgr_color color4)
{
	renderer_inst->SetCharacterParameters(color1, color2, color3, color4);
}

// Sets the color of fog
void rend_SetFogColor(ddgr_color fogcolor)
{
	renderer_inst->SetFogColor(fogcolor);
}

// sets the alpha type
void rend_SetAlphaType(sbyte type)
{
	renderer_inst->SetAlphaType(type);
}

// Sets the constant alpha value
void rend_SetAlphaValue(ubyte val)
{
	renderer_inst->SetAlphaValue(val);
}

// Sets the overall alpha scale factor (all alpha values are scaled by this value)
// usefull for motion blur effect
void rend_SetAlphaFactor(float val)
{
	renderer_inst->SetAlphaFactor(val);
}

// Returns the current Alpha factor
float rend_GetAlphaFactor()
{
	return renderer_inst->GetAlphaFactor();
}

// Sets the wrap parameter
void rend_SetWrapType(wrap_type val)
{
	renderer_inst->SetWrapType(val);
}

// Takes a screenshot of the current frame and puts it into the handle passed
void rend_Screenshot(int bm_handle)
{
	renderer_inst->Screenshot(bm_handle);
}

// Adds a bias to each coordinates z value.  This is useful for making 2d bitmaps
// get drawn without being clipped by the zbuffer
void rend_SetZBias(float z_bias)
{
	renderer_inst->SetZBias(z_bias);
}

// Enables/disables writes the depth buffer
void rend_SetZBufferWriteMask(int state)
{
	renderer_inst->SetZBufferWriteMask(state);
}

// Sets where the software renderer should write to
void rend_SetSoftwareParameters(float aspect, int width, int height, int pitch, ubyte* framebuffer)
{
	Int3();
}

// Fills in some variables so the 3d math routines know how to project
void rend_GetProjectionParameters(int* width, int* height)
{
	renderer_inst->GetProjectionParameters(width, height);
}

void rend_GetProjectionScreenParameters(int& screenLX, int& screenTY, int& screenW, int& screenH)
{
	renderer_inst->GetProjectionScreenParameters(screenLX, screenTY, screenW, screenH);
}

// Returns the aspect ratio of the physical screen
float rend_GetAspectRatio()
{
	return renderer_inst->GetAspectRatio();
}

// Gets a pointer to a linear frame buffer
void rend_GetLFBLock(renderer_lfb* lfb)
{
	Int3();
}

// Releases an lfb lock
void rend_ReleaseLFBLock(renderer_lfb* lfb)
{
	Int3();
}

// Given a source x,y and width,height, draws any sized bitmap into the renderer lfb
void rend_DrawLFBBitmap(int sx, int sy, int w, int h, int dx, int dy, ushort* data, int rowsize)
{
	Int3();
}

//	given a chunked bitmap, renders it.
void rend_DrawChunkedBitmap(chunked_bitmap* chunk, int x, int y, ubyte alpha)
{
	renderer_inst->DrawChunkedBitmap(chunk, x, y, alpha);
}

//	given a chunked bitmap, renders it.scaled
void rend_DrawScaledChunkedBitmap(chunked_bitmap* chunk, int x, int y, int neww, int newh, ubyte alpha)
{
	renderer_inst->DrawScaledChunkedBitmap(chunk, x, y, neww, newh, alpha);
}

// Draws a line using the states of the renderer
void rend_DrawSpecialLine(g3Point* p0, g3Point* p1)
{
	renderer_inst->DrawSpecialLine(p0, p1);
}

// Sets some global preferences for the renderer
// Returns -1 if it had to use the default resolution/bitdepth
int rend_SetPreferredState(renderer_preferred_state* pref_state)
{
	return renderer_inst->SetPreferredState(pref_state);
}

// Sets the gamma value 
void rend_SetGammaValue(float val)
{
	renderer_inst->SetGammaValue(val);
}

// Fills in the passed in pointer with the current rendering state
void rend_GetRenderState(rendering_state* rstate)
{
	renderer_inst->GetRenderState(rstate);
}

// Fills in the passed in pointer with the current rendering state
// Uses legacy structure for compatibiltity with current DLLs. 
void rend_DLLGetRenderState(DLLrendering_state* rstate)
{
	renderer_inst->DLLGetRenderState(rstate);
}

// Draws a simple bitmap at the specified x,y location
void rend_DrawSimpleBitmap(int bm_handle, int x, int y)
{
	renderer_inst->DrawSimpleBitmap(bm_handle, x, y);
}

// Sets the hardware bias level for coplanar polygons
// This helps reduce z buffer artifaces
void rend_SetCoplanarPolygonOffset(float factor)
{
	renderer_inst->SetCoplanarPolygonOffset(factor);
}

char Renderer_error_message[256];
// Retrieves an error message
char* rend_GetErrorMessage()
{
	return (char*)Renderer_error_message;
}

// Sets an error message
void rend_SetErrorMessage(char* str)
{
	ASSERT(strlen(str) < 256);
	strcpy(Renderer_error_message, str);
}

// Preuploads a bitmap to the card
void rend_PreUploadTextureToCard(int a, int b)
{
	renderer_inst->PreUploadTextureToCard(a, b);
}

void rend_FreePreUploadedTexture(int a, int b)
{
	renderer_inst->FreePreUploadedTexture(a, b);
}

// Returns 1 if there is mid video memory, 2 if there is low vid memory, or 0 if there is large vid memory
int rend_LowVidMem()
{
	return renderer_inst->LowVidMem();
}

// Returns 1 if the renderer supports bumpmapping
int rend_SupportsBumpmapping()
{
	return renderer_inst->SupportsBumpmapping();
}

// Gets a bumpmap ready for drawing, or turns off bumpmapping
void rend_SetBumpmapReadyState(int state, int map)
{
	renderer_inst->SetBumpmapReadyState(state, map);
}

// Clears the zbuffer
void rend_ClearZBuffer()
{
	renderer_inst->ClearZBuffer();
}

// Clears the texture cache
void rend_ResetCache()
{
	renderer_inst->ResetCache();
}

// Takes a bitmap and blits it to the screen using linear frame buffer stuff
// X and Y are the destination X,Y.
void rend_CopyBitmapToFramebuffer(int bm_handle, int x, int y)
{
	renderer_inst->CopyBitmapToFramebuffer(bm_handle, x, y);
}

// Gets a renderer ready for a framebuffer copy, or stops a framebuffer copy
void rend_SetFrameBufferCopyState(bool state)
{
	renderer_inst->SetFrameBufferCopyState(state);
}

void rend_UpdateCommon(float* projection, float* modelview, int depth)
{
	renderer_inst->UpdateCommon(projection, modelview, depth);
}

void rend_SetCommonDepth(int depth)
{
	renderer_inst->SetCommonDepth(depth);
}

//These are temporary, used to test shader code.
//Use the test shader.
void rend_UseShaderTest()
{
	renderer_inst->UseShaderTest();
}

//Revert to non-shader rendering
void rend_EndShaderTest()
{
	renderer_inst->EndShaderTest();
}

void rend_SetCullFace(bool state)
{
	renderer_inst->SetCullFace(state);
}

//Gets a handle to a shader by name
uint32_t rend_GetPipelineByName(const char* name)
{
	return renderer_inst->GetPipelineByName(name);
}

//Given a handle from rend_GetShaderByName, binds that particular pipeline object
void rend_BindPipeline(uint32_t handle)
{
	renderer_inst->BindPipeline(handle);
}

//Updates specular components
void rend_UpdateSpecular(SpecularBlock* specularstate)
{
	renderer_inst->UpdateSpecular(specularstate);
}

//Updates brightness/fog components
void rend_UpdateFogBrightness(RoomBlock* roomstate, int numrooms)
{
	renderer_inst->UpdateFogBrightness(roomstate, numrooms);
}

void rend_SetCurrentRoomNum(int roomblocknum)
{
	renderer_inst->SetCurrentRoomNum(roomblocknum);
}

void rend_UpdateTerrainFog(float color[4], float start, float end)
{
	renderer_inst->UpdateTerrainFog(color, start, end);
}

void rend_BindBitmap(int handle)
{
	renderer_inst->BindBitmap(handle);
}

void rend_BindLightmap(int handle)
{
	renderer_inst->BindLightmap(handle);
}

void rend_RestoreLegacy()
{
	renderer_inst->RestoreLegacy();
}

void rend_GetScreenSize(int& screen_width, int& screen_height)
{
	renderer_inst->GetScreenSize(screen_width, screen_height);
}

void rend_SetLighting(light_state state)
{
	renderer_inst->SetLighting(state);
}

void rend_SetColorModel(color_model model)
{
	renderer_inst->SetColorModel(model);
}

MeshBuilder::MeshBuilder()
{
	m_initialized = false;
	m_vertexstartoffset = m_vertexstartcount = 0;
	m_vertexstarted = false;
	m_indexstartoffset = m_indexstartcount = 0;
	m_indexstarted = false;
}

void MeshBuilder::BeginVertices()
{
	m_vertexstartoffset = m_vertices.size();
	m_vertexstartcount = 0;
	m_vertexstarted = true;
}

void MeshBuilder::BeginIndices()
{
	m_indexstartoffset = m_indicies.size();
	m_indexstartcount = 0;
	m_indexstarted = true;
}

void MeshBuilder::SetVertices(int numverts, RendVertex* vertices)
{
	if (!m_vertexstarted)
	{
		Error("MeshBuilder::SetVertices: BeginVertices not called!");
	}
	//This needs some optimization
	for (int i = 0; i < numverts; i++)
		m_vertices.push_back(vertices[i]);

	m_vertexstartcount += numverts;
}

void MeshBuilder::AddVertex(RendVertex& vertex)
{
	m_vertices.push_back(vertex);
	m_vertexstartcount++;
}

void MeshBuilder::SetIndicies(int numindices, int* indicies)
{
	if (!m_indexstarted)
	{
		Error("MeshBuilder::SetIndicies: BeginIndices not called!");
	}
	for (int i = 0; i < numindices; i++)
		m_indicies.push_back(indicies[i]);

	m_indexstartcount += numindices;
}

ElementRange MeshBuilder::EndVertices()
{
	if (!m_vertexstarted)
		Error("MeshBuilder::EndVertices: Not started!");
	m_vertexstarted = false;
	return ElementRange(m_vertexstartoffset, m_vertexstartcount);
}

ElementRange MeshBuilder::EndIndices()
{
	if (!m_indexstarted)
		Error("MeshBuilder::EndIndices: Not started!");
	m_indexstarted = false;
	return ElementRange(m_indexstartoffset, m_indexstartcount);
}

void MeshBuilder::BuildVertices(IVertexBuffer& buffer)
{
	buffer.Initialize(m_vertices.size(), m_vertices.size() * sizeof(m_vertices[0]), m_vertices.data());
}

void MeshBuilder::BuildIndicies(IIndexBuffer& buffer)
{
	buffer.Initialize(m_indicies.size(), m_indicies.size() * sizeof(m_indicies[0]), m_indicies.data());
}

ElementRange MeshBuilder::AppendVertices(IVertexBuffer& buffer)
{
	uint32_t offset = buffer.Append(m_vertices.size() * sizeof(m_vertices[0]), m_vertices.data());
	return ElementRange(offset, m_vertexstartcount);
}

void MeshBuilder::UpdateVertices(IVertexBuffer& buffer, uint32_t offset)
{
	buffer.Update(offset, m_vertices.size() * sizeof(m_vertices[0]), m_vertices.data());
}

void MeshBuilder::UpdateIndicies(IIndexBuffer& buffer, uint32_t offset)
{
	buffer.Update(offset, m_indicies.size() * sizeof(m_indicies[0]), m_indicies.data());
}

void MeshBuilder::Reset()
{
	m_vertices.clear();
	m_indicies.clear();
	m_vertexstartoffset = m_vertexstartcount = 0;
	m_indexstartoffset = m_indexstartcount = 0;
}
