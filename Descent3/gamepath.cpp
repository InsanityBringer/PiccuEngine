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

#include "gamepath.h"
#include <stdlib.h>
#include <string.h>
#include "pserror.h"
#include "pstypes.h"

game_path GamePaths[MAX_GAME_PATHS];
int Num_game_paths = 0;

// Frees gamepath n for future use
void FreeGamePath (int n)
{
	if(!GamePaths[n].used) return;

	mem_free(GamePaths[n].pathnodes);
	mprintf((0, "Path %d lost some\n", n));

	GamePaths[n].num_nodes = 0;
	GamePaths[n].used=0;
	Num_game_paths--;
}

void InitGamePaths()
{
	static bool f_game_paths_init = false;
	int i;

	if(f_game_paths_init)
	{
		// Clear out the current path info
		for(i = 0; i < MAX_GAME_PATHS; i++)
		{
			FreeGamePath(i);
		}
	}

	f_game_paths_init = true;	


	for(i = 0; i < MAX_GAME_PATHS; i++)
	{
		GamePaths[i].num_nodes = 0;
		GamePaths[i].used=0;
	}
	
	Num_game_paths = 0;
}

// searches through GamePath index and returns index of path matching name
// returns -1 if not found
int FindGamePathName (char *name)
{
	int i;

	for (i=0;i<MAX_GAME_PATHS;i++)
	{
		if (GamePaths[i].used && !stricmp (GamePaths[i].name,name))
			return i;
	}
	return -1;
}

