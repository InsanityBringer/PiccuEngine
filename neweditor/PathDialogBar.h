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
 #if !defined(AFX_PATHDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
#define AFX_PATHDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PathDialogBar.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPathDialogBar dialog

class CPathDialogBar;

class CPathDialogBar : public CDialogBar
{
// Construction
public:
	CPathDialogBar();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPathDialogBar)
	enum { IDD = IDD_PATHBAR };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPathDialogBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPathDialogBar)
	afx_msg LONG OnInitDialog ( UINT, LONG );
	afx_msg void OnDestroy();
	afx_msg void OnControlUpdate(CCmdUI* pCmdUI);
	afx_msg void OnBnodeCreate();
	afx_msg void OnBnodeDestroy();
	afx_msg void OnBnodeVerify();
	afx_msg void OnPathRadio();
	afx_msg void OnBnodeRadio();
	afx_msg void OnBnodeAutoMakeEdges();
	afx_msg void OnBnodeAutoEdgeRoom();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

bool ned_EBNode_VerifyGraph();

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PATHDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
