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
 // LightingProgress.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "LightingProgress.h"
#include "radiosity.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightingProgress dialog


CLightingProgress::CLightingProgress(CWnd* pParent )
	: CDialog(CLightingProgress::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightingProgress)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLightingProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightingProgress)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightingProgress, CDialog)
	//{{AFX_MSG_MAP(CLightingProgress)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_STOPLIGHTING, OnStoplighting)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightingProgress message handlers

BOOL CLightingProgress::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (!SetTimer(1234,500,NULL))
	{
		mprintf((0,"Unable to create timer\n"));
		Int3();
	}
		
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightingProgress::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	char str[100];

	sprintf (str,"Iteration: %d/%d - Convergence=%.2f %%",rad_StepCount,rad_MaxStep,(1.0-rad_Convergence)*100.0);
	CEdit *ebox;
	ebox=(CEdit *) GetDlgItem (IDC_ITERATION_TEXT);
	ebox->SetWindowText (str);
		
	CDialog::OnTimer(nIDEvent);
}

BOOL CLightingProgress::DestroyWindow() 
{
	CWnd::KillTimer (1);
	
	return CDialog::DestroyWindow();
}

void CLightingProgress::OnStoplighting() 
{
	int answer=MessageBox ("Are you sure you wish to stop lighting?","Light Question",MB_YESNO);
	if (answer==IDNO)
		return;

	OutrageMessageBox ("Lighting will stop at the next iteration, this might take a minute...");
	
	rad_DoneCalculating=1;
	
}
