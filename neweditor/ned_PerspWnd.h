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
 #if !defined(AFX_NED_PERSPWND_H__EFD19520_EC59_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_NED_PERSPWND_H__EFD19520_EC59_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ned_PerspWnd.h : header file
//

#include "ned_GrWnd.h"

#include "globals.h"	// Added by ClassView

#define PAN_LEFT		0
#define PAN_RIGHT		1
#define PAN_UP			2
#define PAN_DOWN		3
#define ROT_LEFT		4
#define ROT_RIGHT		5
#define ROT_UP			6
#define ROT_DOWN		7
#define BANK_LEFT		8
#define BANK_RIGHT		9
#define ZOOM_INWARD		10
#define ZOOM_OUTWARD	11
#define MOVE_FORWARD	12
#define MOVE_BACKWARD	13

class CRoomFrm;

/////////////////////////////////////////////////////////////////////////////
// Cned_PerspWnd window

class Cned_PerspWnd : public Cned_GrWnd
{
	DECLARE_DYNCREATE(Cned_PerspWnd)

	friend CRoomFrm;

// Construction
public:
	Cned_PerspWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cned_PerspWnd)
	public:
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, LPCTSTR name, UINT nID);
	//}}AFX_VIRTUAL

// Implementation
public:
	unsigned int m_nID;
	char m_Title[128];
	bool m_InFocus;
	bool m_View_changed;
	bool m_State_changed;
	bool m_bTextured;
	bool m_bOutline;
	bool m_bFlat;
	bool m_bCaptured;

	CRoomFrm *m_pParentFrame;	// the parent room frame window
	// Pointers to parent room frame members
	prim *m_pPrim;					// the primitives
	vector *m_pvec_InsertPos; // where we will insert a new vertex
	vector *m_pvec_RefPos; // the origin of the reference frame
	bool *m_pVert_marks; // marked flags for each vertex
	int *m_pNum_marked_verts; // number of marked verts
	bool *m_pFace_marks; // marked flags for each face
	int *m_pNum_marked_faces; // number of marked faces

	// Button down found
	int m_BDOWN_Found_type;
	int m_BDOWN_Found_roomnum;
	int m_BDOWN_Found_facenum;
	int m_BDOWN_Found_x;
	int m_BDOWN_Found_y;

	unsigned int m_TimerHandle;
	camera m_Cam;
	virtual ~Cned_PerspWnd();
	void Render(bool force_repaint = true);
	void TGWRenderRoom(vector *pos,matrix *orient,float zoom,int start_roomnum);
	void InitCamera();
	void CenterRoom(room *rp);
	void CenterFace(room *rp,int facenum);
	void CenterOrigin();
	void PlaceCameraAtRoomFace(room *roomp,int facenum,bool room_center);
	void PlaceCamera(vector target,matrix orient,float dist = 0);
	void AdjustCamera(int code);
	void StopCamera();

	// Generated message map functions
protected:
	//{{AFX_MSG(Cned_PerspWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnCenterRoom();
	afx_msg void OnCenterOrigin();
	afx_msg void OnPaint();
	afx_msg void OnTextured();
	afx_msg void OnWireframe();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnViewMoveCameraToCurrentFace();
	afx_msg void OnCenterFace();
	afx_msg void OnViewMoveCameraToCurrentObject();
	afx_msg void OnUpdateViewMoveCameraToCurrentObject(CCmdUI* pCmdUI);
	afx_msg void OnUpdateViewMoveCameraToCurrentFace(CCmdUI* pCmdUI);
	afx_msg void OnUpdateTextured(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWireframe(CCmdUI* pCmdUI);
	afx_msg void OnViewTexturedOutline();
	afx_msg void OnUpdateViewTexturedOutline(CCmdUI* pCmdUI);
	afx_msg void OnViewMoveCameraToCurrentRoom();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	BOOL m_Movable;						// is this floating?
	BOOL m_StartFlip;					// shall we flip the screen when getting to OnPaint.

	vector m_ViewPos;
	matrix m_ViewMatrix;					// these are used by the object mover.

	//for FindRoomFace()
	vector 	m_last_viewer_eye;
	matrix 	m_last_viewer_orient;
	float		m_last_zoom;
	int		m_last_start_room;
	void TexGrStartOpenGL();
	void TexGrStopOpenGL();

	stMouse m_Mouse;
	stKeys m_Keys;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NED_PERSPWND_H__EFD19520_EC59_11D2_A6A1_006097E07445__INCLUDED_)
