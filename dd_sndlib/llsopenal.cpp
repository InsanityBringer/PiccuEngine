/* 
* Descent 3: Piccu Engine
* Copyright (C) 2024 SaladBadger
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
#include <string.h>
#include <stdlib.h>
#include <algorithm>
#include "mono.h"
#include "pserror.h"

#include "llsopenal.h"
#include "ddsndgeometry.h"

//Configuration
bool ConfigUseReverbs = false;

LPALGENAUXILIARYEFFECTSLOTS dalGenAuxiliaryEffectSlots;
LPALGENEFFECTS dalGenEffects;
LPALDELETEEFFECTS dalDeleteEffects;
LPALDELETEAUXILIARYEFFECTSLOTS dalDeleteAuxiliaryEffectSlots;
LPALEFFECTI dalEffecti;
LPALEFFECTF dalEffectf;
LPALEFFECTFV dalEffectfv;
LPALAUXILIARYEFFECTSLOTI dalAuxiliaryEffectSloti;
LPALBUFFERCALLBACKSOFT dalBufferCallbackSOFT;
LPALGETBUFFERPTRSOFT dalGetBufferPtrSOFT;
LPALGETBUFFER3PTRSOFT dalGetBuffer3PtrSOFT;
LPALGETBUFFERPTRVSOFT dalGetBufferPtrvSOFT;

//Hack: Maybe will fix problems with streaming system
bool streamStartedthisFrame = false;

float SoundDopplerMult = 0.6f;

//llsOpenAL implementation
void llsOpenAL::SetSoundCard(const char* name)
{
}

int llsOpenAL::InitSoundLib(char mixer_type, oeApplication* sos, unsigned char max_sounds_played)
{
	int i, numsends;
	ALCint attribs[4] = {};
	Device = alcOpenDevice(nullptr);
	//ALErrorCheck("Opening device");
	if (!Device)
	{
		mprintf((0, "OpenAL LLS failed to open a device."));
		return 0;
	}


	EffectsSupported = alcIsExtensionPresent(nullptr, "ALC_EXT_EFX") != AL_FALSE;
	if (!EffectsSupported)
		mprintf((0, "OpenAL effects extension not present!"));
	else
	{
		if (ConfigUseReverbs)
		{
			EffectsSupported = false;
		}
		else
		{
			//Just one environment available, so only one send needed
			attribs[0] = ALC_MAX_AUXILIARY_SENDS;
			attribs[1] = 1;
		}
	}

	Context = alcCreateContext(Device, attribs);
	//ALErrorCheck("Creating context");
	if (!Context)
	{
		alcCloseDevice(Device);
		mprintf((0, "OpenAL LLS failed to create a context."));
		return 0;
	}
	alcMakeContextCurrent(Context);

	const char* test = alGetString(AL_EXTENSIONS);
	mprintf((0, "AL_EXTENSIONS: %s\n", test));

	LoopPointsSupported = alIsExtensionPresent("AL_SOFT_loop_points") != AL_FALSE;
	if (!LoopPointsSupported)
		mprintf((0, "OpenAL Soft loop points extension not present!"));

	if (alIsExtensionPresent("AL_SOFT_callback_buffer") == AL_FALSE)
	{
		Error("Required OpenAL extension AL_SOFT_callback_buffer not present!");
	}

	dalBufferCallbackSOFT = (LPALBUFFERCALLBACKSOFT)alGetProcAddress("alBufferCallbackSOFT");
	dalGetBufferPtrSOFT = (LPALGETBUFFERPTRSOFT)alGetProcAddress("alGetBufferPtrSOFT");
	dalGetBuffer3PtrSOFT = (LPALGETBUFFER3PTRSOFT)alGetProcAddress("alGetBuffer3PtrSOFT");
	dalGetBufferPtrvSOFT = (LPALGETBUFFERPTRVSOFT)alGetProcAddress("alGetBufferPtrvSOFT");

	if (!dalBufferCallbackSOFT || !dalGetBufferPtrSOFT || !dalGetBuffer3PtrSOFT || !dalGetBufferPtrvSOFT)
	{
		Error("Failed to load functions from AL_SOFT_callback_buffer!");
	}

	//Check if that one send is available, though there's no reason to assume it won't...
	alcGetIntegerv(Device, ALC_MAX_AUXILIARY_SENDS, 1, &numsends);

	if (numsends < 1)
	{
		mprintf((0, "OpenAL effects extension failed to provide any sends?"));
		EffectsSupported = false;
	}

	alDistanceModel(AL_LINEAR_DISTANCE_CLAMPED);

	NumSoundChannels = max_sounds_played;
	SoundEntries = (llsOpenALSoundEntry*)malloc(sizeof(llsOpenALSoundEntry) * NumSoundChannels);
	if (!SoundEntries)
	{
		Error("llsOpenAL::InitSoundLib: Failed to allocate SoundEntries!");
		return 0;
	}
	memset(SoundEntries, 0, sizeof(llsOpenALSoundEntry) * NumSoundChannels);

	//Create sources for all entries now
	for (i = 0; i < NumSoundChannels; i++)
	{
		alGenSources(1, &SoundEntries[i].handle);
	}
	ALErrorCheck("Creating default sources");
	for (i = 0; i < NumSoundChannels; i++)
	{
		alGenBuffers(1, &SoundEntries[i].bufferHandle);
	}
	ALErrorCheck("Creating default buffers");

	if (EffectsSupported)
	{
		//Load and validate function pointers
		dalGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
		dalDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
		dalGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
		dalDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
		dalEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
		dalEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");
		dalEffectfv = (LPALEFFECTFV)alGetProcAddress("alEffectfv");
		dalAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");

		if (!dalGenAuxiliaryEffectSlots || !dalDeleteAuxiliaryEffectSlots || !dalGenEffects || !dalDeleteEffects || !dalEffecti || !dalEffectf || !dalAuxiliaryEffectSloti)
		{
			mprintf((0, "Failed to get OpenAL effects extension function pointers."));
			EffectsSupported = false;
		}
		else
		{
			//Generate the effect and aux effect slot
			//Just one since there's only a single global environment.
			dalGenEffects(1, &EffectSlot);
			ALErrorCheck("Creating effect");
			dalGenAuxiliaryEffectSlots(1, &AuxEffectSlot);
			ALErrorCheck("Creating aux effect");

			//Make the effect an EAX reverb
			dalEffecti(EffectSlot, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB);
			ALErrorCheck("Setting effect type");
			//SetGlobalReverbProperties(0.0, 0.0, 0.1);
		}
	}

	mprintf((0, "OpenAL LLS started successfully. %d channels specified.", max_sounds_played));
	Initalized = true;

	return 1;
}

void llsOpenAL::DestroySoundLib(void)
{
	//MessageBoxA(nullptr, "Horrible hack has been shutdown", "Cursed", MB_OK);
	if (Initalized)
	{
		alcMakeContextCurrent(nullptr);
		alcDestroyContext(Context);
		alcCloseDevice(Device);

		Context = nullptr; Device = nullptr;

		if (SoundEntries)
			free(SoundEntries);

		mprintf((0, "OpenAL LLS shut down."));
		Initalized = false;
	}
}

bool llsOpenAL::LockSound(int sound_uid)
{
	return false;
}

bool llsOpenAL::UnlockSound(int sound_uid)
{
	return false;
}

bool llsOpenAL::SetSoundQuality(char quality)
{
	mprintf((0, "LLS Quality set to %d", quality));
	if (Quality == quality) return false;
	Quality = SQT_HIGH;
	return true;
}

char llsOpenAL::GetSoundQuality(void)
{
	return Quality;
}

bool llsOpenAL::SetSoundMixer(char mixer_type)
{
	return false;
}

char llsOpenAL::GetSoundMixer(void)
{
	return SOUND_MIXER_DS3D_16;
}

int llsOpenAL::PlaySound2d(play_information* play_info, int sound_index, float volume, float pan, bool f_looped)
{
	ALErrorCheck("Clearing entry error in play sound 2d.");
	if (!Initalized) return -1;
	if (SoundFiles[Sounds[sound_index].sample_index].used == 0) return -1;
	short sound_uid = FindSoundSlot(volume, play_info->priority);
	if (sound_uid < 0) return -1;

	bool looped = f_looped || (Sounds[sound_index].flags & SPF_LOOPED) != 0;

	//PutLog(LogLevel::Info, "Initializing 2D source.");
	InitSource2D(SoundEntries[sound_uid].handle, &Sounds[sound_index], volume);
	if (looped)
		alSourcei(SoundEntries[sound_uid].handle, AL_LOOPING, AL_TRUE);
	if (ALErrorCheck("Setting 2D sound source properties.")) return -1;

	play_info->left_volume = play_info->right_volume = volume;

	//Unbind the buffer so I can rewrite it:
	//TODO: Should each entry in pSound have their own buffer instead?
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, 0);
	//PutLog(LogLevel::Info, "Setting 2D source data.");
	BindBufferData(SoundEntries[sound_uid].bufferHandle, sound_index, looped);
	if (ALErrorCheck("Binding 2D sound source buffer")) return -1;
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, SoundEntries[sound_uid].bufferHandle);
	if (ALErrorCheck("Setting 2D sound source buffer")) return -1;

	//PutLog(LogLevel::Info, "Starting 2D source.");
	alSourcePlay(SoundEntries[sound_uid].handle);
	ALErrorCheck("Starting 2D source");
	NextUID++;
	SoundEntries[sound_uid].playing = true;
	SoundEntries[sound_uid].streaming = false;
	SoundEntries[sound_uid].volume = volume;
	SoundEntries[sound_uid].info = play_info;
	SoundEntries[sound_uid].soundNum = sound_index;
	SoundEntries[sound_uid].soundUID = NextUID * 256 + sound_uid;
	//PutLog(LogLevel::Info, "Starting 2D sound %s with uid %d (slot %d)", pSoundFiles[pSounds[sound_index].sample_index].name, SoundEntries[sound_uid].soundUID, sound_uid);

	NumSoundsPlaying++;
	return SoundEntries[sound_uid].soundUID;
}

int llsOpenAL::PlayStream(play_information* play_info)
{
	ALErrorCheck("Clearing entry error in play stream.");
	float peakVolume = std::max(play_info->left_volume, play_info->right_volume);
	if (!Initalized) return -1;
	short sound_uid = FindSoundSlot(peakVolume, play_info->priority);
	if (sound_uid < 0) 
		return -1;

	//PutLog(LogLevel::Info, "Starting a stream");
	InitSourceStreaming(SoundEntries[sound_uid].handle, peakVolume);

	//Generate buffers
	alGenBuffers(NUM_STREAMING_BUFFERS, SoundEntries[sound_uid].bufferHandles);
	memset(SoundEntries[sound_uid].bufferStatus, 0, sizeof(SoundEntries[sound_uid].bufferStatus)); //all ready to use
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, 0); //ensure no buffer is bound

	SoundEntries[sound_uid].streamFormat = -1;

	if (play_info->m_stream_format & SIF_STREAMING_8_M)
		SoundEntries[sound_uid].streamFormat = AL_FORMAT_MONO8;
	else if (play_info->m_stream_format & SIF_STREAMING_16_M)
		SoundEntries[sound_uid].streamFormat = AL_FORMAT_MONO16;
	else if (play_info->m_stream_format & SIF_STREAMING_8_S)
		SoundEntries[sound_uid].streamFormat = AL_FORMAT_STEREO8;
	else if (play_info->m_stream_format & SIF_STREAMING_16_S)
		SoundEntries[sound_uid].streamFormat = AL_FORMAT_STEREO16;

	if (SoundEntries[sound_uid].streamFormat == -1) return -1;

	//PutLog(LogLevel::Info, "Queueing initial data");
	alBufferData(SoundEntries[sound_uid].bufferHandles[0], SoundEntries[sound_uid].streamFormat, play_info->m_stream_data, play_info->m_stream_bufsize, 22050);
	ALErrorCheck("Creating initial stream buffer");
	alSourceQueueBuffers(SoundEntries[sound_uid].handle, 1, &SoundEntries[sound_uid].bufferHandles[0]);
	SoundEntries[sound_uid].bufferQueue[0] = 0;
	SoundEntries[sound_uid].bufferStatus[0] = true;
	ALErrorCheck("Queueing initial streaming buffer");

	//PutLog(LogLevel::Info, "Starting");
	alSourcePlay(SoundEntries[sound_uid].handle);
	ALErrorCheck("Starting streaming source");

	NextUID++;
	SoundEntries[sound_uid].playing = true;
	SoundEntries[sound_uid].streaming = true;
	SoundEntries[sound_uid].terminate = false;
	SoundEntries[sound_uid].volume = peakVolume;
	SoundEntries[sound_uid].info = play_info;
	SoundEntries[sound_uid].soundUID = NextUID * 256 + sound_uid;

	NumSoundsPlaying++;
	//PutLog(LogLevel::Info, "Starting stream with uid %d (slot %d). Format is %d. Buffer size is %d. Stream size is %d.", SoundEntries[sound_uid].soundUID, sound_uid, play_info->m_stream_format, play_info->m_stream_bufsize, play_info->m_stream_size);
	streamStartedthisFrame = true;

	return SoundEntries[sound_uid].soundUID;
}

void llsOpenAL::SetListener(pos_state* cur_pos)
{
	ALErrorCheck("Clearing entry error in listener properties.");
	//PutLog(LogLevel::Info, "Setting listener.");
	ListenerOrient = *cur_pos->orient;
	ListenerPosition = *cur_pos->position;
	ListenerVelocty = *cur_pos->velocity;
	ListenerRoomNum = cur_pos->roomnum;

	//Update listener
	ALfloat buf[6];
	buf[0] = -ListenerOrient.fvec.x; buf[1] = ListenerOrient.fvec.y; buf[2] = ListenerOrient.fvec.z;
	buf[3] = -ListenerOrient.uvec.x; buf[4] = ListenerOrient.uvec.y; buf[5] = ListenerOrient.uvec.z;
	alListener3f(AL_POSITION, -ListenerPosition.x, ListenerPosition.y, ListenerPosition.z);
	ALErrorCheck("Updating listener position");
	alListener3f(AL_VELOCITY, -ListenerVelocty.x, ListenerVelocty.y, ListenerVelocty.z);
	ALErrorCheck("Updating listener velocity");
	alListenerfv(AL_ORIENTATION, (const ALfloat*)&buf);
	ALErrorCheck("Updating listener orientation");
}

int llsOpenAL::PlaySound3d(play_information* play_info, int sound_index, pos_state* cur_pos, float master_volume, bool f_looped, float reverb)
{
	ALErrorCheck("Clearing entry error in play sound 3d.");
	if (!Initalized) return -1;
	if (SoundFiles[Sounds[sound_index].sample_index].used == 0) return -1;
	short sound_uid = FindSoundSlot(master_volume, play_info->priority);
	if (sound_uid < 0) return -1;

	//PutLog(LogLevel::Info, "Starting 3D source");

	bool looped = f_looped || (Sounds[sound_index].flags & SPF_LOOPED) != 0;

	InitSource3D(SoundEntries[sound_uid].handle, &Sounds[sound_index], cur_pos, master_volume);
	if (looped)
		alSourcei(SoundEntries[sound_uid].handle, AL_LOOPING, AL_TRUE);
	if (ALErrorCheck("Setting 3D sound source properties.")) return -1;

	play_info->left_volume = play_info->right_volume = master_volume;

	//Unbind the buffer so I can rewrite it:
	//TODO: Should each entry in pSound have their own buffer instead?
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, 0);
	BindBufferData(SoundEntries[sound_uid].bufferHandle, sound_index, looped);
	if (ALErrorCheck("Binding 3D sound source buffer.")) return -1;
	alSourcei(SoundEntries[sound_uid].handle, AL_BUFFER, SoundEntries[sound_uid].bufferHandle);
	if (ALErrorCheck("Setting 3D sound source buffer.")) return -1;

	alSourcePlay(SoundEntries[sound_uid].handle);
	ALErrorCheck("Starting 3D source.");
	NextUID++;
	SoundEntries[sound_uid].playing = true;
	SoundEntries[sound_uid].streaming = false;
	SoundEntries[sound_uid].volume = master_volume;
	SoundEntries[sound_uid].info = play_info;
	SoundEntries[sound_uid].soundNum = sound_index;
	SoundEntries[sound_uid].soundUID = NextUID * 256 + sound_uid;
	//PutLog(LogLevel::Info, "Starting 3D sound %s with uid %d (slot %d)", pSoundFiles[pSounds[sound_index].sample_index].name, SoundEntries[sound_uid].soundUID, sound_uid);

	NumSoundsPlaying++;
	return SoundEntries[sound_uid].soundUID;
}

void llsOpenAL::AdjustSound(int sound_uid, float f_volume, float f_pan, unsigned short frequency)
{
	ALErrorCheck("Clearing entry error in adjust volume.");
	int id = sound_uid & 255;
	if (!Initalized) return;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return;

	ALuint handle = SoundEntries[id].handle;
	alSourcef(handle, AL_GAIN, f_volume);
	ALErrorCheck("Adjusting sound volume.");
	//TODO: pan, frequency. Are these used?
}

void llsOpenAL::AdjustSound(int sound_uid, pos_state* cur_pos, float adjusted_volume, float reverb)
{
	ALErrorCheck("Clearing entry error in adjust sound.");
	int id = sound_uid & 255;
	if (!Initalized) return;
	//gotta trap nans because apparently sometimes objects exist at undefined locations nice
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid || isnan<float>(cur_pos->position->x)) return;

	ALuint handle = SoundEntries[id].handle;
	alSource3f(handle, AL_DIRECTION, -cur_pos->orient->fvec.x, cur_pos->orient->fvec.y, cur_pos->orient->fvec.z);
	ALErrorCheck("Adjusting sound direction.");
	alSource3f(handle, AL_VELOCITY, -cur_pos->velocity->x, cur_pos->velocity->y, cur_pos->velocity->z);
	alSource3f(handle, AL_POSITION, -cur_pos->position->x, cur_pos->position->y, cur_pos->position->z);
	if (ALErrorCheck("Adjusting sound position."))
		mprintf((0, "\t(%f %f %f) (%f %f %f)", -cur_pos->velocity->x, cur_pos->velocity->y, cur_pos->velocity->z, -cur_pos->position->x, cur_pos->position->y, cur_pos->position->z));
	alSourcef(handle, AL_GAIN, adjusted_volume);
	ALErrorCheck("Adjusting sound gain.");
}

void llsOpenAL::StopAllSounds(void)
{
	if (!Initalized) return;
	//PutLog(LogLevel::Info, "Request to stop all sounds");
	for (int i = 0; i < NumSoundChannels; i++)
	{
		StopSound(SoundEntries[i].soundUID);
	}
}

bool llsOpenAL::IsSoundInstancePlaying(int sound_uid)
{
	int id = sound_uid & 255;
	if (!Initalized) return false;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return false;
	return SoundEntries[id].playing;
}

int llsOpenAL::IsSoundPlaying(int sound_index)
{
	if (!Initalized) return -1;
	for (int i = 0; i < NumSoundChannels; i++)
	{
		if (SoundEntries[i].soundNum == sound_index && SoundEntries[i].playing)
			return SoundEntries[i].soundUID;
	}
	return -1;
}

void llsOpenAL::StopSound(int sound_uid, unsigned char f_immediately)
{
	ALErrorCheck("Clearing entry error in stop sound.");
	int id = sound_uid & 255;
	if (!Initalized) return;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return;
	ALint islooping;
	alGetSourcei(SoundEntries[id].handle, AL_LOOPING, &islooping);
	if (SoundEntries[id].playing)
	{
		if (f_immediately == 1)
		{
			alSourceStop(SoundEntries[id].handle);
			SoundCleanup(id);

			//PutLog(LogLevel::Info, "External code trying to stop sound %d (%s). Elimination mode %d.", sound_uid, pSoundFiles[pSounds[SoundEntries[id].soundNum].sample_index].name, f_immediately);
		}
		else if (islooping)
		{
			//Disable looping traits on the sound instead
			alSourcei(SoundEntries[id].handle, AL_LOOPING, AL_FALSE);
			//alSourcePlay(SoundEntries[id].handle);
		}

	}
}

// Pause all sounds/resume all sounds
void llsOpenAL::PauseSounds(void)
{
	if (!Initalized) return;
	int i;
	for (int i = 0; i < NumSoundChannels; i++)
	{
		PauseSound(i);
	}
}

void llsOpenAL::ResumeSounds(void)
{
	if (!Initalized) return;
	int i;
	for (int i = 0; i < NumSoundChannels; i++)
	{
		ResumeSound(i);
	}
}

void llsOpenAL::PauseSound(int sound_uid)
{
	int id = sound_uid & 255;
	if (!Initalized) return;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return;
	alSourcePause(SoundEntries[id].handle);
}

void llsOpenAL::ResumeSound(int sound_uid)
{
	int id = sound_uid & 255;
	if (!Initalized) return;
	if (!SoundEntries || id < 0 || id >= NumSoundChannels || SoundEntries[id].soundUID != sound_uid) return;
	alSourcePlay(SoundEntries[id].handle);
}

bool llsOpenAL::CheckAndForceSoundDataAlloc(int sound_file_index)
{
	int sound_file_index_00;
	char cVar1;

	//PutLog(LogLevel::Info, "Allocating sound file");

	sound_file_index_00 = Sounds[sound_file_index].sample_index;
	if ((sound_file_index_00 < 0) || (999 < sound_file_index_00))
	{
		return false;
	}
	if ((SoundFiles[sound_file_index_00].sample_16bit == NULL) && (SoundFiles[sound_file_index_00].sample_8bit == NULL))
	{
		//PutLog(LogLevel::Info, "Force loading sound %d (%s)", sound_file_index, pSoundFiles[sound_file_index_00].name);
		cVar1 = SoundLoadWaveFile(SoundFiles[sound_file_index_00].name, Sounds[sound_file_index].import_volume, sound_file_index_00, Quality == SQT_HIGH, true, NULL);
		return cVar1 != '\0';
	}
	return true;
}

// Begin sound frame
void llsOpenAL::SoundStartFrame(void)
{
	if (!Initalized) return;

	ALErrorCheck("Clearing entry error in sound system frame start");

	//Service streams
	for (int i = 0; i < NumSoundChannels; i++)
	{
		if (SoundEntries[i].playing && SoundEntries[i].streaming)
		{
			//PutLog(LogLevel::Info, "Servicing a stream");
			ServiceStream(i);
		}
	}
}
// End sound frame 
void llsOpenAL::SoundEndFrame(void)
{
	if (!Initalized) return;
	int i;
	ALint state;

	ALErrorCheck("Clearing entry error in sound system frame.");

	//PutLog(LogLevel::Info, "Ending sound system frame");

	//Free buffers and update playing state of all current sounds.
	for (int i = 0; i < NumSoundChannels; i++)
	{
		if (SoundEntries[i].playing)
		{
			//if (SoundEntries[i].streaming)
			//	ServiceStream(i);

			alGetSourcei(SoundEntries[i].handle, AL_SOURCE_STATE, &state);
			if (state == AL_STOPPED && (!SoundEntries[i].streaming || SoundEntries[i].terminate))
			{
				SoundCleanup(i);
				//PutLog(LogLevel::Info, "Sound %d has been stopped", SoundEntries[i].soundUID);
			}
		}
	}
}

bool llsOpenAL::SetGlobalReverbProperties(const EAX2Reverb* reverb)
{
	ALErrorCheck("Clearing entry error in reverb properties.");
	if (!EffectsSupported)
		return false;

	if (reverb == LastReverb)
		return true;

	LastReverb = reverb;

	//Make the aux effect slot use the effect
	dalAuxiliaryEffectSloti(AuxEffectSlot, AL_EFFECTSLOT_EFFECT, 0);
	ALErrorCheck("Setting aux effect slot");

#ifndef NDEBUG
	extern EAX2Reverb EnvAudio_Presets[];
	mprintf((0, "setting environment %d\n", (int)(reverb - EnvAudio_Presets)));
#endif

	if (reverb->density == 0)
		return true; //Turn off reverbs

	dalEffectf(EffectSlot, AL_EAXREVERB_DENSITY, reverb->density);
	ALErrorCheck("Setting reverb density");
	dalEffectf(EffectSlot, AL_EAXREVERB_DIFFUSION, reverb->diffusion);
	ALErrorCheck("Setting reverb diffusion");
	dalEffectf(EffectSlot, AL_EAXREVERB_GAIN, reverb->gain);
	ALErrorCheck("Setting reverb gain");
	dalEffectf(EffectSlot, AL_EAXREVERB_GAINHF, reverb->gain_hf);
	ALErrorCheck("Setting reverb gain hf");
	dalEffectf(EffectSlot, AL_EAXREVERB_GAINLF, reverb->gain_lf);
	ALErrorCheck("Setting reverb gain lf");
	dalEffectf(EffectSlot, AL_EAXREVERB_DECAY_TIME, reverb->decay_time);
	ALErrorCheck("Setting reverb decay time");
	dalEffectf(EffectSlot, AL_EAXREVERB_DECAY_HFRATIO, reverb->decay_hf_ratio);
	ALErrorCheck("Setting reverb decay hf ratio");
	dalEffectf(EffectSlot, AL_EAXREVERB_DECAY_LFRATIO, reverb->decay_lf_ratio);
	ALErrorCheck("Setting reverb decay lf ratio");
	dalEffectf(EffectSlot, AL_EAXREVERB_REFLECTIONS_GAIN, reverb->reflection_gain);
	ALErrorCheck("Setting reverb reflection gain");
	dalEffectf(EffectSlot, AL_EAXREVERB_REFLECTIONS_DELAY, reverb->reflection_delay);
	ALErrorCheck("Setting reverb reflection delay");
	dalEffectfv(EffectSlot, AL_EAXREVERB_REFLECTIONS_PAN, reverb->reflection_pan);
	ALErrorCheck("Setting reverb reflection pan");
	dalEffectf(EffectSlot, AL_EAXREVERB_LATE_REVERB_DELAY, reverb->late_reverb_delay);
	ALErrorCheck("Setting reverb late reverb delay");
	dalEffectf(EffectSlot, AL_EAXREVERB_LATE_REVERB_GAIN, reverb->late_reverb_gain);
	ALErrorCheck("Setting reverb late reverb gain");
	dalEffectfv(EffectSlot, AL_EAXREVERB_LATE_REVERB_PAN, reverb->late_reverb_pan);
	ALErrorCheck("Setting reverb late_reverb_pan");
	dalEffectf(EffectSlot, AL_EAXREVERB_ECHO_TIME, reverb->echo_time);
	ALErrorCheck("Setting reverb echo time");
	dalEffectf(EffectSlot, AL_EAXREVERB_ECHO_DEPTH, reverb->echo_depth);
	ALErrorCheck("Setting reverb echo depth");
	dalEffectf(EffectSlot, AL_EAXREVERB_MODULATION_TIME, reverb->modulation_time);
	ALErrorCheck("Setting reverb modulation time");
	dalEffectf(EffectSlot, AL_EAXREVERB_MODULATION_DEPTH, reverb->modulation_depth);
	ALErrorCheck("Setting reverb modulation depth");
	dalEffectf(EffectSlot, AL_EAXREVERB_AIR_ABSORPTION_GAINHF, reverb->air_absorption_gain_hf);
	ALErrorCheck("Setting reverb air absorption gain hf");
	dalEffectf(EffectSlot, AL_EAXREVERB_HFREFERENCE, reverb->hf_reference);
	ALErrorCheck("Setting reverb hf reference");
	dalEffectf(EffectSlot, AL_EAXREVERB_LFREFERENCE, reverb->lf_reference);
	ALErrorCheck("Setting reverb lf reference");
	dalEffectf(EffectSlot, AL_EAXREVERB_ROOM_ROLLOFF_FACTOR, reverb->room_rolloff_factor);
	ALErrorCheck("Setting reverb room rolloff factor");
	dalEffecti(EffectSlot, AL_EAXREVERB_DECAY_HFLIMIT, reverb->decay_hf_limit);
	ALErrorCheck("Setting reverb max decay limit");

	//Make the aux effect slot use the effect
	dalAuxiliaryEffectSloti(AuxEffectSlot, AL_EFFECTSLOT_EFFECT, EffectSlot);
	ALErrorCheck("Setting aux effect slot");

	return true;
}

void llsOpenAL::SetEnvironmentValues(const t3dEnvironmentValues* env)
{
}

void llsOpenAL::GetEnvironmentValues(t3dEnvironmentValues* env)
{
}

void llsOpenAL::SetEnvironmentToggles(const t3dEnvironmentToggles* env)
{
}

void llsOpenAL::GetEnvironmentToggles(t3dEnvironmentToggles* env)
{
	env->doppler = true;
	env->geometry = false;
	env->flags = ENV3DVALF_DOPPLER;
}

void llsOpenAL::InitMovieBuffer(bool is16bit, int samplerate, bool stereo, llsMovieCallback callback)
{
	MovieSampleRate = samplerate;
	alGenSources(1, &MovieSourceName);
	ALErrorCheck("Creating movie source");
	alGenBuffers(1, &MovieBufferName);
	ALErrorCheck("Creating movie buffer");
	InitSourceStreaming(MovieSourceName, .5);
	ALErrorCheck("Setting movie source properties");

	if (!is16bit)
	{
		if (stereo)
			MovieSoundFormat = AL_FORMAT_STEREO8;
		else
			MovieSoundFormat = AL_FORMAT_MONO8;
	}
	else
	{
		if (stereo)
			MovieSoundFormat = AL_FORMAT_STEREO16;
		else
			MovieSoundFormat = AL_FORMAT_MONO16;
	}

	dalBufferCallbackSOFT(MovieBufferName, MovieSoundFormat, samplerate, (ALBUFFERCALLBACKTYPESOFT)callback, nullptr);
	ALErrorCheck("Setting movie buffer callback");
	alSourcei(MovieSourceName, AL_BUFFER, MovieBufferName);
	ALErrorCheck("Setting movie source buffer");
}

void llsOpenAL::KillMovieBuffer()
{
	if (!MovieSourceName)
		return;

	ALint status;
	alGetSourcei(MovieSourceName, AL_SOURCE_STATE, &status);
	if (status != AL_STOPPED)
		alSourceStop(MovieSourceName);
	//Buffer must be detached before deleting
	alSourcei(MovieSourceName, AL_BUFFER, 0);
	ALErrorCheck("Stopping source");

	alDeleteBuffers(1, &MovieBufferName);
	ALErrorCheck("Destroying movie buffers");

	alDeleteSources(1, &MovieSourceName);
	ALErrorCheck("Destroying movie source");

	MovieStarted = false;
}

void llsOpenAL::DequeueMovieBuffers()
{
}

void llsOpenAL::QueueMovieBuffer(int length, void* data)
{
	/*DequeueMovieBuffers();

	alBufferData(MovieBufferNames[MovieBufferQHead], MovieSoundFormat, data, length, MovieSampleRate);
	//ALint status;
	alSourceQueueBuffers(MovieSourceName, 1, &MovieBufferNames[MovieBufferQHead]);
	//alGetSourcei(MovieSourceName, AL_SOURCE_STATE, &status);
	if (!MovieStarted)
	{
		alSourcePlay(MovieSourceName);
		MovieStarted = true;
	}
	//ALErrorCheck("Queuing movie buffer");

	//mprintf((0, "queueing buffer %d\n", MovieBufferQHead));

	MovieBufferQHead = (MovieBufferQHead + 1) % NUM_MOVIE_BUFFERS;
	//uh oh
	if (MovieBufferQHead == MovieBufferQTail)
		Int3();*/

	if (!MovieStarted)
	{
		alSourcePlay(MovieSourceName);
		MovieStarted = true;
	}
}

const char* ALErrors[4] = { "Invalid enum", "Invalid name", "Invalid operation", "Invalid value" };

bool llsOpenAL::ALErrorCheck(const char* context)
{
	int error;
	error = alGetError();
	const char* msg = "";
	if (error != AL_NO_ERROR)
	{
		if (error == AL_INVALID_ENUM)
			msg = ALErrors[0];
		else if (error == AL_INVALID_NAME)
			msg = ALErrors[1];
		else if (error == AL_INVALID_OPERATION)
			msg = ALErrors[2];
		else if (error == AL_INVALID_VALUE)
			msg = ALErrors[3];
		else
			msg = "Unknown error code";

		mprintf((0, "Error in context %s: %s.", context, msg));
		return true;
	}
	return false;
}

short llsOpenAL::FindSoundSlot(float volume, int priority)
{
	int i;
	int bestPriority = INT_MAX;
	int bestSlot = -1;
	float bestVolume = 1.0f;
	//PutLog(LogLevel::Info, "Finding a sound slot");
	//No free slots, so bump a low priorty one.
	if (NumSoundsPlaying >= NumSoundChannels)
	{
		for (i = 0; i < NumSoundChannels; i++)
		{
			//check for lower priority sound
			if (SoundEntries[i].info->priority < bestPriority)
			{
				bestSlot = i;
				bestVolume = SoundEntries[i].volume;
				bestPriority = SoundEntries[i].info->priority;
			}
			//At least attempt to get the quietest sound possible
			else if (SoundEntries[i].info->priority == bestPriority)
			{
				if (SoundEntries[i].volume < bestVolume)
				{
					bestSlot = i;
					bestVolume = SoundEntries[i].volume;
				}
			}
		}

		if (bestSlot != -1)
		{
			StopSound(SoundEntries[bestSlot].soundUID);
			//PutLog(LogLevel::Warning, "Bumping sound %d due to insufficient slots.", bestSlot);
		}
		else
		{
			mprintf((0, "Can't find sound slot and unable to bump sound."));
		}

		return bestSlot;
	}
	else //Free slots available, so find one.
	{
		for (i = 0; i < NumSoundChannels; i++)
		{
			if (!SoundEntries[i].playing)
				return i;
		}
	}
	mprintf((0, "Can't find sound slot. NumSoundsPlaying is %d.", NumSoundsPlaying));
	return -1;
}

void llsOpenAL::InitSource2D(uint32_t handle, sound_info* soundInfo, float volume)
{
	ALErrorCheck("Clearing entry error in 2d source properties.");
	alSourcei(handle, AL_SOURCE_RELATIVE, AL_TRUE); //should glue source to listener pos
	alSourcef(handle, AL_ROLLOFF_FACTOR, 0.0f);
	alSource3f(handle, AL_DIRECTION, 0.f, 0.f, 0.f);
	alSource3f(handle, AL_VELOCITY, 0.f, 0.f, 0.f);
	alSource3f(handle, AL_POSITION, 0.f, 0.f, 0.f);
	alSourcef(handle, AL_MAX_GAIN, 1.f);
	alSourcef(handle, AL_GAIN, volume);
	alSourcef(handle, AL_PITCH, 1.f);
	alSourcef(handle, AL_DOPPLER_FACTOR, 0.f);

	alSourcef(handle, AL_CONE_INNER_ANGLE, 360.0f);
	alSourcef(handle, AL_CONE_OUTER_ANGLE, 360.0f);

	alSourcei(handle, AL_LOOPING, AL_FALSE);
}

void llsOpenAL::InitSourceStreaming(uint32_t handle, float volume)
{
	ALErrorCheck("Clearing entry error in streaming source properties.");
	alSourcei(handle, AL_SOURCE_RELATIVE, AL_TRUE); //should glue source to listener pos
	alSourcef(handle, AL_ROLLOFF_FACTOR, 0.0f);
	alSource3f(handle, AL_DIRECTION, 0.f, 0.f, 0.f);
	alSource3f(handle, AL_VELOCITY, 0.f, 0.f, 0.f);
	alSource3f(handle, AL_POSITION, 0.f, 0.f, 0.f);
	alSourcef(handle, AL_MAX_GAIN, 1.f);
	alSourcef(handle, AL_GAIN, volume);
	alSourcef(handle, AL_PITCH, 1.f);
	alSourcef(handle, AL_DOPPLER_FACTOR, 0.f);

	alSourcef(handle, AL_CONE_INNER_ANGLE, 360.0f);
	alSourcef(handle, AL_CONE_OUTER_ANGLE, 360.0f);

	alSourcei(handle, AL_LOOPING, AL_FALSE);
}

void llsOpenAL::InitSource3D(uint32_t handle, sound_info* soundInfo, pos_state* posInfo, float volume)
{
	ALErrorCheck("Clearing entry error in 3d source properties.");
	alSourcei(handle, AL_SOURCE_RELATIVE, AL_FALSE);
	alSourcef(handle, AL_ROLLOFF_FACTOR, 1.0f);
	alSource3f(handle, AL_DIRECTION, -posInfo->orient->fvec.x, posInfo->orient->fvec.y, posInfo->orient->fvec.z);
	alSource3f(handle, AL_VELOCITY, -posInfo->velocity->x, posInfo->velocity->y, posInfo->velocity->z);
	alSource3f(handle, AL_POSITION, -posInfo->position->x, posInfo->position->y, posInfo->position->z);
	ALErrorCheck("Setting 3D sound source position.");
	alSourcef(handle, AL_MAX_GAIN, 1.f);
	alSourcef(handle, AL_GAIN, volume);
	ALErrorCheck("Setting 3D sound source volume.");
	alSourcef(handle, AL_PITCH, 1.f);
	alSourcef(handle, AL_DOPPLER_FACTOR, SoundDopplerMult);

	//PutLog(LogLevel::Info, "Starting 3d sound at %f %f %f", posInfo->position->x, posInfo->position->y, posInfo->position->z);

	if (soundInfo->flags & SPF_USE_CONE)
	{
		alSourcef(handle, AL_CONE_OUTER_GAIN, soundInfo->outer_cone_volume);
		alSourcef(handle, AL_CONE_INNER_ANGLE, soundInfo->inner_cone_angle);
		alSourcef(handle, AL_CONE_OUTER_ANGLE, soundInfo->outer_cone_angle);
		ALErrorCheck("Setting 3D sound source count.");
	}
	else
	{
		alSourcef(handle, AL_CONE_INNER_ANGLE, 360.0f);
		alSourcef(handle, AL_CONE_OUTER_ANGLE, 360.0f);
	}

	alSourcef(handle, AL_REFERENCE_DISTANCE, soundInfo->min_distance);
	alSourcef(handle, AL_MAX_DISTANCE, soundInfo->max_distance);
	ALErrorCheck("Setting 3D sound source distance.");

	alSourcei(handle, AL_LOOPING, AL_FALSE);

	if (EffectsSupported)
		alSource3i(handle, AL_AUXILIARY_SEND_FILTER, AuxEffectSlot, 0, NULL);
	ALErrorCheck("Setting 3D sound source effect.");
}

void llsOpenAL::BindBufferData(uint32_t handle, int sound_index, bool looped)
{
	ALenum fmt = Quality == SQT_HIGH ? AL_FORMAT_MONO16 : AL_FORMAT_MONO8;
	int bpp = (Quality == SQT_HIGH ? 2 : 1);
	void* ptr = Quality == SQT_HIGH ? (ALvoid*)SoundFiles[Sounds[sound_index].sample_index].sample_16bit : (ALvoid*)SoundFiles[Sounds[sound_index].sample_index].sample_8bit;
	int len = bpp * SoundFiles[Sounds[sound_index].sample_index].np_sample_length;

	//PutLog(LogLevel::Info, "Buffering sound id %d (%s)", sound_index, pSoundFiles[pSounds[sound_index].sample_index].name);
	if (!ptr)
	{
		mprintf((0, "Tried to start sound %s but got nullptr.", SoundFiles[Sounds[sound_index].sample_index].name));
		return;
	}
	alBufferData(handle, fmt, ptr, len, 22050);

	SoundFiles[Sounds[sound_index].sample_index].use_count++;
	ALErrorCheck("Binding buffer data");


	if (looped && LoopPointsSupported)
	{
		ALint loopPoints[2];
		loopPoints[0] = Sounds[sound_index].loop_start;
		loopPoints[1] = Sounds[sound_index].loop_end;

		//uh
		if (bpp == 1)
		{
			loopPoints[0] >>= 1;
			loopPoints[1] >>= 1;
		}

		//bounds check
		if (loopPoints[0] > len) loopPoints[0] = 0;
		if (loopPoints[1] > len) loopPoints[1] = len;

		alBufferiv(handle, AL_LOOP_POINTS_SOFT, &loopPoints[0]);
		ALErrorCheck("Setting buffer loop points");
	}
}

void llsOpenAL::SoundCleanup(int soundID)
{
	SoundEntries[soundID].playing = false;
	SoundFiles[Sounds[SoundEntries[soundID].soundNum].sample_index].use_count--;
	//PutLog(LogLevel::Info, "Sound %d (%s) has been stopped", SoundEntries[i].soundUID, pSoundFiles[pSounds[SoundEntries[i].soundNum].sample_index].name);
	if (NumSoundsPlaying > 0)
		NumSoundsPlaying--;
	else
		mprintf((0, "LLS Bookkeeping failed, NumSoundsPlaying went negative"));

	//Unbind the buffer for later use
	alSourcei(SoundEntries[soundID].handle, AL_BUFFER, 0);
	//Clear the sound's send filter, if it has one. 
	if (EffectsSupported)
	{
		alSource3i(SoundEntries[soundID].handle, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, NULL);
		ALErrorCheck("Clearing source send filter");
	}

	//Streaming sounds need to clean up all their buffers
	if (SoundEntries[soundID].streaming)
	{
		ALint numProcessedBuffers, numQueuedBuffers;
		ALuint dequeueList[NUM_STREAMING_BUFFERS];
		int i;

		alSourceStop(SoundEntries[soundID].handle); //ensure its actually stopped I guess
		alGetSourcei(SoundEntries[soundID].handle, AL_BUFFERS_QUEUED, &numProcessedBuffers);
		if (numProcessedBuffers > 0)
		{
			//back up current buffers in buffer queue
			for (i = 0; i < NUM_STREAMING_BUFFERS - numProcessedBuffers; i++)
			{
				dequeueList[i] = SoundEntries[soundID].bufferHandles[SoundEntries[soundID].bufferQueue[i]];
			}

			alSourceUnqueueBuffers(SoundEntries[soundID].handle, numProcessedBuffers, SoundEntries[soundID].bufferQueue);
		}
		ALErrorCheck("Dequeueing ended stream buffers");

		alDeleteBuffers(NUM_STREAMING_BUFFERS, SoundEntries[soundID].bufferHandles);
		ALErrorCheck("Destroying stream buffers");
		SoundEntries[soundID].streaming = SoundEntries[soundID].terminate = false;
		for (i = 0; i < NUM_STREAMING_BUFFERS; i++)
		{
			SoundEntries[soundID].bufferStatus[i] = false;
		}
	}
}

void llsOpenAL::ServiceStream(int soundID)
{
	ALint numProcessedBuffers, numQueuedBuffers;
	ALint position;
	ALuint dequeueList[NUM_STREAMING_BUFFERS];
	int i;

	if (SoundEntries[soundID].terminate)
		return;

	if (streamStartedthisFrame)
	{
		streamStartedthisFrame = false;
		return;
	}

	alGetSourcei(SoundEntries[soundID].handle, AL_BUFFERS_PROCESSED, &numProcessedBuffers);
	if (numProcessedBuffers > 0)
	{
		//mark freed buffers as usable again
		for (i = 0; i < numProcessedBuffers; i++)
		{
			dequeueList[i] = SoundEntries[soundID].bufferHandles[SoundEntries[soundID].bufferQueue[i]];
			SoundEntries[soundID].bufferStatus[SoundEntries[soundID].bufferQueue[i]] = false;
		}
		//remove the unqueued buffers from the list by moving further buffer entries over them. 
		for (i = 0; i < NUM_STREAMING_BUFFERS - numProcessedBuffers; i++)
		{
			SoundEntries[soundID].bufferQueue[i] = SoundEntries[soundID].bufferQueue[i + numProcessedBuffers];
		}

		alSourceUnqueueBuffers(SoundEntries[soundID].handle, numProcessedBuffers, dequeueList);
		//alGetSourcei(SoundEntries[soundID].handle, AL_BUFFERS_QUEUED, &numQueuedBuffers);
		//PutLog(LogLevel::Info, "%d buffers dequeued. %d currently queued. [%d %d %d]", numProcessedBuffers, numQueuedBuffers, SoundEntries[soundID].bufferQueue[0], SoundEntries[soundID].bufferQueue[1], SoundEntries[soundID].bufferQueue[2]);
	}
	ALErrorCheck("Dequeueing buffers");

	alGetSourcei(SoundEntries[soundID].handle, AL_BUFFERS_QUEUED, &numQueuedBuffers);
	alGetSourcei(SoundEntries[soundID].handle, AL_BYTE_OFFSET, &position);
	if (numQueuedBuffers < NUM_STREAMING_BUFFERS) //queue up a new one
	{
		int newSlot = -1;
		int size = SoundEntries[soundID].info->m_stream_size;
		void* data;
		//Find first slot that's available
		for (i = 0; i < NUM_STREAMING_BUFFERS; i++)
		{
			if (!SoundEntries[soundID].bufferStatus[i])
			{
				newSlot = i;
				break;
			}
		}

		if (newSlot == -1)
		{
			//eh, this needs to be reconsidered some. Can be race conditions here since the song is still playing before stuff is updated. 
			return;
		}

		//Get available data and queue it
		data = SoundEntries[soundID].info->m_stream_cback(SoundEntries[soundID].info->user_data, SoundEntries[soundID].info->m_stream_handle, &size);
		//mprintf((0, "Stream %d got a buffer of size %d\n", soundID, size));

		SoundEntries[soundID].info->m_stream_size = size;
		SoundEntries[soundID].info->m_stream_data = data;

		if (!data)
		{
			//PutLog(LogLevel::Error, "Tried to queue streaming buffer, but got nullptr.");
			SoundEntries[soundID].info->m_stream_size = 0;
			//When this occurs, the stream needs to terminate, the streaming service will then start a new stream. 
			SoundEntries[soundID].terminate = true;
			return;
		}

		if (size != 0 && data != nullptr)
		{
			alBufferData(SoundEntries[soundID].bufferHandles[newSlot], SoundEntries[soundID].streamFormat, data, size, 22050);
			ALErrorCheck("Creating stream buffer");
			alSourceQueueBuffers(SoundEntries[soundID].handle, 1, &SoundEntries[soundID].bufferHandles[newSlot]);
			SoundEntries[soundID].bufferQueue[numQueuedBuffers] = newSlot;
			SoundEntries[soundID].bufferStatus[newSlot] = true;
			ALErrorCheck("Queueing buffer");


			SoundEntries[soundID].info->m_samples_played += size;
			//PutLog(LogLevel::Info, "Buffer queued to %d.", newSlot);
		}
		else
		{
			mprintf((0, "Slots available to queue but callback returned no bytes."));
		}

		ALint state;
		alGetSourcei(SoundEntries[soundID].handle, AL_SOURCE_STATE, &state);
		if (state != AL_PLAYING)
		{
			alSourcePlay(SoundEntries[soundID].handle);
		}
	}
}


//null geometry system, it's sad. thanks creative. 
llsGeometry::llsGeometry()
{
}


// specify a sound library to associate geometry with
bool llsGeometry::Init(llsSystem* snd_sys)
{
	return false;
}


// closes low level geometry system.
void llsGeometry::Shutdown()
{
}


void llsGeometry::StartFrame()
{
}


void llsGeometry::EndFrame()
{
}



// polygon lists

// marks beginning of a list of polygons to render
//	-1 group if non cached (user doesn't want to reuse this.
void llsGeometry::StartPolygonGroup(int group)
{
}


// ends a list of polygons to render.
void llsGeometry::EndPolygonGroup(int group)
{
}


// renders a group.
void llsGeometry::RenderGroup(int group)
{
}


void llsGeometry::Clear()
{
}


// primatives
// 4 verts here.
void llsGeometry::AddQuad(unsigned tag, vector** verts)
{
}


// 3 verts here.
void llsGeometry::AddTriangle(unsigned tag, vector** verts)
{
}


void llsGeometry::AddPoly(int nv, vector** verts, unsigned tag, tSoundMaterial material)
{
}


// values MUST be from 0 to 1 for gain and highfreq.
void llsGeometry::CreateMaterial(tSoundMaterial material, float transmit_gain, float transmit_highfreq, float reflect_gain, float reflect_highfreq)
{
}


void llsGeometry::DestroyMaterial(tSoundMaterial material)
{
}


