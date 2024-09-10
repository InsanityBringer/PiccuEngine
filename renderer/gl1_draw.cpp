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

void GLCompatibilityRenderer::SetDrawDefaults()
{
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#ifdef _DEBUG
	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

	glVertexPointer(3, GL_FLOAT, 0, GL_verts);
	glColorPointer(4, GL_FLOAT, 0, GL_colors);
	glTexCoordPointer(4, GL_FLOAT, 0, GL_tex_coords);

#ifdef _DEBUG
	err = glGetError();
	if (err != GL_NO_ERROR)
		Int3();
#endif

	if (UseMultitexture)
	{
		glClientActiveTextureARB(GL_TEXTURE0_ARB + 1);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(4, GL_FLOAT, 0, GL_tex_coords2);
		glClientActiveTextureARB(GL_TEXTURE0_ARB + 0);

#ifdef _DEBUG
		err = glGetError();
		if (err != GL_NO_ERROR)
			Int3();
#endif
	}
}

// Takes nv vertices and draws the 3D polygon defined by those vertices.
// Uses bitmap "handle" as a texture
void GLCompatibilityRenderer::DrawPolygon3D(int handle, g3Point** p, int nv, int map_type)
{
	g3Point* pnt;
	int i;
	float fr, fg, fb;
	float alpha;
	vector* vertp;
	compat_color_array* colorp;
	compat_tex_array* texp;

	ASSERT(nv < 100);

	if (OpenGL_state.cur_texture_quality == 0)
	{
		DrawFlatPolygon3D(p, nv);
		return;
	}

	if (Overlay_type != OT_NONE && UseMultitexture)
	{
		DrawMultitexturePolygon3D(handle, p, nv, map_type);
		return;
	}

	int x_add = OpenGL_state.clip_x1;
	int y_add = OpenGL_state.clip_y1;

	if (OpenGL_state.cur_light_state == LS_FLAT_GOURAUD)
	{
		fr = GR_COLOR_RED(OpenGL_state.cur_color) / 255.0;
		fg = GR_COLOR_GREEN(OpenGL_state.cur_color) / 255.0;
		fb = GR_COLOR_BLUE(OpenGL_state.cur_color) / 255.0;
	}

	if (UseMultitexture)
	{
		SetMultitextureBlendMode(false);
	}

	// make sure our bitmap is ready to be drawn
	MakeBitmapCurrent(handle, map_type, 0);
	MakeWrapTypeCurrent(handle, map_type, 0);
	MakeFilterTypeCurrent(handle, map_type, 0);

	alpha = Alpha_multiplier * OpenGL_Alpha_factor;

	vertp = &GL_verts[0];
	texp = &GL_tex_coords[0];
	colorp = &GL_colors[0];

	// Specify our coordinates
	for (i = 0; i < nv; i++, vertp++, texp++, colorp++)
	{
		pnt = p[i];

		if (OpenGL_state.cur_alpha_type & ATF_VERTEX)
		{
			alpha = pnt->p3_a * Alpha_multiplier * OpenGL_Alpha_factor;
		}

		// If we have a lighting model, apply the correct lighting!
		if (OpenGL_state.cur_light_state != LS_NONE)
		{
			if (OpenGL_state.cur_light_state == LS_FLAT_GOURAUD)
			{
				colorp->r = fr;
				colorp->g = fg;
				colorp->b = fb;
				colorp->a = alpha;
			}
			else
			{
				// Do lighting based on intesity (MONO) or colored (RGB)
				if (OpenGL_state.cur_color_model == CM_MONO)
				{
					colorp->r = pnt->p3_l;
					colorp->g = pnt->p3_l;
					colorp->b = pnt->p3_l;
					colorp->a = alpha;
				}
				else
				{
					colorp->r = pnt->p3_r;
					colorp->g = pnt->p3_g;
					colorp->b = pnt->p3_b;
					colorp->a = alpha;
				}
			}
		}
		else
		{
			colorp->r = 1;
			colorp->g = 1;
			colorp->b = 1;
			colorp->a = alpha;
		}

		// Texture this polygon!
		float texw = 1.0 / (pnt->p3_z + Z_bias);
		texp->s = pnt->p3_u * texw;
		texp->t = pnt->p3_v * texw;
		texp->r = 0;
		texp->w = texw;

		// Finally, specify a vertex
		vertp->x = pnt->p3_sx + x_add;
		vertp->y = pnt->p3_sy + y_add;

		float z = std::max(0.f, std::min(1.0f, 1.0f - (1.0f / (pnt->p3_z + Z_bias))));
		vertp->z = -z;
	}

	// And draw!
	glDrawArrays(GL_POLYGON, 0, nv);
	OpenGL_polys_drawn++;
	OpenGL_verts_processed += nv;

	CHECK_ERROR(10)

		// If there is a lightmap to draw, draw it as well
		if (Overlay_type != OT_NONE)
		{
			return;	// Temp fix until I figure out whats going on
			Int3();	// Shouldn't reach here
		}
}

// Takes nv vertices and draws the 2D polygon defined by those vertices.
// Uses bitmap "handle" as a texture
void GLCompatibilityRenderer::DrawPolygon2D(int handle, g3Point** p, int nv)
{
	ASSERT(nv < 100);
	ASSERT(Overlay_type == OT_NONE);

	if (UseMultitexture)
	{
		SetMultitextureBlendMode(false);
	}

	int xAdd = OpenGL_state.clip_x1;
	int yAdd = OpenGL_state.clip_y1;

	float fr, fg, fb;
	if (OpenGL_state.cur_light_state == LS_FLAT_GOURAUD || OpenGL_state.cur_texture_quality == 0)
	{
		float scale = 1.0f / 255.0f;
		fr = GR_COLOR_RED(OpenGL_state.cur_color) * scale;
		fg = GR_COLOR_GREEN(OpenGL_state.cur_color) * scale;
		fb = GR_COLOR_BLUE(OpenGL_state.cur_color) * scale;
	}

	// make sure our bitmap is ready to be drawn
	MakeBitmapCurrent(handle, MAP_TYPE_BITMAP, 0);
	MakeWrapTypeCurrent(handle, MAP_TYPE_BITMAP, 0);
	MakeFilterTypeCurrent(handle, MAP_TYPE_BITMAP, 0);

	float alpha = Alpha_multiplier * OpenGL_Alpha_factor;

	vector* vertp = &GL_verts[0];
	compat_tex_array* texp = &GL_tex_coords[0];
	compat_color_array* colorp = &GL_colors[0];

	// Specify our coordinates
	int i;
	for (i = 0; i < nv; ++i, ++vertp, ++texp, ++colorp)
	{
		g3Point* pnt = p[i];

		if (OpenGL_state.cur_alpha_type & ATF_VERTEX)
		{
			// the alpha should come from the vertex
			alpha = pnt->p3_a * Alpha_multiplier * OpenGL_Alpha_factor;
		}

		// If we have a lighting model, apply the correct lighting!
		if (OpenGL_state.cur_light_state == LS_FLAT_GOURAUD || OpenGL_state.cur_texture_quality == 0)
		{
			// pull the color from the constant color data
			colorp->r = fr;
			colorp->g = fg;
			colorp->b = fb;
			colorp->a = alpha;
		}
		else if (OpenGL_state.cur_light_state != LS_NONE)
		{
			// Do lighting based on intensity (MONO) or colored (RGB)
			if (OpenGL_state.cur_color_model == CM_MONO)
			{
				colorp->r = pnt->p3_l;
				colorp->g = pnt->p3_l;
				colorp->b = pnt->p3_l;
				colorp->a = alpha;
			}
			else
			{
				colorp->r = pnt->p3_r;
				colorp->g = pnt->p3_g;
				colorp->b = pnt->p3_b;
				colorp->a = alpha;
			}
		}
		else
		{
			// force white
			colorp->r = 1.0f;
			colorp->g = 1.0f;
			colorp->b = 1.0f;
			colorp->a = alpha;
		}

		texp->s = pnt->p3_u;
		texp->t = pnt->p3_v;
		texp->r = 0.0f;
		texp->w = 1.0f;

		// Finally, specify a vertex
		vertp->x = pnt->p3_sx + xAdd;
		vertp->y = pnt->p3_sy + yAdd;
		vertp->z = 0.0f;
	}

	// And draw!
	if (OpenGL_state.cur_texture_quality == 0)
	{
		// force disable textures
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	// draw the data in the arrays
	glDrawArrays(GL_POLYGON, 0, nv);

	if (OpenGL_state.cur_texture_quality == 0)
	{
		// re-enable textures
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	OpenGL_polys_drawn++;
	OpenGL_verts_processed += nv;

	CHECK_ERROR(10)
}

// draws a scaled 2d bitmap to our buffer
void GLCompatibilityRenderer::DrawScaledBitmap(int x1, int y1, int x2, int y2,
	int bm, float u0, float v0, float u1, float v1, int color, float* alphas)
{
	g3Point* ptr_pnts[4];
	g3Point pnts[4];
	float r, g, b;
	if (color != -1)
	{
		r = GR_COLOR_RED(color) / 255.0;
		g = GR_COLOR_GREEN(color) / 255.0;
		b = GR_COLOR_BLUE(color) / 255.0;
	}
	for (int i = 0; i < 4; i++)
	{
		if (color == -1)
			pnts[i].p3_l = 1.0;
		else
		{
			pnts[i].p3_r = r;
			pnts[i].p3_g = g;
			pnts[i].p3_b = b;
		}
		if (alphas)
		{
			pnts[i].p3_a = alphas[i];
		}

		pnts[i].p3_z = 1.0f;
		pnts[i].p3_flags = PF_PROJECTED;
	}

	pnts[0].p3_sx = x1;
	pnts[0].p3_sy = y1;
	pnts[0].p3_u = u0;
	pnts[0].p3_v = v0;
	pnts[1].p3_sx = x2;
	pnts[1].p3_sy = y1;
	pnts[1].p3_u = u1;
	pnts[1].p3_v = v0;
	pnts[2].p3_sx = x2;
	pnts[2].p3_sy = y2;
	pnts[2].p3_u = u1;
	pnts[2].p3_v = v1;
	pnts[3].p3_sx = x1;
	pnts[3].p3_sy = y2;
	pnts[3].p3_u = u0;
	pnts[3].p3_v = v1;
	ptr_pnts[0] = &pnts[0];
	ptr_pnts[1] = &pnts[1];
	ptr_pnts[2] = &pnts[2];
	ptr_pnts[3] = &pnts[3];
	SetTextureType(TT_LINEAR);
	DrawPolygon2D(bm, ptr_pnts, 4);
}

void GLCompatibilityRenderer::DrawScaledBitmapWithZ(int x1, int y1, int x2, int y2,
	int bm, float u0, float v0, float u1, float v1, float zval, int color, float* alphas)
{
	g3Point* ptr_pnts[4];
	g3Point pnts[4];
	float r, g, b;

	if (color != -1)
	{
		r = GR_COLOR_RED(color) / 255.0;
		g = GR_COLOR_GREEN(color) / 255.0;
		b = GR_COLOR_BLUE(color) / 255.0;
	}

	for (int i = 0; i < 4; i++)
	{
		if (color == -1)
			pnts[i].p3_l = 1.0;
		else
		{
			pnts[i].p3_r = r;
			pnts[i].p3_g = g;
			pnts[i].p3_b = b;
		}

		if (alphas)
		{
			pnts[i].p3_a = alphas[i];
		}

		pnts[i].p3_z = zval;
		pnts[i].p3_flags = PF_PROJECTED;
	}



	pnts[0].p3_sx = x1;
	pnts[0].p3_sy = y1;
	pnts[0].p3_u = u0;
	pnts[0].p3_v = v0;

	pnts[1].p3_sx = x2;
	pnts[1].p3_sy = y1;
	pnts[1].p3_u = u1;
	pnts[1].p3_v = v0;

	pnts[2].p3_sx = x2;
	pnts[2].p3_sy = y2;
	pnts[2].p3_u = u1;
	pnts[2].p3_v = v1;

	pnts[3].p3_sx = x1;
	pnts[3].p3_sy = y2;
	pnts[3].p3_u = u0;
	pnts[3].p3_v = v1;

	ptr_pnts[0] = &pnts[0];
	ptr_pnts[1] = &pnts[1];
	ptr_pnts[2] = &pnts[2];
	ptr_pnts[3] = &pnts[3];

	SetTextureType(TT_LINEAR);
	DrawPolygon3D(bm, ptr_pnts, 4);
}

// Fills a rectangle on the display
void GLCompatibilityRenderer::FillRect(ddgr_color color, int x1, int y1, int x2, int y2)
{
	int r = GR_COLOR_RED(color);
	int g = GR_COLOR_GREEN(color);
	int b = GR_COLOR_BLUE(color);

	int width = x2 - x1;
	int height = y2 - y1;

	x1 += OpenGL_state.clip_x1;
	y1 += OpenGL_state.clip_y1;

	glEnable(GL_SCISSOR_TEST);
	glScissor(x1, OpenGL_state.screen_height - (height + y1), width, height);
	glClearColor((float)r / 255.0, (float)g / 255.0, (float)b / 255.0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	width = OpenGL_state.clip_x2 - OpenGL_state.clip_x1;
	height = OpenGL_state.clip_y2 - OpenGL_state.clip_y1;

	glScissor(OpenGL_state.clip_x1, OpenGL_state.screen_height - (OpenGL_state.clip_y1 + height), width, height);
	glDisable(GL_SCISSOR_TEST);
}

// Sets a pixel on the display
void GLCompatibilityRenderer::SetPixel(ddgr_color color, int x, int y)
{
	int r = (color >> 16 & 0xFF);
	int g = (color >> 8 & 0xFF);
	int b = (color & 0xFF);

	glColor3ub(r, g, b);

	glBegin(GL_POINTS);
	glVertex2i(x, y);
	glEnd();
}

// Sets a pixel on the display
ddgr_color GLCompatibilityRenderer::GetPixel(int x, int y)
{
	ddgr_color color[4];
	glReadPixels(x, (OpenGL_state.screen_height - 1) - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)color);
	return color[0];
}

void GLCompatibilityRenderer::FillCircle(ddgr_color col, int x, int y, int rad)
{
}

void GLCompatibilityRenderer::DrawCircle(int x, int y, int rad)
{
}

// Sets up a font character to draw.  We draw our fonts as pieces of textures
void GLCompatibilityRenderer::DrawFontCharacter(int bm_handle, int x1, int y1, int x2, int y2, float u, float v, float w, float h)
{
	g3Point* ptr_pnts[4];
	g3Point pnts[4];
	for (int i = 0; i < 4; i++)
	{
		pnts[i].p3_z = 1;	// Make REALLY close!
		pnts[i].p3_flags = PF_PROJECTED;
		ptr_pnts[i] = &pnts[i];
	}
	pnts[0].p3_sx = x1;
	pnts[0].p3_sy = y1;
	pnts[0].p3_u = u;
	pnts[0].p3_v = v;
	pnts[1].p3_sx = x2;
	pnts[1].p3_sy = y1;
	pnts[1].p3_u = u + w;
	pnts[1].p3_v = v;
	pnts[2].p3_sx = x2;
	pnts[2].p3_sy = y2;
	pnts[2].p3_u = u + w;
	pnts[2].p3_v = v + h;
	pnts[3].p3_sx = x1;
	pnts[3].p3_sy = y2;
	pnts[3].p3_u = u;
	pnts[3].p3_v = v + h;
	rend_DrawPolygon2D(bm_handle, ptr_pnts, 4);
}

// Draws a line
void GLCompatibilityRenderer::DrawLine(int x1, int y1, int x2, int y2)
{
	sbyte atype;
	light_state ltype;
	texture_type ttype;
	int color = OpenGL_state.cur_color;

	int r = GR_COLOR_RED(color);
	int g = GR_COLOR_GREEN(color);
	int b = GR_COLOR_BLUE(color);

	atype = OpenGL_state.cur_alpha_type;
	ltype = OpenGL_state.cur_light_state;
	ttype = OpenGL_state.cur_texture_type;

	SetAlphaType(AT_ALWAYS);
	SetLighting(LS_NONE);
	SetTextureType(TT_FLAT);


	glBegin(GL_LINES);
	glColor4ub(r, g, b, 255);
	glVertex2i(x1 + OpenGL_state.clip_x1, y1 + OpenGL_state.clip_y1);
	glColor4ub(r, g, b, 255);
	glVertex2i(x2 + OpenGL_state.clip_x1, y2 + OpenGL_state.clip_y1);
	glEnd();

	SetAlphaType(atype);
	SetLighting(ltype);
	SetTextureType(ttype);
}

// Sets the argb characteristics of the font characters.  color1 is the upper left and proceeds clockwise
void GLCompatibilityRenderer::SetCharacterParameters(ddgr_color color1, ddgr_color color2, ddgr_color color3, ddgr_color color4)
{
	rend_FontRed[0] = (float)(GR_COLOR_RED(color1) / 255.0f);
	rend_FontRed[1] = (float)(GR_COLOR_RED(color2) / 255.0f);
	rend_FontRed[2] = (float)(GR_COLOR_RED(color3) / 255.0f);
	rend_FontRed[3] = (float)(GR_COLOR_RED(color4) / 255.0f);
	rend_FontGreen[0] = (float)(GR_COLOR_GREEN(color1) / 255.0f);
	rend_FontGreen[1] = (float)(GR_COLOR_GREEN(color2) / 255.0f);
	rend_FontGreen[2] = (float)(GR_COLOR_GREEN(color3) / 255.0f);
	rend_FontGreen[3] = (float)(GR_COLOR_GREEN(color4) / 255.0f);
	rend_FontBlue[0] = (float)(GR_COLOR_BLUE(color1) / 255.0f);
	rend_FontBlue[1] = (float)(GR_COLOR_BLUE(color2) / 255.0f);
	rend_FontBlue[2] = (float)(GR_COLOR_BLUE(color3) / 255.0f);
	rend_FontBlue[3] = (float)(GR_COLOR_BLUE(color4) / 255.0f);
	rend_FontAlpha[0] = (color1 >> 24) / 255.0f;
	rend_FontAlpha[1] = (color2 >> 24) / 255.0f;
	rend_FontAlpha[2] = (color3 >> 24) / 255.0f;
	rend_FontAlpha[3] = (color4 >> 24) / 255.0f;
}

// Turns on/off multitexture blending
void GLCompatibilityRenderer::SetMultitextureBlendMode(bool state)
{
	if (OpenGL_multitexture_state == state)
		return;
	OpenGL_multitexture_state = state;
	if (state)
	{
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glEnable(GL_TEXTURE_2D);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		Last_texel_unit_set = 0;

	}
	else
	{
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glClientActiveTextureARB(GL_TEXTURE1_ARB);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);

		glActiveTextureARB(GL_TEXTURE0_ARB);
		glClientActiveTextureARB(GL_TEXTURE0_ARB);
		Last_texel_unit_set = 0;
	}
}

// Takes nv vertices and draws the polygon defined by those vertices.  Uses bitmap "handle"
// as a texture
void GLCompatibilityRenderer::DrawMultitexturePolygon3D(int handle, g3Point** p, int nv, int map_type)
{
	g3Point* pnt;
	int i, fr, fg, fb;
	float alpha;
	vector* vertp;
	compat_color_array* colorp;
	compat_tex_array* texp, * texp2;

	float one_over_square_res = 1.0 / GameLightmaps[Overlay_map].square_res;
	float xscalar = (float)GameLightmaps[Overlay_map].width * one_over_square_res;
	float yscalar = (float)GameLightmaps[Overlay_map].height * one_over_square_res;

	ASSERT(nv < 100);

	int x_add = OpenGL_state.clip_x1;
	int y_add = OpenGL_state.clip_y1;

	if (OpenGL_state.cur_light_state == LS_NONE)
	{
		fr = GR_COLOR_RED(OpenGL_state.cur_color);
		fg = GR_COLOR_GREEN(OpenGL_state.cur_color);
		fb = GR_COLOR_BLUE(OpenGL_state.cur_color);
	}

	alpha = Alpha_multiplier * OpenGL_Alpha_factor;

	vertp = &GL_verts[0];
	texp = &GL_tex_coords[0];
	texp2 = &GL_tex_coords2[0];
	colorp = &GL_colors[0];

	// Specify our coordinates
	for (i = 0; i < nv; i++, vertp++, texp++, colorp++, texp2++)
	{
		pnt = p[i];

		if (OpenGL_state.cur_alpha_type & ATF_VERTEX)
			alpha = pnt->p3_a * Alpha_multiplier * OpenGL_Alpha_factor;

		// If we have a lighting model, apply the correct lighting!
		if (OpenGL_state.cur_light_state != LS_NONE)
		{
			// Do lighting based on intesity (MONO) or colored (RGB)
			if (OpenGL_state.cur_color_model == CM_MONO)
			{
				colorp->r = pnt->p3_l;
				colorp->g = pnt->p3_l;
				colorp->b = pnt->p3_l;
				colorp->a = alpha;
			}
			else
			{
				colorp->r = pnt->p3_r;
				colorp->g = pnt->p3_g;
				colorp->b = pnt->p3_b;
				colorp->a = alpha;
			}
		}
		else
		{
			colorp->r = 1;
			colorp->g = 1;
			colorp->b = 1;
			colorp->a = alpha;
		}


		// Texture this polygon!
		float texw = 1.0 / (pnt->p3_z + Z_bias);
		texp->s = pnt->p3_u * texw;
		texp->t = pnt->p3_v * texw;
		texp->r = 0;
		texp->w = texw;

		texp2->s = pnt->p3_u2 * xscalar * texw;
		texp2->t = pnt->p3_v2 * yscalar * texw;
		texp2->r = 0;
		texp2->w = texw;

		// Finally, specify a vertex

		vertp->x = pnt->p3_sx + x_add;
		vertp->y = pnt->p3_sy + y_add;
		float z = std::max(0.f, std::min(1.0f, 1.0f - (1.0f / (pnt->p3_z + Z_bias))));
		vertp->z = -z;
	}

	// make sure our bitmap is ready to be drawn
	MakeBitmapCurrent(handle, map_type, 0);
	MakeWrapTypeCurrent(handle, map_type, 0);
	MakeFilterTypeCurrent(handle, map_type, 0);

	// make sure our bitmap is ready to be drawn
	MakeBitmapCurrent(Overlay_map, MAP_TYPE_LIGHTMAP, 1);
	MakeWrapTypeCurrent(Overlay_map, MAP_TYPE_LIGHTMAP, 1);
	MakeFilterTypeCurrent(Overlay_map, MAP_TYPE_LIGHTMAP, 1);

	SetMultitextureBlendMode(true);

	// And draw!
	glDrawArrays(GL_POLYGON, 0, nv);
	OpenGL_polys_drawn++;
	OpenGL_verts_processed += nv;

	CHECK_ERROR(10)
}

void GLCompatibilityRenderer::DrawFlatPolygon3D(g3Point** p, int nv)
{
	float fr, fg, fb;
	int i;

	if (UseMultitexture)
	{
		SetMultitextureBlendMode(false);
	}

	float alpha = Alpha_multiplier * OpenGL_Alpha_factor;

	fr = GR_COLOR_RED(OpenGL_state.cur_color);
	fg = GR_COLOR_GREEN(OpenGL_state.cur_color);
	fb = GR_COLOR_BLUE(OpenGL_state.cur_color);
	fr /= 255.0;
	fg /= 255.0;
	fb /= 255.0;

	int x_add = OpenGL_state.clip_x1;
	int y_add = OpenGL_state.clip_y1;

	// And draw!
	glBegin(GL_POLYGON);
	for (i = 0; i < nv; i++)
	{
		g3Point* pnt = p[i];

		if (OpenGL_state.cur_alpha_type & ATF_VERTEX)
			alpha = pnt->p3_a * Alpha_multiplier * OpenGL_Alpha_factor;

		// If we have a lighting model, apply the correct lighting!
		if (OpenGL_state.cur_light_state != LS_NONE)
		{
			// Do lighting based on intesity (MONO) or colored (RGB)
			if (OpenGL_state.cur_color_model == CM_MONO)
				glColor4f(pnt->p3_l, pnt->p3_l, pnt->p3_l, alpha);
			else
			{
				glColor4f(pnt->p3_r, pnt->p3_g, pnt->p3_b, alpha);
			}

		}
		else
		{
			glColor4f(fr, fg, fb, alpha);
		}


		// Finally, specify a vertex
		float z = std::max(0.f, std::min(1.0f, 1.0f - (1.0f / (pnt->p3_z + Z_bias))));
		glVertex3f(pnt->p3_sx + x_add, pnt->p3_sy + y_add, -z);
	}

	glEnd();
	CHECK_ERROR(11)
		OpenGL_polys_drawn++;
	OpenGL_verts_processed += nv;
}

// Draws a line using the states of the renderer
void GLCompatibilityRenderer::DrawSpecialLine(g3Point* p0, g3Point* p1)
{
	int x_add = OpenGL_state.clip_x1;
	int y_add = OpenGL_state.clip_y1;
	float fr, fg, fb, alpha;
	int i;

	fr = GR_COLOR_RED(OpenGL_state.cur_color);
	fg = GR_COLOR_GREEN(OpenGL_state.cur_color);
	fb = GR_COLOR_BLUE(OpenGL_state.cur_color);

	fr /= 255.0f;
	fg /= 255.0f;
	fb /= 255.0f;

	alpha = Alpha_multiplier * OpenGL_Alpha_factor;

	// And draw!
	glBegin(GL_LINES);
	for (i = 0; i < 2; i++)
	{
		g3Point* pnt = p0;

		if (i == 1)
			pnt = p1;

		if (OpenGL_state.cur_alpha_type & ATF_VERTEX)
			alpha = pnt->p3_a * Alpha_multiplier * OpenGL_Alpha_factor;

		// If we have a lighting model, apply the correct lighting!
		if (OpenGL_state.cur_light_state != LS_NONE)
		{
			if (OpenGL_state.cur_light_state == LS_FLAT_GOURAUD)
			{
				glColor4f(fr, fg, fb, alpha);
			}
			else
			{
				// Do lighting based on intesity (MONO) or colored (RGB)
				if (OpenGL_state.cur_color_model == CM_MONO)
					glColor4f(pnt->p3_l, pnt->p3_l, pnt->p3_l, alpha);
				else
				{
					glColor4f(pnt->p3_r, pnt->p3_g, pnt->p3_b, alpha);
				}
			}
		}
		else
		{
			glColor4f(fr, fg, fb, alpha);
		}

		// Finally, specify a vertex
		float z = std::max(0., std::min(1.0, 1.0 - (1.0 / (pnt->p3_z + Z_bias))));
		glVertex3f(pnt->p3_sx + x_add, pnt->p3_sy + y_add, -z);
	}

	glEnd();
}

//	given a chunked bitmap, renders it.
void GLCompatibilityRenderer::DrawChunkedBitmap(chunked_bitmap* chunk, int x, int y, ubyte alpha)
{
	int* bm_array = chunk->bm_array;
	int w = chunk->w;
	int h = chunk->h;
	int piece_w = bm_w(bm_array[0], 0);
	int piece_h = bm_h(bm_array[0], 0);
	int screen_w, screen_h;
	int i, t;
	SetZBufferState(0);
	GetProjectionParameters(&screen_w, &screen_h);
	for (i = 0; i < h; i++)
	{
		for (t = 0; t < w; t++)
		{
			int dx = x + (piece_w * t);
			int dy = y + (piece_h * i);
			int dw, dh;
			if ((dx + piece_w) > screen_w)
				dw = piece_w - ((dx + piece_w) - screen_w);
			else
				dw = piece_w;
			if ((dy + piece_h) > screen_h)
				dh = piece_h - ((dy + piece_h) - screen_h);
			else
				dh = piece_h;

			float u2 = (float)dw / (float)piece_w;
			float v2 = (float)dh / (float)piece_h;
			DrawSimpleBitmap(bm_array[i * w + t], dx, dy);
		}
	}
	SetZBufferState(1);
}

//	given a chunked bitmap, renders it.scaled
void GLCompatibilityRenderer::DrawScaledChunkedBitmap(chunked_bitmap* chunk, int x, int y, int neww, int newh, ubyte alpha)
{
	int* bm_array = chunk->bm_array;
	int w = chunk->w;
	int h = chunk->h;
	int piece_w;
	int piece_h;
	int screen_w, screen_h;
	int i, t;

	float scalew, scaleh;

	scalew = ((float)neww) / ((float)chunk->pw);
	scaleh = ((float)newh) / ((float)chunk->ph);
	piece_w = scalew * ((float)bm_w(bm_array[0], 0));
	piece_h = scaleh * ((float)bm_h(bm_array[0], 0));
	GetProjectionParameters(&screen_w, &screen_h);
	SetOverlayType(OT_NONE);
	SetLighting(LS_NONE);
	SetColorModel(CM_MONO);
	SetZBufferState(0);
	SetAlphaType(AT_CONSTANT_TEXTURE);
	SetAlphaValue(alpha);
	SetWrapType(WT_WRAP);
	for (i = 0; i < h; i++)
	{
		for (t = 0; t < w; t++)
		{
			int dx = x + (piece_w * t);
			int dy = y + (piece_h * i);
			int dw, dh;
			if ((dx + piece_w) > screen_w)
				dw = piece_w - ((dx + piece_w) - screen_w);
			else
				dw = piece_w;
			if ((dy + piece_h) > screen_h)
				dh = piece_h - ((dy + piece_h) - screen_h);
			else
				dh = piece_h;

			float u2 = (float)dw / (float)piece_w;
			float v2 = (float)dh / (float)piece_h;
			DrawScaledBitmap(dx, dy, dx + dw, dy + dh, bm_array[i * w + t], 0, 0, u2, v2);

		}
	}
	SetZBufferState(1);
}

// Draws a simple bitmap at the specified x,y location
void GLCompatibilityRenderer::DrawSimpleBitmap(int bm_handle, int x, int y)
{
	SetAlphaType(AT_CONSTANT_TEXTURE);
	SetAlphaValue(255);
	SetLighting(LS_NONE);
	SetColorModel(CM_MONO);
	SetOverlayType(OT_NONE);
	SetFiltering(0);
	DrawScaledBitmap(x, y, x + bm_w(bm_handle, 0), y + bm_h(bm_handle, 0), bm_handle, 0, 0, 1, 1);
	SetFiltering(1);
}
