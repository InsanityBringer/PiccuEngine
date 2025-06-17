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
 // DallasTextureDlg.cpp : implementation file
//

#include "../neweditor/StdAfx.h"
#include "editor.h"
#include "gametexture.h"
#include "DallasTextureDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDallasTextureDlg dialog


CDallasTextureDlg::CDallasTextureDlg(CWnd* pParent )
	: CDialog(CDallasTextureDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDallasTextureDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_TextureName="";
	m_TextureIndex=-1;
}


void CDallasTextureDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDallasTextureDlg)
	DDX_Control(pDX, IDC_TEXTURE_LIST, m_TextureList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDallasTextureDlg, CDialog)
	//{{AFX_MSG_MAP(CDallasTextureDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDallasTextureDlg message handlers

BOOL CDallasTextureDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	FillTextureList();
	
	if(!m_TextureName.IsEmpty())
		m_TextureList.SelectString(-1,m_TextureName.GetBuffer(0));
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CDallasTextureDlg::OnOK() 
{
	int index=m_TextureList.GetCurSel();
	if(index==LB_ERR) return;

	m_TextureIndex=m_TextureList.GetItemData(index);
	
	CDialog::OnOK();
}

void CDallasTextureDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CDallasTextureDlg::FillTextureList(void)
{
	int i;

	// Fill the menus with sounds
	for (i=0;i<MAX_TEXTURES;i++) {
		if((GameTextures[i].used) && (strlen(GameTextures[i].name)>0)) {
			int index;
			index=m_TextureList.AddString(GameTextures[i].name);
			if(index!=LB_ERR) {
				m_TextureList.SetItemData(index,i);
			}
		}
	}
}
