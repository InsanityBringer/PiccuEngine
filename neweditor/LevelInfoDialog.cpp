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
 // LevelInfoDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "LevelInfoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLevelInfoDialog dialog


CLevelInfoDialog::CLevelInfoDialog(level_info *li, CWnd* pParent )
	: CDialog(CLevelInfoDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLevelInfoDialog)
	m_copyright = _T("");
	m_designer = _T("");
	m_name = _T("");
	m_notes = _T("");
	//}}AFX_DATA_INIT

	m_level_info = li;

	m_copyright = li->copyright;
	m_designer = li->designer;
	m_name = li->name;
	m_notes = li->notes;
}

void CLevelInfoDialog::GetLevelInfo(level_info *li)
{
	strcpy(li->copyright, (LPCSTR) m_copyright);
	strcpy(li->designer, (LPCSTR) m_designer);
	strcpy(li->name, (LPCSTR) m_name);
	strcpy(li->notes, (LPCSTR) m_notes);
}

void CLevelInfoDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLevelInfoDialog)
	DDX_Text(pDX, IDC_COPYRIGHT, m_copyright);
	DDV_MaxChars(pDX, m_copyright, 100);
	DDX_Text(pDX, IDC_DESIGNER, m_designer);
	DDV_MaxChars(pDX, m_designer, 100);
	DDX_Text(pDX, IDC_NAME, m_name);
	DDV_MaxChars(pDX, m_name, 100);
	DDX_Text(pDX, IDC_NOTES, m_notes);
	DDV_MaxChars(pDX, m_notes, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLevelInfoDialog, CDialog)
	//{{AFX_MSG_MAP(CLevelInfoDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLevelInfoDialog message handlers

void CLevelInfoDialog::OnOK() 
{
	// TODO: Add extra validation here
	// CDialog::OnOK() must be called first for DDX to set the strings
	CDialog::OnOK();

	//Copy the new data into our original struct
	GetLevelInfo(m_level_info);
	World_changed = 1;
}
