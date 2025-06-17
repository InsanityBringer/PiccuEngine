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
 

/*	D3 Editor Globals */

#ifndef _D3EDIT_H
#define _D3EDIT_H

#include "pstypes.h"
#include "descent.h"
#include "vecmat.h"

#include <stdlib.h>

//For putting up editor messageboxes in the main body (& should be in the editor code too, probably)
#ifdef EDITOR
#define EditorMessageBox OutrageMessageBox
#endif

//Define group & room structs so we don't have to include group.h & room.h
typedef struct group group;
typedef struct room room;

const int	TEXSCREEN_WIDTH		= 512,	// Texture screen base width and height
			TEXSCREEN_HEIGHT	= 384,
			WIRESCREEN_WIDTH	= 640,	// Wirefrane screen base width and height
			WIRESCREEN_HEIGHT	= 480;

const int	EDITOR_RESOLUTION_X	= 1024,	// Used to try to make editor resolution independent
			EDITOR_RESOLUTION_Y	= 768;

int CALC_PIXELS_WITH_ASPECTX(int pixels);
int CALC_PIXELS_WITH_ASPECTY(int pixels);


//	keypad constants
enum {	TAB_TEXTURE_KEYPAD,
			TAB_MEGACELL_KEYPAD,
			TAB_TERRAIN_KEYPAD,
			TAB_OBJECTS_KEYPAD,
			TAB_ROOM_KEYPAD,
			TAB_DOORWAY_KEYPAD,
			TAB_TRIGGER_KEYPAD,
			TAB_LIGHTING_KEYPAD,
			TAB_PATHS_KEYPAD,
			TAB_LEVEL_KEYPAD,
			TAB_MATCEN_KEYPAD};

//Constants for d3edit_state variables
enum {IN_WINDOW,ACROSS_EDGE};									//values for box_selection_mode
enum {REL_OBJECT,REL_VIEWER};									//values for object_move_mode
enum {GM_WINDOWED,GM_FULLSCREEN_SW,GM_FULLSCREEN_HW};	//values for game_render_mode

class grSurface;
class grViewport;

//Structure to store various editor state & preference values
typedef struct d3edit_state {

	// Values for current item in the various dialogs
	int	texdlg_texture;						// current texture in texdialog
	int	current_obj_type;					  	// current type of object
	int	current_powerup;						// current powerup id
	int	current_door;							// current door in door page dialog
	int	current_robot;						  	// current robot in robot page dialog
	int	current_ship;							// current ship in ship page dialog
	int	current_sound;						  	// current sound in sound page dialog
	int	current_weapon;						// current weapon in weapon page dialog
	int	current_path;							// currently selected path for a robot to follow
	int	current_node;							// currently selected node of preceding path
	int	current_megacell;					  	// currently selected megacell
	int	current_room;							// currently selected room
	int	current_gamefile;						// currently selected gamefile
	int	current_building;						// currently selected building
	int	current_clutter;						// currently selected clutter

	//	Values for the different editor windows
	bool	texscr_visible;						// is texture mine view up?
	int	texscr_x, texscr_y, 
			texscr_w, texscr_h;					// dims of floating texture mine view

	bool	wirescr_visible;					  	// is wireframe model up?
	int	wirescr_x, wirescr_y, 
			wirescr_w, wirescr_h;				// dims of floating wireframe model

	bool	keypad_visible;						// is keypad visible?
	int	keypad_current;						// which keypad tab are we on?
	bool	float_keypad_moved;				  	// has floating keypad moved?
	int	float_keypad_x, float_keypad_y, 
			float_keypad_w, float_keypad_h;	// floating keypad width and height, x, y
	int	objmodeless_x, objmodeless_y;		// object modeless list x and y.
	bool	objmodeless_on;						// is modeless on?

	bool	tile_views;								// tile or floating view windows, keypad

	//Values for terrain renderer
	bool	terrain_dots;						  	// show terrain dots?
	bool	terrain_flat_shade;				  	// flat shade terrain?

	//Misc preferences
	int	game_render_mode;						// what mode to we play the game in?  See constants above.
	bool	randomize_megacell;				  	// randomize when placing a megacell?
	int	box_selection_mode; 					// How editor box selection works.  See constants above.
	int	object_move_mode;						// How object movements works.  See constants above.
	int	object_move_axis;						// This is the axis on which objects move with mouse.
	bool	fullscreen_debug_state;				// do we allow for fullscreen debugging?
	bool	hemicube_radiosity;
	float	node_movement_inc;
	int	texture_display_flags;				// which textures to display on the texture tab
	float texscale;								// the scalar for moving texture UVs
	bool	joy_slewing;							// shall we allow joystick slewing?
	bool	objects_in_wireframe;				// should we draw objects in the wireframe view?

} d3edit_state;

//	Editor.cpp:: Current state of the editor UI.
extern d3edit_state D3EditState;

//	Editor.cpp:: Surface describing the actual desktop where the editor is running.
extern grSurface *Desktop_surf;			

//flags for the textured views changed
extern int TV_changed;

//Set this flag if a new world is loaded/created
extern int New_mine;

//Set this when the mine has changed
extern int World_changed;

//Set this when the editor state (but not the world itself) has changed
extern int State_changed;

//Set this when the viewer (i.e., player) has moved
extern int Viewer_moved;

//Set this when an object has moved
extern int Object_moved;

//Set this when the editor viewpoint has changed
extern int Edview_changed;

//Current room & face
extern room *Curroomp;
extern int Curface,Curedge,Curvert;
extern int Curportal;

//Current object 
extern int Cur_object_index;

//Marked room & face
extern room *Markedroomp;
extern int Markedface,Markededge,Markedvert;

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

//Vars for the list of selected rooms
extern int N_selected_rooms;
extern int Selected_rooms[];

//Flag for if mine has changed (& thus needs to be saved)
extern int Mine_changed;

//	Current trigger in mine displayed in trigger dialog
extern int Current_trigger;

// The scrap buffer
extern group *Scrap;

//Pointer to the scripts for this level
extern char *Current_level_script;

// What mode we're currently in
enum {VM_MINE,VM_TERRAIN,VM_ROOM,NUM_VIEW_MODES};
extern int Editor_view_mode;

//The ID of the most recent viewer object (not counting room view)
extern int Editor_viewer_id;

//	current directories for file dialogs.
extern char Current_files_dir[_MAX_PATH];
extern char Current_bitmap_dir[_MAX_PATH];
extern char Current_scrap_dir[_MAX_PATH];
extern char Current_room_dir[_MAX_PATH];
extern char Current_model_dir[_MAX_PATH];
extern char Current_sounds_dir[_MAX_PATH];
extern char Current_weapon_dir[_MAX_PATH];

//	object id clipboard.
extern int Copied_object_id;


//	FUNCTIONS
void EditorStatus(const char *format,...);
void SplashMessage(const char *format, ...);
void StartEditorFrame(grViewport *vp, vector *view_vec, matrix *id_mat, float zoom);
void EndEditorFrame();

//Set the editor error message.  A function that's going to return a failure
//code should call this with the error message.
void SetErrorMessage(char *fmt,...);

//Get the error message from the last function that returned failure
char *GetErrorMessage();

#endif