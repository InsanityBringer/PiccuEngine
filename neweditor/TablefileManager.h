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
 



#if !defined(AFX_TABLEFILEMANAGER_H__D6210D80_ED2C_11D2_AB2B_006008BF0B09__INCLUDED_)
#define AFX_TABLEFILEMANAGER_H__D6210D80_ED2C_11D2_AB2B_006008BF0B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TablefileManager.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTablefileManager dialog


class CTablefileManager : public CDialog
{
// Construction
public:
	CTablefileManager(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTablefileManager)
	enum { IDD = IDD_TABLEFILE_MANAGER };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTablefileManager)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	char *m_Base_tablefile;
	char *m_Mission_tablefile;
	char *m_Module_tablefile;
	void UpdateDialog(void);
	void DisableAll(void);
	void ChangeTablefile(char *filename,int type);

	// Generated message map functions
	//{{AFX_MSG(CTablefileManager)
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	afx_msg void OnBrowseA();
	afx_msg void OnBrowseB();
	afx_msg void OnBrowseC();
	afx_msg void OnRemoveA();
	afx_msg void OnRemoveB();
	afx_msg void OnRemoveC();
	afx_msg void OnDisplayAll();
	afx_msg void OnDisplayA();
	afx_msg void OnDisplayB();
	afx_msg void OnDisplayC();
	afx_msg void OnDestroy();
	afx_msg void OnHelp();
	afx_msg void OnBrowseHogA();
	afx_msg void OnBrowseHogB();
	afx_msg void OnBrowseHogC();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABLEFILEMANAGER_H__D6210D80_ED2C_11D2_AB2B_006008BF0B09__INCLUDED_)
