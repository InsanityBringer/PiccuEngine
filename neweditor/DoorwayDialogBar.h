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
 #if !defined(AFX_DOORWAYDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
#define AFX_DOORWAYDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// DoorwayDialogBar.h : header file
//
#include "resource.h"
#include "ned_Door.h"

/////////////////////////////////////////////////////////////////////////////
// CDoorwayDialogBar dialog

enum {	TIMER_DOORWAYS = 1234};

class CDoorwayDialogBar;

class CDoorwayDialogBar : public CDialogBar
{
// Construction
public:
	void SetCurrentDoorway(int Doorway);
	int GetCurrentDoor(void);
	void SetCurrentDoor(int Door);
	CDoorwayDialogBar();   // standard constructor
	void AddRemoveDoorway(char *name,bool add);
	int AddDoorToList(int door_slot);
	int RemoveDoorFromList(int door_slot);
	void ForceSelChange();

// Dialog Data
	//{{AFX_DATA(CDoorwayDialogBar)
	enum { IDD = IDD_DOORWAYBAR };
	CComboBox	m_Combo;
	CStatic	m_CDView;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDoorwayDialogBar)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
//	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool m_CurrentDoorChanged;
	bool m_CurrentDoorwayChanged;
	int m_CurrentDoorway;
	int m_CurrentRoom;
	int m_CurrentDoor;
	void InitKeyPad(void);
	void RedrawArea();
	void PaintPreviewArea(CWnd *pWnd);
	void RedrawPreviewArea(CWnd *pWnd);
	void KeyCheck(int id,int keynum);
	char door_in_mem[MAX_DOORS];
	// Generated message map functions
	//{{AFX_MSG(CDoorwayDialogBar)
	afx_msg LONG OnInitDialog ( UINT, LONG );
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClose();
	afx_msg void OnDoorInsert();
	afx_msg void OnSelchangeDoors();
	afx_msg void OnPlaceDoor();
	afx_msg void OnAttachDoor();
	afx_msg void OnControlUpdate(CCmdUI* pCmdUI);
	afx_msg void OnCdwDoorName();
	afx_msg void OnKey1();
	afx_msg void OnKey2();
	afx_msg void OnKey3();
	afx_msg void OnKey4();
	afx_msg void OnKey5();
	afx_msg void OnKey6();
	afx_msg void OnKey7();
	afx_msg void OnKey8();
	afx_msg void OnLocked();
	afx_msg void OnNeedsAll();
	afx_msg void OnNeedsOne();
	afx_msg void OnAutoClose();
	afx_msg void OnGBIgnore();
	afx_msg void OnKillFocusInitialPos();
	afx_msg void OnKillFocusHitPoints();
	afx_msg void OnPrevDoorway();
	afx_msg void OnNextDoorway();
	afx_msg void OnDeleteDoor();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

int GetDoorwayCount();
// void DrawItemModel(int x,int y,grSurface *ds,grHardwareSurface *surf,int model_num);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DOORWAYDIALOGBAR_H__8B433C34_EE1C_11D2_B5CD_000000000000__INCLUDED_)
