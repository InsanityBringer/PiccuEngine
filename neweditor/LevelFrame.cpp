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
 

// LevelFrame.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "LevelFrame.h"
#include "ned_Object.h"
#include "ned_Trigger.h"
#include "gamepath.h"
#include "levelgoal.h"
#include "matcen.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// CLevelFrame

IMPLEMENT_DYNCREATE(CLevelFrame, CMDIChildWnd)

CLevelFrame::CLevelFrame()
{
}

CLevelFrame::~CLevelFrame()
{
}

BOOL CLevelFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	if( !CMDIChildWnd::PreCreateWindow(cs) )
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.style |= WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CLevelFrame, CMDIChildWnd)
	//{{AFX_MSG_MAP(CLevelFrame)
	ON_WM_SETFOCUS()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(ID_WIREFRAME, OnWireframe)
	ON_UPDATE_COMMAND_UI(ID_WIREFRAME, OnUpdateWireframe)
	ON_COMMAND(ID_CENTER_MINE, OnCenterMine)
	ON_COMMAND(ID_CENTER_ORIGIN, OnCenterOrigin)
	ON_COMMAND(ID_CENTER_ROOM, OnCenterRoom)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTFACE, OnViewMoveCameraToCurrentFace)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTOBJECT, OnViewMoveCameraToCurrentObject)
	ON_WM_CLOSE()
	ON_COMMAND(ID_TEXTURED, OnTextured)
	ON_UPDATE_COMMAND_UI(ID_TEXTURED, OnUpdateTextured)
	ON_COMMAND(ID_VIEW_TEXTURED_OUTLINE, OnViewTexturedOutline)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TEXTURED_OUTLINE, OnUpdateViewTexturedOutline)
	ON_COMMAND(ID_FILE_CLOSE, OnFileClose)
	ON_COMMAND(ID_DISPLAY_CURRENTROOMVIEW, OnDisplayCurrentRoomView)
	ON_COMMAND(ID_VIEW_MOVECAMERATOCURRENTROOM, OnViewMoveCameraToCurrentRoom)
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SIZE()
	ON_WM_KILLFOCUS()
	ON_WM_SYSKEYDOWN()
	ON_WM_SYSKEYUP()
	ON_WM_MBUTTONDOWN()
	ON_WM_MBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEWHEEL()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLevelFrame diagnostics

#ifdef _DEBUG
void CLevelFrame::AssertValid() const
{
	CMDIChildWnd::AssertValid();
}

void CLevelFrame::Dump(CDumpContext& dc) const
{
	CMDIChildWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLevelFrame message handlers

void CLevelFrame::OnSetFocus(CWnd* pOldWnd) 
{
	CMDIChildWnd::OnSetFocus(pOldWnd);
	
	// TODO: Add your message handler code here
	m_LevelWnd.OnSetFocus(pOldWnd);
}

BOOL CLevelFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo) 
{
	// TODO: Add your specialized code here and/or call the base class
	// let the view have first crack at the command
	if (m_LevelWnd.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;
	
	// otherwise, do default handling
	return CMDIChildWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

int CLevelFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CMDIChildWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	theApp.m_ThisLevelFrame = this;
	RECT rect;
	GetClientRect(&rect);
	// create a view to occupy the client area of the frame
	if (!m_LevelWnd.Create(rect, this, "", 0))
	{
		TRACE0("Failed to create level window\n");
		return -1;
	}
	return 0;
}


void CLevelFrame::InitLevelResources()
{
	LevelTexInitLevel();
	LevelObjInitLevel();
	LevelDoorInitLevel();
}

void CLevelFrame::ShutdownLevelResources()
{
	// Reset dialog bars
	Editor_state.SetCurrentRoomFaceTexture(-1, -1);
	Editor_state.SetCurrentLevelObject(-1);
	Editor_state.SetCurrentDoorway(NULL);

	// Mark all resources unused and remove them from their dialogs' lists
	LevelTexPurgeAllTextures();
	LevelObjPurgeAllObjects();
	LevelDoorPurgeAllDoorways();
}


void CLevelFrame::OnDestroy() 
{
	CMDIChildWnd::OnDestroy();
	
	// TODO: Add your message handler code here
	// Close the current room frame if it's open
	if (theApp.m_pRoomFrm != NULL)
		theApp.m_pRoomFrm->DestroyWindow();

	theApp.m_pLevelWnd = NULL;
	theApp.m_ThisLevelFrame = NULL;

	Markedroomp = NULL;
	Markedface = -1;
	Markededge = -1;
	Markedvert = -1;

	//Get rid of old mine
	FreeAllRooms();

	// Get rid of old matcens
	DestroyAllMatcens();

	// Get rid of old level goals
	Level_goals.CleanupAfterLevel();

	extern int Num_objects;

	// Get rid of old objects
	if (Num_objects > 0)
		FreeAllObjects();

	//Reset all the objects
	ResetObjectList();

	//Free all triggers
	FreeTriggers();

	// Resets the game paths to blank
	InitGamePaths();

	//Clear terrain sounds
	ClearTerrainSound();

	ShutdownLevelResources();

	State_changed = true; // force update of dialogs in OnIdle; should probably use a separate Editor_changed var, but oh well
}

void CLevelFrame::OnTextured() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnTextured();
}

void CLevelFrame::OnViewTexturedOutline() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnViewTexturedOutline();
}

void CLevelFrame::OnWireframe() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnWireframe();
}

void CLevelFrame::OnUpdateTextured(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(theApp.m_pLevelWnd->m_bTextured && !theApp.m_pLevelWnd->m_bOutline);
}

void CLevelFrame::OnUpdateViewTexturedOutline(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(theApp.m_pLevelWnd->m_bTextured && theApp.m_pLevelWnd->m_bOutline);
}

void CLevelFrame::OnUpdateWireframe(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	pCmdUI->SetCheck(!theApp.m_pLevelWnd->m_bTextured);
}

void CLevelFrame::OnCenterMine() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnCenterMine();
}

void CLevelFrame::OnCenterOrigin() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnCenterOrigin();
}

void CLevelFrame::OnCenterRoom() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnCenterRoom();
}


void CLevelFrame::OnViewMoveCameraToCurrentFace() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnViewMoveCameraToCurrentFace();
}

void CLevelFrame::OnViewMoveCameraToCurrentObject() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnViewMoveCameraToCurrentObject();
}

void CLevelFrame::OnClose() 
{
	// TODO: Add your message handler code here and/or call default
	if(theApp.m_pLevelWnd->m_Modified)
	{
		if(IDYES==AfxMessageBox("Would you like to save your changes?",MB_YESNO))
		{
			theApp.OnFileSaveAs();
		}
	}

	CMDIChildWnd::OnClose();
}

void CLevelFrame::CloseLevel() 
{
	OnClose();
}

void CLevelFrame::OnFileClose() 
{
	// TODO: Add your command handler code here
	CloseLevel();
}

void CLevelFrame::OnDisplayCurrentRoomView() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnDisplayCurrentRoomView();
}

void CLevelFrame::OnViewMoveCameraToCurrentRoom() 
{
	// TODO: Add your command handler code here
	theApp.m_pLevelWnd->OnViewMoveCameraToCurrentRoom();
}

void CLevelFrame::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	m_LevelWnd.Render();

	// Do not call CMDIChildWnd::OnPaint() for painting messages
}

void CLevelFrame::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	m_LevelWnd.OnKeyDown(nChar, nRepCnt, nFlags);
	
	CMDIChildWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CLevelFrame::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// TODO: Add your message handler code here and/or call default
	m_LevelWnd.OnKeyUp(nChar, nRepCnt, nFlags);
	
	CMDIChildWnd::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CLevelFrame::OnSize(UINT nType, int cx, int cy) 
{
	CMDIChildWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	RECT rect;

	GetClientRect(&rect);
	
	m_LevelWnd.MoveWindow(&rect);
}

void CLevelFrame::OnKillFocus(CWnd* pNewWnd) 
{
	CMDIChildWnd::OnKillFocus(pNewWnd);
	
	// TODO: Add your message handler code here
	m_LevelWnd.OnKillFocus(pNewWnd);
}

void CLevelFrame::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_LevelWnd.OnSysKeyDown(nChar, nRepCnt, nFlags);
	
	
}

void CLevelFrame::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_LevelWnd.OnSysKeyUp(nChar, nRepCnt, nFlags);
	
	
}

void CLevelFrame::OnMButtonDown(UINT nFlags, CPoint point) 
{
	m_LevelWnd.OnMButtonDown(nFlags, point);
}

void CLevelFrame::OnMButtonUp(UINT nFlags, CPoint point) 
{
		
	m_LevelWnd.OnMButtonUp(nFlags, point);
}

void CLevelFrame::OnRButtonDown(UINT nFlags, CPoint point) 
{
	
	
	m_LevelWnd.OnRButtonDown(nFlags, point);
}

void CLevelFrame::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_LevelWnd.OnRButtonUp(nFlags, point);
	
	//CWnd ::OnRButtonUp(nFlags, point);
}

BOOL CLevelFrame::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	return m_LevelWnd.OnMouseWheel(nFlags, zDelta, pt);

	//return CWnd ::OnMouseWheel(nFlags, zDelta, pt);
}
