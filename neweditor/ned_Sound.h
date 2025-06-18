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
 #ifndef _NED_SOUND_H
#define _NED_SOUND_H


struct ned_sound_info
{
	char name[PAGENAME_LEN];	
	char				used;
	
	int				sample_index;

	int				loop_start;			// Start byte of repeated loop for looping samples
	int				loop_end;			// End byte of repeating loop for looping samples
	unsigned int	flags;				// 2d/3d, variable frequency
	float				max_distance;		// Maximum distance in which a sound is heard
	float				min_distance;		// Sound gets no louder at min_distance
	int				inner_cone_angle;	// Angle in which sound is played at full base volume
	int	         outer_cone_angle;	// Angle in which sound is at its lowest base volume
	float				outer_cone_volume;// A sounds lowest base volume level
	float				import_volume;		// Volume multiplier

	char raw_filename[PAGENAME_LEN];

	int table_file_id;
	int table_stack[MAX_LOADED_TABLE_FILES];
};

#define MAX_SOUNDS			750
extern ned_sound_info	Sounds[MAX_SOUNDS];
extern int Num_sounds;

// Initializes the Sound system
int ned_InitSounds ();

// ===========================
// ned_InitializeSoundData
// ===========================
//
// Given a Sounds slot this function initializes the data inside it
// to default information
// DO NOT TOUCH TABLE STACK DATA
void ned_InitializeSoundData(int slot);

// =======================
// ned_FreeSoundData
// =======================
//
// Given a Sounds slot this function frees any memory that may 
// need to be freed before a sound is destroyed
// DO NOT TOUCH TABLE STACK DATA
void ned_FreeSoundData(int slot);

// Sets all sounds to unused
void InitSounds ();
// Allocs a sound for use, returns -1 if error, else index on success
int AllocSound ();
// Frees sound index n
void FreeSound (int n);
int ned_FindSound(char *name);
// Frees all sounds from memory
void ned_FreeAllSounds ();

// =====================
// ned_AllocSound
// =====================
//
//	Searches for an available sound slot, and returns it, -1 if none
//  if name is not NULL, then it is being allocated from a table file, and thus, the tablefile
//	parameter must also be specified.  If there is already a texture by that name (loaded from 
//  a table file), than it will be pushed onto the stack.
int ned_AllocSound(char *name=NULL,int tablefile=-1);

// ==================
// ned_FreeSound
// ==================
//
// given a sound slot it free's it and makes it available
// if force_unload is true, and this slot was loaded from a table file, then
// it will unload all instances (based on it's table stack) from memory, else
// it will just pop off the current instance and load back in the popped version
void ned_FreeSound(int slot,bool force_unload=false);

#endif 
