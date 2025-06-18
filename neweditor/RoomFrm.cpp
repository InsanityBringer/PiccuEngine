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

#include "../editor/HRoom.h"
#include "../editor/Erooms.h"
#include "room_external.h"
#include "ned_Geometry.h"
#include "ned_OrthoWnd.h"
#include "ned_PerspWnd.h"
#include "neweditor.h"
#include "RoomFrm.h"
#include "ned_Object.h"
#include "object.h"
#include "ExtrudeDialog.h"
#include "EditLineDialog.h"
#include "ned_HFile.h"
#include "gamepath.h"
#include "../editor/EPath.h"
#include "ned_PathNode.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

short vertnums[MAX_VERTS_PER_ROOM];
short facenums[MAX_FACES_PER_ROOM];

/////////////////////////////////////////////////////////////////////////////
// CRoomFrm

IMPLEMENT_DYNCREATE(CRoomFrm, CMDIChildWnd)

CRoomFrm::CRoomFrm()
{
	m_PanesInitted = false;
	m_Focused_pane = -1;
	m_Room_changed = false;
	m_State_changed = true;
	memset(&m_Prim,0,sizeof(m_Prim));
	memset(&m_Textures_in_use,0,MAX_TEXTURES*sizeof(ushort));
	m_Prim.roomp = NULL;
	m_Prim.portal = -1;
	memset(&m_vec_InsertPos,0,sizeof(vector));
	memset(&m_vec_RefPos,0,sizeof(vector));
	memset(&m_vec_MousePos,0,sizeof(vector));
	memset(&m_Vert_marks,0,MAX_VERTS_PER_ROOM*sizeof(bool));
	m_Num_marked_verts = 0;
	memset(&m_Face_marks,0,MAX_FACES_PER_ROOM*sizeof(bool));
	m_Num_marked_faces = 0;
	memset(&vertnums,0,MAX_VERTS_PER_ROOM*sizeof(short));
	memset(&facenums,0,MAX_FACES_PER_ROOM*sizeof(short));
	m_bHandle_Nodes = false;
	m_bAutoCenter = true;
	m_bSmoothCenter = true;
	m_Modified = false;
	m_Current_nif_vert = -1;
	m_ExtrudeTimer = 0;
	memset(m_Path,0,sizeof(m_Path));
	m_Path[0] = '\0';
}

CRoomFrm::~CRoomFrm()
{
}

BOOL CRoomFrm::OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext)
{
	SIZE size = {200, 200};

	// TODO: Add your specialized code here and/or call the base class
	m_wndSplitter.CreateStatic(this, 2, 2);
	m_wndSplitter.CreateView(0, 0, m_wndXY.GetRuntimeClass(), size, pContext);
	m_wndSplitter.CreateView(0, 1, m_wndXZ.GetRuntimeClass(), size, pContext);
	m_wndSplitter.CreateView(1, 0, m_wndZY.GetRuntimeClass(), size, pContext);
	m_wndSplitter.CreateView(1, 1, m_wnd3D.GetRuntimeClass(), size, pContext);
	m_wndSplitter.SetRowInfo(0, 200, 0);
	m_wndSplitter.SetRowInfo(1, 200, 0);
	m_wndSplitter.SetColumnInfo(0, 200, 0);
	m_wndSplitter.SetColumnInfo(1, 200, 0);

	return CMDIChildWnd::OnCreateClient(lpcs, pContext);
}


BEGIN_MESSAGE_MAP(CRoomFrm, CMDIChildWnd)
	//{{AFX_MSG_MAP(CRoomFrm)
	ON_COMMAND(ID_UNMARK_ALL, OnUnmarkAll)
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_KILLFOCUS()
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_WM_CHAR()
	ON_COMMAND(ID_SHOW_NORMALS, OnShowNormals)
	ON_UPDATE_COMMAND_UI(ID_SHOW_NORMALS, OnUpdateShowNormals)
	ON_COMMAND(ID_SHOW_VERTS, OnShowVerts)
	ON_UPDATE_COMMAND_UI(ID_SHOW_VERTS, OnUpdateShowVerts)
	ON_COMMAND(ID_SHOW_GRID, OnShowGrid)
	ON_UPDATE_COMMAND_UI(ID_SHOW_GRID, OnUpdateShowGrid)
	ON_COMMAND(ID_SHOW_ATCH_ROOMS, OnShowAtchRooms)
	ON_UPDATE_COMMAND_UI(ID_SHOW_ATCH_ROOMS, OnUpdateShowAtchRooms)
	ON_COMMAND(ID_CENTER_ROOM, OnCenterRoom)
	ON_COMMAND(ID_CENTER_ORIGIN, OnCenterOrigin)
	ON_COMMAND(ID_GRID_SIZE_1, OnGridSize1)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_1, OnUpdateGridSize1)
	ON_COMMAND(ID_GRID_SIZE_2, OnGridSize2)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_2, OnUpdateGridSize2)
	ON_COMMAND(ID_GRID_SIZE_3, OnGridSize3)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_3, OnUpdateGridSize3)
	ON_COMMAND(ID_GRID_SIZE_4, OnGridSize4)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_4, OnUpdateGridSize4)
	ON_COMMAND(ID_GRID_SIZE_5, OnGridSize5)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_5, OnUpdateGridSize5)
	ON_COMMAND(ID_TEXTURED, OnTextured)
	ON_UPDATE_COMMAND_UI(ID_TEXTURED, OnUpdateTextured)
	ON_COMMAND(ID_WIREFRAME, OnWireframe)
	ON_UPDATE_COMMAND_UI(ID_WIREFRAME, OnUpdateWireframe)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTFACE, OnViewMoveCameraToCurrentFace)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVECAMERATOCURRENTFACE, OnUpdateViewMoveCameraToCurrentFace)
	ON_UPDATE_COMMAND_UI(ID_CENTER_ROOM, OnUpdateCenterRoom)
	ON_UPDATE_COMMAND_UI(ID_CENTER_ORIGIN, OnUpdateCenterOrigin)
	ON_COMMAND(ID_CENTER_FACE, OnCenterFace)
	ON_WM_CLOSE()
	ON_COMMAND(ID_SHOW_OBJECTS, OnShowObjects)
	ON_UPDATE_COMMAND_UI(ID_SHOW_OBJECTS, OnUpdateShowObjects)
	ON_COMMAND(ID_MODE_VERTEX, OnModeVertex)
	ON_COMMAND(ID_MODE_FACE, OnModeFace)
	ON_COMMAND(ID_MODE_OBJECT, OnModeObject)
	ON_UPDATE_COMMAND_UI(ID_MODE_VERTEX, OnUpdateModeVertex)
	ON_UPDATE_COMMAND_UI(ID_MODE_FACE, OnUpdateModeFace)
	ON_UPDATE_COMMAND_UI(ID_MODE_OBJECT, OnUpdateModeObject)
	ON_COMMAND(ID_MARK_TOGGLE, OnMarkToggle)
	ON_COMMAND(ID_MARK_ALL, OnMarkAll)
	ON_COMMAND(ID_INVERT_MARKINGS, OnInvertMarkings)
	ON_COMMAND(ID_EDIT_INSERT, OnEditInsert)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_GRID_SIZE_0, OnGridSize0)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_0, OnUpdateGridSize0)
	ON_COMMAND(ID_SNAP_TO_GRID, OnSnapToGrid)
	ON_UPDATE_COMMAND_UI(ID_SNAP_TO_GRID, OnUpdateSnapToGrid)
	ON_COMMAND(ID_DEFAULT_EXTRUDE, OnDefaultExtrude)
	ON_UPDATE_COMMAND_UI(ID_MARK_ALL, OnUpdateMarkAll)
	ON_UPDATE_COMMAND_UI(ID_MARK_TOGGLE, OnUpdateMarkToggle)
	ON_UPDATE_COMMAND_UI(ID_INVERT_MARKINGS, OnUpdateInvertMarkings)
	ON_UPDATE_COMMAND_UI(ID_UNMARK_ALL, OnUpdateUnmarkAll)
	ON_COMMAND(ID_GRID_SIZE_6, OnGridSize6)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_6, OnUpdateGridSize6)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateEditCopy)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_INSERT, OnUpdateEditInsert)
	ON_UPDATE_COMMAND_UI(ID_EDIT_DELETE, OnUpdateEditDelete)
	ON_COMMAND(ID_GRID_SIZE_CUSTOM, OnGridSizeCustom)
	ON_UPDATE_COMMAND_UI(ID_GRID_SIZE_CUSTOM, OnUpdateGridSizeCustom)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTOBJECT, OnViewMoveCameraToCurrentObject)
	ON_COMMAND(ID_FILE_SHOWROOMSTATS, OnFileShowRoomStats)
	ON_COMMAND(ID_VIEW_TEXTURED_OUTLINE, OnViewTexturedOutline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTURED_OUTLINE, OnUpdateViewTexturedOutline)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_SHOWROOMCENTER, OnShowRoomCenter)
	ON_UPDATE_COMMAND_UI(ID_SHOWROOMCENTER, OnUpdateShowRoomCenter)
	ON_WM_TIMER()
	ON_COMMAND(ID_MODE_PATH, OnModePath)
	ON_UPDATE_COMMAND_UI(ID_MODE_PATH, OnUpdateModePath)
	ON_COMMAND(ID_EDIT_PASTEONTOP, OnEditPasteOnTop)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTEONTOP, OnUpdateEditPasteOnTop)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTROOM, OnViewMoveCameraToCurrentRoom)
	ON_UPDATE_COMMAND_UI(ID_VIEW_MOVECAMERATOCURRENTOBJECT, OnUpdateViewMoveCameraToCurrentObject)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateEditCut)
	ON_COMMAND(ID_PREV_PANE, OnPrevPane)
	ON_COMMAND(ID_NEXT_PANE, OnNextPane)
	//}}AFX_MSG_MAP
	ON_NOTIFY_EX(TTN_NEEDTEXT, 0, OnToolTipNotify)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoomFrm message handlers

#include "TextureDialogBar.h"
extern CTextureDialogBar *dlgTextureDialogBar;

void CRoomFrm::OnSetFocus(CWnd* pOldWnd) 
{
	//m_wndSplitter.GetPane(1,1);
//	if(IsWindow(m_wndSplitter.GetPane(1,1)->m_hWnd))
//		m_wndSplitter.GetPane(1,1)->SetFocus();
	//CMDIChildWnd::OnSetFocus(pOldWnd);	
	theApp.m_pFocusedRoomFrm = this;
	prim *prim = theApp.AcquirePrim();

	if (m_PanesInitted)
	{
		//@@dlgTextureDialogBar->RoomReplaceTexStrings(m_Textures_in_use);
		dlgTextureDialogBar->SetCurrentFace(ROOMNUM(prim->roomp),prim->face);
//		dlgTextureDialogBar->SetCurrentTexture(0);
	}
}

void CRoomFrm::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	CWnd *wnd;

	if (m_PanesInitted)
	{
		wnd = m_wndSplitter.GetPane(0,0);
		if(IsWindow(wnd->m_hWnd))
			((Cned_OrthoWnd *) wnd)->Render();
		wnd = m_wndSplitter.GetPane(0,1);
		if(IsWindow(wnd->m_hWnd))
			((Cned_OrthoWnd *) wnd)->Render();
		wnd = m_wndSplitter.GetPane(1,0);
		if(IsWindow(wnd->m_hWnd))
			((Cned_OrthoWnd *) wnd)->Render();
		wnd = m_wndSplitter.GetPane(1,1);
		if(IsWindow(wnd->m_hWnd))
			((Cned_PerspWnd *) wnd)->Render();
	}

	// Do not call CMDIChildWnd::OnPaint() for painting messages
}


void CRoomFrm::InitPanes()
{
	//One time initialization because during the OnCreateClient call
	//the windows don't exist yet.
	CWnd *wnd;
	m_PanesInitted = true;

	// Enable tooltips for this window
	EnableToolTips(TRUE);
	// Create tooltip for this window
	m_TTip.Create(this);

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		InitOrthoPane((Cned_OrthoWnd *)wnd,"Front (XY)",VIEW_XY);
	else
		m_PanesInitted = false;

	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		InitOrthoPane((Cned_OrthoWnd *)wnd,"Top (XZ)",VIEW_XZ);
	else
		m_PanesInitted = false;

	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		InitOrthoPane((Cned_OrthoWnd *)wnd,"Side (ZY)",VIEW_ZY);
	else
		m_PanesInitted = false;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		InitPerspPane((Cned_PerspWnd *)wnd,"Perspective",VIEW_3D);
	else
		m_PanesInitted = false;

}


void CRoomFrm::InitOrthoPane(Cned_OrthoWnd *wnd, char *title, int id)
{
	wnd->m_pParentFrame = this;
	strncpy(wnd->m_Title,title,sizeof(wnd->m_Title));
	wnd->m_nID = id;
	wnd->m_pPrim = &m_Prim;
	if (this == theApp.m_pRoomFrm)
		wnd->m_pPrim = &theApp.m_pLevelWnd->m_Prim;
	wnd->InitCamera();
	wnd->m_pvec_InsertPos = &m_vec_InsertPos;
	wnd->m_pvec_RefPos = &m_vec_RefPos;
	wnd->m_pvec_MousePos = &m_vec_MousePos;
	wnd->m_pNum_marked_verts = &m_Num_marked_verts;
	wnd->m_pVert_marks = m_Vert_marks;
	wnd->m_pNum_marked_faces = &m_Num_marked_faces;
	wnd->m_pFace_marks = m_Face_marks;
	wnd->CenterRoom(wnd->m_pPrim->roomp);
	// Register window as a tool
	m_TTip.AddTool(wnd,LPSTR_TEXTCALLBACK,NULL,0);
}


void CRoomFrm::InitPerspPane(Cned_PerspWnd *wnd, char *title, int id)
{
	wnd->m_pParentFrame = this;
	strncpy(wnd->m_Title,title,sizeof(wnd->m_Title));
	wnd->m_nID = id;
	wnd->m_pPrim = &m_Prim;
	if (this == theApp.m_pRoomFrm)
		wnd->m_pPrim = &theApp.m_pLevelWnd->m_Prim;
	wnd->InitCamera();
	wnd->m_pvec_InsertPos = &m_vec_InsertPos;
	wnd->m_pvec_RefPos = &m_vec_RefPos;
//	wnd->m_pvec_MousePos = &m_vec_MousePos;
	wnd->m_pNum_marked_verts = &m_Num_marked_verts;
	wnd->m_pVert_marks = m_Vert_marks;
	wnd->m_pNum_marked_faces = &m_Num_marked_faces;
	wnd->m_pFace_marks = m_Face_marks;
	wnd->CenterRoom(wnd->m_pPrim->roomp);
	wnd->m_bInitted = true;
}


void CRoomFrm::UpdateAllPanes()
{
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->m_State_changed = true;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->m_State_changed = true;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->m_State_changed = true;
	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->m_State_changed = true;
	
}



void CRoomFrm::OnDestroy() 
{

	CMDIChildWnd::OnDestroy();

	// TODO: Add your message handler code here
}



void CRoomFrm::SetPrim(prim *prim)
{
	if (this != theApp.m_pRoomFrm)
	{
		ASSERT(prim->portal == -1);
		prim->portal = -1;
		m_Prim = *prim;
	}
	else
		theApp.m_pLevelWnd->SetPrim(prim);

	m_State_changed = true;
}


void CRoomFrm::SetPrim(room *rp, int face, int portal, int edge, int vert)
{
	if (this != theApp.m_pRoomFrm)
	{
		m_Prim.roomp = rp;
		m_Prim.face = face;
		ASSERT(portal == -1);
		m_Prim.portal = portal;
		m_Prim.edge = edge;
		m_Prim.vert = vert;

		if (rp->num_verts)
		{
			 if (vert != -1)
				m_Current_nif_vert = -1;
		}
		else
		{
			ASSERT(vert == -1);
			m_Current_nif_vert = -1;
		}

	}
	else
		theApp.m_pLevelWnd->SetPrim(rp,face,portal,edge,vert);

	m_State_changed = true;
}

void CRoomFrm::InitTex()
{
	// Can't do this unfortunately:
//	if (this == theApp.m_pRoomFrm)
//		m_Textures_in_use = Level_textures_in_use;

	if (this != theApp.m_pRoomFrm) // only allow for ORFs
		RoomTexInitRoom(m_Prim.roomp, m_Textures_in_use);
}

void CRoomFrm::ShutdownTex()
{
	// Reset texture dialog bar's room/face members
	Editor_state.SetCurrentRoomFaceTexture(-1, -1);

	if (this != theApp.m_pRoomFrm) // only allow for ORFs
		RoomTexPurgeAllTextures(m_Textures_in_use);
}

void CRoomFrm::OnKillFocus(CWnd* pNewWnd) 
{
	CMDIChildWnd::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	// NOTE: do not set theApp.m_pFocusedRoomFrm = NULL !!!
//	dlgTextureDialogBar->SetCurrentTexture(0);
}


bool CRoomFrm::IsVertMarked(int vertnum)
{
	ASSERT(vertnum != -1);
	if (vertnum != -1)
		return m_Vert_marks[vertnum];
	return false;
}


void CRoomFrm::MarkVert(int vertnum)
{
	if (!m_Vert_marks[vertnum])
	{
		m_Vert_marks[vertnum] = true;
		m_Num_marked_verts++;
	}

	m_State_changed = true;
}


void CRoomFrm::UnMarkVert(int vertnum)
{
	if (m_Vert_marks[vertnum])
	{
		m_Vert_marks[vertnum] = false;
		m_Num_marked_verts--;
	}

	m_State_changed = true;
}


void CRoomFrm::MarkVertsInFace()
{
	prim *prim = theApp.AcquirePrim();
	face *fp = &prim->roomp->faces[prim->face];

	for (int i=0; i<fp->num_verts; i++)
		MarkVert(fp->face_verts[i]);
}

void CRoomFrm::UnMarkVertsInFace()
{
	prim *prim = theApp.AcquirePrim();
	face *fp = &prim->roomp->faces[prim->face];

	for (int i=0; i<fp->num_verts; i++)
		UnMarkVert(fp->face_verts[i]);
}


void CRoomFrm::MarkFace(int facenum)
{
	if (!m_Face_marks[facenum])
	{
		m_Face_marks[facenum] = true;
		m_Num_marked_faces++;
	}

	m_State_changed = true;
}


void CRoomFrm::UnMarkFace(int facenum)
{
	if (m_Face_marks[facenum])
	{
		m_Face_marks[facenum] = false;
		m_Num_marked_faces--;
	}

	m_State_changed = true;
}


int CRoomFrm::GetMarkedVerts(short *list)
{
	room *rp = theApp.AcquirePrim()->roomp;
	int j = 0;
	for (int i=0; i<rp->num_verts; i++)
	{
		if (m_Vert_marks[i])
		{
			list[j] = i;
			j++;
		}
	}
	return j;
}


int CRoomFrm::GetMarkedFaces(short *list)
{
	room *rp = theApp.AcquirePrim()->roomp;
	int j = 0;
	for (int i=0; i<rp->num_faces; i++)
	{
		if (m_Face_marks[i])
		{
			list[j] = i;
			j++;
		}
	}
	return j;
}


void CRoomFrm::CopyVerts(bool unmark) 
{
	room *rp = theApp.AcquirePrim()->roomp;
	int i;

	// Get list of marked verts
	int num_m_verts = GetMarkedVerts(vertnums);
	ASSERT(num_m_verts == m_Num_marked_verts);

	// Copy the vertices into the buffer
	for (i=0; i<num_m_verts; i++)
	{
		Vert_clipboard[i].vertex = rp->verts[vertnums[i]];
		Vert_clipboard[i].orignum = vertnums[i];
	}

	if (unmark)
	{
		// Unmark the verts
		for (i=0; i<num_m_verts; i++)
			UnMarkVert(vertnums[i]);
	}

	Num_clipboard_verts = num_m_verts;
}

void CRoomFrm::DeleteMarkedFaces() 
{
	prim *prim = theApp.AcquirePrim();
	room *rp = prim->roomp;
	int vert_idx, vcount = 0;

	// Get lists of marked faces and verts
	int num_m_faces = GetMarkedFaces(facenums);
	ASSERT(num_m_faces == m_Num_marked_faces);
	int num_m_verts = GetMarkedVerts(vertnums);
	ASSERT(num_m_verts == m_Num_marked_verts);

	for (int i=0; i<num_m_faces; i++)
		if (rp->faces[facenums[i]].portal_num != -1) {
			OutrageMessageBox("You cannot delete a face that is part of a portal.");
			return;
		}

	if (num_m_faces == rp->num_faces && ROOMNUM(rp) < MAX_ROOMS)
	{
		OutrageMessageBox("You cannot delete the only face in a level room. Unmark at least one face and try again.\n\n"
			"If you wish to delete the room itself, choose Delete Current Room from the World View's Room menu");
		return;
	}

	// If the current face is marked, it will be deleted, so update the current face/texture displays
	for (int i=0; i<num_m_faces; i++)
	{
		if (facenums[i] == prim->face)
		{
			Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), -1);
			break;
		}
	}

	while (m_Num_marked_faces != 0)
	{
		// Get list of marked faces, because face indices change each time through loop
		num_m_faces = GetMarkedFaces(facenums);
		// Get list of marked verts, because vertex indices change each time through loop
		int num_m_verts = GetMarkedVerts(vertnums);
		ASSERT(num_m_verts == m_Num_marked_verts);
		// Delete the first face in list, and unmark it
		int facenum = facenums[0];
		// Unmark the face and any verts that are part of the face
		for (int i=0; i<rp->faces[facenum].num_verts; i++)
		{
			vert_idx = rp->faces[facenum].face_verts[i];
			if ( m_Vert_marks[vert_idx] )
			{
				UnMarkVert(vert_idx);
				vcount++;
			}
		}
		UnMarkFace(facenum);

		// Delete the bastid
		DeleteRoomFace(rp,facenum);

		// Adjust current primitives
		int portnum,edgenum,vertnum;
		if (prim->face >= rp->num_faces)
			prim->face = rp->num_faces - 1;
		if (prim->face != -1)
		{
			portnum = rp->faces[prim->face].portal_num;
			edgenum = vertnum = 0;
		}
		else
		{
			portnum = -1;
			edgenum = vertnum = -1;
		}
		SetPrim(rp,prim->face,portnum,edgenum,vertnum);
		int num_faces = rp->num_faces;
		int j;
		for (j=facenum; j<num_faces; j++)
			m_Face_marks[j] = m_Face_marks[j+1];
		// Now zero out the unused element in the array (IMPORTANT)
		m_Face_marks[j] = 0;
	}

	// Get count of marked verts again and compare for sanity's sake
	int num_m_verts_now = GetMarkedVerts(vertnums);
	ASSERT(num_m_verts_now == (num_m_verts-vcount));

	// Update the current face/texture displays again
	Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), prim->face);
}

void DeleteRoomVert(room *rp,int vertnum);
int VertInFace(room *rp, int facenum, int vertnum);

void CRoomFrm::DeleteMarkedVerts()
{
	prim *prim = theApp.AcquirePrim();
	room *rp = prim->roomp;
	face *fp;
	int f,v;

	// Get list of marked verts
	int num_m_verts = GetMarkedVerts(vertnums);
	ASSERT(num_m_verts == m_Num_marked_verts);

	for (int i=0; i<num_m_verts; i++)
	{
		for (int j=0; j<rp->num_faces; j++)
		{
			if (VertInFace(rp,j,vertnums[i]) != -1)
			{
				OutrageMessageBox("Cannot perform operation. One or more marked verts belong to faces.");
					return;
			}
		}
	}

	while (m_Num_marked_verts != 0)
	{
top:
		// Get list of marked verts, because vertex indices change each time through loop
		num_m_verts = GetMarkedVerts(vertnums);
		if (!num_m_verts)
			continue;
		// Delete the first vertex in list, and unmark it
		int vertnum = vertnums[0];
		// Make sure it's not in use (part of a face)
		for (f=0,fp=rp->faces;f<rp->num_faces;f++,fp++)
			for (v=0;v<fp->num_verts;v++)
				if (fp->face_verts[v] == vertnum)
				{
					OutrageMessageBox("Can't delete vertex %d because it's in use by a face.", vertnum);
					UnMarkVert(vertnum);
					goto top;
				}
		UnMarkVert(vertnum);
		DeleteRoomVert(rp,vertnum);
		// Adjust current vert if necessary
		if (prim->vert >= rp->num_verts)
			prim->vert = vertnum - 1;
		if (!rp->num_verts && prim->vert == -1)
			m_Current_nif_vert = -1;
		int num_verts = rp->num_verts;
		int j;
		for (j=vertnum; j<num_verts; j++)
			m_Vert_marks[j] = m_Vert_marks[j+1];
		// Now zero out the unused element in the array (IMPORTANT)
		m_Vert_marks[j] = 0;
		num_m_verts--;
	}

	ASSERT(num_m_verts == m_Num_marked_verts);
}

bool CRoomFrm::MergeMarkedVerts()
{
	room *rp = theApp.AcquirePrim()->roomp;
//	short m_vertnums[MAX_VERTS_PER_ROOM];
	int i,j,k,m;

	// Get list of marked verts
	int num_m_verts = GetMarkedVerts(vertnums);
	ASSERT(num_m_verts == m_Num_marked_verts);

/*
	// Record marked states
	// Unmark verts so that we can mark them for deletion if necessary
	for (i=0; i<num_m_verts; i++)
	{
		m_vertnums[i] = vertnums[i];
		UnMarkVert(i);
	}
*/

	// Unmark verts so that we can mark them for deletion if necessary
	for (i=0; i<num_m_verts; i++)
		UnMarkVert(vertnums[i]);

	for (i=0; i<num_m_verts; i++)
	{
		for (j=i+1; j<num_m_verts; j++)
		{
			if (vertnums[j] > rp->num_verts-1)
				continue;
			if (rp->verts[vertnums[i]] == rp->verts[vertnums[j]])
			{
				for (k=0; k<rp->num_faces; k++)
				{
					for (m=0; m<rp->faces[k].num_verts; m++)
					{
						if (rp->faces[k].face_verts[m] == vertnums[j])
							rp->faces[k].face_verts[m] = vertnums[i];
					}
				}
				// Mark vert for deletion
				MarkVert(vertnums[j]);
			}
		}
	}

	DeleteMarkedVerts();

	return true;
}

void CRoomFrm::OnEditPaste() 
{
	// TODO: Add your command handler code here
	if (g_bVerts)
		PasteVerts(true,false);
	if (g_bFaces)
		PasteFaces(true,false);

	UpdateAllPanes();
}

void CRoomFrm::PasteVerts(bool mark,bool on_top)
{
	room *rp = theApp.AcquirePrim()->roomp;
	int i;
	vector center = {0,0,0};
	vector *c = &center;
	vector *min_x,*max_x,*min_y,*max_y,*min_z,*max_z,*vp;
	float dx,dy,dz;
	float _rad,_rad2;

	// Add copy of verts to room
	int vertnum = RoomAddVertices(rp,Num_clipboard_verts);
	for (i=0; i<Num_clipboard_verts; i++)
		rp->verts[vertnum+i] = Vert_clipboard[i].vertex;

	// Update their positions to the insert position
	if (!on_top)
	{
		// Copied from ComputeRoomBoundingSphere

		//Initialize min, max vars
		min_x = max_x = min_y = max_y = min_z = max_z = &rp->verts[vertnum];

		//First, find the points with the min & max x,y, & z coordinates
		for (i=vertnum,vp=&rp->verts[vertnum];i<vertnum+Num_clipboard_verts;i++,vp++) {

			if (vp->x < min_x->x)
				min_x = vp;

			if (vp->x > max_x->x)
				max_x = vp;

			if (vp->y < min_y->y)
				min_y = vp;

			if (vp->y > max_y->y)
				max_y = vp;

			if (vp->z < min_z->z)
				min_z = vp;

			if (vp->z > max_z->z)
				max_z = vp;
		}

		//Calculate initial sphere

		dx = vm_VectorDistance(min_x,max_x);
		dy = vm_VectorDistance(min_y,max_y);
		dz = vm_VectorDistance(min_z,max_z);

		if (dx > dy)
			if (dx > dz) {
				*c = (*min_x + *max_x) / 2;  _rad = dx / 2;
			}
			else {
				*c = (*min_z + *max_z) / 2;  _rad = dz / 2;
			}
		else
			if (dy > dz) {
				*c = (*min_y + *max_y) / 2;  _rad = dy / 2;
			}
			else {
				*c = (*min_z + *max_z) / 2;  _rad = dz / 2;
			}


		//Go through all points and look for ones that don't fit
		_rad2 = _rad * _rad;
		for (i=vertnum,vp=&rp->verts[vertnum];i<vertnum+Num_clipboard_verts;i++,vp++) {
			vector delta;
			float t2;
			
			delta = *vp - *c;
			t2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;

			//If point outside, make the sphere bigger
			if (t2 > rad2) {
				float t;

				t = sqrt(t2);
				_rad = (_rad + t) / 2;
				_rad2 = _rad * _rad;
				*c += delta * (t - _rad) / t;
			}
		}
		// Move the verts
		for (i=vertnum,vp=&rp->verts[vertnum];i<vertnum+Num_clipboard_verts;i++,vp++)
			*vp += m_vec_InsertPos-*c;
	}

	// Mark the verts
	if (mark)
		for (i=0; i<Num_clipboard_verts; i++)
			MarkVert(vertnum+i);
}

void CRoomFrm::PasteFaces(bool mark,bool on_top)
{
	room *rp = theApp.AcquirePrim()->roomp;
	face *fp;
	int i,j;

	for (i=0; i<Num_clipboard_faces; i++)
	{
		fp = &Face_clipboard[i];
		ASSERT(fp->num_verts > 0);
	}

	// Save number of verts
	int num_verts = rp->num_verts;

	// Add copy of verts to room (but don't mark them)
	PasteVerts(false,on_top);

	// Add copy of faces to room
	int facenum = RoomAddFaces(rp,Num_clipboard_faces);
	for (i=0; i<Num_clipboard_faces; i++)
	{
		fp = &rp->faces[facenum+i];
		CopyFace(fp, &Face_clipboard[i]);
		// Update the face_verts
		for (j=0; j<fp->num_verts; j++)
		{
			fp->face_verts[j] += num_verts;
		}
	}

	// Mark the faces
	if (mark)
		for (i=0; i<Num_clipboard_faces; i++)
			MarkFace(facenum+i);
}

void CRoomFrm::CopyFaces(bool unmark) 
{
	room *rp = theApp.AcquirePrim()->roomp;
	int i,j,k;

	// Get list of marked faces
	int num_m_faces = GetMarkedFaces(facenums);
	ASSERT(num_m_faces == m_Num_marked_faces);

	// Unmark any verts that might be marked (this is necessary because CopyVerts depends on it)
	// TODO : maybe prompt the user first, so they can cancel the action and the entire copy
	int num_m_verts = GetMarkedVerts(vertnums);
	ASSERT(num_m_verts == m_Num_marked_verts);
	for (j=0; j<num_m_verts; j++)
		UnMarkVert(vertnums[j]);

	// Mark all verts in marked faces
	for (i=0; i<num_m_faces; i++)
	{
		face *fp = &rp->faces[facenums[i]];
		for (j=0; j<fp->num_verts; j++)
			MarkVert(fp->face_verts[j]);
	}

	// Copy the verts
	CopyVerts(true);

	// Copy the faces
	for (i=0; i<num_m_faces; i++)
	{
		face *fp = &rp->faces[facenums[i]];
		CopyFace(&Face_clipboard[i], fp);

		// Assign the copied verts to the copied face (face_verts are updated later in PasteFaces)
		for (j=0; j<fp->num_verts; j++)
		{
			for (k=0; k<Num_clipboard_verts; k++)
			{
				if (Vert_clipboard[k].orignum == fp->face_verts[j])
				{
					Face_clipboard[i].face_verts[j] = k;
					break;
				}
			}
		}
	}

	if (unmark)
	{
		// Unmark the faces
		for (i=0; i<num_m_faces; i++)
			UnMarkFace(facenums[i]);
	}

	Num_clipboard_faces = num_m_faces;
}


void CRoomFrm::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	// TODO : figure out why this is only called when frame window is refocused
	CWnd *wnd;

	switch (nChar)
	{
	case 0x31:		// '1'
		wnd = m_wndSplitter.GetPane(0,0);
		if(IsWindow(wnd->m_hWnd))
			wnd->SetFocus();
		break;

	case 0x32:		// '2'
		wnd = m_wndSplitter.GetPane(0,1);
		if(IsWindow(wnd->m_hWnd))
			wnd->SetFocus();
		break;

	case 0x33:		// '3'
		wnd = m_wndSplitter.GetPane(1,0);
		if(IsWindow(wnd->m_hWnd))
			wnd->SetFocus();
		break;

	case 0x34:		// '4'
		wnd = m_wndSplitter.GetPane(1,1);
		if(IsWindow(wnd->m_hWnd))
			wnd->SetFocus();
		break;
	}

	CMDIChildWnd::OnChar(nChar, nRepCnt, nFlags);
}

void CRoomFrm::CycleEdge(prim *prim,int which)
{
	room *rp = prim->roomp;

	if (prim->face != -1 && prim->edge != -1)
	{
	prim->vert = 0;
	if (which == PREV)
		(prim->edge <= 0) ? (prim->edge = rp->faces[prim->face].num_verts-1) : (prim->edge--);
	else if (which == NEXT)
		(prim->edge == rp->faces[prim->face].num_verts-1) ? (prim->edge = 0) : (prim->edge++);
	else
		Int3();
	m_State_changed = true;
	}
	else
		OutrageMessageBox("You must have a current face to use this function.");
}

void CRoomFrm::CycleFace(prim *prim,int which)
{
	room *rp = prim->roomp;

	if (rp->num_faces > 0)
	{
	int facenum = prim->face;
	if (which == PREV)
		(facenum <= 0) ? (facenum = rp->num_faces-1) : (facenum--);
	else if (which == NEXT)
		(facenum == rp->num_faces-1) ? (facenum = 0) : (facenum++);
	else
		Int3();
	SetPrim(rp,facenum,rp->faces[facenum].portal_num,0,0);
	// Update the current face/texture displays
	Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), prim->face);
	}
	else
		MessageBox("No faces in this room!");
}

void CRoomFrm::CyclePortal(prim *prim,int which)
{
	room *rp = prim->roomp;

	if (rp->num_portals > 0)
	{
		int portnum = prim->portal;
		if (which == PREV)
			(portnum <= 0) ? (portnum = rp->num_portals-1) : (portnum--);
		else if (which == NEXT)
			(portnum == rp->num_portals-1) ? (portnum = 0) : (portnum++);
		else
			Int3();
		SetPrim(rp,rp->portals[portnum].portal_face,portnum,0,0);
		// Update the current face/texture displays
		Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), prim->face);
	}
	else
		PrintStatus("There are no portals in the current room");
}

void CRoomFrm::CycleRoom(prim *prim,int which)
{
	ASSERT(prim == &theApp.m_pLevelWnd->m_Prim);

	room *rp = prim->roomp;

try_again:
	if (Highest_room_index != 0)
	{
		// Force unmark everything
		ForceUnmarkAll();

		if (which == PREV)
			(ROOMNUM(rp) == 0) ? (rp = &Rooms[Highest_room_index]) : (rp = --prim->roomp);
		else if (which == NEXT)
			(ROOMNUM(rp) == Highest_room_index) ? (rp = &Rooms[0]) : (rp = ++prim->roomp);
		else
			Int3();
		if (!rp->used)
			goto try_again;
		SetPrim(rp,0,rp->faces[0].portal_num,0,0);
		// Update the current face/texture displays
		Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), prim->face);
		// If this room is a door room, set the current doorway
		Editor_state.SetCurrentDoorway(rp);
	}
	else
		OutrageMessageBox("Sorry, but there are no other rooms.");
}

void CRoomFrm::CycleVert(prim *prim,int which)
{
	room *rp = prim->roomp;
	int vert_idx;
	float x,y,z;

	if (prim->face != -1 && prim->edge != -1 && prim->vert != -1)
	{
		if (which == PREV)
			(prim->vert <= 0) ? (prim->vert = rp->faces[prim->face].num_verts-1) : (prim->vert--);
		else if (which == NEXT)
			(prim->vert == rp->faces[prim->face].num_verts-1) ? (prim->vert = 0) : (prim->vert++);
		else
			Int3();
		m_State_changed = true;
		vert_idx = rp->faces[prim->face].face_verts[prim->vert];
		x = rp->verts[vert_idx].x; y = rp->verts[vert_idx].y; z = rp->verts[vert_idx].z;
		PrintStatus("Face %d vert %d selected. Position: %.2f, %.2f, %.2f",prim->face,prim->vert,x,y,z);
	}
/*
	else if (m_Current_nif_vert != -1)
	{
		// TODO : cycle through non-face verts?
		// PSEUDOCODE
		if (which == PREV)
			(m_Current_nif_vert == 0) ? (m_Current_nif_vert = last_non_face_vert) : (prev_non_face_vert);
		else if (which == NEXT)
			(m_Current_nif_vert == last_non_face_vert) ? (m_Current_nif_vert = first_non_face_vert) : (next_non_face_vert);
		else
			Int3();
		m_State_changed = true;
	}
*/
	else
		OutrageMessageBox("You must have a current face to use this function."); // temp
}

void CRoomFrm::CycleObject(prim *prim,int which)
{
	ASSERT(prim == &theApp.m_pLevelWnd->m_Prim);

	room *rp = prim->roomp;
	int next;

	switch (which)
	{
	case PREV:
		// TODO : select previous object
prev_object:
		next = 0;
		SelectObject(next);
		m_State_changed = true;
		// Update the current object display
		Editor_state.SetCurrentLevelObject(Cur_object_index);
		if (Objects[Cur_object_index].type == OBJ_DOOR)
			goto prev_object;
		break;

	case NEXT:
next_object:
		if (Objects[Cur_object_index].roomnum != ROOMNUM(rp)) {
			next = rp->objects;
			if (next == -1) {
				PrintStatus("There are no objects in the current room");
				return;
			}
		}
		else {
			next = Objects[Cur_object_index].next;
			if (next == -1) {
				next = rp->objects;
				if (next == Cur_object_index) {
					MessageBox("There are no other objects in this room.");
					return;
				}
			}
		}
		SelectObject(next);
		m_State_changed = true;
		// Update the current object display
		Editor_state.SetCurrentLevelObject(Cur_object_index);
		if (Objects[Cur_object_index].type == OBJ_DOOR)
			goto next_object;
		break;

	default:
		Int3();
		break;
	}
}

void CRoomFrm::CyclePath(int which)
{
	int path,node,room,edge;

	if (Editor_state.GetCurrentNodeType() == NODETYPE_GAME)
	{
		if (Num_game_paths<1)
			return;
		path = Editor_state.GetCurrentPath();
		node = 0;

		switch (which)
		{
		case PREV:
			path = GetPrevPath(path);
			break;

		case NEXT:
			path = GetNextPath(path);
			break;

		default:
			Int3();
			break;
		}

		Editor_state.SetCurrentPath(path);
		Editor_state.SetCurrentNode(node);

		if (path != -1)
			PrintStatus("Path %d, Node %d selected. Path Name = %s",path,node+1,GamePaths[path].name);
		m_State_changed = true;
	}
	else
	{
		Editor_state.GetCurrentBNode(&room,&node,&edge);
		if (Rooms[room].bn_info.num_nodes<1)
			return;

		switch (which)
		{
		case PREV:
			edge--;
			if (edge<0)
				edge=Rooms[room].bn_info.nodes[node].num_edges-1;
			break;

		case NEXT:
			edge++;
			edge%=Rooms[room].bn_info.nodes[node].num_edges;
			break;

		default:
			Int3();
			break;
		}

		Editor_state.SetCurrentBNode(room,node,edge);

		PrintStatus("Edge %d of Node %d selected.",edge,node+1);
		m_State_changed = true;
	}
}

void CRoomFrm::CycleNode(int which)
{
	int path,node,room,edge;

	if (Editor_state.GetCurrentNodeType() == NODETYPE_GAME)
	{
		if (Num_game_paths<1)
			return;
		path = Editor_state.GetCurrentPath();
		node = Editor_state.GetCurrentNode();

		switch (which)
		{
		case PREV:
			node--;
			if (node<0)
				node=GamePaths[path].num_nodes-1;
			break;

		case NEXT:
			node++;
			node%=GamePaths[path].num_nodes;
			break;

		default:
			Int3();
			break;
		}

		Editor_state.SetCurrentNode(node);

		if (path != -1)
			PrintStatus("Path %d, Node %d selected. Path Name = %s",path,node+1,GamePaths[path].name);
		m_State_changed = true;
	}
	else
	{
		Editor_state.GetCurrentBNode(&room,&node,&edge);
		if (Rooms[room].bn_info.num_nodes<1)
			return;

		switch (which)
		{
		case PREV:
			node--;
			if (node<0)
				node=Rooms[room].bn_info.num_nodes-1;
			break;

		case NEXT:
			node++;
			node%=Rooms[room].bn_info.num_nodes;
			break;

		default:
			Int3();
			break;
		}

		Editor_state.SetCurrentNode(node);

		PrintStatus("Room %d, BNode %d selected",room,node+1);
		m_State_changed = true;
	}
}

void CRoomFrm::OnShowNormals() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;
	bool bShow = false;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowNormals)
			bShow = true;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowNormals)
			bShow = true;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowNormals)
			bShow = true;

	if (bShow)
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowNormals = true;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowNormals = true;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowNormals = true;
	}
	else
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowNormals = false;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowNormals = false;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowNormals = false;
	}

	m_State_changed = true;
}

void CRoomFrm::OnUpdateShowNormals(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowNormals)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowNormals)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowNormals)
			bCheck = false;

	pCmdUI->SetCheck(bCheck);
}

void CRoomFrm::OnShowVerts() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;
	bool bShow = false;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowVerts)
			bShow = true;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowVerts)
			bShow = true;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowVerts)
			bShow = true;

	if (bShow)
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowVerts = true;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowVerts = true;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowVerts = true;
	}
	else
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowVerts = false;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowVerts = false;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowVerts = false;
	}

	m_State_changed = true;
}

void CRoomFrm::OnUpdateShowVerts(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowVerts)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowVerts)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowVerts)
			bCheck = false;

	pCmdUI->SetCheck(bCheck);
}

void CRoomFrm::OnShowGrid() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;
	bool bShow = false;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridShow)
			bShow = true;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridShow)
			bShow = true;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridShow)
			bShow = true;

	if (bShow)
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bGridShow = true;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bGridShow = true;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bGridShow = true;
	}
	else
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bGridShow = false;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bGridShow = false;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bGridShow = false;
	}

	m_State_changed = true;
}

void CRoomFrm::OnUpdateShowGrid(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridShow)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridShow)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridShow)
			bCheck = false;

	pCmdUI->SetCheck(bCheck);
}

void CRoomFrm::OnShowAtchRooms() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;
	bool bShow = false;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowAttached)
			bShow = true;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowAttached)
			bShow = true;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowAttached)
			bShow = true;

	if (bShow)
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowAttached = true;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowAttached = true;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowAttached = true;
	}
	else
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowAttached = false;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowAttached = false;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowAttached = false;
	}

	m_State_changed = true;
}

void CRoomFrm::OnUpdateShowAtchRooms(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowAttached)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowAttached)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowAttached)
			bCheck = false;

	pCmdUI->SetCheck(bCheck);
}

void CRoomFrm::OnCenterRoom() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterRoom();
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterRoom();
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterRoom();
	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnCenterRoom();
}

void CRoomFrm::OnCenterOrigin() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterOrigin();
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterOrigin();
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterOrigin();
	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnCenterOrigin();
}

void CRoomFrm::GridSize(int num) 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->GridSize(num);
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->GridSize(num);
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->GridSize(num);
}

bool CRoomFrm::UpdateGridSize(int num) 
{
	// TODO: Add your command update UI handler code here
	ASSERT(num >= 0 && num < NUM_GRIDS);

	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (((Cned_OrthoWnd *) wnd)->m_nGrid != num)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (((Cned_OrthoWnd *) wnd)->m_nGrid != num)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (((Cned_OrthoWnd *) wnd)->m_nGrid != num)
			bCheck = false;

	return bCheck;
}

void CRoomFrm::OnGridSize1() 
{
	// TODO: Add your command handler code here
	GridSize(1);
}

void CRoomFrm::OnUpdateGridSize1(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(1));
}

void CRoomFrm::OnGridSize2() 
{
	// TODO: Add your command handler code here
	GridSize(2);
}

void CRoomFrm::OnUpdateGridSize2(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(2));
}

void CRoomFrm::OnGridSize3() 
{
	// TODO: Add your command handler code here
	GridSize(3);
}

void CRoomFrm::OnUpdateGridSize3(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(3));
}

void CRoomFrm::OnGridSize4() 
{
	// TODO: Add your command handler code here
	GridSize(4);
}

void CRoomFrm::OnUpdateGridSize4(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(4));
}

void CRoomFrm::OnGridSize5() 
{
	// TODO: Add your command handler code here
	GridSize(5);
}

void CRoomFrm::OnUpdateGridSize5(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(5));
}

void CRoomFrm::OnTextured() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnTextured();
}

void CRoomFrm::OnUpdateTextured(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		pCmdUI->SetCheck(((Cned_PerspWnd *) wnd)->m_bTextured && !((Cned_PerspWnd *) wnd)->m_bOutline);
}

void CRoomFrm::OnWireframe() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnWireframe();
}

void CRoomFrm::OnUpdateWireframe(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		if (((Cned_PerspWnd *) wnd)->m_bTextured)
			bCheck = false;

	pCmdUI->SetCheck(bCheck);
}

void CRoomFrm::OnViewMoveCameraToCurrentFace() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnViewMoveCameraToCurrentFace();
}

void CRoomFrm::OnUpdateViewMoveCameraToCurrentFace(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable;

	m_Prim.face != -1 ? enable = true : enable = false;
	pCmdUI->Enable(enable);
}

void CRoomFrm::OnUpdateCenterRoom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool bEnable = false;
	room *rp = theApp.AcquirePrim()->roomp;

	if (rp != NULL && ROOMNUM(rp) >= 0 && ROOMNUM(rp) <= MAX_ROOMS+MAX_PALETTE_ROOMS)
		bEnable = true;

	pCmdUI->Enable(bEnable);
}

void CRoomFrm::OnUpdateCenterOrigin(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

bool CRoomFrm::SetRefFrame(vector pos) 
{
	m_vec_RefPos = pos;
	PrintStatus("Reference frame moved to (%d, %d, %d).", (int)pos.x, (int)pos.y, (int)pos.z);
	m_State_changed = true;

	return true;
}


void CRoomFrm::OnCenterFace() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterFace();
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterFace();
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->OnCenterFace();
	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnCenterFace();
}

bool CRoomFrm::GetModifiedStatus()
{
	return m_Modified;
}

void CRoomFrm::SetModifiedFlag(bool modified)
{
	
	if(modified!=m_Modified)
	{
		//Now we can do whatever we like to notify other window's we're modified
		if(modified)
		{
			//theApp.m_pLevelWnd->SetModifiedFlag(true);
		}

		CString title;
		GetWindowText(title);
		if(modified && (title[title.GetLength()-1] != '*') )
		{
			title += " *";
			
		}
		else if((!modified) && (title[title.GetLength()-1] == '*') )
		{
			title = title.Left(title.GetLength()-2);
		}
		SetWindowText(title);
		SetTitle(title);
		m_Modified = modified;
	}
	
	
}

void CRoomFrm::OnClose() 
{
	char str[2*_MAX_PATH+256];
	char roomname[ROOM_NAME_LEN] = "";

	if(m_Modified)
	{
		if (m_Prim.roomp->name != NULL)
			strcpy(roomname,m_Prim.roomp->name);
		else
			strcpy(roomname,"Untitled.orf");
		sprintf(str,"Would you like to save your changes to %s?",roomname);
		if(IDYES==AfxMessageBox(str,MB_YESNO))
		{
			theApp.OnFileSaveRoom();
		}
	}

	m_State_changed = true;

	CMDIChildWnd::OnClose();
}

void CRoomFrm::CloseRoom() 
{
	OnClose();
}

void CRoomFrm::OnShowObjects() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;
	bool bShow = false;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowObjects)
			bShow = true;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowObjects)
			bShow = true;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowObjects)
			bShow = true;

	if (bShow)
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowObjects = true;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowObjects = true;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowObjects = true;
	}
	else
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowObjects = false;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowObjects = false;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowObjects = false;
	}

	m_State_changed = true;
}

void CRoomFrm::OnUpdateShowObjects(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowObjects)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowObjects)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowObjects)
			bCheck = false;

	pCmdUI->SetCheck(bCheck);
}

void CRoomFrm::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	// Update the current face/texture displays
	ShutdownTex();
	int i = 0;
	do
		if ( theApp.m_ppPaletteRoomFrms[i] == this )
		{
			theApp.m_ppPaletteRoomFrms[i] = NULL;
			break;
		}
	while ( i++ < MAX_PALETTE_ROOMS );
	// As long as this is only a paletted room, it can be freed
	if (theApp.m_pRoomFrm != this)
		FreeRoom(m_Prim.roomp);
	else
		theApp.m_pRoomFrm = NULL;

	if (theApp.m_pFocusedRoomFrm == this)
		theApp.m_pFocusedRoomFrm = NULL;

	m_State_changed = true; // force update of dialogs in OnIdle; should probably use a separate Editor_changed var, but oh well

	CMDIChildWnd::PostNcDestroy();
}

void CRoomFrm::StopCameras() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->StopCamera();
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->StopCamera();
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		((Cned_OrthoWnd *) wnd)->StopCamera();
	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->StopCamera();
}


void CRoomFrm::OnModeVertex() 
{
	// TODO: Add your command handler code here
	Editor_state.mode = MODE_VERTEX;
}

void CRoomFrm::OnModeFace() 
{
	// TODO: Add your command handler code here
	Editor_state.mode = MODE_FACE;
}

void CRoomFrm::OnModeObject() 
{
	// TODO: Add your command handler code here
	if (this == theApp.m_pRoomFrm) // only allow for the primary room frame
		Editor_state.mode = MODE_OBJECT;
}

void CRoomFrm::OnUpdateModeVertex(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool check;

	Editor_state.mode == MODE_VERTEX ? check = true : check = false;
	pCmdUI->SetCheck(check);
}

void CRoomFrm::OnUpdateModeFace(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool check;

	Editor_state.mode == MODE_FACE ? check = true : check = false;
	pCmdUI->SetCheck(check);
}

void CRoomFrm::OnUpdateModeObject(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable = false;
	bool check;

	if ( theApp.m_pLevelWnd != NULL && this == theApp.m_pRoomFrm )
		enable = true;
	pCmdUI->Enable(enable);

	Editor_state.mode == MODE_OBJECT ? check = true : check = false;
	pCmdUI->SetCheck(check);
}

void CRoomFrm::OnMarkToggle() 
{
	// TODO: Add your command handler code here
	prim *prim = theApp.AcquirePrim();
	face *fp;
	int vertnum = -1;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Toggle marking for current vertex
		if (prim->face != -1 && prim->vert != -1)
		{
			fp = &prim->roomp->faces[prim->face];
			vertnum = fp->face_verts[prim->vert];
		}
		else
		{
			if (m_Current_nif_vert != -1)
				vertnum = m_Current_nif_vert;
		}

		if (vertnum != -1)
		{
			if (!m_Vert_marks[vertnum])
				MarkVert(vertnum);
			else
				UnMarkVert(vertnum);
		}
		else
			OutrageMessageBox("No current vert!");
		break;

	case MODE_FACE:
		// Toggle marking for current face
		if (prim->face != -1)
		{
			if (!m_Face_marks[prim->face])
				MarkFace(prim->face);
			else
				UnMarkFace(prim->face);
		}
		else
			OutrageMessageBox("No current face!");
		break;
	}

	m_State_changed = true;
}

void CRoomFrm::OnMarkAll() 
{
	// TODO: Add your command handler code here
	room *rp = theApp.AcquirePrim()->roomp;
	int i;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Mark all vertices
		for (i=0; i<rp->num_verts; i++)
			MarkVert(i);
		break;

	case MODE_FACE:
		// Mark all faces
		for (i=0; i<rp->num_faces; i++)
			MarkFace(i);
		break;
	}

	m_State_changed = true;
}

void CRoomFrm::OnUnmarkAll() 
{
	// TODO: Add your command handler code here
	room *rp = theApp.AcquirePrim()->roomp;
	int i;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Unmark all vertices
		for (i=0; i<rp->num_verts; i++)
			UnMarkVert(i);
		break;

	case MODE_FACE:
		// Unmark all faces
		for (i=0; i<rp->num_faces; i++)
			UnMarkFace(i);
		break;
	}

	m_State_changed = true;
}

void CRoomFrm::OnInvertMarkings() 
{
	// TODO: Add your command handler code here
	room *rp = theApp.AcquirePrim()->roomp;
	int i;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Invert vertex markings
		for (i=0; i<rp->num_verts; i++)
			m_Vert_marks[i] ? UnMarkVert(i) : MarkVert(i);
		break;

	case MODE_FACE:
		// Invert face markings
		for (i=0; i<rp->num_faces; i++)
			m_Face_marks[i] ? UnMarkFace(i) : MarkFace(i);
		break;
	}

	m_State_changed = true;
}

void CRoomFrm::OnEditInsert() 
{
	// TODO: Add your command handler code here
	prim *prim = theApp.AcquirePrim();
	room *rp = prim->roomp;
	int vertnum,facenum;
	int i = 0;
	int num_m_verts;
	int ret;
//	vector pos;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Insert a vertex and mark it
		vertnum = InsertVertex(rp,m_vec_InsertPos);
		if (vertnum != -1)
		{
			PrintStatus("Vertex created at (%d, %d, %d).", (int)m_vec_InsertPos.x, (int)m_vec_InsertPos.y, (int)m_vec_InsertPos.z);
			MarkVert(vertnum);
			m_Room_changed = true;
		}
		else
			MessageBox("You cannot add any more vertices to this room.");
		break;

	case MODE_FACE:
		// Insert a face and mark it
		num_m_verts = GetMarkedVerts(vertnums);
		ASSERT(num_m_verts == m_Num_marked_verts);

		facenum = InsertFace(prim->roomp,vertnums,num_m_verts);
		if (facenum != -1)
		{
			SetPrim(rp,facenum,rp->faces[facenum].portal_num,0,0);
			MarkFace(facenum);
			while (i<num_m_verts)
				UnMarkVert(vertnums[i++]);
			PrintStatus("Face created from marked vertices.");
			// Update the current face/texture displays
			Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), prim->face);
			m_Room_changed = true;
		}
		// Error messages are in InsertFace
//		else
//			MessageBox("Face could not be created from the marked vertices!");
		break;

	case MODE_OBJECT:
		// Insert an object
		if (this == theApp.m_pRoomFrm) // only allow for the primary room frame
		{
			room *rp = theApp.m_pLevelWnd->m_Prim.roomp;
//			ComputeRoomCenter(&pos,rp);

			int obj_id = Editor_state.GetCurrentObject();
			if (obj_id != -1)
			{
/*
				switch (m_Focused_pane)
				{
				case VIEW_XY:
					m_vec_InsertPos.z = pos.z;
					break;
				case VIEW_XZ:
					m_vec_InsertPos.y = pos.y;
					break;
				case VIEW_ZY:
					m_vec_InsertPos.x = pos.x;
					break;
				default:
					return;
				}
*/
				matrix orient = IDENTITY_MATRIX;
				ret = InsertObject(rp,-1,obj_id,m_vec_InsertPos,orient);
				if (ret == -1)
					mprintf((0, "Attempt to place object outside mine failed!\n"));
				else
					OutrageMessageBox("Object inserted at %d, %d, %d",(int)m_vec_InsertPos.x,(int)m_vec_InsertPos.y,(int)m_vec_InsertPos.z);
			}
		}
		break;

	case MODE_PATH:
		// Insert a gamepath, gamepath node, BNode, or edge
		if (this == theApp.m_pRoomFrm) // only allow for the primary room frame
		{
			room *rp = theApp.m_pLevelWnd->m_Prim.roomp;
			matrix orient = IDENTITY_MATRIX;
			if (Editor_state.GetCurrentNodeType() == NODETYPE_GAME)
				m_bHandle_Nodes ? ned_InsertNode(ROOMNUM(rp),m_vec_InsertPos,orient) : ned_InsertPath(ROOMNUM(rp),m_vec_InsertPos,orient);
			else
				m_bHandle_Nodes ? ned_InsertBNode(ROOMNUM(rp),m_vec_InsertPos) : ned_InsertEdge();
		}
		break;
	}

	m_State_changed = true;
}

void CRoomFrm::OnEditDelete() 
{
	// TODO: Add your command handler code here
	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Delete the marked vertices!
		DeleteMarkedVerts();
		break;

	case MODE_FACE:
		// Delete the marked faces!
		DeleteMarkedFaces();
		break;

	case MODE_OBJECT:
		// Delete the current object!
		if (this == theApp.m_pRoomFrm) // only allow for the primary room frame
		{
			HObjectDelete();
			Editor_state.SetCurrentLevelObject(Cur_object_index);
			m_Room_changed = true;
		}
		break;

	case MODE_PATH:
		if (this == theApp.m_pRoomFrm) // only allow for the primary room frame
		{
			if (Editor_state.GetCurrentNodeType() == NODETYPE_GAME)
				m_bHandle_Nodes ? ned_DeleteNode() : ned_DeletePath();
			else
				m_bHandle_Nodes ? ned_DeleteBNode() : ned_DeleteEdge();
		}
		break;
	}

	m_State_changed = true;
}

void CRoomFrm::OnGridSize0() 
{
	// TODO: Add your command handler code here
	GridSize(0);
}

void CRoomFrm::OnUpdateGridSize0(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(0));
}

void CRoomFrm::RemoveExtraPoints()
{
	room *rp = theApp.AcquirePrim()->roomp;

	// Force unmark verts in room
	int saved = Editor_state.mode;
	Editor_state.mode = MODE_VERTEX;
	OnUnmarkAll();
	Editor_state.mode = saved;

	// Remove the extra verts
	int num = RemoveDuplicateAndUnusedPointsInRoom(rp);
	if (num > 0)
		m_Room_changed = true;
}

void CRoomFrm::OnSnapToGrid() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;
	bool bSnap = false;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridSnap)
			bSnap = true;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridSnap)
			bSnap = true;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridSnap)
			bSnap = true;

	if (bSnap)
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bGridSnap = true;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bGridSnap = true;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bGridSnap = true;
	}
	else
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bGridSnap = false;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bGridSnap = false;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bGridSnap = false;
	}

	m_State_changed = true;
}

void CRoomFrm::OnUpdateSnapToGrid(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridSnap)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridSnap)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bGridSnap)
			bCheck = false;

	pCmdUI->SetCheck(bCheck);
}

CWnd * CRoomFrm::GetFocusedPane()
{
	CWnd *wnd;

	switch (m_Focused_pane)
	{
	case VIEW_XY:
		wnd = m_wndSplitter.GetPane(0,0);
		break;

	case VIEW_XZ:
		wnd = m_wndSplitter.GetPane(0,1);
		break;

	case VIEW_ZY:
		wnd = m_wndSplitter.GetPane(1,0);
		break;

	case VIEW_3D:
		wnd = m_wndSplitter.GetPane(1,1);
		break;

	default:
		// panes not initted/focused yet
		wnd = m_wndSplitter.GetPane(0,0);
		break;
	}

	if(IsWindow(wnd->m_hWnd))
		return wnd;
	else
		return NULL;
}


void CRoomFrm::OnDefaultExtrude() 
{
	// TODO: Add your command handler code here
	int which,inward,faces;
	float dist;
	BOOL delete_base_face,use_default;
	prim *prim = theApp.AcquirePrim();
	static int oldface = -1;

	if (!m_ExtrudeTimer || prim->face != oldface)
	{
	oldface = prim->face;
	theApp.GetExtrudeDefaults(&which,&dist,&delete_base_face,&inward,&faces,&use_default);

	if (!use_default)
	{
		CExtrudeDialog ex_dlg;
		if ( ex_dlg.DoModal() == IDCANCEL )
			return;
		HandleExtrude(ex_dlg.m_Direction,ex_dlg.m_Distance,ex_dlg.m_DeleteBaseFace,ex_dlg.m_Inward,ex_dlg.m_Faces);
	}
	else
		HandleExtrude(which,dist,delete_base_face,inward,faces);

	// Don't let the user accidentally extrude twice in a row; a delay less than one second seems reasonable.
	m_ExtrudeTimer = SetTimer(EXTRUDE_TIMER,500,NULL);
	}
#ifdef _DEBUG
	else
		MessageBox("No.");
#endif
}

void CRoomFrm::OnUpdateMarkAll(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	BOOL enable;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
	case MODE_FACE:		enable = TRUE;		break;
	case MODE_OBJECT:
	case MODE_PATH:
	default:			enable = FALSE;		break;
	}
	pCmdUI->Enable(enable);
}

void CRoomFrm::OnUpdateMarkToggle(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	BOOL enable;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
	case MODE_FACE:		enable = TRUE;		break;
	case MODE_OBJECT:
	case MODE_PATH:
	default:			enable = FALSE;		break;
	}
	pCmdUI->Enable(enable);
}

void CRoomFrm::OnUpdateInvertMarkings(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	BOOL enable;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
	case MODE_FACE:		enable = TRUE;		break;
	case MODE_OBJECT:
	case MODE_PATH:
	default:			enable = FALSE;		break;
	}
	pCmdUI->Enable(enable);
}

void CRoomFrm::OnUpdateUnmarkAll(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	BOOL enable;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
	case MODE_FACE:		enable = TRUE;		break;
	case MODE_OBJECT:
	case MODE_PATH:
	default:			enable = FALSE;		break;
	}
	pCmdUI->Enable(enable);
}

void CRoomFrm::OnGridSize6() 
{
	// TODO: Add your command handler code here
	GridSize(6);
}

void CRoomFrm::OnUpdateGridSize6(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(6));
}

void CRoomFrm::OnEditCopy() 
{
	// TODO: Add your command handler code here
	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		g_bFaces = false;
		g_bVerts = true;
		CopyVerts(true);
		UpdateAllPanes();
		break;
	case MODE_FACE:
		g_bFaces = true;
		g_bVerts = false;
		CopyFaces(true);
		UpdateAllPanes();
		break;
	}
}

void CRoomFrm::OnUpdateEditCopy(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	int num_m_verts,num_m_faces;
	bool bEnable = false;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Get list of marked verts
		num_m_verts = GetMarkedVerts(vertnums);
		ASSERT(num_m_verts == m_Num_marked_verts);
		if (num_m_verts)
			bEnable = true;
		break;
	case MODE_FACE:
		// Get list of marked faces
		num_m_faces = GetMarkedFaces(facenums);
		ASSERT(num_m_faces == m_Num_marked_faces);
		if (num_m_faces)
			bEnable = true;
		break;
	}

	pCmdUI->Enable(bEnable);
}

void CRoomFrm::OnEditUndo() 
{
	// TODO: Add your command handler code here
	
}

void CRoomFrm::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(false);
}

void CRoomFrm::OnUpdateEditPaste(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool bEnable = false;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		if (g_bVerts)
			bEnable = true;
		break;
	case MODE_FACE:
		if (g_bFaces)
			bEnable = true;
		break;
	}

	pCmdUI->Enable(bEnable);
}

void CRoomFrm::OnUpdateEditInsert(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool bEnable = true;

	switch (Editor_state.mode)
	{
	case MODE_OBJECT:
		if ( Editor_state.GetCurrentObject() == -1 )
			bEnable = false;
		break;
	}

	pCmdUI->Enable(bEnable);
}

void CRoomFrm::OnUpdateEditDelete(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	int num_m_verts,num_m_faces;
	bool bEnable = true;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Get list of marked verts
		num_m_verts = GetMarkedVerts(vertnums);
		ASSERT(num_m_verts == m_Num_marked_verts);
		if (!num_m_verts)
			bEnable = false;
		break;
	case MODE_FACE:
		// Get list of marked faces
		num_m_faces = GetMarkedFaces(facenums);
		ASSERT(num_m_faces == m_Num_marked_faces);
		if (!num_m_faces)
			bEnable = false;
		break;
	case MODE_PATH:
		break;
	}

	pCmdUI->Enable(bEnable);
}

// supplies the text for the tooltip
BOOL CRoomFrm::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{
	CWnd *wnd;
	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;
	UINT nID = pNMHDR->idFrom;

	if (pTTT->uFlags & TTF_IDISHWND)
	{
		// idFrom is actually the HWND of the tool
		wnd = m_wndSplitter.GetPane(0,0);
		if ((HWND)nID == wnd->m_hWnd)
		{
			vec2D pt1 = ((Cned_OrthoWnd *) wnd)->GetMousePos();
			vec2D pt2 = ((Cned_OrthoWnd *) wnd)->GetCursorPos();
			char str[128];
			sprintf(str,"%.2f %.2f",pt1.x-pt2.x,pt1.y-pt2.y);
			strcpy(pTTT->szText,str);
			pTTT->hinst = NULL;
			return TRUE;
		}
	}

	return FALSE;
}

void CRoomFrm::OnGridSizeCustom() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;
	bool bCheck = true;
	int num=10,grid_size=10;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		grid_size = ((Cned_OrthoWnd *) wnd)->m_CustomGrid;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (grid_size != ((Cned_OrthoWnd *) wnd)->m_CustomGrid)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (grid_size != ((Cned_OrthoWnd *) wnd)->m_CustomGrid)
			bCheck = false;

	if (bCheck)
		num = grid_size;

	if (InputNumber(&num,"Custom Grid","Enter a custom grid size",this))
	{
		if (num<=0)
		{
			OutrageMessageBox("You must enter a value greater than 0.");
			return;
		}

		wnd = m_wndSplitter.GetPane(0,0);
		if(IsWindow(wnd->m_hWnd))
		{
			((Cned_OrthoWnd *) wnd)->m_nGrid = NUM_GRIDS-1;
			((Cned_OrthoWnd *) wnd)->m_CustomGrid = num;
		}
		wnd = m_wndSplitter.GetPane(0,1);
		if(IsWindow(wnd->m_hWnd))
		{
			((Cned_OrthoWnd *) wnd)->m_nGrid = NUM_GRIDS-1;
			((Cned_OrthoWnd *) wnd)->m_CustomGrid = num;
		}
		wnd = m_wndSplitter.GetPane(1,0);
		if(IsWindow(wnd->m_hWnd))
		{
			((Cned_OrthoWnd *) wnd)->m_nGrid = NUM_GRIDS-1;
			((Cned_OrthoWnd *) wnd)->m_CustomGrid = num;
		}
		GridSize(NUM_GRIDS-1);
	}
}

void CRoomFrm::OnUpdateGridSizeCustom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(UpdateGridSize(NUM_GRIDS-1));
}

void CRoomFrm::OnViewMoveCameraToCurrentObject() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnViewMoveCameraToCurrentObject();
}

void CRoomFrm::OnFileShowRoomStats() 
{
	// TODO: Add your command handler code here
	room *rp = theApp.AcquirePrim()->roomp;
	ShowRoomStats(rp);
}

void CRoomFrm::OnViewTexturedOutline() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnViewTexturedOutline();
}

void CRoomFrm::OnUpdateViewTexturedOutline(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		pCmdUI->SetCheck(((Cned_PerspWnd *) wnd)->m_bTextured && ((Cned_PerspWnd *) wnd)->m_bOutline);
}

void CRoomFrm::UnMarkAll() 
{
	OnUnmarkAll();
}

void CRoomFrm::CenterRoom()
{
	OnCenterRoom();
}

void CRoomFrm::OnFileClose() 
{
	// TODO: Add your command handler code here
	if (this != theApp.m_pRoomFrm)
		CloseRoom();
	else
		DestroyWindow();
}

CWnd * CRoomFrm::GetPane(int i,int j)
{
	ASSERT(i<2 && j<2);
	return m_wndSplitter.GetPane(i,j);
}

void CRoomFrm::OnShowRoomCenter() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;
	bool bShow = false;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowCenters)
			bShow = true;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowCenters)
			bShow = true;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowCenters)
			bShow = true;

	if (bShow)
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowCenters = true;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowCenters = true;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowCenters = true;
	}
	else
	{
		wnd = m_wndSplitter.GetPane(0,0);
		((Cned_OrthoWnd *) wnd)->m_bShowCenters = false;
		wnd = m_wndSplitter.GetPane(0,1);
		((Cned_OrthoWnd *) wnd)->m_bShowCenters = false;
		wnd = m_wndSplitter.GetPane(1,0);
		((Cned_OrthoWnd *) wnd)->m_bShowCenters = false;
	}

	m_State_changed = true;
}

void CRoomFrm::OnUpdateShowRoomCenter(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	CWnd *wnd;
	bool bCheck = true;

	wnd = m_wndSplitter.GetPane(0,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowCenters)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(0,1);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowCenters)
			bCheck = false;
	wnd = m_wndSplitter.GetPane(1,0);
	if(IsWindow(wnd->m_hWnd))
		if (!((Cned_OrthoWnd *) wnd)->m_bShowCenters)
			bCheck = false;

	pCmdUI->SetCheck(bCheck);
}

void CRoomFrm::ForceUnmarkAll()
{
	int saved = Editor_state.mode;
	for (int i=0; i<NUM_MODES; i++)
	{
		Editor_state.mode = i;
		OnUnmarkAll();
	}
	Editor_state.mode = saved;
}

void CRoomFrm::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	switch (nIDEvent)
	{
	case EXTRUDE_TIMER:
		ASSERT(m_ExtrudeTimer);
		KillTimer(m_ExtrudeTimer);
		m_ExtrudeTimer = 0;
	}

	CMDIChildWnd::OnTimer(nIDEvent);
}

char * CRoomFrm::GetPath()
{
	return m_Path;
}

void CRoomFrm::SetPath(char *path)
{
	ASSERT(strlen(path) < sizeof(m_Path));
	strcpy(m_Path,path);
}


void RoomChanged(room *rp)
{
	if (theApp.m_pLevelWnd != NULL)
		(rp == Curroomp) ? (World_changed = 1) : (theApp.m_pFocusedRoomFrm->m_Room_changed = 1);
	else if (theApp.m_pFocusedRoomFrm != NULL)
		theApp.m_pFocusedRoomFrm->m_Room_changed = 1;
}


void CRoomFrm::OnModePath() 
{
	// TODO: Add your command handler code here
	if (this == theApp.m_pRoomFrm) // only allow for the primary room frame
		Editor_state.mode = MODE_PATH;
}

void CRoomFrm::OnUpdateModePath(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable = false;
	bool check;

	if ( theApp.m_pLevelWnd != NULL && this == theApp.m_pRoomFrm )
		enable = true;
	pCmdUI->Enable(enable);

	Editor_state.mode == MODE_PATH ? check = true : check = false;
	pCmdUI->SetCheck(check);
}

void CRoomFrm::OnEditPasteOnTop() 
{
	// TODO: Add your command handler code here
	if (g_bVerts)
		PasteVerts(true,true);
	if (g_bFaces)
		PasteFaces(true,true);

	UpdateAllPanes();
}

void CRoomFrm::OnUpdateEditPasteOnTop(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool bEnable = false;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		if (g_bVerts)
			bEnable = true;
		break;
	case MODE_FACE:
		if (g_bFaces)
			bEnable = true;
		break;
	}

	pCmdUI->Enable(bEnable);
}

void CRoomFrm::OnViewMoveCameraToCurrentRoom() 
{
	// TODO: Add your command handler code here
	CWnd *wnd;

	wnd = m_wndSplitter.GetPane(1,1);
	if(IsWindow(wnd->m_hWnd))
		((Cned_PerspWnd *) wnd)->OnViewMoveCameraToCurrentRoom();
}

void CRoomFrm::OnUpdateViewMoveCameraToCurrentObject(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable = false;

	if ( theApp.m_pLevelWnd != NULL && 
		this == theApp.m_pRoomFrm && 
		Cur_object_index != -1 && 
		Highest_object_index >= 0 && 
		Objects[Cur_object_index].roomnum == ROOMNUM(m_Prim.roomp) )
		enable = true;

	pCmdUI->Enable(enable);
}



void CRoomFrm::OnEditCut() 
{
	// TODO: Add your command handler code here
	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		g_bFaces = false;
		g_bVerts = true;
		CopyVerts(false);
		DeleteMarkedVerts();
		UpdateAllPanes();
		break;
	case MODE_FACE:
		g_bFaces = true;
		g_bVerts = false;
		CopyFaces(false);
		DeleteMarkedFaces();
		UpdateAllPanes();
		break;
	}
}

void CRoomFrm::OnUpdateEditCut(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	int num_m_verts,num_m_faces;
	bool bEnable = false;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		// Get list of marked verts
		num_m_verts = GetMarkedVerts(vertnums);
		ASSERT(num_m_verts == m_Num_marked_verts);
		if (num_m_verts)
			bEnable = true;
		break;
	case MODE_FACE:
		// Get list of marked faces
		num_m_faces = GetMarkedFaces(facenums);
		ASSERT(num_m_faces == m_Num_marked_faces);
		if (num_m_faces)
			bEnable = true;
		break;
	}

	pCmdUI->Enable(bEnable);
}

void CRoomFrm::OnPrevPane() 
{
	// TODO: Add your command handler code here
	m_wndSplitter.ActivateNext(TRUE);
}

void CRoomFrm::OnNextPane() 
{
	// TODO: Add your command handler code here
	m_wndSplitter.ActivateNext(FALSE);
}

