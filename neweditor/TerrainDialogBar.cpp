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
 

#include "stdafx.h"
#include "neweditor.h"
#include "TerrainDialogBar.h"
#include "terrain.h"
#include "ned_Util.h"
#include "EditLineDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int Current_cell = 0;

/////////////////////////////////////////////////////////////////////////////
// CTerrainDialogBar dialog
CTerrainDialogBar *dlgTerrainDialogBar = NULL;

CTerrainDialogBar::CTerrainDialogBar()
{
	Current_satellite = 0;
	//{{AFX_DATA_INIT(CTerrainDialogBar)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTerrainDialogBar::DoDataExchange(CDataExchange* pDX)
{
	CDialogBar::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTerrainDialogBar)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTerrainDialogBar, CDialogBar)
	//{{AFX_MSG_MAP(CTerrainDialogBar)
	ON_UPDATE_COMMAND_UI(IDC_TERRAIN_OCCLUSION,OnControlUpdate)
	ON_BN_CLICKED(IDC_TERRAIN_OCCLUSION, OnTerrainOcclusion)
	ON_BN_CLICKED(IDC_SKY_CONTRACT, OnSkyContract)
	ON_BN_CLICKED(IDC_SKY_EXPAND, OnSkyExpand)
	ON_BN_CLICKED(IDC_SKY_STARS, OnSkyStars)
	ON_BN_CLICKED(IDC_SKY_SATELLITES, OnSkySatellites)
	ON_BN_CLICKED(IDC_SKY_PREV_SAT, OnSkyPrevSat)
	ON_BN_CLICKED(IDC_SKY_NEXT_SAT, OnSkyNextSat)
	ON_BN_CLICKED(IDC_SKY_FOG, OnSkyFog)
	ON_EN_KILLFOCUS(IDC_SKY_DAMAGE, OnKillfocusSkyDamage)
	ON_BN_CLICKED(IDC_IMPORT_PCX_MAP, OnImportPcxMap)
	ON_BN_CLICKED(IDC_EXPORT_PCX_MAP, OnExportPcxMap)
	ON_BN_CLICKED(IDC_SKY_TEXTURED, OnSkyTextured)
	ON_BN_CLICKED(IDC_SKY_ROTATE_SKY, OnSkyRotateSky)
	ON_BN_CLICKED(IDC_SKY_ROTATE_STARS, OnSkyRotateStars)
	ON_EN_KILLFOCUS(IDC_SKY_ROT_DEG, OnKillfocusSkyRotDeg)
	ON_EN_KILLFOCUS(IDC_SKY_FOG_SCALAR, OnKillfocusSkyFogScalar)
	ON_BN_CLICKED(IDC_SKY_MORE_SATS, OnSkyMoreSats)
	ON_BN_CLICKED(IDC_SKY_LESS_SATS, OnSkyLessSats)
	ON_BN_CLICKED(IDC_SKY_HALO, OnSkyHalo)
	ON_BN_CLICKED(IDC_SKY_ATMOSPHERE, OnSkyAtmosphere)
	ON_BN_CLICKED(IDC_SKY_SAT_MOVE_UP, OnSkySatMoveUp)
	ON_BN_CLICKED(IDC_SKY_SAT_MOVE_RIGHT, OnSkySatMoveRight)
	ON_BN_CLICKED(IDC_SKY_SAT_MOVE_LEFT, OnSkySatMoveLeft)
	ON_BN_CLICKED(IDC_SKY_SAT_MOVE_DOWN, OnSkySatMoveDown)
	ON_BN_CLICKED(IDC_TOGGLE_CELL_VISIBLE, OnToggleCellVisible)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_SKY_MOVE_SAT_CLOSER, OnSkyMoveSatCloser)
	ON_BN_CLICKED(IDC_SKY_MOVE_SAT_FARTHER, OnSkyMoveSatFarther)
	ON_BN_CLICKED(IDC_TERRAIN_SELECT, OnTerrainSelect)
	ON_UPDATE_COMMAND_UI(IDC_IMPORT_PCX_MAP,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_EXPORT_PCX_MAP,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_MORE_SATS,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_LESS_SATS,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_PREV_SAT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_NEXT_SAT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_SAT_MOVE_UP,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_SAT_MOVE_RIGHT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_SAT_MOVE_LEFT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_SAT_MOVE_DOWN,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_CONTRACT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_EXPAND,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_SAT_MOVE_DOWN,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_SAT_MOVE_DOWN,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_MOVE_SAT_CLOSER,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_MOVE_SAT_FARTHER,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_TERRAIN_SELECT,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_TOGGLE_CELL_VISIBLE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_FORCE_CELL_VISIBLE,OnControlUpdate)
	ON_BN_CLICKED(IDC_FORCE_CELL_VISIBLE, OnForceCellVisible)
	ON_UPDATE_COMMAND_UI(IDC_SKY_TEXTURED,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_HALO,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_ATMOSPHERE,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_STARS,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_SATELLITES,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_FOG,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_ROTATE_SKY,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_ROTATE_STARS,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_ROT_DEG,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_FOG_SCALAR,OnControlUpdate)
	ON_UPDATE_COMMAND_UI(IDC_SKY_DAMAGE,OnControlUpdate)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_INITDIALOG, OnInitDialog )
END_MESSAGE_MAP()

void CTerrainDialogBar::InitBar()
{
	// TODO: Add extra initialization here
	HINSTANCE hinst;
//	HICON hicon;

	hinst = AfxGetInstanceHandle();
	// Associate bitmaps with the buttons
/*
	hicon = (HICON)::LoadImage(hinst,MAKEINTRESOURCE(IDI_ROOM_EXPAND),IMAGE_ICON,0,0,LR_DEFAULTCOLOR);
	((CButton *)GetDlgItem(IDC_ROOM_EXPAND))->SetIcon(hicon);
*/
}

/////////////////////////////////////////////////////////////////////////////
// CTerrainDialogBar message handlers

LONG CTerrainDialogBar::OnInitDialog ( UINT wParam, LONG lParam)
{
	dlgTerrainDialogBar = this;

	if ( !HandleInitDialog(wParam, lParam) || !UpdateData(FALSE))
	{
		TRACE0("Warning: UpdateData failed during dialog init.\n");
		return FALSE;
	}

	InitBar();
	Current_satellite = 0;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// enable controls
void CTerrainDialogBar::OnControlUpdate(CCmdUI* pCmdUI)
{
	bool bEnable = false;

	// All controls for the terrain bar require an open level
	if (theApp.m_pLevelWnd != NULL)
		bEnable = true;

	pCmdUI->Enable(bEnable);
}


void CTerrainDialogBar::UpdateDialog(void) 
{
	char str[20];
	CEdit *ebox;

	ebox=(CEdit *) GetDlgItem (IDC_SKY_DAMAGE);
	sprintf (str,"%f",Terrain_sky.damage_per_second);
	ebox->SetWindowText (str);

	ebox=(CEdit *) GetDlgItem (IDC_SKY_FOG_SCALAR);
	sprintf (str,"%f",Terrain_sky.fog_scalar);
	ebox->SetWindowText (str);
	
	ebox=(CEdit *) GetDlgItem (IDC_SKY_QUANT_SAT);
	sprintf (str,"%d",Terrain_sky.num_satellites);
	ebox->SetWindowText (str);

	ebox=(CEdit *) GetDlgItem (IDC_SKY_NUM_SAT);
	sprintf (str,"%d",Current_satellite);
	ebox->SetWindowText (str);

	ebox=(CEdit *) GetDlgItem (IDC_SKY_ROT_DEG);
	sprintf (str,"%f",Terrain_sky.rotate_rate);
	ebox->SetWindowText (str);

	CheckDlgButton (IDC_SKY_TEXTURED,Terrain_sky.textured?1:0);
	CheckDlgButton (IDC_SKY_STARS,Terrain_sky.flags & TF_STARS?1:0);
	CheckDlgButton (IDC_SKY_ROTATE_STARS,Terrain_sky.flags & TF_ROTATE_STARS?1:0);
	CheckDlgButton (IDC_SKY_ROTATE_SKY,Terrain_sky.flags & TF_ROTATE_SKY?1:0);
	CheckDlgButton (IDC_SKY_SATELLITES,Terrain_sky.flags & TF_SATELLITES?1:0);
	CheckDlgButton (IDC_SKY_FOG,Terrain_sky.flags & TF_FOG?1:0);
	CheckDlgButton (IDC_SKY_HALO,Terrain_sky.satellite_flags[Current_satellite] & TSF_HALO?1:0);
	CheckDlgButton (IDC_SKY_ATMOSPHERE,Terrain_sky.satellite_flags[Current_satellite] & TSF_ATMOSPHERE?1:0);
}


#define FILL_COLOR		1
static ubyte *Terrain_heights,*Terrain_fill;

#define PUSH_FILL(x) {fill_stack[stack_count]=x; Terrain_fill[x]=1; stack_count++; ASSERT (stack_count<65536);}
#define POP_FILL() {stack_count--; cell=fill_stack[stack_count];}

void FillTerrainHeights (int cell)
{
	terrain_segment *tseg=&Terrain_seg[cell];
	ushort fill_stack[65536];
	int stack_count=0;

	ASSERT (cell>=0 && cell<TERRAIN_WIDTH*TERRAIN_DEPTH);

	memset (Terrain_fill,0,TERRAIN_WIDTH*TERRAIN_DEPTH);
	
	PUSH_FILL(cell);

	while (stack_count>0)
	{
		POP_FILL();
		if (Terrain_heights[cell]==0)
		{
			int x=cell%256;
			int z=cell/256;

			Terrain_heights[cell]=FILL_COLOR;

			if (x!=0 && Terrain_fill[cell-1]==0)
				PUSH_FILL(cell-1);
			if (x!=TERRAIN_WIDTH-1 && Terrain_fill[cell+1]==0)
				PUSH_FILL(cell+1);
			if (z!=0 && Terrain_fill[cell-TERRAIN_WIDTH]==0)
				PUSH_FILL(cell-TERRAIN_WIDTH);
			if (z!=TERRAIN_DEPTH-1 && Terrain_fill[cell+TERRAIN_WIDTH]==0)
				PUSH_FILL(cell+TERRAIN_DEPTH);
		}
	}

}


void CTerrainDialogBar::OnTerrainOcclusion() 
{
	// TODO: Add your control notification handler code here
	if (Terrain_occlusion_checksum==(Terrain_checksum+1))
	{
		OutrageMessageBox ("The occlusion map is already calculated for this terrain.");
		//return;
	}

	if ((MessageBox("Are you sure you wish to calculate terrain occlusion?","Question",MB_YESNO))==IDNO)
		return;

	mprintf ((0,"Now doing occlusion tests...\n"));
	
	int count=0;
	int occlude_count=0;
	ubyte *touch_buffer[256];

	for (int i=0;i<256;i++)
	{
		memset (Terrain_occlusion_map[i],0,32);
		touch_buffer[i]=(ubyte *)mem_malloc(256);
		ASSERT (touch_buffer[i]);
		memset (touch_buffer[i],255,256);
	}


	ubyte *save_buffer;

	// Build a height map so we can flood fill
	Terrain_heights=(ubyte *)mem_malloc (TERRAIN_WIDTH*TERRAIN_DEPTH);
	Terrain_fill=(ubyte *)mem_malloc (TERRAIN_WIDTH*TERRAIN_DEPTH);
	save_buffer=(ubyte *)mem_malloc (TERRAIN_WIDTH*TERRAIN_DEPTH);

	ASSERT (Terrain_heights);
	ASSERT (save_buffer);
	ASSERT (Terrain_fill);

	memset(save_buffer,0,TERRAIN_WIDTH*TERRAIN_DEPTH);
	memset(Terrain_fill,0,TERRAIN_WIDTH*TERRAIN_DEPTH);

	for (int i=0;i<TERRAIN_WIDTH*TERRAIN_DEPTH;i++)
	{
		if (Terrain_seg[i].ypos==255)
			save_buffer[i]=255;
	}

	for (int i=0;i<OCCLUSION_SIZE;i++)
	{
		for (int t=0;t<OCCLUSION_SIZE;t++)
		{
			int src_occlusion_index=i*OCCLUSION_SIZE+t;
			int start_x=t*OCCLUSION_SIZE;
			int start_z=i*OCCLUSION_SIZE;
			int k,j;
			int dest_x,dest_z;
			
			for (dest_z=0;dest_z<OCCLUSION_SIZE;dest_z++)
			{
				for (dest_x=0;dest_x<OCCLUSION_SIZE;dest_x++)
				{
					int end_z=dest_z*OCCLUSION_SIZE;
					int end_x=dest_x*OCCLUSION_SIZE;
					int dest_occlusion_index=dest_z*OCCLUSION_SIZE+dest_x;

					mprintf_at((2,5,0,"Count=%7d",count++));

					if (dest_occlusion_index==src_occlusion_index)
					{
						// This is us!
						int occ_byte=dest_occlusion_index/8;
						int occ_bit=dest_occlusion_index % 8;

						Terrain_occlusion_map[src_occlusion_index][occ_byte]|=(1<<occ_bit);
						touch_buffer[src_occlusion_index][dest_occlusion_index]=1;
						continue;
					}

					if (touch_buffer[dest_occlusion_index][src_occlusion_index]!=255)
					{
						// this one has already been computed
						int hit=touch_buffer[dest_occlusion_index][src_occlusion_index];

						int dest_occ_byte=dest_occlusion_index/8;
						int dest_occ_bit=dest_occlusion_index % 8;
						
						if (hit)
							Terrain_occlusion_map[src_occlusion_index][dest_occ_byte]|=(1<<dest_occ_bit);
						else
							Terrain_occlusion_map[src_occlusion_index][dest_occ_byte]&=~(1<<dest_occ_bit);

						touch_buffer[src_occlusion_index][dest_occlusion_index]=hit;
						continue;
					}


					// See if this ray is close enough
					vector src_vec,dest_vec;

					src_vec.x=start_x;
					src_vec.y=0;
					src_vec.z=start_z;

					dest_vec.x=end_x;
					dest_vec.y=0;
					dest_vec.z=end_z;

					//vector subvec=dest_vec-src_vec;
					
					// Set our heights to max
					memset (Terrain_heights,255,TERRAIN_WIDTH*TERRAIN_DEPTH);

					// Now carve out a path from the src to the dest
					int x_major=1;
					if (abs(end_z-start_z)>abs(end_x-start_x))
						x_major=0;

					// We're iterating over the x axis
					if (x_major)
					{
						int x_add=1;
						int limit=end_x-start_x;
						if (limit<0)
						{
							x_add=-1;
							limit=-limit;
						}

						int cur_x=start_x;
						float cur_z=start_z;
						float delta_z=(float)(end_z-start_z)/(float)limit;

						for (j=0;j<limit;j++,cur_x+=x_add,cur_z+=delta_z)
						{
							for (int z=0;z<OCCLUSION_SIZE;z++)
							{
								for (int x=0;x<OCCLUSION_SIZE;x++)
								{	
									int index=(cur_z+z)*TERRAIN_WIDTH;
									index+=(cur_x+x);

									Terrain_heights[index]=0;
								}
							}
						}
					}
					else	// iterate over z axis
					{
						int z_add=1;
						int limit=end_z-start_z;
						if (limit<0)
						{
							z_add=-1;
							limit=-limit;
						}

						int cur_z=start_z;
						float cur_x=start_x;
						float delta_x=(float)(end_x-start_x)/(float)limit;

						for (j=0;j<limit;j++,cur_z+=z_add,cur_x+=delta_x)
						{
							for (int z=0;z<OCCLUSION_SIZE;z++)
							{
								for (int x=0;x<OCCLUSION_SIZE;x++)
								{	
									int index=(cur_z+z)*TERRAIN_WIDTH;
									index+=(cur_x+x);

									Terrain_heights[index]=0;
								}
							}
						}
					}

					// Ok, we have a carved path
					// Now put back in all the max values from our original map
					for (k=0;k<TERRAIN_WIDTH*TERRAIN_DEPTH;k++)
					{
						if (save_buffer[k]==255)
							Terrain_heights[k]=255;
					}

					// Set our src block to zero
					for (k=0;k<OCCLUSION_SIZE;k++)
					{
						for (j=0;j<OCCLUSION_SIZE;j++)
						{
							Terrain_heights[((start_z+k)*TERRAIN_WIDTH)+start_x+j]=0;
						}
					}

					// Fill that valley
					FillTerrainHeights ((start_z*TERRAIN_WIDTH)+start_x);

					// Check to see if we hit our destination
					int seen=0;
					for (int z=0;z<OCCLUSION_SIZE && !seen;z++)
					{
						for (int x=0;x<OCCLUSION_SIZE && !seen;x++)
						{
							int index=(end_z+z)*TERRAIN_WIDTH;
							index+=(end_x+x);

							if (Terrain_heights[index]==1)
								seen=1;
						}
					}

					touch_buffer[src_occlusion_index][dest_occlusion_index]=seen;
					if (seen)
					{
						int occ_byte=dest_occlusion_index/8;
						int occ_bit=dest_occlusion_index%8;
						Terrain_occlusion_map[src_occlusion_index][occ_byte]|=(1<<occ_bit);
					}
					else
					{
						int occ_byte=dest_occlusion_index/8;
						int occ_bit=dest_occlusion_index%8;
						Terrain_occlusion_map[src_occlusion_index][occ_byte]&=~(1<<occ_bit);

						occlude_count++;
					}
				}
			}
		}
	}

	mem_free (Terrain_heights);
	mem_free (Terrain_fill);
	mem_free (save_buffer);

	for (int i=0;i<256;i++)
		mem_free (touch_buffer[i]);
		
	Terrain_occlusion_checksum=Terrain_checksum+1;

	mprintf ((0,"%d cells were occluded.\n",occlude_count));
	OutrageMessageBox ("Occlusion checking complete. Remember to save!");
}

void CTerrainDialogBar::OnSkyContract() 
{
	// TODO: Add your control notification handler code here
	if (Terrain_sky.radius>500)
	{

		SetupSky (Terrain_sky.radius-500,Terrain_sky.flags);
		World_changed=1;
	}
}

void CTerrainDialogBar::OnSkyExpand() 
{
	// TODO: Add your control notification handler code here
	SetupSky (Terrain_sky.radius+500,Terrain_sky.flags);
	World_changed=1;
}

void CTerrainDialogBar::OnSkyStars() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_SKY_STARS);

	if (c)
		Terrain_sky.flags|=TF_STARS;
	else
		Terrain_sky.flags&=~TF_STARS;

	World_changed=1;
}

void CTerrainDialogBar::OnSkySatellites() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_SKY_SATELLITES);

	if (c)
		Terrain_sky.flags|=TF_SATELLITES;
	else
		Terrain_sky.flags&=~TF_SATELLITES;

	World_changed=1;
}

void CTerrainDialogBar::OnSkyPrevSat() 
{
	// TODO: Add your control notification handler code here
	Current_satellite--;
	if (Current_satellite<0)
		Current_satellite=4;
	
	UpdateDialog();
}

void CTerrainDialogBar::OnSkyNextSat() 
{
	// TODO: Add your control notification handler code here
	Current_satellite++;

	Current_satellite%=5;

	UpdateDialog();
}

void CTerrainDialogBar::OnSkyFog() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_SKY_FOG);

	if (c)
		Terrain_sky.flags|=TF_FOG;
	else
		Terrain_sky.flags&=~TF_FOG;

	World_changed=1;
}

void CTerrainDialogBar::OnKillfocusSkyDamage() 
{
	// TODO: Add your control notification handler code here
	CEdit *ebox;
	char str[20];
	float err;
	
	ebox=(CEdit *) GetDlgItem (IDC_SKY_DAMAGE);
	ebox->GetWindowText (str,20);

	err = atof (str);
	if (err<0)
		err=0;
	if (err>200)
		err=200;

	Terrain_sky.damage_per_second=err;

	World_changed=1;
	UpdateDialog();
}

void CTerrainDialogBar::OnImportPcxMap() 
{
	// TODO: Add your control notification handler code here
	char name[_MAX_PATH];
	char ext[10];
	char path[_MAX_PATH*2];

	static char szFilter[] = "PCX files (*.pcx)|*.pcx||";
	CString SelectedFile;
	CString DefaultPath;

	DefaultPath = theApp.GetProfileString("Defaults","TerrainDir","");
	CFileDialog *fd;
	fd = new CFileDialog(true,".pcx",NULL,OFN_FILEMUSTEXIST|OFN_HIDEREADONLY,szFilter);
	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();
	delete fd;

//	Okay, we selected a file. Lets do what needs to be done here.
	LoadPCXTerrain(SelectedFile.GetBuffer(0));

	SplitPath(SelectedFile,path,name,ext);
	WriteProfileString("Defaults","TerrainDir",path);
}

void CTerrainDialogBar::OnExportPcxMap() 
{
	// TODO: Add your control notification handler code here
	char name[_MAX_PATH];
	char ext[10];
	char path[_MAX_PATH*2];

	static char szFilter[] = "PCX files (*.pcx)|*.pcx||";

	CString SelectedFile;
	CString DefaultPath;

	DefaultPath = theApp.GetProfileString("Defaults","TerrainDir","");
	CFileDialog *fd;
	fd = new CFileDialog(true,".pcx",NULL,OFN_HIDEREADONLY,szFilter);
	if (fd->DoModal() == IDCANCEL) { delete fd; return; }

	SelectedFile = fd->GetPathName();
	delete fd;

	CFILE *outfile;

	outfile=(CFILE *)cfopen(SelectedFile.GetBuffer(0),"wb");
	if (!outfile)
	{
		OutrageMessageBox ("Couldn't open that filename to save to!");
		return;
	}

	SplitPath(SelectedFile,path,name,ext);
	WriteProfileString("Defaults","TerrainDir",path);

	// Header info
	cf_WriteByte (outfile,10);
	cf_WriteByte (outfile,5);
	cf_WriteByte (outfile,1);
	cf_WriteByte (outfile,8);

	// Dimensions of image
	cf_WriteShort (outfile,0);
	cf_WriteShort (outfile,0);
	cf_WriteShort (outfile,255);
	cf_WriteShort (outfile,255);

	// Display adapter dimensions (bash to 256)
	cf_WriteShort (outfile,256);
	cf_WriteShort (outfile,256);

	for (int i=0;i<48;i++)
		cf_WriteByte (outfile,0);

	cf_WriteByte (outfile,0);
	cf_WriteByte (outfile,1);
	cf_WriteShort (outfile,256);
	cf_WriteShort (outfile,2);

	for (int i=0;i<58;i++)
		cf_WriteByte (outfile,0);

	for (int i=0;i<TERRAIN_DEPTH;i++)
	{
		for (int t=0;t<TERRAIN_WIDTH;t++)
		{
			ubyte val=Terrain_seg[(((TERRAIN_DEPTH-1)-i)*TERRAIN_WIDTH)+t].ypos;
			ubyte runlen=0xc1;

			cf_WriteByte (outfile,runlen);
			cf_WriteByte (outfile,val);
		}
	}

	cf_WriteByte (outfile,12);
	for (int i=0;i<256;i++)
	{
		cf_WriteByte (outfile,i);
		cf_WriteByte (outfile,i);
		cf_WriteByte (outfile,i);
	}

	cfclose (outfile);
	OutrageMessageBox ("PCX exported!\n");
}


void CTerrainDialogBar::OnSkyTextured() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_SKY_TEXTURED);
	Terrain_sky.textured=c;

	World_changed=1;
}

void CTerrainDialogBar::OnSkyRotateSky() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_SKY_ROTATE_SKY);

	if (c)
		Terrain_sky.flags|=TF_ROTATE_SKY;
	else
		Terrain_sky.flags&=~TF_ROTATE_SKY;

	World_changed=1;
}

void CTerrainDialogBar::OnSkyRotateStars() 
{
	// TODO: Add your control notification handler code here
	int c=IsDlgButtonChecked(IDC_SKY_ROTATE_STARS);

	if (c)
		Terrain_sky.flags|=TF_ROTATE_STARS;
	else
		Terrain_sky.flags&=~TF_ROTATE_STARS;

	World_changed=1;
}

void CTerrainDialogBar::OnKillfocusSkyRotDeg() 
{
	// TODO: Add your control notification handler code here
	CEdit *ebox;
	char str[20];
	float err;
	
	ebox=(CEdit *) GetDlgItem (IDC_SKY_ROT_DEG);
	ebox->GetWindowText (str,20);

	err = atof (str);
	if (err<0)
		err=0;
	if (err>180)
		err=180;

	Terrain_sky.rotate_rate=err;

	World_changed=1;
	UpdateDialog();
}

void CTerrainDialogBar::OnKillfocusSkyFogScalar() 
{
	// TODO: Add your control notification handler code here
	CEdit *ebox;
	char str[20];
	float err;
	
	ebox=(CEdit *) GetDlgItem (IDC_SKY_FOG_SCALAR);
	ebox->GetWindowText (str,20);

	err = atof (str);
	if (err<.2)
		err=.2f;
	if (err>1.0)
		err=1.0;

	Terrain_sky.fog_scalar=err;

	World_changed=1;
	UpdateDialog();
}

void CTerrainDialogBar::OnSkyMoreSats() 
{
	// TODO: Add your control notification handler code here
	if (Terrain_sky.num_satellites<5)
		Terrain_sky.num_satellites++;

	LevelTexIncrementTexture(Terrain_sky.satellite_texture[Terrain_sky.num_satellites-1]);

	World_changed=1;

	UpdateDialog();
}

void CTerrainDialogBar::OnSkyLessSats() 
{
	// TODO: Add your control notification handler code here
	if (Terrain_sky.num_satellites>0)
		Terrain_sky.num_satellites--;

	LevelTexDecrementTexture(Terrain_sky.satellite_texture[Terrain_sky.num_satellites]);

	World_changed=1;

	UpdateDialog();
}

void CTerrainDialogBar::OnSkyHalo() 
{
	// TODO: Add your control notification handler code here
	int n=Current_satellite;
	int c=IsDlgButtonChecked(IDC_SKY_HALO);

	if (c)
		Terrain_sky.satellite_flags[n]|=TSF_HALO;
	else
		Terrain_sky.satellite_flags[n]&=~TSF_HALO;
	
	World_changed=1;
}

void CTerrainDialogBar::OnSkyAtmosphere() 
{
	// TODO: Add your control notification handler code here
	int n=Current_satellite;
	int c=IsDlgButtonChecked(IDC_SKY_ATMOSPHERE);

	if (c)
		Terrain_sky.satellite_flags[n]|=TSF_ATMOSPHERE;
	else
		Terrain_sky.satellite_flags[n]&=~TSF_ATMOSPHERE;
	
	World_changed=1;
}

void CTerrainDialogBar::MoveSat (int pitch,int heading)
{
	int n=Current_satellite;
	matrix rot_matrix,orient;
	vm_AnglesToMatrix (&rot_matrix,pitch,heading,0);	// move up a little
	
	orient=rot_matrix;
	
	vector sat_vec=Terrain_sky.satellite_vectors[n]-Viewer_object->pos;
	float mag=vm_GetMagnitude (&sat_vec);
	vector rot_vec;

	vm_NormalizeVector (&sat_vec);
	vm_MatrixMulVector (&rot_vec,&sat_vec,&orient);
	
	Terrain_sky.satellite_vectors[n]=Viewer_object->pos+(rot_vec*mag);
}

void CTerrainDialogBar::OnSkySatMoveUp() 
{
	// TODO: Add your control notification handler code here
	MoveSat (1500,0);
	World_changed=1;
}

void CTerrainDialogBar::OnSkySatMoveRight() 
{
	// TODO: Add your control notification handler code here
	MoveSat (0,1500);
	World_changed=1;
}

void CTerrainDialogBar::OnSkySatMoveLeft() 
{
	// TODO: Add your control notification handler code here
	MoveSat (0,64000);
	World_changed=1;
}

void CTerrainDialogBar::OnSkySatMoveDown() 
{
	// TODO: Add your control notification handler code here
	MoveSat (64000,0);
	World_changed=1;
}

void CTerrainDialogBar::OnToggleCellVisible() 
{
	// TODO: Add your control notification handler code here
	for (int i=0;i<TERRAIN_WIDTH*TERRAIN_DEPTH;i++)
	{
		if (TerrainSelected[i])
		{
			int c=Terrain_seg[i].flags & TF_INVISIBLE;

			if (c)
				Terrain_seg[i].flags &=~TF_INVISIBLE;
			else
				Terrain_seg[i].flags |=TF_INVISIBLE;

			World_changed=1;
	
		}
	}

	if (World_changed)
		GenerateLODDeltas ();

	UpdateDialog();
}

void CTerrainDialogBar::OnDestroy() 
{
	CDialogBar::OnDestroy();
	
	// TODO: Add your message handler code here
	dlgTerrainDialogBar = NULL;
}

void CTerrainDialogBar::SetCurrentSat(int n) 
{
	Current_satellite=n;
	UpdateDialog();
}


void CTerrainDialogBar::OnSkyMoveSatCloser() 
{
	// TODO: Add your control notification handler code here
	int n=Current_satellite;
	
	Terrain_sky.satellite_size[n]*=1.1f;

	World_changed=1;
}

void CTerrainDialogBar::OnSkyMoveSatFarther() 
{
	// TODO: Add your control notification handler code here
	int n=Current_satellite;
	
	Terrain_sky.satellite_size[n]*=.9f;

	World_changed=1;
}

void CTerrainDialogBar::OnTerrainSelect() 
{
	// TODO: Add your control notification handler code here
	int num = 0, num2 = 0;
	int top,bottom,left,right;
	int i,t;

	if (InputNumber(&num,"Select Cells","Enter first cell of block",this))
	{
		if (num<0)
		{
			OutrageMessageBox("You must enter a value greater than -1.");
			return;
		}
		if (num>=TERRAIN_DEPTH*TERRAIN_WIDTH)
		{
			OutrageMessageBox("You must enter a value less than %d.",TERRAIN_DEPTH*TERRAIN_WIDTH);
			return;
		}
		bottom = num/TERRAIN_WIDTH;
		left = num%TERRAIN_DEPTH;

	}
	else
		return;

	if (InputNumber(&num2,"Select Cells","Enter last cell of block",this))
	{
		if (num2<=num)
		{
			OutrageMessageBox("Last cell must be greater than first cell.");
			return;
		}
		if (num2<=0)
		{
			OutrageMessageBox("You must enter a value greater than 0.");
			return;
		}
		if (num2>=TERRAIN_DEPTH*TERRAIN_WIDTH)
		{
			OutrageMessageBox("You must enter a value less than %d.",TERRAIN_DEPTH*TERRAIN_WIDTH);
			return;
		}
		top = num2/TERRAIN_WIDTH;
		right = num2%TERRAIN_DEPTH;

	}
	else
		return;

	for (i=0;i<TERRAIN_WIDTH*TERRAIN_DEPTH;i++)
		TerrainSelected[i]=0;
	Num_terrain_selected=0;

	for (i=bottom;i<=top;i++)
	{
		for (t=left;t<=right;t++)
		{
			TerrainSelected[i*TERRAIN_WIDTH+t]=1;
			Num_terrain_selected++;
		}
	}
	State_changed = 1;
}

void CTerrainDialogBar::OnForceCellVisible() 
{
	// TODO: Add your control notification handler code here
	for (int i=0;i<TERRAIN_WIDTH*TERRAIN_DEPTH;i++)
	{
		if (TerrainSelected[i])
		{
			int c=Terrain_seg[i].flags & TF_INVISIBLE;

			if (c)
			{
				Terrain_seg[i].flags &=~TF_INVISIBLE;
				World_changed=1;
			}
		}
	}

	if (World_changed)
		GenerateLODDeltas ();

	UpdateDialog();
}
