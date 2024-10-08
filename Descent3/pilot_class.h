/* 
* Descent 3 
* Copyright (C) 2024 Parallax Software
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __PILOT_CLASS_H_
#define __PILOT_CLASS_H_

#include "pstypes.h"
#include "controls.h"
#include "Controller.h"
#include "CFILE.H"
#include "weapon.h"
#include "config.h"

//YUCK!
#include "descent.h"	//just for MSN_NAMELEN


/*
=======================================================================

IIIIIIIII MM     MM PPPP   OOOO  RRRR  TTTTTTT   AA   NN   NN TTTTTTT
   II     MM     MM P   P O    O R   R    T      AA   NNN  NN    T
   II     M M   M M P   P O    O R   R    T     A  A  NN N NN    T
   II     M  M M  M P  P  O    O R  R     T     A  A  NN N NN    T
   II     M  M M  M PPP   O    O RRR      T    AAAAAA NN  NNN    T
   II     M   M   M P     O    O R  R     T    A    A NN  NNN    T
IIIIIIIII N       N P      OOOO  R   R    T    A    A NN   NN    T

=======================================================================

  If you add new data members to the pilot class, you _must_ do the following:

  1) Add functionality to the pilot::initialize();
  2) Add functionality to the pilot::clean();
  3) Add functionality to the pilot::verify();
  4) Create a pilot::write_ function
  5) Create a corresponding pilot::read_ function
  6) Add to calls to your write_ and read_ functions in pilot::flush() and pilot::read()
*/

// Maximum size of the pilot name
#define PILOT_STRING_SIZE	20
// Invalid Pilot Pic ID
#define PPIC_INVALID_ID		65535
// should be put somewhere else, but here for the demo.
#define N_MOUSE_AXIS			2
#define N_JOY_AXIS				6

// Number of multiplayer taunts
#define MAX_PILOT_TAUNTS	8
// Maximum string length of a taunt
#define PILOT_TAUNT_SIZE	60	

//file error codes
#define PLTW_NO_ERROR		0	//there was no error
#define PLTW_NO_FILENAME	1	//no filename has been set
#define PLTW_FILE_EXISTS	2	//the file already exists
#define PLTW_FILE_CANTOPEN	3	//file couldn't be opened
#define PLTW_CFILE_FATAL	4	//a CFILE error had occurred
#define PLTW_UNKNOWN_FATAL	5	//an unknown exception occurred

#define PLTR_NO_ERROR		0	//there was no error
#define PLTR_NO_FILENAME	1	//no filename has been set
#define PLTR_FILE_NOEXIST	2	//the file doesn't exist to read
#define PLTR_FILE_CANTOPEN	3	//file couldn't be opened
#define PLTR_CFILE_FATAL	4	//a CFILE error had occurred
#define PLTR_UNKNOWN_FATAL	5	//an uknown exception occurred
#define PLTR_TOO_NEW		6	//pilot file too new

struct tMissionData
{
	ubyte highest_level;			// highlest level completed in the mission
	int ship_permissions;			// Ship permissions at highest level achieved
	bool finished;					// was mission finished? (different than highest level,btw)
	char mission_name[MSN_NAMELEN];	// name of the mission (from the mission file)
	int num_restores;				// number of game loads for this mission
	int num_saves;					// number of game saves for this mission
};


struct cntrldata
{
	int id;
	ct_type type[2];
	ct_config_data value;
	ubyte flags[2];
};

class pilot
{

public:
	~pilot();
	pilot();
	pilot(pilot *copy);
	pilot(char *fname);

	// This function guts the data so it's virgin (fresh for reading)
	// frees any memory that needs to be freed, etc.
	// if reset is true, it resets all data to what it is at initialization
	// else it is an unknown state
	void clean(bool reset=true);

	// This function verifies all the pilot data, making sure nothing is out of whack
	// It will correct any messed data.
	void verify(void);

	// This function makes the pilot file so it's write pending, meaning that
	// on the next call to flush, it will actually write out the data.  There is
	// no need to constantly do file access unless it's really needed
	void write(void);

	// This function makes certain that the pilot data is up to date with the 
	// actual pilot file, writing if needed.
	int flush(bool new_file);

	// This function sets the filename that is associated with this pilot
	void set_filename(char *fname);
	void get_filename(char *fname);

	// This function reads in the data from file (from the filename associated)
	// into the pilot data.
	int read(bool skip_control_set,bool skip_mission_data);

	// [ISB] Commits the pilot's state to global state.
	// Used to work around the problems where merely reading a pilot file would commit global state.
	void commit_state() const;

	// [ISB] Gets the state from global
	void fetch_state();

public:
	// data access functions
	void set_name(char *name);
	void get_name(char *name);

	void set_ship(char *ship);
	void get_ship(char *ship);

	void set_multiplayer_data(char *logo=NULL,char *audio1=NULL,char *audio2=NULL,ushort *ppic=NULL,char *audio3=NULL,char *audio4=NULL);
	void get_multiplayer_data(char *logo=NULL,char *audio1=NULL,char *audio2=NULL,ushort *ppic=NULL,char *audio3=NULL,char *audio4=NULL);

	void set_difficulty(ubyte diff);
	void get_difficulty(ubyte *diff);

	void set_hud_data(ubyte *hmode=NULL,ushort *hstat=NULL,ushort *hgraphicalstat=NULL,int *gw_w=NULL,int *gw_h=NULL);
	void get_hud_data(ubyte *hmode=NULL,ushort *hstat=NULL,ushort *hgraphicalstat=NULL,int *gw_w=NULL,int *gw_h=NULL);

	void set_profanity_filter(bool enable);
	void get_profanity_filter(bool *enabled);

	void set_audiotaunts(bool enable);
	void get_audiotaunts(bool *enabled);

	void set_guidebot_name(char *name);
	void get_guidebot_name(char *name);

	void add_mission_data(tMissionData *data);
	void edit_mission_data(int index,tMissionData *data);
	void get_mission_data(int index,tMissionData *data);
	int find_mission_data(char *mission_name);

private:
	void initialize(void);	//initializes all the data (for constructors)
	bool write_pending;	//data has changed and pilot data is out of sync with file
private:	
	// internal file access functions
	void write_name(CFILE *file);
	void write_ship_info(CFILE *file);
	void write_custom_multiplayer_data(CFILE *file);
	void write_difficulty(CFILE *file);
	void write_hud_data(CFILE *file);
	void write_mission_data(CFILE *file);
	void write_taunts(CFILE *file);
	void write_weapon_select(CFILE *file);
	void write_controls(CFILE *file);
	void write_profanity_filter(CFILE *file);
	void write_audiotaunts(CFILE *file);
	void write_gameplay_toggles(CFILE *file);
	void write_guidebot_name(CFILE *file);

	// for the read functions, skip is true if the data should actually
	// just be skipped and not processed
	int file_version;
	void read_name(CFILE *file,bool skip);
	void read_ship_info(CFILE *file,bool skip);
	void read_custom_multiplayer_data(CFILE *file,bool skip);
	void read_difficulty(CFILE *file,bool skip);
	void read_hud_data(CFILE *file,bool skip);
	void read_mission_data(CFILE *file,bool skip);
	void read_taunts(CFILE *file,bool skip);
	void read_weapon_select(CFILE *file);
	void read_controls(CFILE *file,bool skip);
	void read_profanity_filter(CFILE *file,bool skip);
	void read_audiotaunts(CFILE *file,bool skip);
	void read_gameplay_toggles(CFILE *file,bool skip);
	void read_guidebot_name(CFILE *file,bool skip);

private:
	//--- Pilot data				---//
	//--- Try to preserve alignment	---//
	char *filename;		//filename location of this pilot
	char *name;			//name of the pilot (used in the game)
	char *ship_logo;	//ship logo for multiplayer play (filename)
	char *ship_model;	//what ship does this pilot fly
	char *audio1_file;	//audio taunt #1 (filename)
	char *audio2_file;	//audio taunt #2 (filename)	
	char *audio3_file;	//audio taunt #1 (filename)
	char *audio4_file;	//audio taunt #2 (filename)	
	char *guidebot_name;	//guidebot name

	ushort picture_id;	//pilot picture image id
	ubyte difficulty;	//difficulty setting for this pilot (DIFFICULTY_*)
	ubyte hud_mode;		// hud display mode
	bool profanity_filter_on,audiotaunts;

	ushort hud_stat;	// hud layout using the STAT mask
	ushort hud_graphical_stat;	
	
	int game_window_w, game_window_h;	//game window size

	int num_missions_flown;		//number of mission's flown
	tMissionData *mission_data;	//mission data

	ushort PrimarySelectList[MAX_PRIMARY_WEAPONS];
	ushort SecondarySelectList[MAX_SECONDARY_WEAPONS];

	tGameToggles gameplay_toggles;	// special options in config menu.

public:

	char taunts[MAX_PILOT_TAUNTS][PILOT_TAUNT_SIZE];// taunt macros

	cntrldata controls[NUM_CONTROLLER_FUNCTIONS];//controller settings
	float mouse_sensitivity[N_MOUSE_AXIS];	// axis sensitivities
	float joy_sensitivity[N_JOY_AXIS];		// axis sensitivities
	float key_ramping;
	char read_controller;	// do we read the controller port also (beyond keyboard/mouse)	
	bool mouselook_control;		// mouselook control.
	bool lrearview_enabled;
	bool rrearview_enabled;		// are these small views enabled?
	
	ubyte ingame_difficulty;	//DAJ for optimization
};

#endif
