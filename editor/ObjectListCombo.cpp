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
 // ObjectListCombo.cpp : implementation file
//

#include "../neweditor/StdAfx.h"

#ifndef NEWEDITOR
#include "editor.h"
#else
#include "../neweditor/NewEditor.h"
#endif

#include "ObjectListCombo.h"
#include "object.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CObjectListCombo

CObjectListCombo::CObjectListCombo()
{
}

CObjectListCombo::~CObjectListCombo()
{
}


BEGIN_MESSAGE_MAP(CObjectListCombo, CComboBox)
	//{{AFX_MSG_MAP(CObjectListCombo)
	ON_CONTROL_REFLECT(CBN_SETFOCUS, OnSetfocus)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectListCombo message handlers

void CObjectListCombo::Init(int type,int selected_handle)
{
	//Save the type
	m_type = type;

	//Clear the combo box
	ResetContent();

	//Add the "none" sound
	AddString("<none>");
	SetItemData(0,OBJECT_HANDLE_NONE);
	SetCurSel(0);		//default
	
	//Now add all the sounsd
	for (int i=0;i<=Highest_object_index;i++) 
	{
		if(Objects[i].type == OBJ_NONE)
		{
			continue;
		}

		//Only lists named objects.  If we ever need all objects, add a flag.
		if (Objects[i].name && ((type == OBJ_NONE) || (Objects[i].type == type)))
		{
			char str[100];
			sprintf(str,"%s (%s, %x)",Objects[i].name?Objects[i].name:"<no name>",Object_type_names[Objects[i].type],Objects[i].handle);
			
			int index = AddString(str);
			SetItemData(index,Objects[i].handle);
			if (Objects[i].handle == selected_handle)
			{
				SetCurSel(index);
			}
		}
	}
}

//takes object handle
void CObjectListCombo::SetSelected(int selected_handle)
{
	//Now add all the sounsd
	int count = GetCount();
	for (int i=0;i<count;i++) {
		if (selected_handle == (int) GetItemData(i)) {
			SetCurSel(i);
			return;
		}
	}

	//Couldn't find selected -- use none
	SetCurSel(0);
}

int CObjectListCombo::GetSelected() 
{
	return (int) GetItemData(GetCurSel());
}


void CObjectListCombo::OnSetfocus() 
{
	//Update list in case the items have changed
	Init(m_type,GetSelected());
}
