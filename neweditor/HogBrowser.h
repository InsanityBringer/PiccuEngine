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
 



#if !defined(AFX_HOGBROWSER_H__DF1B55C0_1D0C_11D3_AB2B_006008BF0B09__INCLUDED_)
#define AFX_HOGBROWSER_H__DF1B55C0_1D0C_11D3_AB2B_006008BF0B09__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// HogBrowser.h : header file
//

void RegisterHogFile(char *filename,int lib_id);
void UnRegisterHogFile(int lib_id);

/////////////////////////////////////////////////////////////////////////////
// CHogBrowser dialog
typedef struct tWildcardNode
{
	char description[256];
	char wildcard[128];
	tWildcardNode *next;
}tWildcardNode;

class CHogBrowser : public CDialog
{
// Construction
public:
	CHogBrowser(char *wildcard_list = NULL, CWnd* pParent = NULL);   // standard constructor
	~CHogBrowser();
	bool GetFilename(char *buffer,int buffer_size);

// Dialog Data
	//{{AFX_DATA(CHogBrowser)
	enum { IDD = IDD_HOGBROWSER };
	CComboBox	m_HogName;
	CComboBox	m_FileType;
	CListBox	m_FileList;
	CString	m_FileName;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CHogBrowser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EnableButtons(bool enable);
	bool m_Selected;
	bool m_Busy;
	bool m_FirstPaint;
	tWildcardNode *m_WildcardData;

	// Generated message map functions
	//{{AFX_MSG(CHogBrowser)
	afx_msg void OnOpen();
	afx_msg void OnSelchangeHogfileChoose();
	afx_msg void OnSelchangeFiletype();
	afx_msg void OnDblclkFileList();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeFileList();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.


#endif // !defined(AFX_HOGBROWSER_H__DF1B55C0_1D0C_11D3_AB2B_006008BF0B09__INCLUDED_)
