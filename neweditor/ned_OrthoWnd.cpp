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
#include "room_external.h"
#include "gr.h"
#include "selectedroom.h"
#include "SelManager.h"
#include "erooms.h"
#include "globals.h"
#include "ned_DrawWorld.h"
#include "ned_OrthoWnd.h"
#include "ned_HFile.h"
#include "HRoom.h"
#include "neweditor.h"
#include "ned_Geometry.h"
#include "ObjectDialogBar.h"
#include "ned_Object.h"
#include "object.h"
#include "MainFrm.h"
#include "EditLineDialog.h"
#include "gamepath.h"
#include "epath.h"
#include "ned_PathNode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Movement controls
#define PANNING ( (m_Mouse.left && m_Keys.shift && !m_Keys.ctrl && !m_Keys.alt) || (m_Mouse.mid && !m_Keys.ctrl && !m_Keys.shift && !m_Keys.alt) )
#define ZOOMING ( (m_Mouse.left && m_Keys.shift && m_Keys.ctrl && !m_Keys.alt)  || (m_Mouse.mid && !m_Keys.ctrl && m_Keys.shift && !m_Keys.alt) )

// often-used GDI objects
CPen g_pens[NUM_PENS];
CBrush g_brushes[NUM_BRUSHES];

void InitPens()
{
	g_pens[O_CUR_ROOM_COLOR].CreatePen(PS_SOLID,0,RGB(0,0,0));
	g_pens[O_CUR_FACE_COLOR].CreatePen(PS_SOLID,0,RGB(0,255,0));
	g_pens[O_CUR_EDGE_COLOR].CreatePen(PS_SOLID,0,RGB(255,255,0));
	g_pens[O_ROOM_COLOR].CreatePen(PS_SOLID,0,RGB(255,255,255));
	g_pens[O_VERT_OUTLINE_COLOR].CreatePen(PS_SOLID,0,RGB(0,0,0));
	g_pens[O_MARKED_FACE_COLOR].CreatePen(PS_SOLID,0,RGB(0,0,255));
	g_pens[O_NORMAL_COLOR].CreatePen(PS_SOLID,0,RGB(192,0,192));
	g_pens[O_GRIDLINE_COLOR].CreatePen(PS_SOLID,0,RGB(224,224,224));
	g_pens[O_ORIGINLINE_COLOR].CreatePen(PS_SOLID,3,RGB(255,255,255));
	g_pens[O_REFFRAME_COLOR].CreatePen(PS_DASH,0,RGB(0,192,0));
	g_pens[O_ROOMCENTER_COLOR].CreatePen(PS_DASH,0,RGB(0,255,255));
	g_pens[O_BR_GRIDLINE_COLOR].CreatePen(PS_SOLID,0,RGB(255,255,255));
	g_pens[O_GAMEPATH_COLOR].CreatePen(PS_SOLID,0,RGB(36,99,238));
	g_pens[O_AIPATH_COLOR].CreatePen(PS_SOLID,0,RGB(255,0,0));
}

void InitBrushes()
{
	g_brushes[O_CUR_VERT_COLOR].CreateSolidBrush(RGB(255,255,0));
	g_brushes[O_VERT_COLOR].CreateSolidBrush(RGB(192,192,192));
	g_brushes[O_MARKED_VERT_COLOR].CreateSolidBrush(RGB(0,0,255));
	g_brushes[O_NODE_COLOR].CreateSolidBrush(RGB(0,128,160));
	g_brushes[O_BNODE_COLOR].CreateSolidBrush(RGB(0,128,160));
}

// arrays of selectable grid spacings and snap distances
int Grids[NUM_GRIDS] = {1,2,5,10,20,50,100,10}; // last element is for custom
float Snaps[NUM_SNAPS] = {0.1f,0.2f,0.5f,1,2,5,10,20,50,100,10}; // last element is for custom

vec2D GetWPFromLP(POINT pt)
{
	vec2D pos = {pt.x,pt.y};

	pos.x /= WOS;
	pos.y /= WOS;

	return pos;
}

int FindPointRoom(vector *pnt);

/////////////////////////////////////////////////////////////////////////////
// Cned_OrthoWnd

IMPLEMENT_DYNCREATE(Cned_OrthoWnd, CWnd)

Cned_OrthoWnd::Cned_OrthoWnd()
{
	m_BackColor = RGB(192,192,192);
	m_nID = 0;
	m_InFocus = false;
	m_View_changed = false;
	m_State_changed = false;
	m_bShowVerts = true;
	m_bShowNormals = true;
	m_bShowObjects = true;
	m_bShowCenters = false;
	m_nGrid = 3;
	m_nSnap = 6;
	m_bGridShow = true;
	m_bGridSnap = true;
	m_bShowAttached = true;
	m_bCaptured = false;
	m_ScaleStep = 1.1f;
	m_rTracker.m_nStyle = CRectTracker::solidLine;
	m_pParentFrame = NULL;
	m_pPrim = NULL;
	m_pvec_InsertPos = NULL;
	m_pvec_RefPos = NULL;
	m_pVert_marks = NULL;
	m_pNum_marked_verts = NULL;
	m_pFace_marks = NULL;
	m_pNum_marked_faces = NULL;
	memset(&m_Mouse,0,sizeof(m_Mouse));
	memset(&m_Keys,0,sizeof(m_Keys));
	m_TT_ready = false;
	m_CustomGrid = 10;
}

Cned_OrthoWnd::~Cned_OrthoWnd()
{
}


BEGIN_MESSAGE_MAP(Cned_OrthoWnd, CWnd)
	//{{AFX_MSG_MAP(Cned_OrthoWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_KEYUP()
	ON_WM_MOUSEWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_SYSCHAR()
	ON_WM_TIMER()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_PAINT()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_COMMAND(ID_GRID_SIZE_1, OnGridSize1)
	ON_COMMAND(ID_GRID_SIZE_2, OnGridSize2)
	ON_COMMAND(ID_GRID_SIZE_3, OnGridSize3)
	ON_COMMAND(ID_GRID_SIZE_4, OnGridSize4)
	ON_COMMAND(ID_GRID_SIZE_5, OnGridSize5)
	ON_COMMAND(ID_SHOW_ATCH_ROOMS, OnShowAtchRooms)
	ON_COMMAND(ID_SHOW_GRID, OnShowGrid)
	ON_COMMAND(ID_SHOW_NORMALS, OnShowNormals)
	ON_COMMAND(ID_SHOW_VERTS, OnShowVerts)
	ON_COMMAND(ID_CENTER_ROOM, OnCenterRoom)
	ON_COMMAND(ID_CENTER_ORIGIN, OnCenterOrigin)
	ON_WM_CHAR()
	ON_COMMAND(ID_CENTER_FACE, OnCenterFace)
	ON_WM_ERASEBKGND()
	ON_COMMAND(ID_SHOW_OBJECTS, OnShowObjects)
	ON_COMMAND(ID_GRID_SIZE_0, OnGridSize0)
	ON_COMMAND(ID_SNAP_TO_GRID, OnSnapToGrid)
	ON_COMMAND(ID_GRID_SIZE_6, OnGridSize6)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_0, OnUpdateGridSize0)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_1, OnUpdateGridSize1)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_2, OnUpdateGridSize2)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_3, OnUpdateGridSize3)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_4, OnUpdateGridSize4)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_5, OnUpdateGridSize5)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_6, OnUpdateGridSize6)
	ON_UPDATE_COMMAND_UI(ID_SHOW_VERTS, OnUpdateShowVerts)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NORMALS, OnUpdateShowNormals)
	ON_UPDATE_COMMAND_UI(ID_SHOW_OBJECTS, OnUpdateShowObjects)
	ON_UPDATE_COMMAND_UI(ID_SNAP_TO_GRID, OnUpdateSnapToGrid)
	ON_UPDATE_COMMAND_UI(ID_SHOW_GRID, OnUpdateShowGrid)
	ON_UPDATE_COMMAND_UI(ID_SHOW_ATCH_ROOMS, OnUpdateShowAtchRooms)
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_GRID_SIZE_CUSTOM, OnGridSizeCustom)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_CUSTOM, OnUpdateGridSizeCustom)
	ON_COMMAND(ID_SHOWROOMCENTER, OnShowRoomCenter)
	ON_UPDATE_COMMAND_UI(ID_SHOWROOMCENTER, OnUpdateShowRoomCenter)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void Cned_OrthoWnd::Render()
{
	InvalidateRect(NULL, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Cned_OrthoWnd message handlers

int Cned_OrthoWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here
	m_TimerHandle = SetTimer(gTimerID++,10,NULL);

	return 0;
}

void Cned_OrthoWnd::OnDestroy() 
{
	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here
}

BOOL Cned_OrthoWnd::Create(const RECT& rect, CWnd* pParentWnd, LPCTSTR name, UINT nID) 
{
	// TODO: Add your specialized code here and/or call the base class
	// this is not even called when pane is created within a splitter window
	DWORD dwStyle = GRWND_STATIC_STYLE;

	return CWnd::Create(NULL, name, dwStyle, rect, pParentWnd, nID);
}


void Cned_OrthoWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	CClientDC dc(this);
	CRect rc;

	m_Mouse.x = point.x;
	m_Mouse.y = point.y;

	GetClientRect(&rc);

	// Set up mapping mode and coordinate system
	dc.SetMapMode(MM_ISOTROPIC);
	dc.SetWindowExt(rc.Width()/m_Cam.scale,rc.Height()/m_Cam.scale);
	dc.SetWindowOrg(WOS*m_Cam.pos.x,WOS*m_Cam.pos.y);
	dc.SetViewportExt(rc.Width(), -rc.Height());
	dc.SetViewportOrg(rc.Width()/2, rc.Height()/2);

	// Calculate logical coordinates
	POINT pt = point;
	dc.DPtoLP(&pt);
	// Get world coordinates
	vec2D pos = GetWPFromLP(pt);

	vec2D snap_pt = pos;
	// Snap the point
	if (m_bGridSnap)
		SnapPoint(&snap_pt);

	// Save position (for use as the mouse position for status bar update)
	SetMousePos(snap_pt);

	CWnd::OnMouseMove(nFlags, point);
}


void EndSel(editorSelectorManager *esm);

void Cned_OrthoWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.left = true;

	if (!m_InFocus)
	{
		SetFocus();
		return;
	}

	CClientDC dc(this);
	CRect rc;

	GetClientRect(&rc);

	// Set up mapping mode and coordinate system
	dc.SetMapMode(MM_ISOTROPIC);
	dc.SetWindowExt(rc.Width()/m_Cam.scale,rc.Height()/m_Cam.scale);
	dc.SetWindowOrg(WOS*m_Cam.pos.x,WOS*m_Cam.pos.y);
	dc.SetViewportExt(rc.Width(), -rc.Height());
	dc.SetViewportOrg(rc.Width()/2, rc.Height()/2);

	// Calculate logical coordinates
	POINT pt = point;
	dc.DPtoLP(&pt);
	// Get world coordinates
	vec2D pos = GetWPFromLP(pt);

	vec2D snap_pt = pos;
	// Snap the point
	if (m_bGridSnap)
		SnapPoint(&snap_pt);

	// Save position (for use as the position to insert something)
	SetCursorPos(snap_pt);

	// Ready the tooltip for activation
	m_TT_ready = true;

	if (m_Keys.ctrl)
		SetRefFrame(snap_pt);

	if (!m_Keys.shift) {
		m_rTracker.TrackRubberBand(this, point);
//		m_rTracker.GetTrueRect(&m_rTracker.m_rect);
		m_Mouse.left = false;
	}

	room *rp = m_pPrim->roomp;
	int oldvert,vertnum;
	bool not_in_face;

	// This part is VERY touchy -- don't mess with it!
	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
	case MODE_FACE:
		oldvert = m_pPrim->vert;
		vertnum = SelectVert(m_nID,pos,&not_in_face);

		if (not_in_face) {
			m_pParentFrame->m_Current_nif_vert = vertnum;
			m_pParentFrame->SetPrim(rp,-1,-1,-1,-1);
		}
		else
		{
			m_pPrim->vert = vertnum;
			m_pParentFrame->m_Current_nif_vert = -1;

//			if (m_pPrim->roomp->faces[m_pPrim->face].face_verts[m_pPrim->vert] == m_pPrim->roomp->faces[m_pPrim->face].face_verts[oldvert])
//			{
				int facenum = SelectFace(m_nID,pos);
				if (facenum != -1)
					m_pParentFrame->SetPrim(rp,facenum,rp->faces[facenum].portal_num,0,0);
//			}
			m_pPrim->vert = SelectVert(m_nID,pos,&not_in_face);
			ASSERT(not_in_face == false);
			// Get a face and face vert from selected vert
			if (m_pPrim->face != -1)
				m_pPrim->portal = rp->faces[m_pPrim->face].portal_num;
			// Update the current face/texture displays
			Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), m_pPrim->face);
		}

		// Mark whatever is in the selection box
		if (Editor_state.mode == MODE_VERTEX)
			MarkVertsInSelBox(&dc,!m_Keys.alt);
		else
			MarkFacesInSelBox(&dc,!m_Keys.alt);

		break;

	case MODE_OBJECT:
		{
			int objnum = OrthoSelectObject(m_nID,pos);
			if (objnum != -1)
			{
				SelectObject(objnum);
				// Update the current object display
				Editor_state.SetCurrentLevelObject(Cur_object_index);
			}
		}
		break;

	case MODE_PATH:
		{
			int path,node;
			OrthoSelectPathNode(m_nID,pos,&path,&node);
			Editor_state.SetCurrentPath(path);
			Editor_state.SetCurrentNode(node);

			if (path != -1)
				PrintStatus("Path %d, Node %d selected. Path Name = %s",path,node+1,GamePaths[path].name);
			m_State_changed = true;
		}
		break;
	}

	CWnd::OnLButtonDown(nFlags, point);
}

int GetNumObjectsInRoom(room *rp);

int Cned_OrthoWnd::OrthoSelectObject(int view_flag,vec2D search_pos)
{
	int closest[MAX_MOUSE_SELECT];
	int close_dist[MAX_MOUSE_SELECT];
	int low_dist;
	int cur_dist;
	int cur_close;
	int swap,swap2;
	int pos;
	int i,k;
	int count = 0;
	int sel_obj = -1;

	int num_objects = GetNumObjectsInRoom(m_pPrim->roomp);
	cur_close = -1;
	low_dist = VERT_SELECT_SIZE * 10;
	if (m_pPrim->roomp->objects == -1)
	{
		ASSERT(num_objects == 0);
		return sel_obj;
	}
	object *obj = &Objects[m_pPrim->roomp->objects];

	for (i=0; i<=num_objects; i++,obj=&Objects[obj->next])
	{
		// Don't select objects of certain types
		if (obj->type==OBJ_DOOR || obj->type==OBJ_VIEWER || obj->type==OBJ_CAMERA)
			continue;

		cur_dist = Find2DDistance(view_flag,search_pos,obj->pos);
		if (cur_dist < VERT_SELECT_SIZE && count < MAX_MOUSE_SELECT )
		{
			closest[count] = OBJNUM(obj);
			close_dist[count] = cur_dist;
			count++;
		}
		else if (cur_dist < low_dist)
		{
			low_dist = cur_dist;
			cur_close = OBJNUM(obj);
		}
	}

	if (count > 0)
	{
		// Sort the objects by z-value (most positive at 0)
		for (i=count; i>0; i--)
		{
			for (k=1; k<i; k++)
			{
				if ( CompareDepth(view_flag,closest[k-1],closest[k]) )
				{
					swap = closest[k-1];
					closest[k-1] = closest[k];
					closest[k] = swap;
					swap2 = close_dist[k-1];
					close_dist[k-1] = close_dist[k];
					close_dist[k] = swap2;
				}
			}
		}

		pos = -1;
		// Check if current object is the closest
		for (i=0; i<count; i++)
		{
			if (closest[i] == sel_obj)
				pos = i;
		}

		if (pos > -1)
		{
			if (pos == count - 1)
				pos = 0;
			else
				pos++;
			sel_obj = closest[pos];
		}
		else
		{
			// m_pPrim->vert = closest[0];
			// Choose the object that is physically closer
			low_dist = VERT_SELECT_SIZE + 1;
			for (i=0; i<count; i++)
			{
				if ( close_dist[i] < low_dist )
				{
					low_dist = close_dist[i];
					sel_obj = closest[i];
				}
			}
		}
	}
	else
	{
		// Failed to find a close object
		if (cur_close > -1)
			sel_obj = cur_close;
	}

	return sel_obj;
}

void Cned_OrthoWnd::OrthoSelectPathNode(int view_flag,vec2D search_pos,int *path,int *node)
{
	int closest_node[MAX_MOUSE_SELECT];
	int closest_path[MAX_MOUSE_SELECT];
	int close_dist[MAX_MOUSE_SELECT];
	int low_dist;
	int cur_dist;
	int cur_close_node,cur_close_path;
	int swap,swap2,swap3;
	int pos;
	int i,j,k;
	int count = 0;
	int sel_path = Editor_state.GetCurrentPath();
	int sel_node = Editor_state.GetCurrentNode();
	game_path *gp;

	cur_close_node = -1;
	cur_close_path = -1;
	low_dist = VERT_SELECT_SIZE * 10;
	if (!Num_game_paths)
	{
		*path = sel_path;
		*node = sel_node;
	}
	int current_path_index = GetFirstPath();

	for (i=0; i<Num_game_paths; i++,current_path_index=GetNextPath(current_path_index))
	{
		gp = &GamePaths[current_path_index];
		for (j=0; j<gp->num_nodes; j++)
		{
			cur_dist = Find2DDistance(view_flag,search_pos,gp->pathnodes[j].pos);
			if (cur_dist < VERT_SELECT_SIZE && count < MAX_MOUSE_SELECT )
			{
				closest_node[count] = j;
				closest_path[count] = current_path_index;
				close_dist[count] = cur_dist;
				count++;
			}
			else if (cur_dist < low_dist)
			{
				low_dist = cur_dist;
				cur_close_node = j;
				cur_close_path = current_path_index;
			}
		}
	}

	if (count > 0)
	{
		// Sort the nodes by z-value (most positive at 0)
		for (i=count; i>0; i--)
		{
			for (k=1; k<i; k++)
			{
				if ( CompareDepth(view_flag,closest_node[k-1],closest_node[k]) )
				{
					swap = closest_node[k-1];
					closest_node[k-1] = closest_node[k];
					closest_node[k] = swap;
					swap2 = close_dist[k-1];
					close_dist[k-1] = close_dist[k];
					close_dist[k] = swap2;
					swap3 = closest_node[k-1];
					closest_node[k-1] = closest_path[k];
					closest_path[k] = swap3;
				}
			}
		}

		pos = -1;
		// Check if current node is the closest
		for (i=0; i<count; i++)
		{
			if (closest_node[i] == sel_node && closest_path[i] == sel_path)
				pos = i;
		}

		if (pos > -1)
		{
			if (pos == count - 1)
				pos = 0;
			else
				pos++;
			sel_path = closest_path[pos];
			sel_node = closest_node[pos];
		}
		else
		{
			// m_pPrim->vert = closest[0];
			// Choose the node that is physically closer
			low_dist = VERT_SELECT_SIZE + 1;
			for (i=0; i<count; i++)
			{
				if ( close_dist[i] < low_dist )
				{
					low_dist = close_dist[i];
					sel_path = closest_path[i];
					sel_node = closest_node[i];
				}
			}
		}
	}
	else
	{
		// Failed to find a close node
		if (cur_close_node > -1 && cur_close_path > -1)
		{
			sel_path = cur_close_path;
			sel_node = cur_close_node;
		}
	}

	*path = sel_path;
	*node = sel_node;
}

void Cned_OrthoWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.left = false;

	CWnd::OnLButtonUp(nFlags, point);
}

void Cned_OrthoWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.right = true;

	CWnd::OnRButtonDown(nFlags, point);
}

void Cned_OrthoWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.right = false;

	CWnd::OnRButtonUp(nFlags, point);
}

void Cned_OrthoWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.mid = true;

	if (!m_InFocus)
		SetFocus();

	CWnd::OnMButtonDown(nFlags, point);
}

void Cned_OrthoWnd::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.mid = false;

	CWnd::OnMButtonUp(nFlags, point);
}

void Cned_OrthoWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch (nChar)
	{
	case VK_CONTROL:
		m_Keys.ctrl = false;
		break;

	case VK_SHIFT:
		m_Keys.shift = false;
		break;
	}
	
	CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL Cned_OrthoWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	float old_scale = m_Cam.scale;

	if (zDelta > 0)
		m_Cam.scale *= zDelta/WHEEL_DELTA*m_ScaleStep;
	else
		m_Cam.scale /= abs(zDelta)/WHEEL_DELTA*m_ScaleStep;
	if (m_Cam.scale > O_MAX_SCALE)
		m_Cam.scale = O_MAX_SCALE;
	else if (m_Cam.scale < O_MIN_SCALE)
		m_Cam.scale = O_MIN_SCALE;

	if (old_scale != m_Cam.scale)
		m_Cam.view_changed = true;
	
	return true;//CWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void DoManualPathPointSet(room *rp,vector *pos)
{
	if ( FindPointRoom(pos) != -1 )
	{
		rp->path_pnt = *pos;
		rp->flags |= RF_MANUAL_PATH_PNT;

		OutrageMessageBox("Manual path point for room %d set to %.2f, %.2f, %.2f",ROOMNUM(rp),pos->x,pos->y,pos->z);

		World_changed = 1;
	}
	else
		OutrageMessageBox("Error setting manual path point for room %d: current position is outside of the room",ROOMNUM(rp));
}

void Cned_OrthoWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	static int vert_idx = 0;
	room *rp = m_pPrim->roomp;
	face *fp;
	short marked_vert_list[MAX_VERTS_PER_ROOM],marked_face_list[MAX_FACES_PER_ROOM];
	short verts_in_faces[MAX_VERTS_PER_ROOM];
	angle ang;
	float dist;
	int num_m_verts = 0;
	int num_m_faces = 0;
	int i=0,j=0,k=0;
	bool ret;
	object *objp = &Objects[Cur_object_index];
	int grid_size;
	(m_nGrid == NUM_GRIDS-1) ? (grid_size = m_CustomGrid) : (grid_size = Grids[m_nGrid]);
	int room,path,node,edge;

	memset(verts_in_faces,0,MAX_VERTS_PER_ROOM);

	switch (nChar)
	{
	case VK_CONTROL:
		m_Keys.ctrl = true;
		break;

	case VK_SHIFT:
		m_Keys.shift = true;
		break;

	case VK_NUMPAD7:
numpad7:
		if (Editor_state.mode == MODE_OBJECT && objp->roomnum == ROOMNUM(m_pPrim->roomp))
		{
			if (HIWORD(::GetKeyState(VK_CONTROL)))
				ang = (angle)(0x2000); // 45 degrees
			else if (HIWORD(::GetKeyState(VK_SHIFT)))
				ang = (angle)(0x80); // ~0.7 degrees // 0xB6 = ~1 degree
			else
				ang = (angle)(0x800); // 45/4 degrees

			OrientObject(Cur_object_index,65535-ang);
		}
		else if (Editor_state.mode == MODE_PATH)
		{
		}
		break;

	case VK_NUMPAD9:
numpad9:
		if (Editor_state.mode == MODE_OBJECT && objp->roomnum == ROOMNUM(m_pPrim->roomp))
		{
			if (HIWORD(::GetKeyState(VK_CONTROL)))
				ang = (angle)(0x2000); // 45 degrees
			else if (HIWORD(::GetKeyState(VK_SHIFT)))
				ang = (angle)(0x80); // ~0.7 degrees // 0xB6 = ~1 degree
			else
				ang = (angle)(0x800); // 45/4 degrees

			OrientObject(Cur_object_index,ang);
		}
		else if (Editor_state.mode == MODE_PATH)
		{
		}
		break;

	case VK_NUMPAD1:
numpad1:
		if (HIWORD(::GetKeyState(VK_CONTROL)))
			ang = (angle)(0x2000); // 45 degrees
		else if (HIWORD(::GetKeyState(VK_SHIFT)))
			ang = (angle)(0x80); // ~0.7 degrees // 0xB6 = ~1 degree
		else
			ang = (angle)(0x800); // 45/4 degrees

		switch (Editor_state.mode)
		{
		case MODE_VERTEX:
		case MODE_FACE:
			// Get list of marked verts
			num_m_verts = m_pParentFrame->GetMarkedVerts(marked_vert_list);
			if (!num_m_verts)
				break;

			switch (m_nID)
			{
				case VIEW_XY:
					ret = RotateVerts(rp,marked_vert_list,num_m_verts,0,0,-ang,*m_pvec_RefPos);
					break;
				case VIEW_XZ:
					ret = RotateVerts(rp,marked_vert_list,num_m_verts,0,-ang,0,*m_pvec_RefPos);
					break;
				case VIEW_ZY:
					ret = RotateVerts(rp,marked_vert_list,num_m_verts,-ang,0,0,*m_pvec_RefPos);
					break;
			}
			if (ret)
			{
				PrintStatus("Marked vertices rotated.");
				m_pParentFrame->m_Room_changed = true;
			}
			else
				OutrageMessageBox("Vertices not rotated.");
			break;

			case MODE_OBJECT:
			if (objp->roomnum != ROOMNUM(m_pPrim->roomp))
				break;

			switch (m_nID)
			{
			case VIEW_XY:
				ret = RotateObjectAroundPoint(objp,0,0,ang,*m_pvec_RefPos);
				break;
			case VIEW_XZ:
				ret = RotateObjectAroundPoint(objp,0,-ang,0,*m_pvec_RefPos);
				break;
			case VIEW_ZY:
				ret = RotateObjectAroundPoint(objp,-ang,0,0,*m_pvec_RefPos);
				break;
			}
			if (ret)
			{
				PrintStatus("Current object rotated.");
				World_changed = true;
				m_pParentFrame->UpdateAllPanes();
			}
			else
				OutrageMessageBox("Current object not rotated.");
			break;

		case MODE_PATH:
			break;
		} // END OF switch (Editor_state.mode)
		break;

	case VK_NUMPAD3:
numpad3:
		if (HIWORD(::GetKeyState(VK_CONTROL)))
			ang = (angle)(0x2000); // 45 degrees
		else if (HIWORD(::GetKeyState(VK_SHIFT)))
			ang = (angle)(0x80); // ~0.7 degrees // 0xB6 = ~1 degree
		else
			ang = (angle)(0x800); // 45/4 degrees

		switch (Editor_state.mode)
		{
		case MODE_VERTEX:
		case MODE_FACE:
			// Get list of marked verts
			num_m_verts = m_pParentFrame->GetMarkedVerts(marked_vert_list);
			if (!num_m_verts)
				break;

			switch (m_nID)
			{
				case VIEW_XY:
					ret = RotateVerts(rp,marked_vert_list,num_m_verts,0,0,ang,*m_pvec_RefPos);
					break;
				case VIEW_XZ:
					ret = RotateVerts(rp,marked_vert_list,num_m_verts,0,ang,0,*m_pvec_RefPos);
					break;
				case VIEW_ZY:
					ret = RotateVerts(rp,marked_vert_list,num_m_verts,ang,0,0,*m_pvec_RefPos);
					break;
			}
			if (ret)
			{
				PrintStatus("Marked vertices rotated.");
				m_pParentFrame->m_Room_changed = true;
			}
			else
				OutrageMessageBox("Vertices not rotated.");
			break;

		case MODE_OBJECT:
			if (objp->roomnum != ROOMNUM(m_pPrim->roomp))
				break;

			switch (m_nID)
			{
			case VIEW_XY:
				ret = RotateObjectAroundPoint(objp,0,0,ang,*m_pvec_RefPos);
				break;
			case VIEW_XZ:
				ret = RotateObjectAroundPoint(objp,0,ang,0,*m_pvec_RefPos);
				break;
			case VIEW_ZY:
				ret = RotateObjectAroundPoint(objp,ang,0,0,*m_pvec_RefPos);
				break;
			}
			if (ret)
			{
				PrintStatus("Current object rotated.");
				World_changed = true;
				m_pParentFrame->UpdateAllPanes();
			}
			else
				OutrageMessageBox("Current object not rotated.");
			break;

		case MODE_PATH:
			break;
		} // END OF switch (Editor_state.mode)
		break;

	case VK_NUMPAD4:
	case VK_LEFT:
	case 0x41:			// 'A'
		if (m_bGridShow)
			dist = -grid_size;
		else
			dist = -0.1f;

		if (HIWORD(::GetKeyState(VK_CONTROL)))
			MoveCursor(dist,0);
		else if (nChar == VK_NUMPAD4 || HIWORD(::GetKeyState(VK_SHIFT)))
		{
			switch (Editor_state.mode)
			{
			case MODE_VERTEX:
				// Get list of marked verts
				num_m_verts = m_pParentFrame->GetMarkedVerts(marked_vert_list);
				if (!num_m_verts)
					break;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,dist,0,0);
					break;
				case VIEW_XZ:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,dist,0,0);
					break;
				case VIEW_ZY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,0,dist);
					break;
				}
				if (ret)
				{
					PrintStatus("Marked vertices moved %.2f units.",fabs(dist));
					m_pParentFrame->m_Room_changed = true;
				}
				else
					OutrageMessageBox("Vertices could not be moved.");
				break;

			case MODE_FACE:
				// Get list of marked faces
				num_m_faces = m_pParentFrame->GetMarkedFaces(marked_face_list);
				if (!num_m_faces)
					break;

				// Determine which verts are in the marked faces
				for (i=0; i<num_m_faces; i++)
				{
					fp = &rp->faces[marked_face_list[i]];
					for (j=0; j<fp->num_verts; j++)
					{
						vert_idx = rp->faces[marked_face_list[i]].face_verts[j];
						if ( !verts_in_faces[vert_idx] )
							verts_in_faces[vert_idx] = true;
					}
				}
				// Fill the marked_vert_list array with all the verts in the marked faces
				memset(marked_vert_list,0,sizeof(short)*MAX_VERTS_PER_ROOM);
				for (j=0; j<rp->num_verts; j++)
					if ( verts_in_faces[j] )
						marked_vert_list[num_m_verts++] = j;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,dist,0,0);
					break;
				case VIEW_XZ:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,dist,0,0);
					break;
				case VIEW_ZY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,0,dist);
					break;
				}
				if (ret)
				{
					PrintStatus("Marked faces moved %.2f units.",fabs(dist));
					m_pParentFrame->m_Room_changed = true;
				}
				else
					OutrageMessageBox("Faces could not be moved.");
				break;

			case MODE_OBJECT:
				if (objp->roomnum != ROOMNUM(m_pPrim->roomp))
					break;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveObject(objp,objp->pos.x+dist,objp->pos.y,objp->pos.z);
					break;
				case VIEW_XZ:
					ret = MoveObject(objp,objp->pos.x+dist,objp->pos.y,objp->pos.z);
					break;
				case VIEW_ZY:
					ret = MoveObject(objp,objp->pos.x,objp->pos.y,objp->pos.z+dist);
					break;
				}
				if (ret)
				{
					PrintStatus("Current object moved %.2f units.",fabs(dist));
					World_changed = true;
					m_pParentFrame->UpdateAllPanes();
				}
				else
					OutrageMessageBox("Current object could not be moved.");
				break;

			case MODE_PATH:
				if (Editor_state.GetCurrentNodeType() == NODETYPE_GAME)
				{
					path = Editor_state.GetCurrentPath();
					node = Editor_state.GetCurrentNode();

					switch (m_nID)
					{
					case VIEW_XY:
						ret = ned_MoveNode(path,node,(float)dist,0.0f,0.0f);
						break;

					case VIEW_XZ:
						ret = ned_MoveNode(path,node,(float)dist,0.0f,0.0f);
						break;

					case VIEW_ZY:
						ret = ned_MoveNode(path,node,0.0f,0.0f,(float)dist);
						break;
					}
				}
				else
				{
					Editor_state.GetCurrentBNode(&room,&node,&edge);

					switch (m_nID)
					{
					case VIEW_XY:
						ret = ned_MoveBNode(room,node,(float)dist,0.0f,0.0f);
						break;

					case VIEW_XZ:
						ret = ned_MoveBNode(room,node,(float)dist,0.0f,0.0f);
						break;

					case VIEW_ZY:
						ret = ned_MoveBNode(room,node,0.0f,0.0f,(float)dist);
						break;
					}
				}

				if (ret)
				{
					PrintStatus("Current node moved %.2f units.",fabs(dist));
					World_changed = true;
					m_pParentFrame->UpdateAllPanes();
				}
				break;
			} // END OF switch (Editor_state.mode)
		}
		else
		{
			m_Cam.pos.x -= grid_size;
			m_Cam.view_changed = true;
		}
		break;

	case VK_NUMPAD6:
	case VK_RIGHT:
	case 0x44:				// 'D'
		if (m_bGridShow)
			dist = grid_size;
		else
			dist = 0.1f;

		if (HIWORD(::GetKeyState(VK_CONTROL)))
			MoveCursor(dist,0);
		else if (nChar == VK_NUMPAD6 || HIWORD(::GetKeyState(VK_SHIFT)))
		{
			switch (Editor_state.mode)
			{
			case MODE_VERTEX:
				// Get list of marked verts
				num_m_verts = m_pParentFrame->GetMarkedVerts(marked_vert_list);
				if (!num_m_verts)
					break;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,dist,0,0);
					break;
				case VIEW_XZ:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,dist,0,0);
					break;
				case VIEW_ZY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,0,dist);
					break;
				}
				if (ret)
				{
					PrintStatus("Marked vertices moved %.2f units.",fabs(dist));
					m_pParentFrame->m_Room_changed = true;
				}
				else
					OutrageMessageBox("Vertices could not be moved.");
				break;

			case MODE_FACE:
				// Get list of marked faces
				num_m_faces = m_pParentFrame->GetMarkedFaces(marked_face_list);
				if (!num_m_faces)
					break;

				// Determine which verts are in the marked faces
				for (i=0; i<num_m_faces; i++)
				{
					fp = &rp->faces[marked_face_list[i]];
					for (j=0; j<fp->num_verts; j++)
					{
						vert_idx = rp->faces[marked_face_list[i]].face_verts[j];
						if ( !verts_in_faces[vert_idx] )
							verts_in_faces[vert_idx] = true;
					}
				}
				// Fill the marked_vert_list array with all the verts in the marked faces
				memset(marked_vert_list,0,sizeof(short)*MAX_VERTS_PER_ROOM);
				for (j=0; j<rp->num_verts; j++)
					if ( verts_in_faces[j] )
						marked_vert_list[num_m_verts++] = j;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,dist,0,0);
					break;
				case VIEW_XZ:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,dist,0,0);
					break;
				case VIEW_ZY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,0,dist);
					break;
				}
				if (ret)
				{
					PrintStatus("Marked faces moved %.2f units.",fabs(dist));
					m_pParentFrame->m_Room_changed = true;
				}
				else
					OutrageMessageBox("Faces could not be moved.");
				break;

			case MODE_OBJECT:
				if (objp->roomnum != ROOMNUM(m_pPrim->roomp))
					break;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveObject(objp,objp->pos.x+dist,objp->pos.y,objp->pos.z);
					break;
				case VIEW_XZ:
					ret = MoveObject(objp,objp->pos.x+dist,objp->pos.y,objp->pos.z);
					break;
				case VIEW_ZY:
					ret = MoveObject(objp,objp->pos.x,objp->pos.y,objp->pos.z+dist);
					break;
				}
				if (ret)
				{
					PrintStatus("Current object moved %.2f units.",fabs(dist));
					World_changed = true;
					m_pParentFrame->UpdateAllPanes();
				}
				else
					OutrageMessageBox("Current object could not be moved.");
				break;

			case MODE_PATH:
				if (Editor_state.GetCurrentNodeType() == NODETYPE_GAME)
				{
					path = Editor_state.GetCurrentPath();
					node = Editor_state.GetCurrentNode();

					switch (m_nID)
					{
					case VIEW_XY:
						ret = ned_MoveNode(path,node,(float)dist,0.0f,0.0f);
						break;

					case VIEW_XZ:
						ret = ned_MoveNode(path,node,(float)dist,0.0f,0.0f);
						break;

					case VIEW_ZY:
						ret = ned_MoveNode(path,node,0.0f,0.0f,(float)dist);
						break;
					}
				}
				else
				{
					Editor_state.GetCurrentBNode(&room,&node,&edge);

					switch (m_nID)
					{
					case VIEW_XY:
						ret = ned_MoveBNode(room,node,(float)dist,0.0f,0.0f);
						break;

					case VIEW_XZ:
						ret = ned_MoveBNode(room,node,(float)dist,0.0f,0.0f);
						break;

					case VIEW_ZY:
						ret = ned_MoveBNode(room,node,0.0f,0.0f,(float)dist);
						break;
					}
				}

				if (ret)
				{
					PrintStatus("Current node moved %.2f units.",fabs(dist));
					World_changed = true;
					m_pParentFrame->UpdateAllPanes();
				}
				break;
			} // END OF switch (Editor_state.mode)
		}
		else
		{
			m_Cam.pos.x += grid_size;
			m_Cam.view_changed = true;
		}
		break;

	case VK_NUMPAD8:
	case VK_UP:
		if (m_bGridShow)
			dist = grid_size;
		else
			dist = 0.1f;

		if (HIWORD(::GetKeyState(VK_CONTROL)))
			MoveCursor(0,dist);
		else if (nChar == VK_NUMPAD8 || HIWORD(::GetKeyState(VK_SHIFT)))
		{
			switch (Editor_state.mode)
			{
			case MODE_VERTEX:
				// Get list of marked verts
				num_m_verts = m_pParentFrame->GetMarkedVerts(marked_vert_list);
				if (!num_m_verts)
					break;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,dist,0);
					break;
				case VIEW_XZ:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,0,dist);
					break;
				case VIEW_ZY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,dist,0);
					break;
				}
				if (ret)
				{
					PrintStatus("Marked vertices moved %.2f units.",fabs(dist));
					m_pParentFrame->m_Room_changed = true;
				}
				else
					OutrageMessageBox("Vertices could not be moved.");
				break;

			case MODE_FACE:
				// Get list of marked faces
				num_m_faces = m_pParentFrame->GetMarkedFaces(marked_face_list);
				if (!num_m_faces)
					break;

				// Determine which verts are in the marked faces
				for (i=0; i<num_m_faces; i++)
				{
					fp = &rp->faces[marked_face_list[i]];
					for (j=0; j<fp->num_verts; j++)
					{
						vert_idx = rp->faces[marked_face_list[i]].face_verts[j];
						if ( !verts_in_faces[vert_idx] )
							verts_in_faces[vert_idx] = true;
					}
				}
				// Fill the marked_vert_list array with all the verts in the marked faces
				memset(marked_vert_list,0,sizeof(short)*MAX_VERTS_PER_ROOM);
				for (j=0; j<rp->num_verts; j++)
					if ( verts_in_faces[j] )
						marked_vert_list[num_m_verts++] = j;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,dist,0);
					break;
				case VIEW_XZ:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,0,dist);
					break;
				case VIEW_ZY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,dist,0);
					break;
				}
				if (ret)
				{
					PrintStatus("Marked faces moved %.2f units.",fabs(dist));
					m_pParentFrame->m_Room_changed = true;
				}
				else
					OutrageMessageBox("Faces could not be moved.");
				break;

			case MODE_OBJECT:
				if (objp->roomnum != ROOMNUM(m_pPrim->roomp))
					break;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveObject(objp,objp->pos.x,objp->pos.y+dist,objp->pos.z);
					break;
				case VIEW_XZ:
					ret = MoveObject(objp,objp->pos.x,objp->pos.y,objp->pos.z+dist);
					break;
				case VIEW_ZY:
					ret = MoveObject(objp,objp->pos.x,objp->pos.y+dist,objp->pos.z);
					break;
				}
				if (ret)
				{
					PrintStatus("Current object moved %.2f units.",fabs(dist));
					World_changed = true;
					m_pParentFrame->UpdateAllPanes();
				}
				else
					PrintStatus("Current object could not be moved.");
				break;

			case MODE_PATH:
				if (Editor_state.GetCurrentNodeType() == NODETYPE_GAME)
				{
					path = Editor_state.GetCurrentPath();
					node = Editor_state.GetCurrentNode();

					switch (m_nID)
					{
					case VIEW_XY:
						ret = ned_MoveNode(path,node,0.0f,(float)dist,0.0f);
						break;

					case VIEW_XZ:
						ret = ned_MoveNode(path,node,0.0f,0.0f,(float)dist);
						break;

					case VIEW_ZY:
						ret = ned_MoveNode(path,node,0.0f,(float)dist,0.0f);
						break;
					}
				}
				else
				{
					Editor_state.GetCurrentBNode(&room,&node,&edge);

					switch (m_nID)
					{
					case VIEW_XY:
						ret = ned_MoveBNode(room,node,0.0f,(float)dist,0.0f);
						break;

					case VIEW_XZ:
						ret = ned_MoveBNode(room,node,0.0f,0.0f,(float)dist);
						break;

					case VIEW_ZY:
						ret = ned_MoveBNode(room,node,0.0f,(float)dist,0.0f);
						break;
					}
				}

				if (ret)
				{
					PrintStatus("Current node moved %.2f units.",fabs(dist));
					World_changed = true;
					m_pParentFrame->UpdateAllPanes();
				}
				break;
			} // END OF switch (Editor_state.mode)
		}
		else if (nChar == VK_UP && HIWORD(::GetKeyState(VK_CONTROL)))
		{
			// Go through portal to next room
			if (m_pPrim->portal != -1)
			{
				rp = &Rooms[rp->portals[m_pPrim->portal].croom];
				// Pick a portal in this room
				ASSERT(rp->num_portals);
				m_pParentFrame->SetPrim(rp,rp->portals[0].portal_face,0,0,0);
				// Update the current face/texture displays
				Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), m_pPrim->face);
				if (m_pParentFrame->m_bAutoCenter)
					m_pParentFrame->OnCenterRoom();
			}
		}
		else
		{
up:
			m_Cam.pos.y += grid_size;
			m_Cam.view_changed = true;
		}
		break;

	case VK_NUMPAD2:
	case VK_DOWN:
		if (m_bGridShow)
			dist = -grid_size;
		else
			dist = -0.1f;

		if (HIWORD(::GetKeyState(VK_CONTROL)))
			MoveCursor(0,dist);
		else if (nChar == VK_NUMPAD2 || HIWORD(::GetKeyState(VK_SHIFT)))
		{
			switch (Editor_state.mode)
			{
			case MODE_VERTEX:
				// Get list of marked verts
				num_m_verts = m_pParentFrame->GetMarkedVerts(marked_vert_list);
				if (!num_m_verts)
					break;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,dist,0);
					break;
				case VIEW_XZ:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,0,dist);
					break;
				case VIEW_ZY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,dist,0);
					break;
				}
				if (ret)
				{
					PrintStatus("Marked vertices moved %.2f units.",fabs(dist));
					m_pParentFrame->m_Room_changed = true;
				}
				else
					OutrageMessageBox("Vertices could not be moved.");
				break;

			case MODE_FACE:
				// Get list of marked faces
				num_m_faces = m_pParentFrame->GetMarkedFaces(marked_face_list);
				if (!num_m_faces)
					break;

				// Determine which verts are in the marked faces
				for (i=0; i<num_m_faces; i++)
				{
					fp = &rp->faces[marked_face_list[i]];
					for (j=0; j<fp->num_verts; j++)
					{
						vert_idx = rp->faces[marked_face_list[i]].face_verts[j];
						if ( !verts_in_faces[vert_idx] )
							verts_in_faces[vert_idx] = true;
					}
				}
				// Fill the marked_vert_list array with all the verts in the marked faces
				memset(marked_vert_list,0,sizeof(short)*MAX_VERTS_PER_ROOM);
				for (j=0; j<rp->num_verts; j++)
					if ( verts_in_faces[j] )
						marked_vert_list[num_m_verts++] = j;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,dist,0);
					break;
				case VIEW_XZ:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,0,dist);
					break;
				case VIEW_ZY:
					ret = MoveVerts(rp,marked_vert_list,num_m_verts,0,dist,0);
					break;
				}
				if (ret)
				{
					PrintStatus("Marked faces moved %.2f units.",fabs(dist));
					m_pParentFrame->m_Room_changed = true;
				}
				else
					OutrageMessageBox("Faces could not be moved.");
				break;

			case MODE_OBJECT:
				if (objp->roomnum != ROOMNUM(m_pPrim->roomp))
					break;

				switch (m_nID)
				{
				case VIEW_XY:
					ret = MoveObject(objp,objp->pos.x,objp->pos.y+dist,objp->pos.z);
					break;
				case VIEW_XZ:
					ret = MoveObject(objp,objp->pos.x,objp->pos.y,objp->pos.z+dist);
					break;
				case VIEW_ZY:
					ret = MoveObject(objp,objp->pos.x,objp->pos.y+dist,objp->pos.z);
					break;
				}
				if (ret)
				{
					PrintStatus("Current object moved %.2f units.",fabs(dist));
					World_changed = true;
					m_pParentFrame->UpdateAllPanes();
				}
				else
					OutrageMessageBox("Current object could not be moved.");
				break;

			case MODE_PATH:
				if (Editor_state.GetCurrentNodeType() == NODETYPE_GAME)
				{
					path = Editor_state.GetCurrentPath();
					node = Editor_state.GetCurrentNode();

					switch (m_nID)
					{
					case VIEW_XY:
						ret = ned_MoveNode(path,node,0.0f,(float)dist,0.0f);
						break;

					case VIEW_XZ:
						ret = ned_MoveNode(path,node,0.0f,0.0f,(float)dist);
						break;

					case VIEW_ZY:
						ret = ned_MoveNode(path,node,0.0f,(float)dist,0.0f);
						break;
					}
				}
				else
				{
					Editor_state.GetCurrentBNode(&room,&node,&edge);

					switch (m_nID)
					{
					case VIEW_XY:
						ret = ned_MoveBNode(room,node,0.0f,(float)dist,0.0f);
						break;

					case VIEW_XZ:
						ret = ned_MoveBNode(room,node,0.0f,0.0f,(float)dist);
						break;

					case VIEW_ZY:
						ret = ned_MoveBNode(room,node,0.0f,(float)dist,0.0f);
						break;
					}
				}

				if (ret)
				{
					PrintStatus("Current node moved %.2f units.",fabs(dist));
					World_changed = true;
					m_pParentFrame->UpdateAllPanes();
				}
				break;
			} // END OF switch (Editor_state.mode)
		}
		else
		{
down:
			m_Cam.pos.y -= grid_size;
			m_Cam.view_changed = true;
		}
		break;

	case VK_INSERT: // insert something where the user clicked
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
			if (Editor_state.mode == MODE_VERTEX)
			{
				// quick face insert from vert mode
				Editor_state.mode = MODE_FACE;
				m_pParentFrame->OnEditInsert();
				Editor_state.mode = MODE_VERTEX;
			}
			else if (Editor_state.mode == MODE_PATH)
			{
				// tell the room frame that we want to insert a path, not a node
				m_pParentFrame->m_bHandle_Nodes = false;
				m_pParentFrame->OnEditInsert();
			}
		}
		else
		{
			if (Editor_state.mode == MODE_PATH)
			{
				// tell the room frame that we want to insert a node, not a path
				m_pParentFrame->m_bHandle_Nodes = true;
				m_pParentFrame->OnEditInsert();
			}
			else
				m_pParentFrame->OnEditInsert();
		}
		break;

	case VK_DELETE:
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
			if (Editor_state.mode == MODE_PATH)
			{
				// tell the room frame that we want to delete a path, not a node
				m_pParentFrame->m_bHandle_Nodes = false;
			}
		}
		else
		{
			if (Editor_state.mode == MODE_PATH)
			{
				// tell the room frame that we want to delete a node, not a path
				m_pParentFrame->m_bHandle_Nodes = true;
			}
		}
		m_pParentFrame->OnEditDelete();
		break;

	case VK_NEXT:
		if (HIWORD(::GetKeyState(VK_SHIFT)))
			goto numpad3;
		break;

	case VK_PRIOR:
		if (HIWORD(::GetKeyState(VK_SHIFT)))
			goto numpad9;
		break;

	case VK_END:
		if (HIWORD(::GetKeyState(VK_SHIFT)))
			goto numpad1;
		break;

	case VK_HOME:
		if (HIWORD(::GetKeyState(VK_SHIFT)))
			goto numpad7;
		else
			CenterOrigin();
		break;

	case 0x43:				// 'C'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			CenterFace(rp,m_pPrim->face);
		else
			CenterRoom(rp);
		break;

	case 0x57:				// 'W'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
			// User pressed shift so wants to move view up -- so goto VK_UP handler
			goto up;
		}
		else
		{
			// User wants to zoom in
			m_Cam.scale *= m_ScaleStep;
			if (m_Cam.scale > O_MAX_SCALE)
				m_Cam.scale = O_MAX_SCALE;
			else
				m_Cam.view_changed = true;
		}
		break;

	case 0x53:				// 'S'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
			// User pressed shift so wants to move view down -- so goto VK_DOWN handler
			goto down;
		}
		else
		{
			// User wants to zoom out
			m_Cam.scale /= m_ScaleStep;
			if (m_Cam.scale < O_MIN_SCALE)
				m_Cam.scale = O_MIN_SCALE;
			else
				m_Cam.view_changed = true;
		}
		break;

	case 0x47:				// 'G'
		m_bGridShow = !m_bGridShow;
		m_State_changed = true;
		break;

	case 0x4D:				// 'M'
		if ( Editor_state.mode == MODE_VERTEX && HIWORD(::GetKeyState(VK_SHIFT)) )
			m_pParentFrame->MarkVertsInFace();
		else
			m_pParentFrame->OnMarkAll();
		break;

	case VK_SPACE:
		m_pParentFrame->OnMarkToggle();
		break;

	case 0x55:				// 'U'
		if ( Editor_state.mode == MODE_VERTEX && HIWORD(::GetKeyState(VK_SHIFT)) )
			m_pParentFrame->UnMarkVertsInFace();
		else
		m_pParentFrame->OnUnmarkAll();
		break;

	case 0x48:				// 'H'
		if (Editor_state.mode == MODE_PATH)
		{
			if ( HIWORD(::GetKeyState(VK_SHIFT)) )
				m_pParentFrame->CyclePath(PREV);
			else
				m_pParentFrame->CyclePath(NEXT);
		}
		break;

	case 0x49:				// 'I'
		m_pParentFrame->OnInvertMarkings();
		break;

	case 0x4E:				// 'N'
		if (Editor_state.mode != MODE_PATH)
		{
			// Get list of marked faces
			num_m_faces = m_pParentFrame->GetMarkedFaces(marked_face_list);
			ASSERT(num_m_faces == *m_pNum_marked_faces);
			bool change = false;
			// Flip the normals
			for (i=0; i<num_m_faces; i++)
			{
				if (rp->faces[marked_face_list[i]].portal_num == -1)
				{
					FlipFace(m_pPrim->roomp,marked_face_list[i]);
					change = true;
				}
				else
					MessageBox("Can't flip a portal face.");
			}
			if (change)
			{
				m_pParentFrame->m_Room_changed = true;
				MessageBox("Marked faces flipped.");
			}
		}
		else
		{
			if ( HIWORD(::GetKeyState(VK_SHIFT)) )
				m_pParentFrame->CycleNode(PREV);
			else
				m_pParentFrame->CycleNode(NEXT);
		}
		break;

	case 0x45:				// 'E'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			m_pParentFrame->CycleEdge(m_pPrim,PREV);
		else
			m_pParentFrame->CycleEdge(m_pPrim,NEXT);
		break;

	case 0x46:				// 'F'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			m_pParentFrame->CycleFace(m_pPrim,PREV);
		else
			m_pParentFrame->CycleFace(m_pPrim,NEXT);
		if (m_pParentFrame->m_bAutoCenter)
			m_pParentFrame->OnCenterFace();
		break;

	case 0x50:				// 'P'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			m_pParentFrame->CyclePortal(m_pPrim,PREV);
		else
			m_pParentFrame->CyclePortal(m_pPrim,NEXT);
		break;

	case 0x52:				// 'R'
		if (m_pParentFrame == theApp.m_pRoomFrm) // only allow for main room frame
		{
			m_pParentFrame->StopCameras();
			do {
				if ( HIWORD(::GetKeyState(VK_SHIFT)) )
					m_pParentFrame->CycleRoom(m_pPrim,PREV);
				else
					m_pParentFrame->CycleRoom(m_pPrim,NEXT);
			} while (!m_pPrim->roomp->used);
			if (m_pParentFrame->m_bAutoCenter)
				m_pParentFrame->OnCenterRoom();
		}
		break;

	case 0x56:				// 'V'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			m_pParentFrame->CycleVert(m_pPrim,PREV);
		else
			m_pParentFrame->CycleVert(m_pPrim,NEXT);
		break;

	case 0x4F:				// 'O'
		if (m_pParentFrame == theApp.m_pRoomFrm) // only allow for main room frame
		{
			if ( HIWORD(::GetKeyState(VK_SHIFT)) )
				m_pParentFrame->CycleObject(m_pPrim,PREV);
			else
				m_pParentFrame->CycleObject(m_pPrim,NEXT);
		}
		break;

	case VK_F8:
		// TODO : put this in the UI
		if (m_pParentFrame == theApp.m_pRoomFrm) // only allow for main room frame
		{
			if ( HIWORD(::GetKeyState(VK_CONTROL)) && HIWORD(::GetKeyState(VK_SHIFT)) )
				DoManualPathPointSet(rp,m_pvec_InsertPos);
		}
		break;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void Cned_OrthoWnd::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	CWnd::OnSysChar(nChar, nRepCnt, nFlags);
}

void Cned_OrthoWnd::OnTimer(UINT nIDEvent) 
{
	//Do our thing here
	int dx,dy;

	if (m_Cam.view_changed || m_State_changed) 
	{
		m_State_changed = m_Cam.view_changed = false;
		InvalidateRect(NULL, FALSE);
	}

	//Only process input when we have the focus
	if(m_InFocus)
	{
		dx = m_Mouse.x-m_Mouse.oldx;
		dy = m_Mouse.y-m_Mouse.oldy;

		if ( (dx!=0) || (dy!=0) ) {
			if (PANNING || ZOOMING)
			{
				if (!m_bCaptured)
				{
					SetCapture();
					m_bCaptured = true;
				}
				if ( PANNING ) {
					m_Cam.pos.x -= dx;
					m_Cam.pos.y += dy;
					m_View_changed = true;
				}
				else if ( ZOOMING ) {
					float delta_y = (float)dy;
					m_Cam.scale *= 1-delta_y/10;
					if (m_Cam.scale > O_MAX_SCALE)
						m_Cam.scale = O_MAX_SCALE;
					else if (m_Cam.scale < O_MIN_SCALE)
						m_Cam.scale = O_MIN_SCALE;
					else
						m_View_changed = true;
				}
			}
			else
			{
				if (m_bCaptured)
				{
					ReleaseCapture();
					m_bCaptured = false;
				}
			}
		}

		// If there was mouse input, update the view immediately
		if (m_View_changed) {
			m_View_changed = false;
			InvalidateRect(NULL, FALSE);
		}
	}

	// Save mouse position
	m_Mouse.oldx = m_Mouse.x;
	m_Mouse.oldy = m_Mouse.y;

	if (m_Cam.moving)
	{
		POINT pos = {m_Cam.pos.x+m_Cam.step.x, m_Cam.pos.y+m_Cam.step.y};
		SetCameraPos(pos);
		if ( abs(m_Cam.pos.x-m_Cam.destpos.x) <= abs(m_Cam.step.x) )
			m_Cam.step.x = 0;
		if ( abs(m_Cam.pos.y-m_Cam.destpos.y) <= abs(m_Cam.step.y) )
			m_Cam.step.y = 0;
		if (m_Cam.step.x == 0 && m_Cam.step.y == 0)
		{
			SetCameraPos(m_Cam.destpos);
			m_Cam.moving = false;
		}
	}

	CWnd::OnTimer(nIDEvent);
}

void Cned_OrthoWnd::SetCameraPos(POINT pos)
{
	m_Cam.pos.x = pos.x;
	m_Cam.pos.y = pos.y;
	m_Cam.view_changed = true;
}

void Cned_OrthoWnd::OnSetFocus(CWnd* pOldWnd) 
{
	CWnd::OnSetFocus(pOldWnd);
	
	if(gCameraSlewer)
		gCameraSlewer->AttachOrthoCamera(&m_Cam,(char *)LPCSTR(m_Title));

	m_pParentFrame->m_Focused_pane = m_nID;

	m_InFocus = true;

	// Update the text color (ok, so it's a hack)
	RECT rc = {0,0,100,50};
	InvalidateRect(&rc);
}

void Cned_OrthoWnd::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);
	memset(&m_Mouse,0,sizeof(m_Mouse));
	memset(&m_Keys,0,sizeof(m_Keys));

	m_InFocus = false;
	
	// Update the text color
	RECT rc = {0,0,100,50};
	InvalidateRect(&rc);
}

void Cned_OrthoWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	switch(nChar)
	{
	case VK_MENU:
		m_Keys.alt = true;
		break;
	default:
		CWnd ::OnSysKeyDown(nChar, nRepCnt, nFlags);
	}
	
}

void Cned_OrthoWnd::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	
	switch(nChar)
	{
	case VK_MENU:
		m_Keys.alt = false;
		break;
	default:
		CWnd ::OnSysKeyUp(nChar, nRepCnt, nFlags);
	}
	
}


/////////////////////////////////////////////////////////////////////////////
// Cned_OrthoWnd funcs

void Cned_OrthoWnd::InitCamera()
{
	m_Cam.scale = 0.3f;
	m_Cam.moving = false;

	CenterRoom(m_pPrim->roomp);
}


void Cned_OrthoWnd::OnPaint() 
{
	CPaintDC paint_dc(this); // device context for painting

	// TODO: Add your message handler code here
	CRect	rc;

	GetClientRect(&rc);
	// Make sure the window has an area
	if (! (rc.Width() && rc.Height()) )
		return;

	CDC *dc = (CDC *) &paint_dc;
	CMemDC	pDC(dc, rc);

	// Paint the background
	pDC->FillSolidRect(&rc, m_BackColor);

	// Set up mapping mode and coordinate system
	pDC->SetMapMode(MM_ISOTROPIC);
	CSize oldWExt = pDC->SetWindowExt(rc.Width()/m_Cam.scale,rc.Height()/m_Cam.scale);
	CPoint oldWOrg = pDC->SetWindowOrg(WOS*m_Cam.pos.x,WOS*m_Cam.pos.y);
	CSize oldVExt = pDC->SetViewportExt(rc.Width(), -rc.Height());
	CPoint oldVOrg = pDC->SetViewportOrg(rc.Width()/2, rc.Height()/2);

	int grid_size;
	(m_nGrid == NUM_GRIDS-1) ? (grid_size = m_CustomGrid) : (grid_size = Grids[m_nGrid]);

	// Draw grid lines
	if (m_bGridShow)
	{
		DrawOrthoGrid(pDC,grid_size);
		DrawOrthoRefFrame(pDC);
	}

	// Draw the world
	DrawOrthoWorld(pDC);

	// Print the window title
	pDC->SetMapMode(MM_TEXT);
	pDC->SetWindowExt(oldWExt);
	pDC->SetWindowOrg(oldWOrg);
	pDC->SetViewportExt(oldVExt);
	pDC->SetViewportOrg(oldVOrg);
	if (m_InFocus)
	{
		ASSERT(m_nID == m_pParentFrame->m_Focused_pane);
		pDC->SetTextColor(RGB(255,255,255));
	}
	else
		pDC->SetTextColor(RGB(0,0,0));
	pDC->TextOut(10,10,m_Title,lstrlen(m_Title));

	// Do not call CWnd::OnPaint() for painting messages
}


void Cned_OrthoWnd::DrawOrthoGrid(CMemDC *pDC,int size)
{
	CSize WExt = pDC->GetWindowExt();
	CPoint WOrg = pDC->GetWindowOrg();
	CSize VExt = pDC->GetViewportExt();
	CPoint VOrg = pDC->GetViewportOrg();
	CPoint pt;
	CPen *pOldPen;
	int i,j;
	RECT rc = {WOrg.x-WExt.cx/2, WOrg.y-WExt.cy/2, WOrg.x+WExt.cx/2, WOrg.y+WExt.cy/2};
	const int s_fact = WOS*size;

	// Select pen
	pOldPen = pDC->SelectObject(&g_pens[O_ORIGINLINE_COLOR]);

	// Draw origin grid lines
	POINT top_pt = {0,rc.top};
	POINT bottom_pt = {0,rc.bottom};
	POINT left_pt = {rc.left,0};
	POINT right_pt = {rc.right,0};

	pDC->LPtoDP(&top_pt); pDC->LPtoDP(&bottom_pt);
	pDC->LPtoDP(&left_pt); pDC->LPtoDP(&right_pt);
	int saved = pDC->SaveDC();
	pDC->SetMapMode(MM_TEXT);
	pDC->DPtoLP(&top_pt); pDC->DPtoLP(&bottom_pt);
	pDC->DPtoLP(&left_pt); pDC->DPtoLP(&right_pt);

	if (rc.left < 0 && 0 < rc.right) {
		pDC->MoveTo(top_pt);
		pDC->LineTo(bottom_pt);
	}
	if (rc.top < 0 && 0 < rc.bottom) {
		pDC->MoveTo(left_pt);
		pDC->LineTo(right_pt);
	}

	pDC->RestoreDC(saved);

	// Unselect pen
	pDC->SelectObject(&pOldPen);

	// Don't draw any lines if they'd be too close to each other (i.e. too dense)
	POINT test_pt1 = {rc.left,0};
	pDC->LPtoDP(&test_pt1);
	POINT test_pt2 = {rc.left+s_fact,0};
	pDC->LPtoDP(&test_pt2);
	if ((test_pt2.x-test_pt1.x) < 5)
		return;

	// Select another pen
	pOldPen = pDC->SelectObject(&g_pens[O_GRIDLINE_COLOR]);

	// Draw vertical grid lines
	for (i=rc.left; i<WOrg.x+size; i++) {
		pt.x = i; pt.y = rc.top;
		// Draw first line
		if ( !(pt.x % s_fact) ) {
			pDC->MoveTo(pt);
			pt.y = rc.bottom;
			pDC->LineTo(pt);
			// Draw the rest of the lines
			for (j=i+s_fact; j<rc.right; j+=s_fact) {
				pt.x = j; pt.y = rc.top;
				pDC->MoveTo(pt);
				pt.y = rc.bottom;
				pDC->LineTo(pt);
			}
			break;
		}
	}

	// Draw horizontal grid lines
	for (i=rc.top; i<WOrg.y+size; i++) {
		pt.x = rc.left; pt.y = i;
		// Draw first line
		if ( !(pt.y % s_fact) ) {
			pDC->MoveTo(pt);
			pt.x = rc.right;
			pDC->LineTo(pt);
			// Draw the rest of the lines
			for (j=i+s_fact; j<rc.bottom; j+=s_fact) {
				pt.x = rc.left; pt.y = j;
				pDC->MoveTo(pt);
				pt.x = rc.right;
				pDC->LineTo(pt);
			}
			break;
		}
	}

	// Unselect pen
	pDC->SelectObject(&pOldPen);

	// Select another pen
	pOldPen = pDC->SelectObject(&g_pens[O_BR_GRIDLINE_COLOR]);

	// Draw major vertical grid lines
	for (i=rc.left; i<WOrg.x+50*size; i++) {
		pt.x = i; pt.y = rc.top;
		// Draw first line
		if ( !(pt.x % (5*s_fact)) ) {
			pDC->MoveTo(pt);
			pt.y = rc.bottom;
			pDC->LineTo(pt);
			// Draw the rest of the lines
			for (j=i+5*s_fact; j<rc.right; j+=5*s_fact) {
				pt.x = j; pt.y = rc.top;
				pDC->MoveTo(pt);
				pt.y = rc.bottom;
				pDC->LineTo(pt);
			}
			break;
		}
	}

	// Draw major horizontal grid lines
	for (i=rc.top; i<WOrg.y+50*size; i++) {
		pt.x = rc.left; pt.y = i;
		// Draw first line
		if ( !(pt.y % (5*s_fact)) ) {
			pDC->MoveTo(pt);
			pt.x = rc.right;
			pDC->LineTo(pt);
			// Draw the rest of the lines
			for (j=i+5*s_fact; j<rc.bottom; j+=5*s_fact) {
				pt.x = rc.left; pt.y = j;
				pDC->MoveTo(pt);
				pt.x = rc.right;
				pDC->LineTo(pt);
			}
			break;
		}
	}

	// Unselect pen
	pDC->SelectObject(&pOldPen);
}


void Cned_OrthoWnd::DrawOrthoRefFrame(CMemDC *pDC)
{
	CSize WExt = pDC->GetWindowExt();
	CPoint WOrg = pDC->GetWindowOrg();
	CSize VExt = pDC->GetViewportExt();
	CPoint VOrg = pDC->GetViewportOrg();
	CPoint pt;
	vec2D pos;
	RECT rc = {WOrg.x-WExt.cx/2, WOrg.y-WExt.cy/2, WOrg.x+WExt.cx/2, WOrg.y+WExt.cy/2};

	// Select pen
	CPen *pOldPen = pDC->SelectObject(&g_pens[O_REFFRAME_COLOR]);

	switch (m_nID)
	{
	case VIEW_XY:
		pos.x = WOS*m_pvec_RefPos->x;
		pos.y = WOS*m_pvec_RefPos->y;
		break;

	case VIEW_XZ:
		pos.x = WOS*m_pvec_RefPos->x;
		pos.y = WOS*m_pvec_RefPos->z;
		break;

	case VIEW_ZY:
		pos.x = WOS*m_pvec_RefPos->z;
		pos.y = WOS*m_pvec_RefPos->y;
		break;
	}

	pt.x = Round(pos.x);
	pt.y = Round(pos.y);

	// Draw reference frame lines
	if (rc.left < pt.x && pt.x < rc.right) {
		pDC->MoveTo(pt.x,rc.top);
		pDC->LineTo(pt.x,rc.bottom);
	}
	if (rc.top < pt.y && pt.y < rc.bottom) {
		pDC->MoveTo(rc.left,pt.y);
		pDC->LineTo(rc.right,pt.y);
	}

	// Unselect pen
	pDC->SelectObject(&pOldPen);
}


void Cned_OrthoWnd::DrawOrthoWorld(CMemDC *pDC)
{
	room *rp = m_pPrim->roomp;
	object *objp;
	int num_objects;

	// Draw objects in room
	if (m_bShowObjects && rp->objects != -1 && m_pParentFrame == theApp.m_pRoomFrm)
	{
		num_objects = GetNumObjectsInRoom(rp);
		objp = &Objects[rp->objects];
		DrawOrthoObjects(pDC,rp,objp,num_objects);
	}

	// Draw attached rooms before current room
	if (m_bShowAttached)
		for (int i=0; i<rp->num_portals; i++)
			DrawOrthoRoom(pDC,&Rooms[rp->portals[i].croom],O_ROOM_COLOR);

	// Draw current room
	DrawOrthoRoom(pDC,rp,O_CUR_ROOM_COLOR);

	// Draw paths
	if (m_pParentFrame == theApp.m_pRoomFrm)
	{
		DrawOrthoGamePaths(pDC,rp,O_GAMEPATH_COLOR,O_NODE_COLOR);
		DrawOrthoAIPaths(pDC,rp,O_AIPATH_COLOR,O_BNODE_COLOR);
	}

	// Draw verts
	if (m_bShowVerts)
		DrawVerts(pDC,rp->verts,rp->num_verts,O_VERT_COLOR);

	// Draw marked primitives
	DrawMarkedPrims(pDC);

	// Draw current primitives
	DrawCurrentPrims(pDC);

	// Draw the room center
	if (m_bShowCenters)
		DrawRoomCenter(pDC,rp,O_ROOMCENTER_COLOR);

	// Draw the insert cursor
	DrawCursor(pDC);
}

//Colors for objects
#define	ROBOT_COLOR			RGB( 255,   0,   0)		//a robot
#define	PLAYER_COLOR		RGB(   0, 255,   0)		//a player object
#define  POWERUP_COLOR		RGB(   0,   0, 255)		//a powerup
#define	MISCOBJ_COLOR		RGB(   0, 100, 100)		//some other object


void Cned_OrthoWnd::DrawOrthoObjects(CMemDC *pDC,room *rp,object *obj,int num_objects)
{
	RECT rect;
	CBrush brush, *pOldBrush = NULL;
	CPen pen, *pOldPen = NULL;
	COLORREF oldcolor = RGB(255,255,255);
	COLORREF color = RGB(0,0,0);
	bool bDrawOrient = false;

	for (int i=0; i<=num_objects; i++,obj=&Objects[obj->next])
	{
		// Don't draw objects of certain types
		if (obj->type==OBJ_DOOR || obj->type==OBJ_VIEWER || obj->type==OBJ_CAMERA)
			continue;

  		switch (obj->type) {
  			case OBJ_POWERUP:	color = POWERUP_COLOR; bDrawOrient = true; break;
  			case OBJ_ROBOT:	color = ROBOT_COLOR; bDrawOrient = true; break;
  			case OBJ_PLAYER:	color = PLAYER_COLOR; bDrawOrient = true; break;
  			default:				color = MISCOBJ_COLOR; break;
  		}

		if (color != oldcolor)
		{
			if (pOldBrush != NULL && pOldPen != NULL)
			{
				// Unselect brush
				pDC->SelectObject(pOldBrush);
				// Delete brush
				brush.DeleteObject();
				// Unselect pen
				pDC->SelectObject(pOldPen);
				// Delete pen
				pen.DeleteObject();
			}
			// Create brush
			brush.CreateSolidBrush(color);
			// Select brush
			pOldBrush = pDC->SelectObject(&brush);
			// Create pen
			pen.CreatePen(PS_SOLID,0,color);
			// Select pen
			pOldPen = pDC->SelectObject(&pen);
		}

		switch (m_nID)
		{
		case VIEW_XY:
			rect.left = Round(obj->pos.x - obj->size);
			rect.right = Round(obj->pos.x + obj->size);
			rect.top = Round(obj->pos.y + obj->size);
			rect.bottom = Round(obj->pos.y - obj->size);
			break;

		case VIEW_XZ:
			rect.left = Round(obj->pos.x - obj->size);
			rect.right = Round(obj->pos.x + obj->size);
			rect.top = Round(obj->pos.z + obj->size);
			rect.bottom = Round(obj->pos.z - obj->size);
			break;

		case VIEW_ZY:
			rect.left = Round(obj->pos.z - obj->size);
			rect.right = Round(obj->pos.z + obj->size);
			rect.top = Round(obj->pos.y + obj->size);
			rect.bottom = Round(obj->pos.y - obj->size);
			break;
		}

		// Convert to logical coordinates
		rect.top *= WOS;
		rect.bottom *= WOS;
		rect.left *= WOS;
		rect.right *= WOS;

		// Draw sphere
		pDC->Ellipse(&rect);

		if (bDrawOrient)
		{

		POINT begin_up,end_up,begin_front,end_front,arrow_left,arrow_right;

		switch (m_nID)
		{
		case VIEW_XY:
			begin_up.x = Round(obj->pos.x+obj->size*obj->orient.uvec.x);
			begin_up.y = Round(obj->pos.y+obj->size*obj->orient.uvec.y);
			end_up.x = Round(obj->pos.x+2*obj->size*obj->orient.uvec.x);
			end_up.y = Round(obj->pos.y+2*obj->size*obj->orient.uvec.y);
			begin_front.x = Round(obj->pos.x+obj->size*obj->orient.fvec.x);
			begin_front.y = Round(obj->pos.y+obj->size*obj->orient.fvec.y);
			end_front.x = Round(obj->pos.x+2*obj->size*obj->orient.fvec.x);
			end_front.y = Round(obj->pos.y+2*obj->size*obj->orient.fvec.y);
			arrow_left.x = Round(end_front.x - 0.25*obj->size*(obj->orient.rvec.x + obj->orient.fvec.x));
			arrow_left.y = Round(end_front.y - 0.25*obj->size*(obj->orient.rvec.y + obj->orient.fvec.y));
			arrow_right.x = Round(end_front.x + 0.25*obj->size*(obj->orient.rvec.x - obj->orient.fvec.x));
			arrow_right.y = Round(end_front.y + 0.25*obj->size*(obj->orient.rvec.y - obj->orient.fvec.y));
			break;

		case VIEW_XZ:
			begin_up.x = Round(obj->pos.x+obj->size*obj->orient.uvec.x);
			begin_up.y = Round(obj->pos.z+obj->size*obj->orient.uvec.z);
			end_up.x = Round(obj->pos.x+2*obj->size*obj->orient.uvec.x);
			end_up.y = Round(obj->pos.z+2*obj->size*obj->orient.uvec.z);
			begin_front.x = Round(obj->pos.x+obj->size*obj->orient.fvec.x);
			begin_front.y = Round(obj->pos.z+obj->size*obj->orient.fvec.z);
			end_front.x = Round(obj->pos.x+2*obj->size*obj->orient.fvec.x);
			end_front.y = Round(obj->pos.z+2*obj->size*obj->orient.fvec.z);
			arrow_left.x = Round(end_front.x - 0.25*obj->size*(obj->orient.rvec.x + obj->orient.fvec.x));
			arrow_left.y = Round(end_front.y - 0.25*obj->size*(obj->orient.rvec.z + obj->orient.fvec.z));
			arrow_right.x = Round(end_front.x + 0.25*obj->size*(obj->orient.rvec.x - obj->orient.fvec.x));
			arrow_right.y = Round(end_front.y + 0.25*obj->size*(obj->orient.rvec.z - obj->orient.fvec.z));
			break;

		case VIEW_ZY:
			begin_up.x = Round(obj->pos.z+obj->size*obj->orient.uvec.z);
			begin_up.y = Round(obj->pos.y+obj->size*obj->orient.uvec.y);
			end_up.x = Round(obj->pos.z+2*obj->size*obj->orient.uvec.z);
			end_up.y = Round(obj->pos.y+2*obj->size*obj->orient.uvec.y);
			begin_front.x = Round(obj->pos.z+obj->size*obj->orient.fvec.z);
			begin_front.y = Round(obj->pos.y+obj->size*obj->orient.fvec.y);
			end_front.x = Round(obj->pos.z+2*obj->size*obj->orient.fvec.z);
			end_front.y = Round(obj->pos.y+2*obj->size*obj->orient.fvec.y);
			arrow_left.x = Round(end_front.x - 0.25*obj->size*(obj->orient.rvec.z + obj->orient.fvec.z));
			arrow_left.y = Round(end_front.y - 0.25*obj->size*(obj->orient.rvec.y + obj->orient.fvec.y));
			arrow_right.x = Round(end_front.x + 0.25*obj->size*(obj->orient.rvec.z - obj->orient.fvec.z));
			arrow_right.y = Round(end_front.y + 0.25*obj->size*(obj->orient.rvec.y - obj->orient.fvec.y));
			break;
		}

		// Convert to logical coordinates
		begin_up.x *= WOS;
		begin_up.y *= WOS;
		end_up.x *= WOS;
		end_up.y *= WOS;
		begin_front.x *= WOS;
		begin_front.y *= WOS;
		end_front.x *= WOS;
		end_front.y *= WOS;
		arrow_left.x *= WOS;
		arrow_left.y *= WOS;
		arrow_right.x *= WOS;
		arrow_right.y *= WOS;

		// Draw up vector
		pDC->MoveTo(begin_up);
		pDC->LineTo(end_up);
		// Draw forward vector
		pDC->MoveTo(begin_front);
		pDC->LineTo(end_front);
		// Draw arrow head for forward vector
		pDC->MoveTo(arrow_left);
		pDC->LineTo(end_front);
		pDC->LineTo(arrow_right);

		}

		// Save color so that we know whether or not to make a new brush
		oldcolor = color;
	}

	// Unselect brush
	pDC->SelectObject(pOldBrush);
	// Unselect pen
	pDC->SelectObject(pOldPen);

}


void Cned_OrthoWnd::DrawRoomCenter(CMemDC *pDC,room *rp,const int color)
{
	int size = 5; // mark size;
	vector center;
	vec2D pos;
	POINT pt;

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	ComputeRoomBoundingSphere(&center,rp); // ComputeRoomCenter

	// Draw a mark at the room center
	switch (m_nID)
	{
	case VIEW_XY:
		// Position pen center of room
		pos.x = WOS*center.x;
		pos.y = WOS*center.y;
		break;

	case VIEW_XZ:
		// Position pen center of room
		pos.x = WOS*center.x;
		pos.y = WOS*center.z;
		break;

	case VIEW_ZY:
		// Position pen center of room
		pos.x = WOS*center.z;
		pos.y = WOS*center.y;
		break;
	}

	pt.x = Round(pos.x);
	pt.y = Round(pos.y);

	// Draw the room center mark
	pDC->LPtoDP(&pt);
	int saved = pDC->SaveDC();
	pDC->SetMapMode(MM_TEXT);
	pDC->DPtoLP(&pt);
	pDC->MoveTo(pt.x-size,pt.y);
	pDC->LineTo(pt.x+size+1,pt.y);
	pDC->MoveTo(pt.x,pt.y+size);
	pDC->LineTo(pt.x,pt.y-size-1);
	pDC->RestoreDC(saved);

	// Unselect pen
	pDC->SelectObject(pOldPen);
}


void Cned_OrthoWnd::DrawOrthoRoom(CMemDC *pDC,room *rp,const int color)
{
	int fn,vn;
	face *fp = rp->faces;
	vec2D pos;

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	// Draw the room
	switch (m_nID)
	{
	case VIEW_XY:
		for (fn=0,fp=rp->faces;fn<rp->num_faces;fn++,fp++)
		{
			// Position pen at first vertex
			pos.x = WOS*rp->verts[fp->face_verts[0]].x;
			pos.y = WOS*rp->verts[fp->face_verts[0]].y;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw edges
			for (vn=1;vn<fp->num_verts;vn++)
			{
				pos.x = WOS*rp->verts[fp->face_verts[vn]].x;
				pos.y = WOS*rp->verts[fp->face_verts[vn]].y;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
			// Draw last edge
			pos.x = WOS*rp->verts[fp->face_verts[0]].x;
			pos.y = WOS*rp->verts[fp->face_verts[0]].y;
			pDC->LineTo(Round(pos.x), Round(pos.y));
		}
		break;

	case VIEW_XZ:
		for (fn=0,fp=rp->faces;fn<rp->num_faces;fn++,fp++)
		{
			// Position pen at first vertex
			pos.x = WOS*rp->verts[fp->face_verts[0]].x;
			pos.y = WOS*rp->verts[fp->face_verts[0]].z;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw edges
			for (vn=1;vn<fp->num_verts;vn++)
			{
				pos.x = WOS*rp->verts[fp->face_verts[vn]].x;
				pos.y = WOS*rp->verts[fp->face_verts[vn]].z;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
			// Draw last edge
			pos.x = WOS*rp->verts[fp->face_verts[0]].x;
			pos.y = WOS*rp->verts[fp->face_verts[0]].z;
			pDC->LineTo(Round(pos.x), Round(pos.y));
		}
		break;

	case VIEW_ZY:
		for (fn=0,fp=rp->faces;fn<rp->num_faces;fn++,fp++)
		{
			// Position pen at first vertex
			pos.x = WOS*rp->verts[fp->face_verts[0]].z;
			pos.y = WOS*rp->verts[fp->face_verts[0]].y;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw edges
			for (vn=1;vn<fp->num_verts;vn++)
			{
				pos.x = WOS*rp->verts[fp->face_verts[vn]].z;
				pos.y = WOS*rp->verts[fp->face_verts[vn]].y;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
			// Draw last edge
			pos.x = WOS*rp->verts[fp->face_verts[0]].z;
			pos.y = WOS*rp->verts[fp->face_verts[0]].y;
			pDC->LineTo(Round(pos.x), Round(pos.y));
		}
		break;
	}

	// Unselect pen
	pDC->SelectObject(pOldPen);

}


void Cned_OrthoWnd::DrawOrthoGamePaths(CMemDC *pDC,room *rp,const int color,const int br_color)
{
	vec2D pos;
	game_path *gp;
	RECT rect;
	int size = 1;
	int i,j;

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	// Select brush
	ASSERT(br_color >=0 && br_color < NUM_BRUSHES);
	CBrush *pOldBrush = pDC->SelectObject(&g_brushes[br_color]);

	int current_path_index = GetFirstPath();

	// Draw the game paths
	switch (m_nID)
	{
	case VIEW_XY:
		for (i=0; i<Num_game_paths; i++,current_path_index=GetNextPath(current_path_index))
		{
			gp = &GamePaths[current_path_index];
			// Position pen at first node
			pos.x = WOS*gp->pathnodes[0].pos.x;
			pos.y = WOS*gp->pathnodes[0].pos.y;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw game path segments
			for (j=1; j<gp->num_nodes; j++)
			{
				pos.x = WOS*gp->pathnodes[j].pos.x;
				pos.y = WOS*gp->pathnodes[j].pos.y;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
			// Draw nodes
			for (j=0; j<gp->num_nodes; j++)
			{
				rect.left = WOS*Round(gp->pathnodes[j].pos.x - size);
				rect.right = WOS*Round(gp->pathnodes[j].pos.x + size);
				rect.top = WOS*Round(gp->pathnodes[j].pos.y + size);
				rect.bottom = WOS*Round(gp->pathnodes[j].pos.y - size);
				// Draw sphere
				pDC->Ellipse(&rect);
				// Draw node number
				pos.x = WOS*Round(gp->pathnodes[j].pos.x);
				pos.y = WOS*Round(gp->pathnodes[j].pos.y);
				DrawOrthoNumber(pDC,j+1,pos,size,O_BR_GRIDLINE_COLOR);
			}
		}
		break;

	case VIEW_XZ:
		for (i=0; i<Num_game_paths; i++,current_path_index=GetNextPath(current_path_index))
		{
			gp = &GamePaths[current_path_index];
			// Position pen at first node
			pos.x = WOS*gp->pathnodes[0].pos.x;
			pos.y = WOS*gp->pathnodes[0].pos.z;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw game path segments
			for (j=1; j<gp->num_nodes; j++)
			{
				pos.x = WOS*gp->pathnodes[j].pos.x;
				pos.y = WOS*gp->pathnodes[j].pos.z;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
			// Draw nodes
			for (j=0; j<gp->num_nodes; j++)
			{
				rect.left = WOS*Round(gp->pathnodes[j].pos.x - size);
				rect.right = WOS*Round(gp->pathnodes[j].pos.x + size);
				rect.top = WOS*Round(gp->pathnodes[j].pos.z + size);
				rect.bottom = WOS*Round(gp->pathnodes[j].pos.z - size);
				// Draw sphere
				pDC->Ellipse(&rect);
				// Draw node number
				pos.x = WOS*Round(gp->pathnodes[j].pos.x);
				pos.y = WOS*Round(gp->pathnodes[j].pos.z);
				DrawOrthoNumber(pDC,j+1,pos,size,O_BR_GRIDLINE_COLOR);
			}
		}
		break;

	case VIEW_ZY:
		for (i=0; i<Num_game_paths; i++,current_path_index=GetNextPath(current_path_index))
		{
			gp = &GamePaths[current_path_index];
			// Position pen at first node
			pos.x = WOS*gp->pathnodes[0].pos.z;
			pos.y = WOS*gp->pathnodes[0].pos.y;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw game path segments
			for (j=1; j<gp->num_nodes; j++)
			{
				pos.x = WOS*gp->pathnodes[j].pos.z;
				pos.y = WOS*gp->pathnodes[j].pos.y;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
			// Draw nodes
			for (j=0; j<gp->num_nodes; j++)
			{
				rect.left = WOS*Round(gp->pathnodes[j].pos.z - size);
				rect.right = WOS*Round(gp->pathnodes[j].pos.z + size);
				rect.top = WOS*Round(gp->pathnodes[j].pos.y + size);
				rect.bottom = WOS*Round(gp->pathnodes[j].pos.y - size);
				// Draw sphere
				pDC->Ellipse(&rect);
				// Draw node number
				pos.x = WOS*Round(gp->pathnodes[j].pos.z);
				pos.y = WOS*Round(gp->pathnodes[j].pos.y);
				DrawOrthoNumber(pDC,j+1,pos,size,O_BR_GRIDLINE_COLOR);
			}
		}
		break;
	}

	// Unselect brush
	pDC->SelectObject(pOldBrush);
	// Unselect pen
	pDC->SelectObject(pOldPen);

}


void Cned_OrthoWnd::DrawOrthoAIPaths(CMemDC *pDC,room *rp,const int color,const int br_color)
{
	vec2D pos;
	RECT rect;
	int size = 1;
	int i,j;

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	// Select brush
	ASSERT(br_color >=0 && br_color < NUM_BRUSHES);
	CBrush *pOldBrush = pDC->SelectObject(&g_brushes[br_color]);

	bn_list *bn_info = BNode_GetBNListPtr(ROOMNUM(rp));
	bn_list *ebn_info;

	// Draw the AI paths
	switch (m_nID)
	{
	case VIEW_XY:
		for (i=0; i<bn_info->num_nodes; i++)
		{
			// Position pen at first edge of Bnode
			if (!bn_info->nodes[i].num_edges)
				break;
			ebn_info = BNode_GetBNListPtr(bn_info->nodes[i].edges[0].end_room);
			pos.x = WOS*ebn_info->nodes[bn_info->nodes[i].edges[0].end_index].pos.x;
			pos.y = WOS*ebn_info->nodes[bn_info->nodes[i].edges[0].end_index].pos.y;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw AI path segments
			for(j=1; j<bn_info->nodes[i].num_edges; j++)
			{
				ebn_info = BNode_GetBNListPtr(bn_info->nodes[i].edges[j].end_room);
				pos.x = WOS*ebn_info->nodes[bn_info->nodes[i].edges[j].end_index].pos.x;
				pos.y = WOS*ebn_info->nodes[bn_info->nodes[i].edges[j].end_index].pos.y;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
		}
		// Draw Bnodes
		for (i=0; i<bn_info->num_nodes; i++)
		{
			rect.left = WOS*Round(bn_info->nodes[i].pos.x - size);
			rect.right = WOS*Round(bn_info->nodes[i].pos.x + size);
			rect.top = WOS*Round(bn_info->nodes[i].pos.y + size);
			rect.bottom = WOS*Round(bn_info->nodes[i].pos.y - size);
			// Draw sphere
			pDC->Ellipse(&rect);
			// Draw Bnode number
			pos.x = WOS*bn_info->nodes[i].pos.x;
			pos.y = WOS*bn_info->nodes[i].pos.y;
			DrawOrthoNumber(pDC,i+1,pos,size,O_BR_GRIDLINE_COLOR);
		}
		break;

	case VIEW_XZ:
		for (i=0; i<bn_info->num_nodes; i++)
		{
			// Position pen at first edge of Bnode
			if (!bn_info->nodes[i].num_edges)
				break;
			ebn_info = BNode_GetBNListPtr(bn_info->nodes[i].edges[0].end_room);
			pos.x = WOS*ebn_info->nodes[bn_info->nodes[i].edges[0].end_index].pos.x;
			pos.y = WOS*ebn_info->nodes[bn_info->nodes[i].edges[0].end_index].pos.z;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw AI path segments
			for(j=1; j<bn_info->nodes[i].num_edges; j++)
			{
				ebn_info = BNode_GetBNListPtr(bn_info->nodes[i].edges[j].end_room);
				pos.x = WOS*ebn_info->nodes[bn_info->nodes[i].edges[j].end_index].pos.x;
				pos.y = WOS*ebn_info->nodes[bn_info->nodes[i].edges[j].end_index].pos.z;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
		}
		// Draw Bnodes
		for (i=0; i<bn_info->num_nodes; i++)
		{
			rect.left = WOS*Round(bn_info->nodes[i].pos.x - size);
			rect.right = WOS*Round(bn_info->nodes[i].pos.x + size);
			rect.top = WOS*Round(bn_info->nodes[i].pos.z + size);
			rect.bottom = WOS*Round(bn_info->nodes[i].pos.z - size);
			// Draw sphere
			pDC->Ellipse(&rect);
			// Draw Bnode number
			pos.x = WOS*bn_info->nodes[i].pos.x;
			pos.y = WOS*bn_info->nodes[i].pos.z;
			DrawOrthoNumber(pDC,i+1,pos,size,O_BR_GRIDLINE_COLOR);
		}
		break;

	case VIEW_ZY:
		for (i=0; i<bn_info->num_nodes; i++)
		{
			// Position pen at first edge of Bnode
			if (!bn_info->nodes[i].num_edges)
				break;
			ebn_info = BNode_GetBNListPtr(bn_info->nodes[i].edges[0].end_room);
			pos.x = WOS*ebn_info->nodes[bn_info->nodes[i].edges[0].end_index].pos.z;
			pos.y = WOS*ebn_info->nodes[bn_info->nodes[i].edges[0].end_index].pos.y;
			pDC->MoveTo(Round(pos.x), Round(pos.y));
			// Draw AI path segments
			for(j=1; j<bn_info->nodes[i].num_edges; j++)
			{
				ebn_info = BNode_GetBNListPtr(bn_info->nodes[i].edges[j].end_room);
				pos.x = WOS*ebn_info->nodes[bn_info->nodes[i].edges[j].end_index].pos.z;
				pos.y = WOS*ebn_info->nodes[bn_info->nodes[i].edges[j].end_index].pos.y;
				pDC->LineTo(Round(pos.x), Round(pos.y));
			}
		}
		// Draw Bnodes
		for (i=0; i<bn_info->num_nodes; i++)
		{
			rect.left = WOS*Round(bn_info->nodes[i].pos.z - size);
			rect.right = WOS*Round(bn_info->nodes[i].pos.z + size);
			rect.top = WOS*Round(bn_info->nodes[i].pos.y + size);
			rect.bottom = WOS*Round(bn_info->nodes[i].pos.y - size);
			// Draw sphere
			pDC->Ellipse(&rect);
			// Draw Bnode number
			pos.x = WOS*bn_info->nodes[i].pos.z;
			pos.y = WOS*bn_info->nodes[i].pos.y;
			DrawOrthoNumber(pDC,i+1,pos,size,O_BR_GRIDLINE_COLOR);
		}
		break;
	}

	// Unselect brush
	pDC->SelectObject(pOldBrush);
	// Unselect pen
	pDC->SelectObject(pOldPen);

}



static float ArrayX[10][20]={	{-1,1,1,-1,-1},
								{-.25, 0.0, 0.0, 0.0, -1.0, 1.0},
								{-1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, -1.0, -1.0, 1.0},
					            {-1.0, 1.0, 1.0, -1.0, 1.0, 1.0, -.5},
								{-1.0, -1.0, -1.0, 1.0, 1.0, 1.0},
								{-1.0, 1.0, -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0},
								{-1.0, 1.0, -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0},
								{-1.0, 1.0, 1.0, 1.0},
								{-1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, -1.0, -1.0, 1.0},
								{1.0, -1.0, -1.0, 1.0, 1.0, 1.0}
							};

static float ArrayY[10][20]={	{1,1,-1,-1,1},
								{.75, 1.0, 1.0, -1.0, -1.0, -1.0},
								{1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, -1.0, -1.0},
								{1.0, 1.0, -1.0, -1.0, -1.0, 0, 0.0},
								{1.0, 0.0, 0.0, 0.0, 1.0, -1.0},
								{1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, -1.0, -1.0, -1.0},
								{1.0, 1.0, 1.0, -1.0, -1.0, -1.0, -1.0, 0.0, 0.0, 0.0},
								{1.0, 1.0, 1.0, -1.0},
								{1.0, 1.0, 1.0, -1.0, -1.0, -1.0, -1.0, 1.0, 0.0, 0.0},
								{1.0, 1.0, 0.0, 0.0, 1.0, -1.0}
							};

static int NumOfPoints[]={5,6,10,7,6,10,10,4,10,6};

void Cned_OrthoWnd::DrawOrthoNumber(CMemDC *pDC,int num,vec2D pos,float size,const int color)
{
	int num_array[10];
	int i,j;
	int total = num;
	size/=2;

	if(num < 0) 
	{
		return;
	}

	int num_numbers = log10(num) + 1;
	if (num_numbers > 10)
	{
		mprintf((0, "Cannot represent a number with over 10 digits\n"));
		Int3();
		return;
	}

	for(j = num_numbers - 1; j >= 0; j--)
	{
		num_array[j] = total / (pow(10, j));
		total -= num_array[j] * pow(10,j);
	}	

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	// Draw the number
	for(j = 0; j < num_numbers; j++)
	{
		vec2D cur_pos; 
		POINT pt;
		
		if (num_numbers & 0x00000001)
		{
			cur_pos.x = pos.x + (num_numbers >> 1) - j;
			cur_pos.y = pos.y + (num_numbers >> 1) - j;
		}
		else
		{
			cur_pos.x = pos.x + (num_numbers >> 1) - j - size;
			cur_pos.y = pos.y + (num_numbers >> 1) - j;
		}

		// Position pen at first point
		pt.x = Round(cur_pos.x + WOS*ArrayX[num_array[j]][0]);
		pt.y = Round(cur_pos.y + WOS*ArrayY[num_array[j]][0]);
		pDC->MoveTo(pt);

		// Draw lines
		for (i=1;i<NumOfPoints[num_array[j]];i++)
		{
			pt.x = Round(cur_pos.x + WOS*ArrayX[num_array[j]][i]);
			pt.y = Round(cur_pos.y + WOS*ArrayY[num_array[j]][i]);
			pDC->LineTo(pt);
		}
	}

	// Unselect pen
	pDC->SelectObject(pOldPen);

}


void Cned_OrthoWnd::DrawVerts(CMemDC *pDC,vector *verts,int num_verts,const int color)
{
	int vn;
	int size = O_VERT_SIZE;
	vec2D pos;
	POINT pt;
	int saved;

	// Select pen
	CPen *pOldPen = pDC->SelectObject(&g_pens[O_VERT_OUTLINE_COLOR]);

	// Select brush
	ASSERT(color >= 0 && color < NUM_BRUSHES);
	CBrush *pOldBrush = pDC->SelectObject(&g_brushes[color]);

	// Draw verts
	switch (m_nID)
	{
	case VIEW_XY:
		for (vn=0; vn<num_verts; vn++)
		{
			pos.x = WOS*verts[vn].x;
			pos.y = WOS*verts[vn].y;
			pt.x = Round(pos.x);
			pt.y = Round(pos.y);
			if ( !IsVertDrawn(pt,vn) )
			{
				pDC->LPtoDP(&pt);
				saved = pDC->SaveDC();
				pDC->SetMapMode(MM_TEXT);
				pDC->DPtoLP(&pt);
				pDC->Rectangle(pt.x-size,pt.y-size,pt.x+size,pt.y+size);
				pDC->RestoreDC(saved);
			}
		}
		break;

	case VIEW_XZ:
		for (vn=0; vn<num_verts; vn++)
		{
			pos.x = WOS*verts[vn].x;
			pos.y = WOS*verts[vn].z;
			pt.x = Round(pos.x);
			pt.y = Round(pos.y);
			if ( !IsVertDrawn(pt,vn) )
			{
				pDC->LPtoDP(&pt);
				saved = pDC->SaveDC();
				pDC->SetMapMode(MM_TEXT);
				pDC->DPtoLP(&pt);
				pDC->Rectangle(pt.x-size,pt.y-size,pt.x+size,pt.y+size);
				pDC->RestoreDC(saved);
			}
		}
		break;

	case VIEW_ZY:
		for (vn=0; vn<num_verts; vn++)
		{
			pos.x = WOS*verts[vn].z;
			pos.y = WOS*verts[vn].y;
			pt.x = Round(pos.x);
			pt.y = Round(pos.y);
			if ( !IsVertDrawn(pt,vn) )
			{
				pDC->LPtoDP(&pt);
				saved = pDC->SaveDC();
				pDC->SetMapMode(MM_TEXT);
				pDC->DPtoLP(&pt);
				pDC->Rectangle(pt.x-size,pt.y-size,pt.x+size,pt.y+size);
				pDC->RestoreDC(saved);
			}
		}
		break;
	}

	// Unselect pen
	pDC->SelectObject(pOldPen);

	// Unselect brush
	pDC->SelectObject(pOldBrush);
}


void Cned_OrthoWnd::CenterRoom(room *rp)
{
	vector pos;

	ComputeRoomBoundingSphere(&pos,rp); // ComputeRoomCenter

	switch (m_nID)
	{
	case VIEW_XY:
		m_Cam.pos.x = pos.x;
		m_Cam.pos.y = pos.y;
		break;

	case VIEW_XZ:
		m_Cam.pos.x = pos.x;
		m_Cam.pos.y = pos.z;
		break;

	case VIEW_ZY:
		m_Cam.pos.x = pos.z;
		m_Cam.pos.y = pos.y;
		break;
	}

	m_Cam.view_changed = true;
}


void Cned_OrthoWnd::CenterFace(room *rp,int facenum)
{
	vector pos;

	if (facenum == -1)
		return;

	ComputeFaceBoundingCircle(&pos,rp,facenum); // ComputeCenterPointOnFace

//	if (!m_pParentFrame->m_bSmoothCenter)
//	{
		switch (m_nID)
		{
		case VIEW_XY:
			m_Cam.pos.x = pos.x;
			m_Cam.pos.y = pos.y;
			break;

		case VIEW_XZ:
			m_Cam.pos.x = pos.x;
			m_Cam.pos.y = pos.z;
			break;

		case VIEW_ZY:
			m_Cam.pos.x = pos.z;
			m_Cam.pos.y = pos.y;
			break;
		}
/*
	}
	else
	{
		switch (m_nID)
		{
		case VIEW_XY:
			m_Cam.destpos.x = pos.x;
			m_Cam.destpos.y = pos.y;
			break;
		case VIEW_XZ:
			m_Cam.destpos.x = pos.x;
			m_Cam.destpos.y = pos.z;
			break;
		case VIEW_ZY:
			m_Cam.destpos.x = pos.z;
			m_Cam.destpos.y = pos.y;
			break;
		}
		// Start the camera moving
		m_Cam.step.x = (m_Cam.destpos.x-m_Cam.pos.x)/10;
		m_Cam.step.y = (m_Cam.destpos.y-m_Cam.pos.y)/10;
		m_Cam.moving = true;
	}
*/

	m_Cam.view_changed = true;
}


void Cned_OrthoWnd::DrawCurrentPrims(CMemDC *pDC)
{
	int vert_idx = -1;
	face *fp = NULL;
	room *rp = m_pPrim->roomp;

	if (m_pPrim->face != -1)
		fp = &rp->faces[m_pPrim->face];

	// Draw current vertex
	if (m_bShowVerts)
	{
		if (m_pParentFrame->m_Current_nif_vert != -1)
		{
			ASSERT(rp->num_verts);
			vert_idx = m_pParentFrame->m_Current_nif_vert;
		}
		else if (fp != NULL && m_pPrim->vert != -1)
			vert_idx = fp->face_verts[m_pPrim->vert];
		if (vert_idx != -1)
			DrawVerts(pDC,&rp->verts[vert_idx],1,O_CUR_VERT_COLOR);
	}

	// Draw current face
	if (fp != NULL)
		DrawOrthoFace(pDC, fp, rp, O_CUR_FACE_COLOR);
	if (m_bShowNormals && m_pPrim->face != -1)
		DrawOrthoFaceNormal(pDC, rp, m_pPrim->face, O_NORMAL_COLOR);

	// Draw current edge
	if (fp != NULL && m_pPrim->edge != -1)
		DrawOrthoEdge(pDC, fp, rp, O_CUR_EDGE_COLOR);

	// Draw current object indicator
	// Only draw it if the object is in the current room
	if (m_bShowObjects && rp->objects != -1 && m_pParentFrame == theApp.m_pRoomFrm)
		if ( Objects[Cur_object_index].roomnum == ROOMNUM(rp) )
			DrawOrthoObjectMark(pDC,rp,&Objects[Cur_object_index],O_ROOM_COLOR);
}


void Cned_OrthoWnd::DrawMarkedPrims(CMemDC *pDC)
{
	room *rp = m_pPrim->roomp;
	int i;

	// Draw marked vertices
	if (m_bShowVerts)
	{
		for (i=0; i<rp->num_verts; i++)
			if ( m_pVert_marks[i] )
				DrawVerts(pDC,&rp->verts[i],1,O_MARKED_VERT_COLOR);
	}

	// Draw marked faces
	for (i=0; i<rp->num_faces; i++)
	{
		if ( m_pFace_marks[i] )
			DrawOrthoFace(pDC, &rp->faces[i], rp, O_MARKED_FACE_COLOR);
		if (m_bShowNormals)
			DrawOrthoFaceNormal(pDC, rp, i, O_NORMAL_COLOR);
	}
}


void Cned_OrthoWnd::DrawOrthoFace(CMemDC *pDC,face *fp,room *rp,const int color)
{
	int vn;
	vec2D pos;

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	switch (m_nID)
	{
	case VIEW_XY:
		// Position pen at first vertex
		pos.x = WOS*rp->verts[fp->face_verts[0]].x;
		pos.y = WOS*rp->verts[fp->face_verts[0]].y;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		// Draw edges
		for (vn=1;vn<fp->num_verts;vn++)
		{
			pos.x = WOS*rp->verts[fp->face_verts[vn]].x;
			pos.y = WOS*rp->verts[fp->face_verts[vn]].y;
			pDC->LineTo(Round(pos.x), Round(pos.y));
		}
		// Draw last edge
		pos.x = WOS*rp->verts[fp->face_verts[0]].x;
		pos.y = WOS*rp->verts[fp->face_verts[0]].y;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;

	case VIEW_XZ:
		// Position pen at first vertex
		pos.x = WOS*rp->verts[fp->face_verts[0]].x;
		pos.y = WOS*rp->verts[fp->face_verts[0]].z;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		// Draw edges
		for (vn=1;vn<fp->num_verts;vn++)
		{
			pos.x = WOS*rp->verts[fp->face_verts[vn]].x;
			pos.y = WOS*rp->verts[fp->face_verts[vn]].z;
			pDC->LineTo(Round(pos.x), Round(pos.y));
		}
		// Draw last edge
		pos.x = WOS*rp->verts[fp->face_verts[0]].x;
		pos.y = WOS*rp->verts[fp->face_verts[0]].z;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;

	case VIEW_ZY:
		// Position pen at first vertex
		pos.x = WOS*rp->verts[fp->face_verts[0]].z;
		pos.y = WOS*rp->verts[fp->face_verts[0]].y;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		// Draw edges
		for (vn=1;vn<fp->num_verts;vn++)
		{
			pos.x = WOS*rp->verts[fp->face_verts[vn]].z;
			pos.y = WOS*rp->verts[fp->face_verts[vn]].y;
			pDC->LineTo(Round(pos.x), Round(pos.y));
		}
		// Draw last edge
		pos.x = WOS*rp->verts[fp->face_verts[0]].z;
		pos.y = WOS*rp->verts[fp->face_verts[0]].y;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;
	}

	// Unselect pen
	pDC->SelectObject(pOldPen);
}


void Cned_OrthoWnd::DrawOrthoEdge(CMemDC *pDC,face *fp,room *rp,const int color)
{
	int nextvert = m_pPrim->vert;
	vec2D pos;

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	int v0 = fp->face_verts[m_pPrim->edge];
	int v1 = fp->face_verts[(m_pPrim->edge+1)%fp->num_verts];

	switch (m_nID)
	{
	case VIEW_XY:
		// Position pen at first vertex
		pos.x = WOS*rp->verts[v0].x;
		pos.y = WOS*rp->verts[v0].y;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		// Draw edge
		(m_pPrim->vert == rp->faces[m_pPrim->face].num_verts-1) ? (nextvert = 0) : (nextvert++);
		pos.x = WOS*rp->verts[v1].x;
		pos.y = WOS*rp->verts[v1].y;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;

	case VIEW_XZ:
		// Position pen at first vertex
		pos.x = WOS*rp->verts[v0].x;
		pos.y = WOS*rp->verts[v0].z;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		// Draw edge
		(m_pPrim->vert == rp->faces[m_pPrim->face].num_verts-1) ? (nextvert = 0) : (nextvert++);
		pos.x = WOS*rp->verts[v1].x;
		pos.y = WOS*rp->verts[v1].z;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;

	case VIEW_ZY:
		// Position pen at first vertex
		pos.x = WOS*rp->verts[v0].z;
		pos.y = WOS*rp->verts[v0].y;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		// Draw edge
		(m_pPrim->vert == rp->faces[m_pPrim->face].num_verts-1) ? (nextvert = 0) : (nextvert++);
		pos.x = WOS*rp->verts[v1].z;
		pos.y = WOS*rp->verts[v1].y;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;
	}

	// Unselect pen
	pDC->SelectObject(pOldPen);
}


void Cned_OrthoWnd::DrawOrthoFaceNormal(CMemDC *pDC,room *rp,int facenum,const int color)
{
	vec2D pos;
	vector center;
	face *fp = &rp->faces[facenum];

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	ComputeFaceBoundingCircle(&center,rp,facenum); // ComputeCenterPointOnFace

	switch (m_nID)
	{
	case VIEW_XY:
		// Position pen at face center point and draw the normal
		pos.x = WOS*center.x;
		pos.y = WOS*center.y;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		pos.x += WOS*5*fp->normal.x;
		pos.y += WOS*5*fp->normal.y;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;

	case VIEW_XZ:
		// Position pen at face center point and draw the normal
		pos.x = WOS*center.x;
		pos.y = WOS*center.z;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		pos.x += WOS*5*fp->normal.x;
		pos.y += WOS*5*fp->normal.z;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;

	case VIEW_ZY:
		// Position pen at face center point and draw the normal
		pos.x = WOS*center.z;
		pos.y = WOS*center.y;
		pDC->MoveTo(Round(pos.x), Round(pos.y));
		pos.x += WOS*5*fp->normal.z;
		pos.y += WOS*5*fp->normal.y;
		pDC->LineTo(Round(pos.x), Round(pos.y));
		break;
	}

	// Unselect pen
	pDC->SelectObject(pOldPen);
}

void Cned_OrthoWnd::DrawOrthoObjectMark(CMemDC *pDC,room *rp,object *objp,const int color)
{
	RECT rect;
	vec2D pos;
	POINT pt;

	// Select pen
	ASSERT(color >= 0 && color < NUM_PENS);
	CPen *pOldPen = pDC->SelectObject(&g_pens[color]);

	switch (m_nID)
	{
	case VIEW_XY:
		rect.left = objp->pos.x - objp->size;
		rect.right = objp->pos.x + objp->size;
		rect.top = objp->pos.y + objp->size;
		rect.bottom = objp->pos.y - objp->size;
		break;

	case VIEW_XZ:
		rect.left = objp->pos.x - objp->size;
		rect.right = objp->pos.x + objp->size;
		rect.top = objp->pos.z + objp->size;
		rect.bottom = objp->pos.z - objp->size;
		break;

	case VIEW_ZY:
		rect.left = objp->pos.z - objp->size;
		rect.right = objp->pos.z + objp->size;
		rect.top = objp->pos.y + objp->size;
		rect.bottom = objp->pos.y - objp->size;
		break;
	}

	// Convert to world coordinates
	rect.left *= WOS;
	rect.right *= WOS;
	rect.top *= WOS;
	rect.bottom *= WOS;

	// Draw brackets
	// Draw upper left bracket
	pos.x = rect.left;
	pos.y = rect.top - WOS*objp->size/2;
	pt.x = Round(pos.x);
	pt.y = Round(pos.y);
	pDC->MoveTo(pt.x, pt.y);
	pDC->LineTo(pt.x, rect.top);
	pDC->LineTo(Round(pos.x + WOS*objp->size/2), rect.top);
	// Draw upper right bracket
	pos.x = rect.right - WOS*objp->size/2;
	pos.y = rect.top;
	pt.x = Round(pos.x);
	pt.y = Round(pos.y);
	pDC->MoveTo(pt.x, pt.y);
	pDC->LineTo(rect.right, rect.top);
	pDC->LineTo(rect.right, Round(rect.top - WOS*objp->size/2));
	// Draw lower right bracket
	pos.x = rect.right;
	pos.y = rect.bottom + WOS*objp->size/2;
	pt.x = Round(pos.x);
	pt.y = Round(pos.y);
	pDC->MoveTo(pt.x, pt.y);
	pDC->LineTo(pt.x, rect.bottom);
	pDC->LineTo(Round(pos.x - WOS*objp->size/2), rect.bottom);
	// Draw lower left bracket
	pos.x = rect.left + WOS*objp->size/2;
	pos.y = rect.bottom;
	pt.x = Round(pos.x);
	pt.y = Round(pos.y);
	pDC->MoveTo(pt.x, pt.y);
	pDC->LineTo(rect.left, rect.bottom);
	pDC->LineTo(rect.left, Round(rect.bottom + WOS*objp->size/2));

	// Unselect pen
	pDC->SelectObject(pOldPen);
}

vec2D Cned_OrthoWnd::GetCursorPos()
{
	vec2D pos;

	switch (m_nID)
	{
	case VIEW_XY:
		pos.x = m_pvec_InsertPos->x;
		pos.y = m_pvec_InsertPos->y;
		break;

	case VIEW_XZ:
		pos.x = m_pvec_InsertPos->x;
		pos.y = m_pvec_InsertPos->z;
		break;

	case VIEW_ZY:
		pos.x = m_pvec_InsertPos->z;
		pos.y = m_pvec_InsertPos->y;
		break;
	}

	return pos;
}

void Cned_OrthoWnd::MoveCursor(float amount_x, float amount_y)
{
	switch (m_nID)
	{
	case VIEW_XY:
		m_pvec_InsertPos->x += amount_x;
		m_pvec_InsertPos->y += amount_y;
		break;

	case VIEW_XZ:
		m_pvec_InsertPos->x += amount_x;
		m_pvec_InsertPos->z += amount_y;
		break;

	case VIEW_ZY:
		m_pvec_InsertPos->z += amount_x;
		m_pvec_InsertPos->y += amount_y;
		break;
	}

	m_pParentFrame->m_State_changed = true;
}

void Cned_OrthoWnd::DrawCursor(CMemDC *pDC)
{
	int size = 5; // cursor size;

	// Create and select pen
	CPen pen(PS_SOLID,0,RGB(255,0,0));
	CPen *pOldPen = pDC->SelectObject(&pen);

	// Get the cursor location
	vec2D pos = GetCursorPos();
	pos.x *= WOS; pos.y *= WOS;

	POINT pt = {Round(pos.x),Round(pos.y)};

	// Draw the cursor
	pDC->LPtoDP(&pt);
	int saved = pDC->SaveDC();
	pDC->SetMapMode(MM_TEXT);
	pDC->DPtoLP(&pt);
	pDC->MoveTo(pt.x-size,pt.y-size);
	pDC->LineTo(pt.x+size+1,pt.y+size+1);
	pDC->MoveTo(pt.x-size,pt.y+size);
	pDC->LineTo(pt.x+size-1,pt.y-size-1);
	pDC->RestoreDC(saved);

	// Unselect pen
	pDC->SelectObject(pOldPen);
}

void Cned_OrthoWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	//Load top-level menu from resource
	CMenu mnuTop;
	mnuTop.LoadMenu(IDR_ORTHOWND_POPUP);

	//get popup menu from first submenu
	CMenu *pPopup = mnuTop.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	//Checked state for popup menu items is automatically managed by MFC
	//UPDATE_COMMAND_UI 

	//Display popup menu
	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this,NULL);

//	CWnd::OnContextMenu(pWnd, point);
}

void Cned_OrthoWnd::GridSize(int num) 
{
	ASSERT(num >=0 && num < NUM_GRIDS);

	if (m_nGrid != num || m_nGrid == NUM_GRIDS-1) {
		m_nGrid = num;
		m_State_changed = true;
	}
}

void Cned_OrthoWnd::OnGridSize1() 
{
	GridSize(1);
}

void Cned_OrthoWnd::OnGridSize2() 
{
	GridSize(2);
}

void Cned_OrthoWnd::OnGridSize3() 
{
	GridSize(3);
}

void Cned_OrthoWnd::OnGridSize4() 
{
	GridSize(4);
}

void Cned_OrthoWnd::OnGridSize5() 
{
	GridSize(5);
}

void Cned_OrthoWnd::OnShowAtchRooms() 
{
	m_bShowAttached = !m_bShowAttached;
	m_State_changed = true;
}

void Cned_OrthoWnd::OnShowGrid() 
{
	m_bGridShow = !m_bGridShow;
	m_State_changed = true;
}

void Cned_OrthoWnd::OnShowNormals() 
{
	m_bShowNormals = !m_bShowNormals;
	m_State_changed = true;
}

void Cned_OrthoWnd::OnShowVerts() 
{
	m_bShowVerts = !m_bShowVerts;
	m_State_changed = true;
}

void Cned_OrthoWnd::OnUpdateShowGrid(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(m_bGridShow);
}


// Keep track of positions already drawn to, so that there is no wasteful redraw
bool Cned_OrthoWnd::IsVertDrawn(POINT pos,int index)
{
	static POINT Already_drawn_pos[MAX_VERTS_PER_ROOM];
	bool Already_drawn = false;
	int i;

	// Search for vert already drawn at this point
	for (i=0; i<index; i++)
		if (Already_drawn_pos[i].x == pos.x && Already_drawn_pos[i].y == pos.y)
		{
			Already_drawn = true;
			break;
		}
	Already_drawn_pos[index] = pos;

	return Already_drawn;
}


void Cned_OrthoWnd::SetCursorPos(vec2D pos)
{
	switch (m_nID)
	{
	case VIEW_XY:
		m_pvec_InsertPos->x = pos.x;
		m_pvec_InsertPos->y = pos.y;
		m_pvec_InsertPos->z = m_pvec_RefPos->z;
		break;

	case VIEW_XZ:
		m_pvec_InsertPos->x = pos.x;
		m_pvec_InsertPos->y = m_pvec_RefPos->y;
		m_pvec_InsertPos->z = pos.y;
		break;

	case VIEW_ZY:
		m_pvec_InsertPos->x = m_pvec_RefPos->x;
		m_pvec_InsertPos->y = pos.y;
		m_pvec_InsertPos->z = pos.x;
		break;
	}

	m_pParentFrame->m_State_changed = true;
}



void Cned_OrthoWnd::SetMousePos(vec2D pos)
{
	switch (m_nID)
	{
	case VIEW_XY:
		m_pvec_MousePos->x = pos.x;
		m_pvec_MousePos->y = pos.y;
		m_pvec_MousePos->z = m_pvec_RefPos->z;
		break;

	case VIEW_XZ:
		m_pvec_MousePos->x = pos.x;
		m_pvec_MousePos->y = m_pvec_RefPos->y;
		m_pvec_MousePos->z = pos.y;
		break;

	case VIEW_ZY:
		m_pvec_MousePos->x = m_pvec_RefPos->x;
		m_pvec_MousePos->y = pos.y;
		m_pvec_MousePos->z = pos.x;
		break;
	}
}


vec2D Cned_OrthoWnd::GetMousePos()
{
	vec2D pos;

	switch (m_nID)
	{
	case VIEW_XY:
		pos.x = m_pvec_MousePos->x;
		pos.y = m_pvec_MousePos->y;
		break;

	case VIEW_XZ:
		pos.x = m_pvec_MousePos->x;
		pos.y = m_pvec_MousePos->z;
		break;

	case VIEW_ZY:
		pos.x = m_pvec_MousePos->z;
		pos.y = m_pvec_MousePos->y;
		break;
	}

	return pos;
}

void Cned_OrthoWnd::OnCenterRoom() 
{
	// TODO: Add your command handler code here
	CenterRoom(m_pPrim->roomp);
}

bool Cned_OrthoWnd::UpdateGridSize(int num) 
{
	// TODO: Add your command update UI handler code here
	bool bCheck = true;

	if (m_nGrid != num)
		bCheck = false;

	return bCheck;
}

void Cned_OrthoWnd::OnUpdateGridSize1(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(1));
}

void Cned_OrthoWnd::OnUpdateGridSize2(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(2));
}

void Cned_OrthoWnd::OnUpdateGridSize3(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(3));
}

void Cned_OrthoWnd::OnUpdateGridSize4(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(4));
}

void Cned_OrthoWnd::OnUpdateGridSize5(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(5));
}

void Cned_OrthoWnd::OnUpdateShowAtchRooms(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowAttached);
}

void Cned_OrthoWnd::OnUpdateShowNormals(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowNormals);
}

void Cned_OrthoWnd::OnUpdateShowVerts(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowVerts);
}

void Cned_OrthoWnd::MarkVertsInSelBox(CClientDC *pDC,bool bMark)
{
	// Search selection box
	CRect selbox;
	POINT vert_pt;
	int i;
	room *rp = m_pPrim->roomp;

	selbox.CopyRect(&m_rTracker.m_rect);
	selbox.NormalizeRect();

	// Check each vertex
	switch (m_nID)
	{
	case VIEW_XY:
		for (i=0; i<rp->num_verts; i++)
		{
			vert_pt.x = Round(WOS*rp->verts[i].x);
			vert_pt.y = Round(WOS*rp->verts[i].y);
			pDC->LPtoDP(&vert_pt);
			// Mark vertex
			if ( selbox.PtInRect(vert_pt) )
				if (bMark)
					m_pParentFrame->MarkVert(i);
				else
					m_pParentFrame->UnMarkVert(i);
		}
		break;

	case VIEW_XZ:
		for (i=0; i<rp->num_verts; i++)
		{
			vert_pt.x = Round(WOS*rp->verts[i].x);
			vert_pt.y = Round(WOS*rp->verts[i].z);
			pDC->LPtoDP(&vert_pt);
			// Mark vertex
			if ( selbox.PtInRect(vert_pt) )
				if (bMark)
					m_pParentFrame->MarkVert(i);
				else
					m_pParentFrame->UnMarkVert(i);
		}
		break;

	case VIEW_ZY:
		for (i=0; i<rp->num_verts; i++)
		{
			vert_pt.x = Round(WOS*rp->verts[i].z);
			vert_pt.y = Round(WOS*rp->verts[i].y);
			pDC->LPtoDP(&vert_pt);
			// Mark vertex
			if ( selbox.PtInRect(vert_pt) )
				if (bMark)
					m_pParentFrame->MarkVert(i);
				else
					m_pParentFrame->UnMarkVert(i);
		}
		break;
	}
}

void Cned_OrthoWnd::MarkFacesInSelBox(CClientDC *pDC,bool bMark)
{
	// Search selection box
	CRect selbox;
	POINT vert_pt;
	int i,j;
	bool bInclude;
	room *rp = m_pPrim->roomp;
	face *fp;

	selbox.CopyRect(&m_rTracker.m_rect);
	selbox.NormalizeRect();

	// Check each vertex
	switch (m_nID)
	{
	case VIEW_XY:
		for (i=0; i<rp->num_faces; i++)
		{
			fp = &rp->faces[i];
			bInclude = true;
			for (j=0; j<fp->num_verts; j++)
			{
				vert_pt.x = Round(WOS*rp->verts[fp->face_verts[j]].x);
				vert_pt.y = Round(WOS*rp->verts[fp->face_verts[j]].y);
				pDC->LPtoDP(&vert_pt);
				// Check for all face verts in selbox
				if ( !selbox.PtInRect(vert_pt) )
					bInclude = false;
			}
			// Mark face
			if (bInclude && bMark)
				m_pParentFrame->MarkFace(i);
			else if (bInclude && !bMark)
				m_pParentFrame->UnMarkFace(i);
		}
		break;

	case VIEW_XZ:
		for (i=0; i<rp->num_faces; i++)
		{
			fp = &rp->faces[i];
			bInclude = true;
			for (j=0; j<fp->num_verts; j++)
			{
				vert_pt.x = Round(WOS*rp->verts[fp->face_verts[j]].x);
				vert_pt.y = Round(WOS*rp->verts[fp->face_verts[j]].z);
				pDC->LPtoDP(&vert_pt);
				// Check for all face verts in selbox
				if ( !selbox.PtInRect(vert_pt) )
					bInclude = false;
			}
			// Mark face
			if (bInclude && bMark)
				m_pParentFrame->MarkFace(i);
			else if (bInclude && !bMark)
				m_pParentFrame->UnMarkFace(i);
		}
		break;

	case VIEW_ZY:
		for (i=0; i<rp->num_faces; i++)
		{
			fp = &rp->faces[i];
			bInclude = true;
			for (j=0; j<fp->num_verts; j++)
			{
				vert_pt.x = Round(WOS*rp->verts[fp->face_verts[j]].z);
				vert_pt.y = Round(WOS*rp->verts[fp->face_verts[j]].y);
				pDC->LPtoDP(&vert_pt);
				// Check for all face verts in selbox
				if ( !selbox.PtInRect(vert_pt) )
					bInclude = false;
			}
			// Mark face
			if (bInclude && bMark)
				m_pParentFrame->MarkFace(i);
			else if (bInclude && !bMark)
				m_pParentFrame->UnMarkFace(i);
		}
		break;
	}
}


int Cned_OrthoWnd::SelectFace(int view_flag,vec2D search_pos)
{
	room *rp = m_pPrim->roomp;
	int closest[MAX_MOUSE_SELECT];
	int i;
	int count = 0;
	int facenum = m_pPrim->face;

	for (i=0; i<rp->num_faces; i++)
	{
		if (PointInFace(view_flag,search_pos,i) && count < MAX_MOUSE_SELECT)
		{
			closest[count] = i;
			count++;
		}
	}

	if (count > 0)
	{
		for (i=count-1; i>=0; i--)
		{
			for (int k=1; k<=i; k++)
			{
				vector center1,center2;
				ComputeFaceBoundingCircle(&center1,rp,closest[k-1]); // ComputeCenterPointOnFace
				ComputeFaceBoundingCircle(&center2,rp,closest[k]); // ComputeCenterPointOnFace
				if (CompareVectorDepth(view_flag,center1,center2))
				{
					int swap = closest[k-1];
					closest[k-1] = closest[k];
					closest[k] = swap;
				}
			}
		}

		int pos = -1;
		// check if current face is in closest
		for (i=0; i<count; i++)
			if (closest[i] == facenum)
				pos = i;
		if (pos > -1)
		{
			if (pos == count-1)
				pos = 0;
			else
				pos++;
			facenum = closest[pos];
		}
		else
			facenum = closest[0];
	}

	return facenum;
}

bool Cned_OrthoWnd::PointInFace(int view_flag,vec2D search_pos,int facenum)
{
	room *rp = m_pPrim->roomp;
	bool inface = false;
	int num_verts = rp->faces[facenum].num_verts;
	int vert_idx = rp->faces[facenum].face_verts[num_verts - 1];
	vector i_vert;
	vector j_vert = rp->verts[vert_idx];
	int i;

	switch (view_flag)
	{
	case VIEW_XY:
		for (i=0; i<num_verts; i++)
		{
			vert_idx = rp->faces[facenum].face_verts[i];
			i_vert = rp->verts[vert_idx];
			if ( ( (i_vert.y < search_pos.y && search_pos.y < j_vert.y) || (j_vert.y < search_pos.y && search_pos.y < i_vert.y) ) && (search_pos.x < (j_vert.x - i_vert.x) * (search_pos.y - i_vert.y) / (j_vert.y - i_vert.y + 0.000001) + i_vert.x))
				inface = !inface;
			j_vert = i_vert;
		}
		break;
	case VIEW_XZ:
		for (i=0; i<num_verts; i++)
		{
			vert_idx = rp->faces[facenum].face_verts[i];
			i_vert = rp->verts[vert_idx];
			if ( ( (i_vert.z < search_pos.y && search_pos.y < j_vert.z) || (j_vert.z < search_pos.y && search_pos.y < i_vert.z) ) && (search_pos.x < (j_vert.x - i_vert.x) * (search_pos.y - i_vert.z) / (j_vert.z - i_vert.z + 0.000001) + i_vert.x))
				inface = !inface;
			j_vert = i_vert;
		}
		break;
	case VIEW_ZY:
		for (i=0; i<num_verts; i++)
		{
			vert_idx = rp->faces[facenum].face_verts[i];
			i_vert = rp->verts[vert_idx];
			if ( ( (i_vert.y < search_pos.y && search_pos.y < j_vert.y) || (j_vert.y < search_pos.y && search_pos.y < i_vert.y) ) && (search_pos.x < (j_vert.z - i_vert.z) * (search_pos.y - i_vert.y) / (j_vert.y - i_vert.y + 0.000001) + i_vert.z))
				inface = !inface;
			j_vert = i_vert;
		}
		break;
	}

	return inface;
}

int Cned_OrthoWnd::SelectVert(int view_flag,vec2D search_pos,bool *not_in_face)
{
	room *rp = m_pPrim->roomp;
	int closest[MAX_MOUSE_SELECT];
	int close_dist[MAX_MOUSE_SELECT];
	int low_dist;
	int cur_dist;
	int cur_close;
	int swap,swap2;
	int pos;
	int i,j,k;
	int count = 0;
	int sel_vert;
	int vert_idx;
	face *fp;
	short verts_in_faces[MAX_VERTS_PER_ROOM];

	memset(verts_in_faces,0,sizeof(short)*MAX_VERTS_PER_ROOM);

	cur_close = -1;
	low_dist = VERT_SELECT_SIZE * 10;
	sel_vert = m_pPrim->vert;

	*not_in_face = true;

	// Determine whether the clicked point is inside the bounds of a face, and then branch
	// (sure, it's an ugly function, but hey, no time to waste).
	for (i=0; i<rp->num_faces; i++)
	{
		if ( PointInFace(view_flag,search_pos,i) )
			*not_in_face = false;
	}

	if (*not_in_face)
	{

		// We're not searching in a face, so look for non-face verts within range (this doesn't handle the problem of non-face verts that are seen as "inside" faces)
		for (i=0; i<rp->num_faces; i++)
		{
			fp = &rp->faces[i];
			for (j=0; j<fp->num_verts; j++)
			{
				vert_idx = rp->faces[i].face_verts[j];
				if ( !verts_in_faces[vert_idx] )
					verts_in_faces[vert_idx] = true;
			}
		}
		for (i=0; i<rp->num_verts; i++)
		{
			if ( !verts_in_faces[i] )
			{
				cur_dist = Find2DDistance(view_flag,search_pos,rp->verts[i]);
				if (cur_dist < VERT_SELECT_SIZE && count < MAX_MOUSE_SELECT )
				{
					closest[count] = i;
					close_dist[count] = cur_dist;
					count++;
				}
				else if (cur_dist < low_dist)
				{
					low_dist = cur_dist;
					cur_close = i;
				}
			}
		}

		if (count > 0)
		{
			// Sort the verts by z-value (most positive at 0)
			for (i=count; i>0; i--)
			{
				for (k=1; k<i; k++)
				{
					if ( CompareDepth(view_flag,closest[k-1],closest[k]) )
					{
						swap = closest[k-1];
						closest[k-1] = closest[k];
						closest[k] = swap;
						swap2 = close_dist[k-1];
						close_dist[k-1] = close_dist[k];
						close_dist[k] = swap2;
					}
				}
			}

			pos = -1;
			// Check if current vert is the closest
			for (i=0; i<count; i++)
			{
				if (closest[i] == sel_vert)
					pos = i;
			}

			if (pos > -1)
			{
				if (pos == count - 1)
					pos = 0;
				else
					pos++;
				sel_vert = closest[pos];
			}
			else
			{
				// m_pPrim->vert = closest[0];
				// Choose the vert that is physically closer
				low_dist = VERT_SELECT_SIZE + 1;
				for (i=0; i<count; i++)
				{
					if ( close_dist[i] < low_dist )
					{
						low_dist = close_dist[i];
						sel_vert = closest[i];
					}
				}
			}
		}
		else
		{
			// Failed to find a close vert
			if (cur_close > -1)
				sel_vert = cur_close;
		}

	}
	else if (m_pPrim->face != -1)
	{

		for (i=0; i<rp->faces[m_pPrim->face].num_verts; i++)
		{
			vert_idx = rp->faces[m_pPrim->face].face_verts[i];
			cur_dist = Find2DDistance(view_flag,search_pos,rp->verts[vert_idx]);
			if (cur_dist < VERT_SELECT_SIZE && count < MAX_MOUSE_SELECT )
			{
				closest[count] = i;
				close_dist[count] = cur_dist;
				count++;
			}
			else if (cur_dist < low_dist)
			{
				low_dist = cur_dist;
				cur_close = i;
			}

		}

		if (count > 0)
		{
			// Sort the verts by z-value (most positive at 0)
			for (i=count; i>0; i--)
			{
				for (k=1; k<i; k++)
				{
					if ( CompareDepth(view_flag,rp->faces[m_pPrim->face].face_verts[closest[k-1]],rp->faces[m_pPrim->face].face_verts[closest[k]]) )
					{
						swap = closest[k-1];
						closest[k-1] = closest[k];
						closest[k] = swap;
						swap2 = close_dist[k-1];
						close_dist[k-1] = close_dist[k];
						close_dist[k] = swap2;
					}
				}
			}

			pos = -1;
			// Check if current vert is the closest
			for (i=0; i<count; i++)
			{
				if (closest[i] == sel_vert)
					pos = i;
			}

			if (pos > -1)
			{
				if (pos == count - 1)
					pos = 0;
				else
					pos++;
				sel_vert = closest[pos];
			}
			else
			{
				// m_pPrim->vert = closest[0];
				// Choose the vert that is physically closer
				low_dist = VERT_SELECT_SIZE + 1;
				for (i=0; i<count; i++)
				{
					if ( close_dist[i] < low_dist )
					{
						low_dist = close_dist[i];
						sel_vert = closest[i];
					}
				}
			}
		}
		else
		{
			// Failed to find a close vert
			if (cur_close > -1)
				sel_vert = cur_close;
		}

	}

	return sel_vert;
}


int Cned_OrthoWnd::Find2DDistance(int view_flag,vec2D search_pos,vector vec)
{
	int x_delta,y_delta;

	switch (view_flag)
	{
	case VIEW_XY:
		x_delta = abs(vec.x - search_pos.x);
		y_delta = abs(vec.y - search_pos.y);
		break;

	case VIEW_XZ:
		x_delta = abs(vec.x - search_pos.x);
		y_delta = abs(vec.z - search_pos.y);
		break;

	case VIEW_ZY:
		x_delta = abs(vec.z - search_pos.x);
		y_delta = abs(vec.y - search_pos.y);
		break;
	}

	return dist_2d(x_delta,y_delta);
}


bool Cned_OrthoWnd::CompareDepth(int view_flag,int a,int b)
{
	room *rp = m_pPrim->roomp;
	bool compare = false;

	switch (view_flag)
	{
	case VIEW_XY:
		compare = (rp->verts[a].z > rp->verts[b].z);
		break;

	case VIEW_XZ:
		compare = (rp->verts[a].y > rp->verts[b].y);
		break;

	case VIEW_ZY:
		compare = (rp->verts[a].x > rp->verts[b].x);
		break;
	}

	return compare;
}


bool Cned_OrthoWnd::CompareVectorDepth(int view_flag,vector a,vector b)
{
	room *rp = m_pPrim->roomp;
	bool compare = false;

	switch (view_flag)
	{
	case VIEW_XY:
		compare = a.z > b.z;
		break;

	case VIEW_XZ:
		compare = a.y > b.y;
		break;

	case VIEW_ZY:
		compare = a.x > b.x;
		break;
	}

	return compare;
}


void Cned_OrthoWnd::OnCenterOrigin() 
{
	// TODO: Add your command handler code here
	CenterOrigin();
}


void Cned_OrthoWnd::CenterOrigin() 
{
	POINT pos = {0,0};

	m_Cam.pos = pos;

	m_Cam.view_changed = true;
}



void Cned_OrthoWnd::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	switch (nChar)
	{
	case 0x5B:				// '['
	case 0x7B:				// '{'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
			if (m_nSnap != 0)
				m_nSnap--;
			m_State_changed = true;
		}
		else
		{
			if (m_nGrid != 0)
				m_nGrid--;
			m_State_changed = true;
		}
		break;

	case 0x5D:				// ']'
	case 0x7D:				// '}'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
		{
			if (m_nSnap != NUM_SNAPS-2)
				m_nSnap++;
			m_State_changed = true;
		}
		else
		{
			if (m_nGrid != NUM_GRIDS-2)
				m_nGrid++;
			m_State_changed = true;
		}
		break;

	case 0x2C:				// ','
	case 0x3C:				// '<'
		m_ScaleStep -= 0.5f;
		if (m_ScaleStep < 1.1f)
			m_ScaleStep = 1.1f;
		break;

	case 0x2E:				// '.'
	case 0x3E:				// '>'
		m_ScaleStep += 0.5f;
		if (m_ScaleStep > 10.1f)
			m_ScaleStep = 10.1f;
		break;
	}

	CWnd::OnChar(nChar, nRepCnt, nFlags);
}

#define ROUND_FLOAT(val) ((val > 0.0f) ? (floor(val)) : (ceil(val)))

void Cned_OrthoWnd::SnapPoint(vec2D *pos)
{
	vec2D found_pos;
	float dist;
	int sign;
	int grid_size;
	(m_nGrid == NUM_GRIDS-1) ? (grid_size = m_CustomGrid) : (grid_size = Grids[m_nGrid]);

/*
	pos->x = (int)(pos->x/grid_size + 0.5)*grid_size;
	pos->y = (int)(pos->y/grid_size + 0.5)*grid_size;
*/

	dist = fmod(pos->x,grid_size);
	dist < 0 ? sign = -1 : sign = 1;
	if (abs(dist) <= grid_size/2)
		found_pos.x = ROUND_FLOAT(pos->x - dist);
	else
		found_pos.x = ROUND_FLOAT(pos->x + sign*(grid_size - abs(dist)));
	dist = fmod(pos->y,grid_size);
	dist < 0 ? sign = -1 : sign = 1;
	if (abs(dist) <= grid_size/2)
		found_pos.y = ROUND_FLOAT(pos->y - dist);
	else
		found_pos.y = ROUND_FLOAT(pos->y + sign*(grid_size - abs(dist)));
	*pos = found_pos;
}

void Cned_OrthoWnd::OnCenterFace() 
{
	// TODO: Add younr command handler code here
	CenterFace(m_pPrim->roomp,m_pPrim->face);
}

BOOL Cned_OrthoWnd::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	return FALSE;
}

void Cned_OrthoWnd::OnShowObjects() 
{
	// TODO: Add your command handler code here
	m_bShowObjects = !m_bShowObjects;
	m_State_changed = true;
}

void Cned_OrthoWnd::OnUpdateShowObjects(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowObjects);
}

void Cned_OrthoWnd::StopCamera()
{
	m_Cam.moving = false; // stop the camera from moving
}

void Cned_OrthoWnd::SetRefFrame(vec2D pt)
{
	vector pos;

	switch (m_nID)
	{
	case VIEW_XY:
		pos.x = pt.x;
		pos.y = pt.y;
		pos.z = m_pvec_RefPos->z;
		break;
	case VIEW_XZ:
		pos.x = pt.x;
		pos.z = pt.y;
		pos.y = m_pvec_RefPos->y;
		break;
	case VIEW_ZY:
		pos.z = pt.x;
		pos.y = pt.y;
		pos.x = m_pvec_RefPos->x;
		break;
	}

	m_pParentFrame->SetRefFrame(pos);
}

void Cned_OrthoWnd::OrientObject(int objnum,angle ang)
{
	ASSERT(Editor_state.mode == MODE_OBJECT);
	bool ret;

	switch (m_nID)
	{
	case VIEW_XY:
		ret = RotateObject(objnum,0,0,-ang);
		break;
	case VIEW_XZ:
		ret = RotateObject(objnum,0,ang,0);
		break;
	case VIEW_ZY:
		ret = RotateObject(objnum,ang,0,0);
		break;
	}
	if (ret)
	{
		PrintStatus("Current object reoriented.");
		World_changed = true;
		m_pParentFrame->UpdateAllPanes();
	}
	else
		OutrageMessageBox("Object not reoriented.");
}

/*
void Cned_RectTracker::DrawTrackerRect(LPCRECT lpRect, CWnd *pWndClipTo, CDC *pDC, CWnd *pWnd)
{
	CRectTracker::DrawTrackerRect(lpRect,pWndClipTo,pDC,pWnd);
}
*/

void Cned_OrthoWnd::OnGridSize0() 
{
	// TODO: Add your command handler code here
	GridSize(0);
}

void Cned_OrthoWnd::OnUpdateGridSize0(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(0));
}

void Cned_OrthoWnd::OnSnapToGrid() 
{
	// TODO: Add your command handler code here
	m_bGridSnap = !m_bGridSnap;
	m_State_changed = true;
}

void Cned_OrthoWnd::OnUpdateSnapToGrid(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bGridSnap);
}

void Cned_OrthoWnd::OnGridSize6() 
{
	// TODO: Add your command handler code here
	GridSize(6);
}

void Cned_OrthoWnd::OnUpdateGridSize6(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(6));
}



void Cned_OrthoWnd::OnGridSizeCustom() 
{
	// TODO: Add your command handler code here
	int num = m_CustomGrid;

	if (InputNumber(&num,"Custom Grid","Enter a custom grid size",this))
	{
		if (num<=0)
		{
			OutrageMessageBox("You must enter a value greater than 0.");
			return;
		}

		m_CustomGrid = num;
		GridSize(NUM_GRIDS-1);
	}
}

void Cned_OrthoWnd::OnUpdateGridSizeCustom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(NUM_GRIDS-1));
}

void Cned_OrthoWnd::OnShowRoomCenter() 
{
	// TODO: Add your command handler code here
	m_bShowCenters = !m_bShowCenters;
	m_State_changed = true;
}

void Cned_OrthoWnd::OnUpdateShowRoomCenter(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bShowCenters);
}
