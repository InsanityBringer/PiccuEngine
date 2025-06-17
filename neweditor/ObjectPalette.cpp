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
 



// ObjectPalette.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "ObjectPalette.h"
#include "ned_Object.h"
#include "ObjectDialogBar.h"
#include "ned_TableFile.h"
#include "ProgressDialog.h"
#include "ned_Util.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CObjectPalette dialog


CObjectPalette::CObjectPalette(CWnd* pParent /*=NULL*/)
	: CDialog(CObjectPalette::IDD, pParent)
{
	//{{AFX_DATA_INIT(CObjectPalette)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	ASSERT(pParent != NULL);

	m_pParent = pParent;
	m_nID = CObjectPalette::IDD;
	bCalledClose = false;
	m_ShowFilter = 0;
}


void CObjectPalette::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CObjectPalette)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CObjectPick, CGrListBox)
	//{{AFX_MSG_MAP(CObjectPick)
	ON_WM_LBUTTONDOWN()
	ON_WM_CREATE()	
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CObjectPick::SetParent(CWnd *wnd)
{
	ASSERT_VALID(wnd);
	ParentWnd = wnd;
}

void CObjectPick::OnLButtonDown(UINT nFlags, CPoint point) 
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

			if(item>=0 && item<MAX_OBJECT_IDS && Object_info[item].used && Object_info[item].type == m_ShowFilter)
			{
				CGrListBox::OnLButtonDown(nFlags, point);
				left_called = true;
				ForceUpdate();

				OnCopyToClip(Object_info[item].name);
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

void CObjectPick::OnCopyToClip(char *string) 
{
	mprintf((0,"Putting '%s' into dd clipboard\n",string));

	COleDataSource pSource;
	CSharedFile	sf(GMEM_MOVEABLE|GMEM_DDESHARE|GMEM_ZEROINIT);
	UINT		format = RegisterClipboardFormat("GenericDrop");
	CString	text = string;

	sf.Write(text, text.GetLength()); // You can write to the clipboard as you would to any CFile

	HGLOBAL hMem = sf.Detach();
	if (!hMem) return;
	pSource.CacheGlobalData(format, hMem);
	pSource.DoDragDrop(DROPEFFECT_COPY);		
}

int CObjectPick::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	int ret = CGrListBox::OnCreate(lpCreateStruct); 
	m_DropTarget.Register(this);
	return ret;
}

BEGIN_MESSAGE_MAP(CObjectPalette, CDialog)
	//{{AFX_MSG_MAP(CObjectPalette)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_D_POWERUPS, OnDPowerups)
	ON_BN_CLICKED(IDC_D_CLUTTER, OnDClutter)
	ON_BN_CLICKED(IDC_D_ROBOTS, OnDRobots)
	ON_BN_CLICKED(IDC_D_BUILDINGS, OnDBuildings)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO1, OnRadio1)
	ON_BN_CLICKED(IDC_RADIO2, OnRadio2)
	ON_BN_CLICKED(IDC_RADIO3, OnRadio3)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CObjectPalette message handlers

void CObjectPalette::OnOK() 
{
	bCalledClose = true;
	((CObjectDialogBar*)m_pParent)->ObjectPaletteDone();
	DestroyWindow();
}

int CObjectPalette::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
	
	return 0;
}

BOOL CObjectPalette::Create()
{
	return CDialog::Create(m_nID, m_pParent);
}

void CObjectPalette::OnCancel()
{
	bCalledClose = true;
	((CObjectDialogBar*)m_pParent)->ObjectPaletteDone();
	DestroyWindow();
}

void CObjectPalette::PostNcDestroy()
{
	delete this;
}

BOOL CObjectPalette::DestroyWindow() 
{
	return CDialog::DestroyWindow();
}

BOOL CObjectPalette::OnInitDialog() 
{
	CDialog::OnInitDialog();

	for(int i=0;i<MAX_OBJECT_IDS;i++)
	{
		object_in_mem[i] = 0;
	}
	
	//TODO: load from reg and prep to real filter here
	PrepObjects(0);

	//Initialize object dialog
	RECT rect;
	CWnd *wnd = GetDlgItem(IDC_VIEW);
	wnd->GetClientRect(&rect);
	
	grSurface *ds = Editor_state.GetDesktopSurface();

	m_ObjectList.SetShowFilter(m_ShowFilter);
	m_ObjectList.SetParent(this);
	m_ObjectList.Create(wnd,IDC_VIEW, rect, MEDIUM_SIZE,ds);	
	m_ObjectList.SelectItem(0);

	CheckDlgButton(IDC_RADIO2,TRUE);

	return TRUE;
}

//	enumerates items maximum in the list box.  this is here to insure that any changes in
//	the user-managed list.
int CObjectPick::ListEnumerate()
{
	int count = 0,i;

	for (i=0;i<MAX_OBJECT_IDS;i++)
	{
		bool countit=true;;
		if (!Object_info[i].used)
			countit=false;
		if (!(Object_info[i].type == m_ShowFilter))
			countit=false;
		//if (Show_used && !CurrentlyUsedObjects[i])
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
int CObjectPick::ListFirstItem(int firstobj)
{
	if (firstobj < 0) firstobj = 0;

//	find first object that is used if the object passed became invalid.
	if (!Object_info[firstobj].used || !(Object_info[firstobj].type == m_ShowFilter)) {
		int new_first = ListNextItem(firstobj);
		if (new_first == firstobj)
			return -1;
		else
			return new_first;
	}
	else 
		return firstobj;

	return -1;

}

// Specify the next/prev item to be drawn in the box by DrawItem. 
//	Returns the item # following or preceding the passed the item #
int CObjectPick::ListNextItem(int curobj)
{
	int next_object = -1;

	if (curobj >= 0) 
		next_object = ned_GetNextObject(curobj);
	if (next_object <= curobj) 
		return curobj;

	while (!(Object_info[next_object].type == m_ShowFilter) )
	{
		next_object = ned_GetNextObject(next_object);
	//	mprintf((0, "next_obj = %d\n", next_object));
		if (next_object == curobj) {
			break;
		}
	}

	return next_object;
}

int CObjectPick::ListPrevItem(int curobj)
{
	int prev_object = -1;

	if (curobj >=0) 
		prev_object = ned_GetPreviousObject(curobj);
	if (prev_object >= curobj) 
		return curobj;

	while (!(Object_info[prev_object].type == m_ShowFilter) )
	{
		prev_object = ned_GetPreviousObject(prev_object);
		if (prev_object == curobj) {
			break;
		}
	}

	return prev_object;
}

void CObjectPick::DrawItem(int x, int y, grHardwareSurface *itemsurf, int item)
{
	grSurface *ds = Editor_state.GetDesktopSurface();
	bool draw_large = false;
	int text_x = x;
	int text_y = y + m_SurfDimension;
	char str[PAGENAME_LEN];

	if(!ds)
		return;

	int model_num = GetObjectImage(item);

	strcpy(str,Object_info[item].name);

	if(model_num != -1)
	{
		if (item == m_ItemSel)
			draw_large = true;
		DrawItemModel(x,y,ds,itemsurf,model_num,draw_large);
		DrawText(GR_RGB(255,255,255),text_x,text_y,itemsurf,str);
	}
	else
	{
		itemsurf->clear();
		ds->blt(0, 0, itemsurf);
	}
}

void CObjectPick::DrawText(ddgr_color color, int x, int y, grHardwareSurface *itemsurf, char *str)
{
	grSurface *ds = Editor_state.GetDesktopSurface();

	if(!ds)
		return;

	if(str != '\0')
	{
		//Set up surface & vuewport
		itemsurf->clear();
		grViewport *vport = new grViewport(itemsurf);
		vport->clear();
		vport->lock();
		vport->set_text_color(color);
		vport->puts(x,y,str);
		vport->unlock();
		ds->blt(x,y,itemsurf);
		delete vport;
	}
	else
	{
		itemsurf->clear();
		ds->blt(0, 0, itemsurf);
	}
}

//	if item was selected from the list box, this function is invoked.
void CObjectPick::OnItemSelected(int item)
{
	CWnd *wnd = ParentWnd->GetDlgItem(IDC_CURSEL);

	if(item<0 || item>=MAX_OBJECT_IDS || !Object_info[item].used)
	{
		wnd->SetWindowText("");
	}else
	{
		wnd->SetWindowText(Object_info[item].name);
	}
}

void CObjectPick::SetShowFilter(int filter)
{
	m_ShowFilter = filter;	
}

void CObjectPalette::PrepObjects(int filter)
{
	int i;
	bool was_old,is_new;
	int pending_page_in = 0;

	m_ObjectList.EnableSelection(false);

	for(i=0;i<MAX_OBJECT_IDS;i++)
	{
		was_old = is_new = false;

		if(object_in_mem[i]==1)
			was_old = true;

		if(Object_info[i].used && Object_info[i].type == filter)
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
			object_in_mem[i] = 0;
			ned_MarkObjectInUse(i,false);
		}else
		{
			//entering memory
			object_in_mem[i] = 2;	//pending to page in
			pending_page_in++;
		}
	}

	if(pending_page_in==0)
	{
		m_ObjectList.EnableSelection(true);
		return;
	}

	int count = 0;

	DoPageInProgressDialog(0,0,pending_page_in);
	for(i=0;i<MAX_OBJECT_IDS;i++)
	{
		if(object_in_mem[i]==2)
		{
			ned_MarkObjectInUse(i,true);
			DoPageInProgressDialog(1,count,pending_page_in);
			count++;
			object_in_mem[i] = 1;
		}
	}	
	DoPageInProgressDialog(2,pending_page_in,pending_page_in);
	m_ObjectList.EnableSelection(true);
}

void CObjectPalette::SetFilterFlag(int flag,bool enabled)
{
	if(enabled)
	{
		m_ShowFilter |= flag;
	}else
	{
		m_ShowFilter &= ~flag;
	}	
	m_ObjectList.SetShowFilter(m_ShowFilter);
	PrepObjects(m_ShowFilter);
	m_ObjectList.ForceUpdate();
}

void CObjectPalette::OnDPowerups() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_POWERUPS);
	SetFilterFlag(OBJ_POWERUP,(bool)((button->GetState()&0x0003)!=0));
}

void CObjectPalette::OnDRobots() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_ROBOTS);
	SetFilterFlag(OBJ_ROBOT,(bool)((button->GetState()&0x0003)!=0));
}

void CObjectPalette::OnDClutter() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_CLUTTER);
	SetFilterFlag(OBJ_CLUTTER,(bool)((button->GetState()&0x0003)!=0));
}

void CObjectPalette::OnDBuildings() 
{
	CButton *button = (CButton *)GetDlgItem(IDC_D_BUILDINGS);
	SetFilterFlag(OBJ_BUILDING,(bool)((button->GetState()&0x0003)!=0));
}


void CObjectPalette::EnableControls(bool enable)
{
	((CButton *)GetDlgItem(IDC_D_POWERUPS))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_ROBOTS))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_CLUTTER))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_D_BUILDINGS))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDOK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_VIEW))->EnableWindow(enable);
}

// Displays/Updates the page-in progress bar
// state = 0 //create
// state = 1 //update
// state = 2 //destroy
void CObjectPalette::DoPageInProgressDialog(int state,int count,int total)
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
			gProgressDialog->m_TitleText = "Paging In Objects";
			
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
				sprintf(buffer,"Paging In Object (%d of %d)",count,total);
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


void CObjectPalette::OnDestroy() 
{
	CDialog::OnDestroy();
	
	PrepObjects(0);	//remove all objects we referenced

	if(!bCalledClose)
	{
		((CObjectDialogBar*)m_pParent)->ObjectPaletteDone();
	}
}

void CObjectPalette::OnRadio1() 
{
	// TODO: Add your control notification handler code here
	m_ObjectList.SetBitmapSize(SMALL_SIZE);
	m_ObjectList.ForceUpdate();
}

void CObjectPalette::OnRadio2() 
{
	// TODO: Add your control notification handler code here
	m_ObjectList.SetBitmapSize(MEDIUM_SIZE);
	m_ObjectList.ForceUpdate();
}

void CObjectPalette::OnRadio3() 
{
	// TODO: Add your control notification handler code here
	m_ObjectList.SetBitmapSize(LARGE_SIZE);
	m_ObjectList.ForceUpdate();
}
