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

#ifndef __PILOT_H_
#define __PILOT_H_

#include <stdlib.h>
#include "pilot_class.h"

/*
#include "controls.h"
#include "Controller.h"
#include "ship.h"

#define PILOT_STRING_SIZE	20
#define PPIC_INVALID_ID		65535

//errors reported by PltWriteFile()
#define PLT_FILE_FATAL			-3	//can't open file, possibly out of hd space
#define PLT_FILE_CANT_CREATE	-2	//bad filename, most likely 0 length string for filename
#define PLT_FILE_EXISTS			-1	//file already exists (will only return this if newpilot=true on PltWriteFile
#define PLT_FILE_NOERR			0	//no errors, success

// should be put somewhere else, but here for the demo.
#define N_MOUSE_AXIS				2
#define N_JOY_AXIS				6

#define MAX_PILOT_TAUNTS	8
#define PILOT_TAUNT_SIZE	60	

typedef struct
{
	int id;
	ct_type type[2];
	ct_config_data value;
	ubyte flags[2];
} cntrldata;

typedef struct
{
	ubyte highest_level;
	bool finished;									// was mission finished? (different than highest level,btw)
	char mission_name[MSN_NAMELEN];
} tMissionData;

typedef struct
{
	char filename[_MAX_FNAME];	//added because of some weird .plt renaming bugs that can happen
										//not saved out
	//Name of the Pilot
	char name[PILOT_STRING_SIZE];
	//number of kills and number of times killed
	int kills,deaths;
	//name of ship model
	char ship_model[PAGENAME_LEN];
	//filename of custom texture
	char ship_logo[PAGENAME_LEN];
	//filename of audio taunt 1 (including CRC)
	char audio1_file[PAGENAME_LEN];
	//filename of audio taunt 2 (including CRC)
	char audio2_file[PAGENAME_LEN];
	//difficulty setting (DIFFICULTY_*)
	char difficulty;
	//controller settings
	cntrldata controls[NUM_CONTROLLER_FUNCTIONS];
	// hud layout using the STAT mask
	ushort hud_stat;
	ushort hud_graphical_stat;
	// hud display mode	
	ubyte hud_mode;
	// do we read the controller port also (beyond keyboard/mouse)
	char read_controller;
	// axis sensitivities
	float mouse_sensitivity[N_MOUSE_AXIS], joy_sensitivity[N_JOY_AXIS];

	int game_window_w, game_window_h;

	//pilot picture image id
	ushort picture_id;

	//number of mission's flown
	int num_missions_flown;
	tMissionData *mission_data;

	// taunt macros
	char taunts[MAX_PILOT_TAUNTS][PILOT_TAUNT_SIZE];

}pilot;
*/

//	PilotSelect
//
//	Brings up a window where you can select a pilot to use
void PilotSelect(void);

//Display the pilot display/selection screen
//void PilotDisplay(bool forceselection = false);

//brings up the configuration screen for a pilot
// Returns: true on success
// Pilot: pointer to pilot to configure
// newpilot: true if this is a new pilot, false if an existing pilot
bool PilotConfig(pilot *Pilot,bool newpilot,bool forceok = false);

//creates a new pilot (and pilot file) (if you call this you should call PilotConfig() after it)
// Returns: true on success
// Pilot: pointer to pilot structure to be filled in with name and filename (should then configure the rest)
bool PilotCreate(pilot *Pilot,bool dontallowcancel);

//copies a pilot to another
bool PilotCopy(pilot *Src,pilot *Dest);

//Write a Pilot out to file
// Pilot - Pilot to write (filename field is root of filename extension .plt)
//			Make sure both filename and name are filled in before calling
// newpilot - Whether it is supposed to create a new file
//				false = overwrite existing file
//				true = don't overwrite any existing file, returns error if so
// Returns: PLT_FILE_*
int PltWriteFile(pilot *Pilot,bool newpilot=false);

//Reads a Pilot from file
// Pilot - pointer to structure to fill in...
//			MAKE SURE filename field is filled in with correct filename to read in before calling!!
// keyconfig - whether to set the controls on load
void PltReadFile(pilot *Pilot,bool keyconfig=false,bool missiondata=false);

//Given a string it will make a valid filename out of it
void PltMakeFNValid(char *name);

void PilotInit(void);

void PltClearList(void);
char **PltGetPilots(int *count,char *ignore_filename=NULL,int display_default_configs=0);
void PltGetPilotsFree(void);

// VerifyPilotData
//
//	Call this function to check the data that is in the given pilot struct...it will verify that all files
//	listed are available, if they are not, then it will set them to defaults. Returns true if it had to
//	fix the data (you may want to save the pilot immediatly)
bool VerifyPilotData(pilot *Pilot);

// updates the current pilot's information (level played, mission played, etc)
//	call after every successful mission completion (by passing false)
//  call when a mission is loaded (pass true)
void CurrentPilotUpdateMissionStatus(bool just_add_data=false);

// gets highest level flown for mission
int PilotGetHighestLevelAchieved(pilot *Pilot,char *mission_name);
int GetPilotShipPermissions(pilot *Pilot,const char *mission_name);

bool HasPilotFinishedMission(pilot* Pilot, const char *mission_name);
bool HasPilotFlownMission(pilot *Pilot,const char *mission_name);

extern pilot Current_pilot;
extern char Default_pilot[_MAX_PATH];

// "Current Pilot" access functions
void dCurrentPilotName(char *buffer);
ubyte dCurrentPilotDifficulty(void);

void IncrementPilotRestoredGamesForMission(pilot *Pilot,const char *mission_name);
void IncrementPilotSavedGamesForMission(pilot *Pilot,const char *mission_name);
int GetPilotNumRestoredGamesForMission(pilot *Pilot,const char *mission_name);
int GetPilotNumSavedGamesForMission(pilot *Pilot,const char *mission_name);

#endif
