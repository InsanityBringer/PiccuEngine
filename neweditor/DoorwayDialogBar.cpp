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
 // DoorwayDialogBar.cpp : implementation file
//

#include "stdafx.h"
#include "globals.h"
#include "ned_Util.h"
#include "neweditor.h"
#include "DoorwayDialogBar.h"
#include "EditLineDialog.h"
#include "ProgressDialog.h"
#include "mono.h"
#include "descent.h"
#include "ned_Door.h"
#include "ned_Tablefile.h"
#include "doorway.h"
#include "polymodel.h"
#include "../editor/Edoors.h"
#include "../editor/HRoom.h"
#include "ned_Rend.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int Current_door_room=-1;

/////////////////////////////////////////////////////////////////////////////
// CDoorwayDialogBar dialog

inline bool IsDoorValid(int id)
{
	if(id<0 || id>=MAX_DOORS || !Doors[id].used)
		return false;
	return true;
}

void ldoor_init(void);


CDoorwayDialogBar::CDoorwayDialogBar()
{
	//{{AFX_DATA_INIT(CDoorwayDialogBar)
	//}}AFX_DATA_INIT
	m_CurrentDoor = -1;
	m_CurrentDoorway = -1;
	m_CurrentDoorwayChanged = true;
	m_CurrentDoorChanged = true;
}


void CDoorwayDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDoorwayDialogBar)
	DDX_Control(pDX, IDC_DOORS, m_Combo);
	DDX_Control(pDX, IDC_CD_VIEW, m_CDView);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDoorwayDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CDoorwayDialogBar)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_DOORS, OnSelchangeDoors)
	ON_BN_CLICKED(IDC_PLACE_DOOR, OnPlaceDoor)
	ON_BN_CLICKED(IDC_ATTACH_DOOR, OnAttachDoor)
	ON_UPDATE_COMMAND_UI(IDC_PLACE_DOOR,OnControlUpdate)
	ON_BN_CLICKED(IDC_CDW_DOORNAME, OnCdwDoorName)
	ON_BN_CLICKED(IDC_KEY1, OnKey1)
	ON_BN_CLICKED(IDC_KEY2, OnKey2)
	ON_BN_CLICKED(IDC_KEY3, OnKey3)
	ON_BN_CLICKED(IDC_KEY4, OnKey4)
	ON_BN_CLICKED(IDC_KEY5, OnKey5)
	ON_BN_CLICKED(IDC_KEY6, OnKey6)
	ON_BN_CLICKED(IDC_KEY7, OnKey7)
	ON_BN_CLICKED(IDC_KEY8, OnKey8)
	ON_BN_CLICKED(IDC_LOCKED, OnLocked)
	ON_BN_CLICKED(IDC_NEEDSALL, OnNeedsAll)
	ON_BN_CLICKED(IDC_NEEDSONE, OnNeedsOne)
	ON_BN_CLICKED(IDC_AUTOCLOSE, OnAutoClose)
	ON_BN_CLICKED(IDC_GB_IGNORE, OnGBIgnore)
	ON_EN_KILLFOCUS(IDC_INITIALPOS, OnKillFocusInitialPos)
	ON_EN_KILLFOCUS(IDC_HITPOINTS, OnKillFocusHitPoints)
	ON_BN_CLICKED(IDC_PREV_DOORWAY, OnPrevDoorway)
	ON_BN_CLICKED(IDC_NEXT_DOORWAY, OnNextDoorway)
	ON_UPDATE_COMMAND_UI(IDC_ATTACH_DOOR,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_CDW_DOORNAME,OnControlUpdate)
	ON_BN_CLICKED(IDC_DELETE_DOORWAY, OnDeleteDoor)
	ON_UPDATE_COMMAND_UI(IDC_DELETE_DOORWAY,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PREV_DOORWAY,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_NEXT_DOORWAY,OnControlUpdate)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

CDoorwayDialogBar *dlgDoorwayDialogBar = NULL;


/////////////////////////////////////////////////////////////////////////////
// CDoorwayDialogBar message handlers

LONG CDoorwayDialogBar::OnInitDialog ( UINT wParam, LONG lParam)
{
	dlgDoorwayDialogBar = this;
	ldoor_init();

	HandleInitDialog(wParam, lParam);
	if (!UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
		return FALSE;
	}

	InitKeyPad();

	//Setup update timers
	if(!SetTimer(TIMER_DOORWAYS,100,NULL))
	{
		mprintf((0,"Unable to create timer\n"));
		Int3();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CDoorwayDialogBar::PreTranslateMessage(MSG* pMsg) 
{
	return CDialogBar::PreTranslateMessage(pMsg);
}

//this is what enables your controls:
void CDoorwayDialogBar::OnControlUpdate(CCmdUI* pCmdUI)
{
	bool bEnable = false;
	int num_doors;

	if (theApp.m_pLevelWnd != NULL)
		bEnable = true;

	switch (pCmdUI->m_nID)
	{
	case IDC_CDW_DOORNAME:
		if ( m_CurrentDoorway<0 || m_CurrentDoorway>Highest_object_index || !IsDoorValid(Objects[m_CurrentDoorway].id) )
			bEnable = false;
		break;

	case IDC_PREV_DOORWAY:
	case IDC_NEXT_DOORWAY:
	case IDC_DELETE_DOORWAY:
		num_doors = GetDoorwayCount();
		if ( !num_doors || (pCmdUI->m_nID == IDC_DELETE_DOORWAY && !IsDoorValid(Objects[m_CurrentDoorway].id)) )
			bEnable = false;
		break;

	case IDC_PLACE_DOOR:
		if (!IsDoorValid(m_CurrentDoor))
			bEnable = false;
		break;

	case IDC_ATTACH_DOOR:
		if (Placed_door == -1)
			bEnable = false;
		break;
	}

	pCmdUI->Enable(bEnable);
}

extern int GetDoorImage(int handle);
extern void DrawItemModel(int x,int y,RendHandle& handle,int model_num,bool draw_large);

void CDoorwayDialogBar::PaintPreviewArea(CWnd *pWnd, RendHandle& handle)
{
	int door_id,model_num;

	ASSERT_VALID(pWnd);

	COLORREF bk_col = ::GetSysColor(COLOR_BTNFACE);

	//Get placehold coords
	CRect rect;
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);
	int width,height,bm_handle = -1;
	width = rect.Width();
	height = rect.Height();

	//Draw here
	/*grSurface *ds = Editor_state.GetDesktopSurface();
	if(!ds)
		return;

	//setup the drawing area
	grHardwareSurface surface;
	ds->attach_to_window((unsigned)pWnd->m_hWnd);
	surface.create(width,height,BPP_16);*/

	rend_MakeCurrent(handle);

	//render here
	if(pWnd==&m_CDView)
	{
		//Current Door View
		if(IsDoorValid(m_CurrentDoor))
		{
			door_id = m_CurrentDoor;
			model_num = GetDoorImage(door_id);

			DrawItemModel(0,0,handle,model_num,true);
		}
		else
		{
			rend_ClearScreen(0);
			//surface.clear(bk_col);
			//ds->blt(0, 0, &surface);
		}
	}else
	{
		Int3();
	}

	//free up
	//surface.free();
	//ds->attach_to_window(NULL);
}

void CDoorwayDialogBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
		
	//PaintPreviewArea(&m_CDView);
	
	// Do not call CDialogBar::OnPaint() for painting messages
}

void CDoorwayDialogBar::RedrawPreviewArea(CWnd *pWnd)
{
	ASSERT_VALID(pWnd);

	//Get Placeholder coordinates
	CRect rect;
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);

	//force update
	InvalidateRect(rect,false);	
}

void CDoorwayDialogBar::RedrawArea()
{
	RedrawPreviewArea(&m_CDView);
	UpdateWindow();
}

void CDoorwayDialogBar::InitKeyPad(void)
{
	for(int i=0;i<MAX_DOORS;i++)
	{
		AddDoorToList(i);
	}
}


void _doordlg_destroydoorlist(CDoorwayDialogBar *wnd)
{
	//remove all string from it
	int id,count = wnd->m_Combo.GetCount();
	CString buffer;

	//select none
	wnd->m_Combo.SetCurSel(-1);
	wnd->ForceSelChange();

	for(int i=0;i<count;i++)
	{
		wnd->m_Combo.GetLBText(0,buffer);
		id = ned_FindDoor(buffer.GetBuffer(0));
		ASSERT(id!=-1);

		wnd->RemoveDoorFromList(id);
	}

}

void CDoorwayDialogBar::ForceSelChange() 
{
	OnSelchangeDoors();
}

void CDoorwayDialogBar::OnDestroy() 
{
	_doordlg_destroydoorlist(this);

	CDialogBar::OnDestroy();

	dlgDoorwayDialogBar = NULL;
}

// ======================
// GetCurrentDoor
// ======================
//
// Gets the current door id
int CDoorwayDialogBar::GetCurrentDoor(void)
{
	return m_CurrentDoor;
}


// ======================
// SetCurrentDoor
// ======================
//
// Marks a door the current door to work with in the editor
void CDoorwayDialogBar::SetCurrentDoor(int Door)
{
	if(Door<-1 || Door>=MAX_DOORS)
	{
		Int3();
		return;
	}

	if(Door!=-1 && !Doors[Door].used)
	{
		Int3();
		return;
	}

	if(IsDoorValid(m_CurrentDoor))
	{
		ned_MarkDoorInUse(m_CurrentDoor,false);
		m_CurrentDoor = -1;
	}

	m_CurrentDoor = Door;
	m_CurrentDoorChanged = true;

	if(IsDoorValid(m_CurrentDoor))
	{
		ned_MarkDoorInUse(m_CurrentDoor,true);
	}	
}

// ======================
// SetCurrentDoorway
// ======================
//
// Marks the current doorway for the Doorway Dialog
void CDoorwayDialogBar::SetCurrentDoorway(int Doorway)
{
	object *oldobjp;

	if( (Doorway<-1 || Doorway>Highest_object_index) )
	{
		Int3();//bad door
		return;
	}

	//mark old doorway unused here
	if (m_CurrentDoorway != -1)
	{
		oldobjp = &Objects[m_CurrentDoorway];
		ASSERT(oldobjp->type == OBJ_DOOR);
//		ned_MarkDoorInUse(oldobjp->id,false);
		m_CurrentDoorway = -1;
	}

	m_CurrentDoorway = Doorway;
	m_CurrentDoorwayChanged = true;

	if (m_CurrentDoorway != -1)
	{
		//set new doorway used here
		oldobjp = &Objects[m_CurrentDoorway];
		ASSERT(oldobjp->type == OBJ_DOOR);
//		ned_MarkDoorInUse(oldobjp->id,true);
	}
}

short Level_doors_in_use[MAX_DOORS];
void ldoor_init(void)
{
	for(int i=0;i<MAX_DOORS;i++)
	{
		Level_doors_in_use[i] = 0;
	}
}

// =========================
// LevelDoorIncrementDoorway
// =========================
//
// Call this function whenever a doorway is added to a level
// _EVERYTIME_ it is added to a level.  Make sure it is called on level load
void LevelDoorIncrementDoorway(int doornum)
{
	if(doornum<0 || doornum>=MAX_DOORS || !Doors[doornum].used)
	{
		//bad door coming in
		Int3();
		return;
	}

	Level_doors_in_use[doornum]++;
	ned_MarkDoorInUse(doornum,true);

	if(Level_doors_in_use[doornum]==1)
	{
		//our first add
		ASSERT_VALID(dlgDoorwayDialogBar);
	}
}

// ===========================
// LevelDoorDecrementDoorway
// ===========================
//
// Call this function when a doorway is being removed from the level.
void LevelDoorDecrementDoorway(int doornum)
{
	if(doornum<0 || doornum>=MAX_DOORS || !Doors[doornum].used)
	{
		//bad door coming in
		Int3();
		return;
	}

	Level_doors_in_use[doornum]--;
	ASSERT(Level_doors_in_use[doornum]>=0);

	ned_MarkDoorInUse(doornum,false);

	if(Level_doors_in_use[doornum]==0)
	{
		//it's gone from the level...delete
		ASSERT_VALID(dlgDoorwayDialogBar);
	}
}

// ==========================
// LevelDoorPurgeAllDoorways
// ==========================
//
// Call this when a level is being unloaded to purge all doorways associated with it
void LevelDoorPurgeAllDoorways(void)
{
	int j,size;
	for(int i=0;i<MAX_DOORS;i++)
	{
		size = Level_doors_in_use[i];
		for(j=0;j<size;j++)
		{
			ned_MarkDoorInUse(i,false);
		}

		Level_doors_in_use[i] = 0;

		if(size>0)
		{
			ASSERT_VALID(dlgDoorwayDialogBar);
		}
	}
}

// ==========================
// LevelDoorInitLevel
// ==========================
//
// Call this as soon as a level is loaded, it will go through the level and set it up.
void LevelDoorInitLevel(void)
{
	int rm;
	room *rp;
	object *objp;

	for(rm = 0; rm < MAX_ROOMS; rm++)
	{
		rp = &Rooms[rm];
		if (rp->used && rp->objects != -1)
		{
			objp = &Objects[rp->objects];
			if (objp->type == OBJ_DOOR)
				LevelDoorIncrementDoorway(objp->id);
			while (objp->next != -1)
			{
				objp = &Objects[objp->next];
				if (objp->type == OBJ_DOOR)
					LevelDoorIncrementDoorway(objp->id);
			}
		}
	}
}

void CDoorwayDialogBar::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case TIMER_DOORWAYS:
		{
			if(m_CurrentDoorChanged)
			{
				//force update the current door
				//change the necessary text
				char doorname[PAGENAME_LEN];

				if(IsDoorValid(m_CurrentDoor))
					strcpy(doorname,Doors[m_CurrentDoor].name);
				else
					strcpy(doorname,"NONE");
				CWnd *wnd;
				wnd = GetDlgItem(IDC_CD_DOORNAME);
				wnd->SetWindowText(doorname);

				RedrawPreviewArea(&m_CDView);
				UpdateWindow();
				m_CurrentDoorChanged = false;
			}

			//force update the current doorway
			short door_to_use = -1;

			if(m_CurrentDoorway>=0 && m_CurrentDoorway<=Highest_object_index)
			{
				door_to_use = Objects[m_CurrentDoorway].id;
			}

			if(m_CurrentDoorwayChanged)
			{
				//change the necessary text
				char buffer[PAGENAME_LEN+OBJ_NAME_LEN+1] = "";
				bool enabled;
				CWnd *wnd;

				if(IsDoorValid(door_to_use))
				{
					if (Objects[m_CurrentDoorway].name && strcmp(Objects[m_CurrentDoorway].name, ""))
						sprintf(buffer,"%s \"%s\"",Doors[door_to_use].name,Objects[m_CurrentDoorway].name);
					else
						strcpy(buffer,Doors[door_to_use].name);
					enabled = true;
				}
				else
				{
					strcpy(buffer,"NONE");
					enabled = false;
				}

				wnd = GetDlgItem(IDC_CDW_DOORNAME);
				wnd->SetWindowText(buffer);
				wnd = GetDlgItem(IDC_CDW_DOORNAME);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_KEY1);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_KEY2);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_KEY3);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_KEY4);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_KEY5);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_KEY6);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_KEY7);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_KEY8);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_LOCKED);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_NEEDSALL);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_NEEDSONE);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_INITIALPOS);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_HITPOINTS);
				wnd->EnableWindow(false);
				wnd = GetDlgItem(IDC_AUTOCLOSE);
				wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_GB_IGNORE);
				wnd->EnableWindow(enabled);

	if (enabled) {
	if (m_CurrentDoorway==-1 || Objects[m_CurrentDoorway].type!=OBJ_DOOR)
		Current_door_room = -1;
	else
	{
				object *objp = &Objects[m_CurrentDoorway];
				Current_door_room = objp->roomnum;
				// Make the door room the current room
				theApp.m_pLevelWnd->SetPrim(&Rooms[Current_door_room],0,Rooms[Current_door_room].faces[0].portal_num,0,0);
				// Update the current face/texture displays
				Editor_state.SetCurrentRoomFaceTexture(Current_door_room, Curface);
				// Center the room
				ASSERT(theApp.m_pLevelWnd != NULL);
				if (theApp.m_pLevelWnd->m_bAutoCenter)
					theApp.m_pLevelWnd->CenterRoom(Curroomp);
				if (theApp.m_pRoomFrm != NULL)
					theApp.m_pRoomFrm->CenterRoom();
				ASSERT(Curroomp->flags & RF_DOOR);
				doorway *dp = Curroomp->doorway_data;
				ASSERT(dp != NULL);

				ned_door_info *door = &Doors[dp->doornum];
				bool blastable = ((door->flags & DF_BLASTABLE) != 0);
				wnd = GetDlgItem(IDC_HITPOINTS);
				wnd->EnableWindow(blastable);

					PrintToDlgItem((CDialog *)this,IDC_INITIALPOS,"%f",dp->position);
					PrintToDlgItem((CDialog *)this,IDC_HITPOINTS,"%f",Objects[m_CurrentDoorway].shields);
					CheckDlgButton(IDC_KEY1,dp->keys_needed & KEY_FLAG(1));
					CheckDlgButton(IDC_KEY2,dp->keys_needed & KEY_FLAG(2));
					CheckDlgButton(IDC_KEY3,dp->keys_needed & KEY_FLAG(3));
					CheckDlgButton(IDC_KEY4,dp->keys_needed & KEY_FLAG(4));
					CheckDlgButton(IDC_KEY5,dp->keys_needed & KEY_FLAG(5));
					CheckDlgButton(IDC_KEY6,dp->keys_needed & KEY_FLAG(6));
					CheckDlgButton(IDC_KEY7,dp->keys_needed & KEY_FLAG(7));
					CheckDlgButton(IDC_KEY8,dp->keys_needed & KEY_FLAG(8));
					CheckDlgButton(IDC_LOCKED,dp->flags & DF_LOCKED);
					CheckDlgButton(IDC_NEEDSALL,! (dp->flags & DF_KEY_ONLY_ONE));
					CheckDlgButton(IDC_NEEDSONE,dp->flags & DF_KEY_ONLY_ONE);
					CheckDlgButton(IDC_AUTOCLOSE,dp->flags & DF_AUTO);
					CheckDlgButton(IDC_GB_IGNORE,dp->flags & DF_GB_IGNORE_LOCKED);
	}
	}
				UpdateWindow();
				m_CurrentDoorwayChanged = false;
			}
		}break;
	}
	
	CDialogBar::OnTimer(nIDEvent);
}



int CDoorwayDialogBar::AddDoorToList(int door_slot)
{
	if(!IsDoorValid(door_slot))
		return 0;

	int slot = m_Combo.FindStringExact(0,Doors[door_slot].name);
	if(slot>=0)
		return 0;

	m_Combo.AddString(Doors[door_slot].name);
//	ned_MarkDoorInUse(door_slot,true);

	return 1;
}

int CDoorwayDialogBar::RemoveDoorFromList(int door_slot)
{
	if(!IsDoorValid(door_slot))
		return 0;

	int slot = m_Combo.FindStringExact(0,Doors[door_slot].name);
	if(slot>=0)
	{
		m_Combo.DeleteString(slot);
//		ned_MarkDoorInUse(door_slot,false);
		return 1;
	}

	return 0;
}

void CDoorwayDialogBar::OnSelchangeDoors() 
{
	// TODO: Add your control notification handler code here
	int sel = m_Combo.GetCurSel();
	if(sel<0)
	{
		SetCurrentDoor(-1);
		return;
	}

	CString buffer;
	m_Combo.GetLBText(sel,buffer);
	int doorid = ned_FindDoor(buffer.GetBuffer(0));
	SetCurrentDoor(doorid);	
}

void CDoorwayDialogBar::OnPlaceDoor() 
{
	// TODO: Add your control notification handler code here
	room *rp = theApp.m_pLevelWnd->m_Prim.roomp;
	int facenum = theApp.m_pLevelWnd->m_Prim.face;

	if (Num_doors<=0)
		return;

	if (m_CurrentDoor == -1) {
		OutrageMessageBox("You must have a current door for this operation");
		return;
	}

	if (rp->faces[facenum].portal_num != -1) {
		OutrageMessageBox("There's already a connection at the current room:face.");
		return;
	}

	PlaceDoor(rp,facenum,m_CurrentDoor);

//	UpdateDialog();

	World_changed = 1;
}

void CDoorwayDialogBar::OnAttachDoor() 
{
	// TODO: Add your control notification handler code here
	ASSERT(Placed_door != -1);

	AttachRoom();

//	UpdateDialog();
}

void CDoorwayDialogBar::OnCdwDoorName() 
{
	// TODO: Add your control notification handler code here
	ASSERT(Objects[m_CurrentDoorway].type == OBJ_DOOR);

	char tempname[OBJ_NAME_LEN+1] = "";
	object *curobj=&Objects[m_CurrentDoorway];
	if (curobj->name) {
		ASSERT(strlen(curobj->name) <= OBJ_NAME_LEN);
		strcpy(tempname,curobj->name);
	}

try_again:;
	if (! InputString(tempname,OBJ_NAME_LEN,"Door Name","Enter a name for this door:"))
		return;

	if (StripLeadingTrailingSpaces(tempname))
		EditorMessageBox("Note: Leading and/or trailing spaces have been removed from this name (\"%s\")",tempname);

	int handle = osipf_FindObjectName(tempname);
	if ((handle != OBJECT_HANDLE_NONE) && (handle != curobj->handle)) {
		EditorMessageBox("Door %d already has this name.",OBJNUM(ObjGet(handle)));
		goto try_again;
	}

	if (curobj->name) {
		mem_free(curobj->name);
		curobj->name = NULL;
	}

	if (strlen(tempname)) {
		curobj->name = (char *) mem_malloc(strlen(tempname)+1);
		strcpy(curobj->name,tempname);
	}

	World_changed = 1;

	char buffer[PAGENAME_LEN+OBJ_NAME_LEN+1] = "";
	sprintf(buffer,"%s \"%s\"",Doors[curobj->id].name,curobj->name);
	SetDlgItemText(IDC_CDW_DOORNAME,buffer);
}

void CDoorwayDialogBar::OnKey1() 
{
	// TODO: Add your control notification handler code here
	KeyCheck(IDC_KEY1,1);
}

void CDoorwayDialogBar::OnKey2() 
{
	// TODO: Add your control notification handler code here
	KeyCheck(IDC_KEY2,2);
}

void CDoorwayDialogBar::OnKey3() 
{
	// TODO: Add your control notification handler code here
	KeyCheck(IDC_KEY3,3);
}

void CDoorwayDialogBar::OnKey4() 
{
	// TODO: Add your control notification handler code here
	KeyCheck(IDC_KEY4,4);
}

void CDoorwayDialogBar::OnKey5() 
{
	// TODO: Add your control notification handler code here
	KeyCheck(IDC_KEY5,5);
}

void CDoorwayDialogBar::OnKey6() 
{
	// TODO: Add your control notification handler code here
	KeyCheck(IDC_KEY6,6);
}

void CDoorwayDialogBar::OnKey7() 
{
	// TODO: Add your control notification handler code here
	KeyCheck(IDC_KEY7,7);
}

void CDoorwayDialogBar::OnKey8() 
{
	// TODO: Add your control notification handler code here
	KeyCheck(IDC_KEY8,8);
}

void CDoorwayDialogBar::KeyCheck(int id,int keynum) 
{
	if (Current_door_room == -1)
		return;

	doorway *dp = Rooms[Current_door_room].doorway_data;

	ASSERT(dp != NULL);

	if (IsDlgButtonChecked(id))
		dp->keys_needed |= KEY_FLAG(keynum);
	else
		dp->keys_needed &= ~KEY_FLAG(keynum);
}

void CDoorwayDialogBar::OnLocked() 
{
	// TODO: Add your control notification handler code here
	if (Current_door_room == -1)
		return;

	doorway *dp = Rooms[Current_door_room].doorway_data;

	ASSERT(dp != NULL);

	int c=IsDlgButtonChecked(IDC_LOCKED);

	if (c)
		dp->flags|=DF_LOCKED;
	else
		dp->flags&=~DF_LOCKED;
}

void CDoorwayDialogBar::OnNeedsAll() 
{
	// TODO: Add your control notification handler code here
	if (Current_door_room == -1)
		return;

	doorway *dp = Rooms[Current_door_room].doorway_data;

	ASSERT(dp != NULL);

	if (IsDlgButtonChecked(IDC_NEEDSALL))
		dp->flags &= ~DF_KEY_ONLY_ONE;
	else
		dp->flags |= DF_KEY_ONLY_ONE;

	World_changed = 1;
	
//	UpdateDialog();
}

void CDoorwayDialogBar::OnNeedsOne() 
{
	// TODO: Add your control notification handler code here
	if (Current_door_room == -1)
		return;

	doorway *dp = Rooms[Current_door_room].doorway_data;

	ASSERT(dp != NULL);

	if (IsDlgButtonChecked(IDC_NEEDSONE))
		dp->flags |= DF_KEY_ONLY_ONE;
	else
		dp->flags &= ~DF_KEY_ONLY_ONE;

	World_changed = 1;
	
//	UpdateDialog();
}

void CDoorwayDialogBar::OnAutoClose() 
{
	// TODO: Add your control notification handler code here
	if (Current_door_room == -1)
		return;

	doorway *dp = Rooms[Current_door_room].doorway_data;

	ASSERT(dp != NULL);

	int c=IsDlgButtonChecked(IDC_AUTOCLOSE);

	if (c)
		dp->flags |= DF_AUTO;
	else
		dp->flags &= ~DF_AUTO;
}

void CDoorwayDialogBar::OnGBIgnore() 
{
	// TODO: Add your control notification handler code here
	if (Current_door_room == -1)
		return;

	doorway *dp = Rooms[Current_door_room].doorway_data;

	ASSERT(dp != NULL);

	int c=IsDlgButtonChecked(IDC_GB_IGNORE);

	if (c)
		dp->flags |= DF_GB_IGNORE_LOCKED;
	else
		dp->flags &= ~DF_GB_IGNORE_LOCKED;
}

void CDoorwayDialogBar::OnKillFocusInitialPos() 
{
	// TODO: Add your control notification handler code here
	if (Current_door_room == -1)
		return;

	doorway *dp = Rooms[Current_door_room].doorway_data;

	ASSERT(dp != NULL);

	char str[100];

	CEdit *ebox=(CEdit *) GetDlgItem(IDC_INITIALPOS);
	ebox->GetWindowText(str,sizeof(str));

	dp->position = dp->dest_pos = atof(str);

	if (dp->position < 0.0)
		dp->position = 0.0;
	if (dp->position > 1.0)
		dp->position = 1.0;

	dp->dest_pos = dp->position;

//	DoorwayUpdateAnimation(&Rooms[Current_door_room]);

	World_changed = 1;
}

void CDoorwayDialogBar::OnKillFocusHitPoints() 
{
	// TODO: Add your control notification handler code here
	if (Current_door_room == -1)
		return;

	doorway *dp = Rooms[Current_door_room].doorway_data;

	ASSERT(dp != NULL);

	char str[100];

	CEdit *ebox=(CEdit *) GetDlgItem(IDC_HITPOINTS);
	ebox->GetWindowText(str,sizeof(str));

	ASSERT(Objects[m_CurrentDoorway].type == OBJ_DOOR);

	Objects[m_CurrentDoorway].shields = atof(str);

	World_changed = 1;
}



void CDoorwayDialogBar::OnPrevDoorway() 
{
	// TODO: Add your control notification handler code here
	int next = m_CurrentDoorway;
prev_object:
	(next == 0) ? (next = Highest_object_index) : (next--);
	// Skip non-doors or unlinked objects
	if (Objects[next].type != OBJ_DOOR || Objects[next].roomnum == -1)
		goto prev_object;
	SetCurrentDoorway(next);
}

void CDoorwayDialogBar::OnNextDoorway() 
{
	// TODO: Add your control notification handler code here
	int next = m_CurrentDoorway;
next_object:
	(next == Highest_object_index) ? (next = 0) : (next++);
	// Skip non-doors and unlinked objects
	if (Objects[next].type != OBJ_DOOR || Objects[next].roomnum == -1)
		goto next_object;
	SetCurrentDoorway(next);
}

void CDoorwayDialogBar::OnDeleteDoor() 
{
	// TODO: Add your control notification handler code here
	room *rp, *doorrp = &Rooms[Current_door_room];
	int i,p;

	if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to delete Door %d?",ROOMNUM(doorrp)) != IDYES)
		return;

	if (doorrp == Curroomp) {
		// Before deleting the room, we need to change the current face texture for the texture dialog bar
		//Look for connected room
		for (p=0;p<doorrp->num_portals;p++)
			if (doorrp->portals[p].croom != -1) {
				rp = &Rooms[doorrp->portals[0].croom];
				break;
			}

		//If didn't find connected room, look for any room
		if (rp == NULL)
		{
			for (i=0;i<=Highest_room_index;i++)
			{
				if (Rooms[i].used && (i != ROOMNUM(doorrp))) 
				{
					rp = &Rooms[i];
					break;
				}
			}
		}

		ASSERT(rp != NULL);

		theApp.m_pLevelWnd->SetPrim(rp,0,-1,0,0);

		PrintStatus("Current room set to %d.",ROOMNUM(Curroomp));

		// Update the current face/texture displays again
		Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(Curroomp), Curface);
	}

	DeleteRoomFromMine(doorrp);

	int num_doors = GetDoorwayCount();

	if (m_CurrentDoorway >= num_doors)
	{
		m_CurrentDoorway--;
		m_CurrentDoorwayChanged = true;
	}

	World_changed = 1;
}

int GetDoorwayCount()
{
	int i;
	int num_doors = 0;
	int num_door_rooms = 0;

	for (i=0; i<=Highest_object_index; i++)
	{
		if (Objects[i].type == OBJ_DOOR)
			num_doors++;
	}

	for (i=0; i<=Highest_room_index; i++)
	{
		if ( Rooms[i].used && (Rooms[i].flags & RF_DOOR) )
			num_door_rooms++;
	}

	ASSERT(num_doors == num_door_rooms);

	return num_doors;
}

