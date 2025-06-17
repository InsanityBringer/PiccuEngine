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
 #if !defined(AFX_OBJECTDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
#define AFX_OBJECTDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ObjectDialogBar.h : header file
//
#include "resource.h"
#include "ObjectPalette.h"

extern int Insert_mode;

/////////////////////////////////////////////////////////////////////////////
// CObjectDialogBar dialog
enum {	TAB_OBJECT_CUSTOM,
		TAB_OBJECT_LEVEL};

enum {	TIMER_OBJECTS = 1234};

class CObjectDialogBar;
/////////////////////////////////////////////////////////////////////////////
// CObjectBarList dialog

class CObjectBarList : public CDialog
{
// Construction
public:
	CObjectBarList(CWnd* pParent = NULL);   // standard constructor
	int AddObjectToList(int obj_slot);
	int RemoveObjectFromList(int obj_slot);

// Dialog Data
	//{{AFX_DATA(CObjectBarList)
	enum { IDD = IDD_BAR_LIST };
	CListBox	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectBarList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	CObjectDialogBar *m_ParentWindow;
	bool ignore_mark;

	// Generated message map functions
	//{{AFX_MSG(CObjectBarList)
	afx_msg void OnSelchangeList();
	virtual void OnOK();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnAddToCustom();
	afx_msg void OnUpdateAddToCustom(CCmdUI* pCmdUI);
	afx_msg void OnRemove();
	afx_msg void OnUpdateRemove(CCmdUI* pCmdUI);
	afx_msg void OnSaveList();
	afx_msg void OnUpdateSaveList(CCmdUI* pCmdUI);
	afx_msg void OnLoadList();
	afx_msg void OnUpdateLoadList(CCmdUI* pCmdUI);
	afx_msg void OnRemoveAll();
	afx_msg void OnUpdateRemoveAll(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

class CObjectDialogBar : public CDialogBar
{
friend class CObjectBarList;
// Construction
public:
	CObjectBarList *m_CustomDialog; // quick hack; temp public
	void SetCurrentLevelObject(int Object);
	int GetCurrentObject(void);
	void SetCurrentObject(int Object);
	CObjectDialogBar();   // standard constructor
	void ObjectPaletteDone();
	//	Display current tab dialog	
	void ShowCurrentObjectTab();
	void AddRemoveLevelObject(char *name,int num,bool add);
#ifdef _DEBUG
	void UpdateLevelObjectCount(char *name,int newnum,int oldnum);
#endif
	int GetFreePlayerIndex();
	bool WriteObjectList(CFILE *outfile,int list);
	int ReadObjectList(CFILE *infile);

// Dialog Data
	//{{AFX_DATA(CObjectDialogBar)
	enum { IDD = IDD_OBJECTBAR };
	CStatic	m_COView;
	CStatic	m_CLOView;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectDialogBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_CurrentObjectChanged;
	bool m_CurrentLevelObjectChanged;
	bool m_CurrentPlayerChanged;
	int m_CurrentLevelObject;
	int m_CurrentRoom;
	int m_CurrentObject;
	int m_current_start_pos;
	void InitKeyPad(void);
	void RedrawArea();
	void PaintPreviewArea(CWnd *pWnd);
	void RedrawPreviewArea(CWnd *pWnd);
	void DoObjectTabNotify(NMHDR *nmhdr);
	int m_CurrentObjectTab;
	void SetPlayerChecks();

	CObjectPalette* m_pObjectPalette;

	CObjectBarList *m_ObjectTabDialog;
	CObjectBarList *m_LevelDialog;
	// Generated message map functions
	//{{AFX_MSG(CObjectDialogBar)
	afx_msg LONG OnInitDialog ( UINT, LONG );
	afx_msg void OnPaint();
	afx_msg void OnControlUpdate(CCmdUI*);
	afx_msg void OnObjectPalette();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCoProperties();
	afx_msg void OnCloProperties();
	afx_msg void OnClose();
	afx_msg void OnObjectInsert();
	afx_msg void OnCloObjectName();
	afx_msg void OnButton5();
	afx_msg void OnButton6();
	afx_msg void OnPlayerDelete();
	afx_msg void OnPlayerRed();
	afx_msg void OnPlayerBlue();
	afx_msg void OnPlayerGreen();
	afx_msg void OnPlayerYellow();
	afx_msg void OnPlayerNext();
	afx_msg void OnPlayerPrev();
	afx_msg void OnButton1();
	afx_msg void OnInsertMine();
	afx_msg void OnInsertTerrain();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void DrawItemModel(int x,int y,grSurface *ds,grHardwareSurface *surf,int model_num,bool draw_large);

/////////////////////////////////////////////////////////////////////////////
// CObjectProperties dialog

class CObjectProperties : public CDialog
{
// Construction
public:
	CObjectProperties(int Object,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CObjectProperties)
	enum { IDD = IDD_OBJECT_PROPERTIES };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObjectProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_Object;

	// Generated message map functions
	//{{AFX_MSG(CObjectProperties)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OBJECTDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
