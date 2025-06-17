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
 // MissionHogConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include <stdlib.h>

#include "CFILE.h"
#include "pserror.h"
#include "mono.h"
#include "ddio.h"

#include "MissionHogConfigDlg.h"
#include "HogBrowser.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

MissionHogInfo MsnHogInfo;


/////////////////////////////////////////////////////////////////////////////
// CMissionHogConfigDlg dialog


CMissionHogConfigDlg::CMissionHogConfigDlg(CWnd* pParent )
	: CDialog(CMissionHogConfigDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMissionHogConfigDlg)
	m_Name = _T("");
	//}}AFX_DATA_INIT

	m_ChangesMade = false;
	
	m_HadOriginalHog = MsnHogInfo.GetPathName(m_OriginalHog);
}


void CMissionHogConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMissionHogConfigDlg)
	DDX_Text(pDX, IDC_NAME, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMissionHogConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CMissionHogConfigDlg)
	ON_BN_CLICKED(IDC_UNLOAD, OnUnload)
	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMissionHogConfigDlg message handlers

void CMissionHogConfigDlg::OnUnload() 
{
	if(MessageBox("Are you sure you want to unload the mission hog?","Confirmation",MB_YESNO)!=IDYES)
		return;

	MsnHogInfo.UnloadHog();	
	UpdateDlg();
}

void CMissionHogConfigDlg::OnChange() 
{
	char oldhog[_MAX_PATH];
	bool hadoldhog;
	hadoldhog = MsnHogInfo.GetPathName(oldhog);

	CString new_hog_path;
	bool got_new_hog = false;

	//get new hog file
	static char szFilter[] = "Mission Hog file (*.mn3)|*.mn3||";
	char path[_MAX_PATH*2];
	char filename[_MAX_PATH];
	CFileDialog *fd;
	
	fd = new CFileDialog(true,".hog",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,szFilter);

	if(fd->DoModal()==IDOK)
	{		
		new_hog_path = fd->GetPathName();
		got_new_hog = true;
	}
	delete fd;
	if(!got_new_hog)
		return;

	if(hadoldhog && !stricmp(new_hog_path.GetBuffer(0),oldhog))
	{
		char buffer[_MAX_PATH];
		sprintf(buffer,"'%s' is already set as the mission hog!",oldhog);
		MessageBox(buffer,"Error");
		return;
	}

	//do a check on the path, lets make sure they didn't select our d3.hog we
	//have set for the main hog
	ddio_SplitPath(new_hog_path.GetBuffer(0),path,filename,NULL);
	if(!stricmp(filename,"d3") || !stricmp(filename,"extra") )
	{
		char d3_hog_dir[_MAX_PATH];
		char new_hog_dir[_MAX_PATH];
		strcpy(new_hog_dir,path);
		ddio_SplitPath(D3HogDir,d3_hog_dir,NULL,NULL); //clear any possible ambiguities with ending '\'

		if(!stricmp(new_hog_dir,d3_hog_dir))
		{
			MessageBox("You cannot select your system hog for a mission hog!","Error",MB_OK);
			return;
		}
	}	

	//now try to change the hog
	if(!MsnHogInfo.ChangeHog(new_hog_path.GetBuffer(0),NULL))
	{
		MessageBox("Unable to load selected hog file","Error",MB_OK);
		UpdateDlg();
		return;
	}	

	UpdateDlg();

	m_ChangesMade = true;	
}

void CMissionHogConfigDlg::OnOK() 
{
	char name[_MAX_PATH];
	if(MsnHogInfo.GetPathName(name))
	{
		MessageBox("Since you have a mission hog selected, don't forget to set this hog in the correct spot of your msn/mn3 file so it is referenced correctly.  Also, make sure that you set this hog as your mission hog whenever you plan on working	on this level/mission.",name,MB_OK);
	}

	CDialog::OnOK();
}

void CMissionHogConfigDlg::OnCancel() 
{
	//did we make any changes?
	if(m_ChangesMade)
	{
		if(MessageBox("Are you sure you want to undo any changes?","Changes Made",MB_YESNO)!=IDYES)
			return;

		if(m_HadOriginalHog)
		{
			//there was a hog loaded before, revert
			mprintf((0,"Reverting to old hog %s\n",m_OriginalHog));
			MsnHogInfo.ChangeHog(m_OriginalHog,NULL);			
		}else
		{
			//there was no hog originally loaded...so just unload any we may have loaded
			mprintf((0,"Unloading any loaded mission hogs\n"));
			MsnHogInfo.UnloadHog();				
		}
	}

	char name[_MAX_PATH];
	if(MsnHogInfo.GetPathName(name))
	{
		MessageBox("Since you have a mission hog selected, don't forget to set this hog in the correct spot of your msn/mn3 file so it is referenced correctly.  Also, make sure that you set this hog as your mission hog whenever you plan on working	on this level/mission.",name,MB_OK);	
	}
	
	CDialog::OnCancel();
}


BOOL CMissionHogConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	UpdateDlg();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMissionHogConfigDlg::UpdateDlg(void)
{
	bool hog_loaded;
	char hog_name[_MAX_PATH];
	CWnd *wnd;

	hog_loaded = MsnHogInfo.GetPathName(hog_name);

	if(hog_loaded)
	{
		wnd = GetDlgItem(IDC_UNLOAD);
		wnd->EnableWindow(true);

		m_Name = hog_name;
	}else
	{
		m_Name = "";
		wnd = GetDlgItem(IDC_UNLOAD);
		wnd->EnableWindow(false);
	}

	UpdateData(false);
}

bool MissionHogInfo::ChangeHog(char *path,int *lib_id)
{
	ASSERT(path);

	if(loaded)
	{
		UnloadHog();
	}
	
	if(!cfexist(path))
		return false;

	library_id = cf_OpenLibrary(path);
	if(library_id<=0)
	{
		//error loading
		mprintf((0,"Mission Hog: Error loading %s\n",path));
		Int3();
		library_id = -1;
		return false;
	}

	mprintf((0,"Mission Hog: Changing Hog to %s\n",path));
	ASSERT(strlen(path)<_MAX_PATH);
	strncpy(pathname,path,_MAX_PATH-1);
	pathname[_MAX_PATH-1] = '\0';

	RegisterHogFile(pathname,library_id);

	loaded = true;
	if(lib_id)
		*lib_id = library_id;

	return true;
}

bool MissionHogInfo::GetLibraryHandle(int *lib_id)
{
	ASSERT(lib_id);

	*lib_id = -1;

	if(!loaded)
		return false;

	*lib_id = library_id;
	return true;
}

bool MissionHogInfo::GetPathName(char *path)
{
	ASSERT(path);

	*path = '\0';

	if(!loaded)
		return false;

	strcpy(path,pathname);

	return true;
}

bool MissionHogInfo::UnloadHog(void)
{
	if(!loaded)
		return false;
	loaded = false;

	mprintf((0,"Mission Hog: Closing %s\n",pathname));
	ASSERT(library_id>0);

	UnRegisterHogFile(library_id);

	cf_CloseLibrary(library_id);	

	library_id = -1;
	*pathname = '\0';
	
	return true;
}

