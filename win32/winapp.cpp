/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#define OEAPP_INTERNAL_MODULE	


#include "application.h"
//#include "AppConsole.h"
#include "mono.h"
#include "networking.h"
#include "win32res.h"

#define WIN32_LEAN_AND_MEAN
#include <shellapi.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include "pserror.h"
#include "win\directx\ddraw.h"
#include "win\directx\dsound.h"

// taken from winuser.h
#ifndef WHEEL_DELTA
#define WHEEL_DELTA	120
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL	0x20a
#endif

/* Main Windows Procedure for this OS object
*/
const int MAX_WIN32APPS = 4;

static struct tAppNodes 
{
	HWND hWnd;
	oeWin32Application *app;
} Win32_AppObjects[MAX_WIN32APPS];

// system mouse info.
short w32_msewhl_delta=0;						// -val = up, pos val = down, 0 = no change.
bool w32_mouseman_hack=false;


/*	Win32 Application Object
		This object entails initialization and cleanup of all operating system
		elements, as well as data that libraries may need to initialize their 
		systems.  

	The Win32 Application object creates the application window and housekeeps
	the window and instance handle for the application.

	We also allow the option of setting these handles from outside the Application object.
*/
extern LRESULT WINAPI MyConProc( HWND hWnd,UINT msg,UINT wParam,LPARAM lParam);
extern void con_Defer();

bool oeWin32Application::os_initialized = false;
bool oeWin32Application::first_time = true;

//	this is the app's window proc.
LRESULT WINAPI MyWndProc( HWND hWnd,UINT msg,UINT wParam,LPARAM lParam);

//	Creates the window handle and instance
oeWin32Application::oeWin32Application(const char *name, unsigned flags, HInstance hinst)
	:oeApplication()
{
	WNDCLASS wc;
	RECT rect;
	
	if (oeWin32Application::first_time) {
		int i;
		for (i = 0; i < MAX_WIN32APPS; i++) 
		{
			Win32_AppObjects[i].hWnd = NULL;
			Win32_AppObjects[i].app = NULL;
		}
		oeWin32Application::first_time = false;
	}

	wc.hCursor				= NULL;
	//wc.hIcon				= LoadIcon((HINSTANCE)hinst, MAKEINTRESOURCE(IDI_APPICON));
	wc.hIcon				= (HICON)LoadImage((HINSTANCE)hinst, TEXT("IDI_APPICONSMALL"), IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
	wc.lpszMenuName			= NULL;
	wc.lpszClassName 		= (LPCSTR)name;
	wc.hInstance 			= (HINSTANCE)hinst;
	wc.style				= CS_DBLCLKS;
	wc.lpfnWndProc			= (flags & OEAPP_CONSOLE) ? (WNDPROC)MyConProc : (WNDPROC)(MyWndProc);
	wc.cbWndExtra			= 0;
	wc.cbClsExtra			= 0;

#ifdef RELEASE
	wc.hbrBackground 		= (flags & OEAPP_CONSOLE) ? (HBRUSH)GetStockObject(WHITE_BRUSH) : (HBRUSH)GetStockObject(BLACK_BRUSH);
#else
	wc.hbrBackground 		= (flags & OEAPP_CONSOLE) ? (HBRUSH)GetStockObject(WHITE_BRUSH) : (HBRUSH)GetStockObject(HOLLOW_BRUSH);
#endif

	if (!RegisterClass(&wc)) {
		mprintf((0, "Failure to register window class (err:%x).\n",GetLastError()));
		return;
	}

	if (flags & OEAPP_CONSOLE) {
		m_X = CW_USEDEFAULT;
		m_Y = CW_USEDEFAULT;
		m_W = 640;
		m_H = 480;
	}
	else {
	//	initialize main window and display it.
#ifdef RELEASE
		SetRect(&rect, 0,0,GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
#else
		SetRect(&rect, 0,0,640,480);
#endif
//		AdjustWindowRect(&rect, WS_POPUP | WS_SYSMENU, FALSE);
		m_X = 0;
		m_Y = 0;
		m_W = rect.right - rect.left;
		m_H = rect.bottom - rect.top;
	}

	m_hInstance = hinst;
	m_Flags = flags;
	strcpy(m_WndName, name);

	os_init();

	m_hWnd = NULL;
	m_WasCreated = true;
	m_DeferFunc = NULL;
	w32_mouseman_hack = false;

	memset(m_MsgFn, 0, sizeof(m_MsgFn));
}
	
//	Create object with a premade window handle/instance
oeWin32Application::oeWin32Application(tWin32AppInfo *appinfo)
	:oeApplication()
{
	RECT rect;

//	store handles
	m_hWnd = appinfo->hwnd;
	m_hInstance = appinfo->hinst;
	m_Flags = appinfo->flags;

//	returns the dimensions of the window
	GetWindowRect((HWND)m_hWnd, &rect);
	appinfo->wnd_x = m_X = rect.left;
	appinfo->wnd_y = m_Y = rect.bottom;
	appinfo->wnd_w = m_W = rect.right-rect.left;
	appinfo->wnd_h = m_H = rect.bottom - rect.top;

	m_WasCreated = false;

	os_init();

	clear_handlers();

	m_DeferFunc = NULL;
	w32_mouseman_hack = false;
}
	

oeWin32Application::~oeWin32Application()
{
	HWND hwnd = (HWND)m_hWnd;
	HINSTANCE hinst = (HINSTANCE)m_hInstance;
	char str[32];

/*	I guess we should destroy this window, here, now. */
	GetClassName(hwnd, str, sizeof(str));

	if (m_WasCreated) {
	//	do this only if we created the window, not just initializing the window
		if (hwnd) 
			DestroyWindow(hwnd);

		UnregisterClass(str, hinst);
	}
}


//	initializes the object
void oeWin32Application::init()
{
	DWORD style, winstyle;

	if (!m_WasCreated) 
		return;

	if (m_Flags & OEAPP_CONSOLE || m_Flags & OEAPP_WINDOWED) 
	{
		style = 0;
		winstyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX;
	}
	else 
	{
		style = (m_Flags & OEAPP_TOPMOST) ? WS_EX_TOPMOST : 0;
		winstyle = WS_POPUP | WS_SYSMENU;
	}

	m_hWnd = (HWnd)CreateWindowEx(style,
						(LPCSTR)m_WndName, 
						(LPCSTR)m_WndName,
						winstyle,
						m_X, m_Y,
						m_W, m_H,
						NULL, 
						NULL, 
						(HINSTANCE)m_hInstance, 
						(LPVOID)this);

	if (m_hWnd == NULL) {
		DWORD err = GetLastError();
		mprintf((0, "Failed to create game window (err: %x)\n",err));
		return;
	}
	if (m_Flags & OEAPP_FULLSCREEN)
		ShowWindow((HWND)m_hWnd, SW_SHOWMAXIMIZED);
	else
		ShowWindow((HWND)m_hWnd, SW_SHOWNORMAL);
	UpdateWindow((HWND)m_hWnd);
}

void oeWin32Application::change_window()
{
	if (m_Flags & OEAPP_FULLSCREEN) 
	{
		SetWindowLongPtr((HWND)m_hWnd, GWL_STYLE, 0);
		SetWindowLongPtr((HWND)m_hWnd, GWL_EXSTYLE, WS_EX_TOPMOST);

		RECT r{};
		GetWindowRect((HWND)m_hWnd, &r);

		m_X = r.left;
		m_Y = r.top;
		m_W = r.right - r.left;
		m_H = r.bottom - r.top;

		int hehwidth = GetSystemMetrics(SM_CXSCREEN);
		int hehheight = GetSystemMetrics(SM_CYSCREEN);

		SetWindowPos((HWND)m_hWnd, HWND_TOP, 0, 0, hehwidth, hehheight, SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowWindow((HWND)m_hWnd, SW_NORMAL);
	}
	else 
	{
		SetWindowLongPtr((HWND)m_hWnd, GWL_STYLE, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX);
		SetWindowLongPtr((HWND)m_hWnd, GWL_EXSTYLE, 0);

		ShowWindow((HWND)m_hWnd, SW_NORMAL);
		SetWindowPos((HWND)m_hWnd, HWND_TOP, m_X, m_Y, m_W, m_H, SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
}

void oeWin32Application::set_flags(int newflags)
{
	int oldflags = m_Flags;
	m_Flags = newflags;

	if (m_Flags != oldflags)
		change_window();
}


//	Function to retrieve information from object through a platform defined structure.
void oeWin32Application::get_info(void *info)
{
	tWin32AppInfo *appinfo = (tWin32AppInfo *)info;

	appinfo->hwnd = m_hWnd;
	appinfo->hinst = m_hInstance;
	appinfo->flags = m_Flags;
	appinfo->wnd_x = m_X;
	appinfo->wnd_y = m_Y;
	appinfo->wnd_w = m_W;
	appinfo->wnd_h = m_H;
}

int oeWin32Application::flags(void) const
{
	return m_Flags;
}


void oeWin32Application::set_sizepos(int x, int y, int w, int h)
{
	if (!m_hWnd) 
		return;

	m_X = x;
	m_Y = y;
	m_W = w;
	m_H = h;

	MoveWindow((HWND)m_hWnd, x, y, w, h, TRUE);
}


// real defer code.
#define DEFER_PROCESS_ACTIVE			1		// process is still active
#define DEFER_PROCESS_INPUT_IDLE		2		// process input from os not pending.

//#include "ddio.h"

int oeWin32Application::defer_block()
{
	MSG msg;
//	static int n_iterations = 0;
//
//	if ((n_iterations % 200)==0) {
//		mprintf((0, "System timer at %f\n", timer_GetTime()));
//	}
//	n_iterations++;

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
		//	QUIT APP.
			exit(1);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		return DEFER_PROCESS_ACTIVE;
	}
	else {
	// IDLE PROCESSING
		if (m_DeferFunc)
			(*m_DeferFunc)(this->active());

		if (this->active()) {
		#ifndef _DEBUG
			if (GetForegroundWindow() != (HWND)this->m_hWnd && !(m_Flags & OEAPP_CONSOLE)) {
				mprintf((0, "forcing this window into the foreground.\n"));
				SetForegroundWindow((HWND)this->m_hWnd);
			}
		#endif
			return (DEFER_PROCESS_ACTIVE+DEFER_PROCESS_INPUT_IDLE);
		}
		else {
				//JEFF - Commented out because we don't want to wait until
				//a message is available, else we won't be able to do any 
				//multiplayer game background processing.
				//
				//WaitMessage();
				//mprintf((0, "Waiting...\n"));
			return (DEFER_PROCESS_INPUT_IDLE);
		}
	}

	DebugBreak();
	return (0);
}



//	defer returns some flags.   essentially this function defers program control to OS.
unsigned oeWin32Application::defer()
{
	int result;

	//con_Defer();

// system mouse info.
	w32_msewhl_delta = 0;

	int num_defers = 0;
	do
	{
		result = defer_block();
	}
	while ((result == DEFER_PROCESS_ACTIVE) || (result == DEFER_PROCESS_INPUT_IDLE) && ++num_defers < 6);

	return 0;
}


//	set a function to run when deferring to OS.
void oeWin32Application::set_defer_handler(void (*func)(bool))
{
	m_DeferFunc = func;
}


// initializes OS components.
void oeWin32Application::os_init()				
{
	tWin32OS os;
	int major, minor, build;

	os = oeWin32Application::version(&major, &minor, &build);

	


/*	We only need to do this once */
	if (!os_initialized) {
	//	Are we NT for 95.
		if (os == NoWin32) {
			MessageBox(NULL, "This application will only run under Win32 systems.", "Outrage Error", MB_OK);
			exit(1);
		}

//		OutrageMessageBox("%d", (int)os);
		

//@@	if (os == Win9x) {
//@@			OutrageMessageBox("Win9x");
//@@		}
//@@		else if (os == WinNT) {
//@@			OutrageMessageBox("WinNT %d.%d.%d system", major, minor, build);
//@@		}
//@@		else {
//@@			OutrageMessageBox("Non Win32");
//@@		}

	#ifdef _DEBUG
		if (os == Win9x) {
			mprintf((0, "Win9x system\n"));
		}
		else if (os == WinNT) {
			mprintf((0, "WinNT %d.%d.%d system\n", major, minor, build));
		}
		else {
			mprintf((0, "Win32 non-standard operating system\n"));
		}
	#endif
		
// Detect if mmx is present
	/*	int mmx_present=1;

		__asm
		{
			mov     eax, 1          ; request for feature flags
			_emit 0x0f 
 			_emit 0xa2 
			test    edx, 800000h    ; Is IA MMX technology bit (Bit 23 of EDX) in feature flags set?
			jz     done
			mov mmx_present, 1

			done:
		}
		
		if (mmx_present) {
			mprintf ((0,"MMX technology found!\n"));
		}
		else {
			mprintf ((0,"MMX not detected.\n"));
		}


	mprintf((0, "\n"));
	*/

		os_initialized = true;
	}

	m_NTFlag = (os==WinNT) ? true : false;
}


// retreive full version information
tWin32OS oeWin32Application::version(int *major, int *minor, int *build, char *strinfo)
{
	OSVERSIONINFO osinfo;
	tWin32OS os;

	osinfo.dwOSVersionInfoSize = sizeof(osinfo);

	if (!GetVersionEx(&osinfo)) {
		return NoWin32;
	}

	switch (osinfo.dwPlatformId)
	{
	case VER_PLATFORM_WIN32_WINDOWS:
		os = Win9x;
		if (build) {
			*build = LOWORD(osinfo.dwBuildNumber);
		}
		break;

	case VER_PLATFORM_WIN32_NT:
		os = WinNT;
		if (build) {
			*build = osinfo.dwBuildNumber;
		}
		break;

//@@	case VER_PLATFORM_WIN32_CE:
//@@		os = WinCE;
//@@		if (build) {
//@@			*build = osinfo.dwBuildNumber;
//@@		}
//@@		break;

	default:
		os = NoWin32;
		if (*build) {
			*build = osinfo.dwBuildNumber;
		}
	}

	*major = (int)osinfo.dwMajorVersion;
	*minor = (int)osinfo.dwMinorVersion;

	if (strinfo) {
		strcpy(strinfo, osinfo.szCSDVersion);
	}

	return os;
}


//	This Window Procedure is called from the global WindowProc.
int oeWin32Application::WndProc( HWnd hwnd, unsigned msg, unsigned wParam, long lParam)
{
 	switch (msg)
	{
	case WM_ACTIVATEAPP:
		m_AppActive = wParam ? true : false; 
	//	mprintf((0, "WM_ACTIVATEAPP (%u,%l)\n", wParam, lParam));
		break;
	}
			
	return DefWindowProc((HWND)hwnd, (UINT)msg, (UINT)wParam, (LPARAM)lParam);
}


//	These functions allow you to add message handlers.
bool oeWin32Application::add_handler(unsigned msg, tOEWin32MsgCallback fn)
{
	int i=0;

//	search for redundant callbacks.
	for (i = 0; i < MAX_MSG_FUNCTIONS; i++)
	{
		if (m_MsgFn[i].msg == msg && m_MsgFn[i].fn == fn) 
			return true;
	}

	for (i = 0; i < MAX_MSG_FUNCTIONS; i++)
	{
		if (m_MsgFn[i].fn == NULL) {
			m_MsgFn[i].msg = msg;
			m_MsgFn[i].fn = fn;
			return true;
		}
	}

	DebugBreak();							// We have reached the max number of message functions!

	return false;
}


// These functions remove a handler
bool oeWin32Application::remove_handler(unsigned msg, tOEWin32MsgCallback fn)
{
	int i;

	if (!fn)
		DebugBreak();

	for (i = 0; i < MAX_MSG_FUNCTIONS; i++) 
	{
		if (msg == m_MsgFn[i].msg && m_MsgFn[i].fn == fn) {
			m_MsgFn[i].fn = NULL;
			return true;
		}
	}

	return false;
}


// Run handler for message (added by add_handler)
bool oeWin32Application::run_handler(HWnd wnd, unsigned msg, unsigned wParam, long lParam)
{
	int j;
//	run user-defined message handlers
// the guess here is that any callback that returns a 0, will not want to handle the window's WndProc function.
	for (j = 0; j < MAX_MSG_FUNCTIONS; j++)
		if (msg == m_MsgFn[j].msg && m_MsgFn[j].fn) {
			if (!(*m_MsgFn[j].fn)(wnd, msg, wParam, lParam)) 
				return false;
		}

	return true;
}


void oeWin32Application::clear_handlers()
{
	int j;

	for (j = 0; j < MAX_MSG_FUNCTIONS; j++) 
		m_MsgFn[j].fn = NULL;
}


void oeWin32Application::delay(float secs)
{
	int result;
	DWORD msecs = (DWORD)(secs * 1000.0);
	DWORD time_start;

	w32_msewhl_delta = 0;

	time_start = timeGetTime();
	Sleep(0);
	while (timeGetTime() < (time_start+msecs)) 
	{
		this->defer_block();
	}

// block if messages are still pending (for task switching too, this call will not return until messages are clear
	/*do
	{
		result = this->defer_block();
	}
	while (result == DEFER_PROCESS_ACTIVE || result == DEFER_PROCESS_INPUT_IDLE);*/
}


LRESULT WINAPI MyWndProc( HWND hWnd,UINT msg,UINT wParam,LPARAM lParam)
{
	int i=-1;
	bool force_default = false;

	for (i = 0; i < MAX_WIN32APPS; i++)
		if (Win32_AppObjects[i].hWnd == hWnd) break;

	if (i == MAX_WIN32APPS)	
		i = -1;

	switch (msg)
	{
		LPCREATESTRUCT lpCreateStruct;
		case WM_CREATE:
		// here we store the this pointer to the app object this instance belongs to.
			lpCreateStruct = (LPCREATESTRUCT)lParam;
			for (i = 0; i < MAX_WIN32APPS; i++) 
				if (Win32_AppObjects[i].hWnd == NULL) break;
			if (i == MAX_WIN32APPS) 
				debug_break();
			Win32_AppObjects[i].hWnd = hWnd;
			Win32_AppObjects[i].app = (oeWin32Application *)lpCreateStruct->lpCreateParams;

			Win32_AppObjects[i].app->clear_handlers();

			force_default = true;
			break;

		case WM_DESTROY:
		//	get window handle and clear it.
			if (i == MAX_WIN32APPS) 
				debug_break();
			Win32_AppObjects[i].hWnd = NULL;
			Win32_AppObjects[i].app = NULL;
			i = -1;
			break;

		case WM_SYSCOMMAND:
		// bypass screen saver and system menu.
			if((wParam & 0xFFF0)==SC_SCREENSAVE || (wParam & 0xFFF0)==SC_MONITORPOWER) 
				return 0; 
			if ((wParam & 0xfff0)== SC_KEYMENU) 
				return 0;                                                     
			break;      

		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
			if (lParam & 0x20000000) 
				return 0;
			break;

		case WM_POWERBROADCAST:					// Won't allow OS to suspend operation for now.
			mprintf((0, "WM_POWERBROADCAST=%u,%d\n", wParam, lParam));
			if (wParam == PBT_APMQUERYSUSPEND) {
				return BROADCAST_QUERY_DENY;
			}
			break;

		case WM_MOUSEWHEEL:
		case 0xcc41:
			if (w32_mouseman_hack) {
				if (msg!=0xcc41) {
					w32_msewhl_delta = HIWORD(wParam);
				}
				else {
					w32_msewhl_delta = (short)(wParam);
				}
			}
			else if (msg == WM_MOUSEWHEEL) {
				w32_msewhl_delta = HIWORD(wParam);
			}
			break;
	}
	
	oeWin32Application *winapp = nullptr;

	// zar: this used to just addres -1 directly. it didn't break anything but
	// it sure triggered asan.
	if (i != -1) 
		winapp = Win32_AppObjects[i].app;


//	if this window not on list, then run default window proc.
	if (i == -1 || winapp == NULL || force_default) 
		return DefWindowProc(hWnd, msg, wParam, lParam);

	if (!winapp->run_handler((HWnd)hWnd, (unsigned)msg, (unsigned)wParam, (long)lParam))
		return 0;
	
// run user defined window procedure.
	return 
		(LRESULT)winapp->WndProc((HWnd)hWnd, (unsigned)msg, (unsigned)wParam, (long)lParam);
}


// detect if application can handle what we want of it.
bool oeWin32Application::GetSystemSpecs(const char *fname)
{
	FILE *fp = fopen(fname, "wt");
	tWin32OS os;
	int maj,min, build;
	char desc[256];

	if (!fp) return false;

	os = oeWin32Application::version(&maj,&min,&build, desc);

	fprintf(0, "OS: %s %d.%d.%d %s\n", (os==Win9x) ? "Win9x" : (os==WinNT) ? "WinNT" : (os==WinCE) ? "WinCE" : "Non standard Win32", maj,min, build,desc);

// get system memory info
	MEMORYSTATUS mem_stat;

	mem_stat.dwLength = sizeof(MEMORYSTATUS);
	GlobalMemoryStatus(&mem_stat);

	fprintf(fp, "Memory:\n");
	fprintf(fp, "\tLoad:\t\t\t%u\n\tTotalPhys:\t\t%u\n\tAvailPhys:\t\t%u\nPageFile:\t\t%u\n", 
					(unsigned)mem_stat.dwMemoryLoad, (unsigned)mem_stat.dwTotalPhys, (unsigned)mem_stat.dwAvailPhys, (unsigned)mem_stat.dwTotalPageFile);
	fprintf(fp, "\tPageFileFree:\t%u\n\tVirtual:\t\t%u\n\tVirtualFree:\t%u\n", 
					(unsigned)mem_stat.dwAvailPageFile, (unsigned)mem_stat.dwTotalVirtual, (unsigned)mem_stat.dwAvailVirtual);

	fclose(fp);

	return true;
}


