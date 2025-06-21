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
 // SoundDialogBar.cpp : implementation file
//

#include "stdafx.h"
#include "ned_Tablefile.h"
#include "SoundDialogBar.h"
#include "ned_Sound.h"
#include "neweditor.h"
#include "../editor/DallasSoundDlg.h"
#include "EditLineDialog.h"
#include "ned_Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSoundDialogBar dialog

inline bool IsSoundValid(int id)
{
	if(id<0 || id>=MAX_SOUNDS || !Sounds[id].used)
		return false;
	return true;
}

// void lsound_init(void);


CSoundDialogBar::CSoundDialogBar()
{
	//{{AFX_DATA_INIT(CSoundDialogBar)
	m_volume = 1.00f;
	//}}AFX_DATA_INIT
}


void CSoundDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundDialogBar)
	DDX_Control(pDX, IDC_SOUND_COMBO, m_sound_combo);
	DDX_Text(pDX, IDC_SOUND_VOLUME, m_volume);
	DDV_MinMaxFloat(pDX, m_volume, 0.f, 1.f);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSoundDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CSoundDialogBar)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(IDC_SOUND_NAME,OnControlUpdate)
	ON_BN_CLICKED(IDC_SOUND_DELETE, OnSoundDelete)
	ON_BN_CLICKED(IDC_SOUND_INSERT, OnSoundInsert)
	ON_BN_CLICKED(IDC_SOUND_NAME, OnSoundName)
	ON_CBN_SELCHANGE(IDC_SOUND_COMBO, OnSelchangeSoundCombo)
	ON_EN_KILLFOCUS(IDC_SOUND_VOLUME, OnKillfocusSoundVolume)
	ON_BN_CLICKED(IDC_WAYPOINT_INSERT, OnWaypointInsert)
	ON_BN_CLICKED(IDC_WAYPOINT_DELETE, OnWaypointDelete)
	ON_BN_CLICKED(IDC_SOUND_PLAY, OnSoundPlay)
	ON_UPDATE_COMMAND_UI(IDC_SOUND_COMBO,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SOUND_VOLUME,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SOUND_INSERT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SOUND_DELETE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_WAYPOINT_INSERT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_WAYPOINT_DELETE,OnControlUpdate)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

CSoundDialogBar *dlgSoundDialogBar = NULL;


/////////////////////////////////////////////////////////////////////////////
// CSoundDialogBar message handlers

LONG CSoundDialogBar::OnInitDialog ( UINT wParam, LONG lParam)
{
	dlgSoundDialogBar = this;
//	lsound_init();

	//[ISB] In currentyear this function never returns true. heh. 
	HandleInitDialog(wParam, lParam);
	if (!UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
		return FALSE;
	}

	InitKeyPad();


	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CSoundDialogBar::PreTranslateMessage(MSG* pMsg) 
{
	return CDialogBar::PreTranslateMessage(pMsg);
}

//this is what enables your controls:
void CSoundDialogBar::OnControlUpdate(CCmdUI* pCmdUI)
{
	bool bEnable = false;
	int sel = -1;

	if (theApp.m_pLevelWnd != NULL)
	{
		switch (pCmdUI->m_nID)
		{
//		case IDC_SOUND_BROWSE:
		case IDC_SOUND_DELETE:
		case IDC_SOUND_NAME:
		case IDC_SOUND_COMBO:
		case IDC_SOUND_VOLUME:
			if (Cur_object_index != -1 && Cur_object_index<=Highest_object_index && 
				Objects[Cur_object_index].type == OBJ_SOUNDSOURCE) //  && IsSoundValid(Objects[Cur_object_index].id)
				bEnable = true;
			break;

		case IDC_SOUND_INSERT:
			bEnable = true;
			break;

		case IDC_SOUND_PLAY:
			sel = m_sound_combo.GetCurSel();
			if(sel>0)
			{
				CString buffer;
				m_sound_combo.GetLBText(sel,buffer);
				int soundid = ned_FindSound(buffer.GetBuffer(0));
				if (soundid != -1)
					bEnable = true;
			}
			break;

		case IDC_WAYPOINT_INSERT:
			bEnable = true;
			break;

		case IDC_WAYPOINT_DELETE:
			if (Cur_object_index != -1 && Cur_object_index<=Highest_object_index && 
				Objects[Cur_object_index].type == OBJ_WAYPOINT)
				bEnable = true;
			break;
		}
	}

	pCmdUI->Enable(bEnable);
}

void CSoundDialogBar::InitKeyPad(void)
{
	for(int i=0;i<MAX_SOUNDS;i++)
	{
		AddSoundToList(i);
	}
}

void CSoundDialogBar::UpdateDialog(void)
{
	char buffer[OBJ_NAME_LEN+PAGENAME_LEN+1] = "";
	char name[PAGENAME_LEN+1] = "";
	char str[20];
	object *curobj=&Objects[Cur_object_index];

	if (curobj->type == OBJ_SOUNDSOURCE)
	{
		strcpy(name,Sounds[curobj->ctype.soundsource_info.sound_index].name);
//		int id = ned_FindSound(name);
//		if (id != -1)
		{
			// Update name
			if (curobj->name != NULL && strcmp(curobj->name, ""))
			{
				sprintf(buffer,"%s",curobj->name);
				SetDlgItemText(IDC_SOUND_NAME,buffer);
			}

			// Update id
			int slot = m_sound_combo.FindStringExact(0,name);
			if (slot != -1)
				m_sound_combo.SetCurSel(slot);
		}

		// Update volume
		sprintf(str,"%.2f",curobj->ctype.soundsource_info.volume);
		SetDlgItemText(IDC_SOUND_VOLUME,str);
		UpdateData(true);
	}
}

void _sounddlg_destroysoundlist(CSoundDialogBar *wnd)
{
	//remove all string from it
	int id,count = wnd->m_sound_combo.GetCount();
	CString buffer;

	//select none
	wnd->m_sound_combo.SetCurSel(-1);
//	wnd->OnSelchangeSounds();

	for(int i=0;i<count;i++)
	{
		wnd->m_sound_combo.GetLBText(0,buffer);
		id = ned_FindSound(buffer.GetBuffer(0));
		ASSERT(id!=-1);

		wnd->RemoveSoundFromList(id);
	}

}

void CSoundDialogBar::OnDestroy() 
{
	_sounddlg_destroysoundlist(this);

	CDialogBar::OnDestroy();

	dlgSoundDialogBar = NULL;
}

/*
short Level_sounds_in_use[MAX_SOUNDS];
void lsound_init(void)
{
	for(int i=0;i<MAX_SOUNDS;i++)
	{
		Level_sounds_in_use[i] = 0;
	}
}
*/


int CSoundDialogBar::AddSoundToList(int sound_slot)
{
	if(!IsSoundValid(sound_slot))
		return 0;

	int slot = m_sound_combo.FindStringExact(0,Sounds[sound_slot].name);
	if(slot>=0)
		return 0;

	m_sound_combo.AddString(Sounds[sound_slot].name);

	return 1;
}

int CSoundDialogBar::RemoveSoundFromList(int sound_slot)
{
	if(!IsSoundValid(sound_slot))
		return 0;

	int slot = m_sound_combo.FindStringExact(0,Sounds[sound_slot].name);
	if(slot>=0)
	{
		m_sound_combo.DeleteString(slot);
		return 1;
	}

	return 0;
}

/*
void CSoundDialogBar::OnSoundBrowse() 
{
	// TODO: Add your control notification handler code here
	CDallasSoundDlg dlg;

	int selected = m_sound_combo.GetCurSel();
	if (selected != -1)
		dlg.m_SoundName = Sounds[selected].name;

	if (dlg.DoModal() == IDCANCEL)
		return;

	m_sound_combo.SetCurSel(dlg.m_SoundIndex);
}
*/

void CSoundDialogBar::OnSoundDelete() 
{
	// TODO: Add your control notification handler code here
	HObjectDelete();
	Editor_state.SetCurrentLevelObject(Cur_object_index);
}

void CSoundDialogBar::OnSoundInsert() 
{
	// TODO: Add your control notification handler code here
	int roomnum;
	vector pos;
	matrix orient = IDENTITY_MATRIX;
	void *wnd = theApp.AcquireWnd();

	if (wnd != NULL && theApp.m_pLevelWnd != NULL && theApp.m_pLevelWnd->m_Prim.roomp != NULL)
	{
		if (wnd == theApp.m_pRoomFrm)
		{
			room *rp = theApp.m_pLevelWnd->m_Prim.roomp;
			roomnum = ROOMNUM(rp);
/*
			ComputeRoomCenter(&pos,prim->roomp);

			switch (((CRoomFrm *) wnd)->m_Focused_pane)
			{
			case 0:
				((CRoomFrm *) wnd)->m_vec_InsertPos.z = pos.z;
				break;
			case 1:
				((CRoomFrm *) wnd)->m_vec_InsertPos.y = pos.y;
				break;
			case 2:
				((CRoomFrm *) wnd)->m_vec_InsertPos.x = pos.x;
				break;
			default:
				return;
			}
*/
			pos = ((CRoomFrm *) wnd)->m_vec_InsertPos;

			int objnum;

			objnum = ObjCreate(OBJ_SOUNDSOURCE,0,roomnum,&pos,&orient);

			if (objnum == -1) {
				Int3();
				return;
			}

			object *objp = &Objects[objnum];
			objp->ctype.soundsource_info.sound_index = -1;
			objp->ctype.soundsource_info.volume = 1.0;

			//select none
			m_sound_combo.SetCurSel(-1);

			OutrageMessageBox("SoundSource created as object %d\n",objnum);
	
			Cur_object_index = objnum;

			World_changed = 1;

		}
		else
			OutrageMessageBox("You must be in the current room view to insert soundsources.");

	}

}

void CSoundDialogBar::OnSoundName() 
{
	// TODO: Add your control notification handler code here
	char tempname[OBJ_NAME_LEN+1] = "";
	object *curobj=&Objects[Cur_object_index];
	if (curobj->name) {
		ASSERT(strlen(curobj->name) <= OBJ_NAME_LEN);
		strcpy(tempname,curobj->name);
	}

try_again:;
	if (! InputString(tempname,OBJ_NAME_LEN,"SoundSource Name","Enter a name for this soundsource:"))
		return;

	if (StripLeadingTrailingSpaces(tempname))
		EditorMessageBox("Note: Leading and/or trailing spaces have been removed from this name (\"%s\")",tempname);

	int handle = osipf_FindObjectName(tempname);
	if ((handle != OBJECT_HANDLE_NONE) && (handle != curobj->handle)) {
		EditorMessageBox("SoundSource %d already has this name.",OBJNUM(ObjGet(handle)));
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
	sprintf(buffer,"%s \"%s\"",Sounds[curobj->ctype.soundsource_info.sound_index].name,curobj->name);
	SetDlgItemText(IDC_SOUND_NAME,buffer);
}

void CSoundDialogBar::OnSelchangeSoundCombo() 
{
	// TODO: Add your control notification handler code here
	int sel = m_sound_combo.GetCurSel();
	if(sel<0)
	{
		//select none
		return;
	}

	CString buffer;
	m_sound_combo.GetLBText(sel,buffer);
	int soundid = ned_FindSound(buffer.GetBuffer(0));
	object *curobj=&Objects[Cur_object_index];
	curobj->ctype.soundsource_info.sound_index = soundid;
}

void CSoundDialogBar::OnKillfocusSoundVolume() 
{
	// TODO: Add your control notification handler code here
	if ( UpdateData(true) )
	{
		object *curobj=&Objects[Cur_object_index];
		curobj->ctype.soundsource_info.volume = m_volume;
	}
}

#include "player.h"

void CSoundDialogBar::OnWaypointInsert() 
{
	// TODO: Add your control notification handler code here
	int objnum;
	int count=0;
	int roomnum;
	vector pos;
	matrix orient = IDENTITY_MATRIX;
	void *wnd = theApp.AcquireWnd();

	if (wnd != NULL && theApp.m_pLevelWnd != NULL && theApp.m_pLevelWnd->m_Prim.roomp != NULL)
	{
		if (wnd == theApp.m_pRoomFrm)
		{
			room *rp = theApp.m_pLevelWnd->m_Prim.roomp;
			roomnum = ROOMNUM(rp);

			pos = ((CRoomFrm *) wnd)->m_vec_InsertPos;

			//Count the number of waypoints
			for (int i=0;i<=Highest_object_index;i++)
				if (Objects[i].type == OBJ_WAYPOINT)
					count++;

			if (count >= MAX_WAYPOINTS) {
				EditorMessageBox("Cannot add waypoint: Already at maximum number of waypoints.");
				return;
			}

			//Check for waypoint already in this room
//			if (OBJECT_OUTSIDE(Viewer_object))
//				objnum = Terrain_seg[CELLNUM(Viewer_object->roomnum)].objects;
//			else
				objnum = Rooms[roomnum].objects;

			for (objnum=Rooms[roomnum].objects;objnum!=-1;objnum=Objects[objnum].next) {
				if (Objects[objnum].type == OBJ_WAYPOINT) {
					EditorMessageBox("Cannot add waypoint: There's already a waypoint in this room (object %d)",objnum);
					return;
				}
			}

			objnum = ObjCreate(OBJ_WAYPOINT,0,roomnum,&pos,&orient);

			if (objnum == -1) {
				Int3();
				return;
			}

			EditorStatus("Waypoint created as object %d\n",objnum);

			Cur_object_index = objnum;
			
			World_changed = 1;

		}
		else
			OutrageMessageBox("You must be in the current room view to insert waypoints.");

	}

}

void CSoundDialogBar::OnWaypointDelete() 
{
	// TODO: Add your control notification handler code here
	HObjectDelete();
	Editor_state.SetCurrentLevelObject(Cur_object_index);
}

void CSoundDialogBar::OnSoundPlay() 
{
	// TODO: Add your control notification handler code here
	
}
