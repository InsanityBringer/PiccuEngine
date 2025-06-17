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
 

#ifndef __NED_TABLEFILE_H_
#define __NED_TABLEFILE_H_

#include <stdlib.h>

#include "bitmap.h"

#define MAX_LOADED_TABLE_FILES	3
#define TABLE_FILE_BASE			0
#define TABLE_FILE_MISSION		1
#define TABLE_FILE_MODULE		2

#define PAGENAME_LEN		35
#define PAGETYPE_TEXTURE	1
#define PAGETYPE_DOOR		5
#define PAGETYPE_SOUND		7
#define PAGETYPE_GENERIC	10

// Object stuff now in ned_Object.h
// Texture stuff now in ned_GameTexture.h
// Door stuff now in ned_Door.h
// Sound stuff now in ned_Sound.h

#define MAX_IDENTIFIER_NAME		256

typedef struct
{
	bool used;			// if this slot is in use
	int type;			// base, mission, module
	int count;			// how many files are loaded from it
	char identifier[MAX_IDENTIFIER_NAME];	//name
}tTableFileInfo;
extern tTableFileInfo Ntbl_loaded_table_files[MAX_LOADED_TABLE_FILES];

// ntbl_LoadTableFile() return codes
#define NTBL_NOT_FOUND		-1
#define NTBL_CORRUPT		-2
#define NTBL_TABLE_IN_MEM	-3
#define NTBL_FATAL			-4
#define NTBL_NO_AVAIL_TABLE_SLOTS	-5

// ==========================
// ntbl_LoadTableFile
// ==========================
//
// Parses a table file and keeps it in memory for use
// if the return value is negative, then check the above #define's for what the error was
// if the return value is >=0 then it is the slot it was loaded into
int ntbl_LoadTableFile(char *filename);

// ========================
// ntbl_DumpToFile
// ========================
//
//	Dumps all the current table file information to file
bool ntbl_DumpToFile(char *filename);

// ============================
// ntbl_DeleteTableFilePages
// ============================
//
// deletes all the pages associated with a table file, will revert stacked pages
void ntbl_DeleteTableFilePages(char *filename);

// =========================
// ntbl_Initialize
// =========================
//
// Initializes the manage system
void ntbl_Initialize(void);


// ====================
// ned_FindObjectID
// ====================
//
// given the name of a generic object it will return it's id...-1 if it doesn't exist
// it will only search generic objects loaded from tablefiles
int ned_FindObjectID(char *name);

// ====================
// ned_FindSound
// ====================
//
// given the name of a sound it will return it's id...-1 if it doesn't exist
// it will only search sounds loaded from tablefiles
int ned_FindSound(char *name);

/////////////////////////////////////////////////////////////////////////////////
// ==================
// ntbl_DecrementTableRef
// ==================
//
// Reduces the reference count of the table file by 1, unloading when needed
void ntbl_DecrementTableRef(int tid);

// =======================
// ntbl_IncrementTableRef
// =======================
//
// Increases the reference count of the table file by 1
void ntbl_IncrementTableRef(int tid);

// ======================
// ntbl_PopTableStack
// ======================
//
// Pops the top most tid off the table file stack and returns it's id, -1 if none on stack
int ntbl_PopTableStack(int *table_stack);

// =======================
// ntbl_PushTableStack
// =======================
//
// Pushes a tablefile id on to the table stack
void ntbl_PushTableStack(int *table_stack,int id);

// =============================
// ntbl_OverlayPage
// =============================
//
// Given the pagetype and an id, this function takes the information from table_file_id and
// reloads it's data into the page
bool ntbl_OverlayPage(int pagetype,int slot);


#endif
