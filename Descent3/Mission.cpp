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

#include "Mission.h"
#include "3d.h"
#include "LoadLevel.h"
#include "pserror.h"
#include "CFILE.H"
#include "gamefont.h"
#include "grdefs.h"
#include "descent.h"
#include "ddio.h"
#include "movie.h"
#include "program.h"
#include "object.h"
#include "objinit.h"
#include "ObjScript.h"
#include "application.h"
#include "TelCom.h"
#include "game.h"
#include "cinematics.h"
#include "player.h"
#include "gamesequence.h"
#include "mem.h"
#include "newui.h"
#include "stringtable.h"
#include "AppConsole.h"
#include "pstring.h"
#include "dedicated_server.h"
#include "osiris_dll.h"
#include "mission_download.h"
#include "manage.h"
#include <string.h>
#include <stdlib.h>
#include "ship.h"
#include "BOA.h"
#include "terrain.h"
#include "multi.h"
#include "render.h"
#include "newrender.h"

//	---------------------------------------------------------------------------
//	Data
//	---------------------------------------------------------------------------
//Info about the current level
level_info Level_info;
tMission Current_mission;
tLevelNode* Current_level = NULL;
char D3MissionsDir[PSPATHNAME_LEN];
extern int Times_game_restored;
extern msn_urls Net_msn_URLs;

//	---------------------------------------------------------------------------
//	Function prototypes
//	---------------------------------------------------------------------------
bool InitMissionScript();
void DoEndMission();
void DoMissionMovie(char* movie);
void FreeMission();

//	MN3 based mission functions.
// loads the msn file from the mn3 file specified, specifies the hog and table file.
bool mn3_Open(const char* mn3file);
// returns mission information given the mn3 file.
bool mn3_GetInfo(const char* mn3file, tMissionInfo* msn);
// closes the current mn3 file
void mn3_Close();


inline bool IS_MN3_FILE(const char* fname)
{
	char name[PSFILENAME_LEN];
	char ext[PSFILENAME_LEN];
	ddio_SplitPath(fname, NULL, name, ext);
	return (strcmpi(ext, ".mn3") == 0) ? true : false;
}

inline char* MN3_TO_MSN_NAME(const char* mn3name, char* msnname)
{
	char fname[PSFILENAME_LEN];
	ddio_SplitPath(mn3name, NULL, fname, NULL);

	if (stricmp(fname, "d3_2") == 0)
	{
		strcpy(fname, "d3");
	}
	strcat(fname, ".msn");
	strcpy(msnname, fname);
	return msnname;
}


//	---------------------------------------------------------------------------
//	Functions
//	---------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////
//	High level mission stuff
///////////////////////////////////////////////////////////////////////////////
void InitMission()
{
	mprintf((0, "In InitMission()\n"));
	Current_mission.num_levels = 0;
	Current_mission.cur_level = 0;
	memset(Current_mission.desc, 0, sizeof(Current_mission.desc));
	memset(Current_mission.name, 0, MSN_NAMELEN);
	memset(Current_mission.author, 0, MSN_NAMELEN);
	memset(Current_mission.email, 0, MSN_URLLEN);
	memset(Current_mission.web, 0, MSN_URLLEN);
	Current_mission.hog = NULL;
	Current_mission.levels = NULL;
	Current_mission.filename = NULL;
	Current_mission.game_state_flags = 0;
	Current_mission.mn3_handle = 0;
	//	create add on mission directory
	ddio_MakePath(D3MissionsDir, LocalD3Dir, "missions", NULL);
	if (!ddio_DirExists(D3MissionsDir))
	{
		if (!ddio_CreateDir(D3MissionsDir))
			Error(TXT_MSN_FAILEDCREATEDIR);
	}
	atexit(ResetMission);
}

//	reset all states for a mission
void ResetMission()
{
	mprintf((0, "In ResetMission()\n"));
	FreeMission();
	Current_mission.num_levels = 0;
	Current_mission.cur_level = 0;
	memset(Current_mission.desc, 0, sizeof(Current_mission.desc));
	memset(Current_mission.name, 0, MSN_NAMELEN);
	memset(Current_mission.author, 0, MSN_NAMELEN);
	memset(Current_mission.email, 0, MSN_URLLEN);
	memset(Current_mission.web, 0, MSN_URLLEN);
	Current_mission.hog = NULL;
	Current_mission.levels = NULL;
	Current_mission.filename = NULL;
	Current_mission.game_state_flags = 0;

	// clear out old URLs from memory
	for (int a = 0; a < MAX_MISSION_URL_COUNT; a++)
	{
		Net_msn_URLs.URL[a][0] = '\0';
	}
}
#if (defined(OEM) || defined(DEMO))
bool DemoMission(int mode = 0)
{
	tMission* msn = &Current_mission;
	tLevelNode* lvls = (tLevelNode*)mem_malloc(sizeof(tLevelNode) * 5);
	msn->cur_level = 1;
	msn->num_levels = 1;
	msn->levels = lvls;

	msn->multiplayable = true;
	msn->singleplayable = true;

	memset(lvls, 0, sizeof(tLevelNode) * msn->num_levels);
	strncpy(msn->author, "Outrage", MSN_NAMELEN - 1);
	if (!mode)
	{
		strcpy(msn->name, "Descent 3: Sol Ascent");
		msn->num_levels = 5;
		memset(lvls, 0, sizeof(tLevelNode) * msn->num_levels);
#ifdef DEMO
		msn->multiplayable = false;
		strcpy(msn->name, "Descent 3: Demo2");
		msn->num_levels = 1;
		msn->filename = mem_strdup("d3demo.mn3");
#else
		strcpy(msn->name, "Descent 3: Sol Ascent");
		msn->num_levels = 5;
		msn->filename = mem_strdup("d3oem.mn3");
#endif
		//strncpy(lvls[0].name, "Search for Sweitzer", MSN_NAMELEN-1);
		lvls[0].flags = LVLFLAG_BRIEFING | LVLFLAG_SCORE;
		lvls[0].filename = mem_strdup("level1.d3l");
		lvls[0].briefname = mem_strdup("level1.brf");
		lvls[0].score = mem_strdup("level1.omf");
		lvls[0].progress = mem_strdup("l1load.ogf");
		lvls[0].moviename = mem_strdup("level1.mve");
#ifndef DEMO
		//strncpy(lvls[0].name, "Into the Heart of the Ship", MSN_NAMELEN-1);
		lvls[1].flags = LVLFLAG_BRIEFING | LVLFLAG_SCORE;
		lvls[1].filename = mem_strdup("level3.d3l");
		lvls[1].briefname = mem_strdup("level2o.brf");
		lvls[1].score = mem_strdup("level3.omf");
		lvls[1].progress = mem_strdup("l3load.ogf");
		//strncpy(lvls[0].name, "The Nomad Caves", MSN_NAMELEN-1);
		lvls[2].flags = LVLFLAG_BRIEFING | LVLFLAG_SCORE;
		lvls[2].filename = mem_strdup("level6.d3l");
		lvls[2].briefname = mem_strdup("level3o.brf");
		lvls[2].score = mem_strdup("level6.omf");
		lvls[2].progress = mem_strdup("l6load.ogf");
		//strncpy(lvls[0].name, "The Transmode Virus", MSN_NAMELEN-1);
		lvls[3].flags = LVLFLAG_BRIEFING | LVLFLAG_SCORE;
		lvls[3].filename = mem_strdup("level7.d3l");
		lvls[3].briefname = mem_strdup("level4o.brf");
		lvls[3].score = mem_strdup("level7.omf");
		lvls[3].progress = mem_strdup("l7load.ogf");
		//strncpy(lvls[0].name, "The Rescue", MSN_NAMELEN-1);
		lvls[4].flags = LVLFLAG_BRIEFING | LVLFLAG_SCORE;
		lvls[4].filename = mem_strdup("level11.d3l");
		lvls[4].briefname = mem_strdup("level5o.brf");
		lvls[4].score = mem_strdup("level11.omf");
		lvls[4].progress = mem_strdup("l11load.ogf");
		mn3_Open("d3oem.mn3");
#endif

	}

	else if (mode == 1)
	{
		strcpy(msn->name, "Polaris");
		msn->filename = mem_strdup("Polaris.d3l");
		//strncpy(lvls[0].name, "Polaris", MSN_NAMELEN-1);
		lvls[0].filename = mem_strdup("polaris.d3l");
		lvls[0].flags |= LVLFLAG_BRANCH;
		lvls[0].lvlbranch0 = 1;
		lvls[0].progress = mem_strdup("polaris.ogf");
	}
	else if (mode == 2)
	{
		strcpy(msn->name, "The Core");
		msn->filename = mem_strdup("TheCore.d3l");
		//strncpy(lvls[0].name, "The Core", MSN_NAMELEN-1);
		lvls[0].filename = mem_strdup("thecore.d3l");
		lvls[0].flags |= LVLFLAG_BRANCH;
		lvls[0].lvlbranch0 = 1;
		lvls[0].progress = mem_strdup("thecore.ogf");
	}
	else if (mode == 3)
	{
		strcpy(msn->name, "Taurus");
		msn->filename = mem_strdup("Taurus.d3l");
		//strncpy(lvls[0].name, "Taurus", MSN_NAMELEN-1);
		lvls[0].filename = mem_strdup("taurus.d3l");
		lvls[0].flags |= LVLFLAG_BRANCH;
		lvls[0].lvlbranch0 = 1;
		lvls[0].progress = mem_strdup("taurus.ogf");
	}
#ifndef DEMO
	else if (mode == 4)
	{
		strcpy(msn->name, "Pilot Training");
		msn->filename = mem_strdup("training.mn3");
		//strncpy(lvls[0].name, "Training", MSN_NAMELEN-1);
		lvls[0].filename = mem_strdup("trainingmission.d3l");
		lvls[0].briefname = NULL;//mem_strdup("training.brf");
		lvls[0].flags = 0;
		lvls[0].lvlbranch0 = 0;
		lvls[0].progress = mem_strdup("trainingload.ogf");
		mn3_Open("training.mn3");
	}
#endif
	//	load default script here.
	InitMissionScript();
	return true;
}
#endif

bool LoadMission(const char* mssn)
{
	Times_game_restored = 0;
	mprintf((0, "In LoadMission()\n"));
	//	ShowProgressScreen(TXT_LOADINGLEVEL);
#if (defined(OEM) || defined(DEMO))
#ifdef OEM
	if (strcmpi(mssn, "d3oem.mn3") == 0)
		return DemoMission(0);
#elif defined(DEMO)
	if (strcmpi(mssn, "d3demo.mn3") == 0)
		return DemoMission(0);
#endif
	else if (strcmpi(mssn, "polaris.d3l") == 0)
		return DemoMission(1);
	else if (strcmpi(mssn, "thecore.d3l") == 0)
		return DemoMission(2);
	else if (strcmpi(mssn, "taurus.d3l") == 0)
		return DemoMission(3);
#ifdef OEM
	else if (strcmpi(mssn, "training.mn3") == 0)
		return DemoMission(4);
#endif
#else

//#endif
	tLevelNode* lvls = NULL;				// Temporary storage for level data.
	tMission* msn;
	CFILE* fp = NULL;							// Mission file
	char errtext[80];							// Stores error if unable to read mission
	char msnfname[PSFILENAME_LEN];
	char mission[_MAX_PATH * 2];
	int srclinenum = 0;							// Current line of source.
	int curlvlnum;								// Current level number
	int numlevels;								// Number of levels required to read in.
	int cur_objective;						// current objective reading.
	bool indesc;								// are we in a multi-line block
	bool res = false;							// used to specify if no error has occurred.
	char pathname[_MAX_PATH * 2];
	ResetMission();							// Reset everything.
	// open MN3 if filename passed was an mn3 file.

		//Correct for mission split hack

	if (strcmpi(mssn, "d3_2.mn3") == 0)
	{
		strcpy(mission, "d3_2.mn3");
		strcpy(pathname, "d3_2.mn3");
	}
	else if (strcmpi(mssn, "d3.mn3") == 0)
	{
		strcpy(mission, "d3.mn3");
		strcpy(pathname, "d3.mn3");
	}
	else if (IS_MN3_FILE(mssn))
	{
		strcpy(mission, mssn);
		ddio_MakePath(pathname, D3MissionsDir, mission, NULL);
	}
	else
	{
		strcpy(mission, mssn);
		strcpy(pathname, mssn);
		strcpy(msnfname, mssn);
		//ddio_MakePath(pathname, D3MissionsDir, mission, NULL);
	}
	if (IS_MN3_FILE(mission))
	{

		if (!mn3_Open(pathname))
		{
			strcpy(errtext, TXT_MSN_OPENMN3FAILED);
			goto msnfile_error;
		}
		MN3_TO_MSN_NAME(mission, msnfname);
	}
	//	open mission file
	fp = cfopen(msnfname, "rt");
	if (!fp)
	{
		strcpy(errtext, TXT_MISSIONNOTFOUND);
		goto msnfile_error;
	}
	msn = &Current_mission;

	//	read in mission file
	srclinenum = 1;
	numlevels = -1;
	curlvlnum = 0;
	indesc = 0;
	cur_objective = -1;
	msn->multiplayable = true;
	msn->singleplayable = true;
	msn->training_mission = false;

	while (!cfeof(fp))
	{
		char srcline[128];					// One line of mission source
		char command[32];						// Command read from line.
		char operand[96];						// operand read from line
		char* keyword;							// parsed keyword
		int readcount;							// read-in count
		readcount = cf_ReadString(srcline, sizeof(srcline), fp);
		if (readcount)
		{
			//	we have a line of source.  parse out primary keyword
			//	then parse out remainder.
			keyword = strtok(srcline, " \t");
			CleanupStr(command, srcline, sizeof(command));
			CleanupStr(operand, srcline + strlen(command) + 1, sizeof(operand));
			if (strlen(command) && indesc) indesc = 0;
			if (!strcmpi(command, "NAME"))
				strncpy(msn->name, operand, MSN_NAMELEN - 1);

			else if (!strcmpi(command, "MULTI"))
			{
				if (strcmpi("no", operand) == 0)
					msn->multiplayable = false;
			}
			else if (!strcmpi(command, "SINGLE"))
			{
				if (strcmpi("no", operand) == 0)
					msn->singleplayable = false;
			}
			else if (!strcmpi(command, "TRAINER"))
			{
				if (curlvlnum == 0)
					msn->training_mission = true;
				else
				{
					strcpy(errtext, TXT_MSN_MSNCOMMAND);
					goto msnfile_error;
				}
			}
			else if (!strcmpi(command, "AUTHOR"))
				strncpy(msn->author, operand, MSN_NAMELEN - 1);
			else if (!strcmpi(command, "KEYWORDS")) {
				//Don't do anything with this
			}
			else if (!strcmpi(command, "DESCRIPTION") || indesc)
			{
				//	multi-line descriptions require the strcat.  the initial
				//	strings should be empty for this to work.
				strcat(msn->desc, operand);
				if (indesc) 	strcat(msn->desc, "\n");
				indesc = 1;				// this is a multiline command
			}
			else if (!strcmpi(command, "URL"))
			{
				if (curlvlnum != 0) {
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					for (int a = 0; a < MAX_MISSION_URL_COUNT; a++)
					{
						if (Net_msn_URLs.URL[a][0] == '\0')
						{
							strncpy(Net_msn_URLs.URL[a], operand, MAX_MISSION_URL_LEN - 1);
							Net_msn_URLs.URL[a][MAX_MISSION_URL_LEN-1] = '\0';
							mprintf((0, "Found a Mission URL: %s\n", operand));
							break;
						}
					}
				}
			}
			else if (!strcmpi(command, "SHIP"))
			{
				// there is a different default ship
				if (curlvlnum != 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					int ship_index = FindShipName(operand);
					if (ship_index != -1)
					{
						// ok the ship exists, make this guy the new default ship
						PlayerResetShipPermissions(-1, false);
						PlayerSetShipPermission(-1, operand, true);
						mprintf((0, "MAKING %s THE NEW DEFAULT SHIP!\n", operand));
					}
					else
					{
						Int3();
					}
				}
			}
			else if (!strcmpi(command, "EMAIL"))
			{
				if (curlvlnum != 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
					strncpy(msn->email, operand, MSN_URLLEN - 1);
			}
			else if (!strcmpi(command, "SCORE"))
			{
				if (curlvlnum == 0) {
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					lvls[curlvlnum - 1].flags |= LVLFLAG_SCORE;
					lvls[curlvlnum - 1].score = mem_strdup(operand);
					if (!lvls[curlvlnum - 1].score) goto fatal_error;
				}
			}
			else if (!strcmpi(command, "PROGRESS"))
			{
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					lvls[curlvlnum - 1].progress = mem_strdup(operand);
					if (!lvls[curlvlnum - 1].progress) goto fatal_error;
				}
			}
			else if (!strcmpi(command, "HOG"))
			{
				if (curlvlnum == 0)
				{
					msn->hog = mem_strdup(operand);
					if (!msn->hog) goto fatal_error;
				}
				else
				{
					lvls[curlvlnum - 1].flags |= LVLFLAG_SPECIALHOG;
					if (!(lvls[curlvlnum - 1].hog = mem_strdup(operand))) goto fatal_error;
				}
			}
			else if (!strcmpi(command, "NUMLEVELS"))
			{
				if (curlvlnum != 0) {
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					//	get number of levels
					int value = atoi(operand);
					if (value == 0)
					{
						strcpy(errtext, TXT_MSN_LVLNUMINVALID);
						goto msnfile_error;
					}
					lvls = (tLevelNode*)mem_malloc(sizeof(tLevelNode) * value);
					memset(lvls, 0, sizeof(tLevelNode) * value);
					numlevels = value;
				}
			}
			else if (!strcmpi(command, "LEVEL"))
			{
				// first check if number of level is greater than num_levels
				if ((curlvlnum == numlevels) && (numlevels != -1))
				{
					strcpy(errtext, TXT_MSN_NUMLVLSINVALID);
					goto msnfile_error;
				}
				curlvlnum = atoi(operand);
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLNUMINVALID);
					goto msnfile_error;
				}
				else if (curlvlnum > numlevels || curlvlnum < 0)
				{
					strcpy(errtext, TXT_MSN_LVLNUMINVALID);
					goto msnfile_error;
				}
			}
			else if (!strcmpi(command, "INTROMOVIE"))
			{
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					lvls[curlvlnum - 1].flags |= LVLFLAG_STARTMOVIE;
					if (!(lvls[curlvlnum - 1].moviename = mem_strdup(operand))) goto fatal_error;
				}
			}
			else if (!strcmpi(command, "INTRODEFAULT"))
			{
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else if (!Descent_overrided_intro)
				{
					lvls[curlvlnum - 1].flags |= LVLFLAG_STARTMOVIE;
					if (!(lvls[curlvlnum - 1].moviename = mem_strdup(operand))) goto fatal_error;
				}
			}
			else if (!strcmpi(command, "ENDMOVIE"))
			{
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					lvls[curlvlnum - 1].flags |= LVLFLAG_ENDMOVIE;
					if (!(lvls[curlvlnum - 1].endmovie = mem_strdup(operand))) goto fatal_error;
				}
			}
			else if (!strcmpi(command, "MINE"))
			{
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					if (!(lvls[curlvlnum - 1].filename = mem_strdup(operand)))
						goto fatal_error;
				}
			}
			else if (!strcmpi(command, "SECRET"))
			{
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					lvls[curlvlnum - 1].flags |= LVLFLAG_SPAWNSECRET;
					lvls[curlvlnum - 1].secretlvl = atoi(operand);
				}
			}
			else if (!strcmpi(command, "BRIEFING"))
			{
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
				{
					lvls[curlvlnum - 1].flags |= LVLFLAG_BRIEFING;
					if (!(lvls[curlvlnum - 1].briefname = mem_strdup(operand))) goto fatal_error;
				}
			}
			else if (!strcmpi(command, "BRANCH"))
			{
				// first check if number of level is greater than num_levels
				int lvlnum;
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				lvlnum = atoi(operand);
				if (lvlnum == 0 || lvlnum > numlevels)
				{
					strcpy(errtext, TXT_MSN_LVLNUMINVALID);
					goto msnfile_error;
				}
				lvls[curlvlnum - 1].flags |= LVLFLAG_BRANCH;
				lvls[curlvlnum - 1].lvlbranch0 = lvlnum;
			}
			else if (!strcmpi(command, "ENDMISSION"))
			{
				if (curlvlnum == 0)
				{
					strcpy(errtext, TXT_MSN_LVLCOMMAND);
					goto msnfile_error;
				}
				else
					lvls[curlvlnum - 1].flags |= LVLFLAG_FINAL;
			}
			else
			{
				sprintf(errtext, TXT_MSN_ILLEGALCMD, command);
				goto msnfile_error;
			}
		}
		srclinenum++;
	}
	//	set up current mission (movies are already set above)
	msn->cur_level = 1;
	msn->num_levels = numlevels;
	msn->levels = lvls;
	msn->filename = mem_strdup((char*)mission);
	msn->game_state_flags = 0;
	strcpy(Net_msn_URLs.msnname, mission);
	res = true;							// everthing is ok.
	//	if error, print it out, else end.
msnfile_error:
	if (!res)
	{
		char str_text[128];
		sprintf(str_text, "%s\nline %d.", errtext, srclinenum);
		if (!Dedicated_server)
		{
			DoMessageBox(TXT_ERROR, str_text, MSGBOX_OK);
		}
		else
		{
			PrintDedicatedMessage("%s: %s\n", TXT_ERROR, str_text);
		}
		if (lvls) mem_free(lvls);
	}
	if (fp) cfclose(fp);
	//	load default script here.
	if (!InitMissionScript())
		return false;

	return res;
fatal_error:
	mem_error();
	return false;
#endif
	return false;
}
void FreeMission()
{
	//	Free up mission script
	int i;	//,j;
	// close MN3 file if there is one.
	mn3_Close();
	// Tell Osiris to shutdown the Osiris Mission Memory System, freeing all memory
	Osiris_CloseOMMS();
	if (Current_mission.levels)
	{
		//	free up any data allocated per level node.
		for (i = 0; i < Current_mission.num_levels; i++)
		{
			if (Current_mission.levels[i].filename)
				mem_free(Current_mission.levels[i].filename);
			if (Current_mission.levels[i].briefname)
				mem_free(Current_mission.levels[i].briefname);
			if (Current_mission.levels[i].hog)
				mem_free(Current_mission.levels[i].hog);
			if (Current_mission.levels[i].moviename)
				mem_free(Current_mission.levels[i].moviename);
			if (Current_mission.levels[i].endmovie)
				mem_free(Current_mission.levels[i].endmovie);
			if (Current_mission.levels[i].score)
				mem_free(Current_mission.levels[i].score);
			if (Current_mission.levels[i].progress)
				mem_free(Current_mission.levels[i].progress);
		}
		mem_free(Current_mission.levels);
		Current_mission.levels = NULL;
	}
	if (Current_mission.hog)
		mem_free(Current_mission.hog);
	if (Current_mission.filename)
	{
		//these DON't USE mem_free since we use _strdup, which doesn't use our memory routines.
		mem_free(Current_mission.filename);
		Current_mission.filename = NULL;
	}
	Current_mission.hog = NULL;
	Current_level = NULL;
}
#include "localization.h"
#include "levelgoal.h"
//Load the text (goal strings) for a level
void LoadLevelText(char* level_filename)
{
	char pathname[_MAX_FNAME], filename[_MAX_FNAME];
	int n_strings;
	ddio_SplitPath(level_filename, pathname, filename, NULL);
	strcat(pathname, filename);
	strcat(pathname, ".str");
	char** goal_strings;
	if (CreateStringTable(pathname, &goal_strings, &n_strings))
	{
		int n_goals = Level_goals.GetNumGoals();
		ASSERT(n_strings == (n_goals * 3));
		for (int i = 0; i < n_goals; i++)
		{
			Level_goals.GoalSetName(i, goal_strings[i * 3]);
			Level_goals.GoalSetItemName(i, goal_strings[i * 3 + 1]);
			Level_goals.GoalSetDesc(i, goal_strings[i * 3 + 2]);
		}
		DestroyStringTable(goal_strings, n_strings);
	}

}
extern bool Hud_show_controls;
/*	loads a level and sets it as current level in mission
*/
bool LoadMissionLevel(int level)
{
	Hud_show_controls = false;
	LoadLevelProgress(LOAD_PROGRESS_START, 0);
	if (!LoadLevel(Current_mission.levels[level - 1].filename, NULL))
	{
		char buf[128];
		sprintf(buf, TXT_MSNERROR, Current_mission.levels[level - 1].filename);
		SetUICallback(DEFAULT_UICALLBACK);
		DoMessageBox(TXT_ERROR, buf, MSGBOX_OK);
		LoadLevelProgress(LOAD_PROGRESS_DONE, 0);
		return false;
	}
	// Test to see if levels are valid
	bool boa = true, terrain = true;
	char str[255];

	if (BOAGetMineChecksum() != BOA_vis_checksum)
		boa = false;
	if (GetTerrainGeometryChecksum() != Terrain_checksum)
		terrain = false;

	//Remove this incredibly well considered code that was absolutely a good idea.
	/*if (!boa)
	{
		sprintf (str,"BOA NOT VALID! %s",terrain?"":"TERRAINOCC NOT VALID!");
		ShowProgressScreen(str,NULL,true);
		Sleep (2000);
	}*/

	//[ISB] Prepare the new renderer
	//I love the game code being so tightly coupled with the renderer
	if (!Dedicated_server)
	{
		NewRender_InitNewLevel();
	}

	LoadLevelText(Current_mission.levels[level - 1].filename);

	return true;
}
#define LOADBAR_X		(16 * (float)Max_window_w/(float)FIXED_SCREEN_WIDTH)
#define LOADBAR_Y1		(376 * (float)Max_window_h/(float)FIXED_SCREEN_HEIGHT)
#define LOADBAR_Y2		(440 * (float)Max_window_h/(float)FIXED_SCREEN_HEIGHT)
#define LOADBAR_W		(260 * (float)Max_window_w/(float)FIXED_SCREEN_WIDTH)
#define LOADBAR_H		(22 * (float)Max_window_h/(float)FIXED_SCREEN_HEIGHT)
#define N_LOAD_MSGS	12
extern int need_to_page_in;
extern int paged_in_count;
bool started_page = 0;
/*
$$TABLE_GAMEFILE "tunnelload.ogf"
*/
bool Progress_screen_loaded = false;
void LoadLevelProgress(int step, float percent, char* chunk)
{
	static tLargeBitmap level_bmp;
	static bool level_bmp_loaded = false;
	static int dedicated_last_string_len = -1;
	float lvl_percent_loaded = 0;
	float pag_percent_loaded = 0;
	int i;
	static int n_text_msgs = 0;
	static char text_msgs[N_LOAD_MSGS][80];
	char str[80];
	// If we're loading a level, send out a heartbeat
	if ((Game_mode & GM_MULTI))
		MultiSendHeartbeat();

	if (percent > 1.0f)
		percent = 1.0f;
	else if (percent < 0.0f)
		percent = 0.0f;

	if ((!Progress_screen_loaded) && (step != LOAD_PROGRESS_START))
		return;
	switch (step)
	{
	case LOAD_PROGRESS_START:
	{
		lvl_percent_loaded = 0.0f;
		pag_percent_loaded = 0.0f;
		dedicated_last_string_len = -1;
		char* p = NULL;
		if ((!(Game_mode & GM_MULTI)) && (!Current_mission.levels[Current_mission.cur_level - 1].progress))
		{
			p = "tunnelload.ogf";
		}
		else
		{
			p = Current_mission.levels[Current_mission.cur_level - 1].progress;
		}

		if (p)
		{
			if (LoadLargeBitmap(IGNORE_TABLE(p), &level_bmp))
			{
				SetScreenMode(SM_MENU);
				level_bmp_loaded = true;
				n_text_msgs = 0;
			}
		}
		/*
		else
		{
			ShowProgressScreen (TXT_LOADINGLEVEL);
		}
		*/
		Progress_screen_loaded = true;
		return;
	}
	break;
	case LOAD_PROGRESS_LOADING_LEVEL:
	{
		lvl_percent_loaded = percent;
	}
	break;
	case LOAD_PROGRESS_PAGING_DATA:
	{
		lvl_percent_loaded = 1.0f;
		pag_percent_loaded = percent;
	}
	break;
	case LOAD_PROGRESS_PREPARE:
		if (Dedicated_server)
		{
			PrintDedicatedMessage("\n");
		}
		lvl_percent_loaded = 1.0f;
		pag_percent_loaded = 1.0f;
		mprintf((0, "Prepare for Descent goes here...\n"));
		//ShowProgressScreen(TXT_PREPARE_FOR_DESCENT,NULL,true);
		//return;
		break;
	case LOAD_PROGRESS_DONE:
	{
		if (level_bmp_loaded)
		{
			// print out final message
			FreeLargeBitmap(&level_bmp);
			level_bmp_loaded = false;
		}
		Progress_screen_loaded = false;
		if (Dedicated_server)
		{
			PrintDedicatedMessage("\n");
		}
		return;
	}
	break;
	default:
		mprintf((0, "Unknown step in LoadLevelProgress()\n"));
		Int3();
	}
	if (Dedicated_server)
	{
		char tbuffer[512];
		char tbuffer1[512];
		nw_DoNetworkIdle();
		if (dedicated_last_string_len != -1)
		{
			memset(tbuffer, '\b', dedicated_last_string_len);
			tbuffer[dedicated_last_string_len] = '\0';
			PrintDedicatedMessage(tbuffer);
		}
		char levelname[100];
		sprintf(levelname, "%s level %d", Current_mission.name, Current_mission.cur_level);
		if (lvl_percent_loaded < 1.0f)
		{
			sprintf(tbuffer1, TXT_DS_LEVELLOADSTATUS, levelname, lvl_percent_loaded * 100.0f);
			sprintf(tbuffer, "%s%s", TXT_LOADINGLEVEL, tbuffer1);
			PrintDedicatedMessage(tbuffer);
			dedicated_last_string_len = strlen(tbuffer);
		}
		else
		{
			sprintf(tbuffer1, TXT_DS_LEVELLOADSTATUS, levelname, pag_percent_loaded * 100.0f);
			sprintf(tbuffer, "%s%s", TXT_LL_PAGINGDATA, tbuffer1);
			PrintDedicatedMessage(tbuffer);
			dedicated_last_string_len = strlen(tbuffer);
		}
		return;
	}
	StartFrame(0, 0, Max_window_w, Max_window_h);
	// do background.
	if (!level_bmp_loaded)
	{
		int text_height;
		ASSERT(Current_level);
		rend_ClearScreen(GR_BLACK);
		grtext_SetFont(MENU_FONT);
		text_height = grfont_GetHeight(MENU_FONT);
		grtext_SetColor(GR_WHITE);
		grtext_CenteredPrintf(0, Max_window_h / 2, TXT_LOADINGLEVEL);
		//grtext_CenteredPrintf(0, (Max_window_h/2)+(text_height*2), Current_level->name);
	}
	else
	{
#ifdef STEALTH	//DAJ
		rend_ClearScreen(GR_BLACK);
#else	
		DrawLargeBitmap(&level_bmp, 0, 0, 1.0f);
#endif
		// do relevent text.
		str[0] = 0;
		if (chunk)
		{
			if (strncmp(CHUNK_TERRAIN, chunk, strlen(CHUNK_TERRAIN)) == 0)
				strcpy(str, TXT_LL_TERRAIN);
			else if (strncmp(CHUNK_SCRIPT_CODE, chunk, strlen(CHUNK_SCRIPT_CODE)) == 0)
				strcpy(str, TXT_LL_SCRIPTLOADED);
			else if (strncmp(CHUNK_ROOMS, chunk, strlen(CHUNK_ROOMS)) == 0)
				strcpy(str, TXT_LL_ROOMSLOADED);
			else if (strncmp(CHUNK_OBJECTS, chunk, strlen(CHUNK_OBJECTS)) == 0)
				strcpy(str, TXT_LL_OBJECTSLOADED);
		}
		if (str[0])
		{
			if (n_text_msgs == N_LOAD_MSGS)
			{
				for (i = 1; i < n_text_msgs; i++)
					strcpy(text_msgs[i - 1], text_msgs[i]);
			}
			else
				n_text_msgs++;

			strcpy(text_msgs[n_text_msgs - 1], str);
		}
		grtext_SetFont(BRIEFING_FONT);
		grtext_SetColor(GR_WHITE);
		grtext_SetAlpha(230);
		for (i = 0; i < n_text_msgs; i++)
		{
			grtext_Printf(14, 32 + (i * 15), text_msgs[i]);
		}
	}
	g3Point* pntlist[4], points[4];
	points[0].p3_sx = LOADBAR_X;
	points[0].p3_sy = LOADBAR_Y1;
	points[1].p3_sx = LOADBAR_X + LOADBAR_W;
	points[1].p3_sy = LOADBAR_Y1;
	points[2].p3_sx = LOADBAR_X + LOADBAR_W;
	points[2].p3_sy = LOADBAR_Y1 + LOADBAR_H;
	points[3].p3_sx = LOADBAR_X;
	points[3].p3_sy = LOADBAR_Y1 + LOADBAR_H;
	for (i = 0; i < 4; i++)
	{
		points[i].p3_z = 0;
		points[i].p3_flags = PF_PROJECTED;
		pntlist[i] = &points[i];
	}
	rend_SetZBufferState(0);
	rend_SetTextureType(TT_FLAT);
	rend_SetAlphaType(AT_CONSTANT);
	rend_SetLighting(LS_NONE);
	rend_SetAlphaValue(230);
	if (!level_bmp_loaded)
	{
		rend_SetFlatColor(GR_RGB(192, 192, 192));
		rend_DrawPolygon2D(0, pntlist, 4);
	}

	pntlist[1]->p3_sx = LOADBAR_X + ((float)LOADBAR_W * lvl_percent_loaded);
	pntlist[2]->p3_sx = LOADBAR_X + ((float)LOADBAR_W * lvl_percent_loaded);

	rend_SetFlatColor(GR_RGB(166, 7, 7));
	rend_DrawPolygon2D(0, pntlist, 4);

	grtext_SetFont(BRIEFING_FONT);
	grtext_SetColor(GR_WHITE);
	if (lvl_percent_loaded != 1.0)
	{
		strcpy(str, TXT_LOADINGLEVEL);
	}
	else
	{
		strcpy(str, TXT_DONE);
	}
	grtext_Printf(LOADBAR_X + (LOADBAR_W - grtext_GetLineWidth(str)) / 2, LOADBAR_Y1 + (LOADBAR_H - grtext_GetHeight(str)) / 2, str);
	points[0].p3_sx = LOADBAR_X;
	points[0].p3_sy = LOADBAR_Y2;
	points[1].p3_sx = LOADBAR_X + LOADBAR_W;
	points[1].p3_sy = LOADBAR_Y2;
	points[2].p3_sx = LOADBAR_X + LOADBAR_W;
	points[2].p3_sy = LOADBAR_Y2 + LOADBAR_H;
	points[3].p3_sx = LOADBAR_X;
	points[3].p3_sy = LOADBAR_Y2 + LOADBAR_H;
	for (i = 0; i < 4; i++)
	{
		points[i].p3_z = 0;
		points[i].p3_flags = PF_PROJECTED;
		pntlist[i] = &points[i];
	}
	rend_SetZBufferState(0);
	rend_SetTextureType(TT_FLAT);
	rend_SetAlphaType(AT_CONSTANT);
	rend_SetLighting(LS_NONE);
	rend_SetAlphaValue(230);
	if (!level_bmp_loaded)
	{
		rend_SetFlatColor(GR_RGB(192, 192, 192));
		rend_DrawPolygon2D(0, pntlist, 4);
	}

	pntlist[1]->p3_sx = LOADBAR_X + ((float)LOADBAR_W * pag_percent_loaded);
	pntlist[2]->p3_sx = LOADBAR_X + ((float)LOADBAR_W * pag_percent_loaded);

	rend_SetFlatColor(GR_RGB(166, 7, 7));
	rend_DrawPolygon2D(0, pntlist, 4);

	grtext_SetFont(BRIEFING_FONT);
	grtext_SetColor(GR_WHITE);
	if (pag_percent_loaded == 1.0)
		strcpy(str, TXT_DONE);
	else
		strcpy(str, TXT_LL_PAGINGDATA);

	if (pag_percent_loaded > 0.0)
		grtext_Printf(LOADBAR_X + (LOADBAR_W - grtext_GetLineWidth(str)) / 2, LOADBAR_Y2 + (LOADBAR_H - grtext_GetHeight(str)) / 2, str);

	//Display data errors
	extern int Data_error_count;
	if (Data_error_count)
	{
		char buf[1024];
		sprintf(buf, "This mission has %d data errors", Data_error_count);
		int y = 200 + 40;
		int x = 320 - (grtext_GetLineWidth(buf) / 2);
		grtext_Printf(x, y, buf);
	}

	if (step == LOAD_PROGRESS_PREPARE)
	{
		grtext_SetFont(BIG_BRIEFING_FONT);
		grtext_SetColor(GR_WHITE);

		int preparey = 200;
		int preparex = 320 - (grtext_GetLineWidth(TXT_PREPARE_FOR_DESCENT) / 2);
		if (preparex < 0)
			preparex = 0;
		grtext_Printf(preparex, preparey, TXT_PREPARE_FOR_DESCENT);
	}

	// Display level name if still loading
	if ((pag_percent_loaded > 0.0) && step != LOAD_PROGRESS_PREPARE)
	{
		char str[255];

		sprintf(str, "\"%s\"", Level_info.name);
		grtext_SetFont(BIG_BRIEFING_FONT);
		grtext_SetColor(GR_WHITE);

		int preparey = 200;
		int preparex = 320 - (grtext_GetLineWidth(str) / 2);
		if (preparex < 0)
			preparex = 0;
		grtext_Printf(preparex, preparey, str);
	}
	grtext_Flush();
	EndFrame();
	rend_Flip();
}


/*	this functions performs the end mission code
*/
void DoEndMission()
{
	if (Game_mode & GM_MULTI)	// If multiplayer, just loop
	{
		if (Dedicated_server)
			PrintDedicatedMessage(TXT_DS_MISSIONDONE);
		SetCurrentLevel(1);
		return;
	}
}


// Shows some text on a background, useful for telling the player what is going on
// ie "Loading level...", "Receiving data...", etc
void ShowProgressScreen(char* str, char* str2, bool flip)
{
	if (Dedicated_server)
	{
		PrintDedicatedMessage("%s\n", str);
		if (str2)
			PrintDedicatedMessage("%s\n", str2);
		return;
	}
	StartFrame(0, 0, Max_window_w, Max_window_h);
	rend_ClearScreen(GR_BLACK);
	grtext_SetFont(MENU_FONT);
	int text_height = grfont_GetHeight(MENU_FONT);
	grtext_SetColor(GR_WHITE);
	grtext_CenteredPrintf(0, Max_window_h / 2, str);
	if (str2)
		grtext_CenteredPrintf(0, (Max_window_h / 2) + (text_height * 2), str2);
	grtext_Flush();
	EndFrame();
	if (flip)
		rend_Flip();

}


/*	does a mission briefing, returns if mission was canceled, a false, or 0 value.
	first displays mission goals and some warnings or advice.
	may allow for selection of player ships
*/
bool DoMissionBriefing(int level)
{
	tLevelNode* lvl = Current_level;
	bool ret = true;
	int num_ships = 0;
	for (int i = 0; i < MAX_SHIPS; i++)
	{
		if (Ships[i].used && PlayerIsShipAllowed(Player_num, i))
			num_ships++;
	}
	if (lvl->briefname[0] || num_ships > 1)
		ret = !TelComShow(TS_MISSION, false, true);
	return ret;
}


extern bool FirstGame;
bool Skip_next_movie = false;
//	---------------------------------------------------------------------------
//	 play movie
void DoMissionMovie(char* movie)
{
	char temppath[PSPATHNAME_LEN + PSFILENAME_LEN];
	if (PROGRAM(windowed)) {
		mprintf((0, "Skipping movie...can't do in windowed mode!\n"));
		return;
	}
	//Don't play this movie the first time through. This is a horrible hack.
	if (Skip_next_movie)
	{
		Skip_next_movie = false;
		return;
	}
#ifdef D3_FAST
	return;
#endif
	if (movie && *movie)
	{
		char mpath[_MAX_PATH];
		ddio_MakePath(mpath, LocalD3Dir, "movies", movie, NULL);
		int sm_old = GetScreenMode();
		SetScreenMode(SM_CINEMATIC);
		PlayMovie(mpath);
		SetScreenMode(sm_old);
	}
	//PlayMovie(movie);
}
///////////////////////////////////////////////////////////////////////////////
//	Script Management for Missions and Levels
///////////////////////////////////////////////////////////////////////////////
//	Initializes a mission's default script
bool InitMissionScript()
{
	return true;
}
extern bool IsRestoredGame;
void InitLevelScript()
{
	if (Current_level->filename)
	{
		char filename[_MAX_PATH], ext[_MAX_EXT];
		ddio_SplitPath(Current_level->filename, NULL, filename, ext);
#if defined (WIN32)
		strcat(filename, ".dll");
#else
#if defined(MACOSX)
		strcat(filename, ".dylib");
#else
		strcat(filename, ".so");
#endif
#endif
		Osiris_LoadLevelModule(filename);
	}

	tOSIRISEventInfo ei;
	//This is a hack... we don't want the level start script to be called when restoring a save game
	if (!IsRestoredGame)
		Osiris_CallLevelEvent(EVT_LEVELSTART, &ei);
	AssignScriptsForLevel();								// initialize all scripts for level.
}


void FreeLevelScript()
{
	Osiris_UnloadLevelModule();
	if (Current_level)
	{
		//	free level's script and thread
		tOSIRISEventInfo ei;
		Osiris_CallLevelEvent(EVT_LEVELEND, &ei);
	}
}


//	return information about a mission
bool GetMissionInfo(const char* msnfile, tMissionInfo* msn)
{
	CFILE* fp;
	bool indesc = false;						// are we in a multi-line block
	//	open mission file
	if (IS_MN3_FILE(msnfile))
		return mn3_GetInfo(msnfile, msn);

	fp = cfopen(msnfile, "rt");
	if (!fp)
	{
		mprintf((0, "Failed to open mission file %s in GetMissionInfo.\n", msnfile));
		return false;
	}

	msn->multi = true;
	msn->single = true;
	msn->training = false;
	msn->author[0] = 0;
	msn->desc[0] = 0;
	while (!cfeof(fp))
	{
		char srcline[128];					// One line of mission source
		char command[32];					// Command read from line.
		char operand[96];					// operand read from line
		char* keyword;						// parsed keyword
		int readcount;						// read-in count
		readcount = cf_ReadString(srcline, sizeof(srcline), fp);
		if (readcount)
		{
			//	we have a line of source.  parse out primary keyword
			//	then parse out remainder.
			keyword = strtok(srcline, " \t");
			CleanupStr(command, srcline, sizeof(command));
			CleanupStr(operand, srcline + strlen(command) + 1, sizeof(operand));
			if (strlen(command) && indesc)
				indesc = false;
			if (!strcmpi(command, "NAME"))
			{
				strncpy(msn->name, operand, MSN_NAMELEN - 1);
				msn->name[MSN_NAMELEN - 1] = '\0';
			}
			else if (!strcmpi(command, "MULTI"))
			{
				if (strcmpi("no", operand) == 0)
					msn->multi = false;
			}
			else if (!strcmpi(command, "SINGLE"))
			{
				if (strcmpi("no", operand) == 0)
					msn->single = false;
			}
			else if (!strcmpi(command, "TRAINER"))
			{
				msn->training = true;
			}
			else if (!strcmpi(command, "AUTHOR"))
			{
				strncpy(msn->author, operand, MSN_NAMELEN - 1);
				msn->author[MSN_NAMELEN - 1] = '\0';
			}
			else if (!strcmpi(command, "DESCRIPTION") || indesc)
			{
				//	multi-line descriptions require the strcat.  the initial
				//	strings should be empty for this to work.
				strcat(msn->desc, operand);
				if (indesc)
					strcat(msn->desc, "\n");
				indesc = true;				// this is a multiline command
			}
			else if (!strcmpi(command, "NUMLEVELS"))
			{
				//	get number of levels
				int value = atoi(operand);
				msn->n_levels = value;
			}
			else if (!strcmpi(command, "LEVEL"))
			{
				break;
			}
			else if (!strcmpi(command, "KEYWORDS"))
			{
				//Read in all the keywords
				strncpy(msn->keywords, operand, MAX_KEYWORDLEN);
				msn->keywords[MAX_KEYWORDLEN - 1] = '\0';
			}
		}
	}
	cfclose(fp);
	return true;
}
//	---------------------------------------------------------------------------
char* GetMissionName(char* mission)
{
	tMissionInfo msninfo;
	static char msnname[MSN_NAMELEN];
	msnname[0] = 0;
	if (GetMissionInfo(mission, &msninfo))
		strcpy(msnname, msninfo.name);
	else
	{
		mprintf((0, "MISSION:GetMissionName failed from call to GetMissionInfo\n"));
	}
	return msnname;
}
bool IsMissionMultiPlayable(char* mission)
{
	tMissionInfo msninfo;
	if (GetMissionInfo(mission, &msninfo))
		return msninfo.multi;

	return false;
}
bool IsMissionSinglePlayable(char* mission)
{
	tMissionInfo msninfo;
	if (GetMissionInfo(mission, &msninfo))
		return msninfo.single;

	return false;
}
int Mission_voice_hog_handle = 0;
//	MN3 based mission functions.
// loads the msn file from the mn3 file specified, specifies the hog and table file.
bool mn3_Open(const char* mn3file)
{
	char pathname[PSPATHNAME_LEN + PSFILENAME_LEN];
	char filename[PSFILENAME_LEN];
	char ext[PSFILENAME_LEN];
	int mn3_handle;
	// concatanate the mn3 extension if it isn't there.
	if (!IS_MN3_FILE(mn3file))
		strcat(filename, ".mn3");

	char* p = GetMultiCDPath((char*)mn3file);
	//ddio_MakePath(pathname, D3MissionsDir, mn3file, NULL);
	if (!p)
		return false;

	strcpy(pathname, p);
	// open MN3 HOG.
	mn3_handle = cf_OpenLibrary(pathname);
	if (mn3_handle == 0)
		return false;
	else
		Osiris_ExtractScriptsFromHog(mn3_handle, true);

	// do table file stuff.
	ddio_SplitPath(mn3file, NULL, filename, ext);

	//	char voice_hog[_MAX_PATH*2];
	if ((strcmpi(filename, "d3") == 0) || (strcmpi(filename, "training") == 0))
	{
		//Open audio hog file
		//ddio_MakePath(voice_hog, D3MissionsDir, "d3voice1.hog", NULL);//Audio for levels 1-4
		char* v = GetMultiCDPath("d3voice1.hog");
		Mission_voice_hog_handle = cf_OpenLibrary(v);
	}
	else if (strcmpi(filename, "d3_2") == 0)
	{
		//Open audio hog file
		//ddio_MakePath(voice_hog, D3MissionsDir, "d3voice2.hog", NULL);//Audio for levels 5-17
		char* v = GetMultiCDPath("d3voice2.hog");
		Mission_voice_hog_handle = cf_OpenLibrary(v);
	}
	strcat(filename, ".gam");
	mng_SetAddonTable(filename);
	Current_mission.mn3_handle = mn3_handle;
	return true;
}


// returns mission information given the mn3 file.
bool mn3_GetInfo(const char* mn3file, tMissionInfo* msn)
{
	int handle;
	bool retval;
	char pathname[PSPATHNAME_LEN + PSFILENAME_LEN];
	char filename[PSFILENAME_LEN];

	if (strcmpi(mn3file, "d3.mn3") == 0)
	{
		char* p = GetMultiCDPath((char*)mn3file);
		if (!p)
			return false;
		strcpy(pathname, p);
	}
	else
		ddio_MakePath(pathname, D3MissionsDir, mn3file, NULL);

	handle = cf_OpenLibrary(pathname);
	if (handle == 0)
	{
		mprintf((0, "MISSION: MN3 failed to open.\n"));
		return false;
	}

	MN3_TO_MSN_NAME(mn3file, filename);
	retval = GetMissionInfo(filename, msn);
	cf_CloseLibrary(handle);
	return retval;
}
// closes the current mn3 file
void mn3_Close()
{
	if (Current_mission.mn3_handle)
	{
		Osiris_ClearExtractedScripts(true);
		cf_CloseLibrary(Current_mission.mn3_handle);

	}
	if (Mission_voice_hog_handle)
	{
		cf_CloseLibrary(Mission_voice_hog_handle);
		Mission_voice_hog_handle = 0;
	}
	Current_mission.mn3_handle = 0;
}


#define KEYWORD_LEN	16
#define NUM_KEYWORDS	16
#define GOALSTEXT	"GOALS"
#define GOALSTEXTLEN	strlen(GOALSTEXT)
#define MODMINGOALS	"MINGOALS"
#define MODMINGOALSLEN	strlen(MODMINGOALS)
//Returns the max number of teams this mission can support for this mod, or 
//-1 if this level shouldn't be played with this mission
//Return values:
// -1	Bad match -- this level and this mod shouldn't be played together!
// MAX_NET_PLAYERS	-- This is playable with any number of teams the mod wants
int MissionGetKeywords(char* mission, char* keywords)
{
	ASSERT(mission);
	ASSERT(keywords);

	char msn_keywords[NUM_KEYWORDS][KEYWORD_LEN];
	char mod_keywords[NUM_KEYWORDS][KEYWORD_LEN];
	int i;
	char* parse_keys = mem_strdup(keywords);
	char seps[] = ",";
	int teams = MAX_NET_PLAYERS;
	int goals = 0;
	int goalsneeded = 0;
	int mod_key_count = 0;
	int msn_key_count = 0;
	bool goal_per_team = false;
	tMissionInfo msn_info;

	memset(msn_keywords, 0, sizeof(msn_keywords));
	memset(mod_keywords, 0, sizeof(mod_keywords));
	mprintf((0, "MissionGetKeywords(%s,%s)\n", mission, keywords));
	if (!GetMissionInfo(mission, &msn_info))
		return -1;

	if (!*parse_keys)
		return MAX_NET_PLAYERS;

	//Break up the mod keywords into an array	
	char* tokp = strtok(parse_keys, seps);
	if (tokp)
	{
		do
		{
			strcpy(mod_keywords[mod_key_count], tokp);
			mod_key_count++;
			tokp = strtok(NULL, seps);
		} while ((tokp) && (mod_key_count < NUM_KEYWORDS));
	}
	//Break up the msn keywords into an array
	tokp = strtok(msn_info.keywords, seps);
	if (tokp)
	{
		do
		{
			strcpy(msn_keywords[msn_key_count], tokp);
			msn_key_count++;
			tokp = strtok(NULL, seps);
		} while ((tokp) && (msn_key_count < NUM_KEYWORDS));
	}

	for (i = 0; i < NUM_KEYWORDS; i++)
	{
		if (msn_keywords[i][0] == 0)
		{
			continue;
		}
		if (strnicmp(GOALSTEXT, msn_keywords[i], GOALSTEXTLEN) == 0)
		{
			//Get the number of goals this game has
			goals = atoi(msn_keywords[i] + GOALSTEXTLEN);
		}
	}
	mem_free(parse_keys);

	//Loop through looking for matches
	for (i = 0; i < NUM_KEYWORDS; i++)
	{
		if (mod_keywords[i][0] == 0)
			continue;
		if (strnicmp(mod_keywords[i], MODMINGOALS, MODMINGOALSLEN) == 0)
		{
			goalsneeded = atoi(mod_keywords[i] + MODMINGOALSLEN);
		}
		else if (strcmpi("GOALPERTEAM", mod_keywords[i]) == 0)
		{
			goal_per_team = true;
		}
		else
		{
			bool found_keyword = false;
			//Loop through looking for matches
			for (int a = 0; a < NUM_KEYWORDS; a++)
			{
				if (msn_keywords[a][0] == 0)
				{
					continue;
				}
				if (strcmpi(msn_keywords[a], mod_keywords[i]) == 0)
				{
					//Woohoo! it's found
					found_keyword = true;
					break;
				}
			}
			//We never found one we needed, so return -1;
			if (!found_keyword)
			{
				mprintf((0, "%s keyword needed in %s not found!\n", mod_keywords[i], mission));
				return -1;
			}
		}
	}
	if (goal_per_team)
	{
		teams = goals;
	}
	if (teams < goalsneeded)
	{
		mprintf((0, "Not enough goals in this level!\n"));
		teams = -1;
	}
	if (goals < goalsneeded)
	{
		mprintf((0, "Not enough goals in this level!\n"));
		teams = -1;
	}
	return teams;
}
//////////////////////////////////////////////////////////////////////////////
#ifdef EDITOR
//	Used by editor->game to load all necessary elements for level playing for systems
//	not initialized in editor, but only in game.
extern char Editor_quickplay_levelname[_MAX_PATH];
void QuickStartMission()
{
	//	create the level script code here, if we really need to.
	ResetGamemode();
	//	this initializes a mini one level mission with no frills.
	Current_mission.cur_level = 1;
	Current_mission.num_levels = 1;
	Current_mission.levels = (tLevelNode*)mem_malloc(sizeof(tLevelNode));
	memset(Current_mission.levels, 0, sizeof(tLevelNode));
	Current_level = Current_mission.levels;
	if (Editor_quickplay_levelname[0] != '\0')
		Current_level->filename = mem_strdup(Editor_quickplay_levelname);
	else
		Current_level->filename = NULL;
	//	proceed with initialization.
	InitMissionScript();
	SetGameState(GAMESTATE_LVLSTART);
}
// Used by game->editor to free up all extra game mission elements not needed in editor.
void QuickEndMission()
{
	ResetMission();
}
#endif
