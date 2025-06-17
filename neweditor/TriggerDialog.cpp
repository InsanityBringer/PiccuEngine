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
 // TriggerDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "globals.h"
#include "ned_Util.h"
#include "TriggerDialog.h"
#include "EditLineDialog.h"
#include "ned_Trigger.h"
#include "erooms.h"

#include "DallasMainDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTriggerDialog dialog


CTriggerDialog::CTriggerDialog(CWnd* pParent )
	: CDialog(CTriggerDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTriggerDialog)
	//}}AFX_DATA_INIT
}


void CTriggerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTriggerDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTriggerDialog, CDialog)
	//{{AFX_MSG_MAP(CTriggerDialog)
	ON_BN_CLICKED(IDC_TRIG_ADDTRIGGER, OnTrigAddTrigger)
	ON_BN_CLICKED(IDC_TRIG_DELETE, OnTrigDelete)
	ON_BN_CLICKED(IDC_TRIG_EDITNAME, OnTrigEditName)
	ON_BN_CLICKED(IDC_TRIG_EDITSCRIPT, OnTrigEditScript)
	ON_BN_CLICKED(IDC_TRIG_CLUTTER, OnTrigClutter)
	ON_BN_CLICKED(IDC_TRIG_ONESHOT, OnTrigOneShot)
	ON_BN_CLICKED(IDC_TRIG_PLAYER, OnTrigPlayer)
	ON_BN_CLICKED(IDC_TRIG_PLAYERWEAPON, OnTrigPlayerWeapon)
	ON_BN_CLICKED(IDC_TRIG_ROBOT, OnTrigRobot)
	ON_BN_CLICKED(IDC_TRIG_ROBOTWEAPON, OnTrigRobotWeapon)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_TRIG_PREVMINE, OnTrigPrevMine)
	ON_BN_CLICKED(IDC_TRIG_PREVROOM, OnTrigPrevRoom)
	ON_BN_CLICKED(IDC_TRIG_SELECT, OnTrigSelect)
	ON_BN_CLICKED(IDC_TRIG_NEXTROOM, OnTrigNextRoom)
	ON_BN_CLICKED(IDC_TRIG_NEXTMINE, OnTrigNextMine)
	ON_BN_CLICKED(IDC_TRIG_FLOATING, OnTrigFloating)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTriggerDialog message handlers

void CTriggerDialog::OnTrigAddTrigger() 
{
	// TODO: Add your control notification handler code here
	room *rp = Curroomp;
	int facenum, activator;
	char name[TRIG_NAME_LEN+1] = "";

	// Add trigger to face
	// Add trigger to portal or to face
	if (Curportal >= 0)
	{
		facenum = rp->portals[Curportal].portal_face;
		ASSERT(! (rp->faces[facenum].flags & FF_HAS_TRIGGER));
		activator = AF_PLAYER;
	}
	else
	{
		facenum = Curface;
		ASSERT(! (rp->faces[facenum].flags & FF_HAS_TRIGGER));
		activator = (rp->faces[facenum].portal_num != -1) ? AF_PLAYER : AF_PLAYER_WEAPON;
	}
	if (! InputString(name,sizeof(name),"Trigger Name","Enter a name for this trigger:"))
		return;
	Current_trigger = AddTrigger(name,ROOMNUM(rp),facenum,activator,NULL);

	// Compute the room shell so we can determine whether the face is non-shell, 
	// and thus allow the trigger to be floating
	if ((rp->num_portals > 0) && !(rp->flags & RF_EXTERNAL))
	{
		int shell_errors = ComputeRoomShell(rp);
		if (shell_errors)
			OutrageMessageBox("The current room has a bad shell. You will not be able to make this trigger a floating trigger until the shell is repaired.");
	}

	UpdateDialog();
}

void CTriggerDialog::OnTrigDelete() 
{
	// TODO: Add your control notification handler code here
	ASSERT(Current_trigger != -1);

	if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to delete the current trigger?") != IDYES)
		return;

	trigger *tp = &Triggers[Current_trigger];
	room *rp = &Rooms[tp->roomnum];
	int facenum = tp->facenum;

	//Delete the trigger
	DeleteTrigger(Current_trigger);

	if (Current_trigger == Num_triggers)
		Current_trigger--;

	World_changed = 1;

	UpdateDialog();
}

void CTriggerDialog::OnTrigEditName() 
{
	// TODO: Add your control notification handler code here
	ASSERT(Current_trigger != -1);

	trigger *tp = &Triggers[Current_trigger];

	char name[TRIG_NAME_LEN+1];

	strcpy(name,tp->name);

try_again:;
	if (! InputString(name,sizeof(name),"Trigger Name","Enter a new name:"))
		return;

	if (StripLeadingTrailingSpaces(name))
		EditorMessageBox("Note: Leading and/or trailing spaces have been removed from this name (\"%s\")",name);

	int n = FindTriggerName(name); // osipf_FindTriggerName
	if ((n != -1) && (n != Current_trigger)) {
		EditorMessageBox("Trigger %d already has this name.",n);
		goto try_again;
	}

	strcpy(tp->name,name);

	UpdateDialog();
}

void CTriggerDialog::OnTrigEditScript() 
{
	// TODO: Add your control notification handler code here
	mprintf((0,"Edit script for trigger %d\n",Current_trigger));
	
	if(Level_name.IsEmpty()) {
		AfxMessageBox("You must give your level a filename before opening Dallas.",MB_OK);
		return;
	}

	// Make sure Dallas is open
	if(theApp.m_DallasModelessDlgPtr==NULL) {
		theApp.m_DallasModelessDlgPtr = new CDallasMainDlg;
		theApp.m_DallasModelessDlgPtr->Create(IDD_DALLAS_MAIN_DIALOG,this);
	  	theApp.m_DallasModelessDlgPtr->ShowWindow(SW_SHOW);
	}
	else
		theApp.m_DallasModelessDlgPtr->ShowWindow(SW_RESTORE);

	//make sure a trigger was selected
	if(Current_trigger==-1)
		return;

	// Tell Dallas to add a new script with this trigger as the owner
	theApp.m_DallasModelessDlgPtr->m_ScriptOwnerType=TRIGGER_TYPE;
	theApp.m_DallasModelessDlgPtr->m_ScriptOwnerHandle=Current_trigger;
	theApp.m_DallasModelessDlgPtr->PostMessage(WM_HIGHLIGHT_SCRIPTS);
}

void CTriggerDialog::OnTrigClutter() 
{
	// TODO: Add your control notification handler code here
	trigger *tp = &Triggers[Current_trigger];

	if (IsDlgButtonChecked(IDC_TRIG_CLUTTER))
		tp->activator |= AF_CLUTTER;
	else
		tp->activator &= ~AF_CLUTTER;

	World_changed = 1;
}

void CTriggerDialog::OnTrigOneShot() 
{
	// TODO: Add your control notification handler code here
	trigger *tp = &Triggers[Current_trigger];

	if (IsDlgButtonChecked(IDC_TRIG_ONESHOT))
		tp->flags |= TF_ONESHOT;
	else
		tp->flags &= ~TF_ONESHOT;

	World_changed = 1;
}

void CTriggerDialog::OnTrigPlayer() 
{
	// TODO: Add your control notification handler code here
	trigger *tp = &Triggers[Current_trigger];

	if (IsDlgButtonChecked(IDC_TRIG_PLAYER))
		tp->activator |= AF_PLAYER;
	else
		tp->activator &= ~AF_PLAYER;

	World_changed = 1;
}

void CTriggerDialog::OnTrigPlayerWeapon() 
{
	// TODO: Add your control notification handler code here
	trigger *tp = &Triggers[Current_trigger];

	if (IsDlgButtonChecked(IDC_TRIG_PLAYERWEAPON))
		tp->activator |= AF_PLAYER_WEAPON;
	else
		tp->activator &= ~AF_PLAYER_WEAPON;

	World_changed = 1;
}

void CTriggerDialog::OnTrigRobot() 
{
	// TODO: Add your control notification handler code here
	trigger *tp = &Triggers[Current_trigger];

	if (IsDlgButtonChecked(IDC_TRIG_ROBOT))
		tp->activator |= AF_ROBOT;
	else
		tp->activator &= ~AF_ROBOT;

	World_changed = 1;
}

void CTriggerDialog::OnTrigRobotWeapon() 
{
	// TODO: Add your control notification handler code here
	trigger *tp = &Triggers[Current_trigger];

	if (IsDlgButtonChecked(IDC_TRIG_ROBOTWEAPON))
		tp->activator |= AF_ROBOT_WEAPON;
	else
		tp->activator &= ~AF_ROBOT_WEAPON;

	World_changed = 1;
}

void CTriggerDialog::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	theApp.m_pTriggerDlg = NULL;
	delete this;
}

void CTriggerDialog::OnCancel() 
{
	// TODO: Add extra cleanup here
	DestroyWindow();
}

void CTriggerDialog::EnableControls(bool enable)
{
	((CButton *)GetDlgItem(IDC_TRIG_NEXTMINE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_PREVMINE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_SELECT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_ADDTRIGGER))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_DELETE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_EDITSCRIPT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_EDITNAME))->EnableWindow(enable);
  	((CButton *)GetDlgItem(IDC_TRIG_PLAYER))->EnableWindow(enable);
  	((CButton *)GetDlgItem(IDC_TRIG_PLAYERWEAPON))->EnableWindow(enable);
  	((CButton *)GetDlgItem(IDC_TRIG_ROBOT))->EnableWindow(enable);
  	((CButton *)GetDlgItem(IDC_TRIG_ROBOTWEAPON))->EnableWindow(enable);
  	((CButton *)GetDlgItem(IDC_TRIG_CLUTTER))->EnableWindow(enable);
  	((CButton *)GetDlgItem(IDC_TRIG_ONESHOT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_NEXTROOM))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_PREVROOM))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_TRIG_FLOATING))->EnableWindow(enable);
}

trigger *FindTrigger(int roomnum,int facenum);

void CTriggerDialog::UpdateDialog()
{
	if (theApp.m_pLevelWnd == NULL || Curroomp == NULL || Curface == -1)
	{
//		EnableTabControls(this->m_hWnd,false,IDC_TRIG_CURFACE,-1);
		EnableControls(false);
		return;
	}
	else
	{
//		EnableTabControls(this->m_hWnd,true,IDC_TRIG_CURFACE,-1);
		EnableControls(true);
	}

	bool portal = 0,floating = 0;
	static int last_curroom = -1, last_curface = -1, last_curportal = -1;

	if (Current_trigger >= Num_triggers)
		Current_trigger = Num_triggers - 1;

	//If current room/face/portal change, select current trigger from current room/face/portal
	if ((ROOMNUM(Curroomp) != last_curroom) || (Curface != last_curface)) {
			if (Curroomp->faces[Curface].flags & FF_HAS_TRIGGER)
				Current_trigger = (FindTrigger(ROOMNUM(Curroomp),Curface) - Triggers);
	}
	else if ((Curportal != -1) && (Curportal != last_curportal))
		if (Curroomp->portals[Curportal].flags & FF_HAS_TRIGGER)
			Current_trigger = (FindTrigger(ROOMNUM(Curroomp),Curroomp->portals[Curportal].portal_face) - Triggers);
	last_curroom = ROOMNUM(Curroomp); last_curface = Curface, last_curportal = Curportal;

	//Enable/Disable some buttons, etc.
	((CButton *)GetDlgItem(IDC_TRIG_NEXTMINE))->EnableWindow(Num_triggers > 1);
	((CButton *)GetDlgItem(IDC_TRIG_PREVMINE))->EnableWindow(Num_triggers > 1);
	((CButton *)GetDlgItem(IDC_TRIG_SELECT))->EnableWindow(Num_triggers > 0);
	((CButton *)GetDlgItem(IDC_TRIG_ADDTRIGGER))->EnableWindow(! (Curroomp->faces[Curface].flags & FF_HAS_TRIGGER));
	((CButton *)GetDlgItem(IDC_TRIG_DELETE))->EnableWindow(Current_trigger != -1);
	((CButton *)GetDlgItem(IDC_TRIG_EDITSCRIPT))->EnableWindow(Current_trigger != -1);
	((CButton *)GetDlgItem(IDC_TRIG_EDITNAME))->EnableWindow(Current_trigger != -1);
  	((CButton *)GetDlgItem(IDC_TRIG_PLAYER))->EnableWindow(Current_trigger != -1);
  	((CButton *)GetDlgItem(IDC_TRIG_PLAYERWEAPON))->EnableWindow(Current_trigger != -1);
  	((CButton *)GetDlgItem(IDC_TRIG_ROBOT))->EnableWindow(Current_trigger != -1);
  	((CButton *)GetDlgItem(IDC_TRIG_ROBOTWEAPON))->EnableWindow(Current_trigger != -1);
  	((CButton *)GetDlgItem(IDC_TRIG_CLUTTER))->EnableWindow(Current_trigger != -1);
  	((CButton *)GetDlgItem(IDC_TRIG_ONESHOT))->EnableWindow(Current_trigger != -1);
  	((CButton *)GetDlgItem(IDC_TRIG_FLOATING))->EnableWindow(Current_trigger != -1);
	int n = FindNextTrigInRoom(ROOMNUM(Curroomp),Current_trigger);
	((CButton *)GetDlgItem(IDC_TRIG_NEXTROOM))->EnableWindow((n != -1) && (n != Current_trigger));
	((CButton *)GetDlgItem(IDC_TRIG_PREVROOM))->EnableWindow((n != -1) && (n != Current_trigger));

	//Update current trigger info
	if (Current_trigger > -1) {
		trigger *tp = &Triggers[Current_trigger];
		room *rp = &Rooms[tp->roomnum];
		face *fp = &rp->faces[tp->facenum];

		portal = (fp->portal_num != -1);
		floating = ((fp->flags & FF_FLOATING_TRIG) != 0);
		ASSERT(! (portal && floating));		//Shouldn't be both portal and floating

		PrintToDlgItem(this,IDC_TRIG_NAME,"Name:\t %s",tp->name[0]?tp->name:"<none>");
		PrintToDlgItem(this,IDC_TRIG_NUMBER,"Number:\t %d",Current_trigger);
		PrintToDlgItem(this,IDC_TRIG_TYPE,"Type:\t %s",portal?"PORTAL":(floating?"FLOATING":"FACE"));
		PrintToDlgItem(this,IDC_TRIG_ROOM,"Room:\t%d",tp->roomnum);
		PrintToDlgItem(this,IDC_TRIG_FACE,"Face:\t%d",tp->facenum);
		PrintToDlgItem(this,IDC_TRIG_PORTAL,(portal)?"Portal:\t%d":"",fp->portal_num);

		CheckDlgButton(IDC_TRIG_PLAYER,tp->activator & AF_PLAYER);
		CheckDlgButton(IDC_TRIG_PLAYERWEAPON,tp->activator & AF_PLAYER_WEAPON);
		CheckDlgButton(IDC_TRIG_ROBOTWEAPON,tp->activator & AF_ROBOT_WEAPON);
		CheckDlgButton(IDC_TRIG_ROBOT,tp->activator & AF_ROBOT);
		CheckDlgButton(IDC_TRIG_CLUTTER,tp->activator & AF_CLUTTER);

		CheckDlgButton(IDC_TRIG_ONESHOT,tp->flags & TF_ONESHOT);
		CheckDlgButton(IDC_TRIG_FLOATING,fp->flags & FF_FLOATING_TRIG);
	 	((CButton *)GetDlgItem(IDC_TRIG_FLOATING))->EnableWindow((fp->portal_num == -1) && (fp->flags & FF_NOT_SHELL));
	}
	else {
		PrintToDlgItem(this,IDC_TRIG_NAME,"Name:");
		PrintToDlgItem(this,IDC_TRIG_NUMBER,"Number:");
		PrintToDlgItem(this,IDC_TRIG_TYPE,"Type:");
		PrintToDlgItem(this,IDC_TRIG_ROOM,"Room:");
		PrintToDlgItem(this,IDC_TRIG_FACE,"Face:");
		PrintToDlgItem(this,IDC_TRIG_PORTAL,"Portal:");

		CheckDlgButton(IDC_TRIG_PLAYER,0);
		CheckDlgButton(IDC_TRIG_PLAYERWEAPON,0);
		CheckDlgButton(IDC_TRIG_ROBOTWEAPON,0);
		CheckDlgButton(IDC_TRIG_ROBOT,0);

		CheckDlgButton(IDC_TRIG_ONESHOT,0);
	}

}

void CTriggerDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	UpdateDialog();	
	// Do not call CDialog::OnPaint() for painting messages
}

BOOL CTriggerDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	//Check current trigger
	if (Current_trigger >= Num_triggers)
		Current_trigger = Num_triggers - 1;

	// Change the current room to that of the trigger (might be annoying, though it fixes an irksome bug)
	if (theApp.m_pLevelWnd != NULL && Curroomp != NULL && Current_trigger != -1)
		SetCurroomFromTrigger();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTriggerDialog::OnTrigPrevMine() 
{
	// TODO: Add your control notification handler code here
	ASSERT(Current_trigger != -1);

	if (--Current_trigger < 0)
		Current_trigger += Num_triggers;

	SetCurroomFromTrigger();

	UpdateDialog();
}

void CTriggerDialog::OnTrigPrevRoom() 
{
	// TODO: Add your control notification handler code here
	int n = FindPrevTrigInRoom(ROOMNUM(Curroomp),Current_trigger);

	ASSERT(n != -1);

	Current_trigger = n;

	SetCurroomFromTrigger();

	UpdateDialog();
}

void CTriggerDialog::OnTrigSelect() 
{
	// TODO: Add your control notification handler code here
	int n = Current_trigger;

	if (InputNumber(&n,"Select Trigger","Enter trigger number to select",this)) {

		if (n >= Num_triggers) {
			OutrageMessageBox("Invalid trigger number.");
			return;
		}

		Current_trigger = n;

		SetCurroomFromTrigger();

		UpdateDialog();
	}
}

void CTriggerDialog::OnTrigNextRoom() 
{
	// TODO: Add your control notification handler code here
	int n = FindNextTrigInRoom(ROOMNUM(Curroomp),Current_trigger);

	ASSERT(n != -1);

	Current_trigger = n;

	SetCurroomFromTrigger();

	UpdateDialog();
}

void CTriggerDialog::OnTrigNextMine() 
{
	// TODO: Add your control notification handler code here
	ASSERT(Current_trigger != -1);

	Current_trigger = (Current_trigger+1) % Num_triggers;

	SetCurroomFromTrigger();

	UpdateDialog();
}

void CTriggerDialog::SetCurroomFromTrigger()
{
	ASSERT(Current_trigger != -1);

	if (ROOMNUM(Curroomp) != Triggers[Current_trigger].roomnum) {

		theApp.m_pLevelWnd->SetPrim(&Rooms[Triggers[Current_trigger].roomnum],0,-1,0,0);
		ASSERT(theApp.m_pLevelWnd != NULL);
		if (theApp.m_pLevelWnd->m_bAutoCenter)
			theApp.m_pLevelWnd->CenterRoom(Curroomp);
		if (theApp.m_pRoomFrm != NULL)
			theApp.m_pRoomFrm->CenterRoom();
	}

	Curface = Triggers[Current_trigger].facenum;

	// Update the current face/texture displays
	Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(Curroomp), Curface);

	if (Curroomp->faces[Curface].portal_num != -1)
		Curportal = Curroomp->faces[Curface].portal_num;

	State_changed = 1;
}

int FindPrevTrigInRoom(int roomnum,int start)
{
	if (Triggers[start].roomnum != roomnum)
		start = 0;
	else
		start--;

	for (int i=Num_triggers;i>0;i--) {
		int n = (start + i) % Num_triggers;

		if (Triggers[n].roomnum == roomnum)
			return n;
	}

	//Couldn't find one
	return -1;
}

int FindNextTrigInRoom(int roomnum,int start)
{
	if ((start == -1) || (Triggers[start].roomnum != roomnum))
		start = 0;
	else
		start++;

	for (int i=0;i<Num_triggers;i++) {
		int n = (start + i) % Num_triggers;

		if (Triggers[n].roomnum == roomnum)
			return n;
	}

	//Couldn't find one
	return -1;
}



void CTriggerDialog::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}

void CTriggerDialog::OnTrigFloating() 
{
	// TODO: Add your control notification handler code here
	trigger *tp = &Triggers[Current_trigger];
	room *rp = &Rooms[tp->roomnum];

	if (IsDlgButtonChecked(IDC_TRIG_FLOATING))
		rp->faces[tp->facenum].flags |= FF_FLOATING_TRIG;
	else
		rp->faces[tp->facenum].flags &= ~FF_FLOATING_TRIG;

	World_changed = 1;

	UpdateDialog();
}

