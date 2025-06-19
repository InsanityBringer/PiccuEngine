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
 

#ifndef GLOBALS_HEADER
#define GLOBALS_HEADER

#include "stdafx.h"
#include <stdlib.h>

#include "3d.h"
#include "room.h"
#include "../editor/group.h"
#include "../editor/SelManager.h"
#include "RendHandle.h"

#define EditorMessageBox OutrageMessageBox
extern int gTimerID;

#define IDC_LEVEL_WND		20000

#define TIMER_MINERVA		4321

//Set this when the editor viewpoint has changed
extern bool Edview_changed;

//flags for the textured views changed
extern int TV_changed;

extern int LightSpacing;
//Set this when the mine has changed
extern int World_changed;

//Set this when the editor state (but not the world itself) has changed
extern bool State_changed;

// More old editor vars
extern float GlobalMultiplier;
extern float Ambient_red;
extern float Ambient_green;
extern float Ambient_blue;
extern int rad_MaxStep;

//Structure to describe a 3D camera
typedef struct {
	matrix orient;
	vector target;
	vector desttarget;
	vector step;
	float dist;
	float rad;
	bool view_changed;
	bool moving;
} camera;

//Variables for current view position and orientation
#define DEFAULT_VIEW_DIST	500
#define DEFAULT_VIEW_RAD	5000

#define MOVE_SCALE  3.0
#define ZOOM_SCALE 10.0
#define RAD_SCALE	 10.0

#define dist_2d(x,y)  sqrt((x)*(x) + (y)*(y))

// variables for current primitives
extern room *Curroomp,*Markedroomp;
extern int Curface,Curedge,Curvert,Curportal;
extern int Markedface,Markededge,Markedvert;
extern int Cur_object_index;
extern int Current_trigger;

//Placed room info
extern int Placed_room;
extern group *Placed_group;
extern int Placed_room_face;
extern int Placed_door;
extern float Placed_room_angle;
extern vector Placed_room_origin;
extern matrix Placed_room_orient;
extern vector Placed_room_attachpoint;
extern matrix Placed_room_rotmat;
extern room *Placed_baseroomp;
extern int Placed_baseface;

extern float Gametime;
extern bool Katmai;

// What mode we're currently in
enum {VM_MINE,VM_TERRAIN,VM_ROOM,NUM_VIEW_MODES};
extern int Editor_view_mode; // (read from LoadLevel; not used in NEWEDITOR)

extern CString Level_name;

//The ID of the most recent viewer object (not counting room view)
extern int Editor_viewer_id; // (read from LoadLevel; not used in NEWEDITOR)

// defines (old editor -> new editor) to make LoadLevel happy
// NOTE: Level_cam and Room_cam variables only exist for backwards compatibility with LoadLevel.cpp - ONLY time to use these is when loading or saving
#define wireframe_view camera;
#define Wireframe_view_mine Level_cam
#define Wireframe_view_room Room_cam
#define EditorStatus PrintStatus
#define D3EditState Editor_state
#define Curroomp theApp.m_pLevelWnd->m_Prim.roomp
#define Curface theApp.m_pLevelWnd->m_Prim.face
#define Curportal theApp.m_pLevelWnd->m_Prim.portal
#define Curedge theApp.m_pLevelWnd->m_Prim.edge
#define Curvert theApp.m_pLevelWnd->m_Prim.vert

extern int Current_ship;

// Camera for the level window
extern camera Level_cam;
// Camera for the most recently focused room window
extern camera Room_cam;

extern bool  Dedicated_server;
extern float Room_ambience_r[],Room_ambience_g[],Room_ambience_b[];

//Vars for the list of selected rooms
extern int N_selected_rooms;
extern int Selected_rooms[];

extern float Room_multiplier[];

extern char LocalScriptDir[_MAX_PATH];

// key state flags
typedef struct{
	bool up;
	bool down;
	bool left;
	bool right;
	bool subtract;
	bool add;
	bool num0;
	bool num5;
	bool num1;
	bool num3;
	bool num7;
	bool num9;
	bool shift;
	bool ctrl;
	bool alt;
	int zoom;
} stKeys;

extern stKeys keys;

// mouse state
typedef struct
{
	int x, y;			// current position
	int oldx, oldy;		// last position
	bool left;			// left button
	bool right;			// right button
	bool mid;
} stMouse;

extern stMouse mouse;

typedef struct t_prim
{
	room *roomp;
	int face;
	int portal;
	int edge;
	int vert;
} prim;

// Defines for cycling primitives
#define PREV	0
#define NEXT	1

// extrude directions
#define X_AXIS	0
#define Y_AXIS	1
#define Z_AXIS	2
#define NORMAL	3

// extruded faces
#define CURRENT	0
#define MARKED	1

#include "ned_Object.h"
#include "ned_GameTexture.h"
#include "ned_Door.h"

extern short Level_textures_in_use[MAX_TEXTURES];
extern short Level_objects_in_use[MAX_OBJECT_IDS];
extern short Level_doors_in_use[MAX_DOORS];

extern char D3HogDir[_MAX_PATH*2];

// D3Edit Resource Lists (*.DER)
#define DER_VERSION			1
#define LIST_NAME_LEN		_MAX_PATH
#define CUSTOM_TEX_LIST		1
#define LEVEL_TEX_LIST		2
#define CUSTOM_OBJ_LIST		4
#define LEVEL_OBJ_LIST		8

//Look for player objects & set player starts
void FindPlayerStarts();
void StartEditorFrame(RendHandle& handle, vector *viewer_eye, matrix *viewer_orient, float zoom);
void EndEditorFrame();
void GetMouseRotation( int idx, int idy, matrix * RotMat );
void LoadLevelProgress(int step,float percent,char *chunk);
//Function to print to an edit box
//Parameters:	dlg - the dialog that the edit box is in
//					id - the ID of the edit box
//					fmt - printf-style text & arguments
void PrintToDlgItem(CDialog *dlg,int id,char *fmt,...);
// PrintStatus: this works exactly like EditorStatus from the old editor, except it doesn't require a preexisting main frame pointer
void PrintStatus( const char *format, ... );
//Set the editor error message.  A function that's going to return a failure
//code should call this with the error message.
void SetErrorMessage(char *fmt,...);
//Get the error message from the last function that returned failure
char *GetErrorMessage();
// Selection
void EndSel(editorSelectorManager *esm);
// Defer's back to windows to do some processing
void defer(void);

// ==================
// ned_SetupSearchPaths
// ==================
//
// Sets up the file search paths, based off base directory passed in
void ned_SetupSearchPaths(char *base_dir);

// ==========================
// LevelTexInitLevel
// ==========================
//
// Call this as soon as a level is loaded, it will go through the level and set it up.
void LevelTexInitLevel(void);

// ==========================
// RoomTexInitRoom
// ==========================
//
// Call this as soon as a paletted room file (ORF) is loaded, it will go through the room and set it up.
void RoomTexInitRoom(room *rp, short *Textures_in_use);

// ==========================
// LevelTexPurgeAllTextures
// ==========================
//
// Call this when a level is being unloaded to purge all textures associated with it
void LevelTexPurgeAllTextures(void);

// ==========================
// RoomTexPurgeAllTextures
// ==========================
//
// Call this when a room is being unloaded to purge all textures associated with it
void RoomTexPurgeAllTextures(short *Textures_in_use);

// ===========================
// LevelTexDecrementTexture
// ===========================
//
// Call this function when a texture is being removed from a face in the level (i.e it's
// being retextured).
void LevelTexDecrementTexture(int texnum);

// ===========================
// RoomTexDecrementTexture
// ===========================
//
// Call this function when a texture is being removed from a face in a paletted room (i.e it's
// being retextured).
//@@void RoomTexDecrementTexture(int texnum, short *Textures_in_use, bool mark = true);

// =========================
// LevelTexIncrementTexture
// =========================
//
// Call this function whenever a texture is applied to a face in the level
// _EVERYTIME_ it is applied to a face.  Make sure it is called on level load
void LevelTexIncrementTexture(int texnum);

// =========================
// RoomTexIncrementTexture
// =========================
//
// Call this function whenever a texture is applied to a face in the room
// _EVERYTIME_ it is applied to a face.  Make sure it is called on room (ORF) load
//@@void RoomTexIncrementTexture(int texnum, short *Textures_in_use, bool mark = true);

// ==========================
// LevelObjInitLevel
// ==========================
//
// Call this as soon as a level is loaded, it will go through the level and set it up.
void LevelObjInitLevel(void);

// ==========================
// LevelObjPurgeAllObjects
// ==========================
//
// Call this when a level is being unloaded to purge all objects associated with it
void LevelObjPurgeAllObjects(void);

// ===========================
// LevelObjDecrementObject
// ===========================
//
// Call this function when an object is being removed from the level
void LevelObjDecrementObject(int objnum);

// =========================
// LevelObjIncrementObject
// =========================
//
// Call this function whenever an object is added to a level
// _EVERYTIME_ it is added to a level.  Make sure it is called on level load
void LevelObjIncrementObject(int objnum);

//Object types
#define OBJ_NONE				255	//unused object
#define OBJ_WALL				0		//A wall... not really an object, but used for collisions
#define OBJ_FIREBALL			1		//a fireball, part of an explosion
#define OBJ_ROBOT				2		//an evil enemy
#define OBJ_SHARD				3		//a piece of glass
#define OBJ_PLAYER			4		//the player on the console
#define OBJ_WEAPON			5		//a laser, missile, etc
#define OBJ_VIEWER			6		//a viewed object in the editor
#define OBJ_POWERUP			7		//a powerup you can pick up
#define OBJ_DEBRIS			8		//a piece of robot
#define OBJ_CAMERA			9		//a camera object in the game
#define OBJ_SHOCKWAVE		10		//a shockwave
#define OBJ_CLUTTER			11		//misc objects
#define OBJ_GHOST				12		//what the player turns into when dead
#define OBJ_LIGHT				13		//a light source, & not much else
#define OBJ_COOP				14		//a cooperative player object.
#define OBJ_MARKER			15		//a map marker
#define OBJ_BUILDING			16		//a building
#define OBJ_DOOR				17		//a door
#define OBJ_ROOM				18		//a room, visible on the terrain
#define OBJ_PARTICLE			19		//a particle
#define OBJ_SPLINTER			20		//a splinter piece from an exploding object
#define OBJ_DUMMY				21		//a dummy object, ignored by everything
#define OBJ_OBSERVER			22		//an observer in a multiplayer game
#define OBJ_DEBUG_LINE		23		//something for debugging, I guess.  I sure wish people would add comments.
#define OBJ_SOUNDSOURCE		24		//an object that makes a sound but does nothing else
#define OBJ_WAYPOINT			25		//a object that marks a waypoint
#define MAX_OBJECT_TYPES	26		//Update this when adding new types

// =========================
// LevelDoorIncrementDoorway
// =========================
//
// Call this function whenever a doorway is added to a level
// _EVERYTIME_ it is added to a level.  Make sure it is called on level load
void LevelDoorIncrementDoorway(int doornum);

// ===========================
// LevelDoorDecrementDoorway
// ===========================
//
// Call this function when a doorway is being removed from the level.
void LevelDoorDecrementDoorway(int doornum);

// ==========================
// LevelDoorPurgeAllDoorways
// ==========================
//
// Call this when a level is being unloaded to purge all doorways associated with it
void LevelDoorPurgeAllDoorways(void);

// ==========================
// LevelDoorInitLevel
// ==========================
//
// Call this as soon as a level is loaded, it will go through the level and set it up.
void LevelDoorInitLevel(void);


#if !defined(AFX_STDAFX_H__A96779E5_E43B_11D2_8F49_00104B27BFF0__INCLUDED_)
#error "You must include stdafx.h before globals.h"
#endif

class CTextureOLETarget : public COleDropTarget
{
public:
protected:
	DROPEFFECT OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );	
	DROPEFFECT OnDropEx( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point );
	void OnDragLeave( CWnd* pWnd );
	void DrawText(int x,int y,CWnd *pWnd,char *text);
};

class CGenericOLETarget : public COleDropTarget
{
public:
protected:
	DROPEFFECT OnDragOver( CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point );	
	DROPEFFECT OnDropEx( CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point );
	void OnDragLeave( CWnd* pWnd );
	void DrawText(int x,int y,CWnd *pWnd,char *text);
};

#define MODE_VERTEX	0
#define MODE_FACE	1
#define MODE_OBJECT	2
#define MODE_PATH	3
#define NUM_MODES	4

#define NODETYPE_GAME	0
#define NODETYPE_AI		1

class CEditorState
{
public:
	int mode;
	float texscale;
	int nodetype;
	int path;
	int node;
	int bn_room;
	int bn_node;
	int bn_edge;

	CEditorState();
	~CEditorState();

	//Initializes the Editor State
	void Initialize(void);

	//Shutsdown the Editor State
	void Shutdown(void);

	//Attaches the desktop grSurface to a window
	void AttachDesktopSurface(unsigned hWnd);

	//Clears the desktop surface
	void DesktopClear(int x,int y,int w,int h);

	//Sets up texture dragging
	bool RegisterTextureDropWindow(CWnd *wnd);

	//writes a texture list to an open file
	bool WriteTextureList(CFILE *outfile,int list);

	//reads a texture list from an open file
	int ReadTextureList(CFILE *infile);

	//sets the current room/face for the texture view
	void SetCurrentRoomFaceTexture(int room,int face);

	// gets the current texture id
	int GetCurrentTexture();

	// grabs the texture of the currently focused window and makes it the current texture
	int GrabTexture();

	//writes an object list to an open file
	bool WriteObjectList(CFILE *outfile,int list);

	//reads an object list from an open file
	int ReadObjectList(CFILE *infile);

	//sets the current level object for the object view
	void SetCurrentLevelObject(int object);

	// gets the current object id
	int GetCurrentObject();

	//sets the current doorway
	void SetCurrentDoorway(room *rp);

	// gets the current door id
	int GetCurrentDoor();

	//Sets up object dragging
	bool RegisterGenericDropWindow(CWnd *wnd);

	//sets the current object for the object view
	void SetCurrentObject(int object);

	// gets the current node type (gamepath node, bnode)
	int GetCurrentNodeType();

	// sets the current node type
	void SetCurrentNodeType(int type);

	// gets the current path
	int GetCurrentPath();

	// gets the current node
	int GetCurrentNode();

	// sets the current path
	void SetCurrentPath(int p);

	// gets the current node
	void SetCurrentNode(int n);

	// BNodes
	void GetCurrentBNode(int *r,int *n,int *e);
	void SetCurrentBNode(int r,int n,int e);

private:
	CTextureOLETarget m_TextureDrop;
	CGenericOLETarget m_GenericDrop;
};
extern CEditorState Editor_state;

int FindTextureName(char *p);
int FindDoorName(char *p);

#include "viseffect.h"

extern vis_effect *VisEffects;

// Renders a vis effect
void DrawVisEffect (vis_effect *vis);

#ifdef RELEASE
int OutrageMessageBox(int type, char *str, ...);
void OutrageMessageBox(char *str, ...) ;

#endif

int osipf_FindSoundName(char *name);
int osipf_FindRoomName(char *name);
int osipf_FindTriggerName(char *name);
int osipf_FindObjectName(char *name);
int osipf_GetTriggerRoom(int trigger_id);
int osipf_GetTriggerFace(int trigger_id);
int osipf_FindDoorName(char *name);
int osipf_FindTextureName(char *name);
int osipf_FindMatcenName(char *name);
int osipf_FindPathName(char *name);
int osipf_FindLevelGoalName(char *name);

void ClearTerrainSound();

#endif