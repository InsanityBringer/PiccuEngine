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
#include "gl1_local.h"
#include "rtperformance.h"

void GLCompatibilityRenderer::UpdateWindow()
{
	int width, height;
	if (!OpenGL_preferred_state.fullscreen)
	{
		OpenGL_state.view_width = OpenGL_preferred_state.window_width;
		OpenGL_state.view_height = OpenGL_preferred_state.window_height;
		width = OpenGL_preferred_state.width;
		height = OpenGL_preferred_state.height;

		//[ISB] center window
		ParentApplication->set_flags(OEAPP_WINDOWED);
#ifdef SDL3
		ParentApplication->set_sizepos(OEAPP_COORD_CENTERED, OEAPP_COORD_CENTERED, OpenGL_state.view_width, OpenGL_state.view_height);
#elif WIN32
		int mWidth = GetSystemMetrics(SM_CXSCREEN);
		int mHeight = GetSystemMetrics(SM_CYSCREEN);

		int orgX = (mWidth / 2 - OpenGL_state.view_width / 2);
		int orgY = (mHeight / 2 - OpenGL_state.view_height / 2);
		RECT rect = { orgX, orgY, orgX + OpenGL_state.view_width, orgY + OpenGL_state.view_height };
		AdjustWindowRectEx(&rect, WS_CAPTION, FALSE, 0);
		ParentApplication->set_sizepos(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
#endif
	}
	else
	{
		ParentApplication->set_flags(OEAPP_FULLSCREEN);
#ifdef SDL3
		SDL_GetWindowSizeInPixels(GLWindow, &OpenGL_state.view_width, &OpenGL_state.view_height);
#elif WIN32
		RECT rect;
		GetWindowRect((HWND)hOpenGLWnd, &rect);
		mprintf((0, "rect=%d %d %d %d\n", rect.top, rect.right, rect.bottom, rect.left));

		OpenGL_state.view_width = rect.right - rect.left;
		OpenGL_state.view_height = rect.bottom - rect.top;
#endif

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

void GLCompatibilityRenderer::SetViewport()
{
	//[ISB] the hardware t&l code is AWFUL and the software t&l code won't compile. 
	// Reverting it back to only ever using passthrough. 
	// Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho((GLfloat)0.0f, (GLfloat)(OpenGL_preferred_state.width), (GLfloat)(OpenGL_preferred_state.height), (GLfloat)0.0f, 0.0f, 1.0f);

	// Viewport
	glViewport(0, 0, OpenGL_preferred_state.width, OpenGL_preferred_state.height);

	// ModelView
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// Sets some global preferences for the renderer
int GLCompatibilityRenderer::SetPreferredState(renderer_preferred_state* pref_state)
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

#ifdef SDL3
		if (pref_state->vsync_on)
			SDL_GL_SetSwapInterval(1);
		else
			SDL_GL_SetSwapInterval(0);
#elif WIN32
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

	CHECK_ERROR(33);

	return retval;
}

void GLCompatibilityRenderer::StartFrame(int x1, int y1, int x2, int y2, int clear_flags)
{
	if (clear_flags & RF_CLEAR_ZBUFFER)
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	OpenGL_state.clip_x1 = x1;
	OpenGL_state.clip_y1 = y1;
	OpenGL_state.clip_x2 = x2;
	OpenGL_state.clip_y2 = y2;

	CHECK_ERROR(31);
}

// Flips the screen
void GLCompatibilityRenderer::Flip(void)
{
#ifndef NDEBUG
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		Int3();
	}
#endif
#ifndef RELEASE
	int i;

	RTP_INCRVALUE(texture_uploads, OpenGL_uploads);
	RTP_INCRVALUE(polys_drawn, OpenGL_polys_drawn);

	mprintf_at((1, 1, 0, "Uploads=%d    Polys=%d   Verts=%d   ", OpenGL_uploads, OpenGL_polys_drawn, OpenGL_verts_processed));
	mprintf_at((1, 2, 0, "Sets= 0:%d   1:%d   2:%d   3:%d   ", OpenGL_sets_this_frame[0], OpenGL_sets_this_frame[1], OpenGL_sets_this_frame[2], OpenGL_sets_this_frame[3]));
	mprintf_at((1, 3, 0, "Sets= 4:%d   5:%d  ", OpenGL_sets_this_frame[4], OpenGL_sets_this_frame[5]));
	for (i = 0; i < 10; i++)
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
	ShaderProgram::ClearBinding();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

#ifndef NDEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
	{
		Int3();
	}
#endif

#if defined(SDL3)
	SDL_GL_SwapWindow(GLWindow);
#elif defined(WIN32)	
	SwapBuffers((HDC)hOpenGLDC);
#elif defined(__LINUX__)
	SDL_GL_SwapBuffers();
#endif

	framebuffer_current_draw = (framebuffer_current_draw + 1) % NUM_GL1_FBOS;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[framebuffer_current_draw].Handle());

#ifndef NDEBUG
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

void GLCompatibilityRenderer::EndFrame()
{
}


// returns true if the passed in extension name is supported
bool GLCompatibilityRenderer::CheckExtension(char* extName)
{
	char* p = (char*)glGetString(GL_EXTENSIONS);
	int extNameLen = strlen(extName);
	char* end = p + strlen(p);

	while (p < end)
	{
		int n = strcspn(p, " ");
		if ((extNameLen == n) && (strncmp(extName, p, n) == 0))
			return true;

		p += (n + 1);
	}

	return false;
}

void GLCompatibilityRenderer::SetGammaValue(float val)
{
	blitshader.Use();
	glUniform1f(blitshader_gamma, 1.f / val);
}

void GLCompatibilityRenderer::SetFlatColor(ddgr_color color)
{
	OpenGL_state.cur_color = color;
}

// Sets the fog state to TRUE or FALSE
void GLCompatibilityRenderer::SetFogState(sbyte state)
{
	if (state == OpenGL_state.cur_fog_state)
		return;

	OpenGL_state.cur_fog_state = state;
	if (state == 1)
	{
		glEnable(GL_FOG);
	}
	else
	{
		glDisable(GL_FOG);
	}
}

// Sets the near and far plane of fog
void GLCompatibilityRenderer::SetFogBorders(float nearz, float farz)
{
	// Sets the near and far plane of fog
	float fog_start = std::max(0.f, std::min(1.0f, 1.0f - (1.0f / nearz)));
	float fog_end = std::max(0.f, std::min(1.0f, 1.0f - (1.0f / farz)));

	OpenGL_state.cur_fog_start = fog_start;
	OpenGL_state.cur_fog_end = fog_end;

	glFogi(GL_FOG_MODE, GL_LINEAR);
	glFogf(GL_FOG_START, fog_start);
	glFogf(GL_FOG_END, fog_end);
}

void GLCompatibilityRenderer::SetColorModel(color_model state)
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

void GLCompatibilityRenderer::SetTextureType(texture_type state)
{
	if (state == OpenGL_state.cur_texture_type)
		return;	// No redundant state setting
	if (UseMultitexture && Last_texel_unit_set != 0)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB + 0);
		Last_texel_unit_set = 0;
	}
	OpenGL_sets_this_frame[3]++;

	switch (state)
	{
	case TT_FLAT:
		glDisable(GL_TEXTURE_2D);
		OpenGL_state.cur_texture_quality = 0;
		break;
	case TT_LINEAR:
	case TT_LINEAR_SPECIAL:
	case TT_PERSPECTIVE:
	case TT_PERSPECTIVE_SPECIAL:
		glEnable(GL_TEXTURE_2D);
		OpenGL_state.cur_texture_quality = 2;
		break;
	default:
		Int3();	// huh? Get Jason
		break;
	}

	CHECK_ERROR(12);
	OpenGL_state.cur_texture_type = state;
}

// Sets the state of bilinear filtering for our textures
void GLCompatibilityRenderer::SetFiltering(sbyte state)
{
	OpenGL_state.cur_bilinear_state = state;
}

// Sets the state of z-buffering to on or off
void GLCompatibilityRenderer::SetZBufferState(sbyte state)
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

	CHECK_ERROR(14);
}

// Sets the near and far planes for z buffer
void GLCompatibilityRenderer::SetZValues(float nearz, float farz)
{
	OpenGL_state.cur_near_z = nearz;
	OpenGL_state.cur_far_z = farz;
	//	mprintf ((0,"OPENGL:Setting depth range to %f - %f\n",nearz,farz));
}

// Sets a bitmap as a overlay map to rendered on top of the next texture map
// a -1 value indicates no overlay map
void GLCompatibilityRenderer::SetOverlayMap(int handle)
{
	Overlay_map = handle;
}

void GLCompatibilityRenderer::SetOverlayType(ubyte type)
{
	Overlay_type = type;
}

// Clears the display to a specified color
void GLCompatibilityRenderer::ClearScreen(ddgr_color color)
{
	int r = (color >> 16 & 0xFF);
	int g = (color >> 8 & 0xFF);
	int b = (color & 0xFF);

	glClearColor((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	CHECK_ERROR(20);
}

// Clears the zbuffer for the screen
void GLCompatibilityRenderer::ClearZBuffer(void)
{
	glClear(GL_DEPTH_BUFFER_BIT);
}

// Sets the color of fog
void GLCompatibilityRenderer::SetFogColor(ddgr_color color)
{
	if (color == OpenGL_state.cur_fog_color)
		return;

	float fc[4];
	fc[0] = GR_COLOR_RED(color);
	fc[1] = GR_COLOR_GREEN(color);
	fc[2] = GR_COLOR_BLUE(color);
	fc[3] = 1;

	fc[0] /= 255.0f;
	fc[1] /= 255.0f;
	fc[2] /= 255.0f;

	glFogfv(GL_FOG_COLOR, fc);
}

// Sets the lighting state of opengl
void GLCompatibilityRenderer::SetLighting(light_state state)
{
	if (state == OpenGL_state.cur_light_state)
		return;	// No redundant state setting

	if (UseMultitexture && Last_texel_unit_set != 0)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB + 0);
		Last_texel_unit_set = 0;
	}

	OpenGL_sets_this_frame[4]++;

	switch (state)
	{
	case LS_NONE:
		glShadeModel(GL_SMOOTH);
		OpenGL_state.cur_light_state = LS_NONE;
		break;
	case LS_FLAT_GOURAUD:
		glShadeModel(GL_SMOOTH);
		OpenGL_state.cur_light_state = LS_FLAT_GOURAUD;
		break;
	case LS_GOURAUD:
	case LS_PHONG:
		glShadeModel(GL_SMOOTH);
		OpenGL_state.cur_light_state = LS_GOURAUD;
		break;
	default:
		Int3();
		break;
	}

	CHECK_ERROR(13);
}

// returns the alpha that we should use
float GLCompatibilityRenderer::GetAlphaMultiplier(void)
{
	switch (OpenGL_state.cur_alpha_type)
	{
	case AT_ALWAYS:
		return 1.0;
	case AT_CONSTANT:
		return OpenGL_state.cur_alpha / 255.0;
	case AT_TEXTURE:
		return 1.0;
	case AT_CONSTANT_TEXTURE:
		return OpenGL_state.cur_alpha / 255.0;
	case AT_VERTEX:
		return 1.0;
	case AT_CONSTANT_TEXTURE_VERTEX:
	case AT_CONSTANT_VERTEX:
		return OpenGL_state.cur_alpha / 255.0;
	case AT_TEXTURE_VERTEX:
		return 1.0;
	case AT_LIGHTMAP_BLEND:
	case AT_LIGHTMAP_BLEND_SATURATE:
		return OpenGL_state.cur_alpha / 255.0;
	case AT_SATURATE_TEXTURE:
		return OpenGL_state.cur_alpha / 255.0;
	case AT_SATURATE_VERTEX:
		return 1.0;
	case AT_SATURATE_CONSTANT_VERTEX:
		return OpenGL_state.cur_alpha / 255.0;
	case AT_SATURATE_TEXTURE_VERTEX:
		return 1.0;
	case AT_SPECULAR:
		return 1.0;
	default:
		//Int3();		// no type defined,get jason
		return 0;
	}
}


void GLCompatibilityRenderer::SetAlwaysAlpha(bool state)
{
	if (state && OpenGL_blending_on)
	{
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		OpenGL_blending_on = false;
	}
	else if (!state)
	{
		glEnable(GL_BLEND);
		glEnable(GL_ALPHA_TEST);
		OpenGL_blending_on = true;
	}
}

void GLCompatibilityRenderer::SetAlphaType(sbyte atype)
{
	if (atype == OpenGL_state.cur_alpha_type)
		return;		// don't set it redundantly
	if (UseMultitexture && Last_texel_unit_set != 0)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB + 0);
		Last_texel_unit_set = 0;

	}
	OpenGL_sets_this_frame[6]++;

	OpenGL_blending_on = true;
	SetAlwaysAlpha(true);
	glBlendFunc(GL_ONE, GL_ZERO);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);

	switch (atype)
	{
	case AT_ALWAYS:
		rend_SetAlphaValue(255);
		SetAlwaysAlpha(true);
		glBlendFunc(GL_ONE, GL_ZERO);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_CONSTANT:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_TEXTURE:
		rend_SetAlphaValue(255);
		SetAlwaysAlpha(true);
		glBlendFunc(GL_ONE, GL_ZERO);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		break;
	case AT_CONSTANT_TEXTURE:
	case AT_CONSTANT_TEXTURE_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_TEXTURE_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_PRIMARY_COLOR);
		//I'm not sure why this works? The terminal switches in level 1 use a alpha texture and it works fine in D3D. 
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PRIMARY_COLOR);
		break;
	case AT_CONSTANT_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_LIGHTMAP_BLEND:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_SATURATE_TEXTURE:
	case AT_LIGHTMAP_BLEND_SATURATE:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_SATURATE_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_SATURATE_CONSTANT_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_SATURATE_TEXTURE_VERTEX:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		break;
	case AT_SPECULAR:
		SetAlwaysAlpha(false);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		//hack
		glEnable(GL_TEXTURE_2D);
		OpenGL_state.cur_texture_quality = 2;
		OpenGL_state.cur_texture_type = TT_PERSPECTIVE;

		glGetError();
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_PRIMARY_COLOR);
		if (glGetError() != GL_NO_ERROR)
			Int3();

		break;
	default:
		Int3();		// no type defined,get jason
		break;
	}
	OpenGL_state.cur_alpha_type = atype;
	Alpha_multiplier = GetAlphaMultiplier();
	CHECK_ERROR(15);
}

// Sets the alpha value for constant alpha
void GLCompatibilityRenderer::SetAlphaValue(ubyte val)
{
	OpenGL_state.cur_alpha = val;
	Alpha_multiplier = GetAlphaMultiplier();
}

// Sets the overall alpha scale factor (all alpha values are scaled by this value)
// usefull for motion blur effect
void GLCompatibilityRenderer::SetAlphaFactor(float val)
{
	if (val < 0.0f) val = 0.0f;
	if (val > 1.0f) val = 1.0f;
	OpenGL_Alpha_factor = val;
}

// Returns the current Alpha factor
float GLCompatibilityRenderer::GetAlphaFactor(void)
{
	return OpenGL_Alpha_factor;
}

// Sets the texture wrapping type
void GLCompatibilityRenderer::SetWrapType(wrap_type val)
{
	OpenGL_state.cur_wrap_type = val;
}

void GLCompatibilityRenderer::SetZBias(float z_bias)
{
	if (Z_bias != z_bias)
	{
		Z_bias = z_bias;
	}
}

// Enables/disables writes the depth buffer
void GLCompatibilityRenderer::SetZBufferWriteMask(int state)
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
void GLCompatibilityRenderer::GetProjectionParameters(int* width, int* height)
{
	*width = OpenGL_state.clip_x2 - OpenGL_state.clip_x1;
	*height = OpenGL_state.clip_y2 - OpenGL_state.clip_y1;
}

void GLCompatibilityRenderer::GetProjectionScreenParameters(int& screenLX, int& screenTY, int& screenW, int& screenH)
{
	screenLX = OpenGL_state.clip_x1;
	screenTY = OpenGL_state.clip_y1;
	screenW = OpenGL_state.clip_x2 - OpenGL_state.clip_x1 + 1;
	screenH = OpenGL_state.clip_y2 - OpenGL_state.clip_y1 + 1;
}

// Returns the aspect ratio of the physical screen
float GLCompatibilityRenderer::GetAspectRatio(void)
{
	float aspect_ratio = (float)((3.0f * OpenGL_state.screen_width) / (4.0f * OpenGL_state.screen_height));
	return aspect_ratio;
}

// Sets the hardware bias level for coplanar polygons
// This helps reduce z buffer artifacts
void GLCompatibilityRenderer::SetCoplanarPolygonOffset(float factor)
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

void GLCompatibilityRenderer::SetCullFace(bool state)
{
	if (state)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
}

// Preuploads a texture to the video card
void GLCompatibilityRenderer::PreUploadTextureToCard(int handle, int map_type)
{
}

// Frees an uploaded texture from the video card
void GLCompatibilityRenderer::FreePreUploadedTexture(int handle, int map_type)
{
}

// Returns 1 if there is mid video memory, 2 if there is low vid memory, or 0 if there is large vid memory
int GLCompatibilityRenderer::LowVidMem(void)
{
	return 0;
}

// Returns 1 if the renderer supports bumpmapping
int GLCompatibilityRenderer::SupportsBumpmapping(void)
{
	return 0;
}

// Sets a bumpmap to be rendered, or turns off bumpmapping altogether
void GLCompatibilityRenderer::SetBumpmapReadyState(int state, int map)
{
}

// returns rendering statistics for the frame
void GLCompatibilityRenderer::GetStatistics(tRendererStats* stats)
{
	stats->poly_count = OpenGL_last_frame_polys_drawn;
	stats->vert_count = OpenGL_last_frame_verts_processed;
	stats->texture_uploads = OpenGL_last_uploaded;
}

// Tells the software renderer whether or not to use mipping
void GLCompatibilityRenderer::SetMipState(sbyte mipstate)
{
	OpenGL_state.cur_mip_state = mipstate;
}

// Fills in the passed in pointer with the current rendering state
void GLCompatibilityRenderer::GetRenderState(rendering_state* rstate)
{
	memcpy(rstate, &OpenGL_state, sizeof(rendering_state));
}

void GLCompatibilityRenderer::DLLGetRenderState(DLLrendering_state* rstate)
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
void GLCompatibilityRenderer::Screenshot(int bm_handle)
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

	framebuffers[framebuffer_current_draw].BindForRead();
	glReadPixels(0, 0, OpenGL_state.screen_width, OpenGL_state.screen_height, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)temp_data);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

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

void GLCompatibilityRenderer::UpdateFramebuffer(void)
{
	for (int i = 0; i < NUM_GL1_FBOS; i++)
	{
		framebuffers[i].Update(OpenGL_state.screen_width, OpenGL_state.screen_height, OpenGL_preferred_state.antialised);
	}

	framebuffer_current_draw = 0;
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffers[0].Handle());
	//Unbind the read framebuffer so that OBS can capture the window properly
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	GL_InitFramebufferVAO();
}

void GLCompatibilityRenderer::CloseFramebuffer(void)
{
	for (int i = 0; i < NUM_GL1_FBOS; i++)
	{
		framebuffers[i].Destroy();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_DestroyFramebufferVAO();
}

void GLCompatibilityRenderer::GetScreenSize(int& screen_width, int& screen_height)
{
	screen_width = OpenGL_state.screen_width;
	screen_height = OpenGL_state.screen_height;
}

GLCompatibilityRenderer::GLCompatibilityRenderer()
{
}

//A bunch of stuff that doesn't make sense in the compatibility renderer, but alas I must have for the moment
void GLCompatibilityRenderer::UpdateCommon(float* projection, float* modelview, int depth)
{
}

void GLCompatibilityRenderer::SetCommonDepth(int depth)
{
}

uint32_t GLCompatibilityRenderer::GetPipelineByName(const char* name)
{
	return 0xFFFFFFFF;
}

void GLCompatibilityRenderer::BindPipeline(uint32_t handle)
{
}

void GLCompatibilityRenderer::UpdateSpecular(SpecularBlock* specularstate)
{
}

void GLCompatibilityRenderer::UpdateFogBrightness(RoomBlock* roomstate, int numrooms)
{
}

void GLCompatibilityRenderer::SetCurrentRoomNum(int roomblocknum)
{
}

void GLCompatibilityRenderer::UpdateTerrainFog(float color[4], float start, float end)
{
}

void GLCompatibilityRenderer::UseShaderTest()
{
}

void GLCompatibilityRenderer::EndShaderTest()
{
}

void GLCompatibilityRenderer::BindBitmap(int handle)
{
}

void GLCompatibilityRenderer::BindLightmap(int handle)
{
}

void GLCompatibilityRenderer::RestoreLegacy()
{
	glBindVertexArray(0);
}
