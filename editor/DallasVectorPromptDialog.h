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
 #if !defined(AFX_DALLASVECTORPROMPTDIALOG_H__9E6377E1_95AF_11D2_A4E0_00A0C96ED60D__INCLUDED_)
#define AFX_DALLASVECTORPROMPTDIALOG_H__9E6377E1_95AF_11D2_A4E0_00A0C96ED60D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DallasVectorPromptDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDallasVectorPromptDialog dialog

class CDallasVectorPromptDialog : public CDialog
{
// Construction
public:
	CDallasVectorPromptDialog(CWnd* pParent = NULL);   // standard constructor

	float m_PromptData1;
	float m_PromptData2;
	float m_PromptData3;

// Dialog Data
	//{{AFX_DATA(CDallasVectorPromptDialog)
	enum { IDD = IDD_DALLAS_VECTOR_DIALOG };
	CEdit	m_Data3Edit;
	CEdit	m_Data2Edit;
	CEdit	m_Data1Edit;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDallasVectorPromptDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CDallasVectorPromptDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DALLASVECTORPROMPTDIALOG_H__9E6377E1_95AF_11D2_A4E0_00A0C96ED60D__INCLUDED_)
