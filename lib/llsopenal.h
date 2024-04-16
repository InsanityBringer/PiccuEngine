#include <stdint.h>
#include "ssl_lib.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>

//OpenAL LLS system implementation

//This is the amount of buffers that will be queued in OpenAL for a streaming audio source. This needs to be high enough to avoid starvation,
//but not too high to add noticable latency to stream changes. 
#define NUM_STREAMING_BUFFERS 3

struct llsOpenALSoundEntry
{
	uint32_t handle, bufferHandle;
	int soundNum;
	int soundUID;
	float volume;
	//this can be figured out by asking OpenAL, but this is effectively a race condition since the mixer is running on another thread. 
	//This will be updated in EndSoundFrame, which is still a race condition but I don't want it happening on every call to IsSoundPlaying
	bool playing;
	//True if the source is streaming, since streaming sources need to be kept fed and also closed differently.
	bool streaming;
	//True if the source is streaming and ready to be terminated.
	bool terminate;
	play_information* info;

	//Streaming buffer handles. These are the actual handles that will be queued.
	ALuint bufferHandles[NUM_STREAMING_BUFFERS];
	//The state of each buffer, true if the buffer is currently queued and shouldn't be reused. 
	bool bufferStatus[NUM_STREAMING_BUFFERS];

	//Buffer queue, list of handles currently in use.
	ALuint bufferQueue[NUM_STREAMING_BUFFERS];
	uint32_t streamFormat;

	t3dEnvironmentValues envValues;
};


class llsOpenAL : public llsSystem
{
	vector ListenerPosition;
	vector ListenerVelocty;
	matrix ListenerOrient;
	int ListenerRoomNum;

	bool Initalized;
	bool LoopPointsSupported, EffectsSupported;
	char Quality;

	int NumSoundChannels;
	int NumSoundsPlaying;
	int NextUID;
	llsOpenALSoundEntry* SoundEntries;

	//OpenAL state
	ALCcontext* Context;
	ALCdevice* Device;

	ALuint AuxEffectSlot, EffectSlot;

	float Volume, Damping, Decay;

	bool ALErrorCheck(const char* context);

	short FindSoundSlot(float volume, int priority);

	void InitSource2D(uint32_t handle, sound_info* soundInfo, float volume);
	void InitSourceStreaming(uint32_t handle, float volume);
	void InitSource3D(uint32_t handle, sound_info* soundInfo, pos_state* posInfo, float volume);
	void BindBufferData(uint32_t handle, int sound_index, bool looped);
	void SoundCleanup(int soundID);

	void ServiceStream(int soundID);

public:
	llsOpenAL()
	{
		Initalized = false;
		LoopPointsSupported = EffectsSupported = false;
		Quality = SQT_HIGH;
		NumSoundChannels = 0;
		NumSoundsPlaying = 0;
		NextUID = 0;
		SoundEntries = nullptr;

		vm_MakeZero(&ListenerPosition);
		vm_MakeZero(&ListenerVelocty);
		ListenerOrient = IDENTITY_MATRIX;
		ListenerRoomNum = 0;

		Context = nullptr; Device = nullptr;

		AuxEffectSlot = EffectSlot = 0;
		Volume = Damping = Decay = 0;
	}
	// may be called before init (must be to be valid, the card passed here will be initialized in InitSoundLib)
	void SetSoundCard(const char* name) override;

	// Starts the sound library, maybe have it send back some information -- 3d support?
	int InitSoundLib(char mixer_type, oeApplication* sos, unsigned char max_sounds_played) override;
	// Cleans up after the Sound Library
	void DestroySoundLib(void) override;

	// Locks and unlocks sounds (used when changing play_info data)
	bool LockSound(int sound_uid) override;
	bool UnlockSound(int sound_uid) override;

	bool SetSoundQuality(char quality) override;
	char GetSoundQuality(void) override;
	bool SetSoundMixer(char mixer_type) override;
	char GetSoundMixer(void) override;

	// Plays a 2d sound
	int PlaySound2d(play_information* play_info, int sound_index, float volume, float pan, bool f_looped) override;
	int PlayStream(play_information* play_info) override;

	void SetListener(pos_state* cur_pos) override;
	int PlaySound3d(play_information* play_info, int sound_index, pos_state* cur_pos, float master_volume, bool f_looped, float reverb = 0.5f) override;
	void AdjustSound(int sound_uid, float f_volume, float f_pan, unsigned short frequency) override;
	void AdjustSound(int sound_uid, pos_state* cur_pos, float adjusted_volume, float reverb = 0.5f) override;

	void StopAllSounds(void) override;

	// Checks if a sound is playing (removes finished sound);
	bool IsSoundInstancePlaying(int sound_uid) override;
	int IsSoundPlaying(int sound_index) override;

	//	virtual void AdjustSound(int sound_uid, play_information *play_info) = 0;

	// Stops 2d and 3d sounds
	void StopSound(int sound_uid, unsigned char f_immediately = SKT_STOP_IMMEDIATELY) override;

	// Pause all sounds/resume all sounds
	void PauseSounds(void) override;
	void ResumeSounds(void) override;
	void PauseSound(int sound_uid) override;
	void ResumeSound(int sound_uid) override;

	bool CheckAndForceSoundDataAlloc(int sound_file_index) override;

	// Begin sound frame
	void SoundStartFrame(void) override;

	// End sound frame 
	void SoundEndFrame(void) override;

	// environmental sound interface
	// volume modifier (0-1), damping(0-1), 1 = complete, 0 = none
	//	decay 0.1 to 100 seconds, how long it takes for a sound to die.
	bool SetGlobalReverbProperties(float volume, float damping, float decay) override;

	// set special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	void SetEnvironmentValues(const t3dEnvironmentValues* env) override;

	// get special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	void GetEnvironmentValues(t3dEnvironmentValues* env) override;

	// enable special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	void SetEnvironmentToggles(const t3dEnvironmentToggles* env) override;

	// get states of special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	void GetEnvironmentToggles(t3dEnvironmentToggles* env) override;
};
