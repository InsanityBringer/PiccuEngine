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

#ifndef OBJECT_LIGHTING_H
#define OBJECT_LIGHTING_H

#include "object.h"

// Casts light from an object onto the rooms or terrain
void DoObjectLight(object *obj);

// Frees all the memory associated with lightmap objects
void ClearAllObjectLightmaps (int terrain);

// Frees all the memory associated with this objects lightmap
void ClearObjectLightmaps (object *obj);

// Sets up the memory to be used by an object for lightmaps
void SetupObjectLightmapMemory (object *obj);

//	makes the an object cloaked
void MakeObjectInvisible(object *obj,float time,float fade_time=1.0,bool no_hud_message=false);

//	makes the player visbile
void MakeObjectVisible(object *obj);

//Returns a pointer to the lighting info for the specified object
light_info *ObjGetLightInfo(object *objp);

//Sets an object to have local lighting info
void ObjSetLocalLighting(object *objp);

#endif
