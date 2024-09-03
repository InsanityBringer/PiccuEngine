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

// High level sound object.  This code is 100% machine independant and
// completely encapsulates the lowel-level machine-dependant sound code.
#include "soundload.h"
#include "hlsoundlib.h"
#include "ssl_lib.h"
#include <stdio.h>
#include <stdlib.h>
#include "mono.h"
#include "pserror.h"
#include "vecmat.h"
#include "args.h"
#include "sounds.h"
#include "game.h"
#include "room.h"
#include "BOA.h"
#include "mem.h"
#include "streamaudio.h"
#include "doorway.h"
#include "dedicated_server.h"
#include "sndrender.h"
#include "descent.h"

#include "llsopenal.h"

#include <string.h>
hlsSystem Sound_system;
char Sound_quality = SQT_NORMAL;
char Sound_mixer = SOUND_MIXER_SOFTWARE_16;
bool Sound_reverb = true;
bool Sound_doppler = true;

char Sound_card_name[256] = "";

EAX2Reverb EnvAudio_Presets[N_ENVAUDIO_PRESETS] =
{
	//Null (normally Generic)
	{},
	//Padded cell
	{0.1715f, 1.0000f, 0.3162f, 0.0010f, 1.0000f, 0.1700f, 0.1000f, 1.0000f, 0.2500f, 0.0010f, {0.0000f, 0.0000f, 0.0000f}, 1.2691f, 0.0020f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true},
	//Room
	{0.4287f, 1.0000f, 0.3162f, 0.5929f, 1.0000f, 0.4000f, 0.8300f, 1.0000f, 0.1503f, 0.0020f, {0.0000f, 0.0000f, 0.0000f}, 1.0629f, 0.0030f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true},
	//Bathroom
	{0.1715f, 1.0000f, 0.3162f, 0.2512f, 1.0000f, 1.4900f, 0.5400f, 1.0000f, 0.6531f, 0.0070f, {0.0000f, 0.0000f, 0.0000f}, 3.2734f, 0.0110f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Living room
	{0.9766f, 1.0000f, 0.3162f, 0.0010f, 1.0000f, 0.5000f, 0.1000f, 1.0000f, 0.2051f, 0.0030f, {0.0000f, 0.0000f, 0.0000f}, 0.2805f, 0.0040f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true},
	//Stone room
	{1.0000f, 1.0000f, 0.3162f, 0.7079f, 1.0000f, 2.3100f, 0.6400f, 1.0000f, 0.4411f, 0.0120f, {0.0000f, 0.0000f, 0.0000f}, 1.1003f, 0.0170f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Auditorium
	{1.0000f, 1.0000f, 0.3162f, 0.5781f, 1.0000f, 4.3200f, 0.5900f, 1.0000f, 0.4032f, 0.0200f, {0.0000f, 0.0000f, 0.0000f}, 0.7170f, 0.0300f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Concert hall
	{1.0000f, 1.0000f, 0.3162f, 0.5623f, 1.0000f, 3.9200f, 0.7000f, 1.0000f, 0.2427f, 0.0200f, {0.0000f, 0.0000f, 0.0000f}, 0.9977f, 0.0290f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Cave
	{1.0000f, 1.0000f, 0.3162f, 1.0000f, 1.0000f, 2.9100f, 1.3000f, 1.0000f, 0.5000f, 0.0150f, {0.0000f, 0.0000f, 0.0000f}, 0.7063f, 0.0220f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, false },
	//Arena
	{1.0000f, 1.0000f, 0.3162f, 0.4477f, 1.0000f, 7.2400f, 0.3300f, 1.0000f, 0.2612f, 0.0200f, {0.0000f, 0.0000f, 0.0000f}, 1.0186f, 0.0300f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Hangar
	{1.0000f, 1.0000f, 0.3162f, 0.3162f, 1.0000f, 10.0500f, 0.2300f, 1.0000f, 0.5000f, 0.0200f, {0.0000f, 0.0000f, 0.0000f}, 1.2560f, 0.0300f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Carpeted hallway
	{0.4287f, 1.0000f, 0.3162f, 0.0100f, 1.0000f, 0.3000f, 0.1000f, 1.0000f, 0.1215f, 0.0020f, {0.0000f, 0.0000f, 0.0000f}, 0.1531f, 0.0300f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Hallway
	{0.3645f, 1.0000f, 0.3162f, 0.7079f, 1.0000f, 1.4900f, 0.5900f, 1.0000f, 0.2458f, 0.0070f, {0.0000f, 0.0000f, 0.0000f}, 1.6615f, 0.0110f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Stone corridor
	{1.0000f, 1.0000f, 0.3162f, 0.7612f, 1.0000f, 2.7000f, 0.7900f, 1.0000f, 0.2472f, 0.0130f, {0.0000f, 0.0000f, 0.0000f}, 1.5758f, 0.0200f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Alley
	{1.0000f, 0.3000f, 0.3162f, 0.7328f, 1.0000f, 1.4900f, 0.8600f, 1.0000f, 0.2500f, 0.0070f, {0.0000f, 0.0000f, 0.0000f}, 0.9954f, 0.0110f, {0.0000f, 0.0000f, 0.0000f}, 0.1250f, 0.9500f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true},
	//Forest
	{1.0000f, 0.3000f, 0.3162f, 0.0224f, 1.0000f, 1.4900f, 0.5400f, 1.0000f, 0.0525f, 0.1620f, {0.0000f, 0.0000f, 0.0000f}, 0.7682f, 0.0880f, {0.0000f, 0.0000f, 0.0000f}, 0.1250f, 1.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//City
	{1.0000f, 0.5000f, 0.3162f, 0.3981f, 1.0000f, 1.4900f, 0.6700f, 1.0000f, 0.0730f, 0.0070f, {0.0000f, 0.0000f, 0.0000f}, 0.1427f, 0.0110f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true},
	//Mountains
	{1.0000f, 0.2700f, 0.3162f, 0.0562f, 1.0000f, 1.4900f, 0.2100f, 1.0000f, 0.0407f, 0.3000f, {0.0000f, 0.0000f, 0.0000f}, 0.1919f, 0.1000f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 1.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, false },
	//Quarry
	{1.0000f, 1.0000f, 0.3162f, 0.3162f, 1.0000f, 1.4900f, 0.8300f, 1.0000f, 0.0000f, 0.0610f, {0.0000f, 0.0000f, 0.0000f}, 1.7783f, 0.0250f, {0.0000f, 0.0000f, 0.0000f}, 0.1250f, 0.7000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true},
	//Plain
	{1.0000f, 0.2100f, 0.3162f, 0.1000f, 1.0000f, 1.4900f, 0.5000f, 1.0000f, 0.0585f, 0.1790f, {0.0000f, 0.0000f, 0.0000f}, 0.1089f, 0.1000f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 1.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Parking lot
	{1.0000f, 1.0000f, 0.3162f, 1.0000f, 1.0000f, 1.6500f, 1.5000f, 1.0000f, 0.2082f, 0.0080f, {0.0000f, 0.0000f, 0.0000f}, 0.2652f, 0.0120f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, false},
	//Sewer pipe
	{0.3071f, 0.8000f, 0.3162f, 0.3162f, 1.0000f, 2.8100f, 0.1400f, 1.0000f, 1.6387f, 0.0140f, {0.0000f, 0.0000f, 0.0000f}, 3.2471f, 0.0210f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 0.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Underwater
	{0.3645f, 1.0000f, 0.3162f, 0.0100f, 1.0000f, 1.4900f, 0.1000f, 1.0000f, 0.5963f, 0.0070f, {0.0000f, 0.0000f, 0.0000f}, 7.0795f, 0.0110f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 1.1800f, 0.3480f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, true },
	//Drugged
	{0.4287f, 0.5000f, 0.3162f, 1.0000f, 1.0000f, 8.3900f, 1.3900f, 1.0000f, 0.8760f, 0.0020f, {0.0000f, 0.0000f, 0.0000f}, 3.1081f, 0.0300f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 0.2500f, 1.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, false },
	//Dizzy
	{0.3645f, 0.6000f, 0.3162f, 0.6310f, 1.0000f, 17.2300f, 0.5600f, 1.0000f, 0.1392f, 0.0200f, {0.0000f, 0.0000f, 0.0000f}, 0.4937f, 0.0300f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 1.0000f, 0.8100f, 0.3100f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, false },
	//Psychotic
	{0.0625f, 0.5000f, 0.3162f, 0.8404f, 1.0000f, 7.5600f, 0.9100f, 1.0000f, 0.4864f, 0.0200f, {0.0000f, 0.0000f, 0.0000f}, 2.4378f, 0.0300f, {0.0000f, 0.0000f, 0.0000f}, 0.2500f, 0.0000f, 4.0000f, 1.0000f, 0.9943f, 5000.0000f, 250.0000f, 0.0000f, false }
};

//////////////////////////////////////////////////////////////////////////////
hlsSystem::hlsSystem()
{
	m_f_hls_system_init = 0;
	m_ll_sound_ptr = NULL;
	m_sounds_played = 0;
	m_master_volume = 1.0;
	m_pause_new = false;
	n_lls_sounds = MAX_SOUNDS_MIXED;
}

bool hlsSystem::IsActive(void)
{
	if (m_f_hls_system_init) return true;
	return false;
}

void hlsSystem::SetLLSoundQuantity(int n_sounds)
{
	if (n_sounds > MAX_SOUNDS_MIXED)
		n_sounds = MAX_SOUNDS_MIXED;

	n_lls_sounds = n_sounds;
	mprintf((1, "SNDLIB: Allow %d sounds to be mixed.\n", n_sounds));
	if (m_f_hls_system_init)
		InitSoundLib(NULL, Sound_mixer, Sound_quality, false);

}

int hlsSystem::GetLLSoundQuantity()
{
	return n_lls_sounds;
}

// Start and clean-up after the sound library
int hlsSystem::InitSoundLib(oeApplication* sos, char mixer_type, char quality, bool f_kill_sound_list)
{
	int status;
	// Turn off sound if desired
	if ((FindArg("-nosound")) || Dedicated_server)
	{
		m_ll_sound_ptr = NULL;
		return 0;
	}

	if (m_f_hls_system_init)
		KillSoundLib(f_kill_sound_list);

	// Create and initialize the low-level sound library
	if (m_ll_sound_ptr == NULL)
		m_ll_sound_ptr = new llsOpenAL();
	ASSERT(m_ll_sound_ptr);
	if (m_ll_sound_ptr == NULL) return 0;
	if (Sound_card_name[0])
		m_ll_sound_ptr->SetSoundCard(Sound_card_name);
	else
		m_ll_sound_ptr->SetSoundCard(NULL);

	status = m_ll_sound_ptr->InitSoundLib(mixer_type, sos, n_lls_sounds);
	// Initialize the high-level sound library
	// Set the flag to initialized if all was o.k.
	if (status)
	{
		m_f_hls_system_init = 1;
		SetSoundQuality(quality);
		Sound_quality = GetSoundQuality();
		Sound_mixer = GetSoundMixer();

		// invoke high level stream system
		AudioStream::InitSystem(this->m_ll_sound_ptr);
		//	set current environment
		m_cur_environment = ENVAUD_PRESET_NONE;
	}

	UpdateEnvironmentToggles();

	// All done
	return status;
}

void hlsSystem::UpdateEnvironmentToggles()
{
	if (!m_ll_sound_ptr)
		return;

	t3dEnvironmentToggles env = {};
	m_ll_sound_ptr->GetEnvironmentToggles(&env);

	env.doppler = Sound_doppler;
	env.reverb = Sound_reverb;
	m_ll_sound_ptr->SetEnvironmentToggles(&env);
}

// Kills the sound library
void hlsSystem::KillSoundLib(bool f_kill_sound_list)
{
	if (m_f_hls_system_init) 
	{
		mprintf((1, "m_sounds_played %d\n", m_sounds_played));
		// clean up stream system
		AudioStream::Shutdown();
		for (int i = 0; i < MAX_SOUND_OBJECTS; i++)
		{
			m_sound_objects[i].m_obj_type_flags = SIF_UNUSED;
			m_sound_objects[i].m_hlsound_uid = -1;
		}
		m_ll_sound_ptr->DestroySoundLib();
		m_f_hls_system_init = 0;
	}

	if (m_ll_sound_ptr)
	{
		delete m_ll_sound_ptr;
		m_ll_sound_ptr = NULL;
	}

	// This is independant of the actual sound_lib status.  It happened because sounds
	// use the manage system.
	// moved this from beginning of function because won't the data freed here screw up streaming/looping sounds?
	// before the sound library is closed?
	if (f_kill_sound_list)
	{
		for (int i = 0; i < MAX_SOUND_FILES; i++)
			SoundLoadFree(i);
	}
}

// Pause and Resume the library
// Pauses all sounds
void hlsSystem::PauseSounds(bool f_all_pause)
{
	if (!m_f_hls_system_init) return;
	m_pause_new = f_all_pause;
	AudioStream::PauseAll();
	m_ll_sound_ptr->PauseSounds();
}

// Resumes all sounds
void hlsSystem::ResumeSounds()
{
	if (!m_f_hls_system_init) return;
	m_pause_new = false;
	m_ll_sound_ptr->ResumeSounds();
	AudioStream::ResumeAll();
}

//int hlsSystem::SetSoundPos(int sound_uid, int pos) 
//{	
//	if(!m_f_hls_system_init) return -1;
//	
//	return m_ll_sound_ptr->SetSoundPos(sound_uid, pos);
//	return -1;
//}
//int hlsSystem::GetSoundPos(int sound_uid) 
//{
//	if(!m_f_hls_system_init) return -1;
//	
//	return m_ll_sound_ptr->GetSoundPos(sound_uid);
//	return -1;
//}
// 
// Stops all sound from playing
void hlsSystem::StopAllSounds()
{
	int i;
	if (!m_f_hls_system_init) return;
	for (i = 0; i < MAX_SOUND_OBJECTS; i++)
	{
		m_sound_objects[i].m_obj_type_flags = SIF_UNUSED;
		m_sound_objects[i].m_hlsound_uid = -1;
	}
	m_ll_sound_ptr->StopAllSounds();

	BeginSoundFrame(false);
	EndSoundFrame();

	SoundRenderReset();

	mprintf((0, "Stopped all sounds\n"));
}

// Code for the beginning and ending of a frame of action
// Begin_sound_frame(listener pos/orient/velocity)
// SyncSounds
// Do sound pos updates -- IF VOLUME IS LOW AND NOT FOREVER (or looping), THEN STOP SOUND
// compute echo / reverb
// indirect/direct path sounds

#include "findintersection.h"

void hlsSystem::BeginSoundFrame(bool f_in_game)
{
	bool hwsound_support;							// if this is true, sound_render_system is being used
	short* sound_room_list;							// these values are only meant for sound render system
	int i, n, counter = 0;
	pos_state listener_pos;
	const int num_samples_this_frame = (int)(Frametime * 22050);
	if (!m_f_hls_system_init) return;
	DebugBlockPrint("S ");
	//	determine if we're using hardware for sound support.
	m_ll_sound_ptr->SoundStartFrame();
	// non game sound frame, this, just updates currently playing sounds.
	// placed here to leave early and ensure any further processing involves game only.
	if (!f_in_game)
	{
		sound_room_list = NULL;
		for (i = 0, counter = 0; i < MAX_SOUND_OBJECTS; i++)
		{
			if (m_sound_objects[i].m_obj_type_flags && m_sound_objects[i].m_sound_uid != -1)
			{
				counter++;
				ASSERT(m_sound_objects[i].m_sound_uid > -1);
				if (!m_ll_sound_ptr->IsSoundInstancePlaying(m_sound_objects[i].m_sound_uid))
				{
					StopSound(i);			// stop any sounds playing.
				}
			}
		}

		goto end_beginsoundframe;		// this will handle any remaining processing
	}
	//	FROM HERE ON, WE ASSUME WE'RE IN GAME.
	hwsound_support = (sound_render_start_frame());
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
				m_ll_sound_ptr->SetGlobalReverbProperties(&EnvAudio_Presets[n]);
				m_cur_environment = n;
			}
		}
	}
	else {
		if (m_cur_environment != ENVAUD_PRESET_MOUNTAINS) {
			n = ENVAUD_PRESET_MOUNTAINS;
			m_ll_sound_ptr->SetGlobalReverbProperties(&EnvAudio_Presets[n]);
			m_cur_environment = n;
		}
	}
	// render all rooms within a certain range from player (traverse rooms through portals)
	sound_room_list = hwsound_support ? sound_render_audible_rooms(&listener_pos) : NULL;
	//	SoundQ_process();
	// if sound object is outside listener's cone of audibility, emulate the sound.
	//	if sound is audible, then determine its playback properties.
	//
	for (i = 0, counter = 0; i < MAX_SOUND_OBJECTS; i++)
	{
		if (m_sound_objects[i].m_obj_type_flags)
		{
			counter++;
			//			oldvolume = m_sound_objects[i].volume;
			//			oldpan = m_sound_objects[i].pan;
			//			mprintf((0, "SO %d playing: uid %d\n", i, m_sound_objects[i].m_sound_uid));
						// Handle high-level only sounds
			if (m_sound_objects[i].m_obj_type_flags & SIF_TOO_FAR)
			{
				if (m_sound_objects[i].m_obj_type_flags & SIF_LOOPING)
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
				if (m_sound_objects[i].m_obj_type_flags & SIF_OBJ_UPDATE)
				{
					//If object not alive, kill its sound
					object* obj_sound = ObjGet(m_sound_objects[i].m_link_info.object_handle);
					if (!obj_sound || obj_sound->type == OBJ_DUMMY)
					{
						StopSound(i);
						continue;		// Go on to next sound...
					}
				}
				// determine new properties of sound if it's still playing, its volume, other properties.
				if (Emulate3dSound(i))
				{
					//	sound is audible now.
					if (m_sound_objects[i].m_sound_uid != -1)
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
			if (!m_ll_sound_ptr->IsSoundInstancePlaying(m_sound_objects[i].m_sound_uid))
			{
				StopSound(i);
				continue;		// Go on to next sound...
			}
			if ((Sounds[m_sound_objects[i].m_sound_index].flags & SPF_LISTENER_UPDATE) &&
				!(m_sound_objects[i].m_obj_type_flags & SIF_NO_3D_EFFECTS))
			{
				if (m_sound_objects[i].m_obj_type_flags & SIF_OBJ_UPDATE)
				{
					object* obj_sound = ObjGet(m_sound_objects[i].m_link_info.object_handle);
					if (!obj_sound || obj_sound->type == OBJ_DUMMY)
					{
						StopSound(i);
						continue;		// Go on to next sound...
					}
				}
				if ((m_sound_objects[i].m_obj_type_flags & (SIF_PLAYING_3D | SIF_OBJ_UPDATE)))
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

						// handle additional 3d sound support
						sound_object* so = &m_sound_objects[i];
						if (m_ll_sound_ptr->SoundPropertySupport() && (so->m_obj_type_flags & SIF_OBJ_UPDATE)) {
							float obstruction = 0.0f;
							object* game_obj;

							// sound must be in same room as listener
							// if so, then use FVI to determine obstruction properties.
							// and then set the obstruction properties.
							if (OBJECT_OUTSIDE(Viewer_object)) continue;
							game_obj = ObjGet(so->m_link_info.object_handle);

							if (game_obj && game_obj->roomnum == Viewer_object->roomnum) {
								//	inside and same room?   go for it.
								fvi_info hit_data;
								fvi_query fq;

								//	mprintf((0, "Obstruction test.\n"));

								hit_data.hit_type[0] = HIT_WALL;
								fq.p1 = &Viewer_object->pos;
								fq.p0 = &game_obj->pos;
								fq.startroom = game_obj->roomnum;
								fq.rad = .5;
								fq.thisobjnum = so->m_link_info.object_handle & HANDLE_OBJNUM_MASK;
								fq.ignore_obj_list = NULL;
								fq.flags = 0;
								fvi_FindIntersection(&fq, &hit_data);

								if (hit_data.hit_type[0] == HIT_WALL) {
									obstruction = 1.0f;
								}
								m_ll_sound_ptr->SetSoundProperties(so->m_sound_uid, obstruction);
							}
							else {
								m_ll_sound_ptr->SetSoundProperties(so->m_sound_uid, 0.0f);
							}
						}
					}
				}
			}
		}
	}
	//mprintf((0, "BeginSoundFrame: used sound_objects %d\n", counter));
end_beginsoundframe:
	AudioStream::Frame();
	DebugBlockPrint("DS");
	mprintf_at((3, 1, 0, "HNS: %04d", counter));
}

// Plays the deffered 3d stuff
void hlsSystem::EndSoundFrame()
{
	DebugBlockPrint("X ");
	if (!m_f_hls_system_init) return;
	sound_render_end_frame();
	m_ll_sound_ptr->SoundEndFrame();
	DebugBlockPrint("DX");
}
// Allows for changes in a currently playing sound
int hlsSystem::Update2dSound(int hlsound_uid, float volume, float pan)
{
	int sound_obj_index;

	if (!m_f_hls_system_init) return -1;
	if (volume < 0.0f) volume = 0.0f;
	else if (volume > 1.0f) volume = 1.0f;
	if (pan < -1.0f) pan = -1.0f;
	else if (pan > 1.0f) pan = 1.0f;
	if (!m_f_hls_system_init) return -1;
	sound_obj_index = ValidateUniqueId(hlsound_uid);
	if (sound_obj_index == -1) return -1;
	m_ll_sound_ptr->AdjustSound(m_sound_objects[sound_obj_index].m_sound_uid, volume, pan, 22050);
	return hlsound_uid;
}
bool hlsSystem::ComputePlayInfo(int sound_obj_index, vector* virtual_pos, vector* virtual_vel, float* adjusted_volume)
{
	int sound_index;

	int sound_seg;
	int ear_seg;
	vector sound_pos;
	m_sound_objects[sound_obj_index].play_info.sample_skip_interval = 0;
	vector dir_to_sound;
	float dist;
	if (m_master_volume <= 0.0)
		return false;

	*adjusted_volume = (m_master_volume * m_sound_objects[sound_obj_index].volume_3d);
	if (*adjusted_volume <= 0.0)
	{
		*adjusted_volume = 0.0;
		return false;
	}

	sound_index = m_sound_objects[sound_obj_index].m_sound_index;
	ASSERT(sound_index >= 0 && sound_index < MAX_SOUNDS);

	if ((Sounds[m_sound_objects[sound_obj_index].m_sound_index].flags & SPF_LISTENER_UPDATE) &&
		!(m_sound_objects[sound_obj_index].m_obj_type_flags & SIF_NO_3D_EFFECTS) &&
		(m_sound_objects[sound_obj_index].m_obj_type_flags & SIF_OBJ_UPDATE))
	{	//this is an updating object sound
		object* objp = ObjGet(m_sound_objects[sound_obj_index].m_link_info.object_handle);
		if (!objp || objp->type == OBJ_DUMMY)
		{						// Couldn't find object for given handle
			//Int3();			// get Chris//removed for outside testers
			return false;
		}
		if (objp->movement_type == MT_PHYSICS || objp->movement_type == MT_WALKING)
			*virtual_vel = objp->mtype.phys_info.velocity;
		else
			*virtual_vel = Zero_vector;

		sound_pos = objp->pos;
		sound_seg = objp->roomnum;
	}
	else
	{
		*virtual_vel = Zero_vector;
		sound_pos = m_sound_objects[sound_obj_index].m_link_info.pos_info.pos;
		sound_seg = m_sound_objects[sound_obj_index].m_link_info.pos_info.segnum;
	}

	sound_seg = BOA_INDEX(sound_seg);
	ear_seg = BOA_INDEX(Viewer_object->roomnum);
	if (!BOA_IsSoundAudible(sound_seg, ear_seg))
		return false;

#ifndef MACINTOSH
	if (sound_seg != ear_seg &&
		!(sound_seg == Highest_room_index + 1 && ear_seg > Highest_room_index) &&
		!(ear_seg == Highest_room_index + 1 && sound_seg > Highest_room_index))
	{
		int cur_room = sound_seg;
		int last_room;

		do
		{
			last_room = cur_room;

			if (cur_room <= Highest_room_index && (Rooms[cur_room].flags & RF_DOOR) && (cur_room != sound_seg))
			{
				float door_position = DoorwayGetPosition(&Rooms[cur_room]);
				// Closed doors antenuate a lot
				if (door_position == 0.0)
				{
					m_sound_objects[sound_obj_index].play_info.sample_skip_interval = 4;
					*adjusted_volume *= 0.2f;
				}
				else
				{
					*adjusted_volume *= (0.6f + (0.4 * door_position));
				}
			}
			cur_room = BOA_NEXT_ROOM(cur_room, ear_seg);
			int last_portal;
			if (BOA_INDEX(last_room) == BOA_INDEX(cur_room) || cur_room == BOA_NO_PATH)
				return false;
			if (BOA_INDEX(last_room) != BOA_INDEX(cur_room))
			{
				last_portal = BOA_DetermineStartRoomPortal(last_room, NULL, cur_room, NULL);
				if (last_portal == -1)
				{
					return false;
				}
			}
			if (last_room == sound_seg)
			{
				if (cur_room == ear_seg)
				{
					dir_to_sound = sound_pos - Viewer_object->pos;
					dist = vm_NormalizeVector(&dir_to_sound);
				}
				else if ((cur_room != last_room) && (cur_room != BOA_NO_PATH))
				{
					int this_portal = BOA_DetermineStartRoomPortal(cur_room, NULL, last_room, NULL);
					dist = BOA_cost_array[cur_room][this_portal];

					if (last_room > Highest_room_index)
					{
						vector pnt = Rooms[cur_room].portals[this_portal].path_pnt;
						dist += vm_VectorDistance(&sound_pos, &pnt);
					}
					else
					{
						dist += vm_VectorDistance(&sound_pos, &Rooms[last_room].portals[last_portal].path_pnt);
					}
				}
			}
			else if (cur_room == ear_seg)
			{
				dist += BOA_cost_array[last_room][last_portal];

				if (last_room > Highest_room_index)
				{
					int this_portal = BOA_DetermineStartRoomPortal(cur_room, NULL, last_room, NULL);
					vector pnt = Rooms[cur_room].portals[this_portal].path_pnt;
					dist += vm_VectorDistance(&Viewer_object->pos, &pnt);
				}
				else
				{
					dist += vm_VectorDistance(&Viewer_object->pos, &Rooms[last_room].portals[last_portal].path_pnt);
				}
			}
			else if ((cur_room != last_room) && (cur_room != BOA_NO_PATH))
			{
				int this_portal = BOA_DetermineStartRoomPortal(cur_room, NULL, last_room, NULL);
				dist += BOA_cost_array[last_room][last_portal] + BOA_cost_array[cur_room][this_portal];
			}
		} while ((cur_room != ear_seg) &&
			(cur_room != last_room) &&
			(cur_room != BOA_NO_PATH));
		if (cur_room == BOA_NO_PATH)
		{
			*adjusted_volume = 0.0;
		}
		else if ((last_room != ear_seg) && (last_room != sound_seg))
		{
			dir_to_sound = Rooms[last_room].path_pnt - Viewer_object->pos;
			vm_NormalizeVector(&dir_to_sound);
		}
	}
	else
#endif
	{
		dir_to_sound = sound_pos - Viewer_object->pos;
		dist = vm_NormalizeVector(&dir_to_sound);
	}
	if (dist >= Sounds[sound_index].max_distance)
		return false;

#ifndef MACINTOSH
	if (*adjusted_volume <= 0.0)
	{
		*adjusted_volume = 0.0;
		return false;
	}
	if (dist == 0.0f)
	{
		dir_to_sound = Viewer_object->orient.fvec;
	}
	if ((m_sound_objects[sound_obj_index].play_info.sample_skip_interval == 0) &&
		(*adjusted_volume > 0.0f) &&
		(dir_to_sound * Viewer_object->orient.fvec < -.5))
		m_sound_objects[sound_obj_index].play_info.sample_skip_interval = 1;
#endif
	* virtual_pos = Viewer_object->pos + (dir_to_sound * dist);
	return true;

}
bool hlsSystem::Emulate3dSound(int sound_obj_index)
{
	bool f_audible;
	vector virtual_pos;
	vector virtual_vel;
	float adjusted_volume;
	f_audible = ComputePlayInfo(sound_obj_index, &virtual_pos, &virtual_vel, &adjusted_volume);

	if (f_audible)
	{
		pos_state cur_pos;
		matrix orient = Identity_matrix;
		cur_pos.velocity = &virtual_vel;
		cur_pos.position = &virtual_pos;
		cur_pos.orient = &orient;
		m_sound_objects[sound_obj_index].m_sound_uid = m_ll_sound_ptr->PlaySound3d(&m_sound_objects[sound_obj_index].play_info, m_sound_objects[sound_obj_index].m_sound_index, &cur_pos, adjusted_volume, (m_sound_objects[sound_obj_index].m_obj_type_flags & SIF_LOOPING) != 0);
		if (m_sound_objects[sound_obj_index].m_sound_uid == -1)
			f_audible = false;
	}
	else
	{
		m_sound_objects[sound_obj_index].m_sound_uid = 0;  // This is a dummy value -- it just cannot be -1 (that would mean that the low level sound system is full)
	}
	return f_audible;
}
// Functions that play a 3d sound -- includes the 2d emulation of 3d sound
int hlsSystem::Play3dSound(int sound_index, pos_state* cur_pos, float volume, int flags, float offset)
{
	return Play3dSound(sound_index, SND_PRIORITY_NORMAL, cur_pos, volume, flags, offset);
}
// Functions that play a 3d sound -- includes the 2d emulation of 3d sound
int hlsSystem::Play3dSound(int sound_index, object* cur_obj, float volume, int flags, float offset)
{
	return Play3dSound(sound_index, SND_PRIORITY_NORMAL, cur_obj, volume, flags, offset);
}
int hlsSystem::Play3dSound(int sound_index, int priority, pos_state* cur_pos, float volume, int flags, float offset)
{
	if (!m_f_hls_system_init) return -1;
	return Play3dSound(sound_index, cur_pos, NULL, priority, volume, flags, offset);
}
// Functions that play a 3d sound -- includes the 2d emulation of 3d sound
int hlsSystem::Play3dSound(int sound_index, int priority, object* cur_obj, float volume, int flags, float offset)
{
	pos_state cur_pos;
	if (!m_f_hls_system_init) return -1;
	cur_pos.position = &cur_obj->pos;
	cur_pos.orient = &cur_obj->orient;
	cur_pos.velocity = &cur_obj->mtype.phys_info.velocity;
	cur_pos.roomnum = cur_obj->roomnum;
	return Play3dSound(sound_index, &cur_pos, cur_obj, priority, volume, flags, offset);
}
// Functions that play a 3d sound -- includes the 2d emulation of 3d sound
int hlsSystem::Play3dSound(int sound_index, pos_state* cur_pos, object* cur_obj, int priority, float volume, int flags, float offset)
{
	int i;
	int sound_uid;
	int sound_obj_index;
	bool f_audible;
	if (!m_f_hls_system_init) return -1;
	if (sound_index == SOUND_NONE_INDEX) return -1;
	if (sound_index < 0) return -1;
	if (sound_index >= MAX_SOUNDS || Sounds[sound_index].used == 0)	return -1;
	// initialize sound.
	Sound_system.CheckAndForceSoundDataAlloc(sound_index);
	int sample_offset = offset * 22050.0f;
	if (sample_offset >= SoundFiles[Sounds[sound_index].sample_index].np_sample_length)
		return -1;
	// Handles exclusive sounds
	if (Sounds[sound_index].flags & SPF_PLAYS_EXCLUSIVELY)
	{
		sound_uid = m_ll_sound_ptr->IsSoundPlaying(sound_index);
		if (sound_uid != -1) return -1;
	}
	// Handles play once sounds
	if (Sounds[sound_index].flags & SPF_PLAYS_ONCE)
	{
		for (sound_obj_index = 0; sound_obj_index < MAX_SOUND_OBJECTS; sound_obj_index++)
		{
			if ((m_sound_objects[sound_obj_index].m_sound_index == sound_index) &&
				((sound_uid = m_sound_objects[sound_obj_index].m_sound_uid) != -1))
			{
				StopSound(sound_obj_index);
			}
		}
	}
	// Handles play once per object
	if ((Sounds[sound_index].flags & SPF_ONCE_PER_OBJ) && cur_obj)
	{
		for (sound_obj_index = 0; sound_obj_index < MAX_SOUND_OBJECTS; sound_obj_index++)
		{
			if ((m_sound_objects[sound_obj_index].m_sound_index == sound_index) &&
				((sound_uid = m_sound_objects[sound_obj_index].m_sound_uid) != -1))
			{
				if (m_sound_objects[sound_obj_index].m_obj_type_flags & SIF_OBJ_UPDATE)
				{
					if (m_sound_objects[sound_obj_index].m_link_info.object_handle == cur_obj->handle)
					{
						return -1;
					}
				}
			}
		}
	}
	// find free sound slot.
	for (i = 0; i < MAX_SOUND_OBJECTS; i++)
	{
		if (m_sound_objects[i].m_obj_type_flags == SIF_UNUSED) break;//
	}
	// no free slots? hmmm....
	if (i >= MAX_SOUND_OBJECTS)
	{
		mprintf((2, "HLSOUNDLIB HOLY COW: Over %d sounds trying to play(beyond max) - %s\n", MAX_SOUND_OBJECTS, Sounds[sound_index].name));
		//		Int3();
		return -1;
	}
	// Set the current sound
	m_sound_objects[i].m_sound_index = sound_index;
	// Insert the passed flags
	m_sound_objects[i].m_obj_type_flags = flags;
	// Determine if the sound is linked to the object
	if ((cur_obj != NULL) &&
		(Sounds[sound_index].flags & SPF_LISTENER_UPDATE) &&
		(Sounds[sound_index].flags & SPF_OBJ_UPDATE))
	{
		m_sound_objects[i].m_obj_type_flags |= SIF_OBJ_UPDATE;
		m_sound_objects[i].m_link_info.object_handle = cur_obj->handle;
	}
	else
	{
		m_sound_objects[i].m_link_info.pos_info.pos = *cur_pos->position;
		m_sound_objects[i].m_link_info.pos_info.orient = *cur_pos->orient;
		m_sound_objects[i].m_link_info.pos_info.segnum = cur_pos->roomnum;
		// NOTE: Velocity is zero for all non object-linked sounds
	}
	m_sound_objects[i].volume_3d = volume;
	//	m_sound_objects[i].m_sound_uid = m_ll_sound_ptr->PlaySound3d(sound_index, cur_pos, 22050);
		// Increment the total number of sounds played
	m_sounds_played++;
	// Initialize the play information to nice values
	memset(&m_sound_objects[i].play_info, 0, sizeof(play_information));
	m_sound_objects[i].play_info.samples_per_22khz_sample = 1.0;
	m_sound_objects[i].play_info.sample_skip_interval = 0;
	m_sound_objects[i].play_info.priority = priority;		// Set sound's priority rating
	m_sound_objects[i].m_hlsound_uid = MakeUniqueId(i);
	if (Sounds[sound_index].flags & SPF_LOOPED)
		m_sound_objects[i].m_obj_type_flags |= SIF_LOOPING;
	if (!m_pause_new)
	{
		f_audible = Emulate3dSound(i);
		if ((!f_audible) && (!(m_sound_objects[i].m_obj_type_flags & SIF_LOOPING)))
		{
			StopSound(i, SKT_STOP_IMMEDIATELY);
			return -1;
		}
	}
	else
	{
		f_audible = false;
		m_sound_objects[i].m_sound_uid = -1;
	}
	if (m_sound_objects[i].m_sound_uid == -1)
	{
		m_sound_objects[i].m_obj_type_flags |= SIF_TOO_FAR;
		// mprintf((0, "LLSound full 2\n"));
	}
	else if (!f_audible)
	{
		m_sound_objects[i].m_obj_type_flags |= SIF_TOO_FAR;
	}
	m_sound_objects[i].m_obj_type_flags |= SIF_PLAYING_3D;
	if (m_pause_new)
	{
		PauseSounds(true);
	}

	return m_sound_objects[i].m_hlsound_uid;
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
	if (!m_f_hls_system_init) return -1;
	if (sound_index == SOUND_NONE_INDEX) return -1;
	if (m_pause_new) return -1;
	if (sound_index < 0)
	{
		return -1;
	}
	Sound_system.CheckAndForceSoundDataAlloc(sound_index);
	// Handles exclusive sounds
	if (Sounds[sound_index].flags & SPF_PLAYS_EXCLUSIVELY)
	{
		sound_uid = m_ll_sound_ptr->IsSoundPlaying(sound_index);
		if (sound_uid != -1) return -1;
	}
	// Handles play once sounds
	if (Sounds[sound_index].flags & SPF_PLAYS_ONCE)
	{
		for (sound_obj_index = 0; sound_obj_index < MAX_SOUND_OBJECTS; sound_obj_index++)
		{
			if ((m_sound_objects[sound_obj_index].m_sound_index == sound_index) &&
				((sound_uid = m_sound_objects[sound_obj_index].m_sound_uid) != -1))
			{
				StopSound(sound_obj_index);
			}
		}
	}
	for (i = 0; i < MAX_SOUND_OBJECTS; i++)
	{
		if (m_sound_objects[i].m_obj_type_flags == SIF_UNUSED) break;

	}
	//	mprintf((0, "HL %d\n", i));
	//	ASSERT(i < MAX_SOUND_OBJECTS);
	if (i >= MAX_SOUND_OBJECTS)
	{
		mprintf((3, "Play2DSound: Max Sounds Objects used\n"));
		//		Int3();
		return -1;
	}
	// Increment the total number of sounds played
	m_sounds_played++;
	// Initialize the play information to nice values
	memset(&m_sound_objects[i].play_info, 0, sizeof(play_information));
#ifndef MACINTOSH
	m_sound_objects[i].play_info.samples_per_22khz_sample = 1.0;
#endif
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
	if (m_sound_objects[i].m_sound_uid == -1)
	{
		m_sound_objects[i].m_obj_type_flags = SIF_UNUSED;
		m_sound_objects[i].m_hlsound_uid = -1;
		mprintf((1, "Play2DSound: $%d Unplayed\n", i));
		return -1;
	}

	m_sound_objects[i].m_obj_type_flags |= SIF_PLAYING_2D;

	if (Sounds[m_sound_objects[i].m_sound_index].flags & SPF_LOOPED)
		m_sound_objects[i].m_obj_type_flags |= SIF_LOOPING;
	return m_sound_objects[i].m_hlsound_uid;
}
int hlsSystem::PlayStream(int unique_handle, void* data, int size, int stream_format, float volume, void* stream_callback(void* user_data, int handle, int* size))
{
	int i = 0;
	ASSERT(stream_format == SIF_STREAMING_8_M || stream_format == SIF_STREAMING_16_M ||
		stream_format == SIF_STREAMING_8_S || stream_format == SIF_STREAMING_16_S);
	if (!m_f_hls_system_init)
		return -1;
	if (data == NULL || size <= 0)
		return -1;
	for (i = 0; i < MAX_SOUND_OBJECTS; i++)
	{
		if (m_sound_objects[i].m_obj_type_flags == SIF_UNUSED) break;
	}
	if (i >= MAX_SOUND_OBJECTS)
	{
		mprintf((2, "PlayStream:Max Sounds Objects\n"));
		//		Int3();
		return -1;
	}
	// Increment the total number of sounds played
	m_sounds_played++;
	// Initialize the play information to nice values
	memset(&m_sound_objects[i].play_info, 0, sizeof(play_information));
#ifndef MACINTOSH
	m_sound_objects[i].play_info.samples_per_22khz_sample = 1.0;
	m_sound_objects[i].play_info.left_volume = m_sound_objects[i].play_info.right_volume = volume * m_master_volume;
#endif
	m_sound_objects[i].play_info.m_stream_cback = stream_callback;
	m_sound_objects[i].play_info.m_stream_data = data;
	m_sound_objects[i].play_info.m_stream_format = stream_format;
	m_sound_objects[i].play_info.m_stream_size = size;
	m_sound_objects[i].play_info.m_stream_handle = unique_handle;
	m_sound_objects[i].play_info.m_stream_bufsize = size;
	m_sound_objects[i].m_hlsound_uid = MakeUniqueId(i);
	m_sound_objects[i].m_sound_index = -1;

	m_sound_objects[i].m_sound_uid = m_ll_sound_ptr->PlayStream(&m_sound_objects[i].play_info);
	//ASSERT(m_sound_objects[i].m_sound_uid != -1);
	if (m_sound_objects[i].m_sound_uid == -1)
	{
		m_sound_objects[i].m_obj_type_flags = SIF_UNUSED;
		m_sound_objects[i].m_hlsound_uid = -1;
		mprintf((2, "LLSound full 1\n"));
		return -1;
	}

	m_sound_objects[i].m_obj_type_flags |= SIF_PLAYING_2D | stream_format;
	if (m_pause_new)
	{
		PauseSounds(true);
	}
	return m_sound_objects[i].m_hlsound_uid;
}
// Stop a looping sound and plays end snipit
void hlsSystem::StopSoundLooping(int hlsound_uid)
{
	int sound_object_index;
	if (m_f_hls_system_init < 1) return;	//DAJ -1FIX
	sound_object_index = ValidateUniqueId(hlsound_uid);
	if (sound_object_index == -1)
		return;
	StopSound(sound_object_index, SKT_STOP_AFTER_LOOP);
}
void hlsSystem::StopSoundImmediate(int hlsound_uid)
{
	int sound_object_index;
	if (m_f_hls_system_init < 1) return;	//DAJ -1FIX
	sound_object_index = ValidateUniqueId(hlsound_uid);
	if (sound_object_index == -1)
		return;
	StopSound(sound_object_index, SKT_STOP_IMMEDIATELY);
}
// Forcefully ends a sound
void hlsSystem::StopSound(int sound_obj_index, unsigned char f_stop_priority)
{
	if (!m_f_hls_system_init)
		return;
	m_sound_objects[sound_obj_index].m_obj_type_flags &= (~SIF_LOOPING);

	if (m_sound_objects[sound_obj_index].m_sound_uid != -1)
	{
		mprintf((1, "stopSound %d \n", m_sound_objects[sound_obj_index].m_sound_uid));
		m_ll_sound_ptr->StopSound(m_sound_objects[sound_obj_index].m_sound_uid, f_stop_priority);
	}

	if (f_stop_priority == SKT_STOP_IMMEDIATELY ||
		m_sound_objects[sound_obj_index].m_sound_uid == -1)
	{
		m_sound_objects[sound_obj_index].m_obj_type_flags = SIF_UNUSED;
		m_sound_objects[sound_obj_index].m_hlsound_uid = -1;
	}
}
//Stops all sounds attached to an object
void hlsSystem::StopObjectSound(int objhandle)
{
	int i;
	for (i = 0; i < MAX_SOUND_OBJECTS; i++)
		if (m_sound_objects[i].m_obj_type_flags & SIF_OBJ_UPDATE)
			if (m_sound_objects[i].m_link_info.object_handle == objhandle)
				StopSound(i, SKT_STOP_IMMEDIATELY);
}
//Set the volume for all the sounds attached to an object
void hlsSystem::SetVolumeObject(int objhandle, float volume)
{
	for (int i = 0; i < MAX_SOUND_OBJECTS; i++)
		if (m_sound_objects[i].m_obj_type_flags & SIF_OBJ_UPDATE)
			if (m_sound_objects[i].m_link_info.object_handle == objhandle)
				m_sound_objects[i].volume_3d = volume;
}
// Queued sound functions
// Adds a sound to a queue
void hlsSystem::Add2dSoundQueued(int q_num, int sound_index, float volume, float pan, unsigned short frequency)
{
	if (!m_f_hls_system_init) return;
}
// Cleans a queue (if sound is playing it will finish normally)
void hlsSystem::KillQueue(int q_num)
{
	if (!m_f_hls_system_init) return;
}
// Kills all sound queues
void hlsSystem::KillAllQueues()
{
	if (!m_f_hls_system_init) return;
}
// Midi play stuff
void hlsSystem::PlayMidi()
{
	if (!m_f_hls_system_init) return;
}
void hlsSystem::StopMidi()
{
	if (!m_f_hls_system_init) return;
}
void hlsSystem::PauseMidi()
{
	if (!m_f_hls_system_init) return;
}
void hlsSystem::ResumeMidi()
{
	if (!m_f_hls_system_init) return;
}
void hlsSystem::SetMidiVolume()
{
	if (!m_f_hls_system_init) return;
}
void hlsSystem::GetMidiVolume()
{
	if (!m_f_hls_system_init) return;
}
// Sets the master volume
void hlsSystem::SetMasterVolume(float volume)
{
	extern void StreamVolume(float master_volume);
	ASSERT(volume >= 0.0 && volume <= 1.0);

	if (volume == 0)
	{
		StopAllSounds();
		KillSoundLib(true);
		m_master_volume = 0;
	}
	else
	{
		if (m_master_volume == 0)
			InitSoundLib(Descent, Sound_mixer, Sound_quality, false);
	}
	m_master_volume = volume;

	StreamVolume(m_master_volume);
}

// Gets the master volume
float hlsSystem::GetMasterVolume(void)
{
	return m_master_volume;
}
inline int hlsSystem::MakeUniqueId(int sound_obj_index)
{
	return ((((int)m_sounds_played) << 12) + sound_obj_index);
}
inline int hlsSystem::ValidateUniqueId(int hl_sound_uid)
{
	ASSERT(MAX_SOUND_OBJECTS <= 0x0FFF);
	int index = hl_sound_uid & 0x0FFF;
	if (index < 0 || index >= MAX_SOUND_OBJECTS)
		return -1;


	if (hl_sound_uid == m_sound_objects[index].m_hlsound_uid)
	{
		return index;
	}
	else
	{
		return -1;
	}
}
bool hlsSystem::CheckAndForceSoundDataAlloc(int sound_index)
{
	if (!m_f_hls_system_init)
		return false;
	return m_ll_sound_ptr->CheckAndForceSoundDataAlloc(sound_index);
}
bool hlsSystem::IsSoundPlaying(int hlsound_uid)
{
	int hl_index;
	if (!m_f_hls_system_init)
		return false;
	hl_index = ValidateUniqueId(hlsound_uid);
	if (hl_index != -1)
	{
		return m_ll_sound_ptr->IsSoundInstancePlaying(m_sound_objects[hl_index].m_sound_uid);
	}
	return false;
}

bool hlsSystem::GetDopplerState(void)
{
	return false;
}

void hlsSystem::SetDopplerState(bool newstate)
{
}

bool hlsSystem::GetReverbState(void)
{
	return false;
}

void hlsSystem::SetReverbState(bool newstate)
{
}

bool hlsSystem::SetSoundQuality(char quality)
{
	if (!m_f_hls_system_init)
		return false;
	m_ll_sound_ptr->SetSoundQuality(quality);
	Sound_quality = m_ll_sound_ptr->GetSoundQuality();
	return true;
}
char hlsSystem::GetSoundQuality(void)
{
	if (!m_f_hls_system_init)
		return -1;
	return (Sound_quality = m_ll_sound_ptr->GetSoundQuality());
}
bool hlsSystem::SetSoundMixer(char mixer)
{
	if (!m_f_hls_system_init)
		return false;
	m_ll_sound_ptr->SetSoundMixer(mixer);
	Sound_mixer = m_ll_sound_ptr->GetSoundMixer();
	return true;
}
char hlsSystem::GetSoundMixer(void)
{
	if (!m_f_hls_system_init)
		return SOUND_MIXER_NONE;
	return (Sound_mixer = m_ll_sound_ptr->GetSoundMixer());
}
