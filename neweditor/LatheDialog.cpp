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
 // LatheDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "LatheDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLatheDialog dialog


CLatheDialog::CLatheDialog(CWnd* pParent )
	: CDialog(CLatheDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLatheDialog)
	m_bEndCaps = FALSE;
	m_Num_sides = 8;
	m_Rotation = -1;
	//}}AFX_DATA_INIT
	m_Inward = -1;
}


void CLatheDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLatheDialog)
	DDX_Check(pDX, IDC_ENDCAPS, m_bEndCaps);
	DDX_Text(pDX, IDC_SIDES, m_Num_sides);
	DDV_MinMaxUInt(pDX, m_Num_sides, 3, 100);
	DDX_Radio(pDX, IDC_XAXIS, m_Rotation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLatheDialog, CDialog)
	//{{AFX_MSG_MAP(CLatheDialog)
	ON_BN_CLICKED(IDC_XAXIS, OnXAxis)
	ON_BN_CLICKED(IDC_YAXIS, OnYAxis)
	ON_BN_CLICKED(IDC_ZAXIS, OnZAxis)
	ON_BN_CLICKED(IDC_INWARD, OnInward)
	ON_BN_CLICKED(IDC_OUTWARD, OnOutward)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLatheDialog message handlers

BOOL CLatheDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_bEndCaps = FALSE;
	m_Num_sides = 8;
	m_Rotation = X_AXIS;
	m_Inward = 1;
	CheckRadioButton(IDC_INWARD,IDC_OUTWARD,IDC_INWARD);
	CheckRadioButton(IDC_XAXIS,IDC_ZAXIS,IDC_XAXIS);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLatheDialog::OnXAxis() 
{
	// TODO: Add your control notification handler code here
	m_Rotation = X_AXIS;
}

void CLatheDialog::OnYAxis() 
{
	// TODO: Add your control notification handler code here
	m_Rotation = Y_AXIS;
}

void CLatheDialog::OnZAxis() 
{
	// TODO: Add your control notification handler code here
	m_Rotation = Z_AXIS;
}


void CLatheDialog::OnInward() 
{
	// TODO: Add your control notification handler code here
	m_Inward = 1;
}

void CLatheDialog::OnOutward() 
{
	// TODO: Add your control notification handler code here
	m_Inward = 0;
}
