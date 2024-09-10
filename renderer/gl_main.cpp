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
#include "gl_local.h"
#include "rtperformance.h"

static float mat4_identity[16] =
{ 1, 0, 0, 0,
	0, 1, 0, 0,
	0, 0, 1, 0,
	0, 0, 0, 1 };


void GL_Ortho(float* mat, float left, float right, float bottom, float top, float znear, float zfar)
{
	memset(mat, 0, sizeof(float[16]));
	mat[0] = 2 / (right - left);
	mat[5] = 2 / (top - bottom);
	mat[10] = -2 / (zfar - znear);
	mat[12] = -((right + left) / (right - left));
	mat[13] = -((top + bottom) / (top - bottom));
	mat[14] = -((zfar + znear) / (zfar - znear));
	mat[15] = 1;
}

void GL3Renderer::UpdateWindow()
{
	int width, height;
	if (!OpenGL_preferred_state.fullscreen)
	{
		OpenGL_state.view_width = OpenGL_preferred_state.window_width;
		OpenGL_state.view_height = OpenGL_preferred_state.window_height;
		width = OpenGL_preferred_state.width;
		height = OpenGL_preferred_state.height;

		//[ISB] center window
		int mWidth = GetSystemMetrics(SM_CXSCREEN);
		int mHeight = GetSystemMetrics(SM_CYSCREEN);

		int orgX = (mWidth / 2 - OpenGL_state.view_width / 2);
		int orgY = (mHeight / 2 - OpenGL_state.view_height / 2);
		RECT rect = { orgX, orgY, orgX + OpenGL_state.view_width, orgY + OpenGL_state.view_height };
		AdjustWindowRectEx(&rect, WS_CAPTION, FALSE, 0);
		ParentApplication->set_sizepos(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
		ParentApplication->set_flags(OEAPP_WINDOWED);
	}
	else
	{
		ParentApplication->set_flags(OEAPP_FULLSCREEN);

		RECT rect;
		GetWindowRect((HWND)hOpenGLWnd, &rect);
		mprintf((0, "rect=%d %d %d %d\n", rect.top, rect.right, rect.bottom, rect.left));

		OpenGL_state.view_width = rect.right - rect.left;
		OpenGL_state.view_height = rect.bottom - rect.top;

		width = OpenGL_preferred_state.width;
		height = OpenGL_preferred_state.height;
	}

	float baseAspect = width / (float)height;
	float trueAspect = OpenGL_state.view_width / (float)OpenGL_state.view_height;

	if (baseAspect < trueAspect) //base screen is less wide, so pillarbox it
	{
		framebuffer_blit_h = OpenGL_state.view_height; framebuffer_blit_y = 0;
		framebuffer_blit_w = OpenGL_state.view_height * baseAspect; framebuffer_blit_x = (OpenGL_state.view_width - framebuffer_blit_w) / 2;
	}
	else //base screen is more wide, so letterbox it
	{
		framebuffer_blit_w = OpenGL_state.view_width; framebuffer_blit_x = 0;
		framebuffer_blit_h = OpenGL_state.view_width / baseAspect; framebuffer_blit_y = (OpenGL_state.view_height - framebuffer_blit_h) / 2;
	}

	OpenGL_state.screen_width = width;
	OpenGL_state.screen_height = height;
}

void GL3Renderer::SetViewport()
{
	//[ISB] the hardware t&l code is AWFUL and the software t&l code won't compile. 
	// Reverting it back to only ever using passthrough. 
	// Projection
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho((GLfloat)0.0f, (GLfloat)(OpenGL_preferred_state.width), (GLfloat)(OpenGL_preferred_state.height), (GLfloat)0.0f, 0.0f, 1.0f);

	float left = 0;
	float right = OpenGL_preferred_state.width;
	float bottom = OpenGL_preferred_state.height;
	float top = 0;
	float znear = 0;
	float zfar = 1;

	float projection[16];
	GL_Ortho(projection, left, right, bottom, top, znear, zfar);

	UpdateLegacyBlock(projection, mat4_identity);
	// Viewport
	glViewport(0, 0, OpenGL_preferred_state.width, OpenGL_preferred_state.height);

	// ModelView
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
}

// Sets some global preferences for the renderer
int GL3Renderer::SetPreferredState(renderer_preferred_state* pref_state)
{
	int retval = 1;
	renderer_preferred_state old_state = OpenGL_preferred_state;

	OpenGL_preferred_state = *pref_state;
	if (OpenGL_state.initted)
	{
		int reinit = 0;
		mprintf((0, "Inside pref state!\n"));

		// Change gamma if needed
		/*if( pref_state->width!=OpenGL_state.screen_width || pref_state->height!=OpenGL_state.screen_height || old_state.bit_depth!=pref_state->bit_depth)
		{
			reinit=1;
		}

		if( reinit )
		{
			opengl_Close();
			retval = opengl_Init( NULL, &OpenGL_preferred_state );
		}
		else
		{*/

		if (pref_state->width != OpenGL_state.screen_width || pref_state->height != OpenGL_state.screen_height
			|| pref_state->window_width != OpenGL_state.view_width || pref_state->window_height != OpenGL_state.view_height
			|| pref_state->fullscreen != old_state.fullscreen || pref_state->antialised != old_state.antialised)
		{
			UpdateWindow();
			SetViewport();
			UpdateFramebuffer();
		}

		if (old_state.gamma != pref_state->gamma)
		{
			SetGammaValue(pref_state->gamma);
		}

#ifdef WIN32
		if (dwglSwapIntervalEXT)
		{
			if (pref_state->vsync_on)
				dwglSwapIntervalEXT(1);
			else
				dwglSwapIntervalEXT(0);
		}
#endif
		//}
	}
	else
	{
		OpenGL_preferred_state = *pref_state;
	}

	return retval;
}

void GL3Renderer::StartFrame(int x1, int y1, int x2, int y2, int clear_flags)
{
	GLenum glclearflags = 0;
	if (clear_flags & RF_CLEAR_ZBUFFER)
		glclearflags |= GL_DEPTH_BUFFER_BIT;
	
	if (clear_flags & RF_CLEAR_COLOR)
	{
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glclearflags |= GL_COLOR_BUFFER_BIT;
	}

	if (glclearflags != 0)
		glClear(glclearflags);

	OpenGL_state.clip_x1 = x1;
	OpenGL_state.clip_y1 = y1;
	OpenGL_state.clip_x2 = x2;
	OpenGL_state.clip_y2 = y2;

	//[ISB] Use the viewport to constrain the clipping window so that the new hardware code 
	//can work with the legacy code.
	float projection[16];
	GL_Ortho(projection, 0, x2 - x1, y2 - y1, 0, 0, 1);

	UpdateLegacyBlock(projection, mat4_identity);

	glViewport(x1, OpenGL_state.screen_height - y2, x2 - x1, y2 - y1);
}

// Flips the screen
void GL3Renderer::Flip()
{
#ifdef _DEBUG
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		Int3();
	}
#endif
#ifndef RELEASE
	RTP_INCRVALUE(texture_uploads, OpenGL_uploads);
	RTP_INCRVALUE(polys_drawn, OpenGL_polys_drawn);

	mprintf_at((1, 1, 0, "Uploads=%d    Polys=%d   Verts=%d   ", OpenGL_uploads, OpenGL_polys_drawn, OpenGL_verts_processed));
	mprintf_at((1, 2, 0, "Sets= 0:%d   1:%d   2:%d   3:%d   ", OpenGL_sets_this_frame[0], OpenGL_sets_this_frame[1], OpenGL_sets_this_frame[2], OpenGL_sets_this_frame[3]));
	mprintf_at((1, 3, 0, "Sets= 4:%d   5:%d  ", OpenGL_sets_this_frame[4], OpenGL_sets_this_frame[5]));
	for (int i = 0; i < 10; i++)
	{
		OpenGL_sets_this_frame[i] = 0;
	}
#endif

	OpenGL_last_frame_polys_drawn = OpenGL_polys_drawn;
	OpenGL_last_frame_verts_processed = OpenGL_verts_processed;
	OpenGL_last_uploaded = OpenGL_uploads;

	OpenGL_uploads = 0;
	OpenGL_polys_drawn = 0;
	OpenGL_verts_processed = 0;

	//[ISB] remove the BlitToRaw call so I can hack around drivers that do things like forced antialiasing that cause an otherwise valid operation to stop working. 
	blitshader.Use();
	framebuffers[framebuffer_current_draw].BlitTo(0, framebuffer_blit_x, framebuffer_blit_y, framebuffer_blit_w, framebuffer_blit_h);
	glUseProgram(0);
	
	UseDrawVAO();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

#ifdef _DEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
	{
		Int3();
	}
#endif

#if defined(WIN32)	
	SwapBuffers((HDC)hOpenGLDC);
#elif defined(__LINUX__)
	SDL_GL_SwapBuffers();
#endif

	framebuffer_current_draw = (framebuffer_current_draw + 1) % NUM_GL3_FBOS;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[framebuffer_current_draw].Handle());

#ifdef _DEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
	{
		Int3();
	}
#endif

#ifdef __PERMIT_GL_LOGGING
	if (__glLog == true)
	{
		DGL_LogNewFrame();
	}
#endif
}

void GL3Renderer::EndFrame(void)
{
}


// returns true if the passed in extension name is supported
bool GL3Renderer::CheckExtension(char* extName)
{
	GLint extcount;
	glGetIntegerv(GL_NUM_EXTENSIONS, &extcount);
	for (int i = 0; i < extcount; i++)
	{
		const GLubyte* extname = glGetStringi(GL_EXTENSIONS, i);
		if (!stricmp((const char*)extname, extName))
			return true;
	}

	return false;
}

void GL3Renderer::SetGammaValue(float val)
{
	blitshader.Use();

	glUniform1f(blitshader_gamma, 1.f / val);

	glUseProgram(0);
}

void GL3Renderer::SetFlatColor(ddgr_color color)
{
	OpenGL_state.cur_color = color;
}

// Sets the fog state to TRUE or FALSE
void GL3Renderer::SetFogState(sbyte state)
{
	if (state == OpenGL_state.cur_fog_state)
		return;

	OpenGL_state.cur_fog_state = state;
}

// Sets the near and far plane of fog
void GL3Renderer::SetFogBorders(float nearz, float farz)
{
	// Sets the near and far plane of fog
	float fog_start = std::max(0.f, std::min(1.0f, 1.0f - (1.0f / nearz)));
	float fog_end = std::max(0.f, std::min(1.0f, 1.0f - (1.0f / farz)));

	OpenGL_state.cur_fog_start = fog_start;
	OpenGL_state.cur_fog_end = fog_end;
}

// Sets the color of fog
void GL3Renderer::SetFogColor(ddgr_color color)
{
	float fc[4];
	fc[0] = GR_COLOR_RED(color);
	fc[1] = GR_COLOR_GREEN(color);
	fc[2] = GR_COLOR_BLUE(color);
	fc[3] = 1;

	fc[0] /= 255.0f;
	fc[1] /= 255.0f;
	fc[2] /= 255.0f;

	UpdateTerrainFog(fc, OpenGL_state.cur_fog_start, OpenGL_state.cur_fog_end);
}

void GL3Renderer::SetLighting(light_state state)
{
	if (state == OpenGL_state.cur_light_state)
		return;	// No redundant state setting
	if (UseMultitexture && Last_texel_unit_set != 0)
	{
		glActiveTexture(GL_TEXTURE0 + 0);
		Last_texel_unit_set = 0;
	}

	OpenGL_sets_this_frame[4]++;

	switch (state)
	{
	case LS_NONE:
		OpenGL_state.cur_light_state = LS_NONE;
		break;
	case LS_FLAT_GOURAUD:
		OpenGL_state.cur_light_state = LS_FLAT_GOURAUD;
		break;
	case LS_GOURAUD:
	case LS_PHONG:
		OpenGL_state.cur_light_state = LS_GOURAUD;
		break;
	default:
		Int3();
		break;
	}

	CHECK_ERROR(13)
}

void GL3Renderer::SetColorModel(color_model state)
{
	switch (state)
	{
	case CM_MONO:
		OpenGL_state.cur_color_model = CM_MONO;
		break;
	case CM_RGB:
		OpenGL_state.cur_color_model = CM_RGB;
		break;
	default:
		Int3();
		break;
	}
}

void GL3Renderer::SetTextureType(texture_type state)
{
	if (state == OpenGL_state.cur_texture_type)
		return;	// No redundant state setting
	if (UseMultitexture && Last_texel_unit_set != 0)
	{
		glActiveTexture(GL_TEXTURE0 + 0);
		Last_texel_unit_set = 0;
	}
	OpenGL_sets_this_frame[3]++;

	switch (state)
	{
	case TT_FLAT:
		OpenGL_state.cur_texture_quality = 0;
		break;
	case TT_LINEAR:
	case TT_LINEAR_SPECIAL:
	case TT_PERSPECTIVE:
	case TT_PERSPECTIVE_SPECIAL:
		OpenGL_state.cur_texture_quality = 2;
		break;
	default:
		Int3();	// huh? Get Jason
		break;
	}

	CHECK_ERROR(12)
		OpenGL_state.cur_texture_type = state;
}

// Sets the state of bilinear filtering for our textures
void GL3Renderer::SetFiltering(sbyte state)
{
	OpenGL_state.cur_bilinear_state = state;
}

// Sets the state of z-buffering to on or off
void GL3Renderer::SetZBufferState(sbyte state)
{
	if (state == OpenGL_state.cur_zbuffer_state)
		return;	// No redundant state setting

	OpenGL_sets_this_frame[5]++;
	OpenGL_state.cur_zbuffer_state = state;

	//	mprintf ((0,"OPENGL: Setting zbuffer state to %d.\n",state)); 

	if (state)
	{
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	CHECK_ERROR(14)
}

// Sets the near and far planes for z buffer
void GL3Renderer::SetZValues(float nearz, float farz)
{
	OpenGL_state.cur_near_z = nearz;
	OpenGL_state.cur_far_z = farz;
	//	mprintf ((0,"OPENGL:Setting depth range to %f - %f\n",nearz,farz));
}

// Sets a bitmap as a overlay map to rendered on top of the next texture map
// a -1 value indicates no overlay map
void GL3Renderer::SetOverlayMap(int handle)
{
	Overlay_map = handle;
}

void GL3Renderer::SetOverlayType(ubyte type)
{
	Overlay_type = type;
}

// Clears the display to a specified color
void GL3Renderer::ClearScreen(ddgr_color color)
{
	int r = (color >> 16 & 0xFF);
	int g = (color >> 8 & 0xFF);
	int b = (color & 0xFF);

	glClearColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

// Clears the zbuffer for the screen
void GL3Renderer::ClearZBuffer(void)
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

// returns the alpha that we should use
float GL3Renderer::GetAlphaMultiplier()
{
	switch (OpenGL_state.cur_alpha_type)
	{
	case AT_ALWAYS:
		return 255;
	case AT_CONSTANT:
		return OpenGL_state.cur_alpha;
	case AT_TEXTURE:
		return 255;
	case AT_CONSTANT_TEXTURE:
		return OpenGL_state.cur_alpha;
	case AT_VERTEX:
		return 255;
	case AT_CONSTANT_TEXTURE_VERTEX:
	case AT_CONSTANT_VERTEX:
		return OpenGL_state.cur_alpha;
	case AT_TEXTURE_VERTEX:
		return 255;
	case AT_LIGHTMAP_BLEND:
	case AT_LIGHTMAP_BLEND_SATURATE:
		return OpenGL_state.cur_alpha;
	case AT_SATURATE_TEXTURE:
		return OpenGL_state.cur_alpha;
	case AT_SATURATE_VERTEX:
		return 255;
	case AT_SATURATE_CONSTANT_VERTEX:
		return OpenGL_state.cur_alpha;
	case AT_SATURATE_TEXTURE_VERTEX:
		return 255;
	case AT_SPECULAR:
		return 255;
	default:
		//Int3();		// no type defined,get jason
		return 0;
	}
}


void GL3Renderer::SetAlwaysAlpha(bool state)
{
	if (state && OpenGL_blending_on)
	{
		glDisable(GL_BLEND);
		//glDisable(GL_ALPHA_TEST);
		OpenGL_blending_on = false;
	}
	else if (!state)
	{
		glEnable(GL_BLEND);
		//glEnable(GL_ALPHA_TEST);
		OpenGL_blending_on = true;
	}
}

void GL3Renderer::SetAlphaType(sbyte atype)
{
	if (atype == OpenGL_state.cur_alpha_type)
		return;		// don't set it redundantly
	if (UseMultitexture && Last_texel_unit_set != 0)
	{
		glActiveTexture(GL_TEXTURE0 + 0);
		Last_texel_unit_set = 0;

	}
	OpenGL_sets_this_frame[6]++;

	switch (atype)
	{
	case AT_ALWAYS:
		SetAlphaValue(255);
		SetAlwaysAlpha(true);
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case AT_CONSTANT:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_TEXTURE:
		SetAlphaValue(255);
		SetAlwaysAlpha(true);
		glBlendFunc(GL_ONE, GL_ZERO);
		break;
	case AT_CONSTANT_TEXTURE:
	case AT_CONSTANT_TEXTURE_VERTEX:
	case AT_TEXTURE_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_CONSTANT_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		break;
	case AT_LIGHTMAP_BLEND:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		break;
	case AT_SATURATE_TEXTURE:
	case AT_LIGHTMAP_BLEND_SATURATE:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case AT_SATURATE_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case AT_SATURATE_CONSTANT_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case AT_SATURATE_TEXTURE_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		break;
	case AT_SPECULAR:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		//hack
		OpenGL_state.cur_texture_quality = 2;
		OpenGL_state.cur_texture_type = TT_PERSPECTIVE;

		break;
	default:
		Int3();		// no type defined,get jason
		break;
	}
	OpenGL_state.cur_alpha_type = atype;
	Alpha_multiplier = GetAlphaMultiplier();
	CHECK_ERROR(15)
}

// Sets the alpha value for constant alpha
void GL3Renderer::SetAlphaValue(ubyte val)
{
	OpenGL_state.cur_alpha = val;
	Alpha_multiplier = GetAlphaMultiplier();
}

// Sets the overall alpha scale factor (all alpha values are scaled by this value)
// usefull for motion blur effect
void GL3Renderer::SetAlphaFactor(float val)
{
	if (val < 0.0f) val = 0.0f;
	if (val > 1.0f) val = 1.0f;
	OpenGL_Alpha_factor = val;
}

// Returns the current Alpha factor
float GL3Renderer::GetAlphaFactor(void)
{
	return OpenGL_Alpha_factor;
}

// Sets the texture wrapping type
void GL3Renderer::SetWrapType(wrap_type val)
{
	OpenGL_state.cur_wrap_type = val;
}

void GL3Renderer::SetZBias(float z_bias)
{
	if (Z_bias != z_bias)
	{
		Z_bias = z_bias;
	}
}

// Enables/disables writes the depth buffer
void GL3Renderer::SetZBufferWriteMask(int state)
{
	OpenGL_sets_this_frame[5]++;
	if (state)
	{
		glDepthMask(GL_TRUE);
	}
	else
	{
		glDepthMask(GL_FALSE);
	}
}

// Returns the aspect ratio of the physical screen
void GL3Renderer::GetProjectionParameters(int* width, int* height)
{
	*width = OpenGL_state.clip_x2 - OpenGL_state.clip_x1;
	*height = OpenGL_state.clip_y2 - OpenGL_state.clip_y1;
}

void GL3Renderer::GetProjectionScreenParameters(int& screenLX, int& screenTY, int& screenW, int& screenH)
{
	screenLX = OpenGL_state.clip_x1;
	screenTY = OpenGL_state.clip_y1;
	screenW = OpenGL_state.clip_x2 - OpenGL_state.clip_x1 + 1;
	screenH = OpenGL_state.clip_y2 - OpenGL_state.clip_y1 + 1;
}

// Returns the aspect ratio of the physical screen
float GL3Renderer::GetAspectRatio()
{
	float aspect_ratio = (float)((3.0f * OpenGL_state.screen_width) / (4.0f * OpenGL_state.screen_height));
	return aspect_ratio;
}

// Sets the hardware bias level for coplanar polygons
// This helps reduce z buffer artifacts
void GL3Renderer::SetCoplanarPolygonOffset(float factor)
{
	if (factor == 0.0f)
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}
	else
	{
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.0f, -1.0f);
	}
}

// Preuploads a texture to the video card
void GL3Renderer::PreUploadTextureToCard(int handle, int map_type)
{
}

// Frees an uploaded texture from the video card
void GL3Renderer::FreePreUploadedTexture(int handle, int map_type)
{
}

// Returns 1 if there is mid video memory, 2 if there is low vid memory, or 0 if there is large vid memory
int GL3Renderer::LowVidMem()
{
	return 0;
}

// Returns 1 if the renderer supports bumpmapping
int GL3Renderer::SupportsBumpmapping()
{
	return 0;
}

// Sets a bumpmap to be rendered, or turns off bumpmapping altogether
void GL3Renderer::SetBumpmapReadyState(int state, int map)
{
}

// returns rendering statistics for the frame
void GL3Renderer::GetStatistics(tRendererStats* stats)
{
	stats->poly_count = OpenGL_last_frame_polys_drawn;
	stats->vert_count = OpenGL_last_frame_verts_processed;
	stats->texture_uploads = OpenGL_last_uploaded;
}

// Tells the software renderer whether or not to use mipping
void GL3Renderer::SetMipState(sbyte mipstate)
{
	OpenGL_state.cur_mip_state = mipstate;
}

// Fills in the passed in pointer with the current rendering state
void GL3Renderer::GetRenderState(rendering_state* rstate)
{
	memcpy(rstate, &OpenGL_state, sizeof(rendering_state));
}

void GL3Renderer::DLLGetRenderState(DLLrendering_state* rstate)
{
#define COPY_ELEMENT(element) rstate->element = OpenGL_state.element;
	COPY_ELEMENT(initted);
	COPY_ELEMENT(cur_bilinear_state);
	COPY_ELEMENT(cur_zbuffer_state);
	COPY_ELEMENT(cur_fog_state);
	COPY_ELEMENT(cur_mip_state);
	COPY_ELEMENT(cur_texture_type);
	COPY_ELEMENT(cur_color_model);
	COPY_ELEMENT(cur_light_state);
	COPY_ELEMENT(cur_alpha_type);
	COPY_ELEMENT(cur_wrap_type);
	COPY_ELEMENT(cur_fog_start);
	COPY_ELEMENT(cur_fog_end);
	COPY_ELEMENT(cur_near_z);
	COPY_ELEMENT(cur_far_z);
	COPY_ELEMENT(gamma_value);
	COPY_ELEMENT(cur_alpha);
	COPY_ELEMENT(cur_color);
	COPY_ELEMENT(cur_fog_color);
	COPY_ELEMENT(cur_texture_quality);
	COPY_ELEMENT(clip_x1);
	COPY_ELEMENT(clip_x2);
	COPY_ELEMENT(clip_y1);
	COPY_ELEMENT(clip_y2);
	COPY_ELEMENT(screen_width);
	COPY_ELEMENT(screen_height);
#undef COPY_ELEMENT
}

// Takes a screenshot of the current frame and puts it into the handle passed
void GL3Renderer::Screenshot(int bm_handle)
{
	ushort* dest_data;
	uint* temp_data;
	int i, t;
	int total = OpenGL_state.screen_width * OpenGL_state.screen_height;

	ASSERT((bm_w(bm_handle, 0)) == OpenGL_state.screen_width);
	ASSERT((bm_h(bm_handle, 0)) == OpenGL_state.screen_height);

	int w = bm_w(bm_handle, 0);
	int h = bm_h(bm_handle, 0);

	temp_data = (uint*)mem_malloc(total * 4);
	ASSERT(temp_data);	// Ran out of memory?

	dest_data = bm_data(bm_handle, 0);

	glReadPixels(0, 0, OpenGL_state.screen_width, OpenGL_state.screen_height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)temp_data);

	for (i = 0; i < h; i++)
	{
		for (t = 0; t < w; t++)
		{
			uint spix = temp_data[i * w + t];

			int r = spix & 0xff;
			int g = (spix >> 8) & 0xff;
			int b = (spix >> 16) & 0xff;

			dest_data[(((h - 1) - i) * w) + t] = GR_RGB16(r, g, b);
		}
	}

	mem_free(temp_data);
}

void GL3Renderer::UpdateFramebuffer(void)
{
	for (int i = 0; i < NUM_GL3_FBOS; i++)
	{
		framebuffers[i].Update(OpenGL_state.screen_width, OpenGL_state.screen_height, OpenGL_preferred_state.antialised);
	}

	framebuffer_current_draw = 0;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[0].Handle());
	//Unbind the read framebuffer so that OBS can capture the window properly
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	GL_InitFramebufferVAO();
}

void GL3Renderer::CloseFramebuffer(void)
{
	for (int i = 0; i < NUM_GL3_FBOS; i++)
	{
		framebuffers[i].Destroy();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_DestroyFramebufferVAO();
}

//shader test
void GL3Renderer::UseShaderTest(void)
{
	testshader.Use();
}

void GL3Renderer::EndShaderTest(void)
{
	//glUseProgram(0);
}

void GL3Renderer::SetCullFace(bool state)
{
	if (state)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

void GL3Renderer::GetScreenSize(int& screen_width, int& screen_height)
{
	screen_width = OpenGL_state.screen_width;
	screen_height = OpenGL_state.screen_height;
}

GL3Renderer::GL3Renderer()
{
}
