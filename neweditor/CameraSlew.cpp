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
 // CameraSlew.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCameraSlew dialog


CCameraSlew::CCameraSlew()
		: CDialogBar()	
//: CDialogBar(CCameraSlew::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCameraSlew)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_AttachedCamera = NULL;
	m_AttachedOrthoCamera = NULL;
	m_bOrtho = false;
	m_Title.Empty();

	m_MovementScaler = .5;

	m_LastSlideUp = 0;
	m_LastSlideLeft = 0;	
	m_LastSlideRight = 0;
	m_LastSlideDown = 0;
	m_LastSlideForward = 0;
	m_LastSlideReverse = 0;
	m_LastRotateUp = 0;
	m_LastRotateDown = 0;
	m_LastRotateLeft = 0;
	m_LastRotateRight = 0;
	m_LastZoomIn = 0;
	m_LastZoomOut = 0;
}


void CCameraSlew::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCameraSlew)
	DDX_Control(pDX, IDC_SENSSLIDER, m_SensSlider);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCameraSlew, CDialogBar)
	//{{AFX_MSG_MAP(CCameraSlew)
	ON_BN_CLICKED(IDC_ORIGINDOWN, OnOrigindown)
	ON_BN_CLICKED(IDC_ORIGINFORWARD, OnOriginforward)
	ON_BN_CLICKED(IDC_ORIGINLEFT, OnOriginleft)
	ON_BN_CLICKED(IDC_ORIGINREVERSE, OnOriginreverse)
	ON_BN_CLICKED(IDC_ORIGINRIGHT, OnOriginright)
	ON_BN_CLICKED(IDC_ORIGINUP, OnOriginup)
	ON_BN_CLICKED(IDC_ROTATELEFT, OnRotateleft)
	ON_BN_CLICKED(IDC_ROTATERIGHT, OnRotateright)
	ON_BN_CLICKED(IDC_ROTATEUP, OnRotateup)
	ON_BN_CLICKED(IDC_ROTATEDOWN, OnRotatedown)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SENSSLIDER, OnReleasedcaptureSensslider)
	ON_BN_CLICKED(IDC_ZOOMIN, OnZoomin)
	ON_BN_CLICKED(IDC_ZOOMOUT, OnZoomout)
	ON_WM_CREATE()
	ON_NOTIFY(NM_OUTOFMEMORY, IDC_SENSSLIDER, OnOutofmemorySensslider)
	ON_WM_DESTROY()
	ON_UPDATE_COMMAND_UI(IDC_ORIGINDOWN,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ORIGINFORWARD,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ORIGINLEFT,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ORIGINREVERSE,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ORIGINRIGHT,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ORIGINUP,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ROTATELEFT,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ROTATERIGHT,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ROTATEUP,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ROTATEDOWN,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ZOOMIN,OnEngineerUpdStyle)
	ON_UPDATE_COMMAND_UI(IDC_ZOOMOUT,OnEngineerUpdStyle)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCameraSlew message handlers

void CCameraSlew::OnOrigindown() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		m_AttachedCamera->target += m_AttachedCamera->orient.uvec * (-(100*m_MovementScaler));
		m_AttachedCamera->view_changed = 1;
	}
	else if (m_AttachedOrthoCamera && m_bOrtho)
	{
		m_AttachedOrthoCamera->pos.y -= 100*m_MovementScaler;
		m_AttachedOrthoCamera->view_changed = 1;
	}
}

void CCameraSlew::OnOriginforward() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		m_AttachedCamera->target += m_AttachedCamera->orient.fvec * (100*m_MovementScaler);
		m_AttachedCamera->view_changed = 1;
	}
}

void CCameraSlew::OnOriginleft() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		m_AttachedCamera->target += m_AttachedCamera->orient.rvec * (-(100*m_MovementScaler));
		m_AttachedCamera->view_changed = 1;
	}
	else if (m_AttachedOrthoCamera && m_bOrtho)
	{
		m_AttachedOrthoCamera->pos.x -= 100*m_MovementScaler;
		m_AttachedOrthoCamera->view_changed = 1;
	}
}

void CCameraSlew::OnOriginreverse() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		m_AttachedCamera->target += m_AttachedCamera->orient.fvec * (-(100*m_MovementScaler));
		m_AttachedCamera->view_changed = 1;
	}
}

void CCameraSlew::OnOriginright() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		m_AttachedCamera->target += m_AttachedCamera->orient.rvec * (100*m_MovementScaler);
		m_AttachedCamera->view_changed = 1;
	}
	else if (m_AttachedOrthoCamera && m_bOrtho)
	{
		m_AttachedOrthoCamera->pos.x += 100*m_MovementScaler;
		m_AttachedOrthoCamera->view_changed = 1;
	}
}

void CCameraSlew::OnOriginup() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		m_AttachedCamera->target += m_AttachedCamera->orient.uvec * (100*m_MovementScaler);
		m_AttachedCamera->view_changed = 1;
	}
	else if (m_AttachedOrthoCamera && m_bOrtho)
	{
		m_AttachedOrthoCamera->pos.y += 100*m_MovementScaler;
		m_AttachedOrthoCamera->view_changed = 1;
	}
}


void CCameraSlew::OnRotatedown() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		matrix tempm,rotmat;

		vm_AnglesToMatrix(&rotmat, (((float)(65535.0f/20))*1)*m_MovementScaler, 0.0, 0.0);
		tempm = m_AttachedCamera->orient;
		m_AttachedCamera->orient = tempm * rotmat;
		vm_Orthogonalize(&m_AttachedCamera->orient);
		m_AttachedCamera->view_changed = 1;
	}	
	
}



void CCameraSlew::OnRotateleft() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		matrix tempm,rotmat;
		
		vm_AnglesToMatrix(&rotmat, 0.0, (((float)(65535.0f/20))*-1)*m_MovementScaler, 0.0);
		tempm = m_AttachedCamera->orient;
		vm_Orthogonalize(&m_AttachedCamera->orient);
		m_AttachedCamera->orient = tempm * rotmat;

		m_AttachedCamera->view_changed = 1;
	}	
	
}

void CCameraSlew::OnRotateright() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		matrix tempm,rotmat;

		
		vm_AnglesToMatrix(&rotmat, 0.0, (((float)(65535.0f/20))*	1)*m_MovementScaler, 0.0);
		tempm = m_AttachedCamera->orient;
		m_AttachedCamera->orient = tempm * rotmat;
		vm_Orthogonalize(&m_AttachedCamera->orient);

		
		m_AttachedCamera->view_changed = 1;
	}	
	
}

void CCameraSlew::OnRotateup() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		matrix tempm,rotmat;

		vm_AnglesToMatrix(&rotmat, (((float)(65535.0f/20))*-1)*m_MovementScaler, 0.0, 0.0);
		tempm = m_AttachedCamera->orient;
		m_AttachedCamera->orient = tempm * rotmat;
		vm_Orthogonalize(&m_AttachedCamera->orient);
		m_AttachedCamera->view_changed = 1;
	}	
	
}

void CCameraSlew::OnReleasedcaptureSensslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int pos = m_SensSlider.GetPos();

	m_MovementScaler = (float)((float)pos/100);
		
	*pResult = 0;
}

void CCameraSlew::OnZoomin() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		m_AttachedCamera->dist -= (100*m_MovementScaler);
		m_AttachedCamera->view_changed = 1;
	}
	else if (m_AttachedOrthoCamera && m_bOrtho)
	{
		m_AttachedOrthoCamera->scale += 5*m_MovementScaler;
		if (m_AttachedOrthoCamera->scale > O_MAX_SCALE)
			m_AttachedOrthoCamera->scale = O_MAX_SCALE;
		m_AttachedOrthoCamera->view_changed = 1;
	}
}

void CCameraSlew::OnZoomout() 
{
	if(m_AttachedCamera && !m_bOrtho)
	{
		m_AttachedCamera->dist += (100*m_MovementScaler);
		m_AttachedCamera->view_changed = 1;
	}
	else if (m_AttachedOrthoCamera && m_bOrtho)
	{
		m_AttachedOrthoCamera->scale -= 5*m_MovementScaler;
		if (m_AttachedOrthoCamera->scale < O_MIN_SCALE)
			m_AttachedOrthoCamera->scale = O_MIN_SCALE;
		m_AttachedOrthoCamera->view_changed = 1;
	}
}

void CCameraSlew::AttachCamera(camera *cam,char *title)
{
	m_AttachedCamera = cam;
	m_AttachedOrthoCamera = NULL;
	m_bOrtho = false;
	m_Title = title;	
	if(IsWindow(m_hWnd))
		SetWindowText(m_Title);
	
	if(!IsWindow(m_SensSlider.m_hWnd))
		m_SensSlider.Attach(GetDlgItem(IDC_SENSSLIDER)->m_hWnd);
	
	if(IsWindow(m_SensSlider.m_hWnd))
	{
		m_SensSlider.SetRange(0,100);
		m_SensSlider.SetPos(m_MovementScaler*100);
	}
}

void CCameraSlew::AttachOrthoCamera(o_camera *cam,char *title)
{
	m_AttachedCamera = NULL;
	m_AttachedOrthoCamera = cam;
	m_bOrtho = true;
	m_Title = title;	
	if(IsWindow(m_hWnd))
		SetWindowText(m_Title);
	
	if(!IsWindow(m_SensSlider.m_hWnd))
		m_SensSlider.Attach(GetDlgItem(IDC_SENSSLIDER)->m_hWnd);
	
	if(IsWindow(m_SensSlider.m_hWnd))
	{
		m_SensSlider.SetRange(0,100);
		m_SensSlider.SetPos(m_MovementScaler*100);
	}
}



int CCameraSlew::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialogBar::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	SetWindowText(m_Title);
	
	return 0;
}

void CCameraSlew::OnOutofmemorySensslider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	// TODO: Add your control notification handler code here
	
	*pResult = 0;
}

void CCameraSlew::OnDestroy() 
{
	if(IsWindow(m_SensSlider.m_hWnd))
		m_SensSlider.Detach();

	CDialogBar::OnDestroy();

	delete gCameraSlewer;
	gCameraSlewer = NULL;

}

void CCameraSlew::OnEngineerUpdStyle(CCmdUI* pCmdUI)
{
	bool bEnable = false;

	switch (pCmdUI->m_nID)
	{
	case IDC_ORIGINDOWN:
	case IDC_ZOOMIN:
	case IDC_ZOOMOUT:
	case IDC_ORIGINLEFT:
	case IDC_ORIGINRIGHT:
	case IDC_ORIGINUP:
		if (m_AttachedCamera != NULL || m_AttachedOrthoCamera != NULL)
			bEnable = true;
		break;

	case IDC_ORIGINFORWARD:
	case IDC_ORIGINREVERSE:
	case IDC_ROTATELEFT:
	case IDC_ROTATERIGHT:
	case IDC_ROTATEUP:
	case IDC_ROTATEDOWN:
		if (m_AttachedCamera != NULL && !m_bOrtho)
			bEnable = true;
		break;
	}

	pCmdUI->Enable(bEnable);
}


void CCameraSlew::InitBar()
{
	HINSTANCE hinst;
	HICON hicon;

	hinst = AfxGetInstanceHandle();
	// Associate bitmaps with the buttons
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_LEFT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ORIGINLEFT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_RIGHT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ORIGINRIGHT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_UP),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ORIGINUP))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_DOWN),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ORIGINDOWN))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ORIGINFORWARD),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ORIGINFORWARD))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ORIGINREVERSE),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ORIGINREVERSE))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ROTATELEFT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ROTATELEFT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ROTATERIGHT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ROTATERIGHT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ZOOMIN),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ZOOMIN))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ZOOMOUT),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ZOOMOUT))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ROTATEDOWN),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ROTATEDOWN))->SetIcon(hicon);
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ROTATEUP),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ROTATEUP))->SetIcon(hicon);
}


LONG CCameraSlew::OnInitDialog ( UINT wParam, LONG lParam)
{
	// TODO: Add extra initialization here
	InitBar();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
