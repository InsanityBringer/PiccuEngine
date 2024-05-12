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

#ifndef GAMESEQUENCE_H
#define GAMESEQUENCE_H


//	gamesequencer states: 
//		game states

enum tGameState 
{
	GAMESTATE_IDLE,					// no state
	GAMESTATE_NEW,						// starts a new game (with current mission.)
	GAMESTATE_LVLNEXT,				// go to the next level
	GAMESTATE_LVLSTART,				// happens when a level starts, game time starts here.
	GAMESTATE_LVLPLAYING,			// calls gameloop, polls for input.
	GAMESTATE_LOADGAME,				// a load game request has been issued.
	GAMESTATE_LVLEND,					// calls endlevel and any other endlevel stuff.
	GAMESTATE_LVLFAILED,				// a level was unsuccessfully ended
	GAMESTATE_LVLWARP,				// warp to a new level
	GAMESTATE_LOADDEMO,				// Load whatever demo was chosen in the UI
	GAMESTATE_GAMEGAUGEDEMO			// Play a gamegauge demo and exit
};


//	top level interfaces for game.
#define GAME_INTERFACE				0
#define GAME_OPTIONS_INTERFACE		1
#define GAME_PAUSE_INTERFACE		2
#define GAME_HELP_INTERFACE			3
#define GAME_BUDDY_INTERFACE		4
#define GAME_TELCOM_BRIEFINGS		5
#define GAME_TELCOM_AUTOMAP			6
#define GAME_TELCOM_CARGO			7
#define GAME_EXIT_CONFIRM			8
#define GAME_DLL_INTERFACE			9
#define GAME_SAVE_INTERFACE			10
#define GAME_LOAD_INTERFACE			11
#define GAME_TOGGLE_DEMO			12
#define GAME_POST_DEMO				13
#define GAME_DEMO_LOOP				14
#define GAME_LEVEL_WARP				15
#define GAME_DEBUGGRAPH_INTERFACE	16

//	variables
extern tGameState Game_state;
extern int Game_interface_mode;	// current interface mode of game (menu, game?)


//	functions
//	main sequencing code for game.  run this to execute a game.
//		before calling this function make sure that a 
//			mission has been loaded/initialized.
bool GameSequencer();

//Sets the current level for subsequent level loads, movies, or briefings
void SetCurrentLevel(int level);

//	loads and starts the specified leve. starts a new level (usually called internally)
bool LoadAndStartCurrentLevel();

//	creates a simple mission to play one level.
bool SimpleStartLevel(char *level_name);

//	sets and retrieves the current gamestate
inline void SetGameState(tGameState state) 
{
	Game_state = state; 
}

inline tGameState GetGameState() 
{
	return Game_state; 
}

#endif
