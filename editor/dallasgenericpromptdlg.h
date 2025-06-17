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
 


#if !defined(AFX_DALLASGENERICPROMPTDLG_H__94807B41_929B_11D2_A4E0_00A0C96ED60D__INCLUDED_)
#define AFX_DALLASGENERICPROMPTDLG_H__94807B41_929B_11D2_A4E0_00A0C96ED60D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DallasGenericPromptDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDallasGenericPromptDlg dialog

class CDallasGenericPromptDlg : public CDialog
{
// Construction
public:
	CDallasGenericPromptDlg(CWnd* pParent = NULL);   // standard constructor

	CString m_DialogTitle;
	CString m_PromptText;
	CString m_PromptData;
	int m_MaxDataLength;

// Dialog Data
	//{{AFX_DATA(CDallasGenericPromptDlg)
	enum { IDD = IDD_DALLAS_GENERIC_PROMPT_DIALOG };
	CEdit	m_GenericDataEdit;
	CStatic	m_GenericPromptStatic;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDallasGenericPromptDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDallasGenericPromptDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DALLASGENERICPROMPTDLG_H__94807B41_929B_11D2_A4E0_00A0C96ED60D__INCLUDED_)
