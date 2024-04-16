#include <windows.h>
#include <assert.h>

#if 0

#include "mono.h"
//#include "pserror.h"
#include "gameos.h"


LRESULT WINAPI MyWndProc( HWND hWnd,UINT msg,UINT wParam,LPARAM lParam);


/* class osWinObject
	contains code to initialize all the OS dependent info and structures for an application.
	the game os kernal, so to speak.
*/

osWinObject::osWinObject()
{
	m_WinWidth = -1;
	m_WinHeight = -1;
	m_TopMost = 0;
	ZeroMemory(&m_WinInfo, sizeof(win_app_info));
}


osWinObject::~osWinObject() 
{
	HWND hwnd = (HWND)m_WinInfo.hwnd;
	HINSTANCE hinst = (HINSTANCE)m_WinInfo.hinstance;

/*	I guess we should destroy this window, here, now. */
	char str[32];
	GetClassName(hwnd, str, 31);

	if (hwnd && m_Created) {
	//	do this only if we created the window, not just initializing the window
		DestroyWindow(hwnd);
		UnregisterClass(str, hinst);
	}
}


/*	Creation functions
		Creates the application window and does other initialization
		res_handle = generic resource handle provided by OS to app.
*/

void osWinObject::init(osObject *parent_os, void *info)
{
	win_app_info *winfo = (win_app_info *)info;
	
	memcpy(&m_WinInfo, info, sizeof(win_app_info));

	m_ParentOS = parent_os;

//	Invoke os_init from virtual class object and hopefully it will call this osWinObject::os_init
	if (!parent_os) os_init();

	m_Init = 1;
}


bool osWinObject::create(osObject *parent_os, void *info)
{
	win_app_info *winfo = (win_app_info *)info;
	WNDCLASS wc;
	HWND hwnd;
	HINSTANCE hinst;

	assert(!m_Init && !m_Created);

	m_ParentOS = parent_os;

	hinst = (HINSTANCE)winfo->hinstance;

	wc.hCursor				= LoadCursor(NULL, IDC_ARROW);
	wc.hIcon				= NULL;
	wc.lpszMenuName			= NULL;
	wc.lpszClassName 		= (LPCSTR)winfo->name;
	wc.hbrBackground 		= (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.hInstance 			= hinst;
	wc.style				= CS_DBLCLKS;
	wc.lpfnWndProc			= (WNDPROC)(MyWndProc);
	wc.cbWndExtra			= 0;
	wc.cbClsExtra			= 0;

	if (!RegisterClass(&wc)) {
		mprintf((0, "Failure to register window class (err:%x).\n",GetLastError()));
		return 0;
	}

//	initialize main window and display it.
	RECT rect;
	DWORD dw;
	SetRect(&rect, 0, 0, m_WinWidth, m_WinHeight);
	AdjustWindowRect(&rect, WS_POPUP | WS_SYSMENU, FALSE);

	if (m_WinWidth == -1) m_WinWidth = GetSystemMetrics(SM_CXSCREEN);
	else m_WinWidth = rect.right - rect.left;
	if (m_WinHeight == -1) m_WinHeight= GetSystemMetrics(SM_CYSCREEN);
	else m_WinHeight = rect.bottom - rect.top;

	if (m_TopMost) dw = WS_EX_TOPMOST;
	else dw = 0;

	hwnd = CreateWindowEx(dw,
					(LPCSTR)winfo->name, 
					(LPCSTR)winfo->name,
					WS_POPUP | WS_SYSMENU,
					0, 0,
					m_WinWidth,
					m_WinHeight,
					NULL, 
					NULL, 
					hinst, 
					NULL);
	
	if (hwnd == NULL) {
		mprintf((0, "Failed to create game window (topmost=%d) (err: %x)\n", m_TopMost,GetLastError()));
		return 0;
	}

	ShowWindow(hwnd, SW_SHOWNORMAL);
	UpdateWindow(hwnd);

	winfo->hwnd = (unsigned)hwnd;

	memcpy(&m_WinInfo, winfo, sizeof(win_app_info));

//	Invoke os_init from virtual class object and hopefully it will call this osWinObject::os_init
	if (!parent_os) os_init();

	m_Created = m_Init = 1;

	return 1;
}


/*	Main OS interface:  The message pump
*/

gameos_packet *osWinObject::defer()
{
	MSG msg;

	assert(m_Init || (m_Created && m_Init));

	m_OSPacket.code = GAMEOS_UNKNOWN;

	while (1)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				m_OSPacket.code = GAMEOS_QUIT;
				m_OSPacket.time_stamp = (unsigned)msg.time;
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			m_OSPacket.code = GAMEOS_IDLE;
			m_OSPacket.time_stamp = 0;
			break;
		} 
	}

	return &m_OSPacket;
}


//	---------------------------------------------------------------------------
//	OS Specific initialization
//	---------------------------------------------------------------------------


void osWinObject::size_window(int width, int height, int topmost)
{
	HWND hwnd = (HWND)m_WinInfo.hwnd;
	RECT rect;
	void *dw;

	if (m_Created) {
		if (topmost) dw = HWND_TOPMOST;
		else dw = HWND_TOP;
		SetRect(&rect, 0,0,width,height);
		AdjustWindowRect(&rect, GetWindowLong(hwnd, GWL_STYLE), FALSE);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, rect.right-rect.left, 
				rect.bottom - rect.top, SWP_NOMOVE);
	}
	else {
		m_WinWidth = width, m_WinHeight= height;
		if (topmost) m_TopMost = 1;
		else m_TopMost = 0;
	}
}


void osWinObject::get_info(void *info, int size_str)
{
	memcpy(info, &m_WinInfo, size_str);
}


void osWinObject::os_init()
{
	int mmx_present;

//	initialization of debugging consoles.
	os_ConsoleInit();

	os_ConsoleOpen( 0, 9, 1, 78, 15, "Debug Spew");
    os_ConsoleOpen( 1, 1, 1, 58,  6, "Warnings");
	os_ConsoleOpen( 2, 1,61, 18,  6, "Stats");

	mprintf((0, "Win32 initialization...\n"));

// Detect if mmx is present
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

	if (mmx_present)
		mprintf ((0,"MMX technology found!\n"));
	else 
		mprintf ((0,"MMX not detected.\n"));

	mprintf((0, "\n"));
}


// For our self-modifying code, we must tell Windows that it is ok to write in the 
// code segment
// This function makes our speed intensive loops self-modifiable (sp?)



/* Main Windows Procedure for this OS object
*/
LRESULT WINAPI MyWndProc( HWND hWnd,UINT msg,UINT wParam,LPARAM lParam)
{
	return osWinObject::OEWndProc((unsigned)hWnd, (unsigned)msg, (unsigned)wParam, (long)lParam);
}
		

int osWinObject::OEWndProc(unsigned hWnd, unsigned msg, unsigned wParam, long lParam)
{
	return DefWindowProc((HWND)hWnd, (UINT)msg, (UINT)wParam, (LPARAM)lParam);
}

#endif //dead
