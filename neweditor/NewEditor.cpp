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
 


// NewEditor.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"

#include "CameraSlew.h"
#include "MainFrm.h"
#include "ned_LevelWnd.h"
#include "ned_OrthoWnd.h"
#include "ned_PerspWnd.h"
#include "LevelFrame.h"
#include "RoomFrm.h"
#include "globals.h"
#include "NewEditor.h"
#include "FileNewDialog.h"
#include "ned_HFile.h"
#include "EditLineDialog.h"

#include "erooms.h"
#include "bitmap.h"
#include "gametexture.h"
#include "texture.h"
#include "gr.h"
#include "lightmap_info.h"
#include "special_face.h"
#include "Application.h"
#include "AppDatabase.h"
#include "ned_Util.h"
#include "ned_Rend.h"
#include "ned_Object.h"
#include "SettingsDialog.h"
#include "TablefileManager.h"
#include "ned_Tablefile.h"
#include "ned_GameTexture.h"
#include "vclip.h"
#include "SplashScreen.h"
#include "TipOfTheDay.h"
#include "HogBrowser.h"
#include "DallasMainDlg.h"
#include "findintersection.h"
#include "ned_Trigger.h"
#include "ned_Door.h"
#include "ned_Sound.h"
#include "ship.h"
#include "ambient.h"
#include "matcen.h"

#include "terrain.h"
#include "boa.h"

#include "MissionHogConfigDlg.h"

#include <direct.h>

grScreen *Game_screen = NULL;				// The one and only video screen
oeWin32Application *g_OuroeApp = NULL;		// The Main application
//oeAppDatabase *g_Database = NULL;			// Application database.

extern bool Use_software_zbuffer;
extern bool f_allow_objects_to_be_pushed_through_walls;

extern CTerrainDialogBar *dlgTerrainDialogBar;

void InitDynamicLighting ();

// Clipboard stuff
face Face_clipboard[MAX_FACES_PER_ROOM];	// face clipboard
ned_vert Vert_clipboard[MAX_VERTS_PER_ROOM];	// vert clipboard
int Num_clipboard_faces;					// number of clipboard faces
int Num_clipboard_verts;					// number of clipboard verts
bool g_bFaces = false;						// is face clipboard valid?
bool g_bVerts = false;						// is vert clipboard valid?

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewEditorApp

BEGIN_MESSAGE_MAP(CNewEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CNewEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
	ON_COMMAND(ID_FILE_NEW, OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	ON_COMMAND(ID_FILE_SETTINGS, OnFileSettings)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)
	ON_COMMAND(ID_FILE_SAVEAS, OnFileSaveAs)
	ON_COMMAND(ID_TABLEFILE_MANAGER, OnTablefileManager)
	ON_COMMAND(ID_HELP_CHECKFORUPDATES, OnHelpCheckforupdates)
	ON_COMMAND(ID_HELP_SHOWTIPS, OnHelpShowtips)
	ON_COMMAND(ID_SCRIPTCOMPILE, OnScriptcompile)
	ON_UPDATE_COMMAND_UI(ID_SCRIPTCOMPILE, OnUpdateScriptcompile)
	ON_COMMAND(ID_FILE_DALLASVISUALSCRIPTINTERFACE, OnFileDallasvisualscriptinterface)
	ON_UPDATE_COMMAND_UI(ID_FILE_DALLASVISUALSCRIPTINTERFACE, OnUpdateFileDallasvisualscriptinterface)
	ON_COMMAND(ID_FILE_MRU_FILE1, OnFileMruFile1)
	ON_COMMAND(ID_FILE_MRU_FILE2, OnFileMruFile2)
	ON_COMMAND(ID_FILE_MRU_FILE3, OnFileMruFile3)
	ON_COMMAND(ID_FILE_MRU_FILE4, OnFileMruFile4)
	ON_COMMAND(ID_DATA_MISSIONHOGSETTINGS, OnDataMissionhogsettings)
	ON_UPDATE_COMMAND_UI(ID_DATA_MISSIONHOGSETTINGS, OnUpdateDataMissionhogsettings)
	ON_COMMAND(ID_FILE_SAVE_ROOM, OnFileSaveRoom)
	ON_COMMAND(ID_FILE_SAVEASROOM, OnFileSaveasroom)
	ON_COMMAND(ID_FILE_MN3PACKAGER, OnFileMn3packager)
	ON_COMMAND(ID_FILE_ROOM_PROPERTIES, OnFileRoomProperties)
	ON_COMMAND(ID_FILE_SAVERESOURCELISTS, OnFileSaveResourceLists)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVERESOURCELISTS, OnUpdateFileSaveResourceLists)
	ON_COMMAND(ID_FILE_OPENRESOURCELISTS, OnFileOpenResourceLists)
	ON_UPDATE_COMMAND_UI(ID_FILE_OPENRESOURCELISTS, OnUpdateFileOpenResourceLists)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


// Function to copy a Dallas script file
int CopyScriptFile(char *old_file, char *new_file)
{
	char old_fullpath[_MAX_PATH];
	char new_fullpath[_MAX_PATH];

	ddio_MakePath(old_fullpath,LocalScriptDir,old_file,NULL);
	ddio_MakePath(new_fullpath,LocalScriptDir,new_file,NULL);

	return(cf_CopyFile(new_fullpath,old_fullpath));
}

/////////////////////////////////////////////////////////////////////////////
// CNewEditorApp construction

CNewEditorApp::CNewEditorApp()
{
	m_pLevelWnd = NULL;
	m_pRoomFrm = NULL;
	m_pFocusedRoomFrm = NULL;
	m_pTexAlignDlg = NULL;
	m_pTriggerDlg = NULL;
	m_pGoalDlg = NULL;
	m_pLightingDlg = NULL;
	m_pRoomPropsDlg = NULL;
	m_pMatcensDlg = NULL;
	m_ScriptCompileDlg = NULL;
	m_DallasModelessDlgPtr = NULL;
	m_ThisLevelFrame = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CNewEditorApp object

CNewEditorApp theApp;

CSplashScreen *gSplashScreen = NULL;
CTipOfTheDay gTipOfTheDay;
/////////////////////////////////////////////////////////////////////////////
// CNewEditorApp initialization

BOOL CNewEditorApp::InitInstance()
{
	AfxOleInit();
	AfxInitRichEdit();
	//AfxEnableControlContainer();

	gSplashScreen = new CSplashScreen(NULL);
	if(IsWindow(gSplashScreen->m_hWnd))
	{
		gSplashScreen->BringWindowToTop();
	}
	
	// Standard initialization

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	// Change the registry key under which our settings are stored.
	SetRegistryKey(_T("Outrage"));

	LoadStdProfileSettings();  // Load standard INI file options (including MRU)

	// Get our pens and brushes ready
	InitPens();
	InitBrushes();

#ifdef _DEBUG
	bool debugging = true, console_output = true;

	error_Init(debugging, console_output, "Descent 3 Level Editor");
#endif

	mem_Init();

	World_point_buffer = (small_point *)mem_malloc(sizeof(small_point)*(TERRAIN_WIDTH*TERRAIN_DEPTH));

	// Grab the directories and setup paths
	bool directory_setup = false;
	CString m_DataDir = GetProfileString("settings","D3Dir","");
	if(ddio_SetWorkingDir(m_DataDir.GetBuffer(0)))
	{
		strcpy(D3HogDir,m_DataDir);
		directory_setup = true;		
	}else
	{
		strcpy(D3HogDir,"");
	}

	CString m_ScriptDir = GetProfileString("settings","ScriptDirectory","");
	if(m_ScriptDir.GetLength()>1)
	{
		strcpy(LocalScriptDir,m_ScriptDir.GetBuffer(0));
	}else
	{
		//set to the d3 dir, so we have something
		strcpy(LocalScriptDir,m_DataDir.GetBuffer(0));
	}

	if(directory_setup)
		ned_SetupSearchPaths(D3HogDir);

	// Get various editor settings
	UINT val;
	val = GetProfileInt("settings","ZBuffer",0);
	(val) ? (Use_software_zbuffer = true) : (Use_software_zbuffer = false);
	val = GetProfileInt("settings","TerrainLOD",0);
	(val) ? (Editor_LOD_engine_off = false) : (Editor_LOD_engine_off = true);
	val = GetProfileInt("settings","PushObjects",0);
	(val) ? (f_allow_objects_to_be_pushed_through_walls = true) : (f_allow_objects_to_be_pushed_through_walls = false);

	ntbl_Initialize();
	InitMathTables();

	InitMatcens();

	// Initializes the fvi system
	InitFVI();

	ned_InitSounds();

	// Initialize the Object/Object_info system
	ned_InitObjects();
	InitObjectInfo();
	InitObjects();

	ned_InitDoors();

	InitShips();

	InitTriggers();

	// Init our bitmaps (and lightmaps required for LoadLevel). Must be called before InitTextures and InitTerrain
	bm_InitBitmaps(); // calls lm_InitLightmaps() for us

	// Initializes the Texture system
	ned_InitTextures ();

	// Initialize the terrain
	InitTerrain();

	// initialize lighting systems
	InitLightmapInfo();
	InitSpecialFaces();
	InitDynamicLighting();

	// Set z-buffer state
	tex_SetZBufferState (Use_software_zbuffer);
	State_changed = 1;

	char hogpath[_MAX_PATH*2];
	bool hogfile_opened = false;
	int hog_lib_id;


	ddio_MakePath(hogpath,m_DataDir,"d3.hog",NULL);
	if((hog_lib_id = cf_OpenLibrary(hogpath))!=0)
	{
		RegisterHogFile(hogpath,hog_lib_id);
		mprintf((0,"Hog file %s opened\n",hogpath));
		hogfile_opened = true;
	}
	else
	{
		bool cancel_settings = false;
		
		//Make them enter a good directory!
		do
		{
			AfxMessageBox("Invalid Descent 3 directory!\nYou must set the Descent 3 directory in the settings dialog!",MB_OK|MB_ICONSTOP);
			CSettingsDialog sd;
			if(IDCANCEL==sd.DoModal())
				cancel_settings = true;
			m_DataDir = GetProfileString("settings","D3Dir","");
			ddio_MakePath(hogpath,m_DataDir,"d3.hog",NULL);

		}while( (!cancel_settings) && (((hog_lib_id = cf_OpenLibrary(hogpath))==0)) );

		if(cancel_settings)
		{
			PostQuitMessage(0);
			return FALSE;
		}
		
		RegisterHogFile(hogpath,hog_lib_id);
		mprintf((0,"Hog file %s opened\n",hogpath));
		hogfile_opened = true;

	}

	ddio_MakePath(hogpath,m_DataDir,"extra.hog",NULL);
	if((hog_lib_id = cf_OpenLibrary(hogpath))!=0)
	{
		RegisterHogFile(hogpath,hog_lib_id);
		mprintf((0,"Hog file %s opened\n",hogpath));
	}

	if(hogfile_opened)
	{
		//attempt to setup the base tablefile
		int location = cfexist("Table.gam");
		
		//make sure it's in a hog file
		if(location==CF_IN_LIBRARY)
		{
			//load it in
			int ret = ntbl_LoadTableFile("Table.gam");
			if(ret>=0)
			{
				Ntbl_loaded_table_files[ret].type = TABLE_FILE_BASE;
			}
		}
	}

	char *p = GetCommandLine();
	GatherArgs(p);
	
	InitAmbientSoundSystem();

	InitVClips();

	CMDIFrameWnd* pFrame = new CMainFrame;
	m_pMainWnd = pFrame;

	// create main MDI frame window
	if (!pFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;

	// support drag-and-drop for files
	pFrame->DragAcceptFiles();

	// try to load shared MDI menus and accelerator table

	HINSTANCE hInst = AfxGetResourceHandle();
	m_hMDI_D3L_Menu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_D3LTYPE));
	m_hMDI_D3L_Accel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_D3LTYPE));
	m_hMDI_ORF_Menu  = ::LoadMenu(hInst, MAKEINTRESOURCE(IDR_ORFTYPE));
	m_hMDI_ORF_Accel = ::LoadAccelerators(hInst, MAKEINTRESOURCE(IDR_ORFTYPE));


	pFrame->ShowWindow(m_nCmdShow);
	pFrame->UpdateWindow();

	

	tWin32AppInfo appinfo;

	appinfo.hwnd = (HWnd)pFrame->m_hWnd;
	appinfo.hinst = (HInstance)m_hInstance;
	appinfo.flags = OEAPP_WINDOWED;

	g_OuroeApp = new oeWin32Application(&appinfo);
	//g_Database = new oeD3Win32Database;
	char *driver = "GDIX";
	
	if (!ddgr_Init(g_OuroeApp, driver, false)) 
		AfxMessageBox("DDGR graphic system init failed.",MB_ICONSTOP|MB_OK);

	// Init our renderer
	grSurface::init_system();
	rend_Init(RENDERER_SOFTWARE_16BIT,g_OuroeApp,NULL);

	//Initialize the editor state (Desktop_surf,etc)
	Editor_state.Initialize();

	int showtips = GetProfileInt("Defaults","ShowTips",1);
	if(showtips)
	{
		gTipOfTheDay.Create(IDD_TIPDIALOG,pFrame);
		gTipOfTheDay.ShowWindow(SW_SHOW);
		gTipOfTheDay.BringWindowToTop();
	}

	//Bring the splash screen forward..
	if(IsWindow(gSplashScreen->m_hWnd))
	{
		gSplashScreen->BringWindowToTop();
	}

	// Init "clipboard" buffers
	memset(&Face_clipboard,0,MAX_FACES_PER_ROOM*sizeof(face *));
	memset(&Vert_clipboard,0,MAX_VERTS_PER_ROOM*sizeof(vector));
	Num_clipboard_faces = 0;
	Num_clipboard_verts = 0;

	return TRUE;
}


extern void RoomMemClose();
extern void FreeAllObjects();
/////////////////////////////////////////////////////////////////////////////
// CNewEditorApp message handlers


int CNewEditorApp::ExitInstance() 
{
	int i;

	// Write various editor settings to registry
	WriteProfileInt("settings","ZBuffer",Use_software_zbuffer);
	WriteProfileInt("settings","TerrainLOD",!Editor_LOD_engine_off);

	// Delete GDI objects
	for (i=0; i<NUM_PENS; i++)
		g_pens[i].DeleteObject();

	for (i=0; i<NUM_BRUSHES; i++)
		g_brushes[i].DeleteObject();

	// Destroy menus and accelerator tables (NOTE: do NOT use FreeResource for this)
	if (m_hMDI_D3L_Menu != NULL)
		::DestroyMenu(m_hMDI_D3L_Menu);
	if (m_hMDI_D3L_Accel != NULL)
		::DestroyAcceleratorTable(m_hMDI_D3L_Accel);
	if (m_hMDI_ORF_Menu != NULL)
		::DestroyMenu(m_hMDI_ORF_Menu);
	if (m_hMDI_ORF_Accel != NULL)
		::DestroyAcceleratorTable(m_hMDI_ORF_Accel);

	// Free up other resources
	if(g_OuroeApp)
		delete g_OuroeApp;

	Editor_state.Shutdown();

	RoomMemClose();

	if(World_point_buffer)
		mem_free(World_point_buffer);

	delete gSplashScreen;

	return CWinApp::ExitInstance();
}

extern vector Mine_origin;

void CNewEditorApp::OnFileNew() 
{
	char title[_MAX_PATH];
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	CFileNewDialog fn_dlg;

	if ( fn_dlg.DoModal() == IDCANCEL )
		return;

	int which = fn_dlg.m_Sel;

	CRoomFrm * rFrame;
	CLevelFrame * cFrame;
	int frm_index;
	CString SelectedFile;

	vector origin = {0,0,0};

	switch (which)
	{
	case 0:
	case 1:
		// If Dallas is running, don't create a new document
		if(m_DallasModelessDlgPtr!=NULL) {
			OutrageMessageBox("The Dallas Script Editor is open!\n\n"
									"You must close down Dallas before creating a new level.");
			return;
		}

		//  If a level is already open, don't create a new document
		if (m_pLevelWnd!=NULL) {
			OutrageMessageBox("A level is already open!\n\n"
				"You must close the current level before creating a new level.");
			return;
		}

		// create a new MDI child window for the level (must be done before CreateNewMine)
		cFrame = (CLevelFrame *) pFrame->CreateNewChild(
			RUNTIME_CLASS(CLevelFrame), IDR_D3LTYPE, m_hMDI_D3L_Menu, m_hMDI_D3L_Accel);

		cFrame->ShowWindow(SW_HIDE);

		mprintf((0, "Creating new level...\n"));	

		//	Create new mine
		if (which == 0)
			CreateNewMine();
		else
		{
			SelectedFile = BrowseRooms();
			if ( (SelectedFile == "") || ! (CreateNewMineFromRoom((char *)LPCSTR(SelectedFile))) )
			{
				cFrame->DestroyWindow();
				return;
			}
		}

		cFrame->ShowWindow(SW_SHOW);

		cFrame->SetWindowText("Untitled.d3l - World View");
		cFrame->SetTitle("Untitled.d3l - World View");
		cFrame->InitLevelResources();
		m_ThisLevelFrame = cFrame;
		m_pLevelWnd->InitCamera();
		m_pLevelWnd->m_bInitted = true;
		// TODO : get Level_cam working, then we won't need this
		m_pLevelWnd->CenterMine();

		// create a new MDI child window for the room(s)
		rFrame = (CRoomFrm *) pFrame->CreateNewChild(
			RUNTIME_CLASS(CRoomFrm), IDR_ORFTYPE, m_hMDI_ORF_Menu, m_hMDI_ORF_Accel);

		m_pRoomFrm = rFrame;
		sprintf(title,"Untitled.d3l - Current Room View - \"%s\"", (Curroomp->name != NULL) ? Curroomp->name : "<none>");

		rFrame->SetWindowText(title);
		rFrame->SetTitle(title);
		rFrame->SetPrim(Curroomp, Curface, Curportal, Curedge, Curvert);
		rFrame->SetRefFrame(Mine_origin);
		rFrame->InitTex();
		rFrame->InitPanes();
		Cur_object_index = 0;

		Level_name="";
		break;

	case 2:
		// Must find an available room frame before creating new room
		frm_index = GetFreeRoomFrame();
		if ( frm_index != -1 )
		{
			mprintf((0, "Creating new room...\n"));	

			// Create a new palette room with 0 verts and 0 faces.
			room *rp = CreateNewRoom(0,0,true);
			if (rp == NULL)
			{
				OutrageMessageBox("Cannot create room!");
				return;
			}

			// create a new MDI child window for the room(s)
			rFrame = (CRoomFrm *) pFrame->CreateNewChild(
				RUNTIME_CLASS(CRoomFrm), IDR_ORFTYPE, m_hMDI_ORF_Menu, m_hMDI_ORF_Accel);

			m_ppPaletteRoomFrms[frm_index] = rFrame;
			strcpy(title,"Untitled.orf");
			rFrame->SetWindowText(title);
			rFrame->SetTitle(title);
			rFrame->SetPrim(rp, -1, -1, -1, -1);
			rFrame->SetRefFrame(origin);
			rFrame->InitTex();
			rFrame->InitPanes();
		}
		else
			AfxMessageBox("You've reached the limit of open rooms. Please close a room first.",MB_OK);
		break;

	default:
		return;

	} // end switch
}


int CNewEditorApp::GetFreeRoomFrame()
{
	int i = 0;

	do
		if ( m_ppPaletteRoomFrms[i] == NULL )
			return i;
	while ( i++ < MAX_PALETTE_ROOMS );

	return -1;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_SysInfo;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_SysInfo = _T("");
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_SYSINFOTEXT, m_SysInfo);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CNewEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CNewEditorApp message handlers

#include "loadlevel.h"

#define LOAD_PROGRESS_START    1
#define LOAD_PROGRESS_DONE					200

void CNewEditorApp::OpenNedFile(CString SelectedFile)
{
	char filename[_MAX_PATH];
	char name[_MAX_PATH];
	char ext[10];
	char path[_MAX_PATH*2];
	int n = 0;
	char title[_MAX_PATH];

	mprintf((0,"Opening %s\n",(const char *)SelectedFile));

	SplitPath(SelectedFile,path,name,ext);
	sprintf(filename, "%s%s", name, ext);
	if ( !stricmp(ext,".d3l") )
		WriteProfileString("Defaults","LastDir",path);
	else if ( !stricmp(ext,".orf") )
		WriteProfileString("Defaults","LastRoomDir",path);
	WriteProfileString("Defaults","LastType",ext);
	CString fileonly;
	fileonly.Format("Loading %s",filename);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();

	if (!strcmp(ext,".d3l"))
	{
		// If Dallas is running, don't open the document
		if(m_DallasModelessDlgPtr!=NULL) {
			OutrageMessageBox("The Dallas Script Editor is open!\n\n"
									"You must close down Dallas before loading a new level...");
			return;
		}

		// Check for level already loaded.
		if (m_pLevelWnd != NULL)
		{
			AfxMessageBox("You can only have one level loaded at a time. Close the current level and try again.",MB_OK);
			return;
		}

		// create a new MDI child window for the level (must be done before LoadLevel)
		CLevelFrame * cFrame = (CLevelFrame *) pFrame->CreateNewChild(
			RUNTIME_CLASS(CLevelFrame), IDR_D3LTYPE, m_hMDI_D3L_Menu, m_hMDI_D3L_Accel);

		cFrame->ShowWindow(SW_HIDE);

		LoadLevelProgress(LOAD_PROGRESS_START,0,(char *)LPCSTR(fileonly));

		if(!LoadLevel((char *)LPCSTR(SelectedFile),NULL))
		{
			AfxMessageBox("Unable to load level!",MB_OK);
			LoadLevelProgress(LOAD_PROGRESS_DONE,0,NULL);
			cFrame->DestroyWindow();
			return;
		}
		Level_name=SelectedFile;

		LoadLevelProgress(LOAD_PROGRESS_DONE,0,NULL);

		cFrame->ShowWindow(SW_SHOW);

		sprintf(title, "%s - World View", filename);
		cFrame->SetWindowText(title);
		cFrame->SetTitle(title);
		cFrame->InitLevelResources();
		m_ThisLevelFrame = cFrame;

		/* Textures are loaded now, so we can make BOA */
		MakeBOA();

		m_pLevelWnd->InitCamera();
		m_pLevelWnd->m_bInitted = true;
		// TODO : get Level_cam working, then we won't need this
		m_pLevelWnd->CenterMine();

		// create a new MDI child window for the room(s)
		CRoomFrm * rFrame = (CRoomFrm *) pFrame->CreateNewChild(
			RUNTIME_CLASS(CRoomFrm), IDR_ORFTYPE, m_hMDI_ORF_Menu, m_hMDI_ORF_Accel);

		m_pRoomFrm = rFrame;
		sprintf(title, "%s - Current Room View - \"%s\"", filename, (Curroomp->name != NULL) ? Curroomp->name : "<none>");

		rFrame->SetWindowText(title);
		rFrame->SetTitle(title);
		rFrame->SetPrim(Curroomp, Curface, Curportal, Curedge, Curvert);
		rFrame->SetRefFrame(Mine_origin);
		rFrame->InitTex();
		rFrame->InitPanes();

		// Update the current face/texture displays
		Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(Curroomp), Curface);
	}
	else if (!strcmp(ext,".orf"))
	{
		PrintStatus("Loading %s", name);
		strcat (name,".orf");
		int val=FindRoomName (name);
		int find_new=0;
		int done=0;
		int count=1;

		if (val!=-1)
		{
			int answer=::MessageBox (m_pMainWnd->m_hWnd,"Do you wish to overwrite the room in memory that already has this name?",name,MB_YESNO);
			if (answer==IDYES)
			{
				// Search for this room's window, and destroy it (this will also free the room)
				CRoomFrm *roomwnd = FindRoomWnd(&Rooms[val]);
				if ( roomwnd != NULL )
					roomwnd->DestroyWindow();
			}
		}

		// Must find an available room frame before loading room
		int frm_index = GetFreeRoomFrame();
		if ( frm_index != -1 )
		{

			n = AllocLoadRoom((char *)LPCSTR(SelectedFile),false);
			if(n<0)
			{
				AfxMessageBox("Unable to load room!",MB_OK);
				return;
			}

			ASSERT(Rooms[n].name == NULL);
			Rooms[n].name = (char *) mem_malloc(strlen(name)+1);
			strcpy (Rooms[n].name,name);
			PrintStatus("Room %s loaded", name);

			// create a new MDI child window for the room(s)
			CRoomFrm * rFrame = (CRoomFrm *) pFrame->CreateNewChild(
				RUNTIME_CLASS(CRoomFrm), IDR_ORFTYPE, m_hMDI_ORF_Menu, m_hMDI_ORF_Accel);

			m_ppPaletteRoomFrms[frm_index] = rFrame;
			strcpy(title,filename);

			vector origin = {0,0,0};

			rFrame->SetWindowText(title);
			rFrame->SetTitle(title);
			room *rp = &Rooms[n];
			int facenum,portnum,edgenum,vertnum;
			if (rp->num_faces)
			{
				ASSERT (rp->num_verts);
				facenum = edgenum = vertnum = 0;
				portnum = rp->faces[0].portal_num;
			}
			else
				facenum = portnum = edgenum = vertnum = -1;
			rFrame->SetPrim(rp, facenum, portnum, edgenum, vertnum);
			rFrame->SetRefFrame(origin);
			rFrame->InitTex();
			rFrame->InitPanes();
			// Always set the modified flag for this build, so users don't have to worry about losing data
			rFrame->SetModifiedFlag(true);

			
			// Update the current face/texture displays
			Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(&Rooms[n]), -1);
		}
		else
		{
			AfxMessageBox("You've reached the limit of open rooms. Please close a room first.",MB_OK);
		}
	}
	else
	{
		AfxMessageBox("Unsupported file type.",MB_OK);
		return;
	}

	AddToRecentFileList(SelectedFile);
}

CString CNewEditorApp::BrowseRooms() 
{
	static char szFilter[] = "Descent 3 Room (*.orf)|*.orf||";
	CString SelectedFile;
	CString DefaultPath;
	int n = 0;
	CFileDialog *fd;
	DefaultPath = GetProfileString("Defaults","LastRoomDir","");


	//Need to do something with the default path
	_chdir(DefaultPath);
	fd = new CFileDialog(true,".orf",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,szFilter);

	if (fd->DoModal() == IDCANCEL) { delete fd; return SelectedFile; }

	SelectedFile = fd->GetPathName();

	delete fd;

	return SelectedFile;
}

void CNewEditorApp::OnFileOpen() 
{
	static char szFilter[] = "Descent 3 Level (*.d3l)|*.d3l|Descent 3 Room (*.orf)|*.orf|All Files (*.*)|*.*||";
	CString SelectedFile;
	CString DefaultPath;
	CString DefaultType;
	int n = 0;
	CFileDialog *fd;
	DefaultType = GetProfileString("Defaults","LastType","");
	if (DefaultType == ".d3l")
		DefaultPath = GetProfileString("Defaults","LastDir","");
	else if (DefaultType == ".orf")
		DefaultPath = GetProfileString("Defaults","LastRoomDir","");

	//Need to do something with the default path
	_chdir(DefaultPath);
	fd = new CFileDialog(true,DefaultType,NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,szFilter);
	if (DefaultType == ".d3l")
		fd->m_ofn.nFilterIndex = 1;
	else if (DefaultType == ".orf")
		fd->m_ofn.nFilterIndex = 2;

	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();

	delete fd;

	OpenNedFile(SelectedFile);
}

BOOL CNewEditorApp::OnIdle(LONG lCount) 
{
	// TODO: Add your specialized code here and/or call the base class
	char title[_MAX_PATH];
	bool dlg_update = false;
	
	// Update the main room window
	if (m_pRoomFrm != NULL && (m_pRoomFrm->m_State_changed || m_pRoomFrm->m_Room_changed || World_changed))
	{
		if (m_pRoomFrm->m_Room_changed) // TODO : must set m_Room_changed anytime modification to room is made..this is not yet being done
		{
			World_changed = true; // important that this precedes the following test
//			m_pRoomFrm->SetModifiedFlag(true); // don't need this
			m_pRoomFrm->m_Room_changed = false;
		}
		else
		{
			sprintf(title, "%s - Current Room View - \"%s\"", Level_name, (Curroomp->name != NULL) ? Curroomp->name : "<none>");
			m_pRoomFrm->SetWindowText(title);
			m_pRoomFrm->SetTitle(title);
		}
		m_pRoomFrm->UpdateAllPanes();
		m_pRoomFrm->m_State_changed = false;
		dlg_update = true;
	}

	// Update the focused room window (World_changed check is temporary hack)
	if (m_pFocusedRoomFrm != NULL && m_pFocusedRoomFrm != m_pRoomFrm && (m_pFocusedRoomFrm->m_State_changed || m_pFocusedRoomFrm->m_Room_changed || World_changed))
	{
		if (m_pFocusedRoomFrm->m_Room_changed) // TODO : must set m_Room_changed anytime modification to room is made..this is not yet being done
		{
			m_pFocusedRoomFrm->SetModifiedFlag(true);
			m_pFocusedRoomFrm->m_Room_changed = false;
		}
		m_pFocusedRoomFrm->UpdateAllPanes();
		m_pFocusedRoomFrm->m_State_changed = false;
		dlg_update = true;
	}

	if (World_changed && m_pLevelWnd != NULL)
	{
		m_pLevelWnd->SetModifiedFlag(true);
		ComputeAABB(false);
	}

	if (State_changed || World_changed || dlg_update) {
		// TODO : ASSERT(m_pLevelWnd != NULL);
		if (State_changed || World_changed)
		{
			State_changed = World_changed = false;
			// Update the level window
			if (m_pLevelWnd != NULL)
				m_pLevelWnd->Render();
		}
		// Update some dialogs
		if (m_pTriggerDlg != NULL)
			m_pTriggerDlg->UpdateDialog();
		if (m_pGoalDlg != NULL)
			m_pGoalDlg->UpdateDialog();
		if (m_pRoomPropsDlg != NULL)
			m_pRoomPropsDlg->UpdateDialog();
		if (m_pMatcensDlg != NULL)
			m_pMatcensDlg->UpdateDialog();
		if (m_pTexAlignDlg != NULL)
			m_pTexAlignDlg->UpdateDialog();
		if (m_pLightingDlg != NULL)
			m_pLightingDlg->UpdateDialog();
		if (dlgTerrainDialogBar != NULL)
			dlgTerrainDialogBar->UpdateDialog();
	}

	static float last_time = -1.0f;
	
	float curr_time = timer_GetTime();
	float frametime;
	
	if(last_time == -1.0f)
	{
		frametime = 0.0001f;
	}else
	{
		frametime = curr_time - last_time;
	}

	Gametime += frametime;
	last_time = curr_time;

	return CWinApp::OnIdle(lCount);
}

void CNewEditorApp::OnFileSettings() 
{
	CSettingsDialog sd;

	sd.DoModal();
	
}

void CNewEditorApp::OnFileSave() 
{
	// This redirects to OnFileSaveRoom in case the accelerator (Ctrl+S) or toolbar button is pressed while a room window is in focus
	void *wnd = AcquireWnd();
	if (wnd != NULL)
	{
		if (wnd == m_pLevelWnd)
		{
		if (strcmp(Level_name,""))
		{
			if(SaveLevel((char *)LPCSTR(Level_name),true))
			{
				m_pLevelWnd->SetModifiedFlag(false);
			}
		}
		else
			OnFileSaveAs();
		}
		else if (wnd == m_pFocusedRoomFrm)
		{
			OnFileSaveRoom();
		}
		else
			Int3();
	}
}

void CNewEditorApp::OnFileSaveAs() 
{
	static char szFilter[] = "Descent 3 Level (*.d3l)|*.d3l|All Files (*.*)|*.*||";
	CString SelectedFile;
	char name[_MAX_PATH];
	char ext[10];
	char title[_MAX_PATH];
	CString DefaultPath;
	char path[_MAX_PATH*2];
	CFileDialog *fd;
	DefaultPath = GetProfileString("Defaults","LastDir","");


	//Need to do something with the default path
	_chdir(DefaultPath);
	fd = new CFileDialog(false,".d3l",NULL,OFN_HIDEREADONLY,szFilter);

	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();

	mprintf((0,"Saving %s\n",(const char *)SelectedFile));

	SplitPath(SelectedFile,path,name,ext);
	// Easter egg
	if (strstr(name,"minerva") != NULL)
	{
		AfxMessageBox("Are you attempting to save a Minerva level?",MB_ICONQUESTION);
		AfxMessageBox("No, you can't! I won't let you do it!",MB_ICONEXCLAMATION);
		AfxMessageBox("ARGGHHH! Why must you torture me so?",MB_ICONQUESTION);
		AfxMessageBox("Nope, not gonna happen!",MB_ICONSTOP);
		AfxMessageBox("Did you ask Spaz for permission?",MB_ICONQUESTION);
		AfxMessageBox("You're putting me in quite a situation you know.",MB_ICONINFORMATION);
		AfxMessageBox("Well, I suppose I *HAVE* to let you save Minerva. Sigh.",MB_ICONINFORMATION);
		// Set timer for more taunt messages every few seconds.
		::SetTimer(AfxGetMainWnd()->m_hWnd,TIMER_MINERVA,10000,NULL);
	}
	CString fileonly;
	fileonly.Format("Saving %s",name);
	delete fd;

	{
		int retval=SaveLevel((char *)LPCSTR(SelectedFile),true);

		// If the Save was successfull, handle the Dallas coordination
		if(retval) {
			sprintf(title, "%s - World View", SelectedFile);
			m_ThisLevelFrame->SetWindowText(title);
			m_ThisLevelFrame->SetTitle(title);
			CString Prev_Level_name=Level_name;
			Level_name=SelectedFile;
			WriteProfileString("Defaults","LastDir",path);

			// If Dallas is up, change its filenames
			m_pLevelWnd->SetModifiedFlag(false);
			if(m_DallasModelessDlgPtr!=NULL) {
				m_DallasModelessDlgPtr->SetAllFilenamesToThis(SelectedFile.GetBuffer(0));
			}

			// Copy Dallas files (if doing a 'Save As')
			char old_filename[PSPATHNAME_LEN+1];
			char new_filename[PSPATHNAME_LEN+1];

			CString old_level_fname, new_level_fname;
			old_level_fname = Prev_Level_name;
			new_level_fname = SelectedFile;

			if(old_level_fname.IsEmpty())
				strcpy(old_filename,"Untitled");
			else
				ddio_SplitPath(old_level_fname.GetBuffer(0),NULL,old_filename,NULL);

			if(new_level_fname.IsEmpty())
				strcpy(new_filename,"Untitled");
			else
				ddio_SplitPath(new_level_fname.GetBuffer(0),NULL,new_filename,NULL);
			
			// Make sure names are different (ie 'Save As')
			if(stricmp(old_filename,new_filename)!=0) {
				CString old_file, new_file;

				old_file.Format("%s.cpp",old_filename);
				new_file.Format("%s.cpp",new_filename);
				CopyScriptFile(old_file.GetBuffer(0),new_file.GetBuffer(0));

				old_file.Format("%s.msg",old_filename);
				new_file.Format("%s.msg",new_filename);
				CopyScriptFile(old_file.GetBuffer(0),new_file.GetBuffer(0));

				old_file.Format("%s.dll",old_filename);
				new_file.Format("%s.dll",new_filename);
				CopyScriptFile(old_file.GetBuffer(0),new_file.GetBuffer(0));
			}
		}
		else {
			AfxMessageBox("The save was not successful.",MB_OK);
		}
	}
}


BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	MEMORYSTATUS ms;
	GlobalMemoryStatus(&ms);
	
	m_SysInfo.Format("Memory:\r\nTotal Physical: %d\r\nFree Physical: %d\r\nFree Virtual: %d",ms.dwTotalPhys,ms.dwAvailPhys,ms.dwAvailPageFile);
	
	UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CNewEditorApp::OnTablefileManager()
{
	CTablefileManager dlg;
	dlg.DoModal();	
}

void CNewEditorApp::OnHelpCheckforupdates() 
{
	ShellExecute(AfxGetMainWnd()->m_hWnd,"open","http://www.outrage.com/",NULL,NULL,SW_SHOW);	
}

void CNewEditorApp::OnHelpShowtips() 
{
	if(!(IsWindow(gTipOfTheDay.m_hWnd)))
	{
		gTipOfTheDay.Create(IDD_TIPDIALOG,AfxGetMainWnd());		
	}
	gTipOfTheDay.ShowWindow(SW_SHOW);
	gTipOfTheDay.BringWindowToTop();
}

// AcquirePrim: call this function anytime you need to obtain the correct prim struct 
// (the one the user intends to manipulate). Note that the main room frames primitive is 
// NOT for use, the level's prim should be used instead; hence this function handles the choice for you.
prim * CNewEditorApp::AcquirePrim()
{
	prim *prim;

	if (m_pLevelWnd != NULL)
	{
		if (m_pLevelWnd->m_InFocus)
		{
			prim = &m_pLevelWnd->m_Prim;
			return prim;
		}
	}
	if (m_pFocusedRoomFrm != NULL)
	{
		if (m_pRoomFrm == m_pFocusedRoomFrm && m_pLevelWnd != NULL)
			prim = &m_pLevelWnd->m_Prim;
		else
			prim = &m_pFocusedRoomFrm->m_Prim;
		return prim;
	}

	return NULL;
}

// AcquireWnd: call this function anytime you need to obtain the focused window 
// (the one the user intends to manipulate).
void * CNewEditorApp::AcquireWnd()
{
	void *wnd;
	CFrameWnd *frmwnd = ((CFrameWnd *)AfxGetMainWnd())->GetActiveFrame();

	if (frmwnd->IsKindOf(RUNTIME_CLASS(CLevelFrame)))
	{
		wnd = m_pLevelWnd;
		return (Cned_LevelWnd *)wnd;
	}
	else if (frmwnd->IsKindOf(RUNTIME_CLASS(CRoomFrm)))
	{
		wnd = m_pFocusedRoomFrm;
		return (CRoomFrm *)wnd;
	}
	else return NULL;
}

void CNewEditorApp::OnScriptcompile() 
{
	if (m_ScriptCompileDlg == NULL)
	{
		m_ScriptCompileDlg = new CScriptCompiler(this);
		if (m_ScriptCompileDlg->Create() == TRUE)
		{
			m_ScriptCompileDlg->ShowWindow(SW_SHOW);
		}
	}
	else
		m_ScriptCompileDlg->SetActiveWindow();	
}

void CNewEditorApp::OnUpdateScriptcompile(CCmdUI* pCmdUI) 
{
	if(m_ScriptCompileDlg)
		pCmdUI->Enable(FALSE);
	else
		pCmdUI->Enable(TRUE);
}

void CNewEditorApp::ScriptCompilerDone()
{
	mprintf((0,"Removing Script Compiler\n"));
	m_ScriptCompileDlg = NULL;
}

void CNewEditorApp::DallasDone()
{
	mprintf((0,"Removing Dallas\n"));
	m_DallasModelessDlgPtr = NULL;
}

void CNewEditorApp::OnFileDallasvisualscriptinterface() 
{
	if(Level_name.IsEmpty()) {
		AfxMessageBox("You must give your level a filename before opening Dallas.",MB_OK);
		return;
	}

	if(m_DallasModelessDlgPtr==NULL) {
		m_DallasModelessDlgPtr = new CDallasMainDlg;
		m_DallasModelessDlgPtr->Create(IDD_DALLAS_MAIN_DIALOG,NULL);
	  	m_DallasModelessDlgPtr->ShowWindow(SW_SHOW);
	}
	else
		m_DallasModelessDlgPtr->ShowWindow(SW_RESTORE);
}

void CNewEditorApp::OnUpdateFileDallasvisualscriptinterface(CCmdUI* pCmdUI) 
{

}

void CNewEditorApp::OnFileMruFile(int nIndex) 
{
	ASSERT((*m_pRecentFileList)[nIndex].GetLength() != 0);

	TRACE2("MRU: open file (%d) '%s'.\n", (nIndex) + 1,
			(LPCTSTR)(*m_pRecentFileList)[nIndex]);

	OpenNedFile((*m_pRecentFileList)[nIndex]);
}

void CNewEditorApp::OnFileMruFile1()
{
	OnFileMruFile(0);
}

void CNewEditorApp::OnFileMruFile2()
{
	OnFileMruFile(1);
}

void CNewEditorApp::OnFileMruFile3() 
{
	OnFileMruFile(2);
}

void CNewEditorApp::OnFileMruFile4() 
{
	OnFileMruFile(3);
}

void CNewEditorApp::OnDataMissionhogsettings() 
{
	CMissionHogConfigDlg dlg;
	dlg.DoModal();	
}

void CNewEditorApp::OnUpdateDataMissionhogsettings(CCmdUI* pCmdUI) 
{
	char loaded_hog[_MAX_PATH];
	if(MsnHogInfo.GetPathName(loaded_hog))
	{
		char buffer[_MAX_PATH];
		sprintf(buffer,"Change Mission Hog (%s)",loaded_hog);
		pCmdUI->SetText(buffer);
	}else
	{
		pCmdUI->SetText("Configure A Mission Hog");
	}
}

void CNewEditorApp::OnFileSaveRoom() 
{
	char title[_MAX_PATH];
	char lvl_name[_MAX_PATH];
	char lvl_ext[10];
	char lvl_path[_MAX_PATH*2];
	char filename[_MAX_PATH*2];
		
	prim * pm = AcquirePrim();
	room *rp = pm->roomp;
	if(rp)
	{
		if((!rp->name) || (!*rp->name) || m_pFocusedRoomFrm == m_pRoomFrm)
		{
			//AfxMessageBox("You must first name or save this room!",MB_OK);
			OnFileSaveasroom();
			return;
		}
		// There has to be a room frame if we get this far, and it's not the current level room frame.
		ASSERT(m_pFocusedRoomFrm != NULL);
		ASSERT(m_pFocusedRoomFrm != m_pRoomFrm);

		SplitPath(Level_name,lvl_path,lvl_name,lvl_ext);
		if ( (strlen(rp->name)) <= ROOM_NAME_LEN )
		{
			mprintf((0,"Saving %s\n",rp->name));
			sprintf(filename,"%s%s",m_pFocusedRoomFrm->GetPath(),rp->name);
			SaveRoom(ROOMNUM(rp),filename);
			sprintf(title, "%s", filename);
			m_pFocusedRoomFrm->SetWindowText(title);
			m_pFocusedRoomFrm->SetTitle(title);
			m_pFocusedRoomFrm->SetModifiedFlag(false);
		}
		else
		{
			AfxMessageBox("Filename exceeds allowed number of characters. Room not saved.",MB_ICONERROR);
			return;
		}
	}
	else
	{
		AfxMessageBox("There is no active room to save!",MB_OK);
	}
	
}

void CNewEditorApp::OnFileSaveasroom() 
{

	static char szFilter[] = "Descent 3 Room (*.orf)|*.orf|All Files (*.*)|*.*||";
	CString SelectedFile;
	char name[_MAX_PATH];
	char ext[10];
	char title[_MAX_PATH];
	char lvl_name[_MAX_PATH];
	char lvl_ext[10];
	char lvl_path[_MAX_PATH*2];
	CString DefaultPath;
	char path[_MAX_PATH*2];
	CFileDialog *fd;
	DefaultPath = GetProfileString("Defaults","LastRoomDir","");


	//Need to do something with the default path
	_chdir(DefaultPath);
	fd = new CFileDialog(false,".orf",NULL,OFN_HIDEREADONLY,szFilter);

	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();

	mprintf((0,"Saving %s\n",(const char *)SelectedFile));

	SplitPath(SelectedFile,path,name,ext);
	CString fileonly;
	fileonly.Format("Saving %s",name);
	delete fd;

	SplitPath(Level_name,lvl_path,lvl_name,lvl_ext);

	char roomname[ROOM_NAME_LEN] = "";
	if ( (strlen(name) + strlen(ext)) <= ROOM_NAME_LEN )
	{
		sprintf(roomname,"%s%s",name,ext);
		room *rp = AcquirePrim()->roomp;
		SaveRoom(ROOMNUM(rp),roomname);
		if (m_pFocusedRoomFrm != m_pRoomFrm)
			RenameRoom(rp,roomname);
		WriteProfileString("Defaults","LastRoomDir",path);

		if(m_pFocusedRoomFrm != NULL)
		{
			if (m_pFocusedRoomFrm == m_pRoomFrm)
			{
				int input = AfxMessageBox("The level's current room has been saved out as a separate room file (ORF). Would you like to open it now?",MB_YESNO);
				if (input == IDYES)
					OpenNedFile(SelectedFile);
			}
			else
			{
				sprintf(title, "%s", rp->name);
				m_pFocusedRoomFrm->SetWindowText(title);
				m_pFocusedRoomFrm->SetTitle(title);
				m_pFocusedRoomFrm->SetPath(path);
			}
			m_pFocusedRoomFrm->SetModifiedFlag(false);
		}
		else
			AfxMessageBox("Room saved.",MB_OK);
	}
	else
	{
		AfxMessageBox("Filename exceeds allowed number of characters. Room not saved.",MB_ICONERROR);
		return;
	}
	
}

void CNewEditorApp::OnFileMn3packager() 
{
	CString argv1;

	CString path,filepath;
	argv1 = GetCommandLine();

	if(argv1[0]=='\"')
		argv1 = argv1.Right(argv1.GetLength()-1);

	int lastslash = argv1.ReverseFind('\\');
	if(lastslash!=-1)
		path = argv1.Left(lastslash+1);

	filepath = path;
	filepath += "mn3edit.exe";

	ShellExecute(NULL,"open",filepath,"",path,SW_SHOW);	
	
}

void CNewEditorApp::OnFileRoomProperties() 
{
	// TODO: Add your command handler code here
	((CMainFrame *)AfxGetMainWnd())->RoomProperties();
}


void RenameRoom(room *rp,char *name)
{
	char tempname[ROOM_NAME_LEN+1] = "";

	if (rp->name) {
		ASSERT(strlen(rp->name) <= ROOM_NAME_LEN);
		strcpy(tempname,rp->name);
	}

	strcpy(tempname,name);

	int n = FindRoomName(tempname);
	if ((n != -1) && (n != ROOMNUM(rp))) {
		EditorMessageBox("Room %d already has this name.",n);
		return;
	}

	if (rp->name) {
		mem_free(rp->name);
		rp->name = NULL;
	}

	if (strlen(tempname)) {
		rp->name = (char *) mem_malloc(strlen(tempname)+1);
		strcpy(rp->name,tempname);
	}

	World_changed = 1;
}

void RenameRoom(room *rp)
{
	char tempname[ROOM_NAME_LEN+1] = "";

	if (rp->name) {
		ASSERT(strlen(rp->name) <= ROOM_NAME_LEN);
		strcpy(tempname,rp->name);
	}

try_again:;
	if (! InputString(tempname,ROOM_NAME_LEN,"Room Name","Enter a new name:"))
		return;

	if (StripLeadingTrailingSpaces(tempname))
		EditorMessageBox("Note: Leading and/or trailing spaces have been removed from this name (\"%s\")",tempname);

	int n = FindRoomName(tempname);
	if ((n != -1) && (n != ROOMNUM(rp))) {
		EditorMessageBox("Room %d already has this name.",n);
		goto try_again;
	}

	if (rp->name) {
		mem_free(rp->name);
		rp->name = NULL;
	}

	if (strlen(tempname)) {
		rp->name = (char *) mem_malloc(strlen(tempname)+1);
		strcpy(rp->name,tempname);
	}

	World_changed = 1;
}


void CNewEditorApp::SetExtrudeDefaults(int which,float dist,BOOL delete_base_face,int inward,int faces,BOOL use_default)
{
	WriteProfileInt("Defaults","ex_which",which);
	CString strDist;
	strDist.Format("%.5f",dist);
	WriteProfileString("Defaults","ex_dist",strDist);
	WriteProfileInt("Defaults","ex_delete_base_face",delete_base_face);
	WriteProfileInt("Defaults","ex_inward",inward);
	WriteProfileInt("Defaults","ex_faces",faces);
	WriteProfileInt("Defaults","ex_use_default",use_default);
}

void CNewEditorApp::GetExtrudeDefaults(int *which,float *dist,BOOL *delete_base_face,int *inward,int *faces,BOOL *use_default)
{
	*which = GetProfileInt("Defaults","ex_which",NORMAL);
	CString strDist = GetProfileString("Defaults","ex_dist","20.0");
	*dist = atof(strDist);
	*delete_base_face = GetProfileInt("Defaults","ex_delete_base_face",FALSE);
	*inward = GetProfileInt("Defaults","ex_inward",1);
	*faces = GetProfileInt("Defaults","ex_faces",CURRENT);
	*use_default = GetProfileInt("Defaults","ex_use_default",FALSE);
}

void CNewEditorApp::OnFileSaveResourceLists() 
{
	// TODO: Add your command handler code here
	DlgSaveResourceLists(0);
}

void CNewEditorApp::OnUpdateFileSaveResourceLists(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

void CNewEditorApp::OnFileOpenResourceLists() 
{
	// TODO: Add your command handler code here
	DlgOpenResourceLists(CUSTOM_TEX_LIST | CUSTOM_OBJ_LIST);
}

void CNewEditorApp::OnUpdateFileOpenResourceLists(CCmdUI* pCmdUI) 
{
	// TODO: Add your command update UI handler code here
	
}

// SUPPORT FUNCTIONS

void SaveResourceLists(char *filename,DWORD flags)
{
	CFILE *outfile;
	CString buffer;
	char str[PAGENAME_LEN+10] = "";

	outfile=(CFILE *)cfopen (filename,"wt");
	if (!outfile)
	{
		mprintf ((0,"Couldn't save list %s!\n",filename));
		Int3();
		return;
	}

	if (!flags)
	{
		// Nothing to save!
		return;
	}

	// write out header
	cf_WriteString(outfile,DER_HEADER);
	buffer.Format("Version: %d",DER_VERSION);
	cf_WriteString(outfile,buffer.GetBuffer(0));
	cf_WriteString(outfile,"// !!!DO NOT EDIT ABOVE THIS LINE!!!");

	cf_WriteString(outfile,"");

	// Write texture header
	cf_WriteString(outfile,DER_TAG_TEXTURES);

	// TODO : check return values
	if (flags & CUSTOM_TEX_LIST)
		Editor_state.WriteTextureList(outfile,CUSTOM_TEX_LIST);
	if (flags & LEVEL_TEX_LIST)
		Editor_state.WriteTextureList(outfile,LEVEL_TEX_LIST);

	cf_WriteString(outfile,"");

	// Write object header
	cf_WriteString(outfile,DER_TAG_OBJECTS);

	// TODO : check return values
	if (flags & CUSTOM_OBJ_LIST)
		Editor_state.WriteObjectList(outfile,CUSTOM_OBJ_LIST);
	if (flags & LEVEL_OBJ_LIST)
		Editor_state.WriteObjectList(outfile,LEVEL_OBJ_LIST);

	cfclose(outfile);
}

void OpenResourceLists(char *filename,DWORD flags)
{
	CFILE *infile;
	char filebuffer[MAX_DER_FILE_BUFFER_LEN];
	int line_num;
	CString buffer;
	char str[PAGENAME_LEN+10] = "";
	int version = 0;

	infile=(CFILE *)cfopen (filename,"rt");
	if (!infile)
	{
		mprintf ((0,"Couldn't open list %s!\n",filename));
		Int3();
		return;
	}

	if (!flags)
	{
		// Nothing to open!
		return;
	}

	// Check for DER_HEADER
	cf_ReadString(filebuffer,MAX_DER_FILE_BUFFER_LEN,infile);
	if (strncmp(filebuffer,DER_HEADER,strlen(DER_HEADER)) != 0)
	{
		OutrageMessageBox("This is not a valid %s!",DER_HEADER);
		cfclose(infile);
		return;
	}

	// Get version number
	cf_ReadString(filebuffer,MAX_DER_FILE_BUFFER_LEN,infile);
	version = atoi(filebuffer+8);
	if (!version)
	{
		OutrageMessageBox("Cannot open file -- couldn't get version number!");
		cfclose(infile);
		return;
	}
	if (version > MAX_DER_VERSION_NUMBER)
	{
		OutrageMessageBox("%s version %d is not supported in this version of D3Edit!",DER_HEADER,version);
		cfclose(infile);
		return;
	}

	line_num = 0;

	// Parse lists
	if (version == 1)
	{
	while (!cfeof(infile))
	{
		// Read in a line from the file
		cf_ReadString(filebuffer,MAX_DER_FILE_BUFFER_LEN,infile);
		line_num++;

		// Remove whitespace padding at start and end of line
		StripLeadingTrailingSpaces(filebuffer);

		// If line is a comment, or empty, discard it
		if (strlen(filebuffer) == 0 || strncmp(filebuffer,"//",2)==0)
			continue;

		// If line begins with '[', identify the block and go to the 
		// appropriate handler
		if (*filebuffer == '[')
		{

		if ((flags & CUSTOM_TEX_LIST) && 
			strncmp(filebuffer,DER_TAG_TEXTURES,strlen(DER_TAG_TEXTURES)) == 0)
		{
			line_num += Editor_state.ReadTextureList(infile);
			continue;
		}

		if ((flags & CUSTOM_OBJ_LIST) && 
			strncmp(filebuffer,DER_TAG_OBJECTS,strlen(DER_TAG_OBJECTS)) == 0)
		{
			line_num += Editor_state.ReadObjectList(infile);
			continue;
		}

		}
	}
	}

	cfclose(infile);
}

UINT CALLBACK SaveListHook(HWND hDlg,UINT uiMsg,WPARAM wParam,LPARAM lParam)
{
	static OPENFILENAME *ofn = NULL;
	static DWORD flags = 0;

	switch (uiMsg)
	{
	case WM_INITDIALOG:
		ofn = (OPENFILENAME *) lParam;
		flags = ofn->lCustData;
		return TRUE;

	case WM_NOTIFY:
		switch (((NMHDR *) lParam)->code)
		{
		case CDN_INITDONE:
			if (flags & CUSTOM_TEX_LIST)
				::CheckDlgButton(hDlg,IDC_CUSTOM_TEX_LIST,BST_CHECKED);
			if (flags & LEVEL_TEX_LIST)
				::CheckDlgButton(hDlg,IDC_LEVEL_TEX_LIST,BST_CHECKED);
			if (flags & CUSTOM_OBJ_LIST)
				::CheckDlgButton(hDlg,IDC_CUSTOM_OBJ_LIST,BST_CHECKED);
			if (flags & LEVEL_OBJ_LIST)
				::CheckDlgButton(hDlg,IDC_LEVEL_OBJ_LIST,BST_CHECKED);
			break;

		case CDN_FILEOK:
			if (::IsDlgButtonChecked(hDlg,IDC_CUSTOM_TEX_LIST))
				flags |= CUSTOM_TEX_LIST;
			if (::IsDlgButtonChecked(hDlg,IDC_LEVEL_TEX_LIST))
				flags |= LEVEL_TEX_LIST;
			if (::IsDlgButtonChecked(hDlg,IDC_CUSTOM_OBJ_LIST))
				flags |= CUSTOM_OBJ_LIST;
			if (::IsDlgButtonChecked(hDlg,IDC_LEVEL_OBJ_LIST))
				flags |= LEVEL_OBJ_LIST;
			ASSERT(ofn != NULL);
			ofn->lCustData = flags;
			break;
		}
		return TRUE;
	}
	return FALSE;
}

UINT CALLBACK OpenListHook(HWND hDlg,UINT uiMsg,WPARAM wParam,LPARAM lParam)
{
	static OPENFILENAME *ofn = NULL;
	static DWORD flags = 0;

	switch (uiMsg)
	{
	case WM_INITDIALOG:
		ofn = (OPENFILENAME *) lParam;
		flags = ofn->lCustData;
		return TRUE;

	case WM_NOTIFY:
		switch (((NMHDR *) lParam)->code)
		{
		case CDN_INITDONE:
			if (flags & CUSTOM_TEX_LIST)
				::CheckDlgButton(hDlg,IDC_CUSTOM_TEX_LIST,BST_CHECKED);
			if (flags & CUSTOM_OBJ_LIST)
				::CheckDlgButton(hDlg,IDC_CUSTOM_OBJ_LIST,BST_CHECKED);
			break;

		case CDN_FILEOK:
			if (::IsDlgButtonChecked(hDlg,IDC_CUSTOM_TEX_LIST))
				flags |= CUSTOM_TEX_LIST;
			if (::IsDlgButtonChecked(hDlg,IDC_CUSTOM_OBJ_LIST))
				flags |= CUSTOM_OBJ_LIST;
			ASSERT(ofn != NULL);
			ofn->lCustData = flags;
			break;
		}
		return TRUE;
	}
	return FALSE;
}

void DlgSaveResourceLists(DWORD flags) 
{
	static char szFilter[] = "D3Edit Resource List (*.der)|*.der|All Files (*.*)|*.*||";
	CString SelectedFile;
	char name[_MAX_PATH];
	char ext[10];
	CString DefaultPath;
	char path[_MAX_PATH*2];
	CFileDialog *fd;
	DefaultPath = theApp.GetProfileString("Defaults","LastListDir","");


	//Need to do something with the default path
	_chdir(DefaultPath);
	fd = new CFileDialog(false,".der",NULL,OFN_HIDEREADONLY|OFN_EXPLORER|OFN_ENABLEHOOK|OFN_ENABLETEMPLATE,szFilter);
	fd->m_ofn.lpstrTitle = "Save Resource List(s)";
	fd->m_ofn.lpfnHook = &SaveListHook;
	fd->m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_SAVELISTS);
	fd->m_ofn.lCustData = flags;

	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();

	mprintf((0,"Saving %s\n",(const char *)SelectedFile));

	SplitPath(SelectedFile,path,name,ext);

	flags = fd->m_ofn.lCustData;

	delete fd;

	char listname[LIST_NAME_LEN] = "";
	if ( (strlen(name) + strlen(ext)) <= LIST_NAME_LEN )
	{
		sprintf(listname,"%s%s",name,ext);
		SaveResourceLists(listname,flags);
		AfxGetApp()->WriteProfileString("Defaults","LastListDir",path);
	}
	else
	{
		AfxMessageBox("Filename exceeds allowed number of characters. List not saved.",MB_ICONERROR);
		return;
	}
}

void DlgOpenResourceLists(DWORD flags) 
{
	static char szFilter[] = "D3Edit Resource List (*.der)|*.der|All Files (*.*)|*.*||";
	CString SelectedFile;
	char name[_MAX_PATH];
	char ext[10];
	CString DefaultPath;
	char path[_MAX_PATH*2];
	CFileDialog *fd;
	DefaultPath = theApp.GetProfileString("Defaults","LastListDir","");


	//Need to do something with the default path
	_chdir(DefaultPath);
	fd = new CFileDialog(true,".der",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY|OFN_EXPLORER|OFN_ENABLEHOOK|OFN_ENABLETEMPLATE,szFilter);
	fd->m_ofn.lpstrTitle = "Open Resource List(s)";
	fd->m_ofn.lpfnHook = &OpenListHook;
	fd->m_ofn.lpTemplateName = MAKEINTRESOURCE(IDD_OPENLISTS);
	fd->m_ofn.lCustData = flags;

	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();

	mprintf((0,"Opening %s\n",(const char *)SelectedFile));

	SplitPath(SelectedFile,path,name,ext);

	flags = fd->m_ofn.lCustData;

	delete fd;

	char path_and_listname[_MAX_PATH+LIST_NAME_LEN] = "";
	if ( (strlen(name) + strlen(ext)) <= LIST_NAME_LEN )
	{
		sprintf(path_and_listname,"%s%s%s",path,name,ext);
		OpenResourceLists(path_and_listname,flags);
		AfxGetApp()->WriteProfileString("Defaults","LastListDir",path);
		AfxGetApp()->WriteProfileString("Defaults","LastListName",path_and_listname);
	}
	else
	{
		AfxMessageBox("Filename exceeds allowed number of characters. List not opened.",MB_ICONERROR);
		return;
	}
}

CRoomFrm * CNewEditorApp::FindRoomWnd(room *rp)
{
	// Check the main room frame
	if ( m_pRoomFrm != NULL )
	{
		ASSERT(m_pLevelWnd != NULL);
		if ( m_pLevelWnd->m_Prim.roomp == rp )
			return m_pRoomFrm;
	}

	// Check the palette room frames
	for (int i=0; i<MAX_PALETTE_ROOMS; i++)
		if ( m_ppPaletteRoomFrms[i] != NULL )
			if ( m_ppPaletteRoomFrms[i]->m_Prim.roomp == rp )
				return m_ppPaletteRoomFrms[i];

	return NULL;
}

