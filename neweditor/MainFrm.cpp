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
 

// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "NewEditor.h"
#include "globals.h"
#include "MainFrm.h"
#include "ned_HFile.h"
#include "render.h"
#include "HRoom.h"
#include "LevelInfoDialog.h"
#include "texture.h"
#include "boa.h"
#include "ListDialog.h"
#include "EditLineDialog.h"
#include "erooms.h"
#include "ned_Geometry.h"
#include "RefFrameDialog.h"
#include "selectedroom.h"
#include "ned_Util.h"

#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// The scrap buffer
group *Scrap=NULL;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_COMMAND(ID_FILE_SHOWLEVELSTATS, OnFileShowLevelStats)
	ON_COMMAND(ID_FILE_EDITLEVELINFO, OnFileEditLevelInfo)
	ON_COMMAND(ID_VIEW_TEXTURES, OnViewTextures)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTURES, OnUpdateViewTextures)
	ON_WM_CONTEXTMENU()
	ON_WM_CLOSE()	
	ON_COMMAND(ID_VIEW_ROOMBAR, OnViewRoombar)
	ON_UPDATE_COMMAND_UI(ID_VIEW_ROOMBAR, OnUpdateViewRoombar)
	ON_COMMAND(ID_ROOM_PLACEROOM, OnRoomPlaceRoom)
	ON_COMMAND(ID_ROOM_ATTACHROOM, OnRoomAttachRoom)
	ON_COMMAND(ID_ROOM_ADD, OnRoomAdd)
	ON_COMMAND(ID_ROOM_BUILDBRIDGE, OnRoomBuildBridge)
	ON_COMMAND(ID_ROOM_BUILDSMOOTHBRIDGE, OnRoomBuildSmoothBridge)
	ON_COMMAND(ID_ROOM_COMBINE, OnRoomCombine)
	ON_COMMAND(ID_ROOM_DELETE, OnRoomDelete)
	ON_COMMAND(ID_ROOM_DELETEFACE, OnRoomDeleteFace)
	ON_COMMAND(ID_ROOM_DELETEPORTAL, OnRoomDeletePortal)
	ON_COMMAND(ID_ROOM_DELETEVERT, OnRoomDeleteVert)
	ON_COMMAND(ID_ROOM_DROPROOM, OnRoomDropRoom)
	ON_COMMAND(ID_ROOM_JOIN_ADJACENT_FACES, OnRoomJoinAdjacentFaces)
	ON_COMMAND(ID_ROOM_JOINROOMS, OnRoomJoinRooms)
	ON_COMMAND(ID_ROOM_JOINROOMSEXACT, OnRoomJoinRoomsExact)
	ON_COMMAND(ID_ROOM_MARK, OnRoomMark)
	ON_COMMAND(ID_ROOM_PROPAGATETOALL, OnRoomPropagateToAll)
	ON_COMMAND(ID_ROOM_SNAPPLACEDROOM, OnRoomSnapPlacedRoom)
	ON_COMMAND(ID_ROOM_SNAPPOINTTOEDGE, OnRoomSnapPointToEdge)
	ON_COMMAND(ID_ROOM_SNAPPOINTTOFACE, OnRoomSnapPointToFace)
	ON_COMMAND(ID_ROOM_SNAPPOINTTOPOINT, OnRoomSnapPointToPoint)
	ON_COMMAND(ID_ROOM_SWAPMAKEDANDCURRENTROOMFACE, OnRoomSwapMarkedAndCurrentRoomFace)
	ON_COMMAND(ID_VIEW_OBJECTS, OnViewObjects)
	ON_UPDATE_COMMAND_UI(ID_VIEW_OBJECTS, OnUpdateViewObjects)
	ON_COMMAND(ID_ZBUFFER, OnZBuffer)
	ON_UPDATE_COMMAND_UI(ID_ZBUFFER, OnUpdateZBuffer)
	ON_COMMAND(ID_WINDOW_TEXTUREALIGNMENT, OnWindowTextureAlignment)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TEXTUREALIGNMENT, OnUpdateWindowTextureAlignment)
	ON_COMMAND(ID_WINDOW_TRIGGERS, OnWindowTriggers)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TRIGGERS, OnUpdateWindowTriggers)
	ON_COMMAND(ID_VIEW_DOORWAYS, OnViewDoorways)
	ON_UPDATE_COMMAND_UI(ID_VIEW_DOORWAYS, OnUpdateViewDoorways)
	ON_COMMAND(ID_VIEW_LIGHTING, OnViewLighting)
	ON_UPDATE_COMMAND_UI(ID_VIEW_LIGHTING, OnUpdateViewLighting)
	ON_COMMAND(ID_BUTTON_BOA_VIS, OnButtonBoaVis)
	ON_UPDATE_COMMAND_UI(ID_BUTTON_BOA_VIS, OnUpdateButtonBoaVis)
	ON_COMMAND(ID_WINDOW_GOALS, OnWindowGoals)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_GOALS, OnUpdateWindowGoals)
	ON_COMMAND(ID_FILE_VERIFY_MINE, OnFileVerifyMine)
	ON_COMMAND(ID_FILE_VERIFY_ROOM, OnFileVerifyRoom)
	ON_COMMAND(ID_HELP_CONTENTS, OnHelpContents)
	ON_COMMAND(ID_ROOM_UNPLACEROOM, OnRoomUnplaceRoom)
	ON_UPDATE_COMMAND_UI(ID_ROOM_UNPLACEROOM, OnUpdateRoomUnplaceRoom)
	ON_COMMAND(ID_FILE_REMOVEEXTRAPOINTS, OnFileRemoveExtraPoints)
	ON_UPDATE_COMMAND_UI(ID_ROOM_ATTACHROOM, OnUpdateRoomAttachRoom)
	ON_UPDATE_COMMAND_UI(ID_ROOM_SNAPPLACEDROOM, OnUpdateRoomSnapPlacedRoom)
	ON_COMMAND(ID_WINDOW_ROOMPROPERTIES, OnWindowRoomProperties)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_ROOMPROPERTIES, OnUpdateWindowRoomProperties)
	ON_COMMAND(ID_FILE_REMOVEEXTRAPOINTS_ROOM, OnFileRemoveExtraPointsInRoom)
	ON_COMMAND(ID_ROOM_SELECTBYNUMBER, OnRoomSelectByNumber)
	ON_COMMAND(ID_ROOM_SELECTFACEBYNUMBER, OnRoomSelectFaceByNumber)
	ON_WM_TIMER()
	ON_WM_DROPFILES()
	ON_COMMAND(ID_FILE_GRAVITY, OnFileGravity)
	ON_COMMAND(ID_MOVE_MINE, OnMoveMine)
	ON_COMMAND(ID_VIEW_SOUNDS, OnViewSounds)
	ON_UPDATE_COMMAND_UI(ID_VIEW_SOUNDS, OnUpdateViewSounds)
	ON_COMMAND(ID_WINDOW_MATCENS, OnWindowMatcens)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_MATCENS, OnUpdateWindowMatcens)
	ON_COMMAND(ID_VIEW_CAMERASLEWER, OnViewCameraSlewer)
	ON_UPDATE_COMMAND_UI(ID_VIEW_CAMERASLEWER, OnUpdateViewCameraSlewer)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_ATTACH, OnEditAttach)
	ON_COMMAND(ID_EDIT_DELETE, OnEditDelete)
	ON_COMMAND(ID_EDIT_LOADSCRAP, OnEditLoadscrap)
	ON_COMMAND(ID_EDIT_SAVESCRAP, OnEditSavescrap)
	ON_COMMAND(ID_EDIT_REMOVESELECT, OnEditRemoveselect)
	ON_COMMAND(ID_EDIT_SELECTATTACHED, OnEditSelectattached)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_UPDATE_COMMAND_UI(ID_EDIT_ATTACH, OnUpdateEditAttach)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SAVESCRAP, OnUpdateEditSavescrap)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_COMMAND(ID_EDIT_CLEARSELECTED, OnEditClearselected)
	ON_COMMAND(ID_EDIT_ADDSELECT, OnEditAddselect)
	ON_COMMAND(ID_ROOM_MERGEOBJECTINTOROOM, OnRoomMergeObjectIntoRoom)
	ON_UPDATE_COMMAND_UI(ID_ROOM_MERGEOBJECTINTOROOM, OnUpdateRoomMergeObjectIntoRoom)
	ON_COMMAND(ID_FILE_SCALEMINE, OnFileScaleMine)
	ON_COMMAND(ID_EDIT_UNPLACEGROUP, OnEditUnplaceGroup)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNPLACEGROUP, OnUpdateEditUnplaceGroup)
	ON_COMMAND(ID_WINDOW_EDITORBAR, OnWindowEditorBar)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_EDITORBAR, OnUpdateWindowEditorBar)
	ON_COMMAND(ID_WINDOW_EDITORBAR2, OnWindowEditorBar2)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_EDITORBAR2, OnUpdateWindowEditorBar2)
	ON_COMMAND(ID_WINDOW_PATHBAR, OnWindowPathBar)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_PATHBAR, OnUpdateWindowPathBar)
	ON_COMMAND(ID_WINDOW_TERRAINBAR, OnWindowTerrainBar)
	ON_UPDATE_COMMAND_UI(ID_WINDOW_TERRAINBAR, OnUpdateWindowTerrainBar)
	ON_COMMAND(ID_ROOM_PLACETERRAINROOM, OnRoomPlaceTerrainRoom)
	ON_COMMAND(ID_ROOM_LINKTONEWEXTERNAL, OnRoomLinkToNewExternal)
	ON_COMMAND(ID_EDIT_PLACE_TERRAIN, OnEditPlaceTerrain)
	//}}AFX_MSG_MAP
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_NUM_MARKED, OnUpdateStatusMarked)
	ON_UPDATE_COMMAND_UI(ID_INSERT_POSITION, OnUpdateStatusPosition)
	ON_UPDATE_COMMAND_UI(ID_MOUSE_POSITION, OnUpdateStatusMouse)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_CURRENT, OnUpdateStatusCurrent)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_GRID_SIZE, OnUpdateStatusGridSize)
	ON_UPDATE_COMMAND_UI(ID_INDICATOR_MODE, OnUpdateStatusMode)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,				// status line indicator
};

static UINT indicators2[] =
{
	ID_INDICATOR_CURRENT,		// current primitives
	ID_INDICATOR_GRID_SIZE,		// grid size
	ID_MOUSE_POSITION,			// current position of mouse cursor in orthogonal view
	ID_INSERT_POSITION,			// current position of insert cursor in orthogonal view
	ID_INDICATOR_MODE,			// current mode (vert, face, object, path)
	ID_INDICATOR_NUM_MARKED,	// number of marked items in current mode
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	strD3EditDialogBarState = _T("D3EditDialogBarState");
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	
	if (!m_wndToolBar.CreateEx(this) ||
		!m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}

/*
	if (!m_wndDlgBar.Create(this, IDR_MAINFRAME, 
		CBRS_ALIGN_TOP, AFX_IDW_DIALOGBAR))
	{
		TRACE0("Failed to create dialogbar\n");
		return -1;		// fail to create
	}
*/

	if (!m_wndReBar.Create(this) ||
		!m_wndReBar.AddBar(&m_wndToolBar) )
		
	{
		TRACE0("Failed to create rebar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	if (!m_wndStatusBar2.Create(this) ||
		!m_wndStatusBar2.SetIndicators(indicators2,
		  sizeof(indicators2)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar 2\n");
		return -1;      // fail to create
	}

/*
	int bar_dim[4] = {80,60,140,-1};
	CStatusBarCtrl& m_wndStatusBarCtrl = m_wndStatusBar.GetStatusBarCtrl();
	m_wndStatusBarCtrl.SetParts(4,bar_dim);
*/

	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY);

	// Enable Docking for the mainframe
	EnableDocking(CBRS_ALIGN_ANY);

	// Create the texture dialog bar
	if(!m_TextureDialog.Create(this,IDD_TEXTUREBAR,CBRS_RIGHT,UNIQUE_ID_TEXTUREBAR))
	{
		mprintf((0,"Texture Dialog create failed\n"));
	}else
	{		
		m_TextureDialog.SetBarStyle(m_TextureDialog.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
		m_TextureDialog.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
		m_TextureDialog.SetWindowText("Textures");
		DockControlBar(&m_TextureDialog);
	}

	// Create the object dialog bar
	if(!m_ObjectDialog.Create(this,IDD_OBJECTBAR,CBRS_RIGHT,UNIQUE_ID_OBJECTBAR))
	{
		mprintf((0,"Object Dialog create failed\n"));
	}else
	{		
		m_ObjectDialog.SetBarStyle(m_ObjectDialog.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
		m_ObjectDialog.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
		m_ObjectDialog.SetWindowText("Objects");
		DockControlBar(&m_ObjectDialog);
	}

	// Create the room dialog bar
	if(!m_RoomDialog.Create(this,IDD_ROOMBAR,CBRS_LEFT,UNIQUE_ID_ROOMBAR))
	{
		mprintf((0,"Room Dialog create failed\n"));
	}else
	{		
		m_RoomDialog.SetBarStyle(m_RoomDialog.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
		m_RoomDialog.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
		m_RoomDialog.SetWindowText("Rooms");
		DockControlBar(&m_RoomDialog);
	}

	// Create the terrain dialog bar
	if(!m_TerrainDialog.Create(this,IDD_TERRAINBAR,CBRS_LEFT,UNIQUE_ID_TERRAINBAR))
	{
		mprintf((0,"Terrain Dialog create failed\n"));
	}else
	{		
		m_TerrainDialog.SetBarStyle(m_TerrainDialog.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
		m_TerrainDialog.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
		m_TerrainDialog.SetWindowText("Terrain");
		DockControlBar(&m_TerrainDialog);
	}

	// Create the doorway dialog bar
	if(!m_DoorwayDialog.Create(this,IDD_DOORWAYBAR,CBRS_LEFT,UNIQUE_ID_DOORWAYBAR))
	{
		mprintf((0,"Doorway Dialog create failed\n"));
	}else
	{		
		m_DoorwayDialog.SetBarStyle(m_DoorwayDialog.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
		m_DoorwayDialog.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
		m_DoorwayDialog.SetWindowText("Doors");
		DockControlBar(&m_DoorwayDialog);
	}

	// Create the sound dialog bar
	if(!m_SoundDialog.Create(this,IDD_SOUNDBAR,CBRS_LEFT,UNIQUE_ID_SOUNDBAR))
	{
		mprintf((0,"Sound Dialog create failed\n"));
	}else
	{		
		m_SoundDialog.SetBarStyle(m_SoundDialog.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
		m_SoundDialog.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
		m_SoundDialog.SetWindowText("Sounds/Waypoints");
		DockControlBar(&m_SoundDialog);
	}

	// Create the path dialog bar
	if(!m_PathDialog.Create(this,IDD_PATHBAR,CBRS_LEFT,UNIQUE_ID_PATHBAR))
	{
		mprintf((0,"Path Dialog create failed\n"));
	}else
	{		
		m_PathDialog.SetBarStyle(m_PathDialog.GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
		m_PathDialog.EnableDocking(CBRS_ALIGN_LEFT|CBRS_ALIGN_RIGHT);
		m_PathDialog.SetWindowText("Paths");
		DockControlBar(&m_PathDialog);
	}

	// Create the camera slewer dialog bar
	gCameraSlewer = new CCameraSlew;
	if (!gCameraSlewer->Create(this,IDD_CAMERASLEW,CBRS_TOP,UNIQUE_ID_SLEWER))
	{
	}else
	{
		gCameraSlewer->SetBarStyle(gCameraSlewer->GetBarStyle()|CBRS_TOOLTIPS|CBRS_FLYBY);
		gCameraSlewer->EnableDocking(CBRS_ALIGN_BOTTOM|CBRS_ALIGN_TOP);
		gCameraSlewer->SetWindowText("Camera Slewer");
		DockControlBar(gCameraSlewer);
	}

	// Create tooltip control
	if(!m_ToolTip.Create(this))
	{
		TRACE0("Unable to create ToolTip control\n");
	}else
	{
		//add the dialog box's controls to the ToolTip here!

		//activate the tooltip control
		m_ToolTip.Activate(TRUE);
	}

	LoadBarState(strD3EditDialogBarState);

	// Load the last resource list
	CString ListName = theApp.GetProfileString("Defaults","LastListName","");
	OpenResourceLists(ListName.GetBuffer(0),CUSTOM_TEX_LIST | CUSTOM_OBJ_LIST | LEVEL_TEX_LIST | LEVEL_OBJ_LIST);

	ShowWindow(SW_SHOWMAXIMIZED);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CMDIFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers


void CMainFrame::OnFileShowLevelStats() 
{
	// TODO: Add your command handler code here
	ShowLevelStats();	
}

void CMainFrame::OnFileEditLevelInfo() 
{
	// TODO: Add your command handler code here
	CLevelInfoDialog dlg(&Level_info);
	dlg.DoModal();
}


/////////////////////////////////////////////////////////////////////////////
// CMainFrame member functions

void CMainFrame::SetStatusMessage(char *str)
{
	m_wndStatusBar.SetPaneText(0, str);
}

void CMainFrame::SetStatusMessage(int index, char *str, bool update)
{
	m_wndStatusBar.SetPaneText(index, str, update);
}

void CMainFrame::OnUpdateStatusMouse(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(); 
	CString str;
	void *wnd = theApp.AcquireWnd();
	vector vec = {0,0,0};

	if (wnd != NULL && wnd == theApp.m_pFocusedRoomFrm)
	{
		vec = ((CRoomFrm *) wnd)->m_vec_MousePos;
	}

	str.Format( "%d, %d, %d",(int)vec.x,(int)vec.y,(int)vec.z ); 
	pCmdUI->SetText( str ); 
}

void CMainFrame::OnUpdateStatusPosition(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(); 
	CString str;
	void *wnd = theApp.AcquireWnd();
	vector vec = {0,0,0};

	if (wnd != NULL && wnd == theApp.m_pFocusedRoomFrm)
	{
		vec = ((CRoomFrm *) wnd)->m_vec_InsertPos;
	}

	str.Format( "%d, %d, %d",(int)vec.x,(int)vec.y,(int)vec.z ); 
	pCmdUI->SetText( str ); 
}

void CMainFrame::OnUpdateStatusCurrent(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(); 
	CString str;
	prim *prim = theApp.AcquirePrim();
	int roomnum=-1,face=-1,portal=-1,edge=-1,vert=-1,vert_idx=-1;
	int n_rooms=0,n_faces=0,n_portals=0,n_edges=0,n_verts=0,n_room_verts=0;
	room *rp;
	int i;

	if (prim != NULL && prim->roomp != NULL)
	{
		face = prim->face;
		portal = prim->portal;
		edge = prim->edge;
		vert = prim->vert;
		if (face != -1 && vert != -1)
			vert_idx = prim->roomp->faces[face].face_verts[vert];
		else if (theApp.m_pFocusedRoomFrm != NULL && theApp.m_pFocusedRoomFrm->m_Current_nif_vert != -1)
			vert_idx = theApp.m_pFocusedRoomFrm->m_Current_nif_vert;

		if (prim == &theApp.m_pLevelWnd->m_Prim)
		{
			roomnum = ROOMNUM(prim->roomp);
			for (i=0,rp=Rooms;i<=Highest_room_index;i++,rp++)
				if (rp->used)
					n_rooms++;
		}
		else
			roomnum = (MAX_ROOMS-1)-ROOMNUM(prim->roomp);
		n_faces = prim->roomp->num_faces;
		n_portals = prim->roomp->num_portals;
		if (face != -1)
			n_edges = n_verts = prim->roomp->faces[face].num_verts;
		else
			n_edges = n_verts = 0;
		n_room_verts = prim->roomp->num_verts;
	}

	str.Format( "R: %d/%d F: %d/%d P: %d/%d E: %d/%d V (idx): %d/%d (%d/%d)",roomnum,n_rooms,face,n_faces,portal,n_portals,edge,n_edges,vert,n_verts,vert_idx,n_room_verts );
	pCmdUI->SetText( str ); 
}

extern int Grids[];

void CMainFrame::OnUpdateStatusGridSize(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(); 
	CString str;
	void *wnd = theApp.AcquireWnd();
	int grid_size = 0;
	int nGrid;

	if (wnd != NULL && wnd == theApp.m_pFocusedRoomFrm)
	{
		CWnd *pane = ((CRoomFrm *) wnd)->GetFocusedPane();
		if ( pane->IsKindOf(RUNTIME_CLASS(Cned_OrthoWnd)) )
		{
			nGrid = ((Cned_OrthoWnd *) pane)->m_nGrid;
			(nGrid == NUM_GRIDS-1) ? (grid_size = ((Cned_OrthoWnd *) pane)->m_CustomGrid) : (grid_size = Grids[nGrid]);
		}
	}

	str.Format( "Grid: %d",grid_size ); 
	pCmdUI->SetText( str ); 
}

void CMainFrame::OnUpdateStatusMode(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(); 
	CString str;

	switch (Editor_state.mode)
	{
	case MODE_VERTEX:
		str = "Vertex mode";
		break;

	case MODE_FACE:
		str = "Face mode";
		break;

	case MODE_OBJECT:
		str = "Object mode";
		break;

	case MODE_PATH:
		str = "Path mode";
		break;
	}

	pCmdUI->SetText( str );
}

BOOL EkIsBarVisible(CControlBar* pBar)
{
	ASSERT_VALID(pBar);

	return ((pBar->GetStyle()&WS_VISIBLE)!=0);
}

void CMainFrame::OnViewTextures() 
{
	ShowControlBar(&m_TextureDialog,!EkIsBarVisible(&m_TextureDialog),FALSE);	
}

void CMainFrame::OnUpdateViewTextures(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck(EkIsBarVisible(&m_TextureDialog));	
}

void CMainFrame::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// Ensure the right-click was on a docking area
	
	if(pWnd->IsKindOf(RUNTIME_CLASS(CControlBar)))//this should check for CDockBar, but that no longer exists(?)
	{
		//Load top-level menu from resource
		CMenu mnuTop;
		mnuTop.LoadMenu(IDR_TOOLBAR_POPUP);

		//get popup menu from first submenu
		CMenu *pPopup = mnuTop.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		//Checked state for popup menu items is automatically managed by MFC
		//UPDATE_COMMAND_UI 

		//Display popup menu
		pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this,NULL);

		//popup menu commands are automatically handled by standard MFC command routing
		return;
	}	
}

void CMainFrame::OnClose() 
{
	// Save control bar states
	SaveBarState(strD3EditDialogBarState);

	// Close level and rooms
	if (theApp.m_ThisLevelFrame != NULL)
		theApp.m_ThisLevelFrame->CloseLevel();
	for (int i=0; i<MAX_PALETTE_ROOMS; i++)
		if ( theApp.m_ppPaletteRoomFrms[i] != NULL )
			theApp.m_ppPaletteRoomFrms[i]->CloseRoom();

	CMDIFrameWnd::OnClose();
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg) 
{
	m_ToolTip.RelayEvent(pMsg);
	
	return CMDIFrameWnd::PreTranslateMessage(pMsg);
}

void CMainFrame::OnViewRoombar() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_RoomDialog,!EkIsBarVisible(&m_RoomDialog),FALSE);	
}

void CMainFrame::OnUpdateViewRoombar(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(&m_RoomDialog));	
}

void CMainFrame::OnRoomPlaceRoom() 
{
	// TODO: Add your command handler code here
	if (theApp.m_pFocusedRoomFrm != NULL && theApp.m_pFocusedRoomFrm->m_Prim.roomp != NULL)
	{
		if (theApp.m_pFocusedRoomFrm->m_Prim.face != -1)
		{
			if (Curface != -1)
			{
				if (Curroomp->faces[Curface].portal_num != -1) {
					OutrageMessageBox("There's already a connection at the current room:face.");
					return;
				}

				PlaceRoom(Curroomp,Curface,ROOMNUM(theApp.m_pFocusedRoomFrm->m_Prim.roomp),theApp.m_pFocusedRoomFrm->m_Prim.face,-1);
			}
			else
				OutrageMessageBox("You do not have a face selected in the level!");
		}
		else
			OutrageMessageBox("You do not have a face selected in the room you're trying to place!");
	}
	else
		OutrageMessageBox("No room to place.");
}

void CMainFrame::OnRoomAttachRoom() 
{
	// TODO: Add your command handler code here
	AttachRoom();
}

void CMainFrame::OnRoomAdd() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
		AddRoom();
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnRoomBuildBridge() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if ((Markedroomp == NULL) || (Markedface == -1)) {
		OutrageMessageBox("You must have a marked face to use this function.");
		return;
	}

	if ((Markedroomp == Curroomp) && (Markedface == Curface)) {
		OutrageMessageBox("The marked room:face must be different from the current room:face.");
		return;
	}

	BuildBridge(Curroomp,Curface,Markedroomp,Markedface);

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnRoomBuildSmoothBridge() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if ((Markedroomp == NULL) || (Markedface == -1)) {
		OutrageMessageBox("You must have a marked face to use this function.");
		return;
	}

	if ((Markedroomp == Curroomp) && (Markedface == Curface)) {
		OutrageMessageBox("The marked room:face must be different from the current room:face.");
		return;
	}

	BuildSmoothBridge(Markedroomp,Markedface,Curroomp,Curface);

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnRoomCombine() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if (Markedroomp == NULL) {
		OutrageMessageBox("You must have a Marked Room for this operation.");
		return;
	}

	int marked_roomnum = ROOMNUM(Markedroomp);

	if (CombineRooms(Curroomp,Markedroomp))
		PrintStatus("Room %d merged into room %d\n",marked_roomnum,ROOMNUM(Curroomp));

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void ned_DeleteRoom()
{
	room *rp;
	int i,p;

	if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to delete Room %d?",ROOMNUM(Curroomp)) != IDYES)
		return;

	// Before deleting the room, we need to change the current face texture for the texture dialog bar
	//Look for connected room
	for (p=0;p<Curroomp->num_portals;p++)
		if (Curroomp->portals[p].croom != -1) {
			rp = &Rooms[Curroomp->portals[0].croom];
			break;
		}

	//If didn't find connected room, look for any room
	if (rp == NULL)
	{
		for (i=0;i<=Highest_room_index;i++)
		{
			if (Rooms[i].used && (i != ROOMNUM(Curroomp))) 
			{
				rp = &Rooms[i];
				break;
			}
		}
	}

	ASSERT(rp != NULL);

	// Update the current face/texture displays
	Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(rp), 0);

	DeleteRoomFromMine(Curroomp);

	// Update the current face/texture displays again
	Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(Curroomp), Curface);

	World_changed = 1;
}

void CMainFrame::OnRoomDelete() 
{
	// TODO: Add your command handler code here
	ned_DeleteRoom();
}

void CMainFrame::OnRoomDeleteFace() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if (Curroomp->faces[Curface].portal_num != -1) {
		OutrageMessageBox("You cannot delete a face that is part of a portal.");
		return;
	}

	if (Curroomp->num_faces == 1) {
		OutrageMessageBox("You cannot delete the only face in a level room.");
		return;
	}

	if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to delete Face %d from Room %d?",Curface,ROOMNUM(Curroomp)) != IDYES)
		return;

	DeleteRoomFace(Curroomp,Curface);

	PrintStatus("Face %d deleted from room %d.",ROOMNUM(Curroomp),Curface);

	if (Curface == Curroomp->num_faces)
		Curface--;

	if (Markedface == Curroomp->num_faces)
		Markedface--;

	// Update the current face/texture displays
	Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(Curroomp), Curface);

	World_changed = 1;	

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnRoomDeletePortal() 
{
	// TODO: Add your command handler code here
	if (Curportal == -1) {
		OutrageMessageBox("You must have a current portal for this operation.");
		return;
	}

	if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to delete portal %d from room %d (& its connecting portal)?",Curportal,ROOMNUM(Curroomp)) == IDYES) {
		DeletePortalPair(Curroomp,Curportal);
		Curportal = -1;
	}
}

void CMainFrame::OnRoomDeleteVert() 
{
	// TODO: Add your command handler code here
	if (Curface != -1 && Curvert != -1)
	{

	if (Curroomp->faces[Curface].num_verts <= 3) {
		OutrageMessageBox("You cannot delete a vertex from a face with three or fewer vertices.");
		return;
	}

	if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to delete vertex %d (%d) from room %d face %d?",Curvert,Curroomp->faces[Curface].face_verts[Curvert],ROOMNUM(Curroomp),Curface) == IDYES) {
		DeletePointFromFace(Curroomp,Curface,Curvert);
		Curvert = 0;
	}

	}
	else
		OutrageMessageBox("You must have a face and vertex selected!");
}

void CMainFrame::OnRoomDropRoom() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if (theApp.m_pFocusedRoomFrm != NULL && theApp.m_pFocusedRoomFrm->m_Prim.roomp != NULL)
	{
		if (ROOMNUM(Curroomp) == -1) {
			OutrageMessageBox("You must have a current room for this operation");
			return;
		}

		DropRoom(Curroomp,Curface,ROOMNUM(theApp.m_pFocusedRoomFrm->m_Prim.roomp));
	}
	else
		OutrageMessageBox("No room to drop.");

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnRoomJoinAdjacentFaces() 
{
	// TODO: Add your command handler code here
	if (Markedroomp == NULL) {
		OutrageMessageBox("You must have a Marked Room for this operation.");
		return;
	}

	if (Markedroomp == Curroomp) {
		OutrageMessageBox("The Marked room cannot be the same as the Current room for this operation.");
		return;
	}

	JoinAllAdjacentFaces(Curroomp,Markedroomp);
}

void CMainFrame::OnRoomJoinRooms() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if ((Markedroomp == NULL) || (Markedface == -1)) {
		OutrageMessageBox("You must have a marked face to use this function.");
		return;
	}

	JoinRooms(Curroomp,Curface,Markedroomp,Markedface);

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnRoomJoinRoomsExact() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if ((Markedroomp == NULL) || (Markedface == -1)) {
		OutrageMessageBox("You must have a marked face to use this function.");
		return;
	}

	JoinRoomsExact(Curroomp,Curface,Markedroomp,Markedface);

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnRoomMark() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	Markedroomp = Curroomp;
	Markedface = Curface;
	Markededge = Curedge;
	Markedvert = Curvert;

	PrintStatus("Room:face %d:%d marked.",ROOMNUM(Markedroomp),Markedface);

	State_changed = 1;

	}
	else
		OutrageMessageBox("You must have a face, edge, and vert selected!");
}

void CMainFrame::OnRoomPropagateToAll() 
{
	// TODO: Add your command handler code here
	int answer;

	if (Curface != -1)
	{

	answer = OutrageMessageBox(MBOX_YESNOCANCEL,"Propagate only to faces with the same texture?\n(Answering No will propagate to all faces in the room.)");
	
	if (answer == IDCANCEL)
		return;

	PropagateToAllFaces(Curroomp,Curface,(answer==IDYES));

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnRoomSnapPlacedRoom() 
{
	// TODO: Add your command handler code here
	ASSERT((Placed_room != -1) || (Placed_group != NULL));

	if (Curface != -1 && Curvert != -1)
	{

	if ((Curroomp != Placed_baseroomp) || (Curface != Placed_baseface)) {
		OutrageMessageBox("The current room & face must be the same as the base room & face for this operation.");
		return;
	}

	SnapRoom(Curvert);

	}
	else
		OutrageMessageBox("You must have a face and vert selected!");
}

void CMainFrame::OnRoomSnapPointToEdge() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if (Markedroomp == NULL) {
		OutrageMessageBox("You must have a marked vertex to use this function.");
		return;
	}

	SnapPointToEdge(Markedroomp,Markedroomp->faces[Markedface].face_verts[Markedvert],
		&Curroomp->verts[Curroomp->faces[Curface].face_verts[Curedge]],
		&Curroomp->verts[Curroomp->faces[Curface].face_verts[(Curedge+1)%Curroomp->faces[Curface].num_verts]]);

	}
	else
		OutrageMessageBox("You must have a face and edge selected!");
}

void CMainFrame::OnRoomSnapPointToFace() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if (Markedroomp == NULL) {
		OutrageMessageBox("You must have a marked vertex to use this function.");
		return;
	}

	SnapPointToFace(Markedroomp,Markedroomp->faces[Markedface].face_verts[Markedvert],
		&Curroomp->verts[Curroomp->faces[Curface].face_verts[Curedge]],
		&Curroomp->faces[Curface].normal);

	}
	else
		OutrageMessageBox("You must have a face, edge, and vert selected!");
}

void CMainFrame::OnRoomSnapPointToPoint() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if (Markedroomp == NULL) {
		OutrageMessageBox("You must have a marked vertex to use this function.");
		return;
	}

	SnapPointToPoint(Markedroomp,Markedroomp->faces[Markedface].face_verts[Markedvert],
							Curroomp,Curroomp->faces[Curface].face_verts[Curvert]);

	}
	else
		OutrageMessageBox("You must have a face and vert selected!");
}

void CMainFrame::OnRoomSwapMarkedAndCurrentRoomFace() 
{
	// TODO: Add your command handler code here
	if (Curface != -1)
	{

	if (Markedroomp == NULL)
		return;

	room *troomp = Markedroomp;
	int tface = Markedface,tedge = Markededge, tvert = Markedvert;

	Markedroomp = Curroomp;
	Markedface = Curface;
	Markededge = Curedge;
	Markedvert = Curvert;

	theApp.m_pLevelWnd->SetPrim(troomp,tface,Curportal,tedge,tvert);

	State_changed = 1;

	// Update the current face/texture displays
	Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(Curroomp), Curface);

	}
	else
		OutrageMessageBox("You must have a face selected!");
}

void CMainFrame::OnViewObjects() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_ObjectDialog,!EkIsBarVisible(&m_ObjectDialog),FALSE);	
}

void CMainFrame::OnUpdateViewObjects(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(&m_ObjectDialog));	
}

extern bool Use_software_zbuffer;

void CMainFrame::OnZBuffer() 
{
	// TODO: Add your command handler code here
	Use_software_zbuffer=!Use_software_zbuffer;
	tex_SetZBufferState (Use_software_zbuffer);
	State_changed=1;
}

void CMainFrame::OnUpdateZBuffer(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	static bool first_time = 1, opengl;
	if (first_time) {
		first_time=0;
		opengl = (FindArg("-WindowGL") != 0);
	}

	pCmdUI->Enable(!opengl);
	pCmdUI->SetCheck(opengl || Use_software_zbuffer);
}

void CMainFrame::OnWindowTextureAlignment() 
{
	// TODO: Add your command handler code here
	// If the texture alignment dialog is already up, just set its focus
	if (theApp.m_pTexAlignDlg != NULL)
	{
		theApp.m_pTexAlignDlg->SetFocus();
		return;
	}

	prim *prim = theApp.AcquirePrim();

	// Create the modeless texture alignment dialog
	theApp.m_pTexAlignDlg = new CTexAlignDialog(prim);
	theApp.m_pTexAlignDlg->Create(IDD_TEXTURE_ALIGN,this);
}

void CMainFrame::OnUpdateWindowTextureAlignment(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (theApp.m_pTexAlignDlg != NULL)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CMainFrame::OnWindowTriggers() 
{
	// TODO: Add your command handler code here
	// If the trigger dialog is already up, just set its focus
	if (theApp.m_pTriggerDlg != NULL)
	{
		theApp.m_pTriggerDlg->SetFocus();
		return;
	}

	// Create the modeless trigger dialog
	theApp.m_pTriggerDlg = new CTriggerDialog();
	theApp.m_pTriggerDlg->Create(IDD_TRIGGERDIALOG,this);
}

void CMainFrame::OnUpdateWindowTriggers(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (theApp.m_pTriggerDlg != NULL)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CMainFrame::OnViewDoorways() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_DoorwayDialog,!EkIsBarVisible(&m_DoorwayDialog),FALSE);	
}

void CMainFrame::OnUpdateViewDoorways(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(&m_DoorwayDialog));	
}

void CMainFrame::OnViewLighting() 
{
	// TODO: Add your command handler code here
	// If the lighting dialog is already up, just set its focus
	if (theApp.m_pLightingDlg != NULL)
	{
		theApp.m_pLightingDlg->SetFocus();
		return;
	}

	// Create the modeless trigger dialog
	theApp.m_pLightingDlg = new CLightingDialog();
	theApp.m_pLightingDlg->Create(IDD_LIGHTINGDIALOG,this);
}

void CMainFrame::OnUpdateViewLighting(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (theApp.m_pLightingDlg != NULL)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CMainFrame::OnButtonBoaVis() 
{
	// TODO: Add your command handler code here
	int answer=MessageBox ("Do you wish to compute the BOA vis table?","Question",MB_YESNO);

	if (answer==IDNO)
		return;
	
	MakeBOAVisTable();
}

void CMainFrame::OnUpdateButtonBoaVis(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (theApp.m_pLevelWnd != NULL)
		pCmdUI->Enable(1);
	else
		pCmdUI->Enable(0);
}

void CMainFrame::OnWindowGoals() 
{
	// TODO: Add your command handler code here
	if (theApp.m_pGoalDlg != NULL)
	{
		theApp.m_pGoalDlg->SetFocus();
		return;
	}

	// Create the modeless goal dialog
	theApp.m_pGoalDlg = new CGoalDialog();
	theApp.m_pGoalDlg->Create(IDD_GOALDIALOG,this);
}

void CMainFrame::OnUpdateWindowGoals(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (theApp.m_pGoalDlg != NULL)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

#define BUF_LEN 100000

extern int CheckSoundsourceObjects();
extern char error_buf[BUF_LEN];
extern int error_buf_offset;

CListDialog *mverify_dlg = NULL;

//Test the mine for validity
void ned_VerifyMine()
{
	int r;
	room *rp;
	char str[256];
	int errors=0,bad_normals=0,concave_faces=0,degenerate_faces=0,bad_portals=0,duplicate_faces=0,duplicate_points=0,duplicate_face_points=0,
			unused_points=0,nonplanar_faces=0,mismatched_portals=0,tjoints=0,bad_shells=0,quiet_soundsource_objects=0;

	//Reset error buffer
	error_buf_offset = 0;

	if (mverify_dlg == NULL)
	{
		mverify_dlg = new CListDialog;
		mverify_dlg->Create(IDD_LISTDIALOG);
	}
	else
		mverify_dlg->SetActiveWindow();
	mverify_dlg->SetWindowText("Verify Mine");
	CListBox *lb = (CListBox *) mverify_dlg->GetDlgItem(IDC_LIST1);
	// Delete any strings that may be in the listbox
	while ( lb->DeleteString(0) != LB_ERR );
	CStatic *hdr = (CStatic *) mverify_dlg->GetDlgItem(IDC_HEADER_TEXT);
	hdr->SetWindowText("Mine check results:");
	CStatic *status = (CStatic *) mverify_dlg->GetDlgItem(IDC_STATUS_TEXT);
	status->SetWindowText("Verifying mine...");

	// Go through the mine checking for errors, one error type at a time (we do this so that we can update the list box regularly)

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			bad_normals += CheckNormals(rp);
			errors += bad_normals;
		}
	}
	sprintf(str,"Found %d bad normals",bad_normals);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			concave_faces += CheckConcaveFaces(rp);
			errors += concave_faces;
		}
	}
	sprintf(str,"Found %d concave faces",concave_faces);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			degenerate_faces += CheckDegenerateFaces(rp);
			errors += degenerate_faces;
		}
	}
	sprintf(str,"Found %d degenerate faces",degenerate_faces);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			bad_portals += CheckPortals(rp);
			errors += bad_portals;
		}
	}
	sprintf(str,"Found %d bad portals",bad_portals);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			duplicate_faces += CheckDuplicateFaces(rp);
			errors += duplicate_faces;
		}
	}
	sprintf(str,"Found %d duplicate faces",duplicate_faces);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			nonplanar_faces += CheckNonPlanarFaces(rp);
			errors += nonplanar_faces;
		}
	}
	sprintf(str,"Found %d non-planar faces",nonplanar_faces);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			duplicate_points += CheckDuplicatePoints(rp);
			errors += duplicate_points;
		}
	}
	sprintf(str,"Found %d duplicate vertices",duplicate_points);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			duplicate_face_points += CheckDuplicateFacePoints(rp);
			errors += duplicate_face_points;
		}
	}
	sprintf(str,"Found %d duplicate face vertices",duplicate_face_points);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			unused_points += CheckUnusedPoints(rp);
			errors += unused_points;
		}
	}
	sprintf(str,"Found %d unused vertices",unused_points);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			mismatched_portals += CheckRoomPortalFaces(rp);
			errors += mismatched_portals;
		}
	}
	sprintf(str,"Found %d mismatched portals",mismatched_portals);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			tjoints += FindTJoints(rp);
			errors += tjoints;
		}
	}
	sprintf(str,"Found %d t-joints",tjoints);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			if ((rp->num_portals > 0) && !(rp->flags & RF_EXTERNAL)) {
				int shell_errors = ComputeRoomShell(rp);
				if (shell_errors) {
					bad_shells++;
					errors++;
				}
			}
		}
	}
	sprintf(str,"Found %d bad shells",bad_shells);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	quiet_soundsource_objects = CheckSoundsourceObjects();
	errors += quiet_soundsource_objects;
	sprintf(str,"Found %d quiet soundsources",quiet_soundsource_objects);
	lb->InsertString(-1,str);
	lb->UpdateWindow();

	// Count the number of unique textures used in the mine
	CountUniqueTextures();

	// Check BOA
	bool boa_ran=false;
	if (BOAGetMineChecksum()==BOA_vis_checksum)
		boa_ran=true;
	else
		errors=true;
	lb->InsertString(-1,boa_ran?"BOA: Valid":"BOA: NOT VALID");
	lb->UpdateWindow();

	// Check terrain occlusion
	bool terrain_occluded=false;
	if (Terrain_occlusion_checksum==(Terrain_checksum+1))
		terrain_occluded=true;
	else	
		errors=true;
	lb->InsertString(-1,terrain_occluded?"Terrain occlusion: Valid":"Terrain occlusion: NOT VALID");
	lb->UpdateWindow();

	// Check terrain dynamic lighting
	bool terrain_volume=false;
	for (int i=0;i<TERRAIN_DEPTH*TERRAIN_WIDTH && !terrain_volume;i++)
	{
		if (Terrain_dynamic_table[i]!=255)
			terrain_volume=true;
	}
	if (!terrain_volume)
		errors=true;
	lb->InsertString(-1,terrain_volume?"Terrain volume lighting: Valid":"Terrain volume lighting: NOT VALID");
	lb->UpdateWindow();

	mprintf((0,"Error buf size = %d\n",strlen(error_buf)));
	DumpTextToClipboard(error_buf);

	//Show message
	if (errors)
		status->SetWindowText("NOTE: Check the system's clipboard for detailed error info, as well as texture counts for this mine.");
	else
		status->SetWindowText("NOTE: This mine has no recorded errors, but check the system's clipboard for texture counts.");
}

void CMainFrame::OnFileVerifyMine() 
{
	// TODO: Add your command handler code here
	ned_VerifyMine();
}

// Counts the number of unique textures in a room, plus gives names of textures used
void ned_CountUniqueRoomTextures(room *rp)
{
	ASSERT(rp != NULL);

//	ushort *texture_tracking=(ushort *)mem_malloc (MAX_TEXTURES*2);
//	ASSERT (texture_tracking);
	ushort texture_tracking[MAX_TEXTURES]; // TODO : tried using mem_malloc and I get unhandled exceptions on HeapAlloc
	memset (texture_tracking,0,MAX_TEXTURES*sizeof(ushort));

	if (rp->used)
	{
		for (int t=0;t<rp->num_faces;t++)
		{
			face *fp=&rp->faces[t];

			if (fp->portal_num!=-1)
			{
				if (!(rp->portals[fp->portal_num].flags & PF_RENDER_FACES))
					continue;
			}
	
			if (fp->tmap!=-1 && !(GameTextures[fp->tmap].flags & (TF_PROCEDURAL|TF_TEXTURE_64|TF_TEXTURE_32)))
			{
				texture_tracking[fp->tmap]++;
			}
		}
	}

	// Now count totals
	int total=0,total_with_lights=0;
	for (int i=0;i<MAX_TEXTURES;i++)
	{
		if (texture_tracking[i])
		{
			if (!(GameTextures[i].flags & TF_LIGHT))
			{
				total++;
				total_with_lights++;
			}
			else
				total_with_lights++;
		}
	}
	
	CheckError ("There are %d unique 128x128 textures (excluding lights) in this room:\n",total);
	CheckError ("There are %d unique 128x128 textures (including lights) in this room:\n",total_with_lights);

	if (total>60)
		CheckError ("ERROR: YOU HAVE MORE THAT 60 128x128 TEXTURES...YOU *MUST* FIX THIS!\n");

	for (i=0;i<MAX_TEXTURES;i++)
	{
		if (texture_tracking[i] && !(GameTextures[i].flags & TF_LIGHT))
		{
			CheckError ("%d : %s %s bmp=%s\n",texture_tracking[i],GameTextures[i].name,(GameTextures[i].flags & TF_ANIMATED)?"(Animated)":"",(GameTextures[i].flags & TF_ANIMATED)?"":GameBitmaps[GameTextures[i].bm_handle].name);
		}
	}

	for (i=0;i<MAX_TEXTURES;i++)
	{
		if (texture_tracking[i] && (GameTextures[i].flags & TF_LIGHT))
		{
			CheckError ("%d : %s %s %s bmp=%s\n",texture_tracking[i],GameTextures[i].name,(GameTextures[i].flags & TF_ANIMATED)?"(Animated)":"",(GameTextures[i].flags & TF_LIGHT)?"(Light)":"",(GameTextures[i].flags & TF_ANIMATED)?"":GameBitmaps[GameTextures[i].bm_handle].name);
		}
	}

//	mem_free (texture_tracking);
}

CListDialog *rverify_dlg = NULL;

void ned_VerifyRoom(room *rp)
{
	char str[256];
	int errors=0,bad_normals,concave_faces,degenerate_faces,duplicate_faces,duplicate_points,duplicate_face_points,unused_points,nonplanar_faces,tjoints;

	//Reset error buffer
	error_buf_offset = 0;

	if (rverify_dlg == NULL)
	{
		rverify_dlg = new CListDialog;
		rverify_dlg->Create(IDD_LISTDIALOG);
	}
	else
		rverify_dlg->SetActiveWindow();
	rverify_dlg->SetWindowText("Verify Room");
	CListBox *lb = (CListBox *) rverify_dlg->GetDlgItem(IDC_LIST1);
	// Delete any strings that may be in the listbox
	while ( lb->DeleteString(0) != LB_ERR );
	CStatic *hdr = (CStatic *) rverify_dlg->GetDlgItem(IDC_HEADER_TEXT);
	hdr->SetWindowText("Room check results:");
	CStatic *status = (CStatic *) rverify_dlg->GetDlgItem(IDC_STATUS_TEXT);
	status->SetWindowText("Verifying room...");

	bad_normals = CheckNormals(rp);
	errors += bad_normals;

	concave_faces = CheckConcaveFaces(rp);
	errors += concave_faces;

	degenerate_faces = CheckDegenerateFaces(rp);
	errors += degenerate_faces;

	duplicate_faces = CheckDuplicateFaces(rp);
	errors += duplicate_faces;

	nonplanar_faces = CheckNonPlanarFaces(rp);
	errors += nonplanar_faces;

	duplicate_points = CheckDuplicatePoints(rp);
	errors += duplicate_points;

	duplicate_face_points = CheckDuplicateFacePoints(rp);
	errors += duplicate_face_points;

	unused_points = CheckUnusedPoints(rp);
	errors += unused_points;

	tjoints = FindTJoints(rp);
	errors += tjoints;

	ned_CountUniqueRoomTextures(rp);

	// Interestingly, CListBox::AddString does not add the strings in the correct order, so we use CListBox::InsertString instead.
	sprintf(str,"Found %d bad normals",bad_normals);
	lb->InsertString(-1,str);
	sprintf(str,"Found %d concave faces",concave_faces);
	lb->InsertString(-1,str);
	sprintf(str,"Found %d degenerate faces",degenerate_faces);
	lb->InsertString(-1,str);
	sprintf(str,"Found %d duplicate faces",duplicate_faces);
	lb->InsertString(-1,str);
	sprintf(str,"Found %d non-planar faces",nonplanar_faces);
	lb->InsertString(-1,str);
	sprintf(str,"Found %d duplicate vertices",duplicate_points);
	lb->InsertString(-1,str);
	sprintf(str,"Found %d duplicate face vertices",duplicate_face_points);
	lb->InsertString(-1,str);
	sprintf(str,"Found %d unused vertices",unused_points);
	lb->InsertString(-1,str);
	sprintf(str,"Found %d t-joints",tjoints);
	lb->InsertString(-1,str);

	mprintf((0,"Error buf size = %d\n",strlen(error_buf)));
	DumpTextToClipboard(error_buf);

	//Show message
	if (errors)
		status->SetWindowText("NOTE: Check the system's clipboard for detailed error info, as well as texture counts for this room.");
	else
		status->SetWindowText("NOTE: This room has no recorded errors, but check the system's clipboard for texture counts.");
}

void CMainFrame::OnFileVerifyRoom() 
{
	// TODO: Add your command handler code here
	room *rp = theApp.AcquirePrim()->roomp;
	ned_VerifyRoom(rp);
}


void CMainFrame::OnHelpContents() 
{
	// TODO: Add your command handler code here
	WinHelp(0,HELP_FINDER);
}

void CMainFrame::OnRoomUnplaceRoom() 
{
	// TODO: Add your command handler code here
	if (Placed_door!=-1) {
		FreeRoom (&Rooms[Placed_room]);
		Placed_door = -1;
	}

	Placed_room = -1;
	Placed_group = NULL;

	State_changed = 1;
}

void CMainFrame::OnUpdateRoomUnplaceRoom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (Placed_room != -1) {
		pCmdUI->SetText((Placed_door != -1)?"Un-place Door":"Un-place Room");
		pCmdUI->Enable(1); 
	}
	else if (Placed_group != NULL) {
		pCmdUI->SetText("Un-place Group");
		pCmdUI->Enable(1); 
	}
	else
		pCmdUI->Enable(0);
}

void CMainFrame::OnFileRemoveExtraPoints() 
{
	// TODO: Add your command handler code here
	// Force unmark verts in current room, just to be safe
	if (theApp.m_pRoomFrm != NULL)
	{
		int saved = Editor_state.mode;
		Editor_state.mode = MODE_VERTEX;
		theApp.m_pRoomFrm->UnMarkAll();
		Editor_state.mode = saved;
	}
	// Remove the extra verts
	RemoveAllDuplicateAndUnusedPoints();
}

void CMainFrame::OnUpdateRoomAttachRoom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(Placed_room != -1); 
}

void CMainFrame::OnUpdateRoomSnapPlacedRoom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(((Placed_room != -1) || (Placed_group != NULL)) && (Placed_baseroomp != NULL)); 

	if (Placed_room != -1) {
		pCmdUI->SetText((Placed_door != -1)?"Snap Placed Door":"Snap Placed Room");
		pCmdUI->Enable(1); 
	}
	else if (Placed_group != NULL) {
		pCmdUI->SetText("Snap Placed Group");
		pCmdUI->Enable(1); 
	}
	else
		pCmdUI->Enable(0);
}

void CMainFrame::OnWindowRoomProperties() 
{
	// TODO: Add your command handler code here
	// If the room properties dialog is already up, just set its focus
	if (theApp.m_pRoomPropsDlg != NULL)
	{
		theApp.m_pRoomPropsDlg->SetFocus();
		return;
	}

	// Create the modeless room properties dialog
	theApp.m_pRoomPropsDlg = new CRoomProperties();
	theApp.m_pRoomPropsDlg->Create(IDD_ROOM_PROPERTIES,this);
}

void CMainFrame::OnUpdateWindowRoomProperties(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code ~here
	if (theApp.m_pRoomPropsDlg != NULL)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CMainFrame::RoomProperties() 
{
	OnWindowRoomProperties();
}

void CMainFrame::OnFileRemoveExtraPointsInRoom() 
{
	// TODO: Add your command handler code here
	if (theApp.m_pFocusedRoomFrm != NULL)
		theApp.m_pFocusedRoomFrm->RemoveExtraPoints();
}


void CMainFrame::OnRoomSelectByNumber() 
{
	// TODO: Add your command handler code here
	int n = 0;

	if (InputNumber(&n,"Select Room","Enter room number to select",this)) {

		if ((n > Highest_room_index) || !Rooms[n].used) {
			OutrageMessageBox("Invalid room number.");
			return;
		}

		theApp.m_pLevelWnd->SetPrim(&Rooms[n],0,-1,0,0);

		PrintStatus("Room %d selected.",n);

		State_changed = 1;
	}
}

void CMainFrame::OnRoomSelectFaceByNumber() 
{
	// TODO: Add your command handler code here
	int n = 0;

	if (InputNumber(&n,"Select Face","Enter face number to select",this)) {

		if (n >= Curroomp->num_faces) {
			OutrageMessageBox("Invalid face number.");
			return;
		}

		Curface = n;
		Curedge = Curvert = 0;

		PrintStatus("Face %d selected.",Curface);

		State_changed = 1;
	}
}

#define NUM_MINERVA 4

typedef struct {
	char strings[NUM_MINERVA][256];
	UINT icons[NUM_MINERVA];
} Minrva;

Minrva Minerva;

void CMainFrame::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	static int count = -1;

	switch (nIDEvent)
	{
	case TIMER_MINERVA:
		if (count == -1)
		{
			strcpy(Minerva.strings[0],"Happy now?");
			Minerva.icons[0] = MB_ICONQUESTION;
			strcpy(Minerva.strings[1],"Usually, I don't get that upset about your habits, but this...");
			Minerva.icons[1] = MB_ICONINFORMATION;
			strcpy(Minerva.strings[2],"THIS IS JUST OUTRAGEOUS!! What were you THINKING?!! That someone wants to play your stupid Minerva clone?\n\n"
				"Don't we all have something better to do?");
			Minerva.icons[2] = MB_ICONEXCLAMATION;
			strcpy(Minerva.strings[3],"I won't forget about this, y'know. Sniff.");
			Minerva.icons[3] = MB_ICONINFORMATION;
			count = 0;
		}
		AfxMessageBox(Minerva.strings[count],Minerva.icons[count]);
		if (++count == NUM_MINERVA)
		{
			KillTimer(TIMER_MINERVA);
			count = 0;
		}
		break;

	default:
		break;
	}
	
	CMDIFrameWnd::OnTimer(nIDEvent);
}


void CMainFrame::OnUpdateStatusMarked(CCmdUI *pCmdUI)
{
	pCmdUI->Enable();
	CString strMarked;
	int num_verts=0,num_faces=0;

	void *wnd = theApp.AcquireWnd();
	if (wnd != NULL)
	{
		if (wnd == theApp.m_pFocusedRoomFrm)
		{
			num_verts = ((CRoomFrm *) wnd)->m_Num_marked_verts;
			num_faces = ((CRoomFrm *) wnd)->m_Num_marked_faces;
		}
	}

	strMarked.Format("Marked: V: %d F: %d",num_verts,num_faces);
	pCmdUI->SetText(strMarked);
}


void CMainFrame::OnDropFiles(HDROP hDropInfo) 
{
	// TODO: Add your message handler code here and/or call default
	char *filename = (char *)mem_malloc(_MAX_PATH);

	DragQueryFile(hDropInfo,0,filename,_MAX_PATH);

	CString strFileName(filename);

	theApp.OpenNedFile(strFileName);

	DragFinish(hDropInfo);

	mem_free(filename);
}

extern float Gravity_strength;

void CMainFrame::OnFileGravity() 
{
	// TODO: Add your command handler code here
	char str[20] = "";
	char message[256] = "";

	sprintf(message,"Gravity is currently set to %.2f. Enter a new value for gravity (32.2 is Earth):",-Gravity_strength);
	if (! InputString(str,sizeof(str),"Edit Gravity",message))
		return;

	Gravity_strength = -atof(str);

	World_changed = 1;

	sprintf(message,"Gravity has been set to %.2f.",-Gravity_strength);
	OutrageMessageBox(message);
}

void CMainFrame::OnMoveMine() 
{
	// TODO: Add your command handler code here
	vector room_center;
	int i=0,j=0;
	object *objp;
	vector newpos,objpos;
	short vert_list[MAX_VERTS_PER_ROOM];
	room *rp = NULL;

	OutrageMessageBox("\
In the following dialog, enter the current room's new center position.\n\
All rooms, objects, and other elements of the mine will be moved along with the current room.\n\
Do not let any part of the mine go outside of the bounds (x,z) = (-2048,-2048), (2048,2048) or your level\n\
will not run properly in multiplayer.\n\n\
(NOTE: Moving the mine requires that you recompute the BOA vis table and relight the level afterwards.)");

	ComputeRoomBoundingSphere(&newpos,Curroomp); // ComputeRoomCenter(&newpos,Curroomp);

	if (! InputVector(&newpos,"Set Current Room's Position","Where would you like to move the current room?\n",this))
		return;

	// Move the current room to the specified position (and bring the entire mine with it)
	while ( ! Rooms[i++].used );
	ComputeRoomBoundingSphere(&room_center,&Rooms[i-1]); // ComputeRoomCenter(&room_center,&Rooms[i-1]);
	for (i=0,rp=Rooms; i<=Highest_room_index; i++,rp++)
	{
		if (!rp->used)
			continue;
		for (j=0; j<rp->num_verts; j++)
			vert_list[j] = j;
		MoveVerts(rp,vert_list,rp->num_verts,newpos-room_center);
		for (j=rp->objects; j!=-1; j=Objects[j].next)
		{
			objp = &Objects[j];
			ASSERT(objp->roomnum == i);
			objpos = objp->pos+newpos-room_center;
			ObjSetPos(objp,&objpos,i,&objp->orient,false);
		}
	}

	World_changed = 1;

	OutrageMessageBox("Move completed. You should recompute the BOA vis table and relight the level.");
}

void CMainFrame::OnViewSounds() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_SoundDialog,!EkIsBarVisible(&m_SoundDialog),FALSE);	
}

void CMainFrame::OnUpdateViewSounds(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(&m_SoundDialog));	
}

void CMainFrame::OnWindowMatcens() 
{
	// TODO: Add your command handler code here
	if (theApp.m_pMatcensDlg != NULL)
	{
		theApp.m_pMatcensDlg->SetFocus();
		return;
	}

	// Create the modeless room properties dialog
	theApp.m_pMatcensDlg = new CMatcenDialog();
	theApp.m_pMatcensDlg->Create(IDD_MATCENS,this);
}

void CMainFrame::OnUpdateWindowMatcens(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	if (theApp.m_pMatcensDlg != NULL)
		pCmdUI->SetCheck(1);
	else
		pCmdUI->SetCheck(0);
}

void CMainFrame::OnViewCameraSlewer() 
{
	// TODO: Add your command handler code here
	ShowControlBar(gCameraSlewer,!EkIsBarVisible(gCameraSlewer),FALSE);	
}

void CMainFrame::OnUpdateViewCameraSlewer(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(gCameraSlewer));	
}

//Copies the selected rooms to the scrap buffer.
//Returns:	true if copied ok, 0 if some error
int CopySelectedRooms()
{
	if (N_selected_rooms == 0) {
		OutrageMessageBox("You must have rooms selected for this operation.");
		return 0;
	}

	if (! Markedroomp) {
		OutrageMessageBox("You must have a marked room for this operation.");
		return 0;
	}

	if (! IsRoomSelected(ROOMNUM(Markedroomp))) {
		OutrageMessageBox("The Marked room must be selected for this operation.");
		return 0;
	}

	if (! ((Markedroomp->faces[Markedface].portal_num == -1) || (Markedroomp->portals[Markedroomp->faces[Markedface].portal_num].croom == -1) || !IsRoomSelected(Markedroomp->portals[Markedroomp->faces[Markedface].portal_num].croom))) {
		OutrageMessageBox("The Marked room/face must be unattached for this operation.");
		return 0;
	}

	//Get rid of old contents of scrap buffer
	if (Scrap)
		FreeGroup(Scrap);

	Scrap = CopyGroup(N_selected_rooms,Selected_rooms,ROOMNUM(Markedroomp),Markedface);

	return 1;
}

void CMainFrame::OnEditCut() 
{
	// TODO: Add your command handler code here
	if (CopySelectedRooms()) {
		int n = N_selected_rooms;
		DeleteGroup(N_selected_rooms,Selected_rooms);
		EditorStatus("%d rooms cut to group",n);
		ClearRoomSelectedList();
	}
}

void CMainFrame::OnEditCopy() 
{
	// TODO: Add your command handler code here
	if (CopySelectedRooms())
		EditorStatus("%d rooms copied to group",N_selected_rooms);
}

void CMainFrame::OnEditAttach() 
{
	// TODO: Add your command handler code here
	ASSERT(Placed_group != NULL);

	AttachGroup();
}

void CMainFrame::OnEditDelete() 
{
	// TODO: Add your command handler code here
	if (N_selected_rooms == 0) {
		OutrageMessageBox("You must have rooms selected for this operation.");
		return;
	}

	DeleteGroup(N_selected_rooms,Selected_rooms);
	EditorStatus("%d rooms deleted",N_selected_rooms);
	ClearRoomSelectedList();
}

void CMainFrame::OnEditLoadscrap() 
{
	// TODO: Add your command handler code here
	static char filter[] = "D3 Group Files (*.grp)|*.grp||";
	CString SelectedFile;
	CString DefaultPath;
	group *newscrap;
	CFileDialog *fd;
	DefaultPath = theApp.GetProfileString("Defaults","LastGroupDir","");
	char filename[_MAX_PATH];
	char name[_MAX_PATH];
	char ext[10];
	char path[_MAX_PATH*2];

	//Need to do something with the default path
	_chdir(DefaultPath);
	fd = new CFileDialog(true,".grp",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,filter);

	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();

	delete fd;

	SplitPath(SelectedFile,path,name,ext);

	newscrap = LoadGroup(SelectedFile.GetBuffer(0));

	if (newscrap) {
		if (Scrap)
			FreeGroup(Scrap);
		Scrap = newscrap;
		theApp.WriteProfileString("Defaults","LastGroupDir",path);
		sprintf(filename, "%s%s", name, ext);
		EditorStatus("Group loaded from %s",filename);
	}
}

void CMainFrame::OnEditSavescrap() 
{
	// TODO: Add your command handler code here
	if (Scrap == NULL) {
		OutrageMessageBox("There is no group to save.");
		return;
	}

	static char filter[] = "D3 Group Files (*.grp)|*.grp||";
	CString SelectedFile;
	CString DefaultPath;
	CFileDialog *fd;
	DefaultPath = theApp.GetProfileString("Defaults","LastGroupDir","");
	char filename[_MAX_PATH];
	char name[_MAX_PATH];
	char ext[10];
	char path[_MAX_PATH*2];

	//Need to do something with the default path
	_chdir(DefaultPath);
	fd = new CFileDialog(false,".grp",NULL,OFN_HIDEREADONLY,filter);

	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();

	delete fd;

	SplitPath(SelectedFile,path,name,ext);

	SaveGroup(SelectedFile.GetBuffer(0),Scrap);	

	theApp.WriteProfileString("Defaults","LastGroupDir",path);

	sprintf(filename, "%s%s", name, ext);

	EditorStatus("Group saved to %s",filename);
}

void CMainFrame::OnEditRemoveselect() 
{
	// TODO: Add your command handler code here
	RemoveRoomFromSelectedList(ROOMNUM(Curroomp));
}

void CMainFrame::OnEditSelectattached() 
{
	// TODO: Add your command handler code here
	int count;
	
	count = SelectConnectedRooms(ROOMNUM(Curroomp));

	EditorStatus("%d rooms selected",count);
}

void CMainFrame::OnEditUndo() 
{
	// TODO: Add your command handler code here
	
}

void CMainFrame::OnUpdateEditUndo(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(false);
}

void CMainFrame::OnUpdateEditAttach(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(Placed_group != NULL); 
}

void CMainFrame::OnUpdateEditSavescrap(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(Scrap != NULL); 
}

void CMainFrame::OnEditPaste() 
{
	// TODO: Add your command handler code here
	if (! Scrap) {
		OutrageMessageBox("There is no group to place.");
		return;
	}

	if (Curface != -1)
	{
		if (Curroomp->faces[Curface].portal_num != -1) {
			OutrageMessageBox("The selected Room/Face must be free for this operation.");
			return;
		}

		PlaceGroup(Curroomp,Curface,Scrap);
	}
	else
		OutrageMessageBox("You do not have a face selected in the level!");
}

void CMainFrame::OnEditClearselected() 
{
	// TODO: Add your command handler code here
	ClearRoomSelectedList();
}

void CMainFrame::OnEditAddselect() 
{
	// TODO: Add your command handler code here
	AddRoomToSelectedList(ROOMNUM(Curroomp));
}

void CMainFrame::OnRoomMergeObjectIntoRoom() 
{
	// TODO: Add your command handler code here
	ASSERT(Cur_object_index != -1);
	if (MergeObjectIntoRoom(Curroomp,Cur_object_index))
		Cur_object_index = -1;
}

void CMainFrame::OnUpdateRoomMergeObjectIntoRoom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->Enable(Cur_object_index != -1); 
}

void CMainFrame::OnFileScaleMine() 
{
	// TODO: Add your command handler code here
	OutrageMessageBox("\
In the following dialog, enter the scale factor.\n\
All rooms will be scaled in size by this value.\n\
Do not let any part of the mine go outside of the bounds (x,z) = (-2048,-2048), (2048,2048) or your level\n\
will not run properly in multiplayer.\n\n\
(NOTE: Scaling the mine requires that you recompute the BOA vis table and relight the level afterwards.)");

	char buf[10]="";

	if (! InputString(buf,sizeof(buf),"Scale Amount","Enter an amount by which to scale the mine"))
		return;

	float scale = atof(buf);

	int r;
	room *rp;

	for (r=0,rp=Rooms;r<=Highest_room_index;r++,rp++) {
		if (rp->used) {
			for (int v=0;v<rp->num_verts;v++)
				rp->verts[v] *= scale;

			for (int objnum=rp->objects;objnum!=-1;objnum=Objects[objnum].next)
				Objects[objnum].pos *= scale;
		}
	}

	World_changed = 1;

	OutrageMessageBox("Scale completed. You should recompute the BOA vis table and relight the level.");
}

void CMainFrame::OnWindowPathBar() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_PathDialog,!EkIsBarVisible(&m_PathDialog),FALSE);	
}

void CMainFrame::OnUpdateWindowPathBar(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(&m_PathDialog));	
}

void CMainFrame::OnEditUnplaceGroup() 
{
	// TODO: Add your command handler code here
	Placed_group = NULL;

	State_changed = 1;
}

void CMainFrame::OnUpdateEditUnplaceGroup(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool enable;

	(Placed_group != NULL) ? (enable = true) : enable = false;

	pCmdUI->Enable(enable);
}

void CMainFrame::OnWindowEditorBar() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndStatusBar2,!EkIsBarVisible(&m_wndStatusBar2),FALSE);	
}

void CMainFrame::OnUpdateWindowEditorBar(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(&m_wndStatusBar2));	
}

void CMainFrame::OnWindowEditorBar2() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_wndStatusBar2,!EkIsBarVisible(&m_wndStatusBar2),FALSE);	
}

void CMainFrame::OnUpdateWindowEditorBar2(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(&m_wndStatusBar2));	
}

void CMainFrame::OnWindowTerrainBar() 
{
	// TODO: Add your command handler code here
	ShowControlBar(&m_TerrainDialog,!EkIsBarVisible(&m_TerrainDialog),FALSE);	
}

void CMainFrame::OnUpdateWindowTerrainBar(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(EkIsBarVisible(&m_TerrainDialog));	
}

void CMainFrame::OnRoomPlaceTerrainRoom() 
{
	// TODO: Add your command handler code here
	int cellnum;

	if (theApp.m_pFocusedRoomFrm == NULL || theApp.m_pFocusedRoomFrm->m_Prim.roomp == NULL) {
		OutrageMessageBox("You must have a current room for this operation");
		return;
	}

	if (theApp.m_pFocusedRoomFrm->m_Prim.face == -1) {
		OutrageMessageBox("You must have a face selected on the current room for this operation");
		return;
	}

	if (Num_terrain_selected != 1) {
		OutrageMessageBox("You must have one and only one cell selected for this operation");
		return;
	}
	else {
		for (cellnum=0;cellnum<TERRAIN_WIDTH*TERRAIN_DEPTH;cellnum++)
			if (TerrainSelected[cellnum])
				break;

		ASSERT(cellnum != TERRAIN_WIDTH*TERRAIN_DEPTH);
	}

	int result = OutrageMessageBox(MBOX_YESNOCANCEL,"Do you want to align to the terrain? NO will align with gravity.");

	if (result == IDCANCEL)
		return;

	PlaceExternalRoom(cellnum,ROOMNUM(theApp.m_pFocusedRoomFrm->m_Prim.roomp),theApp.m_pFocusedRoomFrm->m_Prim.face,(result == IDYES));
}

#define MARK_TEXTURE	1

void CMainFrame::OnRoomLinkToNewExternal() 
{
	// TODO: Add your command handler code here
	void LinkToExternalRoom(room *rp,int nfaces,int *facenums);
	int n=0,facelist[MAX_FACES_PER_ROOM];

	for (int i=0;i<Curroomp->num_faces;i++) {
		if ((Curroomp->faces[i].portal_num == -1) && (Curroomp->faces[i].tmap == MARK_TEXTURE))
			facelist[n++] = i;
	}

	if (n) {
		if (OutrageMessageBox(MBOX_YESNO,"Do you want to create a new room linked to the %d selected faces in room %d?",n,ROOMNUM(Curroomp)) != IDYES)
			return;

		LinkToExternalRoom(Curroomp,n,facelist);
	}
	else
		OutrageMessageBox("You have no faces selected in room %d.\n\nTo select a face, texture it with Texture %d, \"%s\".",ROOMNUM(Curroomp),MARK_TEXTURE,GameTextures[MARK_TEXTURE].name);
}

void CMainFrame::OnEditPlaceTerrain() 
{
	// TODO: Add your command handler code here
	if (! Scrap) {
		OutrageMessageBox("There is no group to place.");
		return;
	}

	int cellnum;

	if (Num_terrain_selected != 1) {
		OutrageMessageBox("You must have one and only one cell selected for this operation");
		return;
	}
	else {
		for (cellnum=0;cellnum<TERRAIN_WIDTH*TERRAIN_DEPTH;cellnum++)
			if (TerrainSelected[cellnum])
				break;

		ASSERT(cellnum != TERRAIN_WIDTH*TERRAIN_DEPTH);
	}


	PlaceGroupTerrain(cellnum,Scrap,true);
}
