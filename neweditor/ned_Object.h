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
 

#ifndef _NED_OBJECT_H
#define _NED_OBJECT_H

#include "ned_Tablefile.h"
#include "object_external_struct.h"

struct ned_object_info
{
	bool used;
	char name[PAGENAME_LEN];
	
	int type;
	float size;
	int flags;
	char image_filename[PAGENAME_LEN];
	int	render_handle;			//handle for bitmap/polygon model(hi-res)

	// Valid for polygon models with weapons
	otype_wb_info static_wb[MAX_WBS_PER_OBJ];

	// Valid for lighting of objects
	light_info lighting_info;

	int	hit_points;				//if destroyable, the hit points

	int ref_count;
	int table_file_id;
	int table_stack[MAX_LOADED_TABLE_FILES];
};

#define MAX_OBJECT_IDS	810
extern ned_object_info	Object_info[MAX_OBJECT_IDS];
extern int Num_objects;

// ned_FindObjectID
// ====================
//
// given the name of an object it will return it's id...-1 if it doesn't exist
// it will only search objects loaded from tablefiles
int ned_FindObjectID(char *name);

// =====================
// ned_AllocObjectInfo
// =====================
//
//	Searches for an available object ID, and returns it, -1 if none
//  if name is not NULL, then it is being allocated from a table file, and thus, the tablefile
//	parameter must also be specified.  If there is already an object by that name (loaded from 
//  a table file), than it will be pushed onto the stack.
int ned_AllocObjectInfo(char *name,int tablefile=-1);

// ==================
// ned_FreeObjectInfo
// ==================
//
// given an object slot it free's it and makes it available
// if force_unload is true, and this slot was loaded from a table file, then
// it will unload all instances (based on it's table stack) from memory, else
// it will just pop off the current instance and load back in the popped version
void ned_FreeObjectInfo(int slot,bool force_unload=false);

// =========================
// ned_FreeAllObjects
// =========================
//
// Frees all objects from memory
void ned_FreeAllObjects ();

// ======================
// ned_InitObjects
// ======================
// 
// Initializes the Object system
int ned_InitObjects ();

// ===========================
// ned_InitializeObjectData
// ===========================
//
// Given a Object_info slot this function initializes the data inside it
// to default information
// DO NOT TOUCH TABLE STACK DATA
void ned_InitializeObjectData(int slot);

// =======================
// ned_FreeObjectData
// =======================
//
// Given a Object_info slot this function frees any memory that may 
// need to be freed before a object is destroyed
// DO NOT TOUCH TABLE STACK DATA
void ned_FreeObjectData(int slot);

// ======================
// ned_GetPreviousObject
// ======================
//
// Given current index, gets index of prev object in use
int ned_GetPreviousObject (int n);

// ==========================
// ned_GetNextObject
// ==========================
//
// Given current index, gets index of next object in use
int ned_GetNextObject (int n);

// ========================
// ned_MarkObjectInUse
// ========================
//
// Handles memory management for a object.  Call this, passing true when you need to use a object
// when the object is no longer needed, call this again, passing false.
void ned_MarkObjectInUse(int slot,bool inuse);

void ObjSetAABB(object *obj);

//sets the orientation of an object.  This should be called to orient an object
void ObjSetOrient(object *obj,const matrix *orient);

//Resets the object list: sets all objects to unused, intializes handles, & sets roomnums to -1
//Called by the editor to init a new level.
void ResetObjectList();

//sets up the free list & init player & whatever else
void InitObjects();

//link the object into the list for its room
void ObjLink(int objnum,int roomnum);

void ObjUnlink(int objnum);

//when an object has moved into a new room, this function unlinks it
//from its old room and links it into the new room
void ObjRelink(int objnum,int newroomnum);

// from main\objinfo.cpp
// Sets all objects to unused
void InitObjectInfo();

//Creates the player object in the center of the given room
void CreatePlayerObject(int roomnum);

void InitBigObjects();

void BigObjAdd(int objnum);

void BigObjRemove(int objnum);

//frees up an object.  Generally, ObjDelete() should be called to get
//rid of an object.  This function deallocates the object entry after
//the object has been unlinked
void ObjFree(int objnum);

//-----------------------------------------------------------------------------
//	Scan the object list, freeing down to num_used objects
//	Returns number of slots freed.
int FreeObjectSlots(int num_used);

//returns the number of a free object, updating Highest_object_index.
//Generally, ObjCreate() should be called to get an object, since it
//fills in important fields and does the linking.
//returns -1 if no free objects
int ObjAllocate(void);

void FreeAllObjects();

// HObjectPlace copied from HObject.cpp, and slightly modified
// This function assumes a generic object unless is_player = true
// This is different from HObjectPlace in that it returns the object number (-1 on error)
int InsertObject(room *rp,int facenum,int obj_id,vector pos,matrix orient,bool is_player = false);

bool MoveObject(object *obj, float x, float y, float z);

// copied from HObject.cpp
//	Attempt to set new object position.  May only move part of the way, or maybe not at all.
// Use FVI to find new object position
//	Return:	TRUE if moved, FALSE if can't move
bool MoveObject(object *obj, vector *newpos);

void HObjectDelete();

bool MoveObjects(short *list, int num_objects, vector vec);

bool MoveObjects(short *list, int num_objects, float x, float y, float z);

bool RotateObject(int objnum, angle ang_x, angle ang_y, angle ang_z);

bool RotateObjectAroundPoint(object *objp, angle ang_x, angle ang_y, angle ang_z, vector offset);

// Given an object handle, returns an index to that object's model
int GetObjectImage(int handle);

#endif /* _NED_OBJECT_H */