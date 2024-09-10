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

#pragma once

#include <stdint.h>
#include <vector>
#include "pstypes.h"
#include "vecmat.h"
#include "renderer.h"

//A batch of 0-2 texture handles.
struct MeshBatch
{
	int primaryhandle;
	int secondaryhandle;
	int vertexoffset, vertexcount;
	int indexoffset, indexcount;
};

//Future note: These will need to become interfaces if Vulkan support is added. 
//The same MeshBuilder should be usable across any API. 
class VertexBuffer : public IVertexBuffer
{
	uint32_t m_name, m_vaoname;
	uint32_t m_size;
	uint32_t m_vertexcount;
	uint32_t m_appendcounter;
	bool m_dynamic_hint;
public:
	VertexBuffer();
	VertexBuffer(bool allow_dynamic, bool dynamic_hint);

	void Initialize(uint32_t numvertices, uint32_t datasize, void* data) override;
	//Performs a dynamic update of part of the vertex buffer.
	//This cannot change the size of the underlying buffer object.
	void Update(uint32_t byteoffset, uint32_t datasize, void* data) override;
	//Used for streaming buffers, appends data to the end of the buffer.
	//When the buffer fills, will orphan the previous one to avoid sync if possible. 
	//Buffer must have been initialized before using. Returns offset of data. 
	uint32_t Append(uint32_t size, void* data) override;
	void Bind() const override;

	//TODO: Temp interface to load textures
	void BindBitmap(int bmhandle) const override;
	void BindLightmap(int lmhandle) const override;

	//Draws all the vertices in the buffer in one go
	void Draw(PrimitiveType mode) const override;
	//Draws a range of vertices from the buffer.
	void Draw(PrimitiveType mode, ElementRange range) const override;
	//Draws a range of vertices from the buffer, from the range of the currently bound index buffer
	void DrawIndexed(PrimitiveType mode, ElementRange range) const override;

	void Destroy() override;
};

class IndexBuffer : public IIndexBuffer
{
	uint32_t m_name;
	uint32_t m_size;
	bool m_dynamic_hint;
public:
	IndexBuffer();
	IndexBuffer(bool allow_dynamic, bool dynamic_hint);

	void Initialize(uint32_t numindices, uint32_t datasize, void* data) override;
	//Performs a dynamic update of part of the index buffer.
	//This cannot change the size of the underlying buffer object.
	void Update(uint32_t byteoffset, uint32_t datasize, void* data) override;

	void Bind() const override;

	void Destroy() override;
};


//TEMP CODE: Sets vertex buffer back to draw vertex buffer. 
//Call at the end of all mesh based rendering
void rendTEMP_UnbindVertexBuffer();
