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

#ifndef AITERRAIN_H_
#define AITERRAIN_H_

struct ground_information
{
	float highest_y;
	float lowest_y;
};

bool ait_GetGroundInfo(ground_information *ground_info, vector *p0, vector *p1, float rad = 0.0f, angle fov = 0);
void ait_Init(void);

#endif
