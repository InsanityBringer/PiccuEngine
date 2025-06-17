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
 // SplashScreen.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "SplashScreen.h"

#include "Application.h"

extern oeWin32Application *g_OuroeApp;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSplashScreen dialog

#define SPLASH_SCREEN_LIFETIME	3000

CSplashScreen::CSplashScreen(CWnd* pParent )
	: CDialog(CSplashScreen::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSplashScreen)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Positioned = false;
	m_SplashBmp.LoadBitmap(IDB_SPLASH);
	Create(IDD_SPLASH);
	if(IsWindow(m_hWnd))
	{
		ShowWindow(SW_SHOW);
		BringWindowToTop();
	}
}


void CSplashScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSplashScreen)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSplashScreen, CDialog)
	//{{AFX_MSG_MAP(CSplashScreen)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSplashScreen message handlers

int CSplashScreen::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	m_Timer = SetTimer(0x142,SPLASH_SCREEN_LIFETIME,NULL);
	
	return 0;
}

void CSplashScreen::OnPaint() 
{
	
	CPaintDC dc(this); // device context for painting
	
	CDC sdc;			// source dc
	CBitmap *bmp;
	CSize textdim;
	BITMAP bm;
	RECT uprect;

	if(!m_Positioned)
	{
		//Set the default position here 
		m_Positioned = true;

	}

	GetClientRect(&uprect);

	m_SplashBmp.GetObject(sizeof(bm), &bm);
	
	sdc.CreateCompatibleDC(NULL);
	bmp = sdc.SelectObject(&m_SplashBmp);
	dc.StretchBlt(uprect.left+1,uprect.top+1,uprect.right-uprect.left-2,uprect.bottom-uprect.top-2, &sdc, 0,0,bm.bmWidth,bm.bmHeight,SRCCOPY); 
	sdc.SelectObject(bmp);

	//fnt = dc.SelectObject(GetFont());
	//dc.SetBkMode(TRANSPARENT);
	//dc.SetTextColor(RGB(255,255,255));

	//dc.SelectObject(fnt);
	
	// Do not call CDialog::OnPaint() for painting messages
}

void CSplashScreen::OnTimer(UINT nIDEvent) 
{
	if(m_Timer)
	{
		KillTimer(m_Timer);
		
		if(g_OuroeApp)
			DestroyWindow();
		else
			m_Timer = SetTimer(0x142,SPLASH_SCREEN_LIFETIME,NULL);
	}
	
	CDialog::OnTimer(nIDEvent);
}

BOOL CSplashScreen::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= WS_EX_TOPMOST;
	
	return CDialog::PreCreateWindow(cs);
}
