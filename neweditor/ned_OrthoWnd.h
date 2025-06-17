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
 #if !defined(AFX_NED_ORTHOWND_H__8F6C1721_EC20_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_NED_ORTHOWND_H__8F6C1721_EC20_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ned_OrthoWnd.h : header file
//

// #include "ned_GrWnd.h"

#include "globals.h"	// Added by ClassView
#include "MemDC.h"

// Constants for orthogonal views

#define NUM_GRIDS		8
#define NUM_SNAPS		11

// colors (format: O_TYPE_COLOR, where the 'O' stands for orthographic to distinguish from colors in perspective views)

// Pen colors
#define O_CUR_ROOM_COLOR		0
#define O_CUR_FACE_COLOR		1
#define O_CUR_EDGE_COLOR		2
#define O_ROOM_COLOR			3
#define O_VERT_OUTLINE_COLOR	4
#define O_MARKED_FACE_COLOR		5
#define O_NORMAL_COLOR			6
#define O_GRIDLINE_COLOR		7
#define O_ORIGINLINE_COLOR		8
#define O_REFFRAME_COLOR		9
#define O_ROOMCENTER_COLOR		10
#define O_BR_GRIDLINE_COLOR		11
#define O_GAMEPATH_COLOR		12
#define O_AIPATH_COLOR			13

// Brush colors
#define O_CUR_VERT_COLOR		0
#define O_VERT_COLOR			1
#define O_MARKED_VERT_COLOR		2
#define O_NODE_COLOR			3
#define O_BNODE_COLOR			4

// scale factors
#define O_MAX_SCALE				1.00f
#define O_MIN_SCALE				0.01f

#define O_ROTATE_SCALE		0.03125f // this is for a 45/4 degree rotation (1/32 of a circle)

#define VERT_SELECT_SIZE		5
#define MAX_MOUSE_SELECT		10

// size of drawn vertex
#define O_VERT_SIZE 3

// World to screen translation factor
#define WOS 10

// often-used GDI objects
#define NUM_PENS 14
#define NUM_BRUSHES 5

extern CPen g_pens[NUM_PENS];
extern CBrush g_brushes[NUM_BRUSHES];

void InitPens();
void InitBrushes();

// arrays of selectable grid spacings and snap distances
extern int Grids[];
extern float Snaps[];

//Structure to describe an orthogonal camera
typedef struct {
	float scale;
	POINT pos;
	POINT destpos;
	POINT step;
	bool view_changed;
	bool moving;
} o_camera;

// 2D position vector in world coordinates
typedef struct tagvec2D
{
	float x;
	float y;
} vec2D;


class CRoomFrm;
// class Cned_RectTracker;



/////////////////////////////////////////////////////////////////////////////
// Cned_OrthoWnd window

class Cned_OrthoWnd : public CWnd
{
	DECLARE_DYNCREATE(Cned_OrthoWnd)

	friend CRoomFrm;

// Construction
public:
	Cned_OrthoWnd();

// Attributes
public:

// Operations
protected:
	void GridSize(int num);
	bool UpdateGridSize(int num);

public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(Cned_OrthoWnd)
	public:
	virtual BOOL Create(const RECT& rect, CWnd* pParentWnd, LPCTSTR name, UINT nID);
	//}}AFX_VIRTUAL

// Implementation
public:
	COLORREF m_BackColor;
	unsigned int m_nID;
	char m_Title[128];
	bool m_InFocus;
	bool m_View_changed;
	bool m_State_changed;
	bool m_bShowVerts;				// display variables
	bool m_bShowNormals;
	bool m_bShowObjects;
	bool m_bShowCenters;
	bool m_bGridShow;
	bool m_bGridSnap;
	int m_nGrid;
	int m_nSnap;
	bool m_bShowAttached;
	bool m_bCaptured;
	float m_ScaleStep;
	CRectTracker m_rTracker;		// the selection box

	CRoomFrm *m_pParentFrame;	// the parent room frame window
	// Pointers to parent room frame members
	prim *m_pPrim;					// the primitives
	vector *m_pvec_InsertPos; // where we will insert a new vertex (screen-z equals screen-z of ref frame)
	vector *m_pvec_RefPos; // the origin of the reference frame
	vector *m_pvec_MousePos; // where the mouse is (screen-z equals screen-z of ref frame)
	bool *m_pVert_marks; // marked flags for each vertex
	int *m_pNum_marked_verts; // number of marked verts
	bool *m_pFace_marks; // marked flags for each face
	int *m_pNum_marked_faces; // number of marked faces
	bool m_TT_ready;		// ready the tool tip
	int m_CustomGrid;		// custom grid size

	unsigned int m_TimerHandle;
	o_camera m_Cam;
	virtual ~Cned_OrthoWnd();
	virtual void Render();
	void InitCamera();
	void DrawOrthoGrid(CMemDC *pDC,int size);
	void DrawOrthoWorld(CMemDC *pDC);
	void DrawOrthoRefFrame(CMemDC *pDC);
	void DrawOrthoObjects(CMemDC *pDC,room *rp,object *obj,int num_objects);
	void DrawRoomCenter(CMemDC *pDC,room *rp,const int color);
	void DrawOrthoRoom(CMemDC *pDC,room *rp,const int color);
	void DrawOrthoFace(CMemDC *pDC,face *fp,room *rp,const int color);
	void DrawOrthoEdge(CMemDC *pDC,face *fp,room *rp,const int color);
	void DrawOrthoFaceNormal(CMemDC *pDC,room *rp,int facenum,const int color);
	void DrawOrthoObjectMark(CMemDC *pDC,room *rp,object *objp,const int color);
	void DrawOrthoGamePaths(CMemDC *pDC,room *rp,const int color,const int br_color);
	void DrawOrthoAIPaths(CMemDC *pDC,room *rp,const int color,const int br_color);
	void DrawOrthoNumber(CMemDC *pDC,int num,vec2D pos,float size,const int color);
	vec2D GetCursorPos();
	void SetCursorPos(vec2D pos);
	void MoveCursor(float amount_x, float amount_y);
	void DrawCursor(CMemDC *pDC);
	void CenterRoom(room *rp);
	void CenterFace(room *rp,int facenum);
	void CenterOrigin();
	void DrawVerts(CMemDC *pDC,vector *verts,int num_verts,const int color);
	void DrawCurrentPrims(CMemDC *pDC);
	void DrawMarkedPrims(CMemDC *pDC);
	bool IsVertDrawn(POINT pos,int index);
	void MarkVertsInSelBox(CClientDC *pDC,bool bMark);
	void MarkFacesInSelBox(CClientDC *pDC,bool bMark);
	int SelectVert(int view_flag,vec2D search_pos,bool *not_in_face);
	int SelectFace(int view_flag,vec2D search_pos);
	int OrthoSelectObject(int view_flag,vec2D search_pos);
	void OrthoSelectPathNode(int view_flag,vec2D search_pos,int *path,int *node);
	bool PointInFace(int view_flag,vec2D search_pos,int facenum);
	int Find2DDistance(int view_flag,vec2D search_pos,vector vec);
	bool CompareDepth(int view_flag,int a,int b);
	bool CompareVectorDepth(int view_flag,vector a,vector b);
	void SnapPoint(vec2D *pos);
	void SetCameraPos(POINT pos);
	void StopCamera();
	void SetRefFrame(vec2D pos);
	void OrientObject(int objnum,angle ang);
	void SetMousePos(vec2D pos);
	vec2D GetMousePos();

	// Generated message map functions
protected:
	//{{AFX_MSG(Cned_OrthoWnd)
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
	afx_msg void OnPaint();
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnGridSize1();
	afx_msg void OnGridSize2();
	afx_msg void OnGridSize3();
	afx_msg void OnGridSize4();
	afx_msg void OnGridSize5();
	afx_msg void OnShowAtchRooms();
	afx_msg void OnShowGrid();
	afx_msg void OnShowNormals();
	afx_msg void OnShowVerts();
	afx_msg void OnCenterRoom();
	afx_msg void OnCenterOrigin();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnCenterFace();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnShowObjects();
	afx_msg void OnGridSize0();
	afx_msg void OnSnapToGrid();
	afx_msg void OnGridSize6();
	afx_msg void OnUpdateGridSize0(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize1(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize2(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize3(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize4(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize5(CCmdUI* pCmdUI);
	afx_msg void OnUpdateGridSize6(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowVerts(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowNormals(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowObjects(CCmdUI* pCmdUI);
	afx_msg void OnUpdateSnapToGrid(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowGrid(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowAtchRooms(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnGridSizeCustom();
	afx_msg void OnUpdateGridSizeCustom(CCmdUI* pCmdUI);
	afx_msg void OnShowRoomCenter();
	afx_msg void OnUpdateShowRoomCenter(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	stMouse m_Mouse;
	stKeys m_Keys;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NED_ORTHOWND_H__8F6C1721_EC20_11D2_A6A1_006097E07445__INCLUDED_)
