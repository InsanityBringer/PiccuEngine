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
 


#include "stdafx.h"
#include "neweditor.h"
#include "ned_GrWnd.h"
#include "ned_Rend.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// define taken from main/texmap/texture.cpp
#define ZBUFFER_MAX_DIMENSION	1000

extern bool  UseHardware;

/////////////////////////////////////////////////////////////////////////////
// Cned_GrWnd

IMPLEMENT_DYNCREATE(Cned_GrWnd, CWnd)

Cned_GrWnd::Cned_GrWnd()
{
	m_BackColor = GR_RGB(0,0,0);
	m_bInitted = false;
}

Cned_GrWnd::~Cned_GrWnd()
{
}



BEGIN_MESSAGE_MAP(Cned_GrWnd, CWnd)
	//{{AFX_MSG_MAP(Cned_GrWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// Cned_GrWnd message handlers

int Cned_GrWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// TODO: Add your specialized creation code here
//	Create a screen the size of the new client area
	RECT rect;
	int client_width, client_height;

	GetClientRect(&rect);
	client_width = (rect.right-rect.left);
	client_height = (rect.bottom-rect.top);

	if (!UseHardware)
	{
		if (client_width > ZBUFFER_MAX_DIMENSION)
			client_width = ZBUFFER_MAX_DIMENSION;
		if (client_height> ZBUFFER_MAX_DIMENSION)
			client_height = ZBUFFER_MAX_DIMENSION;
	}

	if (client_width && client_height) 
	{
		/*ASSERT(m_grScreen == NULL);
		// TODO : add m_Name and pass it to grScreen here, instead of NULL
		m_grScreen = new grScreen(client_width, client_height, BPP_16, NULL);
		m_grScreen->attach_to_window((unsigned)m_hWnd);
		m_grViewport = new grViewport(m_grScreen);*/

		m_handle = rend_NewContext(m_hWnd);
	}

	return 0;
}

void Cned_GrWnd::OnDestroy() 
{
	CWnd::OnDestroy();
	
	// TODO: Add your message handler code here
//	destroy screen element
	if (m_handle)
		rend_DestroyContext(m_handle);
}

void Cned_GrWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	switch(Renderer_type)
	{
	case RENDERER_OPENGL:
		{
			//SwapBuffers(renderer_hDC);
			//ValidateRect(NULL);
		}break;
	}
	
	// Do not call CWnd::OnPaint() for painting messages
}

void Cned_GrWnd::OnSize(UINT nType, int cx, int cy) 
{
	if (!UseHardware)
	{
		if (cx > ZBUFFER_MAX_DIMENSION)
			cx = ZBUFFER_MAX_DIMENSION;
		if (cy> ZBUFFER_MAX_DIMENSION)
			cy = ZBUFFER_MAX_DIMENSION;
	}

	CWnd::OnSize(nType, cx, cy);

	/*if (m_grViewport) {
		delete m_grViewport;
		delete m_grScreen;
		m_grViewport = NULL;
		m_grScreen = NULL;
	}
	if (cx && cy && !m_grViewport) {
		// TODO : add m_Name and pass it to grScreen here, instead of NULL
		m_grScreen = new grScreen(cx, cy, BPP_16, NULL);
		m_grScreen->attach_to_window((unsigned)m_hWnd);
		m_grViewport = new grViewport(m_grScreen);
	}*/

	m_handle.default_viewport.width = cx;
	m_handle.default_viewport.height = cy;
}

BOOL Cned_GrWnd::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style &= ~CS_PARENTDC;
		
	return CWnd::PreCreateWindow(cs);
}


void Cned_GrWnd::SetDCPixelFormat(HDC hDC)
{
	//handled in rend
	/*int nPixelFormat;
	static PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof(PIXELFORMATDESCRIPTOR),		//size of this structure
		1,									//version of this structure
		PFD_DRAW_TO_WINDOW|					//Draw to window (not bitmap)
		PFD_SUPPORT_OPENGL|					//Support OpenGL calls
		PFD_DOUBLEBUFFER,					//Double buffered mode (?)
		PFD_TYPE_RGBA,						//RGBA Color mode
		16,									//Want 16 bit color
		0,0,0,0,0,0,						//Not used to select mode
		0,0,								//Not used to select mode
		0,0,0,0,0,							//Not used to select mode
		16,									//size of depth buffer
		0,									//Not used to select mode
		0,									//Not used to select mode
		PFD_MAIN_PLANE,						//Draw in main plane
		0,									//Not used to select mode
		0,0,0								//Not used to select mode
	};

	//Choose a pixel format that best matches that described
	nPixelFormat = ChoosePixelFormat(hDC,&pfd);

	// Set Pixel format for device context
	SetPixelFormat(hDC,nPixelFormat,&pfd);*/
}
