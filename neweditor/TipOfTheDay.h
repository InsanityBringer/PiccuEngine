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
 #if !defined(AFX_TIPOFTHEDAY_H__9A35DE80_0141_11D3_8F49_00104B27BFF0__INCLUDED_)
#define AFX_TIPOFTHEDAY_H__9A35DE80_0141_11D3_8F49_00104B27BFF0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TipOfTheDay.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTipOfTheDay dialog

class CTipOfTheDay : public CDialog
{
// Construction
public:
	int ReadTips(char *file);
	int CountTips(char *file);
	char ** m_lpszTips;
	int m_CurTip;
	int m_NumTips;
	CTipOfTheDay(CWnd* pParent = NULL);   // standard constructor
	~CTipOfTheDay();

// Dialog Data
	//{{AFX_DATA(CTipOfTheDay)
	enum { IDD = IDD_TIPDIALOG };
	CString	m_strTip;
	BOOL	m_ShowNextTime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTipOfTheDay)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTipOfTheDay)
	afx_msg void OnOutragewebpage();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnNexttip();
	afx_msg void OnPrevtip();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TIPOFTHEDAY_H__9A35DE80_0141_11D3_8F49_00104B27BFF0__INCLUDED_)
