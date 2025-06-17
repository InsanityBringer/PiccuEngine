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
 // RefFrameDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "RefFrameDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRefFrameDialog dialog


CRefFrameDialog::CRefFrameDialog(char *title, char *caption, vector *initial, CWnd* pParent)
	: CDialog(CRefFrameDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRefFrameDialog)
	m_Xcoord = 0.0f;
	m_Ycoord = 0.0f;
	m_Zcoord = 0.0f;
	//}}AFX_DATA_INIT

	m_Caption = caption;
	m_Xcoord = initial->x;
	m_Ycoord = initial->y;
	m_Zcoord = initial->z;
	m_Title = title;
}


void CRefFrameDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRefFrameDialog)
	DDX_Text(pDX, IDC_EDIT1, m_Xcoord);
	DDX_Text(pDX, IDC_EDIT2, m_Ycoord);
	DDX_Text(pDX, IDC_EDIT3, m_Zcoord);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRefFrameDialog, CDialog)
	//{{AFX_MSG_MAP(CRefFrameDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRefFrameDialog message handlers


BOOL CRefFrameDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	CDialog::OnInitDialog();
	
	SetWindowText(m_Title);

	((CEdit *) GetDlgItem(IDC_PROMPT))->SetWindowText(m_Caption);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//Gets a vector from the user
//Parameters:	n - filled in the with return value
//					title - the title for the input window
//					prompt - the prompt for the input box
//Returns:	false if cancel was pressed on the dialog, else true
//				If false returned, n is unchanged
bool InputVector(vector *vec,char *title,char *prompt,CWnd *wnd)
{
	char buf[100] = "";
	CRefFrameDialog dlg(title,prompt,vec,wnd);

	if (dlg.DoModal() == IDOK) {
		vec->x = dlg.m_Xcoord;
		vec->y = dlg.m_Ycoord;
		vec->z = dlg.m_Zcoord;
		return 1;
	}
	else
		return 0;
}


