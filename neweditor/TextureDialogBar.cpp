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
 // TextureDialogBar.cpp : implementation file
//

#include "stdafx.h"

#include "../editor/HTexture.h"

#include "globals.h"
#include "neweditor.h"
#include "TextureDialogBar.h"
#include "TexturePalette.h"
#include "TexAlignDialog.h"
#include "room_external.h"
#include "terrain.h"
#include "mono.h"
#include "ned_GameTexture.h"
#include "ned_Util.h"

#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextureDialogBar dialog

inline bool IsTextureValid(int id)
{
	if(id<0 || id>=MAX_TEXTURES || !GameTextures[id].used)
		return false;
	return true;
}

void ltex_init(void);


CTextureDialogBar::CTextureDialogBar()
{
	//{{AFX_DATA_INIT(CTextureDialogBar)
	//}}AFX_DATA_INIT
	m_pTexturePalette = NULL;
	m_CurrentTexture = -1;
	m_CurrentFaceTexture = -1;
	m_CurrentRoom = -1;
	m_CurrentTextureChanged = true;
	m_CurrentFaceTextureChanged = true;
}


void CTextureDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureDialogBar)
	DDX_Control(pDX, IDC_CT_LIGHT_VALUE_RED, m_CTLightValueRed);
	DDX_Control(pDX, IDC_CT_LIGHT_VALUE_GREEN, m_CTLightValueGreen);
	DDX_Control(pDX, IDC_CT_LIGHT_VALUE_BLUE, m_CTLightValueBlue);
	DDX_Control(pDX, IDC_CF_LIGHT_VALUE_RED, m_CFLightValueRed);
	DDX_Control(pDX, IDC_CF_LIGHT_VALUE_GREEN, m_CFLightValueGreen);
	DDX_Control(pDX, IDC_CF_LIGHT_VALUE_BLUE, m_CFLightValueBlue);
	DDX_Control(pDX, IDC_CT_LIGHT_COLOR, m_CTLightView);
	DDX_Control(pDX, IDC_CF_LIGHT_COLOR, m_CFLightView);
	DDX_Control(pDX, IDC_CT_VIEW, m_CTView);
	DDX_Control(pDX, IDC_CF_VIEW, m_CFView);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextureDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CTextureDialogBar)
	ON_WM_PAINT()
	ON_UPDATE_COMMAND_UI(IDC_CF_PROPERTIES,OnControlUpdate)
	ON_BN_CLICKED(IDC_TEXTURE_PALETTE, OnTexturePalette)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CT_PROPERTIES, OnCtProperties)
	ON_BN_CLICKED(IDC_CF_PROPERTIES, OnCfProperties)
	ON_BN_CLICKED(IDC_APPLY_TO_CURRENT, OnApplyToCurrent)
	ON_BN_CLICKED(IDC_APPLY_TO_MARKED, OnApplyToMarked)
	ON_BN_CLICKED(IDC_PROPAGATE, OnPropagate)
	ON_BN_CLICKED(IDC_ALIGNMENT, OnAlignment)
	ON_UPDATE_COMMAND_UI(IDC_CT_PROPERTIES,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_APPLY_TO_CURRENT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_APPLY_TO_MARKED,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_PROPAGATE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_ALIGNMENT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_TEXTURE_PALETTE,OnControlUpdate)
	ON_BN_CLICKED(IDC_GRAB, OnGrab)
	ON_UPDATE_COMMAND_UI(IDC_GRAB,OnControlUpdate)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureBarList dialog
CTextureDialogBar *dlgTextureDialogBar = NULL;

CTextureBarList::CTextureBarList(CWnd* pParent )
	: CDialog(CTextureBarList::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTextureBarList)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT_VALID(dlgTextureDialogBar);
	m_ParentWindow = dlgTextureDialogBar;
	ignore_mark = false;
}


void CTextureBarList::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureBarList)
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextureBarList, CDialog)
	//{{AFX_MSG_MAP(CTextureBarList)
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
// CTextureBarList message handlers

int CTextureBarList::AddTextureToList(int tex_slot)
{
	int slot;
	char listing[PAGENAME_LEN+10];

	if(!IsTextureValid(tex_slot))
		return 0;

#ifdef _DEBUG
	if (this == m_ParentWindow->m_LevelDialog)
		sprintf(listing,"%d %s",Level_textures_in_use[tex_slot],GameTextures[tex_slot].name);
	else if (this == m_ParentWindow->m_CustomDialog)
#endif
		strcpy(listing,GameTextures[tex_slot].name);

	slot = m_List.FindStringExact(0,listing);
	if(slot>=0)
		return 0;

	m_List.AddString(GameTextures[tex_slot].name);
//	ned_MarkTextureInUse(tex_slot,true);

	return 1;
}

int CTextureBarList::RemoveTextureFromList(int tex_slot)
{
	int slot;
	char listing[PAGENAME_LEN+10];

	if(!IsTextureValid(tex_slot))
		return 0;

#ifdef _DEBUG
	if (this == m_ParentWindow->m_LevelDialog)
		sprintf(listing,"%d %s",Level_textures_in_use[tex_slot],GameTextures[tex_slot].name);
	else if (this == m_ParentWindow->m_CustomDialog)
#endif
		strcpy(listing,GameTextures[tex_slot].name);

	slot = m_List.FindStringExact(0,listing);
	if(slot>=0)
	{
		m_List.DeleteString(slot);
//		ned_MarkTextureInUse(tex_slot,false);
		return 1;
	}

	return 0;
}

void CTextureBarList::OnSelchangeList() 
{
	int sel = m_List.GetCurSel();
	if(sel<0)
	{
		//select none
		if(!ignore_mark)		
			m_ParentWindow->SetCurrentTexture(-1);
		ignore_mark = false;
		return;
	}

	CString buffer;
	m_List.GetText(sel,buffer);
	char *texstr;
	char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
	if (this != m_ParentWindow->m_CustomDialog) // quick hack
	{
		texstr = strtok(buffer.GetBuffer(0)," ");
		texstr += strlen(texstr) + 1;
	}
	else
	{
		strcpy(str,buffer.GetBuffer(0));
		texstr = str;
	}
#else
	strcpy(str,buffer.GetBuffer(0));
	texstr = str;
#endif

	int texid = ned_FindTexture(texstr);
	m_ParentWindow->SetCurrentTexture(texid);	
}

void CTextureBarList::OnOK() 
{
	//this is here because since there are no buttons on the list, this is attached to a hidden
	//OK button, so if the user hits enter in the list box, it doesn't close it
}

void CTextureBarList::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);

	int curr_tex = m_ParentWindow->GetCurrentTexture();
	int our_curr = m_List.GetCurSel();
	int our_tex = -1;
	char listing[PAGENAME_LEN+10];

	if(our_curr!=-1)
	{
		CString buffer;
		m_List.GetText(our_curr,buffer);
		char *texstr;
		char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
		if (this != m_ParentWindow->m_CustomDialog) // quick hack
		{
			texstr = strtok(buffer.GetBuffer(0)," ");
			texstr += strlen(texstr) + 1;
		}
		else
		{
			strcpy(str,buffer.GetBuffer(0));
			texstr = str;
		}
#else
		strcpy(str,buffer.GetBuffer(0));
		texstr = str;
#endif

		our_tex = ned_FindTexture(texstr);
	}

	if(our_tex!=curr_tex)
	{
		//our selections don't match...blah, unselect without causing a comotion
		ignore_mark = true;
		m_List.SetCurSel(-1);
		ignore_mark = false;
	}

	if(curr_tex==-1)
		return;

#ifdef _DEBUG
	if (this == m_ParentWindow->m_LevelDialog)
		sprintf(listing,"%d %s",Level_textures_in_use[curr_tex],GameTextures[curr_tex].name);
	else if (this == m_ParentWindow->m_CustomDialog)
#endif
		strcpy(listing,GameTextures[curr_tex].name);

	//see if the curr texture is in our list
	int new_sel = m_List.FindStringExact(0,listing);
	if(new_sel ==-1)
		return;	//not in our list

	//select the new selection
	m_List.SetCurSel(new_sel);		
}


/////////////////////////////////////////////////////////////////////////////
// CTextureDialogBar message handlers

LONG CTextureDialogBar::OnInitDialog ( UINT wParam, LONG lParam)
{
	dlgTextureDialogBar = this;
	ltex_init();

	if ( !HandleInitDialog(wParam, lParam) || !UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
		return FALSE;
	}

	m_TextureTabDialog = NULL;
	m_CustomDialog = NULL;
	m_LevelDialog = NULL;
	//@@m_RoomDialog = NULL;
	m_CurrentTextureTab = TAB_TEXTURE_CUSTOM;

	InitKeyPad();
	ShowCurrentTextureTab();

	//TOOLTIP:m_CFLightValue.EnableToolTips(TRUE);	

	//Setup update timers
	if(!SetTimer(TIMER_TEXTURES,100,NULL))
	{
		mprintf((0,"Unable to create timer\n"));
		Int3();
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CTextureDialogBar::PreTranslateMessage(MSG* pMsg) 
{
	return CDialogBar::PreTranslateMessage(pMsg);
}

// enable controls:
void CTextureDialogBar::OnControlUpdate(CCmdUI* pCmdUI)
{
	bool bEnable = false;

	// Most controls for the texture bar require an open room or level
	switch (pCmdUI->m_nID)
	{
	case IDC_CT_PROPERTIES:
		if (IsTextureValid(m_CurrentTexture))
			bEnable = true;
		break;
	case IDC_TEXTURE_PALETTE:
		bEnable = true;
		break;
	default:
		if (theApp.m_pLevelWnd != NULL || theApp.m_pFocusedRoomFrm != NULL)
			bEnable = true;
		break;
	}

	pCmdUI->Enable(bEnable);
}

void CTextureDialogBar::PaintPreviewArea(CWnd *pWnd)
{
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

	int room_tex = m_CurrentFaceTexture;

	//render here
	if(pWnd==&m_CTLightView)
	{
		//Current Texture Light
		ddgr_color light_col;
		float intensity;

		if(IsTextureValid(m_CurrentTexture))
		{
			float r,g,b;
			r = GameTextures[m_CurrentTexture].r;
			g = GameTextures[m_CurrentTexture].g;
			b = GameTextures[m_CurrentTexture].b;
			// Scaled color
			intensity = max(max(GameTextures[m_CurrentTexture].r,GameTextures[m_CurrentTexture].g),GameTextures[m_CurrentTexture].b);
			if (intensity>=.001)
			{
				r/=intensity;
				g/=intensity;
				b/=intensity;
			}
			light_col = GR_RGB(r*255,g*255,b*255);
		}else
		{
			light_col = GR_RGB(0,0,0);
		}
		//surface.clear(light_col);
		//ds->blt(0, 0, &surface);
	}else if(pWnd==&m_CFLightView)
	{
		//Current Face Light
		ddgr_color light_col;
		float intensity;

		if(IsTextureValid(room_tex))
		{
			float r,g,b;
			r = GameTextures[room_tex].r;
			g = GameTextures[room_tex].g;
			b = GameTextures[room_tex].b;
			// Scaled color
			intensity = max(max(GameTextures[room_tex].r,GameTextures[room_tex].g),GameTextures[room_tex].b);
			if (intensity>=.001)
			{
				r/=intensity;
				g/=intensity;
				b/=intensity;
			}
			light_col = GR_RGB(r*255,g*255,b*255);
		}else
		{
			light_col = GR_RGB(0,0,0);
		}
		//surface.clear(light_col);
		//ds->blt(0, 0, &surface);
	}else if(pWnd==&m_CTView)
	{
		//Current Texture View
		bm_handle = ned_GetTextureBitmap (m_CurrentTexture,0);
		if(bm_handle<BAD_BITMAP_HANDLE)
			bm_handle = BAD_BITMAP_HANDLE;

		CString TextureString;
		GetDlgItemText(IDC_CT_TEXTURENAME,TextureString);

		// Display the bad texture if it's the sample texture
		/*if (bm_handle == BAD_BITMAP_HANDLE && TextureString != "SAMPLE TEXTURE")
			surface.clear(bk_col);
		else
			surface.load(bm_handle);
		ds->blt(0, 0, &surface);*/
	}else if(pWnd==&m_CFView)
	{
		//Current Face View		
		bm_handle = ned_GetTextureBitmap (room_tex,0);
		if(bm_handle<BAD_BITMAP_HANDLE)
			bm_handle = BAD_BITMAP_HANDLE;

		CString TextureString;
		GetDlgItemText(IDC_CF_TEXTURENAME,TextureString);

		// Display the bad texture if it's the sample texture
		/*if (bm_handle == BAD_BITMAP_HANDLE && TextureString != "SAMPLE TEXTURE")
			surface.clear(bk_col);
		else
			surface.load(bm_handle);
		ds->blt(0, 0, &surface);*/
	}else
	{
		Int3();
	}

	//free up
	//surface.free();
	//ds->attach_to_window(NULL);
}

void CTextureDialogBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
		
	PaintPreviewArea(&m_CTLightView);
	PaintPreviewArea(&m_CFLightView);
	PaintPreviewArea(&m_CTView);
	PaintPreviewArea(&m_CFView);
	
	// Do not call CDialogBar::OnPaint() for painting messages
}

void CTextureDialogBar::RedrawPreviewArea(CWnd *pWnd)
{
	ASSERT_VALID(pWnd);

	//Get Placeholder coordinates
	CRect rect;
	pWnd->GetWindowRect(&rect);
	ScreenToClient(&rect);

	//force update
	InvalidateRect(rect,false);	
}

void CTextureDialogBar::RedrawArea()
{
	RedrawPreviewArea(&m_CTLightView);
	RedrawPreviewArea(&m_CFLightView);
	RedrawPreviewArea(&m_CTView);
	RedrawPreviewArea(&m_CFView);
	UpdateWindow();
}

void CTextureDialogBar::TexturePaletteDone()
{
	mprintf((0,"Removing Texture Palette\n"));
	m_pTexturePalette = NULL;
	GetDlgItem(IDC_TEXTURE_PALETTE)->EnableWindow();

	if(m_TextureTabDialog)
	{
		//refresh just incase they added a custom texture which just happens to be the 
		//selected texture
		m_TextureTabDialog->ShowWindow(SW_HIDE);
		m_TextureTabDialog->ShowWindow(SW_SHOW);
	}
}

void CTextureDialogBar::OnTexturePalette() 
{
	// if our modeless child isn't already up, create it and display it
	// otherwise, just set focus to it

	if (m_pTexturePalette == NULL)
	{
		m_pTexturePalette = new CTexturePalette(this);
		if (m_pTexturePalette->Create() == TRUE)
		{
			GetDlgItem(IDC_TEXTURE_PALETTE)->EnableWindow(FALSE);
			m_pTexturePalette->ShowWindow(SW_SHOW);
		}
	}
	else
		m_pTexturePalette->SetActiveWindow();
}

void CTextureDialogBar::InitKeyPad(void)
{
	CTabCtrl *tab_ctrl;
	TC_ITEM tc_item;
//	LONG style;

	tab_ctrl = (CTabCtrl *)GetDlgItem(IDC_TEXTURE_GROUP_TAB);

	//	Add tabs to tabcontrol in the dialog bar.
	tc_item.mask = TCIF_TEXT;
	tc_item.pszText = "Custom";
	tc_item.cchTextMax = lstrlen(tc_item.pszText)+1;
	tab_ctrl->InsertItem(TAB_TEXTURE_CUSTOM, &tc_item);

	tc_item.mask = TCIF_TEXT;
	tc_item.pszText = "Level";
	tc_item.cchTextMax = lstrlen(tc_item.pszText)+1;
	tab_ctrl->InsertItem(TAB_TEXTURE_LEVEL, &tc_item);	

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
	m_CustomDialog = new CTextureBarList(tab_ctrl);
	m_CustomDialog->Create(IDD_BAR_LIST,tab_ctrl);
	m_CustomDialog->SetWindowPos(tab_ctrl, tx, ty, tw, th, SWP_NOZORDER);
/*
	style = GetWindowLong(m_CustomDialog->m_hWnd,GWL_STYLE);
	style |= WS_VSCROLL;
	SetWindowLong(m_CustomDialog->m_hWnd,GWL_STYLE,style);
*/
	//only for the custom dialog do we set the OLE reciever
	bool oleret = Editor_state.RegisterTextureDropWindow(m_CustomDialog);

	m_LevelDialog = new CTextureBarList(tab_ctrl);
	m_LevelDialog->Create(IDD_BAR_LIST,tab_ctrl);
	m_LevelDialog->SetWindowPos(tab_ctrl, tx, ty, tw, th, SWP_NOZORDER);
/*
	style = GetWindowLong(m_CustomDialog->m_hWnd,GWL_STYLE);
	style |= WS_VSCROLL;
	SetWindowLong(m_CustomDialog->m_hWnd,GWL_STYLE,style);
*/
}

//	Display current tab dialog	
void CTextureDialogBar::ShowCurrentTextureTab()
{
	m_TextureTabDialog = NULL;
	switch (m_CurrentTextureTab) 
	{
		case TAB_TEXTURE_CUSTOM:	m_TextureTabDialog = m_CustomDialog; break;
		case TAB_TEXTURE_LEVEL:		m_TextureTabDialog = m_LevelDialog; break;
		//@@case TAB_TEXTURE_ROOM:		m_TextureTabDialog = m_RoomDialog; break;
		default: Int3();
	}

	if (m_TextureTabDialog) {
		CTabCtrl *tab_ctrl = (CTabCtrl *)GetDlgItem(IDC_TEXTURE_GROUP_TAB);

	  	m_TextureTabDialog->ShowWindow(SW_SHOW);
		tab_ctrl->SetCurSel(m_CurrentTextureTab);
	}
}

void CTextureDialogBar::DoTextureTabNotify(NMHDR *nmhdr)
{
	CTabCtrl *tab_ctrl;
		
	tab_ctrl = (CTabCtrl *)GetDlgItem(IDC_TEXTURE_GROUP_TAB);

	switch (nmhdr->code)
	{
		case TCN_SELCHANGE:				
		// Tab control changed. So update the controls currently displayed.
			m_CurrentTextureTab = (int)tab_ctrl->GetCurSel(); 
			assert(m_CurrentTextureTab > -1);

			switch (m_CurrentTextureTab)
			{
				case TAB_TEXTURE_CUSTOM:
					if (m_TextureTabDialog) m_TextureTabDialog->ShowWindow(SW_HIDE);
					m_TextureTabDialog = m_CustomDialog;
					m_TextureTabDialog->ShowWindow(SW_SHOW);
					break;

				case TAB_TEXTURE_LEVEL:
					if (m_TextureTabDialog) m_TextureTabDialog->ShowWindow(SW_HIDE);
					m_TextureTabDialog = m_LevelDialog;
					m_TextureTabDialog->ShowWindow(SW_SHOW);
					break;
			}
			break;
	}
}

BOOL CTextureDialogBar::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
//	Perform notification requests on the keypad tab dialog bar and other 
//	custom control bars.
	NMHDR *nmhdr;

	assert(lParam != 0);
	
	nmhdr = (NMHDR *)lParam;

	switch (wParam)
	{
		case IDC_TEXTURE_GROUP_TAB:
			DoTextureTabNotify(nmhdr);
			break;
	}
	
	return CDialogBar::OnNotify(wParam, lParam, pResult);
}

void _texdlg_destroytexlist(CTextureBarList *wnd)
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
		char *texstr;
		char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
		if (wnd != wnd->m_ParentWindow->m_CustomDialog) // quick hack (made custom dialog public too; only temporary)
		{
			texstr = strtok(buffer.GetBuffer(0)," ");
			texstr += strlen(texstr) + 1;
		}
		else
		{
			strcpy(str,buffer.GetBuffer(0));
			texstr = str;
		}
#else
		strcpy(str,buffer.GetBuffer(0));
		texstr = str;
#endif

		id = ned_FindTexture(texstr);
		ASSERT(id!=-1);

		wnd->RemoveTextureFromList(id);
	}

	wnd->DestroyWindow();
	delete wnd;
}

void CTextureDialogBar::OnDestroy() 
{
	CDialogBar::OnDestroy();
	
	dlgTextureDialogBar = NULL;

	if(m_CustomDialog){
		_texdlg_destroytexlist(m_CustomDialog);
		m_CustomDialog = NULL;
	}	

	if(m_LevelDialog){
		_texdlg_destroytexlist(m_LevelDialog);
		m_LevelDialog = NULL;
	}	
}

// ======================
// GetCurrentTexture
// ======================
//
// Returns the currently selected texture
int CTextureDialogBar::GetCurrentTexture(void)
{
	int tex;

	(m_CurrentTexture != -1) ? tex = m_CurrentTexture : tex = 0;

	return tex;
}


// ======================
// SetCurrentTexture
// ======================
//
// Marks a texture the current texture to work with in the editor
void CTextureDialogBar::SetCurrentTexture(int Texture)
{
	if(Texture<-1 || Texture>=MAX_TEXTURES)
	{
		Int3();
		return;
	}

	if(Texture!=-1 && !GameTextures[Texture].used)
	{
		Int3();
		return;
	}

	if(IsTextureValid(m_CurrentTexture))
	{
		ned_MarkTextureInUse(m_CurrentTexture,false);
		m_CurrentTexture = -1;
	}

	m_CurrentTexture = Texture;
	m_CurrentTextureChanged = true;

	if(IsTextureValid(m_CurrentTexture))
	{
		ned_MarkTextureInUse(m_CurrentTexture,true);
	}	
}

// ======================
// SetCurrentFace
// ======================
//
// Marks the current room/face for the Texture Dialog
void CTextureDialogBar::SetCurrentFace(int Room, int Face)
{
	//Do Room/Face checks here
	room *rp = NULL;

	if (Room != -1)
	{
		if(Room<-1 || Room>=MAX_ROOMS+MAX_PALETTE_ROOMS || !Rooms[Room].used)
		{
			Int3();//bad room
			return;
		}

		rp = &Rooms[Room];
	}

	if (rp != NULL)
	{
		if(Face<-1 || Face>=rp->num_faces)
		{
			Int3();//bad face
			return;
		}
	}

	//mark old room/face texture unused here
	if(m_CurrentRoom!=-1 && m_CurrentFaceTexture!=-1)
		ned_MarkTextureInUse(m_CurrentFaceTexture,false);

	m_CurrentRoom = Room;
	(Face != -1) ? (m_CurrentFaceTexture = rp->faces[Face].tmap) : (m_CurrentFaceTexture = -1);
	m_CurrentFaceTextureChanged = true;

	//set new room/face texture used here
	if(m_CurrentRoom!=-1 && m_CurrentFaceTexture!=-1)
		ned_MarkTextureInUse(m_CurrentFaceTexture,true);
}

#ifdef _DEBUG
void CTextureDialogBar::UpdateLevelTextureCount(char *name,int newnum,int oldnum)
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

void CTextureDialogBar::AddRemoveLevelTexture(char *name,int num,bool add)
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


short Level_textures_in_use[MAX_TEXTURES];
void ltex_init(void)
{
	for(int i=0;i<MAX_TEXTURES;i++)
	{
		Level_textures_in_use[i] = 0;
	}
}

// =========================
// LevelTexIncrementTexture
// =========================
//
// Call this function whenever a texture is applied to a face in the level
// _EVERYTIME_ it is applied to a face.  Make sure it is called on level load
void LevelTexIncrementTexture(int texnum)
{
	if(texnum<0 || texnum>=MAX_TEXTURES || !GameTextures[texnum].used)
	{
		//bad texture coming in
		Int3();
		return;
	}

	if(!stricmp(GameTextures[texnum].name,"SAMPLE TEXTURE"))
	{
		//there shouldn't be a bad texture
		//Int3();
		return;
	}

	int save = Level_textures_in_use[texnum]++;
	ned_MarkTextureInUse(texnum,true);

	ASSERT_VALID(dlgTextureDialogBar);
	if(Level_textures_in_use[texnum]==1)
	{
		//our first add
		dlgTextureDialogBar->AddRemoveLevelTexture(GameTextures[texnum].name,Level_textures_in_use[texnum],true);
	}
#ifdef _DEBUG
	else
	{
		//update the count record
		dlgTextureDialogBar->UpdateLevelTextureCount(GameTextures[texnum].name,Level_textures_in_use[texnum],Level_textures_in_use[texnum]-1);
	}
#endif
}


// ===========================
// LevelTexDecrementTexture
// ===========================
//
// Call this function when a texture is being removed from a face in the level (i.e it's
// being retextured).
void LevelTexDecrementTexture(int texnum)
{
	if(texnum<0 || texnum>=MAX_TEXTURES || !GameTextures[texnum].used)
	{
		//bad texture coming in
		Int3();
		return;
	}

	if(!stricmp(GameTextures[texnum].name,"SAMPLE TEXTURE"))
	{
		//there shouldn't be a bad texture
		//Int3();
		return;
	}

	int save = Level_textures_in_use[texnum]--;
	ASSERT(Level_textures_in_use[texnum]>=0);

	ned_MarkTextureInUse(texnum,false);

	ASSERT_VALID(dlgTextureDialogBar);
	if(Level_textures_in_use[texnum]==0)
	{
		//it's gone from the level...delete
		dlgTextureDialogBar->AddRemoveLevelTexture(GameTextures[texnum].name,save,false);
	}
#ifdef _DEBUG
	else
	{
		//update the count record
		dlgTextureDialogBar->UpdateLevelTextureCount(GameTextures[texnum].name,save-1,save);
	}
#endif
}


// ==========================
// LevelTexPurgeAllTextures
// ==========================
//
// Call this when a level is being unloaded to purge all textures associated with it
void LevelTexPurgeAllTextures(void)
{
	int j,size;
	for(int i=0;i<MAX_TEXTURES;i++)
	{
		size = Level_textures_in_use[i];
		for(j=0;j<size;j++)
		{
			ned_MarkTextureInUse(i,false);
		}

		Level_textures_in_use[i] = 0;

		if(size>0)
		{
			ASSERT_VALID(dlgTextureDialogBar);
			dlgTextureDialogBar->AddRemoveLevelTexture(GameTextures[i].name,size,false);
		}
	}
}

// ==========================
// RoomTexPurgeAllTextures
// ==========================
//
// Call this when a room is being unloaded to purge all textures associated with it
void RoomTexPurgeAllTextures(short *Textures_in_use)
{
	int j,size;
	for(int i=0;i<MAX_TEXTURES;i++)
	{
		size = Textures_in_use[i];
		for(j=0;j<size;j++)
		{
			ned_MarkTextureInUse(i,false);
		}

		Textures_in_use[i] = 0;
	}
}


// ==========================
// LevelTexInitLevel
// ==========================
//
// Call this as soon as a level is loaded, it will go through the level and set it up.
void LevelTexInitLevel(void)
{
	int rm,f,num_faces,i,t,s;
	room *rp;

	for(rm = 0; rm < MAX_ROOMS; rm++)
	{
		if(Rooms[rm].used)
		{
			rp = &Rooms[rm];
			num_faces = rp->num_faces;

			for(f = 0; f<num_faces; f++)
			{
				LevelTexIncrementTexture(rp->faces[f].tmap);
			}
		}
	}
	LevelTexIncrementTexture(Terrain_sky.dome_texture);
	for (i=0; i<Terrain_sky.num_satellites; i++)
		LevelTexIncrementTexture(Terrain_sky.satellite_texture[i]);
	for (i=0; i<TERRAIN_TEX_DEPTH; i++)
	{
		for (t=0; t<TERRAIN_TEX_WIDTH; t++)
		{
			s=i*TERRAIN_TEX_WIDTH+t;
			LevelTexIncrementTexture(Terrain_tex_seg[s].tex_index);
		}
	}
}

// ==========================
// RoomTexInitRoom
// ==========================
//
// Call this as soon as a paletted room file (ORF) is loaded, it will go through the room and set it up.
void RoomTexInitRoom(room *rp, short *Textures_in_use)
{
	int f;

	for(int i=0;i<MAX_TEXTURES;i++)
	{
		Textures_in_use[i] = 0;
	}

	if(rp->used)
	{
		for(f = 0; f<rp->num_faces; f++)
		{
			ned_MarkTextureInUse(rp->faces[f].tmap,true);
		}
	}
}

void CTextureDialogBar::OnTimer(UINT nIDEvent) 
{
	switch(nIDEvent)
	{
	case TIMER_TEXTURES:
		{
			if(m_CurrentTextureChanged)
			{
				//force update the current texture
				//change the necessary text
				float red;
				float green;
				float blue;
				char red_str[10];
				char green_str[10];
				char blue_str[10];
				char texname[PAGENAME_LEN];
				bool enabled = false;					
				CWnd *wnd;

				if(IsTextureValid(m_CurrentTexture))
				{					
					red = GameTextures[m_CurrentTexture].r;
					green = GameTextures[m_CurrentTexture].g;
					blue = GameTextures[m_CurrentTexture].b;
					strcpy(texname,GameTextures[m_CurrentTexture].name);

					enabled = true;
				}else
				{
					//invalid texture
					strcpy(texname,"NONE");
					red = green = blue = 0.0f;

					enabled = false;
				}
				sprintf(red_str,"%.1f",red);
				sprintf(green_str,"%.1f",green);
				sprintf(blue_str,"%.1f",blue);

				//disable/enable texture buttons
				wnd = GetDlgItem(IDC_CT_PROPERTIES);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_APPLY_TO_MARKED);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_APPLY_TO_CURRENT);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);					
				
				//set text strings
				wnd = GetDlgItem(IDC_CT_LIGHT_VALUE_RED);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(red_str);
				wnd = GetDlgItem(IDC_CT_LIGHT_VALUE_GREEN);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(green_str);
				wnd = GetDlgItem(IDC_CT_LIGHT_VALUE_BLUE);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(blue_str);

				wnd = GetDlgItem(IDC_CT_TEXTURENAME);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(texname);

				RedrawPreviewArea(&m_CTView);
				RedrawPreviewArea(&m_CTLightView);
				UpdateWindow();
				m_CurrentTextureChanged = false;
			}else if(IsTextureValid(m_CurrentTexture) && GameTextures[m_CurrentTexture].flags&TF_ANIMATED)
			{				
				//animated texture update
				RedrawPreviewArea(&m_CTView);
				UpdateWindow();			
			}

			//force update the current face texture
			int tex_to_use = m_CurrentFaceTexture;

			if(m_CurrentFaceTextureChanged)
			{
				//change the necessary text
				float red;
				float green;
				float blue;
				char texname[PAGENAME_LEN];
				char red_str[10];
				char green_str[10];
				char blue_str[10];
				bool enabled = false;					
				CWnd *wnd;

				wnd = GetDlgItem(IDC_CF_PROPERTIES);

				if(IsTextureValid(tex_to_use))
				{
					red = GameTextures[tex_to_use].r;
					green = GameTextures[tex_to_use].g;
					blue = GameTextures[tex_to_use].b;
					strcpy(texname,GameTextures[tex_to_use].name);
				
					enabled = true;
				}else
				{
					//invalid texture
					strcpy(texname,"NONE");
					red = green = blue = 0.0f;

					enabled = false;
				}
				sprintf(red_str,"%.1f",red);
				sprintf(green_str,"%.1f",green);
				sprintf(blue_str,"%.1f",blue);
				
				//disable/enable texture buttons
				wnd = GetDlgItem(IDC_CF_PROPERTIES);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_ALIGNMENT);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);
				wnd = GetDlgItem(IDC_PROPAGATE);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->EnableWindow(enabled);

				//set text strings
				wnd = GetDlgItem(IDC_CF_LIGHT_VALUE_RED);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(red_str);
				wnd = GetDlgItem(IDC_CF_LIGHT_VALUE_GREEN);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(green_str);
				wnd = GetDlgItem(IDC_CF_LIGHT_VALUE_BLUE);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(blue_str);

				wnd = GetDlgItem(IDC_CF_TEXTURENAME);
				if(wnd != NULL && IsWindow(wnd->m_hWnd))
					wnd->SetWindowText(texname);

				RedrawPreviewArea(&m_CFView);
				RedrawPreviewArea(&m_CFLightView);
				UpdateWindow();
				m_CurrentFaceTextureChanged = false;
			}else if(IsTextureValid(tex_to_use) && GameTextures[tex_to_use].flags&TF_ANIMATED)
			{				
				//animated texture update
				RedrawPreviewArea(&m_CFView);
				UpdateWindow();			
			}
		}break;
	}
	
	CDialogBar::OnTimer(nIDEvent);
}


void CTextureDialogBar::OnCtProperties() 
{
	if (!IsTextureValid(m_CurrentTexture))
		return;

	//Current Texture Properties
	CTextureProperties dlg(m_CurrentTexture,this);
	dlg.DoModal();
}

void CTextureDialogBar::OnCfProperties() 
{
	if (m_CurrentFaceTexture==-1)
		return;

	//Current Face properties
	prim *prim = theApp.AcquirePrim();
	if (prim != NULL)
	{
		CTextureProperties dlg(m_CurrentFaceTexture,this);
		dlg.DoModal();
	}
}

void ApplyTexture(room *rp, int facenum, int tnum, short *Textures_in_use)
{
	if ( facenum < 0 || !IsTextureValid(tnum) )
		return;

	HTextureApplyToRoomFace(rp,facenum,tnum);
}

void CTextureDialogBar::OnApplyToCurrent()
{
	prim *prim = theApp.AcquirePrim();
	short *Textures_in_use;
	int room,tex;

	// Sanity check
	room = ROOMNUM(prim->roomp);
	(prim->face != -1) ? (tex = prim->roomp->faces[prim->face].tmap) : (tex = -1);
	ASSERT(room == m_CurrentRoom);
	ASSERT(tex == m_CurrentFaceTexture);

	if (prim != NULL && prim->face != -1)
	{
//		m_CurrentRoom = ROOMNUM(prim->roomp);
//		m_CurrentFace = prim->face;
//		m_CurrentFaceChanged = true;

		void *wnd = theApp.AcquireWnd();
		if (wnd == theApp.m_pLevelWnd || wnd == theApp.m_pRoomFrm)
		{
			Textures_in_use = Level_textures_in_use;
			World_changed = true;
			if (wnd == theApp.m_pRoomFrm)
				((CRoomFrm *) wnd)->m_Room_changed = true;
		}
		else if (wnd == theApp.m_pFocusedRoomFrm)
		{
			Textures_in_use = theApp.m_pFocusedRoomFrm->m_Textures_in_use;
			((CRoomFrm *) wnd)->m_Room_changed = true;
		}

		ApplyTexture(prim->roomp,prim->face,m_CurrentTexture,Textures_in_use);
		SetCurrentFace(ROOMNUM(prim->roomp),prim->face);
	}
	else
		MessageBox("No current face!");
}

void CTextureDialogBar::OnApplyToMarked()
{
	short face_list[MAX_FACES_PER_ROOM];
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim;

	if (wnd != NULL)
	{
		if (theApp.m_pRoomFrm == wnd)
			prim = &theApp.m_pLevelWnd->m_Prim;
		else
			prim = &wnd->m_Prim;
//		m_CurrentRoom = ROOMNUM(prim->roomp);
//		m_CurrentFace = prim->face;
//		m_CurrentFaceChanged = true;

		int num_faces = wnd->GetMarkedFaces(face_list);
		ASSERT(num_faces == wnd->m_Num_marked_faces);
		if (!num_faces)
			return;

		for (int i=0; i<wnd->m_Num_marked_faces; i++)
			ApplyTexture(prim->roomp,face_list[i],m_CurrentTexture,wnd->m_Textures_in_use);

		wnd->m_Room_changed = true;
	}
}

void CTextureDialogBar::OnPropagate()
{
}

void CTextureDialogBar::OnAlignment()
{
	// If the texture alignment dialog is already up, just set its focus
	if (theApp.m_pTexAlignDlg != NULL)
	{
		theApp.m_pTexAlignDlg->SetFocus();
		return;
	}

	prim *prim = theApp.AcquirePrim();

	// Create the modeless texture alignment dialog
	theApp.m_pTexAlignDlg = new CTexAlignDialog(prim);
	theApp.m_pTexAlignDlg->Create(IDD_TEXTURE_ALIGN,this);
}

/////////////////////////////////////////////////////////////////////////////
// CTextureProperties dialog


CTextureProperties::CTextureProperties(int TextureSlot,CWnd* pParent /*=NULL*/)
	: CDialog(CTextureProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTextureProperties)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Texture = TextureSlot;
}


void CTextureProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureProperties)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTextureProperties, CDialog)
	//{{AFX_MSG_MAP(CTextureProperties)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureProperties message handlers

BOOL CTextureProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// check the texture, make sure it's valid
	if(!IsTextureValid(m_Texture) || !stricmp(GameTextures[m_Texture].name,"SAMPLE TEXTURE"))
	{
		// invalid texture 
		CDialog::OnCancel();
	}

	// set our title bar
	char title[256];
	sprintf(title,"Texture Properties: %s",GameTextures[m_Texture].name);
	SetWindowText(title);

	// set name,id fields
	SetDlgItemText(IDC_EDITFILENAME,GameTextures[m_Texture].image_filename);
	SetDlgItemInt(IDC_EDITID,m_Texture);
	// set misc. property fields
	char red[10]; char green[10]; char blue[10]; char alpha[10]; char reflectivity[10]; char slide_u[10]; char slide_v[10]; char speed[10];
	sprintf(red,"%.1f",GameTextures[m_Texture].r);
	sprintf(green,"%.1f",GameTextures[m_Texture].g);
	sprintf(blue,"%.1f",GameTextures[m_Texture].b);
	sprintf(alpha,"%.1f",GameTextures[m_Texture].alpha);
	sprintf(reflectivity,"%.1f",GameTextures[m_Texture].reflectivity);
	sprintf(slide_u,"%.1f",GameTextures[m_Texture].slide_u);
	sprintf(slide_v,"%.1f",GameTextures[m_Texture].slide_v);
	sprintf(speed,"%.1f",GameTextures[m_Texture].speed);
	SetDlgItemText(IDC_EDIT_RED,red);
	SetDlgItemText(IDC_EDIT_GREEN,green);
	SetDlgItemText(IDC_EDIT_BLUE,blue);
	SetDlgItemText(IDC_EDIT_ALPHA,alpha);
	SetDlgItemText(IDC_EDIT_REFLECTIVITY,reflectivity);
	SetDlgItemText(IDC_EDIT_SLIDE_U,slide_u);
	SetDlgItemText(IDC_EDIT_SLIDE_V,slide_v);
	SetDlgItemText(IDC_EDIT_SPEED,speed);

	return TRUE;
}

void CTextureBarList::OnContextMenu(CWnd* pWnd, CPoint point) 
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

void CTextureBarList::OnAddToCustom() 
{
	// TODO: Add your command handler code here
	int sel = m_List.GetCurSel();
	if(sel<0)
		return;

	CString buffer;
	m_List.GetText(sel,buffer);
	char *texstr;
	char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
	if (this != m_ParentWindow->m_CustomDialog) // quick hack
	{
		texstr = strtok(buffer.GetBuffer(0)," ");
		texstr += strlen(texstr) + 1;
	}
	else
	{
		strcpy(str,buffer.GetBuffer(0));
		texstr = str;
	}
#else
	strcpy(str,buffer.GetBuffer(0));
	texstr = str;
#endif

	int texid = ned_FindTexture(texstr);
	if (texid != -1)
		m_ParentWindow->m_CustomDialog->AddTextureToList(texid);
}

void CTextureBarList::OnUpdateAddToCustom(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CTextureBarList::OnRemove() 
{
	// TODO: Add your command handler code here
	room *rp;
	face *fp;
	int i,j;

	int sel = m_List.GetCurSel();
	if(sel<0)
		return;

	CString buffer;
	m_List.GetText(sel,buffer);
	char *texstr;
	char str[PAGENAME_LEN+10] = "";

#ifdef _DEBUG
	if (this != m_ParentWindow->m_CustomDialog) // quick hack
	{
		texstr = strtok(buffer.GetBuffer(0)," ");
		texstr += strlen(texstr) + 1;
	}
	else
	{
		strcpy(str,buffer.GetBuffer(0));
		texstr = str;
	}
#else
	strcpy(str,buffer.GetBuffer(0));
	texstr = str;
#endif

	int texid = ned_FindTexture(texstr);
	if (texid != -1)
	{
		if (this == m_ParentWindow->m_LevelDialog)
		{
			int answer = MessageBox("Remove all instances of this texture from the level?", "Remove texture", MB_YESNO);
			if (answer==IDYES)
			{
				for (i=0,rp=&Rooms[0]; i<=Highest_room_index; i++,rp++)
					for (j=0,fp=&rp->faces[0]; j<rp->num_faces; j++,fp++)
					{
						if (fp->tmap == texid)
						{
							SwitchTexture(rp,texid,0);
							fp->tmap = 0;
						}
					}
			}
		}
		else
			RemoveTextureFromList(texid);
	}
}

void CTextureBarList::OnUpdateRemove(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}


void CTextureDialogBar::OnGrab() 
{
	// TODO: Add your control notification handler code here
	GrabTexture();
}

int CTextureDialogBar::GrabTexture() 
{
	prim *prim = theApp.AcquirePrim();

	ASSERT(prim != NULL);
	if (!prim->roomp || prim->roomp->used==0 || prim->face>=prim->roomp->num_faces)
		return -1;

	int texnum = -1;
	if (prim->face != -1)
	{
		texnum = prim->roomp->faces[prim->face].tmap;
		SetCurrentTexture(texnum);
		PrintStatus("Texture %d, \"%s\", selected.",texnum,GameTextures[texnum].name);
	}
	else
		MessageBox("No current face!");

	return texnum;
}

//writes a texture list to an open file
bool CTextureDialogBar::WriteTextureList(CFILE *outfile,int list)
{
	CTextureBarList *texlst = NULL;
	CString buffer;
	char *texstr;
	char str[PAGENAME_LEN+10] = "";
	int texid;
	int i,count;
	int j=0;

	ASSERT(outfile != NULL);
	if (outfile == NULL)
		return false;

	ASSERT((list == CUSTOM_TEX_LIST) || (list == LEVEL_TEX_LIST));
	if (list == CUSTOM_TEX_LIST)
		texlst = m_CustomDialog;
	else if (list == LEVEL_TEX_LIST)
		texlst = m_LevelDialog;
	else Int3(); // texlst is NULL

	// Write a blank line
	cf_WriteString(outfile,"");

	// Write out texture names
	count = texlst->m_List.GetCount();
	for (i=0; i<count; i++)
	{
		texlst->m_List.GetText(i,buffer);

#ifdef _DEBUG
		if (texlst != m_CustomDialog) // quick hack
		{
			texstr = strtok(buffer.GetBuffer(0)," ");
			texstr += strlen(texstr) + 1;
		}
		else
		{
			strcpy(str,buffer.GetBuffer(0));
			texstr = str;
		}
#else
		strcpy(str,buffer.GetBuffer(0));
		texstr = str;
#endif

		texid = ned_FindTexture(texstr);
		if (texid != -1)
		{
/*
			found = false;
			while (j++ < MAX_TEXTURES)
			{
				cf_ReadString(str,sizeof(str),outfile);
				if ( !strcmp(str,texstr) )
				{
					found = true;
					break;
				}
			}
			if (!found)
*/
				cf_WriteString(outfile,GameTextures[texid].name);
		}
	}

	return true;
}

//reads a texture list from an open file
int CTextureDialogBar::ReadTextureList(CFILE *infile)
{
	char filebuffer[MAX_DER_FILE_BUFFER_LEN];
	CTextureBarList *texlst = NULL;
	CString buffer;
	char str[PAGENAME_LEN+10] = "";
	int texid;
	int line_num = 0;
	int filepos;

	ASSERT(infile != NULL);
	if (infile == NULL)
		return false;

	texlst = m_CustomDialog;

	int count = texlst->m_List.GetCount();
	if (count != 0)
	{
		int answer = MessageBox("Choose Yes to replace the items in the list. Choose No to append.","Replace Items?",MB_YESNO);
		if (answer == IDYES)
			texlst->m_List.ResetContent();
	}

	bool done = 0;

	// Parse textures
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
			// Verify that the string is a valid texture name for the loaded tablefile(s), 
			// then add it to the list
			texid = ned_FindTexture(filebuffer);
			if (texid != -1)
				texlst->AddTextureToList(texid);
		}
	}

	return line_num;
}


void CTextureBarList::OnSaveList() 
{
	// TODO: Add your command handler code here
	DWORD flags = 0;

	if (this == m_ParentWindow->m_CustomDialog)
		flags |= CUSTOM_TEX_LIST;
	else if (this == m_ParentWindow->m_LevelDialog)
		flags |= LEVEL_TEX_LIST;

	DlgSaveResourceLists(flags);
}

void CTextureBarList::OnUpdateSaveList(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}


void CTextureBarList::OnLoadList() 
{
	// TODO: Add your command handler code here
	if (this == m_ParentWindow->m_CustomDialog)
		DlgOpenResourceLists(CUSTOM_TEX_LIST);
	else
		MessageBox("Texture lists can only be opened in the custom tab.");
}

void CTextureBarList::OnUpdateLoadList(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}


void CTextureBarList::OnRemoveAll() 
{
	// TODO: Add your command handler code here
	ASSERT(this == m_ParentWindow->m_CustomDialog);
	if (this != m_ParentWindow->m_CustomDialog)
		return;

	m_List.ResetContent();
}

void CTextureBarList::OnUpdateRemoveAll(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	bool bEnable = false;

	if (this == m_ParentWindow->m_CustomDialog)
		bEnable = true;

	pCmdUI->Enable(bEnable);
}
