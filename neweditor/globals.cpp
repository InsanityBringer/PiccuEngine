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
#include "globals.h"
#include "pstring.h"
#include "ned_DrawWorld.h"
#include "../editor/selectedroom.h"
#include "MainFrm.h"
#include "../editor/Erooms.h"
#include "../editor/RoomUVs.h"

#include "CameraSlew.h"
#include "TextureDialogBar.h"
#include "ObjectDialogBar.h"

#include "ned_Tablefile.h"


//Set this when the mine has changed
int World_changed = 0;

//Set this when the editor state (but not the world itself) has changed
bool State_changed = false;

// Stores full path of currently open level
CString Level_name="";

// Dialogs

CCameraSlew *gCameraSlewer = NULL;

room *Markedroomp=NULL;

int Markedface=0,Markededge=0,Markedvert=0;
int Cur_object_index = 0;
int Current_trigger=-1;

bool Disable_editor_rendering = false;

//Placed room info
int Placed_door=-1;
int Placed_room=-1;
group *Placed_group = NULL;
int Placed_room_face;
float Placed_room_angle;
vector Placed_room_origin;
matrix Placed_room_orient;
matrix Placed_room_rotmat;
vector Placed_room_attachpoint;
room *Placed_baseroomp;
int Placed_baseface;

short  use_opengl_1555_format;

float Gametime = 1.0f;
bool Dedicated_server = false;

int Low_vidmem=0;
bool Katmai=false;

extern int Current_ship = 0;

// Global cameras (see the defines in globals.h)
camera Level_cam,Room_cam;

int Editor_view_mode = VM_MINE; // (read from LoadLevel; not used in NEWEDITOR)

//The ID of the most recent viewer object (not counting room view)
int Editor_viewer_id; // (read from LoadLevel; not used in NEWEDITOR)

//Vars for the list of selected rooms
int N_selected_rooms=0;
int Selected_rooms[MAX_ROOMS];

// input

stKeys keys;	// key state flags
stMouse mouse;	// mouse state flags and position

//Misc

int gTimerID = 1;

char D3HogDir[_MAX_PATH*2];

float Frametime;

int FrameCount;

int  paged_in_num;
int  paged_in_count;

int  BSPChecksum;
float  Gravity_strength;
int  Game_mode;
int  Num_triggers;

unsigned char  Overlay_type;
int  Overlay_map;

#include "renderer.h"

//BSPs

#include "bsp.h"

bsptree MineBSP;

//Level & Mission info

#include "mission.h"

level_info Level_info;

//Doors

#include "ned_Door.h"

// Objects
#include "ned_Object.h"

// Triggers
#include "trigger.h"

//Weapons

#include "weapon.h"
#include "vecmat.h"
#include "polymodel.h"

weapon Weapons[MAX_WEAPONS];
int Num_weapons=0;

void InitWeapons ()
{
	for (int i=0;i<MAX_WEAPONS;i++)
	{
		Weapons[i].used=0;
		Weapons[i].name[0]=0;
	}
	Num_weapons=0;

}

bool WeaponCalcGun(vector *gun_point, vector *gun_normal, object *obj, int gun_num)
{
	poly_model *pm;
	vector pnt;
	vector normal;
	matrix m;
	int mn;				//submodel number
	float normalized_time[MAX_SUBOBJECTS];
	bool f_good_gp = true;

	if (!(obj->flags & OF_POLYGON_OBJECT))
	{
		// mprintf ((0,"Object type %d is not a polyobj!\n",obj->type));

		if(gun_point)
			*gun_point = obj->pos;
		if(gun_normal)
			*gun_normal = obj->orient.fvec;

		return false;
	}
	
	pm = &Poly_models[obj->rtype.pobj_info.model_num];

	if(pm->n_guns == 0)
	{
		mprintf((0, "WARNING: Object with no weapons is firing.\n", gun_num));
		
		if(gun_point)
			*gun_point = obj->pos;
		if(gun_normal)
			*gun_normal = obj->orient.fvec;

		return false;
	}

	SetNormalizedTimeObj(obj, normalized_time);
	SetModelAnglesAndPos (pm,normalized_time);

	if (gun_num < 0 || gun_num >= pm->n_guns)
	{
		mprintf((0, "WARNING: Bashing gun num %d to 0.\n", gun_num));
		if(gun_point)
			*gun_point = obj->pos;
		if(gun_normal)
			*gun_normal = obj->orient.fvec;
		return false;
	}

	pnt    = pm->gun_slots[gun_num].pnt;
	normal = pm->gun_slots[gun_num].norm;
	mn     = pm->gun_slots[gun_num].parent;

	// Instance up the tree for this gun
	while (mn != -1)
	{
		vector tpnt;

		vm_AnglesToMatrix(&m, pm->submodel[mn].angs.p,pm->submodel[mn].angs.h, pm->submodel[mn].angs.b);
		vm_TransposeMatrix(&m);

		if(gun_point)
			tpnt = pnt * m;
		
		if(gun_normal)
			normal = normal * m;

		if(gun_point)
			pnt = tpnt + pm->submodel[mn].offset + pm->submodel[mn].mod_pos;
		
		mn = pm->submodel[mn].parent;
	}	

	m = obj->orient;
	vm_TransposeMatrix(&m);

   if(gun_point)
		*gun_point  = pnt * m;
	if(gun_normal)
		*gun_normal = normal * m;

	if(gun_point)
		*gun_point += obj->pos;

	return f_good_gp;
} 


//Lighting
#include "lighting.h"

// Returns the total number of bytes needed for volume lighting in this room
int GetVolumeSizeOfRoom (room *rp,int *w,int *h,int *d)
{
	int width=((rp->max_xyz.x-rp->min_xyz.x)/VOLUME_SPACING)+1;
	int height=((rp->max_xyz.y-rp->min_xyz.y)/VOLUME_SPACING)+1;
	int depth=((rp->max_xyz.z-rp->min_xyz.z)/VOLUME_SPACING)+1;

	if (w)
		*w=width;
	if (h)
		*h=height;
	if (d)
		*d=depth;


	return (width*height*depth);

}


//Terrain

// resets the weather so there is nothing happening
void ResetWeather ()
{

}

//Fireballs

#include "fireball.h"


fireball Fireballs[NUM_FIREBALLS]=
	{{"ExplosionAA.oaf",FT_EXPLOSION,SMALL_TEXTURE,.9f,3.0},			//	MED_EXPLOSION2
	{"ExplosionBB.oaf",FT_EXPLOSION,SMALL_TEXTURE,.9f,2.0},			//	SMALL_EXPLOSION2
	{"explosionCC.oaf",FT_EXPLOSION,SMALL_TEXTURE,.9f,3.0f},			// MED_EXPLOSION
	{"explosionDD.oaf",FT_EXPLOSION,SMALL_TEXTURE,.9f,3.0f},			// MED_EXPLOSION3
	{"ExplosionE.oaf",FT_EXPLOSION,SMALL_TEXTURE,.9f,3.0},			// BIG_EXPLOSION
	{"ExplosionFF.oaf",FT_EXPLOSION,SMALL_TEXTURE,1.0f,1.0f},		// BILLOWING
	{"explosionG.oaf",FT_EXPLOSION,SMALL_TEXTURE,1.0f,2.0f},			// SMALL_EXPLOSION_INDEX
	{"smokepuff.oaf",FT_SMOKE,SMALL_TEXTURE,.7f,.7f},					// MED_SMOKE_INDEX
	{"black_smoke.oaf",FT_SMOKE,SMALL_TEXTURE,.7f,1.0f},				// BLACK_SMOKE
	{"red_blast_wave.ogf",FT_EFFECT,NORMAL_TEXTURE,1.0,1.0},			// RED_BLAST_RING	
	{"smokepuff.oaf",FT_SMOKE,SMALL_TEXTURE,.4f,.7f},					// SMOKE_TRAIL
	{"smokepuff.oaf",FT_EXPLOSION,SMALL_TEXTURE,.7f,3.0},				// CUSTOM_EXPLOSION
	{"explosionblast2.ogf",FT_EXPLOSION,NORMAL_TEXTURE,.7f,.7f},	// SHRINKING_BLAST
	{"black_smoke.oaf",FT_SMOKE,SMALL_TEXTURE,.7f,1.0f},				// SMOLDERING	
	{"warp.oaf",FT_EFFECT,NORMAL_TEXTURE,1.0,1.0},						// SHRINKING_BLAST2
	{"Hotspark.ogf",FT_SPARK,SMALL_TEXTURE,1.0,1.0},					// HOT_SPARK
	{"Coolspark.ogf",FT_SPARK,SMALL_TEXTURE,1.0,1.0},					// COOL_SPARK
	{"thrustball.ogf",FT_EFFECT,SMALL_TEXTURE,1.0,1.0},				// GRADIENT_BALL
	{"NOIMAGE",FT_EFFECT,SMALL_TEXTURE,.7f,3.0},							// SPRAY
	{"NOIMAGE",FT_EFFECT,SMALL_TEXTURE,.7f,3.0},							// FADING_LINE
	{"muzzleflash.ogf",FT_EFFECT,SMALL_TEXTURE,.7f,3.0},				// MUZZLE_FLASH
	{"shiphit.ogf",FT_EFFECT,NORMAL_TEXTURE,.7f,3.0},					// SHIP HIT EFFECT
	{"ConverterTube.ogf",FT_EFFECT,SMALL_TEXTURE,.7f,3.0},			// BLUE SHIELD RING
	{"NOIMAGE",FT_EFFECT,SMALL_TEXTURE,.7f,3.0},							// PARTICLE
	{"explosion.oaf",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},				// AFTERBURNER
	{"NOIMAGE",FT_EXPLOSION,SMALL_TEXTURE,1.0f,2.0f},					// NAPALM BALL
	{"LightningOriginA.ogf",FT_EXPLOSION,SMALL_TEXTURE,1.0f,2.0f},	// LIGHTNING ORIGINA
	{"LightningOriginB.ogf",FT_EXPLOSION,SMALL_TEXTURE,1.0f,2.0f}, // LIGHTNING ORIGINB
	{"Raindrop.ogf",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},					// Windshield drop
	{"Puddle.ogf",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},					//	Puddle drop
	{"NOIMAGE",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},						// Gravity effect
	{"NOIMAGE",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},						// LIGHTNING_BOLT_INDEX
	{"InvulnerabilityHit.ogf",FT_EFFECT,NORMAL_TEXTURE,1.0f,2.0f},	// Invul shield hit effect
	{"NOIMAGE",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},						// SINE_WAVE_INDEX
	{"NOIMAGE",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},						// AXIS_BILLBOARD_INDEX
	{"StarFlare6.ogf",FT_EFFECT,NORMAL_TEXTURE,1.0f,2.0f},			// DEFAULT_CORONA
	{"HeadlightFlare.ogf",FT_EFFECT,NORMAL_TEXTURE,1.0f,2.0f},		// HEADLIGHT_CORONA
	{"StarFlare.ogf",FT_EFFECT,NORMAL_TEXTURE,1.0f,2.0f},				// STAR_CORONA
	{"SunFlare.ogf",FT_EFFECT,NORMAL_TEXTURE,1.0f,2.0f},				// SUN_CORONA
	{"Whiteball.ogf",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},				// SNOWFLAKE_INDEX
	{"NOIMAGE",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},						// THICK_LIGHTNING_INDEX
	{"NapalmFire.oaf",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},				// BLUE_FIRE_INDEX
	{"Rocklette1.ogf",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},				// RUBBLE1_INDEX
	{"Rocklette2.ogf",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},				// RUBBLE2_INDEX
	{"Whiteball.ogf",FT_EFFECT,TINY_TEXTURE,1.0f,2.0f},					// WATER_SPLASH_INDEX
	{"Shatterbig.oaf",FT_EFFECT,SMALL_TEXTURE,1.0f,2.0f},				// SHATTER_INDEX
	{"Shattersmall.oaf",FT_EFFECT,SMALL_TEXTURE,1.0f,2.0f}};			// SHATTER_INDEX2

//Ships

#include "ship.h"

//Players

#include "player.h"
#include "inventory.h"

player Players[MAX_PLAYERS];
int Player_num;

//player_start_position_flags Player_start_flags[MAX_PLAYERS];


char LocalScriptDir[_MAX_PATH];


//Functions that are copied from the main app source

//Function to print to an edit box
//Parameters:	dlg - the dialog that the edit box is in
//					id - the ID of the edit box
//					fmt - printf-style text & arguments
void PrintToDlgItem(CDialog *dlg,int id,char *fmt,...)
{
	va_list arglist;
	char str[3000];
	int len;
	CEdit *ebox;

	va_start(arglist,fmt);
	len = Pvsprintf(str,3000,fmt,arglist);
	va_end(arglist);
	ASSERT(len < 3000);

	ebox = (CEdit *) dlg->GetDlgItem(id);
	if (ebox == NULL) {
		Int3();
		return;
	}
	ebox->SetWindowText(str);
}


// PrintStatus: this works exactly like EditorStatus from the old editor, except it doesn't require a preexisting main frame pointer
void PrintStatus( const char *format, ... )
{
	CMainFrame *mframe;

	mframe = (CMainFrame *) AfxGetMainWnd();
	if (mframe) {
		char status_line[256];
		va_list ap;

		va_start(ap, format);
		Pvsprintf(status_line, 256,format, ap);
		va_end(ap);

		mframe->SetStatusMessage(status_line);	
	}
}

char Editor_error_message[1000]="";

//Set the editor error message.  A function that's going to return a failure
//code should call this with the error message.
void SetErrorMessage(char *fmt,...)
{
	va_list arglist;

	va_start(arglist,fmt);
	Pvsprintf(Editor_error_message,sizeof(Editor_error_message),fmt,arglist);
	va_end(arglist);

	mprintf ((0,"Editor error: %s\n",Editor_error_message));
}

//Get the error message from the last function that returned failure
char *GetErrorMessage()
{
	return Editor_error_message;
}

void defer(void)
{
	MSG msg;

	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
	{
		if (msg.message == WM_QUIT) {
		//	QUIT APP.
			exit(1);
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

function_mode GetFunctionMode()
{
	return EDITOR_MODE; // always EDITOR_MODE for new editor
}

#include "resource.h"
#include "ProgressDialog.h"

CProgressDialog *gProgressDialog = NULL;

void LoadLevelProgress(int step,float percent,char *chunk)
{

	switch(step)
	{
	case LOAD_PROGRESS_START:
		{
			RECT prect,rect;
			int width,height;
			int pwidth,pheight;
			if(gProgressDialog)
			{
				delete gProgressDialog;
				gProgressDialog = NULL;
			}
			CWnd *parent_wnd = AfxGetMainWnd();
			
			//Add code here to center the new window
			if(parent_wnd)
			{
				parent_wnd->GetClientRect(&prect);
				pwidth = prect.right - prect.left;
				pheight = prect.bottom - prect.top;
			}
			
			
			gProgressDialog = new CProgressDialog;
			
			gProgressDialog->Create(IDD_LOADLEVELPROGRESS,parent_wnd);//Create( UINT nIDTemplate, CWnd* pParentWnd = NULL );
			if(!chunk)
				gProgressDialog->m_TitleText = "Loading level";
			else
				gProgressDialog->m_TitleText = chunk;
			gProgressDialog->ShowWindow(SW_SHOW);
			
			gProgressDialog->GetWindowRect(&rect);
			width = rect.right - rect.left;
			height = rect.bottom - rect.top;

			rect.top = (pheight/2)-(height/2);
			rect.left = (pwidth/2)-(width/2);
			rect.bottom = rect.top + height;
			rect.right = rect.left + width;

			gProgressDialog->MoveWindow(&rect);
			gProgressDialog->UpdateData(false);
			gProgressDialog->m_ProgressBar.SetStep(50);
			gProgressDialog->m_ProgressBar.SetPos(0);
		}
		break;
	case LOAD_PROGRESS_LOADING_LEVEL:
		if(gProgressDialog)
		{
			gProgressDialog->m_ProgressBar.SetPos((int)((float)100*(float)percent));
		}
		break;

	case LOAD_PROGRESS_PAGING_DATA:
		break;
	case LOAD_PROGRESS_PREPARE:
		break;
	case LOAD_PROGRESS_DONE:
		if(gProgressDialog)
		{
			gProgressDialog->DestroyWindow();
			delete gProgressDialog;
			gProgressDialog = NULL;
		}
		break;

	}

}


//constructor
Inventory::Inventory(void)
{

}

//destructor
Inventory::~Inventory(void)
{
	
}

void PrintDedicatedMessage(const char *fmt,...)
{


}

float Ubyte_to_float[256];

void InitDynamicLighting ()
{
	int i;

	// Setup ubyte to float
	for (i=0;i<256;i++)
		Ubyte_to_float[i]=(float)i/255.0;

}

ubyte Float_to_ubyte (float fnum)
{
	int i;

	if (fnum<0 || fnum>1.0)
	{
		mprintf ((0,"ERROR: Trying to get value in Float_to_ubyte that is %f!\n",fnum));
		return 0;
	}

	for (i=0;i<256;i++)
	{	
		if (fnum<Ubyte_to_float[i])
			return i;
	}
	
	return 255;
}

void rend_FreePreUploadedTexture(int a,int b)
{


}

int GetTextureBitmap (int handle,int framenum,bool force)
{
	return ned_GetTextureBitmap(handle, framenum, force);
}

int FindSoundName(char *p)
{
	return ned_FindSound(p);
}

int FindTextureName(char *p)
{
	return ned_FindTexture(p);
}

int FindObjectIDName(char *p)
{
	return ned_FindObjectID(p);
}

int FindDoorName(char *p)
{
	return ned_FindDoor(p);
}

//Look for player objects & set player starts
void FindPlayerStarts()
{
	int i;

	//Flag all players as unused
	for (i=0;i<MAX_PLAYERS;i++)
		Players[i].start_roomnum = -1;

	//Now look for player objects
	for (i=0;i<=Highest_object_index;i++)
		if (Objects[i].type == OBJ_PLAYER) {
			ASSERT(Players[Objects[i].id].start_roomnum == -1);		//make sure not used twice
			Players[Objects[i].id].start_pos = Objects[i].pos;
			Players[Objects[i].id].start_roomnum = Objects[i].roomnum;
			Players[Objects[i].id].start_orient = Objects[i].orient;
//			Players[Objects[i].id].start_flags = Objects[i].flags;
			Players[Objects[i].id].objnum=i;
		}

}

char *GetStringFromTable(int index)
{
	index;
	static char errstr[] = "No String table!";
	return errstr;
}


// Loads a bsp node from an open file and recurses with its children
void LoadBSPNode (CFILE *infile,bspnode **node)
{
	//Does nothing right now	
}

void InitDefaultBSP(void)
{

}


// from main\gameloop.cpp

//Current zoom factor (this is the tan of 29.25, which is half our FOV of 58.5)
float Render_FOV	= D3_DEFAULT_FOV;
float Render_zoom = D3_DEFAULT_ZOOM;

// from main\editor\gameeditor.cpp
static grViewport *Editor_g3_vp = NULL;

void StartEditorFrame(grViewport *vp, vector *viewer_eye, matrix *viewer_orient, float zoom)
{
	grSurface *surf;

	ASSERT(vp != NULL);

	Editor_g3_vp = vp;
	surf = Editor_g3_vp->lock();
	if (!surf) 
		Int3();													// This surface should be locked.  bad?

	rend_StartFrame(0,0,surf->width(),surf->height());
	g3_StartFrame(viewer_eye,viewer_orient,zoom);
}



void EndEditorFrame()
{
	Editor_g3_vp->unlock();
	Editor_g3_vp = NULL;
	g3_EndFrame();
	rend_EndFrame();
}



// from main\editor\moveworld.cpp
void GetMouseRotation( int idx, int idy, matrix * RotMat )
{
	float dr, cos_theta, sin_theta, denom, cos_theta1;
	float Radius = 100.0;
	float dx,dy;
	float dxdr,dydr;

	idy *= -1;

	dx = idx; dy = idy;

	dr = dist_2d(dx,dy);

	denom = dist_2d(Radius,dr);

	cos_theta = Radius / denom;
	sin_theta = dr / denom;

	cos_theta1 = 1.0 - cos_theta;

	dxdr = dx / dr;
	dydr = dy / dr;

	RotMat->rvec.x = cos_theta + ((dydr * dydr) * cos_theta1);
	RotMat->uvec.x = - ((dxdr * dydr) * cos_theta1);
	RotMat->fvec.x = (dxdr * sin_theta);

	RotMat->rvec.y = RotMat->uvec.x;
	RotMat->uvec.y = cos_theta + ((dxdr * dxdr) * cos_theta1);
	RotMat->fvec.y = (dydr * sin_theta);

	RotMat->rvec.z = -RotMat->fvec.x;
	RotMat->uvec.z = -RotMat->fvec.y;
	RotMat->fvec.z = cos_theta;

}




// Selection

//How big a box needs to be before we recognize it
#define MIN_BOX_SIZE 3


void EndSel(editorSelectorManager *esm)
{
	int left, top, right, bot;
	int n;

	esm->GetSelectedRect(&left, &top, &right, &bot);

	if ((right-left < MIN_BOX_SIZE) || (bot-top < MIN_BOX_SIZE))
		return;

	if (! keys.ctrl )		//ctrl + select means add to list
		ClearRoomSelectedList();

	n = SelectRoomsInBox(NULL,left,top,right,bot);
	PrintStatus("%d rooms selected",n);

	esm->EndSelection();

}

// ==================
// ned_SetupSearchPaths
// ==================
//
// Sets up the file search paths, based off base directory passed in
void ned_SetupSearchPaths(char *base_dir)
{
	mprintf((0,"Setting up data search paths\n"));

	//Add in the search paths
	char buffer[_MAX_PATH];
	cf_ClearAllSearchPaths();
	cf_SetSearchPath(LocalScriptDir,NULL);
	ddio_MakePath(buffer,base_dir,"data","",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","graphics",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","art",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","sounds",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","models",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","scripts",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","misc",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","music",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","levels",NULL);
	cf_SetSearchPath(buffer,NULL);
	ddio_MakePath(buffer,base_dir,"data","briefings",NULL);	
	cf_SetSearchPath(buffer,NULL);
}

CEditorState Editor_state;

CEditorState::CEditorState()
{
	mode = MODE_VERTEX;
	texscale = 1.0f;
	nodetype = NODETYPE_GAME;
	path = -1;
	node = -1;
	bn_room = 0;
	bn_node = 0;
	bn_edge = 0;
	Desktop_surf = NULL;
}

CEditorState::~CEditorState()
{
	if (Desktop_surf) {
		delete Desktop_surf;
		Desktop_surf = NULL;
	}
}

//Attaches the desktop grSurface to a window
void CEditorState::AttachDesktopSurface(unsigned hWnd)
{
	if(Desktop_surf)
	{
		Desktop_surf->attach_to_window(hWnd);
	}
}

//Initializes the Editor State
void CEditorState::Initialize(void)
{
	// TODO : read values from registry
	texscale = 1.0f;
	Desktop_surf = new grSurface(0,0,0, SURFTYPE_VIDEOSCREEN, 0);
}

//Shutsdown the Editor State
void CEditorState::Shutdown(void)
{
	if (Desktop_surf) {
		delete Desktop_surf;
		Desktop_surf = NULL;
	}
}

//Returns a pointer to the desktop surface
grSurface *CEditorState::GetDesktopSurface(void)
{
	return Desktop_surf;
}

//Blits a surface to the desktop surface
bool CEditorState::DesktopBlit(int x,int y,grSurface *sSource)
{
	if(!Desktop_surf)
		return false;

	Desktop_surf->blt(x, y, sSource);
	return true;
}

//Clears the desktop surface
void CEditorState::DesktopClear(int x,int y,int w,int h)
{
	if(!Desktop_surf)
		return;

	Desktop_surf->clear(x,y,w,h);
}

//Sets up texture dragging
bool CEditorState::RegisterTextureDropWindow(CWnd *wnd)
{
	return (bool)(m_TextureDrop.Register(wnd)!=0);
}

extern CTextureDialogBar *dlgTextureDialogBar;
//writes a texture list to an open file
bool CEditorState::WriteTextureList(CFILE *outfile,int list)
{
	ASSERT_VALID(dlgTextureDialogBar);
	if(!dlgTextureDialogBar)
		return false;

	return dlgTextureDialogBar->WriteTextureList(outfile,list);
}

//reads a texture list from an open file
int CEditorState::ReadTextureList(CFILE *infile)
{
	ASSERT_VALID(dlgTextureDialogBar);
	if(!dlgTextureDialogBar)
		return false;

	return dlgTextureDialogBar->ReadTextureList(infile);
}

//sets the current room/face for the texture view
void CEditorState::SetCurrentRoomFaceTexture(int room,int face)
{
	ASSERT_VALID(dlgTextureDialogBar);
	if(!dlgTextureDialogBar)
		return;

	dlgTextureDialogBar->SetCurrentFace(room,face);
}

// gets the current texture for the texture view
int CEditorState::GetCurrentTexture()
{
	ASSERT_VALID(dlgTextureDialogBar);
	if(!dlgTextureDialogBar)
		return BAD_BITMAP_HANDLE;

	return dlgTextureDialogBar->GetCurrentTexture();
}

// grabs the texture of the currently focused window and makes it the current texture
int CEditorState::GrabTexture()
{
	ASSERT_VALID(dlgTextureDialogBar);
	if(!dlgTextureDialogBar)
		return -1;

	return dlgTextureDialogBar->GrabTexture();
}

// gets the current node type (gamepath node, bnode)
int CEditorState::GetCurrentNodeType()
{
	return nodetype;
}

// sets the current node type
void CEditorState::SetCurrentNodeType(int type)
{
	nodetype = type;
}

// gets the current path
int CEditorState::GetCurrentPath()
{
	return path;
}

// gets the current node
int CEditorState::GetCurrentNode()
{
	return node;
}

// sets the current path
void CEditorState::SetCurrentPath(int p)
{
	path = p;
}

// sets the current node
void CEditorState::SetCurrentNode(int n)
{
	node = n;
}

// BNodes
void CEditorState::GetCurrentBNode(int *r,int *n,int *e)
{
	*r = bn_room;
	*n = bn_node;
	*e = bn_edge;
}

void CEditorState::SetCurrentBNode(int r,int n,int e)
{
	bn_room = r;
	bn_node = n;
	bn_edge = e;
}

DROPEFFECT CTextureOLETarget::OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point )
{
	UINT format = RegisterClipboardFormat("TextureDrop");

	HGLOBAL hmem = pDataObject->GetGlobalData(format);
	CMemFile sf((BYTE*) ::GlobalLock(hmem), ::GlobalSize(hmem));
	CString buffer;
	LPSTR str = buffer.GetBufferSetLength(::GlobalSize(hmem));
	sf.Read(str, ::GlobalSize(hmem));
	::GlobalUnlock(hmem);

	POINT pt = point;
	DrawText(pt.x+15,pt.y+5,pWnd,buffer.GetBuffer(0));
	
	if (pDataObject->IsDataAvailable(format)) 
	{		
		return DROPEFFECT_COPY;
	}
	return DROPEFFECT_NONE;
}

DROPEFFECT CTextureOLETarget::OnDropEx( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point )
{
	UINT format = RegisterClipboardFormat("TextureDrop");
	if (pDataObject->IsDataAvailable(format)) 
	{		
		HGLOBAL hmem = pDataObject->GetGlobalData(format);
		
		CMemFile sf((BYTE*) ::GlobalLock(hmem), ::GlobalSize(hmem));
		
		CString buffer;
		
		LPSTR str = buffer.GetBufferSetLength(::GlobalSize(hmem));

		sf.Read(str, ::GlobalSize(hmem));
		::GlobalUnlock(hmem);

		// Do something with the data in 'buffer'
		mprintf((0,"OnDrop received = '%s'\n", buffer.GetBuffer(0)));

		int tex_slot = ned_FindTexture(buffer.GetBuffer(0));
		((CTextureBarList *)pWnd)->AddTextureToList(tex_slot);

		pWnd->InvalidateRect(NULL,false);
		defer();

		return DROPEFFECT_COPY;
	}

	return DROPEFFECT_NONE;
}

void CTextureOLETarget::OnDragLeave( CWnd* pWnd )
{
	pWnd->InvalidateRect(NULL);
	defer();
}

void CTextureOLETarget::DrawText(int x,int y,CWnd *pWnd,char *text)
{	
	pWnd->InvalidateRect(NULL,false);
	defer();

	CDC *dc = pWnd->GetDC();	

	COLORREF crFore = GetSysColor(COLOR_MENUTEXT);
	COLORREF crBack = GetSysColor(COLOR_MENU);

	COLORREF crOldTextColor = dc->SetTextColor(crFore);
	int nOldBkMode = dc->SetBkMode(TRANSPARENT);
	COLORREF crOldBackColor = dc->SetBkColor(crBack);

	dc->TextOut(x,y,text,strlen(text));

	dc->SetTextColor(crOldTextColor);
	dc->SetBkColor(crOldBackColor);
	dc->SetBkMode(nOldBkMode);

	pWnd->ReleaseDC(dc);
}


//Sets up object dragging
bool CEditorState::RegisterGenericDropWindow(CWnd *wnd)
{
	return (bool)(m_GenericDrop.Register(wnd)!=0);
}

extern CObjectDialogBar *dlgObjectDialogBar;
extern CSoundDialogBar *dlgSoundDialogBar;
//writes an object list to an open file
bool CEditorState::WriteObjectList(CFILE *outfile,int list)
{
	ASSERT_VALID(dlgObjectDialogBar);
	if(!dlgObjectDialogBar)
		return false;

	return dlgObjectDialogBar->WriteObjectList(outfile,list);
}

//reads an object list from an open file
int CEditorState::ReadObjectList(CFILE *infile)
{
	ASSERT_VALID(dlgObjectDialogBar);
	if(!dlgObjectDialogBar)
		return false;

	return dlgObjectDialogBar->ReadObjectList(infile);
}

//sets the current level object for the object view
void CEditorState::SetCurrentLevelObject(int object)
{
	ASSERT_VALID(dlgObjectDialogBar);
	if(!dlgObjectDialogBar)
		return;

	dlgObjectDialogBar->SetCurrentLevelObject(object);

	// Piggyback an update to the sound dialog bar
	ASSERT_VALID(dlgSoundDialogBar);
	if(!dlgSoundDialogBar)
		return;

	dlgSoundDialogBar->UpdateDialog();
}

// gets the current object id
int CEditorState::GetCurrentObject()
{
	ASSERT_VALID(dlgObjectDialogBar);
	if(!dlgObjectDialogBar)
		return -1;

	return dlgObjectDialogBar->GetCurrentObject();
}

extern CDoorwayDialogBar *dlgDoorwayDialogBar;
//sets the current doorway
void CEditorState::SetCurrentDoorway(room *rp)
{
	ASSERT_VALID(dlgDoorwayDialogBar);
	if(!dlgDoorwayDialogBar)
		return;

	// If this room is a door room, set the current doorway
	if (rp != NULL && rp->flags & RF_DOOR)
	{
		doorway *dp = rp->doorway_data;
		ASSERT(dp != NULL);
		ASSERT(rp->objects != -1 && Objects[rp->objects].type & OBJ_DOOR);
		dlgDoorwayDialogBar->SetCurrentDoorway(rp->objects);
	}
	else
		dlgDoorwayDialogBar->SetCurrentDoorway(-1);
}

// gets the current door id
int CEditorState::GetCurrentDoor()
{
	ASSERT_VALID(dlgDoorwayDialogBar);
	if(!dlgDoorwayDialogBar)
		return -1;

	return dlgDoorwayDialogBar->GetCurrentDoor();
}

DROPEFFECT CGenericOLETarget::OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point )
{
	UINT format = RegisterClipboardFormat("GenericDrop");

	HGLOBAL hmem = pDataObject->GetGlobalData(format);
	CMemFile sf((BYTE*) ::GlobalLock(hmem), ::GlobalSize(hmem));
	CString buffer;
	LPSTR str = buffer.GetBufferSetLength(::GlobalSize(hmem));
	sf.Read(str, ::GlobalSize(hmem));
	::GlobalUnlock(hmem);

	POINT pt = point;
	DrawText(pt.x+15,pt.y+5,pWnd,buffer.GetBuffer(0));
	
	if (pDataObject->IsDataAvailable(format)) 
	{		
		return DROPEFFECT_COPY;
	}
	return DROPEFFECT_NONE;
}

DROPEFFECT CGenericOLETarget::OnDropEx( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point )
{
	UINT format = RegisterClipboardFormat("GenericDrop");
	if (pDataObject->IsDataAvailable(format)) 
	{		
		HGLOBAL hmem = pDataObject->GetGlobalData(format);
		
		CMemFile sf((BYTE*) ::GlobalLock(hmem), ::GlobalSize(hmem));
		
		CString buffer;
		
		LPSTR str = buffer.GetBufferSetLength(::GlobalSize(hmem));

		sf.Read(str, ::GlobalSize(hmem));
		::GlobalUnlock(hmem);

		// Do something with the data in 'buffer'
		mprintf((0,"OnDrop received = '%s'\n", buffer.GetBuffer(0)));

		int obj_slot = ned_FindObjectID(buffer.GetBuffer(0));
		((CObjectBarList *)pWnd)->AddObjectToList(obj_slot);

		pWnd->InvalidateRect(NULL,false);
		defer();

		return DROPEFFECT_COPY;
	}

	return DROPEFFECT_NONE;
}

void CGenericOLETarget::OnDragLeave( CWnd* pWnd )
{
	pWnd->InvalidateRect(NULL);
	defer();
}

void CGenericOLETarget::DrawText(int x,int y,CWnd *pWnd,char *text)
{	
	pWnd->InvalidateRect(NULL,false);
	defer();

	CDC *dc = pWnd->GetDC();	

	COLORREF crFore = GetSysColor(COLOR_MENUTEXT);
	COLORREF crBack = GetSysColor(COLOR_MENU);

	COLORREF crOldTextColor = dc->SetTextColor(crFore);
	int nOldBkMode = dc->SetBkMode(TRANSPARENT);
	COLORREF crOldBackColor = dc->SetBkColor(crBack);

	dc->TextOut(x,y,text,strlen(text));

	dc->SetTextColor(crOldTextColor);
	dc->SetBkColor(crOldBackColor);
	dc->SetBkMode(nOldBkMode);

	pWnd->ReleaseDC(dc);
}



// from Main\viseffect.cpp

vis_effect *VisEffects=NULL;//[MAX_VIS_EFFECTS];

// Renders a vis effect
// (stub)
void DrawVisEffect (vis_effect *vis)
{
}


#if 0//def RELEASE

//Return types are the same the Windows return values
int Debug_MessageBox(int type, const char *title, const char *str)
{
	DWORD flags;
	int answer;
	HWND wnd = GetActiveWindow();

	if (type == OSMBOX_OK) flags = MB_OK;
	else if (type == OSMBOX_YESNO) flags = MB_YESNO;
	else if (type == OSMBOX_YESNOCANCEL) flags = MB_YESNOCANCEL;
	else if (type == OSMBOX_ABORTRETRYIGNORE) flags = MB_ABORTRETRYIGNORE;
	else DebugBreak();

	ShowCursor(TRUE);
//	ShowWindow(wnd, SW_SHOWMINNOACTIVE);
	answer = MessageBox(NULL, str, title, flags | MB_TASKMODAL | MB_SETFOREGROUND);
	ShowCursor(FALSE);
//	ShowWindow(wnd, SW_SHOWMAXIMIZED);

	return answer;
}

#define BUF_LEN 1024
char Messagebox_title[] = "Descent 3 Level Editor";

// Pops up a dialog box to display a message
void OutrageMessageBox(char *str, ...) 
{
	char buf[BUF_LEN];
	va_list arglist;
	int nchars;

	va_start(arglist, str);
		nchars = Pvsprintf(buf, BUF_LEN,str, arglist);
	va_end(arglist);

	if (nchars >= BUF_LEN)
		Debug_MessageBox(OSMBOX_OK, Messagebox_title, "The dialog that follows this one overflowed its text buffer.  The program may crash.");	

	Debug_MessageBox(OSMBOX_OK, Messagebox_title, buf);	

}


int OutrageMessageBox(int type, char *str, ...)
{
	char buf[BUF_LEN];
	va_list arglist;
	int os_flags;
	int nchars;

	va_start(arglist, str);
	nchars = Pvsprintf(buf, BUF_LEN,str, arglist);
	va_end(arglist);
	
	if (type == MBOX_OK) 
		os_flags = OSMBOX_OK;
	else if (type == MBOX_YESNO) 
		os_flags = OSMBOX_YESNO;
	else if (type == MBOX_YESNOCANCEL) 
		os_flags = OSMBOX_YESNOCANCEL;
	else if (type == MBOX_ABORTRETRYIGNORE) 
		os_flags = OSMBOX_ABORTRETRYIGNORE;
	else
		Int3();

	if (nchars >= BUF_LEN)
		Debug_MessageBox(OSMBOX_OK, Messagebox_title, "The dialog that follows this one overflowed its text buffer.  The program may crash.");	

	return Debug_MessageBox(os_flags, Messagebox_title, buf);
}


#endif


int osipf_FindSoundName(char *name)
{
	return FindSoundName(IGNORE_TABLE(name));
}

int osipf_FindRoomName(char *name)
{
	for(int i=0;i<=Highest_room_index;i++){
		if(Rooms[i].used && Rooms[i].name){
			if(!stricmp(name,Rooms[i].name))
				return i;
		}
	}
	return -1;
}

int osipf_FindTriggerName(char *name)
{
	for(int i=0;i<Num_triggers;i++){
		if(Triggers[i].name){
			if(!stricmp(name,Triggers[i].name))
				return i;
		}
	}
	return -1;
}

int osipf_FindObjectName(char *name)
{
	for (int i=0;i<MAX_OBJECTS;i++){
		if(Objects[i].type!=OBJ_NONE && Objects[i].name!=NULL){
			if(!stricmp(name,Objects[i].name))
				return Objects[i].handle;
		}
	}
	return OBJECT_HANDLE_NONE;
}

int osipf_GetTriggerRoom(int trigger_id)
{
	if(trigger_id<0 || trigger_id>=Num_triggers)
		return -1;

	return Triggers[trigger_id].roomnum;
}

int osipf_GetTriggerFace(int trigger_id)
{
	if(trigger_id<0 || trigger_id>=Num_triggers)
		return -1;

	return Triggers[trigger_id].facenum;
}

int osipf_FindDoorName(char *name)
{
	for(int i=0;i<=MAX_OBJECTS;i++){
		if(Objects[i].type==OBJ_DOOR && Objects[i].name && !stricmp(Objects[i].name,name) ){
			return Objects[i].handle;
		}
	}
	return OBJECT_HANDLE_NONE;
}

int osipf_FindTextureName(char *name)
{
	return FindTextureName(IGNORE_TABLE(name));
}

#include "matcen.h"

int osipf_FindMatcenName(char *name)
{
	return FindMatcenIndex(name);
}

#include "gamepath.h"

int osipf_FindPathName(char *name)
{
	return FindGamePathName (name);
}

#include "levelgoal.h"

int osipf_FindLevelGoalName(char *name)
{
	return Level_goals.GoalFindId(name);
}

//Retruns a pointer to an object given its handle.  Returns NULL if object no longer exists.
object *ObjGet(int handle)
{
	if (handle == OBJECT_HANDLE_NONE)
		return NULL;

	if (handle == OBJECT_HANDLE_BAD) {
		Int3();
		return NULL;
	}

	int objnum = handle & HANDLE_OBJNUM_MASK;
	object *objp;

	ASSERT((handle & HANDLE_COUNT_MASK) != 0);		//count == 0 means never-used object
	ASSERT(objnum < MAX_OBJECTS);

	objp = &Objects[objnum];

	if ((objp->type != OBJ_NONE) && (objp->handle == handle))
		return objp;
	else
		return NULL;
}

#undef PLAY_SOUND_SYSTEM

#ifndef PLAY_SOUND_SYSTEM

// Stub sound system functions (Dallas need them)
/////////////////////////////////////////////////////
#include "hlsoundlib.h"
// Begin_sound_frame(listener pos/orient/velocity)
// SyncSounds
// Do sound pos updates -- IF VOLUME IS LOW AND NOT FOREVER, THEN STOP SOUND
// compute echo / reverb
// indirect/direct path sounds
void hlsSystem::BeginSoundFrame(bool f_in_game){}
// Plays the deffered 3d stuff
hlsSystem::hlsSystem(void){}
void hlsSystem::EndSoundFrame(void){}
void hlsSystem::StopAllSounds(void){}
int hlsSystem::Play2dSound(int,float,float,unsigned short){return -1;}
void hlsSystem::KillSoundLib(bool){}
int StreamPlay(char const *,float,int){return -1;}
// sound_info Sounds[MAX_SOUNDS];
hlsSystem Sound_system;

#endif

#include "game.h"

int sound_override_force_field = -1;
int sound_override_glass_breaking = -1;

int   force_field_bounce_texture[MAX_FORCE_FIELD_BOUNCE_TEXTURES] = {-1, -1, -1};
float force_field_bounce_multiplier[MAX_FORCE_FIELD_BOUNCE_TEXTURES] = {1.0f, 1.0f, 1.0f};

bool Level_powerups_ignore_wind = false;

void DestroyStringTable(char * *blah,int iblah)
{


}

bool CreateStringTable(char *file,char * * *ptr,int *num)
{

	return false;
}

void Localization_SetLanguage(int i)
{

}

int Localization_GetLanguage(void)
{

	return 1;
}

terrain_sound_band Terrain_sound_bands[NUM_TERRAIN_SOUND_BANDS];

//Clear out all the terrain sound bands
void ClearTerrainSound()
{
	for (int b=0;b<NUM_TERRAIN_SOUND_BANDS;b++) {
		Terrain_sound_bands[b].sound_index	= -1;
		Terrain_sound_bands[b].low_alt		= 0;
		Terrain_sound_bands[b].high_alt		= 0;
		Terrain_sound_bands[b].low_volume	= 0.0;
		Terrain_sound_bands[b].high_volume	= 0.0;
	}
}


#include "boa.h"

// from main/AImain.cpp
int AIMakeNextRoomList(int roomnum, int *next_rooms, int max_rooms)
{
	int num_next_rooms = 0;
	int i,j;
	int croom;

	if(!ROOMNUM_OUTSIDE(roomnum) && roomnum <= Highest_room_index)
	{
		for(i = 0; i < Rooms[roomnum].num_portals; i++)
		{
			bool f_found = false;

			if(Rooms[roomnum].portals[i].croom >= 0)
			{
				if(Rooms[Rooms[roomnum].portals[i].croom].flags & RF_EXTERNAL)
				{
					croom = Highest_room_index + TERRAIN_REGION(GetTerrainRoomFromPos(&Rooms[Rooms[roomnum].portals[i].croom].portals[Rooms[roomnum].portals[i].cportal].path_pnt)) + 1;
				}
				else
				{
					croom = Rooms[roomnum].portals[i].croom;
				}

				for(j = 0; j < num_next_rooms; j++)
				{
					if(next_rooms[j] == croom)
					{
						f_found = true;
						break;
					}
				}

				if(!f_found)
				{
					// If you hit assert, get chris -- make constant larger
					ASSERT(num_next_rooms < max_rooms);

					ASSERT((croom >= 0 && croom <= Highest_room_index + 8) || (ROOMNUM_OUTSIDE(croom) && CELLNUM(roomnum) > 0 && CELLNUM(roomnum) < TERRAIN_WIDTH * TERRAIN_DEPTH));

					next_rooms[num_next_rooms] = croom;
					num_next_rooms++;
				}
			}
		}
	}
	else
	{
		int t_index;

		if(BOA_num_terrain_regions == 0)
		{
			return 0;
		}

		if(roomnum > Highest_room_index && roomnum <= Highest_room_index + 8)
		{
			t_index = roomnum - Highest_room_index - 1;
		}
		else
		{
			t_index = TERRAIN_REGION(roomnum);
		}

		ASSERT(t_index >= 0 && t_index < BOA_num_terrain_regions);

		for(i = 0; i < BOA_num_connect[t_index]; i++)
		{
			bool f_found = false;
			croom = BOA_connect[t_index][i].roomnum;

			for(j = 0; j < num_next_rooms; j++)
			{
				if(next_rooms[j] == croom)
				{
					f_found = true;
					break;
				}
			}

			if(!f_found)
			{
				// If you hit assert, get chris -- make constant larger
				ASSERT(num_next_rooms < max_rooms);
				next_rooms[num_next_rooms++] = croom;
			}
		}
	}

	return num_next_rooms;
}


// from physics\physics.cpp
bool PhysCalcGround(vector *ground_point, vector *ground_normal, object *obj, int ground_num)
{
	poly_model *pm;
	vector pnt;
	vector normal;
	matrix m;
	int mn;				//submodel number
	float normalized_time[MAX_SUBOBJECTS];
	bool f_good_gp = true;

	if (obj->render_type!=RT_POLYOBJ && obj->type!=OBJ_PLAYER)
	{
		// mprintf ((0,"Object type %d is not a polyobj!\n",obj->type));

		if(ground_point)
			*ground_point = obj->pos;
		if(ground_normal)
			*ground_normal = obj->orient.fvec;

		return false;
	}
	
	pm = &Poly_models[obj->rtype.pobj_info.model_num];

	if(pm->n_ground == 0)
	{
		mprintf((0, "WARNING: Object with no weapons is firing.\n", ground_num));
		
		if(ground_point)
			*ground_point = obj->pos;
		if(ground_normal)
			*ground_normal = obj->orient.fvec;

		return false;
	}

	SetNormalizedTimeObj(obj, normalized_time);

	SetModelAnglesAndPos (pm,normalized_time);

	if (ground_num < 0 || ground_num >= pm->n_ground)
	{
		mprintf((0, "WARNING: Bashing ground num %d to 0.\n", ground_num));
		ground_num = 0;
		f_good_gp = false;
	}

	pnt    = pm->ground_slots[ground_num].pnt;
	normal = pm->ground_slots[ground_num].norm;
	mn     = pm->ground_slots[ground_num].parent;

	// Instance up the tree for this ground
	while (mn != -1)
	{
		vector tpnt;

		vm_AnglesToMatrix(&m, pm->submodel[mn].angs.p,pm->submodel[mn].angs.h, pm->submodel[mn].angs.b);
		vm_TransposeMatrix(&m);

		tpnt    = pnt * m;
		normal = normal * m;

		pnt = tpnt + pm->submodel[mn].offset + pm->submodel[mn].mod_pos;
		
		mn = pm->submodel[mn].parent;
	}	

	m = obj->orient;
	vm_TransposeMatrix(&m);

	if(ground_point)
	   *ground_point  = pnt * m;
	if(ground_normal)
		*ground_normal = normal * m;

	if(ground_point)
		*ground_point += obj->pos;

	return f_good_gp;
} 

