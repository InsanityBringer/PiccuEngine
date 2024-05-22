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
#include <string.h>
#include "gl_shader.h"
#include "pserror.h"

GLuint commonbuffername;

void opengl_InitCommonBuffer(void)
{
	glGenBuffers(1, &commonbuffername);
	glBindBuffer(GL_COPY_WRITE_BUFFER, commonbuffername);
	glBufferData(GL_COPY_WRITE_BUFFER, sizeof(CommonBlock), nullptr, GL_DYNAMIC_READ);

	//Ensure this is always ready for usage later. 
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, commonbuffername);
}

void rend_UpdateCommon(float* projection, float* modelview)
{
	CommonBlock newblock;
	memcpy(newblock.projection, projection, sizeof(newblock.projection));
	memcpy(newblock.modelview, modelview, sizeof(newblock.modelview));

	glBindBuffer(GL_COPY_WRITE_BUFFER, commonbuffername);
	glBufferSubData(GL_COPY_WRITE_BUFFER, 0, sizeof(CommonBlock), &newblock);
}

static GLuint CompileShader(GLenum type, const char* src)
{
	GLuint name = glCreateShader(type);
	GLint length = strlen(src);
	glShaderSource(name, 1, &src, &length);
	glCompileShader(name);
	GLint status;
	glGetShaderiv(name, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(name, GL_INFO_LOG_LENGTH, &length);
		char* buf = new char[length];
		glGetShaderInfoLog(name, length, &length, buf);

		Error("CompileShader: Failed to compile shader! This error message needs more context..\n%s", buf);
	}

	return name;
}

void ShaderProgram::CreateCommonBindings()
{
	Use();

	//Find colortexture
	GLint index = glGetUniformLocation(m_name, "colortexture");
	if (index != -1)
		glUniform1i(index, 0); //Set to GL_TEXTURE0

	//Find lightmaptexture
	index = glGetUniformLocation(m_name, "lightmaptexture");
	if (index != -1)
		glUniform1i(index, 1); //Set to GL_TEXTURE1

	//Find CommonBlock
	GLuint uboindex = glGetUniformBlockIndex(m_name, "CommonBlock");
	if (uboindex != GL_INVALID_INDEX)
	{
		//Bind to GL_UNIFORM_BUFFER 0. Do I actually need to do this
		glUniformBlockBinding(m_name, uboindex, 0);
	}

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();

	glUseProgram(0);
}

void ShaderProgram::AttachSource(const char* vertexsource, const char* fragsource)
{
	GLuint vertexprog = CompileShader(GL_VERTEX_SHADER, vertexsource);
	GLuint fragmentprog = CompileShader(GL_FRAGMENT_SHADER, fragsource);

	m_name = glCreateProgram();
	glAttachShader(m_name, vertexprog);
	glAttachShader(m_name, fragmentprog);
	glLinkProgram(m_name);
	GLint status;
	glGetProgramiv(m_name, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(m_name, GL_INFO_LOG_LENGTH, &length);
		char* buf = new char[length];
		glGetProgramInfoLog(m_name, length, &length, buf);

		Error("ShaderProgram::AttachSource: Failed to link program! This error message needs more context..\n%s", buf);
	}

	glDeleteShader(vertexprog);
	glDeleteShader(fragmentprog);

	CreateCommonBindings();
}

GLint ShaderProgram::FindUniform(const char* uniform)
{
	return glGetUniformLocation(m_name, uniform);
}

void ShaderProgram::Destroy()
{
	glUseProgram(0);
	glDeleteProgram(m_name);
	m_name = 0;
}
