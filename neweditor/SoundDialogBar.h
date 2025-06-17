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
 #if !defined(AFX_SOUNDDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
#define AFX_SOUNDDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SoundDialogBar.h : header file
//
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CSoundDialogBar dialog

class CSoundDialogBar;

class CSoundDialogBar : public CDialogBar
{
// Construction
public:
	CSoundDialogBar();   // standard constructor
	void UpdateDialog();
	int AddSoundToList(int sound_slot);
	int RemoveSoundFromList(int sound_slot);

// Dialog Data
	//{{AFX_DATA(CSoundDialogBar)
	enum { IDD = IDD_SOUNDBAR };
	CComboBox	m_sound_combo;
	float	m_volume;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundDialogBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void InitKeyPad(void);
	// Generated message map functions
	//{{AFX_MSG(CSoundDialogBar)
	afx_msg LONG OnInitDialog ( UINT, LONG );
	afx_msg void OnDestroy();
	afx_msg void OnControlUpdate(CCmdUI* pCmdUI);
	afx_msg void OnSoundDelete();
	afx_msg void OnSoundInsert();
	afx_msg void OnSoundName();
	afx_msg void OnSelchangeSoundCombo();
	afx_msg void OnKillfocusSoundVolume();
	afx_msg void OnWaypointInsert();
	afx_msg void OnWaypointDelete();
	afx_msg void OnSoundPlay();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SOUNDDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
