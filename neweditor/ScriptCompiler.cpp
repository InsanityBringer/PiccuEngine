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
 



// ScriptCompiler.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "ScriptCompiler.h"
#include "SettingsDialog.h"
#include "../editor/ScriptCompilerAPI.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

ubyte DetermineScriptType(char *filename);

/////////////////////////////////////////////////////////////////////////////
// CScriptCompiler dialog


CScriptCompiler::CScriptCompiler(CWinApp* pParent /*=NULL*/)
	: CDialog(IDD_SCRIPTCOMPILE, NULL)
{
	//{{AFX_DATA_INIT(CScriptCompiler)
	m_ScriptText = _T("");
	m_Output = _T("");
	//}}AFX_DATA_INIT

	ASSERT(pParent != NULL);

	m_pParent = pParent;
	m_nID = CScriptCompiler::IDD;
	bCalledClose = false;
}


void CScriptCompiler::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CScriptCompiler)
	DDX_Control(pDX, IDC_SCRIPTLIST, m_List);
	DDX_Text(pDX, IDC_SCRIPTTXT, m_ScriptText);
	DDX_Text(pDX, IDC_OUTPUT, m_Output);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CScriptCompiler, CDialog)
	//{{AFX_MSG_MAP(CScriptCompiler)
	ON_BN_CLICKED(IDC_COMPILE, OnCompile)
	ON_BN_CLICKED(IDC_CONFIGURE, OnConfigure)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_LBN_DBLCLK(IDC_SCRIPTLIST, OnDblclkScriptlist)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CScriptCompiler message handlers
void CScriptCompiler::OnCancel()
{
	bCalledClose = true;
	((CNewEditorApp*)m_pParent)->ScriptCompilerDone();
	DestroyWindow();
}

int CScriptCompiler::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}

BOOL CScriptCompiler::Create()
{
	return CDialog::Create(IDD_SCRIPTCOMPILE, NULL);
}


void CScriptCompiler::PostNcDestroy() 
{
	delete this;
}

BOOL CScriptCompiler::DestroyWindow() 
{
	return CDialog::DestroyWindow();
}

void CScriptCompiler::OnDestroy() 
{
	CDialog::OnDestroy();
	
	if(!bCalledClose)
	{
		((CNewEditorApp*)m_pParent)->ScriptCompilerDone();
	}	
}

//////////////////////////////////////////////////////////////////////////////
typedef struct
{
	CEdit *edit;
	CString text;
}tScrptCompileData;
tScrptCompileData ScriptCompileData;

void script_compile_callback(char *str)
{
#define NUM_LINES 11
	ScriptCompileData.text += str;

	ScriptCompileData.edit->SetWindowText(ScriptCompileData.text.GetBuffer(0));

	int total_lines = ScriptCompileData.edit->GetLineCount();
	int curr_index = ScriptCompileData.edit->GetFirstVisibleLine();
	if( (total_lines-curr_index)>NUM_LINES){
		//we need to scroll down a line
		ScriptCompileData.edit->LineScroll((total_lines-curr_index)-NUM_LINES);
	}

	defer();
}

void CScriptCompiler::SetDialogName(char *name,...)
{
	char buffer[512];
	if(!name)
	{
		strcpy(buffer,"Script Compile");
	}else
	{
		va_list marker;
		va_start(marker,name);
		_vsnprintf(buffer,511,name,marker);
		buffer[511] = '\0';
		va_end(marker);
	}

	this->SetWindowText(buffer);
	defer();
}

void CScriptCompiler::OnCompile() 
{
	ubyte script_type;
	char buffer[_MAX_PATH];
	char fullpath[_MAX_PATH];
	char olddir[_MAX_PATH];
	int cur_sel;

	SetDialogName(NULL);

	cur_sel = m_List.GetCurSel();
	if(cur_sel==LB_ERR)
	{
		MessageBox("No Script Selected To Compile","Error",MB_OK);
		return;
	}
	m_List.GetText(cur_sel,buffer);

	ddio_MakePath(fullpath,LocalScriptDir,buffer,NULL);

	// Init what we need for the callback
	ScriptCompileData.edit = (CEdit *)GetDlgItem(IDC_OUTPUT);
	ScriptCompileData.text = "";	

	// Change directories into the script directory
	ddio_GetWorkingDir(olddir,_MAX_PATH);
	if(ddio_SetWorkingDir(LocalScriptDir))
	{

		// See what kind of script we have to compile
		script_type = DetermineScriptType(fullpath);	
		if(script_type==255)
		{
			MessageBox("Unable to find selected script","Error",MB_OK);
			m_Output.Format("Unable to find selected script: %s",fullpath);
			ScriptCompileData.edit->SetWindowText(m_Output);
			ddio_SetWorkingDir(olddir);
			return;
		}

		tCompilerInfo ci;
		ci.callback = script_compile_callback;
		ci.script_type = script_type;
		strcpy(ci.source_filename,buffer);

		SetDialogName("Compiling: %s",buffer);		

		int ret = ScriptCompile(&ci);
		switch(ret)
		{
		case CERR_NOERR:
		case CERR_NOCOMPILERDEFINED:
		case CERR_COMPILERMISSING:
			break;
		case CERR_SOURCENOEXIST:
			MessageBox("Unable to open source script to compile","Compile Error",MB_OK);
			m_Output.Format("Unable to open source script to compile: %s",fullpath);
			ScriptCompileData.edit->SetWindowText(m_Output);
			break;
		}
	}else
	{
		MessageBox("Invalid Script Directory","Error",MB_OK);
		m_Output.Format("Invalid Script Directory: %s",LocalScriptDir);
		ScriptCompileData.edit->SetWindowText(m_Output);
	}
	ddio_SetWorkingDir(olddir);

	SetDialogName(NULL);
}

void CScriptCompiler::OnConfigure() 
{
	CSettingsDialog dlg;
	if(dlg.DoModal()==IDOK)
	{
		//update our stuff
		UpdateAll();
	}
}

void CScriptCompiler::OnOK() 
{
	bCalledClose = true;
	((CNewEditorApp*)m_pParent)->ScriptCompilerDone();
	DestroyWindow();
}


BOOL CScriptCompiler::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	//update our stuff
	UpdateAll();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CScriptCompiler::UpdateAll(void)
{
	char old_dir[_MAX_PATH];
	ddio_GetWorkingDir(old_dir,_MAX_PATH);
	
	m_ScriptText.Format("Scripts From: %s",LocalScriptDir);

	m_List.ResetContent();
	
	if(ddio_SetWorkingDir(LocalScriptDir))
	{
		//get all the script files in that directory
		char buffer[_MAX_PATH];
		if(ddio_FindFileStart("*.cpp",buffer))
		{
			m_List.AddString(buffer);
			while(ddio_FindNextFile(buffer))
			{
				m_List.AddString(buffer);
			}
		}
		ddio_FindFileClose();
	}

	ddio_SetWorkingDir(old_dir);

	UpdateData(false);
}

ubyte DetermineScriptType(char *filename)
{
	CFILE *file;
	file = cfopen(filename,"rt");
	if(!file){
		return 255;
	}

	char buffer[4096];
	bool done = false;
	ubyte script_type = ST_GAME;

	cf_ReadString(buffer,4096,file);
	while(!(cfeof(file) || done)){
		if(strstr(buffer,"GetTriggerScriptID")){
			//we found it!
			script_type = ST_LEVEL;
			done = true;
		}
	
		cf_ReadString(buffer,4096,file);
	}

	cfclose(file);

	return script_type;		
}


void CScriptCompiler::OnDblclkScriptlist() 
{
	int cur_sel = m_List.GetCurSel();
	if(cur_sel==LB_ERR)
		return;
	OnCompile();	
}
