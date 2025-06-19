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
 

// NewEditor.h : main header file for the NEWEDITOR application
//

#if !defined(AFX_NEWEDITOR_H__A96779E3_E43B_11D2_8F49_00104B27BFF0__INCLUDED_)
#define AFX_NEWEDITOR_H__A96779E3_E43B_11D2_8F49_00104B27BFF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "ned_LevelWnd.h"
#include "ned_PerspWnd.h"
#include "ned_OrthoWnd.h"
#include "LevelFrame.h"
#include "CameraSlew.h"
#include "RoomFrm.h"
#include "TexAlignDialog.h"
#include "TriggerDialog.h"
#include "GoalDialog.h"
#include "LightingDialog.h"
#include "RoomProperties.h"
#include "MatcenDialog.h"
#include "ScriptCompiler.h"

class CDallasMainDlg;

/////////////////////////////////////////////////////////////////////////////
// CNewEditorApp:
// See NewEditor.cpp for the implementation of this class
//

class CNewEditorApp : public CWinApp
{
friend class Cned_LevelWnd;

public:
	CNewEditorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL OnIdle(LONG lCount);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HMENU m_hMDI_D3L_Menu;
	HACCEL m_hMDI_D3L_Accel;
	HMENU m_hMDI_ORF_Menu;
	HACCEL m_hMDI_ORF_Accel;

public:
	//{{AFX_MSG(CNewEditorApp)
	afx_msg void OnAppAbout();
	afx_msg void OnFileNew();
	afx_msg void OnFileOpen();
	afx_msg void OnFileSettings();
	afx_msg void OnFileSave();
	afx_msg void OnFileSaveAs();
	afx_msg void OnTablefileManager();
	afx_msg void OnHelpCheckforupdates();
	afx_msg void OnHelpShowtips();
	afx_msg void OnScriptcompile();
	afx_msg void OnUpdateScriptcompile(CCmdUI* pCmdUI);
	afx_msg void OnFileDallasvisualscriptinterface();
	afx_msg void OnUpdateFileDallasvisualscriptinterface(CCmdUI* pCmdUI);
	afx_msg void OnFileMruFile1();
	afx_msg void OnFileMruFile2();
	afx_msg void OnFileMruFile3();
	afx_msg void OnFileMruFile4();
	afx_msg void OnDataMissionhogsettings();
	afx_msg void OnUpdateDataMissionhogsettings(CCmdUI* pCmdUI);
	afx_msg void OnFileSaveRoom();
	afx_msg void OnFileSaveasroom();
	afx_msg void OnFileMn3packager();
	afx_msg void OnFileRoomProperties();
	afx_msg void OnFileSaveResourceLists();
	afx_msg void OnUpdateFileSaveResourceLists(CCmdUI* pCmdUI);
	afx_msg void OnFileOpenResourceLists();
	afx_msg void OnUpdateFileOpenResourceLists(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CLevelFrame		*m_ThisLevelFrame;
	Cned_LevelWnd	*m_pLevelWnd;				// the level window ("World View")
	CRoomFrm		*m_pRoomFrm;				// the primary room frame
	CRoomFrm		*m_pFocusedRoomFrm;			// the focused room frame
	CRoomFrm		*m_ppPaletteRoomFrms[MAX_PALETTE_ROOMS];	// the palette room frames
	CTexAlignDialog	*m_pTexAlignDlg;			// the modeless texture alignment dialog
	CTriggerDialog	*m_pTriggerDlg;				// the modeless trigger dialog
	CGoalDialog		*m_pGoalDlg;				// the modeless goal dialog
	CLightingDialog	*m_pLightingDlg;			// the modeless lighting dialog
	CRoomProperties	*m_pRoomPropsDlg;			// the modeless room properties dialog
	CMatcenDialog	*m_pMatcensDlg;				// the modeless matcens dialog
	CScriptCompiler	*m_ScriptCompileDlg;		// the modeless script compiler dialog
	CDallasMainDlg	*m_DallasModelessDlgPtr;	// modeless dialog for Dallas

	int GetFreeRoomFrame();
	prim * AcquirePrim();
	void * AcquireWnd();
	CRoomFrm * FindRoomWnd(room *rp);
	void ScriptCompilerDone();
	void DallasDone();

	void OpenNedFile(CString SelectedFile);
	void OnFileMruFile(int nIndex);
	CString BrowseRooms() ;
	void SetExtrudeDefaults(int which,float dist,BOOL delete_base_face,int inward,int faces,BOOL use_default);
	void GetExtrudeDefaults(int *which,float *dist,BOOL *delete_base_face,int *inward,int *faces,BOOL *use_default);
};

extern CNewEditorApp theApp;

#include "CameraSlew.h"

extern CCameraSlew *gCameraSlewer;

#include "application.h"
#include "AppDatabase.h"

extern oeWin32Application *g_OuroeApp;		// The Main application
//extern oeAppDatabase *g_Database;			// Application database.

typedef struct TAGned_vert
{
	vector vertex;
	int orignum;
} ned_vert;

// Clipboard stuff
extern face Face_clipboard[];				// face clipboard
extern ned_vert Vert_clipboard[];				// vert clipboard
extern int Num_clipboard_faces;				// number of clipboard faces
extern int Num_clipboard_verts;				// number of clipboard verts
extern bool g_bFaces;						// is face clipboard valid?
extern bool g_bVerts;						// is vertex clipboard valid?

void RenameRoom(room *rp,char *name);
void RenameRoom(room *rp);

// SUPPORT FUNCTIONS

#include "ned_Tablefile.h"

#define MAX_DER_FILE_BUFFER_LEN		(PAGENAME_LEN+256)
#define MAX_DER_VERSION_NUMBER		1
#define DER_HEADER					"D3Edit Resource List"
#define DER_TAG_TEXTURES			"[TEXTURES]"
#define DER_TAG_OBJECTS				"[OBJECTS]"

void SaveResourceLists(char *filename,DWORD flags);
void DlgSaveResourceLists(DWORD flags);
UINT CALLBACK SaveListHook(HWND hDlg,UINT uiMsg,WPARAM wParam,LPARAM lParam);
void OpenResourceLists(char *filename,DWORD flags);
void DlgOpenResourceLists(DWORD flags);
UINT CALLBACK OpenListHook(HWND hDlg,UINT uiMsg,WPARAM wParam,LPARAM lParam);

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_NEWEDITOR_H__A96779E3_E43B_11D2_8F49_00104B27BFF0__INCLUDED_)
