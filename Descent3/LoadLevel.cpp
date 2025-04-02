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

#ifdef NEWEDITOR
#include "..\neweditor\stdafx.h"
#endif

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "LoadLevel.h"

#include "CFILE.H"

#include "descent.h"
#include "object.h"
#include "gametexture.h"

#ifdef NEWEDITOR
#include "..\neweditor\ned_gametexture.h"
#include "..\neweditor\ned_Object.h"
#include "editor\Erooms.h"
#endif

#include "trigger.h"
#include "doorway.h"
#include "terrain.h"
#include "player.h"
#include "door.h"
#include "objinit.h"
#include "room.h"
#include "objinfo.h"
#include "lightmap.h"
#include "lightmap_info.h"
#include "findintersection.h"
#include "polymodel.h"
#include "object_lighting.h"
#include "bsp.h"
#include "gamepath.h"
#include "game.h"
#include "BOA.h"
#include "mem.h"
#include "lighting.h"
#include "Mission.h"
#include "render.h"
#include "weapon.h"
#include "special_face.h"
#include "stringtable.h"
#include "ambient.h"
#include "matcen.h"
#include "dedicated_server.h"
#include "PHYSICS.H"
#include "levelgoal.h"
#include "aiambient.h"
#include "args.h"
#include "ddio.h"
#include "ship.h"
#include "fireball.h"
#include "sounds.h"
#include "soundload.h"
#include "bnode.h"
#include "localization.h"

#ifdef EDITOR
#include "editor\d3edit.h"
#include "editor\HFile.h"
#include "editor\Erooms.h"
#include "editor\moveworld.h"
#include "editor\editor_lighting.h"
#endif

#ifdef NEWEDITOR
#include "..\neweditor\neweditor.h"
#include "..\neweditor\globals.h"
#endif

MD5* Level_md5 = NULL;

char* LocalizeLevelName(char* level);

#define LEVEL_FILE_TAG "D3LV"

// Lightmap remap array
int Num_lightmap_infos_read = 0;
ushort LightmapInfoRemap[MAX_LIGHTMAP_INFOS];

//Arrays for mapping saved data to the current data
short texture_xlate[MAX_TEXTURES];
short door_xlate[MAX_DOORS];
short generic_xlate[MAX_OBJECT_IDS];

#ifdef EDITOR
extern float GlobalMultiplier;
#endif


//Xlate types
#define XT_GENERIC	0
#define XT_DOOR		1

#if (defined(EDITOR) || defined(NEWEDITOR))
//Code to keep track of failed xlate items so can print out a nice message if the item is used

#define MAX_FAILED_XLATE_ITEMS	1500

struct {
	short	type, id;
	char	name[PAGENAME_LEN];
} Failed_xlate_items[MAX_FAILED_XLATE_ITEMS];

int Num_failed_xlate_items;

void AddFailedXLateItem(int type, int id, char* name)
{
	ASSERT(Num_failed_xlate_items < MAX_FAILED_XLATE_ITEMS);

	Failed_xlate_items[Num_failed_xlate_items].type = type;
	Failed_xlate_items[Num_failed_xlate_items].id = id;
	strcpy(Failed_xlate_items[Num_failed_xlate_items].name, name);

	Num_failed_xlate_items++;
}

char* GetFailedXLateItemName(int type, int id)
{
	for (int i = 0; i < Num_failed_xlate_items; i++) {
		if ((Failed_xlate_items[i].type == type) && (Failed_xlate_items[i].id == id))
			return Failed_xlate_items[i].name;
	}

	return "<unknown>";
}

#endif

//Useful macros
#define cf_ReadVector(f,v)  do {(v)->x=cf_ReadFloat(f); (v)->y=cf_ReadFloat(f); (v)->z=cf_ReadFloat(f); } while (0)
#define cf_ReadMatrix(f,m)  do {cf_ReadVector((f),&(m)->rvec); cf_ReadVector((f),&(m)->uvec); cf_ReadVector((f),&(m)->fvec); } while (0)

//Prior to version 49, terrain objects had this flag set
#define OLD_OBJECT_OVER_TERRAIN_FLAG	256

//Macro to translate old file handles (pre-version 94) to new ones
#define OLD_HANDLE_OBJNUM_MASK  		0x3ff			//to mask off the object number part of the handle
#define OLD_HANDLE_COUNT_MASK			0xfffffc00	//to maks off the count part of the handle
#define XLATE_HANDLE(handle) ((((handle) & OLD_HANDLE_COUNT_MASK) << 1) + ((handle) & OLD_HANDLE_OBJNUM_MASK))

/*

  The following struct/array is used for (right now only in OEM) to convert
  a type/id object when loading a level to another type/id.  It's only for
  generic objects.  For the object_convert[]:

	index 0 = old object index 1 = new object
	index 2 = old object index 3 = new object
	...

  Note: Set the id to -2, if it is -2, then I resolve it.
*/
#define CONV_MULTI	0x01
#define CONV_SINGLE	0x02
typedef struct
{
	int type, id;
	char* name;
	ubyte flag;
}tConvertObject;

tConvertObject object_convert[] =
{
#ifdef DEMO
	{OBJ_POWERUP,-2,"Blackshark",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"ImpactMortar",0},
	{OBJ_POWERUP,-2,"Mega",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"ImpactMortar",0},
	{OBJ_POWERUP,-2,"EMDlauncher",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"SuperLaser",0},
	{OBJ_POWERUP,-2,"Omegacannon",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Massdriver",0},
#else
	{OBJ_POWERUP,-2,"Blackshark",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"SuperLaser",0},
	{OBJ_POWERUP,-2,"Mega",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Plasmacannon",0},
	{OBJ_POWERUP,-2,"EMDlauncher",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Microwave",0},
	{OBJ_POWERUP,-2,"Omegacannon",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Massdriver",0},
#endif

#ifdef DEMO
	{OBJ_POWERUP,-2,"Plasmacannon",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"SuperLaser",0},
	{OBJ_POWERUP,-2,"Microwave",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"SuperLaser",0},
	{OBJ_POWERUP,-2,"NapalmRocket",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Homing",0},
	{OBJ_POWERUP,-2,"Guided",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Homing",0},
	{OBJ_POWERUP,-2,"4packGuided",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Homing",0},
	{OBJ_POWERUP,-2,"Cyclone",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"ImpactMortar",0},
#endif
	{OBJ_POWERUP,-2,"Fusioncannon",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Vauss",0},
	{OBJ_POWERUP,-2,"Smart",CONV_MULTI | CONV_SINGLE},{OBJ_POWERUP,-2,"Homing",0},
	{OBJ_POWERUP,-2,"chaff",CONV_MULTI},{OBJ_POWERUP,-2,"energy",0},
	{OBJ_POWERUP,-2,"Betty4pack",CONV_MULTI},{OBJ_POWERUP,-2,"energy",0},
	{OBJ_POWERUP,-2,"Seeker3pack",CONV_MULTI},{OBJ_POWERUP,-2,"energy",0},
	{OBJ_POWERUP,-2,"ProxMinepowerup",CONV_MULTI},{OBJ_POWERUP,-2,"energy",0}
};


int object_convert_size = sizeof(object_convert) / sizeof(tConvertObject);

int  chunk_start, chunk_size, filelen;

int CountDataToPageIn();

//Find a valid, usable ID of the specified type
int FindValidID(int type)
{
	int id = -1;

	switch (type) 
	{
	case OBJ_ROBOT:
	case OBJ_POWERUP:
	case OBJ_BUILDING:
	case OBJ_CLUTTER:
		id = GetObjectID(type);
		break;

	case OBJ_DOOR: 
		{
		int i;

		for (i = 0; i < MAX_DOORS; i++)
			if (Doors[i].used) {
				id = i;
				break;
			}
		ASSERT(i < MAX_DOORS);
		break;
	}

	default:
		Int3();
	}

	return id;
}

//bash/converts generics, from those not allowed, to something that is allowed
void ConvertObject(int* type, int* id)
{
	//NOTE: Only type/id are valid at this point
#if ( (!defined(OEM)) && (!defined(DEMO)) )
	//Non-OEM build
	return;
#else
#ifdef EDITOR
	//Editor OEM?
	return;
#else
	int new_type = *type, new_id = *id;

	bool is_multi = (Game_mode & GM_MULTI) ? true : false;
	bool c_multi, c_single, convert;

	//bash/convert

	//look at even numbered items, they are the object to convert
	//odd number items are destination
	for (int i = 0; i < object_convert_size; i += 2)
	{
		if (*type == object_convert[i].type)
		{
			//type match, check id's
			if (object_convert[i].id == -2)
			{
				object_convert[i].id = FindObjectIDName(object_convert[i].name);
			}

			ASSERT(object_convert[i].id >= 0);

			if (*id == object_convert[i].id)
			{
				convert = false;
				c_multi = (object_convert[i].flag & CONV_MULTI) ? true : false;
				c_single = (object_convert[i].flag & CONV_SINGLE) ? true : false;

				if (is_multi && c_multi)
					convert = true;
				if ((!is_multi) && c_single)
					convert = true;

				//convert!
				if (convert)
				{
					int convert_to = i + 1;

					if (object_convert[convert_to].id == -2)
					{
						object_convert[convert_to].id = FindObjectIDName(object_convert[convert_to].name);
					}

					ASSERT(object_convert[convert_to].id >= 0);

					if (object_convert[convert_to].id >= 0)
					{
						mprintf((0, "LEVELLOAD: Converting: '%s' -> '%s'\n", object_convert[i].name, object_convert[convert_to].name));

						new_id = object_convert[convert_to].id;
						new_type = object_convert[convert_to].type;
					}
				}
			}
		}
	}

	*type = new_type;
	*id = new_id;
#endif
#endif
}

//reads an object
//returns 1 if read ok
int ReadObject(CFILE* ifile, object* objp, int handle, int fileversion)
{
	int type, id, old_id, i;
	int roomnum;
	float door_shields;
	char tempname[OBJ_NAME_LEN + 1] = "";

	type = cf_ReadByte(ifile);

	if (fileversion >= 34)
		id = cf_ReadShort(ifile);
	else
		id = (ubyte)cf_ReadByte(ifile);

	//Translate id 
	old_id = id;
	switch (type) {
	case OBJ_ROBOT:
	case OBJ_POWERUP:
	case OBJ_BUILDING:
	case OBJ_CLUTTER:
		id = generic_xlate[id];
		break;

	case OBJ_DOOR:
		id = door_xlate[id];
		break;
	}
	//Check for object not found
	if (id == -1) 
	{
		id = FindValidID(type);		//find any valid id

		ASSERT(id != -1);

#if (defined(EDITOR) || defined(NEWEDITOR))
		if (GetFunctionMode() == EDITOR_MODE) 
		{
			char* old_name = GetFailedXLateItemName((type == OBJ_DOOR) ? XT_DOOR : XT_GENERIC, old_id);
			char* new_name = (type == OBJ_DOOR) ? Doors[id].name : Object_info[id].name;
			OutrageMessageBox("Object %d (type %s) has undefined ID %d, \"%s\".\n\n"
				"This object will be converted to ID %d, \"%s\".",
				objp - Objects, Object_type_names[type], old_id, old_name, id, new_name);
		}
#endif
	}

	//Convert un-allowed OEM objects
	if (IS_GENERIC(type))
	{
		ConvertObject(&type, &id);
	}

	//Read the object's name
	if (fileversion >= 95) 
	{
		cf_ReadString(tempname, sizeof(tempname), ifile);
		if (strlen(tempname)) 
		{
			if (type == OBJ_PLAYER) 
			{
#if (defined(EDITOR) || defined(NEWEDITOR))
				if (GetFunctionMode() == EDITOR_MODE)
					OutrageMessageBox("Object %d, Player %d has a name (\"%s\") which has been deleted.",
						objp - Objects, id, tempname);
#endif
				tempname[0] = 0;	//kill name
			}
		}
	}

	//Read the flags
	uint flags;
	if (fileversion >= 101)
		flags = cf_ReadInt(ifile);
	else
		flags = (ushort)cf_ReadShort(ifile);

	//Make sure no objects except viewers have the outside mine flags set
	ASSERT((type == OBJ_VIEWER) || !(flags & OF_OUTSIDE_MINE));

	//If a door, read the shields
	if ((type == OBJ_DOOR) && (fileversion >= 109))
		door_shields = cf_ReadShort(ifile);

	//Get the room number
	roomnum = cf_ReadInt(ifile);

	//For old files, check if this object is on terrain
	if (fileversion < 49)
	{
		if (flags & OLD_OBJECT_OVER_TERRAIN_FLAG)
		{
			flags &= ~OLD_OBJECT_OVER_TERRAIN_FLAG;
			roomnum = MAKE_ROOMNUM(roomnum);
		}
	}

	//Get the position
	vector pos;
	cf_ReadVector(ifile, &pos);

	//Initialize the object
	ObjInit(objp, type, id, handle, &pos, roomnum, 0.0);

	//Set the stuff we've already read in
	objp->flags |= (flags & OBJECT_SAVE_LOAD_FLAGS);
	objp->roomnum = roomnum;

	//Set shields if this is a door
	if ((type == OBJ_DOOR) && (fileversion >= 109))
		objp->shields = door_shields;

	//Do some wacky weapon thing
	if (fileversion <= 36)
	{
		if (objp->type == OBJ_PLAYER || objp->type == OBJ_POWERUP || objp->type == OBJ_ROBOT ||
			objp->type == OBJ_BUILDING || objp->type == OBJ_DOOR || objp->type == OBJ_DEBRIS)
			objp->flags |= OF_POLYGON_OBJECT;
		if (objp->type == OBJ_WEAPON)
		{
			if (!((Weapons[objp->id].flags & WF_IMAGE_BITMAP) || (Weapons[objp->id].flags & WF_IMAGE_VCLIP)))
				objp->flags |= OF_POLYGON_OBJECT;
		}
	}

	//Load and set the orientation
	matrix orient;
	cf_ReadMatrix(ifile, &orient);
	vm_Orthogonalize(&orient);
	ObjSetOrient(objp, &orient);
	ObjSetAABB(objp);

	//Set the name
	if (tempname[0]) 
	{
		objp->name = (char*)mem_malloc(strlen(tempname) + 1);
		strcpy(objp->name, tempname);
	}

	// Update checksum
	AppendToLevelChecksum(objp->pos.x);
	AppendToLevelChecksum(objp->pos.y);
	AppendToLevelChecksum(objp->pos.z);

	objp->contains_type = cf_ReadByte(ifile);
	objp->contains_id = cf_ReadByte(ifile);
	objp->contains_count = cf_ReadByte(ifile);

	objp->lifeleft = cf_ReadFloat(ifile);

	if ((fileversion >= 65) && (fileversion < 111))
		cf_ReadInt(ifile);		//was parent_handle

	//Read sound info if this is a soundsource object
	if (objp->control_type == CT_SOUNDSOURCE) 
	{
		ASSERT(objp->type == OBJ_SOUNDSOURCE);
		if (fileversion < 119)
			objp->ctype.soundsource_info.sound_index = cf_ReadInt(ifile);
		else 
		{
			char soundname[PAGENAME_LEN];
			cf_ReadString(soundname, sizeof(soundname), ifile);
			objp->ctype.soundsource_info.sound_index = soundname[0] ? FindSoundName(IGNORE_TABLE(soundname)) : -1;
		}
		objp->ctype.soundsource_info.volume = cf_ReadFloat(ifile);
	}

	if (fileversion >= 105)
	{
		//read in default script override information
		i = cf_ReadByte(ifile);
		if (i > 0) 
		{
			objp->custom_default_script_name = (char*)mem_malloc(i + 1);
			cf_ReadBytes((ubyte*)objp->custom_default_script_name, i, ifile);
			objp->custom_default_script_name[i] = '\0';
		}
		else 
		{
			objp->custom_default_script_name = NULL;
		}

		i = cf_ReadByte(ifile);
		if (i > 0) 
		{
			objp->custom_default_module_name = (char*)mem_malloc(i + 1);
			cf_ReadBytes((ubyte*)objp->custom_default_module_name, i, ifile);
			objp->custom_default_module_name[i] = '\0';
		}
		else 
		{
			objp->custom_default_module_name = NULL;
		}
	}
	else
	{
		objp->custom_default_module_name = NULL;
		objp->custom_default_script_name = NULL;
	}

	//	load in obsolete script info
	if (fileversion < LEVEL_FILE_OSIRIS1DEAD) 
	{
		if (fileversion < LEVEL_FILE_SCRIPTNAMES) 
		{
			cf_ReadInt(ifile);
		}
		else 
		{
			char name[MAX_D3XID_NAME];
			cf_ReadString(name, MAX_D3XID_NAME, ifile);
		}

		if (fileversion >= LEVEL_FILE_SCRIPTPARMS) 
		{
			short s = cf_ReadShort(ifile);

			for (i = 0; i < s; i++)
			{
				sbyte stype = cf_ReadByte(ifile);
				if (stype == PARMTYPE_NUMBER || stype == PARMTYPE_REF) 
				{
					cf_ReadFloat(ifile);
				}
				else if (stype == PARMTYPE_VECTOR) 
				{
					cf_ReadFloat(ifile);
					cf_ReadFloat(ifile);
					cf_ReadFloat(ifile);
				}
				else {
					Int3();				//	-get samir.
				}
			}
		}


		if (fileversion >= LEVEL_FILE_SCRIPTCHECK) 
		{
			cf_ReadByte(ifile);
		}
	}

	//	lightmap data.
	int lmdata;
	if (fileversion <= 34)
		lmdata = 0;
	else
		lmdata = cf_ReadByte(ifile);


	if (lmdata)	// lightmap data follows
	{
		poly_model* pm = &Poly_models[objp->rtype.pobj_info.model_num];
		int num_models, num_faces, num_verts;
		int i, t, k;
		int model_changed = 0, clear_lightmaps = 0;

		num_models = cf_ReadByte(ifile);
		if (pm->n_models != num_models)
		{
			model_changed = 1;
			mprintf((0, "Polymodel %s has changed since this level was lit!\n", pm->name));
		}

		if (!model_changed)
			SetupObjectLightmapMemory(objp);

		for (i = 0; i < num_models; i++)
		{
			int submodel_changed = 0;
			num_faces = cf_ReadShort(ifile);

			if (num_faces != objp->lm_object.num_faces[i])
			{
				submodel_changed = 1;
				clear_lightmaps = 1;
			}

			for (t = 0; t < num_faces; t++)
			{
				if (submodel_changed == 0 && model_changed == 0)
				{
					lightmap_object_face* fp = &objp->lm_object.lightmap_faces[i][t];
					fp->lmi_handle = LightmapInfoRemap[(ushort)cf_ReadShort(ifile)];

					if (!Dedicated_server)
						LightmapInfo[fp->lmi_handle].used++;
					else
						clear_lightmaps = 1;

					if (fileversion <= 88)
					{
						cf_ReadByte(ifile);
						cf_ReadByte(ifile);
						cf_ReadByte(ifile);
						cf_ReadByte(ifile);
					}

					if (fileversion >= 58)
					{
						// Read normal,uvec,rvec
						if (fileversion <= 59)
						{
							vector tvec;
							cf_ReadVector(ifile, &tvec);
						}
						cf_ReadVector(ifile, &fp->rvec);
						cf_ReadVector(ifile, &fp->uvec);
					}
					else
					{
						vm_MakeZero(&fp->rvec);
						vm_MakeZero(&fp->uvec);
						fp->rvec.x = 1;
						fp->uvec.y = 1;
					}

					num_verts = cf_ReadByte(ifile);
					if (num_verts == fp->num_verts)
					{
						for (k = 0; k < num_verts; k++)
						{
							fp->u2[k] = cf_ReadFloat(ifile);
							fp->v2[k] = cf_ReadFloat(ifile);
						}
					}
					else
					{
						for (k = 0; k < num_verts; k++)
						{
							cf_ReadFloat(ifile);
							cf_ReadFloat(ifile);
						}
					}
				}
				else
				{
					cf_ReadShort(ifile);
					if (fileversion <= 88)
					{
						cf_ReadByte(ifile);
						cf_ReadByte(ifile);
						cf_ReadByte(ifile);
						cf_ReadByte(ifile);
					}

					if (fileversion >= 58)
					{
						vector tvec;

						// Read normal,uvec,rvec
						if (fileversion <= 59)
						{
							cf_ReadVector(ifile, &tvec);
						}
						cf_ReadVector(ifile, &tvec);
						cf_ReadVector(ifile, &tvec);
					}

					num_verts = cf_ReadByte(ifile);
					for (k = 0; k < num_verts; k++)
					{
						cf_ReadFloat(ifile);
						cf_ReadFloat(ifile);
					}
				}
			}
		}

		if (clear_lightmaps)
		{
			mprintf((0, "Freeing lightmaps because model %s  has changed since this level was saved!\n", pm->name));
			ClearObjectLightmaps(objp);
		}

	}
	return 1;
}

//Old portal trigger flag
#define OLD_TF_PORTAL 1

//Reads a trigger
//Returns: true if read ok, else false
int ReadTrigger(CFILE* ifile, trigger* tp, int fileversion)
{
	if (fileversion >= 99)
		cf_ReadString(tp->name, sizeof(tp->name), ifile);
	else
		tp->name[0] = 0;

	tp->roomnum = cf_ReadShort(ifile);
	tp->facenum = cf_ReadShort(ifile);
	tp->flags = cf_ReadShort(ifile);
	tp->activator = cf_ReadShort(ifile);

	//Kill old portal trigger flag
	if (fileversion < 103) 
	{
		if (tp->flags & OLD_TF_PORTAL) 
		{
			tp->facenum = Rooms[tp->roomnum].portals[tp->facenum].portal_face;
			tp->flags &= ~OLD_TF_PORTAL;
		}
	}

	ASSERT(Rooms[tp->roomnum].faces[tp->facenum].flags & FF_HAS_TRIGGER);

	if (fileversion < LEVEL_FILE_OSIRIS1DEAD) 
	{
		if (fileversion < LEVEL_FILE_SCRIPTNAMES)
			cf_ReadInt(ifile);
		else 
		{
			char name[MAX_D3XID_NAME];
			cf_ReadString(name, MAX_D3XID_NAME, ifile);
			if (fileversion >= LEVEL_FILE_TRIGPARMS) 
			{
				int i;
				short s = cf_ReadShort(ifile);
				for (i = 0; i < s; i++)
				{
					sbyte type = cf_ReadByte(ifile);
					if (type == PARMTYPE_NUMBER || type == PARMTYPE_REF) 
					{
						cf_ReadFloat(ifile);
					}
					else if (type == PARMTYPE_VECTOR) 
					{
						cf_ReadFloat(ifile);
						cf_ReadFloat(ifile);
						cf_ReadFloat(ifile);
					}
					else 
					{
						Int3();				//	-get samir.
					}
				}
			}
		}

		if (fileversion >= LEVEL_FILE_SCRIPTCHECK) 
		{
			cf_ReadByte(ifile);
		}
	}

	return 1;
}

//Old portal trigger face flag
#define OLD_FF_PORTAL_TRIG 0x0020

//Reads a face from a disk file
//Parameters:	ifile - file to read from
//					fp - face to read
//					version - the version number of the file being read
//Returns:		1 if read ok, else 0
int ReadFace(CFILE* ifile, face* fp, int version)
{
	int nverts, i;

	//Get number of verts
	nverts = cf_ReadByte(ifile);

	//Initialize & alloc memory
	InitRoomFace(fp, nverts);

	//Read vertices
	for (i = 0; i < fp->num_verts; i++)
		fp->face_verts[i] = cf_ReadShort(ifile);

	//Read uvls, and adjust alpha settings
	int alphaed = 0;

	for (i = 0; i < fp->num_verts; i++) 
	{
		fp->face_uvls[i].u = cf_ReadFloat(ifile);
		fp->face_uvls[i].v = cf_ReadFloat(ifile);

		if (version < 56)
		{
			// Read old lrgb stuff
			cf_ReadFloat(ifile);
			cf_ReadFloat(ifile);
			cf_ReadFloat(ifile);
			cf_ReadFloat(ifile);
		}

		if (version >= 21)
		{
			if (version < 61)
				fp->face_uvls[i].alpha = Float_to_ubyte(cf_ReadFloat(ifile));
			else
				fp->face_uvls[i].alpha = cf_ReadByte(ifile);
		}
		else
			fp->face_uvls[i].alpha = 1.0;

		if (fp->face_uvls[i].alpha != 255)
			alphaed = 1;
	}

	//Read flags
	if (version < 27)
		fp->flags = cf_ReadByte(ifile);
	else
		fp->flags = cf_ReadShort(ifile);

	//Kill old portal trigger flag
	if (version < 103)
		fp->flags &= ~OLD_FF_PORTAL_TRIG;

	//Set vertex alpha flag
	if (alphaed)
		fp->flags |= FF_VERTEX_ALPHA;
	else
		fp->flags &= ~FF_VERTEX_ALPHA;

	//Read the portal number
	if (version >= 23)
		fp->portal_num = cf_ReadByte(ifile);
	else
		fp->portal_num = cf_ReadShort(ifile);

	//Read and translate the texture number
	fp->tmap = texture_xlate[cf_ReadShort(ifile)];

	//Check for failed xlate
	if (fp->tmap == -1) {
		fp->tmap = 0;
	}

	// Check to see if there is a lightmap
	if ((fp->flags & FF_LIGHTMAP) && (version >= 19))
	{
		if (version <= 29)
		{
			ubyte w, h;

			w = cf_ReadByte(ifile);
			h = cf_ReadByte(ifile);

			for (i = 0; i < w * h; i++)
				cf_ReadShort(ifile);

			fp->flags &= ~FF_LIGHTMAP;
		}
		else
		{
			// Read lightmap info handle
			int lmi_handle = (ushort)cf_ReadShort(ifile);
			if (!Dedicated_server)
			{
				fp->lmi_handle = LightmapInfoRemap[lmi_handle];
				LightmapInfo[fp->lmi_handle].used++;
			}

			if (version <= 88)
			{
				cf_ReadByte(ifile);
				cf_ReadByte(ifile);
				cf_ReadByte(ifile);
				cf_ReadByte(ifile);
			}
		}


		// Read UV2s
		for (i = 0; i < fp->num_verts; i++)
		{
			fp->face_uvls[i].u2 = cf_ReadFloat(ifile);
			fp->face_uvls[i].v2 = cf_ReadFloat(ifile);

			// Stupid fix for bad lightmap uvs
			if (fp->face_uvls[i].u2 < 0)
				fp->face_uvls[i].u2 = 0;
			if (fp->face_uvls[i].u2 > 1)
				fp->face_uvls[i].u2 = 1.0;

			if (fp->face_uvls[i].v2 < 0)
				fp->face_uvls[i].v2 = 0;
			if (fp->face_uvls[i].v2 > 1)
				fp->face_uvls[i].v2 = 1.0;

		}
	}

	if (version >= 22 && version <= 29)
	{
		vector vec;
		cf_ReadVector(ifile, &vec);
	}

	if (version >= 40 && version <= 60) // was shadow room,face
	{
		cf_ReadShort(ifile);
		cf_ReadShort(ifile);
	}

	if (version >= 50)
	{
		fp->light_multiple = cf_ReadByte(ifile);

		if (fp->light_multiple == 186)
			fp->light_multiple = 4;	// Get Jason, I'm looking for this bug!  Its safe to go past it, but I'm just on the lookout


		if (version <= 52)
		{
			if (fp->light_multiple >= 32)
				Int3();	// Get Jason
			fp->light_multiple *= 4;
		}
	}
	else
		fp->light_multiple = 4;

	if (version >= 71)
	{
		ubyte special = cf_ReadByte(ifile);
		if (special)
		{
			if (version < 77)	// Ignore old specular data
			{
				vector center;
				cf_ReadByte(ifile);
				cf_ReadVector(ifile, &center);
				cf_ReadShort(ifile);
			}
			else
			{
				vector center;

				ubyte smooth = 0;
				ubyte num_smooth_verts = 0;
				ubyte type = cf_ReadByte(ifile);
				ubyte num = cf_ReadByte(ifile);

				if (version >= 117)
				{
					// Read if smoothed
					smooth = cf_ReadByte(ifile);
					if (smooth)
					{
						num_smooth_verts = cf_ReadByte(ifile);
						fp->special_handle = AllocSpecialFace(type, num, true, num_smooth_verts);
					}
					else
						fp->special_handle = AllocSpecialFace(type, num);

				}
				else
					fp->special_handle = AllocSpecialFace(type, num);

				ASSERT(fp->special_handle != BAD_SPECIAL_FACE_INDEX);

				for (i = 0; i < num; i++)
				{
					cf_ReadVector(ifile, &center);
					ushort color = cf_ReadShort(ifile);

					SpecialFaces[fp->special_handle].spec_instance[i].bright_center = center;
					SpecialFaces[fp->special_handle].spec_instance[i].bright_color = color;
				}

				if (smooth)
				{
					for (i = 0; i < num_smooth_verts; i++)
					{
						vector vertnorm;
						cf_ReadVector(ifile, &vertnorm);
						SpecialFaces[fp->special_handle].vertnorms[i] = vertnorm;
					}
				}

			}
		}
	}

	return 1;
}

//Old has trigger portal flag
#define OLD_PF_HAS_TRIGGER 4

//Reads a portal to a disk file
//Parameters:	ifile - file to Read to
//					pp - portal to Read
//Returns:		1 if written ok, else 0
int ReadPortal(CFILE* ifile, portal* pp, int version)
{
	int i, num_faces;

	pp->flags = cf_ReadInt(ifile);

	//Kill old trigger flag
	if (version < 103)
		pp->flags &= ~OLD_PF_HAS_TRIGGER;

	if (version < 80) {		//read old list of portal verts
		int num_verts = cf_ReadShort(ifile);
		for (i = 0; i < num_verts; i++)
			cf_ReadShort(ifile);		//was pp->portal_verts[i]

		num_faces = cf_ReadShort(ifile);
		ASSERT(num_faces == 1);
	}

	pp->portal_face = cf_ReadShort(ifile);

	pp->croom = cf_ReadInt(ifile);
	pp->cportal = cf_ReadInt(ifile);

	if (version >= 123)
		pp->bnode_index = cf_ReadShort(ifile);
	else
		pp->bnode_index = -1;

	if (version >= 63)
	{
		cf_ReadVector(ifile, &pp->path_pnt);
	}

	if (version >= 100)
		pp->combine_master = cf_ReadInt(ifile);

	return 1;
}

int n_degenerate_faces_removed;

//Remove faces that have no area, as determined by their surface normal being NULL
//This code should come out after the clipper is fixed to not create degenerate faces
#ifdef EDITOR
void RemoveDegenerateFaces(room* rp)
{
	int f;
	int n_removed = 0;

	for (f = rp->num_faces - 1; f >= 0; f--) {
		face* fp = &rp->faces[f];

		if ((fp->normal.x == 0.0) && (fp->normal.y == 0.0) && (fp->normal.z == 0.0)) {
			mprintf((0, "Deleting face %d from room %d\n", f, ROOMNUM(rp)));
			DeleteRoomFace(rp, f);
			n_degenerate_faces_removed++;
		}
	}
}
#endif

#define NO_COMPRESS	0
#define COMPRESS	1

// Does a RLE compression run of the values given the byte array 'val'.  
int WriteCompressionByte(CFILE* fp, ubyte* val, int total, int just_count, int compress)
{
	int done = 0;
	int written = 0;
	int curptr = 0;

	ASSERT(!(just_count == 1 && compress == 0));

	if (compress == NO_COMPRESS)
	{
		int i;

		// Write 0 to indicate no compression
		cf_WriteByte(fp, NO_COMPRESS);
		for (i = 0; i < total; i++)
			cf_WriteByte(fp, val[i]);


		return total;
	}
	else
	{
		// Indicate compression
		if (!just_count)
			cf_WriteByte(fp, COMPRESS);
	}



	while (!done)
	{
		if (curptr == total)
		{
			done = 1;
			continue;
		}

		ASSERT(curptr < total);

		ubyte curval = val[curptr];
		ubyte count = 1;

		while ((curptr + count) < total && val[curptr + count] == curval && count < 250)
			count++;

		written += 2;

		if (just_count)
		{
			curptr += count;
			continue;
		}

		if (count == 1)
		{
			cf_WriteByte(fp, 0);
			cf_WriteByte(fp, curval);
		}
		else
		{
			cf_WriteByte(fp, count);
			cf_WriteByte(fp, curval);
		}

		curptr += count;
	}

	return written;

}

// Does a RLE compression run of the values given the short array 'val'.  
// If just_count is 1, doesn't write anything
int WriteCompressionShort(CFILE* fp, ushort* val, int total, int just_count, int compress)
{
	int done = 0, written = 0;

	int curptr = 0;


	ASSERT(!(just_count == 1 && compress == 0));

	if (compress == NO_COMPRESS)
	{
		int i;

		// Write 0 to indicate no compression
		cf_WriteByte(fp, NO_COMPRESS);

		for (i = 0; i < total; i++)
			cf_WriteShort(fp, val[i]);

		return total;
	}
	else
	{
		// Indicate compression
		if (!just_count)
			cf_WriteByte(fp, COMPRESS);
	}

	while (!done)
	{
		if (curptr == total)
		{
			done = 1;
			continue;
		}

		ASSERT(curptr < total);

		ushort curval = val[curptr];
		ubyte count = 1;

		while ((curptr + count) < total && val[curptr + count] == curval && count < 250)
			count++;

		written += 3;

		if (just_count)
		{
			curptr += count;
			continue;
		}

		if (count == 1)
		{
			cf_WriteByte(fp, 0);
			cf_WriteShort(fp, curval);
		}
		else
		{
			cf_WriteByte(fp, count);
			cf_WriteShort(fp, curval);
		}

		curptr += count;
	}

	return written;

}


// Given an array of values, checks to see if it would be better to write it out
// as a raw array or RLE array
void CheckToWriteCompressByte(CFILE* fp, ubyte* vals, int total)
{
	int count = WriteCompressionByte(fp, vals, total, 1, COMPRESS);

	if (count >= total)
		WriteCompressionByte(fp, vals, total, 0, NO_COMPRESS);
	else
		WriteCompressionByte(fp, vals, total, 0, COMPRESS);
}

// Given an array of values, checks to see if it would be better to write it out
// as a raw array or RLE array
void CheckToWriteCompressShort(CFILE* fp, ushort* vals, int total)
{
	int count = WriteCompressionShort(fp, vals, total, 1, COMPRESS);

	if (count >= total)
		WriteCompressionShort(fp, vals, total, 0, NO_COMPRESS);
	else
		WriteCompressionShort(fp, vals, total, 0, COMPRESS);
}



void ReadCompressionByte(CFILE* fp, ubyte* vals, size_t total)
{
	size_t count = 0;
	ubyte compressed = cf_ReadByte(fp);

	if (compressed == 0)
	{
		for (size_t i = 0; i < total; i++)
			vals[i] = cf_ReadByte(fp);

		return;
	}

	while (count < total)
	{
		ubyte command = cf_ReadByte(fp);

		if (command == 0)	// next byte is raw
		{
			ubyte height = cf_ReadByte(fp);

			vals[count] = height;
			count++;
		}
		else if (command >= 2 && command <= 250)	// next pixel is run of pixels
		{
			//Fine for count to reach total exactly, since the loop will stop running then. 
			if ((count + command) > total) //[ISB]: Should be no cases where this will ever reach 4GB - 1, so should be safe?
				Error("ReadCompressionByte: compression buffer overrun");
			ubyte height = cf_ReadByte(fp);
			for (int k = 0; k < command; k++)
			{
				vals[count] = height;
				count++;
			}
		}
		else Int3();		// bad compression run
	}
}

void ReadCompressionShort(CFILE* fp, ushort* vals, size_t total)
{
	size_t count = 0;

	ubyte compressed = cf_ReadByte(fp);

	if (compressed == 0)
	{
		for (int i = 0; i < total; i++)
			vals[i] = cf_ReadShort(fp);

		return;
	}

	while (count < total)
	{
		ubyte command = cf_ReadByte(fp);

		if (command == 0)	// next byte is raw
		{
			ushort height = cf_ReadShort(fp);

			vals[count] = height;
			count++;
		}
		else if (command >= 2 && command <= 250)	// next pixel is run of pixels
		{
			if ((count + command) > total) //[ISB]: Should be no cases where this will ever reach 4GB - 1, so should be safe?
				Error("ReadCompressionShort: compression buffer overrun");
			ushort height = cf_ReadShort(fp);
			for (int k = 0; k < command; k++)
			{
				vals[count] = height;
				count++;
			}
		}
		else Int3();		// bad compression run
	}
}


//Reads a room from a disk file
//Parameters:	ifile - file to read from
//					rp - room to read
//					version - version number of file
//Returns:		1 if read ok, else 0
int ReadRoom(CFILE* ifile, room* rp, int version)
{
	int i, nverts, nfaces, nportals;

	ASSERT(rp->used == 0);		//should be free

	//Get counts for room elements
	nverts = cf_ReadInt(ifile);
	nfaces = cf_ReadInt(ifile);
	nportals = cf_ReadInt(ifile);

	//Initialize the room
	InitRoom(rp, nverts, nfaces, nportals);

	//Get room name
	rp->name = NULL;
	if (version >= 96) {
		char tempname[ROOM_NAME_LEN + 1];
		cf_ReadString(tempname, sizeof(tempname), ifile);
		if (strlen(tempname)) {
			rp->name = (char*)mem_malloc(strlen(tempname) + 1);
			strcpy(rp->name, tempname);
		}
	}

	//Read path point
	if (version >= 63)
	{
		cf_ReadVector(ifile, &rp->path_pnt);
	}

	//Read in verts
	for (i = 0; i < rp->num_verts; i++)
	{
		cf_ReadVector(ifile, &rp->verts[i]);
		if (version >= 52 && version <= 67)
		{
			cf_ReadByte(ifile);
			cf_ReadByte(ifile);
		}
		else if (version >= 68 && version < 71)
		{
			vector tempv;
			cf_ReadVector(ifile, &tempv);
			cf_ReadShort(ifile);
		}

		// Update checksum
		AppendToLevelChecksum(rp->verts[i]);

	}

	//Read in faces
	for (i = 0; i < rp->num_faces; i++) {
		bool t;

		//Load the face
		ReadFace(ifile, &rp->faces[i], version);

		for (int k = 0; k < rp->faces[i].num_verts; k++)
		{
			AppendToLevelChecksum(rp->faces[i].face_verts[k]);
		}
		AppendToLevelChecksum(rp->faces[i].tmap);

		//Get the surface normal for this face
		// Note we're not computing the checksum on the normal because of
		// differences between cpu architectures having different internal precision
		// models for floating point. Instead we took the face verts into account above.
		t = ComputeFaceNormal(rp, i);

		//Check for bad normal
		if (!t)
		{
			mprintf((1, "WARNING:  Room %d face %d has bad normal!\n", rp - Rooms, i));
		}
	}

	//Read in portals
	for (i = 0; i < rp->num_portals; i++)
		ReadPortal(ifile, &rp->portals[i], version);

	//Read rest of fields
	rp->flags = cf_ReadInt(ifile);
	if (version < 29)
		cf_ReadFloat(ifile);		//was rp->static_light

	if (version >= 68)
	{
		rp->pulse_time = cf_ReadByte(ifile);
		rp->pulse_offset = cf_ReadByte(ifile);
	}

	if (version >= 79)
	{
		rp->mirror_face = cf_ReadShort(ifile);
	}
	else
		rp->mirror_face = -1;

	if ((rp->flags & RF_DOOR))
	{
		int doornum, flags = 0, keys = 0, position = 0.0;

		if (version >= 28 && version <= 32)
		{
			doornum = door_xlate[cf_ReadInt(ifile)];
			cf_ReadFloat(ifile);	//was open_time
			cf_ReadFloat(ifile);	//was wait_time
			cf_ReadFloat(ifile);	//was close_time
		}
		else if (version >= 33)
		{
			if (version < 106)
				cf_ReadInt(ifile);	//was doorway_num

			flags = cf_ReadByte(ifile);
			if (version < 106)
				flags |= DF_AUTO;

			if (version >= 36)
				keys = cf_ReadByte(ifile);

			doornum = door_xlate[cf_ReadInt(ifile)];

			if (version >= 106)
				position = cf_ReadFloat(ifile);

			if (version < 106) {
				cf_ReadFloat(ifile);	//was open_time
				cf_ReadFloat(ifile);	//was wait_time
				cf_ReadFloat(ifile);	//was close_time
			}
		}
		else
		{
			Error("ReadRoom: Found door room from unsupported level version!");
		}

		if (doornum == -1)
			doornum = FindValidID(OBJ_DOOR);		//find any valid id

		ASSERT(doornum != -1);
		doorway* dp = DoorwayAdd(rp, doornum);
		dp->position = position;
		dp->flags = flags;
		dp->keys_needed = keys;
		dp->dest_pos = dp->position;
	}

	if (version >= 67)
	{
		// Read in volume lights
		if (cf_ReadByte(ifile) == 1)
		{
			int w = cf_ReadInt(ifile);
			int h = cf_ReadInt(ifile);
			int d = cf_ReadInt(ifile);

			int size = w * h * d;

			if (size) 
			{
				rp->volume_lights = (ubyte*)mem_malloc(size);
			}
			else
				rp->volume_lights = NULL;

			ReadCompressionByte(ifile, rp->volume_lights, w * h * d);

			rp->volume_width = w;
			rp->volume_height = h;
			rp->volume_depth = d;


		}
	}

	if (version >= 73)
	{
		// Read fog stuff
		rp->fog_depth = cf_ReadFloat(ifile);
		rp->fog_r = cf_ReadFloat(ifile);
		rp->fog_g = cf_ReadFloat(ifile);
		rp->fog_b = cf_ReadFloat(ifile);
	}

	//Read ambient sound pattern name
	if (version >= 78) {
		char tbuf[PAGENAME_LEN];
		cf_ReadString(tbuf, sizeof(tbuf), ifile);
		rp->ambient_sound = FindAmbientSoundPattern(tbuf);
	}
	else
		rp->ambient_sound = -1;

	//	read reverb value for room.
	rp->env_reverb = (version >= 98) ? ((ubyte)cf_ReadByte(ifile)) : 0;

	//Read damage
	if (version >= 108) {
		rp->damage = cf_ReadFloat(ifile);
		rp->damage_type = cf_ReadByte(ifile);
	}

#if (defined(EDITOR) || defined(NEWEDITOR))
	//Get rid of bogus faces
	//RemoveDegenerateFaces(rp);
#endif


	if (Katmai)
	{
		// If katmai, copy all of our verts into our verts4 array
		for (i = 0; i < nverts; i++)
		{
			rp->verts4[i].x = rp->verts[i].x;
			rp->verts4[i].y = rp->verts[i].y;
			rp->verts4[i].z = rp->verts[i].z;
		}
	}

	return 1;
}

//Build a translation table for various items
void BuildXlateTable(CFILE* ifile, int (*lookup_func)(char*), short* xlate_table, int max_items, int type)
{
	char name[PAGENAME_LEN];
	int n;
	int i;

	n = cf_ReadInt(ifile); 								// get num items
	for (i = 0; i < n; i++) {

		cf_ReadString(name, sizeof(name), ifile);	// get old name

		if (!name[0])										//if null name, don't look up
			xlate_table[i] = -1;
		else {												//look up, and warn if not found
			xlate_table[i] = lookup_func(name);		// returns -1 if not found
#if (defined(EDITOR) || defined(NEWEDITOR))
			if ((xlate_table[i] == -1) && (type != -1))
				AddFailedXLateItem(type, i, name);
#endif
		}
	}
	for (; i < max_items; i++)								// clear out rest of array
		xlate_table[i] = -1;
}

#define ISCHUNK(name)	(!strncmp(chunk_name,name,4))


// Reads info about our lightmaps
void ReadNewLightmapChunk(CFILE* fp, int version)
{
	int nummaps;
	int i;

	//Don't read lightmaps for dedicated server
	if (Dedicated_server) {
		Num_lightmap_infos_read = 0;	//I don't think this will be used, but clear it just in case
		return;
	}

	ushort* lightmap_remap = (ushort*)mem_malloc(MAX_LIGHTMAPS * sizeof(ushort));

	nummaps = cf_ReadInt(fp);

	mprintf((0, "Reading %d unique lightmaps\n", nummaps));


	for (i = 0; i < nummaps; i++)
	{
		int w, h;
		if (nummaps >= 5)
		{
			if ((i % (nummaps / 5)) == 0)
			{
				LoadLevelProgress(LOAD_PROGRESS_LOADING_LEVEL, (filelen) ? (float)(chunk_start + ((((chunk_size) / nummaps) * i) / 2)) / (float)filelen : 0.0f, CHUNK_LIGHTMAPS);
			}
		}
		w = cf_ReadShort(fp);
		h = cf_ReadShort(fp);

		int lm_handle = lm_AllocLightmap(w, h);

		lightmap_remap[i] = lm_handle;
		ushort* data = (ushort*)lm_data(lm_handle);
		ReadCompressionShort(fp, data, w * h);
	}

	nummaps = cf_ReadInt(fp);

	Num_lightmap_infos_read = nummaps;
	ASSERT(nummaps < MAX_LIGHTMAP_INFOS);

	for (i = 0; i < nummaps; i++)
	{
		int w, h, lmi;
		ubyte type;
		if (nummaps >= 50)
		{
			if ((i % (nummaps / 50)) == 0)
			{
				LoadLevelProgress(LOAD_PROGRESS_LOADING_LEVEL, (filelen) ? (float)(chunk_start + (chunk_size / 2) + (((chunk_size / 2) / nummaps) * i)) / (float)filelen : 0.0f, CHUNK_LIGHTMAPS);
			}
		}
		int remap_handle = cf_ReadShort(fp);
		w = cf_ReadShort(fp);
		h = cf_ReadShort(fp);
		type = cf_ReadByte(fp);

		if (!Dedicated_server)
		{
			lmi = AllocLightmapInfo(w, h, type, false);
			ASSERT(lmi != BAD_LMI_INDEX);
			LightmapInfoRemap[i] = lmi;
			LightmapInfo[lmi].lm_handle = lightmap_remap[remap_handle];
		}

		if (version >= 91)
		{
			int x1 = cf_ReadShort(fp);
			int y1 = cf_ReadShort(fp);

			if (!Dedicated_server)
			{
				LightmapInfo[lmi].x1 = x1;
				LightmapInfo[lmi].y1 = y1;
			}
		}

		vector ul, norm;

		int xspacing = cf_ReadByte(fp);
		int yspacing = cf_ReadByte(fp);
		cf_ReadVector(fp, &ul);
		cf_ReadVector(fp, &norm);

		if (!Dedicated_server)
		{
			LightmapInfo[lmi].xspacing = xspacing;
			LightmapInfo[lmi].yspacing = yspacing;

			LightmapInfo[lmi].upper_left = ul;
			LightmapInfo[lmi].normal = norm;
		}
	}

	mem_free(lightmap_remap);
}

// Reads info about our lightmaps
void ReadLightmapChunk(CFILE* fp, int version)
{
	//Don't read lightmaps for dedicated server
	if (Dedicated_server) 
	{
		Num_lightmap_infos_read = 0;	//I don't think this will be used, but clear it just in case
		return;
	}

	int nummaps;
	int i, t;
	ushort* ded_dummy_data;

	if (Dedicated_server)
	{
		ded_dummy_data = (ushort*)mem_malloc(128 * 128 * 2);
		ASSERT(ded_dummy_data);
	}

	nummaps = cf_ReadInt(fp);
	Num_lightmap_infos_read = nummaps;
	ASSERT(nummaps < MAX_LIGHTMAP_INFOS);

	for (i = 0; i < nummaps; i++)
	{
		if ((i % 10) == 0)
		{
			LoadLevelProgress(LOAD_PROGRESS_LOADING_LEVEL, (filelen) ? (float)chunk_start + ((float)chunk_size / nummaps) / (float)filelen : 0.0f, CHUNK_LIGHTMAPS);
		}
		int lmi;

		int w = cf_ReadInt(fp);
		int h = cf_ReadInt(fp);
		ubyte type = cf_ReadByte(fp);

		if (!Dedicated_server)
		{
			lmi = AllocLightmapInfo(w, h, type);
			ASSERT(lmi != BAD_LMI_INDEX);
			LightmapInfoRemap[i] = lmi;
		}

		if (version <= 38)
		{
			LightmapInfo[lmi].xspacing = (float)cf_ReadByte(fp);
			LightmapInfo[lmi].yspacing = (float)cf_ReadByte(fp);
		}
		else if (version < 66)
		{
			LightmapInfo[lmi].xspacing = cf_ReadFloat(fp);
			LightmapInfo[lmi].yspacing = cf_ReadFloat(fp);
		}
		else
		{
			int x = cf_ReadByte(fp);
			int y = cf_ReadByte(fp);

			if (!Dedicated_server)
			{
				LightmapInfo[lmi].xspacing = x;
				LightmapInfo[lmi].yspacing = y;
			}
		}
		vector v;

		cf_ReadVector(fp, &v);
		if (!Dedicated_server)
			LightmapInfo[lmi].upper_left = v;

		if (version < 58)	// Don't read centers anymore
		{
			vector tvec;
			cf_ReadVector(fp, &tvec); // used to be normal
			cf_ReadVector(fp, &tvec); // used to be center
		}
		if (version >= 59)
		{
			cf_ReadVector(fp, &v);
			if (!Dedicated_server)
				LightmapInfo[lmi].normal = v;
		}
		else
		{
			LightmapInfo[lmi].normal.x = 0;
			LightmapInfo[lmi].normal.y = 0;
			LightmapInfo[lmi].normal.z = 1;
		}


		ushort* data;

		if (Dedicated_server)
			data = ded_dummy_data;
		else
			data = (ushort*)lm_data(LightmapInfo[lmi].lm_handle);

		if (version <= 37)
		{
			for (t = 0; t < w * h; t++)
				data[t] = cf_ReadShort(fp);
		}
		else
		{
			ReadCompressionShort(fp, data, w * h);
		}

		// Adjust lightmap data to account for our new 1555 format
		if (version < 64)
		{
			for (t = 0; t < w * h; t++)
			{
				ushort pixel = data[t];

				if (pixel == 0x07e0)
					pixel = NEW_TRANSPARENT_COLOR;
				else
				{
					int r = ((pixel & 0xF800) >> 11) << 3;
					int g = ((pixel & 0x07e0) >> 5) << 2;
					int b = (pixel & 0x001f) << 3;

					pixel = OPAQUE_FLAG | GR_RGB16(r, g, b);
				}

				data[t] = pixel;

			}
		}
	}

	// Free dummy data for dedicated server
	if (Dedicated_server)
		mem_free(ded_dummy_data);
}

// Reads information about the terrain mesh
void ReadTerrainHeightChunk(CFILE* fp, int version)
{
	int i;

	if (version <= 31)
	{
		for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			Terrain_seg[i].ypos = cf_ReadByte(fp);
	}
	else
	{
		ubyte* byte_vals = (ubyte*)mem_malloc(TERRAIN_DEPTH * TERRAIN_WIDTH);

		ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);

		for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
		{
			Terrain_seg[i].ypos = byte_vals[i];
			AppendToLevelChecksum(Terrain_seg[i].ypos);
		}

		mem_free(byte_vals);
	}
}

void ReadGamePathsChunk(CFILE* fp, int version)
{
	// Currently, we will have no version specific info
	int i, j;

	// Get the number of paths
	Num_game_paths = cf_ReadShort(fp);

	for (i = 0; i < Num_game_paths; i++)
	{
		GamePaths[i].used = 1;
		GamePaths[i].name[0] = 0;
		GamePaths[i].pathnodes = (node*)mem_malloc(MAX_NODES_PER_PATH * sizeof(node));
		GamePaths[i].flags = 0;

		// Read in the path's info
		cf_ReadString(GamePaths[i].name, PAGENAME_LEN, fp);
		GamePaths[i].num_nodes = cf_ReadInt(fp);
		GamePaths[i].flags = cf_ReadByte(fp);

		for (j = 0; j < GamePaths[i].num_nodes; j++)
		{
			cf_ReadVector(fp, &GamePaths[i].pathnodes[j].pos);
			GamePaths[i].pathnodes[j].roomnum = cf_ReadInt(fp);
			GamePaths[i].pathnodes[j].flags = cf_ReadInt(fp);

			if (version >= 51)
			{
				cf_ReadVector(fp, &GamePaths[i].pathnodes[j].fvec);
				cf_ReadVector(fp, &GamePaths[i].pathnodes[j].uvec);
			}
			else
			{
				GamePaths[i].pathnodes[j].fvec.x = 0.0;
				GamePaths[i].pathnodes[j].fvec.y = 0.0;
				GamePaths[i].pathnodes[j].fvec.z = 1.0;

				GamePaths[i].pathnodes[j].uvec.x = 0.0;
				GamePaths[i].pathnodes[j].uvec.y = 1.0;
				GamePaths[i].pathnodes[j].uvec.z = 0.0;
			}
		}
	}
}

void ReadOverrideSoundChunk(CFILE* fp, int version)
{
	char soundname[PAGENAME_LEN];

	cf_ReadString(soundname, sizeof(soundname), fp);
	if (strcmp(soundname, "") != 0)
	{
		sound_override_force_field = FindSoundName(soundname);
	}

	cf_ReadString(soundname, sizeof(soundname), fp);
	if (strcmp(soundname, "") != 0)
	{
		sound_override_glass_breaking = FindSoundName(soundname);
	}
}

void ReadFFTMChunk(CFILE* fp, int version)
{
	char texturename[PAGENAME_LEN];
	int i;

	int num_items = MAX_FORCE_FIELD_BOUNCE_TEXTURES;

	if (version < 132)
	{
		num_items = 2;
	}

	for (i = 0; i < num_items; i++)
	{
		cf_ReadString(texturename, sizeof(texturename), fp);

		if (strcmp(texturename, "") != 0)
		{
			force_field_bounce_texture[i] = FindTextureName(texturename);
			force_field_bounce_multiplier[i] = cf_ReadFloat(fp);
			GameTextures[force_field_bounce_texture[i]].flags |= TF_FORCEFIELD;
		}
	}
}

void ReadBNodeChunk(CFILE* fp, int version)
{
	int hr_index = cf_ReadShort(fp);
	ASSERT(hr_index == Highest_room_index + 8);

	for (int i = 0; i <= hr_index; i++)
	{
		char f_good_room = cf_ReadByte(fp);
		if (f_good_room)
		{
			bn_list* bnlist = BNode_GetBNListPtr(i, true);

			bnlist->num_nodes = cf_ReadShort(fp);
			ASSERT(!(i <= Highest_room_index && (Rooms[i].flags & RF_EXTERNAL) && bnlist->num_nodes > 0));

			if (bnlist->num_nodes)
			{
				bnlist->nodes = (bn_node*)mem_malloc(sizeof(bn_node) * bnlist->num_nodes);
				for (int j = 0; j < bnlist->num_nodes; j++)
				{
					bnlist->nodes[j].pos.x = cf_ReadFloat(fp);
					bnlist->nodes[j].pos.y = cf_ReadFloat(fp);
					bnlist->nodes[j].pos.z = cf_ReadFloat(fp);

					bnlist->nodes[j].num_edges = cf_ReadShort(fp);
					if (bnlist->nodes[j].num_edges)
					{
						bnlist->nodes[j].edges = (bn_edge*)mem_malloc(sizeof(bn_edge) * bnlist->nodes[j].num_edges);
						for (int k = 0; k < bnlist->nodes[j].num_edges; k++)
						{
							bnlist->nodes[j].edges[k].end_room = cf_ReadShort(fp);
							bnlist->nodes[j].edges[k].end_index = cf_ReadByte(fp);
							if (version < 125)
								cf_ReadByte(fp);

							bnlist->nodes[j].edges[k].flags = cf_ReadShort(fp);
							bnlist->nodes[j].edges[k].cost = cf_ReadShort(fp);

							// Chrisnote -- fixes earlier versions of the bnode system that would allow for zero length edges
							if (bnlist->nodes[j].edges[k].cost < 1)
								bnlist->nodes[j].edges[k].cost = 1;

							bnlist->nodes[j].edges[k].max_rad = cf_ReadFloat(fp);
						}
					}
					else
					{
						bnlist->nodes[j].edges = NULL;
					}
				}
			}
			else
			{
				bnlist->nodes = NULL;
			}
		}
	}

	if (version <= 123)
		BNode_verified = false;
	else
		BNode_verified = cf_ReadByte(fp) != 0;

	BNode_allocated = true;
}

void ReadBOAChunk(CFILE* fp, int version)
{
	// Currently, we will have no version specific info

	int max_path_portals;

	// Get the number of paths
	BOA_AABB_checksum = BOA_mine_checksum = cf_ReadInt(fp);

	if (version >= 76)
		BOA_vis_checksum = cf_ReadInt(fp);
	else
		BOA_vis_checksum = 0;

	int max_rooms = cf_ReadInt(fp);

	if (version < 62)
	{
		cfseek(fp, sizeof(short) * max_rooms * max_rooms, SEEK_CUR);

		mprintf((0, "We will need to remake boa.  New cost structure added\n"));
		BOA_AABB_checksum = BOA_mine_checksum = 0;
	}
	else
	{
		max_path_portals = cf_ReadInt(fp);

		ASSERT(max_rooms - 1 <= MAX_ROOMS + 8);
		if (max_rooms - 1 > MAX_ROOMS + 8)
			Error("ReadBOAChunk: max_rooms exceeds MAX_ROOMS!");
		if (max_path_portals > MAX_PATH_PORTALS)
			Error("ReadBOAChunk: max_path_portals exceeds MAX_PATH_PORTALS!");

		if (version < 110 || (max_path_portals != MAX_PATH_PORTALS))
		{
			cfseek(fp, sizeof(short) * max_rooms * max_rooms + max_rooms * max_path_portals * sizeof(float), SEEK_CUR);

			if (version >= 107)
			{
				// Read BOA terrain info (temporary, just so vis data works with multiplay)
				cfseek(fp, max_rooms * sizeof(float), SEEK_CUR);
			}

			mprintf((0, "We will need to remake boa.  Data size changed\n"));
			BOA_AABB_checksum = BOA_mine_checksum = 0;
		}
		else
		{
			for (int i = 0; i <= max_rooms; i++)
			{
				for (int j = 0; j <= max_rooms; j++)
				{
					BOA_Array[i][j] = cf_ReadShort(fp);
				}
			}

			for (int i = 0; i <= max_rooms; i++)
			{
				for (int j = 0; j < max_path_portals; j++)
				{
					BOA_cost_array[i][j] = cf_ReadFloat(fp);
				}
			}

			BOA_num_mines = cf_ReadInt(fp);
			BOA_num_terrain_regions = cf_ReadInt(fp);

			if (version < 112)
			{
				mprintf((0, "We will need to remake boa.\n"));
				BOA_AABB_checksum = BOA_mine_checksum = 0;
			}
			else
			{
				for (int i = 0; i < BOA_num_terrain_regions; i++)
				{
					BOA_num_connect[i] = cf_ReadInt(fp);

					for (int j = 0; j < BOA_num_connect[i]; j++)
					{
						BOA_connect[i][j].roomnum = cf_ReadInt(fp);
						BOA_connect[i][j].portal = cf_ReadInt(fp);
					}
				}
			}
		}
	}
}

void ReadRoomAABBChunk(CFILE* fp, int version)
{
	int save_highest_room_index = cf_ReadInt(fp);
	ASSERT(save_highest_room_index < MAX_ROOMS); // Someone decreased the max amount of rooms
	ASSERT(save_highest_room_index == Highest_room_index); // Someone decreased the max amount of rooms
	if (save_highest_room_index >= MAX_ROOMS)
		Error("ReadRoomAABBChunk: highest room index out of bounds!");

	for (int i = 0; i <= save_highest_room_index; i++)
	{
		BOA_AABB_ROOM_checksum[i] = cf_ReadInt(fp);
	}

	for (int i = 0; i <= Highest_room_index; i++)
	{
		int used = cf_ReadInt(fp);
		ASSERT(Rooms[i].used == used);

		if (used)
		{
			int n_faces = cf_ReadInt(fp);
			ASSERT(Rooms[i].num_faces == n_faces);

			for (int j = 0; j < Rooms[i].num_faces; j++)
			{
				Rooms[i].faces[j].min_xyz.x = cf_ReadFloat(fp);
				Rooms[i].faces[j].min_xyz.y = cf_ReadFloat(fp);
				Rooms[i].faces[j].min_xyz.z = cf_ReadFloat(fp);

				Rooms[i].faces[j].max_xyz.x = cf_ReadFloat(fp);
				Rooms[i].faces[j].max_xyz.y = cf_ReadFloat(fp);
				Rooms[i].faces[j].max_xyz.z = cf_ReadFloat(fp);
			}

			BOA_AABB_ROOM_checksum[i] = cf_ReadInt(fp);

			Rooms[i].bbf_min_xyz.x = cf_ReadFloat(fp);
			Rooms[i].bbf_min_xyz.y = cf_ReadFloat(fp);
			Rooms[i].bbf_min_xyz.z = cf_ReadFloat(fp);

			Rooms[i].bbf_max_xyz.x = cf_ReadFloat(fp);
			Rooms[i].bbf_max_xyz.y = cf_ReadFloat(fp);
			Rooms[i].bbf_max_xyz.z = cf_ReadFloat(fp);

			Rooms[i].num_bbf_regions = cf_ReadShort(fp);
			Rooms[i].num_bbf = (short*)mem_malloc(sizeof(short) * Rooms[i].num_bbf_regions);
			Rooms[i].bbf_list = (short**)mem_malloc(sizeof(short*) * Rooms[i].num_bbf_regions);
			Rooms[i].bbf_list_min_xyz = (vector*)mem_malloc(sizeof(vector) * Rooms[i].num_bbf_regions);
			Rooms[i].bbf_list_max_xyz = (vector*)mem_malloc(sizeof(vector) * Rooms[i].num_bbf_regions);
			Rooms[i].bbf_list_sector = (unsigned char*)mem_malloc(sizeof(char) * Rooms[i].num_bbf_regions);

			for (int j = 0; j < Rooms[i].num_bbf_regions; j++)
			{
				Rooms[i].num_bbf[j] = cf_ReadShort(fp);
				Rooms[i].bbf_list[j] = (short*)mem_malloc(sizeof(short) * Rooms[i].num_bbf[j]);
			}

			for (int j = 0; j < Rooms[i].num_bbf_regions; j++)
			{
				for (int k = 0; k < Rooms[i].num_bbf[j]; k++)
				{
					Rooms[i].bbf_list[j][k] = cf_ReadShort(fp);
				}

				Rooms[i].bbf_list_min_xyz[j].x = cf_ReadFloat(fp);
				Rooms[i].bbf_list_min_xyz[j].y = cf_ReadFloat(fp);
				Rooms[i].bbf_list_min_xyz[j].z = cf_ReadFloat(fp);

				Rooms[i].bbf_list_max_xyz[j].x = cf_ReadFloat(fp);
				Rooms[i].bbf_list_max_xyz[j].y = cf_ReadFloat(fp);
				Rooms[i].bbf_list_max_xyz[j].z = cf_ReadFloat(fp);

				Rooms[i].bbf_list_sector[j] = cf_ReadByte(fp);
			}
		}
		else
		{
			BOA_AABB_ROOM_checksum[i] = 0;  // Not used
		}
	}
}

void ReadMatcenChunk(CFILE* fp, int version)
{
	Num_matcens = cf_ReadInt(fp);
	if (Num_matcens > MAX_MATCENS)
		Error("ReadMatcenChunk: Too many matcens!");
	for (int i = 0; i < Num_matcens; i++)
	{
		Matcen[i] = new matcen;
		Matcen[i]->LoadData(fp);
	}
}

void ReadALifeChunk(CFILE* fp, int version)
{
#ifndef NEWEDITOR
	a_life.LoadData(fp);
#endif
}

void ReadLevelGoalsChunk(CFILE* fp, int version)
{
	Level_goals.LoadLevelGoalInfo(fp);
}

// Reads relevant info about the terrain sky and lights
void ReadTerrainSkyAndLightChunk(CFILE* fp, int version)
{
	if (version >= 41)
	{
		if (version >= 88)
			Terrain_sky.fog_scalar = cf_ReadFloat(fp);

		if (version >= 87)
			Terrain_sky.damage_per_second = cf_ReadFloat(fp);

		Terrain_sky.textured = cf_ReadByte(fp);

		if (version < 43)
		{
			for (int i = 0; i < MAX_HORIZON_PIECES; i++)
				cf_ReadShort(fp);
		}
		else if (version < 74)
		{
			for (int i = 0; i < MAX_HORIZON_PIECES; i++)
			{
				cf_ReadShort(fp);
				cf_ReadShort(fp);
			}

		}
		else
		{
			if (version < 104)
			{
				for (int i = 0; i < 4; i++)
					cf_ReadShort(fp);
			}
			else 
			{
				Terrain_sky.dome_texture = texture_xlate[cf_ReadShort(fp)];

				//Check for failed xlate
				if (Terrain_sky.dome_texture == -1) 
				{
					Terrain_sky.dome_texture = 0;
				}
			}

			if (version < 114)
				for (int i = 0; i < MAX_HORIZON_PIECES; i++)
					cf_ReadShort(fp);	// Was band textures
		}
	}

	Terrain_sky.sky_color = cf_ReadInt(fp);
	Terrain_sky.horizon_color = cf_ReadInt(fp);
	Terrain_sky.fog_color = cf_ReadInt(fp);

	Terrain_sky.flags = cf_ReadInt(fp);

#ifdef __LINUX__
	if (FindArg("-noterrainfog") > 0)
	{
		Terrain_sky.flags &= ~TF_FOG;
	}
#endif

	if (version >= 69)
	{
		Terrain_sky.radius = cf_ReadFloat(fp);
		if (version < 74)
			Terrain_sky.radius = 2500.0;

		SetupSky(Terrain_sky.radius, Terrain_sky.flags, 1);
	}

	if (version >= 104)
		Terrain_sky.rotate_rate = cf_ReadFloat(fp);
	else
		Terrain_sky.rotate_rate = 0;


	int num_sats = cf_ReadInt(fp);

	ASSERT(num_sats <= MAX_SATELLITES);
	if (num_sats > MAX_SATELLITES)
		Error("ReadTerrainSkyAndLightChunk: Too many satellites in the sky!");

	Terrain_sky.num_satellites = num_sats;

	for (int i = 0; i < num_sats; i++)
	{
		Terrain_sky.satellite_texture[i] = texture_xlate[cf_ReadShort(fp)];

		//Check for failed xlate
		if (Terrain_sky.satellite_texture[i] == -1) {
			Terrain_sky.satellite_texture[i] = 0;
		}

		cf_ReadVector(fp, &Terrain_sky.satellite_vectors[i]);
		if (version >= 44)
			Terrain_sky.satellite_flags[i] = cf_ReadByte(fp);
		else
			Terrain_sky.satellite_flags[i] = 0;

		if (version >= 116)
			Terrain_sky.satellite_size[i] = cf_ReadFloat(fp);
		else
		{
			vector subvec = Terrain_sky.satellite_vectors[i];
			vm_NormalizeVector(&subvec);

			Terrain_sky.satellite_vectors[i] = subvec * (Terrain_sky.radius * 3);

			Terrain_sky.satellite_size[i] = 500;
		}

		if (version >= 75)
		{
			Terrain_sky.satellite_r[i] = cf_ReadFloat(fp);
			Terrain_sky.satellite_g[i] = cf_ReadFloat(fp);
			Terrain_sky.satellite_b[i] = cf_ReadFloat(fp);
		}
	}

	// Now read misc stuff	
	if (version <= 56)
	{
		cf_ReadFloat(fp);
		cf_ReadFloat(fp);
	}

	// Read lighting
	if (version <= 31)
	{
		for (int i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
		{
			if (version >= 24)
			{
				Terrain_seg[i].l = cf_ReadByte(fp);
				Terrain_seg[i].r = cf_ReadByte(fp);
				Terrain_seg[i].g = cf_ReadByte(fp);
				Terrain_seg[i].b = cf_ReadByte(fp);
			}
			else
			{
				Terrain_seg[i].l = cf_ReadByte(fp);
				Terrain_seg[i].r = Terrain_seg[i].l;
				Terrain_seg[i].g = Terrain_seg[i].l;
				Terrain_seg[i].b = Terrain_seg[i].l;
			}
		}
		if (version >= 25)
		{
			for (int i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
				Terrain_dynamic_table[i] = cf_ReadByte(fp);
		}
	}
	else
	{
		ubyte* byte_vals = (ubyte*)mem_malloc(TERRAIN_DEPTH * TERRAIN_WIDTH);

		ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
		for (int i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			Terrain_seg[i].l = byte_vals[i];

		ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
		for (int i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			Terrain_seg[i].r = byte_vals[i];

		ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
		for (int i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			Terrain_seg[i].g = byte_vals[i];

		ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
		for (int i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			Terrain_seg[i].b = byte_vals[i];

		ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
		for (int i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			Terrain_dynamic_table[i] = byte_vals[i];

		if (version >= 72)
		{
			Terrain_occlusion_checksum = cf_ReadInt(fp);
			ReadCompressionByte(fp, byte_vals, OCCLUSION_SIZE * OCCLUSION_SIZE * 32);
			for (int i = 0; i < OCCLUSION_SIZE * OCCLUSION_SIZE * 32; i++)
			{
				int row = i / 32;
				int column = i % 32;

				Terrain_occlusion_map[row][column] = byte_vals[i];
			}
		}

		mem_free(byte_vals);
	}
}

// Reads the tmaps of the terrain, plus flags
void ReadTerrainTmapFlagChunk(CFILE* fp, int version)
{
	int i, t;

	if (version <= 31)
	{
		for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			cf_ReadShort(fp);
		for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			cf_ReadShort(fp);

		if (version >= 16)
		{
			for (i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
				Terrain_seg[i].flags = cf_ReadByte(fp);

			for (i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
				cf_ReadByte(fp);
			for (i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
				cf_ReadByte(fp);
		}
		else
		{
			for (i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
			{
				for (t = 0; t < 4; t++)
				{
					cf_ReadByte(fp);
					cf_ReadByte(fp);
				}
			}
		}
	}
	else
	{
		ubyte* byte_vals = (ubyte*)mem_malloc(TERRAIN_DEPTH * TERRAIN_WIDTH);
		ushort* short_vals = (ushort*)mem_malloc(2 * TERRAIN_DEPTH * TERRAIN_WIDTH);

		if (version < 102)
		{
			// Read tmap1
			ReadCompressionShort(fp, short_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
			// Read tmap2
			ReadCompressionShort(fp, short_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
		}
		else
		{
			ReadCompressionShort(fp, short_vals, TERRAIN_TEX_DEPTH * TERRAIN_TEX_WIDTH);
			for (i = 0; i < TERRAIN_TEX_DEPTH * TERRAIN_TEX_WIDTH; i++) {
				Terrain_tex_seg[i].tex_index = texture_xlate[short_vals[i]];

				//Check for failed xlate
				if (Terrain_tex_seg[i].tex_index == -1) {
					Terrain_tex_seg[i].tex_index = 0;
				}
			}

			ReadCompressionByte(fp, byte_vals, TERRAIN_TEX_DEPTH * TERRAIN_TEX_WIDTH);
			for (i = 0; i < TERRAIN_TEX_DEPTH * TERRAIN_TEX_WIDTH; i++)
			{
				Terrain_tex_seg[i].rotation = byte_vals[i];
				if ((Terrain_tex_seg[i].rotation >> 4) == 0)
					Terrain_tex_seg[i].rotation |= (1 << 4);
			}
		}

		// Read flags
		ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
		for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
			Terrain_seg[i].flags = byte_vals[i];

		if (version < 102)
		{
			// Read rotation_factor
			ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);

			// Read tile_factor
			ReadCompressionByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);
		}

		mem_free(byte_vals);
		mem_free(short_vals);
	}
}

// MUST BE SAVED BEFORE OBJECTS because of ResetTerrain

void ReadTerrainChunks(CFILE* fp, int version)
{
	char chunk_name[4];
	int  terr_chunk_start, terr_chunk_size;
	int done = 0;

	ResetTerrain();

	while (!done)
	{
		cf_ReadBytes((ubyte*)chunk_name, 4, fp);
		terr_chunk_start = cftell(fp);
		terr_chunk_size = cf_ReadInt(fp);

		LoadLevelProgress(LOAD_PROGRESS_LOADING_LEVEL, (filelen) ? (float)(terr_chunk_start + terr_chunk_size) / (float)filelen : 0.0f, NULL);

		if (ISCHUNK(CHUNK_TERRAIN_HEIGHT))
		{
			ReadTerrainHeightChunk(fp, version);
		}
		else if (ISCHUNK(CHUNK_TERRAIN_TMAPS_FLAGS))
		{
			ReadTerrainTmapFlagChunk(fp, version);
		}
		else if (ISCHUNK(CHUNK_TERRAIN_SKY))
		{
			ReadTerrainSkyAndLightChunk(fp, version);
		}
		else if (ISCHUNK(CHUNK_TERRAIN_END))
		{
			done = 1;
			cfseek(fp, terr_chunk_start + terr_chunk_size, SEEK_SET);
			continue;
		}
		// else unknown chunk
		cfseek(fp, terr_chunk_start + terr_chunk_size, SEEK_SET);
	}


	// Generate needed info
	BuildMinMaxTerrain();
	BuildTerrainNormals();
	UpdateTerrainLightmaps();

#if (defined(EDITOR) || defined(NEWEDITOR))

	memset(TerrainSelected, 0, TERRAIN_WIDTH * TERRAIN_DEPTH);
	Num_terrain_selected = 0;
#endif

}

//Read the texture names & build the xlate table
void ReadTextureList(CFILE* ifile)
{
	BuildXlateTable(ifile, FindTextureName, texture_xlate, MAX_TEXTURES, -1);
}

void ReadPlayerStarts(CFILE* infile, int fileversion)
{
	int n = (fileversion >= 120) ? cf_ReadShort(infile) : 32;
	if (n > MAX_PLAYERS)
		Error("ReadPlayerStarts: Too many player starts!");

	for (int i = 0; i < n; i++)
	{
		Players[i].startpos_flags = cf_ReadInt(infile);
	}
}


void VerifyObjectList()
{
	ubyte already_listed[MAX_OBJECTS] = {};

	for (int i = 0; i <= Highest_room_index; i++)
	{
		room* rp = &Rooms[i];
		if (rp->flags & RF_EXTERNAL)
			continue;
		if (rp->used == 0)
			continue;
		for (int objnum = rp->objects; objnum != -1; objnum = Objects[objnum].next)
		{
			ASSERT(already_listed[objnum] == 0);
			already_listed[objnum] = 1;
		}
	}

	for (int i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
	{
		terrain_segment* tseg = &Terrain_seg[i];

		for (int objnum = tseg->objects; objnum != -1; objnum = Objects[objnum].next)
		{
			ASSERT(already_listed[objnum] == 0);
			already_listed[objnum] = 1;
		}
	}

}

//Data to deal with a bunch of renamed doors
char* Old_door_names[] = {
	"markroomdoor.OOF1",
	"cellblockdoor.OOF1",
	"towerringdoor.OOF1",
	"hangdoorinverse.oof1",
	"Big Lock Door",
	"Oval Door",
	"Layered Door",
	"steamdoor1",
	"heatsinkdoornew1",
	"Vault Door Large",
	"Ceddoor1",
	"trapdoor1",
	"Lukesecretdoor1",
	"ptmc11",
	"ptmc31",
	"ptmc021",
	"Bulkhead1",
	"Placeholderdoor1",
	"ptmcbd1",
	"energysecretdoor1",
	"PTMC Industrial 1",
	"PTMC Covert 1"
};

char* New_door_names[] = {
	"MARK'S OLD DOOR",
	"SEAN'S NOVAK DOOR 1",
	"SEAN'S NOVAK DOOR 2",
	"SEAN'S NOVAK DOOR 3",
	"PTMC Industrial 2",
	"SEAN'S DUCTWORK DOOR",
	"PTMC Industrial 4",
	"SEAN'S STEAMVENT DOOR",
	"SEAN'S HEATSINK DOOR",
	"DAN'S VAULT DOOR",
	"CED 1",
	"SEAN'S TRAPDOOR",
	"LUKE'S SECRET DOOR",
	"PTMC Industrial 5",
	"PTMC Industrial 6",
	"PTMC Industrial 7",
	"!!!CED BROKEN!!!",
	"PLACEHOLDER DOOR",
	"PTMC Industrial 3",
	"SEAN'S ENERGY SECRET DOOR",
	"PTMC Industrial 1",
	"PTMC Covert 1"
};

#define NUM_RENAMED_DOORS (sizeof(Old_door_names) / sizeof(*Old_door_names))

//Deals with some renamed doors.  Translates the old name to the new name, then looks up the id
int SpecialFindDoorName(char* name)
{
	//Look up the old name, and return it if found
	int id = FindDoorName(name);
	if (id != -1)
		return id;

	//Didn't find old name, so see if there's a new name, and look for that
	for (int i = 0; i < NUM_RENAMED_DOORS; i++) {
		if (stricmp(name, Old_door_names[i]) == 0)
			return FindDoorName(New_door_names[i]);
	}

	//Didn't find old name, didn't find xlation for the old name
	return -1;
}

extern bool Disable_editor_rendering;

#define LEVEL_LOADED_PCT_CALC	(filelen) ? (float)(chunk_size+chunk_start)/(float)filelen : 0.0f
//Load a level file
//Returns 1 if file read ok, else 0
int LoadLevel(char* filename, void (*cb_fn)(const char*, int, int))
{
	CFILE* ifile;
	char tag[4];
	int n, i, retval = 1;

	int version;
	bool f_read_AABB = false;
	bool no_128s = true;
	int total = 0;
#ifdef EDITOR
	Disable_editor_rendering = 1;
	Num_failed_xlate_items = 0;
#endif
	PrintDedicatedMessage(TXT_DS_OPENLEVEL, filename);

	RestartLevelMD5();

	ifile = cfopen(filename, "rb");
	if (!ifile)
	{
		retval = 0;
		goto end_loadlevel;
	}

	try 
	{
		//catch cfile errors
		filelen = cfilelength(ifile);

		//Read & check tag
		cf_ReadBytes((ubyte*)tag, 4, ifile);
		if (strncmp(tag, LEVEL_FILE_TAG, 4)) 
		{
			cfclose(ifile);
			retval = 0;
			goto end_loadlevel;
		}

		//Read & check version number
		version = cf_ReadInt(ifile);

		//Check for too-new version
		if (version > LEVEL_FILE_VERSION) 
		{
			cfclose(ifile);
#if (defined(EDITOR) || defined(NEWEDITOR))
			if (GetFunctionMode() == EDITOR_MODE)
				EditorMessageBox("Can't load level file \"%s\": Its version (%d) is newer than current version (%d).", filename, version, LEVEL_FILE_VERSION);
#endif
			retval = 0;
			goto end_loadlevel;
		}

		//Check for too-old version
		if (version < LEVEL_FILE_OLDEST_COMPATIBLE_VERSION) 
		{
			cfclose(ifile);
#if (defined(EDITOR) || defined(NEWEDITOR))
			if (GetFunctionMode() == EDITOR_MODE)
				EditorMessageBox("Can't load level file \"%s\": Its version (%d) is older than the oldest compatible version (%d).", filename, version, LEVEL_FILE_OLDEST_COMPATIBLE_VERSION);
#endif
			retval = 0;
			goto end_loadlevel;
		}

		//Get rid of old mine
		FreeAllRooms();

		// Get rid of old matcens
		DestroyAllMatcens();

#ifndef NEWEDITOR
		// Resets the ambient life controller
		a_life.ALReset();
#endif

		// Get rid of old level goals
		Level_goals.CleanupAfterLevel();

		// Get rid of old objects
		FreeAllObjects();

#ifndef NEWEDITOR
		// Clear glows
		ResetLightGlows();
#endif

		//Reset all the objects
		ResetObjectList();

		//Free all triggers
		FreeTriggers();

		// Resets the game paths to blank
		InitGamePaths();

		//Clear terrain sounds
		ClearTerrainSound();

		//Init level info
		strcpy(Level_info.name, "Unnamed");
		strcpy(Level_info.designer, "Anonymous");
		strcpy(Level_info.copyright, "");
		strcpy(Level_info.notes, "");

		BOA_AABB_checksum = BOA_mine_checksum = 0;
		for (i = 0; i < MAX_ROOMS; i++)
		{
			BOA_AABB_ROOM_checksum[i] = 0;
		}

		BNode_ClearBNodeInfo();
		FVI_always_check_ceiling = false;

		sound_override_force_field = -1;
		sound_override_glass_breaking = -1;

		for (i = 0; i < MAX_FORCE_FIELD_BOUNCE_TEXTURES; i++)
		{
			force_field_bounce_texture[i] = -1;
			force_field_bounce_multiplier[i] = 1.0f;
		}

		Level_powerups_ignore_wind = false;

		LoadLevelProgress(LOAD_PROGRESS_LOADING_LEVEL, 0.01f);

		//Read and parse chunks
		while (!cfeof(ifile)) 
		{
			char chunk_name[4];

			cf_ReadBytes((ubyte*)chunk_name, 4, ifile);
			chunk_start = cftell(ifile);
			chunk_size = cf_ReadInt(ifile);
			mprintf((0, "Chunk: %c%c%c%c, size=%d\n", chunk_name[0], chunk_name[1], chunk_name[2], chunk_name[3], chunk_size));

			if (ISCHUNK(CHUNK_TEXTURE_NAMES))
				ReadTextureList(ifile);
			else if (ISCHUNK(CHUNK_GENERIC_NAMES)) 
			{
				BuildXlateTable(ifile, FindObjectIDName, generic_xlate, MAX_OBJECT_IDS, XT_GENERIC);
			}
			else if (ISCHUNK(CHUNK_DOOR_NAMES)) 
			{
				if (version < 82)
					BuildXlateTable(ifile, SpecialFindDoorName, door_xlate, MAX_DOORS, XT_DOOR);
				else
					BuildXlateTable(ifile, FindDoorName, door_xlate, MAX_DOORS, XT_DOOR);
			}
			else if (ISCHUNK(CHUNK_ROOMS)) 
			{
				int num_rooms = cf_ReadInt(ifile);

				extern void RoomMemInit(int nverts, int nfaces, int nfaceverts, int nportals);	//MOVE TO HEADER FILE
				if (version >= 85) 
				{
					int nverts = cf_ReadInt(ifile);
					int nfaces = cf_ReadInt(ifile);
					int nfaceverts = cf_ReadInt(ifile);
					int nportals = cf_ReadInt(ifile);
					RoomMemInit(nverts, nfaces, nfaceverts, nportals);
				}
				else
					RoomMemInit(0, 0, 0, 0);	//0 means we don't know how much memory we need

				n_degenerate_faces_removed = 0;
				int roomnum;
				for (i = 0; i < num_rooms; i++) 
				{
					if ((i % 10) == 0)
					{
						LoadLevelProgress(LOAD_PROGRESS_LOADING_LEVEL, (filelen) ? (float)(chunk_start + (((chunk_size) / num_rooms) * i)) / (float)filelen : 0.0f, NULL);
					}

					roomnum = (version >= 96) ? cf_ReadShort(ifile) : i;
					ReadRoom(ifile, &Rooms[roomnum], version);
				}
				mprintf((1, "%d degenerate faces removed\n", n_degenerate_faces_removed));

				Highest_room_index = roomnum;
				ASSERT(Highest_room_index < MAX_ROOMS);

			}
			else if (ISCHUNK(CHUNK_ROOM_WIND)) 
			{
				int num_rooms = cf_ReadInt(ifile);

				if (num_rooms > MAX_ROOMS)
					Error("LoadLevel: Too many room wind vectors!");

				for (i = 0; i < num_rooms; i++) 
				{
					int roomnum = cf_ReadShort(ifile);
					cf_ReadVector(ifile, &Rooms[roomnum].wind);
				}
			}
			else if (ISCHUNK(CHUNK_PLAYER_STARTS))
			{
				ReadPlayerStarts(ifile, version);
			}
			else if (ISCHUNK(CHUNK_OBJECT_HANDLES)) 
			{
				//Read in any non-zero handles for deleted objects
				ubyte already_loaded[MAX_OBJECTS];
				memset(already_loaded, 0, MAX_OBJECTS);

				//Get the number of handles in the file
				n = cf_ReadInt(ifile);

				for (i = 0; i < n; i++)
				{
					int handle = cf_ReadInt(ifile);

					if (version < 94)
						handle = XLATE_HANDLE(handle);

					int objnum = handle & HANDLE_OBJNUM_MASK;
					if (objnum >= MAX_OBJECTS)
						Error("LoadLevel: Associating handle with out-of-bounds object!");

					ASSERT(already_loaded[objnum] == 0);

					already_loaded[objnum] = 1;
					Objects[objnum].handle = handle;
				}
			}
			else if (ISCHUNK(CHUNK_OBJECTS)) 
			{
				n = cf_ReadInt(ifile);
				for (i = 0; i < n; i++) 
				{
					int handle, objnum, roomnum;

					if (version >= 45)
					{
						handle = cf_ReadInt(ifile);
						if (version < 94)
							handle = XLATE_HANDLE(handle);
						objnum = handle & HANDLE_OBJNUM_MASK;
					}
					else 
					{
						objnum = i;
						handle = i + HANDLE_COUNT_INCREMENT;
					}

					if (objnum >= MAX_OBJECTS)
						Error("LoadLevel: Reading out-of-bounds object!");
					ReadObject(ifile, &Objects[objnum], handle, version);

					//Link the object into the mine
					roomnum = Objects[objnum].roomnum;

					Objects[objnum].roomnum = -1;			 //ObjLink() expects the roomnum to be -1

					if ((roomnum > Highest_room_index) && !ROOMNUM_OUTSIDE(roomnum)) 
					{
						//bad roomnum
						Int3();										//loading object with invalid room number
						Objects[objnum].type = OBJ_NONE;		//kill the object
					}
					else 
					{
						if (!ROOMNUM_OUTSIDE(roomnum) && Rooms[roomnum].flags & RF_EXTERNAL) 
						{
							mprintf((0, "Internal object %d linked to external room %d (type = %d)!!!\n", objnum, roomnum, Objects[objnum].type));
							if (Objects[objnum].type == OBJ_VIEWER)
								Objects[objnum].type = OBJ_NONE;		//kill the object
							else
							{
								Int3();
								int done = 0;
								for (int k = 0; k <= Highest_room_index && !done; k++)
								{
									if (Rooms[k].used && !(Rooms[k].flags & RF_EXTERNAL))
									{
										done = 1;
										ObjLink(objnum, k);
									}
								}

							}
						}
						else
							ObjLink(objnum, roomnum);
					}

#ifndef NEWEDITOR
					ObjInitPositionHistory(&Objects[objnum]);
#endif
				}

				//Rebuild the free object list
				ResetFreeObjects();
			}
			else if (ISCHUNK(CHUNK_TRIGGERS)) 
			{
				Num_triggers = cf_ReadInt(ifile);
				for (i = 0; i < Num_triggers; i++) 
				{
					ReadTrigger(ifile, &Triggers[i], version);
				}
			}
			else if (ISCHUNK(CHUNK_GAME_PATHS))
			{
				ReadGamePathsChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_BOA))
			{
				ReadBOAChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_BNODES))
			{
				ReadBNodeChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_OVERRIDE_SOUNDS))
			{
				ReadOverrideSoundChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_FFT_MOD))
			{
				ReadFFTMChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_ROOM_AABB))
			{
				ReadRoomAABBChunk(ifile, version);
				f_read_AABB = true;
			}
			else if (ISCHUNK(CHUNK_MATCEN_DATA))
			{
				ReadMatcenChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_LEVEL_GOALS))
			{
				ReadLevelGoalsChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_LIGHTMAPS))
			{
				ReadLightmapChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_NEW_LIGHTMAPS))
			{
				ReadNewLightmapChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_ALIFE_DATA))
			{
				ReadALifeChunk(ifile, version);
			}
			else if (ISCHUNK(CHUNK_NEW_BSP))
			{
				InitDefaultBSP();

				BSPChecksum = cf_ReadInt(ifile);
				LoadBSPNode(ifile, &MineBSP.root);
			}
			else if (ISCHUNK(CHUNK_TERRAIN_SOUND)) 
			{
				int n_bands = cf_ReadInt(ifile);
				if (n_bands > NUM_TERRAIN_SOUND_BANDS)
					Error("LoadLevel: Num terrain sound bands exceeds NUM_TERRAIN_SOUND_BANDS!");

				for (int b = 0; b < n_bands; b++) 
				{
					if (version < 119)
						Terrain_sound_bands[b].sound_index = cf_ReadInt(ifile);
					else 
					{
						char soundname[PAGENAME_LEN];
						cf_ReadString(soundname, sizeof(soundname), ifile);
						Terrain_sound_bands[b].sound_index = FindSoundName(IGNORE_TABLE(soundname));
					}
					Terrain_sound_bands[b].low_alt = cf_ReadByte(ifile);
					Terrain_sound_bands[b].high_alt = cf_ReadByte(ifile);
					Terrain_sound_bands[b].low_volume = cf_ReadFloat(ifile);
					Terrain_sound_bands[b].high_volume = cf_ReadFloat(ifile);
				}
			}
#if (defined(EDITOR) || defined(NEWEDITOR))
			else if (ISCHUNK(CHUNK_SCRIPT)) 
			{
#if (defined(EDITOR) || defined(NEWEDITOR))
				if (FindArg("-savescript")) 
				{
					char path[MAX_PATH], name[MAX_PATH];
					ddio_SplitPath(filename, path, name, NULL);
					strcat(path, name);
					strcat(path, ".scr");
					CFILE* ofile = cfopen(path, "wb");
					ubyte buf[1000];
					int n = chunk_size, r;
					while (n) 
					{
						r = cf_ReadBytes(buf, min(n, sizeof(buf)), ifile);
						cf_WriteBytes(buf, r, ofile);
						n -= r;
					}
					cfclose(ofile);
				}
#endif	//ifdef EDITOR
			}
#endif
			else if (ISCHUNK(CHUNK_SCRIPT_CODE)) 
			{
				//	Level_script = D3XLoadProgram(ifile); -- ripped out
			}
			else if (ISCHUNK(CHUNK_TERRAIN))
			{
				ReadTerrainChunks(ifile, version);
			}
			else if (ISCHUNK(CHUNK_LEVEL_INFO)) 
			{
				cf_ReadString(Level_info.name, sizeof(Level_info.name), ifile);

				//Localize level name here...
				strcpy(Level_info.name, LocalizeLevelName(Level_info.name));

				cf_ReadString(Level_info.designer, sizeof(Level_info.designer), ifile);
				cf_ReadString(Level_info.copyright, sizeof(Level_info.copyright), ifile);
				cf_ReadString(Level_info.notes, sizeof(Level_info.notes), ifile);

				if (version >= 83)
				{
					Gravity_strength = cf_ReadFloat(ifile);
				}
				else
				{
					Gravity_strength = -32.2f;
				}

				if (version >= 131)
				{
					Level_powerups_ignore_wind = (cf_ReadInt(ifile) != 0);
				}
				else
				{
					Level_powerups_ignore_wind = false;
				}

				if (version >= 122)
				{
					FVI_always_check_ceiling = (cf_ReadInt(ifile) != 0);
				}
				else
				{
					FVI_always_check_ceiling = false;
				}

				if (version >= 127) 
				{
					Ceiling_height = cf_ReadFloat(ifile);
					AppendToLevelChecksum(Ceiling_height);
				}
				else
					Ceiling_height = MAX_TERRAIN_HEIGHT;
			}
#if (defined(EDITOR) || defined(NEWEDITOR))
			else if (ISCHUNK(CHUNK_EDITOR_INFO)) 
			{
				//Read misc editor vars
				int t = cf_ReadShort(ifile);
				Curroomp = &Rooms[t];
				Curface = cf_ReadShort(ifile);
				if (version >= 81) 
				{
					Curedge = cf_ReadShort(ifile);
					Curvert = cf_ReadShort(ifile);
				}
				t = cf_ReadShort(ifile);
				Markedroomp = (t == -1) ? NULL : &Rooms[t];
				Markedface = cf_ReadShort(ifile);
				if (version >= 81) 
				{
					Markededge = cf_ReadShort(ifile);
					Markedvert = cf_ReadShort(ifile);
				}

				//Read selected list
				N_selected_rooms = cf_ReadInt(ifile);
				for (i = 0; i < N_selected_rooms; i++)
					Selected_rooms[i] = cf_ReadShort(ifile);		//saved as shorts to save disk space

				//Read other misc stuff
				if (version >= 14) 
				{
					Cur_object_index = cf_ReadInt(ifile);
					Current_trigger = cf_ReadInt(ifile);
					if (version < 106)
						cf_ReadInt(ifile);	//was Current_doorway
					Editor_view_mode = cf_ReadInt(ifile);
					Editor_viewer_id = cf_ReadInt(ifile);
					if (version < 47)
						cf_ReadInt(ifile);		//was Editor_viewer_id[VM_TERRAIN]
				}

				if (version >= 55) 
				{
					cf_ReadVector(ifile, &Wireframe_view_mine.target);
					cf_ReadMatrix(ifile, &Wireframe_view_mine.orient);
					Wireframe_view_mine.dist = cf_ReadFloat(ifile);
				}

				if (version >= 113)
				{
					for (i = 0; i < MAX_ROOMS; i++)
					{
						Room_multiplier[i] = cf_ReadFloat(ifile);
						if (version >= 118)
						{
							Room_ambience_r[i] = cf_ReadFloat(ifile);
							Room_ambience_g[i] = cf_ReadFloat(ifile);
							Room_ambience_b[i] = cf_ReadFloat(ifile);
						}
					}
				}
				if (version >= 126)
					LightSpacing = cf_ReadInt(ifile);

				if (version >= 128)
				{
					GlobalMultiplier = cf_ReadFloat(ifile);
					Ambient_red = cf_ReadFloat(ifile);
					Ambient_green = cf_ReadFloat(ifile);
					Ambient_blue = cf_ReadFloat(ifile);
					rad_MaxStep = cf_ReadInt(ifile);
				}

			}
#endif //ifdef EDITOR
			else 
			{
				//unknown chunk
				mprintf((0, "  Unknown chunk: %c%c%c%c, size=%d\n", chunk_name[0], chunk_name[1], chunk_name[2], chunk_name[3], chunk_size));
			}

			//Go to end of chunk
			cfseek(ifile, chunk_start + chunk_size, SEEK_SET);

			LoadLevelProgress(LOAD_PROGRESS_LOADING_LEVEL, LEVEL_LOADED_PCT_CALC, chunk_name);
		}

	}	//try
	catch (cfile_error* cfe) 
	{
		mprintf((0, "Error reading: file = <%s>, error = \"%s\"\n", cfe->file->name, cfe->msg));
		ASSERT(cfe->read_write == CFE_READING);
#if (defined(EDITOR) || defined(NEWEDITOR))
		if (GetFunctionMode() == EDITOR_MODE)
			EditorMessageBox("Error reading file \"%s\": %s", cfe->file->name, cfe->msg);
#endif
		cfclose(ifile);
		return 0;
	}

	//Close the file
	cfclose(ifile);

	//Look for player objects & set player starts
	FindPlayerStarts();
	if (!Player_object && !(Game_mode & GM_MULTI))
		Player_object = &Objects[Players[0].objnum];

	// aabbs
	LoadLevelProgress(LOAD_PROGRESS_LOADING_LEVEL, 1.0f, NULL);

	//Compute the bounding boxes
	if (!f_read_AABB)
		ComputeAABB(true);
	else
		ComputeAABB(false);

	//Create the room objects
	CreateRoomObjects();

#ifndef NEWEDITOR /* we call MakeBoa AFTER textures are marked in use */
	MakeBOA();
#endif

	// Decrement lightmap counters - this must be done because multiple faces can 
	// share 1 lightmap

	if (version >= 34 && !Dedicated_server)
	{
		ubyte* lightmap_spoken_for = (ubyte*)mem_malloc(MAX_LIGHTMAPS);
		ubyte* free_lightmap_info = (ubyte*)mem_malloc(MAX_LIGHTMAP_INFOS);
		ASSERT(lightmap_spoken_for);
		memset(lightmap_spoken_for, 0, MAX_LIGHTMAPS);

		ASSERT(free_lightmap_info);
		memset(free_lightmap_info, 0, MAX_LIGHTMAP_INFOS);

		for (i = 0; i < Num_lightmap_infos_read; i++)
		{
			int index = LightmapInfoRemap[i];

			if (LightmapInfo[index].used == 1)
			{
				free_lightmap_info[index] = 1;

				GameLightmaps[LightmapInfo[index].lm_handle].used++;
				lightmap_spoken_for[LightmapInfo[index].lm_handle] = 1;
			}
			else if (LightmapInfo[index].used > 1)
			{
				LightmapInfo[index].used--;

				// Increase to account for shared
				GameLightmaps[LightmapInfo[index].lm_handle].used++;
				lightmap_spoken_for[LightmapInfo[index].lm_handle] = 1;

				if (LightmapInfo[index].width == 128 || LightmapInfo[index].height == 128)
					no_128s = false;
			}
		}

		// Decrease to account for basic allocation
		for (i = 0; i < MAX_LIGHTMAPS; i++)
		{
			if (lightmap_spoken_for[i])
				GameLightmaps[i].used--;
		}
		for (i = 0; i < MAX_LIGHTMAP_INFOS; i++)
		{
			if (free_lightmap_info[i])
				FreeLightmapInfo(i);
		}


		mem_free(lightmap_spoken_for);
		mem_free(free_lightmap_info);
	}

	VerifyObjectList();
#ifdef EDITOR
	extern void SqueezeLightmaps(int, int);
	if (no_128s && version <= 89)
	{
		SqueezeLightmaps(0, -1);
		SqueezeLightmaps(1, -1);
	}
#endif

	ConsolidateMineMirrors();

	// paging data.
	LoadLevelProgress(LOAD_PROGRESS_PAGING_DATA, 0.0f, NULL);

#ifndef NEWEDITOR
	CountDataToPageIn();
#endif
	//mprintf((0,"%d bytes of data to page in...\n",total));

end_loadlevel:
#ifdef EDITOR
	Disable_editor_rendering = 0;
#endif

	if (!BNode_allocated)
	{
		for (int i = 0; i <= Highest_room_index; i++)
		{
			if (Rooms[i].used)
			{
				Rooms[i].bn_info.num_nodes = 0;
				Rooms[i].bn_info.nodes = NULL;
			}
		}
	}

	// Debug log the current sum 
	mprintf((0, "End of load level checksum = %s\n", GetCurrentSumString()));
	//Done
	return retval;
}


#if ( defined(EDITOR) || defined(NEWEDITOR) )

#define cf_WriteVector(f,v)  do {cf_WriteFloat((f),(v)->x); cf_WriteFloat((f),(v)->y); cf_WriteFloat((f),(v)->z); } while (0)
#define cf_WriteMatrix(f,m)  do {cf_WriteVector((f),&(m)->rvec); cf_WriteVector((f),&(m)->uvec); cf_WriteVector((f),&(m)->fvec); } while (0)

//writes an object
//returns 1 if written ok
int WriteObject(CFILE* ofile, object* objp)
{
	int i, t, k;

	cf_WriteByte(ofile, objp->type);
	cf_WriteShort(ofile, objp->id);

	cf_WriteString(ofile, objp->name ? objp->name : "");

	cf_WriteInt(ofile, objp->flags);

	if (objp->type == OBJ_DOOR)
		cf_WriteShort(ofile, objp->shields);

	cf_WriteInt(ofile, objp->roomnum);
	cf_WriteVector(ofile, &objp->pos);
	cf_WriteMatrix(ofile, &objp->orient);

	cf_WriteByte(ofile, objp->contains_type);
	cf_WriteByte(ofile, objp->contains_id);
	cf_WriteByte(ofile, objp->contains_count);

	cf_WriteFloat(ofile, objp->lifeleft);

	ASSERT(objp->parent_handle == OBJECT_HANDLE_NONE);		//not writing, so make sure none

	//Write sound info if this is a soundsource object
	if (objp->control_type == CT_SOUNDSOURCE) {
		ASSERT(objp->type == OBJ_SOUNDSOURCE);
		cf_WriteString(ofile, (objp->ctype.soundsource_info.sound_index == -1) ? "" : Sounds[objp->ctype.soundsource_info.sound_index].name);
		cf_WriteFloat(ofile, objp->ctype.soundsource_info.volume);
	}

	i = (objp->custom_default_script_name) ? strlen(objp->custom_default_script_name) : 0;
	cf_WriteByte(ofile, i);
	if (i > 0)
		cf_WriteBytes((ubyte*)objp->custom_default_script_name, i, ofile);

	i = (objp->custom_default_module_name) ? strlen(objp->custom_default_module_name) : 0;
	cf_WriteByte(ofile, i);
	if (i > 0)
		cf_WriteBytes((ubyte*)objp->custom_default_module_name, i, ofile);


	// If there is lightmap data for this object, write it out.
	if (objp->lighting_render_type == LRT_LIGHTMAPS)
	{
		if (objp->lm_object.used == 0)
		{
			mprintf((0, "Warning: Object %d is set for lightmaps but has no lightmap data!\n", objp - Objects));
			cf_WriteByte(ofile, 0);
		}
		else
		{
			cf_WriteByte(ofile, 1);
			cf_WriteByte(ofile, objp->lm_object.num_models);
			for (i = 0; i < objp->lm_object.num_models; i++)
			{
				cf_WriteShort(ofile, objp->lm_object.num_faces[i]);
				for (t = 0; t < objp->lm_object.num_faces[i]; t++)
				{
					lightmap_object_face* fp = &objp->lm_object.lightmap_faces[i][t];
					ASSERT(LightmapInfoRemap[fp->lmi_handle] != BAD_LMI_INDEX);
					cf_WriteShort(ofile, (ushort)LightmapInfoRemap[fp->lmi_handle]);

					cf_WriteVector(ofile, &fp->rvec);
					cf_WriteVector(ofile, &fp->uvec);

					cf_WriteByte(ofile, fp->num_verts);

					for (k = 0; k < fp->num_verts; k++)
					{
						cf_WriteFloat(ofile, fp->u2[k]);
						cf_WriteFloat(ofile, fp->v2[k]);
					}
				}
			}
		}
	}
	else
		cf_WriteByte(ofile, 0);

	return 1;
}

//Writes a trigger
//Returns: true if written ok, else false
int WriteTrigger(CFILE* ofile, trigger* tp)
{
	cf_WriteString(ofile, tp->name);
	cf_WriteShort(ofile, tp->roomnum);
	cf_WriteShort(ofile, tp->facenum);
	cf_WriteShort(ofile, tp->flags);
	cf_WriteShort(ofile, tp->activator);

	return 1;
}

//Writes a face to a disk file
//Parameters:	ofile - file to write to
//					fp - face to write
//Returns:		1 if written ok, else 0
int WriteFace(CFILE* ofile, face* fp)
{
	int i;

	//Write number of verts
	cf_WriteByte(ofile, fp->num_verts);

	//Verts
	for (i = 0; i < fp->num_verts; i++)
		cf_WriteShort(ofile, fp->face_verts[i]);

	//uvls
	fp->flags &= ~FF_VERTEX_ALPHA;

	for (i = 0; i < fp->num_verts; i++) {
		cf_WriteFloat(ofile, fp->face_uvls[i].u);
		cf_WriteFloat(ofile, fp->face_uvls[i].v);
		cf_WriteByte(ofile, fp->face_uvls[i].alpha);

		if (fp->face_uvls[i].alpha != 255)
			fp->flags |= FF_VERTEX_ALPHA;
	}

	//Other stuff
	if ((fp->flags & FF_LIGHTMAP) && (fp->lmi_handle == BAD_LMI_INDEX || LightmapInfoRemap[fp->lmi_handle] == BAD_LMI_INDEX))
	{
		fp->flags &= ~FF_LIGHTMAP;
		mprintf((0, "Almost saved a bogus lightmap!\n"));
	}

	cf_WriteShort(ofile, fp->flags);
	cf_WriteByte(ofile, fp->portal_num);
	cf_WriteShort(ofile, fp->tmap);

	if (fp->flags & FF_LIGHTMAP)
	{
		// Write UV2's

		ASSERT(LightmapInfoRemap[fp->lmi_handle] != BAD_LMI_INDEX);
		cf_WriteShort(ofile, (ushort)LightmapInfoRemap[fp->lmi_handle]);

		for (i = 0; i < fp->num_verts; i++)
		{
			cf_WriteFloat(ofile, fp->face_uvls[i].u2);
			cf_WriteFloat(ofile, fp->face_uvls[i].v2);
		}
	}

	if (fp->light_multiple == 186)
	{
		fp->light_multiple = 4;	// Get Jason, I'm looking for this bug!  Its safe to go past it, but I'm just on the lookout
		mprintf((0, "Bogus light multiple detected...bashing!\n"));
	}

	cf_WriteByte(ofile, fp->light_multiple);


	if (fp->special_handle != BAD_SPECIAL_FACE_INDEX)
	{
		// Write out special face stuff
		ubyte smooth = 0;
		cf_WriteByte(ofile, 1);
		cf_WriteByte(ofile, SpecialFaces[fp->special_handle].type);
		cf_WriteByte(ofile, 4);

		if ((GameTextures[fp->tmap].flags & TF_SMOOTH_SPECULAR) && (SpecialFaces[fp->special_handle].flags & SFF_SPEC_SMOOTH))
		{
			smooth = 1;
			cf_WriteByte(ofile, 1);
			cf_WriteByte(ofile, fp->num_verts);
		}
		else
		{
			cf_WriteByte(ofile, 0);
		}


		for (i = 0; i < 4; i++)
		{
			cf_WriteVector(ofile, &SpecialFaces[fp->special_handle].spec_instance[i].bright_center);
			cf_WriteShort(ofile, SpecialFaces[fp->special_handle].spec_instance[i].bright_color);
		}
		if (smooth)
		{
			for (i = 0; i < fp->num_verts; i++)
			{
				cf_WriteVector(ofile, &SpecialFaces[fp->special_handle].vertnorms[i]);
			}
		}
	}
	else
		cf_WriteByte(ofile, 0);



	return 1;
}

//Writes a portal to a disk file
//Parameters:	ofile - file to write to
//					pp - portal to write
//Returns:		1 if written ok, else 0
int WritePortal(CFILE* ofile, portal* pp)
{
	cf_WriteInt(ofile, pp->flags);

	cf_WriteShort(ofile, pp->portal_face);

	cf_WriteInt(ofile, pp->croom);
	cf_WriteInt(ofile, pp->cportal);
	cf_WriteShort(ofile, pp->bnode_index);

	cf_WriteVector(ofile, &pp->path_pnt);

	cf_WriteInt(ofile, pp->combine_master);

	return 1;
}

//Writes a room to a disk file
//Parameters:	ofile - file to write to
//					rp - room to write
//Returns:		1 if written ok, else 0
int WriteRoom(CFILE* ofile, room* rp)
{
	int i;

	//Write counts
	cf_WriteInt(ofile, rp->num_verts);
	cf_WriteInt(ofile, rp->num_faces);
	cf_WriteInt(ofile, rp->num_portals);

	//Write name
	cf_WriteString(ofile, rp->name ? rp->name : "");

	// Write out the path point
	cf_WriteVector(ofile, &rp->path_pnt);

	//Write verts
	for (i = 0; i < rp->num_verts; i++)
	{
		cf_WriteVector(ofile, &rp->verts[i]);
	}

	//Write faces
	for (i = 0; i < rp->num_faces; i++)
		WriteFace(ofile, &rp->faces[i]);

	//Write portals
	for (i = 0; i < rp->num_portals; i++)
		WritePortal(ofile, &rp->portals[i]);

	//Write rest of data
	cf_WriteInt(ofile, rp->flags);

	// Write pulse time
	cf_WriteByte(ofile, rp->pulse_time);
	cf_WriteByte(ofile, rp->pulse_offset);

	// Write mirrored face
	cf_WriteShort(ofile, rp->mirror_face);

	// Save doorway data if this room has a door
	if (rp->flags & RF_DOOR)
	{
		doorway* dp = rp->doorway_data;

		cf_WriteByte(ofile, dp->flags);
		cf_WriteByte(ofile, dp->keys_needed);
		cf_WriteInt(ofile, dp->doornum);
		cf_WriteFloat(ofile, dp->position);
	}

	if (!rp->volume_lights)
		cf_WriteByte(ofile, 0);
	else
	{
		cf_WriteByte(ofile, 1);
		int w = rp->volume_width;
		int h = rp->volume_height;
		int d = rp->volume_depth;

		cf_WriteInt(ofile, w);
		cf_WriteInt(ofile, h);
		cf_WriteInt(ofile, d);

		CheckToWriteCompressByte(ofile, rp->volume_lights, w * h * d);
	}

	cf_WriteFloat(ofile, rp->fog_depth);
	cf_WriteFloat(ofile, rp->fog_r);
	cf_WriteFloat(ofile, rp->fog_g);
	cf_WriteFloat(ofile, rp->fog_b);

	//Write ambient sound pattern name
	cf_WriteString(ofile, (rp->ambient_sound == -1) ? "" : AmbientSoundPatternName(rp->ambient_sound));

	cf_WriteByte(ofile, (sbyte)rp->env_reverb);

	//Write damage
	cf_WriteFloat(ofile, rp->damage);
	cf_WriteByte(ofile, rp->damage_type);

	return 1;
}

//Writes a chunk header.  Writes chunk id & placeholder length.  Returns chunk start pos
int StartChunk(CFILE* ofile, char* chunk_name)
{
	int chunk_start_pos;

	cf_WriteBytes((ubyte*)chunk_name, 4, ofile);
	chunk_start_pos = cftell(ofile);
	cf_WriteInt(ofile, 0);				//placeholder for chunk len

	return chunk_start_pos;
}

//Fill in chunk length when done writing
void EndChunk(CFILE* ofile, int chunk_start_pos)
{
	int save_pos = cftell(ofile);
	int len = save_pos - chunk_start_pos;

	//pad the chunk if necessary to make multiple of four bytes
	while (len & 3) {
		cf_WriteByte(ofile, 0);
		len++;
		save_pos++;
	}

	//seek back to len field and fill in value
	cfseek(ofile, chunk_start_pos, SEEK_SET);
	cf_WriteInt(ofile, len);						//write chunk length

	//go back to end of file
	cfseek(ofile, save_pos, SEEK_SET);
}

void WriteTerrainHeightChunk(CFILE* fp)
{
	int i;
	ubyte heightvals[TERRAIN_WIDTH * TERRAIN_DEPTH];

	int start_pos;
	start_pos = StartChunk(fp, CHUNK_TERRAIN_HEIGHT);

	for (i = 0; i < TERRAIN_WIDTH * TERRAIN_DEPTH; i++)
		heightvals[i] = Terrain_seg[i].ypos;

	CheckToWriteCompressByte(fp, heightvals, TERRAIN_WIDTH * TERRAIN_DEPTH);

	EndChunk(fp, start_pos);
}

void WriteGamePathsChunk(CFILE* fp)
{
	int start_pos;
	int i, j;

	start_pos = StartChunk(fp, CHUNK_GAME_PATHS);

	// Get the number of paths
	cf_WriteShort(fp, Num_game_paths);

	for (i = 0; i < MAX_GAME_PATHS; i++)
	{
		if (GamePaths[i].used)
		{
			// Write out the path's info
			cf_WriteString(fp, GamePaths[i].name);
			cf_WriteInt(fp, GamePaths[i].num_nodes);
			cf_WriteByte(fp, GamePaths[i].flags);

			for (j = 0; j < GamePaths[i].num_nodes; j++)
			{
				cf_WriteVector(fp, &GamePaths[i].pathnodes[j].pos);
				cf_WriteInt(fp, GamePaths[i].pathnodes[j].roomnum);
				cf_WriteInt(fp, GamePaths[i].pathnodes[j].flags);
				cf_WriteVector(fp, &GamePaths[i].pathnodes[j].fvec);
				cf_WriteVector(fp, &GamePaths[i].pathnodes[j].uvec);
			}
		}
	}

	EndChunk(fp, start_pos);
}

void WriteOverrideSoundChunk(CFILE* fp)
{
	int start_pos;
	start_pos = StartChunk(fp, CHUNK_OVERRIDE_SOUNDS);

	if (sound_override_force_field != -1)
	{
		cf_WriteString(fp, Sounds[sound_override_force_field].name);
	}
	else
	{
		cf_WriteString(fp, "");
	}

	if (sound_override_glass_breaking != -1)
	{
		cf_WriteString(fp, Sounds[sound_override_glass_breaking].name);
	}
	else
	{
		cf_WriteString(fp, "");
	}

	EndChunk(fp, start_pos);
}

void WriteFFTMChunk(CFILE* fp)
{
	int start_pos;
	start_pos = StartChunk(fp, CHUNK_FFT_MOD);

	int i;

	for (i = 0; i < MAX_FORCE_FIELD_BOUNCE_TEXTURES; i++)
	{
		if (force_field_bounce_texture[i] != -1)
		{
			cf_WriteString(fp, GameTextures[force_field_bounce_texture[i]].name);
			cf_WriteFloat(fp, force_field_bounce_multiplier[i]);
		}
		else
		{
			cf_WriteString(fp, "");
		}
	}

	EndChunk(fp, start_pos);
}


void WriteBNodeChunk(CFILE* fp)
{
	int i;
	int j;
	int k;

	int start_pos;
	start_pos = StartChunk(fp, CHUNK_BNODES);

	cf_WriteShort(fp, Highest_room_index + 8);

	for (i = 0; i <= Highest_room_index + 8; i++)
	{
		if ((i <= Highest_room_index && !Rooms[i].used))
		{
			cf_WriteByte(fp, 0);
		}
		else
		{
			cf_WriteByte(fp, 1);
			bn_list* bnlist = BNode_GetBNListPtr(i, true);

			cf_WriteShort(fp, bnlist->num_nodes);
			if (bnlist->num_nodes)
			{
				for (j = 0; j < bnlist->num_nodes; j++)
				{
					cf_WriteFloat(fp, bnlist->nodes[j].pos.x);
					cf_WriteFloat(fp, bnlist->nodes[j].pos.y);
					cf_WriteFloat(fp, bnlist->nodes[j].pos.z);

					cf_WriteShort(fp, bnlist->nodes[j].num_edges);
					if (bnlist->nodes[j].num_edges)
					{
						for (k = 0; k < bnlist->nodes[j].num_edges; k++)
						{
							cf_WriteShort(fp, bnlist->nodes[j].edges[k].end_room);
							cf_WriteByte(fp, bnlist->nodes[j].edges[k].end_index);

							cf_WriteShort(fp, bnlist->nodes[j].edges[k].flags);
							cf_WriteShort(fp, bnlist->nodes[j].edges[k].cost);

							cf_WriteFloat(fp, bnlist->nodes[j].edges[k].max_rad);
						}
					}
				}
			}
		}
	}
	cf_WriteByte(fp, BNode_verified ? 1 : 0);

	EndChunk(fp, start_pos);
}

void WriteBOAChunk(CFILE* fp)
{
	int start_pos;
	int i, j;

	start_pos = StartChunk(fp, CHUNK_BOA);

	cf_WriteInt(fp, BOA_mine_checksum);
	cf_WriteInt(fp, BOA_vis_checksum);
	cf_WriteInt(fp, Highest_room_index + 8);
	cf_WriteInt(fp, MAX_PATH_PORTALS);

	for (i = 0; i <= Highest_room_index + 8; i++)
	{
		for (j = 0; j <= Highest_room_index + 8; j++)
		{
			cf_WriteShort(fp, BOA_Array[i][j]);
		}
	}

	for (i = 0; i <= Highest_room_index + 8; i++)
	{
		for (j = 0; j < MAX_PATH_PORTALS; j++)
		{
			cf_WriteFloat(fp, BOA_cost_array[i][j]);
		}
	}

	cf_WriteInt(fp, BOA_num_mines);
	cf_WriteInt(fp, BOA_num_terrain_regions);

	for (i = 0; i < BOA_num_terrain_regions; i++)
	{
		cf_WriteInt(fp, BOA_num_connect[i]);

		for (j = 0; j < BOA_num_connect[i]; j++)
		{
			cf_WriteInt(fp, BOA_connect[i][j].roomnum);
			cf_WriteInt(fp, BOA_connect[i][j].portal);
		}
	}

	EndChunk(fp, start_pos);
}

void WriteRoomAABBChunk(CFILE* fp)
{
	int start_pos;
	int i;

	start_pos = StartChunk(fp, CHUNK_ROOM_AABB);

	cf_WriteInt(fp, Highest_room_index);

	for (i = 0; i <= Highest_room_index; i++)
	{
		cf_WriteInt(fp, BOA_AABB_ROOM_checksum[i]);
	}

	for (i = 0; i <= Highest_room_index; i++)
	{
		if (Rooms[i].used)
		{
			int j;
			int k;

			ASSERT(Rooms[i].bbf_list && Rooms[i].bbf_list_min_xyz && Rooms[i].bbf_list_max_xyz && Rooms[i].bbf_list_sector);

			cf_WriteInt(fp, 1);  // used

			cf_WriteInt(fp, Rooms[i].num_faces);
			for (j = 0; j < Rooms[i].num_faces; j++)
			{
				cf_WriteFloat(fp, Rooms[i].faces[j].min_xyz.x);
				cf_WriteFloat(fp, Rooms[i].faces[j].min_xyz.y);
				cf_WriteFloat(fp, Rooms[i].faces[j].min_xyz.z);

				cf_WriteFloat(fp, Rooms[i].faces[j].max_xyz.x);
				cf_WriteFloat(fp, Rooms[i].faces[j].max_xyz.y);
				cf_WriteFloat(fp, Rooms[i].faces[j].max_xyz.z);
			}

			cf_WriteInt(fp, BOA_AABB_ROOM_checksum[i]);

			cf_WriteFloat(fp, Rooms[i].bbf_min_xyz.x);
			cf_WriteFloat(fp, Rooms[i].bbf_min_xyz.y);
			cf_WriteFloat(fp, Rooms[i].bbf_min_xyz.z);

			cf_WriteFloat(fp, Rooms[i].bbf_max_xyz.x);
			cf_WriteFloat(fp, Rooms[i].bbf_max_xyz.y);
			cf_WriteFloat(fp, Rooms[i].bbf_max_xyz.z);

			cf_WriteShort(fp, Rooms[i].num_bbf_regions);
			for (j = 0; j < Rooms[i].num_bbf_regions; j++)
			{
				cf_WriteShort(fp, Rooms[i].num_bbf[j]);
			}

			for (j = 0; j < Rooms[i].num_bbf_regions; j++)
			{
				for (k = 0; k < Rooms[i].num_bbf[j]; k++)
				{
					cf_WriteShort(fp, Rooms[i].bbf_list[j][k]);
				}

				cf_WriteFloat(fp, Rooms[i].bbf_list_min_xyz[j].x);
				cf_WriteFloat(fp, Rooms[i].bbf_list_min_xyz[j].y);
				cf_WriteFloat(fp, Rooms[i].bbf_list_min_xyz[j].z);

				cf_WriteFloat(fp, Rooms[i].bbf_list_max_xyz[j].x);
				cf_WriteFloat(fp, Rooms[i].bbf_list_max_xyz[j].y);
				cf_WriteFloat(fp, Rooms[i].bbf_list_max_xyz[j].z);

				cf_WriteByte(fp, Rooms[i].bbf_list_sector[j]);
			}
		}
		else
		{
			cf_WriteInt(fp, 0);  // Not used
		}
	}

	EndChunk(fp, start_pos);
}

void WriteMatcenChunk(CFILE* fp)
{
	int start_pos;

	start_pos = StartChunk(fp, CHUNK_MATCEN_DATA);

	cf_WriteInt(fp, Num_matcens);

	int i;

	for (i = 0; i < Num_matcens; i++)
	{
		ASSERT(Matcen[i]);
		Matcen[i]->Reset(); // Makes it like it hasn't run yet
		Matcen[i]->SaveData(fp);
	}

	EndChunk(fp, start_pos);
}


void WriteALifeChunk(CFILE* fp)
{
#ifndef NEWEDITOR
	int start_pos;

	start_pos = StartChunk(fp, CHUNK_ALIFE_DATA);

	a_life.SaveData(fp);

	EndChunk(fp, start_pos);
#endif
}


void WriteLevelGoalsChunk(CFILE* fp)
{
	int start_pos;

	start_pos = StartChunk(fp, CHUNK_LEVEL_GOALS);
	Level_goals.SaveLevelGoalInfo(fp);
	EndChunk(fp, start_pos);
}

void WriteTerrainSkyAndLightChunk(CFILE* fp)
{
	int i;
	int max_sats;

	max_sats = Terrain_sky.num_satellites;

	int start_pos;
	start_pos = StartChunk(fp, CHUNK_TERRAIN_SKY);

	cf_WriteFloat(fp, Terrain_sky.fog_scalar);
	cf_WriteFloat(fp, Terrain_sky.damage_per_second);

	cf_WriteByte(fp, Terrain_sky.textured);

	cf_WriteShort(fp, Terrain_sky.dome_texture);

	cf_WriteInt(fp, Terrain_sky.sky_color);
	cf_WriteInt(fp, Terrain_sky.horizon_color);
	cf_WriteInt(fp, Terrain_sky.fog_color);

	cf_WriteInt(fp, Terrain_sky.flags);
	cf_WriteFloat(fp, Terrain_sky.radius);
	cf_WriteFloat(fp, Terrain_sky.rotate_rate);

	cf_WriteInt(fp, max_sats);


	for (i = 0; i < max_sats; i++)
	{
		cf_WriteShort(fp, Terrain_sky.satellite_texture[i]);
		cf_WriteVector(fp, &Terrain_sky.satellite_vectors[i]);
		cf_WriteByte(fp, Terrain_sky.satellite_flags[i]);
		cf_WriteFloat(fp, Terrain_sky.satellite_size[i]);

		cf_WriteFloat(fp, Terrain_sky.satellite_r[i]);
		cf_WriteFloat(fp, Terrain_sky.satellite_g[i]);
		cf_WriteFloat(fp, Terrain_sky.satellite_b[i]);

	}

	// Write lighting
	ubyte* byte_vals = (ubyte*)mem_malloc(TERRAIN_DEPTH * TERRAIN_WIDTH);

	// l
	for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
		byte_vals[i] = Terrain_seg[i].l;
	CheckToWriteCompressByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);

	// r
	for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
		byte_vals[i] = Terrain_seg[i].r;
	CheckToWriteCompressByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);

	// g
	for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
		byte_vals[i] = Terrain_seg[i].g;
	CheckToWriteCompressByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);

	// b
	for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
		byte_vals[i] = Terrain_seg[i].b;
	CheckToWriteCompressByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);

	// Terrain_dynamic_table
	for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
		byte_vals[i] = Terrain_dynamic_table[i];
	CheckToWriteCompressByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);

	// Terrain occlusion table
	cf_WriteInt(fp, Terrain_occlusion_checksum);
	for (i = 0; i < OCCLUSION_SIZE * OCCLUSION_SIZE * 32; i++)
	{
		int row = i / 32;
		int column = i % 32;

		byte_vals[i] = Terrain_occlusion_map[row][column];
	}
	CheckToWriteCompressByte(fp, byte_vals, OCCLUSION_SIZE * OCCLUSION_SIZE * 32);

	mem_free(byte_vals);

	EndChunk(fp, start_pos);
}


void WriteTerrainTmapChunk(CFILE* fp)
{
	int i;

	ubyte* byte_vals = (ubyte*)mem_malloc(TERRAIN_DEPTH * TERRAIN_WIDTH);
	ushort* short_vals = (ushort*)mem_malloc(2 * TERRAIN_DEPTH * TERRAIN_WIDTH);

	int start_pos;
	start_pos = StartChunk(fp, CHUNK_TERRAIN_TMAPS_FLAGS);

	// Tmap 1s
	for (i = 0; i < TERRAIN_TEX_DEPTH * TERRAIN_TEX_WIDTH; i++)
		short_vals[i] = Terrain_tex_seg[i].tex_index;
	CheckToWriteCompressShort(fp, short_vals, TERRAIN_TEX_DEPTH * TERRAIN_TEX_WIDTH);

	// Rotators
	for (i = 0; i < TERRAIN_TEX_DEPTH * TERRAIN_TEX_WIDTH; i++)
		byte_vals[i] = Terrain_tex_seg[i].rotation;
	CheckToWriteCompressByte(fp, byte_vals, TERRAIN_TEX_DEPTH * TERRAIN_TEX_WIDTH);

	// flags
	for (i = 0; i < TERRAIN_DEPTH * TERRAIN_WIDTH; i++)
		byte_vals[i] = Terrain_seg[i].flags;
	CheckToWriteCompressByte(fp, byte_vals, TERRAIN_DEPTH * TERRAIN_WIDTH);

	mem_free(byte_vals);
	mem_free(short_vals);

	EndChunk(fp, start_pos);
}

void WriteTerrainChunks(CFILE* outfile)
{
	WriteTerrainHeightChunk(outfile);
	WriteTerrainTmapChunk(outfile);
	WriteTerrainSkyAndLightChunk(outfile);

	// Write out end chunk
	int start_pos = StartChunk(outfile, CHUNK_TERRAIN_END);
	EndChunk(outfile, start_pos);
}


void WriteLightmapChunk(CFILE* fp)
{
	int start_pos;
	int i;
	int lightmap_info_count = 0;
	int lightmap_count = 0;

	ushort* lightmap_remap = (ushort*)mem_malloc(MAX_LIGHTMAPS * sizeof(ushort));
	ubyte* lightmap_spoken_for = (ubyte*)mem_malloc(MAX_LIGHTMAPS);

	ASSERT(lightmap_remap);
	ASSERT(lightmap_spoken_for);

	memset(lightmap_spoken_for, 0, MAX_LIGHTMAPS);

	// Build a lightmap remap list
	for (i = 0; i < MAX_LIGHTMAP_INFOS; i++)
	{
		if (LightmapInfo[i].used && LightmapInfo[i].type != LMI_DYNAMIC)
		{
			if (!lightmap_spoken_for[LightmapInfo[i].lm_handle])
			{
				lightmap_spoken_for[LightmapInfo[i].lm_handle] = 1;
				lightmap_remap[LightmapInfo[i].lm_handle] = lightmap_count;

				lightmap_count++;
			}

			LightmapInfoRemap[i] = lightmap_info_count;
			lightmap_info_count++;
		}
		else
			LightmapInfoRemap[i] = BAD_LMI_INDEX;
	}

	memset(lightmap_spoken_for, 0, MAX_LIGHTMAPS);
	start_pos = StartChunk(fp, CHUNK_NEW_LIGHTMAPS);

	cf_WriteInt(fp, lightmap_count);

	// Write out lightmap texture data
	for (i = 0; i < MAX_LIGHTMAP_INFOS; i++)
	{
		if (LightmapInfo[i].used && LightmapInfo[i].type != LMI_DYNAMIC)
		{
			if (!lightmap_spoken_for[LightmapInfo[i].lm_handle])
			{
				lightmap_spoken_for[LightmapInfo[i].lm_handle] = 1;
				int w = lm_w(LightmapInfo[i].lm_handle);
				int h = lm_h(LightmapInfo[i].lm_handle);

				ASSERT(w >= 2 && w <= 128);
				ASSERT(h >= 2 && h <= 128);

				cf_WriteShort(fp, w);
				cf_WriteShort(fp, h);

				ushort* data = lm_data(LightmapInfo[i].lm_handle);
				ASSERT(data != NULL);

				CheckToWriteCompressShort(fp, data, w * h);
			}
		}
	}

	cf_WriteInt(fp, lightmap_info_count);

	for (i = 0; i < MAX_LIGHTMAP_INFOS; i++)
	{
		if (LightmapInfo[i].used && LightmapInfo[i].type != LMI_DYNAMIC)
		{
			cf_WriteShort(fp, lightmap_remap[LightmapInfo[i].lm_handle]);
			int w = lmi_w(i);
			int h = lmi_h(i);

			cf_WriteShort(fp, w);
			cf_WriteShort(fp, h);

			cf_WriteByte(fp, LightmapInfo[i].type);

			cf_WriteShort(fp, LightmapInfo[i].x1);
			cf_WriteShort(fp, LightmapInfo[i].y1);

			cf_WriteByte(fp, LightmapInfo[i].xspacing);
			cf_WriteByte(fp, LightmapInfo[i].yspacing);

			cf_WriteVector(fp, &LightmapInfo[i].upper_left);
			cf_WriteVector(fp, &LightmapInfo[i].normal);
		}
	}

	mem_free(lightmap_remap);
	mem_free(lightmap_spoken_for);

	EndChunk(fp, start_pos);
}

//Macro to write out names of a certain data type
#define WRITE_DATA_NAMES(max_items,item_array)										\
	do {																							\
		highest_used_index = -1;		/* Find index of highest used */			\
		for (i=0;i<max_items;i++)															\
			if (item_array[i].used)															\
				highest_used_index = i;														\
		cf_WriteInt(ofile,highest_used_index+1);										\
		for (i=0;i<=highest_used_index;i++)												\
			cf_WriteString(ofile,item_array[i].used?item_array[i].name:"");  	\
	} while (0)

//Macro to write out names of a certain data type
#define WRITE_DATA_NAMES_GENERIC(max_items,item_array,chunk_name)				\
	do {																							\
		int chunk_start_pos;																	\
		chunk_start_pos = StartChunk(ofile,chunk_name);							\
		highest_used_index = -1;		/* Find index of highest used */			\
		for (i=0;i<max_items;i++)															\
			if (item_array[i].type != OBJ_NONE)											\
				highest_used_index = i;														\
		cf_WriteInt(ofile,highest_used_index+1);										\
		for (i=0;i<=highest_used_index;i++)												\
			cf_WriteString(ofile,(item_array[i].type!=OBJ_NONE)?item_array[i].name:"");  	\
		EndChunk(ofile,chunk_start_pos);												\
	} while (0)

//Write the texture names for remapping when level is loaded
void WriteTextureList(CFILE* ofile)
{
	int i, highest_used_index;

	WRITE_DATA_NAMES(MAX_TEXTURES, GameTextures);
}

void WritePlayerStarts(CFILE* outfile)
{
	int i;

	cf_WriteShort(outfile, MAX_PLAYERS);

	for (i = 0; i < MAX_PLAYERS; i++)
	{
		cf_WriteInt(outfile, Players[i].startpos_flags);
	}
}


#include "editor/ebnode.h"

//Save a level file
//Returns 1 if file saved ok, else 0
int SaveLevel(char* filename, bool f_save_room_AABB)
{
	CFILE* ofile;
	int i, highest_used_index;
	int chunk_start_pos;
	int count;

	//Check for too many path portals
	for (int r = 0; r <= Highest_room_index; r++) {
		if (Rooms[r].num_portals > MAX_PATH_PORTALS)
			if (EditorMessageBox(MBOX_YESNO, "Room %d has %d portals, which is more than the %d portals that the path system allows."
				"\n\nDo you want to save anyway?", r, MAX_PATH_PORTALS) != IDYES)
				return 0;
	}
#ifndef NEWEDITOR
	//Get the level ready to save
	ClearTransientObjects(1);		//1 means clear proximity bombs
#endif
	//CompressMine();					//get rid of holes in room list
	MakeBOA();
	EBNode_VerifyGraph();
	ConsolidateMineMirrors();

	if (f_save_room_AABB)
	{
		//Compute the bounding boxes
		ComputeAABB(true);
	}

	//Find shells for all the rooms
	if (!FindArg("-noshells"))
		ComputeAllRoomShells();

	ofile = cfopen(filename, "wb");

	if (!ofile) {
		EditorMessageBox("Can't open file <%s> for writing: %s.", filename, sys_errlist[errno]);
		return 0;
	}

	try {		//catch cfile errors

		//Write tag & version number
		cf_WriteBytes((ubyte*)LEVEL_FILE_TAG, 4, ofile);
		cf_WriteInt(ofile, LEVEL_FILE_VERSION);


		WriteGamePathsChunk(ofile);

		//Write terrain sounds
		chunk_start_pos = StartChunk(ofile, CHUNK_TERRAIN_SOUND);
		int n_bands = 0, b;
		for (b = 0; b < NUM_TERRAIN_SOUND_BANDS; b++)
			if (Terrain_sound_bands[b].sound_index != -1)
				n_bands++;
		cf_WriteInt(ofile, n_bands);
		for (b = 0; b < NUM_TERRAIN_SOUND_BANDS; b++) {
			if (Terrain_sound_bands[b].sound_index != -1) {
				cf_WriteString(ofile, Sounds[Terrain_sound_bands[b].sound_index].name);
				cf_WriteByte(ofile, Terrain_sound_bands[b].low_alt);
				cf_WriteByte(ofile, Terrain_sound_bands[b].high_alt);
				cf_WriteFloat(ofile, Terrain_sound_bands[b].low_volume);
				cf_WriteFloat(ofile, Terrain_sound_bands[b].high_volume);
			}
		}
		EndChunk(ofile, chunk_start_pos);

		// Write lightmaps
		// NOTE: This WriteLightmapChunk must appear before the object,room,or terrain chunks get 
		// written out!!!!
		WriteLightmapChunk(ofile);

		chunk_start_pos = StartChunk(ofile, CHUNK_PLAYER_STARTS);
		WritePlayerStarts(ofile);
		EndChunk(ofile, chunk_start_pos);

		//Write the texture names for remapping when level is loaded
		chunk_start_pos = StartChunk(ofile, CHUNK_TEXTURE_NAMES);
		WriteTextureList(ofile);
		EndChunk(ofile, chunk_start_pos);

		//Write the object names for remapping when level is loaded
		WRITE_DATA_NAMES_GENERIC(MAX_OBJECT_IDS, Object_info, CHUNK_GENERIC_NAMES);

		//Write the door names for remapping when level is loaded
		chunk_start_pos = StartChunk(ofile, CHUNK_DOOR_NAMES);
		WRITE_DATA_NAMES(MAX_DOORS, Doors);
		EndChunk(ofile, chunk_start_pos);

		//Write rooms
		chunk_start_pos = StartChunk(ofile, CHUNK_ROOMS);

		//Count & write out number of rooms

		//Count * write number of rooms, verts, faces, etc. 
		room* rp;
		int nrooms = 0, nverts = 0, nfaces = 0, nfaceverts = 0, nportals = 0;
		extern int CountRoomFaceVerts(room * rp);		//TEMP: move to room.h
		for (i = 0, rp = Rooms; i <= Highest_room_index; i++, rp++)		//Count the number of faces, verts, etc. in the level
			if (rp->used) {
				nrooms++;
				nverts += rp->num_verts;
				nfaces += rp->num_faces;
				nfaceverts += CountRoomFaceVerts(rp);
				nportals += rp->num_portals;
			}
		cf_WriteInt(ofile, nrooms);
		cf_WriteInt(ofile, nverts);
		cf_WriteInt(ofile, nfaces);
		cf_WriteInt(ofile, nfaceverts);
		cf_WriteInt(ofile, nportals);

		//Write out the rooms
		for (i = 0; i <= Highest_room_index; i++) {
			if (Rooms[i].used) {
				cf_WriteShort(ofile, i);		//write out room number
				WriteRoom(ofile, &Rooms[i]);
			}
		}

		//Done with rooms
		EndChunk(ofile, chunk_start_pos);

		//Write room wind, if any rooms have wind
		for (i = nrooms = 0, rp = Rooms; i <= Highest_room_index; i++, rp++)	//Count the number of rooms with wind
			if ((rp->wind.x != 0.0) || (rp->wind.y != 0.0) || (rp->wind.z != 0.0))
				nrooms++;
		if (nrooms) {
			chunk_start_pos = StartChunk(ofile, CHUNK_ROOM_WIND);
			cf_WriteInt(ofile, nrooms);
			for (i = 0, rp = Rooms; i <= Highest_room_index; i++, rp++) {	//write the wind values
				if ((rp->wind.x != 0.0) || (rp->wind.y != 0.0) || (rp->wind.z != 0.0)) {
					cf_WriteShort(ofile, i);
					cf_WriteVector(ofile, &rp->wind);
				}
			}
			EndChunk(ofile, chunk_start_pos);
		}

		//Write terrain
		chunk_start_pos = StartChunk(ofile, CHUNK_TERRAIN);
		WriteTerrainChunks(ofile);
		EndChunk(ofile, chunk_start_pos);

		//count the number of deleted object handles we need to write
		for (i = 0, count = 0; i < MAX_OBJECTS; i++)
			if ((Objects[i].type == OBJ_NONE) && ((Objects[i].handle & HANDLE_COUNT_MASK) != 0))
				count++;

		//write object handles for deleted objects with non-zero handles
		chunk_start_pos = StartChunk(ofile, CHUNK_OBJECT_HANDLES);
		cf_WriteInt(ofile, count);
		for (i = 0; i < MAX_OBJECTS; i++)
			if ((Objects[i].type == OBJ_NONE) && ((Objects[i].handle & HANDLE_COUNT_MASK) != 0))
				cf_WriteInt(ofile, Objects[i].handle);
		EndChunk(ofile, chunk_start_pos);

		//count the objects
		for (i = 0, count = 0; i <= Highest_object_index; i++)
			if ((Objects[i].type != OBJ_NONE) && (Objects[i].type != OBJ_ROOM))
				count++;

		//write objects
		chunk_start_pos = StartChunk(ofile, CHUNK_OBJECTS);
		cf_WriteInt(ofile, count);
		for (i = 0; i <= Highest_object_index; i++) {
			if ((Objects[i].type != OBJ_NONE) && (Objects[i].type != OBJ_ROOM)) {
				cf_WriteInt(ofile, Objects[i].handle);
				WriteObject(ofile, &Objects[i]);
			}
		}
		EndChunk(ofile, chunk_start_pos);

		//Write triggers
		chunk_start_pos = StartChunk(ofile, CHUNK_TRIGGERS);
		cf_WriteInt(ofile, Num_triggers);
		for (i = 0; i < Num_triggers; i++) {
			WriteTrigger(ofile, &Triggers[i]);
		}
		EndChunk(ofile, chunk_start_pos);

#ifndef NEWEDITOR
		// Save BSP info (if there is any)
		if (BSP_initted)
		{
			chunk_start_pos = StartChunk(ofile, CHUNK_NEW_BSP);
			cf_WriteInt(ofile, BSPChecksum);

			SaveBSPNode(ofile, MineBSP.root);

			EndChunk(ofile, chunk_start_pos);
		}
#endif

		// This has to be after all BOA related fields; so, a read level can build boa on the
		// spot if necessary.
		WriteBOAChunk(ofile);

		if (BNode_allocated)
		{
			WriteBNodeChunk(ofile);
		}

#ifndef NEWEDITOR // for compatibility with Descent 3 v1.2 and earlier
		WriteOverrideSoundChunk(ofile);
		WriteFFTMChunk(ofile);
#endif

		if (f_save_room_AABB)
		{
			WriteRoomAABBChunk(ofile);
		}

		// Matcen Information
		WriteMatcenChunk(ofile);

		// Level Goal Information
		WriteLevelGoalsChunk(ofile);

		// Ambient life Info
		WriteALifeChunk(ofile);

		// Write level info
		chunk_start_pos = StartChunk(ofile, CHUNK_LEVEL_INFO);
		cf_WriteString(ofile, Level_info.name);
		cf_WriteString(ofile, Level_info.designer);
		cf_WriteString(ofile, Level_info.copyright);
		cf_WriteString(ofile, Level_info.notes);
		cf_WriteFloat(ofile, Gravity_strength);
#ifndef NEWEDITOR // for compatibility with Descent 3 v1.2 and earlier
		cf_WriteInt(ofile, (int)Level_powerups_ignore_wind);
#endif
		cf_WriteInt(ofile, (int)FVI_always_check_ceiling);
		cf_WriteFloat(ofile, Ceiling_height);
		EndChunk(ofile, chunk_start_pos);

		//
		// Write editor info.  All editor info should be at the end of the file
		//

		//Start chunk
		chunk_start_pos = StartChunk(ofile, CHUNK_EDITOR_INFO);

		//Save current and marked rooms
		cf_WriteShort(ofile, ROOMNUM(Curroomp));
		cf_WriteShort(ofile, Curface);
		cf_WriteShort(ofile, Curedge);
		cf_WriteShort(ofile, Curvert);
		cf_WriteShort(ofile, Markedroomp ? ROOMNUM(Markedroomp) : -1);
		cf_WriteShort(ofile, Markedface);
		cf_WriteShort(ofile, Markededge);
		cf_WriteShort(ofile, Markedvert);

		//Save selected list
		cf_WriteInt(ofile, N_selected_rooms);
		for (i = 0; i < N_selected_rooms; i++)
			cf_WriteShort(ofile, Selected_rooms[i]);		//save as shorts to save disk space

		//Save several other variables
		cf_WriteInt(ofile, Cur_object_index);
		cf_WriteInt(ofile, Current_trigger);
		cf_WriteInt(ofile, Editor_view_mode);
		cf_WriteInt(ofile, Editor_viewer_id);

		//Save wireframe view info
		cf_WriteVector(ofile, &Wireframe_view_mine.target);
		cf_WriteMatrix(ofile, &Wireframe_view_mine.orient);
		cf_WriteFloat(ofile, Wireframe_view_mine.dist);

		// write out room multiplier
		for (i = 0; i < MAX_ROOMS; i++)
		{
			cf_WriteFloat(ofile, Room_multiplier[i]);
			cf_WriteFloat(ofile, Room_ambience_r[i]);
			cf_WriteFloat(ofile, Room_ambience_g[i]);
			cf_WriteFloat(ofile, Room_ambience_b[i]);
		}

		// Write lighting spacing
		cf_WriteInt(ofile, LightSpacing);
		cf_WriteFloat(ofile, GlobalMultiplier);
		cf_WriteFloat(ofile, Ambient_red);
		cf_WriteFloat(ofile, Ambient_green);
		cf_WriteFloat(ofile, Ambient_blue);
		cf_WriteInt(ofile, rad_MaxStep);

		//Close chunk
		EndChunk(ofile, chunk_start_pos);

	}
	catch (cfile_error* cfe) {
		mprintf((0, "Error writing: file = <%s>, msg = \"%s\"\n", cfe->file->name, cfe->msg));
		ASSERT(cfe->read_write == CFE_WRITING);
		EditorMessageBox("Error writing file %s: %s", cfe->file->name, cfe->msg);
		cfclose(ofile);
		return 0;
	}

	//Close the file
	cfclose(ofile);

	return 1;

}

#endif //ifdef EDITOR

#ifndef NEWEDITOR

int need_to_page_in = 0;
int need_to_page_num = 0;
void AlmostPageInGeneric(int id);

ubyte texture_counted[MAX_BITMAPS];
ubyte sound_counted[MAX_SOUNDS];
ubyte poly_counted[MAX_POLY_MODELS];


void AlmostPageInLevelTexture(int id)
{
	if (id == -1 || id == 0)
		return;

	if ((texture_counted[id] == 0) && (GameBitmaps[id].flags & BF_NOT_RESIDENT))
	{
		texture_counted[id] = 1;
		need_to_page_num++;
		/*
		//Get the size and add it to the count
		CFILE * infile = cfopen (GameBitmaps[id].name,"rb");
		//Get the size and add it to the count
		if(infile)
		{
			need_to_page_in += cfilelength(infile);
			cfclose(infile);
		}
		*/
		if (GameTextures[id].flags & TF_DESTROYABLE && GameTextures[id].destroy_handle != -1)
			AlmostPageInLevelTexture(GameTextures[id].destroy_handle);
	}

}


void AlmostPageInSound(int id)
{
	if (id == -1)
		return;

	// Check if the sample data is already loaded
	if (SoundFiles[id].sample_16bit != NULL || SoundFiles[id].sample_8bit != NULL || sound_counted[id])
		return;
	sound_counted[id] = 1;
	need_to_page_num++;
	/*
	//Check for space this sound takes up, and add it to our counter
	CFILE * infile = cfopen (SoundFiles[id].name,"rb");
	//Get the size and add it to the count
	if(infile)
	{
		need_to_page_in += cfilelength(infile);
		cfclose(infile);
	}
	*/
}

void AlmostPageInPolymodel(int num)
{
	if ((poly_counted[num] == 0) && (Poly_models[num].flags & PMF_NOT_RESIDENT))
	{
		poly_counted[num] = 1;
		need_to_page_num++;
		/*
		CFILE * infile = cfopen (Poly_models[num].name,"rb");
		//Get the size and add it to the count
		if(infile)
		{
			need_to_page_in += cfilelength(infile);
			cfclose(infile);
		}
		*/
	}
}


void AlmostPageInDoor(int id)
{
	//Set sounds
	door* doorpointer = &Doors[id];

	AlmostPageInPolymodel(doorpointer->model_handle);

	poly_model* pm = &Poly_models[doorpointer->model_handle];
	for (int t = 0; t < pm->n_textures; t++)
		AlmostPageInLevelTexture(pm->textures[t]);

	if (doorpointer->open_sound != -1 && doorpointer->open_sound != SOUND_NONE_INDEX)
		AlmostPageInSound(doorpointer->open_sound);
	if (doorpointer->close_sound != -1 && doorpointer->close_sound != SOUND_NONE_INDEX)
		AlmostPageInSound(doorpointer->close_sound);
}

void AlmostPageInWeapon(int id)
{
	weapon* weaponpointer = &Weapons[id];

	if (id == -1)
		return;

	if (!(weaponpointer->flags & (WF_IMAGE_BITMAP | WF_IMAGE_VCLIP)))
	{
		AlmostPageInPolymodel(weaponpointer->fire_image_handle);

		poly_model* pm = &Poly_models[weaponpointer->fire_image_handle];
		for (int t = 0; t < pm->n_textures; t++)
			AlmostPageInLevelTexture(pm->textures[t]);
	}

	// Load the various textures associated with this weapon
	if (weaponpointer->explode_image_handle != -1)
	{
		AlmostPageInLevelTexture(weaponpointer->explode_image_handle);
	}

	if (weaponpointer->particle_handle != -1)
	{
		AlmostPageInLevelTexture(weaponpointer->particle_handle);
	}

	if (weaponpointer->smoke_handle != -1)
	{
		AlmostPageInLevelTexture(weaponpointer->smoke_handle);
	}

	if (weaponpointer->scorch_handle != -1)
	{
		AlmostPageInLevelTexture(weaponpointer->scorch_handle);
	}

	if (weaponpointer->icon_handle != -1)
	{
		AlmostPageInLevelTexture(weaponpointer->icon_handle);
	}

	// Try to load spawn weapons
	if (weaponpointer->spawn_handle != -1 && weaponpointer->spawn_count > 0 && weaponpointer->spawn_handle != id)
	{
		AlmostPageInWeapon(weaponpointer->spawn_handle);
	}

	if (weaponpointer->alternate_spawn_handle != -1 && weaponpointer->spawn_count > 0 && weaponpointer->alternate_spawn_handle != id)
	{
		AlmostPageInWeapon(weaponpointer->alternate_spawn_handle);
	}

	if (weaponpointer->robot_spawn_handle != -1)
	{
		AlmostPageInGeneric(weaponpointer->robot_spawn_handle);
	}

	// Try and load the various sounds
	for (int i = 0; i < MAX_WEAPON_SOUNDS; i++)
	{
		if (weaponpointer->sounds[i] != SOUND_NONE_INDEX)
		{
			AlmostPageInSound(weaponpointer->sounds[i]);
		}
	}
}

void AlmostPageInShip(int id)
{
	ship* shippointer = &Ships[id];

	// Page in all textures for this object

	AlmostPageInPolymodel(shippointer->model_handle);


	poly_model* pm = &Poly_models[shippointer->model_handle];

	for (int t = 0; t < pm->n_textures; t++)
		AlmostPageInLevelTexture(pm->textures[t]);

	if (shippointer->med_render_handle != -1)
	{
		AlmostPageInPolymodel(shippointer->med_render_handle);

		pm = &Poly_models[shippointer->med_render_handle];
		for (int t = 0; t < pm->n_textures; t++)
			AlmostPageInLevelTexture(pm->textures[t]);
	}

	if (shippointer->lo_render_handle != -1)
	{
		AlmostPageInPolymodel(shippointer->lo_render_handle);

		pm = &Poly_models[shippointer->lo_render_handle];
		for (int t = 0; t < pm->n_textures; t++)
			AlmostPageInLevelTexture(pm->textures[t]);
	}

	if (shippointer->dying_model_handle != -1)
	{
		AlmostPageInPolymodel(shippointer->dying_model_handle);

		pm = &Poly_models[shippointer->dying_model_handle];
		for (int t = 0; t < pm->n_textures; t++)
			AlmostPageInLevelTexture(pm->textures[t]);
	}

	// Try and load the various weapons
	for (int i = 0; i < MAX_PLAYER_WEAPONS; i++)
	{
		for (int j = 0; j < MAX_WB_GUNPOINTS; j++)
		{
			if (shippointer->static_wb[i].gp_weapon_index[j] != LASER_INDEX)
			{
				AlmostPageInWeapon(shippointer->static_wb[i].gp_weapon_index[j]);
			}
		}
	}

	// Try and load the various weapons
	for (int i = 0; i < MAX_PLAYER_WEAPONS; i++)
	{
		for (int j = 0; j < MAX_WB_FIRING_MASKS; j++)
		{
			if (shippointer->static_wb[i].fm_fire_sound_index[j] != SOUND_NONE_INDEX)
				AlmostPageInSound(shippointer->static_wb[i].fm_fire_sound_index[j]);
		}
	}

	for (int i = 0; i < MAX_PLAYER_WEAPONS; i++)
	{
		if (shippointer->firing_sound[i] != -1)
			AlmostPageInSound(shippointer->firing_sound[i]);

		if (shippointer->firing_release_sound[i] != -1)
			AlmostPageInSound(shippointer->firing_release_sound[i]);

		if (shippointer->spew_powerup[i] != -1)
			AlmostPageInGeneric(shippointer->spew_powerup[i]);
	}
}

void AlmostPageInGeneric(int id)
{
	if (id == -1)
		return;

	object_info* objinfopointer = &Object_info[id];

	// Page in all textures for this object

	AlmostPageInPolymodel(objinfopointer->render_handle);

	poly_model* pm = &Poly_models[objinfopointer->render_handle];

	for (int t = 0; t < pm->n_textures; t++)
		AlmostPageInLevelTexture(pm->textures[t]);

	if (objinfopointer->med_render_handle != -1)
	{
		AlmostPageInPolymodel(objinfopointer->med_render_handle);

		pm = &Poly_models[objinfopointer->med_render_handle];
		for (int t = 0; t < pm->n_textures; t++)
			AlmostPageInLevelTexture(pm->textures[t]);
	}

	if (objinfopointer->lo_render_handle != -1)
	{
		AlmostPageInPolymodel(objinfopointer->lo_render_handle);

		pm = &Poly_models[objinfopointer->lo_render_handle];
		for (int t = 0; t < pm->n_textures; t++)
			AlmostPageInLevelTexture(pm->textures[t]);
	}

	// Process all sounds for this object
	for (int i = 0; i < MAX_OBJ_SOUNDS; i++)
	{
		if (objinfopointer->sounds[i] != SOUND_NONE_INDEX)
		{
			AlmostPageInSound(objinfopointer->sounds[i]);
		}
	}

	if (objinfopointer->ai_info) 
	{
		for (int i = 0; i < MAX_AI_SOUNDS; i++)
		{
			if (objinfopointer->ai_info->sound[i] != SOUND_NONE_INDEX)
			{
				AlmostPageInSound(objinfopointer->ai_info->sound[i]);
			}
		}
	}

	// Try and load the various wb sounds
	if (objinfopointer->static_wb)
	{
		for (int i = 0; i < MAX_WBS_PER_OBJ; i++)
		{
			for (int j = 0; j < MAX_WB_FIRING_MASKS; j++)
			{
				if (objinfopointer->static_wb[i].fm_fire_sound_index[j] != SOUND_NONE_INDEX)
				{
					AlmostPageInSound(objinfopointer->static_wb[i].fm_fire_sound_index[j]);
				}
			}
		}
	}

	// Try and load the various wb sounds
	if (objinfopointer->anim) 
	{
		for (int i = 0; i < NUM_MOVEMENT_CLASSES; i++)
		{
			for (int j = 0; j < NUM_ANIMS_PER_CLASS; j++)
			{
				if (objinfopointer->anim[i].elem[j].anim_sound_index != SOUND_NONE_INDEX)
				{
					AlmostPageInSound(objinfopointer->anim[i].elem[j].anim_sound_index);
				}
			}
		}
	}

	// Load the spew types
	for (int i = 0; i < MAX_DSPEW_TYPES; i++)
	{
		if (objinfopointer->dspew_number[i] > 0 && objinfopointer->dspew[i] != 0 && objinfopointer->dspew[i] != id)
		{
			AlmostPageInGeneric(objinfopointer->dspew[i]);
		}
	}

	// Try and load the various weapons

	if (objinfopointer->static_wb) 
	{
		// Automatically include laser
		AlmostPageInWeapon(LASER_INDEX);

		for (int i = 0; i < MAX_WBS_PER_OBJ; i++)
		{
			for (int j = 0; j < MAX_WB_GUNPOINTS; j++)
			{
				if (objinfopointer->static_wb[i].gp_weapon_index[j] != LASER_INDEX)
				{
					AlmostPageInWeapon(objinfopointer->static_wb[i].gp_weapon_index[j]);
				}
			}
		}
	}
}

extern char* Static_sound_names[];
void AlmostPageInAllData()
{
	AlmostPageInShip(Players[Player_num].ship_index);
	AlmostPageInLevelTexture(FindTextureName("LightFlareStar"));
	AlmostPageInLevelTexture(FindTextureName("LightFlare"));

	//mprintf((0,"%d bytes to page in for the ship.\n",need_to_page_in));
		// Get static fireballs
	for (int i = 0; i < NUM_FIREBALLS; i++)
	{
		char name[PAGENAME_LEN];
		strcpy(name, Fireballs[i].name);

		name[strlen(name) - 4] = 0;
		int id = IGNORE_TABLE(FindTextureName(name));
		if (id != -1)
			AlmostPageInLevelTexture(id);
	}
	//mprintf((0,"%d bytes to page in for fireballs.\n",need_to_page_in));
		// Get static sounds
	for (int i = 0; i < NUM_STATIC_SOUNDS; i++)
	{
		int sid = FindSoundName(IGNORE_TABLE(Static_sound_names[i]));

		if (sid != -1)
			AlmostPageInSound(sid);
	}
	//mprintf((0,"%d bytes to page in for static sounds.\n",need_to_page_in));
		// First get textures	
	for (int i = 0; i <= Highest_room_index; i++)
	{
		if (!Rooms[i].used)
			continue;

		room* rp = &Rooms[i];
		for (int t = 0; t < rp->num_faces; t++)
		{
			AlmostPageInLevelTexture(rp->faces[t].tmap);
		}
	}
	//mprintf((0,"%d bytes to page in for room textures.\n",need_to_page_in));
		// Touch all terrain textures
	for (int i = 0; i < TERRAIN_TEX_WIDTH * TERRAIN_TEX_DEPTH; i++)
	{
		AlmostPageInLevelTexture(Terrain_tex_seg[i].tex_index);
	}

	if (Terrain_sky.textured)
	{
		AlmostPageInLevelTexture(Terrain_sky.dome_texture);
	}

	for (int i = 0; i < Terrain_sky.num_satellites; i++)
		AlmostPageInLevelTexture(Terrain_sky.satellite_texture[i]);
	//mprintf((0,"%d bytes to page in for the terrain.\n",need_to_page_in));	
		// Touch all objects
	for (int i = 0; i <= Highest_object_index; i++)
	{
		object* obj = &Objects[i];

		if (obj->type == OBJ_POWERUP || obj->type == OBJ_ROBOT || obj->type == OBJ_CLUTTER || obj->type == OBJ_BUILDING)
		{
			AlmostPageInGeneric(obj->id);
			continue;
		}

		if (obj->type == OBJ_DOOR)
		{
			AlmostPageInDoor(obj->id);
			continue;
		}
	}
	//mprintf((0,"%d bytes to page in for objects.\n",need_to_page_in));
}


//Go through all the data needing to be paged in, add it all up.
int CountDataToPageIn()
{
	need_to_page_num = 0;
	need_to_page_in = 0;

	for (int i = 0; i < MAX_BITMAPS; i++)
	{
		texture_counted[i] = 0;
	}
	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		sound_counted[i] = 0;
	}
	for (int i = 0; i < MAX_POLY_MODELS; i++)
	{
		poly_counted[i] = 0;
	}

	AlmostPageInAllData();

	return need_to_page_in;
}

#endif

void Localization_SetLanguage(int type);

int Localization_GetLanguage(void);


char* LocalizeLevelName(char* level)
{
	static char local_name[101];

	char** english_names;
	int num_english_names;

	char** local_names;
	int num_local_names;

	local_name[0] = 0;

	//Don't waste our time if we're already in english
	int save_lang = Localization_GetLanguage();
	if (LANGUAGE_ENGLISH == save_lang)
	{
		strcpy(local_name, level);
		return local_name;
	}

	Localization_SetLanguage(LANGUAGE_ENGLISH);
	//Save the current language, then bash it to english
	if (!CreateStringTable("level_names.str", &english_names, &num_english_names))
	{
		mprintf((0, "Couldn't open level_names stringtable!\n"));
		Localization_SetLanguage(save_lang);
		strcpy(local_name, level);
		return local_name;
	}

	//Restore the correct language
	Localization_SetLanguage(save_lang);

	if (!CreateStringTable("level_names.str", &local_names, &num_local_names))
	{
		mprintf((0, "Couldn't open level_names stringtable!\n"));
		//destroy the english stringtable... 
		DestroyStringTable(english_names, num_english_names);

		strcpy(local_name, level);
		return local_name;
	}

	//Now search for the correct level
	for (int i = 0; i < num_english_names; i++)
	{
		if (0 == strcmpi(level, english_names[i]))
		{
			//Ok, we found a match. So return the local text.
			strcpy(local_name, local_names[i]);
			DestroyStringTable(english_names, num_english_names);
			DestroyStringTable(local_names, num_local_names);
			return local_name;
		}
	}

	//We didn't find it, so just use the english
	strcpy(local_name, level);
	return local_name;
}
