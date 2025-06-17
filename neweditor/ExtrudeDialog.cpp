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
 // ExtrudeDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "ExtrudeDialog.h"
#include "ned_Geometry.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExtrudeDialog dialog


CExtrudeDialog::CExtrudeDialog(CWnd* pParent )
	: CDialog(CExtrudeDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CExtrudeDialog)
	//}}AFX_DATA_INIT
	m_Distance = 20.0f;
	m_DeleteBaseFace = FALSE;
	m_Default = FALSE;
	m_Direction = NORMAL;
	m_Inward = 1;
	m_Faces = CURRENT;
}


void CExtrudeDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExtrudeDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExtrudeDialog, CDialog)
	//{{AFX_MSG_MAP(CExtrudeDialog)
	ON_BN_CLICKED(IDC_XAXIS, OnXAxis)
	ON_BN_CLICKED(IDC_YAXIS, OnYAxis)
	ON_BN_CLICKED(IDC_ZAXIS, OnZAxis)
	ON_BN_CLICKED(IDC_NORMAL, OnNormal)
	ON_BN_CLICKED(IDC_INWARD, OnInward)
	ON_BN_CLICKED(IDC_OUTWARD, OnOutward)
	ON_BN_CLICKED(IDC_CURRENTFACE, OnCurrentFace)
	ON_BN_CLICKED(IDC_MARKEDFACES, OnMarkedFaces)
	ON_BN_CLICKED(IDC_DELETE_FACE, OnDeleteFace)
	ON_BN_CLICKED(IDC_DEFAULTSETTINGS, OnDefaultSettings)
	ON_EN_KILLFOCUS(IDC_DISTANCE, OnKillfocusDistance)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExtrudeDialog message handlers

BOOL CExtrudeDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	theApp.GetExtrudeDefaults(&m_Direction,&m_Distance,&m_DeleteBaseFace,&m_Inward,&m_Faces,&m_Default);
	switch (m_Direction)
	{
	case NORMAL:	CheckRadioButton(IDC_NORMAL,IDC_ZAXIS,IDC_NORMAL);	break;
	case X_AXIS:	CheckRadioButton(IDC_NORMAL,IDC_ZAXIS,IDC_XAXIS);	break;
	case Y_AXIS:	CheckRadioButton(IDC_NORMAL,IDC_ZAXIS,IDC_YAXIS);	break;
	case Z_AXIS:	CheckRadioButton(IDC_NORMAL,IDC_ZAXIS,IDC_ZAXIS);	break;
	default:		CheckRadioButton(IDC_NORMAL,IDC_ZAXIS,IDC_NORMAL);	break;
	}
	switch (m_Inward)
	{
	case 0:			CheckRadioButton(IDC_INWARD,IDC_OUTWARD,IDC_OUTWARD);	break;
	case 1:			CheckRadioButton(IDC_INWARD,IDC_OUTWARD,IDC_INWARD);	break;
	default:		CheckRadioButton(IDC_INWARD,IDC_OUTWARD,IDC_INWARD);	break;
	}
	switch (m_Faces)
	{
	case CURRENT:	CheckRadioButton(IDC_CURRENTFACE,IDC_MARKEDFACES,IDC_CURRENTFACE);	break;
	case MARKED:
		CheckRadioButton(IDC_CURRENTFACE,IDC_MARKEDFACES,IDC_MARKEDFACES);
		// Don't allow deleting the base face when using marked faces (until it's no longer buggy :)
		GetDlgItem(IDC_DELETE_FACE)->EnableWindow(false);
		CheckDlgButton(IDC_DELETE_FACE,false);
		m_DeleteBaseFace = false;
		break;
	default:		CheckRadioButton(IDC_CURRENTFACE,IDC_MARKEDFACES,IDC_CURRENTFACE);	break;
	}
	CheckDlgButton(IDC_DEFAULTSETTINGS,m_Default);
	CheckDlgButton(IDC_DELETE_FACE,m_DeleteBaseFace);
	CString strDist;
	strDist.Format("%.5f",m_Distance);
	SetDlgItemText(IDC_DISTANCE,strDist);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CExtrudeDialog::OnXAxis() 
{
	// TODO: Add your control notification handler code here
	m_Direction = X_AXIS;
}

void CExtrudeDialog::OnYAxis() 
{
	// TODO: Add your control notification handler code here
	m_Direction = Y_AXIS;
}

void CExtrudeDialog::OnZAxis() 
{
	// TODO: Add your control notification handler code here
	m_Direction = Z_AXIS;
}

void CExtrudeDialog::OnNormal() 
{
	// TODO: Add your control notification handler code here
	m_Direction = NORMAL;
}

void CExtrudeDialog::OnInward() 
{
	// TODO: Add your control notification handler code here
	m_Inward = 1;
}

void CExtrudeDialog::OnOutward() 
{
	// TODO: Add your control notification handler code here
	m_Inward = 0;
}

void CExtrudeDialog::OnCurrentFace() 
{
	// TODO: Add your control notification handler code here
	m_Faces = CURRENT;
	// Don't allow deleting the base face when using marked faces (until it's no longer buggy :)
	GetDlgItem(IDC_DELETE_FACE)->EnableWindow(true);
	CheckDlgButton(IDC_DELETE_FACE,true);
	m_DeleteBaseFace = true;
}

void CExtrudeDialog::OnMarkedFaces() 
{
	// TODO: Add your control notification handler code here
	m_Faces = MARKED;
	// Don't allow deleting the base face when using marked faces (until it's no longer buggy :)
	GetDlgItem(IDC_DELETE_FACE)->EnableWindow(false);
	CheckDlgButton(IDC_DELETE_FACE,false);
	m_DeleteBaseFace = false;
}

void CExtrudeDialog::OnOK() 
{
	// TODO: Add extra validation here
	// Set app default settings for extrude
	theApp.SetExtrudeDefaults(m_Direction,m_Distance,m_DeleteBaseFace,m_Inward,m_Faces,m_Default);

	CDialog::OnOK();
}

void HandleExtrude(int which,float dist,BOOL delete_base_face,int inward,int faces)
{
	CRoomFrm *wnd = theApp.m_pFocusedRoomFrm;
	prim *prim = theApp.AcquirePrim();
	int i = 0;

	room *rp = prim->roomp;

	if (rp != NULL)
	{
		if (faces == MARKED && delete_base_face) // don't allow this (until it's no longer buggy :)
		{
			AfxMessageBox("You cannot delete base faces when extruding marked faces in this version.");
			return;
		}

		if (prim->face != -1)
		{
			int facenum = prim->face;
			int num_faces = 1;
			short marked_list[MAX_FACES_PER_ROOM];
			vector vec;

			if (faces == MARKED)
			{
				// Get list of marked faces
				num_faces = wnd->GetMarkedFaces(marked_list);
				ASSERT(num_faces == wnd->m_Num_marked_faces);
			}

			while (i++ < num_faces)
			{
				if (faces == MARKED)
					facenum = marked_list[i-1];

				switch (which)
				{
				case X_AXIS:
					vec.x = 1; vec.y = 0; vec.z = 0;
					break;

				case Y_AXIS:
					vec.x = 0; vec.y = 1; vec.z = 0;
					break;

				case Z_AXIS:
					vec.x = 0; vec.y = 0; vec.z = 1;
					break;

				case NORMAL:
					vec = rp->faces[facenum].normal;
					break;
				}

				if ( ExtrudeFace(rp,facenum,vec,dist,delete_base_face,inward) )
				{
					if (delete_base_face)
					{
						// Unmark the face if it is marked
						wnd->UnMarkFace(facenum);
						// Adjust current primitives
						int portnum;
						if (prim->face > facenum)
						{
							prim->face = facenum - 1;
							prim->face != -1 ? portnum = prim->roomp->faces[prim->face].portal_num : portnum = -1;
							wnd->SetPrim(prim->roomp,prim->face,portnum,0,0);
							// Update the current face/texture displays
							Editor_state.SetCurrentRoomFaceTexture(ROOMNUM(prim->roomp), prim->face);
						}
					}
					wnd->m_Room_changed = true;
					PrintStatus("Face extruded.");
				}
			}
		}
		else
			AfxMessageBox("You must have a current face in order to use the extrude function.");
	}
	else
		AfxMessageBox("No current room!");
}


void CExtrudeDialog::OnDeleteFace() 
{
	// TODO: Add your control notification handler code here
	m_DeleteBaseFace = !m_DeleteBaseFace;
}

void CExtrudeDialog::OnDefaultSettings() 
{
	// TODO: Add your control notification handler code here
	m_Default = !m_Default;
}

void CExtrudeDialog::OnKillfocusDistance() 
{
	// TODO: Add your control notification handler code here
	CString strDist;

	GetDlgItemText(IDC_DISTANCE,strDist);
	m_Distance = atof(strDist);
}
