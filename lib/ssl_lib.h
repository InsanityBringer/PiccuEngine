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

#ifndef __SSL_LIB_H__
#define __SSL_LIB_H__

#include "vecmat.h"

#ifndef NEWEDITOR
#include "manage.h"
#else
	#include "..\neweditor\ned_TableFile.h"
#endif

class oeApplication;									// reference to oeApplication class.

#define MAX_GAME_VOLUME (float) 1.0 //helps against clipping

// Size of streaming buffers
#define STREAM_BUFFER_SIZE 4096	//(4 * 1024)

//object information needed by the sound code.  Calling code should copy data into here & pass to sound
typedef struct
{
	vector *position;						// Where in the world is this object
	matrix *orient;			
	vector *velocity;
	int roomnum;
} pos_state;

//#define MAX_EFFECT_OFFSETS 5

//	sound priority values.
#define SND_PRIORITY_CRITICAL		5	// usually streams have this priority, bumps off any other sounds.
#define SND_PRIORITY_HIGHEST		4
#define SND_PRIORITY_HIGH			3
#define SND_PRIORITY_NORMAL		2
#define SND_PRIORITY_LOW			1
#define SND_PRIORITY_LOWEST		0

struct EAX2Reverb
{
	float density;
	float diffusion;
	float gain;
	float gain_hf;
	float gain_lf;
	float decay_time;
	float decay_hf_ratio;
	float decay_lf_ratio;
	float reflection_gain;
	float reflection_delay;
	float reflection_pan[3];
	float late_reverb_gain;
	float late_reverb_delay;
	float late_reverb_pan[3];
	float echo_time;
	float echo_depth;
	float modulation_time;
	float modulation_depth;
	float air_absorption_gain_hf;
	float hf_reference;
	float lf_reference;
	float room_rolloff_factor;
	bool decay_hf_limit;
};

typedef struct
{

	void     	*(*m_stream_cback)(void *user_data, int handle, int *size);  // Streaming callback
	void    	*m_stream_data;					// passed in
	int			m_stream_size;					// passed in
	int			m_stream_handle;				// passed in
	int			m_stream_bufsize;				// passed in
	void		*user_data;						// this is passed to the stream callback by the caller that defined this.

	ubyte		sample_skip_interval;		// Allows us to skip samples (i.e. simulate lower sampling rates)
	ubyte		priority;						// priority of sound.
	ushort		m_stream_format;				// passed in

//	internal data.
	int		m_samples_played;	

#ifndef MACINTOSH			
	float		samples_per_22khz_sample;	// passed in
	float		left_volume;					
	float		right_volume;					

	float		m_ticks;	
#endif					// Always incrementing counter (current sample position if no looping)
} play_information;

typedef struct sound_file_info 
{
	char				name[PAGENAME_LEN];	
	char				used;
	int				use_count;				// how many buffers does this sound take up.

	unsigned char *sample_8bit;			// 8bit sound data
	short *			sample_16bit;			// 16bit sound data

	int				sample_length;			// Length of sound in samples
	int				np_sample_length;		// non-padded

} sound_file_info;

typedef struct sound_info 
{
	char				name[PAGENAME_LEN];
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
} sound_info;

// Supported sound mixers
#define SOUND_MIXER_SOFTWARE_16	0
#define SOUND_MIXER_DS_16			1
#define SOUND_MIXER_DS_8			2
#define SOUND_MIXER_DS3D_16		3
#define SOUND_MIXER_AUREAL			4
#define SOUND_MIXER_CREATIVE_EAX	6	// switched because launcher uses 5 as NONE.
#define SOUND_MIXER_NONE			5

// Support sound qualities
#ifdef MACINTOSH
#define SQT_LOW							0
#define SQT_NORMAL						1
#define SQT_HIGH						2
#else
#define SQT_NORMAL						0
#define SQT_HIGH						1
#endif
// Parameters of the sound library
#define SLF_USE_3D			1		// Use 3d effects
#define SLF_DELTA_FREQ		2		// Use frequency shifts (i.e. water effects)
#define SLF_USE_16_BIT		4		// Use 16bit samples (else 8bit)
#define SLF_USE_22_KHZ		8		// Use 22khz (else 44khz)
#define SLF_PAUSED			16		// Sound library is currently paused
#define SLF_FULL_3D			32		// Full 3d hardware support
#define SLF_MOST_3D			64		// No fully static 3d -- i.e. cockpit type stuff (use 2d instead)
#define SLF_LIGHT_3D			128	// Dynamically updating 3d sounds if sound is longer than a given threshold
#define SLF_GOOD_2D			256	// all linked sounds update position
#define SLF_OK_2D				512	// if a sound is longer than a threshold, it updates

// Sound Properties Flags
#define SPF_LOOPED				1		// Sound is looped
#define SPF_FIXED_FREQ			2		// No doppler shift
#define SPF_OBJ_UPDATE			4		// Sound updates with attached object movements
#define SPF_FOREVER				8		// Always plays in high-level, this flag should be ignored in low-level
#define SPF_PLAYS_EXCLUSIVELY 16
#define SPF_PLAYS_ONCE			32
#define SPF_USE_CONE				64
#define SPF_LISTENER_UPDATE	128	// Sound updates with listener movements 
#define SPF_ONCE_PER_OBJ		256

// Sound Instance flags (Move this out of here)
#define SIF_UNUSED				0		// Not a valid sound item
#define SIF_PLAYING_2D			1		// Sound is currently playing
#define SIF_PLAYING_3D			2
#define SIF_OBJ_UPDATE			4
#define SIF_TOO_FAR				8		// We will play it, but it currently too far away(stop sound in low-level)		
#define SIF_NO_3D_EFFECTS		16
#define SIF_LOOPING           32
#define SIF_STREAMING_8_M		64
#define SIF_STREAMING_16_M		128
#define SIF_STREAMING_8_S		256
#define SIF_STREAMING_16_S    512
#define SIF_STREAMING         (64 | 128 | 256 | 512)

// What is the sound cone linked to (and mask to make it else to look at the important bits)
#define SPFT_CONE_LINK_MASK		0x00000300
#define SPFT_CONE_LINK_OBJECT		0x00000000
#define SPFT_CONE_LINK_TURRET1	0x00000100
#define SPFT_CONE_LINK_TURRET2   0x00000200
#define SPFT_CONE_LINK_TURRET3   0x00000300

// Direction of the sound cone relative to its link (and mask to make it else to look at the important bits)
#define SPFT_CONE_DIR_MASK			0x00000C00
#define SPFT_CONE_DIR_FORWARD    0x00000000
#define SPFT_CONE_DIR_BACKWARD   0x00000400
#define SPFT_CONE_DIR_UPWARD     0x00000800
#define SPFT_CONE_DIR_DOWNWARD   0x00000C00

// Sound kill types
#define SKT_STOP_AFTER_LOOP	0		// Allows a looping sample to play until the end of the sample
#define SKT_STOP_IMMEDIATELY	1		// Stops and cleans up after a sound (For StopAllSounds)
#define SKT_HOLD_UNTIL_STOP	2		// Hold until sound stops.

// Sound Library Internal Error Codes
#define SSL_OK								0

// structure to get and set environment values
#define ENV3DVALF_DOPPLER		1
#define ENV3DVALF_GEOMETRY		2
#define ENV3dVALF_REVERBS		4

struct t3dEnvironmentValues
{
	int flags;								// use flags above

	float doppler_scalar;				// values from 0.0f to ???? (1.0f = normal)
};

struct t3dEnvironmentToggles
{
	int flags;								// use flags above
	int supported;							// returns flag values to inform caller of supported features (doppler, ie.)

	bool doppler;							// state of doppler effects
	bool reverb;							// state of reverb effects
	bool geometry;							// support hardware geometry
};

typedef int (*llsMovieCallback)(void* userptr, void* sampledata, int numbytes);

/////////////////////////////////////////////////////////////////////////////////
// Looping constants

class llsGeometry;

class llsSystem 
{
protected:
	llsGeometry *m_geometry;			// geometry object.
	int m_lib_error_code;				// library error code 
	char m_error_text[512];				// text for error.

protected:
	void SetError(int code) { m_lib_error_code = code; };
	void ErrorText(char *fmt, ...);	// error text function called inside library.  a stack is kept of errors
	virtual void CheckForErrors();	// called by sound library every frame to reset error count.

// Public functions 
public:
	llsSystem();
	//derived LLS is different size, so a virtual destructor is needed. 
	virtual ~llsSystem()
	{
	}
	
	// may be called before init (must be to be valid, the card passed here will be initialized in InitSoundLib)
	virtual void SetSoundCard(const char *name) = 0;

	// Starts the sound library, maybe have it send back some information -- 3d support?
	virtual int InitSoundLib(char mixer_type, oeApplication *sos, unsigned char max_sounds_played) = 0;
	// Cleans up after the Sound Library
	virtual void DestroySoundLib(void) = 0;

	// Locks and unlocks sounds (used when changing play_info data)
	virtual bool LockSound(int sound_uid) = 0;
	virtual bool UnlockSound(int sound_uid) = 0;

	virtual bool SetSoundQuality(char quality) = 0;
	virtual char GetSoundQuality(void) = 0;
	virtual bool SetSoundMixer(char mixer_type) = 0;
	virtual char GetSoundMixer(void) = 0;

	// Plays a 2d sound
	virtual int PlaySound2d(play_information *play_info, int sound_index, float volume, float pan, bool f_looped) = 0; 
	virtual int PlayStream(play_information *play_info) = 0;

	virtual void SetListener(pos_state *cur_pos) = 0; 
	virtual int PlaySound3d(play_information *play_info, int sound_index, pos_state *cur_pos, float master_volume, bool f_looped, float reverb=0.5f) = 0; //, unsigned short frequency)
	virtual void AdjustSound(int sound_uid, float f_volume, float f_pan, unsigned short frequency) = 0;
	virtual void AdjustSound(int sound_uid, pos_state *cur_pos, float adjusted_volume, float reverb=0.5f) = 0;

	virtual void StopAllSounds(void) = 0;
		
	// Checks if a sound is playing (removes finished sound);
	virtual bool IsSoundInstancePlaying(int sound_uid) = 0;
	virtual int IsSoundPlaying(int sound_index) = 0; 

//	virtual void AdjustSound(int sound_uid, play_information *play_info) = 0;

	// Stops 2d and 3d sounds
	virtual void StopSound(int sound_uid, unsigned char f_immediately = SKT_STOP_IMMEDIATELY) = 0;

	// Pause all sounds/resume all sounds
	virtual void PauseSounds(void) = 0;
	virtual void ResumeSounds(void) = 0;
	virtual void PauseSound(int sound_uid) = 0;
	virtual void ResumeSound(int sound_uid) = 0;

	virtual bool CheckAndForceSoundDataAlloc(int sound_file_index) = 0;

	// Begin sound frame
	virtual void SoundStartFrame(void) = 0;
	
	// End sound frame 
	virtual void SoundEndFrame(void) = 0;

	// Returns current error code
	int GetLastError() { int code = m_lib_error_code; m_lib_error_code = 0; return code; };

	// environmental sound interface
	virtual bool SetGlobalReverbProperties(const EAX2Reverb* reverbs) = 0;

	// set special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	virtual void SetEnvironmentValues(const t3dEnvironmentValues *env) = 0;

	// get special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	virtual void GetEnvironmentValues(t3dEnvironmentValues *env) = 0;

	// enable special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	virtual void SetEnvironmentToggles(const t3dEnvironmentToggles *env) = 0;

	// get states of special parameters for the 3d environment.
	// of strcuture passed, you must set the appropriate 'flags' value for values you wish to modify
	virtual void GetEnvironmentToggles(t3dEnvironmentToggles *env) = 0;

	// returns interface to sound geometry manipulation if available.
	virtual llsGeometry *GetGeometryInterface() {
		return m_geometry;
	};
#ifdef MACINTOSH
	virtual void SetNumChannels(ubyte num_chan) = 0;
#endif
/////////////////////////////////////////////////////////////////////////////////
	// set auxillary 3d sound properties
	virtual bool SoundPropertySupport() const { return false; };

	// sound obstruction from 0 to 1.0 (1.0  = fully obstructed)
	virtual void SetSoundProperties(int sound_uid, float obstruction) {};

	//[ISB] These functions are added for movie support. 
	//I only really intend to support them in my OpenAL backend.
	virtual void InitMovieBuffer(bool is16bit, int samplerate, bool stereo, llsMovieCallback callback) = 0;

	virtual void KillMovieBuffer() = 0;

	virtual void QueueMovieBuffer(int length, void* data) = 0;
};

//	TAKEN FROM SNDLIB SOUNDLOAD.CPP TO SEPARATE CODE REQUIRED BY THE LOWLEVEL SYSTEM AND THE 
//	HIGH LEVEL SYSTEM - Samir

#ifndef NEWEDITOR 

	#define MAX_SOUNDS			1000
	#define MAX_SOUND_FILES		1000
	
	extern sound_info Sounds[MAX_SOUNDS];
	extern sound_file_info SoundFiles[MAX_SOUND_FILES];
#else
	#include "..\neweditor\ned_Sound.h"
#endif

extern sound_file_info SoundFiles[MAX_SOUND_FILES];

//	loads a sound from a wavefile.
char SoundLoadWaveFile(char *filename, float percent_volume, int sound_file_index, bool f_high_quality, bool f_load_sample_data, int *e_type = NULL);

void SoundLoadFree(int sound_file_index);

#endif