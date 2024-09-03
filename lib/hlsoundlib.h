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

#ifndef __HLSOUNDLIB_H__
#define __HLSOUNDLIB_H__

#include "ssl_lib.h"
#include "object.h"

//////////////////////////////////////////////////////////////////////////

#define MAX_SOUNDS_MIXED 40
#define MIN_SOUNDS_MIXED 20
#define MAX_SOUND_OBJECTS 3000

extern char Sound_quality;
extern char Sound_mixer;
extern bool Sound_doppler;
extern bool Sound_reverb;
extern char Sound_card_name[];

class sound_object 
{
public:
	sound_object() { m_obj_type_flags = SIF_UNUSED; }

public:
	unsigned int m_obj_type_flags;
	int m_sound_uid;
	int m_sound_index;
	int m_hlsound_uid;

	play_information play_info;

	float volume_3d;  // Used so that 3d sounds can have a base volume (for 2d this is in play_information)

	union 
	{
		struct 
		{
			int segnum;			// Use physics' bit-bit stuff (inside/outside)
			vector pos;
			matrix orient;	  // only need pitch and heading -- not roll (sound cones are symetrical)
		} pos_info;
		
		int object_handle;
	} m_link_info;
};

class hlsSystem 
{
	int m_f_hls_system_init;

	class sound_object m_sound_objects[MAX_SOUND_OBJECTS];

	float m_master_volume;
	int m_sounds_played;

	bool m_pause_new;
	ubyte m_cur_environment;						// current environment being played.
	int n_lls_sounds;								// number of sounds that we want the low level mixer to mix.

	bool Emulate3dSound(int sound_obj_index);
	bool ComputePlayInfo(int sound_obj_index, vector *virtual_pos, vector *virtual_vel, float *adjusted_volume);

	inline int MakeUniqueId(int sound_obj_index);
	inline int ValidateUniqueId(int hl_sound_uid);

	// Forcefully ends a sound
	void StopSound(int sound_obj_index, unsigned char f_immediately = SKT_STOP_IMMEDIATELY);

private:
	int Play3dSound(int sound_index, pos_state *cur_pos, object *cur_obj, int priority, float volume, int flags, float offset=0.0);

public:
	
		// Include a lowel-level sound system
	class llsSystem *m_ll_sound_ptr;
	
	hlsSystem();// {m_f_hls_system_init = 0; m_sounds_played=0; m_master_volume = 1.0; m_pause_new = false;}
	~hlsSystem() {KillSoundLib(true);}

	bool IsActive(void);

	// Start and clean-up after the sound library
	int InitSoundLib(oeApplication *sos, char mixer_type, char quality, bool f_kill_sound_lib = false);
	void UpdateEnvironmentToggles();
	void KillSoundLib(bool f_kill_sound_list);
	void SetLLSoundQuantity(int n_sounds);
	int GetLLSoundQuantity();
	
	bool SetLLevelType();  // These are 

	// Pause and Resume the library
	void PauseSounds(bool f_all_sounds = false);
	void ResumeSounds();
	void StopAllSounds();

	// Code for the beginning and ending of a frame of action

	// Begin_sound_frame(listener pos/orient/velocity)
	// SyncSounds
	// Do sound pos updates -- IF VOLUME IS LOW AND NOT FOREVER, THEN STOP SOUND
	// compute echo / reverb
	// indirect/direct path sounds
	void BeginSoundFrame(bool f_in_game = true);
	
	// Plays the deffered 3d stuff
	void EndSoundFrame();

	// Functions that play a sound

	// 3d functions (we use the sound flags in the page to determine all the cool stuff)
	// Functions that play a 3d sound -- includes the 2d emulation of 3d sound
	int Play3dSound(int sound_index, pos_state *cur_pos, float volume=MAX_GAME_VOLUME, int flags=0, float offset=0.0);
	int Play3dSound(int sound_index, object *cur_obj, float volume=MAX_GAME_VOLUME, int flags=0, float offset=0.0);

	int Play3dSound(int sound_index, int priority, pos_state *cur_pos, float volume = MAX_GAME_VOLUME, int flags = 0, float offset=0.0);
	int Play3dSound(int sound_index, int priority, object *cur_obj, float volume = MAX_GAME_VOLUME, int flags = 0, float offset=0.0);

	int PlayStream(int unique_handle, void *data, int size, int stream_format, float volume, void *stream_callback(void *user_data, int handle, int *size) = NULL);

	// 2d functions
	int Play2dSound(int sound_index, float volume = MAX_GAME_VOLUME/2, float pan = 0.0, unsigned short frequency = 22050);
	
	int Play2dSound(int sound_index, int priority, float volume = MAX_GAME_VOLUME/2, float pan = 0.0, unsigned short frequency = 22050);

	int Update2dSound(int hlsound_uid, float volume, float pan);

	// Do nice looping stop stuff
	void StopSoundLooping(int hlsound_uid);
	void StopSoundImmediate(int hlsound_uid);

	// Stop all sounds attached to an object
	void StopObjectSound(int objhandle);

	// Set the volume for all the sounds attached to an object
	void SetVolumeObject(int objhandle,float volume);

	// Master volume controls for sound effects
	void SetMasterVolume(float volume);
	float GetMasterVolume();

	// Queued sound functions
	void Add2dSoundQueued(int q_num, int sound_index, float volume, float pan, unsigned short frequency);
	void KillQueue(int q_num = 0);
	void KillAllQueues();

	bool CheckAndForceSoundDataAlloc(int sound_file_index);
	bool SetSoundQuality(char quality);
	char GetSoundQuality(void);
	bool SetSoundMixer(char mixer_type);
	char GetSoundMixer(void);

	bool IsSoundPlaying(int hlsound_uid);

	bool GetDopplerState(void);
	void SetDopplerState(bool newstate);
	bool GetReverbState(void);
	void SetReverbState(bool newstate);

	// Midi play stuff
	void SetMidiVolume();
	void GetMidiVolume();
	void PlayMidi();
	void StopMidi();
	void PauseMidi();
	void ResumeMidi();
};

extern hlsSystem Sound_system;


//////////////////////////////////////////////////////////////////////////
//	ENVIRONMENTAL REVERB PRESETS

#define N_ENVAUDIO_PRESETS		26

#define ENVAUD_PRESET_NONE					0
#define ENVAUD_PRESET_PADDEDCELL			1
#define ENVAUD_PRESET_ROOM					2
#define ENVAUD_PRESET_BATHROOM				3
#define ENVAUD_PRESET_LIVINGROOM			4
#define ENVAUD_PRESET_STONEROOM				5
#define ENVAUD_PRESET_AUDITORIUM			6
#define ENVAUD_PRESET_CONCERTHALL			7
#define ENVAUD_PRESET_CAVE					8
#define ENVAUD_PRESET_ARENA         		9
#define ENVAUD_PRESET_HANGAR        		10
#define ENVAUD_PRESET_CARPETEDHALLWAY		11
#define ENVAUD_PRESET_HALLWAY				12
#define ENVAUD_PRESET_STONECORRIDOR			13
#define ENVAUD_PRESET_ALLEY					14
#define ENVAUD_PRESET_FOREST				15
#define ENVAUD_PRESET_CITY					16
#define ENVAUD_PRESET_MOUNTAINS				17
#define ENVAUD_PRESET_QUARRY				18
#define ENVAUD_PRESET_PLAIN					19
#define ENVAUD_PRESET_PARKINGLOT			20
#define ENVAUD_PRESET_SEWERPIPE				21
#define ENVAUD_PRESET_UNDERWATER			22
#define ENVAUD_PRESET_DRUGGED				23
#define ENVAUD_PRESET_DIZZY					24
#define ENVAUD_PRESET_PSYCHOTIC				25

#endif
