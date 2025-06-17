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
 

// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__A96779E7_E43B_11D2_8F49_00104B27BFF0__INCLUDED_)
#define AFX_MAINFRM_H__A96779E7_E43B_11D2_8F49_00104B27BFF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "TextureDialogBar.h"
#include "ObjectDialogBar.h"
#include "RoomDialogBar.h"
#include "DoorwayDialogBar.h"
#include "SoundDialogBar.h"
#include "PathDialogBar.h"
#include "TerrainDialogBar.h"

#define UNIQUE_ID_SLEWER		0x1420
#define UNIQUE_ID_ROOMBAR		0x1421
#define UNIQUE_ID_TEXTUREBAR	0x1422
#define UNIQUE_ID_OBJECTBAR		0x1423
#define UNIQUE_ID_DOORWAYBAR	0x1424
#define UNIQUE_ID_SOUNDBAR		0x1425
#define UNIQUE_ID_PATHBAR		0x1426
#define UNIQUE_ID_TERRAINBAR	0x1427

class CMainFrame : public CMDIFrameWnd
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

// Attributes
public:
	CString strD3EditDialogBarState;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

public:
	void SetStatusMessage(char *str);
	void SetStatusMessage(int index,char *str,bool update = true);
	void RoomProperties();

// Implementation
public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CStatusBar  m_wndStatusBar2;
	CToolBar    m_wndToolBar;
	CReBar      m_wndReBar;
	CDialogBar      m_wndDlgBar;
	CTextureDialogBar m_TextureDialog;
	CObjectDialogBar m_ObjectDialog;
	CRoomDialogBar m_RoomDialog;
	CTerrainDialogBar m_TerrainDialog;
	CDoorwayDialogBar m_DoorwayDialog;
	CSoundDialogBar m_SoundDialog;
	CPathDialogBar m_PathDialog;
	CToolTipCtrl m_ToolTip;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnFileShowLevelStats();
	afx_msg void OnFileEditLevelInfo();
	afx_msg void OnViewTextures();
	afx_msg void OnUpdateViewTextures(CCmdUI* pCmdUI);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnClose();
	afx_msg void OnViewRoombar();
	afx_msg void OnUpdateViewRoombar(CCmdUI* pCmdUI);
	afx_msg void OnRoomPlaceRoom();
	afx_msg void OnRoomAttachRoom();
	afx_msg void OnRoomAdd();
	afx_msg void OnRoomBuildBridge();
	afx_msg void OnRoomBuildSmoothBridge();
	afx_msg void OnRoomCombine();
	afx_msg void OnRoomDelete();
	afx_msg void OnRoomDeleteFace();
	afx_msg void OnRoomDeletePortal();
	afx_msg void OnRoomDeleteVert();
	afx_msg void OnRoomDropRoom();
	afx_msg void OnRoomJoinAdjacentFaces();
	afx_msg void OnRoomJoinRooms();
	afx_msg void OnRoomJoinRoomsExact();
	afx_msg void OnRoomMark();
	afx_msg void OnRoomPropagateToAll();
	afx_msg void OnRoomSnapPlacedRoom();
	afx_msg void OnRoomSnapPointToEdge();
	afx_msg void OnRoomSnapPointToFace();
	afx_msg void OnRoomSnapPointToPoint();
	afx_msg void OnRoomSwapMarkedAndCurrentRoomFace();
	afx_msg void OnViewObjects();
	afx_msg void OnUpdateViewObjects(CCmdUI* pCmdUI);
	afx_msg void OnZBuffer();
	afx_msg void OnUpdateZBuffer(CCmdUI* pCmdUI);
	afx_msg void OnWindowTextureAlignment();
	afx_msg void OnUpdateWindowTextureAlignment(CCmdUI* pCmdUI);
	afx_msg void OnWindowTriggers();
	afx_msg void OnUpdateWindowTriggers(CCmdUI* pCmdUI);
	afx_msg void OnViewDoorways();
	afx_msg void OnUpdateViewDoorways(CCmdUI* pCmdUI);
	afx_msg void OnViewLighting();
	afx_msg void OnUpdateViewLighting(CCmdUI* pCmdUI);
	afx_msg void OnButtonBoaVis();
	afx_msg void OnUpdateButtonBoaVis(CCmdUI* pCmdUI);
	afx_msg void OnWindowGoals();
	afx_msg void OnUpdateWindowGoals(CCmdUI* pCmdUI);
	afx_msg void OnFileVerifyMine();
	afx_msg void OnFileVerifyRoom();
	afx_msg void OnHelpContents();
	afx_msg void OnRoomUnplaceRoom();
	afx_msg void OnUpdateRoomUnplaceRoom(CCmdUI* pCmdUI);
	afx_msg void OnFileRemoveExtraPoints();
	afx_msg void OnUpdateRoomAttachRoom(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRoomSnapPlacedRoom(CCmdUI* pCmdUI);
	afx_msg void OnWindowRoomProperties();
	afx_msg void OnUpdateWindowRoomProperties(CCmdUI* pCmdUI);
	afx_msg void OnFileRemoveExtraPointsInRoom();
	afx_msg void OnRoomSelectByNumber();
	afx_msg void OnRoomSelectFaceByNumber();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnFileGravity();
	afx_msg void OnMoveMine();
	afx_msg void OnViewSounds();
	afx_msg void OnUpdateViewSounds(CCmdUI* pCmdUI);
	afx_msg void OnWindowMatcens();
	afx_msg void OnUpdateWindowMatcens(CCmdUI* pCmdUI);
	afx_msg void OnViewCameraSlewer();
	afx_msg void OnUpdateViewCameraSlewer(CCmdUI* pCmdUI);
	afx_msg void OnEditCut();
	afx_msg void OnEditCopy();
	afx_msg void OnEditAttach();
	afx_msg void OnEditDelete();
	afx_msg void OnEditLoadscrap();
	afx_msg void OnEditSavescrap();
	afx_msg void OnEditRemoveselect();
	afx_msg void OnEditSelectattached();
	afx_msg void OnEditUndo();
	afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditAttach(CCmdUI* pCmdUI);
	afx_msg void OnUpdateEditSavescrap(CCmdUI* pCmdUI);
	afx_msg void OnEditPaste();
	afx_msg void OnEditClearselected();
	afx_msg void OnEditAddselect();
	afx_msg void OnRoomMergeObjectIntoRoom();
	afx_msg void OnUpdateRoomMergeObjectIntoRoom(CCmdUI* pCmdUI);
	afx_msg void OnFileScaleMine();
	afx_msg void OnEditUnplaceGroup();
	afx_msg void OnUpdateEditUnplaceGroup(CCmdUI* pCmdUI);
	afx_msg void OnWindowEditorBar();
	afx_msg void OnUpdateWindowEditorBar(CCmdUI* pCmdUI);
	afx_msg void OnWindowEditorBar2();
	afx_msg void OnUpdateWindowEditorBar2(CCmdUI* pCmdUI);
	afx_msg void OnWindowPathBar();
	afx_msg void OnUpdateWindowPathBar(CCmdUI* pCmdUI);
	afx_msg void OnWindowTerrainBar();
	afx_msg void OnUpdateWindowTerrainBar(CCmdUI* pCmdUI);
	afx_msg void OnRoomPlaceTerrainRoom();
	afx_msg void OnRoomLinkToNewExternal();
	afx_msg void OnEditPlaceTerrain();
	//}}AFX_MSG
	afx_msg void OnUpdateStatusMarked(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusPosition(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusMouse(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusCurrent(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusGridSize(CCmdUI* pCmdUI);
	afx_msg void OnUpdateStatusMode(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()
};

// NED versions of some Erooms.cpp functions

void ned_VerifyMine();
void ned_VerifyRoom(room *rp);
// Counts the number of unique textures in a room, plus gives names of textures used
void ned_CountUniqueRoomTextures(room *rp);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__A96779E7_E43B_11D2_8F49_00104B27BFF0__INCLUDED_)
