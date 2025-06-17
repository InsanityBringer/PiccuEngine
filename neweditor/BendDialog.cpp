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
 // BendDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "BendDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBendDialog dialog


CBendDialog::CBendDialog(CWnd* pParent )
	: CDialog(CBendDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CBendDialog)
	m_Direction = -1;
	m_Rotation = 0.0f;
	m_Distance = 0.0f;
	//}}AFX_DATA_INIT
}


void CBendDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBendDialog)
	DDX_Radio(pDX, IDC_XAXIS, m_Direction);
	DDX_Text(pDX, IDC_ANGLE, m_Rotation);
	DDV_MinMaxFloat(pDX, m_Rotation, 0.f, 360.f);
	DDX_Text(pDX, IDC_DISTANCE, m_Distance);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBendDialog, CDialog)
	//{{AFX_MSG_MAP(CBendDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBendDialog message handlers
