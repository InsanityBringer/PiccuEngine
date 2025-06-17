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
 

// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "NewEditor.h"
#include "ChildView.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CChildView

CChildView::CChildView()
{
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView,CWnd )
	//{{AFX_MSG_MAP(CChildView)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_SIZE()
	ON_WM_ACTIVATE()
	ON_WM_SETFOCUS()
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
// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.style |= WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS;

	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	m_LevelWnd.Render();

	// Do not call CWnd::OnPaint() for painting messages
}



int CChildView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO: Add your specialized creation code here
	RECT rect;

	GetClientRect(&rect);
	m_LevelWnd.Create(rect, this, "", 0);
	return 0;
}

void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_LevelWnd.OnKeyDown(nChar, nRepCnt, nFlags);

//	CWnd ::OnKeyDown(nChar, nRepCnt, nFlags);
}

void CChildView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_LevelWnd.OnKeyUp(nChar, nRepCnt, nFlags);
	//CWnd ::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CChildView::OnSize(UINT nType, int cx, int cy) 
{
	CWnd ::OnSize(nType, cx, cy);
	RECT rect;

	GetClientRect(&rect);
	
	m_LevelWnd.MoveWindow(&rect);
	
}

void CChildView::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
	m_LevelWnd.OnActivate(nState, pWndOther, bMinimized);
}

void CChildView::OnSetFocus(CWnd* pOldWnd) 
{
	m_LevelWnd.OnSetFocus(pOldWnd);
	
	
	//Reset all key states, because if a key is lifted while we weren't active,
	//when we return we will still think it's down.
	
	//memset(&keys,0,sizeof(keys));
	
}

void CChildView::OnKillFocus(CWnd* pNewWnd) 
{
	
	m_LevelWnd.OnKillFocus(pNewWnd);
	//Reset all key states, because if a key is lifted while we weren't active,
	//when we return we will still think it's down.
	
	//memset(&keys,0,sizeof(keys));
	
}

void CChildView::OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_LevelWnd.OnSysKeyDown(nChar, nRepCnt, nFlags);
	
	
}

void CChildView::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	m_LevelWnd.OnSysKeyUp(nChar, nRepCnt, nFlags);
	
	
}

void CChildView::OnMButtonDown(UINT nFlags, CPoint point) 
{
	m_LevelWnd.OnMButtonDown(nFlags, point);
}

void CChildView::OnMButtonUp(UINT nFlags, CPoint point) 
{
		
	m_LevelWnd.OnMButtonUp(nFlags, point);
}

void CChildView::OnRButtonDown(UINT nFlags, CPoint point) 
{
	
	
	m_LevelWnd.OnRButtonDown(nFlags, point);
}

void CChildView::OnRButtonUp(UINT nFlags, CPoint point) 
{
	m_LevelWnd.OnRButtonUp(nFlags, point);
	
	//CWnd ::OnRButtonUp(nFlags, point);
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
	return m_LevelWnd.OnMouseWheel(nFlags, zDelta, pt);

	//return CWnd ::OnMouseWheel(nFlags, zDelta, pt);
}
