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
 #if !defined(AFX_LATHEDIALOG_H__A0BA7220_FEA8_11D2_A6A1_006097E07445__INCLUDED_)
#define AFX_LATHEDIALOG_H__A0BA7220_FEA8_11D2_A6A1_006097E07445__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LatheDialog.h : header file
//

// lathe rotations
#define X_AXIS	0
#define Y_AXIS	1
#define Z_AXIS	2

/////////////////////////////////////////////////////////////////////////////
// CLatheDialog dialog

class CLatheDialog : public CDialog
{
// Construction
public:
	CLatheDialog(CWnd* pParent = NULL);   // standard constructor

	BOOL m_Inward;

// Dialog Data
	//{{AFX_DATA(CLatheDialog)
	enum { IDD = IDD_LATHE_VERTS };
	BOOL	m_bEndCaps;
	UINT	m_Num_sides;
	int		m_Rotation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLatheDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLatheDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnXAxis();
	afx_msg void OnYAxis();
	afx_msg void OnZAxis();
	afx_msg void OnInward();
	afx_msg void OnOutward();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LATHEDIALOG_H__A0BA7220_FEA8_11D2_A6A1_006097E07445__INCLUDED_)
