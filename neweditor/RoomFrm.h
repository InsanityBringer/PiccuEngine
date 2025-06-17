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
 #if !defined(AFX_ROOMFRM_H__469D69E3_EB53_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_ROOMFRM_H__469D69E3_EB53_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RoomFrm.h : header file
//

// pane ID values
#define VIEW_XY				0
#define VIEW_XZ				1
#define VIEW_ZY				2
#define VIEW_3D				3

// timers
#define EXTRUDE_TIMER		5678

/////////////////////////////////////////////////////////////////////////////
// CRoomFrm frame with splitter

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CRoomFrm : public CMDIChildWnd
{
	DECLARE_DYNCREATE(CRoomFrm)

	friend Cned_OrthoWnd;
	friend Cned_PerspWnd;

protected:
	CRoomFrm();           // protected constructor used by dynamic creation

// Attributes
protected:
	CSplitterWnd	m_wndSplitter;
	Cned_PerspWnd	m_wnd3D;
	Cned_OrthoWnd	m_wndXY;
	Cned_OrthoWnd	m_wndXZ;
	Cned_OrthoWnd	m_wndZY;

public:

	unsigned int m_Focused_pane;		// can be 0, 1, 2, or 3

	short m_Textures_in_use[MAX_TEXTURES];

	prim m_Prim;				// the primitives
	vector m_vec_InsertPos; // where we will insert a new vertex (screen-z equals screen-z of ref frame)
	vector m_vec_RefPos; // the origin of the reference frame
	vector m_vec_MousePos; // where the mouse is (screen-z equals screen-z of ref frame)
	bool m_Vert_marks[MAX_VERTS_PER_ROOM]; // marked flags for each vertex
	int m_Num_marked_verts; // number of marked verts
	bool m_Face_marks[MAX_FACES_PER_ROOM]; // marked flags for each face
	int m_Num_marked_faces; // number of marked faces
	bool m_bHandle_Nodes; // are we handling paths or nodes?
	bool m_bAutoCenter;
	bool m_bSmoothCenter;
	int m_Current_nif_vert;
	UINT m_ExtrudeTimer;
	char m_Path[_MAX_PATH]; // the path this room should be saved to
	CToolTipCtrl m_TTip;		// tooltip for displaying useful info

// Operations
protected:
	void GridSize(int num);
	bool UpdateGridSize(int num);

public:
	void UpdateAllPanes();
	CWnd * GetFocusedPane();
	void InitPanes();
	void InitOrthoPane(Cned_OrthoWnd *wnd, char *title, int id);
	void InitPerspPane(Cned_PerspWnd *wnd, char *title, int id);
	void SetPrim(prim *prim);
	void SetPrim(room *rp, int face, int portal, int edge, int vert);
	void InitTex();
	void ShutdownTex();
	bool IsVertMarked(int vertnum);
	void MarkVert(int vertnum);
	void UnMarkVert(int vertnum);
	void MarkVertsInFace();
	void UnMarkVertsInFace();
	void MarkFace(int facenum);
	void UnMarkFace(int facenum);
	int GetMarkedVerts(short *list);
	int GetMarkedFaces(short *list);
	void DeleteMarkedVerts();
	void DeleteMarkedFaces();
	bool MergeMarkedVerts();
	void CopyVerts(bool unmark);
	void CopyFaces(bool unmark);
	void PasteVerts(bool mark,bool on_top);
	void PasteFaces(bool mark,bool on_top);
	void CycleEdge(prim *prim,int which);
	void CycleFace(prim *prim,int which);
	void CyclePortal(prim *prim,int which);
	void CycleRoom(prim *prim,int which);
	void CycleVert(prim *prim,int which);
	void CycleObject(prim *prim,int which);
	void CyclePath(int which);
	void CycleNode(int which);
	bool SetRefFrame(vector pos);
	void StopCameras();
	void RemoveExtraPoints();
	void CloseRoom();
	void UnMarkAll();
	void CenterRoom();
	CWnd * GetPane(int i,int j);
	void ForceUnmarkAll();
	char * GetPath();
	void SetPath(char *path);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoomFrm)
	protected:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
public:
	bool m_State_changed;
	bool m_Room_changed;

	void SetModifiedFlag(bool modified);
	bool GetModifiedStatus(void);
protected:
	bool m_PanesInitted;
	virtual ~CRoomFrm();

	// Generated message map functions
	//{{AFX_MSG(CRoomFrm)
	afx_msg void OnUnmarkAll();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnEditPaste();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnShowNormals();
	afx_msg void OnUpdateShowNormals(CCmdUI* pCmdUI);
	afx_msg void OnShowVerts();
	afx_msg void OnUpdateShowVerts(CCmdUI* pCmdUI);
	afx_msg void OnShowGrid();
	afx_msg void OnUpdateShowGrid(CCmdUI* pCmdUI);
	afx_msg void OnShowAtchRooms();
	afx_msg void OnUpdateShowAtchRooms(CCmdUI* pCmdUI);
	afx_msg void OnCenterRoom();
	afx_msg void OnCenterOrigin();
	afx_msg void OnGridSize1();
	afx_msg void OnUpdateGridSize1(CCmdUI* pCmdUI);
	afx_msg void OnGridSize2();
	afx_msg void OnUpdateGridSize2(CCmdUI* pCmdUI);
	afx_msg void OnGridSize3();
	afx_msg void OnUpdateGridSize3(CCmdUI* pCmdUI);
	afx_msg void OnGridSize4();
	afx_msg void OnUpdateGridSize4(CCmdUI* pCmdUI);
	afx_msg void OnGridSize5();
	afx_msg void OnUpdateGridSize5(CCmdUI* pCmdUI);
	afx_msg void OnTextured();
	afx_msg void OnUpdateTextured(CCmdUI* pCmdUI);
	afx_msg void OnWireframe();
	afx_msg void OnUpdateWireframe(CCmdUI* pCmdUI);
	afx_msg void OnViewMoveCameraToCurrentFace();
	afx_msg void OnUpdateViewMoveCameraToCurrentFace(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCenterRoom(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCenterOrigin(CCmdUI* pCmdUI);
	afx_msg void OnCenterFace();
	afx_msg void OnClose();
	afx_msg void OnShowObjects();
	afx_msg void OnUpdateShowObjects(CCmdUI* pCmdUI);
	afx_msg void OnModeVertex();
	afx_msg void OnModeFace();
	afx_msg void OnModeObject();
	afx_msg void OnUpdateModeVertex(CCmdUI* pCmdUI);
	afx_msg void OnUpdateModeFace(CCmdUI* pCmdUI);
	afx_msg void OnUpdateModeObject(CCmdUI* pCmdUI);
	afx_msg void OnMarkToggle();
	afx_msg void OnMarkAll();
	afx_msg void OnInvertMarkings();
	afx_msg void OnEditInsert();
	afx_msg void OnEditDelete();
	afx_msg void OnGridSize0();
	afx_msg void OnUpdateGridSize0(CCmdUI* pCmdUI);
	afx_msg void OnSnapToGrid();
	afx_msg void OnUpdateSnapToGrid(CCmdUI* pCmdUI);
	afx_msg void OnDefaultExtrude();
	afx_msg void OnUpdateMarkAll(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMarkToggle(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInvertMarkings(CCmdUI* pCmdUI);
	afx_msg void OnUpdateUnmarkAll(CCmdUI* pCmdUI);
	afx_msg void OnGridSize6();
	afx_msg void OnUpdateGridSize6(CCmdUI* pCmdUI);
	afx_msg void OnEditCopy();
	afx_msg void OnUpdateEditCopy(CCmdUI* pCmdUI);
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditPaste(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditInsert(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditDelete(CCmdUI* pCmdUI);
	afx_msg void OnGridSizeCustom();
	afx_msg void OnUpdateGridSizeCustom(CCmdUI* pCmdUI);
	afx_msg void OnViewMoveCameraToCurrentObject();
	afx_msg void OnFileShowRoomStats();
	afx_msg void OnViewTexturedOutline();
	afx_msg void OnUpdateViewTexturedOutline(CCmdUI* pCmdUI);
	afx_msg void OnFileClose();
	afx_msg void OnShowRoomCenter();
	afx_msg void OnUpdateShowRoomCenter(CCmdUI* pCmdUI);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnModePath();
	afx_msg void OnUpdateModePath(CCmdUI* pCmdUI);
	afx_msg void OnEditPasteOnTop();
	afx_msg void OnUpdateEditPasteOnTop(CCmdUI* pCmdUI);
	afx_msg void OnViewMoveCameraToCurrentRoom();
	afx_msg void OnUpdateViewMoveCameraToCurrentObject(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnUpdateEditCut(CCmdUI* pCmdUI);
	afx_msg void OnPrevPane();
	afx_msg void OnNextPane();
	//}}AFX_MSG
	BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);	// supplies the text for the tooltip
	DECLARE_MESSAGE_MAP()
private:
	bool m_Modified;
};

void RoomChanged(room *rp);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROOMFRM_H__469D69E3_EB53_11D2_A6A1_006097E07445__INCLUDED_)
