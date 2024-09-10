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
#include "args.h"
#ifdef WIN32
#define NOMINMAX
#include <Windows.h>
#endif

// Sets default states for our renderer
void GLCompatibilityRenderer::SetDefaults()
{
	mprintf((0, "Setting states\n"));

	OpenGL_state.cur_color = 0x00FFFFFF;
	OpenGL_state.cur_bilinear_state = -1;
	OpenGL_state.cur_zbuffer_state = -1;
	OpenGL_state.cur_texture_quality = -1;
	OpenGL_state.cur_light_state = LS_GOURAUD;
	OpenGL_state.cur_color_model = CM_MONO;
	OpenGL_state.cur_mip_state = -1;
	OpenGL_state.cur_alpha_type = AT_TEXTURE;

	// Enable some states
	glAlphaFunc(GL_GREATER, 0);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_DITHER);
	OpenGL_blending_on = true;

#ifdef _DEBUG
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

#ifndef RELEASE
	if (Fast_test_render)
	{
		glDisable(GL_DITHER);
	}
#endif

	SetAlphaType(AT_ALWAYS);
	SetAlphaValue(255);
	SetFiltering(1);
	SetLighting(LS_NONE);
	SetTextureType(TT_FLAT);
	SetColorModel(CM_RGB);
	SetZBufferState(1);
	SetZValues(0, 3000);
	OpenGL_last_bound[0] = 9999999;
	OpenGL_last_bound[1] = 9999999;
	Last_texel_unit_set = -1;
	OpenGL_multitexture_state = false;

#ifdef _DEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

	SetDrawDefaults();

#ifdef _DEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glHint(GL_FOG_HINT, GL_NICEST);
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, OpenGL_state.screen_width, OpenGL_state.screen_height);
	glDisable(GL_SCISSOR_TEST);
	glDepthRange(0.0f, 1.0f);

#ifdef _DEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

	if (UseMultitexture)
	{
		glActiveTextureARB(GL_TEXTURE0_ARB + 1);
		glClientActiveTextureARB(GL_TEXTURE0_ARB + 1);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glHint(GL_FOG_HINT, GL_NICEST);

		glClientActiveTextureARB(GL_TEXTURE0_ARB + 0);

		glDisable(GL_TEXTURE_2D);
		glAlphaFunc(GL_GREATER, 0);
		glEnable(GL_ALPHA_TEST);
		glEnable(GL_BLEND);
		glEnable(GL_DITHER);
		glBlendFunc(GL_DST_COLOR, GL_ZERO);
		glActiveTextureARB(GL_TEXTURE0_ARB + 0);

#ifdef _DEBUG
		err = glGetError();
		if (err != GL_NO_ERROR)
			Int3();
#endif
	}
}

#if defined(WIN32)
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

// Check for OpenGL support, 
int GLCompatibilityRenderer::Setup(HDC glhdc)
{
	// Finds an acceptable pixel format to render to
	PIXELFORMATDESCRIPTOR pfd, pfd_copy;
	int pf;

	memset(&pfd, 0, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_GENERIC_ACCELERATED;
	pfd.iPixelType = PFD_TYPE_RGBA;

	// Find the user's "best match" PFD 
	pf = ChoosePixelFormat(glhdc, &pfd);
	if (pf == 0)
	{
		Int3();
		//FreeLibrary(opengl_dll_handle);
		return NULL;
	}

	mprintf((0, "Choose pixel format successful!\n"));

	// Try and set the new PFD
	if (SetPixelFormat(glhdc, pf, &pfd) == FALSE)
	{
		DWORD ret = GetLastError();
		Int3();
		//FreeLibrary(opengl_dll_handle);
		return NULL;
	}

	mprintf((0, "SetPixelFormat successful!\n"));

	// Get a copy of the newly set PFD
	if (DescribePixelFormat(glhdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd_copy) == 0)
	{
		Int3();
		//FreeLibrary(opengl_dll_handle);
		return NULL;
	}

	// Check the returned PFD to see if it is hardware accelerated
	if ((pfd_copy.dwFlags & PFD_GENERIC_ACCELERATED) == 0 && (pfd_copy.dwFlags & PFD_GENERIC_FORMAT) != 0)
	{
		Int3();
		//FreeLibrary(opengl_dll_handle);
		return NULL;
	}

	// Create an OpenGL context, and make it the current context
	ResourceContext = wglCreateContext((HDC)glhdc);
	if (ResourceContext == NULL) {
		DWORD ret = GetLastError();
		//FreeLibrary(opengl_dll_handle);
		Int3();
		return NULL;
	}

	ASSERT(ResourceContext != NULL);
	mprintf((0, "Making context current\n"));
	if (!wglMakeCurrent((HDC)glhdc, ResourceContext))
		Int3();

	if (!Already_loaded)
	{
		if (!gladLoadGL(opengl_GLADLoad))
		{
			rend_SetErrorMessage("Failed to load opengl dll!\n");
			Int3();
			return 0;
		}
	}

	dwglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)opengl_GLADLoad("wglSwapIntervalEXT");

	Already_loaded = true;

	return 1;

}
#endif

// Gets some specific information about this particular flavor of opengl
void GLCompatibilityRenderer::GetInformation()
{
	mprintf((0, "OpenGL Vendor: %s\n", glGetString(GL_VENDOR)));
	mprintf((0, "OpenGL Renderer: %s\n", glGetString(GL_RENDERER)));
	mprintf((0, "OpenGL Version: %s\n", glGetString(GL_VERSION)));
	mprintf((0, "OpenGL Extensions: %s\n", glGetString(GL_EXTENSIONS)));
}

// Sets up our OpenGL rendering context
// Returns 1 if ok, 0 if something bad
int GLCompatibilityRenderer::Init(oeApplication* app, renderer_preferred_state* pref_state)
{
	//int width,height;
	int retval = 1;
	int i;

	mprintf((0, "Setting up opengl mode!\n"));

	if (pref_state)
	{
		OpenGL_preferred_state = *pref_state;
	}

	if (app != NULL)
	{
		ParentApplication = app;
	}

	int windowX = 0, windowY = 0;
#if defined(WIN32)
	/***********************************************************
	*               WINDOWS OPENGL
	***********************************************************
	*/
	static HWnd hwnd = NULL;
	if (ParentApplication != NULL)
	{
		hwnd = static_cast<HWnd>(reinterpret_cast<oeWin32Application*>(ParentApplication)->m_hWnd);
	}

	memset(&OpenGL_state, 0, sizeof(rendering_state));

	//	These values are set here - samir
	if (app != NULL)
	{
		hOpenGLWnd = (HWND)((oeWin32Application*)app)->m_hWnd;
	}

	hOpenGLDC = GetDC(hOpenGLWnd);

	InitImages();
	UpdateWindow();

	if (!Setup(hOpenGLDC))
	{
		//opengl_Close();
		return 0;
	}

#elif defined(__LINUX__)
	/***********************************************************
	*               LINUX OPENGL
	***********************************************************
	*/
	// Setup OpenGL_state.screen_width & OpenGL_state.screen_height & width & height
	width = OpenGL_preferred_state.width;
	height = OpenGL_preferred_state.height;

	if (!opengl_Setup(app, &width, &height))
	{
		opengl_Close();
		return 0;
	}

	memset(&OpenGL_state, 0, sizeof(rendering_state));
	OpenGL_state.screen_width = width;
	OpenGL_state.screen_height = height;
#else
	// Setup OpenGL_state.screen_width & OpenGL_state.screen_height & width & height

#endif
	// Get some info
	GetInformation();

	// Default passthrough viewport. 
	SetViewport();

	// Update framebuffer
	UpdateFramebuffer();

#ifdef _DEBUG
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

	mprintf((0, "Setting up multitexture...\n"));

	// Determine if Multitexture is supported
	bool supportsMultiTexture = CheckExtension("GL_ARB_multitexture");

	if (FindArg("-NoMultitexture"))
		supportsMultiTexture = false;

	if (supportsMultiTexture)
		UseMultitexture = true;
	else
		// No multitexture at all
		UseMultitexture = false;

	// Do we have packed pixel formats?
	OpenGL_packed_pixels = CheckExtension("GL_EXT_packed_pixels");

	InitCache();

#ifdef _DEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

	if (UseMultitexture)
		mprintf((0, "Using multitexture."));
	else
		mprintf((0, "Not using multitexture."));

	SetUploadBufferSize(256, 256);
	SetDefaults();

#ifdef _DEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

	//g3_ForceTransformRefresh();

	CHECK_ERROR(4);

	OpenGL_state.initted = 1;

	mprintf((0, "OpenGL initialization at %d x %d was successful.\n", OpenGL_state.screen_width, OpenGL_state.screen_height));

#ifdef WIN32
	if (dwglSwapIntervalEXT)
	{
		if (pref_state->vsync_on)
			dwglSwapIntervalEXT(1);
		else
			dwglSwapIntervalEXT(0);
	}
#endif

	extern const char* blitVertexSrc;
	extern const char* blitFragmentSrc;
	blitshader.AttachSource(blitVertexSrc, blitFragmentSrc);
	blitshader_gamma = blitshader.FindUniform("gamma");
	if (blitshader_gamma == -1)
		Error("rend_Init: Failed to find gamma uniform!");

	//[ISB] moved here.. stupid. 
	SetGammaValue(OpenGL_preferred_state.gamma);

	return retval;
}

// Releases the rendering context
void GLCompatibilityRenderer::Close()
{
	CHECK_ERROR(5);

	blitshader.Destroy();
	FreeImages();
	CloseFramebuffer();

#if defined(WIN32)
	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(ResourceContext);

#elif defined(__LINUX__)
	// SDL_Quit() handles this for us.
#else

#endif

	FreeUploadBuffers();

	FreeCache();

#if defined(WIN32)
	//	I'm freeing the DC here - samir
	ReleaseDC(hOpenGLWnd, hOpenGLDC);
#endif

	//mod_FreeModule (OpenGLDLLHandle);
	OpenGL_state.initted = 0;
}

