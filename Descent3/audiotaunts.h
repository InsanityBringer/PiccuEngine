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

#ifndef __AUDIO_TAUNT_H_
#define __AUDIO_TAUNT_H_

extern bool Audio_taunts_enabled;

// Error codes:
#define TAUNTIMPERR_NOERROR			0
#define TAUNTIMPERR_NOTFOUND		1
#define TAUNTIMPERR_NOTRIFF			2
#define TAUNTIMPERR_NOTWAVE			3
#define TAUNTIMPERR_INVALIDFILE		4
#define TAUNTIMPERR_NOTSUPPORTED	5
#define TAUNTIMPERR_INVALIDCHANNELS	6
#define TAUNTIMPERR_INVALIDSAMPLES	7
#define TAUNTIMPERR_INVALIDBITDEPTH	8
#define TAUNTIMPERR_NODATA			9
#define TAUNTIMPERR_OSFEXISTS		10
#define TAUNTIMPERR_INTERNALERR		11
#define TAUNTIMPERR_COMPRESSIONFAILURE	12
#define TAUNTIMPERR_OUTOFMEM		13
#define TAUNTIMPERR_NOOPENOSF		14

//	taunt_GetError
//
//	Returns more information about a failed import
int taunt_GetError(void);

//	taunt_GetErrorString
//	Returns a string describing an error code
char *taunt_GetErrorString(int error);

//	taunt_ImportWave
//	Given a fully qualified wave_filename (location of a .wav) and a fully
//	qualified outputfilename (where the .osf is to go), it will convert and
//	compress the wav file.
bool taunt_ImportWave(char *wave_filename,char *outputfilename);

//	taunt_PlayTauntFile
//	
//	Given a path to an .osf file, it will play it
bool taunt_PlayTauntFile(const char *filename);

//	taunt_PlayPlayerTaunt
//
//	Given a playernum, and which taunt (0 or 1) it will play that taunt
bool taunt_PlayPlayerTaunt(int pnum,int index);

// taunt_AreEnabled
//
//	Returns true if taunts are enabled
bool taunt_AreEnabled(void);

// taunt_Enable
//
//	Enables/Disables audio_taunts for the player
void taunt_Enable(bool enable);

// taunt_DelayTime()
//
//	Returns the delay time between taunts
float taunt_DelayTime(void);

// taunt_SetDelayTime()
//
//	Sets the taunt delay time
void taunt_SetDelayTime(float t);

#endif
