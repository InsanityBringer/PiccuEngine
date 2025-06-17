/*
 THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE PROPERTY OF OUTRAGE
 ENTERTAINMENT, INC. ("OUTRAGE").  OUTRAGE, IN DISTRIBUTING THE CODE TO
 END-USERS, AND SUBJECT TO ALL OF THE TERMS AND CONDITIONS HEREIN, GRANTS A
 ROYALTY-FREE, PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY SUCH END-USERS
 IN USING, DISPLAYING,  AND CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
 SUCH USE, DISPLAY OR CREATION IS FOR NON-COMMERCIAL, ROYALTY OR REVENUE
 FREE PURPOSES.  IN NO EVENT SHALL THE END-USER USE THE COMPUTER CODE
 CONTAINED HEREIN FOR REVENUE-BEARING PURPOSES.  THE END-USER UNDERSTANDS
 AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE SAME BY USE OF THIS FILE.
 COPYRIGHT 1996-2000 OUTRAGE ENTERTAINMENT, INC.  ALL RIGHTS RESERVED.
 */
 
#include "stdafx.h"
#include "resource.h"
#include "ProgressDialog.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "globals.h"
#include "cfile.h"
#include "ned_TableFile.h"
#include "ned_Object.h"
#include "ned_GameTexture.h"
#include "ned_Door.h"
#include "ned_Sound.h"

#include "mem.h"


// Ignored Page types
// ------------------
#define PAGETYPE_UNKNOWN	0
#define PAGETYPE_WEAPON		2
#define PAGETYPE_ROBOT		3
#define PAGETYPE_POWERUP	4
#define PAGETYPE_SHIP		6
#define PAGETYPE_MEGACELL	8
#define PAGETYPE_GAMEFILE	9

// General Defines carried from manage.h
// -------------------------------------
// for generic pages
#define MAX_MODULENAME_LEN		32
#define MAX_DSPEW_TYPES			2
#define NUM_MOVEMENT_CLASSES	5
#define NUM_ANIMS_PER_CLASS		24
#define MAX_WBS_PER_OBJ			21 
#define MAX_WB_GUNPOINTS		8
#define MAX_OBJ_SOUNDS			2
#define MAX_AI_SOUNDS			5
#define MAX_WB_FIRING_MASKS		8
// for texture pages
#define TF_PROCEDURAL			(1<<24)	

// ========================================================================

int ntbl_working_table_file = -1;
tTableFileInfo Ntbl_loaded_table_files[MAX_LOADED_TABLE_FILES];

// ========================================================================

// =================
// ntbl_ReadPhysicsChunk
// =================
//
// Reads in a physics chunk from an open file
void ntbl_ReadPhysicsChunk(CFILE *file);

// =====================
// ntbl_ReadLightingChunk
// =====================
//
// Reads a lighting chunk in from an open file
void ntbl_ReadLightingChunk(CFILE *file,ned_object_info *oi);

// ====================
// ntbl_ReadWeaponBatteryChunk
// ====================
//
// Reads in weapon battery info
void ntbl_ReadWeaponBatteryChunk(CFILE *infile,ned_object_info *oi);

// =========================
// ntbl_ReadGenericPage
// =========================
//
//	Reads a generic page into the data structure
int ntbl_ReadGenericPage(CFILE *file,char *read_only_for_name=NULL);

// =========================
// ntbl_ReadTexturePage
// =========================
//
//	Reads a texture page into the data structure
int ntbl_ReadTexturePage(CFILE *file,char *read_only_for_name=NULL);

// ===================
// ntbl_ReadDoorPage
// ===================
//
// Reads a door page from an open file.  Returns 0 on error.  
int ntbl_ReadNewDoorPage(CFILE *file,char *read_only_for_name=NULL);

// ===================
// ntbl_ReadSoundPage
// ===================
//
// Reads a sound page from an open file.  Returns 0 on error.  
int ntbl_ReadNewSoundPage(CFILE *file,char *read_only_for_name=NULL);

// ========================
// ntbl_DoLoadProgressDialog
// ========================
//
// Displays/Updates the Load Progress bar
// state = 0 //create
// state = 1 //update
// state = 2 //destroy
void ntbl_DoLoadProgressDialog(float percentage,int state);

// ========================
// ntbl_DoRemoveProgressDialog
// ========================
//
// Displays/Updates the Remove Progress bar
// state = 0 //create
// state = 1 //update
// state = 2 //destroy
void ntbl_DoRemoveProgressDialog(int state,int count,int total);

// ======================
// ntbl_RemoveFromTableStack
// ======================
//
// Removes the instance of a table file id from a table stack
void ntbl_RemoveFromTableStack(int *table_stack,int id);

// ========================
// ntbl_IsOnTableStack
// ========================
//
// Searches through the table stack and looks for a table file id, returns true if there is one
bool ntbl_IsOnTableStack(int *table_stack,int id);


char *PAGETYPE_NAME(int pagetype)
{
	static char buf[32];
	switch(pagetype)
	{
	case PAGETYPE_SOUND:
		strcpy(buf,"Sound");
		break;
	case PAGETYPE_TEXTURE:
		strcpy(buf,"Texture");
		break;
	case PAGETYPE_DOOR:
		strcpy(buf,"Door");
		break;
	case PAGETYPE_GENERIC:
		strcpy(buf,"Generic");
		break;
	default:
		strcpy(buf,"Unknown");
		break;
	}
	return buf;
}

// =========================
// ntbl_Initialize
// =========================
//
// Initializes the manage system
void ntbl_Initialize(void)
{
	int i,j;

	memset(GameTextures,0,sizeof(ned_texture_info)*MAX_TEXTURES);
	for(i=0;i<MAX_TEXTURES;i++)
	for(j=0;j<MAX_LOADED_TABLE_FILES;j++)
		GameTextures[i].table_stack[j] = -1;

	memset(Object_info,0,sizeof(ned_object_info)*MAX_OBJECT_IDS);
	for(i=0;i<MAX_OBJECT_IDS;i++)
	for(j=0;j<MAX_LOADED_TABLE_FILES;j++)
		Object_info[i].table_stack[j] = -1;

	memset(Doors,0,sizeof(ned_door_info)*MAX_DOORS);
	for(i=0;i<MAX_DOORS;i++)
	for(j=0;j<MAX_LOADED_TABLE_FILES;j++)
		Doors[i].table_stack[j] = -1;

	memset(Sounds,0,sizeof(ned_sound_info)*MAX_SOUNDS);
	for(i=0;i<MAX_SOUNDS;i++)
	for(j=0;j<MAX_LOADED_TABLE_FILES;j++)
		Sounds[i].table_stack[j] = -1;

	ntbl_working_table_file = -1;

	for(i=0;i<MAX_LOADED_TABLE_FILES;i++)
	{
		Ntbl_loaded_table_files[i].used = false;
		Ntbl_loaded_table_files[i].count = 0;
		Ntbl_loaded_table_files[i].identifier[0] = '\0';
		Ntbl_loaded_table_files[i].type = -1;
	}
}

// ==========================
// ntbl_LoadTableFile
// ==========================
//
// Parses a table file and keeps it in memory for use
int ntbl_LoadTableFile(char *filename)
{
	char *identifier = filename;

	int i;
	int free_index = -1;
	for(i=0;i<MAX_LOADED_TABLE_FILES;i++)
	{
		if(	Ntbl_loaded_table_files[i].used )
		{
			if(!strnicmp(identifier,Ntbl_loaded_table_files[i].identifier,MAX_IDENTIFIER_NAME-1))
				return NTBL_TABLE_IN_MEM;
		}else
		{
			if(free_index==-1)
				free_index = i;
		}
	}

	if(free_index==-1)
		return NTBL_NO_AVAIL_TABLE_SLOTS;
	
	// Mark table file to be setup
	Ntbl_loaded_table_files[free_index].used = true;
	Ntbl_loaded_table_files[free_index].count = 0;
	strncpy(Ntbl_loaded_table_files[free_index].identifier,identifier,MAX_IDENTIFIER_NAME-1);
	Ntbl_loaded_table_files[free_index].identifier[MAX_IDENTIFIER_NAME-1] = '\0';	

	CFILE *file = cfopen(filename,"rb");
	if(!file)
	{
		mprintf((0,"NTBL: Unable to open '%s' for parsing\n",filename));
		Ntbl_loaded_table_files[free_index].used = false;
		ntbl_working_table_file = -1;
		return NTBL_NOT_FOUND;
	}
	mprintf((0,"NTBL: Opening '%s' for parsing\n",filename));

	ubyte pagetype;
	int n_pages = 0;
	int ret;
	int length;
	int lastpage = -1;

	cfseek(file,0,SEEK_END);
	float table_file_size = (float)cftell(file);
	float pos;
	cfseek(file,0,SEEK_SET);

	int next_chunk;

	ntbl_DoLoadProgressDialog(0,0);
	ntbl_working_table_file = free_index;

	try{

		// start reading in
		while (!cfeof(file))
		{
			pagetype=cf_ReadByte(file);
			length  =cf_ReadInt(file);
			next_chunk = cftell(file) + length - sizeof(int);

			switch (pagetype)
			{
				case PAGETYPE_TEXTURE:				

					ret = ntbl_ReadTexturePage(file);
					switch(ret)
					{
					case 0:	//invalid read
						{
							ntbl_DoLoadProgressDialog(0,2);
							ntbl_working_table_file = -1;
							cfclose(file);
							ntbl_DeleteTableFilePages(filename);
							return NTBL_CORRUPT;
						}break;
					case 1: //page read
						break;
					case -2: //page too old
						{
							cfseek(file,next_chunk,SEEK_SET);
						}break;
					default:
						Int3();
					}
					break;
				
				case PAGETYPE_GENERIC:
 					ret = ntbl_ReadGenericPage(file);
					switch(ret)
					{
					case 0: //invalid read
						{
							ntbl_DoLoadProgressDialog(0,2);
							ntbl_working_table_file = -1;
							cfclose(file);
							ntbl_DeleteTableFilePages(filename);
							return NTBL_CORRUPT;
						}break;
					case 1: //page read
						break;
					case -2: //page too old
						{
							cfseek(file,next_chunk,SEEK_SET);
						}break;
					default:
						Int3();
					}break;
				
				case PAGETYPE_DOOR:
					ret = ntbl_ReadNewDoorPage(file);
					switch(ret)
					{
					case 0: //invalid read
						{
							ntbl_DoLoadProgressDialog(0,2);
							ntbl_working_table_file = -1;
							cfclose(file);
							ntbl_DeleteTableFilePages(filename);
							return NTBL_CORRUPT;
						}break;
					case 1: //page read
						break;
					case -2: //page too old
						{
							cfseek(file,next_chunk,SEEK_SET);
						}break;
					default:
						Int3();
					}break;

				case PAGETYPE_SOUND:
					ret = ntbl_ReadNewSoundPage(file);
					switch(ret)
					{
					case 0: //invalid read
						{
							ntbl_DoLoadProgressDialog(0,2);
							ntbl_working_table_file = -1;
							cfclose(file);
							ntbl_DeleteTableFilePages(filename);
							return NTBL_CORRUPT;
						}
					case 1: //page read
						break;
					case -2: //page too old
						{
							cfseek(file,next_chunk,SEEK_SET);
						}break;
					default:
						Int3();
					}break;

				case PAGETYPE_GAMEFILE:			
				case PAGETYPE_SHIP:
				case PAGETYPE_WEAPON:
				case PAGETYPE_MEGACELL:
				case PAGETYPE_UNKNOWN:
					cfseek(file,length-sizeof(int),SEEK_CUR);
					break;
				default:
					Int3(); // Unrecognized pagetype, possible corrupt data following
					cfclose(file);
					ntbl_DoLoadProgressDialog(0,2);
					ntbl_working_table_file = -1;
					ntbl_DeleteTableFilePages(filename);
					return NTBL_CORRUPT;
					break;
			}

			n_pages++;
			lastpage = pagetype;

			pos = (float)cftell(file);

			ntbl_DoLoadProgressDialog(pos/table_file_size,1);
		}
	}catch(...)
	{
		mprintf((0,"NTBL: Exception\n"));
		cfclose(file);
		ntbl_DoLoadProgressDialog(0,2);
		ntbl_DeleteTableFilePages(filename);
		Ntbl_loaded_table_files[free_index].used = false;
		ntbl_working_table_file = -1;
		return NTBL_FATAL;
	}

	mprintf((0,"NTBL: %d pages parsed\n",n_pages));

	cfclose(file);
	ntbl_DoLoadProgressDialog(0,2);
	ntbl_working_table_file = -1;
	return free_index;
}

// ============================
// ntbl_DeleteTableFilePages
// ============================
//
// deletes all the pages associated with a table file, will revert stacked pages
void ntbl_DeleteTableFilePages(char *filename)
{
	int index;
	for(index = 0;index<MAX_LOADED_TABLE_FILES;index++)
	{
		if(Ntbl_loaded_table_files[index].used && !stricmp(Ntbl_loaded_table_files[index].identifier,filename))
		{
			break;
		}
	}
	if(index>=MAX_LOADED_TABLE_FILES)
	{
		mprintf((0,"NTBL: Unable to find table file to remove pages (%s)\n",filename));
		Int3();
		return;
	}

	int total_count = 0,count = 0,i;

	//count up how many we have to delete
	for(i=0;i<MAX_OBJECT_IDS;i++)
	{
		if(Object_info[i].used)
		{
			if(Object_info[i].table_file_id==index)
				total_count++;
			else if(ntbl_IsOnTableStack(Object_info[i].table_stack,index))
				total_count++;
		}
	}
	for(i=0;i<MAX_TEXTURES;i++)
	{
		if(GameTextures[i].used)
		{
			if(GameTextures[i].table_file_id==index)
				total_count++;
			else if(ntbl_IsOnTableStack(GameTextures[i].table_stack,index))
				total_count++;
		}
	}
	for(i=0;i<MAX_DOORS;i++)
	{
		if(Doors[i].used)
		{
			if(Doors[i].table_file_id==index)
				total_count++;
			else if(ntbl_IsOnTableStack(Doors[i].table_stack,index))
				total_count++;
		}
	}
	for(i=0;i<MAX_SOUNDS;i++)
	{
		if(Sounds[i].used)
		{
			if(Sounds[i].table_file_id==index)
				total_count++;
			else if(ntbl_IsOnTableStack(Sounds[i].table_stack,index))
				total_count++;
		}
	}

	//now remove all of them
	ntbl_DoRemoveProgressDialog(0,0,0);
	bool update;

	for(i=0;i<MAX_OBJECT_IDS;i++)
	{
		update = false;
		if(Object_info[i].used)
		{
			if(Object_info[i].table_file_id==index)
			{
				update = true;
				count++;
				ned_FreeObjectInfo(i,false);
			}
			else if(ntbl_IsOnTableStack(Object_info[i].table_stack,index))
			{
				update = true;
				count++;
				ntbl_RemoveFromTableStack(Object_info[i].table_stack,index);
			}

			if(update)
				ntbl_DoRemoveProgressDialog(1,count,total_count);
		}
	}
	for(i=0;i<MAX_TEXTURES;i++)
	{
		update = false;
		if(GameTextures[i].used)
		{
			if(GameTextures[i].table_file_id==index)
			{
				update = true;
				count++;
				ned_FreeTexture(i,false);
			}
			else if(ntbl_IsOnTableStack(GameTextures[i].table_stack,index))
			{
				update = true;
				count++;
				ntbl_RemoveFromTableStack(GameTextures[i].table_stack,index);
			}

			if(update)
				ntbl_DoRemoveProgressDialog(1,count,total_count);
		}
	}
	for(i=0;i<MAX_DOORS;i++)
	{
		update = false;
		if(Doors[i].used)
		{
			if(Doors[i].table_file_id==index)
			{
				update = true;
				count++;
				ned_FreeDoor(i,false);
			}
			else if(ntbl_IsOnTableStack(Doors[i].table_stack,index))
			{
				update = true;
				count++;
				ntbl_RemoveFromTableStack(Doors[i].table_stack,index);
			}

			if(update)
				ntbl_DoRemoveProgressDialog(1,count,total_count);
		}
	}
	for(i=0;i<MAX_SOUNDS;i++)
	{
		update = false;
		if(Sounds[i].used)
		{
			if(Sounds[i].table_file_id==index)
			{
				update = true;
				count++;
				ned_FreeSound(i,false);
			}
			else if(ntbl_IsOnTableStack(Sounds[i].table_stack,index))
			{
				update = true;
				count++;
				ntbl_RemoveFromTableStack(Sounds[i].table_stack,index);
			}

			if(update)
				ntbl_DoRemoveProgressDialog(1,count,total_count);
		}
	}	
	ntbl_DoRemoveProgressDialog(2,count,total_count);
}

void ntbl_WriteData(CFILE *file,int pagetype,int slot)
{
	char buffer[1024];
	int *table_stack;
	char *name,*tid;

	switch(pagetype)
	{
	case PAGETYPE_GENERIC:
		ASSERT(slot>=0 && slot<MAX_OBJECT_IDS);
		ASSERT(Object_info[slot].used);

		if(Object_info[slot].name[0]=='\0')
			return;
		if(Object_info[slot].table_file_id==-1)
			tid = NULL;
		else
			tid = Ntbl_loaded_table_files[Object_info[slot].table_file_id].identifier;
		table_stack = Object_info[slot].table_stack;
		break;		
	case PAGETYPE_TEXTURE:
		ASSERT(slot>=0 && slot<MAX_TEXTURES);
		ASSERT(GameTextures[slot].used);

		if(GameTextures[slot].name[0]=='\0')
			return;
		if(GameTextures[slot].table_file_id==-1)
			tid = NULL;
		else
			tid = Ntbl_loaded_table_files[GameTextures[slot].table_file_id].identifier;
		table_stack = GameTextures[slot].table_stack;
		break;		
	case PAGETYPE_DOOR:
		ASSERT(slot>=0 && slot<MAX_DOORS);
		ASSERT(Doors[slot].used);

		if(Doors[slot].name[0]=='\0')
			return;
		if(Doors[slot].table_file_id==-1)
			tid = NULL;
		else
			tid = Ntbl_loaded_table_files[Doors[slot].table_file_id].identifier;
		table_stack = Doors[slot].table_stack;
		break;		
	case PAGETYPE_SOUND:
		ASSERT(slot>=0 && slot<MAX_SOUNDS);
		ASSERT(Sounds[slot].used);

		if(Sounds[slot].name[0]=='\0')
			return;
		if(Sounds[slot].table_file_id==-1)
			tid = NULL;
		else
			tid = Ntbl_loaded_table_files[Sounds[slot].table_file_id].identifier;
		table_stack = Sounds[slot].table_stack;
		break;
	default:
		Int3();
		return;
	}

	int overlay_count = 0;
	int i;
	for(i=0;i<MAX_LOADED_TABLE_FILES;i++)
	{
		if(table_stack[i]!=-1)
		{
			overlay_count++;
		}else
			break;
	}

	
	switch(pagetype)
	{
	case PAGETYPE_GENERIC:
		char type[100];
		switch(Object_info[slot].type)
		{
		case OBJ_BUILDING:
			strcpy(type,"OBJ_BUILDING");
			break;
		case OBJ_ROBOT:
			strcpy(type,"OBJ_ROBOT");
			break;
		case OBJ_CLUTTER:
			strcpy(type,"OBJ_CLUTTER");
			break;
		case OBJ_POWERUP:
			strcpy(type,"OBJ_POWERUP");
			break;
		default:
			Int3();
			strcpy(type,"!OBJ_UNKNOWN!");
			break;
		}
		sprintf(buffer,"%s(%s):%d (%s) [%s(%d)]",PAGETYPE_NAME(pagetype),type,slot,name,tid,overlay_count);
		break;
	case PAGETYPE_TEXTURE:
	case PAGETYPE_DOOR:
	case PAGETYPE_SOUND:		
		sprintf(buffer,"%s:%d (%s) [%s(%d)]",PAGETYPE_NAME(pagetype),slot,name,tid,overlay_count);
		break;
	default:
		Int3();
		strcpy(buffer,"Unknown");
		break;
	}
	
	cf_WriteString(file,buffer);
}

// ========================
// ntbl_DumpToFile
// ========================
//
//	Dumps all the current table file information to file
bool ntbl_DumpToFile(char *filename)
{
	CFILE *file;

	file = cfopen(filename,"wt");
	if(!file)
	{
		return false;
	}

	int i;

	try
	{
		for(i=0;i<MAX_OBJECT_IDS;i++)
		{
			if(Object_info[i].used)
				ntbl_WriteData(file,PAGETYPE_GENERIC,i);
		}

		for(i=0;i<MAX_TEXTURES;i++)
		{
			if(GameTextures[i].used)
				ntbl_WriteData(file,PAGETYPE_TEXTURE,i);
		}

		for(i=0;i<MAX_DOORS;i++)
		{
			if(Doors[i].used)
				ntbl_WriteData(file,PAGETYPE_DOOR,i);
		}

		for(i=0;i<MAX_SOUNDS;i++)
		{
			if(Sounds[i].used)
				ntbl_WriteData(file,PAGETYPE_SOUND,i);
		}

	}catch(...)
	{
		return false;
	}

	cfclose(file);
	return true;
}


// =============================================================================

// =========================
// ntbl_ReadGenericPage
// =========================
//
//	Reads a generic page into the data structure
int ntbl_ReadGenericPage(CFILE *file,char *read_only_for_name)
{
#define KNOWN_GENERIC_VERSION 27
	char buffer[1024];
	int i,j;
	ASSERT (file!=NULL);

	int version=cf_ReadShort(file);
	if(version>KNOWN_GENERIC_VERSION)
	{
		mprintf((0,"NTBL: Generic Page version is newer than we support\n"));
		Int3();
		return -2;
	}

	if(version<KNOWN_GENERIC_VERSION)
	{
		//mprintf((0,"NTBL: Warning! Reading in an early version (%d:%d)\n",version,KNOWN_GENERIC_VERSION));
	}

	//genericpage->objinfo_struct.type
	int type = cf_ReadByte(file);

	// Read object name
	cf_ReadString(buffer,PAGENAME_LEN,file);

	//check to see if we are only supposed to read a certain page
	if(read_only_for_name && stricmp(read_only_for_name,buffer))
		return -1;

	int slot = ned_AllocObjectInfo(buffer,ntbl_working_table_file);
	ASSERT(slot!=-1);
	if(slot==-1)
		return -2;

	Object_info[slot].type = type;

	// Read model names
	//cf_ReadString (genericpage->image_name,PAGENAME_LEN,infile);
	//cf_ReadString (genericpage->med_image_name,PAGENAME_LEN,infile);
	//cf_ReadString (genericpage->lo_image_name,PAGENAME_LEN,infile);
	cf_ReadString(Object_info[slot].image_filename,PAGENAME_LEN,file);
	cf_ReadString(buffer,PAGENAME_LEN,file);
	cf_ReadString(buffer,PAGENAME_LEN,file);
	
	// Read out impact data
	//genericpage->objinfo_struct.impact_size
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.impact_time
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.damage
	cf_ReadFloat(file);

	// Read score
	//genericpage->objinfo_struct.score
	if (version >= 24)
		cf_ReadShort(file);
	else
		cf_ReadByte(file);

	// Read ammo
	if (type == OBJ_POWERUP) {
		if (version >= 25)
		{
			//genericpage->objinfo_struct.ammo_count
			cf_ReadShort(file);
		}
	}
	
	//Read script name (NO LONGER USED)
	cf_ReadString (buffer,PAGENAME_LEN,file);

	if(version>=18)
	{
		//cf_ReadString (genericpage->objinfo_struct.module_name,MAX_MODULENAME_LEN,infile);
		cf_ReadString(buffer,MAX_MODULENAME_LEN,file);
	}

	if(version>=19)
	{
		//cf_ReadString (genericpage->objinfo_struct.script_name_override,PAGENAME_LEN,infile);
		cf_ReadString(buffer,PAGENAME_LEN,file);
	}
	
	int desc=cf_ReadByte(file);
	if (desc)
	{
		// Read description if there is one
		char tempbuf[1024];
		cf_ReadString (tempbuf,1024,file);

		//int slen=strlen (tempbuf)+1;
		//genericpage->objinfo_struct.description=(char *)mem_malloc (slen);
		//ASSERT (genericpage->objinfo_struct.description);
		//strcpy (genericpage->objinfo_struct.description,tempbuf);
	}

	// Read icon name
	//cf_ReadString (genericpage->objinfo_struct.icon_name,PAGENAME_LEN,infile);
	cf_ReadString(buffer,PAGENAME_LEN,file);
		
	// Read LOD distances
	//genericpage->objinfo_struct.med_lod_distance
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.lo_lod_distance
	cf_ReadFloat(file);
	
	// Read physics stuff
	// ==============================================================
	ntbl_ReadPhysicsChunk(file);
	
	// read size
	//genericpage->objinfo_struct.size
	Object_info[slot].size = cf_ReadFloat(file);

	// read light info
	// ==============================================================
	ntbl_ReadLightingChunk(file,&Object_info[slot]);		

	// Read hit points
	//genericpage->objinfo_struct.hit_points
	Object_info[slot].hit_points = cf_ReadInt(file);

	// Read flags
	//genericpage->objinfo_struct.flags
	Object_info[slot].flags = cf_ReadInt(file);

	// Read AI info
	// ==============================================================
	//genericpage->objinfo_struct.ai_info.flags
	cf_ReadInt(file);
	//genericpage->objinfo_struct.ai_info.ai_class
	cf_ReadByte(file);
	//genericpage->objinfo_struct.ai_info.ai_type
	cf_ReadByte(file);
	//genericpage->objinfo_struct.ai_info.movement_type
	cf_ReadByte(file);
	//genericpage->objinfo_struct.ai_info.movement_subtype
	cf_ReadByte(file);
	//genericpage->objinfo_struct.ai_info.fov
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.max_velocity
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.max_delta_velocity
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.max_turn_rate
	cf_ReadFloat(file);	
	//genericpage->objinfo_struct.ai_info.notify_flags
	cf_ReadInt(file);
	//genericpage->objinfo_struct.ai_info.max_delta_turn_rate
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.circle_distance
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.attack_vel_percent
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.dodge_percent
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.dodge_vel_percent
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.flee_vel_percent
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.melee_damage[0]
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.melee_damage[1]
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.melee_latency[0]
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.melee_latency[1]
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.curiousity
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.night_vision
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.fog_vision
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.lead_accuracy
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.lead_varience
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.fire_spread
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.fight_team
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.fight_same
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.agression
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.hearing
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.frustration
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.roaming
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.life_preservation
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.avoid_friends_distance
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.biased_flight_importance
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.biased_flight_min
	cf_ReadFloat(file);
	//genericpage->objinfo_struct.ai_info.biased_flight_max
	cf_ReadFloat(file);
	
	// Read out objects spewed	
	for (i = 0; i<MAX_DSPEW_TYPES; i++)
	{
		//genericpage->objinfo_struct.f_dspew
		cf_ReadByte(file);
		//genericpage->objinfo_struct.dspew_percent[i]
		cf_ReadFloat(file);
		//genericpage->objinfo_struct.dspew_number[i]
		cf_ReadShort(file);

		// Read spew name
		//cf_ReadString(genericpage->dspew_name[i],PAGENAME_LEN,infile);
		cf_ReadString(buffer,PAGENAME_LEN,file);
	}

	// Read out animation info
	for (i=0;i<NUM_MOVEMENT_CLASSES;i++)
	{
		for(j = 0; j < NUM_ANIMS_PER_CLASS; j++)
		{
			//genericpage->objinfo_struct.anim[i][j].from
			cf_ReadShort(file);
			//genericpage->objinfo_struct.anim[i][j].to
			cf_ReadShort(file);
			//genericpage->objinfo_struct.anim[i][j].spc
			cf_ReadFloat(file);
		}
	}

	// read weapon batteries
	for(i = 0; i < MAX_WBS_PER_OBJ; i++)
	{
		ntbl_ReadWeaponBatteryChunk(file,&Object_info[slot]);
	}

	// read weapon names
	for(i = 0; i < MAX_WBS_PER_OBJ; i++)
	{
		for(j = 0; j < MAX_WB_GUNPOINTS; j++)
		{
			//cf_ReadString (genericpage->weapon_name[i][j],PAGENAME_LEN,infile);
			cf_ReadString(buffer,PAGENAME_LEN,file);
		}
	}
		
	// read sounds 
	ASSERT(MAX_OBJ_SOUNDS == 2);
	for (i=0;i<MAX_OBJ_SOUNDS;i++)
	{
		//genericpage->sound_name[i]
		cf_ReadString (buffer,PAGENAME_LEN,file);
	}
	if (version < 26) {		//used to be three sounds
		char temp_sound_name[PAGENAME_LEN];
		cf_ReadString (temp_sound_name,PAGENAME_LEN,file);
	}
		

	for (i=0;i<MAX_AI_SOUNDS;i++)
	{
		//cf_ReadString (genericpage->ai_sound_name[i],PAGENAME_LEN,infile);
		cf_ReadString(buffer,PAGENAME_LEN,file);
	}

	for(i = 0; i < MAX_WBS_PER_OBJ; i++)
	{
		for(j = 0; j < MAX_WB_FIRING_MASKS; j++)
		{
			//cf_ReadString (genericpage->fire_sound_name[i][j],PAGENAME_LEN,infile);
			cf_ReadString(buffer,PAGENAME_LEN,file);
		}
	}

	for(i = 0; i < NUM_MOVEMENT_CLASSES; i++)
	{
		for(j = 0; j < NUM_ANIMS_PER_CLASS; j++)
		{
			//cf_ReadString (genericpage->anim_sound_name[i][j],PAGENAME_LEN,infile);
			cf_ReadString(buffer,PAGENAME_LEN,file);
		}
	}

	// Read respawn scalar
	//genericpage->objinfo_struct.respawn_scalar
	cf_ReadFloat(file);

	int n_death_types = cf_ReadShort(file);
	for (i=0;i<n_death_types;i++) {		
		//genericpage->objinfo_struct.death_types[i].flags
		cf_ReadInt(file);
		//genericpage->objinfo_struct.death_types[i].delay_min
		cf_ReadFloat(file);
		//genericpage->objinfo_struct.death_types[i].delay_max
		cf_ReadFloat(file);
		//genericpage->objinfo_struct.death_probabilities[i]
		cf_ReadByte(file);
	}

	return 1;		// successfully read
}

// =================
// ntbl_ReadPhysicsChunk
// =================
//
// Reads in a physics chunk from an open file
void ntbl_ReadPhysicsChunk(CFILE *file)
{
	//phys_info->mass
	cf_ReadFloat(file);
	//phys_info->drag
	cf_ReadFloat(file);
	//phys_info->full_thrust
	cf_ReadFloat(file);
	//phys_info->flags
	cf_ReadInt(file);
	//phys_info->rotdrag
	cf_ReadFloat(file);
	//phys_info->full_rotthrust
	cf_ReadFloat(file);
	//phys_info->num_bounces
	cf_ReadInt(file);
	//phys_info->velocity.z
	cf_ReadFloat(file);
	//phys_info->rotvel.x
	cf_ReadFloat(file);
	//phys_info->rotvel.y
	cf_ReadFloat(file);
	//phys_info->rotvel.z
	cf_ReadFloat(file);
	//phys_info->wiggle_amplitude
	cf_ReadFloat(file);
	//phys_info->wiggles_per_sec
	cf_ReadFloat(file);
	//phys_info->coeff_restitution
	cf_ReadFloat(file);
	//phys_info->hit_die_dot
	cf_ReadFloat(file);
	//phys_info->max_turnroll_rate
	cf_ReadFloat(file);
	//phys_info->turnroll_ratio
	cf_ReadFloat(file);	
}

// =====================
// ntbl_ReadLightingChunk
// =====================
//
// Reads a lighting chunk in from an open file
void ntbl_ReadLightingChunk(CFILE *file,ned_object_info *oi)
{
	light_info *lighting_info;
	lighting_info = &oi->lighting_info;	

	lighting_info->light_distance		= cf_ReadFloat(file);	
	lighting_info->red_light1			= cf_ReadFloat(file);
	lighting_info->green_light1			= cf_ReadFloat(file);
	lighting_info->blue_light1			= cf_ReadFloat(file);
	lighting_info->time_interval		= cf_ReadFloat(file);
	lighting_info->flicker_distance		= cf_ReadFloat(file);
	lighting_info->directional_dot		= cf_ReadFloat(file);
	lighting_info->red_light2			= cf_ReadFloat(file);
	lighting_info->green_light2			= cf_ReadFloat(file);
	lighting_info->blue_light2			= cf_ReadFloat(file);
	lighting_info->flags				= cf_ReadInt(file);
	lighting_info->timebits				= cf_ReadInt(file);
	lighting_info->angle				= cf_ReadByte(file);
	lighting_info->lighting_render_type	= cf_ReadByte(file);
}

// ====================
// ntbl_ReadWeaponBatteryChunk
// ====================
//
// Reads in weapon battery info
void ntbl_ReadWeaponBatteryChunk(CFILE *infile,ned_object_info *oi)
{
	int j;

	otype_wb_info *static_wb;
	static_wb = oi->static_wb;

	static_wb->energy_usage =				cf_ReadFloat(infile);
	static_wb->ammo_usage =					cf_ReadFloat(infile);
	
	for(j = 0; j < MAX_WB_GUNPOINTS; j++)  
	{
		static_wb->gp_weapon_index[j] =		cf_ReadShort(infile);
	}
				
	for(j = 0; j < MAX_WB_FIRING_MASKS; j++) 
	{
		static_wb->gp_fire_masks[j] =		cf_ReadByte(infile);
		static_wb->gp_fire_wait[j] =		cf_ReadFloat(infile);
		static_wb->anim_time[j] =			cf_ReadFloat(infile);
		static_wb->anim_start_frame[j] =	cf_ReadFloat(infile);
		static_wb->anim_fire_frame[j] =		cf_ReadFloat(infile);
		static_wb->anim_end_frame[j] =		cf_ReadFloat(infile);
	}

	static_wb->num_masks =					cf_ReadByte(infile);
	static_wb->aiming_gp_index =			cf_ReadShort(infile);
	static_wb->aiming_flags =				cf_ReadByte(infile);
	static_wb->aiming_3d_dot =				cf_ReadFloat(infile);
	static_wb->aiming_3d_dist =				cf_ReadFloat(infile);
	static_wb->aiming_XZ_dot =				cf_ReadFloat(infile);
	static_wb->flags =						cf_ReadShort(infile);
	static_wb->gp_quad_fire_mask =			cf_ReadByte(infile);
}


// =========================
// ntbl_ReadTexturePage
// =========================
//
//	Reads a texture page into the data structure
int ntbl_ReadTexturePage(CFILE *file,char *read_only_for_name)
{
#define KNOWN_TEXTURE_VERSION	7
	char buffer[1024];	
	int i;
	ASSERT (file!=NULL);

	int version=cf_ReadShort (file);
	if(version>KNOWN_TEXTURE_VERSION)
	{
		mprintf((0,"NTBL: Texture Page version is newer than we support\n"));
		Int3();
		return -2;
	}

	if(version<KNOWN_TEXTURE_VERSION)
	{
		//mprintf((0,"NTBL: Warning! Reading in an early version\n"));
		//Int3();
	}

	//cf_ReadString(texpage->tex_struct.name,PAGENAME_LEN,infile);
	//cf_ReadString(texpage->bitmap_name,PAGENAME_LEN,infile);
	//cf_ReadString(texpage->destroy_name,PAGENAME_LEN,infile);
	cf_ReadString(buffer,PAGENAME_LEN,file);
		
	//check to see if we are only supposed to read a certain page
	if(read_only_for_name && stricmp(read_only_for_name,buffer))
		return -1;

	int slot;
	slot = ned_AllocTexture(buffer,ntbl_working_table_file);
	ASSERT(slot!=-1);
	if(slot==-1)
		return -2;

	cf_ReadString(GameTextures[slot].image_filename,PAGENAME_LEN,file);
	cf_ReadString(buffer,PAGENAME_LEN,file);

	//texpage->tex_struct.r
	GameTextures[slot].r = cf_ReadFloat(file);

	//texpage->tex_struct.g
	GameTextures[slot].g = cf_ReadFloat(file);

	//texpage->tex_struct.b
	GameTextures[slot].b = cf_ReadFloat(file);

	//texpage->tex_struct.alpha
	GameTextures[slot].alpha = cf_ReadFloat(file);

	//texpage->tex_struct.speed
	GameTextures[slot].speed = cf_ReadFloat(file);

	//texpage->tex_struct.slide_u
	GameTextures[slot].slide_u = cf_ReadFloat(file);
	//texpage->tex_struct.slide_v
	GameTextures[slot].slide_v = cf_ReadFloat(file);
	//texpage->tex_struct.reflectivity
	GameTextures[slot].reflectivity=cf_ReadFloat(file);
	//texpage->tex_struct.corona_type
	cf_ReadByte(file);
	//texpage->tex_struct.damage
	cf_ReadInt(file);
	//texpage->tex_struct.flags
	GameTextures[slot].flags = cf_ReadInt(file);

	if(GameTextures[slot].flags&TF_PROCEDURAL)
	{
		for (i=0;i<255;i++)
		{
			ushort val=cf_ReadShort(file);
			//texpage->proc_palette[i]=val;
		}

		//texpage->proc_heat
		cf_ReadByte(file);
		//texpage->proc_light
		cf_ReadByte(file);
		//texpage->proc_thickness
		cf_ReadByte(file);
		//texpage->proc_evaluation_time
		cf_ReadFloat(file);

		if (version>=6)
		{
			//texpage->osc_time
			cf_ReadFloat(file);
			//texpage->osc_value
			cf_ReadByte(file);
		}

		//texpage->num_proc_elements
		int num_proc_elements = cf_ReadShort(file);

		for (i=0;i<num_proc_elements;i++)
		{
			//texpage->proc_type[i]
			cf_ReadByte(file);
			//texpage->proc_frequency[i]
			cf_ReadByte(file);
			//texpage->proc_speed[i]
			cf_ReadByte(file);
			//texpage->proc_size[i]
			cf_ReadByte(file);
			//texpage->proc_x1[i]
			cf_ReadByte(file);
			//texpage->proc_y1[i]
			cf_ReadByte(file);
			//texpage->proc_x2[i]
			cf_ReadByte(file);
			//texpage->proc_y2[i]
			cf_ReadByte(file);
		}
	}

	if (version >= 5) {

		if (version<7)
		{
			// Kill buggy version of sound resolving code
			// sound id
			cf_ReadInt(file);
		}
		else
		{
			//texpage->sound_name
			cf_ReadString(buffer,PAGENAME_LEN,file);
		}

		//texpage->tex_struct.sound_volume
		cf_ReadFloat(file);
	}

	return 1;		// successfully read
}

// ===================
// ntbl_ReadDoorPage
// ===================
//
// Reads a door page from an open file.  Returns 0 on error.  
int ntbl_ReadNewDoorPage(CFILE *file,char *read_only_for_name)
{
#define KNOWN_DOOR_VERSION	3
	ASSERT (file!=NULL);
	char buffer[1024];	

	int version=cf_ReadShort (file);
	if(version>KNOWN_DOOR_VERSION)
	{
		mprintf((0,"NTBL: Door Page version is newer than we support\n"));
		Int3();
		return -2;
	}

	if(version<KNOWN_DOOR_VERSION)
	{
		//mprintf((0,"NTBL: Warning! Reading in an early version\n"));
		//Int3();
	}

	//cf_ReadString(doorpage->door_struct.name, PAGENAME_LEN, infile);
	//cf_ReadString(doorpage->image_name, PAGENAME_LEN, infile);
	cf_ReadString(buffer,PAGENAME_LEN,file);

	//check to see if we are only supposed to read a certain page
	if(read_only_for_name && stricmp(read_only_for_name,buffer))
		return -1;

	int slot = ned_AllocDoor(buffer,ntbl_working_table_file);
	ASSERT(slot!=-1);
	if(slot==-1)
		return -2;

	cf_ReadString(Doors[slot].image_filename,PAGENAME_LEN,file);

	//doorpage->door_struct.total_open_time
	cf_ReadFloat(file);
	//doorpage->door_struct.total_close_time
	cf_ReadFloat(file);
	//doorpage->door_struct.total_time_open
	cf_ReadFloat(file);
	//doorpage->door_struct.flags
	Doors[slot].flags = cf_ReadByte(file);

	if (version >= 3)
	{
		//doorpage->door_struct.hit_points
		cf_ReadShort(file);
	}

	//cf_ReadString(doorpage->open_sound_name, PAGENAME_LEN, infile);
	//cf_ReadString(doorpage->close_sound_name, PAGENAME_LEN, infile);
	cf_ReadString(buffer,PAGENAME_LEN,file);
	cf_ReadString(buffer,PAGENAME_LEN,file);

	if(version>=2)
	{
		//cf_ReadString(doorpage->door_struct.module_name, MAX_MODULENAME_LEN, infile);
		cf_ReadString(buffer,PAGENAME_LEN,file);
	}

	return 1;		// successfully read
}


// ===================
// ntbl_ReadSoundPage
// ===================
//
// Reads a sound page from an open file.  Returns 0 on error.  
int ntbl_ReadNewSoundPage(CFILE *file,char *read_only_for_name)
{
#define KNOWN_SOUND_VERSION	1
	ASSERT (file!=NULL);
	char buffer[1024];	

	int version=cf_ReadShort (file);
	if(version>KNOWN_SOUND_VERSION)
	{
		mprintf((0,"NTBL: Sound Page version is newer than we support\n"));
		Int3();
		return -2;
	}

	if(version<KNOWN_SOUND_VERSION)
	{
		//mprintf((0,"NTBL: Warning! Reading in an early version\n"));
		//Int3();
	}

	// read in name,rawfile name
	//cf_ReadString (soundpage->sound_struct.name,PAGENAME_LEN,infile);
	cf_ReadString(buffer,PAGENAME_LEN,file);

	//check to see if we are only supposed to read a certain page
	if(read_only_for_name && stricmp(read_only_for_name,buffer))
		return -1;

	int slot = ned_AllocSound(buffer,ntbl_working_table_file);
	ASSERT(slot!=-1);
	if(slot==-1)
		return -2;

	//cf_ReadString (soundpage->raw_name,PAGENAME_LEN,infile);
	cf_ReadString(Sounds[slot].raw_filename,PAGENAME_LEN,file);

	//soundpage->sound_struct.flags
	Sounds[slot].flags = cf_ReadInt(file);	
	//soundpage->sound_struct.loop_start
	cf_ReadInt(file);
	//soundpage->sound_struct.loop_end
	cf_ReadInt(file);
	//soundpage->sound_struct.outer_cone_volume
	cf_ReadFloat(file);
	//soundpage->sound_struct.inner_cone_angle
	cf_ReadInt(file);
	//soundpage->sound_struct.outer_cone_angle
	cf_ReadInt(file);
	//soundpage->sound_struct.max_distance
	cf_ReadFloat(file);
	//soundpage->sound_struct.min_distance
	cf_ReadFloat(file);
	//soundpage->sound_struct.import_volume
	cf_ReadFloat(file);

	return 1;		// successfully read
}

// ========================
// ntbl_DoLoadProgressDialog
// ========================
//
// Displays/Updates the Load Progress bar
// state = 0 //create
// state = 1 //update
// state = 2 //destroy
void ntbl_DoLoadProgressDialog(float percentage,int state)
{
	int pos = percentage*100.0f;
	if(pos<0) pos = 0;
	if(pos>100) pos = 100;

	static CProgressDialog *gProgressDialog = NULL;
	switch(state)
	{
	case 0:
		{
			gProgressDialog = new CProgressDialog;
			
			gProgressDialog->Create(IDD_LOADLEVELPROGRESS,NULL);
			gProgressDialog->m_TitleText = "Loading Tablefile...";
			gProgressDialog->ShowWindow(SW_SHOW);

			gProgressDialog->UpdateData(false);
			gProgressDialog->m_ProgressBar.SetStep(100);
			gProgressDialog->m_ProgressBar.SetPos(pos);

		}break;

	case 1:
		{
			if(gProgressDialog && gProgressDialog->m_hWnd)
			{
				gProgressDialog->m_ProgressBar.SetPos(pos);
				defer();
			}
		}break;

	case 2:
		{
			if(gProgressDialog && gProgressDialog->m_hWnd)
			{
				gProgressDialog->DestroyWindow();
				delete gProgressDialog;
				gProgressDialog = NULL;
			}

		}break;
	}
}

// ========================
// ntbl_DoRemoveProgressDialog
// ========================
//
// Displays/Updates the Remove Progress bar
// state = 0 //create
// state = 1 //update
// state = 2 //destroy
void ntbl_DoRemoveProgressDialog(int state,int count,int total)
{
	float percentage = 0.0f;

	if(state==1)
	{
		if(total>0)
		{
			percentage = (float)count/float(total);
		}
	}	

	int pos = percentage*100.0f;
	if(pos<0) pos = 0;
	if(pos>100) pos = 100;

	static CProgressDialog *gProgressDialog = NULL;
	switch(state)
	{
	case 0:
		{
			gProgressDialog = new CProgressDialog;
			
			gProgressDialog->Create(IDD_LOADLEVELPROGRESS,NULL);
			gProgressDialog->m_TitleText = "Removing and Overlaying Previous Files";
			gProgressDialog->ShowWindow(SW_SHOW);
			
			gProgressDialog->UpdateData(false);
			gProgressDialog->m_ProgressBar.SetStep(100);
			gProgressDialog->m_ProgressBar.SetPos(pos);

		}break;

	case 1:
		{
			if(gProgressDialog)
			{
				gProgressDialog->m_ProgressBar.SetPos(pos);

				char buffer[1024];
				sprintf(buffer,"Removing and Overlaying Previous Files (%d of %d)",count,total);
				gProgressDialog->m_TitleText = buffer;
				gProgressDialog->UpdateData(false);
				defer();
			}
		}break;

	case 2:
		{
			if(gProgressDialog)
			{
				gProgressDialog->DestroyWindow();
				delete gProgressDialog;
				gProgressDialog = NULL;
			}

		}break;
	}
}



// =============================
// ntbl_OverlayPage
// =============================
//
// Given the pagetype and an id, this function takes the information from table_file_id and
// reloads it's data into the page
bool ntbl_OverlayPage(int pagetype,int slot)
{
	int table_index = -1;
	char *name = NULL;

	switch(pagetype)
	{
	case PAGETYPE_GENERIC:
		table_index = Object_info[slot].table_file_id;
		name = Object_info[slot].name;
		break;
	case PAGETYPE_TEXTURE:
		table_index = GameTextures[slot].table_file_id;
		name = GameTextures[slot].name;
		break;
	case PAGETYPE_DOOR:
		table_index = Doors[slot].table_file_id;
		name = Doors[slot].name;
		break;
	case PAGETYPE_SOUND:
		table_index = Sounds[slot].table_file_id;
		name = Sounds[slot].name;
		break;
	default:
		mprintf((0,"NTBL: Invalid pagetype\n"));
		Int3();
		return false;
		break;
	}

	if(table_index<0 || table_index>=MAX_LOADED_TABLE_FILES)
	{
		mprintf((0,"NTBL: Invalid table index\n"));
		Int3();
		return false;
	}

	if(Ntbl_loaded_table_files[table_index].used==false)
	{
		mprintf((0,"NTBL: Invalid table file\n"));
		Int3();
		return false;
	}

	mprintf((0,"NTBL: Attempting to overlay %s(%s) from %s\n",name,PAGETYPE_NAME(pagetype),Ntbl_loaded_table_files[table_index].identifier));

	//now attempt to find it in the tablefile
	CFILE *file;
	file = cfopen(Ntbl_loaded_table_files[table_index].identifier,"rb");
	if(!file)
	{
		mprintf((0,"NTBL: Invalid table file given\n"));
		return false;
	}

	int ptype,length;
	bool found = false;

	int old_curr_read = ntbl_working_table_file;	
	ntbl_working_table_file = table_index;
	int next_block,ret;

	// start reading in
	while (!cfeof(file))
	{
		ptype=cf_ReadByte(file);
		length=cf_ReadInt(file);
		next_block = cftell(file) + length - sizeof(int);
		ret = -1;

		if(ptype==pagetype)
		{
			switch(pagetype)
			{
			case PAGETYPE_TEXTURE:
				ret = ntbl_ReadTexturePage(file,name);
				if(ret==0)
				{
					mprintf((0,"NTBL: Unable to read table file\n"));
					cfclose(file);
					ntbl_working_table_file = old_curr_read;
					return false;
				}
				break;
			case PAGETYPE_DOOR:
				ret = ntbl_ReadNewDoorPage(file,name);
				if(ret==0)
				{
					mprintf((0,"NTBL: Unable to read table file\n"));
					cfclose(file);
					ntbl_working_table_file = old_curr_read;
					return false;
				}
				break;
			case PAGETYPE_SOUND:
				ret = ntbl_ReadNewSoundPage(file,name);
				if(ret==0)
				{
					mprintf((0,"NTBL: Unable to read table file\n"));
					cfclose(file);
					ntbl_working_table_file = old_curr_read;
					return false;
				}
				break;
			case PAGETYPE_GENERIC:
				ret = ntbl_ReadGenericPage(file,name);
				if(ret==0)
				{
					mprintf((0,"NTBL: Unable to read table file\n"));
					cfclose(file);
					ntbl_working_table_file = old_curr_read;
					return false;
				}
				break;
			default:
				Int3();
				break;
			}

			switch(ret)
			{
			case -2: //page version too new
				mprintf((0,"NTBL: Page too new, skipping\n"));
			case -1: //not the page we want
				cfseek(file,next_block,SEEK_SET);
				break;
			case 1:
				{
					//here is the page!
					cfclose(file);
					ntbl_working_table_file = old_curr_read;
					return true;
				}break;
			default:
				Int3();
				cfseek(file,next_block,SEEK_SET);
				break;
			}
		}else
		{
			cfseek(file,length-sizeof(int),SEEK_CUR);
		}
	}

	cfclose(file);
	ntbl_working_table_file = old_curr_read;

	return true;
}

// ==================
// ntbl_DecrementTableRef
// ==================
//
// Reduces the reference count of the table file by 1, unloading when needed
void ntbl_DecrementTableRef(int tid)
{
	ASSERT(tid!=-1);
	if(tid==-1)
		return;

	ASSERT(Ntbl_loaded_table_files[tid].used);
	if(!Ntbl_loaded_table_files[tid].used)
		return;

	Ntbl_loaded_table_files[tid].count--;
	ASSERT(Ntbl_loaded_table_files[tid].count>=0);

	if(Ntbl_loaded_table_files[tid].count<=0)
	{
		//unload the table file
		mprintf((0,"NTBL: Tablefile (%s) ref count at 0, unloading\n",Ntbl_loaded_table_files[tid].identifier));
		Ntbl_loaded_table_files[tid].used = false;
		Ntbl_loaded_table_files[tid].type = -1;
	}
}

// =======================
// ntbl_IncrementTableRef
// =======================
//
// Increases the reference count of the table file by 1
void ntbl_IncrementTableRef(int tid)
{
	ASSERT(tid!=-1);
	if(tid==-1)
		return;

	ASSERT(Ntbl_loaded_table_files[tid].used);
	if(!Ntbl_loaded_table_files[tid].used)
		return;

	Ntbl_loaded_table_files[tid].count++;
}

// ======================
// ntbl_PopTableStack
// ======================
//
// Pops the top most tid off the table file stack and returns it's id, -1 if none on stack
int ntbl_PopTableStack(int *table_stack)
{
	ASSERT(table_stack);
	int new_id = table_stack[0];
	if(new_id<-1 || new_id>=MAX_LOADED_TABLE_FILES)
	{
		Int3();
	}

	for(int i=1;i<MAX_LOADED_TABLE_FILES;i++)
	{
		if(table_stack[i]<-1 || table_stack[i]>=MAX_LOADED_TABLE_FILES)
		{
			Int3();
		}

		table_stack[i-1] = table_stack[i];
	}
	table_stack[MAX_LOADED_TABLE_FILES-1] = -1;

	return new_id;
}

// =======================
// ntbl_PushTableStack
// =======================
//
// Pushes a tablefile id on to the table stack
void ntbl_PushTableStack(int *table_stack,int id)
{
	ASSERT(table_stack);
	ASSERT(id>=0 && id<MAX_LOADED_TABLE_FILES);
	if(id<0 || id>=MAX_LOADED_TABLE_FILES)
		return;

	ASSERT(Ntbl_loaded_table_files[id].used);
	if(!Ntbl_loaded_table_files[id].used)
		return;

	if(table_stack[MAX_LOADED_TABLE_FILES-1]!=-1)
	{
		mprintf((0,"NTBL: Table stack overflow!!!!\n"));
		Int3();
	}

	for(int i=MAX_LOADED_TABLE_FILES-1;i>0;i--)
	{
		if(table_stack[i-1]<-1 || table_stack[i-1]>=MAX_LOADED_TABLE_FILES)
		{
			Int3();
		}

		table_stack[i] = table_stack[i-1];
	}
	table_stack[0] = id;
}

// ========================
// ntbl_IsOnTableStack
// ========================
//
// Searches through the table stack and looks for a table file id, returns true if there is one
bool ntbl_IsOnTableStack(int *table_stack,int id)
{
	ASSERT(table_stack);
	ASSERT(id!=-1);

	for(int i=0;i<MAX_LOADED_TABLE_FILES;i++)
	{
		if(table_stack[i]==-1)
			return false;
		if(table_stack[i]==id)
			return true;
	}

	return false;
}

// ======================
// ntbl_RemoveFromTableStack
// ======================
//
// Removes the instance of a table file id from a table stack
void ntbl_RemoveFromTableStack(int *table_stack,int id)
{
	ASSERT(table_stack);
	ASSERT(id!=-1);

	int i,j;

	for(i=0;i<MAX_LOADED_TABLE_FILES;i++)
	{
		if(table_stack[i]==-1)
			return;

		if(table_stack[i]==id)
		{
			ntbl_DecrementTableRef(id);

			//we're removing this guy, pull the rest up
			for(j=i;j<MAX_LOADED_TABLE_FILES-1;j++)
			{
				table_stack[j] = table_stack[j+1];
			}
			table_stack[MAX_LOADED_TABLE_FILES-1] = -1;
		}
	}	
}