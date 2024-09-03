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

#ifndef INIT_H
#define INIT_H

//	Universal globals for game
//	---------------------------------------------------------------------------

extern int ServerTimeout;
extern float LastPacketReceived;


//	---------------------------------------------------------------------------

// Initializes all the subsystems that need to be initialized BEFORE application creation.
void PreInitD3Systems();

// Initializes all the subsystems that D3/Editor needs to run. 
// Returns 1 if all is good, 0 if something is wrong
void InitD3Systems1(bool editor=false);
void InitD3Systems2(bool editor=false);

//Show the intro screen, or something
void IntroScreen();

//Save game variables to the registry
void SaveGameSettings();

//	This function shutdowns all game systems
void ShutdownD3();

//	This function restarts all game systems
void RestartD3();

void InitMessage(char *c,float progress=-1);

//[ISB] Returns true if Mercenary is installed. 
extern bool MercInstalled();

#endif
