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
 

#if !defined(AFX_SCRIPTCOMPILER_H__F19CB060_1DAA_11D3_AB2B_006008BF0B09__INCLUDED_)
#define AFX_SCRIPTCOMPILER_H__F19CB060_1DAA_11D3_AB2B_006008BF0B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ScriptCompiler.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CScriptCompiler dialog

class CScriptCompiler : public CDialog
{
// Construction
public:
	CScriptCompiler(CWinApp* pParent = NULL);   // standard constructor
	BOOL Create();
	void UpdateAll(void);
	void SetDialogName(char *name,...);

// Dialog Data
	//{{AFX_DATA(CScriptCompiler)
	enum { IDD = IDD_SCRIPTCOMPILE };
	CListBox	m_List;
	CString	m_ScriptText;
	CString	m_Output;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScriptCompiler)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CWinApp* m_pParent;
	int m_nID;
	bool bCalledClose;

	// Generated message map functions
	//{{AFX_MSG(CScriptCompiler)
	afx_msg void OnCompile();
	afx_msg void OnConfigure();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnDblclkScriptlist();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SCRIPTCOMPILER_H__F19CB060_1DAA_11D3_AB2B_006008BF0B09__INCLUDED_)
