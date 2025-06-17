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
 // LightingDialog.cpp : implementation file
//

#include "stdafx.h"
#include "neweditor.h"
#include "LightingDialog.h"
#include "radiosity.h"
#include "editor_lighting.h"
#include "ned_Util.h"
#include "terrain.h"
#include "polymodel.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLightingDialog dialog


CLightingDialog::CLightingDialog(CWnd* pParent )
	: CDialog(CLightingDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLightingDialog)
	//}}AFX_DATA_INIT
}


void CLightingDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLightingDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLightingDialog, CDialog)
	//{{AFX_MSG_MAP(CLightingDialog)
	ON_WM_PAINT()
	ON_EN_KILLFOCUS(IDC_LIGHTMAP_SPACING, OnKillfocusLightmapSpacing)
	ON_EN_KILLFOCUS(IDC_ITERATIONS, OnKillfocusIterations)
	ON_EN_KILLFOCUS(IDC_IGNORE_LIMIT, OnKillfocusIgnoreLimit)
	ON_BN_CLICKED(IDC_COMBINE_FACES, OnCombineFaces)
	ON_BN_CLICKED(IDC_SUPER_DETAIL, OnSuperDetail)
	ON_BN_CLICKED(IDC_VOLUME_LIGHTS, OnVolumeLights)
	ON_BN_CLICKED(IDC_LIGHTS_IN_MINE, OnLightsInMine)
	ON_BN_CLICKED(IDC_MINE_RADIOSITY, OnMineRadiosity)
	ON_BN_CLICKED(IDC_CLEAR_LIGHTMAPS, OnClearLightmaps)
	ON_BN_CLICKED(IDC_RESET_MULTIPLES, OnResetMultiples)
	ON_EN_KILLFOCUS(IDC_AMBIENCE_BLUE, OnKillfocusAmbienceBlue)
	ON_EN_KILLFOCUS(IDC_AMBIENCE_GREEN, OnKillfocusAmbienceGreen)
	ON_EN_KILLFOCUS(IDC_AMBIENCE_RED, OnKillfocusAmbienceRed)
	ON_BN_CLICKED(IDC_FILL_CORONAS, OnFillCoronas)
	ON_EN_KILLFOCUS(IDC_MULTIPLIER, OnKillfocusMultiplier)
	ON_BN_CLICKED(IDC_ROOM_CLEAR_LIGHTMAPS, OnRoomClearLightmaps)
	ON_EN_KILLFOCUS(IDC_ROOM_AMBIENCE_BLUE, OnKillfocusRoomAmbienceBlue)
	ON_EN_KILLFOCUS(IDC_ROOM_AMBIENCE_GREEN, OnKillfocusRoomAmbienceGreen)
	ON_EN_KILLFOCUS(IDC_ROOM_AMBIENCE_RED, OnKillfocusRoomAmbienceRed)
	ON_BN_CLICKED(IDC_ROOM_FILL_CORONAS, OnRoomFillCoronas)
	ON_EN_KILLFOCUS(IDC_ROOM_MULTIPLIER, OnKillfocusRoomMultiplier)
	ON_BN_CLICKED(IDC_ROOM_RADIOSITY, OnRoomRadiosity)
	ON_BN_CLICKED(IDC_ROOM_RESET_MULTIPLES, OnRoomResetMultiples)
	ON_BN_CLICKED(IDC_ROOM_COUNT_LIGHTS, OnRoomCountLights)
	ON_BN_CLICKED(IDC_FACE_CORONA, OnFaceCorona)
	ON_EN_KILLFOCUS(IDC_FACE_MULTIPLIER, OnKillfocusFaceMultiplier)
	ON_BN_CLICKED(IDC_TOUCHES_TERRAIN, OnTouchesTerrain)
	ON_BN_CLICKED(IDC_TERRAIN_RADIOSITY, OnTerrainRadiosity)
	ON_BN_CLICKED(IDC_LIGHTS_ON_TERRAIN, OnLightsOnTerrain)
	ON_BN_CLICKED(IDC_DYNAMIC_TERRAIN_LIGHT, OnDynamicTerrainLight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLightingDialog message handlers

BOOL CLightingDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CLightingDialog::PostNcDestroy() 
{
	// TODO: Add your specialized code here and/or call the base class
	theApp.m_pLightingDlg = NULL;
	delete this;
}

void CLightingDialog::OnCancel() 
{
	// TODO: Add extra cleanup here
	DestroyWindow();
}

void CLightingDialog::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	UpdateDialog();

	// Do not call CDialog::OnPaint() for painting messages
}

void CLightingDialog::EnableControls(bool enable,int which)
{
	switch (which)
	{
	case 0:
		((CButton *)GetDlgItem(IDC_LIGHTMAP_SPACING))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_ITERATIONS))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_IGNORE_LIMIT))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_COMBINE_FACES))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_VOLUME_LIGHTS))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_SUPER_DETAIL))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_LIGHTS_IN_MINE))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_MINE_RADIOSITY))->EnableWindow(enable);
		((CEdit *)GetDlgItem(IDC_MULTIPLIER))->EnableWindow(enable);
		((CEdit *)GetDlgItem(IDC_AMBIENCE_RED))->EnableWindow(enable);
		((CEdit *)GetDlgItem(IDC_AMBIENCE_GREEN))->EnableWindow(enable);
		((CEdit *)GetDlgItem(IDC_AMBIENCE_BLUE))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_FILL_CORONAS))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_RESET_MULTIPLES))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_CLEAR_LIGHTMAPS))->EnableWindow(enable);
		// fall through

	case 1:
		((CEdit *)GetDlgItem(IDC_ROOM_MULTIPLIER))->EnableWindow(enable);
		((CEdit *)GetDlgItem(IDC_ROOM_AMBIENCE_RED))->EnableWindow(enable);
		((CEdit *)GetDlgItem(IDC_ROOM_AMBIENCE_GREEN))->EnableWindow(enable);
		((CEdit *)GetDlgItem(IDC_ROOM_AMBIENCE_BLUE))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_ROOM_COUNT_LIGHTS))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_ROOM_RADIOSITY))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_ROOM_FILL_CORONAS))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_ROOM_RESET_MULTIPLES))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_ROOM_CLEAR_LIGHTMAPS))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_ROOM_CLEAR_LIGHTMAPS))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_TOUCHES_TERRAIN))->EnableWindow(enable);
		// fall through

	case 2:
		((CEdit *)GetDlgItem(IDC_FACE_MULTIPLIER))->EnableWindow(enable);
		((CButton *)GetDlgItem(IDC_FACE_CORONA))->EnableWindow(enable);
		break;

	default:
		Int3();
		break;
	}
}

void CLightingDialog::UpdateDialog() 
{
	char str[100];
	int which=0;

	// Do nothing if level is not open
	if (theApp.m_pLevelWnd == NULL)
	{
//		EnableTabControls(this->m_hWnd,false,IDC_LIGHTMAP_SPACING,-1);
		EnableControls(false,which);
		return;
	}
	else
	{
//		EnableTabControls(this->m_hWnd,true,IDC_LIGHTMAP_SPACING,-1);
		EnableControls(true,which);
		if (Curroomp == NULL)
		{
			which = 1;
//			EnableTabControls(this->m_hWnd,true,IDC_LIGHTMAP_SPACING,-1);
			EnableControls(false,which);
			if (Curface == -1)
				which = 2;
//			EnableTabControls(this->m_hWnd,true,IDC_LIGHTMAP_SPACING,-1);
			EnableControls(false,which);
		}
	}

	// Update all the controls
	sprintf (str,"%d",rad_MaxStep);
	SetDlgItemText(IDC_ITERATIONS,str);

	sprintf (str,"%d",LightSpacing);
	SetDlgItemText(IDC_LIGHTMAP_SPACING,str);

	sprintf (str,"%f",Ignore_limit);
	SetDlgItemText(IDC_IGNORE_LIMIT,str);

	CheckDlgButton (IDC_SUPER_DETAIL,Shoot_from_patch?0:1);
	CheckDlgButton (IDC_COMBINE_FACES,AllowCombining);
	CheckDlgButton (IDC_VOLUME_LIGHTS,UseVolumeLights);

	sprintf (str,"%f",Room_multiplier[Curroomp-Rooms]);
	SetDlgItemText(IDC_ROOM_MULTIPLIER,str);

	sprintf (str,"%f",Room_ambience_r[Curroomp-Rooms]);
	SetDlgItemText(IDC_ROOM_AMBIENCE_RED,str);

	sprintf (str,"%f",Room_ambience_g[Curroomp-Rooms]);
	SetDlgItemText(IDC_ROOM_AMBIENCE_GREEN,str);

	sprintf (str,"%f",Room_ambience_b[Curroomp-Rooms]);
	SetDlgItemText(IDC_ROOM_AMBIENCE_BLUE,str);

	CheckDlgButton (IDC_TOUCHES_TERRAIN,Curroomp->flags & RF_TOUCHES_TERRAIN);

	// Do Global multiplier
	sprintf (str,"%f",GlobalMultiplier);
	SetDlgItemText(IDC_MULTIPLIER,str);

	sprintf (str,"%.2f",Ambient_red);
	SetDlgItemText(IDC_AMBIENCE_RED,str);

	sprintf (str,"%.2f",Ambient_green);
	SetDlgItemText(IDC_AMBIENCE_GREEN,str);

	sprintf (str,"%.2f",Ambient_blue);
	SetDlgItemText(IDC_AMBIENCE_BLUE,str);

	sprintf (str,"%.2f",(Curroomp->faces[Curface].light_multiple)/4);
	SetDlgItemText(IDC_FACE_MULTIPLIER,str);

	CheckDlgButton(IDC_FACE_CORONA,(Curroomp->faces[Curface].flags & FF_CORONA)?1:0);
}

void CLightingDialog::OnKillfocusLightmapSpacing() 
{
	char str[20];
	int val;
		
	GetDlgItemText(IDC_LIGHTMAP_SPACING,str,20);

	val=atoi (str);

	if (val<1)
		val=1;
	if (val>20)
		val=20;

	LightSpacing=val;

	UpdateDialog();
}

void CLightingDialog::OnKillfocusIterations() 
{
	char str[20];

	GetDlgItemText(IDC_ITERATIONS,str,20);

	rad_MaxStep=atoi (str);

	if (rad_MaxStep<1)
		rad_MaxStep=1;

	UpdateDialog();
}

void CLightingDialog::OnKillfocusIgnoreLimit() 
{
	char str[20];
	float val;

	GetDlgItemText(IDC_IGNORE_LIMIT,str,20);

	val=atof (str);

	if (val<0 || val>.5)
	{
		UpdateDialog ();
		return;
	}

	Ignore_limit=val;

	UpdateDialog();
}

void CLightingDialog::OnCombineFaces() 
{
	int c=IsDlgButtonChecked(IDC_COMBINE_FACES);

	if (c)
		AllowCombining=1;
	else
		AllowCombining=0;
}

void CLightingDialog::OnSuperDetail() 
{
	int c=IsDlgButtonChecked(IDC_SUPER_DETAIL);

	if (c)
		Shoot_from_patch=0;
	else
		Shoot_from_patch=1;
}

void CLightingDialog::OnVolumeLights() 
{
	int c=IsDlgButtonChecked(IDC_VOLUME_LIGHTS);

	if (c)
		UseVolumeLights=1;
	else
		UseVolumeLights=0;
}

void CLightingDialog::OnLightsInMine() 
{
	int lightcount,i,t;
	char str[1000];

	for (lightcount=0,i=0;i<MAX_ROOMS;i++)
	{
		if (Rooms[i].used && !(Rooms[i].flags & RF_EXTERNAL))
		{
			for (t=0;t<Rooms[i].num_faces;t++)
			{
				int texnum=Rooms[i].faces[t].tmap;
				if (GameTextures[texnum].r>0 || GameTextures[texnum].g>0 || GameTextures[texnum].b>0)
					lightcount++;
			}
		}
	}

	sprintf (str,"There are %d lights present in the mine.  Do you wish for me to set the 'Iterations' control to match this number?",lightcount);
	int answer=MessageBox (str,"Light Question",MB_YESNO);
	if (answer==IDNO)
		return;

	sprintf (str,"%d",lightcount);
	SetDlgItemText(IDC_ITERATIONS,str);

	rad_MaxStep=lightcount;
}

void CLightingDialog::OnMineRadiosity() 
{
	int answer=MessageBox ("Are you sure you wish to calculate lighting on the indoor mine?","Light Question",MB_YESNO);
	if (answer==IDNO)
		return;

	MessageBox ("I will now calculate radiosity for the indoor engine.  This might take a few minutes to setup, but afterwards you will be able to cancel if you wish.","Message",MB_OK);

	DoRadiosityForRooms();

	World_changed=1;
}


void CLightingDialog::OnOK() 
{
	// TODO: Add extra validation here
	// Do nothing!
}

#include "object_lighting.h"

void CLightingDialog::OnClearLightmaps() 
{
	// TODO: Add your control notification handler code here
	if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to undo all lighting in the mine?") != IDYES)
		return;

	ClearAllRoomLightmaps(0);
	ClearAllObjectLightmaps(0);

	World_changed = 1;

	OutrageMessageBox ("Lightmaps cleared! Your level is now fullbright.");
}

void CLightingDialog::OnResetMultiples() 
{
	// TODO: Add your control notification handler code here
	int answer=MessageBox ("Are you sure you wish to reset all light multiples in the mine?","Light Question",MB_YESNO);
	if (answer==IDNO)
		return;

	int i;

	for (i=0;i<=Highest_room_index;i++)
	{
		if (Rooms[i].used)
		{
			for (int t=0;t<Rooms[i].num_faces;t++)
			{
				Rooms[i].faces[t].light_multiple=4;
			}
		}
	}

	World_changed = 1;

	OutrageMessageBox ("Light multiples reset!");
}

void CLightingDialog::OnKillfocusAmbienceBlue() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;

	GetDlgItemText(IDC_AMBIENCE_BLUE,str,20);

	val=atof (str);

	if (val<0 || val>1.0)
	{
		OutrageMessageBox ("Invalid value.  Must be in the range 0 to 1.0!");
		return;
	}

	Ambient_blue=val;

	UpdateDialog();
}

void CLightingDialog::OnKillfocusAmbienceGreen() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;

	GetDlgItemText(IDC_AMBIENCE_GREEN,str,20);

	val=atof (str);

	if (val<0 || val>1.0)
	{
		OutrageMessageBox ("Invalid value.  Must be in the range 0 to 1.0!");
		return;
	}

	Ambient_green=val;

	UpdateDialog();
}

void CLightingDialog::OnKillfocusAmbienceRed() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float val;
		
	GetDlgItemText(IDC_AMBIENCE_RED,str,20);

	val=atof (str);

	if (val<0 || val>1.0)
	{
		OutrageMessageBox ("Invalid value.  Must be in the range 0 to 1.0!");
		return;
	}

	Ambient_red=val;

	UpdateDialog();
}

void CLightingDialog::OnFillCoronas() 
{
	// TODO: Add your control notification handler code here
	int fill,answer,only_cur_texture;

	fill=IsDlgButtonChecked(IDC_FILL_CORONAS);
	
	if (fill)
		answer=MessageBox ("Are you sure you wish to make all lights in the mine have coronas?","Light Question",MB_YESNO);
	else
		answer=MessageBox ("Are you sure you wish to remove all coronas in this level?","Light Question",MB_YESNO);
		
	if (answer==IDNO)
	{
		CheckDlgButton(IDC_FILL_CORONAS,!fill);
		return;
	}

	only_cur_texture=MessageBox ("Do you wish to do this operation only on the faces that have the same texture as the currently selected face?","Light Question",MB_YESNO);

	int i,t;

	for (i=0;i<=Highest_room_index;i++)
	{
		if (!Rooms[i].used)
			continue;
		if ((Rooms[i].flags & RF_EXTERNAL))
			continue;

		for (t=0;t<Rooms[i].num_faces;t++)
		{
			if (only_cur_texture==IDYES)
			{
				if (Rooms[i].faces[t].tmap!=Curroomp->faces[Curface].tmap)
					continue;
			}

			if (fill)
				Rooms[i].faces[t].flags|=FF_CORONA;
			else
				Rooms[i].faces[t].flags&=~FF_CORONA;

		}
	}
	
	World_changed=1;

	OutrageMessageBox ("Operation complete!");

	UpdateDialog();
}

void CLightingDialog::OnKillfocusMultiplier() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;
		
	GetDlgItemText(IDC_MULTIPLIER,str,20);

	aval=atof (str);
	if (aval<0)
		aval=0;

	GlobalMultiplier=aval;

	UpdateDialog();
}


void CLightingDialog::OnRoomClearLightmaps() 
{
	// TODO: Add your control notification handler code here
	if (OutrageMessageBox(MBOX_YESNO,"Are you sure you want to undo all lighting in the current room?") != IDYES)
		return;

	ClearRoomLightmaps (Curroomp-Rooms);
	for (int i=0;i<=Highest_object_index;i++)
	{
		if (Objects[i].type!=OBJ_NONE && (Objects[i].roomnum==Curroomp-Rooms))
			ClearObjectLightmaps(&Objects[i]);
	}

	World_changed = 1;

	OutrageMessageBox ("Lightmaps cleared! The current room is now fullbright.");
}

void CLightingDialog::OnKillfocusRoomAmbienceBlue() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;
		
	GetDlgItemText(IDC_ROOM_AMBIENCE_BLUE,str,20);

	aval=atof (str);
	if (aval<0)
		aval=0;

	Room_ambience_b[Curroomp-Rooms]=aval;

	UpdateDialog();
}

void CLightingDialog::OnKillfocusRoomAmbienceGreen() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;

	GetDlgItemText(IDC_ROOM_AMBIENCE_GREEN,str,20);

	aval=atof (str);
	if (aval<0)
		aval=0;

	Room_ambience_g[Curroomp-Rooms]=aval;

	UpdateDialog();
}

void CLightingDialog::OnKillfocusRoomAmbienceRed() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;
		
	GetDlgItemText(IDC_ROOM_AMBIENCE_RED,str,20);

	aval=atof (str);
	if (aval<0)
		aval=0;

	Room_ambience_r[Curroomp-Rooms]=aval;

	UpdateDialog();
}

void CLightingDialog::OnRoomFillCoronas() 
{
	// TODO: Add your control notification handler code here
	int fill,answer,only_cur_texture;

	fill=IsDlgButtonChecked(IDC_ROOM_FILL_CORONAS);
	
	if (fill)
		answer=MessageBox ("Are you sure you wish to make all lights in the current room have coronas?","Light Question",MB_YESNO);
	else
		answer=MessageBox ("Are you sure you wish to remove all coronas in the current room?","Light Question",MB_YESNO);

	if (answer==IDNO)
	{
		CheckDlgButton(IDC_ROOM_FILL_CORONAS,!fill);
		return;
	}

	only_cur_texture=MessageBox ("Do you wish to do this operation only on the faces that have the same texture as the currently selected face?","Light Question",MB_YESNO);

	int i;

	for (i=0;i<Curroomp->num_faces;i++)
	{
		if (only_cur_texture==IDYES)
		{
			if (Curroomp->faces[i].tmap!=Curroomp->faces[Curface].tmap)
				continue;
		}

		if (fill)
			Curroomp->faces[i].flags|=FF_CORONA;
		else
			Curroomp->faces[i].flags&=~FF_CORONA;
	}

	World_changed=1;

	OutrageMessageBox ("Operation complete!");

	UpdateDialog();
}

void CLightingDialog::OnKillfocusRoomMultiplier() 
{
	// TODO: Add your control notification handler code here
	char str[20];
	float aval;

	GetDlgItemText(IDC_ROOM_MULTIPLIER,str,20);

	aval=atof (str);
	if (aval<0)
		aval=0;

	Room_multiplier[Curroomp-Rooms]=aval;

	UpdateDialog();
}

void CLightingDialog::OnRoomRadiosity() 
{
	// TODO: Add your control notification handler code here
	MessageBox ("I will now calculate radiosity for the current room.  You may prematurely break out by pressing any key during the calculations","Message",MB_OK);

	DoRadiosityForCurrentRoom(Curroomp);

	World_changed=1;
}

void CLightingDialog::OnRoomResetMultiples() 
{
	// TODO: Add your control notification handler code here
	int i;
	int answer=MessageBox ("Are you sure you wish to reset all light multiples in the current room?","Light Question",MB_YESNO);
	if (answer==IDNO)
		return;

	if (Curroomp->used)
	{
		for (i=0;i<Curroomp->num_faces;i++)
			Curroomp->faces[i].light_multiple=4;

		World_changed=1;

		OutrageMessageBox ("Light multiples reset!");
	}
}

void CLightingDialog::OnRoomCountLights() 
{
	// TODO: Add your control notification handler code here
	int lightcount=0;
	int i;
	char str[1000];

	if (Curroomp->used && !(Curroomp->flags & RF_EXTERNAL))
	{
		for (i=0;i<Curroomp->num_faces;i++)
		{
			int texnum=Curroomp->faces[i].tmap;
			if (GameTextures[texnum].r>0 || GameTextures[texnum].g>0 || GameTextures[texnum].b>0)
				lightcount++;
		}
	}

	sprintf (str,"There are %d lights present in the current room.  Do you wish for me to set the 'Iterations' control to match this number?",lightcount);
	int answer=MessageBox (str,"Light Question",MB_YESNO);
	if (answer==IDNO)
		return;

	sprintf (str,"%d",lightcount);
	SetDlgItemText(IDC_ITERATIONS,str);

	rad_MaxStep=lightcount;
}

void CLightingDialog::OnFaceCorona() 
{
	// TODO: Add your control notification handler code here
	if (Curroomp==NULL)
		return;

	int c=IsDlgButtonChecked(IDC_FACE_CORONA);

	if (c)
		Curroomp->faces[Curface].flags|=FF_CORONA;
	else
		Curroomp->faces[Curface].flags&=~FF_CORONA;

	World_changed=1;
}

void CLightingDialog::OnKillfocusFaceMultiplier() 
{
	// TODO: Add your control notification handler code here
	if (Curroomp==NULL)
		return;

	int val;

	GetDlgItemInt(IDC_FACE_MULTIPLIER,&val,FALSE);

	if (val<1 || val>63)
	{
		OutrageMessageBox ("Invalid value, setting to one!");
		val=1;
	}

	Curroomp->faces[Curface].light_multiple=(val*4);

	World_changed=1;

	UpdateDialog();
}

void CLightingDialog::OnTouchesTerrain() 
{
	// TODO: Add your control notification handler code here
	if (Curroomp==NULL)
		return;

	int c=IsDlgButtonChecked(IDC_TOUCHES_TERRAIN);

	if (c)
		Curroomp->flags |= RF_TOUCHES_TERRAIN;
	else
		Curroomp->flags &= ~RF_TOUCHES_TERRAIN;

	World_changed = 1;
}

void CLightingDialog::OnTerrainRadiosity() 
{
	// TODO: Add your control notification handler code here
	int answer=MessageBox ("Are you sure you wish to calculate lighting on the terrain?","Light Question",MB_YESNO);
	if (answer==IDNO)
		return;

	MessageBox ("I will now calculate radiosity for the terrain engine.  You may prematurely break out by pressing any key during the calculations","Message",MB_OK);
	DoRadiosityForTerrain();
	World_changed=1;
}

void CLightingDialog::OnLightsOnTerrain() 
{
	// TODO: Add your control notification handler code here
	int lightcount,i,t;
	char str[1000];

	for (lightcount=0,i=0;i<TERRAIN_WIDTH*TERRAIN_DEPTH;i++)
	{
		if (Terrain_seg[i].flags & TF_INVISIBLE)
			continue;

		int texnum=Terrain_tex_seg[Terrain_seg[i].texseg_index].tex_index;
		if (GameTextures[texnum].r>0 || GameTextures[texnum].g>0 || GameTextures[texnum].b>0)
			lightcount+=2;
	}

	lightcount+=Terrain_sky.num_satellites;

	for (i=0;i<=Highest_object_index;i++)
	{
		if (! OBJECT_OUTSIDE(&Objects[i]))
				continue;

		if (Objects[i].type!=OBJ_NONE && Objects[i].lighting_render_type==LRT_LIGHTMAPS)
		{
			poly_model *po=&Poly_models[Objects[i].rtype.pobj_info.model_num];

			if (!po->new_style)
				continue;
			
			for (t=0;t<po->n_models;t++)
			{
				bsp_info *sm=&po->submodel[t];
		
				if (IsNonRenderableSubmodel (po,t))
					continue;

				for (int j=0;j<sm->num_faces;j++)
				{
					if (sm->faces[j].texnum==-1)
						continue;

						int texnum=po->textures[sm->faces[j].texnum];
						if (GameTextures[texnum].r>0 || GameTextures[texnum].g>0 || GameTextures[texnum].b>0)
							lightcount++;
				}
			}
		}
	}

	// Count rooms
	for (i=0;i<MAX_ROOMS;i++)
	{
		if (Rooms[i].used && (Rooms[i].flags & RF_EXTERNAL))
		{
			for (t=0;t<Rooms[i].num_faces;t++)
			{
				int texnum=Rooms[i].faces[t].tmap;
				if (GameTextures[texnum].r>0 || GameTextures[texnum].g>0 || GameTextures[texnum].b>0)
					lightcount++;
			}
		}
	}
		
	sprintf (str,"There are %d lights present on the terrain.  Do you wish for me to set the 'light iterations' control to match this number?",lightcount);
	int answer=MessageBox (str,"Light Question",MB_YESNO);
	if (answer==IDNO)
		return;

	CEdit *ebox;
	
	ebox=(CEdit *) GetDlgItem (IDC_ITERATIONS);
	sprintf (str,"%d",lightcount);
	ebox->SetWindowText (str);

	rad_MaxStep=lightcount;
}

void DoTerrainDynamicTable();
void CLightingDialog::OnDynamicTerrainLight() 
{
	// TODO: Add your control notification handler code here
	if ((MessageBox("Are you sure you wish to do dynamic lighting on the terrain.  The reason I ask is that this procedure takes a long time, and I don't want to upset you.","Question",MB_YESNO))==IDYES)
		DoTerrainDynamicTable();
}
