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
 #if !defined(AFX_LEVELFRAME_H__8F6C1720_EC20_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_LEVELFRAME_H__8F6C1720_EC20_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


// LevelFrame.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLevelFrame frame

class CLevelFrame : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CLevelFrame)
protected:
	CLevelFrame();           // protected constructor used by dynamic creation

// Attributes
public:

// Operations
public:
	void InitLevelResources();
	void ShutdownLevelResources();
	void CloseLevel();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLevelFrame)
	public:
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CLevelFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
	//{{AFX_MSG(CLevelFrame)
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnWireframe();
	afx_msg void OnUpdateWireframe(CCmdUI* pCmdUI);
	afx_msg void OnCenterMine();
	afx_msg void OnCenterOrigin();
	afx_msg void OnCenterRoom();
	afx_msg void OnViewMoveCameraToCurrentFace();
	afx_msg void OnViewMoveCameraToCurrentObject();
	afx_msg void OnClose();
	afx_msg void OnTextured();
	afx_msg void OnUpdateTextured(CCmdUI* pCmdUI);
	afx_msg void OnViewTexturedOutline();
	afx_msg void OnUpdateViewTexturedOutline(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnDisplayCurrentRoomView();
	afx_msg void OnViewMoveCameraToCurrentRoom();
	afx_msg void OnPaint();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	Cned_LevelWnd m_LevelWnd;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEVELFRAME_H__8F6C1720_EC20_11D2_A6A1_006097E07445__INCLUDED_)
