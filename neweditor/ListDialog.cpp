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
 // ListDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "ListDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CListDialog *mverify_dlg;
extern CListDialog *rverify_dlg;
extern CListDialog *lstats_dlg;
extern CListDialog *rstats_dlg;
extern CListDialog *bn_verify_dlg;

/////////////////////////////////////////////////////////////////////////////
// CListDialog dialog


CListDialog::CListDialog(CWnd* pParent )
	: CDialog(CListDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CListDialog)
	//}}AFX_DATA_INIT
}


void CListDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CListDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CListDialog, CDialog)
	//{{AFX_MSG_MAP(CListDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListDialog message handlers


void CListDialog::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	if (this == mverify_dlg)
		mverify_dlg = NULL;
	else if (this == rverify_dlg)
		rverify_dlg = NULL;
	else if (this == rstats_dlg)
		rstats_dlg = NULL;
	else if (this == lstats_dlg)
		lstats_dlg = NULL;
	else if (this == bn_verify_dlg)
		bn_verify_dlg = NULL;

	delete this;
}

void CListDialog::OnOK() 
{
	// TODO: Add extra validation here
	DestroyWindow();
}


void CListDialog::OnCancel() 
{
	// TODO: Add extra cleanup here
	DestroyWindow();
}
