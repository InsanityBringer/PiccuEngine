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
 


#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


#include "d3edit.h"			// d3 editor header

class CTextureGrWnd;
class CWireframeGrWnd;
class CEditorView;
class CEditorDoc;
class CMainFrame;
class CDallasMainDlg;
class CViewerPropDlg;


/////////////////////////////////////////////////////////////////////////////
// CEditorApp:
// See editor.cpp for the implementation of this class
//

class CEditorApp : public CWinApp
{
public:
	CEditorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorApp)
	public:
	virtual BOOL InitInstance();
	virtual BOOL OnIdle(LONG lCount);
	virtual CDocument* OpenDocumentFile(LPCTSTR lpszFileName);
	//}}AFX_VIRTUAL

// Implementation
//	ADDED BY SAMIR FOR OLE SUPPORT
	COleTemplateServer m_server;
		// Server object for document creation
//	ADDED BY SAMIR FOR OLE SUPPORT

	//{{AFX_MSG(CEditorApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CTextureGrWnd *textured_view;		// GrWnd views for editor
	CWireframeGrWnd *wireframe_view;
	CEditorDoc	 *main_doc;				// main doc object.
	CEditorView *main_view;				// main view
	CMainFrame *main_frame;				// the application's main_frame

	CDallasMainDlg *m_DallasModelessDlgPtr;		// modeless dialog for Dallas
	CViewerPropDlg *m_ViewerPropDlgPtr;				// modeless viewer properties dialog

	void pause(); 
	void resume();

	BOOL paused() const { return m_Paused; };

private:
	void InitEditorState();

private:
	BOOL m_Paused;
};

extern CEditorApp theApp;
extern bool Editor_active;						// this is false if we are running the game.

/////////////////////////////////////////////////////////////////////////////

//Load editor settings from the registry
void LoadEditorSettings();

//Save current editor settings to the registry
void SaveEditorSettings();

//Function to print to an edit box
//Parameters:	dlg - the dialog that the edit box is in
//					id - the ID of the edit box
//					fmt - printf-style text & arguments
void PrintToDlgItem(CDialog *dlg,int id,char *fmt,...);

//	A quick way to use an openfiledialog/savefiledialog.
//		wnd = parent window
//		filter = a C string containing the extensions to look for in filenames when browsing (in MFC format)
//		pathname = buffer where full pathname will be stored of file opened/saved 
//		initialdir = what directory to start browsing from.  will also contain the new browsing directory if
//						 user changed directories.
//		dirlen = max length of initialdir buffer including terminating null.
bool OpenFileDialog(CWnd *wnd, LPCTSTR filter, char *pathname, char *initialdir=NULL, int dirlen=0);
bool SaveFileDialog(CWnd *wnd, LPCTSTR filter, char *pathname, char *initialdir=NULL, int dirlen=0);

//Move the viewer object.  This should be called whenever the viewer object is moved
void MoveViewer(vector *pos,int roomnum,matrix *orient);
