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
 // MatcenDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "MatcenDialog.h"
#include "matcen.h"
#include "ned_Object.h"
#include "hlsoundlib.h"
#include "sounds.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define ENABLE_BUTTON(tab,id,state)	((CButton *)tab->GetDlgItem(id))->EnableWindow(state);
#define ENABLE_COMBO(tab,id,state)	((CComboBox *)tab->GetDlgItem(id))->EnableWindow(state);
#define ENABLE_EDIT(tab,id,state)	((CEdit *)tab->GetDlgItem(id))->EnableWindow(state);

extern char *MatcenEffectStrings[NUM_MATCEN_EFFECTS];

int m_matcen_id = 0;
CMatcenDialog *PFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMatcenDialog dialog


CMatcenDialog::CMatcenDialog(CWnd* pParent )
	: CDialog(CMatcenDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMatcenDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_CurrentMatcenTab = TAB_MATCEN_ATTACH;
	m_CurrentDlg = NULL;
	m_Adlg = NULL;
	m_Pdlg = NULL;
	m_Ddlg = NULL;
}


void CMatcenDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMatcenDialog)
	DDX_Control(pDX, IDC_MATCEN_TAB, m_TabCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMatcenDialog, CDialog)
	//{{AFX_MSG_MAP(CMatcenDialog)
	ON_BN_CLICKED(IDC_MATCEN_NEW, OnMatcenNew)
	ON_BN_CLICKED(IDC_MATCEN_DELETE, OnMatcenDelete)
	ON_BN_CLICKED(IDC_MATCEN_PREV, OnMatcenPrev)
	ON_BN_CLICKED(IDC_MATCEN_NEXT, OnMatcenNext)
	ON_BN_CLICKED(IDC_MATCEN_NAME, OnMatcenName)
	ON_WM_PAINT()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CMatcenDialog::EnableSpawnFields(int num_enabled)
{
	bool f_enable = true;

	if(num_enabled < 1) f_enable = false;
	ENABLE_EDIT(m_Adlg,IDC_MATCEN_SPAWN_0,f_enable);
	if(num_enabled < 2) f_enable = false;
	ENABLE_EDIT(m_Adlg,IDC_MATCEN_SPAWN_1,f_enable);
	if(num_enabled < 3) f_enable = false;
	ENABLE_EDIT(m_Adlg,IDC_MATCEN_SPAWN_2,f_enable);
	if(num_enabled < 4) f_enable = false;
	ENABLE_EDIT(m_Adlg,IDC_MATCEN_SPAWN_3,f_enable);
}

void CMatcenDialog::EnableProdFields(int num_enabled)
{
	bool f_enable = true;

	if(num_enabled < 1) f_enable = false;
	ENABLE_COMBO(m_Pdlg,IDC_MATCEN_PROD_0_LIST,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_0_QUANTITY,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_0_SECONDS,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_0_PRIORITY,f_enable);
	if(num_enabled < 2) f_enable = false;
	ENABLE_COMBO(m_Pdlg,IDC_MATCEN_PROD_1_LIST,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_1_QUANTITY,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_1_SECONDS,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_1_PRIORITY,f_enable);
	if(num_enabled < 3) f_enable = false;
	ENABLE_COMBO(m_Pdlg,IDC_MATCEN_PROD_2_LIST,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_2_QUANTITY,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_2_SECONDS,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_2_PRIORITY,f_enable);
	if(num_enabled < 4) f_enable = false;
	ENABLE_COMBO(m_Pdlg,IDC_MATCEN_PROD_3_LIST,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_3_QUANTITY,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_3_SECONDS,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_3_PRIORITY,f_enable);
	if(num_enabled < 5) f_enable = false;
	ENABLE_COMBO(m_Pdlg,IDC_MATCEN_PROD_4_LIST,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_4_QUANTITY,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_4_SECONDS,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_4_PRIORITY,f_enable);
	if(num_enabled < 6) f_enable = false;
	ENABLE_COMBO(m_Pdlg,IDC_MATCEN_PROD_5_LIST,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_5_QUANTITY,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_5_SECONDS,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_5_PRIORITY,f_enable);
	if(num_enabled < 7) f_enable = false;
	ENABLE_COMBO(m_Pdlg,IDC_MATCEN_PROD_6_LIST,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_6_QUANTITY,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_6_SECONDS,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_6_PRIORITY,f_enable);
	if(num_enabled < 8) f_enable = false;
	ENABLE_COMBO(m_Pdlg,IDC_MATCEN_PROD_7_LIST,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_7_QUANTITY,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_7_SECONDS,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PROD_7_PRIORITY,f_enable);
}

void CMatcenDialog::EnableFields(bool f_enable)
{
	if(f_enable)
	{
		EnableSpawnFields(Matcen[m_matcen_id]->GetNumSpawnPnts());
		EnableProdFields(Matcen[m_matcen_id]->GetNumProdTypes());
		
		PrintToDlgItem(this, IDC_MATCEN_NUMBER, "%d/%d", m_matcen_id + 1, Num_matcens);
	}
	else
	{
		EnableSpawnFields(0);
		EnableProdFields(0);

		SetDlgItemText(IDC_MATCEN_NUMBER, "0/0");
	}

	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PRODUCE_PRETIME,f_enable);
	ENABLE_COMBO(m_Ddlg,IDC_DISPLAY_PROD_EFFECT,f_enable);
	ENABLE_COMBO(m_Ddlg,IDC_DISPLAY_PROD_SOUND,f_enable);
	ENABLE_COMBO(m_Ddlg,IDC_DISPLAY_PROD_TEXTURE,f_enable);
	ENABLE_COMBO(m_Ddlg,IDC_DISPLAY_ACTIVE_SOUND,f_enable);
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_SC_RADIO,f_enable);
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_WPV_RADIO,f_enable);
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_WPN_RADIO,f_enable);
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_APN_RADIO,f_enable);
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_PRODUCE_RANDOM,f_enable);
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_PRODUCE_ORDERED,f_enable);
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_PRODUCE_ENABLED,f_enable);
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_PRODUCE_HURT,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PRODUCE_MULT,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PRODUCE_MAXALIVE,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_OBJTYPE_NUM,f_enable);
	ENABLE_BUTTON(m_Adlg,IDC_MATCEN_ROOM_ATTACH,f_enable);
	ENABLE_BUTTON(m_Adlg,IDC_MATCEN_OBJECT_ATTACH,f_enable);
	ENABLE_BUTTON(m_Adlg,IDC_MATCEN_UNASSIGNED_ATTACH,f_enable);
	ENABLE_EDIT(m_Adlg,IDC_MATCEN_SPAWN_NUM,f_enable);
	ENABLE_BUTTON(this,IDC_MATCEN_NAME,f_enable);
/*
	((CButton *)GetDlgItem(IDC_MAT_MOVE_VIEWER_BUTTON))->EnableWindow(f_enable);
	((CButton *)GetDlgItem(IDC_MAT_PASTE_OBJREF_BUTTON))->EnableWindow(f_enable);
*/
	ENABLE_BUTTON(this,IDC_MATCEN_PREV,f_enable);
	ENABLE_BUTTON(this,IDC_MATCEN_NEXT,f_enable);

	ENABLE_BUTTON(this,IDC_MATCEN_NEW,Num_matcens < MAX_MATCENS);

	ENABLE_BUTTON(this,IDC_MATCEN_DELETE,f_enable);
/*
	((CButton *)GetDlgItem(IDC_MAT_COPY_BUTTON))->EnableWindow(f_enable);
	((CButton *)GetDlgItem(IDC_MAT_PASTE_BUTTON))->EnableWindow(f_enable);
*/
	ENABLE_BUTTON(m_Pdlg,IDC_MATCEN_APV_RADIO,f_enable);
//	((CButton *)GetDlgItem(IDC_MAT_ATTACH_EDIT))->EnableWindow(f_enable);
	ENABLE_COMBO(m_Adlg,IDC_MATCEN_ROOM_OBJECT_COMBO,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PRODUCE_POSTTIME,f_enable);
	ENABLE_EDIT(m_Pdlg,IDC_MATCEN_PRODUCE_MAX,f_enable);
}

//which class is attached to the room/object combo box?
byte attached = -1;

void CMatcenDialog::UpdateDialog()
{
	bool f_valid = true;
	int i;
	char stemp[MAX_MATCEN_NAME_LEN];
	float ftemp;
	int itemp;

	// Check if the current matcen is valid
	if(!MatcenValid(m_matcen_id))
	{
		f_valid = false;

		// Scan for a valid matcen id
		for(i = 0; i < Num_matcens; i++)
		{
			if(Matcen[i] != NULL)
			{
				m_matcen_id = i;
				f_valid = true;
				break;
			}
		}
	}

	// If valid enable fields and continue -- otherwise disable fields and exit
	if(f_valid)
	{
		EnableFields(true);
	}
	else
	{
		EnableFields(false);
		return;
	}

	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PRODUCE_PRETIME, "%f", Matcen[m_matcen_id]->GetPreProdTime());
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PRODUCE_POSTTIME, "%f", Matcen[m_matcen_id]->GetPostProdTime());

	m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_EFFECT, CB_RESETCONTENT,0,0);
	for (i = 0;i < NUM_MATCEN_EFFECTS; i++)
	{
		m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_EFFECT, CB_ADDSTRING,0,(LPARAM) (LPCTSTR) MatcenEffectStrings[i]);
	}
	m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_EFFECT, CB_SELECTSTRING,0,(LPARAM) (LPCTSTR) MatcenEffectStrings[Matcen[m_matcen_id]->GetCreationEffect()]);

	m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_SOUND, CB_RESETCONTENT,0,0);
	for (i = 0;i < MAX_SOUNDS; i++)
	{
		if (Sounds[i].used)
			m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_SOUND, CB_ADDSTRING,0,(LPARAM) (LPCTSTR) Sounds[i].name);
	}
	m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_SOUND, CB_SELECTSTRING,0,(LPARAM) (LPCTSTR) Sounds[Matcen[m_matcen_id]->GetSound(MATCEN_PROD_SOUND)].name);

	m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_TEXTURE, CB_RESETCONTENT,0,0);
	for (i = 0;i < MAX_TEXTURES; i++)
	{
		if (GameTextures[i].used)
			m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_TEXTURE, CB_ADDSTRING,0,(LPARAM) (LPCTSTR) GameTextures[i].name);
	}
	m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_PROD_TEXTURE, CB_SELECTSTRING,0,(LPARAM) (LPCTSTR) GameTextures[Matcen[m_matcen_id]->GetCreationTexture()].name);

	m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_ACTIVE_SOUND, CB_RESETCONTENT,0,0);
	for (i = 0;i < MAX_SOUNDS; i++)
	{
		if (Sounds[i].used)
			m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_ACTIVE_SOUND, CB_ADDSTRING,0,(LPARAM) (LPCTSTR) Sounds[i].name);
	}
	m_Ddlg->SendDlgItemMessage( IDC_DISPLAY_ACTIVE_SOUND, CB_SELECTSTRING,0,(LPARAM) (LPCTSTR) Sounds[Matcen[m_matcen_id]->GetSound(MATCEN_ACTIVE_SOUND)].name);

	m_Pdlg->CheckDlgButton(IDC_MATCEN_SC_RADIO,(Matcen[m_matcen_id]->GetControlType() == MPC_SCRIPT));
	m_Pdlg->CheckDlgButton(IDC_MATCEN_WPV_RADIO,(Matcen[m_matcen_id]->GetControlType() == MPC_WHILE_PLAYER_VISIBLE));
	m_Pdlg->CheckDlgButton(IDC_MATCEN_WPN_RADIO,(Matcen[m_matcen_id]->GetControlType() == MPC_WHILE_PLAYER_NEAR));
	m_Pdlg->CheckDlgButton(IDC_MATCEN_APN_RADIO,(Matcen[m_matcen_id]->GetControlType() == MPC_AFTER_PLAYER_NEAR));
	m_Pdlg->CheckDlgButton(IDC_MATCEN_APV_RADIO,(Matcen[m_matcen_id]->GetControlType() == MPC_AFTER_PLAYER_VISIBLE));

	m_Pdlg->CheckDlgButton(IDC_MATCEN_PRODUCE_RANDOM,(Matcen[m_matcen_id]->GetStatus() & MSTAT_RANDOM_PROD_ORDER));
	m_Pdlg->CheckDlgButton(IDC_MATCEN_PRODUCE_ORDERED,!(Matcen[m_matcen_id]->GetStatus() & MSTAT_RANDOM_PROD_ORDER));
	m_Pdlg->CheckDlgButton(IDC_MATCEN_PRODUCE_ENABLED,!(Matcen[m_matcen_id]->GetStatus() & MSTAT_DISABLED));
	m_Pdlg->CheckDlgButton(IDC_MATCEN_PRODUCE_HURT,(Matcen[m_matcen_id]->GetStatus() & MSTAT_NOT_HURT_PLAYER));

	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PRODUCE_MULT, "%f", Matcen[m_matcen_id]->GetProdMultiplier());
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PRODUCE_MAXALIVE, "%d", Matcen[m_matcen_id]->GetMaxAliveChildren());

	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_0_LIST))->ResetContent();
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_1_LIST))->ResetContent();
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_2_LIST))->ResetContent();
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_3_LIST))->ResetContent();
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_4_LIST))->ResetContent();
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_5_LIST))->ResetContent();
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_6_LIST))->ResetContent();
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_7_LIST))->ResetContent();

	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_0_LIST))->AddString("None");
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_1_LIST))->AddString("None");
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_2_LIST))->AddString("None");
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_3_LIST))->AddString("None");
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_4_LIST))->AddString("None");
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_5_LIST))->AddString("None");
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_6_LIST))->AddString("None");
	((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_7_LIST))->AddString("None");

	for (i=0;i<MAX_OBJECT_IDS;i++)
	{
		if (Object_info[i].type != OBJ_NONE)
		{
			((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_0_LIST))->AddString(Object_info[i].name);
			((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_1_LIST))->AddString(Object_info[i].name);
			((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_2_LIST))->AddString(Object_info[i].name);
			((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_3_LIST))->AddString(Object_info[i].name);
			((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_4_LIST))->AddString(Object_info[i].name);
			((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_5_LIST))->AddString(Object_info[i].name);
			((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_6_LIST))->AddString(Object_info[i].name);
			((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_7_LIST))->AddString(Object_info[i].name);
		}
	}
	
	Matcen[m_matcen_id]->GetProdInfo(0, &i, NULL, NULL, NULL);	
	if(i >= 0 && i < MAX_OBJECT_IDS && Object_info[i].type != OBJ_NONE)
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_0_LIST))->SelectString(0,Object_info[i].name);
	else
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_0_LIST))->SelectString(0,"None");

	Matcen[m_matcen_id]->GetProdInfo(1, &i, NULL, NULL, NULL);	
	if(i >= 0 && i < MAX_OBJECT_IDS && Object_info[i].type != OBJ_NONE)
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_1_LIST))->SelectString(0,Object_info[i].name);
	else
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_1_LIST))->SelectString(0,"None");

	Matcen[m_matcen_id]->GetProdInfo(2, &i, NULL, NULL, NULL);	
	if(i >= 0 && i < MAX_OBJECT_IDS && Object_info[i].type != OBJ_NONE)
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_2_LIST))->SelectString(0,Object_info[i].name);
	else
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_2_LIST))->SelectString(0,"None");

	Matcen[m_matcen_id]->GetProdInfo(3, &i, NULL, NULL, NULL);	
	if(i >= 0 && i < MAX_OBJECT_IDS && Object_info[i].type != OBJ_NONE)
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_3_LIST))->SelectString(0,Object_info[i].name);
	else
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_3_LIST))->SelectString(0,"None");

	Matcen[m_matcen_id]->GetProdInfo(4, &i, NULL, NULL, NULL);	
	if(i >= 0 && i < MAX_OBJECT_IDS && Object_info[i].type != OBJ_NONE)
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_4_LIST))->SelectString(0,Object_info[i].name);
	else
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_4_LIST))->SelectString(0,"None");

	Matcen[m_matcen_id]->GetProdInfo(5, &i, NULL, NULL, NULL);	
	if(i >= 0 && i < MAX_OBJECT_IDS && Object_info[i].type != OBJ_NONE)
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_5_LIST))->SelectString(0,Object_info[i].name);
	else
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_5_LIST))->SelectString(0,"None");

	Matcen[m_matcen_id]->GetProdInfo(6, &i, NULL, NULL, NULL);	
	if(i >= 0 && i < MAX_OBJECT_IDS && Object_info[i].type != OBJ_NONE)
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_6_LIST))->SelectString(0,Object_info[i].name);
	else
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_7_LIST))->SelectString(0,"None");

	Matcen[m_matcen_id]->GetProdInfo(7, &i, NULL, NULL, NULL);	
	if(i >= 0 && i < MAX_OBJECT_IDS && Object_info[i].type != OBJ_NONE)
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_7_LIST))->SelectString(0,Object_info[i].name);
	else
		((CComboBox *)m_Pdlg->GetDlgItem(IDC_MATCEN_PROD_7_LIST))->SelectString(0,"None");

	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PRODUCE_MAX, "%d", Matcen[m_matcen_id]->GetMaxProd());
	
	Matcen[m_matcen_id]->GetProdInfo(0,NULL,NULL,NULL,&itemp);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_0_QUANTITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(1,NULL,NULL,NULL,&itemp);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_1_QUANTITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(2,NULL,NULL,NULL,&itemp);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_2_QUANTITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(3,NULL,NULL,NULL,&itemp);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_3_QUANTITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(4,NULL,NULL,NULL,&itemp);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_4_QUANTITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(5,NULL,NULL,NULL,&itemp);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_5_QUANTITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(6,NULL,NULL,NULL,&itemp);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_6_QUANTITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(7,NULL,NULL,NULL,&itemp);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_7_QUANTITY, "%d", itemp);
	
	Matcen[m_matcen_id]->GetProdInfo(0,NULL,NULL,&ftemp,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_0_SECONDS, "%f", ftemp);
	Matcen[m_matcen_id]->GetProdInfo(1,NULL,NULL,&ftemp,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_1_SECONDS, "%f", ftemp);
	Matcen[m_matcen_id]->GetProdInfo(2,NULL,NULL,&ftemp,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_2_SECONDS, "%f", ftemp);
	Matcen[m_matcen_id]->GetProdInfo(3,NULL,NULL,&ftemp,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_3_SECONDS, "%f", ftemp);
	Matcen[m_matcen_id]->GetProdInfo(4,NULL,NULL,&ftemp,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_4_SECONDS, "%f", ftemp);
	Matcen[m_matcen_id]->GetProdInfo(5,NULL,NULL,&ftemp,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_5_SECONDS, "%f", ftemp);
	Matcen[m_matcen_id]->GetProdInfo(6,NULL,NULL,&ftemp,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_6_SECONDS, "%f", ftemp);
	Matcen[m_matcen_id]->GetProdInfo(7,NULL,NULL,&ftemp,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_7_SECONDS, "%f", ftemp);
	
	Matcen[m_matcen_id]->GetProdInfo(0,NULL,&itemp,NULL,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_0_PRIORITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(1,NULL,&itemp,NULL,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_1_PRIORITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(2,NULL,&itemp,NULL,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_2_PRIORITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(3,NULL,&itemp,NULL,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_3_PRIORITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(4,NULL,&itemp,NULL,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_4_PRIORITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(5,NULL,&itemp,NULL,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_5_PRIORITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(6,NULL,&itemp,NULL,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_6_PRIORITY, "%d", itemp);
	Matcen[m_matcen_id]->GetProdInfo(7,NULL,&itemp,NULL,NULL);
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_PROD_7_PRIORITY, "%d", itemp);
	
	PrintToDlgItem(m_Pdlg, IDC_MATCEN_OBJTYPE_NUM, "%d", Matcen[m_matcen_id]->GetNumProdTypes());
	PrintToDlgItem(m_Adlg, IDC_MATCEN_SPAWN_0, "%d", Matcen[m_matcen_id]->GetSpawnPnt(0));
	PrintToDlgItem(m_Adlg, IDC_MATCEN_SPAWN_1, "%d", Matcen[m_matcen_id]->GetSpawnPnt(1));
	PrintToDlgItem(m_Adlg, IDC_MATCEN_SPAWN_2, "%d", Matcen[m_matcen_id]->GetSpawnPnt(2));
	PrintToDlgItem(m_Adlg, IDC_MATCEN_SPAWN_3, "%d", Matcen[m_matcen_id]->GetSpawnPnt(3));
	
//	PrintToDlgItem(this, IDC_MAT_ATTACH_EDIT, "%d", Matcen[m_matcen_id]->GetAttach());

	switch (Matcen[m_matcen_id]->GetAttachType()) {
		case MT_ROOM:
			m_Adlg->CheckDlgButton(IDC_MATCEN_ROOM_ATTACH,true);
			m_Adlg->CheckDlgButton(IDC_MATCEN_OBJECT_ATTACH,false);
			m_Adlg->CheckDlgButton(IDC_MATCEN_UNASSIGNED_ATTACH,false);
			if (attached != MT_ROOM) {
				if (attached == MT_OBJECT)
					m_Adlg->m_object_combo.Detach();
				m_Adlg->m_room_combo.Attach(((CWnd *)m_Adlg->GetDlgItem(IDC_MATCEN_ROOM_OBJECT_COMBO))->m_hWnd);
				m_Adlg->m_room_combo.Init(Matcen[m_matcen_id]->GetAttach());
				attached = MT_ROOM;
			}
			break;
		case MT_OBJECT:
			m_Adlg->CheckDlgButton(IDC_MATCEN_ROOM_ATTACH,false);
			m_Adlg->CheckDlgButton(IDC_MATCEN_OBJECT_ATTACH,true);
			m_Adlg->CheckDlgButton(IDC_MATCEN_UNASSIGNED_ATTACH,false);
			if (attached != MT_OBJECT) {
				if (attached == MT_ROOM)
					m_Adlg->m_room_combo.Detach();
				m_Adlg->m_object_combo.Attach(((CWnd *)m_Adlg->GetDlgItem(IDC_MATCEN_ROOM_OBJECT_COMBO))->m_hWnd);
				m_Adlg->m_object_combo.Init(OBJ_NONE,Matcen[m_matcen_id]->GetAttach());
				attached = MT_OBJECT;
			}
			break;
		case MT_UNASSIGNED:
			m_Adlg->CheckDlgButton(IDC_MATCEN_ROOM_ATTACH,false);
			m_Adlg->CheckDlgButton(IDC_MATCEN_OBJECT_ATTACH,false);
			m_Adlg->CheckDlgButton(IDC_MATCEN_UNASSIGNED_ATTACH,true);
			ENABLE_COMBO(m_Adlg,IDC_MATCEN_ROOM_OBJECT_COMBO,false);
			break;
	}

	PrintToDlgItem(m_Adlg, IDC_MATCEN_SPAWN_NUM, "%d", Matcen[m_matcen_id]->GetNumSpawnPnts());
	
	Matcen[m_matcen_id]->GetName(stemp);
	PrintToDlgItem(this, IDC_MATCEN_NAME, "%s", stemp);
}

/////////////////////////////////////////////////////////////////////////////
// CMatcenDialog message handlers

void CMatcenDialog::OnMatcenNew() 
{
	// TODO: Add your control notification handler code here
	char name[MAX_MATCEN_NAME_LEN];
	bool f_name_changed;
	int id;

	strcpy(name, "temp matcen name");
	id = CreateMatcen(name, &f_name_changed);

	if(id == MATCEN_ERROR)
	{
		OutrageMessageBox(MB_OK, "Sorry, no free matcen slots.");
	}
	else
	{
		sprintf(name, "Matcen %d", id + 1);

		Matcen[id]->SetName(name);
		m_matcen_id = id;
	}
	
	UpdateDialog();
}

void CMatcenDialog::OnMatcenDelete() 
{
	// TODO: Add your control notification handler code here
	if(MatcenValid(m_matcen_id))
	{
		DestroyMatcen(m_matcen_id, true);
	}

	UpdateDialog();
}

void CMatcenDialog::OnMatcenPrev() 
{
	// TODO: Add your control notification handler code here
	int i;
	
	if(!MatcenValid(m_matcen_id))
		return;

	for(i = m_matcen_id - 1; i >= 0; i--)
	{
		if(Matcen[i])
		{
			m_matcen_id = i;
			UpdateDialog();
			return;
		}
	}

	for(i = Num_matcens - 1; i >= m_matcen_id; i--)
	{
		if(Matcen[i])
		{
			m_matcen_id = i;
			UpdateDialog();
			return;
		}
	}

	UpdateDialog();
}

void CMatcenDialog::OnMatcenNext() 
{
	// TODO: Add your control notification handler code here
	int i;
	
	if(!MatcenValid(m_matcen_id))
		return;

	for(i = m_matcen_id + 1; i < Num_matcens; i++)
	{
		if(Matcen[i])
		{
			m_matcen_id = i;
			UpdateDialog();
			return;
		}
	}

	for(i = 0; i <= m_matcen_id; i++)
	{
		if(Matcen[i])
		{
			m_matcen_id = i;
			UpdateDialog();
			return;
		}
	}

	UpdateDialog();
}

void CMatcenDialog::OnMatcenName() 
{
	// TODO: Add your control notification handler code here
	char str[MAX_MATCEN_NAME_LEN];
	
	((CEdit *) GetDlgItem(IDC_MATCEN_NAME))->GetWindowText(str,20);
	
	Matcen[m_matcen_id]->SetName(str);

	UpdateDialog();
}

BOOL CMatcenDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	PFrame = this;
	InitTabs();
	ShowCurrentMatcenTab();

	UpdateDialog();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMatcenDialog::InitTabs()
{
	TC_ITEM tc_item;

	//	Add tabs to tab control
	tc_item.mask = TCIF_TEXT;
	tc_item.pszText = "Attach";
	tc_item.cchTextMax = lstrlen(tc_item.pszText)+1;
	m_TabCtrl.InsertItem(TAB_MATCEN_ATTACH, &tc_item);

	tc_item.mask = TCIF_TEXT;
	tc_item.pszText = "Produce";
	tc_item.cchTextMax = lstrlen(tc_item.pszText)+1;
	m_TabCtrl.InsertItem(TAB_MATCEN_PRODUCE, &tc_item);

	tc_item.mask = TCIF_TEXT;
	tc_item.pszText = "Display";
	tc_item.cchTextMax = lstrlen(tc_item.pszText)+1;
	m_TabCtrl.InsertItem(TAB_MATCEN_DISPLAY, &tc_item);

	//	Get coordinates of tab control
	RECT tab_rect, tab_winrect;
	int	tx, ty, tw, th;

	m_TabCtrl.GetWindowRect(&tab_winrect);
	::CopyRect(&tab_rect, &tab_winrect);
	m_TabCtrl.AdjustRect(FALSE, &tab_rect);
	tx = tab_rect.left - tab_winrect.left;
	ty = tab_rect.top - tab_winrect.top;
	tw = tab_rect.right - tab_rect.left;
	th = tab_rect.bottom - tab_rect.top;

	// Create the tab dialogs
	// and resize them to fit the tab control
	m_Adlg = new CMcAttach(IDD_MATCEN_ATTACH,&m_TabCtrl);
	m_Adlg->Create(IDD_MATCEN_ATTACH,&m_TabCtrl);
	m_Adlg->SetWindowPos(&m_TabCtrl, tx, ty, tw, th, SWP_NOZORDER);

	m_Pdlg = new CMcProduce(IDD_MATCEN_PRODUCE,&m_TabCtrl);
	m_Pdlg->Create(IDD_MATCEN_PRODUCE,&m_TabCtrl);
	m_Pdlg->SetWindowPos(&m_TabCtrl, tx, ty, tw, th, SWP_NOZORDER);

	m_Ddlg = new CMcDisplay(IDD_MATCEN_DISPLAY,&m_TabCtrl);
	m_Ddlg->Create(IDD_MATCEN_DISPLAY,&m_TabCtrl);
	m_Ddlg->SetWindowPos(&m_TabCtrl, tx, ty, tw, th, SWP_NOZORDER);
}

void CMatcenDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	UpdateDialog();

	// Do not call CDialog::OnPaint() for painting messages
}

void CMatcenDialog::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}

void CMatcenDialog::OnCancel() 
{
	// TODO: Add extra cleanup here
	DestroyWindow();
}

void CMatcenDialog::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	theApp.m_pMatcensDlg = NULL;
	delete this;
}

void CMatcenDialog::OnDestroy() 
{
	if (attached == MT_OBJECT)
		m_Adlg->m_object_combo.Detach();
	else if (attached == MT_ROOM)
		m_Adlg->m_room_combo.Detach();

	if (m_Adlg)
	{
		delete m_Adlg;
		m_Adlg = NULL;
	}

	if (m_Pdlg)
	{
		delete m_Pdlg;
		m_Pdlg = NULL;
	}

	if (m_Ddlg)
	{
		delete m_Ddlg;
		m_Ddlg = NULL;
	}

	CDialog::OnDestroy();
}

BOOL CMatcenDialog::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) 
{
	// TODO: Add your specialized code here and/or call the base class
	NMHDR *nmhdr;

	assert(lParam != 0);
	
	nmhdr = (NMHDR *)lParam;

	switch (wParam)
	{
		case IDC_MATCEN_TAB:
			DoMatcenTabNotify(nmhdr);
			break;
	}

	return CDialog::OnNotify(wParam, lParam, pResult);
}

void CMatcenDialog::DoMatcenTabNotify(NMHDR *nmhdr)
{
	switch (nmhdr->code)
	{
	case TCN_SELCHANGE:				
		// Tab control changed. So update the controls currently displayed.
		m_CurrentMatcenTab = (int)m_TabCtrl.GetCurSel(); 
		assert(m_CurrentMatcenTab > -1);
		ShowCurrentMatcenTab();
		break;
	}
}

//	Display current tab dialog	
void CMatcenDialog::ShowCurrentMatcenTab()
{
	if (m_CurrentDlg) m_CurrentDlg->ShowWindow(SW_HIDE);

	switch (m_CurrentMatcenTab) 
	{
		case TAB_MATCEN_ATTACH:		m_CurrentDlg = m_Adlg;	break;
		case TAB_MATCEN_PRODUCE:	m_CurrentDlg = m_Pdlg;	break;
		case TAB_MATCEN_DISPLAY:	m_CurrentDlg = m_Ddlg;	break;
		default:					Int3();
	}

  	m_CurrentDlg->ShowWindow(SW_SHOW);
	m_TabCtrl.SetCurSel(m_CurrentMatcenTab);
}

/////////////////////////////////////////////////////////////////////////////
// CMcAttach dialog


CMcAttach::CMcAttach(CWnd* pParent /*=NULL*/)
	: CDialog(CMcAttach::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMcAttach)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CMcAttach::CMcAttach(UINT nIDTemplate,CWnd* pParent /*=NULL*/)
	: CDialog(CMcAttach::IDD, pParent)
{
	CDialog::CDialog(nIDTemplate,pParent);
}

void CMcAttach::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMcAttach)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMcAttach, CDialog)
	//{{AFX_MSG_MAP(CMcAttach)
	ON_BN_CLICKED(IDC_MATCEN_ROOM_ATTACH, OnMatcenRoomAttach)
	ON_BN_CLICKED(IDC_MATCEN_OBJECT_ATTACH, OnMatcenObjectAttach)
	ON_BN_CLICKED(IDC_MATCEN_UNASSIGNED_ATTACH, OnMatcenUnassignedAttach)
	ON_EN_KILLFOCUS(IDC_MATCEN_SPAWN_0, OnKillfocusMatcenSpawn0)
	ON_EN_KILLFOCUS(IDC_MATCEN_SPAWN_1, OnKillfocusMatcenSpawn1)
	ON_EN_KILLFOCUS(IDC_MATCEN_SPAWN_2, OnKillfocusMatcenSpawn2)
	ON_EN_KILLFOCUS(IDC_MATCEN_SPAWN_3, OnKillfocusMatcenSpawn3)
	ON_EN_KILLFOCUS(IDC_MATCEN_SPAWN_NUM, OnKillfocusMatcenSpawnNum)
	ON_CBN_SELENDOK(IDC_MATCEN_ROOM_OBJECT_COMBO, OnSelendokMatcenRoomObjectCombo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMcAttach message handlers
/////////////////////////////////////////////////////////////////////////////
// CMcProduce dialog


CMcProduce::CMcProduce(CWnd* pParent /*=NULL*/)
	: CDialog(CMcProduce::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMcProduce)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CMcProduce::CMcProduce(UINT nIDTemplate,CWnd* pParent /*=NULL*/)
	: CDialog(CMcProduce::IDD, pParent)
{
	CDialog::CDialog(nIDTemplate,pParent);
}

void CMcProduce::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMcProduce)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMcProduce, CDialog)
	//{{AFX_MSG_MAP(CMcProduce)
	ON_BN_CLICKED(IDC_MATCEN_APN_RADIO, OnMatcenApnRadio)
	ON_BN_CLICKED(IDC_MATCEN_APV_RADIO, OnMatcenApvRadio)
	ON_BN_CLICKED(IDC_MATCEN_SC_RADIO, OnMatcenScRadio)
	ON_BN_CLICKED(IDC_MATCEN_WPN_RADIO, OnMatcenWpnRadio)
	ON_BN_CLICKED(IDC_MATCEN_WPV_RADIO, OnMatcenWpvRadio)
	ON_EN_KILLFOCUS(IDC_MATCEN_OBJTYPE_NUM, OnKillfocusMatcenObjtypeNum)
	ON_CBN_SELENDOK(IDC_MATCEN_PROD_0_LIST, OnSelendokMatcenProd0List)
	ON_CBN_SELENDOK(IDC_MATCEN_PROD_1_LIST, OnSelendokMatcenProd1List)
	ON_CBN_SELENDOK(IDC_MATCEN_PROD_2_LIST, OnSelendokMatcenProd2List)
	ON_CBN_SELENDOK(IDC_MATCEN_PROD_3_LIST, OnSelendokMatcenProd3List)
	ON_CBN_SELENDOK(IDC_MATCEN_PROD_4_LIST, OnSelendokMatcenProd4List)
	ON_CBN_SELENDOK(IDC_MATCEN_PROD_5_LIST, OnSelendokMatcenProd5List)
	ON_CBN_SELENDOK(IDC_MATCEN_PROD_6_LIST, OnSelendokMatcenProd6List)
	ON_CBN_SELENDOK(IDC_MATCEN_PROD_7_LIST, OnSelendokMatcenProd7List)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_0_QUANTITY, OnKillfocusMatcenProd0Quantity)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_1_QUANTITY, OnKillfocusMatcenProd1Quantity)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_2_QUANTITY, OnKillfocusMatcenProd2Quantity)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_3_QUANTITY, OnKillfocusMatcenProd3Quantity)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_4_QUANTITY, OnKillfocusMatcenProd4Quantity)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_5_QUANTITY, OnKillfocusMatcenProd5Quantity)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_6_QUANTITY, OnKillfocusMatcenProd6Quantity)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_7_QUANTITY, OnKillfocusMatcenProd7Quantity)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_0_SECONDS, OnKillfocusMatcenProd0Seconds)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_1_SECONDS, OnKillfocusMatcenProd1Seconds)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_2_SECONDS, OnKillfocusMatcenProd2Seconds)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_3_SECONDS, OnKillfocusMatcenProd3Seconds)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_4_SECONDS, OnKillfocusMatcenProd4Seconds)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_5_SECONDS, OnKillfocusMatcenProd5Seconds)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_6_SECONDS, OnKillfocusMatcenProd6Seconds)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_7_SECONDS, OnKillfocusMatcenProd7Seconds)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_0_PRIORITY, OnKillfocusMatcenProd0Priority)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_1_PRIORITY, OnKillfocusMatcenProd1Priority)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_2_PRIORITY, OnKillfocusMatcenProd2Priority)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_3_PRIORITY, OnKillfocusMatcenProd3Priority)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_4_PRIORITY, OnKillfocusMatcenProd4Priority)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_5_PRIORITY, OnKillfocusMatcenProd5Priority)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_6_PRIORITY, OnKillfocusMatcenProd6Priority)
	ON_EN_KILLFOCUS(IDC_MATCEN_PROD_7_PRIORITY, OnKillfocusMatcenProd7Priority)
	ON_BN_CLICKED(IDC_MATCEN_PRODUCE_ENABLED, OnMatcenProduceEnabled)
	ON_BN_CLICKED(IDC_MATCEN_PRODUCE_HURT, OnMatcenProduceHurt)
	ON_BN_CLICKED(IDC_MATCEN_PRODUCE_RANDOM, OnMatcenProduceRandom)
	ON_BN_CLICKED(IDC_MATCEN_PRODUCE_ORDERED, OnMatcenProduceOrdered)
	ON_EN_KILLFOCUS(IDC_MATCEN_PRODUCE_MAX, OnKillfocusMatcenProduceMax)
	ON_EN_KILLFOCUS(IDC_MATCEN_PRODUCE_MULT, OnKillfocusMatcenProduceMult)
	ON_EN_KILLFOCUS(IDC_MATCEN_PRODUCE_MAXALIVE, OnKillfocusMatcenProduceMaxalive)
	ON_EN_KILLFOCUS(IDC_MATCEN_PRODUCE_PRETIME, OnKillfocusMatcenProducePretime)
	ON_EN_KILLFOCUS(IDC_MATCEN_PRODUCE_POSTTIME, OnKillfocusMatcenProducePosttime)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMcProduce message handlers
/////////////////////////////////////////////////////////////////////////////
// CMcDisplay dialog


CMcDisplay::CMcDisplay(CWnd* pParent /*=NULL*/)
	: CDialog(CMcDisplay::IDD, pParent)
{
	//{{AFX_DATA_INIT(CMcDisplay)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CMcDisplay::CMcDisplay(UINT nIDTemplate,CWnd* pParent /*=NULL*/)
	: CDialog(CMcDisplay::IDD, pParent)
{
	CDialog::CDialog(nIDTemplate,pParent);
}

void CMcDisplay::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMcDisplay)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMcDisplay, CDialog)
	//{{AFX_MSG_MAP(CMcDisplay)
	ON_CBN_SELENDOK(IDC_DISPLAY_ACTIVE_SOUND, OnSelendokDisplayActiveSound)
	ON_CBN_SELENDOK(IDC_DISPLAY_PROD_EFFECT, OnSelendokDisplayProdEffect)
	ON_CBN_SELENDOK(IDC_DISPLAY_PROD_SOUND, OnSelendokDisplayProdSound)
	ON_CBN_SELENDOK(IDC_DISPLAY_PROD_TEXTURE, OnSelendokDisplayProdTexture)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMcDisplay message handlers

void CMcAttach::OnMatcenRoomAttach() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetAttachType(MT_ROOM);
	PFrame->UpdateDialog();
}

void CMcAttach::OnMatcenObjectAttach() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetAttachType(MT_OBJECT);
	PFrame->UpdateDialog();
}

void CMcAttach::OnMatcenUnassignedAttach() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetAttachType(MT_UNASSIGNED);
	PFrame->UpdateDialog();
}

void CMcAttach::OnKillfocusMatcenSpawn0() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_SPAWN_0,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetSpawnPnt(0, val);

	PFrame->UpdateDialog();
}

void CMcAttach::OnKillfocusMatcenSpawn1() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_SPAWN_1,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetSpawnPnt(1, val);

	PFrame->UpdateDialog();
}

void CMcAttach::OnKillfocusMatcenSpawn2() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_SPAWN_2,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetSpawnPnt(2, val);

	PFrame->UpdateDialog();
}

void CMcAttach::OnKillfocusMatcenSpawn3() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_SPAWN_3,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetSpawnPnt(3, val);

	PFrame->UpdateDialog();
}

void CMcAttach::OnKillfocusMatcenSpawnNum() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;

	GetDlgItemText(IDC_MATCEN_SPAWN_NUM,str,20);

	val = atoi(str);

	if (val < 0)
		val = 0;
	else if (val > MAX_SPAWN_PNTS)
		val = MAX_SPAWN_PNTS;

	Matcen[m_matcen_id]->SetNumSpawnPnts(val);

	PFrame->UpdateDialog();
}

void CMcAttach::OnSelendokMatcenRoomObjectCombo() 
{
	// TODO: Add your control notification handler code here
	switch (Matcen[m_matcen_id]->GetAttachType()) {
		case MT_ROOM:
			Matcen[m_matcen_id]->SetAttach(m_room_combo.GetSelected());
			break;
		case MT_OBJECT:
			Matcen[m_matcen_id]->SetAttach(m_object_combo.GetSelected());
			break;
		case MT_UNASSIGNED:
			Int3();
			break;
	}

	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenApnRadio() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetControlType(MPC_AFTER_PLAYER_NEAR);
	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenApvRadio() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetControlType(MPC_AFTER_PLAYER_VISIBLE);
	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenScRadio() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetControlType(MPC_SCRIPT);
	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenWpnRadio() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetControlType(MPC_WHILE_PLAYER_NEAR);
	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenWpvRadio() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetControlType(MPC_WHILE_PLAYER_VISIBLE);
	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenObjtypeNum() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_OBJTYPE_NUM,str,20);
	
	val = atoi(str);

	if (val<-1)
		val=-1;
	else if (val>MAX_PROD_TYPES)
		val=MAX_PROD_TYPES;
	
	Matcen[m_matcen_id]->SetNumProdTypes(val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnSelendokMatcenProd0List() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_MATCEN_PROD_0_LIST, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_MATCEN_PROD_0_LIST, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);

	int obj_id = ned_FindObjectID(name);
	Matcen[m_matcen_id]->SetProdInfo(0, &obj_id, NULL, NULL, NULL);
		
	PFrame->UpdateDialog();
}

void CMcProduce::OnSelendokMatcenProd1List() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_MATCEN_PROD_1_LIST, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_MATCEN_PROD_1_LIST, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);

	int obj_id = ned_FindObjectID(name);
	Matcen[m_matcen_id]->SetProdInfo(1, &obj_id, NULL, NULL, NULL);
		
	PFrame->UpdateDialog();
}

void CMcProduce::OnSelendokMatcenProd2List() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_MATCEN_PROD_2_LIST, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_MATCEN_PROD_2_LIST, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);

	int obj_id = ned_FindObjectID(name);
	Matcen[m_matcen_id]->SetProdInfo(2, &obj_id, NULL, NULL, NULL);
		
	PFrame->UpdateDialog();
}

void CMcProduce::OnSelendokMatcenProd3List() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_MATCEN_PROD_3_LIST, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_MATCEN_PROD_3_LIST, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);

	int obj_id = ned_FindObjectID(name);
	Matcen[m_matcen_id]->SetProdInfo(3, &obj_id, NULL, NULL, NULL);
		
	PFrame->UpdateDialog();
}

void CMcProduce::OnSelendokMatcenProd4List() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_MATCEN_PROD_4_LIST, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_MATCEN_PROD_4_LIST, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);

	int obj_id = ned_FindObjectID(name);
	Matcen[m_matcen_id]->SetProdInfo(4, &obj_id, NULL, NULL, NULL);
		
	PFrame->UpdateDialog();
}

void CMcProduce::OnSelendokMatcenProd5List() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_MATCEN_PROD_5_LIST, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_MATCEN_PROD_5_LIST, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);

	int obj_id = ned_FindObjectID(name);
	Matcen[m_matcen_id]->SetProdInfo(5, &obj_id, NULL, NULL, NULL);
		
	PFrame->UpdateDialog();
}

void CMcProduce::OnSelendokMatcenProd6List() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_MATCEN_PROD_6_LIST, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_MATCEN_PROD_6_LIST, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);

	int obj_id = ned_FindObjectID(name);
	Matcen[m_matcen_id]->SetProdInfo(6, &obj_id, NULL, NULL, NULL);
		
	PFrame->UpdateDialog();
}

void CMcProduce::OnSelendokMatcenProd7List() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_MATCEN_PROD_7_LIST, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_MATCEN_PROD_7_LIST, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);

	int obj_id = ned_FindObjectID(name);
	Matcen[m_matcen_id]->SetProdInfo(7, &obj_id, NULL, NULL, NULL);
		
	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd0Quantity() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_0_QUANTITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(0, NULL, NULL, NULL, &val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd1Quantity() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_1_QUANTITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(1, NULL, NULL, NULL, &val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd2Quantity() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_2_QUANTITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(2, NULL, NULL, NULL, &val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd3Quantity() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_3_QUANTITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(3, NULL, NULL, NULL, &val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd4Quantity() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_4_QUANTITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(4, NULL, NULL, NULL, &val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd5Quantity() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_5_QUANTITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(5, NULL, NULL, NULL, &val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd6Quantity() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_6_QUANTITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(6, NULL, NULL, NULL, &val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd7Quantity() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_7_QUANTITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(7, NULL, NULL, NULL, &val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd0Seconds() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PROD_0_SECONDS,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(0, NULL, NULL, &val, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd1Seconds() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PROD_1_SECONDS,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(1, NULL, NULL, &val, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd2Seconds() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PROD_2_SECONDS,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(2, NULL, NULL, &val, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd3Seconds() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PROD_3_SECONDS,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(3, NULL, NULL, &val, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd4Seconds() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PROD_4_SECONDS,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(4, NULL, NULL, &val, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd5Seconds() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PROD_5_SECONDS,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(5, NULL, NULL, &val, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd6Seconds() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PROD_6_SECONDS,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(6, NULL, NULL, &val, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd7Seconds() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PROD_7_SECONDS,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(7, NULL, NULL, &val, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd0Priority() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_0_PRIORITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(0, NULL, &val, NULL, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd1Priority() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_1_PRIORITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(1, NULL, &val, NULL, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd2Priority() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_2_PRIORITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(2, NULL, &val, NULL, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd3Priority() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_3_PRIORITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(3, NULL, &val, NULL, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd4Priority() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_4_PRIORITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(4, NULL, &val, NULL, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd5Priority() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_5_PRIORITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(5, NULL, &val, NULL, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd6Priority() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_6_PRIORITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(6, NULL, &val, NULL, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProd7Priority() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PROD_7_PRIORITY,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetProdInfo(7, NULL, &val, NULL, NULL);

	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenProduceEnabled() 
{
	// TODO: Add your control notification handler code here
	int status = Matcen[m_matcen_id]->GetStatus();

	if(status & MSTAT_DISABLED)
		Matcen[m_matcen_id]->SetStatus(MSTAT_DISABLED, false);
	else
		Matcen[m_matcen_id]->SetStatus(MSTAT_DISABLED, true);

	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenProduceHurt() 
{
	// TODO: Add your control notification handler code here
	int status = Matcen[m_matcen_id]->GetStatus();

	if(status & MSTAT_NOT_HURT_PLAYER)
		Matcen[m_matcen_id]->SetStatus(MSTAT_NOT_HURT_PLAYER, false);
	else
		Matcen[m_matcen_id]->SetStatus(MSTAT_NOT_HURT_PLAYER, true);

	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenProduceRandom() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetStatus(MSTAT_RANDOM_PROD_ORDER, true);
	PFrame->UpdateDialog();
}

void CMcProduce::OnMatcenProduceOrdered() 
{
	// TODO: Add your control notification handler code here
	Matcen[m_matcen_id]->SetStatus(MSTAT_RANDOM_PROD_ORDER, false);
	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProduceMax() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PRODUCE_MAX,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetMaxProd(val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProduceMult() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PRODUCE_MULT,str,20);
	
	val = atof(str);
	Matcen[m_matcen_id]->SetProdMultiplier(val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProduceMaxalive() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	int val;
	
	GetDlgItemText(IDC_MATCEN_PRODUCE_MAXALIVE,str,20);
	
	val = atoi(str);
	Matcen[m_matcen_id]->SetMaxAliveChildren(val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProducePretime() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PRODUCE_PRETIME,str,20);
	
	val = atof(str);

	Matcen[m_matcen_id]->SetPreProdTime(val);

	PFrame->UpdateDialog();
}

void CMcProduce::OnKillfocusMatcenProducePosttime() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
	
	GetDlgItemText(IDC_MATCEN_PRODUCE_POSTTIME,str,20);
	
	val = atof(str);

	Matcen[m_matcen_id]->SetPostProdTime(val);

	PFrame->UpdateDialog();
}

void CMcDisplay::OnSelendokDisplayActiveSound() 
{
	// TODO: Add your control notification handler code here
	int i,cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_DISPLAY_ACTIVE_SOUND, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_DISPLAY_ACTIVE_SOUND, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);
	
	i=ned_FindSound (name);
		
	if (i==-1)
	{
		i = SOUND_NONE_INDEX;
	}

	Matcen[m_matcen_id]->SetSound(MATCEN_ACTIVE_SOUND, i);
	
	PFrame->UpdateDialog();
}

void CMcDisplay::OnSelendokDisplayProdEffect() 
{
	// TODO: Add your control notification handler code here
	int i;

	i = SendDlgItemMessage( IDC_DISPLAY_PROD_EFFECT, CB_GETCURSEL,0,0);
	
	Matcen[m_matcen_id]->SetCreationEffect(i);
	
	PFrame->UpdateDialog();
}

void CMcDisplay::OnSelendokDisplayProdSound() 
{
	// TODO: Add your control notification handler code here
	int i,cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_DISPLAY_PROD_SOUND, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_DISPLAY_PROD_SOUND, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);
	
	i=ned_FindSound (name);
		
	if (i==-1)
	{
		i = SOUND_NONE_INDEX;
	}

	Matcen[m_matcen_id]->SetSound(MATCEN_PROD_SOUND, i);
	
	PFrame->UpdateDialog();
}

void CMcDisplay::OnSelendokDisplayProdTexture() 
{
	// TODO: Add your control notification handler code here
	int i,cur;
	char name[200];

	cur=SendDlgItemMessage( IDC_DISPLAY_PROD_TEXTURE, CB_GETCURSEL,0,0);
	SendDlgItemMessage( IDC_DISPLAY_PROD_TEXTURE, CB_GETLBTEXT,cur,(LPARAM) (LPCTSTR)name);
	
	i=FindTextureName (name);
		
	if (i==-1)
	{
		i = 0;
	}

	Matcen[m_matcen_id]->SetCreationTexture(i);
	
	PFrame->UpdateDialog();
}

void CMcAttach::OnCancel() 
{
	// TODO: Add extra cleanup here
	// Do nothing!
}

void CMcAttach::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}

void CMcDisplay::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}

void CMcDisplay::OnCancel() 
{
	// TODO: Add extra cleanup here
	// Do nothing!
}

void CMcProduce::OnCancel() 
{
	// TODO: Add extra cleanup here
	// Do nothing!
}

void CMcProduce::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}
