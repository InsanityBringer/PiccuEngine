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
 #if !defined(AFX_LEVELINFODIALOG_H__FBB555C1_E754_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_LEVELINFODIALOG_H__FBB555C1_E754_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LevelInfoDialog.h : header file
//

#include "mission.h"

/////////////////////////////////////////////////////////////////////////////
// CLevelInfoDialog dialog

class CLevelInfoDialog : public CDialog
{
// Construction
public:
	CLevelInfoDialog(level_info *li, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CLevelInfoDialog)
	enum { IDD = IDD_LEVELINFO };
	CString	m_copyright;
	CString	m_designer;
	CString	m_name;
	CString	m_notes;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLevelInfoDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	//Copy data back out of dialog
	void GetLevelInfo(level_info *li);

	//The data we're editing
	level_info *m_level_info;

	// Generated message map functions
	//{{AFX_MSG(CLevelInfoDialog)
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LEVELINFODIALOG_H__FBB555C1_E754_11D2_A6A1_006097E07445__INCLUDED_)
