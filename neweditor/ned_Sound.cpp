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
#include "ned_Tablefile.h"
#include "ssl_lib.h"

ned_sound_info		Sounds[MAX_SOUNDS];
sound_file_info SoundFiles[MAX_SOUND_FILES];
int Num_sounds						= 0;
int Num_sound_files					= 0;
char * *  Static_sound_names;

// ====================
// ned_FindSound
// ====================
//
// given the name of a sound it will return it's id...-1 if it doesn't exist
// it will only search sounds loaded from tablefiles
int ned_FindSound(char *name)
{
	ASSERT(name);
	if(!name)
		return -1;

	int i;
	for(i=0;i<MAX_SOUNDS;i++)
	{
		if(Sounds[i].used && Sounds[i].table_file_id!=-1)
		{
			//see if the name matches
			if(!strnicmp(name,Sounds[i].name,PAGENAME_LEN-1))
			{
				//match
				return i;
			}
		}
	}
	return -1;
}

// =====================
// ned_AllocSound
// =====================
//
//	Searches for an available sound slot, and returns it, -1 if none
//  if name is not NULL, then it is being allocated from a table file, and thus, the tablefile
//	parameter must also be specified.  If there is already a texture by that name (loaded from 
//  a table file), than it will be pushed onto the stack.
int ned_AllocSound(char *name,int tablefile)
{
	if(name)
	{
		//this is being allocated for a tablefile load
		ASSERT(tablefile!=-1);
		if(tablefile==-1)
			return -1;

		//check to see if it's already in memory
		int old_id = ned_FindSound(name);
		if(old_id!=-1)
		{
			//this item is already in memory!
			ned_FreeSoundData(old_id);
			ned_InitializeSoundData(old_id);			

			if(Sounds[old_id].table_file_id==tablefile)
			{
				//we're just re-reading it

			}else
			{
				//push it onto the stack
				ntbl_PushTableStack(Sounds[old_id].table_stack,Sounds[old_id].table_file_id);
				ntbl_IncrementTableRef(tablefile);

				Sounds[old_id].table_file_id = tablefile;
			}
			return old_id;
		}
	}

	int i,index = -1;
	for(i=0;i<MAX_SOUNDS;i++)
	{
		if(!Sounds[i].used)
		{
			index = i;
			memset(&Sounds[i],0,sizeof(ned_sound_info));
			Sounds[i].table_file_id = -1;	//we don't belong to any table file right now
			for(int j=0;j<MAX_LOADED_TABLE_FILES;j++) Sounds[i].table_stack[j] = -1;
			break;
		}
	}

	if(index!=-1)
	{
		Sounds[index].used = true;
		Num_sounds++;

		if(name)
		{
			ASSERT(strlen(name)<PAGENAME_LEN);

			//table file load, give it a name and mark it's tablefile
			strncpy(Sounds[index].name,name,PAGENAME_LEN-1);
			Sounds[index].name[PAGENAME_LEN-1] = '\0';
			Sounds[index].table_file_id = tablefile;
			ntbl_IncrementTableRef(tablefile);

		}else
		{
			//not from tablefile
			Sounds[index].name[0] = '\0';
			Sounds[index].table_file_id = -1;
		}

		ned_InitializeSoundData(index);
	}
	
	return index;
}

// =========================
// ned_FreeAllSounds
// =========================
//
// Frees all sounds from memory
void ned_FreeAllSounds ()
{
	for (int i=0;i<MAX_SOUNDS;i++)
	{
		if (Sounds[i].used)
			ned_FreeSound (i,true);
	}
}

// ======================
// ned_InitSounds
// ======================
// 
// Initializes the Sound system
int ned_InitSounds ()
{
	// Initializes the sound system

	int i,j;
	
	mprintf ((0,"Initializing sound system.\n"));

	memset(Sounds,0,sizeof(ned_sound_info)*MAX_SOUNDS);
	InitSounds();
	for(i=0;i<MAX_SOUNDS;i++)
	for(j=0;j<MAX_LOADED_TABLE_FILES;j++)
		Sounds[i].table_stack[j] = -1;
	
	atexit (ned_FreeAllSounds);

	return 1;
}

// ===========================
// ned_InitializeSoundData
// ===========================
//
// Given a Sounds slot this function initializes the data inside it
// to default information
// DO NOT TOUCH TABLE STACK DATA
void ned_InitializeSoundData(int slot)
{
	Sounds[slot].flags = 0;		//default to no type
}

// =======================
// ned_FreeSoundData
// =======================
//
// Given a Sounds slot this function frees any memory that may 
// need to be freed before a sound is destroyed
// DO NOT TOUCH TABLE STACK DATA
void ned_FreeSoundData(int slot)
{
}

// Sets all sounds to unused
void InitSounds ()
{
	int i;
	
	for (i = 0;i<MAX_SOUNDS;i++)
	{
		Sounds[i].used = 0;
		Sounds[i].name[0] = 0;
		Sounds[i].flags = 0;
	}
	Num_sounds = 0;

	for (i = 0;i<MAX_SOUND_FILES;i++)
	{
		SoundFiles[i].name[0] = 0;
		SoundFiles[i].used = 0;
	}
	Num_sound_files = 0;
}

// Allocs a sound for use, returns -1 if error, else index on success
int AllocSound ()
{
	//Don't alloc sound 0, because 0 means something special (default/none)
	for (int i=1;i<MAX_SOUNDS;i++)
	{
		if (Sounds[i].used==0)
		{
			memset(&Sounds[i], 0, sizeof(ned_sound_info));



//			Sounds[i].flags = SPF_LISTENER_UPDATE;

			Sounds[i].used=1;
			Num_sounds++;
			return i;
		}
	}

	Int3();		 // No sounds free!
	return -1;
}

// Frees sound index n
void FreeSound (int n)
{
	ASSERT (Sounds[n].used>0);

	Sounds[n].used = 0;
	Sounds[n].name[0] = 0;
	Sounds[n].flags = 0;
	Num_sounds--;
}

// ==================
// ned_FreeSound
// ==================
//
// given a sound slot it free's it and makes it available
// if force_unload is true, and this slot was loaded from a table file, then
// it will unload all instances (based on it's table stack) from memory, else
// it will just pop off the current instance and load back in the popped version
void ned_FreeSound(int slot,bool force_unload)
{
	ASSERT(slot>=0 && slot<MAX_SOUNDS);
	if(slot<0 || slot>=MAX_SOUNDS)
		return;

	ASSERT(Sounds[slot].used);
	if(!Sounds[slot].used)
		return;

	ned_FreeSoundData(slot);

	/////////////////////////////////////////
	if(Sounds[slot].table_file_id==-1)
	{
		Sounds[slot].used = false;
		Num_sounds--;
		return;
	}

	//it has table file references, decrement
	ntbl_DecrementTableRef(Sounds[slot].table_file_id);

	if(force_unload)
	{
		Sounds[slot].used = false;
		Num_sounds--;

		//go through it's stack, decrement references
		int tid;
		tid = ntbl_PopTableStack(Sounds[slot].table_stack);
		while(tid!=-1)
		{
			ntbl_DecrementTableRef(tid);
			tid = ntbl_PopTableStack(Sounds[slot].table_stack);
		}
	}else
	{
		//see if we have anything on the stack
		Sounds[slot].table_file_id = ntbl_PopTableStack(Sounds[slot].table_stack);
		if(Sounds[slot].table_file_id==-1)
		{
			//nothing on the stack, its a dead one
			Sounds[slot].used = false;
			Num_sounds--;
		}else
		{
			//reload the item
			ned_InitializeSoundData(slot);
			if(!ntbl_OverlayPage(PAGETYPE_SOUND,slot))
			{
				Int3();

				Sounds[slot].used = false;
				Num_sounds--;
			}			
		}
	}
}

#undef PLAY_SOUND_SYSTEM

#ifdef PLAY_SOUND_SYSTEM

////////////////////////////////////////////////////////////

// Some high level sound library functions
// copied from main\sndlib\hlsoundlib.cpp
#include "hlsoundlib.h"
#include "sounds.h"
#include "ds3dlib.h"
#include "streamaudio.h"
extern float Frametime;

hlsSystem Sound_system;
char Sound_quality = SQT_NORMAL;
char Sound_mixer = SOUND_MIXER_SOFTWARE_16;
char Sound_card_name[256] = "";

hlsSystem::hlsSystem()
{
	m_f_hls_system_init = 0;
	m_ll_sound_ptr = NULL;
	m_sounds_played=0; 
	m_master_volume = 1.0; 
	m_pause_new = false;
	n_lls_sounds = MAX_SOUNDS_MIXED;
}
bool hlsSystem::IsActive(void)
{
	if(m_f_hls_system_init) return true;
	return false;
}
// Start and clean-up after the sound library
int hlsSystem::InitSoundLib(oeApplication *sos, char mixer_type, char quality, bool f_kill_sound_list)
{
	int status;
#ifndef NEWEDITOR
	// Turn off sound if desired
	if ((FindArg("-nosound")) || Dedicated_server) {
		m_ll_sound_ptr = NULL;
		return 0;
	}
#endif
	if(m_f_hls_system_init) 
	{
		KillSoundLib(f_kill_sound_list);
	}
	
	// Create and initialize the low-level sound library
	#if defined(WIN32)
		m_ll_sound_ptr = new win_llsSystem;
	#elif defined(MACINTOSH)
		m_ll_sound_ptr = new mac_llsSystem;
	#endif
	ASSERT(m_ll_sound_ptr);
	if(m_ll_sound_ptr == NULL) return 0;
	if (Sound_card_name[0]) {
		m_ll_sound_ptr->SetSoundCard(Sound_card_name);
	}
	else {
		m_ll_sound_ptr->SetSoundCard(NULL);
	}
	status = m_ll_sound_ptr->InitSoundLib(mixer_type, sos, n_lls_sounds);
	// Initialize the high-level sound library
	// Set the flag to initialized if all was o.k.
	if (status) {
		m_f_hls_system_init = 1;
		SetSoundQuality(quality);
		Sound_quality = GetSoundQuality();
		Sound_mixer = GetSoundMixer();
	
	// invoke high level stream system
		AudioStream::InitSystem(this->m_ll_sound_ptr);
	//	set current environment
		m_cur_environment = ENVAUD_PRESET_NONE;
	}
// All done
	return status;
}
// Kills the sound library
void hlsSystem::KillSoundLib(bool f_kill_sound_list)
{
	int i;
	if(m_f_hls_system_init) {
	// clean up stream system
		AudioStream::Shutdown();
		for(i = 0; i < MAX_SOUND_OBJECTS; i++) 
		{
			m_sound_objects[i].m_obj_type_flags = SIF_UNUSED;
			m_sound_objects[i].m_hlsound_uid = -1;
		}
		m_ll_sound_ptr->DestroySoundLib();
		m_f_hls_system_init = 0;
	}
	if (m_ll_sound_ptr) {
		delete m_ll_sound_ptr;
		m_ll_sound_ptr = NULL;
	}
	// This is independant of the actual sound_lib status.  It happened because sounds
	// use the manage system.
// moved this from beginning of function because won't the data freed here screw up streaming/looping sounds?
// before the sound library is closed?
	if(f_kill_sound_list) 
	{
		for (int i=0;i<MAX_SOUND_FILES;i++)
		{
			SoundLoadFree(i);
		}
	}
}
// Stops all sound from playing
void hlsSystem::StopAllSounds()
{
	int i;
	if(!m_f_hls_system_init) return;
	
	for(i = 0; i < MAX_SOUND_OBJECTS; i++) 
	{
		m_sound_objects[i].m_obj_type_flags = SIF_UNUSED;
		m_sound_objects[i].m_hlsound_uid = -1;
	}
	m_ll_sound_ptr->StopAllSounds();
	BeginSoundFrame(false);
	EndSoundFrame();
	mprintf((0, "Stopped all sounds\n"));
}
// Code for the beginning and ending of a frame of action
// Begin_sound_frame(listener pos/orient/velocity)
// SyncSounds
// Do sound pos updates -- IF VOLUME IS LOW AND NOT FOREVER (or looping), THEN STOP SOUND
// compute echo / reverb
// indirect/direct path sounds
void hlsSystem::BeginSoundFrame(bool f_in_game)
{
#ifdef NEWEDITOR
	ASSERT(f_in_game == FALSE);
#else
	bool hwsound_support;							// if this is true, sound_render_system is being used
	int n;
	pos_state listener_pos;	
#endif
	short *sound_room_list;							// these values are only meant for sound render system
	int i, counter = 0;
	const int num_samples_this_frame = (int) (Frametime * 22050);
	if(!m_f_hls_system_init) return;
	DebugBlockPrint("S ");
//	determine if we're using hardware for sound support.
	m_ll_sound_ptr->SoundStartFrame();
// non game sound frame, this, just updates currently playing sounds.
// placed here to leave early and ensure any further processing involves game only.
	if (!f_in_game)
	{
		sound_room_list = NULL;
		for (i=0, counter = 0; i<MAX_SOUND_OBJECTS; i++ )	
		{
			if ( m_sound_objects[i].m_obj_type_flags && m_sound_objects[i].m_sound_uid != -1)	
			{
				counter++;
				ASSERT(m_sound_objects[i].m_sound_uid > -1);
				if ( !m_ll_sound_ptr->IsSoundInstancePlaying(m_sound_objects[i].m_sound_uid) )	
				{
					StopSound(i);			// stop any sounds playing.
				}
			}
		}
		
		goto end_beginsoundframe;		// this will handle any remaining processing
	}
#ifndef NEWEDITOR
//	FROM HERE ON, WE ASSUME WE'RE IN GAME.
	hwsound_support = (sound_render_start_frame() && !ROOMNUM_OUTSIDE(Viewer_object->roomnum));
//	define current environment of the listener.
	listener_pos.velocity = &Viewer_object->mtype.phys_info.velocity;
	listener_pos.position = &Viewer_object->pos;
	listener_pos.orient = &Viewer_object->orient;
	listener_pos.roomnum = Viewer_object->roomnum;
	m_ll_sound_ptr->SetListener(&listener_pos);
// do environmental audio int current room.
	if (!OBJECT_OUTSIDE(Viewer_object)) {
		n = Rooms[Viewer_object->roomnum].env_reverb;
		if (n >= 0) {
			if (m_cur_environment != n) {
				ASSERT(n < N_ENVAUDIO_PRESETS);
				m_ll_sound_ptr->SetGlobalReverbProperties(EnvAudio_Presets[n].volume, EnvAudio_Presets[n].damping, EnvAudio_Presets[n].decay);
				m_cur_environment = n;
			}
		}
	}
	else {
		if (m_cur_environment != ENVAUD_PRESET_NONE) {
			n = ENVAUD_PRESET_NONE;
			m_ll_sound_ptr->SetGlobalReverbProperties(EnvAudio_Presets[n].volume, EnvAudio_Presets[n].damping, EnvAudio_Presets[n].decay);
			m_cur_environment = n;
		}
	}
// render all rooms within a certain range from player (traverse rooms through portals)
	sound_room_list = hwsound_support ? sound_render_audible_rooms(&listener_pos) : NULL;
//	SoundQ_process();
// if sound object is outside listener's cone of audibility, emulate the sound.
//	if sound is audible, then determine its playback properties.
//
	for (i=0, counter = 0; i<MAX_SOUND_OBJECTS; i++ )	
	{
		if ( m_sound_objects[i].m_obj_type_flags )	
		{
			counter++;
//			oldvolume = m_sound_objects[i].volume;
//			oldpan = m_sound_objects[i].pan;
//			mprintf((0, "SO %d playing: uid %d\n", i, m_sound_objects[i].m_sound_uid));
			// Handle high-level only sounds
			if(m_sound_objects[i].m_obj_type_flags & SIF_TOO_FAR)
			{
				if(m_sound_objects[i].m_obj_type_flags & SIF_LOOPING)
				{
//					while(m_sound_objects[i].play_info.m_samples_played >= Sounds[m_sound_objects[i].m_sound_index].loop_end)
//					{
//						m_sound_objects[i].play_info.m_samples_played -= (Sounds[m_sound_objects[i].m_sound_index].loop_end - Sounds[m_sound_objects[i].m_sound_index].loop_start);
//					}
				}
				else
				{
					// Advance sound pointer and see if sound is done
					m_sound_objects[i].play_info.m_samples_played += num_samples_this_frame;
					
					if (m_sound_objects[i].play_info.m_samples_played >= SoundFiles[Sounds[m_sound_objects[i].m_sound_index].sample_index].sample_length)
					{
						StopSound(i);
						continue;
					}
				}
				if(m_sound_objects[i].m_obj_type_flags & SIF_OBJ_UPDATE) 
				{
					//If object not alive, kill its sound
					object *obj_sound = ObjGet(m_sound_objects[i].m_link_info.object_handle);
					if (!obj_sound || obj_sound->type==OBJ_DUMMY)
					{
						StopSound(i);
						continue;		// Go on to next sound...
					}
				}
			// determine new properties of sound if it's still playing, its volume, other properties.
				if(Emulate3dSound(i))
				{
				//	sound is audible now.
					if(m_sound_objects[i].m_sound_uid != -1)
					{
						m_sound_objects[i].m_obj_type_flags &= (~SIF_TOO_FAR);
					}
					else
					{
						// mprintf((0, "LLSound full 3\n"));
					}
				}
				continue;
			}
			ASSERT(m_sound_objects[i].m_sound_uid > -1);
			if ( !m_ll_sound_ptr->IsSoundInstancePlaying(m_sound_objects[i].m_sound_uid) )	
			{
				StopSound(i);
				continue;		// Go on to next sound...
			}
			if ((Sounds[m_sound_objects[i].m_sound_index].flags & SPF_LISTENER_UPDATE) &&
				 !(m_sound_objects[i].m_obj_type_flags & SIF_NO_3D_EFFECTS))
			{
				if(m_sound_objects[i].m_obj_type_flags & SIF_OBJ_UPDATE)
				{
					object *obj_sound = ObjGet(m_sound_objects[i].m_link_info.object_handle);
					if(!obj_sound || obj_sound->type==OBJ_DUMMY)
					{
						StopSound(i);
						continue;		// Go on to next sound...
					}
				}
				if((m_sound_objects[i].m_obj_type_flags & (SIF_PLAYING_3D | SIF_OBJ_UPDATE)))
				{
					bool f_audible;
					vector virtual_pos;
					vector virtual_vel;
					float adjusted_volume;
					f_audible = ComputePlayInfo(i, &virtual_pos, &virtual_vel, &adjusted_volume);
					if (!f_audible)
					{
						m_ll_sound_ptr->StopSound(m_sound_objects[i].m_sound_uid, SKT_STOP_IMMEDIATELY);
						m_sound_objects[i].m_sound_uid = 0;
						m_sound_objects[i].m_obj_type_flags |= SIF_TOO_FAR;
					} 
					else
					{
						matrix orient = Identity_matrix;
						pos_state cur_pos;
						cur_pos.velocity = &virtual_vel;
						cur_pos.position = &virtual_pos;
						cur_pos.orient = &orient;
						m_ll_sound_ptr->AdjustSound(m_sound_objects[i].m_sound_uid, &cur_pos, adjusted_volume);
					}
				}
			}		
		}
	}
#endif
end_beginsoundframe:
#ifndef NEWEDITOR
	AudioStream::Frame();
#endif
	DebugBlockPrint("DS");
	mprintf_at((3,1,0, "HNS: %04d", counter));
}
    
// Plays the deffered 3d stuff
void hlsSystem::EndSoundFrame()
{
	DebugBlockPrint("X ");
	if(!m_f_hls_system_init) return;
//	sound_render_end_frame();
	m_ll_sound_ptr->SoundEndFrame();
	DebugBlockPrint("DX");
}
// General purpose 2d sound play function
int hlsSystem::Play2dSound(int sound_index, float volume, float pan, unsigned short frequency)
{
	return hlsSystem::Play2dSound(sound_index, SND_PRIORITY_NORMAL, volume, pan, frequency);
}
int hlsSystem::Play2dSound(int sound_index, int priority, float volume, float pan, unsigned short frequency)
{
	int i = 0;
	int sound_uid;
	int sound_obj_index;
	if(!m_f_hls_system_init) return -1;
	if(sound_index == SOUND_NONE_INDEX) return -1;
	if(m_pause_new) return -1;
	if(sound_index < 0)
	{
		return -1;
	}
	Sound_system.CheckAndForceSoundDataAlloc(sound_index);
	// Handles exclusive sounds
	if(Sounds[sound_index].flags & SPF_PLAYS_EXCLUSIVELY) 
	{
		sound_uid = m_ll_sound_ptr->IsSoundPlaying(sound_index);
		if(sound_uid != -1) return -1;
	}
	// Handles play once sounds
	if(Sounds[sound_index].flags & SPF_PLAYS_ONCE) 
	{
		for(sound_obj_index = 0; sound_obj_index < MAX_SOUND_OBJECTS; sound_obj_index++) 
		{
			if((m_sound_objects[sound_obj_index].m_sound_index == sound_index) && 
				((sound_uid = m_sound_objects[sound_obj_index].m_sound_uid) != -1))
			{
				StopSound(sound_obj_index);
			}
		}
	}
	for(i = 0; i < MAX_SOUND_OBJECTS; i++) 
	{
		if(m_sound_objects[i].m_obj_type_flags == SIF_UNUSED) break;
	}
//	mprintf((0, "HL %d\n", i));
//	ASSERT(i < MAX_SOUND_OBJECTS);
	if(i >= MAX_SOUND_OBJECTS) 
	{
		mprintf((0, "250 Sounds????  Get Chris\n"));
		return -1;
	}
	// Increment the total number of sounds played
	m_sounds_played++;
	// Initialize the play information to nice values
	memset(&m_sound_objects[i].play_info, 0, sizeof(play_information));
	m_sound_objects[i].play_info.samples_per_22khz_sample = 1.0;
	m_sound_objects[i].m_hlsound_uid = MakeUniqueId(i);
	m_sound_objects[i].play_info.priority = SND_PRIORITY_NORMAL;
//	static_skip++;
//	if (static_skip > 2)
//		static_skip = 0;
//
//	m_sound_objects[i].play_info.sample_skip_interval = static_skip;
//
//	if(m_sound_objects[i].play_info.sample_skip_interval == 0)
//		mprintf((0, "22.5k\n"));
//	else if (m_sound_objects[i].play_info.sample_skip_interval == 1)
//		mprintf((0, "11.25k\n"));
//	else 
//		mprintf((0, "5.75k\n"));
	m_sound_objects[i].m_sound_index = sound_index;
	m_sound_objects[i].m_sound_uid = m_ll_sound_ptr->PlaySound2d(&m_sound_objects[i].play_info, sound_index, 
									volume * m_master_volume, pan, (Sounds[sound_index].flags & SPF_LOOPED));
	//ASSERT(m_sound_objects[i].m_sound_uid != -1);
	if(m_sound_objects[i].m_sound_uid == -1)
	{	
		m_sound_objects[i].m_obj_type_flags = SIF_UNUSED;
		m_sound_objects[i].m_hlsound_uid = -1;
		// mprintf((0, "LLSound full 1\n"));
		return -1;
	}
	
	m_sound_objects[i].m_obj_type_flags |= SIF_PLAYING_2D;
	
	if(Sounds[m_sound_objects[i].m_sound_index].flags & SPF_LOOPED)
		m_sound_objects[i].m_obj_type_flags |= SIF_LOOPING;
	return m_sound_objects[i].m_hlsound_uid;
}
// Forcefully ends a sound
void hlsSystem::StopSound(int sound_obj_index, unsigned char f_stop_priority)
{
	if(!m_f_hls_system_init) return;
	m_sound_objects[sound_obj_index].m_obj_type_flags &= (~SIF_LOOPING);
	if(m_sound_objects[sound_obj_index].m_sound_uid != -1)
		m_ll_sound_ptr->StopSound(m_sound_objects[sound_obj_index].m_sound_uid, f_stop_priority);
	
	if(f_stop_priority == SKT_STOP_IMMEDIATELY ||
		m_sound_objects[sound_obj_index].m_sound_uid == -1) 
	{
		m_sound_objects[sound_obj_index].m_obj_type_flags = SIF_UNUSED;
		m_sound_objects[sound_obj_index].m_hlsound_uid = -1;
	}
}
inline int hlsSystem::MakeUniqueId(int sound_obj_index)
{
	return ((((int)m_sounds_played)<<12) + sound_obj_index);
}
inline int hlsSystem::ValidateUniqueId(int hl_sound_uid) 
{
	ASSERT(MAX_SOUND_OBJECTS <= 0x0FFF);
	if(hl_sound_uid == m_sound_objects[hl_sound_uid & 0x0FFF].m_hlsound_uid) 
	{
		return hl_sound_uid & 0x0FFF;
	}
	else 
	{
		return -1;
	}
}
bool hlsSystem::SetSoundQuality(char quality)
{
	if(!m_f_hls_system_init) 
		return false;
	m_ll_sound_ptr->SetSoundQuality(quality);
	Sound_quality = m_ll_sound_ptr->GetSoundQuality();
	return true;
}
char hlsSystem::GetSoundQuality(void)
{
	if(!m_f_hls_system_init) 
		return -1;
	return (Sound_quality = m_ll_sound_ptr->GetSoundQuality());
}
bool hlsSystem::SetSoundMixer(char mixer)
{
	if(!m_f_hls_system_init) 
		return false;
	m_ll_sound_ptr->SetSoundMixer(mixer);
	Sound_mixer = m_ll_sound_ptr->GetSoundMixer();
	return true;
}
char hlsSystem::GetSoundMixer(void)
{
	if(!m_f_hls_system_init) 
		return SOUND_MIXER_NONE;
	return (Sound_mixer = m_ll_sound_ptr->GetSoundMixer());
}

#endif
