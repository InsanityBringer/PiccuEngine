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

#include "ddaccess.h"
#include "renderer.h"
#include "ned_Rend.h"
#include "room_external.h"
#include "vecmat.h"
#include "../editor/selectedroom.h"
#include "../editor/SelManager.h"
#include "globals.h"
#include "ned_DrawWorld.h"
#include "ned_PerspWnd.h"
#include "ned_HFile.h"
#include "neweditor.h"
#include "ned_Geometry.h"
#include "../editor/HRoom.h"
#include "../editor/epath.h"
#include "../editor/ebnode.h"

#include "render.h"
#include "descent.h"
#include "game.h"
#include "terrain.h"
#include "../editor/HTexture.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Movement controls
#define PANNING  ( (m_Mouse.left && m_Keys.shift && !m_Keys.ctrl) || (m_Mouse.mid && !m_Keys.ctrl && !m_Keys.shift && !m_Keys.alt) )
#define ZOOMING	 ( (m_Mouse.left && m_Keys.shift && m_Keys.ctrl)  || (m_Mouse.mid && !m_Keys.ctrl && m_Keys.shift && !m_Keys.alt) )
#define ROTATING ( (m_Mouse.left && !m_Keys.shift && m_Keys.ctrl) || (m_Mouse.mid && !m_Keys.shift && m_Keys.ctrl && !m_Keys.alt) )
#define RADDING  ( (m_Mouse.left && m_Keys.alt && m_Keys.shift)   || (m_Mouse.mid && m_Keys.alt && !m_Keys.ctrl && !m_Keys.shift) )

extern object *Viewer_object;		//which object we are seeing from
static bool bDoOpenGL = false;
extern HWND save_wnd;
extern oeWin32Application *app;

int FindPointRoom(vector *pnt);

// Button down found

/////////////////////////////////////////////////////////////////////////////
// Cned_PerspWnd

IMPLEMENT_DYNCREATE(Cned_PerspWnd, CWnd)

Cned_PerspWnd::Cned_PerspWnd()
{
	m_nID = 0;
	m_InFocus = false;
	m_View_changed = false;
	m_State_changed = false;
	m_bTextured = false;
	m_bOutline = false;
	m_bCaptured = false;
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
	m_StartFlip = FALSE;
	m_last_start_room = -1;	
	m_bFlat = false;
	m_BDOWN_Found_type = -1;
	m_BDOWN_Found_roomnum = -1;
	m_BDOWN_Found_facenum = -1;
	m_BDOWN_Found_x = -1;
	m_BDOWN_Found_y = -1;
}

Cned_PerspWnd::~Cned_PerspWnd()
{
}


BEGIN_MESSAGE_MAP(Cned_PerspWnd, Cned_GrWnd)
	//{{AFX_MSG_MAP(Cned_PerspWnd)
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
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_CENTER_ROOM, OnCenterRoom)
	ON_COMMAND(ID_CENTER_ORIGIN, OnCenterOrigin)
	ON_WM_PAINT()
	ON_COMMAND(ID_TEXTURED, OnTextured)
	ON_COMMAND(ID_WIREFRAME, OnWireframe)
	ON_WM_SIZE()
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTFACE, OnViewMoveCameraToCurrentFace)
	ON_COMMAND(ID_CENTER_FACE, OnCenterFace)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTOBJECT, OnViewMoveCameraToCurrentObject)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVECAMERATOCURRENTOBJECT, OnUpdateViewMoveCameraToCurrentObject)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVECAMERATOCURRENTFACE, OnUpdateViewMoveCameraToCurrentFace)
	ON_UPDATE_COMMAND_UI(ID_TEXTURED, OnUpdateTextured)
	ON_UPDATE_COMMAND_UI(ID_WIREFRAME, OnUpdateWireframe)
	ON_COMMAND(ID_VIEW_TEXTURED_OUTLINE, OnViewTexturedOutline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTURED_OUTLINE, OnUpdateViewTexturedOutline)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTROOM, OnViewMoveCameraToCurrentRoom)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


void Cned_PerspWnd::TexGrStartOpenGL()
{
	if (bDoOpenGL)
	{
		app=(oeWin32Application *)g_OuroeApp;
		StateLimited=1;
		save_wnd=(HWND)app->m_hWnd;
		app->m_hWnd=(HWnd)m_hWnd;
		rend_SetOpenGLWindowState (1,g_OuroeApp,NULL);
		rend_ClearScreen(0);
		StateLimited=1;
		UseMultitexture=0;
		NoLightmaps=false;
	}
}

void Cned_PerspWnd::TexGrStopOpenGL ()
{
	if (bDoOpenGL)
	{
		rend_SetOpenGLWindowState (0,g_OuroeApp,NULL);
		app->m_hWnd=(HWnd)save_wnd;
	}
}


void Cned_PerspWnd::Render(bool force_repaint)
{
	int flags = 0;

	FrameCount++;

	//	Don't do a thing if these objects aren't created yet or the window isn't initialized for rendering
	if (!m_handle || !m_bInitted)
		return;

	if (Renderer_type == RENDERER_OPENGL)
		bDoOpenGL = true;

	vector view_pos = m_Cam.target - (m_Cam.orient.fvec * m_Cam.dist);

	if (m_bTextured) {
		//TexGrStartOpenGL();
		TGWRenderRoom(&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM,ROOMNUM(m_pPrim->roomp)); // D3EditState.current_room);
//		TGWRenderMine(&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM,ROOMNUM(m_pPrim->roomp)); // D3EditState.current_room);
		if (m_pParentFrame == theApp.m_pRoomFrm) // only draw for main room frame
		{
			DrawAllPaths(m_handle,&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM);
			EBNode_Draw(EBDRAW_ROOM,m_handle,&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM); // EBN_draw_type
		}
		//TexGrStopOpenGL();
	} else {
		flags |= DRAW_CUR | DRAW_OBJ;
		DrawWorld(m_handle,&m_Cam.target,&m_Cam.orient,m_Cam.dist,0,m_Cam.rad,flags,m_pPrim);
	}

	m_StartFlip = TRUE;
//	m_State_changed = true;
	if (force_repaint)
		InvalidateRect(NULL, FALSE);
}

/////////////////////////////////////////////////////////////////////////////
// Cned_PerspWnd message handlers

int Cned_PerspWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (Cned_GrWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here

	matrix orient = IDENTITY_MATRIX;

/*
	if (m_Title = "Perspective")
		m_Cam = Room_cam;
*/
	m_Cam.dist = DEFAULT_VIEW_DIST;
	m_Cam.rad = DEFAULT_VIEW_RAD;
	m_Cam.orient = orient;
	m_Cam.target = Mine_origin;
	m_Cam.view_changed = true;

	m_TimerHandle = SetTimer(gTimerID++,10,NULL);
	return 0;
}

void Cned_PerspWnd::OnDestroy() 
{
	Cned_GrWnd::OnDestroy();
	
	// TODO: Add your message handler code here
}

BOOL Cned_PerspWnd::Create(const RECT& rect, CWnd* pParentWnd, LPCTSTR name, UINT nID) 
{
	// TODO: Add your specialized code here and/or call the base class
	// this is not even called when pane is created within a splitter window
	DWORD dwStyle = GRWND_STATIC_STYLE;

	return CWnd::Create(NULL, name, dwStyle, rect, pParentWnd, nID);
}


void Cned_PerspWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.x = point.x;
	m_Mouse.y = point.y;

	Cned_GrWnd::OnMouseMove(nFlags, point);
}


void EndSel(editorSelectorManager *esm);


void Cned_PerspWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.left = true;

	if (!m_InFocus)
	{
		SetFocus();
		return;
	}

	m_BDOWN_Found_type = -1;
	m_BDOWN_Found_roomnum=-1;
	m_BDOWN_Found_facenum=-1;
	m_BDOWN_Found_x = -1;
	m_BDOWN_Found_y = -1;

	// Different routines depending on whether textures are up
	if (!m_bTextured)
	{
		int found_segnum,found_facenum;
		int find_mode = FM_CLOSEST;

		//See if we're clicking on the current face
		found_segnum = ROOMNUM(m_pPrim->roomp);
		found_facenum = m_pPrim->face;
		// WireframeFindRoomFace requires that the viewport its checking to be the last one that was rendered, so call Render() first.
		Render(false);
		if (WireframeFindRoomFace(m_handle,point.x,point.y,&found_segnum,&found_facenum,FM_SPECIFIC,true)) {
			find_mode = FM_NEXT;
		}

find_again:;

		if (WireframeFindRoomFace(m_handle,point.x,point.y,&found_segnum,&found_facenum,find_mode,true)) {
			ASSERT(found_segnum!=-1);
			ASSERT(found_facenum!=-1);
			room *rp = &Rooms[found_segnum];
			m_pParentFrame->SetPrim(rp,found_facenum,rp->faces[found_facenum].portal_num,0,0);
			// Update the current face/texture displays
			Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), m_pPrim->face);
		}
		else if (find_mode == FM_NEXT) {
			find_mode = FM_CLOSEST;
			goto find_again;
		}

		//	no more dragging, but something is highlighted.   
		//	so we will redraw world if mouse is pressed

		SelManager.StartSelection(this, EndSel, point.x, point.y);

/*
		// Method for inserting vertices from perspective view
		if (m_pPrim->face != -1 && m_pPrim->vert != -1 && m_pPrim->edge != -1)
		{
			vector vec;
			extern matrix last_view_orient;
			extern vector last_viewer_position;
			extern float last_zoom;

			int vert_idx = m_pPrim->roomp->faces[m_pPrim->face].face_verts[m_pPrim->vert];
			g3_StartFrame(&last_viewer_position,&last_view_orient,last_zoom);
			g3_Point2Vec(&vec, point.x, point.y);

// - PSEUDOCODE
//		for each point
//			plane_d = vec * (point - viewer);
//			closest_p = viewer + (d * vec);
//			d = vm_VectorDisance(&closest_p,&point);

			vector p, closest_p, vert_minus_viewer;
			float plane_dist, old_plane_dist = 10000;

			for (int i=0; i<m_pPrim->roomp->num_verts; i++) {
				vert_minus_viewer = m_pPrim->roomp->verts[i] - last_viewer_position;
				plane_dist = vm_DotProduct(&vec, &vert_minus_viewer);
				p = last_viewer_position + (plane_dist * vec);
				plane_dist = vm_VectorDistance(&p,&m_pPrim->roomp->verts[i]);
				if ( plane_dist < old_plane_dist) {
					old_plane_dist = plane_dist;
					closest_p = p;
				}
			}

			g3_EndFrame();

			*m_pvec_InsertPos = closest_p;
		}
*/
	}
	else // Textures
	{
		CRect rc(m_handle.default_viewport.x, m_handle.default_viewport.y, m_handle.default_viewport.x + m_handle.default_viewport.width, m_handle.default_viewport.y + m_handle.default_viewport.height);
		// Make sure the mouse cursor is not outside of the viewport!!!!
		if (rc.PtInRect(point))
		{

		//TexGrStartOpenGL();
			rend_MakeCurrent(m_handle);

//		ResetPostrenderList();

		//	mprintf((0,"ButtonDown %d,%d\n",point.x,point.y));
		TSearch_found_type=-1;
		TSearch_seg=-1;
		TSearch_on=1;
		TSearch_x=point.x;
		TSearch_y=point.y;

		vector view_pos = m_Cam.target - (m_Cam.orient.fvec * m_Cam.dist);

		TGWRenderRoom(&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM,ROOMNUM(m_pPrim->roomp)); // D3EditState.current_room);
//		TGWRenderMine (&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM,Viewer_object->roomnum);
		if (m_pParentFrame == theApp.m_pRoomFrm) // only draw for main room frame
		{
			DrawAllPaths(m_handle,&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM);
			EBNode_Draw(EBDRAW_ROOM,m_handle,&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM); // EBN_draw_type
		}
		TSearch_on=0;

		// Save what we found, for the button up handler
		m_BDOWN_Found_type = TSearch_found_type;
		m_BDOWN_Found_roomnum=TSearch_seg;
		m_BDOWN_Found_facenum=TSearch_face;
		m_BDOWN_Found_x = TSearch_x;
		m_BDOWN_Found_y = TSearch_y;

		//this looks at the last frame rendered
		if  (TSearch_found_type==TSEARCH_FOUND_OBJECT) 
		{	
			//Found a face or object
			ASSERT(m_pParentFrame == theApp.m_pRoomFrm);
			m_StartFlip = FALSE;			// unsignal flip.
			SelectObject(TSearch_seg);
			// Update the current object display
			Editor_state.SetCurrentLevelObject(Cur_object_index);
			m_pParentFrame->m_State_changed = true;
			// start object moving if mouse down in an object.
//			ObjMoveManager.Start(this, m_grViewport->width(), m_grViewport->height(), &m_ViewPos, &m_ViewMatrix, point.x, point.y);							
		}
		else
		if (TSearch_found_type==TSEARCH_FOUND_NODE)
		{
			// Found a gamepath node!
			ASSERT(m_pParentFrame == theApp.m_pRoomFrm);
			int path = TSearch_seg;
			int node = TSearch_face;
			Editor_state.SetCurrentPath(path);
			Editor_state.SetCurrentNode(node);

			if (path != -1)
				PrintStatus("Path %d, Node %d selected. Path Name = %s",path,node+1,GamePaths[path].name);
			m_State_changed = true;
		}
		else
		if (TSearch_found_type==TSEARCH_FOUND_BNODE)
		{
			// Found a Bnode!
			ASSERT(m_pParentFrame == theApp.m_pRoomFrm);
			int room = TSearch_seg;
			int node = TSearch_face;
			Editor_state.SetCurrentBNode(room,node,0);

			PrintStatus("BNode %d in room %d selected.",node+1,room);
			m_State_changed = true;
		}

		//TexGrStopOpenGL();

		}
	}

	Cned_GrWnd::OnLButtonDown(nFlags, point);
}

void Cned_PerspWnd::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.left = false;

	// Different routines depending on whether textures are up
	if (!m_bTextured)
	{
	}
	else
	{
		int roomnum,facenum,type;

		CRect rc(m_handle.default_viewport.x, m_handle.default_viewport.y, m_handle.default_viewport.x + m_handle.default_viewport.width, m_handle.default_viewport.y + m_handle.default_viewport.height);
		// Make sure the mouse cursor is not outside of the viewport!!!!
		if (rc.PtInRect(point))
		{
			//	mprintf((0,"ButtonDown %d,%d\n",point.x,point.y));
			TSearch_found_type=-1;
			TSearch_seg=-1;
			TSearch_on=1;
			TSearch_x=point.x;
			TSearch_y=point.y;

			vector view_pos = m_Cam.target - (m_Cam.orient.fvec * m_Cam.dist);

			rend_MakeCurrent(m_handle);
			//TexGrStartOpenGL();
			TGWRenderRoom(&view_pos,&m_Cam.orient,D3_DEFAULT_ZOOM,ROOMNUM(m_pPrim->roomp)); // D3EditState.current_room);
	//		TGWRenderMine (&Viewer_object->pos,&Viewer_object->orient,D3_DEFAULT_ZOOM,Viewer_object->roomnum);
			//TexGrStopOpenGL();

			TSearch_on=0;

			if (TSearch_found_type==TSEARCH_FOUND_MINE)
			{
				type = TSearch_found_type;
				roomnum=TSearch_seg;
				facenum=TSearch_face;

				// Now, if what we found was the same as what we found in lbuttondown, do the various actions
				// (This is done so that ctrl+click and shift+click only move the view when that is what is desired)
				if (m_BDOWN_Found_x != -1 && m_BDOWN_Found_y != -1 && 
					type == m_BDOWN_Found_type && 
					roomnum == m_BDOWN_Found_roomnum && facenum == m_BDOWN_Found_facenum)
				{

				if ((nFlags & MK_SHIFT) && !(nFlags & MK_CONTROL)) 
				{	//apply current texture to face
					HTextureApplyToRoomFace(&Rooms[TSearch_seg], TSearch_face, Editor_state.GetCurrentTexture());
					PrintStatus("Texture %d applied to face %d\n",Editor_state.GetCurrentTexture(),TSearch_face);
					m_pParentFrame->m_Room_changed = true;
					// Update the current face/texture displays
					Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(&Rooms[TSearch_seg]), TSearch_face);
				}
				else if ((nFlags & MK_CONTROL) && !(nFlags & MK_SHIFT)) 
				{	
					// Propagate texture alignment from current face
					if ((TSearch_seg == ROOMNUM(m_pPrim->roomp)) && (TSearch_face == m_pPrim->face))
						PrintStatus("Cannot propagate from a face to itself.");
					else if (HTexturePropagateToFace(&Rooms[roomnum],facenum,m_pPrim->roomp,m_pPrim->face,false)) 
					{
						PrintStatus("Texture propagated from face %d to face %d\n",m_pPrim->face,facenum);
						m_pParentFrame->m_Room_changed = true;
					}
					else 
					{
						OutrageMessageBox("Face %d is not adjacent to current face",facenum);
						return;
					}
				}
				else if (KEY_STATE(KEY_E)) 
				{
					// Copy uv values from marked face
					static int last_copy_marked_room=-1,last_copy_marked_face=-1;
					static int last_copy_room=-1,last_copy_face=-1;
					static int count;

					if (Markedroomp == NULL) 
					{
						OutrageMessageBox("You must have a marked face for this operation.");
						return;
					}

					if ((roomnum == ROOMNUM(Markedroomp)) && (facenum == Markedface))
						PrintStatus("Cannot copy from a face to itself.");
					else 
					{
						if ((last_copy_marked_room == ROOMNUM(Markedroomp)) && (last_copy_marked_face == Markedface) && (last_copy_room == roomnum) && (last_copy_face == facenum))
							 count++;
						else 
						{
							count=0;
							last_copy_marked_room = ROOMNUM(Markedroomp);
							last_copy_marked_face = Markedface;
							last_copy_room = roomnum;
							last_copy_face = facenum;
						}

						if (HTextureCopyUVsToFace(&Rooms[roomnum],facenum,Markedroomp,Markedface,count)) 
						{
							Rooms[roomnum].faces[facenum].tmap = Markedroomp->faces[Markedface].tmap;
							PrintStatus("Texture & UVs copied from %d:%d to %d:%d (rotation = %d/%d)\n",ROOMNUM(Markedroomp),Markedface,roomnum,facenum,count%Markedroomp->faces[Markedface].num_verts,Markedroomp->faces[Markedface].num_verts);
							World_changed = 1;
						}
						else
							PrintStatus("Can't copy UVs: faces don't have the same number of vertices.");
					}
				}
	/*
				else if (KEY_STATE(KEY_N))
				{
					vector vec=Rooms[roomnum].verts[Rooms[roomnum].faces[facenum].face_verts[0]];
					vector norm=Rooms[roomnum].faces[facenum].normal;
					float plane_dist=-(vec.x*norm.x+vec.y*norm.y+vec.z*norm.z);
					int i;
					for (i=0;i<Rooms[roomnum].faces[facenum].num_verts;i++)
					{
						vec=Rooms[roomnum].verts[Rooms[roomnum].faces[facenum].face_verts[i]];
						float dist = vec.x*norm.x+vec.y*norm.y+vec.z*norm.z+plane_dist;
						mprintf ((0,"Vertex %d distance from plane=%f\n",i,dist));
					}
				}
	*/
				else 
				{	//Just change curface
					// Update the current face/texture displays
					Editor_state.SetCurrentRoomFaceTexture(roomnum, facenum);
				}

				//All clicks on face change the current face
				m_pParentFrame->SetPrim(&Rooms[roomnum],facenum,Rooms[roomnum].faces[facenum].portal_num,0,0);

				}
			}
		}
	}

	Cned_GrWnd::OnLButtonUp(nFlags, point);
}

void Cned_PerspWnd::OnRButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.right = true;

	Cned_GrWnd::OnRButtonDown(nFlags, point);
}

void Cned_PerspWnd::OnRButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.right = false;

	Cned_GrWnd::OnRButtonUp(nFlags, point);
}

void Cned_PerspWnd::OnMButtonDown(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.mid = true;

	if (!m_InFocus)
		SetFocus();

	Cned_GrWnd::OnMButtonDown(nFlags, point);
}

void Cned_PerspWnd::OnMButtonUp(UINT nFlags, CPoint point) 
{
	// TODO: Add your message handler code here and/or call default
	m_Mouse.mid = false;

	Cned_GrWnd::OnMButtonUp(nFlags, point);
}

void Cned_PerspWnd::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
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
	
	Cned_GrWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

BOOL Cned_PerspWnd::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	m_Cam.dist += -((zDelta/WHEEL_DELTA) * ZOOM_SCALE);
//	if (m_Cam.dist < 0)
//		m_Cam.dist = 0;
	m_Cam.view_changed = 1;
	
	return true;//Cned_GrWnd::OnMouseWheel(nFlags, zDelta, pt);
}

void Cned_PerspWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	static int vert_idx = 0;
	room *rp = m_pPrim->roomp;
	short marked_face_list[MAX_FACES_PER_ROOM];
	int num_m_faces;
	int i;

	switch (nChar)
	{
	case VK_CONTROL:
		m_Keys.ctrl = true;
		break;

	case VK_SHIFT:
		m_Keys.shift = true;
		break;

	case VK_NUMPAD2:
		AdjustCamera(ROT_DOWN);
		break;

	case VK_UP:
		if (HIWORD(::GetKeyState(VK_CONTROL)))
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
				// If this room is a door room, set the current doorway
				Editor_state.SetCurrentDoorway(rp);
				if (m_pParentFrame->m_bAutoCenter)
					m_pParentFrame->OnCenterRoom();
			}
		}
		else
			AdjustCamera(PAN_UP);
		break;

	case VK_NUMPAD8:
		AdjustCamera(ROT_UP);
		break;

	case VK_DOWN:
		AdjustCamera(PAN_DOWN);
		break;

	case VK_NUMPAD1:
		AdjustCamera(BANK_LEFT);
		break;

	case VK_NUMPAD3:
		AdjustCamera(BANK_RIGHT);
		break;

	case VK_INSERT:
		// TODO : insert objects in 3D view?
		break;

	case VK_DELETE:
		m_pParentFrame->OnEditDelete();
		break;

	case VK_HOME:
		CenterOrigin();
		break;

	case 0x43:				// 'C'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			OnViewMoveCameraToCurrentFace();
		else
			CenterRoom(rp);
		break;

	case 0x47:				// 'G'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			OnViewMoveCameraToCurrentObject();
		break;

	case 0x57:				// 'W'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			AdjustCamera(PAN_UP);
		else
			AdjustCamera(ZOOM_INWARD);
		break;

	case 0x53:				// 'S'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			AdjustCamera(PAN_DOWN);
		else
			AdjustCamera(ZOOM_OUTWARD);
		break;

	case VK_NUMPAD4:
		AdjustCamera(ROT_LEFT);
		break;

	case VK_LEFT:
	case 0x41:			// 'A'
		AdjustCamera(PAN_LEFT);
		break;

	case VK_NUMPAD6:
		AdjustCamera(ROT_RIGHT);
		break;

	case VK_RIGHT:
	case 0x44:			// 'D'
		AdjustCamera(PAN_RIGHT);
		break;

	case 0x54:				// 'T'
		m_bTextured = !m_bTextured;
		m_State_changed = true;
		Render();
		break;

	case 0x4C:				// 'L'
		if (m_bTextured)
		{
			m_bOutline = !m_bOutline;
			m_State_changed = true;
			Render();
		}
		break;

	case 0x4D:				// 'M'
		if ( HIWORD(::GetKeyState(VK_SHIFT)) )
			OnViewMoveCameraToCurrentRoom();
		else
			m_pParentFrame->OnMarkAll();
		break;

	case VK_SPACE:
		m_pParentFrame->OnMarkToggle();
		break;

	case 0x55:				// 'U'
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
	}
	
	Cned_GrWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void Cned_PerspWnd::OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	
	Cned_GrWnd::OnSysChar(nChar, nRepCnt, nFlags);
}

void Cned_PerspWnd::OnTimer(UINT nIDEvent) 
{
	//Do our thing here
	int dx,dy;

	if (m_Cam.view_changed || m_State_changed) 
	{
		m_State_changed = m_Cam.view_changed = false;
		Render();
	}

	//Only process input when we have the focus
	if(m_InFocus)
	{
		dx = m_Mouse.x-m_Mouse.oldx;
		dy = m_Mouse.y-m_Mouse.oldy;

		if ( (dx!=0) || (dy!=0) ) {
			if (PANNING || ZOOMING || ROTATING || RADDING)
			{
				if (!m_bCaptured)
				{
					SetCapture();
					m_bCaptured = true;
				}
				if ( ROTATING ) {
					matrix rotmat,tempm;
					GetMouseRotation( dx, dy, &rotmat );
					tempm = m_Cam.orient * rotmat;
					m_Cam.orient = tempm;
					m_View_changed = true;
				}
				else if ( PANNING ) {
					m_Cam.target += m_Cam.orient.rvec * -dx * MOVE_SCALE; 
					m_Cam.target += m_Cam.orient.uvec *  dy * MOVE_SCALE; 
					m_View_changed = true;
				}
				else if ( ZOOMING ) {
					m_Cam.dist += dy * ZOOM_SCALE;
//					if (m_Cam.dist < 0)
//						m_Cam.dist = 0;
					m_View_changed = true;
				}
				else if ( RADDING ) {
					m_Cam.rad += dy * RAD_SCALE;
					if (m_Cam.rad < 0)
						m_Cam.rad = 0;
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
			Render();
		}
	}

	// Save mouse position
	m_Mouse.oldx = m_Mouse.x;
	m_Mouse.oldy = m_Mouse.y;

	// If the mouse position changes an appreciable amount, invalidate the saved button down coordinates
	if (m_BDOWN_Found_x != -1 && m_BDOWN_Found_y != -1)
		if ( abs(m_BDOWN_Found_x-m_Mouse.x) > 5 || abs(m_BDOWN_Found_y-m_Mouse.y) > 5 )
			m_BDOWN_Found_x = m_BDOWN_Found_y = -1;

	if (m_Cam.moving)
	{
		vector target = {m_Cam.target.x+m_Cam.step.x, m_Cam.target.y+m_Cam.step.y, m_Cam.target.z+m_Cam.step.z};
		PlaceCamera(target,m_Cam.orient,m_Cam.dist);
		if ( abs(m_Cam.target.x-m_Cam.desttarget.x) <= abs(m_Cam.step.x) )
			m_Cam.step.x = 0;
		if ( abs(m_Cam.target.y-m_Cam.desttarget.y) <= abs(m_Cam.step.y) )
			m_Cam.step.y = 0;
		if ( abs(m_Cam.target.z-m_Cam.desttarget.z) <= abs(m_Cam.step.z) )
			m_Cam.step.z = 0;
		if (m_Cam.step.x == 0 && m_Cam.step.y == 0 && m_Cam.step.z == 0)
		{
			PlaceCamera(m_Cam.desttarget,m_Cam.orient,m_Cam.dist);
			m_Cam.moving = false;
		}
	}

	Cned_GrWnd::OnTimer(nIDEvent);
}

void Cned_PerspWnd::AdjustCamera(int code)
{
	matrix tempm,rotmat;
	float scale = 0.5f;
	angle p = 0, h = 0, b = 0;

	tempm = m_Cam.orient;

	switch (code)
	{
	case PAN_LEFT:
		m_Cam.target += m_Cam.orient.rvec * (-(100*scale));
		break;
	case PAN_RIGHT:
		m_Cam.target += m_Cam.orient.rvec * (100*scale);
		break;
	case PAN_UP:
		m_Cam.target += m_Cam.orient.uvec * (100*scale);
		break;
	case PAN_DOWN:
		m_Cam.target += m_Cam.orient.uvec * (-(100*scale));
		break;
	case ROT_LEFT:
	case ROT_RIGHT:
	case ROT_UP:
	case ROT_DOWN:
	case BANK_LEFT:
	case BANK_RIGHT:
		switch (code)
		{
		case ROT_LEFT:
			h = 65535 - 0x800;
			break;
		case ROT_RIGHT:
			h = 0x800;
			break;
		case ROT_UP:
			p = 65535 - 0x800;
			break;
		case ROT_DOWN:
			p = 0x800;
			break;
		case BANK_LEFT:
			b = 65535 - 0x800;
			break;
		case BANK_RIGHT:
			b = 0x800;
			break;
		}
		vm_AnglesToMatrix(&rotmat, p, h, b);
		m_Cam.orient = tempm * rotmat;
		vm_Orthogonalize(&m_Cam.orient);
		break;
	case ZOOM_INWARD:
		m_Cam.dist -= (100*scale);
		break;
	case ZOOM_OUTWARD:
		m_Cam.dist += (100*scale);
		break;
	case MOVE_FORWARD:
		m_Cam.target += m_Cam.orient.fvec * (100*scale);
		break;
	case MOVE_BACKWARD:
		m_Cam.target += m_Cam.orient.fvec * (-(100*scale));
		break;
	default:
		Int3();
		break;
	}

	m_View_changed = 1;
}

void Cned_PerspWnd::OnSetFocus(CWnd* pOldWnd) 
{
	Cned_GrWnd::OnSetFocus(pOldWnd);
	
	if(gCameraSlewer)
		gCameraSlewer->AttachCamera(&m_Cam,(char *)LPCSTR(m_Title));

	m_pParentFrame->m_Focused_pane = m_nID;

	m_InFocus = true;
	
}

void Cned_PerspWnd::OnKillFocus(CWnd* pNewWnd) 
{
	Cned_GrWnd::OnKillFocus(pNewWnd);
	memset(&m_Mouse,0,sizeof(m_Mouse));
	memset(&m_Keys,0,sizeof(m_Keys));

	m_InFocus = false;
	
}

void Cned_PerspWnd::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
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

void Cned_PerspWnd::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
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




void Cned_PerspWnd::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	//Load top-level menu from resource
	CMenu mnuTop;
	mnuTop.LoadMenu(IDR_PERSPWND_POPUP);

	//get popup menu from first submenu
	CMenu *pPopup = mnuTop.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	//Checked state for popup menu items is automatically managed by MFC
	//UPDATE_COMMAND_UI 

	//Display popup menu
	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this,NULL);

//	CWnd::OnContextMenu(pWnd, point);
}


void Cned_PerspWnd::InitCamera()
{
	m_Cam.dist = DEFAULT_VIEW_DIST;
	m_Cam.rad = DEFAULT_VIEW_RAD;
	vm_MakeIdentity(&m_Cam.orient);
	m_Cam.moving = false;

	CenterRoom(m_pPrim->roomp);
}


void Cned_PerspWnd::OnCenterRoom() 
{
	// TODO: Add your command handler code here
	CenterRoom(m_pPrim->roomp);
}


void Cned_PerspWnd::CenterRoom(room *rp)
{
	vector pos;

	ComputeRoomBoundingSphere(&pos,m_pPrim->roomp); // ComputeRoomCenter(&pos,m_pPrim->roomp);
	m_Cam.target = pos;

	m_Cam.view_changed = true;
}


void Cned_PerspWnd::CenterFace(room *rp,int facenum)
{
	vector pos;

	if (facenum == -1)
		return;

	ComputeFaceBoundingCircle(&pos,rp,facenum); // ComputeCenterPointOnFace(&pos,rp,facenum);
	if (!m_pParentFrame->m_bSmoothCenter)
		m_Cam.target = pos;
	else
	{
		m_Cam.desttarget = pos;
		// Start the camera moving
		m_Cam.step.x = (m_Cam.desttarget.x-m_Cam.target.x)/10;
		m_Cam.step.y = (m_Cam.desttarget.y-m_Cam.target.y)/10;
		m_Cam.step.z = (m_Cam.desttarget.z-m_Cam.target.z)/10;
		m_Cam.moving = true;
	}

	m_Cam.view_changed = true;
}


void Cned_PerspWnd::CenterOrigin()
{
	vector pos = {0,0,0};

	m_Cam.target = pos;

	m_Cam.view_changed = true;
}



void Cned_PerspWnd::OnCenterOrigin() 
{
	// TODO: Add your command handler code here
	CenterOrigin();
}

//Draws the mine, starting at a the specified room
//The rendering surface must be set up, and g3_StartFrame() must have been called
//Parameters:	viewer_roomnum - what room the viewer is in
//					flag_automap - if true, flag segments as visited when rendered
//					called_from_terrain - set if calling this routine from the terrain renderer
void RenderMine(int viewer_roomnum,int flag_automap,int called_from_terrain,bool render_all,bool outline,bool flat,prim *prim);

void Cned_PerspWnd::TGWRenderRoom(vector *pos,matrix *orient,float zoom,int start_roomnum)
{
	if (start_roomnum<0 || start_roomnum>MAX_ROOMS+MAX_PALETTE_ROOMS)
		return;

	//save these vars for possible use by FindRoomFace()
	m_last_viewer_eye = *pos;
	m_last_viewer_orient = *orient;
	m_last_zoom = zoom;
	m_last_start_room = start_roomnum;
	
	StartEditorFrame(m_handle,pos,orient,zoom);
	FrameCount++;

	RenderMine(start_roomnum,0,0,false,m_bOutline,m_bFlat,m_pPrim);
//	PostRender(Viewer_object->roomnum);

	//	we need to save these for object movement.
	g3_GetViewPosition(&m_ViewPos);
	g3_GetViewMatrix(&m_ViewMatrix);

	EndEditorFrame();
}



void Cned_PerspWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
//	Draw what's on the screen back page to the desktop
	if (m_bTextured && m_StartFlip) {	// do if a flip was signaled
		m_StartFlip = FALSE;

		if (bDoOpenGL)
		{
			/*HWND save_wnd;

			app=(oeWin32Application *)g_OuroeApp;

			save_wnd=(HWND)app->m_hWnd;
			app->m_hWnd=(HWnd)m_hWnd;
			rend_SetOpenGLWindowState (1,g_OuroeApp,NULL);

			rend_Flip();
			Cned_GrWnd::OnPaint();

			rend_SetOpenGLWindowState (0,g_OuroeApp,NULL);
			app->m_hWnd=(HWnd)save_wnd;*/
		}
		else
		{
	  		//m_grScreen->flip();
			//m_grViewport->clear(); // GR_RGB(128,128,128)
		}

		
	} else
		Cned_GrWnd::OnPaint();
	// Do not call Cned_GrWnd::OnPaint() for painting messages
}

void Cned_PerspWnd::OnTextured() 
{
	// TODO: Add your command handler code here
	m_bTextured = true;
	m_bOutline = false;
	m_State_changed = true;
	Render();
}

void Cned_PerspWnd::OnViewTexturedOutline() 
{
	// TODO: Add your command handler code here
	m_bTextured = true;
	m_bOutline = true;
	m_State_changed = true;
	Render();
}

void Cned_PerspWnd::OnWireframe() 
{
	// TODO: Add your command handler code here
	if (m_bTextured)
	{
		m_bTextured = false;
		m_bOutline = false;
		m_State_changed = true;
		Render();
	}
}

void Cned_PerspWnd::OnUpdateTextured(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bTextured && !m_bOutline);
}

void Cned_PerspWnd::OnUpdateViewTexturedOutline(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(m_bTextured && m_bOutline);
}

void Cned_PerspWnd::OnUpdateWireframe(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(!m_bTextured);
}

void Cned_PerspWnd::OnSize(UINT nType, int cx, int cy) 
{
	Cned_GrWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	Render();
}

void Cned_PerspWnd::OnViewMoveCameraToCurrentFace() 
{
	// TODO: Add your command handler code here
	if (m_pPrim->face != -1)
		PlaceCameraAtRoomFace(m_pPrim->roomp,m_pPrim->face,false);
}

//Set the camera in the specified room facing the specified face
//If room_center is true, put camera at the center of the room facing the face
//If room_center is false, put the camera directly in front of the selected face
//If the room is external, put the camera a distance away from the room, 
//facing either the center (if room_center is true) or the specified face
void Cned_PerspWnd::PlaceCameraAtRoomFace(room *roomp,int facenum,bool room_center)
{
	vector	vp,newpos;
	matrix	orient;
	int roomnum;
	bool outside_mine = 0;
	float dist = 0.0f;
	float face_rad = 0.0f;

	if (facenum != -1)
		face_rad = ComputeFaceBoundingCircle(&vp,roomp,facenum); // ComputeCenterPointOnFace(&vp,roomp,facenum);
	roomnum = ROOMNUM(roomp);

	if (room_center) {

		//Get position
		ComputeRoomBoundingSphere(&newpos,roomp); // ComputeRoomCenter(&newpos,roomp);

		if (roomp->flags & RF_EXTERNAL) {
/*
			vector t;
			float rad = ComputeRoomBoundingSphere(&t,roomp);

			newpos.z -= rad * 1.5;

			if(newpos.x < 1.0)
				newpos.x = 1.0;
			if(newpos.x > TERRAIN_WIDTH * TERRAIN_SIZE - 1.0)
				newpos.x = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;
			if(newpos.z < 1.0)
				newpos.z = 1.0;
			if(newpos.z > TERRAIN_DEPTH * TERRAIN_SIZE - 1.0) 
				newpos.z = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;

			orient = Identity_matrix;

			roomnum = GetTerrainRoomFromPos(&newpos);
*/
		}
		else {
			//Get orientation
			if (facenum != -1)
				vp -= newpos;		//vector from center of room to face
			else
				vp = newpos;
			vm_VectorToMatrix(&orient,&vp,NULL,NULL);
		}
		dist = 0.0f;
	}
	else {
		ASSERT(facenum != -1);
		face *fp = &roomp->faces[facenum];

		dist = face_rad;
		newpos = vp + fp->normal * dist;

		vector t = -fp->normal;
		vm_VectorToMatrix(&orient,&t,NULL,NULL);

		if (roomp->flags & RF_EXTERNAL) {
/*
			if(newpos.x < 1.0)
				newpos.x = 1.0;
			if(newpos.x > TERRAIN_WIDTH * TERRAIN_SIZE - 1.0)
				newpos.x = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;
			if(newpos.z < 1.0)
				newpos.z = 1.0;
			if(newpos.z > TERRAIN_DEPTH * TERRAIN_SIZE - 1.0) 
				newpos.z = TERRAIN_WIDTH * TERRAIN_SIZE - 1.0;
			roomnum = GetTerrainRoomFromPos(&newpos);
*/
		}
		else {
			int new_roomnum = FindPointRoom(&newpos);
			if (new_roomnum == -1)
				outside_mine = 1;
			else
				roomnum = new_roomnum;
		}
	}

	PlaceCamera(newpos,orient,5.0); // dist
/*
	//Reset viewer
	if (Editor_view_mode == VM_ROOM) {
		Viewer_object->pos = newpos; 
		Viewer_object->orient = orient;
	}
	else
		MoveViewer(&newpos,roomnum,&orient);

	if (outside_mine)
		Viewer_object->flags |= OF_OUTSIDE_MINE;

	Viewer_moved = 1;
*/
}

void Cned_PerspWnd::PlaceCamera(vector pos,matrix orient,float dist)
{
	m_Cam.dist = dist;
	m_Cam.rad = DEFAULT_VIEW_RAD;
	m_Cam.target = pos;
	m_Cam.orient = orient;

	m_Cam.view_changed = true;
}


void Cned_PerspWnd::OnCenterFace() 
{
	// TODO: Add your command handler code here
	if (m_pPrim->face != -1)
		CenterFace(m_pPrim->roomp,m_pPrim->face);
}

void Cned_PerspWnd::StopCamera()
{
	m_Cam.moving = false; // stop the camera from moving
}

void Cned_PerspWnd::OnViewMoveCameraToCurrentObject() 
{
	// TODO: Add your command handler code here
	object *objp = &Objects[Cur_object_index];
	matrix orient;

	//Turn the camera around so facing the object
	orient.fvec = -objp->orient.fvec;
	orient.rvec = -objp->orient.rvec;
	orient.uvec = objp->orient.uvec;

	//Move the camera to the object
	PlaceCamera(objp->pos,orient,2*objp->size);
}

void Cned_PerspWnd::OnUpdateViewMoveCameraToCurrentObject(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable = false;

	if ( theApp.m_pLevelWnd != NULL && 
		m_pParentFrame == theApp.m_pRoomFrm && 
		Cur_object_index != -1 && 
		Highest_object_index >= 0 && 
		Objects[Cur_object_index].roomnum == ROOMNUM(m_pPrim->roomp) )
		enable = true;

	pCmdUI->Enable(enable);
}

void Cned_PerspWnd::OnUpdateViewMoveCameraToCurrentFace(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable;

	m_pPrim->face != -1 ? enable = true : enable = false;
	pCmdUI->Enable(enable);
}



void Cned_PerspWnd::OnViewMoveCameraToCurrentRoom() 
{
	// TODO: Add your command handler code here
	PlaceCameraAtRoomFace(m_pPrim->roomp,m_pPrim->face,true);
}
