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

#include <stdlib.h>
#include <string.h>

#include "ambient.h"

#include "hlsoundlib.h"

#include "room.h"
#include "game.h"
#include "psrand.h"

#define MAX_AMBIENT_SOUND_PATTERNS	100
asp Ambient_sound_patterns[MAX_AMBIENT_SOUND_PATTERNS];
int Num_ambient_sound_patterns=0;

//Computes a floating-point pseudo-random number.
//Returns value in the range 0..1, with the precision 1/RAND_MAX
float randf()
{
	return ((float) ps_rand()) / ((float) RAND_MAX);
}

//Process an Ambient Sound Pattern
void ProcessASP(asp *asp)
{
	//Check for empty ASP
	if (! asp->num_sounds)
		return;

	//Decrement delay
	asp->delay -= Frametime;

	//Time to play?
	if (asp->delay < 0.0) 
	{
		//Figure out which sound to play
		int roll = (ps_rand() * 100) / (RAND_MAX+1);	//roll = 0..99
		int s;
		
		for (s=0;s<asp->num_sounds;s++)
			if ((roll -= asp->sounds[s].probability) < 0)
				break;

		ASSERT(s < asp->num_sounds);

		//Play sound i
		ase *sound = &asp->sounds[s];
		float volume = sound->min_volume + (sound->max_volume - sound->min_volume) * randf();
		Sound_system.Play2dSound(sound->handle,volume);

		//Compute delay until next sound
		asp->delay = asp->min_delay + (asp->max_delay - asp->min_delay) * randf();
	}
}

//Does the ambient sound processing for one frame, maybe playing a sound or two
void DoAmbientSounds()
{
	if (OBJECT_OUTSIDE(Viewer_object))
		return;

	room *rp = &Rooms[Viewer_object->roomnum];

	if (rp->ambient_sound != -1)
		ProcessASP(&Ambient_sound_patterns[rp->ambient_sound]);
}

//Initializes the ambient sounds for the current level
void InitAmbientSounds()
{
	//Loop through all ambient sound patterns
	for (int p=0;p<Num_ambient_sound_patterns;p++) 
	{
		asp *asp = &Ambient_sound_patterns[p];

		//Initialize delay
		asp->delay = asp->min_delay + (asp->max_delay - asp->min_delay) * randf();

		//Make sure the probabilities add up
		int prob = 0;
		for (int s=0;s<asp->num_sounds;s++)
			prob += asp->sounds[s].probability;
		if (asp->num_sounds && prob != 100) {
			Int3();
			asp->sounds[0].probability += 100 - prob;		//make it total 100
		}
	}
}

#include "ddio.h"
#include "CFILE.H"
#include "soundload.h"
#include "descent.h"
#include "mem.h"

//Close down ambient sound system and free data
void FreeAmbientSoundData()
{
	for (int p=0;p<Num_ambient_sound_patterns;p++)
		if (Ambient_sound_patterns[p].num_sounds)
			mem_free(Ambient_sound_patterns[p].sounds);

	Num_ambient_sound_patterns = 0;
}

//Initialize the ambient sound system
void InitAmbientSoundSystem()
{
	atexit(FreeAmbientSoundData);

	ReadAmbientData();

	//Get rid of deleted patterns
	for (int p=0;p<Num_ambient_sound_patterns;p++) 
	{

		asp *asp = &Ambient_sound_patterns[p];

		if (! asp->name[0]) {
			Ambient_sound_patterns[p] = Ambient_sound_patterns[Num_ambient_sound_patterns-1];
			Num_ambient_sound_patterns--;
		}
	}
}

//Return the index of a named ambient sound pattern
//Returns number, or -1 if can't find
int FindAmbientSoundPattern(char *aspname)
{
	if (! aspname[0])
		return -1;

	for (int i=0;i<Num_ambient_sound_patterns;i++)
		if (stricmp(Ambient_sound_patterns[i].name,aspname) == 0)
			return i;

	return -1;
}

//Returns a pointer to the name of the specified ambient sound pattern
char *AmbientSoundPatternName(int n)
{
	return Ambient_sound_patterns[n].name;
}

#define AMBIENT_FILE_ID			"ASPF"
#define AMBIENT_FILE_VERSION	0

//Reads data from the ambient sound data file
void ReadAmbientData()
{
	CFILE *ifile;
	char file_id[sizeof(AMBIENT_FILE_ID)];
	int version;

	//Get rid of any old data
	FreeAmbientSoundData();

	//Build filename
	ifile = cfopen(AMBIENT_FILE_NAME,"rb");

	if (!ifile) 
	{
		Int3();
		return;
	}

	//Read file ID
	cf_ReadBytes((unsigned char *) file_id,strlen(AMBIENT_FILE_ID),ifile);
	if (strncmp(file_id,AMBIENT_FILE_ID,strlen(AMBIENT_FILE_ID)) != 0) 
	{
		Int3();
		cfclose(ifile);
		return;
	}

	//Read version
	version = cf_ReadInt(ifile);

	if (version > AMBIENT_FILE_VERSION) 
	{
		Int3();
		cfclose(ifile);
		return;
	}

	//Get the number of patterns
	Num_ambient_sound_patterns = cf_ReadInt(ifile);

	//Read the patterns
	for (int p=0;p<Num_ambient_sound_patterns;p++) 
	{
		asp *asp = &Ambient_sound_patterns[p];

		cf_ReadString(asp->name,sizeof(asp->name),ifile);

		asp->min_delay = cf_ReadFloat(ifile);
		asp->max_delay = cf_ReadFloat(ifile);

		asp->num_sounds = cf_ReadInt(ifile);

		if (asp->num_sounds>0)
			asp->sounds = (ase *) mem_malloc(sizeof(*asp->sounds) * asp->num_sounds);
		else
			asp->sounds=NULL;

		int prob=0;
		for (int s=0;s<asp->num_sounds;s++) 
		{
			char tbuf[PAGENAME_LEN];

			cf_ReadString(tbuf,sizeof(tbuf),ifile);
			asp->sounds[s].handle = FindSoundName(IGNORE_TABLE(tbuf));

			asp->sounds[s].min_volume = cf_ReadFloat(ifile);
			asp->sounds[s].max_volume = cf_ReadFloat(ifile);
			asp->sounds[s].probability = cf_ReadInt(ifile);

			prob += asp->sounds[s].probability;
		}

		if (asp->num_sounds && (prob != 100)) 
		{
			Int3();
			asp->sounds[0].probability += 100 - prob;		//make it total 100
		}
	}

	cfclose(ifile);
}

#ifdef NEWEDITOR
extern char D3HogDir[_MAX_PATH*2];
#endif

//Writes data from the ambient sound data file
void WriteAmbientData()
{
	char filename[_MAX_PATH];
	CFILE *ofile;

#ifndef NEWEDITOR
	ddio_MakePath(filename,Base_directory,"data","misc",AMBIENT_FILE_NAME,NULL);
#else
	ddio_MakePath(filename,D3HogDir,"data","misc",AMBIENT_FILE_NAME,NULL);
#endif
	ofile = cfopen(filename,"wb");

	if (!ofile) 
	{
		Int3();
		return;
	}

	//Write file ID & version
	cf_WriteBytes((ubyte *) AMBIENT_FILE_ID,strlen(AMBIENT_FILE_ID),ofile);
	cf_WriteInt(ofile,AMBIENT_FILE_VERSION);

	//Write the number of patterns
	cf_WriteInt(ofile,Num_ambient_sound_patterns);

	//Read the patterns
	for (int p=0;p<Num_ambient_sound_patterns;p++) 
	{
		asp *asp = &Ambient_sound_patterns[p];

		cf_WriteString(ofile,asp->name);

		cf_WriteFloat(ofile,asp->min_delay);
		cf_WriteFloat(ofile,asp->max_delay);

		cf_WriteInt(ofile,asp->num_sounds);

		for (int s=0;s<asp->num_sounds;s++) 
		{
			cf_WriteString(ofile,Sounds[asp->sounds[s].handle].name);

			cf_WriteFloat(ofile,asp->sounds[s].min_volume);
			cf_WriteFloat(ofile,asp->sounds[s].max_volume);
			cf_WriteInt(ofile,asp->sounds[s].probability);
		}
	}

	cfclose(ofile);
}

