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

#ifndef _3D_H
#define _3D_H

#include "pstypes.h"
#include "vecmat.h"		//the vector/matrix library
#include "grdefs.h"
#include "float.h"

extern int g3d_interp_outline;		//if on, polygon models outlined in white

extern vector Matrix_scale;		//how the matrix is currently scaled

//[ISB] Plane structure used for frustum culling.
struct g3Plane
{
	float x, y, z, d;
	g3Plane()
	{
		x = y = z = d = 0;
	}
	g3Plane(float nx, float ny, float nz, float nd, bool normalize = false) : x(nx), y(ny), z(nz), d(nd)
	{
		if (normalize)
		{
			float mag = sqrt(x * x + y * y + z * z);
			x /= mag;
			y /= mag;
			z /= mag;
			d /= mag;
		}
	}

	g3Plane(const vector& normal, const vector& pt)
	{
		x = normal.x;
		y = normal.y;
		z = normal.z;
		d = -vm_DotProduct(&normal, &pt);
	}

	g3Plane Normalize() const
	{
		float mag = sqrt(x * x + y * y + z * z);
		return g3Plane(x / mag, y / mag, z / mag, d / mag);
	}

	float Dot(vector& vec) const
	{
		return vec.x * x + vec.y * y + vec.z * z + d;
	}
};

//Structure for storing u,v,light values.  This structure doesn't have a
//prefix because it was defined somewhere else before it was moved here
struct g3UVL
{
	//texture coordinates
	float u,v;
	float u2,v2;
	
	union
	{
		float l;	//intensity lighting
		float r;
	};
	float g,b,a;	//rgba lighting
};

// Structure to store clipping codes in a word
struct g3Codes
{
	ubyte cc_or,cc_and;
};

//A frustum. Used for culling.
//Can code points and probably spheres and boxes for a given viewport. 
class Frustum
{
	g3Plane planes[6];
public:
	Frustum();
	//Constructs a frustum from the specified combined projection/modelview matrix
	Frustum(float* matrix);

	void TestPoint(vector& vec, g3Codes& codes) const;
};

//flags for point structure
#define PF_PROJECTED 	1	//has been projected, so sx,sy valid
#define PF_FAR_ALPHA	2	//past fog zone
#define PF_TEMP_POINT	4	//created during clip
#define PF_UV			8	//has uv values set
#define PF_L			16	//has lighting values set
#define PF_RGBA			32	//has RGBA lighting values set
#define PF_UV2			64	//has lightmap uvs as well
#define PF_ORIGPOINT	128

//clipping codes flags
#define CC_OFF_LEFT		1
#define CC_OFF_RIGHT	2
#define CC_OFF_BOT		4
#define CC_OFF_TOP		8
#define CC_OFF_FAR		16
#define CC_OFF_CUSTOM	32
#define CC_BEHIND		128

//Used to store rotated points for mines.  Has frame count to indicate
//if rotated, and flag to indicate if projected.
struct g3Point
{
	float		p3_sx,p3_sy;		//screen x&y
	ubyte		p3_codes;			//clipping codes
	ubyte		p3_flags;			//projected?
	short		p3_pad;				//keep structure longword aligned
	vector		p3_vec;				//x,y,z of rotated point
	vector		p3_vecPreRot;		//original XYZ of the point
	g3UVL		p3_uvl;				//uv & lighting values
};

//macros to reference x,y,z elements of a 3d point
#define p3_x p3_vec.x
#define p3_y p3_vec.y
#define p3_z p3_vec.z

//macros to reference individual elements of the uvls struct
#define p3_u p3_uvl.u
#define p3_v p3_uvl.v
#define p3_l p3_uvl.l
#define p3_r p3_uvl.r
#define p3_g p3_uvl.g
#define p3_b p3_uvl.b
#define p3_a p3_uvl.a
#define p3_u2 p3_uvl.u2
#define p3_v2 p3_uvl.v2

//Functions in library

//3d system startup and shutdown:

//initialize the 3d system
void g3_Init(void);

//close down the 3d system
void g3_Close(void);


//Frame setup functions:

//start the frame, specifying view position, matrix, & zoom
void g3_StartFrame(vector *view_pos,matrix *view_matrix,float zoom);

//end the frame
void g3_EndFrame(void);

//get the current view position
void g3_GetViewPosition(vector *vp);

//	returns the current view matrix
void g3_GetViewMatrix(matrix *mat);

//	returns the current unscaled view matrix
void g3_GetUnscaledMatrix(matrix *mat);


//Instancing

//instance at specified point with specified orientation
void g3_StartInstanceMatrix(vector *pos,matrix *orient);

//instance by transforming the current modelview with a 4x4 matrix.
//Caveat: This will NOT work for legacy!
void g3_StartInstanceMatrix4(float* mat);

//instance at specified point with specified orientation
void g3_StartInstanceAngles(vector *pos,angvec *angles);

//pops the old context
void g3_DoneInstance();

//Misc utility functions:

//get current field of view.  Fills in angle for x & y
void g3_GetFOV(float *fov_x,float *fov_y);

//get zoom.  For a given window size, return the zoom which will achieve
//the given FOV along the given axis.
float g3_GetZoom(char axis,float fov,short window_width,short window_height);

//returns the normalized, unscaled view vectors
void g3_GetViewVectors(vector *forward,vector *up,vector *right);

//returns true if a plane is facing the viewer. takes the unrotated surface 
//normal of the plane, and a point on it.  The normal need not be normalized
bool g3_CheckNormalFacing(vector *v,vector *norm);

//Point definition and rotation functions:

//returns codes_and & codes_or of a list of points numbers
g3Codes g3_CheckCodes(int nv,g3Point **pointlist);

//rotates a point. returns codes.  does not check if already rotated
ubyte g3_RotatePoint(g3Point *dest,vector *src);

//projects a point
void g3_ProjectPoint(g3Point *point);

//calculate the depth of a point - returns the z coord of the rotated point
float g3_CalcPointDepth(vector *pnt);

//from a 2d point, compute the vector through that point
void g3_Point2Vec(vector *v,short sx,short sy);

//code a point.  fills in the p3_codes field of the point, and returns the codes
ubyte g3_CodePoint(g3Point *point);

//delta rotation functions
vector *g3_RotateDeltaX(vector *dest,float dx);
vector *g3_RotateDeltaY(vector *dest,float dy);
vector *g3_RotateDeltaZ(vector *dest,float dz);
vector *g3_RotateDeltaVec(vector *dest,vector *src);
ubyte g3_AddDeltaVec(g3Point *dest,g3Point *src,vector *deltav);

//Drawing functions:
//draw a polygon
//Parameters:	nv - the number of verts in the poly
//					pointlist - a pointer to a list of pointers to points
//					bm - the bitmap handle if texturing.  ignored if flat shading
int g3_DrawPoly(int nv,g3Point **pointlist,int bm,int map_type=0,g3Codes *clip_codes=NULL);

//draw a sortof sphere - i.e., the 2d radius is proportional to the 3d
//radius, but not to the distance from the eye
void g3_DrawSphere(ddgr_color color,g3Point *pnt,float rad);

//like g3_DrawPoly(), but checks to see if facing.  If surface normal is
//NULL, this routine must compute it, which will be slow.  It is better to 
//pre-compute the normal, and pass it to this function.  When the normal
//is passed, this function works like g3_CheckNormalFacing() plus
//g3_DrawPoly().
void g3_CheckAndDrawPoly(int nv,g3Point **pointlist,int bm,vector *norm,vector *pnt);

//draws a line. takes two points.
void g3_DrawLine(ddgr_color color,g3Point *p0,g3Point *p1);

//draws a bitmap with the specified 3d width & height 
//returns 1 if off screen, 0 if drew
void g3_DrawBitmap(vector *pos,float width,float height,int bm,int color=-1);

// Draws a bitmap that has been rotated about its center.  Angle of rotation is passed as 'rot_angle'
void g3_DrawRotatedBitmap (vector *pos,angle rot_angle,float width,float height,int bm,int color=-1);

//Draw a wireframe box aligned with the screen.  Used for the editor.
//Parameters:	color - the color to draw the lines
//					pnt - the center point
//					rad - specifies the width/2 & height/2 of the box
void g3_DrawBox(ddgr_color color,g3Point *pnt,float rad);

// Sets up a custom clipping plane - g3_StartFrame must be called before this is called
void g3_SetCustomClipPlane (ubyte state,vector *pnt,vector *normal);

// sets the z distance of the far clipping plane
void g3_SetFarClipZ (float z);

//Disables the far clip plane
inline void g3_ResetFarClipZ()
{
	g3_SetFarClipZ(FLT_MAX);
}

//Clips a polygon
//Parameters:	pointlist - pointer to a list of pointers to points
//					nv - the number of points in the polygon
//					cc - the clip codes for this polygon
//Returns:	a pointer to a list of pointer of points in the clipped polygon
//NOTE: You MUST call g3_FreeTempPoints() when you're done with the clipped polygon
g3Point **g3_ClipPolygon(g3Point **pointlist,int *nv,g3Codes *cc);

//Free up any temp points (created by the clipper) in the given pointlist
//Parameters:	pointlist - pointer to list of pointers to points, returned by g3_ClipPolygon()
//					nv - the number of points in pointlist
void g3_FreeTempPoints(g3Point **pointlist,int nv);

// Gets the matrix scale vector
void g3_GetMatrixScale	(vector *matrix_scale);

// Sets the triangulation test to on or off
void g3_SetTriangulationTest (int state);

//draws a line based on the current setting of render states. takes two points.  returns true if drew
void g3_DrawSpecialLine(g3Point *p0,g3Point *p1);

// Draws a bitmap on a specific plane.  Also does rotation.  Angle of rotation is passed as 'rot_angle'
void g3_DrawPlanarRotatedBitmap (vector *pos,vector *norm,angle rot_angle,float width,float height,int bm);

void g3_GenerateReflect(g3Plane& plane, float* mat);
void g3_Mat4Multiply(float* res, float* right);
void g3_Mat4Multiply(float* res, float* left, float* right);
void g3_GetModelViewMatrix( const vector *viewPos, const matrix *viewMatrix, float *mvMat );

extern float gTransformProjection[16];
extern float gTransformModelView[16];
extern float gTransformFull[16];

#endif

