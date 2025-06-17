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
 // ObjectDialogBar.cpp : implementation file
//

#include "stdafx.h"
#include "globals.h"
#include "ned_Util.h"
#include "neweditor.h"
#include "ObjectDialogBar.h"
#include "EditLineDialog.h"
#include "ObjectPalette.h"
#include "mono.h"
#include "ned_Object.h"
#include "descent.h"
#include "object.h"
#include "polymodel.h"
#include "ship.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int Insert_mode = 0;

/////////////////////////////////////////////////////////////////////////////
// CObjectDialogBar dialog

inline bool IsObjectValid(int id)
{
	if(id<0 || id>=MAX_OBJECT_IDS || !Object_info[id].used)
		return false;
	return true;
}

void lobj_init(void);


CObjectDialogBar::CObjectDialogBar()
{
	//{{AFX_DATA_INIT(CObjectDialogBar)
	//}}AFX_DATA_INIT
	m_pObjectPalette = NULL;
	m_CurrentObject = -1;
	m_CurrentLevelObject = -1;
	m_CurrentLevelObjectChanged = true;
	m_CurrentObjectChanged = true;
	m_CurrentPlayerChanged = true;
	m_current_start_pos = 0;
}


void CObjectDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectDialogBar)
	DDX_Control(pDX, IDC_CO_VIEW, m_COView);
	DDX_Control(pDX, IDC_CLO_VIEW, m_CLOView);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CObjectDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CObjectDialogBar)
	ON_WM_PAINT()
	ON_UPDATE_COMMAND_UI(IDC_CLO_PROPERTIES,OnControlUpdate)
	ON_BN_CLICKED(IDC_OBJECT_PALETTE, OnObjectPalette)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CO_PROPERTIES, OnCoProperties)
	ON_BN_CLICKED(IDC_CLO_PROPERTIES, OnCloProperties)
	ON_BN_CLICKED(IDC_BUTTON2, OnObjectInsert)
	ON_BN_CLICKED(IDC_CLO_OBJECTNAME, OnCloObjectName)
	ON_BN_CLICKED(IDC_BUTTON5, OnButton5)
	ON_BN_CLICKED(IDC_BUTTON6, OnButton6)
	ON_BN_CLICKED(IDC_PLAYER_DELETE, OnPlayerDelete)
	ON_BN_CLICKED(IDC_PLAYER_RED, OnPlayerRed)
	ON_BN_CLICKED(IDC_PLAYER_BLUE, OnPlayerBlue)
	ON_BN_CLICKED(IDC_PLAYER_GREEN, OnPlayerGreen)
	ON_BN_CLICKED(IDC_PLAYER_YELLOW, OnPlayerYellow)
	ON_BN_CLICKED(IDC_PLAYER_NEXT, OnPlayerNext)
	ON_BN_CLICKED(IDC_PLAYER_PREV, OnPlayerPrev)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_UPDATE_COMMAND_UI(IDC_CO_PROPERTIES,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON2,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_OBJECT_PALETTE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_CLO_OBJECTNAME,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON5,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON6,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PLAYER_NEXT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PLAYER_PREV,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PLAYER_DELETE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PLAYER_RED,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PLAYER_BLUE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PLAYER_GREEN,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PLAYER_YELLOW,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_BUTTON1,OnControlUpdate)
	ON_BN_CLICKED(IDC_INSERT_MINE, OnInsertMine)
	ON_BN_CLICKED(IDC_INSERT_TERRAIN, OnInsertTerrain)
	ON_UPDATE_COMMAND_UI(IDC_INSERT_MINE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_INSERT_TERRAIN,OnControlUpdate)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectBarList dialog
CObjectDialogBar *dlgObjectDialogBar = NULL;

CObjectBarList::CObjectBarList(CWnd* pParent )
	: CDialog(CObjectBarList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectBarList)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT_VALID(dlgObjectDialogBar);
	m_ParentWindow = dlgObjectDialogBar;
	ignore_mark = false;
}


void CObjectBarList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectBarList)
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CObjectBarList, CDialog)
	//{{AFX_MSG_MAP(CObjectBarList)
	ON_LBN_SELCHANGE(IDC_LIST, OnSelchangeList)
	ON_WM_SHOWWINDOW()
	ON_WM_CONTEXTMENU()
	ON_COMMAND(ID_ADDTOCUSTOM, OnAddToCustom)
	ON_UPDATE_COMMAND_UI(ID_ADDTOCUSTOM, OnUpdateAddToCustom)
	ON_COMMAND(ID_REMOVE, OnRemove)
	ON_UPDATE_COMMAND_UI(ID_REMOVE, OnUpdateRemove)
	ON_COMMAND(ID_SAVELIST, OnSaveList)
	ON_UPDATE_COMMAND_UI(ID_SAVELIST, OnUpdateSaveList)
	ON_COMMAND(ID_LOADLIST, OnLoadList)
	ON_UPDATE_COMMAND_UI(ID_LOADLIST, OnUpdateLoadList)
	ON_COMMAND(ID_REMOVE_ALL, OnRemoveAll)
	ON_UPDATE_COMMAND_UI(ID_REMOVE_ALL, OnUpdateRemoveAll)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
/////////////////////////////////////////////////////////////////////////////
// CObjectBarList message handlers

int CObjectBarList::AddObjectToList(int obj_slot)
{
	int slot;
	char listing[PAGENAME_LEN+10];

	if(!IsObjectValid(obj_slot))
		return 0;

#ifdef _DEBUG
	if (this == m_ParentWindow->m_LevelDialog)
		sprintf(listing,"%d %s",Level_objects_in_use[obj_slot],Object_info[obj_slot].name);
	else if (this == m_ParentWindow->m_CustomDialog)
#endif
		strcpy(listing,Object_info[obj_slot].name);

	slot = m_List.FindStringExact(0,listing);
	if(slot>=0)
		return 0;

	m_List.AddString(Object_info[obj_slot].name);
//	ned_MarkObjectInUse(obj_slot,true);

	return 1;
}

int CObjectBarList::RemoveObjectFromList(int obj_slot)
{
	int slot;
	char listing[PAGENAME_LEN+10];

	if(!IsObjectValid(obj_slot))
		return 0;

#ifdef _DEBUG
	if (this == m_ParentWindow->m_LevelDialog)
		sprintf(listing,"%d %s",Level_objects_in_use[obj_slot],Object_info[obj_slot].name);
	else if (this == m_ParentWindow->m_CustomDialog)
#endif
		strcpy(listing,Object_info[obj_slot].name);

	slot = m_List.FindStringExact(0,listing);
	if(slot>=0)
	{
		m_List.DeleteString(slot);
//		ned_MarkObjectInUse(obj_slot,false);
		return 1;
	}

	return 0;
}

void CObjectBarList::OnSelchangeList() 
{
	int sel = m_List.GetCurSel();
	if(sel<0)
	{
		//select none
		if(!ignore_mark)		
			m_ParentWindow->SetCurrentObject(-1);
		ignore_mark = false;
		return;
	}

	CString buffer;
	m_List.GetText(sel,buffer);
	char *objstr;
	char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
	if (this != m_ParentWindow->m_CustomDialog) // quick hack
	{
		objstr = strtok(buffer.GetBuffer(0)," ");
		objstr += strlen(objstr) + 1;
	}
	else
	{
		strcpy(str,buffer.GetBuffer(0));
		objstr = str;
	}
#else
	strcpy(str,buffer.GetBuffer(0));
	objstr = str;
#endif

	int objid = ned_FindObjectID(objstr);
	m_ParentWindow->SetCurrentObject(objid);	
}

void CObjectBarList::OnOK() 
{
	//this is here because since there are no buttons on the list, this is attached to a hidden
	//OK button, so if the user hits enter in the list box, it doesn't close it
}

void CObjectBarList::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	int curr_obj = m_ParentWindow->GetCurrentObject();
	int our_curr = m_List.GetCurSel();
	int our_obj = -1;
	char listing[PAGENAME_LEN+10];

	if(our_curr!=-1)
	{
		CString buffer;
		m_List.GetText(our_curr,buffer);
		char *objstr;
		char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
		if (this != m_ParentWindow->m_CustomDialog) // quick hack
		{
			objstr = strtok(buffer.GetBuffer(0)," ");
			objstr += strlen(objstr) + 1;
		}
		else
		{
			strcpy(str,buffer.GetBuffer(0));
			objstr = str;
		}
#else
		strcpy(str,buffer.GetBuffer(0));
		objstr = str;
#endif

		our_obj = ned_FindObjectID(objstr);
	}

	if(our_obj!=curr_obj)
	{
		//our selections don't match...blah, unselect without causing a comotion
		ignore_mark = true;
		m_List.SetCurSel(-1);
		ignore_mark = false;
	}

	if(curr_obj==-1)
		return;

#ifdef _DEBUG
	if (this == m_ParentWindow->m_LevelDialog)
		sprintf(listing,"%d %s",Level_objects_in_use[curr_obj],Object_info[curr_obj].name);
	else if (this == m_ParentWindow->m_CustomDialog)
#endif
		strcpy(listing,Object_info[curr_obj].name);

	//see if the curr object is in our list
	int new_sel = m_List.FindStringExact(0,listing);
	if(new_sel ==-1)
		return;	//not in our list

	//select the new selection
	m_List.SetCurSel(new_sel);		
}


/////////////////////////////////////////////////////////////////////////////
// CObjectDialogBar message handlers

LONG CObjectDialogBar::OnInitDialog ( UINT wParam, LONG lParam)
{
	dlgObjectDialogBar = this;
	lobj_init();

	if ( !HandleInitDialog(wParam, lParam) || !UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
		return FALSE;
	}

	m_ObjectTabDialog = NULL;
	m_CustomDialog = NULL;
	m_LevelDialog = NULL;
	m_CurrentObjectTab = TAB_OBJECT_CUSTOM;

	InitKeyPad();
	ShowCurrentObjectTab();

	//Setup update timers
	if(!SetTimer(TIMER_OBJECTS,100,NULL))
	{
		mprintf((0,"Unable to create timer\n"));
		Int3();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CObjectDialogBar::PreTranslateMessage(MSG* pMsg) 
{
	return CDialogBar::PreTranslateMessage(pMsg);
}

//this is what enables your controls:
void CObjectDialogBar::OnControlUpdate(CCmdUI* pCmdUI)
{
	bool bEnable = false;

	// Most controls for the object bar require an open level
	switch (pCmdUI->m_nID)
	{
	case IDC_CO_PROPERTIES:
		if (IsObjectValid(m_CurrentObject))
			bEnable = true;
		break;
	case IDC_OBJECT_PALETTE:
		bEnable = true;
		break;
	default:
		if (theApp.m_pLevelWnd != NULL)
			bEnable = true;
		break;
	}

	pCmdUI->Enable(bEnable);
}

extern int GetObjectImage(int handle);

void CObjectDialogBar::PaintPreviewArea(CWnd *pWnd)
{
	int obj_type,obj_id,model_num;

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
	grSurface *ds = Editor_state.GetDesktopSurface();
	if(!ds)
		return;

	//setup the drawing area
	grHardwareSurface surface;
	ds->attach_to_window((unsigned)pWnd->m_hWnd);
	surface.create(width,height,BPP_16);

	//render here
	if(pWnd==&m_COView)
	{
		//Current Object View
		if(IsObjectValid(m_CurrentObject))
		{
			obj_type = Object_info[m_CurrentObject].type;
			obj_id = m_CurrentObject;
			switch (obj_type)
			{
			case OBJ_CLUTTER:
			case OBJ_BUILDING:
			case OBJ_POWERUP:
			case OBJ_ROBOT: 
				model_num = GetObjectImage(obj_id);
				break;
			case OBJ_PLAYER:
				model_num = -1;
//				model_num = GetShipImage(obj_id);
				break;
			default:
				model_num = -1;
				Int3();
			}

			DrawItemModel(0,0,ds,&surface,model_num,true);
		}
		else
		{
			surface.clear(bk_col);
			ds->blt(0, 0, &surface);
		}
	}else if(pWnd==&m_CLOView)
	{
		//Current Level Object View		
		if (m_CurrentLevelObject != -1)
		{
			obj_type = Objects[m_CurrentLevelObject].type;
			obj_id = Objects[m_CurrentLevelObject].id;
			switch (obj_type)
			{
			case OBJ_CLUTTER:
			case OBJ_BUILDING:
			case OBJ_POWERUP:
			case OBJ_ROBOT: 
				model_num = GetObjectImage(obj_id);
				break;
			case OBJ_PLAYER:
				model_num = -1;
//				model_num = GetShipImage(obj_id);
				break;
			case OBJ_VIEWER:
				model_num = -1;
				break;
			case OBJ_SOUNDSOURCE:
				model_num = -1;
				break;
			case OBJ_CAMERA:
				model_num = -1;
				break;
			case OBJ_WAYPOINT:
				model_num = -1;
				break;
			case OBJ_DOOR:
				model_num = -1; // don't render doors
				break;
			default:
				model_num = -1;
				Int3();
			}

			DrawItemModel(0,0,ds,&surface,model_num,true);
		}
		else
		{
			surface.clear(bk_col);
			ds->blt(0, 0, &surface);
		}
	}else
	{
		Int3();
	}

	//free up
	surface.free();
	ds->attach_to_window(NULL);
}

void CObjectDialogBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
		
	PaintPreviewArea(&m_COView);
	PaintPreviewArea(&m_CLOView);
	
	// Do not call CDialogBar::OnPaint() for painting messages
}

void CObjectDialogBar::RedrawPreviewArea(CWnd *pWnd)
{
	ASSERT_VALID(pWnd);

	//Get Placeholder coordinates
	CRect rect;
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);

	//force update
	InvalidateRect(rect,false);	
}

void CObjectDialogBar::RedrawArea()
{
	RedrawPreviewArea(&m_COView);
	RedrawPreviewArea(&m_CLOView);
	UpdateWindow();
}

void CObjectDialogBar::ObjectPaletteDone()
{
	mprintf((0,"Removing Object Palette\n"));
	m_pObjectPalette = NULL;
	GetDlgItem(IDC_OBJECT_PALETTE)->EnableWindow();

	if(m_ObjectTabDialog)
	{
		//refresh just incase they added a custom object which just happens to be the 
		//selected object
		m_ObjectTabDialog->ShowWindow(SW_HIDE);
		m_ObjectTabDialog->ShowWindow(SW_SHOW);
	}
}

void CObjectDialogBar::OnObjectPalette() 
{
	// if our modeless child isn't already up, create it and display it
	// otherwise, just set focus to it

	if (m_pObjectPalette == NULL)
	{
		m_pObjectPalette = new CObjectPalette(this);
		if (m_pObjectPalette->Create() == TRUE)
		{
			GetDlgItem(IDC_OBJECT_PALETTE)->EnableWindow(FALSE);
			m_pObjectPalette->ShowWindow(SW_SHOW);
		}
	}
	else
		m_pObjectPalette->SetActiveWindow();
}

void CObjectDialogBar::InitKeyPad(void)
{
	CTabCtrl *tab_ctrl;
	TC_ITEM tc_item;

	tab_ctrl = (CTabCtrl *)GetDlgItem(IDC_OBJECT_GROUP_TAB);

	//	Add tabs to tabcontrol in the dialog bar.
	tc_item.mask = TCIF_TEXT;
	tc_item.pszText = "Custom";
	tc_item.cchTextMax = lstrlen(tc_item.pszText)+1;
	tab_ctrl->InsertItem(TAB_OBJECT_CUSTOM, &tc_item);

	tc_item.mask = TCIF_TEXT;
	tc_item.pszText = "Level";
	tc_item.cchTextMax = lstrlen(tc_item.pszText)+1;
	tab_ctrl->InsertItem(TAB_OBJECT_LEVEL, &tc_item);	

	//	Get coordinates of dialog bar tab and 
	RECT tab_rect, tab_winrect;
	int	tx, ty, tw, th;

	tab_ctrl->GetWindowRect(&tab_winrect);
	::CopyRect(&tab_rect, &tab_winrect);
	tab_ctrl->AdjustRect(FALSE, &tab_rect);
	tx = tab_rect.left - tab_winrect.left;
	ty = tab_rect.top - tab_winrect.top;
	tw = tab_rect.right - tab_rect.left;
	th = tab_rect.bottom - tab_rect.top;

	//	Create tab dialogs
	//	resize them to fit the tab control
	//	set scroll bars accordingly
	m_CustomDialog = new CObjectBarList(tab_ctrl);
	m_CustomDialog->Create(IDD_BAR_LIST,tab_ctrl);
	m_CustomDialog->SetWindowPos(tab_ctrl, tx, ty, tw, th, SWP_NOZORDER);
	//only for the custom dialog do we set the OLE reciever
	bool oleret = Editor_state.RegisterGenericDropWindow(m_CustomDialog);

	m_LevelDialog = new CObjectBarList(tab_ctrl);
	m_LevelDialog->Create(IDD_BAR_LIST,tab_ctrl);
	m_LevelDialog->SetWindowPos(tab_ctrl, tx, ty, tw, th, SWP_NOZORDER);

	// Associate icons with some of the buttons
	((CButton *)GetDlgItem(IDC_BUTTON5))->SetIcon(theApp.LoadIcon(IDI_PLAYER_INSERT));
	((CButton *)GetDlgItem(IDC_PLAYER_DELETE))->SetIcon(theApp.LoadIcon(IDI_PLAYER_DELETE));
	((CButton *)GetDlgItem(IDC_PLAYER_RED))->SetIcon(theApp.LoadIcon(IDI_PLAYER_RED));
	((CButton *)GetDlgItem(IDC_PLAYER_BLUE))->SetIcon(theApp.LoadIcon(IDI_PLAYER_BLUE));
	((CButton *)GetDlgItem(IDC_PLAYER_GREEN))->SetIcon(theApp.LoadIcon(IDI_PLAYER_GREEN));
	((CButton *)GetDlgItem(IDC_PLAYER_YELLOW))->SetIcon(theApp.LoadIcon(IDI_PLAYER_YELLOW));
}

//	Display current tab dialog	
void CObjectDialogBar::ShowCurrentObjectTab()
{
	m_ObjectTabDialog = NULL;
	switch (m_CurrentObjectTab) 
	{
		case TAB_OBJECT_CUSTOM:	m_ObjectTabDialog = m_CustomDialog; break;
		case TAB_OBJECT_LEVEL:		m_ObjectTabDialog = m_LevelDialog; break;
		default: Int3();
	}

	if (m_ObjectTabDialog) {
		CTabCtrl *tab_ctrl = (CTabCtrl *)GetDlgItem(IDC_OBJECT_GROUP_TAB);

	  	m_ObjectTabDialog->ShowWindow(SW_SHOW);
		tab_ctrl->SetCurSel(m_CurrentObjectTab);
	}
}

void CObjectDialogBar::DoObjectTabNotify(NMHDR *nmhdr)
{
	CTabCtrl *tab_ctrl;
		
	tab_ctrl = (CTabCtrl *)GetDlgItem(IDC_OBJECT_GROUP_TAB);

	switch (nmhdr->code)
	{
		case TCN_SELCHANGE:				
		// Tab control changed. So update the controls currently displayed.
			m_CurrentObjectTab = (int)tab_ctrl->GetCurSel(); 
			assert(m_CurrentObjectTab > -1);

			switch (m_CurrentObjectTab)
			{
				case TAB_OBJECT_CUSTOM:
					if (m_ObjectTabDialog) m_ObjectTabDialog->ShowWindow(SW_HIDE);
					m_ObjectTabDialog = m_CustomDialog;
					m_ObjectTabDialog->ShowWindow(SW_SHOW);
					break;

				case TAB_OBJECT_LEVEL:
					if (m_ObjectTabDialog) m_ObjectTabDialog->ShowWindow(SW_HIDE);
					m_ObjectTabDialog = m_LevelDialog;
					m_ObjectTabDialog->ShowWindow(SW_SHOW);
					break;
			}
			break;
	}
}

BOOL CObjectDialogBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
//	Perform notification requests on the keypad tab dialog bar and other 
//	custom control bars.
	NMHDR *nmhdr;

	assert(lParam != 0);
	
	nmhdr = (NMHDR *)lParam;

	switch (wParam)
	{
		case IDC_OBJECT_GROUP_TAB:
			DoObjectTabNotify(nmhdr);
			break;
	}
	
	return CDialogBar::OnNotify(wParam, lParam, pResult);
}

void _objdlg_destroyobjlist(CObjectBarList *wnd)
{
	//remove all string from it
	CListBox *list = (CListBox *)wnd->GetDlgItem(IDC_LIST);
	int id,count = list->GetCount();
	CString buffer;

	//select none
	list->SetCurSel(-1);
	wnd->OnSelchangeList();

	for(int i=0;i<count;i++)
	{
		list->GetText(0,buffer);
		char *objstr;
		char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
		if (wnd != wnd->m_ParentWindow->m_CustomDialog) // quick hack (made custom dialog public too; only temporary)
		{
			objstr = strtok(buffer.GetBuffer(0)," ");
			objstr += strlen(objstr) + 1;
		}
		else
		{
			strcpy(str,buffer.GetBuffer(0));
			objstr = str;
		}
#else
		strcpy(str,buffer.GetBuffer(0));
		objstr = str;
#endif

		id = ned_FindObjectID(objstr);
		ASSERT(id!=-1);

		wnd->RemoveObjectFromList(id);
	}

	wnd->DestroyWindow();
	delete wnd;
}

void CObjectDialogBar::OnDestroy() 
{
	CDialogBar::OnDestroy();
	
	dlgObjectDialogBar = NULL;

	if(m_CustomDialog){
		_objdlg_destroyobjlist(m_CustomDialog);
		m_CustomDialog = NULL;
	}	

	if(m_LevelDialog){
		_objdlg_destroyobjlist(m_LevelDialog);
		m_LevelDialog = NULL;
	}	
}

// ======================
// GetCurrentObject
// ======================
//
// Gets the current object id
int CObjectDialogBar::GetCurrentObject(void)
{
	return m_CurrentObject;
}


// ======================
// SetCurrentObject
// ======================
//
// Marks an object the current object to work with in the editor
// ONLY CALL THIS WITH GENERIC OBJECTS!
void CObjectDialogBar::SetCurrentObject(int Object)
{
	if(Object<-1 || Object>=MAX_OBJECT_IDS)
	{
		Int3();
		return;
	}

	if(Object!=-1 && !Object_info[Object].used)
	{
		Int3();
		return;
	}

	if(IsObjectValid(m_CurrentObject))
	{
		ned_MarkObjectInUse(m_CurrentObject,false);
		m_CurrentObject = -1;
	}

	m_CurrentObject = Object;
	m_CurrentObjectChanged = true;

	if(IsObjectValid(m_CurrentObject))
	{
		ned_MarkObjectInUse(m_CurrentObject,true);
	}	
}

// ======================
// SetCurrentLevelObject
// ======================
//
// Marks the current object for the Object Dialog
void CObjectDialogBar::SetCurrentLevelObject(int Object)
{
	object *oldobjp;

	if( (Object<-1 || Object>Highest_object_index) )
	{
		Int3();//bad object
		return;
	}

	//mark old object unused here
	if (m_CurrentLevelObject != -1)
	{
		oldobjp = &Objects[m_CurrentLevelObject];
		if (IS_GENERIC(oldobjp->type))
			ned_MarkObjectInUse(oldobjp->id,false);
		m_CurrentLevelObject = -1;
	}

	m_CurrentLevelObject = Object;
	m_CurrentLevelObjectChanged = true;

	if (m_CurrentLevelObject != -1)
	{
		//set new object used here
		oldobjp = &Objects[m_CurrentLevelObject];
		if (IS_GENERIC(oldobjp->type))
			ned_MarkObjectInUse(oldobjp->id,true);
	}
}

#ifdef _DEBUG
void CObjectDialogBar::UpdateLevelObjectCount(char *name,int newnum,int oldnum)
{
	ASSERT(name);
	char listing[PAGENAME_LEN+10];

	sprintf(listing,"%d %s",oldnum,name);

	int index = m_LevelDialog->m_List.FindStringExact(0,listing);
	if(index<0)
	{
		Int3();
		return;
	}

	m_LevelDialog->m_List.DeleteString(index);

	sprintf(listing,"%d %s",newnum,name);

	m_LevelDialog->m_List.AddString(listing);
}
#endif

void CObjectDialogBar::AddRemoveLevelObject(char *name,int num,bool add)
{
	ASSERT(name);
	char listing[PAGENAME_LEN+10];

#ifdef _DEBUG
	sprintf(listing,"%d %s",num,name);
#else
	strcpy(listing,name);
#endif

	if(add)
	{
		m_LevelDialog->m_List.AddString(listing);
	}else
	{
		int index = m_LevelDialog->m_List.FindStringExact(0,listing);
		if(index<0)
		{
			Int3();
			return;
		}

		m_LevelDialog->m_List.DeleteString(index);
	}
}

short Level_objects_in_use[MAX_OBJECT_IDS];
void lobj_init(void)
{
	for(int i=0;i<MAX_OBJECT_IDS;i++)
	{
		Level_objects_in_use[i] = 0;
	}
}

// =========================
// LevelObjIncrementObject
// =========================
//
// Call this function whenever an object is added to a level
// _EVERYTIME_ it is added to a level.  Make sure it is called on level load
// ONLY CALL THIS WITH GENERIC OBJECTS!
void LevelObjIncrementObject(int objnum)
{
	if(objnum<0 || objnum>=MAX_OBJECT_IDS || !Object_info[objnum].used)
	{
		//bad object coming in
		Int3();
		return;
	}

	Level_objects_in_use[objnum]++;
	ned_MarkObjectInUse(objnum,true);

	ASSERT_VALID(dlgObjectDialogBar);
	if(Level_objects_in_use[objnum]==1)
	{
		//our first add
		dlgObjectDialogBar->AddRemoveLevelObject(Object_info[objnum].name,Level_objects_in_use[objnum],true);
	}
#ifdef _DEBUG
	else
	{
		//update the count record
		dlgObjectDialogBar->UpdateLevelObjectCount(Object_info[objnum].name,Level_objects_in_use[objnum],Level_objects_in_use[objnum]-1);
	}
#endif
}

// ===========================
// LevelObjDecrementObject
// ===========================
//
// Call this function when a Object is being removed from the level.
// ONLY CALL THIS WITH GENERIC OBJECTS!
void LevelObjDecrementObject(int objnum)
{
	if(objnum<0 || objnum>=MAX_OBJECT_IDS || !Object_info[objnum].used)
	{
		//bad object coming in
		Int3();
		return;
	}

	int save = Level_objects_in_use[objnum]--;
	ASSERT(Level_objects_in_use[objnum]>=0);

	ned_MarkObjectInUse(objnum,false);

	ASSERT_VALID(dlgObjectDialogBar);
	if(Level_objects_in_use[objnum]==0)
	{
		//it's gone from the level...delete
		dlgObjectDialogBar->AddRemoveLevelObject(Object_info[objnum].name,save,false);		
	}
#ifdef _DEBUG
	else
	{
		//update the count record
		dlgObjectDialogBar->UpdateLevelObjectCount(Object_info[objnum].name,save-1,save);
	}
#endif
}

// ==========================
// LevelObjPurgeAllObjects
// ==========================
//
// Call this when a level is being unloaded to purge all objects associated with it
void LevelObjPurgeAllObjects(void)
{
	int j,size;
	for(int i=0;i<MAX_OBJECT_IDS;i++)
	{
		size = Level_objects_in_use[i];
		for(j=0;j<size;j++)
		{
			ned_MarkObjectInUse(i,false);
		}

		Level_objects_in_use[i] = 0;

		if(size>0)
		{
			ASSERT_VALID(dlgObjectDialogBar);
			dlgObjectDialogBar->AddRemoveLevelObject(Object_info[i].name,size,false);
		}
	}
}

// ==========================
// LevelObjInitLevel
// ==========================
//
// Call this as soon as a level is loaded, it will go through the level and set it up.
void LevelObjInitLevel(void)
{
	int rm;
	room *rp;
	object *objp;

	// Room objects
	for(rm = 0; rm < MAX_ROOMS; rm++)
	{
		rp = &Rooms[rm];
		if (rp->used && rp->objects != -1)
		{
			objp = &Objects[rp->objects];
			if (IS_GENERIC(objp->type))
				LevelObjIncrementObject(objp->id);
			while (objp->next != -1)
			{
				objp = &Objects[objp->next];
				if (IS_GENERIC(objp->type))
					LevelObjIncrementObject(objp->id);
			}
		}
	}
	// Terrain objects
	for (int i=0;i<=Highest_object_index;i++)
	{
		objp=&Objects[i];
		if (objp->type==OBJ_ROOM)
			continue;
				
		if (objp->flags & OF_DEAD)
			continue;
		if (objp->render_type == RT_NONE)
			continue;
		if (! OBJECT_OUTSIDE(objp))
			continue;

		if (IS_GENERIC(objp->type))
			LevelObjIncrementObject(objp->id);
	}
}

void CObjectDialogBar::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case TIMER_OBJECTS:
		{
			if(m_CurrentObjectChanged)
			{
				//force update the current object
				//change the necessary text
				char objname[PAGENAME_LEN];
				bool enabled = false;					

				if(IsObjectValid(m_CurrentObject))
				{					
					strcpy(objname,Object_info[m_CurrentObject].name);

					enabled = true;
				}else
				{
					//invalid object
					strcpy(objname,"NONE");

					enabled = false;
				}
				//disable/enable object buttons
				CWnd *wnd;
				wnd = GetDlgItem(IDC_CO_PROPERTIES);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_BUTTON1);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_BUTTON2);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_BUTTON4);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);
				
				wnd = GetDlgItem(IDC_CO_OBJECTNAME);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(objname);

				RedrawPreviewArea(&m_COView);
				UpdateWindow();
				m_CurrentObjectChanged = false;
			}

			//force update the current level object
			short obj_to_use = -1;

			// object type check
			if(m_CurrentLevelObject>=0 && m_CurrentLevelObject<=Highest_object_index)
			{

				if (IS_GENERIC(Objects[m_CurrentLevelObject].type))
				{
					obj_to_use = Objects[m_CurrentLevelObject].id;
				}

			}

			if(m_CurrentLevelObjectChanged)
			{
				//change the necessary text
				char buffer[PAGENAME_LEN+OBJ_NAME_LEN+1] = "";
				CWnd *wnd;

				wnd = GetDlgItem(IDC_CLO_PROPERTIES);

				if(IsObjectValid(obj_to_use))
				{
					if (Objects[m_CurrentLevelObject].name && strcmp(Objects[m_CurrentLevelObject].name, ""))
						sprintf(buffer,"%s \"%s\"",Object_info[obj_to_use].name,Objects[m_CurrentLevelObject].name);
					else
						strcpy(buffer,Object_info[obj_to_use].name);
				
					//wnd is the properties button
					if(wnd != NULL && IsWindow(wnd->m_hWnd))
						wnd->EnableWindow(true);
				}else
				{
					//invalid object
					strcpy(buffer,"NONE");

					//wnd is the properties button
					if(wnd != NULL && IsWindow(wnd->m_hWnd))
						wnd->EnableWindow(false);
				}
				wnd = GetDlgItem(IDC_CLO_OBJECTNAME);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(buffer);

				RedrawPreviewArea(&m_CLOView);
				UpdateWindow();
				m_CurrentLevelObjectChanged = false;
			}

			if (m_CurrentPlayerChanged)
			{
				char str[10];
				// Right align text
				if (m_current_start_pos > -1 && m_current_start_pos < 10)
					sprintf(str," %d",m_current_start_pos);
				else
					sprintf(str,"%d",m_current_start_pos);
				SetDlgItemText(IDC_PLAYER_NUMBER,str);
				SetPlayerChecks();
			}
		}break;
	}
	
	CDialogBar::OnTimer(nIDEvent);
}


void CObjectDialogBar::OnCoProperties() 
{
	if (!IsObjectValid(m_CurrentObject))
		return;

	//Current Object Properties
	CObjectProperties dlg(m_CurrentObject,this);
	dlg.DoModal();
}

void CObjectDialogBar::OnCloProperties() 
{
	if (m_CurrentLevelObject==-1)
		return;

	//Current Level Object properties
	CObjectProperties dlg(Objects[m_CurrentLevelObject].id,this);
	dlg.DoModal();	
}

/////////////////////////////////////////////////////////////////////////////
// CObjectProperties dialog


CObjectProperties::CObjectProperties(int ObjectSlot,CWnd* pParent /*=NULL*/)
	: CDialog(CObjectProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectProperties)
	//}}AFX_DATA_INIT
	m_Object = ObjectSlot;
}


void CObjectProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectProperties)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CObjectProperties, CDialog)
	//{{AFX_MSG_MAP(CObjectProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectProperties message handlers

BOOL CObjectProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// check the object, make sure it's valid
	if(!IsObjectValid(m_Object))
	{
		// invalid object
		CDialog::OnCancel();
	}

	// set our title bar
	char title[256];
	sprintf(title,"Object Properties: %s",Object_info[m_Object].name);
	SetWindowText(title);

	// set name,type,id fields
	SetDlgItemText(IDC_EDITFILENAME,Object_info[m_Object].image_filename);
	int type = Object_info[m_Object].type;
	char *str;
	switch (type)
	{
	case OBJ_POWERUP:
		str = "Powerup";
		break;
	case OBJ_ROBOT:
		str = "Robot";
		break;
	case OBJ_CLUTTER:
		str = "Clutter";
		break;
	case OBJ_BUILDING:
		str = "Building";
		break;
	default:
		str = "Other";
		break;
	}
	SetDlgItemText(IDC_EDITTYPE,str);
	SetDlgItemInt(IDC_EDITID,m_Object);
	// Get number of timed animation frames in the polymodel
	int num_frames = -1;
	ASSERT(Object_info[m_Object].ref_count>=0);
	if(Object_info[m_Object].render_handle>=0)
	{
		PageInPolymodel (Object_info[m_Object].render_handle);
		// Mark the polymodel textures in use
		poly_model *pm = GetPolymodelPointer(Object_info[m_Object].render_handle);
		if (pm->flags & PMF_TIMED)
			num_frames = pm->frame_max-pm->frame_min;
	}
	SetDlgItemInt(IDC_EDITFRAMES,num_frames);

	return TRUE;
}

void CObjectDialogBar::OnObjectInsert() 
{
	// TODO: Add your control notification handler code here
	int ret;
	vector pos;
	void *wnd = theApp.AcquireWnd();

	if (wnd != NULL && theApp.m_pLevelWnd != NULL && theApp.m_pLevelWnd->m_Prim.roomp != NULL)
	{
		if (wnd == theApp.m_pRoomFrm || (wnd == theApp.m_pLevelWnd && Insert_mode == 1))
		{
		room *rp = theApp.m_pLevelWnd->m_Prim.roomp;
		int facenum = theApp.m_pLevelWnd->m_Prim.face;
//		ComputeRoomCenter(&pos,rp);

		int obj_id = GetCurrentObject();
		if (obj_id != -1)
		{
/*
			switch (wnd->m_Focused_pane)
			{
			case 0:
				wnd->m_vec_InsertPos.z = pos.z;
				break;
			case 1:
				wnd->m_vec_InsertPos.y = pos.y;
				break;
			case 2:
				wnd->m_vec_InsertPos.x = pos.x;
				break;
			default:
				return;
			}
*/
			matrix orient = IDENTITY_MATRIX;
			if (Insert_mode == 1)
				pos = theApp.m_pLevelWnd->m_Cam.target - (theApp.m_pLevelWnd->m_Cam.orient.fvec * theApp.m_pLevelWnd->m_Cam.dist);
			else
				pos = ((CRoomFrm *)wnd)->m_vec_InsertPos;
			ret = InsertObject(rp,facenum,obj_id,pos,orient);
			if (ret == -1)
				mprintf((0, "Attempt to place object outside mine failed!\n"));
			else
				OutrageMessageBox("Object inserted at %d, %d, %d",(int)pos.x,(int)pos.y,(int)pos.z);

			World_changed = 1;
		}
		}
		else
			OutrageMessageBox("You must be in the current room view to insert objects.");
	}
}


void DrawItemModel(int x,int y,grSurface *ds,grHardwareSurface *surf,int model_num,bool draw_large)
{
	vector zero_vector={0,0,0};
	vector view_pos={0,0,0};
	matrix id_matrix,rot_matrix;
	float dist_factor = 2.5;

	if (model_num != -1)
	{
		poly_model *pm = GetPolymodelPointer(model_num);

		//Calculate viewer position
		vector maxs = pm->maxs, mins = pm->mins;
		view_pos.x = -(mins.x + maxs.x) / 2;
		view_pos.y = (mins.y + maxs.y) / 2;
		maxs.x += view_pos.x; maxs.y -= view_pos.y; maxs.z = 0;
		if (draw_large)
			dist_factor = 1.5;
		view_pos.z = -dist_factor * vm_GetMagnitude(&maxs);

		//Set viewer and object orientations
		vm_MakeIdentity(&id_matrix);
		vm_AnglesToMatrix(&rot_matrix,0,32768,0);
	}

	//Set up surface & vuewport
	surf->clear();
	grViewport *vport = new grViewport(surf);
	vport->clear();

	if (model_num != -1)
	{
		//Draw to our surface
		StartEditorFrame(vport,&view_pos,&id_matrix,D3_DEFAULT_ZOOM);
		DrawPolygonModel(&zero_vector,&rot_matrix,model_num,NULL,0,1.0,1.0,1.0);
		EndEditorFrame();
	}

	//Copy to the screen
	ds->blt(x,y,surf);

	//Delete our viewport
	delete vport;
}


void CObjectBarList::OnContextMenu(CWnd* pWnd, CPoint point) 
{
	// TODO: Add your message handler code here
	//Load top-level menu from resource
	CMenu mnuTop;
	mnuTop.LoadMenu(IDR_BARLIST_POPUP);

	//get popup menu from first submenu
	CMenu *pPopup = mnuTop.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	//Checked state for popup menu items is automatically managed by MFC
	//UPDATE_COMMAND_UI 

	//Display popup menu
	pPopup->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON,point.x,point.y,this,NULL);

//	CWnd::OnContextMenu(pWnd, point);
}

void CObjectBarList::OnAddToCustom() 
{
	// TODO: Add your command handler code here
	int sel = m_List.GetCurSel();
	if(sel<0)
		return;

	CString buffer;
	m_List.GetText(sel,buffer);
	char *objstr;
	char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
	if (this != m_ParentWindow->m_CustomDialog) // quick hack
	{
		objstr = strtok(buffer.GetBuffer(0)," ");
		objstr += strlen(objstr) + 1;
	}
	else
	{
		strcpy(str,buffer.GetBuffer(0));
		objstr = str;
	}
#else
	strcpy(str,buffer.GetBuffer(0));
	objstr = str;
#endif

	int objid = ned_FindObjectID(objstr);
	if (objid != -1)
		m_ParentWindow->m_CustomDialog->AddObjectToList(objid);
}

void CObjectBarList::OnUpdateAddToCustom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CObjectBarList::OnRemove() 
{
	// TODO: Add your command handler code here
	int sel = m_List.GetCurSel();
	if(sel<0)
		return;

	CString buffer;
	m_List.GetText(sel,buffer);
	char *objstr;
	char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
	if (this != m_ParentWindow->m_CustomDialog) // quick hack
	{
		objstr = strtok(buffer.GetBuffer(0)," ");
		objstr += strlen(objstr) + 1;
	}
	else
	{
		strcpy(str,buffer.GetBuffer(0));
		objstr = str;
	}
#else
	strcpy(str,buffer.GetBuffer(0));
	objstr = str;
#endif

	int objid = ned_FindObjectID(objstr);
	if (objid != -1)
	{
		if (this == m_ParentWindow->m_LevelDialog)
		{
			int answer = MessageBox("Remove all instances of this object from the level?", "Remove object", MB_YESNO);
			if (answer==IDYES)
			{
				for (int i=0; i<=Highest_object_index; i++)
					if (Objects[i].id == objid)
					{
						Cur_object_index = i; // needed for next line
						HObjectDelete();
					}
					m_ParentWindow->SetCurrentLevelObject(Cur_object_index);
			}
		}
		else
			RemoveObjectFromList(objid);
	}
}

void CObjectBarList::OnUpdateRemove(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CObjectDialogBar::OnCloObjectName() 
{
	// TODO: Add your control notification handler code here
	if (Objects[Cur_object_index].type == OBJ_PLAYER) {
		EditorMessageBox("You cannot name a player object.");
		return;
	}

	char tempname[OBJ_NAME_LEN+1] = "";
	object *curobj=&Objects[Cur_object_index];
	if (curobj->name) {
		ASSERT(strlen(curobj->name) <= OBJ_NAME_LEN);
		strcpy(tempname,curobj->name);
	}

try_again:;
	if (! InputString(tempname,OBJ_NAME_LEN,"Object Name","Enter a name for this object:"))
		return;

	if (StripLeadingTrailingSpaces(tempname))
		EditorMessageBox("Note: Leading and/or trailing spaces have been removed from this name (\"%s\")",tempname);

	int handle = osipf_FindObjectName(tempname);
	if ((handle != OBJECT_HANDLE_NONE) && (handle != curobj->handle)) {
		EditorMessageBox("Object %d already has this name.",OBJNUM(ObjGet(handle)));
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
	sprintf(buffer,"%s \"%s\"",Object_info[curobj->id].name,curobj->name);
	SetDlgItemText(IDC_CLO_OBJECTNAME,buffer);
}

int CObjectDialogBar::GetFreePlayerIndex()
{
	ubyte slots[MAX_NET_PLAYERS];

	memset (slots,0,MAX_NET_PLAYERS);

	for (int i=0;i<=Highest_object_index;i++)
	{
		if (Objects[i].type==OBJ_PLAYER)
		{
			// Get Jason if these asserts fail
			ASSERT (slots[Objects[i].id]==0);
			ASSERT (Objects[i].id<MAX_NET_PLAYERS);

			slots[Objects[i].id]=1;
		}
	}

	for (i=0;i<MAX_NET_PLAYERS;i++)
	{
		if (slots[i]==0)
		{
			mprintf ((0,"Making new ship with id of %d...\n",i));
			return i;
		}
	}

	return -1;

}

void CObjectDialogBar::OnButton5() 
{
	// TODO: Add your control notification handler code here
	int ret;
	vector pos;
	void *wnd = theApp.AcquireWnd();

	// Insert a player start
	if (wnd != NULL && theApp.m_pLevelWnd != NULL && theApp.m_pLevelWnd->m_Prim.roomp != NULL)
	{
		if (wnd == theApp.m_pRoomFrm || (wnd == theApp.m_pLevelWnd && Insert_mode == 1))
		{
		room *rp = theApp.m_pLevelWnd->m_Prim.roomp;
		int facenum = theApp.m_pLevelWnd->m_Prim.face;
//		ComputeRoomCenter(&pos,rp);

		int obj_id=GetFreePlayerIndex();
		if (obj_id==-1)
		{
			mprintf ((0,"No more player slots!\n"));
			return;
		}
		if (obj_id != -1)
		{
/*
			switch (wnd->m_Focused_pane)
			{
			case 0:
				wnd->m_vec_InsertPos.z = pos.z;
				break;
			case 1:
				wnd->m_vec_InsertPos.y = pos.y;
				break;
			case 2:
				wnd->m_vec_InsertPos.x = pos.x;
				break;
			default:
				return;
			}
*/
			matrix orient = IDENTITY_MATRIX;
			if (Insert_mode == 1)
				pos = theApp.m_pLevelWnd->m_Cam.target - (theApp.m_pLevelWnd->m_Cam.orient.fvec * theApp.m_pLevelWnd->m_Cam.dist);
			else
				pos = ((CRoomFrm *)wnd)->m_vec_InsertPos;
			ret = InsertObject(rp,facenum,obj_id,pos,orient,true); // insert player
			if (ret == -1)
				mprintf((0, "Attempt to place object outside mine failed!\n"));
			else
				OutrageMessageBox("Object inserted at %d, %d, %d",(int)pos.x,(int)pos.y,(int)pos.z);

			World_changed = 1;
		}
		}
		else
			OutrageMessageBox("You must be in the current room view to insert objects.");
	}
}

void CObjectDialogBar::OnButton6() 
{
	// TODO: Add your control notification handler code here
	HObjectDelete();
	SetCurrentLevelObject(Cur_object_index);
}

//writes an object list to an open file
bool CObjectDialogBar::WriteObjectList(CFILE *outfile,int list)
{
	CObjectBarList *objlst = NULL;
	CString buffer;
	char *objstr;
	char str[PAGENAME_LEN+10] = "";
	int objid;
	int i,count;
	int j=0;

	ASSERT(outfile != NULL);
	if (outfile == NULL)
		return false;

	ASSERT((list == CUSTOM_OBJ_LIST) || (list == LEVEL_OBJ_LIST));
	if (list == CUSTOM_OBJ_LIST)
		objlst = m_CustomDialog;
	else if (list == LEVEL_OBJ_LIST)
		objlst = m_LevelDialog;
	else Int3(); // objlst is NULL

	// Write a blank line
	cf_WriteString(outfile,"");

	// Write out object names
	count = objlst->m_List.GetCount();
	for (i=0; i<count; i++)
	{
		objlst->m_List.GetText(i,buffer);

#ifdef _DEBUG
		if (objlst != m_CustomDialog) // quick hack
		{
			objstr = strtok(buffer.GetBuffer(0)," ");
			objstr += strlen(objstr) + 1;
		}
		else
		{
			strcpy(str,buffer.GetBuffer(0));
			objstr = str;
		}
#else
		strcpy(str,buffer.GetBuffer(0));
		objstr = str;
#endif

		objid = ned_FindObjectID(objstr);
		if (objid != -1)
		{
/*
			found = false;
			while (j++ < MAX_OBJECT_IDS)
			{
				cf_ReadString(str,sizeof(str),outfile);
				if ( !strcmp(str,objstr) )
				{
					found = true;
					break;
				}
			}
			if (!found)
*/
				cf_WriteString(outfile,Object_info[objid].name);
		}
	}

	return true;
}

//reads an object list from an open file
int CObjectDialogBar::ReadObjectList(CFILE *infile)
{
	char filebuffer[MAX_DER_FILE_BUFFER_LEN];
	CObjectBarList *objlst = NULL;
	CString buffer;
	char str[PAGENAME_LEN+10] = "";
	int objid;
	int line_num = 0;
	int filepos;

	ASSERT(infile != NULL);
	if (infile == NULL)
		return false;

	objlst = m_CustomDialog;

	int count = objlst->m_List.GetCount();
	if (count != 0)
	{
		int answer = MessageBox("Choose Yes to replace the items in the list. Choose No to append.","Replace Items?",MB_YESNO);
		if (answer == IDYES)
			objlst->m_List.ResetContent();
	}

	bool done = 0;

	// Parse objects
	while (!done && !cfeof(infile))
	{
		filepos = cftell(infile);
		cf_ReadString(filebuffer,sizeof(filebuffer),infile);
		line_num++;

		// Remove whitespace padding at start and end of line
		StripLeadingTrailingSpaces(filebuffer);

		// If line is a comment, or empty, discard it
		if (strlen(filebuffer) == 0 || strncmp(filebuffer,"//",2)==0)
			continue;

		// If line begins with '[', a new block is starting, so we're done
		if (*filebuffer == '[')
		{
			done = 1;
			cfseek(infile,filepos,SEEK_SET);
		}
		else
		{
			// Verify that the string is a valid object name for the loaded tablefile(s), 
			// then add it to the list
			objid = ned_FindObjectID(filebuffer);
			if (objid != -1)
				objlst->AddObjectToList(objid);
		}
	}

	return line_num;
}


void CObjectBarList::OnSaveList() 
{
	// TODO: Add your command handler code here
	DWORD flags = 0;

	if (this == m_ParentWindow->m_CustomDialog)
		flags |= CUSTOM_OBJ_LIST;
	else if (this == m_ParentWindow->m_LevelDialog)
		flags |= LEVEL_OBJ_LIST;

	DlgSaveResourceLists(flags);
}

void CObjectBarList::OnUpdateSaveList(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CObjectBarList::OnLoadList() 
{
	// TODO: Add your command handler code here
	if (this == m_ParentWindow->m_CustomDialog)
		DlgOpenResourceLists(CUSTOM_OBJ_LIST);
	else
		MessageBox("Object lists can only be opened in the custom tab.");
}

void CObjectBarList::OnUpdateLoadList(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CObjectDialogBar::OnPlayerDelete() 
{
	// TODO: Add your control notification handler code here
	int save = Cur_object_index;
	Cur_object_index = Players[m_current_start_pos].objnum;
	HObjectDelete();
	Cur_object_index = save;
}

void CObjectDialogBar::OnPlayerRed() 
{
	// TODO: Add your control notification handler code here
	int cur=m_current_start_pos;

	int c=IsDlgButtonChecked(IDC_PLAYER_RED);
	
	if (c)
		Players[cur].startpos_flags |=PSPF_RED;
	else
		Players[cur].startpos_flags &=~PSPF_RED;
}

void CObjectDialogBar::OnPlayerBlue() 
{
	// TODO: Add your control notification handler code here
	int cur=m_current_start_pos;

	int c=IsDlgButtonChecked(IDC_PLAYER_BLUE);
	
	if (c)
		Players[cur].startpos_flags |=PSPF_BLUE;
	else
		Players[cur].startpos_flags &=~PSPF_BLUE;
}

void CObjectDialogBar::OnPlayerGreen() 
{
	// TODO: Add your control notification handler code here
	int cur=m_current_start_pos;

	int c=IsDlgButtonChecked(IDC_PLAYER_GREEN);
	
	if (c)
		Players[cur].startpos_flags |=PSPF_GREEN;
	else
		Players[cur].startpos_flags &=~PSPF_GREEN;
}

void CObjectDialogBar::OnPlayerYellow() 
{
	// TODO: Add your control notification handler code here
	int cur=m_current_start_pos;

	int c=IsDlgButtonChecked(IDC_PLAYER_YELLOW);
	
	if (c)
		Players[cur].startpos_flags |=PSPF_YELLOW;
	else
		Players[cur].startpos_flags &=~PSPF_YELLOW;
}

void CObjectDialogBar::OnPlayerNext() 
{
	// TODO: Add your control notification handler code here
	m_current_start_pos++;

	if (m_current_start_pos>=MAX_PLAYERS)
		m_current_start_pos=0;

	m_CurrentPlayerChanged = true;

	SetPlayerChecks();
}

void CObjectDialogBar::OnPlayerPrev() 
{
	// TODO: Add your control notification handler code here
	if (m_current_start_pos<=0)
		m_current_start_pos=MAX_PLAYERS-1;
	else
		m_current_start_pos--;
	m_CurrentPlayerChanged = true;

	SetPlayerChecks();
}

void CObjectDialogBar::SetPlayerChecks()
{
	int cur=m_current_start_pos;

	CheckDlgButton (IDC_PLAYER_RED,Players[cur].startpos_flags & PSPF_RED);
	CheckDlgButton (IDC_PLAYER_BLUE,Players[cur].startpos_flags & PSPF_BLUE);
	CheckDlgButton (IDC_PLAYER_GREEN,Players[cur].startpos_flags & PSPF_GREEN);
	CheckDlgButton (IDC_PLAYER_YELLOW,Players[cur].startpos_flags & PSPF_YELLOW);
}


// Convert objects (swap)
void CObjectDialogBar::OnButton1() 
{
	// TODO: Add your control notification handler code here
	object *objp;
	int src_id,dest_id,src_type;
	int i;

	dest_id = m_CurrentObject;

	if (m_CurrentLevelObject != -1)
	{
		src_id = Objects[m_CurrentLevelObject].id;
		src_type = Objects[m_CurrentLevelObject].type;

		if ( !IS_GENERIC(src_type) )
		{
			OutrageMessageBox("The destination type: %s, cannot be converted.",Object_type_names[src_type]);
			return;
		}
	}
	else
	{
		OutrageMessageBox("No current level object!");
		return;
	}

	if(dest_id >= 0 && m_CurrentLevelObject != -1)
	{
		for(i=0,objp=Objects; i<=Highest_object_index; i++,objp++)
		{
			if( IS_GENERIC(objp->type) )
			{
				if(objp->id == src_id)
				{
					vector pos = objp->pos;
					matrix orient = objp->orient;
					int room = objp->roomnum;
					int parent = objp->parent_handle;

					char temp_name[256];
					temp_name[0] = '\0';

					if(objp->name)
					{
						strcpy(temp_name, objp->name);
					}

					Cur_object_index = OBJNUM(objp);
					HObjectDelete();

					int o_index = InsertObject(&Rooms[room],-1,dest_id,pos,orient);

					if(temp_name[0] != '\0')
					{
						Objects[o_index].name = (char *)mem_malloc(strlen(temp_name) + 1);
						strcpy(Objects[o_index].name, temp_name);
					}
				}
			}
		}
	}

	SetCurrentLevelObject(Cur_object_index);
	World_changed = 1;
}

void CObjectBarList::OnRemoveAll() 
{
	// TODO: Add your command handler code here
	ASSERT(this == m_ParentWindow->m_CustomDialog);
	if (this != m_ParentWindow->m_CustomDialog)
		return;

	m_List.ResetContent();
}

void CObjectBarList::OnUpdateRemoveAll(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool bEnable = false;

	if (this == m_ParentWindow->m_CustomDialog)
		bEnable = true;

	pCmdUI->Enable(bEnable);
}

void CObjectDialogBar::OnInsertMine() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_INSERT_MINE);
	
	if (c)
		Insert_mode = 0;
}

void CObjectDialogBar::OnInsertTerrain() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_INSERT_TERRAIN);
	
	if (c)
		Insert_mode = 1;
}
