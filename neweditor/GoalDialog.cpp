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
 // GoalDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "GoalDialog.h"
#include "EditLineDialog.h"
#include "levelgoal.h"
#include "ned_Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGoalDialog dialog


CGoalDialog::CGoalDialog(CWnd* pParent )
	: CDialog(CGoalDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGoalDialog)
	//}}AFX_DATA_INIT
}


void CGoalDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGoalDialog)
	DDX_Control(pDX, IDC_GOAL_ITEM_TRIGGER_COMBO, m_trigger_combo);
	DDX_Control(pDX, IDC_GOAL_ITEM_ROOM_COMBO, m_room_combo);
	DDX_Control(pDX, IDC_GOAL_ITEM_OBJECT_COMBO, m_object_combo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGoalDialog, CDialog)
	//{{AFX_MSG_MAP(CGoalDialog)
	ON_BN_CLICKED(IDC_ADD_GOAL, OnAddGoal)
	ON_BN_CLICKED(IDC_DELETE_GOAL, OnDeleteGoal)
	ON_BN_CLICKED(IDC_NEXT_GOAL, OnNextGoal)
	ON_BN_CLICKED(IDC_PREV_GOAL, OnPrevGoal)
	ON_BN_CLICKED(IDC_GOAL_SELECT, OnGoalSelect)
	ON_EN_KILLFOCUS(IDC_GOAL_NAME, OnKillfocusGoalName)
	ON_EN_KILLFOCUS(IDC_GOAL_LOCOBJ_NAME, OnKillfocusGoalLocobjName)
	ON_EN_KILLFOCUS(IDC_GOAL_DESCRIPTION, OnKillfocusGoalDescription)
	ON_EN_KILLFOCUS(IDC_GOAL_PRIORITY, OnKillfocusGoalPriority)
	ON_EN_KILLFOCUS(IDC_GOAL_LIST, OnKillfocusGoalList)
	ON_BN_CLICKED(IDC_GOAL_COMPLETED, OnGoalCompleted)
	ON_BN_CLICKED(IDC_GOAL_ENABLED, OnGoalEnabled)
	ON_BN_CLICKED(IDC_GOAL_GB_KNOWS, OnGoalGbKnows)
	ON_BN_CLICKED(IDC_GOAL_LOC_BASED, OnGoalLocBased)
	ON_BN_CLICKED(IDC_GOAL_OBJECTIVE, OnGoalObjective)
	ON_BN_CLICKED(IDC_GOAL_SECONDARY, OnGoalSecondary)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_GOAL_ITEM_ACTIVATE, OnGoalItemActivate)
	ON_BN_CLICKED(IDC_GOAL_ITEM_ENTER, OnGoalItemEnter)
	ON_BN_CLICKED(IDC_GOAL_ITEM_PICKUP_DESTROY, OnGoalItemPickupDestroy)
	ON_BN_CLICKED(IDC_GOAL_ITEM_PLAYER_COLLIDE, OnGoalItemPlayerCollide)
	ON_BN_CLICKED(IDC_GOAL_ITEM_PLAYER_WEAPON, OnGoalItemPlayerWeapon)
	ON_BN_CLICKED(IDC_GOAL_ITEM_ADD, OnGoalItemAdd)
	ON_BN_CLICKED(IDC_GOAL_ITEM_DELETE, OnGoalItemDelete)
	ON_BN_CLICKED(IDC_GOAL_ITEM_DALLAS, OnGoalItemDallas)
	ON_BN_CLICKED(IDC_GOAL_ITEM_ROOM, OnGoalItemRoom)
	ON_BN_CLICKED(IDC_GOAL_ITEM_SELECT, OnGoalItemSelect)
	ON_BN_CLICKED(IDC_GOAL_ITEM_TRIGGER, OnGoalItemTrigger)
	ON_BN_CLICKED(IDC_GOAL_ITEM_OBJECT, OnGoalItemObject)
	ON_BN_CLICKED(IDC_NEXT_GOAL_ITEM, OnNextGoalItem)
	ON_BN_CLICKED(IDC_PREV_GOAL_ITEM, OnPrevGoalItem)
	ON_CBN_SELENDOK(IDC_GOAL_ITEM_TRIGGER_COMBO, OnSelendokGoalItemTriggerCombo)
	ON_CBN_SELENDOK(IDC_GOAL_ITEM_ROOM_COMBO, OnSelendokGoalItemRoomCombo)
	ON_CBN_SELENDOK(IDC_GOAL_ITEM_OBJECT_COMBO, OnSelendokGoalItemObjectCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGoalDialog message handlers

void CGoalDialog::OnAddGoal() 
{
	// TODO: Add your control notification handler code here
	m_CurrentGoal = Level_goals.AddGoal(true);
	m_CurrentItem = 0;

	UpdateDialog();

	OnGoalItemAdd();
}

void CGoalDialog::OnDeleteGoal() 
{
	// TODO: Add your control notification handler code here
	Level_goals.DeleteGoal(m_CurrentGoal);
	m_CurrentGoal = 0;
	m_CurrentItem = 0;

	UpdateDialog();
}

void CGoalDialog::OnNextGoal() 
{
	// TODO: Add your control notification handler code here
	int num_goals = Level_goals.GetNumGoals();

	if(m_CurrentGoal < num_goals - 1)
	{
		m_CurrentGoal++;
	}
	else
	{
		m_CurrentGoal = 0;
	}
	m_CurrentItem = 0;

	UpdateDialog();
}

void CGoalDialog::OnPrevGoal() 
{
	// TODO: Add your control notification handler code here
	int num_goals = Level_goals.GetNumGoals();

	if(m_CurrentGoal > 0)
	{
		m_CurrentGoal--;
	}
	else
	{
		m_CurrentGoal = num_goals - 1;
	}
	m_CurrentItem = 0;

	UpdateDialog();
}

void CGoalDialog::OnGoalSelect() 
{
	// TODO: Add your control notification handler code here
	int n;
	int num_goals = Level_goals.GetNumGoals();

	if (InputNumber(&n,"Select Goal","Enter goal number to select",this)) {

		if (n > num_goals || n <= 0) {
			OutrageMessageBox("Invalid goal number.");
			return;
		}

		m_CurrentGoal = n-1;
		m_CurrentItem = 0;

		UpdateDialog();
	}
}

void CGoalDialog::OnKillfocusGoalName() 
{
	// TODO: Add your control notification handler code here
	char str[256];
	
	((CEdit *) GetDlgItem(IDC_GOAL_NAME))->GetWindowText(str,255);
	Level_goals.GoalSetName(m_CurrentGoal, str);

	UpdateDialog();
}

void CGoalDialog::OnKillfocusGoalLocobjName() 
{
	// TODO: Add your control notification handler code here
	char str[256];
	
	((CEdit *) GetDlgItem(IDC_GOAL_LOCOBJ_NAME))->GetWindowText(str,255);
	Level_goals.GoalSetItemName(m_CurrentGoal, str);

	UpdateDialog();
}

void CGoalDialog::OnKillfocusGoalDescription() 
{
	// TODO: Add your control notification handler code here
	char str[2560];
	
	((CEdit *) GetDlgItem(IDC_GOAL_DESCRIPTION))->GetWindowText(str,2559);
	Level_goals.GoalSetDesc(m_CurrentGoal, str);

	UpdateDialog();
}

void CGoalDialog::OnKillfocusGoalPriority() 
{
	// TODO: Add your control notification handler code here
	int priority;
	char str[256];

	((CEdit *) GetDlgItem(IDC_GOAL_PRIORITY))->GetWindowText(str,255);
	sscanf(str, "%d", &priority);

	Level_goals.GoalPriority(m_CurrentGoal, LO_SET_SPECIFIED, &priority);

	UpdateDialog();
}

bool CGoalDialog::ValidateCurrentGoal()
{
	int num_goals = Level_goals.GetNumGoals();

	if(num_goals > 0)
	{
		if(m_CurrentGoal < 0 || m_CurrentGoal >= num_goals)
			m_CurrentGoal = 0;

		return true;
	}
	else
	{
		m_CurrentGoal = -1;
		return false;
	}
}

bool CGoalDialog::ValidateCurrentItem()
{
	if(ValidateCurrentGoal())
	{
		int num_items = Level_goals.GoalGetNumItems(m_CurrentGoal);
		if(num_items > 0)
		{
			if(m_CurrentItem < 0 || m_CurrentItem >= num_items)
				m_CurrentItem = 0;

			return true;
		}
	}

	m_CurrentItem = -1;
	return false;
}

void make_goal_dialog_text(char *big_text_message)
{
	int i;
	int j;

	big_text_message[0] = '\0';

	int num_goals[2 * MAX_GOAL_LISTS];
	int goals[2 * MAX_GOAL_LISTS][MAX_LEVEL_GOALS];
	int p[2 * MAX_GOAL_LISTS][MAX_LEVEL_GOALS];

	for(i = 0; i < 2 * MAX_GOAL_LISTS; i++)
	{
		num_goals[i] = 0;
	}

	int n = Level_goals.GetNumGoals();

	for(i = 0; i < n; i++)
	{
		int priority;
		char list;
		int flags;

		Level_goals.GoalPriority(i, LO_GET_SPECIFIED, &priority);
		Level_goals.GoalGoalList(i, LO_GET_SPECIFIED, &list);
		Level_goals.GoalStatus(i, LO_GET_SPECIFIED, &flags, true);

		if(flags & LGF_SECONDARY_GOAL)
		{
			list += MAX_GOAL_LISTS;
		}

		int insert = num_goals[list];

		for(j = 0; j < num_goals[list]; j++)
		{
			if(priority < p[list][j])
			{
				insert = j;
				break;
			}
		}

		for(j = num_goals[list]; j > insert; j--)
		{
			goals[list][j] = goals[list][j - 1];
			p[list][j] = p[list][j - 1];
		}

		goals[list][insert] = i;
		p[list][insert] = priority;

		num_goals[list]++;
	}

	for(i = 0; i < 2 * MAX_GOAL_LISTS; i++)
	{
		strcat(big_text_message, "\r\n---------------------\r\n");

		if(i < MAX_GOAL_LISTS)
		{
			char cur[50];
			sprintf(cur, "Primary List %d\r\n", i);
			strcat(big_text_message, cur);
		}
		else
		{
			char cur[50];
			sprintf(cur, "Secondary List %d\r\n", i - MAX_GOAL_LISTS);
			strcat(big_text_message, cur);
		}

		strcat(big_text_message, "---------------------\r\n");

		for(j = 0; j < num_goals[i]; j++)
		{
			char name[41];
			char p_text[5];
			
			strcat(big_text_message, "   ");
			Level_goals.GoalGetName(goals[i][j], name, 40);

			int flags;
			Level_goals.GoalStatus(goals[i][j], LO_GET_SPECIFIED, &flags, true);
			if(flags & LGF_TELCOM_LISTS)
			{
				strcat(big_text_message, "(Obj) ");
			}
			strcat(big_text_message, name);
			sprintf(p_text, " (%d)", p[i][j]);
			strcat(big_text_message, p_text);
			strcat(big_text_message, "\r\n");
		}
	}
}

void CGoalDialog::EnableControls(bool enable)
{
	((CButton *)GetDlgItem(IDC_ADD_GOAL))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_DELETE_GOAL))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_NEXT_GOAL))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_PREV_GOAL))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_SELECT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_NAME))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_LOCOBJ_NAME))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_DESCRIPTION))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_PRIORITY))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_LIST))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_COMPLETED))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ENABLED))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_GB_KNOWS))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_LOC_BASED))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_OBJECTIVE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_SECONDARY))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_ACTIVATE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_ENTER))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_PICKUP_DESTROY))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_PLAYER_COLLIDE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_PLAYER_WEAPON))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_ADD))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_DELETE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_DALLAS))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_ROOM))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_SELECT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_TRIGGER))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_OBJECT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_NEXT_GOAL_ITEM))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_PREV_GOAL_ITEM))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_TRIGGER_COMBO))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_ROOM_COMBO))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GOAL_ITEM_OBJECT_COMBO))->EnableWindow(enable);
}

void CGoalDialog::UpdateDialog()
{
	int flags;
	int lgflags;
	int priority;
	char g_list;
	char name[256];
	char iname[256];
	char desc[2560];
	int num_goals;
	int num_items = 0;
	char big_text_message[MAX_LEVEL_GOALS*5*40];

	char item_type;
	int item_handle;

	if (theApp.m_pLevelWnd == NULL || Curroomp == NULL)
	{
//		EnableTabControls(this->m_hWnd,false,IDC_ADD_GOAL,-1);
		EnableControls(false);
		return;
	}
	else
	{
//		EnableTabControls(this->m_hWnd,true,IDC_ADD_GOAL,-1);
		EnableControls(true);
	}

	Level_goals.LGStatus(LO_GET_SPECIFIED, &lgflags);

	if(ValidateCurrentGoal())
	{
		num_goals = Level_goals.GetNumGoals();
		num_items = Level_goals.GoalGetNumItems(m_CurrentGoal);

		Level_goals.GoalStatus(m_CurrentGoal, LO_GET_SPECIFIED, &flags, true);
		Level_goals.GoalPriority(m_CurrentGoal, LO_GET_SPECIFIED, &priority);
		Level_goals.GoalGoalList(m_CurrentGoal, LO_GET_SPECIFIED, &g_list);

		Level_goals.GoalGetName(m_CurrentGoal, name, sizeof(name));
		Level_goals.GoalGetItemName(m_CurrentGoal, iname, sizeof(iname));
		Level_goals.GoalGetDesc(m_CurrentGoal, desc, sizeof(desc));

		make_goal_dialog_text(big_text_message);
	}
	else
	{
		num_goals = 0;
		num_items = 0;

		flags = 0;
		priority = 0;
		g_list = 0;

		strcpy(name, "No goals");
		strcpy(iname, "No goals");
		strcpy(desc, "No goals");

		strcpy(big_text_message, "No goals");
	}

	if(ValidateCurrentItem())
	{
		Level_goals.GoalItemInfo(m_CurrentGoal, m_CurrentItem, LO_GET_SPECIFIED, &item_type, &item_handle, NULL);
	}
	else
	{
		item_type = LIT_OBJECT;
		item_handle = OBJECT_HANDLE_NONE;
	}

	PrintToDlgItem(this, IDC_GOAL_NUMBER, "%d of %d", m_CurrentGoal + 1, num_goals);
	PrintToDlgItem(this, IDC_GOAL_ITEM_NUMBER, "%d of %d", m_CurrentItem + 1, num_items);
	PrintToDlgItem(this, IDC_GOAL_SUMMARY, big_text_message);

	PrintToDlgItem(this, IDC_GOAL_NAME, "%s", name);
	PrintToDlgItem(this, IDC_GOAL_LOCOBJ_NAME, "%s", iname);
	PrintToDlgItem(this, IDC_GOAL_DESCRIPTION, "%s", desc);

	CheckDlgButton(IDC_GOAL_ITEM_ROOM,item_type == LIT_INTERNAL_ROOM);
	CheckDlgButton(IDC_GOAL_ITEM_OBJECT,item_type == LIT_OBJECT);
	CheckDlgButton(IDC_GOAL_ITEM_TRIGGER,item_type == LIT_TRIGGER);

	CheckDlgButton(IDC_GOAL_SECONDARY,flags & LGF_SECONDARY_GOAL);
	CheckDlgButton(IDC_GOAL_ENABLED,flags & LGF_ENABLED);
	CheckDlgButton(IDC_GOAL_COMPLETED,flags & LGF_COMPLETED);
	CheckDlgButton(IDC_GOAL_OBJECTIVE,flags & LGF_TELCOM_LISTS);
	CheckDlgButton(IDC_GOAL_GB_KNOWS,flags & LGF_GB_DOESNT_KNOW_LOC);
	CheckDlgButton(IDC_GOAL_LOC_BASED,flags & LGF_NOT_LOC_BASED);

	CheckDlgButton(IDC_GOAL_ITEM_ACTIVATE,(flags & LGF_COMP_MASK) == LGF_COMP_ACTIVATE);
	CheckDlgButton(IDC_GOAL_ITEM_ENTER,(flags & LGF_COMP_MASK) == LGF_COMP_ENTER);
	CheckDlgButton(IDC_GOAL_ITEM_PICKUP_DESTROY,(flags & LGF_COMP_MASK) == LGF_COMP_DESTROY);
	CheckDlgButton(IDC_GOAL_ITEM_PLAYER_WEAPON,(flags & LGF_COMP_MASK) == LGF_COMP_PLAYER_WEAPON);
	CheckDlgButton(IDC_GOAL_ITEM_PLAYER_COLLIDE,(flags & LGF_COMP_MASK) == LGF_COMP_PLAYER);
	CheckDlgButton(IDC_GOAL_ITEM_DALLAS,(flags & LGF_COMP_MASK) == LGF_COMP_DALLAS);

	if(item_type == LIT_OBJECT)
		m_object_combo.SetSelected(item_handle);
	else
		m_object_combo.SetSelected(-1);

	if(item_type == LIT_INTERNAL_ROOM)
		m_room_combo.SetSelected(item_handle);
	else
		m_room_combo.SetSelected(-1);

	if(item_type == LIT_TRIGGER)
		m_trigger_combo.SetSelected(item_handle);
	else
		m_trigger_combo.SetSelected(-1);

	PrintToDlgItem(this, IDC_GOAL_PRIORITY, "%d", priority);
	PrintToDlgItem(this, IDC_GOAL_LIST, "%d", g_list);
}


BOOL CGoalDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_room_combo.Init(-1);
	m_trigger_combo.Init(-1);
	m_object_combo.Init(OBJ_NONE, OBJECT_HANDLE_NONE);
	UpdateDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CGoalDialog::OnKillfocusGoalList() 
{
	// TODO: Add your control notification handler code here
	int int_g_list;
	char g_list;
	char str[256];

	((CEdit *) GetDlgItem(IDC_GOAL_LIST))->GetWindowText(str,255);
	sscanf(str, "%d", &int_g_list);

	g_list = (char)int_g_list;

	Level_goals.GoalGoalList(m_CurrentGoal, LO_SET_SPECIFIED, &g_list);

	UpdateDialog();
}

void CGoalDialog::DoGoalFlagToggle(int flag)
{
	char op = LO_CLEAR_SPECIFIED;
	int flags;
	bool f_set;

	Level_goals.GoalStatus(m_CurrentGoal, LO_GET_SPECIFIED, &flags, true);
	f_set = ((flags & flag) == 0);

	if(f_set)
	{
		op = LO_SET_SPECIFIED;
	}

	Level_goals.GoalStatus(m_CurrentGoal, op, &flag, true);
	UpdateDialog();
}

void CGoalDialog::OnGoalCompleted() 
{
	// TODO: Add your control notification handler code here
	DoGoalFlagToggle(LGF_COMPLETED);
}

void CGoalDialog::OnGoalEnabled() 
{
	// TODO: Add your control notification handler code here
	DoGoalFlagToggle(LGF_ENABLED);
}

void CGoalDialog::OnGoalGbKnows() 
{
	// TODO: Add your control notification handler code here
	DoGoalFlagToggle(LGF_GB_DOESNT_KNOW_LOC);
}

void CGoalDialog::OnGoalLocBased() 
{
	// TODO: Add your control notification handler code here
	DoGoalFlagToggle(LGF_NOT_LOC_BASED);
}

void CGoalDialog::OnGoalObjective() 
{
	// TODO: Add your control notification handler code here
	DoGoalFlagToggle(LGF_TELCOM_LISTS);
}

void CGoalDialog::OnGoalSecondary() 
{
	// TODO: Add your control notification handler code here
	DoGoalFlagToggle(LGF_SECONDARY_GOAL);
}

void CGoalDialog::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	theApp.m_pGoalDlg = NULL;
	delete this;
}

void CGoalDialog::OnCancel() 
{
	// TODO: Add extra cleanup here
	DestroyWindow();
}

void CGoalDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	UpdateDialog();	
	// Do not call CDialog::OnPaint() for painting messages
}

void CGoalDialog::OnGoalItemActivate() 
{
	// TODO: Add your control notification handler code here
	int flags = LGF_COMP_MASK;
	Level_goals.GoalStatus(m_CurrentGoal, LO_CLEAR_SPECIFIED, &flags, true);

	flags = LGF_COMP_ACTIVATE;
	Level_goals.GoalStatus(m_CurrentGoal, LO_SET_SPECIFIED, &flags, true);
}

void CGoalDialog::OnGoalItemEnter() 
{
	// TODO: Add your control notification handler code here
	int flags = LGF_COMP_MASK;
	Level_goals.GoalStatus(m_CurrentGoal, LO_CLEAR_SPECIFIED, &flags, true);

	flags = LGF_COMP_ENTER;
	Level_goals.GoalStatus(m_CurrentGoal, LO_SET_SPECIFIED, &flags, true);
}

void CGoalDialog::OnGoalItemPickupDestroy() 
{
	// TODO: Add your control notification handler code here
	int flags = LGF_COMP_MASK;
	Level_goals.GoalStatus(m_CurrentGoal, LO_CLEAR_SPECIFIED, &flags, true);

	flags = LGF_COMP_DESTROY;
	Level_goals.GoalStatus(m_CurrentGoal, LO_SET_SPECIFIED, &flags, true);
}

void CGoalDialog::OnGoalItemPlayerCollide() 
{
	// TODO: Add your control notification handler code here
	int flags = LGF_COMP_MASK;
	Level_goals.GoalStatus(m_CurrentGoal, LO_CLEAR_SPECIFIED, &flags, true);

	flags = LGF_COMP_PLAYER;
	Level_goals.GoalStatus(m_CurrentGoal, LO_SET_SPECIFIED, &flags, true);
}

void CGoalDialog::OnGoalItemPlayerWeapon() 
{
	// TODO: Add your control notification handler code here
	int flags = LGF_COMP_MASK;
	Level_goals.GoalStatus(m_CurrentGoal, LO_CLEAR_SPECIFIED, &flags, true);

	flags = LGF_COMP_PLAYER_WEAPON;
	Level_goals.GoalStatus(m_CurrentGoal, LO_SET_SPECIFIED, &flags, true);
}

void CGoalDialog::OnGoalItemAdd() 
{
	// TODO: Add your control notification handler code here
	int item;

	item = Level_goals.GoalAddItem(m_CurrentGoal);
	if(item != -1)
	{
		m_CurrentItem = item;
	}
	else
	{
		OutrageMessageBox(MBOX_OK, "Sorry: No more items are allowed for this goal.");
	}

	UpdateDialog();
}

void CGoalDialog::OnGoalItemDelete() 
{
	// TODO: Add your control notification handler code here
	if(Level_goals.GoalGetNumItems(m_CurrentGoal))
	{
		Level_goals.GoalDeleteItem(m_CurrentGoal, m_CurrentItem);
		m_CurrentItem = 0;
	}

	UpdateDialog();
}

void CGoalDialog::OnGoalItemDallas() 
{
	// TODO: Add your control notification handler code here
	int flags = LGF_COMP_MASK;
	Level_goals.GoalStatus(m_CurrentGoal, LO_CLEAR_SPECIFIED, &flags, true);

	flags = LGF_COMP_DALLAS;
	Level_goals.GoalStatus(m_CurrentGoal, LO_SET_SPECIFIED, &flags, true);
}

void CGoalDialog::OnGoalItemRoom() 
{
	// TODO: Add your control notification handler code here
	char type = LIT_INTERNAL_ROOM;
	int handle = -1;

	Level_goals.GoalItemInfo(m_CurrentGoal, m_CurrentItem, LO_SET_SPECIFIED, &type, &handle, NULL);
	
	UpdateDialog();
}

void CGoalDialog::OnGoalItemSelect() 
{
	// TODO: Add your control notification handler code here
	int n;
	int num_items = Level_goals.GoalGetNumItems(m_CurrentGoal);

	if (InputNumber(&n,"Select Goal Item","Enter goal item number to select",this)) {

		if (n > num_items || n <= 0) {
			OutrageMessageBox("Invalid goal item number.");
			return;
		}

		m_CurrentItem = n-1;

		UpdateDialog();
	}
}

void CGoalDialog::OnGoalItemTrigger() 
{
	// TODO: Add your control notification handler code here
	char type = LIT_TRIGGER;
	int handle = -1;

	Level_goals.GoalItemInfo(m_CurrentGoal, m_CurrentItem, LO_SET_SPECIFIED, &type, &handle, NULL);
	
	UpdateDialog();
}

void CGoalDialog::OnGoalItemObject() 
{
	// TODO: Add your control notification handler code here
	char type = LIT_OBJECT;
	int handle = -1;

	Level_goals.GoalItemInfo(m_CurrentGoal, m_CurrentItem, LO_SET_SPECIFIED, &type, &handle, NULL);
	UpdateDialog();
}

void CGoalDialog::OnNextGoalItem() 
{
	// TODO: Add your control notification handler code here
	if(m_CurrentItem < Level_goals.GoalGetNumItems(m_CurrentGoal) - 1)
		m_CurrentItem++;
	else
		m_CurrentItem = 0;

	UpdateDialog();
}

void CGoalDialog::OnPrevGoalItem() 
{
	// TODO: Add your control notification handler code here
	if(m_CurrentItem > 0)
		m_CurrentItem--;
	else
		m_CurrentItem = Level_goals.GoalGetNumItems(m_CurrentGoal) - 1;

	UpdateDialog();
}

void CGoalDialog::OnSelendokGoalItemTriggerCombo() 
{
	// TODO: Add your control notification handler code here
	int handle = m_trigger_combo.GetSelected();

	Level_goals.GoalItemInfo(m_CurrentGoal, m_CurrentItem, LO_SET_SPECIFIED, NULL, &handle, NULL);

	UpdateDialog();
}

void CGoalDialog::OnSelendokGoalItemRoomCombo() 
{
	// TODO: Add your control notification handler code here
	int handle = m_room_combo.GetSelected();

	Level_goals.GoalItemInfo(m_CurrentGoal, m_CurrentItem, LO_SET_SPECIFIED, NULL, &handle, NULL);

	UpdateDialog();

}

void CGoalDialog::OnSelendokGoalItemObjectCombo() 
{
	// TODO: Add your control notification handler code here
	int handle = m_object_combo.GetSelected();

	Level_goals.GoalItemInfo(m_CurrentGoal, m_CurrentItem, LO_SET_SPECIFIED, NULL, &handle, NULL);

	UpdateDialog();
}

void CGoalDialog::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}
