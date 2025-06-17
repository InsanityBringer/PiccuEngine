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
 



// TexturePalette.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "TexturePalette.h"
#include "ned_GameTexture.h"
#include "TextureDialogBar.h"
#include "ned_TableFile.h"
#include "ProgressDialog.h"
#include "ned_Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTexturePalette dialog


CTexturePalette::CTexturePalette(CWnd* pParent /*=NULL*/)
	: CDialog(CTexturePalette::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTexturePalette)
	//}}AFX_DATA_INIT
	ASSERT(pParent != NULL);

	m_pParent = pParent;
	m_nID = CTexturePalette::IDD;
	bCalledClose = false;
	m_ShowFilter = 0;
}


void CTexturePalette::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTexturePalette)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTexturePick, CGrListBox)
	//{{AFX_MSG_MAP(CTexturePick)
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CTexturePick::SetParent(CWnd *wnd)
{
	ASSERT_VALID(wnd);
	ParentWnd = wnd;
}

void CTexturePick::OnLButtonDown(UINT nFlags, CPoint point) 
{	
	if(!m_bAllowSelection)
		return;

	RECT box_rect;
	CWnd::GetClientRect(&box_rect);
	int box_height = box_rect.bottom - box_rect.top;
	int box_width = box_rect.right - box_rect.left;

	int previtem,num_items = 0;
	int item = m_ListItem;
	int draw_x,draw_y;
	int items_per_row = box_width/(m_SurfDimension+8);
	int items_max_row = box_height/(m_SurfDimension+8);
	
	int left = (box_width/2) - (items_per_row*(m_SurfDimension+8))/2;
	int top = (box_height/2) - (items_max_row*(m_SurfDimension+8))/2;
	bool left_called = false;

	while ((m_ListItem > -1) && (num_items < (m_ListLen)))
	{
		previtem = item;

	// draw at draw_x, draw_y.  go to next item in list and check if we are at end of list.
		draw_x = left + ((num_items % items_per_row) * (m_SurfDimension+8));
		draw_y = top + ((num_items / items_per_row) * (m_SurfDimension+8));

		if ( point.x>=draw_x && point.x<=(draw_x+m_SurfDimension+8) 
			&& point.y>=draw_y && point.y<=(draw_y+m_SurfDimension+8)) {

			//hit!
			mprintf((0,"Left click! %d\n",item));

			if(item>=0 && item<MAX_TEXTURES && GameTextures[item].used && GameTextures[item].flags&m_ShowFilter)
			{
				CGrListBox::OnLButtonDown(nFlags, point);
				left_called = true;
				ForceUpdate();

				OnCopyToClip(GameTextures[item].name);
			}
		}

		item = ListNextItem(item);
		//	break if we hit end of list.
		if (item <= previtem) break;

		num_items++;
	}
	
	if(!left_called)
		CGrListBox::OnLButtonDown(nFlags, point);
}

void CTexturePick::OnCopyToClip(char *string) 
{
	mprintf((0,"Putting '%s' into dd clipboard\n",string));

	COleDataSource pSource;
	CSharedFile	sf(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT);
	UINT		format = RegisterClipboardFormat("TextureDrop");
	CString	text = string;

	sf.Write(text, text.GetLength()); // You can write to the clipboard as you would to any CFile

	HGLOBAL hMem = sf.Detach();
	if (!hMem) return;
	pSource.CacheGlobalData(format, hMem);
	pSource.DoDragDrop(DROPEFFECT_COPY);		
}

int CTexturePick::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ret = CGrListBox::OnCreate(lpCreateStruct); 
	m_DropTarget.Register(this);
	return ret;
}

BEGIN_MESSAGE_MAP(CTexturePalette, CDialog)
	//{{AFX_MSG_MAP(CTexturePalette)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_D_HUD, OnDHud)
	ON_BN_CLICKED(IDC_D_MINE, OnDMine)
	ON_BN_CLICKED(IDC_D_OBJECT, OnDObject)
	ON_BN_CLICKED(IDC_D_TERRAIN, OnDTerrain)
	ON_BN_CLICKED(IDC_D_ANIMATED, OnDAnimated)
	ON_BN_CLICKED(IDC_D_DESTROYABLE, OnDDestroyable)
	ON_BN_CLICKED(IDC_D_FLYTHRU, OnDFlythru)
	ON_BN_CLICKED(IDC_D_FORCEFIELD, OnDForcefield)
	ON_BN_CLICKED(IDC_D_LAVA, OnDLava)
	ON_BN_CLICKED(IDC_D_LIGHT, OnDLight)
	ON_BN_CLICKED(IDC_D_MARBLE, OnDMarble)
	ON_BN_CLICKED(IDC_D_METAL, OnDMetal)
	ON_BN_CLICKED(IDC_D_PLASTIC, OnDPlastic)
	ON_BN_CLICKED(IDC_D_PROCEDURAL, OnDProcedural)
	ON_BN_CLICKED(IDC_D_RUBBLE, OnDRubble)
	ON_BN_CLICKED(IDC_D_WATER, OnDWater)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTexturePalette message handlers

void CTexturePalette::OnOK() 
{
	bCalledClose = true;
	((CTextureDialogBar*)m_pParent)->TexturePaletteDone();
	DestroyWindow();
}

int CTexturePalette::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

BOOL CTexturePalette::Create()
{
	return CDialog::Create(m_nID, m_pParent);
}

void CTexturePalette::OnCancel()
{
	bCalledClose = true;
	((CTextureDialogBar*)m_pParent)->TexturePaletteDone();
	DestroyWindow();
}

void CTexturePalette::PostNcDestroy()
{
	delete this;
}

BOOL CTexturePalette::DestroyWindow() 
{
	return CDialog::DestroyWindow();
}

BOOL CTexturePalette::OnInitDialog() 
{
	CDialog::OnInitDialog();

	for(int i=0;i<MAX_TEXTURES;i++)
	{
		texture_in_mem[i] = 0;
	}
	
	//TODO: load from reg and prep to real filter here
	PrepTextures(0);

	//Initialize texture dialog
	RECT rect;
	CWnd *wnd = GetDlgItem(IDC_VIEW);
	wnd->GetClientRect(&rect);
	
	grSurface *ds = Editor_state.GetDesktopSurface();

	m_TextureList.SetShowFilter(m_ShowFilter);
	m_TextureList.SetParent(this);
	m_TextureList.Create(wnd,IDC_VIEW, rect, MEDIUM_SIZE,ds);	
	m_TextureList.SelectItem(0);
	
	CheckDlgButton(IDC_RADIO2,TRUE);

	return TRUE;
}

//	enumerates items maximum in the list box.  this is here to insure that any changes in
//	the user-managed list.
int CTexturePick::ListEnumerate()
{
	int count = 0,i;

	for (i=0;i<MAX_TEXTURES;i++)
	{
		bool countit=true;;
		if (!GameTextures[i].used)
			countit=false;
		if (!(GameTextures[i].flags & m_ShowFilter))
			countit=false;
		//if (Show_used && !CurrentlyUsedTextures[i])
		//	countit=false;

		if (countit)
			count++;
	}

	if(count==0)
		m_ListItem = 0;

	return count;
}

// Specify the first item to be drawn in the box by DrawItem. returns the item #
//	return -1 if there is no first item (list is empty, maybe)
int CTexturePick::ListFirstItem(int firsttex)
{
//	skip the error texture 
	if (firsttex < 0) firsttex = 0;
	if (firsttex == 0) firsttex++;

//	find first texture that is used if the texture passed became invalid.
	if (!GameTextures[firsttex].used || !(GameTextures[firsttex].flags&m_ShowFilter)) {
		int new_first = ListNextItem(firsttex);
		if (new_first == firsttex)
			return -1;
		else
			return new_first;
	}
	else 
		return firsttex;

	return -1;

}

// Specify the next/prev item to be drawn in the box by DrawItem. 
//	Returns the item # following or preceding the passed the item #
int CTexturePick::ListNextItem(int curtex)
{
	int next_texture = -1;

	if (curtex >= 0) 
		next_texture = ned_GetNextTexture(curtex);
	if (next_texture <= curtex) 
		return curtex;

	while (!(GameTextures[next_texture].flags&m_ShowFilter) )
	{
		next_texture = ned_GetNextTexture(next_texture);
	//	mprintf((0, "next_tex = %d\n", next_texture));
		if (next_texture == curtex) {
			break;
		}
	}

	return next_texture;
}

int CTexturePick::ListPrevItem(int curtex)
{
	int prev_texture = -1;

	if (curtex >=0) 
		prev_texture = ned_GetPreviousTexture(curtex);
	if (prev_texture >= curtex) 
		return curtex;

	while (!(GameTextures[prev_texture].flags&m_ShowFilter) )
	{
		prev_texture = ned_GetPreviousTexture(prev_texture);
		if (prev_texture == curtex) {
			break;
		}
	}

	return prev_texture;
}

void CTexturePick::DrawItem(int x, int y, grHardwareSurface *itemsurf, int item)
{
	int bm_handle = -1;
	grSurface *ds = Editor_state.GetDesktopSurface();
	bm_handle = ned_GetTextureBitmap (item,0);

	if(bm_handle<BAD_BITMAP_HANDLE)
		bm_handle = BAD_BITMAP_HANDLE;

	if(!ds)
		return;

	itemsurf->load(bm_handle);
	ds->blt(x, y, itemsurf);
}

//	if item was selected from the list box, this function is invoked.
void CTexturePick::OnItemSelected(int item)
{
	CWnd *wnd = ParentWnd->GetDlgItem(IDC_CURSEL);

	if(item<0 || item>=MAX_TEXTURES || !GameTextures[item].used || !stricmp(GameTextures[item].name,"SAMPLE TEXTURE"))
	{
		wnd->SetWindowText("");
	}else
	{
		wnd->SetWindowText(GameTextures[item].name);
	}
}

void CTexturePick::SetShowFilter(int filter)
{
	m_ShowFilter = filter;	
}

void CTexturePalette::PrepTextures(int filter)
{
	int i;
	bool was_old,is_new;
	int pending_page_in = 0;

	m_TextureList.EnableSelection(false);

	for(i=0;i<MAX_TEXTURES;i++)
	{
		was_old = is_new = false;

		if(texture_in_mem[i]==1)
			was_old = true;

		if(GameTextures[i].used && GameTextures[i].flags&filter)
			is_new = true;
		
		//see what we should do
		if(was_old && is_new)
			continue;
		if(!was_old && !is_new)
			continue;

		//they are different, we are going to have to make some changes
		if(!is_new)
		{
			//leaving memory
			texture_in_mem[i] = 0;
			ned_MarkTextureInUse(i,false);
		}else
		{
			//entering memory
			texture_in_mem[i] = 2;	//pending to page in
			pending_page_in++;
		}
	}

	if(pending_page_in==0)
	{
		m_TextureList.EnableSelection(true);
		return;
	}

	int count = 0;

	DoPageInProgressDialog(0,0,pending_page_in);
	for(i=0;i<MAX_TEXTURES;i++)
	{
		if(texture_in_mem[i]==2)
		{
			ned_MarkTextureInUse(i,true);
			DoPageInProgressDialog(1,count,pending_page_in);
			count++;
			texture_in_mem[i] = 1;
		}
	}	
	DoPageInProgressDialog(2,pending_page_in,pending_page_in);
	m_TextureList.EnableSelection(true);
}

void CTexturePalette::SetFilterFlag(int flag,bool enabled)
{
	if(enabled)
	{
		m_ShowFilter |= flag;
	}else
	{
		m_ShowFilter &= ~flag;
	}	
	m_TextureList.SetShowFilter(m_ShowFilter);
	PrepTextures(m_ShowFilter);
	m_TextureList.ForceUpdate();
}

void CTexturePalette::OnDHud() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_HUD);
	SetFilterFlag(TF_HUD_COCKPIT,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDMine() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_MINE);
	SetFilterFlag(TF_MINE,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDObject() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_OBJECT);
	SetFilterFlag(TF_OBJECT,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDTerrain() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_TERRAIN);
	SetFilterFlag(TF_TERRAIN,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDAnimated() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_ANIMATED);
	SetFilterFlag(TF_ANIMATED,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDDestroyable() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_DESTROYABLE);
	SetFilterFlag(TF_DESTROYABLE,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDFlythru() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_FLYTHRU);
	SetFilterFlag(TF_FLY_THRU|TF_PASS_THRU,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDForcefield() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_FORCEFIELD);
	SetFilterFlag(TF_FORCEFIELD,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDLava() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_LAVA);
	SetFilterFlag(TF_LAVA,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDLight() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_LIGHT);
	SetFilterFlag(TF_LIGHT,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDMarble() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_MARBLE);
	SetFilterFlag(TF_MARBLE,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDMetal() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_METAL);
	SetFilterFlag(TF_METAL,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDPlastic() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_PLASTIC);
	SetFilterFlag(TF_PLASTIC,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDProcedural() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_PROCEDURAL);
	SetFilterFlag(TF_PROCEDURAL,(bool)((button->GetState()&0x0003)!=0));}

void CTexturePalette::OnDRubble() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_RUBBLE);
	SetFilterFlag(TF_RUBBLE,(bool)((button->GetState()&0x0003)!=0));
}

void CTexturePalette::OnDWater() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_WATER);
	SetFilterFlag(TF_WATER,(bool)((button->GetState()&0x0003)!=0));
}


void CTexturePalette::EnableControls(bool enable)
{
	((CButton *)GetDlgItem(IDC_D_HUD))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_MINE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_OBJECT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_TERRAIN))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_ANIMATED))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_DESTROYABLE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_FLYTHRU))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_FORCEFIELD))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_LAVA))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_LIGHT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_MARBLE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_METAL))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_PLASTIC))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_PROCEDURAL))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_RUBBLE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_WATER))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDOK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_VIEW))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_RADIO1))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_RADIO2))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_RADIO3))->EnableWindow(enable);
}

// Displays/Updates the page-in progress bar
// state = 0 //create
// state = 1 //update
// state = 2 //destroy
void CTexturePalette::DoPageInProgressDialog(int state,int count,int total)
{
	float percentage = 0.0f;

	if(state==1)
	{
		if(total>0)
		{
			percentage = (float)count/float(total);
		}
	}	

	int pos = percentage*100.0f;
	if(pos<0) pos = 0;
	if(pos>100) pos = 100;

	static CProgressDialog *gProgressDialog = NULL;

	switch(state)
	{
	case 0:
		{
//			EnableTabControls(this->m_hWnd,false,IDOK,-1);
			EnableControls(false);

			gProgressDialog = new CProgressDialog;
			
			gProgressDialog->Create(IDD_LOADLEVELPROGRESS,NULL);
			gProgressDialog->m_TitleText = "Paging In Textures";
			
			gProgressDialog->UpdateData(false);
			gProgressDialog->m_ProgressBar.SetStep(100);
			gProgressDialog->m_ProgressBar.SetPos(pos);

		}break;

	case 1:
		{
			if(gProgressDialog)
			{
				gProgressDialog->m_ProgressBar.SetPos(pos);

				char buffer[1024];
				sprintf(buffer,"Paging In Texture (%d of %d)",count,total);
				gProgressDialog->m_TitleText = buffer;
				gProgressDialog->ShowWindow(SW_SHOW);

				gProgressDialog->UpdateData(false);
				defer();
			}
		}break;

	case 2:
		{
			if(gProgressDialog)
			{
				gProgressDialog->DestroyWindow();
				delete gProgressDialog;
				gProgressDialog = NULL;				

//				EnableTabControls(this->m_hWnd,true,IDOK,-1);
				EnableControls(true);
			}

		}break;
	}
}	


void CTexturePalette::OnDestroy() 
{
	CDialog::OnDestroy();
	
	PrepTextures(0);	//remove all textures we referenced

	if(!bCalledClose)
	{
		((CTextureDialogBar*)m_pParent)->TexturePaletteDone();
	}
}

void CTexturePalette::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	m_TextureList.SetBitmapSize(SMALL_SIZE);
	m_TextureList.ForceUpdate();
}

void CTexturePalette::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	m_TextureList.SetBitmapSize(MEDIUM_SIZE);
	m_TextureList.ForceUpdate();
}

void CTexturePalette::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	m_TextureList.SetBitmapSize(LARGE_SIZE);
	m_TextureList.ForceUpdate();
}
