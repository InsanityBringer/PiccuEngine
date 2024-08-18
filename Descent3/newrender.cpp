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

#include <stdlib.h>
#include <algorithm>
#include "descent.h"
#include "game.h"
#include "newrender.h"
#include "render.h"
#include "3d.h"
#include "renderer.h"
#include "room.h"
#include "../renderer/gl_mesh.h"
#include "pserror.h"
#include "special_face.h"
#include "terrain.h"
#include "lightmap_info.h"
#include "TelComAutoMap.h"
#include "config.h"

//[ISB] Checks if a face is completely static and therefore should be in the normal static meshes.
//Portals need to be put into another pass because they may or may not be visible. 
static inline bool FaceIsStatic(room& rp, face& fp)
{
	//Check for a floating trigger, which doesn't get rendered
	if ((fp.flags & FF_FLOATING_TRIG))
		return false;

	//No portals in the normal interactions. 
	//Portal faces will be put in another list since they need to be determined at runtime. 
	if (fp.portal_num != -1)
		return false;

	//Nothing special, so face renders
	return true;
}

//Determines if a face draws with alpha blending
//Parameters:	fp - pointer to the face in question
//					bm_handle - the handle for the bitmap for this frame, or -1 if don't care about transparence
//Returns:		bitmask describing the alpha blending for the face
//					the return bits are the ATF_ flags in renderer.h
static inline int GetFaceAlpha(face& fp, int bm_handle)
{
	int ret = AT_ALWAYS;
	if (GameTextures[fp.tmap].flags & TF_SATURATE)
	{
		ret = AT_SATURATE_TEXTURE;
	}
	else
	{
		//Check the face's texture for an alpha value
		if (GameTextures[fp.tmap].alpha < 1.0)
			ret |= ATF_CONSTANT;

		//Check for transparency
		if (bm_handle >= 0 && GameBitmaps[bm_handle].format != BITMAP_FORMAT_4444 && GameTextures[fp.tmap].flags & TF_TMAP2)
			ret |= ATF_TEXTURE;
	}
	return ret;
}

//Changes that can happen to a face to warrant a remesh
struct FacePrevState
{
	int flags;
	int tmap;
};

//Future profiling: Given the dynamic nature of rooms, does it make sense to have only one large vertex buffer?
//Or would eating the cost of rebinds be paid for by more efficient generation of room meshes?
VertexBuffer Room_VertexBuffer;
IndexBuffer Room_IndexBuffer;

struct RoomDrawElement
{
	int texturenum;
	int lmhandle;
	ElementRange range;
};

struct PostDrawElement
{
	int facenum;
	int texturenum;
	int lmhandle;
	vector avg;
	ElementRange range;
};

struct SpecularDrawElement
{
	int texturenum;
	int lmhandle;
	ElementRange range;
	special_face* special;
};

struct RoomMesh
{
	int roomnum;
	std::vector<RoomDrawElement> LitInteractions;
	std::vector<RoomDrawElement> UnlitInteractions;
	std::vector<SpecularDrawElement> SpecInteractions;
	std::vector<RoomDrawElement> MirrorInteractions;
	std::vector<PostDrawElement> TransparentInteractions;
	//One of these for each face.
	//If the state of FacePrevStates[facenum] != roomptr->faces[facenum], remesh this part of the world. Sigh.
	std::vector<FacePrevState> FacePrevStates;

	uint32_t FirstVertexOffset;
	uint32_t FirstVertex;
	uint32_t FirstIndexOffset;
	uint32_t FirstIndex;
	void ResetInteractions()
	{
		LitInteractions.clear();
		UnlitInteractions.clear();
		SpecInteractions.clear();
		MirrorInteractions.clear();
		TransparentInteractions.clear();
	}

	void Reset()
	{
		ResetInteractions();
		FacePrevStates.clear();
	}

	void GetPostrenders(RenderList& list)
	{
		g3Plane eyeplane = list.GetEyePlane();
		for (int i = 0; i < TransparentInteractions.size(); i++)
		{
			PostDrawElement& element = TransparentInteractions[i];
			//Check if it's a portal, and if so, if it's actually visible
			int portalnum = Rooms[roomnum].faces[element.facenum].portal_num;
			if (portalnum != -1)
			{
				if (!(Rooms[roomnum].portals[portalnum].flags & PF_RENDER_FACES))
					continue;
			}

			float z = eyeplane.Dot(element.avg);
			list.AddPostrender(NewPostRenderType::Wall, roomnum, i, z);
		}
	}

	bool PostrenderLit(int num)
	{
		if (Rooms[roomnum].faces[TransparentInteractions[num].facenum].flags & FF_LIGHTMAP)
			return true;

		return false;
	}

	void DrawPostrender(int num)
	{
		PostDrawElement& element = TransparentInteractions[num];

		//Bind bitmaps. Temp API, should the bitmap system also handle binding? Or does that go elsewhere?
		Room_VertexBuffer.BindBitmap(GetTextureBitmap(element.texturenum, 0));
		Room_VertexBuffer.BindLightmap(element.lmhandle);

		//And draw
		Room_VertexBuffer.DrawIndexed(PrimitiveType::Triangles, element.range);
	}

	void DrawLit()
	{
		for (RoomDrawElement& element : LitInteractions)
		{
			//Bind bitmaps. Temp API, should the bitmap system also handle binding? Or does that go elsewhere?
			Room_VertexBuffer.BindBitmap(GetTextureBitmap(element.texturenum, 0));
			Room_VertexBuffer.BindLightmap(element.lmhandle);

			//And draw
			Room_VertexBuffer.DrawIndexed(PrimitiveType::Triangles, element.range);
		}
	}

	void DrawUnlit()
	{
		for (RoomDrawElement& element : UnlitInteractions)
		{
			//Bind bitmaps. Temp API, should the bitmap system also handle binding? Or does that go elsewhere?
			Room_VertexBuffer.BindBitmap(GetTextureBitmap(element.texturenum, 0));

			//And draw
			Room_VertexBuffer.DrawIndexed(PrimitiveType::Triangles, element.range);
		}
	}

	void DrawMirrorFaces()
	{
		if (MirrorInteractions.size() == 0)
			return;

		assert(Rooms[roomnum].mirror_face != -1);
		Room_VertexBuffer.BindBitmap(GetTextureBitmap(Rooms[roomnum].faces[Rooms[roomnum].mirror_face].tmap, 0));
		for (RoomDrawElement& element : MirrorInteractions)
		{
			Room_VertexBuffer.BindLightmap(element.lmhandle);

			//And draw
			Room_VertexBuffer.DrawIndexed(PrimitiveType::Triangles, element.range);
		}
	}

	void DrawSpecular()
	{
		int last_texture = -1;
		int last_lightmap = -1;
		static SpecularBlock specblock;
		if (Rooms[roomnum].flags & RF_EXTERNAL)
		{
			for (SpecularDrawElement& element : SpecInteractions)
			{
				//External rooms only can have speculars to one satellite in the sky, and it's always white. 
				//But it emits colored light? heh. 
				specblock.num_speculars = 1;
				specblock.speculars[0].bright_center[0] = Terrain_sky.satellite_vectors[0].x;
				specblock.speculars[0].bright_center[1] = Terrain_sky.satellite_vectors[0].y;
				specblock.speculars[0].bright_center[2] = Terrain_sky.satellite_vectors[0].z;
				specblock.speculars[0].bright_center[3] = 1;
				specblock.speculars[0].color[2] =
					specblock.speculars[0].color[1] =
					specblock.speculars[0].color[0] = 1.0f;

				//Bind bitmaps. Temp API, should the bitmap system also handle binding? Or does that go elsewhere?
				if (element.texturenum != last_texture)
				{
					last_texture = element.texturenum;
					Room_VertexBuffer.BindBitmap(GetTextureBitmap(element.texturenum, 0));
					if (GameTextures[element.texturenum].flags & TF_SMOOTH_SPECULAR)
						specblock.strength = 1;
					else
						specblock.strength = 4;

					if (GameTextures[element.texturenum].flags & TF_PLASTIC)
						specblock.exponent = 14;
					else if (GameTextures[element.texturenum].flags & TF_MARBLE)
						specblock.exponent = 4;
					else
						specblock.exponent = 6;
				}

				if (element.lmhandle != last_lightmap)
				{
					last_lightmap = element.lmhandle;
					Room_VertexBuffer.BindLightmap(element.lmhandle);
				}

				rend_UpdateSpecular(&specblock);

				//And draw
				Room_VertexBuffer.DrawIndexed(PrimitiveType::Triangles, element.range);
			}
		}
		else
		{
			for (SpecularDrawElement& element : SpecInteractions)
			{
				//Bind bitmaps. Temp API, should the bitmap system also handle binding? Or does that go elsewhere?
				if (element.texturenum != last_texture)
				{
					last_texture = element.texturenum;
					Room_VertexBuffer.BindBitmap(GetTextureBitmap(element.texturenum, 0));
					if (GameTextures[element.texturenum].flags & TF_SMOOTH_SPECULAR)
						specblock.strength = 1;
					else
						specblock.strength = 4;

					if (GameTextures[element.texturenum].flags & TF_PLASTIC)
						specblock.exponent = 14;
					else if (GameTextures[element.texturenum].flags & TF_MARBLE)
						specblock.exponent = 4;
					else
						specblock.exponent = 6;
				}

				if (element.lmhandle != last_lightmap)
				{
					last_lightmap = element.lmhandle;
					Room_VertexBuffer.BindLightmap(element.lmhandle);
				}

				specblock.num_speculars = element.special->num;
				for (int i = 0; i < specblock.num_speculars; i++) //aaaaaaa
				{
					specblock.speculars[i].bright_center[0] = element.special->spec_instance[i].bright_center.x;
					specblock.speculars[i].bright_center[1] = element.special->spec_instance[i].bright_center.y;
					specblock.speculars[i].bright_center[2] = element.special->spec_instance[i].bright_center.z;
					specblock.speculars[i].bright_center[3] = 1;
					specblock.speculars[i].color[2] = (element.special->spec_instance[i].bright_color & 31) / 31.f;
					specblock.speculars[i].color[1] = ((element.special->spec_instance[i].bright_color >> 5) & 31) / 31.f;
					specblock.speculars[i].color[0] = ((element.special->spec_instance[i].bright_color >> 10) & 31) / 31.f;
				}

				rend_UpdateSpecular(&specblock);

				//And draw
				Room_VertexBuffer.DrawIndexed(PrimitiveType::Triangles, element.range);
			}
		}
	}
};


//[ISB] temp: Should stick this in Room or a separate render-related struct later down the line (dynamic room limit at some point?)
//These are the meshes of all normal room geometry. 
RoomMesh Room_meshes[MAX_ROOMS];

void AddFacesToBuffer(MeshBuilder& mesh, std::vector<SortableElement>& elements, std::vector<RoomDrawElement>& interactions, room& rp, int indexOffset, int firstIndex)
{
	if (elements.empty())
		return;

	int lasttmap = -1;
	int lastlm = -1;
	bool firsttime = true;
	int triindices[3];
	RendVertex vert;
	float alpha = 1;
	for (SortableElement& element : elements)
	{
		if (element.texturehandle != lasttmap || element.lmhandle != lastlm)
		{
			if (!firsttime)
			{
				mesh.EndVertices();
				RoomDrawElement element;
				element.texturenum = lasttmap;
				element.lmhandle = lastlm;
				element.range = mesh.EndIndices();
				element.range.offset += firstIndex;
				interactions.push_back(element);
			}
			else
				firsttime = false;

			mesh.BeginVertices();
			mesh.BeginIndices();
			lasttmap = element.texturehandle;
			lastlm = element.lmhandle;

			vert.uslide = GameTextures[lasttmap].slide_u;
			vert.vslide = GameTextures[lasttmap].slide_v;

			alpha = GameTextures[element.texturehandle].alpha;
		}

		face& fp = rp.faces[element.element];

		int first_index = mesh.NumVertices() + indexOffset;
		for (int i = 0; i < fp.num_verts; i++)
		{
			roomUVL uvs = fp.face_uvls[i];
			vert.position = rp.verts[fp.face_verts[i]];
			vert.normal = fp.normal; //oh no, no support for phong shading..
			vert.r = vert.g = vert.b = 255;
			vert.a = (ubyte)(std::min(1.f, std::max(0.f, alpha)) * 255);
			vert.u1 = uvs.u; vert.v1 = uvs.v;
			vert.u2 = uvs.u2; vert.v2 = uvs.v2;

			mesh.AddVertex(vert);
		}

		//Generate indicies as a triangle fan
		for (int i = 2; i < fp.num_verts; i++)
		{
			triindices[2] = first_index;
			triindices[1] = first_index + i - 1;
			triindices[0] = first_index + i;
			mesh.SetIndicies(3, triindices);
		}
	}

	mesh.EndVertices();
	RoomDrawElement element;
	element.texturenum = lasttmap;
	element.lmhandle = lastlm;
	element.range = mesh.EndIndices();
	element.range.offset += firstIndex;
	interactions.push_back(element);
}

void AddPostFacesToBuffer(MeshBuilder& mesh, std::vector<SortableElement>& elements, std::vector<PostDrawElement>& interactions, room& rp, int indexOffset, int firstIndex)
{
	if (elements.empty())
		return;

	int lasttmap = -1;
	int lastlm = -1;
	int lastfacenum = -1;
	bool firsttime = true;
	int triindices[3];
	RendVertex vert;
	float alpha = 1;

	vector avg = {};
	for (SortableElement& element : elements)
	{
		mesh.BeginVertices();
		mesh.BeginIndices();
		lasttmap = element.texturehandle;
		lastlm = element.lmhandle;
		lastfacenum = element.element;

		vert.uslide = GameTextures[lasttmap].slide_u;
		vert.vslide = GameTextures[lasttmap].slide_v;

		alpha = GameTextures[element.texturehandle].alpha;
		avg.x = avg.y = avg.z = 0;

		face& fp = rp.faces[element.element];

		int first_index = mesh.NumVertices() + indexOffset;
		for (int i = 0; i < fp.num_verts; i++)
		{
			roomUVL uvs = fp.face_uvls[i];
			vert.position = rp.verts[fp.face_verts[i]];
			vert.normal = fp.normal; //oh no, no support for phong shading..
			vert.r = vert.g = vert.b = 255;
			vert.a = (ubyte)(std::min(1.f, std::max(0.f, alpha)) * 255);
			vert.u1 = uvs.u; vert.v1 = uvs.v;
			vert.u2 = uvs.u2; vert.v2 = uvs.v2;

			avg += vert.position;

			mesh.AddVertex(vert);
		}

		//Generate indicies as a triangle fan
		for (int i = 2; i < fp.num_verts; i++)
		{
			triindices[2] = first_index;
			triindices[1] = first_index + i - 1;
			triindices[0] = first_index + i;
			mesh.SetIndicies(3, triindices);
		}

		mesh.EndVertices();
		PostDrawElement element;
		element.facenum = lastfacenum;
		element.texturenum = lasttmap;
		element.lmhandle = lastlm;
		element.range = mesh.EndIndices();
		element.range.offset += firstIndex;
		element.avg = avg / (float)rp.faces[lastfacenum].num_verts;
		interactions.push_back(element);
	}
}

void AddSpecFacesToBuffer(MeshBuilder& mesh, std::vector<SortableElement>& elements, std::vector<SpecularDrawElement>& interactions, room& rp, int indexOffset, int firstIndex)
{
	if (elements.empty())
		return;

	int lasttmap = -1;
	int lastlm = -1;
	bool firsttime = true;
	int triindices[3];
	RendVertex vert;
	for (SortableElement& element : elements)
	{
		mesh.BeginVertices();
		mesh.BeginIndices();
		lasttmap = element.texturehandle;
		lastlm = element.lmhandle;

		vert.uslide = GameTextures[lasttmap].slide_u;
		vert.vslide = GameTextures[lasttmap].slide_v;

		face& fp = rp.faces[element.element];

		int first_index = mesh.NumVertices() + indexOffset;
		if (GameTextures[element.texturehandle].flags & TF_SMOOTH_SPECULAR && fp.special_handle != BAD_SPECIAL_FACE_INDEX)
		{
			for (int i = 0; i < fp.num_verts; i++)
			{
				roomUVL uvs = fp.face_uvls[i];
				vert.position = rp.verts[fp.face_verts[i]];
				vert.normal = SpecialFaces[fp.special_handle].vertnorms[i];
				vert.r = vert.g = vert.b = vert.a = 255;
				vert.u1 = uvs.u; vert.v1 = uvs.v;
				vert.u2 = uvs.u2; vert.v2 = uvs.v2;

				mesh.AddVertex(vert);
			}
		}
		else
		{
			for (int i = 0; i < fp.num_verts; i++)
			{
				roomUVL uvs = fp.face_uvls[i];
				vert.position = rp.verts[fp.face_verts[i]];
				vert.normal = fp.normal;
				vert.r = vert.g = vert.b = vert.a = 255;
				vert.u1 = uvs.u; vert.v1 = uvs.v;
				vert.u2 = uvs.u2; vert.v2 = uvs.v2;

				mesh.AddVertex(vert);
			}
		}

		//Generate indicies as a triangle fan
		for (int i = 2; i < fp.num_verts; i++)
		{
			triindices[2] = first_index;
			triindices[1] = first_index + i - 1;
			triindices[0] = first_index + i;
			mesh.SetIndicies(3, triindices);
		}

		mesh.EndVertices();
		SpecularDrawElement element;
		element.texturenum = lasttmap;
		element.lmhandle = lastlm;
		element.range = mesh.EndIndices();
		element.range.offset += firstIndex;
		element.special = &SpecialFaces[fp.special_handle];
		interactions.push_back(element);
	}
}

//Meshes a given room. 
//Index offset is added to all generated indicies, to allow updating a room at a specific place
//later down the line, even with an empty MeshBuilder. 
//First index is added to all interactions to indicate where the first index to draw is. 
void UpdateRoomMesh(MeshBuilder& mesh, int roomnum, int indexOffset, int firstIndex)
{
	room& rp = Rooms[roomnum];
	if (!rp.used)
		return; //unused room

	//Maybe these should be changed into one pass using a white texture for unlit?
	//But what happens if something silly like HDR lighting is added later?
	std::vector<SortableElement> faces_lit;
	std::vector<SortableElement> faces_unlit;
	std::vector<SortableElement> faces_spec;
	std::vector<SortableElement> faces_mirror;
	std::vector<SortableElement> faces_trans;

	RoomMesh& roommesh = Room_meshes[roomnum];
	if (roommesh.FacePrevStates.size() != rp.num_faces)
		roommesh.FacePrevStates.resize(rp.num_faces);

	roommesh.roomnum = roomnum;

	roommesh.ResetInteractions();

	//Mirrors are defined as "the mirror face and every other face that happens to share the same texture"
	uint32_t mirror_tex_hack = UINT32_MAX;
	if (rp.mirror_face != -1)
	{
		mirror_tex_hack = rp.faces[rp.mirror_face].tmap;
	}

	//Build a sortable list of all faces
	for (int i = 0; i < rp.num_faces; i++)
	{
		face& fp = rp.faces[i];
		roommesh.FacePrevStates[i].flags = fp.flags;
		roommesh.FacePrevStates[i].tmap = fp.tmap;

		//blarg
		int bm_handle = GetTextureBitmap(fp.tmap, 0);
		int alphatype = GetFaceAlpha(fp, -1);

		int tmap = fp.tmap;
		if (fp.flags & FF_DESTROYED && GameTextures[tmap].flags & TF_DESTROYABLE)
			tmap = GameTextures[tmap].destroy_handle;

		if (fp.portal_num != -1)
		{
			faces_trans.push_back(SortableElement{ i, (ushort)tmap, LightmapInfo[fp.lmi_handle].lm_handle });
		}
		else if ((alphatype & (ATF_CONSTANT | ATF_TEXTURE)) != 0)
		{
			faces_trans.push_back(SortableElement{ i, (ushort)tmap, LightmapInfo[fp.lmi_handle].lm_handle });
		}
		else
		{
			//Not a postrender, determine if it is unlit or lit. 
			if (rp.mirror_face != -1 && tmap == mirror_tex_hack)
			{
				faces_mirror.push_back(SortableElement{ i, (ushort)tmap, LightmapInfo[fp.lmi_handle].lm_handle });
			}
			else if (fp.flags & FF_LIGHTMAP)
			{
				//If the face is specular, add it for a post stage. 
				//Specs have to be in a special pass like this so that the size of the room vertex buffer never changes
				//External specular faces don't use a special face, and therefore can never be smooth. Heh. 
				if (GameTextures[tmap].flags & TF_SPECULAR && (fp.special_handle != BAD_SPECIAL_FACE_INDEX || (rp.flags & RF_EXTERNAL)))
				{
					faces_spec.push_back(SortableElement{ i, (ushort)tmap, LightmapInfo[fp.lmi_handle].lm_handle });
				}
				else
				{
					//TODO: Add field names when Piccu becomes C++20.
					faces_lit.push_back(SortableElement{ i, (ushort)tmap, LightmapInfo[fp.lmi_handle].lm_handle });
				}
			}
			else
			{
				faces_unlit.push_back(SortableElement{ i, (ushort)tmap, 0 });
			}
		}
	}

	std::sort(faces_lit.begin(), faces_lit.end());
	AddFacesToBuffer(mesh, faces_lit, Room_meshes[roomnum].LitInteractions, rp, indexOffset, firstIndex);

	std::sort(faces_unlit.begin(), faces_unlit.end());
	AddFacesToBuffer(mesh, faces_unlit, Room_meshes[roomnum].UnlitInteractions, rp, indexOffset, firstIndex);

	std::sort(faces_mirror.begin(), faces_mirror.end());
	AddFacesToBuffer(mesh, faces_mirror, Room_meshes[roomnum].MirrorInteractions, rp, indexOffset, firstIndex);

	//Even though they're not batched up (may be fixable if I can quickly determine if they have identical light sources), 
	//sort specular faces to try to minimize texture state thrashing. Even though that's trivial compared to the buffer state thrashing. 
	std::sort(faces_spec.begin(), faces_spec.end());
	AddSpecFacesToBuffer(mesh, faces_spec, Room_meshes[roomnum].SpecInteractions, rp, indexOffset, firstIndex);

	std::sort(faces_trans.begin(), faces_trans.end());
	AddPostFacesToBuffer(mesh, faces_trans, Room_meshes[roomnum].TransparentInteractions, rp, indexOffset, firstIndex);
}

void FreeRoomMeshes()
{
	for (int i = 0; i < MAX_ROOMS; i++)
	{
		Room_meshes[i].Reset();
	}
	Room_VertexBuffer.Destroy();
	Room_IndexBuffer.Destroy();
}

uint32_t lightmap_room_handle = 0xFFFFFFFFu;
uint32_t lightmap_specular_handle = 0xFFFFFFFFu;
uint32_t lightmap_room_fog_handle = 0xFFFFFFFFu;
uint32_t lightmap_room_specular_fog_handle = 0xFFFFFFFFu;
uint32_t unlit_room_handle = 0xFFFFFFFFu;
uint32_t unlit_room_fog_handle = 0xFFFFFFFFu;

//Called during LoadLevel, builds meshes for every room. 
void MeshRooms()
{
	if (lightmap_specular_handle == 0xFFFFFFFFu)
	{
		lightmap_specular_handle = rend_GetPipelineByName("lightmapped_specular");
		assert(lightmap_specular_handle != 0xFFFFFFFFu);
	}
	if (lightmap_room_fog_handle == 0xFFFFFFFFu)
	{
		lightmap_room_fog_handle = rend_GetPipelineByName("lightmap_room_fog");
		assert(lightmap_room_fog_handle != 0xFFFFFFFFu);
	}
	if (lightmap_room_handle == 0xFFFFFFFFu)
	{
		lightmap_room_handle = rend_GetPipelineByName("lightmap_room");
		assert(lightmap_room_handle != 0xFFFFFFFFu);
	}
	if (lightmap_room_specular_fog_handle == 0xFFFFFFFFu)
	{
		lightmap_room_specular_fog_handle = rend_GetPipelineByName("lightmap_room_specular_fog");
		assert(lightmap_room_specular_fog_handle != 0xFFFFFFFFu);
	}
	if (unlit_room_handle == 0xFFFFFFFFu)
	{
		unlit_room_handle = rend_GetPipelineByName("unlit_room");
		assert(unlit_room_handle != 0xFFFFFFFFu);
	}
	if (unlit_room_fog_handle == 0xFFFFFFFFu)
	{
		unlit_room_fog_handle = rend_GetPipelineByName("unlit_room_fog");
		assert(unlit_room_fog_handle != 0xFFFFFFFFu);
	}
	MeshBuilder mesh;
	FreeRoomMeshes();
	for (int i = 0; i <= Highest_room_index; i++)
	{
		//These can be set here and should remain static, since the amount of vertices and indices should remain static across any room changes
		Room_meshes[i].FirstVertexOffset = mesh.VertexOffset();
		Room_meshes[i].FirstVertex = mesh.NumVertices();
		Room_meshes[i].FirstIndex = mesh.NumIndices();
		Room_meshes[i].FirstIndexOffset = mesh.IndexOffset();

		UpdateRoomMesh(mesh, i, 0, 0);
	}

	mesh.BuildVertices(Room_VertexBuffer);
	mesh.BuildIndicies(Room_IndexBuffer);
}

//Returns true if the room at roomnum needs to have its static mesh regenerated. 
//Because nothing can truly be static when the original rendering code was fully dynamic.
static bool RoomNeedRemesh(int roomnum)
{
	room& rp = Rooms[roomnum];
	RoomMesh& mesh = Room_meshes[roomnum];
	for (int i = 0; i < rp.num_faces; i++)
	{
		if ((rp.faces[i].flags & FF_DESTROYED) != (mesh.FacePrevStates[i].flags & FF_DESTROYED))
			return true;

		if (rp.faces[i].tmap != mesh.FacePrevStates[i].tmap)
			return true;
	}
	return false;
}

static void RemeshRoom(MeshBuilder& mesh, int roomnum)
{
	mprintf((0, "RemeshRoom: Updating room %d\n", roomnum));
	mesh.Destroy();
	UpdateRoomMesh(mesh, roomnum, Room_meshes[roomnum].FirstVertex, Room_meshes[roomnum].FirstIndex);

	mesh.UpdateVertices(Room_VertexBuffer, Room_meshes[roomnum].FirstVertexOffset);
	mesh.UpdateIndicies(Room_IndexBuffer, Room_meshes[roomnum].FirstIndexOffset);
}

struct NewRenderPassInfo
{
	//Pointer to the shader handle that will be used for this pass.
	uint32_t& handle;
	//True if only fog rooms should be rendered.
	bool fog;
	//True if only specular faces should be rendered.
	bool specular;
} renderpass_info[] =
{
	{lightmap_room_handle, false, false},
	{lightmap_room_handle, false, false},
	{lightmap_specular_handle, false, true},
	{lightmap_room_fog_handle, true, false},
	{lightmap_room_fog_handle, true, false},
	{lightmap_room_specular_fog_handle, true, true},
};

//I'm begging you please switch to a newer spec so you can use std::array with deduction guides. 
//Actually would that work with a composite type here? Actually would this just be another use for a C#-like array class?
#define NUM_NEWRENDERPASSES sizeof(renderpass_info) / sizeof(renderpass_info[0])

void ComputeRoomPulseLight(room* rp);

//Primary RenderList for the main view
RenderList gRenderList;
void NewRender_Render(vector& vieweye, matrix& vieworientation, int roomnum)
{
	gRenderList.GatherVisible(vieweye, vieworientation, roomnum);
	gRenderList.Draw();
}

void NewRender_InitNewLevel()
{
	MeshRooms();
	MeshTerrain();
}

static bool NewRenderPastPortal(room& rp, portal& pp)
{
	//If we don't render the portal's faces, then we see through it
	if (!(pp.flags & PF_RENDER_FACES))
		return true;

	//Check if the face's texture has transparency
	face& fp = rp.faces[pp.portal_face];
	if (GameTextures[fp.tmap].flags & TF_PROCEDURAL)
		return true;
	int bm_handle = GetTextureBitmap(fp.tmap, 0);
	if (GetFaceAlpha(fp, bm_handle))
		return true;	  //Face has alpha or transparency, so we can see through it

	return false;		//Not transparent, so no render past
}

void RenderList::SetupLegacyFog(room& rp)
{
	if ((rp.flags & RF_FOG) == 0)
		return;

	if (!Detail_settings.Fog_enabled)
	{
		// fog is disabled
		Room_fog_plane_check = -1;
		return;
	}

	if (EyeRoomnum == (&rp - Rooms))
	{
		// viewer is in the room
		Room_fog_plane_check = 1;
		Room_fog_distance = -vm_DotProduct(&EyeOrient.fvec, &EyePos);
		Room_fog_plane = EyeOrient.fvec;
		return;
	}

	// find the 'fogroom' number (we should have put it in here if we will render the room)
	int found_room = -1;
	for (int i = 0; i < FogPortals.size() && found_room == -1; i++)
	{
		if (FogPortals[i].roomnum == &rp - Rooms)
		{
			found_room = i;
			break;
		}
	}

	if (found_room == -1 || FogPortals[found_room].close_face == NULL)
	{
		// we won't be rendering this room
		Room_fog_plane_check = -1;
		return;
	}

	// Use the closest face
	face* close_face = FogPortals[found_room].close_face;
	Room_fog_plane_check = 0;
	Room_fog_plane = close_face->normal;
	Room_fog_portal_vert = rp.verts[close_face->face_verts[0]];
	Room_fog_distance = -vm_DotProduct(&Room_fog_plane, &Room_fog_portal_vert);
	Room_fog_eye_distance = (EyePos * Room_fog_plane) + Room_fog_distance;
}

bool RenderList::CheckFace(room& rp, face& fp, Frustum& frustum) const
{
	/*g3Codes cc = {};
	cc.cc_and = 0xFF;
	for (int i = 0; i < fp.num_verts; i++)
	{
		vector& pt = rp.verts[fp.face_verts[i]];
		frustum.TestPoint(pt, cc);
	}

	//All off screen?
	if (cc.cc_and != 0)
		return false;

	return true;*/

	//Just a behind check now
	int numbehind = 0;
	g3Plane cameraplane(EyeOrient.fvec, EyePos);

	for (int i = 0; i < fp.num_verts; i++)
	{
		vector& pt = rp.verts[fp.face_verts[i]];
		if (cameraplane.Dot(pt) < 0)
			numbehind++;
	}

	return numbehind < fp.num_verts;
}

NewRenderWindow RenderList::GetWindowForFace(room& rp, face& fp, NewRenderWindow& parent) const
{
	NewRenderWindow window(INT_MAX, INT_MAX, 0, 0);

	g3Point point = {};
	for (int i = 0; i < fp.num_verts; i++)
	{
		int codes = g3_RotatePoint(&point, &rp.verts[fp.face_verts[i]]);
		//Shouldn't need to check andcodes, since a behind test was done earlier.
		if (codes & CC_BEHIND)
			return parent;

		g3_ProjectPoint(&point);
		window.Encompass(point.p3_sx, point.p3_sy);
	}

	return window;
}

void RenderList::MaybeUpdateFogPortal(int roomnum, int portalnum)
{
	room& rp = Rooms[roomnum];
	face& fp = rp.faces[rp.portals[portalnum].portal_face];
	auto it = FogPortals.begin();
	FogPortalData* fogdata = nullptr;
	while (it != FogPortals.end())
	{
		FogPortalData& data = *it;
		if (data.roomnum == roomnum)
		{
			fogdata = &data;
			break;
		}
		it++;
	}

	if (fogdata == nullptr)
	{
		//Couldn't find this room, so add it
		FogPortals.push_back({ roomnum, FLT_MAX });
		fogdata = &FogPortals[FogPortals.size() - 1];
	}

	float distance = -vm_DotProduct(&fp.normal, &rp.verts[fp.face_verts[0]]);
	distance = vm_DotProduct(&fp.normal, &EyePos) + distance;
	if (distance < fogdata->close_dist)
	{
		fogdata->close_dist = distance;
		fogdata->close_face = &fp;
	}
}

void RenderList::AddRoom(RenderListEntry& entry, Frustum& frustum)
{
	room& rp = Rooms[entry.roomnum];

	//Iterate all the portals to see if they're visible
	for (int portalnum = 0; portalnum < rp.num_portals; portalnum++)
	{
		portal& pp = rp.portals[portalnum];
		face& fp = rp.faces[pp.portal_face];
		int croomnum = pp.croom;
		room& crp = Rooms[croomnum];
		
		//Can you actually see through this portal?
		if (!NewRenderPastPortal(rp, pp))
			continue;

		//Can you see this portal face in the first place?
		if (!CheckFace(rp, fp, frustum))
			continue; 

		NewRenderWindow portalwindow = GetWindowForFace(rp, fp, entry.window);
		if (!portalwindow.Clip(entry.window)) //Window off screen?
			continue;

		//Before the check if crp is already iterated, check if it is a fog room, and if it is, check if this portal is closer
		if (crp.flags & RF_FOG)
			MaybeUpdateFogPortal(croomnum, pp.cportal);

		//Don't iterate into a room if it's already been added to the visible room list, but do expand its portal window if needed.
		if (RoomChecked[croomnum] != -1)
		{
			VisibleRooms[RoomChecked[croomnum]].window.Encompass(portalwindow);
			continue;
		}

		//Add this room to the future check list
		RoomChecked[croomnum] = VisibleRooms.size();
		VisibleRooms.emplace_back(croomnum, portalwindow);
		PushRoom(croomnum, portalwindow);

		//Is the room being checked external? If so, oops, gotta render that terrain
		if (crp.flags & RF_EXTERNAL)
		{
			HasFoundTerrain = true;
		}
	}
}

void RenderList::PreDraw()
{
	RoomBlock roomblocks[100];
	int renderrooms = std::min(100, (int)VisibleRooms.size());

	for (int nn = 0; nn < renderrooms; nn++)
	{
		int roomnum = VisibleRooms[nn].roomnum;
		room& rp = Rooms[roomnum];
		RoomBlock& roomblock = roomblocks[nn];

		// Mark it visible for automap
		AutomapVisMap[&rp - Rooms] = 1;

		ComputeRoomPulseLight(&Rooms[roomnum]);
		roomblock.brightness = Room_light_val;

		if (rp.flags & RF_FOG)
		{
			SetupLegacyFog(rp);

			roomblock.fog_distance = rp.fog_depth;
			roomblock.fog_color[0] = rp.fog_r;
			roomblock.fog_color[1] = rp.fog_g;
			roomblock.fog_color[2] = rp.fog_b;

			if (Room_fog_plane_check == 0)
			{
				roomblock.not_in_room = true;
				roomblock.fog_plane[0] = Room_fog_plane.x;
				roomblock.fog_plane[1] = Room_fog_plane.y;
				roomblock.fog_plane[2] = Room_fog_plane.z;
				roomblock.fog_plane[3] = Room_fog_distance;
			}
			else
			{
				roomblock.not_in_room = false;
			}
		}

		rp.last_render_time = Gametime;
		rp.flags &= ~RF_MIRROR_VISIBLE;

		for (int facenum = 0; facenum < rp.num_faces; facenum++)
		{
			face& fp = rp.faces[facenum];
			if (!(fp.flags & FF_NOT_FACING))
			{
				fp.renderframe = FrameCount & 0xFF;
			}
		}

		Room_meshes[roomnum].GetPostrenders(*this);
	}

	rend_UpdateFogBrightness(roomblocks, renderrooms);
}

void RenderList::DrawWorld(int passnum)
{
	assert(passnum >= 0 && passnum < NUM_NEWRENDERPASSES);
	NewRenderPassInfo& passinfo = renderpass_info[passnum];
	static RoomBlock roomblock;

	rend_BindPipeline(passinfo.handle);

	//TODO: The max count should be obtained from the renderer, since some platforms don't have a limit of 100 UBOs. 
	int renderrooms = std::min(100, (int)VisibleRooms.size());

	for (int nn = 0; nn < renderrooms; nn++) //forward first to try to draw nearest rooms first. 
	{
		int roomnum = VisibleRooms[nn].roomnum;
		room& rp = Rooms[roomnum];
		ComputeRoomPulseLight(&Rooms[roomnum]);

		if (passinfo.fog)
		{
			if (Detail_settings.Fog_enabled && !(Rooms[roomnum].flags & RF_FOG))
				continue;
		}
		else
		{
			if (Detail_settings.Fog_enabled && Rooms[roomnum].flags & RF_FOG)
				continue;
		}

		rend_SetCurrentRoomNum(nn);

		if (passinfo.specular)
			Room_meshes[roomnum].DrawSpecular();
		else
		{
			Room_meshes[roomnum].DrawLit();
			Room_meshes[roomnum].DrawMirrorFaces();
		}

		//TEMP mirror test
		/*if (!passinfo.specular)
		{
			if (rp.mirror_face != -1)
			{
				g3Plane plane(rp.faces[rp.mirror_face].normal, rp.verts[rp.faces[rp.mirror_face].face_verts[0]]);
				float reflectmat[16];
				g3_GenerateReflect(plane, reflectmat);
				g3_StartInstanceMatrix4(reflectmat);

				Room_meshes[roomnum].DrawLit();

				g3_DoneInstance();
			}
		}*/
	}
}

void RenderList::DrawPostrenders()
{
	//let's try to avoid juggling state as much as possible.. it's worth a shot
	int lastroomnum = -1;
	uint32_t lastshaderhandle = UINT32_MAX;

	rend_SetAlphaType(AT_CONSTANT);

	for (NewPostRender& postrender : PostRenders)
	{
		if (postrender.roomnum != lastroomnum)
		{
			rend_SetCurrentRoomNum(RoomChecked[postrender.roomnum]);
			lastroomnum = postrender.roomnum;
		}

		if (postrender.type == NewPostRenderType::Wall)
		{
			uint32_t shaderhandle = lightmap_room_handle;
			bool lit = Room_meshes[lastroomnum].PostrenderLit(postrender.elementnum);
			if (Rooms[lastroomnum].flags & RF_FOG)
			{
				if (lit)
					shaderhandle = lightmap_room_fog_handle;
				else
					shaderhandle = unlit_room_fog_handle;
			}
			else
			{
				if (lit)
					shaderhandle = lightmap_room_handle;
				else
					shaderhandle = unlit_room_handle;
			}

			if (shaderhandle != lastshaderhandle)
			{
				lastshaderhandle = shaderhandle;
				rend_BindPipeline(shaderhandle);
			}

			Room_meshes[lastroomnum].DrawPostrender(postrender.elementnum);
		}
	}
}

RenderList::RenderList() 
	: EyePos{},
	EyeOrient{}
{
	HasFoundTerrain = false;
	EyeRoomnum = 0;

	//Reserve space in the vectors to their original limits, to establish a reasonable initial allocation
	VisibleRooms.reserve(100);
	FogPortals.reserve(8);
	PostRenders.reserve(3000);
}

void RenderList::GatherVisible(vector& eye_pos, matrix& eye_orient, int viewroomnum)
{
	//Initialize the room checked list
	RoomChecked.clear();
	RoomChecked.resize(Highest_room_index + 1, -1);
	VisibleRooms.clear();
	FogPortals.clear();
	PostRenders.clear();

	HasFoundTerrain = false;

	EyePos = eye_pos;
	EyeOrient = eye_orient;
	EyeRoomnum = viewroomnum;

	EyePlane = g3Plane(eye_orient.fvec, eye_pos);

	Frustum viewFrustum(gTransformFull);

	NewRenderWindow initialWindow(0, 0, Game_window_w - 1, Game_window_h - 1);

	if (viewroomnum >= 0) //is a room?
	{
		RoomChecked[viewroomnum] = 0;
		VisibleRooms.emplace_back(viewroomnum, initialWindow);
		AddRoom(RenderListEntry(viewroomnum, initialWindow), viewFrustum);
	}
	else
	{
		HasFoundTerrain = true;
		//add external rooms here
	}

	while (PendingRooms())
	{
		RenderListEntry entry = PopRoom();
		AddRoom(entry, viewFrustum);
	}
}

void RenderList::Draw()
{
	rend_SetColorModel(CM_MONO);
	rend_SetLighting(LS_GOURAUD);
	rend_SetWrapType(WT_WRAP);
	rend_SetAlphaType(AT_ALWAYS);
	rend_SetCullFace(true);

	//Walk the room render list for updates
	for (int nn = 0; nn < VisibleRooms.size(); nn++)
	{
		MeshBuilder mesh;
		int roomnum = VisibleRooms[nn].roomnum;
		if (RoomNeedRemesh(roomnum))
		{
			RemeshRoom(mesh, roomnum);
		}
	}

	Room_VertexBuffer.Bind();
	Room_IndexBuffer.Bind();

	PreDraw();
	DrawWorld(0);
	DrawWorld(2);
	DrawWorld(3);
	DrawWorld(5);

	std::sort(PostRenders.begin(), PostRenders.end());
	DrawPostrenders();

	rendTEMP_UnbindVertexBuffer();

	rend_SetCullFace(false);
}

void RenderList::AddPostrender(NewPostRenderType type, int roomnum, int elementnum, float z)
{
	PostRenders.push_back(NewPostRender{ type, roomnum, elementnum, z });
}
