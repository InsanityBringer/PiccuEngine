/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */
 

#include "stdafx.h"
#include <stdlib.h>

#include "descent.h"
#include "globals.h"
#include "room_external.h"
#include "3d.h"
#include "render.h"
#include "selectedroom.h"
#include "gr.h"
#include "terrain.h"
#include "neweditor.h"
#include "ned_DrawWorld.h"
#include "ned_OrthoWnd.h"
#include "ned_Rend.h"
#include "renderer.h"
#include "ned_Object.h"

//the viewport we're currently drawing to
grViewport *Draw_viewport;

//Colors for rooms/edges
#define	FACING_COLOR		GR_RGB( 198, 198, 198)
#define	NOTFACING_COLOR	GR_RGB( 125, 125, 125)	//GR_RGB( 158, 158, 158)
#define	PLACED_COLOR		GR_RGB( 255,   0, 255)
#define	SELECTED_COLOR		GR_RGB( 255, 166,   0)
#define	TERR_PORTAL_COLOR	GR_RGB(   0,   0, 255)
#define	CURROOM_COLOR		GR_RGB( 255, 255, 255)
#define	CURFACE_COLOR		GR_RGB(   0, 255,   0)
#define	CUREDGE_COLOR		GR_RGB( 255, 255,   0)
#define	MARKEDROOM_COLOR	GR_RGB(   0, 255, 255)
#define	MARKEDFACE_COLOR	GR_RGB( 255,   0,   0)
#define	MARKEDEDGE_COLOR	GR_RGB(   0, 150, 150)
#define  CURPORTAL_COLOR	GR_RGB( 200, 150, 255)
#define	FLOAT_TRIG_COLOR	GR_RGB( 255, 100, 100)

//Unused for now
#define	FOUND_COLOR			GR_RGB(   0, 121, 182)
#define	WARNING_COLOR		GR_RGB( 255,   0,   0)

//Colors for objects
#define	ROBOT_COLOR			GR_RGB( 255,   0,   0)		//a robot
#define	PLAYER_COLOR		GR_RGB(   0, 255,   0)		//a player object
#define	VIEWER_COLOR		GR_RGB( 100,   0, 100)		//a viewer
#define  POWERUP_COLOR		GR_RGB(   0,   0, 255)		//a powerup
#define	MISCOBJ_COLOR		GR_RGB(   0, 100, 100)		//some other object
#define  CAMERA_COLOR		GR_RGB( 255, 255,   0)		//a camera

//vars for WireframeFindRoomFace()
static int		Search_mode=0;             //if 1, searching for point, if 2, searching for box
static int		Search_x,Search_y;
static int		Search_find_mode;
static float	Search_min_dist = 0.0;
static int		Search_left,Search_top,Search_right,Search_bot;
static int		Search_roomnum,Search_facenum,Search_vertnum;
static vector	Search_viewer;					//used to determine closest room
static int		Found_roomnum;
static int		Found_facenum;
static int		Found_vertnum;
static float	Found_dist;
static grViewport	*last_vp;
vector		last_viewer_position;
vector		last_viewer_target;
matrix		last_view_orient;
static int			last_start_seg;
static int			last_start_room;
static float		last_rad;
float		last_zoom;

bool Dont_draw_dots = false;

extern matrix View_matrix;
void Terrain_start_frame (vector *eye,matrix *view_orient);

void DrawTerrainPoints (vector *view_pos,matrix *view_orient)
{
	g3Point terr_point;
	int i;

	Terrain_start_frame (view_pos,&View_matrix);

	for (i=0;i<TERRAIN_DEPTH*TERRAIN_WIDTH;i++)
	{
		GetPreRotatedPoint (&terr_point.p3_vec,i%TERRAIN_WIDTH,i/TERRAIN_WIDTH,Terrain_seg[i].ypos);
		terr_point.p3_flags=0;
		g3_CodePoint (&terr_point);
		if (!terr_point.p3_codes)
		{
			g3_ProjectPoint (&terr_point);
			Draw_viewport->setpixel (GR_RGB(255,255,255),terr_point.p3_sx,terr_point.p3_sy);
		}

	}

	
}

// Primitive struct
prim stPrim;

//edge types - lower number types have precedence
#define ET_EMPTY		255	//this entry in array is empty
#define ET_FACING		0		//this edge on a facing face
#define ET_NOTFACING	1		//this edge on a non-facing face
#define N_EDGE_TYPES	2		//how many edge types there are

typedef struct seg_edge {
	union {
		struct {short v0,v1;};
		long vv;
	};
	ushort	type;
} seg_edge;

#define MAX_EDGES (MAX_VERTS_PER_ROOM*2)

seg_edge edge_list[MAX_EDGES];

short used_list[MAX_EDGES];	//which entries in edge_list have been used
int n_used;

int edge_list_size;		//set each frame

#define HASH(a,b)  ((a*5+b) % edge_list_size)

#define swap(a,b) do {int t; t=(a); (a)=(b); (b)=t;} while (0)



//finds edge, filling in edge_ptr. if found old edge, returns index, else return -1
int FindEdge(int v0,int v1,seg_edge **edge_ptr)
{
	long vv;
	short hash,oldhash;
	int ret;

	vv = (v1<<16) + v0;

	oldhash = hash = HASH(v0,v1);

	ret = -1;

	while (ret==-1) {

		if (edge_list[hash].type == ET_EMPTY) ret=0;
		else if (edge_list[hash].vv == vv) ret=1;
		else {
			if (++hash==edge_list_size) hash=0;
			if (hash==oldhash) Error("Edge list full!");
		}
	}

	*edge_ptr = &edge_list[hash];

	if (ret == 0)
		return -1;
	else
		return hash;

}

//adds an edge to the edge list
void AddEdge(int v0,int v1,ubyte type)
{
	int found;

	seg_edge *e;

//mprintf(0, "Verts = %2i %2i, type = %i ", v0, v1, type);
	if (v0 > v1) swap(v0,v1);

	found = FindEdge(v0,v1,&e);

	if (found == -1) {
		e->v0 = v0;
		e->v1 = v1;
		e->type = type;
		used_list[n_used] = e-edge_list;
		n_used++;
	} else {
		if (type < e->type)
			e->type = type;
	}
}

//Adds all the rooms that have a vertex inside of a given screen box to the selected list
//Parameters:	vp - the viewport we're checking.  Must be the same viewport as the last wireframe view rendered
//					left,top,right,bot - the screen coordinates of the box
//Returns:		the number of rooms found
int SelectRoomsInBox(grViewport *vp,int left,int top,int right,int bot)
{

//TEMP: The caller doesn't know the viewport, so we set it here
vp = last_vp;

	if (vp != last_vp) {
		Int3();
		return -1;
	}

	Draw_viewport = vp;

	vp->clear();

	StartEditorFrame(vp,&last_viewer_position,&last_view_orient,last_zoom);

	Search_mode = 2;
	Search_left = left; Search_top = top; Search_right = right; Search_bot = bot;
	Search_viewer = last_viewer_position;
	Found_roomnum = 0;			//we use this as a counter

	DrawAllRooms(&last_viewer_target,last_rad);

	EndEditorFrame();

	Search_mode = 0;

	return Found_roomnum;

}

//Resets the edge list
void ResetEdgeList(int size)
{
	int i;

	edge_list_size = __min(size,MAX_EDGES);		//make maybe smaller than max

	for (i=0; i<edge_list_size; i++) {
		edge_list[i].type = ET_EMPTY;
	}

	n_used = 0;
}



int FindNearestVertInFace(face *fp, int search_x, int search_y)
{
	int v, nearest = 0;
	double x,y;
	double x_delta,y_delta;
	double old_x_delta = 10000,old_y_delta = 10000;

	for (v=0;v<fp->num_verts;v++) {

		g3_ProjectPoint((g3Point *)&World_point_buffer[fp->face_verts[v]]);

//		x = Round(World_point_buffer[v].p3_sx);
//		y = Round(World_point_buffer[v].p3_sy);

		x = World_point_buffer[fp->face_verts[v]].p3_sx;
		y = World_point_buffer[fp->face_verts[v]].p3_sy;

		x_delta = abs(x - search_x);
		y_delta = abs(y - search_y);
		if ( dist_2d(x_delta,y_delta) < dist_2d(old_x_delta,old_y_delta) ) {
			old_x_delta = x_delta; old_y_delta = y_delta;
			nearest = v;
		}
	}
	return nearest;
}


//If searching for a point, see if this room draws at the search x,y position, and if so, 
//if it's the closest to the viewer.  If searching for a box, see if any of the room points are
//inside the box.
void CheckRoom(room *rp)
{
	ubyte	codes_and = 0xff;
	int vn;

	//Rotate all the points
	for (vn=0;vn<rp->num_verts;vn++)
		codes_and &= g3_RotatePoint((g3Point *)&World_point_buffer[vn],&rp->verts[vn]);

	if (! codes_and) {		//all off screen?

		if (Search_mode == 1) {			//searching for a point
			int fn;
			ddgr_color oldcolor;
			face *fp;

			rend_SetLighting(LS_NONE);
			rend_SetTextureType (TT_FLAT);

			for (fn=0,fp=rp->faces;fn<rp->num_faces;fn++,fp++) {
				g3Point *vert_list[MAX_VERTS_PER_FACE];

				codes_and = 0xff;
				for (vn=0;vn<fp->num_verts;vn++) {
					vert_list[vn] = (g3Point *)&World_point_buffer[fp->face_verts[vn]];
					codes_and &= vert_list[vn]->p3_codes;
				}

				if (! codes_and) {

				  	Draw_viewport->setpixel(GR_RGB(0,0,0),Search_x,Search_y);
					oldcolor = Draw_viewport->getpixel(Search_x,Search_y); 	//will be different in 15/16-bit color
					rend_SetFlatColor (GR_RGB(255,255,255));

					g3_CheckAndDrawPoly(fp->num_verts,vert_list,0,NULL,NULL);
	
				  	if (Draw_viewport->getpixel(Search_x,Search_y) != oldcolor) {
						vector t;
						float dist;
	
						ComputeCenterPointOnFace(&t,rp,fn);
	
						dist = vm_VectorDistance(&t,&Search_viewer);
	
						if (Search_find_mode == FM_SPECIFIC) {
							if ((ROOMNUM(rp) == Search_roomnum) && (fn == Search_facenum)) {
								Found_roomnum = ROOMNUM(rp);
								Found_facenum = fn;
								Found_vertnum = FindNearestVertInFace(fp, Search_x, Search_y);
//								stPrim.vert = Found_vertnum;
								Found_dist = dist;
							}
						}
						else if ((Found_roomnum == -1  ||  dist < Found_dist) && (dist > Search_min_dist)) {
							Found_roomnum = ROOMNUM(rp);
							Found_facenum = fn;
							Found_vertnum = FindNearestVertInFace(fp, Search_x, Search_y);
//							stPrim.vert = Found_vertnum;
							Found_dist = dist;
						}
					}
				}
			}
  		}
		else if (Search_mode == 2) {				//searching in a box
			int v;

			for (v=0;v<rp->num_verts;v++) {
				int x,y;

				g3_ProjectPoint((g3Point *)&World_point_buffer[v]);

				x = Round(World_point_buffer[v].p3_sx);
				y = Round(World_point_buffer[v].p3_sy);

				if ((x >= Search_left) && (y >= Search_top) && (x <= Search_right) && (y <= Search_bot)) {
					if (/*(D3EditState.box_selection_mode==ACROSS_EDGE) || */(v==rp->num_verts-1)) {
						AddRoomToSelectedList(ROOMNUM(rp));
						Found_roomnum++;	//doubles as a counter
						break;				//no need to check rest of verts
					}
				}
/*
				else													//this point not in window
					if (D3EditState.box_selection_mode == IN_WINDOW)	//seg not entirely in window
						break;										//..so stop checking this room
*/
			}
		}
		else
			Int3();		//unknown Search_mode
		
	}
}


void DrawLine(ddgr_color color,int pnum0,int pnum1)
{
	g3Point pnt1,pnt2;
	
	
	pnt1=*((g3Point *)&World_point_buffer[pnum0]);
	pnt2=*((g3Point *)&World_point_buffer[pnum1]);

	g3_DrawLine(color,&pnt1,&pnt2);
}


void DrawEdges(int room_color)
{
	int i,type;
	seg_edge *e;
	ddgr_color edge_colors[] = {FACING_COLOR, NOTFACING_COLOR};

	for (type=N_EDGE_TYPES-1;type>=0;type--) {
		for (i=0;i<n_used;i++) {
			e = &edge_list[used_list[i]];
			if (e->type == type)
				DrawLine((room_color==-1)?edge_colors[type]:room_color,e->v0,e->v1);
		}
	}
}


//Finds the closest room:face to the viewer at a given x,y screen position in the wireframe view
//Parameters:	vp - the viewport we're checking.  Must be the same viewport as the last wireframe view rendered
//					x,y - the screen coordinates clicked on
//					roomnum,facenum - pointers to variables to be filled in
//					find_mode - see values in header file
//Returns:		true if found a room/face
bool WireframeFindRoomFace(grViewport *vp,int x,int y,int *roomnum,int *facenum,int find_mode,bool one_room)
{
	if (last_vp == NULL)
		return 0;

	if (vp != last_vp) {
		//Int3();
		return 0;
	}

	Draw_viewport = vp;

	vp->clear();

	StartEditorFrame(vp,&last_viewer_position,&last_view_orient,last_zoom);

	Search_mode = 1;
	Search_find_mode = find_mode;
	Search_x = x; Search_y = y;
	Search_viewer = last_viewer_position;
	Search_min_dist = (find_mode == FM_NEXT)?Found_dist:0.0;
	Search_roomnum = *roomnum;
	Search_facenum = *facenum;
	Found_roomnum = -1;
	Found_facenum = -1;

	if (one_room) {
		ASSERT(Search_roomnum != -1);
		CheckRoom(&Rooms[Search_roomnum]);
	}
	else {
		DrawAllRooms(&last_viewer_target,last_rad);
		CheckRoom(stPrim.roomp);
	}

	EndEditorFrame();

	Search_mode = 0;

	if (Found_roomnum != -1) {
		*roomnum = Found_roomnum;
		*facenum = Found_facenum;
		return 1;
	}
	else
		return 0;
}


void DrawRoomFace(ddgr_color color,room *rp,int facenum)
{
	face *fp = &rp->faces[facenum];
	g3Codes cc;
	int i;

	//Rotate the points in this face
   cc.cc_and = 0xff;  cc.cc_or = 0;
	for (i=0;i<fp->num_verts;i++) {
		int vertnum = fp->face_verts[i];
		g3_RotatePoint((g3Point *)&World_point_buffer[vertnum],&rp->verts[vertnum]);
      cc.cc_and &= World_point_buffer[vertnum].p3_codes;
      cc.cc_or  |= World_point_buffer[vertnum].p3_codes;
	}

	if (! cc.cc_and)		//if not all off screen
		for (i=0;i<fp->num_verts;i++)
			DrawLine(color,fp->face_verts[i],fp->face_verts[(i+1)%fp->num_verts]);
}

void DrawFaceEdge(ddgr_color color,room *rp,int facenum,int edgenum)
{
	face *fp = &rp->faces[facenum];
	int v0,v1;

	v0 = fp->face_verts[edgenum];
	v1 = fp->face_verts[(edgenum+1)%fp->num_verts];
	
	g3_RotatePoint((g3Point *)&World_point_buffer[v0],&rp->verts[v0]);
	g3_RotatePoint((g3Point *)&World_point_buffer[v1],&rp->verts[v1]);

	if (! (World_point_buffer[v0].p3_codes & World_point_buffer[v1].p3_codes))
		DrawLine(color,v0,v1);
}

//Draw the objects on the terrain as spheres
void DrawTerrainObjects()
{
	object *obj = NULL;
	bool bDrawOrient = false;

	for (int i=0;i<=Highest_object_index;i++)
	{
		obj=&Objects[i];
		if (obj->type==OBJ_ROOM)
			continue;
				
		if (obj->flags & OF_DEAD)
			continue;
		if (obj->render_type == RT_NONE)
			continue;
		if (! OBJECT_OUTSIDE(obj))
			continue;

		ddgr_color color;

  		switch (obj->type) {
  			case OBJ_POWERUP:	color = POWERUP_COLOR; bDrawOrient = true; break;
  			case OBJ_ROBOT:	color = ROBOT_COLOR; bDrawOrient = true; break;
  			case OBJ_PLAYER:	color = PLAYER_COLOR; bDrawOrient = true; break;
  			default:				color = MISCOBJ_COLOR; break;
  		}

		// Don't draw objects of certain types
		if (obj->type==OBJ_DOOR || obj->type==OBJ_VIEWER || obj->type==OBJ_CAMERA)
			continue;

		DrawObject(color,obj,bDrawOrient);
	}
}

//Draw the objects in a room as spheres
void DrawRoomObjects(room *rp)
{
	int objnum;
	bool bDrawOrient = false;

//	if (! D3EditState.objects_in_wireframe)
//		return;

	for (objnum=rp->objects;objnum!=-1;objnum=Objects[objnum].next) {
		object *obj = &Objects[objnum];
		ddgr_color color;

  		switch (obj->type) {
  			case OBJ_POWERUP:	color = POWERUP_COLOR; bDrawOrient = true; break;
  			case OBJ_ROBOT:	color = ROBOT_COLOR; bDrawOrient = true; break;
  			case OBJ_PLAYER:	color = PLAYER_COLOR; bDrawOrient = true; break;
  			default:				color = MISCOBJ_COLOR; break;
  		}

		// Don't draw objects of certain types
		if (obj->type==OBJ_DOOR || obj->type==OBJ_VIEWER || obj->type==OBJ_CAMERA)
			continue;

		DrawObject(color,obj,bDrawOrient);
	}
}



//Draw an object in a room as a sphere
void DrawRoomObject(room *rp, int objnum, int colorflag)
{
//	if (! D3EditState.objects_in_wireframe)
//		return;

	object *obj = &Objects[objnum];
	ddgr_color color;
	bool bDrawOrient = false;

	switch (colorflag)
	{
	case CF_DEFAULT:
  		switch (obj->type)
		{
  		case OBJ_POWERUP:	color = POWERUP_COLOR; bDrawOrient = true; break;
  		case OBJ_ROBOT:	color = ROBOT_COLOR; bDrawOrient = true; break;
  		case OBJ_PLAYER:	color = PLAYER_COLOR; bDrawOrient = true; break;
  		default:				color = MISCOBJ_COLOR; break;
		}
		break;

	case CF_CURRENT:
  		switch (obj->type)
		{
  		case OBJ_POWERUP:	color = POWERUP_COLOR; bDrawOrient = true; break;
  		case OBJ_ROBOT:	color = ROBOT_COLOR; bDrawOrient = true; break;
  		case OBJ_PLAYER:	color = PLAYER_COLOR; bDrawOrient = true; break;
  		default:				color = MISCOBJ_COLOR; break;
		}
		break;

	case CF_MARKED:
  		switch (obj->type)
		{
  		case OBJ_POWERUP:	color = POWERUP_COLOR; bDrawOrient = true; break;
  		case OBJ_ROBOT:	color = ROBOT_COLOR; bDrawOrient = true; break;
  		case OBJ_PLAYER:	color = PLAYER_COLOR; bDrawOrient = true; break;
  		default:				color = MISCOBJ_COLOR; break;
		}
		break;
	}

	// Don't draw objects of certain types
	if (obj->type!=OBJ_DOOR && obj->type!=OBJ_VIEWER && obj->type!=OBJ_CAMERA)
		DrawObject(color,obj,bDrawOrient);
}



// Draw object, with orientation vectors
void DrawObject(ddgr_color color,object *obj,bool bDrawOrient)
{
	g3Point sphere_point;

	// Draw sphere
	g3_RotatePoint(&sphere_point,&obj->pos);
	g3_DrawSphere(color,&sphere_point,obj->size);

	if (bDrawOrient)
	{
		g3Point g3_begin_up,g3_end_up,g3_begin_front,g3_end_front,g3_arrow_left,g3_arrow_right;
		vector vec_begin_up = obj->pos+obj->size*obj->orient.uvec;
		vector vec_end_up = obj->pos+2*obj->size*obj->orient.uvec;
		vector vec_begin_front = obj->pos+obj->size*obj->orient.fvec;
		vector vec_end_front = obj->pos+2*obj->size*obj->orient.fvec;
		vector vec_arrow_left = vec_end_front - 0.25*obj->size*(obj->orient.rvec + obj->orient.fvec);
		vector vec_arrow_right = vec_end_front + 0.25*obj->size*(obj->orient.rvec - obj->orient.fvec);

		// Draw up vector
		g3_RotatePoint(&g3_begin_up,&vec_begin_up);
		g3_RotatePoint(&g3_end_up,&vec_end_up);
		g3_DrawLine(color,&g3_begin_up,&g3_end_up);
		// Draw forward vector
		g3_RotatePoint(&g3_begin_front,&vec_begin_front);
		g3_RotatePoint(&g3_end_front,&vec_end_front);
		g3_DrawLine(color,&g3_begin_front,&g3_end_front);
		// Draw arrow head for forward vector
		g3_RotatePoint(&g3_arrow_left,&vec_arrow_left);
		g3_DrawLine(color,&g3_arrow_left,&g3_end_front);
		g3_RotatePoint(&g3_arrow_right,&vec_arrow_right);
		g3_DrawLine(color,&g3_arrow_right,&g3_end_front);
	}
}



#define CROSS_WIDTH  8.0
#define CROSS_HEIGHT 8.0

void DrawVertBox(vector *v,ddgr_color color)
{
  	//Draw a box at the marked vert
  	g3Point p0;
  	ubyte c0 = g3_RotatePoint(&p0,v);
  	if (! c0) {		//on screen?
  		g3_ProjectPoint(&p0);
  		rend_SetFlatColor(color);
  		rend_DrawLine(p0.p3_sx-CROSS_WIDTH,p0.p3_sy,p0.p3_sx,p0.p3_sy-CROSS_HEIGHT);
  		rend_DrawLine(p0.p3_sx,p0.p3_sy-CROSS_HEIGHT,p0.p3_sx+CROSS_WIDTH,p0.p3_sy);
  		rend_DrawLine(p0.p3_sx+CROSS_WIDTH,p0.p3_sy,p0.p3_sx,p0.p3_sy+CROSS_HEIGHT);
  		rend_DrawLine(p0.p3_sx,p0.p3_sy+CROSS_HEIGHT,p0.p3_sx-CROSS_WIDTH,p0.p3_sy);
  	}
}


// Draw x/y/z coordinate axes
void DrawAxes()
{
	vector origin = {0.0, 0.0, 0.0};
	vector x = {50.0, 0.0, 0.0};
	vector y = {0.0, 50.0, 0.0};
	vector z = {0.0, 0.0, 50.0};

	g3Point p0,px,py,pz;
	ubyte c0,cx,cy,cz;

	c0 = g3_RotatePoint(&p0, &origin);
	if (!c0)
		g3_ProjectPoint(&p0);

	cx = g3_RotatePoint(&px, &x);
	if (!cx)
		g3_ProjectPoint(&px);

	if (!c0 || !cx) {
		rend_SetFlatColor(GR_RGB(255,0,0));
		rend_DrawLine(p0.p3_sx,p0.p3_sy,px.p3_sx,px.p3_sy);
	}

	cy = g3_RotatePoint(&py, &y);
	if (!cy)
		g3_ProjectPoint(&py);

	if (!c0 || !cy) {
		rend_SetFlatColor(GR_RGB(0,255,0));
		rend_DrawLine(p0.p3_sx,p0.p3_sy,py.p3_sx,py.p3_sy);
	}

	cz = g3_RotatePoint(&pz, &z);
	if (!cz)
		g3_ProjectPoint(&pz);

	if (!c0 || !cz) {
		rend_SetFlatColor(GR_RGB(0,0,255));
		rend_DrawLine(p0.p3_sx,p0.p3_sy,pz.p3_sx,pz.p3_sy);
	}

}


//	Draw a room
void DrawRoom(room *rp,int room_color)
{
	int	fn,vn;
	face	*fp;
	int	edge_type;

	ResetEdgeList(MAX_EDGES);

	//Rotate all the points
	for (vn=0;vn<rp->num_verts;vn++)
		g3_RotatePoint((g3Point *)&World_point_buffer[vn],&rp->verts[vn]);

	for (fn=0,fp=rp->faces;fn<rp->num_faces;fn++,fp++) {
		if (fp->flags & FF_FLOATING_TRIG) {
			for (vn=0;vn<fp->num_verts;vn++)
				DrawLine(FLOAT_TRIG_COLOR,fp->face_verts[vn],fp->face_verts[(vn+1)%fp->num_verts]);
			DrawLine(FLOAT_TRIG_COLOR,fp->face_verts[0],fp->face_verts[2]);
			DrawLine(FLOAT_TRIG_COLOR,fp->face_verts[1],fp->face_verts[3]);
		}
		else if (fp->portal_num == -1) {
			bool facing = g3_CheckNormalFacing(&rp->verts[fp->face_verts[0]],&fp->normal);
			for (vn=0;vn<fp->num_verts;vn++) {
				if (facing)
					edge_type = ET_NOTFACING;
				else
					edge_type = ET_FACING;

				AddEdge(fp->face_verts[vn],fp->face_verts[(vn+1)%fp->num_verts],edge_type);
			}
		}
	}

	DrawEdges(room_color);

/*
	//Now draw the terrain portals
	for (int p=0;p<rp->num_portals;p++) {
		portal *pp = &rp->portals[p];
		if (pp->croom == -1) {			//terrain portal
			fp = &rp->faces[pp->portal_face];
			for (vn=0;vn<fp->num_verts;vn++)
				DrawLine(TERR_PORTAL_COLOR,fp->face_verts[vn],fp->face_verts[(vn+1)%fp->num_verts]);
		}
	}
*/

}


//	Draw a room rotated and placed in space
void DrawRoomRotated(room *rp,vector *rotpoint,matrix *rotmat,vector *placepoint,int room_color)
{
	int	fn,vn;
	face	*fp;

	ResetEdgeList(MAX_EDGES);
	
	//Rotate all the points
	for (vn=0;vn<rp->num_verts;vn++) {
		vector tv;
		tv = (rp->verts[vn] - *rotpoint) * *rotmat + *placepoint;
		g3_RotatePoint((g3Point *)&World_point_buffer[vn],&tv);
	}

	for (fn=0,fp=rp->faces;fn<rp->num_faces;fn++,fp++)
		for (vn=0;vn<fp->num_verts;vn++)
			AddEdge(fp->face_verts[vn],fp->face_verts[(vn+1)%fp->num_verts],ET_FACING);

	DrawEdges(room_color);
}


void DrawAllRooms(vector *view_target,float rad)
{
	int r;
	room *rp;

	for (r=0,rp=Rooms; r<=Highest_room_index; r++,rp++) {
		if (rp->used && (vm_VectorDistance(&rp->verts[0],view_target) < rad)) {
			if (Search_mode)
				CheckRoom(rp);
			else {
				DrawRoom(rp);
				DrawRoomObjects(rp);
			}
		}
	}
}


void DrawWorld(grViewport *vp,vector *view_target,matrix *view_orient,float view_dist,int start_room,float rad,int drawflags,prim *prim)
{	
	if (vp == NULL)
		return;

	//mprintf((0,"In DrawWorld\n"));
	vector viewer_position;
	float zoom = D3_DEFAULT_ZOOM;

	stPrim = *prim;

	ASSERT(stPrim.roomp != NULL);

	viewer_position = *view_target - (view_orient->fvec * view_dist);

	//save for possible use by find function
	last_vp = vp;
	last_viewer_position = viewer_position;
	last_viewer_target = *view_target;
	last_view_orient = *view_orient;
	last_start_room = start_room;
	last_rad = rad;
	last_zoom = zoom;

	Draw_viewport = vp;

	vp->clear();
	StartEditorFrame(vp,&viewer_position,view_orient,zoom);

	int r;
	room *rp;

	if (drawflags & DRAW_ALL) {
		// Draw terrain points
		if (!Dont_draw_dots)
			DrawTerrainPoints (&viewer_position,view_orient);

		// Draw all rooms
		for (r=0,rp=Rooms; r<=Highest_room_index; r++,rp++) {
			if (rp->used && (vm_VectorDistance(&rp->verts[0],view_target) < rad)) {
				if (rp != stPrim.roomp) {
					int color = -1;
					if (IsRoomSelected(r))
						color = SELECTED_COLOR;
					if (rp == Markedroomp)
						color = MARKEDROOM_COLOR;
					DrawRoom(rp,color);
				}
				DrawRoomObjects(rp);
			}
		}

		if (Placed_room != -1)
			if (Rooms[Placed_room].used)
				DrawRoomRotated(&Rooms[Placed_room],&Placed_room_origin,&Placed_room_rotmat,&Placed_room_attachpoint,PLACED_COLOR);

		if (Placed_group)
			for (int r=0;r<Placed_group->nrooms;r++)
				DrawRoomRotated(&Placed_group->rooms[r],&Placed_room_origin,&Placed_room_rotmat,&Placed_room_attachpoint,PLACED_COLOR);

		// Highlight marked room side.
		if (Markedroomp) {
			ASSERT(Markedface != -1);
			if (Markedface != -1)
			{
				DrawRoomFace(MARKEDFACE_COLOR,Markedroomp,Markedface);
				DrawFaceEdge(MARKEDEDGE_COLOR,Markedroomp,Markedface,Markededge);
				DrawVertBox(&Markedroomp->verts[Markedroomp->faces[Markedface].face_verts[Markedvert]],MARKEDEDGE_COLOR);
			}
		}

		//Draw current primitives (room, face, portal, edge, & vertex)
		DrawRoom(stPrim.roomp,CURROOM_COLOR);
		if (stPrim.face != -1 && stPrim.vert != -1 && stPrim.edge != -1)
		{
			DrawRoomFace(CURFACE_COLOR,stPrim.roomp,stPrim.face);
			if (stPrim.portal != -1)
				DrawRoomFace(CURPORTAL_COLOR,stPrim.roomp,stPrim.roomp->portals[stPrim.portal].portal_face);
			DrawFaceEdge(CUREDGE_COLOR,stPrim.roomp,stPrim.face,stPrim.edge);
			DrawVertBox(&stPrim.roomp->verts[stPrim.roomp->faces[stPrim.face].face_verts[stPrim.vert]],CUREDGE_COLOR);
		}

		// Draw terrain objects
		DrawTerrainObjects();
	} else {
	if (drawflags & DRAW_SEL) {
		// Draw selected rooms
		for (r=0,rp=Rooms; r<=Highest_room_index; r++,rp++) {
			// Draw only selected rooms
			if (IsRoomSelected(r)) {
				if (rp->used && (vm_VectorDistance(&rp->verts[0],view_target) < rad)) {
					int color = -1;
					DrawRoom(rp,color);
					DrawRoomObjects(rp);
				}
			}
		}
	}
	if (drawflags & DRAW_CUR) {
		//Draw current primitives (room, face, portal, edge, & vertex)
		DrawRoom(stPrim.roomp,CURROOM_COLOR);
		if (stPrim.face != -1 && stPrim.vert != -1 && stPrim.edge != -1)
		{
			DrawRoomFace(CURFACE_COLOR,stPrim.roomp,stPrim.face);
			if (stPrim.portal != -1)
				DrawRoomFace(CURPORTAL_COLOR,stPrim.roomp,stPrim.roomp->portals[stPrim.portal].portal_face);
			DrawFaceEdge(CUREDGE_COLOR,stPrim.roomp,stPrim.face,stPrim.edge);
			DrawVertBox(&stPrim.roomp->verts[stPrim.roomp->faces[stPrim.face].face_verts[stPrim.vert]],CUREDGE_COLOR);
		}
		if (stPrim.roomp->objects != -1)
			if ( Objects[Cur_object_index].roomnum == ROOMNUM(stPrim.roomp) )
				DrawRoomObject(stPrim.roomp, Cur_object_index, CF_CURRENT);
	}
	if (drawflags & DRAW_OBJ) {
		if (stPrim.roomp->objects != -1)
				DrawRoomObjects(stPrim.roomp);
	}
	if (drawflags & DRAW_AXES) {
		// Draw x/y/z coordinate axes
		DrawAxes();
	}
	}

	EndEditorFrame();
}