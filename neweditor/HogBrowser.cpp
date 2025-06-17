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
 



// HogBrowser.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "HogBrowser.h"
#include "debug.h"
#include "CFILE.h"
#include "mem.h"
#include "mono.h"

#include <memory.h>
#include <string.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHogBrowser dialog
typedef struct tHogfilenode
{
	int lib_id;
	char *lib_name;
	tHogfilenode *next;
}tHogfilenode;

class CHogfilelistmgr
{
public:
	CHogfilelistmgr()
	{
		Hogfile_list = NULL;
	}
	~CHogfilelistmgr()
	{
		/* NOTE: Since we are using memory functions in constructors/destructores
		We can't be guarenteed that the mem lib is alive when we use them,
		so, no memory function calls in the hog manager!*/
		mprintf((0,"Closing Hogbrowser's hog manager\n"));
		tHogfilenode *curr,*next;
		curr = Hogfile_list;

		while(curr)
		{
			if(curr->lib_name)
				free(curr->lib_name);
			next = curr->next;
			free(curr);
			curr = next;
		}
		Hogfile_list = NULL;
	}

	bool AddHog(int lib_id,char *name)
	{
		tHogfilenode **curr;
		curr = &Hogfile_list;

		while(*curr) curr = &(*curr)->next;

		*curr = (tHogfilenode *)malloc(sizeof(tHogfilenode));
		if(!(*curr))
			return false;
		(*curr)->next = NULL;
		(*curr)->lib_id = lib_id;
		(*curr)->lib_name = strdup(name);
		return true;
	}

	void GetFileList(void (*callback)(char *filename,void *data),void *data,char *wildcard,char *hog_file /*NULL if all*/)
	{
		tHogfilenode *curr;
		curr = Hogfile_list;
		char buffer[_MAX_PATH];

		ASSERT(callback);
		if(!callback)
			return;

		while(curr)
		{
			if(!hog_file || !stricmp(curr->lib_name,hog_file))
			{
				if(cf_LibraryFindFirst(curr->lib_id,wildcard,buffer))
				{
					(*callback)(buffer,data);
					
					while(cf_LibraryFindNext(buffer))
					{
						(*callback)(buffer,data);
					}
				}
				cf_LibraryFindClose();
			}
			curr = curr->next;
		}
	}

	void GetHogList(void (*callback)(char *hogname,void *data),void *data)
	{
		tHogfilenode *curr;
		curr = Hogfile_list;

		ASSERT(callback);
		if(!callback)
			return;

		while(curr)
		{
			(*callback)(curr->lib_name,data);
			curr = curr->next;
		}
	}

	bool DelHog(int lib_id)
	{
		if(!Hogfile_list)
			return false;

		tHogfilenode *curr,*prev;
		
		if(Hogfile_list->lib_id==lib_id)
		{
			//remove root
			curr = Hogfile_list;
			Hogfile_list = Hogfile_list->next;
			if(curr->lib_name)
				free(curr->lib_name);
			free(curr);
			return true;
		}

		curr = Hogfile_list->next;
		prev = Hogfile_list;

		while(curr)
		{
			if(curr->lib_id==lib_id)
			{
				prev->next = curr->next;
				if(curr->lib_name)
					free(curr->lib_name);
				free(curr);
				return true;
			}
			prev = curr;
			curr = curr->next;
		}
		return false;
	}

private:
	tHogfilenode *Hogfile_list;
};

CHogfilelistmgr Hog_mgr;
void get_file_list_callback(char *filename, CListBox *list);
void get_hog_list_callback(char *filename, CComboBox *list);
#define ALL_HOGS_STRING	"All Hogs"

void HogBrowse_AddWildcardNode(tWildcardNode **root,char *wildcard,char *description)
{
	tWildcardNode **curr = root;

	while(*curr) curr = &(*curr)->next;

	*curr = (tWildcardNode *)mem_malloc(sizeof(tWildcardNode));
	if(!(*curr))
		return;
	(*curr)->next = NULL;
	strcpy((*curr)->description,description);
	strcpy((*curr)->wildcard,wildcard);	
}

CHogBrowser::CHogBrowser(char *wildcard_list /*=NULL*/,CWnd* pParent /*=NULL*/)
	: CDialog(CHogBrowser::IDD, pParent)
{
	//{{AFX_DATA_INIT(CHogBrowser)
	m_FileName = _T("");
	//}}AFX_DATA_INIT
	m_Selected = false;
	m_Busy = false;
	m_FirstPaint = true;
	m_WildcardData = NULL;

	//build wildcard data
	if(wildcard_list)
	{
		//a list was given, parse it
		// e.g. : "Wave Files (*.wav)|*.wav|Stream Audio (*.osf)|*.osf||"
		char wc[128];
		char desc[256];
		char *ptr = wildcard_list;
		bool got_desc,got_wc,done;
		done = false;
		int index;

		while(!done)
		{
			got_desc = got_wc = false;

			//read the desc. first
			index = 0;
			while(*ptr && *ptr!='|')
			{
				desc[index++] = *ptr;
				ptr++;
			}
			if(index>0)
			{
				got_desc = true;
				desc[index] = '\0';
			}else
			{
				//we didn't get a description
				strcpy(desc,"NO DESC");
			}

			//advance the ptr past the |
			ptr++;
			index = 0;

			//read the wildcard
			while(*ptr && *ptr!='|')
			{
				wc[index++] = *ptr;
				ptr++;
			}
			if(index>0)
			{
				got_wc = true;
				wc[index] = '\0';
			}else
			{
				//we didn't get a description
				strcpy(wc,"*.*");
			}

			//advance the ptr past the |
			ptr++;

			if(*ptr=='|')
				done = true;	//this is it for this round

			HogBrowse_AddWildcardNode(&m_WildcardData,wc,desc);
		}

	}else
	{
		//set for *.*
		m_WildcardData = (tWildcardNode *)mem_malloc(sizeof(tWildcardNode));
		if(m_WildcardData)
		{
			m_WildcardData->next = NULL;
			strcpy(m_WildcardData->wildcard,"*.*");
			strcpy(m_WildcardData->description,"All Files (*.*)");
		}
	}
}

CHogBrowser::~CHogBrowser()
{
	tWildcardNode *curr,*next;
	curr = m_WildcardData;
	while(curr)
	{
		next = curr->next;
		mem_free(curr);
		curr = next;
	}
	m_WildcardData = NULL;
}


void CHogBrowser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHogBrowser)
	DDX_Control(pDX, IDC_HOGFILE_CHOOSE, m_HogName);
	DDX_Control(pDX, IDC_FILETYPE, m_FileType);
	DDX_Control(pDX, IDC_FILE_LIST, m_FileList);
	DDX_Text(pDX, IDC_FILENAME, m_FileName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHogBrowser, CDialog)
	//{{AFX_MSG_MAP(CHogBrowser)
	ON_BN_CLICKED(IDOK, OnOpen)
	ON_CBN_SELCHANGE(IDC_HOGFILE_CHOOSE, OnSelchangeHogfileChoose)
	ON_CBN_SELCHANGE(IDC_FILETYPE, OnSelchangeFiletype)
	ON_LBN_DBLCLK(IDC_FILE_LIST, OnDblclkFileList)
	ON_LBN_SELCHANGE(IDC_FILE_LIST, OnSelchangeFileList)
	ON_WM_SHOWWINDOW()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHogBrowser message handlers

BOOL CHogBrowser::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_HogName.ResetContent();
	Hog_mgr.GetHogList((void (*)(char *,void *))get_hog_list_callback,&m_HogName);
	int idx = 0;
	if(m_HogName.GetCount()>1)
	{
		//we have hogs available!
		idx = m_HogName.AddString(ALL_HOGS_STRING);	//show all hogs at once		
	}
	m_HogName.SetCurSel((idx!=CB_ERR)?idx:0);

	//build wildcard list
	m_FileType.ResetContent();
	tWildcardNode *curr_wc = m_WildcardData;
	while(curr_wc)
	{
		m_FileType.AddString(curr_wc->description);
		curr_wc = curr_wc->next;
	}
	m_FileType.SetCurSel(0);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHogBrowser::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
}


void CHogBrowser::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if(m_FirstPaint)
	{
		//Do Initial update
		m_FirstPaint = false;
		OnSelchangeHogfileChoose();	
	}
}

void CHogBrowser::EnableButtons(bool enable)
{
	CWnd *wnd;

	defer();	//handle pending events

	m_Busy = !enable;

	wnd = GetDlgItem(IDC_HOGFILE_CHOOSE);
	wnd->EnableWindow(enable);

	//wnd = GetDlgItem(IDC_FILE_LIST);
	//wnd->EnableWindow(enable);

	wnd = GetDlgItem(IDOK);
	wnd->EnableWindow(enable);

	wnd = GetDlgItem(IDCANCEL);
	wnd->EnableWindow(enable);

	wnd = GetDlgItem(IDC_FILENAME);
	wnd->EnableWindow(enable);

	wnd = GetDlgItem(IDC_FILETYPE);
	wnd->EnableWindow(enable);	
}

void CHogBrowser::OnOpen() 
{
	if(m_Busy)
		return;

	UpdateData(true);

	//check the filename given in m_FileName
	int exist = cfexist(m_FileName.GetBuffer(0));
	if(exist==CF_IN_LIBRARY)
	{
		m_Selected = true;	
		OnOK();
		return;
	}

	MessageBox("You Must Choose A Valid Filename","Error");
}

void CHogBrowser::OnSelchangeHogfileChoose() 
{
	EnableButtons(false);

	m_FileList.ResetContent();
	

	char *hog_filename = NULL; //if this is NULL on the call to GetFileList, then it does all hogs
	char buffer[_MAX_PATH];
	m_HogName.GetLBText(m_HogName.GetCurSel(),buffer);
	if(stricmp(buffer,ALL_HOGS_STRING))
	{
		hog_filename = buffer;
	}


	// Determine the wildcard settings to use
	char combo_wildcard[256];
	char wildcard[128];
	bool found = false;
	m_FileType.GetLBText(m_FileType.GetCurSel(),combo_wildcard);
	tWildcardNode *curr_wc_node;
	curr_wc_node = m_WildcardData;
	while(curr_wc_node)
	{
		if(!stricmp(combo_wildcard,curr_wc_node->description))
		{
			found = true;
			strcpy(wildcard,curr_wc_node->wildcard);
			break;
		}
		curr_wc_node = curr_wc_node->next;
	}
	if(!found)
	{
		strcpy(wildcard,"*.*");
	}

	// Get the list
	Hog_mgr.GetFileList((void (*)(char *,void *))get_file_list_callback,&m_FileList,wildcard,hog_filename);
	EnableButtons(true);

	//reset the filename
	m_FileName = "";
	UpdateData(false);
}

void CHogBrowser::OnSelchangeFiletype() 
{
	OnSelchangeHogfileChoose();	
}

void CHogBrowser::OnDblclkFileList() 
{
	OnSelchangeFileList();	//make sure it is selected
	OnOpen();	
}

void CHogBrowser::OnSelchangeFileList() 
{
	// Set the filename string
	char buffer[_MAX_PATH];
	m_FileList.GetText(m_FileList.GetCurSel(),buffer);
	m_FileName = buffer;
	UpdateData(false);
}


bool CHogBrowser::GetFilename(char *buffer,int buffer_size)
{
	if(!m_Selected)
	{
		Int3();	//no file was selected (user canceled out)
		return false;
	}

	strncpy(buffer,m_FileName.GetBuffer(0),buffer_size-1);
	buffer[buffer_size-1] = '\0';

	return true;
}

void get_file_list_callback(char *filename, CListBox *list)
{
	ASSERT(list);
	ASSERT(filename);
	if(!list)
		return;
	if(!filename)
		return;

	if(list->FindStringExact(0,filename)==LB_ERR)
	{
		//no duplicates
		list->AddString(filename);
		defer();
	}
}

void get_hog_list_callback(char *filename, CComboBox *list)
{
	ASSERT(list);
	ASSERT(filename);
	if(!list)
		return;
	if(!filename)
		return;

	list->AddString(filename);
}

void RegisterHogFile(char *filename,int lib_id)
{
	Hog_mgr.AddHog(lib_id,filename);
}

void UnRegisterHogFile(int lib_id)
{
	Hog_mgr.DelHog(lib_id);
}


