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
 // DallasStrmAudioDlg.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "DallasUtilities.h"
#include "manage.h"
#include "streamaudio.h"
#include "DallasStrmAudioDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDallasStrmAudioDlg dialog


CDallasStrmAudioDlg::CDallasStrmAudioDlg(CWnd* pParent )
	: CDialog(CDallasStrmAudioDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDallasStrmAudioDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_Filename="";
}


void CDallasStrmAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDallasStrmAudioDlg)
	DDX_Control(pDX, IDC_STRM_AUDIO_LIST, m_StrmAudioBox);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDallasStrmAudioDlg, CDialog)
	//{{AFX_MSG_MAP(CDallasStrmAudioDlg)
	ON_LBN_DBLCLK(IDC_STRM_AUDIO_LIST, OnDblclkStrmAudioList)
	ON_BN_CLICKED(IDC_PLAY_STRM_AUDIO_BUTTON, OnPlayStrmAudioButton)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDallasStrmAudioDlg message handlers

void CDallasStrmAudioDlg::OnOK() 
{
	// TODO: Add extra validation here
	int index=m_StrmAudioBox.GetCurSel();
	if(index==LB_ERR) return;

	m_StrmAudioBox.GetText(index,m_Filename);
	
	CDialog::OnOK();
}

void CDallasStrmAudioDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

BOOL CDallasStrmAudioDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	FillStrmAudioList();
	
	if(!m_Filename.IsEmpty())
		m_StrmAudioBox.SelectString(-1,m_Filename.GetBuffer(0));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDallasStrmAudioDlg::OnDblclkStrmAudioList() 
{
	CString filename;

	int index=m_StrmAudioBox.GetCurSel();
	if(index==LB_ERR) return;

	m_StrmAudioBox.GetText(index,filename);
	StreamPlay(filename.GetBuffer(0),1,0);
}

void CDallasStrmAudioDlg::OnPlayStrmAudioButton() 
{
	OnDblclkStrmAudioList();
}

void CDallasStrmAudioDlg::FillStrmAudioList(void)
{
	bool file_found;
	char filename[PAGENAME_LEN+1];

	file_found=FindManageFirst(filename,"*.osf");
	while(file_found) {
		m_StrmAudioBox.AddString(filename);
		file_found=FindManageNext(filename);
	}
	FindManageClose();
}
