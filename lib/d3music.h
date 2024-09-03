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

#ifndef D3MUSIC_H
#define D3MUSIC_H

#include "pstypes.h"

//	register constants for the sequencer
#define MUSICREG_TRIGGER_VALUE		1		// trigger value set by calling app to sequencer
//@@#define MUSICREG_MOOD_VALUE		2		// current mood of player stored here.
//@@#define MUSICREG_NEGMOOD_TIMER	3		// time in negative mood
//@@#define MUSICREG_POSMOOD_TIMER	4		// time in positive mood
#define MUSICREG_PEACE_TIMER		5		// amount of time in 'non-combat' mode.

// types of triggers
#define MUSICTRIGGER_NONE			0
#define MUSICTRIGGER_NEWREGION		1		// player entered new region

extern const char *Music_type_names[];		// contains type names.

// structure passed to music frame
struct tMusicSeqInfo
{
// INPUT
	bool started_level;							// player started level
	bool player_dead;								// did player die?
	bool player_damaged;							// was player hit by enemy fire?
	bool player_invulnerable;					// is player invulnerable?
	bool player_terrain;							// is player in terrain (if not, in mine)
	ubyte player_shield_level;					// what shield level the player is at? (0-10)
	ubyte n_hostiles;								// number of hostiles
	ubyte n_hostiles_player_killed;			// number hostiles killed by player this frame.
	ubyte pad;

	float frametime;								// frame time.

// OUTPUT
	short cur_song;								// current song.
	float peace_timer;							// current peace timer

	const char *cur_loop_name;					// current loop playing (NULL if none.)
	int cur_loop_count;
};

// this is located in gameloop.cpp.   kept here so files that need this data structure don't have to include 
// gameloop (and to stop gameloop.h from including d3music.h)
extern tMusicSeqInfo Game_music_info;

#ifdef _DEBUG
// if true, turns on debugging for music.
extern bool Music_debug_verbose;
#endif

//	Music extensions
void InitD3Music(bool allow_music);

//	closes music system
void CloseD3Music();

//	starts up the music sequencer
void D3MusicStart(const char *theme_file);

//	stops the music sequencer
void D3MusicStop();

//	execute music sequencer.
void D3MusicDoFrame(tMusicSeqInfo *music_info);

// toggle music system.
void D3MusicToggle();

// toggle music system.
void D3MusicToggle(bool state);

// pauses and or resumes music
void D3MusicPause();
void D3MusicResume();

// returns true if music system is on.
bool IsD3MusicOn();

//	set music region
void D3MusicSetRegion(short region,bool immediate=false);

// retreives current region (can be different than regin passed to D3MusicSetRegion),
// returns current played region, not waiting region.
short D3MusicGetRegion();

// retreives current pending region
short D3MusicGetPendingRegion();

// starts special in-game cinematic music
void D3MusicStartCinematic();

// stops special in-game cinematic music
void D3MusicStopCinematic();

// volume stuff
float D3MusicGetVolume();
void D3MusicSetVolume(float vol);

#endif
