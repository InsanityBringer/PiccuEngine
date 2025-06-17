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
 



// TableView.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "TableView.h"
#include "globals.h"
#include "ned_Object.h"
#include "ned_GameTexture.h"
#include "ned_Door.h"
#include "ned_Sound.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTableObjectView property page

IMPLEMENT_DYNCREATE(CTableObjectView, CPropertyPage)

CTableObjectView::CTableObjectView() : CPropertyPage(CTableObjectView::IDD)
{
	//{{AFX_DATA_INIT(CTableObjectView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTableObjectView::~CTableObjectView()
{
}

void CTableObjectView::SetFilter(int filter)
{
	m_Filter = filter;
}

void CTableObjectView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTableObjectView)
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTableObjectView, CPropertyPage)
	//{{AFX_MSG_MAP(CTableObjectView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTableObjectView message handlers
/////////////////////////////////////////////////////////////////////////////
// CTableTextureView property page

IMPLEMENT_DYNCREATE(CTableTextureView, CPropertyPage)

CTableTextureView::CTableTextureView() : CPropertyPage(CTableTextureView::IDD)
{
	//{{AFX_DATA_INIT(CTableTextureView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTableTextureView::~CTableTextureView()
{
}

void CTableTextureView::SetFilter(int filter)
{
	m_Filter = filter;
}

void CTableTextureView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTableTextureView)
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTableTextureView, CPropertyPage)
	//{{AFX_MSG_MAP(CTableTextureView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTableTextureView message handlers
/////////////////////////////////////////////////////////////////////////////
// CTableView

IMPLEMENT_DYNAMIC(CTableView, CPropertySheet)

CTableView::CTableView(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CTableView::CTableView(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CTableView::~CTableView()
{
}


BEGIN_MESSAGE_MAP(CTableView, CPropertySheet)
	//{{AFX_MSG_MAP(CTableView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTableView message handlers


#define COLUMN_NAME	0
#define COLUMN_ID	1
#define COLUMN_FILE	2

#define COLUMN_NAME_SIZE	0.49f
#define COLUMN_ID_SIZE		0.10f
#define COLUMN_FILE_SIZE	0.39f

void AddTableViewItems(CListCtrl *list,int filter,int pagetype)
{
	list->DeleteAllItems();

	int item_count = 0;
	int i;
	int loop_total = 0;
	int added = 0;

	switch(pagetype)
	{
	case PAGETYPE_GENERIC:
		loop_total = MAX_OBJECT_IDS;
		break;
	case PAGETYPE_TEXTURE:
		loop_total = MAX_TEXTURES;
		break;
	case PAGETYPE_DOOR:
		loop_total = MAX_DOORS;
		break;
	case PAGETYPE_SOUND:
		loop_total = MAX_SOUNDS;
		break;
	default:
		Int3();
		break;
	}

	bool add_it;
	char *name;
	int table;

	for(i=0;i<loop_total;i++)
	{
		add_it = false;
		table = -1;

		if(filter==-1)
		{
			add_it = true;
		}

		switch(pagetype)
		{
		case PAGETYPE_GENERIC:
			name = Object_info[i].name;
			table = Object_info[i].table_file_id;
			if(!Object_info[i].used)
			{
				add_it = false;
			}else if(filter == Ntbl_loaded_table_files[Object_info[i].table_file_id].type)
			{
				add_it = true;
			}
			break;
		case PAGETYPE_TEXTURE:
			name = GameTextures[i].name;
			table = GameTextures[i].table_file_id;
			if(!GameTextures[i].used)
			{
				add_it = false;
			}else if(filter == Ntbl_loaded_table_files[GameTextures[i].table_file_id].type)
			{
				add_it = true;
			}

			if(!stricmp(GameTextures[i].name,"SAMPLE TEXTURE"))
			{
				//ignore sample texture
				add_it = false;
			}

			break;
		case PAGETYPE_DOOR:
			name = Doors[i].name;
			table = Doors[i].table_file_id;
			if(!Doors[i].used)
			{
				add_it = false;
			}else if(filter == Ntbl_loaded_table_files[Doors[i].table_file_id].type)
			{
				add_it = true;
			}
			break;
		case PAGETYPE_SOUND:
			name = Sounds[i].name;
			table = Sounds[i].table_file_id;
			if(!Sounds[i].used)
			{
				add_it = false;
			}else if(filter == Ntbl_loaded_table_files[Sounds[i].table_file_id].type)
			{
				add_it = true;
			}
			break;
		default:
			Int3();
			name = NULL;
			table = -1;
			break;
		}

		if(add_it)
		{
			added = list->InsertItem(item_count,name);
			if(added>=0)
			{
				ASSERT(table!=-1);
				char buffer[5];
				sprintf(buffer,"%d",i);
				list->SetItem(added,COLUMN_FILE,LVIF_TEXT,Ntbl_loaded_table_files[table].identifier,0,0,0,0);
				list->SetItem(added,COLUMN_ID,LVIF_TEXT,buffer,0,0,0,0);
			}
			item_count++;
		}

		defer();
	}
}


BOOL CTableObjectView::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	//////////////////////////////////////////////////////////////////
	ModifyStyleEx(0,LVS_EX_FULLROWSELECT);

	RECT rect;
	GetWindowRect(&rect);
	int width = rect.right - rect.left;
	int ret;
	
	ret = m_List.InsertColumn(COLUMN_NAME,"Name",LVCFMT_LEFT,(int)(width*COLUMN_NAME_SIZE));
	ret = m_List.InsertColumn(COLUMN_ID,"ID",LVCFMT_LEFT,(int)(width*COLUMN_ID_SIZE));
	ret = m_List.InsertColumn(COLUMN_FILE,"Tablefile",LVCFMT_LEFT,(int)(width*COLUMN_FILE_SIZE));

	AddTableViewItems(&m_List,m_Filter,PAGETYPE_GENERIC);

	//////////////////////////////////////////////////////////////////	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CTableTextureView::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	//////////////////////////////////////////////////////////////////
	ModifyStyleEx(0,LVS_EX_FULLROWSELECT);

	RECT rect;
	GetWindowRect(&rect);
	int width = rect.right - rect.left;
	int ret;
	
	ret = m_List.InsertColumn(COLUMN_NAME,"Name",LVCFMT_LEFT,(int)(width*COLUMN_NAME_SIZE));
	ret = m_List.InsertColumn(COLUMN_ID,"ID",LVCFMT_LEFT,(int)(width*COLUMN_ID_SIZE));
	ret = m_List.InsertColumn(COLUMN_FILE,"Tablefile",LVCFMT_LEFT,(int)(width*COLUMN_FILE_SIZE));

	AddTableViewItems(&m_List,m_Filter,PAGETYPE_TEXTURE);

	//////////////////////////////////////////////////////////////////	

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
/////////////////////////////////////////////////////////////////////////////
// CTableDoorView property page

IMPLEMENT_DYNCREATE(CTableDoorView, CPropertyPage)

CTableDoorView::CTableDoorView() : CPropertyPage(CTableDoorView::IDD)
{
	//{{AFX_DATA_INIT(CTableDoorView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTableDoorView::~CTableDoorView()
{
}

void CTableDoorView::SetFilter(int filter)
{
	m_Filter = filter;
}

void CTableDoorView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTableDoorView)
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTableDoorView, CPropertyPage)
	//{{AFX_MSG_MAP(CTableDoorView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTableDoorView message handlers
/////////////////////////////////////////////////////////////////////////////
// CTableSoundsView property page

IMPLEMENT_DYNCREATE(CTableSoundsView, CPropertyPage)

CTableSoundsView::CTableSoundsView() : CPropertyPage(CTableSoundsView::IDD)
{
	//{{AFX_DATA_INIT(CTableSoundsView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CTableSoundsView::~CTableSoundsView()
{
}

void CTableSoundsView::SetFilter(int filter)
{
	m_Filter = filter;
}


void CTableSoundsView::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTableSoundsView)
	DDX_Control(pDX, IDC_LIST, m_List);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTableSoundsView, CPropertyPage)
	//{{AFX_MSG_MAP(CTableSoundsView)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTableSoundsView message handlers

BOOL CTableSoundsView::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	//////////////////////////////////////////////////////////////////
	ModifyStyleEx(0,LVS_EX_FULLROWSELECT);

	RECT rect;
	GetWindowRect(&rect);
	int width = rect.right - rect.left;
	int ret;
	
	ret = m_List.InsertColumn(COLUMN_NAME,"Name",LVCFMT_LEFT,(int)(width*COLUMN_NAME_SIZE));
	ret = m_List.InsertColumn(COLUMN_ID,"ID",LVCFMT_LEFT,(int)(width*COLUMN_ID_SIZE));
	ret = m_List.InsertColumn(COLUMN_FILE,"Tablefile",LVCFMT_LEFT,(int)(width*COLUMN_FILE_SIZE));

	AddTableViewItems(&m_List,m_Filter,PAGETYPE_SOUND);

	//////////////////////////////////////////////////////////////////	

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CTableDoorView::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	//////////////////////////////////////////////////////////////////
	ModifyStyleEx(0,LVS_EX_FULLROWSELECT);

	RECT rect;
	GetWindowRect(&rect);
	int width = rect.right - rect.left;
	int ret;
	
	ret = m_List.InsertColumn(COLUMN_NAME,"Name",LVCFMT_LEFT,(int)(width*COLUMN_NAME_SIZE));
	ret = m_List.InsertColumn(COLUMN_ID,"ID",LVCFMT_LEFT,(int)(width*COLUMN_ID_SIZE));
	ret = m_List.InsertColumn(COLUMN_FILE,"Tablefile",LVCFMT_LEFT,(int)(width*COLUMN_FILE_SIZE));

	AddTableViewItems(&m_List,m_Filter,PAGETYPE_DOOR);

	//////////////////////////////////////////////////////////////////	

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void HidePropertySheetButtons(CPropertySheet *pPSheet,BOOL bHideOK,BOOL bHideCancel,BOOL bHideApply)
{
	ASSERT_VALID(pPSheet);

	//Hide OK button
	if(bHideOK)
	{
		CWnd *pWnd = pPSheet->GetDlgItem(IDOK);
		ASSERT(pWnd!=NULL);
		pWnd->ShowWindow(SW_HIDE);
	}

	//Hide Cancel Button
	if(bHideCancel)
	{
		CWnd *pWnd = pPSheet->GetDlgItem(IDCANCEL);
		ASSERT(pWnd!=NULL);
		pWnd->ShowWindow(SW_HIDE);
	}

	//Hide Apply
	if(bHideApply)
	{
		CWnd *pWnd = pPSheet->GetDlgItem(ID_APPLY_NOW);
		ASSERT(pWnd!=NULL);
		pWnd->ShowWindow(SW_HIDE);
	}
}

void CenterPropertySheetButtons(CPropertySheet *pPSheet,BOOL bCenterOK,BOOL bCenterCancel,BOOL bCenterApply)
{
	ASSERT_VALID(pPSheet);
	ASSERT(bCenterOK||bCenterCancel||bCenterApply);

	//Find ok rectangle
	CWnd *pWndOK = pPSheet->GetDlgItem(IDOK);
	ASSERT(pWndOK!=NULL);
	CRect rcOK;
	pWndOK->GetWindowRect(&rcOK);
	pPSheet->ScreenToClient(&rcOK);

	//find cancel rectangle
	CWnd *pWndCancel = pPSheet->GetDlgItem(IDCANCEL);
	ASSERT(pWndCancel!=NULL);
	CRect rcCancel;
	pWndCancel->GetWindowRect(&rcCancel);
	pPSheet->ScreenToClient(&rcCancel);

	//find apply rectangle
	CWnd *pWndApply = pPSheet->GetDlgItem(ID_APPLY_NOW);
	ASSERT(pWndApply!=NULL);
	CRect rcApply;
	pWndApply->GetWindowRect(&rcApply);
	pPSheet->ScreenToClient(&rcApply);

	CRect rcClient;
	pPSheet->GetClientRect(&rcClient);

	//Computer layout
	int nButtonWidth = rcOK.Width();
	int nButtonMargin = rcCancel.left - rcOK.right;
	int nButtons = (bCenterOK?1:0) + (bCenterCancel?1:0) + (bCenterApply?1:0);
	int nGlobalWidth = nButtonWidth*nButtons;
	if(nButtons>1)
	{
		nGlobalWidth += nButtonMargin*(nButtons-1);
	}

	int nCurrentX = (rcClient.left + rcClient.right - nGlobalWidth)/2;
	int nTop = rcOK.top;

	//Center OK
	if(bCenterOK)
	{
		pWndOK->SetWindowPos(NULL,nCurrentX,nTop,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		nCurrentX += nButtonWidth+nButtonMargin;
	}

	//Center Cancel
	if(bCenterCancel)
	{
		pWndCancel->SetWindowPos(NULL,nCurrentX,nTop,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		nCurrentX += nButtonWidth+nButtonMargin;
	}

	//Center Apply
	if(bCenterApply)
	{
		pWndApply->SetWindowPos(NULL,nCurrentX,nTop,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		nCurrentX += nButtonWidth+nButtonMargin;
	}

}

BOOL CTableView::OnInitDialog() 
{
	BOOL bResult = CPropertySheet::OnInitDialog();
	HidePropertySheetButtons(this,FALSE,TRUE,TRUE);
	CenterPropertySheetButtons(this,TRUE,FALSE,FALSE);
	
	return bResult;
}
