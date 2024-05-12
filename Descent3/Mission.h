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

#ifndef MISSION_H
#define MISSION_H

#include "pstypes.h"
#include "descent.h"

//	*** CONSTANTS ***
#define LOAD_PROGRESS_START				1
#define LOAD_PROGRESS_LOADING_LEVEL		2
#define LOAD_PROGRESS_PAGING_DATA		3
#define LOAD_PROGRESS_PREPARE			4
#define LOAD_PROGRESS_DONE				200

//Load level progress worker
void LoadLevelProgress(int step,float percent,char *chunk=NULL);

//	array constants
const int	MSN_FILENAMELEN	= PSPATHNAME_LEN,
			MSN_URLLEN = 256;

#define MAX_KEYWORDLEN	300

//	increase this value if you are going to add more levels to a mission than the max.
const int	MAX_LEVELS_PER_MISSION = 30;

//	mission flags.
const unsigned LVLFLAG_STARTMOVIE = 1,
				LVLFLAG_ENDMOVIE = 2,
				LVLFLAG_BRIEFING = 4,
				LVLFLAG_SHIPSELECT = 8,
				LVLFLAG_SPAWNSECRET = 16,
				LVLFLAG_SPECIALHOG = 32,
				LVLFLAG_BRANCH = 64,
				LVLFLAG_UNUSED = 128,
				LVLFLAG_SCORE = 256,
				LVLFLAG_FINAL = 512;


const int LVLOBJ_NUM = 4;
const ushort LVLOBJF_SECONDARY1 = 1,
				LVLOBJF_SECONDARY2 = 2,
				LVLOBJF_SECONDARY3 = 4,
				LVLOBJF_SECONDARY4 = 8;

//Struct for info about the current level
struct level_info 
{
	char name[100];
	char designer[100];
	char copyright[100];
	char notes[1000];
};

//Info about the current level
extern level_info Level_info;

//	level information
struct tLevelNode 
{
//	level flags
	unsigned flags;							// level flags
	unsigned objective_flags;				// level objective flags

//	movies
	char *moviename;
	char *endmovie;

//	level filename
	char *filename;							// mine filename.
	char *briefname;						// briefing filename
	char *hog;								// hog file name for this level
	char *score;							// music score of level
	char *progress;							// File name containing the progress background screen
	
//	level branching
	ubyte lvlbranch0, lvlbranch1;			// FORK or BRANCH command
	ubyte secretlvl;						// SECRET command
	ubyte pad;	
};


// predefine mission state flags
#define MSN_STATE_SECRET_LEVEL			0x80000000

struct tMission 
{
	int mn3_handle;							// if this mission was loaded from an MN3, this is the handle.
	
	unsigned game_state_flags;				// game state information stored here (manipulated by scripting)
	char *filename;							// filename of mission.
	char name[MSN_NAMELEN];					// name of mission
	char author[MSN_NAMELEN];				// author of mission.
	char desc[MSN_NAMELEN*4];				// description of mission
	char *hog;								// default hog filename
	char email[MSN_URLLEN];				 	// email and web location
	char web[MSN_URLLEN];					
	bool multiplayable;						// this level is multiplayer playable
	bool singleplayable;					// this level is playable as a single player game
	bool training_mission;					// is this mission a training mission?
	
//	listing of levels.
	int num_levels;							// number of levels
	int cur_level;							// current level playing.
	tLevelNode *levels;						// array of levels
};


// structyre used to get information about a mission 
struct tMissionInfo
{
	char name[MSN_NAMELEN];
	char author[MSN_NAMELEN];
	char desc[MSN_NAMELEN*4];
	bool multi;
	bool single;
	bool training;
	int n_levels;
	char keywords[MAX_KEYWORDLEN];			// Keywords for mods, so you can see if this mission supports a given mod
};

//	Scripting information

//	the current mission being played.
extern tMission Current_mission;			
extern tLevelNode *Current_level;

extern char D3MissionsDir[];

//Get the name field out of the mission file
char * GetMissionName(char *mission);

//	initializes mission system.
void InitMission();

//	reset all states for a mission
void ResetMission();

//	loads and verifies a mission as the current mission, returns if valid of not.
bool LoadMission(const char *msn);

//	initializes a level's script.
void InitLevelScript();

//	frees a level's script
void FreeLevelScript();

//	does a mission briefing, returns if mission was canceled, a false, or 0 value.
bool DoMissionBriefing(int level);

//	does the end mission stuff
void DoEndMission();

//	does the mission movie stuff
void DoMissionMovie(char *movie);

//	loads a level and sets it as current level in mission
bool LoadMissionLevel(int level);

//	initializes the mission script
bool InitMissionScript();

//	Objectives
void CompletedPrimaryObjective();

// Shows text on a background
void ShowProgressScreen (char *str,char *str2=NULL,bool flip=true);

// See if a mission file is multiplayer playable.
bool IsMissionMultiPlayable(char *mission);

//	return information about a mission
bool GetMissionInfo(const char *msnfile, tMissionInfo *msn);

//Returns the max number of teams this mission can support for this mod, or 
//-1 if this level shouldn't be played with this mission
//Return values:
// -1	Bad match -- this level and this mod shouldn't be played together!
// MAX_NET_PLAYERS	-- This is playable with any number of teams the mod wants
int MissionGetKeywords(char *mission,char *keywords);

#ifdef EDITOR
//	Used by editor to load all necessary elements for level playing for systems
//	not initialized in editor, but only in game.
void QuickStartMission();

// Used by game->editor to free up all extra game mission elements not needed in editor.
void QuickEndMission();

#endif

#endif
