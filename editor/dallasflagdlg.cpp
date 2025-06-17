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
 // DallasFlagDlg.cpp : implementation file
//

#include "../neweditor/StdAfx.h"

#ifdef NEWEDITOR
#include "../neweditor/NewEditor.h"
#else
#include "editor.h"
#endif

#include "cfile.h"
#include "DallasMainDlg.h"
#include "DallasFlagDlg.h"

CDallasMainDlg *GetDallasDialogPtr(void);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDallasFlagDlg dialog


CDallasFlagDlg::CDallasFlagDlg(CWnd* pParent )
	: CDialog(CDallasFlagDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDallasFlagDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_FlagsValue=0;
	m_ValidFlagsMask=0;
	m_FlagsName="";
}


void CDallasFlagDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDallasFlagDlg)
	DDX_Control(pDX, IDC_FLAG_LIST, m_FlagListBox);
	DDX_Control(pDX, IDC_FLAG_PROMPT_STATIC, m_FlagPromptText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDallasFlagDlg, CDialog)
	//{{AFX_MSG_MAP(CDallasFlagDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDallasFlagDlg message handlers

BOOL CDallasFlagDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CDallasMainDlg *m_DallasModelessDlgPtr;
	m_DallasModelessDlgPtr = GetDallasDialogPtr();
	
	// If Dallas is up, fill in the list box
	if(m_DallasModelessDlgPtr!=NULL) {
		m_DallasModelessDlgPtr->FillFlagValuesBox(&m_FlagListBox,m_FlagsName.GetBuffer(0),m_FlagsValue,m_ValidFlagsMask);
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDallasFlagDlg::OnOK() 
{
	int list_size, j, data;

	// Fill the flags value with checklistbox data
	m_FlagsValue=0;
	list_size=m_FlagListBox.GetCount();
	for(j=0;j<list_size;j++) {
		if(m_FlagListBox.GetCheck(j)) {
			data=m_FlagListBox.GetItemData(j);
			m_FlagsValue |= data;
		}
	}	

	CDialog::OnOK();
}

void CDallasFlagDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}
