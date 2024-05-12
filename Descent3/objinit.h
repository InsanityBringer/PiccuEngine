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

#ifndef _OBJINIT_H
#define _OBJINIT_H

#include "object.h"

//Initializes a new object.  All fields not passed in set to defaults.
//Returns 1 if ok, 0 if error
int ObjInit(object *objp,int type,int id,int handle,vector *pos, int roomnum,float creation_time,int parent_handle=OBJECT_HANDLE_NONE);

//Re-copies data to each object from the appropriate page for that object type.
//Called after an object page has changed.
void ObjReInitAll(void);

#endif
