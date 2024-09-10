/*
* Descent 3: Piccu Engine
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
#include "gl_mesh.h"

VertexBuffer::VertexBuffer() : VertexBuffer(true, false)
{
}

VertexBuffer::VertexBuffer(bool allow_dynamic, bool dynamic_hint)
{
	m_name = 0;
	m_vaoname = 0;
	m_size = 0;
	m_vertexcount = 0;
	m_appendcounter = 0;
	m_dynamic_hint = dynamic_hint;
}

void VertexBuffer::Initialize(uint32_t numvertices, uint32_t datasize, void* data)
{
	if (m_vaoname == 0)
	{
		glGenVertexArrays(1, &m_vaoname);
		glGenBuffers(1, &m_name);
	}

	m_appendcounter = 0;
	glBindVertexArray(m_vaoname);

	glBindBuffer(GL_ARRAY_BUFFER, m_name);
	glBufferData(GL_ARRAY_BUFFER, datasize, data, m_dynamic_hint ? GL_STREAM_DRAW : GL_STATIC_DRAW);

	//Create the standard vertex attributes
	//Position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(RendVertex), (void*)offsetof(RendVertex, position));

	//Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_BYTE, GL_TRUE, sizeof(RendVertex), (void*)offsetof(RendVertex, r));

	//Normal
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(RendVertex), (void*)offsetof(RendVertex, normal));

	//Lightmap page
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 1, GL_INT, sizeof(RendVertex), (void*)offsetof(RendVertex, lmpage));

	//Base UV
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, sizeof(RendVertex), (void*)offsetof(RendVertex, u1));

	//Overlay UV
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(RendVertex), (void*)offsetof(RendVertex, u2));

	//UV slide
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(RendVertex), (void*)offsetof(RendVertex, u2));

	m_size = datasize;
	m_vertexcount = numvertices;

	rend_RestoreLegacy();
}

void VertexBuffer::Update(uint32_t byteoffset, uint32_t datasize, void* data)
{
	assert(m_vaoname != 0);
	assert(byteoffset + datasize <= m_size);
	glBindVertexArray(m_vaoname);

	glBindBuffer(GL_ARRAY_BUFFER, m_name);
	glBufferSubData(GL_ARRAY_BUFFER, byteoffset, datasize, data);
}

uint32_t VertexBuffer::Append(uint32_t size, void* data)
{
	assert(m_vaoname != 0);
	assert(size <= m_size);
	glBindVertexArray(m_vaoname);

	glBindBuffer(GL_ARRAY_BUFFER, m_name);
	if (m_appendcounter + size > m_size)
	{
		m_appendcounter = 0;
		glBufferData(GL_ARRAY_BUFFER, m_size, nullptr, m_dynamic_hint ? GL_STREAM_DRAW : GL_STATIC_DRAW);
	}
	glBufferSubData(GL_ARRAY_BUFFER, m_appendcounter, size, data);
	uint32_t initial = m_appendcounter;
	m_appendcounter += size;

	return initial;
}

void VertexBuffer::Bind() const
{
	assert(m_vaoname != 0);
	glBindVertexArray(m_vaoname);
}

void VertexBuffer::BindBitmap(int bmhandle) const
{
	rend_BindBitmap(bmhandle);
}

void VertexBuffer::BindLightmap(int lmhandle) const
{
	rend_BindLightmap(lmhandle);
}

static GLenum GetGLPrimitiveType(PrimitiveType mode)
{
	switch (mode)
	{
	case PrimitiveType::Triangles:
		return GL_TRIANGLES;
	case PrimitiveType::Lines:
		return GL_LINES;
	case PrimitiveType::Points:
		return GL_POINTS;
	}

	return GL_TRIANGLES; //blarg
}

void VertexBuffer::Draw(PrimitiveType mode) const
{
	glDrawArrays(GetGLPrimitiveType(mode), 0, m_vertexcount);
}

void VertexBuffer::Draw(PrimitiveType mode, ElementRange range) const
{
	assert(range.offset + range.count <= m_vertexcount);
	glDrawArrays(GetGLPrimitiveType(mode), range.offset, range.count);
}

void VertexBuffer::DrawIndexed(PrimitiveType mode, ElementRange range) const
{
	glDrawElements(GetGLPrimitiveType(mode), range.count, GL_UNSIGNED_INT, (const void*)(range.offset * sizeof(uint32_t)));
}

void VertexBuffer::Destroy()
{
	if (m_vaoname != 0)
	{
		glDeleteBuffers(1, &m_name);
		glDeleteVertexArrays(1, &m_vaoname);
		m_name = m_vaoname = 0;
	}
	m_size = m_vertexcount = 0;
}

IndexBuffer::IndexBuffer() : IndexBuffer(true, false)
{
}

IndexBuffer::IndexBuffer(bool allow_dynamic, bool dynamic_hint)
{
	m_name = 0;
	m_size = 0;
	m_dynamic_hint = dynamic_hint;
}

void IndexBuffer::Initialize(uint32_t numindices, uint32_t datasize, void* data)
{
	if (m_size != 0 && datasize > m_size)
	{
		//Need to make a new buffer
		Destroy();
	}

	if (m_name == 0)
	{
		glGenBuffers(1, &m_name);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, datasize, data, m_dynamic_hint ? GL_STREAM_DRAW : GL_STATIC_DRAW);
		m_size = datasize;
	}
	else
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, datasize, data, m_dynamic_hint ? GL_STREAM_DRAW : GL_STATIC_DRAW); //always orphan, maybe faster for dynamic meshes?
		//glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, datasize, data);
	}
}

void IndexBuffer::Update(uint32_t byteoffset, uint32_t datasize, void* data)
{
	assert(m_name != 0);
	assert(byteoffset + datasize <= m_size);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, byteoffset, datasize, data);
}

void IndexBuffer::Bind() const
{
	assert(m_name != 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_name);
}

void IndexBuffer::Destroy()
{
	if (m_name != 0)
	{
		glDeleteBuffers(1, &m_name);
		m_name = 0;
	}
}

void rendTEMP_UnbindVertexBuffer()
{
	rend_RestoreLegacy();
}
