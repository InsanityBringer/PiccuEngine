/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
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
#include "3d.h"
#include "pserror.h"
#include "HardwareInternal.h"
#include "renderer.h"
#include <string.h>

void g3_Mat4Multiply(float* res, float* right)
{
	float left[16];
	memcpy(left, res, sizeof(left));

	res[0] = left[0] * right[0] + left[4] * right[1] + left[8] * right[2] + left[12] * right[3]; //i1 j1
	res[1] = left[1] * right[0] + left[5] * right[1] + left[9] * right[2] + left[13] * right[3]; //i2 j1
	res[2] = left[2] * right[0] + left[6] * right[1] + left[10] * right[2] + left[14] * right[3]; //i3 j1
	res[3] = left[3] * right[0] + left[7] * right[1] + left[11] * right[2] + left[15] * right[3]; //14 j1

	res[4] = left[0] * right[4] + left[4] * right[5] + left[8] * right[6] + left[12] * right[7]; //i1 j2
	res[5] = left[1] * right[4] + left[5] * right[5] + left[9] * right[6] + left[13] * right[7]; //i2 j2
	res[6] = left[2] * right[4] + left[6] * right[5] + left[10] * right[6] + left[14] * right[7]; //i3 j2
	res[7] = left[3] * right[4] + left[7] * right[5] + left[11] * right[6] + left[15] * right[7]; //i4 j2

	res[8] = left[0] * right[8] + left[4] * right[9] + left[8] * right[10] + left[12] * right[11]; //i1 j3
	res[9] = left[1] * right[8] + left[5] * right[9] + left[9] * right[10] + left[13] * right[11]; //i2 j3
	res[10] = left[2] * right[8] + left[6] * right[9] + left[10] * right[10] + left[14] * right[11]; //i3 j3
	res[11] = left[3] * right[8] + left[7] * right[9] + left[11] * right[10] + left[15] * right[11]; //i4 j3

	res[12] = left[0] * right[12] + left[4] * right[13] + left[8] * right[14] + left[12] * right[15]; //i1 j4
	res[13] = left[1] * right[12] + left[5] * right[13] + left[9] * right[14] + left[13] * right[15]; //i2 j4
	res[14] = left[2] * right[12] + left[6] * right[13] + left[10] * right[14] + left[14] * right[15]; //i3 j4
	res[15] = left[3] * right[12] + left[7] * right[13] + left[11] * right[14] + left[15] * right[15]; //i4 j4
}

void g3_Mat4Multiply(float* res, float* left, float* right)
{
	res[0] = left[0] * right[0] + left[4] * right[1] + left[8] * right[2] + left[12] * right[3]; //i1 j1
	res[1] = left[1] * right[0] + left[5] * right[1] + left[9] * right[2] + left[13] * right[3]; //i2 j1
	res[2] = left[2] * right[0] + left[6] * right[1] + left[10] * right[2] + left[14] * right[3]; //i3 j1
	res[3] = left[3] * right[0] + left[7] * right[1] + left[11] * right[2] + left[15] * right[3]; //14 j1

	res[4] = left[0] * right[4] + left[4] * right[5] + left[8] * right[6] + left[12] * right[7]; //i1 j2
	res[5] = left[1] * right[4] + left[5] * right[5] + left[9] * right[6] + left[13] * right[7]; //i2 j2
	res[6] = left[2] * right[4] + left[6] * right[5] + left[10] * right[6] + left[14] * right[7]; //i3 j2
	res[7] = left[3] * right[4] + left[7] * right[5] + left[11] * right[6] + left[15] * right[7]; //i4 j2

	res[8] = left[0] * right[8] + left[4] * right[9] + left[8] * right[10] + left[12] * right[11]; //i1 j3
	res[9] = left[1] * right[8] + left[5] * right[9] + left[9] * right[10] + left[13] * right[11]; //i2 j3
	res[10] = left[2] * right[8] + left[6] * right[9] + left[10] * right[10] + left[14] * right[11]; //i3 j3
	res[11] = left[3] * right[8] + left[7] * right[9] + left[11] * right[10] + left[15] * right[11]; //i4 j3

	res[12] = left[0] * right[12] + left[4] * right[13] + left[8] * right[14] + left[12] * right[15]; //i1 j4
	res[13] = left[1] * right[12] + left[5] * right[13] + left[9] * right[14] + left[13] * right[15]; //i2 j4
	res[14] = left[2] * right[12] + left[6] * right[13] + left[10] * right[14] + left[14] * right[15]; //i3 j4
	res[15] = left[3] * right[12] + left[7] * right[13] + left[11] * right[14] + left[15] * right[15]; //i4 j4
}

extern float Z_bias;
void g3_GetModelViewMatrix( const vector *viewPos, const matrix *viewMatrix, float *mvMat )
{
	matrix localOrient = (*viewMatrix);
	vector localPos    = -(*viewPos);
	mvMat[0]  = localOrient.rvec.x;
	mvMat[1]  = localOrient.uvec.x;
	mvMat[2]  = -localOrient.fvec.x;
	mvMat[3]  = 0.0f;
	mvMat[4]  = localOrient.rvec.y;
	mvMat[5]  = localOrient.uvec.y;
	mvMat[6]  = -localOrient.fvec.y;
	mvMat[7]  = 0.0f;
	mvMat[8]  = localOrient.rvec.z;
	mvMat[9]  = localOrient.uvec.z;
	mvMat[10] = -localOrient.fvec.z;
	mvMat[11] = 0.0f;
	mvMat[12] = localPos * localOrient.rvec;
	mvMat[13] = localPos * localOrient.uvec;
	mvMat[14] = localPos * -localOrient.fvec + Z_bias;
	mvMat[15] = 1.0f;
}

void g3_UpdateFullTransform()
{
	g3_Mat4Multiply(gTransformFull, gTransformProjection, gTransformModelView);
}

void g3_GenerateReflect(g3Plane& plane, float* mat)
{
	mat[3] = mat[7] = mat[11] = 0;
	mat[15] = 1;
	mat[0] = -2 * plane.x * plane.x + 1;
	mat[1] = -2 * plane.x * plane.y;
	mat[2] = -2 * plane.x * plane.z;
	mat[12] = -2 * plane.x * plane.d;

	mat[4] = -2 * plane.y * plane.x;
	mat[5] = -2 * plane.y * plane.y + 1;
	mat[6] = -2 * plane.y * plane.z;
	mat[13] = -2 * plane.y * plane.d;

	mat[8] = -2 * plane.z * plane.x;
	mat[9] = -2 * plane.z * plane.y;
	mat[10] = -2 * plane.z * plane.z + 1;
	mat[14] = -2 * plane.z * plane.d;
}

Frustum::Frustum()
{
}

Frustum::Frustum(float* matrix)
	: planes{ 
	g3Plane(matrix[3] - matrix[0], matrix[7] - matrix[4], matrix[11] - matrix[8],  (matrix[15] - matrix[12]), true),
	g3Plane(matrix[3] + matrix[0], matrix[7] + matrix[4], matrix[11] + matrix[8],  (matrix[15] + matrix[12]), true),
	g3Plane(matrix[3] + matrix[1], matrix[7] + matrix[5], matrix[11] + matrix[9],  (matrix[15] + matrix[13]), true),
	g3Plane(matrix[3] - matrix[1], matrix[7] - matrix[5], matrix[11] - matrix[9],  (matrix[15] - matrix[13]), true),
	g3Plane(matrix[3] - matrix[2], matrix[7] - matrix[6], matrix[11] - matrix[10], (matrix[15] - matrix[14]), true),
	g3Plane(matrix[3] + matrix[2], matrix[7] + matrix[6], matrix[11] + matrix[10], (matrix[15] + matrix[14]), true) }
{
}

void Frustum::TestPoint(vector& vec, g3Codes& codes) const
{
	int codebits = 0;
	if (planes[0].Dot(vec) <= 0)
	{
		codebits |= CC_OFF_RIGHT;
	}
	if (planes[1].Dot(vec) <= 0)
	{
		codebits |= CC_OFF_LEFT;
	}
	if (planes[2].Dot(vec) <= 0)
	{
		codebits |= CC_OFF_TOP;
	}
	if (planes[3].Dot(vec) <= 0)
	{
		codebits |= CC_OFF_BOT;
	}
	if (planes[4].Dot(vec) <= 0)
	{
		codebits |= CC_OFF_FAR;
	}
	if (planes[5].Dot(vec) <= 0)
	{
		codebits |= CC_BEHIND;
	}

	codes.cc_or |= codebits;
	codes.cc_and &= codebits;
}
