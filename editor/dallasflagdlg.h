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
 #if !defined(AFX_DALLASFLAGDLG_H__DDAEAF22_B1DC_11D2_A4E0_00A0C96ED60D__INCLUDED_)
#define AFX_DALLASFLAGDLG_H__DDAEAF22_B1DC_11D2_A4E0_00A0C96ED60D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DallasFlagDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDallasFlagDlg dialog

class CDallasFlagDlg : public CDialog
{
// Construction
public:
	CDallasFlagDlg(CWnd* pParent = NULL);   // standard constructor

	int		m_FlagsValue;
	int		m_ValidFlagsMask;
	CString	m_FlagsName;

// Dialog Data
	//{{AFX_DATA(CDallasFlagDlg)
	enum { IDD = IDD_DALLAS_FLAG_DIALOG };
	CCheckListBox	m_FlagListBox;
	CStatic	m_FlagPromptText;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDallasFlagDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDallasFlagDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DALLASFLAGDLG_H__DDAEAF22_B1DC_11D2_A4E0_00A0C96ED60D__INCLUDED_)
