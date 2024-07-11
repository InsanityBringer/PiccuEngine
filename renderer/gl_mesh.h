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

//A sortable element is used to batch up elements by their texture and lightmap handle, if used. 
struct SortableElement
{
	int element;
	ushort texturehandle;
	ushort lmhandle;

	friend bool operator<(const SortableElement& l, const SortableElement& r)
	{
		uint lh = l.texturehandle | l.lmhandle << 16;
		uint rh = r.texturehandle | r.lmhandle << 16;

		return lh < rh;
	}
};

struct RendVertex
{
	vector position;
	ubyte r, g, b, a;
	vector normal;
	int lmpage;
	float u1, v1;
	float u2, v2;
	float uslide, vslide; //only slide uv1 for the moment
};

//A batch of 0-2 texture handles.
struct MeshBatch
{
	int primaryhandle;
	int secondaryhandle;
	int vertexoffset, vertexcount;
	int indexoffset, indexcount;
};

//Start and element count of a pass of a mesh
struct ElementRange
{
	uint32_t offset, count;
	ElementRange()
	{
		offset = count = 0;
	}
	ElementRange(uint32_t offset, uint32_t count) : offset(offset), count(count)
	{
	}
};

//Future note: These will need to become interfaces if Vulkan support is added. 
//The same MeshBuilder should be usable across any API. 
class VertexBuffer
{
	uint32_t m_name, m_vaoname;
	uint32_t m_size;
	uint32_t m_vertexcount;
	bool m_dynamic_hint;
public:
	VertexBuffer();
	VertexBuffer(bool allow_dynamic, bool dynamic_hint);

	void Initialize(uint32_t numvertices, uint32_t datasize, void* data);
	//Performs a dynamic update of part of the vertex buffer.
	//This cannot change the size of the underlying buffer object.
	void Update(uint32_t byteoffset, uint32_t datasize, void* data);
	void Bind() const;

	//TODO: Temp interface to load textures
	void BindBitmap(int bmhandle) const;
	void BindLightmap(int lmhandle) const;

	//Draws all the vertices in the buffer in one go
	void Draw() const;
	//Draws a range of vertices from the buffer.
	void Draw(ElementRange range) const;
	//Draws a range of vertices from the buffer, from the range of the currently bound index buffer
	void DrawIndexed(ElementRange range) const;

	void Destroy();
};

class IndexBuffer
{
	uint32_t m_name;
	uint32_t m_size;
	bool m_dynamic_hint;
public:
	IndexBuffer();
	IndexBuffer(bool allow_dynamic, bool dynamic_hint);

	void Initialize(uint32_t numindices, uint32_t datasize, void* data);
	//Performs a dynamic update of part of the index buffer.
	//This cannot change the size of the underlying buffer object.
	void Update(uint32_t byteoffset, uint32_t datasize, void* data);

	void Bind() const;

	void Destroy();
};


//Should this be split into specialized builders for vertex and index buffers?
class MeshBuilder
{
	bool m_initialized;
	bool m_vertexstarted;

	uint32_t m_vertexstartoffset;
	uint32_t m_vertexstartcount;

	bool m_indexstarted;
	uint32_t m_indexstartoffset;
	uint32_t m_indexstartcount;

	std::vector<RendVertex> m_vertices;
	std::vector<uint32_t> m_indicies;

public:
	MeshBuilder();

	//Begins submitting vertices for a render pass.
	void BeginVertices();
	//Begins submitting indices for a render pass.
	void BeginIndices();

	//Adds vertices to the vertex buffer
	void SetVertices(int numverts, RendVertex* vertices);
	//For when there's no reason to batch up a bunch
	void AddVertex(RendVertex& vertex);

	//Adds indicies to the vertex buffer
	void SetIndicies(int numindices, int* indicies);

	ElementRange EndVertices();
	ElementRange EndIndices();

	void BuildVertices(VertexBuffer& buffer);
	void BuildIndicies(IndexBuffer& buffer);

	void UpdateVertices(VertexBuffer& buffer, uint32_t offset);
	void UpdateIndicies(IndexBuffer& buffer, uint32_t offset);

	void Destroy();

	//For dynamic meshes
	void Reset();


	int NumVertices() const
	{
		return m_vertices.size();
	}

	int NumIndices() const
	{
		return m_indicies.size();
	}

	uint32_t VertexOffset() const
	{
		return m_vertices.size() * sizeof(m_vertices[0]);
	}

	uint32_t IndexOffset() const
	{
		return m_indicies.size() * sizeof(m_indicies[0]);
	}
};

//TEMP CODE: Sets vertex buffer back to draw vertex buffer. 
//Call at the end of all mesh based rendering
void rendTEMP_UnbindVertexBuffer();
