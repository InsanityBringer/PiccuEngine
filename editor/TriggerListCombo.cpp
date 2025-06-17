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
 // TriggerListCombo.cpp : implementation file
//

#include "../neweditor/StdAfx.h"

#ifndef NEWEDITOR
#include "editor.h"
#else
#include "../neweditor/NewEditor.h"
#endif

#include "TriggerListCombo.h"
#include "trigger.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTriggerListCombo

CTriggerListCombo::CTriggerListCombo()
{
}

CTriggerListCombo::~CTriggerListCombo()
{
}


BEGIN_MESSAGE_MAP(CTriggerListCombo, CComboBox)
	//{{AFX_MSG_MAP(CTriggerListCombo)
	ON_CONTROL_REFLECT(CBN_SETFOCUS, OnSetfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTriggerListCombo message handlers

void CTriggerListCombo::Init(int selected) 
{
	//Clear the combo box
	ResetContent();

	//Add the "none" sound
	AddString("<none>");
	SetItemData(0,-1);
	SetCurSel(0);		//default
	
	//Now add all the sounsd
	for (int i=0;i<Num_triggers;i++) {
		int index = AddString(Triggers[i].name);
		SetItemData(index,i);
		if (selected == i)
			SetCurSel(index);
	}
}

void CTriggerListCombo::SetSelected(int selected) 
{
	//Now add all the sounsd
	int count = GetCount();
	for (int i=0;i<count;i++) {
		if (selected == (int) GetItemData(i))
			SetCurSel(i);
	}
}

int CTriggerListCombo::GetSelected() 
{
	return (int) GetItemData(GetCurSel());
}


void CTriggerListCombo::OnSetfocus() 
{
	//Update list in case the items have changed
	Init(GetSelected());
}
