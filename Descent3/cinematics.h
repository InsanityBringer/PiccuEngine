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

#ifndef CINEMATICS_H
#define CINEMATICS_H

#include "renderer.h"
#include "bitmap.h"

//Movie Cinematic
struct tCinematic
{
	unsigned       mvehandle;
	int            filehandle;
	chunked_bitmap frame_chunk;			// used internally, don't access.
};

bool InitCinematics();
void SetMovieProperties(int x, int y, int w, int h, renderer_type type);
bool PlayMovie(const char *moviename);
tCinematic *StartMovie(const char *moviename, bool looping=false);
bool FrameMovie(tCinematic *mve, int x, int y, bool sequence=true);
void EndMovie(tCinematic *mve);

#endif
