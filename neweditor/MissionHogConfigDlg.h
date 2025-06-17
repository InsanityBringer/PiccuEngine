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
 #if !defined(AFX_MISSIONHOGCONFIGDLG_H__7D8298E0_20FA_11D3_AB2B_006008BF0B09__INCLUDED_)
#define AFX_MISSIONHOGCONFIGDLG_H__7D8298E0_20FA_11D3_AB2B_006008BF0B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MissionHogConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMissionHogConfigDlg dialog

class MissionHogInfo
{
private:
	bool loaded;
	int library_id;
	char pathname[_MAX_PATH];

public:
	MissionHogInfo(){loaded = false; library_id = -1; pathname[0] = '\0';}
	~MissionHogInfo(){ UnloadHog(); }

	bool ChangeHog(char *path,int *lib_id=NULL);
	bool GetLibraryHandle(int *lib_id);
	bool GetPathName(char *pathname);
	bool UnloadHog(void);
};
extern MissionHogInfo MsnHogInfo;

class CMissionHogConfigDlg : public CDialog
{
// Construction
public:
	CMissionHogConfigDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMissionHogConfigDlg)
	enum { IDD = IDD_MISSIONHOG };
	CString	m_Name;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMissionHogConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_ChangesMade,m_HadOriginalHog;
	char m_OriginalHog[_MAX_PATH];
	void UpdateDlg(void);

	// Generated message map functions
	//{{AFX_MSG(CMissionHogConfigDlg)
	afx_msg void OnUnload();
	afx_msg void OnChange();
	virtual void OnOK();
	virtual void OnCancel();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MISSIONHOGCONFIGDLG_H__7D8298E0_20FA_11D3_AB2B_006008BF0B09__INCLUDED_)
