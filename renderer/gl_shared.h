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

//gl_shared.h
//Things shared by both the compatibility and core implementations. 

#pragma once
#include <glad/gl.h>
#include "pserror.h"

//uncomment to express your love for the best graphics API ever designed and enable extra error checking to show your love. 
//#define I_LOVE_OPENGL

#ifdef I_LOVE_OPENGL
constexpr void CHECK_ERROR(int n) //need to decide what it does. 
{
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
	{
		mprintf((1, "GL Error in context %d: %x\n", n, err));
		Int3();
	}
}
#else
#define CHECK_ERROR(n)
#endif

struct CommonBlock
{
	float projection[16];
	float modelview[16];
	float pad[32];
};

//Shader definition nonsense
constexpr int SF_HASCOMMON = 1; //Shader will use the common block
constexpr int SF_HASROOM = 2; //Shader will use the room (fog, brightness) block
constexpr int SF_HASSPECULAR = 4; //Shader will use the specular block

struct ShaderDefinition
{
	const char* name;
	int flags; //see SF_ flags above
	const char* vertex_filename;
	const char* fragment_filename;
};

class ShaderProgram
{
	GLuint m_name;
	//CreateCommonBindings will find common uniforms and set their default bindings.
	//This includes the common block, which must be named "common",
	//and sampler2Ds named "colortexture", "lightmaptexture", and others later.
	void CreateCommonBindings(int bindindex);
public:
	ShaderProgram()
	{
		m_name = 0;
	}

	void AttachSource(const char* vertexsource, const char* fragsource);
	void AttachSourceFromDefiniton(ShaderDefinition& def);
	//Attaches strings with some preprocessor statements. 
	//Defines USE_TEXTURING if textured is true.
	//Defines USE_LIGHTMAP if lightmapped is true.
	//Defines USE_SPECULAR if speculared is true. 
	void AttachSourcePreprocess(const char* vertexsource, const char* fragsource, bool textured, bool lightmapped, bool speculared, bool fogged);
	GLint FindUniform(const char* uniform);
	void Destroy();

	void Use();

	//Replacement for glUseProgram(0) that nulls the last binding. 
	static void ClearBinding();

	GLuint Handle() const
	{
		return m_name;
	}
};

class Framebuffer
{
	GLuint		m_name, m_subname;
	GLuint		m_colorname, m_subcolorname, m_depthname;
	uint32_t	m_width, m_height;
	bool		m_msaa;

	//Used when multisampling is enabled. Blits the multisample framebuffer to the non-multisample sub framebuffer
	//Leaves the sub framebuffer bound for reading to finish the blit. 
	void SubColorBlit();
public:
	Framebuffer();
	void Update(int width, int height, bool msaa);
	void Destroy();
	//Blits to the target framebuffer using glBlitFramebuffer.
	//Will set current read framebuffer to m_name.
	void BlitToRaw(GLuint target, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
	//Blits to the target framebuffer using a draw. Bind desired shader before calling. 
	//Will set current read framebuffer to m_name. Will not trash viewport. 
	void BlitTo(GLuint target, unsigned int x, unsigned int y, unsigned int w, unsigned int h);

	//When called without MSAA, just binds m_name to the read slot.
	//When called with MSAA, will resolve to m_subname and bind that. 
	void BindForRead();

	GLuint Handle() const
	{
		return m_name;
	}
};

#if defined(WIN32) && !defined(SDL3)
extern PFNWGLSWAPINTERVALEXTPROC dwglSwapIntervalEXT;
extern PFNWGLCREATECONTEXTATTRIBSARBPROC dwglCreateContextAttribsARB;
#endif
extern bool Already_loaded;

//A dumb bit of global state that needs to be fixed
void GL_InitFramebufferVAO();

void GL_DestroyFramebufferVAO();
