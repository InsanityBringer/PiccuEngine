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
 #if !defined(AFX_ROOMDIALOGBAR_H__BFB2B902_F749_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_ROOMDIALOGBAR_H__BFB2B902_F749_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RoomDialogBar.h : header file
//

class CRoomFrm;

#define X_AXIS	0
#define Y_AXIS	1
#define Z_AXIS	2

/////////////////////////////////////////////////////////////////////////////
// CRoomDialogBar dialog

class CRoomDialogBar : public CDialogBar
{
// Construction
public:
	CRoomDialogBar();   // standard constructor
	void InitBar();
	float SnapSingleVertToGrid(CRoomFrm *wnd,room *rp,short *list,int num_verts,int snap_vert);
	void SnapVertsToGrid(CRoomFrm *wnd,room *rp,short *list,int num_verts);

// Dialog Data
	//{{AFX_DATA(CRoomDialogBar)
	enum { IDD = IDD_ROOMBAR };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRoomDialogBar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRoomDialogBar)
	afx_msg void OnModifyExtrude();
	afx_msg void OnControlUpdate(CCmdUI*);
	afx_msg void OnModifyLathe();
	afx_msg void OnModifyBend();
	afx_msg void OnFacePlanarCheck();
	afx_msg void OnFaceSplitCurrentFace();
	afx_msg void OnFaceTriangulateNonPlanar();
	afx_msg void OnFaceTriangulateCurrent();
	afx_msg void OnEdgeContract();
	afx_msg void OnEdgeExpand();
	afx_msg void OnFaceContract();
	afx_msg void OnFaceExpand();
	afx_msg void OnFaceJoin();
	afx_msg void OnRoomContract();
	afx_msg void OnRoomExpand();
	afx_msg void OnModifySetRefFrame();
	afx_msg void OnVertexRemoveExtras();
	afx_msg void OnFaceFlip();
	afx_msg void OnVertexSnapFace();
	afx_msg void OnVertexSnapEdge();
	afx_msg void OnVertexSnapGrid();
	afx_msg void OnVertexSnapVert();
	afx_msg LONG OnInitDialog ( UINT, LONG );
	afx_msg void OnRoomMirror();
	afx_msg void OnFaceAddVert();
	afx_msg void OnFaceRemoveVert();
	afx_msg void OnFaceSwapVerts();
	afx_msg void OnVertexSnapEdge2();
	afx_msg void OnVertexSnapFace2();
	afx_msg void OnVertexSnapGrid2();
	afx_msg void OnVertexSnapVert2();
	afx_msg void OnFaceTwist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

int VertInFace(room *rp, int facenum, int vertnum);
// Goes through all the vertices in the room and scales them by scale_factor
void SizeRoomVertices (room *rp,float scale_factor);
// Goes through all the vertices in a face and scales them by scale_factor
void SizeFaceVertices (room *rp,int facenum,float scale_factor);
void SizeFaceEdge (room *rp,int facenum,int edge,float scale_factor);

/////////////////////////////////////////////////////////////////////////////
// CMirrorDialog dialog

class CMirrorDialog : public CDialog
{
// Construction
public:
	CMirrorDialog(CWnd* pParent = NULL);   // standard constructor
	int m_Along;	// the axis to mirror along

// Dialog Data
	//{{AFX_DATA(CMirrorDialog)
	enum { IDD = IDD_MIRROR_ROOM };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMirrorDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMirrorDialog)
	afx_msg void OnXaxis();
	afx_msg void OnYaxis();
	afx_msg void OnZaxis();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ROOMDIALOGBAR_H__BFB2B902_F749_11D2_A6A1_006097E07445__INCLUDED_)
