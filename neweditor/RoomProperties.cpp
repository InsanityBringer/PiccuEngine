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
 // RoomProperties.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "RoomProperties.h"
#include "ned_Util.h"
#include "../editor/Erooms.h"
#include "EditLineDialog.h"
#include "damage_external.h"
#include "ned_Rend.h"
#include "ambient.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRoomProperties dialog


CRoomProperties::CRoomProperties(CWnd* pParent )
	: CDialog(CRoomProperties::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRoomProperties)
	//}}AFX_DATA_INIT
	m_pPrim = theApp.AcquirePrim();
}


void CRoomProperties::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRoomProperties)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRoomProperties, CDialog)
	//{{AFX_MSG_MAP(CRoomProperties)
	ON_BN_CLICKED(IDC_REFUELING_CHECK, OnRefuelingCheck)
	ON_BN_CLICKED(IDC_SECRET_CHECK, OnSecretCheck)
	ON_BN_CLICKED(IDC_REDGOAL_CHECK, OnRedGoalCheck)
	ON_BN_CLICKED(IDC_BLUEGOAL_CHECK, OnBlueGoalCheck)
	ON_BN_CLICKED(IDC_GREENGOAL_CHECK, OnGreenGoalCheck)
	ON_BN_CLICKED(IDC_YELLOWGOAL_CHECK, OnYellowGoalCheck)
	ON_BN_CLICKED(IDC_SPECIAL1_CHECK, OnSpecial1Check)
	ON_BN_CLICKED(IDC_SPECIAL2_CHECK, OnSpecial2Check)
	ON_BN_CLICKED(IDC_SPECIAL3_CHECK, OnSpecial3Check)
	ON_BN_CLICKED(IDC_SPECIAL4_CHECK, OnSpecial4Check)
	ON_BN_CLICKED(IDC_SPECIAL5_CHECK, OnSpecial5Check)
	ON_BN_CLICKED(IDC_SPECIAL6_CHECK, OnSpecial6Check)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_ROOM_NAME, OnRoomName)
	ON_EN_KILLFOCUS(IDC_ROOM_DAMAGEAMOUNT, OnKillfocusRoomDamageAmount)
	ON_CBN_SELCHANGE(IDC_ROOM_DAMAGETYPE, OnSelchangeRoomDamageType)
	ON_BN_CLICKED(IDC_FACE_GOAL_CHECK, OnFaceGoalCheck)
	ON_EN_KILLFOCUS(IDC_MIRROR_FACE, OnKillfocusMirrorFace)
	ON_BN_CLICKED(IDC_RADIO_NO_MIRROR, OnRadioNoMirror)
	ON_BN_CLICKED(IDC_RADIO_MIRROR_FACE, OnRadioMirrorFace)
	ON_BN_CLICKED(IDC_SET_MIRROR, OnSetMirror)
	ON_BN_CLICKED(IDC_FOG_ENABLE, OnFogEnable)
	ON_EN_KILLFOCUS(IDC_FOG_DEPTH, OnKillfocusFogDepth)
	ON_EN_KILLFOCUS(IDC_FOG_RED, OnKillfocusFogRed)
	ON_EN_KILLFOCUS(IDC_FOG_GREEN, OnKillfocusFogGreen)
	ON_EN_KILLFOCUS(IDC_FOG_BLUE, OnKillfocusFogBlue)
	ON_BN_CLICKED(IDC_WIND_MAKE, OnWindMake)
	ON_EN_KILLFOCUS(IDC_WIND_SPEED, OnKillfocusWindSpeed)
	ON_EN_KILLFOCUS(IDC_WIND_X, OnKillfocusWindX)
	ON_EN_KILLFOCUS(IDC_WIND_Y, OnKillfocusWindY)
	ON_EN_KILLFOCUS(IDC_WIND_Z, OnKillfocusWindZ)
	ON_CBN_SELCHANGE(IDC_ROOM_AMBIENT, OnSelchangeRoomAmbient)
	ON_BN_CLICKED(IDC_ROOM_EXTERNAL, OnRoomExternal)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRoomProperties message handlers

void CRoomProperties::EnableControls(bool enable)
{
	((CButton *)GetDlgItem(IDC_ROOM_NAME))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_REFUELING_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_REDGOAL_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_BLUEGOAL_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_GREENGOAL_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_YELLOWGOAL_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_SPECIAL1_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_SPECIAL2_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_SPECIAL3_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_SPECIAL4_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_SPECIAL5_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_SPECIAL6_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_SECRET_CHECK))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_FACE_GOAL_CHECK))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_ROOM_DAMAGEAMOUNT))->EnableWindow(enable);
	((CComboBox *)GetDlgItem(IDC_ROOM_DAMAGETYPE))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_MIRROR_FACE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_RADIO_NO_MIRROR))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_RADIO_MIRROR_FACE))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_SET_MIRROR))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_FOG_ENABLE))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_FOG_DEPTH))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_FOG_RED))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_FOG_GREEN))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_FOG_BLUE))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_WIND_SPEED))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_WIND_X))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_WIND_Y))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_WIND_Z))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_WIND_MAKE))->EnableWindow(enable);
	((CEdit *)GetDlgItem(IDC_ROOM_AMBIENT))->EnableWindow(enable);
	((CButton *)GetDlgItem(IDC_ROOM_EXTERNAL))->EnableWindow(enable);
}

#define NULL_NAME "<none>"

void CRoomProperties::UpdateDialog()
{
	CComboBox *cbox;
	int i;

	m_pPrim = theApp.AcquirePrim();
	if (m_pPrim == NULL || m_pPrim->roomp == NULL || ROOMNUM(m_pPrim->roomp) >= MAX_ROOMS)
	{
//		EnableTabControls(this->m_hWnd,false,IDC_ROOM_NAME,-1);
		EnableControls(false);
		return;
	}
	else
	{
//		EnableTabControls(this->m_hWnd,true,IDC_ROOM_NAME,-1);
		EnableControls(true);
	}

	room *rp = m_pPrim->roomp;

	PrintToDlgItem(this,IDC_ROOM_NAME,"%s",rp->name?rp->name:"<none>");

	//Show flags
	CheckDlgButton(IDC_REFUELING_CHECK,rp->flags & RF_FUELCEN);
	CheckDlgButton(IDC_REDGOAL_CHECK,rp->flags & RF_GOAL1);
	CheckDlgButton(IDC_BLUEGOAL_CHECK,rp->flags & RF_GOAL2);
	CheckDlgButton(IDC_GREENGOAL_CHECK,rp->flags & RF_GOAL3);
	CheckDlgButton(IDC_YELLOWGOAL_CHECK,rp->flags & RF_GOAL4);
	CheckDlgButton(IDC_SPECIAL1_CHECK,rp->flags & RF_SPECIAL1);
	CheckDlgButton(IDC_SPECIAL2_CHECK,rp->flags & RF_SPECIAL2);
	CheckDlgButton(IDC_SPECIAL3_CHECK,rp->flags & RF_SPECIAL3);
	CheckDlgButton(IDC_SPECIAL4_CHECK,rp->flags & RF_SPECIAL4);
	CheckDlgButton(IDC_SPECIAL5_CHECK,rp->flags & RF_SPECIAL5);
	CheckDlgButton(IDC_SPECIAL6_CHECK,rp->flags & RF_SPECIAL6);
	CheckDlgButton(IDC_SECRET_CHECK,(rp->flags & RF_SECRET)?1:0);
	CheckDlgButton(IDC_ROOM_EXTERNAL,rp->flags & RF_EXTERNAL);

	// Set goal face check
	if (m_pPrim->face != -1)
		CheckDlgButton(IDC_FACE_GOAL_CHECK,rp->faces[m_pPrim->face].flags & FF_GOALFACE);

	// Enable goal face check
	((CButton *)GetDlgItem(IDC_FACE_GOAL_CHECK))->EnableWindow(m_pPrim->face != -1);

	//Build the ambient sound list
	cbox = (CComboBox *)GetDlgItem(IDC_ROOM_AMBIENT);
	cbox->ResetContent();
	cbox->AddString((LPCTSTR) NULL_NAME);

	for (i=0;i<Num_ambient_sound_patterns;i++)
		if (Ambient_sound_patterns[i].name[0])
			cbox->AddString((LPCTSTR) AmbientSoundPatternName(i));
	cbox->SelectString(0,(LPCTSTR) NULL_NAME);

	//Set the current ambient sound pattern
	cbox->SelectString(0,(LPCTSTR) ((m_pPrim->roomp->ambient_sound == -1) ? NULL_NAME : AmbientSoundPatternName(m_pPrim->roomp->ambient_sound)));

	// Update Mirror Stuff
	CheckDlgButton(IDC_RADIO_NO_MIRROR,rp->mirror_face==-1);
	CheckDlgButton(IDC_RADIO_MIRROR_FACE,rp->mirror_face!=-1);
	((CButton *)GetDlgItem(IDC_RADIO_MIRROR_FACE))->EnableWindow(rp->faces>0);
	((CEdit *)GetDlgItem(IDC_MIRROR_FACE))->EnableWindow(rp->faces>0 && IsDlgButtonChecked(IDC_RADIO_MIRROR_FACE));
	SetDlgItemInt(IDC_MIRROR_FACE,rp->mirror_face,TRUE);
	((CButton *)GetDlgItem(IDC_SET_MIRROR))->EnableWindow(m_pPrim->face != -1);

	// Update Fog Stuff
	CheckDlgButton(IDC_FOG_ENABLE,(rp->flags & RF_FOG)?1:0);

	((CEdit *)GetDlgItem(IDC_FOG_DEPTH))->EnableWindow(rp->flags & RF_FOG);
	((CEdit *)GetDlgItem(IDC_FOG_RED))->EnableWindow(rp->flags & RF_FOG);
	((CEdit *)GetDlgItem(IDC_FOG_GREEN))->EnableWindow(rp->flags & RF_FOG);
	((CEdit *)GetDlgItem(IDC_FOG_BLUE))->EnableWindow(rp->flags & RF_FOG);

	char str[255];
	sprintf (str,"%f",rp->fog_depth);
	SetDlgItemText(IDC_FOG_DEPTH,str);

	sprintf (str,"%f",rp->fog_r);
	SetDlgItemText(IDC_FOG_RED,str);

	sprintf (str,"%f",rp->fog_g);
	SetDlgItemText(IDC_FOG_GREEN,str);

	sprintf (str,"%f",rp->fog_b);
	SetDlgItemText(IDC_FOG_BLUE,str);

	// Update Wind Stuff
	vector vec = {0,0,0};
	float speed = 0.00f;

	GetWind(&speed,&vec);

	sprintf(str,"%.2f",speed);
	SetDlgItemText(IDC_WIND_SPEED,str);

	sprintf(str,"%.2f",vec.x);
	SetDlgItemText(IDC_WIND_X,str);

	sprintf(str,"%.2f",vec.y);
	SetDlgItemText(IDC_WIND_Y,str);

	sprintf(str,"%.2f",vec.z);
	SetDlgItemText(IDC_WIND_Z,str);

	//Update the damage
	CEdit *box = (CEdit *)GetDlgItem(IDC_ROOM_DAMAGEAMOUNT);
	sprintf(str,"%.2f",rp->damage);
	box->SetWindowText(str);

	//Update the damage type
	cbox = (CComboBox *)GetDlgItem(IDC_ROOM_DAMAGETYPE);
	int n = cbox->GetCount();
	for (i=0;i<n;i++) {
		if (cbox->GetItemData(i) == rp->damage_type) {
			cbox->SetCurSel(i);
			break;
		}
	}
	ASSERT(i < n);
}

void CRoomProperties::OnRefuelingCheck() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_REFUELING_CHECK))
		m_pPrim->roomp->flags |= RF_FUELCEN;
	else
		m_pPrim->roomp->flags &= ~RF_FUELCEN;

	World_changed = 1;
}

void CRoomProperties::OnSecretCheck() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_SECRET_CHECK))
		m_pPrim->roomp->flags |= RF_SECRET;
	else
		m_pPrim->roomp->flags &= ~RF_SECRET;

	World_changed = 1;
}

void CRoomProperties::OnRedGoalCheck() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_REDGOAL_CHECK))
		m_pPrim->roomp->flags |= RF_GOAL1;
	else
		m_pPrim->roomp->flags &= ~RF_GOAL1;

	World_changed = 1;
}

void CRoomProperties::OnBlueGoalCheck() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_BLUEGOAL_CHECK))
		m_pPrim->roomp->flags |= RF_GOAL2;
	else
		m_pPrim->roomp->flags &= ~RF_GOAL2;

	World_changed = 1;
}

void CRoomProperties::OnGreenGoalCheck() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_GREENGOAL_CHECK))
		m_pPrim->roomp->flags |= RF_GOAL3;
	else
		m_pPrim->roomp->flags &= ~RF_GOAL3;

	World_changed = 1;
}

void CRoomProperties::OnYellowGoalCheck() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_YELLOWGOAL_CHECK))
		m_pPrim->roomp->flags |= RF_GOAL4;
	else
		m_pPrim->roomp->flags &= ~RF_GOAL4;

	World_changed = 1;
}

void CRoomProperties::OnSpecial1Check() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_SPECIAL1_CHECK))
		m_pPrim->roomp->flags |= RF_SPECIAL1;
	else
		m_pPrim->roomp->flags &= ~RF_SPECIAL1;

	World_changed = 1;
}

void CRoomProperties::OnSpecial2Check() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_SPECIAL2_CHECK))
		m_pPrim->roomp->flags |= RF_SPECIAL2;
	else
		m_pPrim->roomp->flags &= ~RF_SPECIAL2;

	World_changed = 1;
}

void CRoomProperties::OnSpecial3Check() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_SPECIAL3_CHECK))
		m_pPrim->roomp->flags |= RF_SPECIAL3;
	else
		m_pPrim->roomp->flags &= ~RF_SPECIAL3;

	World_changed = 1;
}

void CRoomProperties::OnSpecial4Check() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_SPECIAL4_CHECK))
		m_pPrim->roomp->flags |= RF_SPECIAL4;
	else
		m_pPrim->roomp->flags &= ~RF_SPECIAL4;

	World_changed = 1;
}

void CRoomProperties::OnSpecial5Check() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_SPECIAL5_CHECK))
		m_pPrim->roomp->flags |= RF_SPECIAL5;
	else
		m_pPrim->roomp->flags &= ~RF_SPECIAL5;

	World_changed = 1;
}

void CRoomProperties::OnSpecial6Check() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_SPECIAL6_CHECK))
		m_pPrim->roomp->flags |= RF_SPECIAL6;
	else
		m_pPrim->roomp->flags &= ~RF_SPECIAL6;

	World_changed = 1;
}

void CRoomProperties::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	UpdateDialog();	
	// Do not call CDialog::OnPaint() for painting messages
}

void CRoomProperties::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	theApp.m_pRoomPropsDlg = NULL;
	delete this;
}

void CRoomProperties::OnCancel() 
{
	// TODO: Add extra cleanup here
	DestroyWindow();
}

BOOL CRoomProperties::OnInitDialog() 
{
	CDialog::OnInitDialog();

	int i;

	// TODO: Add extra initialization here
	CEdit *box = (CEdit *) GetDlgItem(IDC_ROOM_NAME);

	box->SetLimitText(ROOM_NAME_LEN);

	//Set up damage type dialog
	CComboBox *cbox = (CComboBox *) GetDlgItem(IDC_ROOM_DAMAGETYPE);
	i=cbox->AddString("None");				cbox->SetItemData(i,PD_NONE);
	i=cbox->AddString("Energy Weapon");		cbox->SetItemData(i,PD_ENERGY_WEAPON);
	i=cbox->AddString("Matter Weapon");		cbox->SetItemData(i,PD_MATTER_WEAPON);
	i=cbox->AddString("Melee Attack");		cbox->SetItemData(i,PD_MELEE_ATTACK);	
	i=cbox->AddString("Concussive Force");	cbox->SetItemData(i,PD_CONCUSSIVE_FORCE);
	i=cbox->AddString("Wall Hit");			cbox->SetItemData(i,PD_WALL_HIT);
	i=cbox->AddString("Volatile Hiss");		cbox->SetItemData(i,PD_VOLATILE_HISS);

	cbox->SetCurSel(0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRoomProperties::OnRoomName() 
{
	// TODO: Add your control notification handler code here
	char tempname[ROOM_NAME_LEN+1] = "";

	RenameRoom(m_pPrim->roomp);
}

void CRoomProperties::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}

void CRoomProperties::OnKillfocusRoomDamageAmount() 
{
	// TODO: Add your control notification handler code here
	char buf[128];

	GetDlgItemText(IDC_ROOM_DAMAGEAMOUNT,buf,sizeof(buf));

	m_pPrim->roomp->damage = atof(buf);

	World_changed = 1;
}

void CRoomProperties::OnSelchangeRoomDamageType() 
{
	// TODO: Add your control notification handler code here
	CComboBox *box = (CComboBox *) GetDlgItem(IDC_ROOM_DAMAGETYPE);

	m_pPrim->roomp->damage_type = (sbyte) box->GetItemData(box->GetCurSel());

	World_changed = 1;
}

void CRoomProperties::OnFaceGoalCheck() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_FACE_GOAL_CHECK))
	{
		if (!(m_pPrim->roomp->flags & GOALROOM|RF_SPECIAL1|RF_SPECIAL2|RF_SPECIAL3|RF_SPECIAL4|RF_SPECIAL5|RF_SPECIAL6))
		{
			OutrageMessageBox ("This room needs to marked as a goal first.");
			return;
		}
		else
			m_pPrim->roomp->faces[m_pPrim->face].flags |=FF_GOALFACE;
	}
	else
		m_pPrim->roomp->faces[m_pPrim->face].flags &=~FF_GOALFACE;

	World_changed = 1;
}

// Builds a list of mirror faces for each room and allocs memory accordingly
void ConsolidateMineMirrors();

void CRoomProperties::OnKillfocusMirrorFace() 
{
	// TODO: Add your control notification handler code here
	m_pPrim->roomp->mirror_face = GetDlgItemInt(IDC_MIRROR_FACE,NULL,FALSE);

	ConsolidateMineMirrors();

	World_changed = 1;
}

void CRoomProperties::OnRadioNoMirror() 
{
	// TODO: Add your control notification handler code here
	m_pPrim->roomp->mirror_face = -1;

	ConsolidateMineMirrors();

	World_changed = 1;
}

void CRoomProperties::OnRadioMirrorFace() 
{
	// TODO: Add your control notification handler code here
	BOOL bSuccess;

	int face = GetDlgItemInt(IDC_MIRROR_FACE,&bSuccess,TRUE);

	if (bSuccess)
	{
		if (face == -1)
		{
			face = 0;
			SetDlgItemInt(IDC_MIRROR_FACE,0,TRUE);
		}
		m_pPrim->roomp->mirror_face = face;
		ConsolidateMineMirrors();
		World_changed = 1;
	}
}

void CRoomProperties::OnSetMirror() 
{
	// TODO: Add your control notification handler code here
	m_pPrim->roomp->mirror_face=m_pPrim->face;
	ConsolidateMineMirrors();

	SetDlgItemInt(IDC_MIRROR_FACE,m_pPrim->face,TRUE);

	World_changed = 1;
}

void CRoomProperties::OnFogEnable() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_FOG_ENABLE);

	if (c)	
		m_pPrim->roomp->flags|=RF_FOG;
	else
		m_pPrim->roomp->flags&=~RF_FOG;

	World_changed=1;
}

void CRoomProperties::OnKillfocusFogDepth() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;

	GetDlgItemText(IDC_FOG_DEPTH,str,20);

	aval=atof (str);
	if (aval<0)
		aval=5;
	if (aval>5000)
		aval=5000;

	m_pPrim->roomp->fog_depth=aval;

	World_changed=1;
}

void CRoomProperties::OnKillfocusFogRed() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;

	GetDlgItemText(IDC_FOG_RED,str,20);

	aval=atof (str);
	if (aval<0)
		aval=0;
	if (aval>1.0)
		aval=1;

	m_pPrim->roomp->fog_r=aval;

	World_changed=1;
}

void CRoomProperties::OnKillfocusFogGreen() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;

	GetDlgItemText(IDC_FOG_GREEN,str,20);

	aval=atof (str);
	if (aval<0)
		aval=0;
	if (aval>1.0)
		aval=1;

	m_pPrim->roomp->fog_g=aval;

	World_changed=1;
}

void CRoomProperties::OnKillfocusFogBlue() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;

	GetDlgItemText(IDC_FOG_BLUE,str,20);

	aval=atof (str);
	if (aval<0)
		aval=0;
	if (aval>1.0)
		aval=1;

	m_pPrim->roomp->fog_b=aval;

	World_changed=1;
}

void CRoomProperties::OnWindMake() 
{
	// TODO: Add your control notification handler code here
	int num1=0,num2=0;
	float speed;
	vector vec;
	char str[20];
	room *rp = m_pPrim->roomp;

	if (InputNumber(&num1,"Wind Start","Enter first vertex number",this))
	{
		if (num1<0 || num1>rp->num_verts-1)
		{
			OutrageMessageBox("Invalid vertex number.");
			return;
		}
		if (InputNumber(&num2,"Wind Finish","Enter second vertex number",this))
		{
			if (num2<0 || num2>rp->num_verts-1)
			{
				OutrageMessageBox("Invalid vertex number.");
				return;
			}
			OutrageMessageBox("Wind direction and speed will be calculated from the positions of the specified vertices. You may change these values at any time.");
			vec = rp->verts[num2] - rp->verts[num1];
			speed = vm_NormalizeVector(&vec);
			sprintf(str,"%.2f",speed);
			SetDlgItemText(IDC_WIND_SPEED,str);
			sprintf(str,"%.2f",vec.x);
			SetDlgItemText(IDC_WIND_X,str);
			sprintf(str,"%.2f",vec.y);
			SetDlgItemText(IDC_WIND_Y,str);
			sprintf(str,"%.2f",vec.z);
			SetDlgItemText(IDC_WIND_Z,str);
			SetWind();
		}
	}
}

void CRoomProperties::OnKillfocusWindSpeed() 
{
	// TODO: Add your control notification handler code here
	SetWind();
}

void CRoomProperties::OnKillfocusWindX() 
{
	// TODO: Add your control notification handler code here
	SetWind();
}

void CRoomProperties::OnKillfocusWindY() 
{
	// TODO: Add your control notification handler code here
	SetWind();
}

void CRoomProperties::OnKillfocusWindZ() 
{
	// TODO: Add your control notification handler code here
	SetWind();
}

void CRoomProperties::SetWind()
{
	vector vec = {0,0,0};
	float speed = 0.00f;
	char str[20];

	GetDlgItemText(IDC_WIND_SPEED,str,20);
	speed = atof(str);
	GetDlgItemText(IDC_WIND_X,str,20);
	vec.x = atof(str);
	GetDlgItemText(IDC_WIND_Y,str,20);
	vec.y = atof(str);
	GetDlgItemText(IDC_WIND_Z,str,20);
	vec.z = atof(str);

	vm_NormalizeVector(&vec);

	m_pPrim->roomp->wind = speed*vec;

	World_changed = 1;
}

void CRoomProperties::GetWind(float *speed,vector *direction)
{
	*direction = m_pPrim->roomp->wind;
	*speed = vm_NormalizeVector(direction);
}

void CRoomProperties::OnSelchangeRoomAmbient() 
{
	// TODO: Add your control notification handler code here
	int cur;
	char name[200];

	CComboBox *combo = (CComboBox *)GetDlgItem(IDC_ROOM_AMBIENT);
	cur = combo->GetCurSel();
	if ( combo->GetLBText(cur,name) != CB_ERR )
		m_pPrim->roomp->ambient_sound = FindAmbientSoundPattern(name);

	World_changed = 1;
}

void CRoomProperties::OnRoomExternal() 
{
	// TODO: Add your control notification handler code here
	if (IsDlgButtonChecked(IDC_ROOM_EXTERNAL))
		m_pPrim->roomp->flags |= RF_EXTERNAL;
	else
		m_pPrim->roomp->flags &= ~RF_EXTERNAL;

	World_changed = 1;
}
