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
 #if !defined(AFX_GOALDIALOG_H__E754F600_237F_11D3_A6A2_9D6DF32FC010__INCLUDED_)
#define AFX_GOALDIALOG_H__E754F600_237F_11D3_A6A2_9D6DF32FC010__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// GoalDialog.h : header file
//

#include "RoomListCombo.h"
#include "ObjectListCombo.h"
#include "TriggerListCombo.h"

/////////////////////////////////////////////////////////////////////////////
// CGoalDialog dialog

class CGoalDialog : public CDialog
{
// Construction
public:
	CGoalDialog(CWnd* pParent = NULL);   // standard constructor
	bool ValidateCurrentGoal();
	bool ValidateCurrentItem();
	void UpdateDialog();
	void DoGoalFlagToggle(int flag);
	int m_CurrentGoal;
	int m_CurrentItem;

// Dialog Data
	//{{AFX_DATA(CGoalDialog)
	enum { IDD = IDD_GOALDIALOG };
	CTriggerListCombo	m_trigger_combo;
	CObjectListCombo	m_object_combo;
	CRoomListCombo	m_room_combo;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGoalDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// Implementation
protected:
	void EnableControls(bool enable);

	// Generated message map functions
	//{{AFX_MSG(CGoalDialog)
	afx_msg void OnAddGoal();
	afx_msg void OnDeleteGoal();
	afx_msg void OnNextGoal();
	afx_msg void OnPrevGoal();
	afx_msg void OnGoalSelect();
	afx_msg void OnKillfocusGoalName();
	afx_msg void OnKillfocusGoalLocobjName();
	afx_msg void OnKillfocusGoalDescription();
	afx_msg void OnKillfocusGoalPriority();
	virtual BOOL OnInitDialog();
	afx_msg void OnKillfocusGoalList();
	afx_msg void OnGoalCompleted();
	afx_msg void OnGoalEnabled();
	afx_msg void OnGoalGbKnows();
	afx_msg void OnGoalLocBased();
	afx_msg void OnGoalObjective();
	afx_msg void OnGoalSecondary();
	virtual void OnCancel();
	afx_msg void OnPaint();
	afx_msg void OnGoalItemActivate();
	afx_msg void OnGoalItemEnter();
	afx_msg void OnGoalItemPickupDestroy();
	afx_msg void OnGoalItemPlayerCollide();
	afx_msg void OnGoalItemPlayerWeapon();
	afx_msg void OnGoalItemAdd();
	afx_msg void OnGoalItemDelete();
	afx_msg void OnGoalItemDallas();
	afx_msg void OnGoalItemRoom();
	afx_msg void OnGoalItemSelect();
	afx_msg void OnGoalItemTrigger();
	afx_msg void OnGoalItemObject();
	afx_msg void OnNextGoalItem();
	afx_msg void OnPrevGoalItem();
	afx_msg void OnSelendokGoalItemTriggerCombo();
	afx_msg void OnSelendokGoalItemRoomCombo();
	afx_msg void OnSelendokGoalItemObjectCombo();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

void make_goal_dialog_text(char *big_text_message);

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GOALDIALOG_H__E754F600_237F_11D3_A6A2_9D6DF32FC010__INCLUDED_)
