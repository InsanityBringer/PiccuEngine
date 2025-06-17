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
 #if !defined(AFX_TRIGGERDIALOG_H__00E59280_1C6B_11D3_A6A2_F3ACA1D5962E__INCLUDED_)
#define AFX_TRIGGERDIALOG_H__00E59280_1C6B_11D3_A6A2_F3ACA1D5962E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TriggerDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTriggerDialog dialog

class CTriggerDialog : public CDialog
{
// Construction
public:
	CTriggerDialog(CWnd* pParent = NULL);   // standard constructor
	void UpdateDialog();

// Dialog Data
	//{{AFX_DATA(CTriggerDialog)
	enum { IDD = IDD_TRIGGERDIALOG };
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTriggerDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
	int		m_AddFloating;

protected:
	void SetCurroomFromTrigger();
	void EnableControls(bool enable);

	// Generated message map functions
	//{{AFX_MSG(CTriggerDialog)
	afx_msg void OnTrigAddTrigger();
	afx_msg void OnTrigDelete();
	afx_msg void OnTrigEditName();
	afx_msg void OnTrigEditScript();
	afx_msg void OnTrigClutter();
	afx_msg void OnTrigOneShot();
	afx_msg void OnTrigPlayer();
	afx_msg void OnTrigPlayerWeapon();
	afx_msg void OnTrigRobot();
	afx_msg void OnTrigRobotWeapon();
	virtual void OnCancel();
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	afx_msg void OnTrigPrevMine();
	afx_msg void OnTrigPrevRoom();
	afx_msg void OnTrigSelect();
	afx_msg void OnTrigNextRoom();
	afx_msg void OnTrigNextMine();
	virtual void OnOK();
	afx_msg void OnTrigFloating();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

int FindTriggerName(char *name);
int FindPrevTrigInRoom(int roomnum,int start);
int FindNextTrigInRoom(int roomnum,int start);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRIGGERDIALOG_H__00E59280_1C6B_11D3_A6A2_F3ACA1D5962E__INCLUDED_)
