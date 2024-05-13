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

#ifndef __SUBTITLES_H_
#define __SUBTITLES_H_

//draw the subtitles for this frame
void SubtDrawSubtitles(int frame_num);
//Shutsdown the subtitle system
void SubtCloseSubtitles();
//Initializes the subtitles for a given movie file
void SubtInitSubtitles(const char *filename);

#endif
