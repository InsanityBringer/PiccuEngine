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
 // SettingsDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "SettingsDialog.h"

#include "ned_Util.h"
#include "ned_Rend.h"
#include "globals.h"
#include "texture.h"
#include "render.h"

#include "FolderDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern bool Use_software_zbuffer;
extern int Editor_LOD_engine_off;
extern bool NED_Render_all_rooms;
extern int NED_Portal_depth;
extern int Render_portals;
extern ubyte Shell_render_flag;
extern bool Render_floating_triggers;
extern bool Render_one_room_only;
bool f_allow_objects_to_be_pushed_through_walls = false;

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog dialog


CSettingsDialog::CSettingsDialog(CWnd* pParent )
	: CDialog(CSettingsDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSettingsDialog)
	m_DataDir = _T("");
	m_ScriptDir = _T("");
	m_VCDir = _T("");
	m_VCWarningLevel = 3;
	m_VCDebugLevel = -1;
	//}}AFX_DATA_INIT
}


void CSettingsDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSettingsDialog)
	DDX_Text(pDX, IDC_DATADIR, m_DataDir);
	DDX_Text(pDX, IDC_SCRIPT_PATH, m_ScriptDir);
	DDX_Text(pDX, IDC_VC_PATH, m_VCDir);
	DDX_Radio(pDX, IDC_VC_WARNING_0, m_VCWarningLevel);
	DDX_Radio(pDX, IDC_VC_DEBUG_OFF, m_VCDebugLevel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSettingsDialog, CDialog)
	//{{AFX_MSG_MAP(CSettingsDialog)
	ON_BN_CLICKED(IDC_SELECTDIR, OnSelectdir)
	ON_BN_CLICKED(IDC_VC_BROWSE, OnVcBrowse)
	ON_BN_CLICKED(IDC_SCRIPT_BROWSE, OnScriptBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSettingsDialog message handlers

void CSettingsDialog::OnSelectdir() 
{

	static char szFilter[] = "Descent 3 Hog file (d3.hog)|d3.hog||";
	CString SelectedFile;
	char path[_MAX_PATH*2];
	CFileDialog *fd;
	
	//Need to do something with the default path

	fd = new CFileDialog(true,"d3.hog",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,szFilter);

	if(fd->DoModal()==IDOK)
	{		
		SelectedFile = fd->GetPathName();
		SplitPath(SelectedFile,path,NULL,NULL);		
		m_DataDir = path;
		UpdateData(false);
	}
	delete fd;
}

void CSettingsDialog::OnOK() 
{
	UpdateData(true);
	
	NED_Portal_depth = GetDlgItemInt(IDC_PORTAL_DEPTH);

	if (IsDlgButtonChecked(IDC_ALL_ROOMS))
	{
		NED_Render_all_rooms = true;
		Render_one_room_only = false;
	}
	else if (IsDlgButtonChecked(IDC_DISPLAY_BY_DEPTH))
	{
		NED_Render_all_rooms = false;
		Render_one_room_only = false;
	}
	else if (IsDlgButtonChecked(IDC_DISPLAY_ONE_ROOM))
	{
		NED_Render_all_rooms = false;
		Render_one_room_only = true;
	}

	if (IsDlgButtonChecked(IDC_SHELL_FACES))
		Shell_render_flag &= ~SRF_NO_SHELL;
	else
		Shell_render_flag |= SRF_NO_SHELL;

	if (IsDlgButtonChecked(IDC_NON_SHELL_FACES))
		Shell_render_flag &= ~SRF_NO_NON_SHELL;
	else
		Shell_render_flag |= SRF_NO_NON_SHELL;

	if (IsDlgButtonChecked(IDC_Z_BUFFER))
		Use_software_zbuffer = true;
	else
		Use_software_zbuffer = false;
	tex_SetZBufferState (Use_software_zbuffer);
	State_changed=1;

	AfxGetApp()->WriteProfileInt("settings","ZBuffer",Use_software_zbuffer);

	if (IsDlgButtonChecked(IDC_TERRAIN_LOD))
		Editor_LOD_engine_off = false;
	else
		Editor_LOD_engine_off = true;
	State_changed=1;

	AfxGetApp()->WriteProfileInt("settings","TerrainLOD",Editor_LOD_engine_off);

	if (IsDlgButtonChecked(IDC_PORTAL_FACES))
		Render_portals = 1;
	else
		Render_portals = 0;

	if (IsDlgButtonChecked(IDC_FLOATING_TRIGGERS))
		Render_floating_triggers = true;
	else
		Render_floating_triggers = false;

	if (IsDlgButtonChecked(IDC_MISC_ALLOWOBJECTSTHROUGHWALLS))
		f_allow_objects_to_be_pushed_through_walls = true;
	else
		f_allow_objects_to_be_pushed_through_walls = false;

	AfxGetApp()->WriteProfileInt("settings","PushObjects",f_allow_objects_to_be_pushed_through_walls);

	AfxGetApp()->WriteProfileString("settings","D3Dir",m_DataDir);

	//write out virtual compiler data
	if(m_VCWarningLevel<0 || m_VCWarningLevel>4)
	{
		Int3();
		m_VCWarningLevel = 3;
	}
	if(m_VCDebugLevel==1)
		m_VCDebugLevel = 2;//force to C7

	AfxGetApp()->WriteProfileInt("settings","EditorVCWarningLevel",m_VCWarningLevel);
	AfxGetApp()->WriteProfileInt("settings","EditorVCDebugLevel",m_VCDebugLevel);
	AfxGetApp()->WriteProfileString("settings","EditorCompiler",m_VCDir);

	strcpy(LocalScriptDir,m_ScriptDir.GetBuffer(0));
	AfxGetApp()->WriteProfileString("settings","ScriptDirectory",LocalScriptDir);

	strcpy(D3HogDir,m_DataDir);

	ned_SetupSearchPaths(m_DataDir.GetBuffer(0));

	CDialog::OnOK();
}

void CSettingsDialog::OnCancel() 
{
	
	
	CDialog::OnCancel();
}

BOOL CSettingsDialog::OnInitDialog() 
{
	CString buffer;
	CDialog::OnInitDialog();

	SetDlgItemInt(IDC_PORTAL_DEPTH,NED_Portal_depth);

	if (NED_Render_all_rooms)
		CheckRadioButton(IDC_ALL_ROOMS,IDC_DISPLAY_ONE_ROOM,IDC_ALL_ROOMS);
	else if (Render_one_room_only)
		CheckRadioButton(IDC_ALL_ROOMS,IDC_DISPLAY_ONE_ROOM,IDC_DISPLAY_ONE_ROOM);
	else
		CheckRadioButton(IDC_ALL_ROOMS,IDC_DISPLAY_ONE_ROOM,IDC_DISPLAY_BY_DEPTH);

	CheckDlgButton(IDC_SHELL_FACES,!(Shell_render_flag & SRF_NO_SHELL));

	CheckDlgButton(IDC_NON_SHELL_FACES,!(Shell_render_flag & SRF_NO_NON_SHELL));

	CheckDlgButton(IDC_PORTAL_FACES,Render_portals);

	CheckDlgButton(IDC_FLOATING_TRIGGERS,Render_floating_triggers);

	CheckDlgButton(IDC_Z_BUFFER,Use_software_zbuffer);

	CheckDlgButton(IDC_TERRAIN_LOD,!Editor_LOD_engine_off);

	CheckDlgButton(IDC_MISC_ALLOWOBJECTSTHROUGHWALLS,f_allow_objects_to_be_pushed_through_walls);

	m_DataDir = AfxGetApp()->GetProfileString("settings","D3Dir","");

	//read in virtual compiler data
	m_VCWarningLevel = AfxGetApp()->GetProfileInt("settings","EditorVCWarningLevel",3);
	if(m_VCWarningLevel<0 || m_VCWarningLevel>4)
	{
		Int3();
		m_VCWarningLevel = 3;
	}
	m_VCDebugLevel = AfxGetApp()->GetProfileInt("settings","EditorVCDebugLevel",2);
	if(m_VCDebugLevel!=0)
		m_VCDebugLevel = 1;

	m_VCDir = AfxGetApp()->GetProfileString("settings","EditorCompiler","");
	m_ScriptDir = AfxGetApp()->GetProfileString("settings","ScriptDirectory","");

	UpdateData(false);
	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSettingsDialog::OnVcBrowse() 
{
	char szFilter[] = "Descent 3 Virtual Compiler (*.exe)|*.exe||";
	CFileDialog *fd;
	
	fd = new CFileDialog(true,".exe",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,szFilter);

	if(fd->DoModal()==IDOK)
	{		
		m_VCDir = fd->GetPathName();
		UpdateData(false);
	}
	delete fd;	
}

void CSettingsDialog::OnScriptBrowse() 
{
	CFolderDialog dlg(m_ScriptDir.GetBuffer(0),0,this);

	if(dlg.DoModal()==IDOK)
	{
		m_ScriptDir = dlg.GetPathName();
		UpdateData(false);
	}
}

