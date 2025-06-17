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
 #if !defined(AFX_TEXTUREDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
#define AFX_TEXTUREDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextureDialogBar.h : header file
//
#include "resource.h"
#include "TexturePalette.h"

/////////////////////////////////////////////////////////////////////////////
// CTextureDialogBar dialog
enum {	TAB_TEXTURE_CUSTOM,
		TAB_TEXTURE_LEVEL};
		//@@TAB_TEXTURE_ROOM};

enum {	TIMER_TEXTURES = 1234};

class CTextureDialogBar;
/////////////////////////////////////////////////////////////////////////////
// CTextureBarList dialog

class CTextureBarList : public CDialog
{
// Construction
public:
	CTextureBarList(CWnd* pParent = NULL);   // standard constructor
	int AddTextureToList(int tex_slot);
	int RemoveTextureFromList(int tex_slot);

// Dialog Data
	//{{AFX_DATA(CTextureBarList)
	enum { IDD = IDD_BAR_LIST };
	CListBox	m_List;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureBarList)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
public:
	CTextureDialogBar *m_ParentWindow;
	bool ignore_mark;

	// Generated message map functions
	//{{AFX_MSG(CTextureBarList)
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

class CTextureDialogBar : public CDialogBar
{
friend class CTextureBarList;
// Construction
public:
	CTextureBarList *m_CustomDialog; // quick hack; temp public
	void SetCurrentFace(int Room,int Face);
	int GetCurrentTexture(void);
	void SetCurrentTexture(int Texture);
	int GrabTexture(void);
	CTextureDialogBar();   // standard constructor
	void TexturePaletteDone();
	//	Display current tab dialog	
	void ShowCurrentTextureTab();
	//@@void RoomReplaceTexStrings(short *Textures_in_use);
	void AddRemoveLevelTexture(char *name,int num,bool add);
	//@@void AddRemoveRoomTexture(char *name,bool add);
#ifdef _DEBUG
	void UpdateLevelTextureCount(char *name,int newnum,int oldnum);
#endif
	bool WriteTextureList(CFILE *outfile,int list);
	int ReadTextureList(CFILE *infile);

// Dialog Data
	//{{AFX_DATA(CTextureDialogBar)
	enum { IDD = IDD_TEXTUREBAR };
	CStatic	m_CTLightValueRed;
	CStatic	m_CTLightValueGreen;
	CStatic	m_CTLightValueBlue;
	CStatic	m_CFLightValueRed;
	CStatic	m_CFLightValueGreen;
	CStatic	m_CFLightValueBlue;
	CStatic	m_CTLightView;
	CStatic	m_CFLightView;
	CStatic	m_CTView;
	CStatic	m_CFView;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureDialogBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_CurrentFaceTextureChanged;
	bool m_CurrentTextureChanged;
	int m_CurrentRoom;
	int m_CurrentTexture;
	int m_CurrentFaceTexture;
	void InitKeyPad(void);
	void RedrawArea();
	void PaintPreviewArea(CWnd *pWnd);
	void RedrawPreviewArea(CWnd *pWnd);
	void DoTextureTabNotify(NMHDR *nmhdr);
	int m_CurrentTextureTab;

	CTexturePalette* m_pTexturePalette;

	CTextureBarList *m_TextureTabDialog;
	CTextureBarList *m_LevelDialog;
	//@@CTextureBarList *m_RoomDialog;
	// Generated message map functions
	//{{AFX_MSG(CTextureDialogBar)
	afx_msg LONG OnInitDialog ( UINT, LONG );
	afx_msg void OnPaint();
	afx_msg void OnControlUpdate(CCmdUI*);
	afx_msg void OnTexturePalette();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnCtProperties();
	afx_msg void OnCfProperties();
	afx_msg void OnApplyToCurrent();
	afx_msg void OnApplyToMarked();
	afx_msg void OnPropagate();
	afx_msg void OnAlignment();
	afx_msg void OnClose();
	afx_msg void OnGrab();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void ApplyTexture(room *rp, int facenum, int tnum, short *Textures_in_use);

/////////////////////////////////////////////////////////////////////////////
// CTextureProperties dialog

class CTextureProperties : public CDialog
{
// Construction
public:
	CTextureProperties(int Texture,CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTextureProperties)
	enum { IDD = IDD_TEXTURE_PROPERTIES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextureProperties)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int m_Texture;

	// Generated message map functions
	//{{AFX_MSG(CTextureProperties)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTUREDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
