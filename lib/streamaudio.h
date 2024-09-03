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

#ifndef  __STREAMAUDIO_H_
#define  __STREAMAUDIO_H_

#include "Adecode.h"

#include "ssl_lib.h"

void *AudioStreamCB(void *user_data, int handle, int *size);
int ADecodeFileRead(void *data, void *buf, unsigned int qty);
int StreamPlay(const char *filename, float volume, int flags);
void StreamStop(int handle);
int StreamGetSoundHandle(int handle);

//////////////////////////////////////////////////////////////////////////////
class llsSystem;
struct CFILE;
//	stream file info
#define OSF_HDR_SIZE			128
#define OSF_TAG				"OSF1"
#define OSF_HDR_TITLE_OFS	64
#define OSF_HDR_TITLE_LEN	32
//	stream type
#define OSF_DIGITAL_STRM	0
// compression type
#define OSF_DIGIRAW_STRM	0
#define OSF_DIGIACM_STRM	1
//	stream format
#define SAF_8BIT_MASK		0x00
#define SAF_16BIT_MASK		0x01
#define SAF_MONO_MASK		0x00
#define SAF_STEREO_MASK		0x10
	
#define SAF_8BIT_M			(SAF_8BIT_MASK | SAF_MONO_MASK)		//0x00
#define SAF_8BIT_S			(SAF_8BIT_MASK | SAF_STEREO_MASK)	//0x10
#define SAF_16BIT_M			(SAF_16BIT_MASK | SAF_MONO_MASK)		//0x01
#define SAF_16BIT_S			(SAF_16BIT_MASK | SAF_STEREO_MASK)	//0x11
//////////////////////////////////////////////////////////////////////////////
struct tOSFDigiHdr					// this struct belongs to OSF_DIGITAL_STRM
{
	uint measure;
};

class OSFArchive
{
	CFILE *m_fp;
	uint m_length;								// file information stored such as length
	ubyte m_type;								// stream type
	ubyte m_comp;								// compression type
	ubyte m_flags;								// format
	uint m_rate;								// frequency
	bool m_writemode;							// are we in write mode?
	union {
		tOSFDigiHdr digi;
	}
	m_hdr;
	char m_name[OSF_HDR_TITLE_LEN];		// title string length.
	int m_datastart;							// pointer to start of data.
public:
	OSFArchive();
	~OSFArchive();
	bool Open(const char *filename, bool write=false);
	void Close();
	bool Opened() const { return m_fp ? true : false; };
	void Rewind();
//	write out operations.
	bool SaveHeader(ubyte type, ubyte comp, ubyte flags, uint rate, uint length, void *hdr, const char *name);
	bool WriteBlock(ubyte *blk, int size);
//	read in operations.
	int Read(ubyte *blk, int size);
//	get archive info.
	ubyte StreamType() const { return m_type; };
	ubyte StreamComp() const { return m_comp; };
	ubyte StreamFormat() const { return m_flags; };
	uint StreamRate() const { return m_rate; };
	uint StreamLength() const { return m_length; };
	const char *StreamName() const { return (const char *)m_name; };
//	get header info.
	const void *StreamHeader() const { return (void *)&m_hdr.digi; };
};
//////////////////////////////////////////////////////////////////////////////
//	streamaudio constants.
#define STRM_BUFCOUNT		4
#define STRM_BUFSIZE			STREAM_BUFFER_SIZE
#define STRM_LIMIT			4
#define STRM_STOPPED			0x0
#define STRM_PLAYING			0x1
#define STRM_PAUSED			0x2
#define STRM_STOPPING		0x4
#define STRM_PENDING		0x8		//DAJ
#define STRM_INVALID			0xff
//	flags used to open stream.
#define STRM_OPNF_ONETIME	0x1
#define STRM_OPNF_GRADUAL	0x2

/*	This class will handle streams for the music system.
	Including allowing an interface to dynamically change the stream.
*/
#define STRM_BUFF_USED				0x1	// allocated buffer with data
#define STRM_BUFF_TERMINAL			0x2	// terminates on this buffer
#define STRM_BUFF_LOOPEND			0x4	// marks last buffer in measure

class AudioStream
{
	OSFArchive m_archive;					// audio stream archive object.
	AudioDecoder::IAudioDecoder* m_decoder; // audio codec object
	struct {										// mixing buffers
		ubyte *data;
		int nbytes;								// number of bytes of valid data.
		int flags;
		int id;
	}
	m_buffer[STRM_BUFCOUNT];
	int m_bufsize;								// size of each buffer.
	float m_measure_timer;					// timer for measure checking.
	float m_measure_time;					// amount of time per measure.
	float m_last_frametime;
	ubyte m_sbufidx;							// stream position markers
	ubyte m_fbufidx;							// file position markers
	ubyte m_curbufidx;						// current buffer in measure index
	ubyte m_playcount;
	bool m_readahead;							// if stream is currently reading from disk
	bool m_readahead_finished_loop;		// if a loop's readahead has finished
	short m_nbufs;								// number of buffers streamed so far.
	play_information m_playinfo;			// used by llsSystem
	float m_volume;							// volume of stream.
	short m_state, m_laststate;			// current state of stream playing
	int m_llshandle;							// internal sound handle.
	int m_flags;								// stream playing options.
	short m_streamindex;						// index into active stream table.
	short m_loopcount;						// loop counter.
	int m_bytesleft;							// number of bytes left in file
	int m_curmeasure;							// current measure.
	int m_playbytesleft, m_playbytestotal;
	int m_curid;									// stream's id #
	int *m_stopflag;							// location of stop flag used in stop function
	bool m_loop;								// are we looping?
	bool m_stopnextmeasure;					// stop on next measure.
	bool m_start_on_frame;					// we will play this stream on the next ::Frame call.
	bool m_start_on_frame_looped;			// the stream that will play on next frame is looped.
private:
	friend void *AudioStreamCB(void *user_data, int handle, int *size);
	friend int ADecodeFileRead(void *data, void *buf, unsigned int qty);
	void *StreamCallback(int *size);		// invoked by omsStreamCB.
	int ReadFileData(int buf, int len);	// reads in decompressed raw data.
	int ReadFileDirect(char * buf, int len);	// reads in decompressed raw data.
	void UpdateData();						// updates file buffers
	void End();									// cleans up after a stop.
	void Reset();								// resets to start of stream.
	bool OpenDigitalStream();				// opens and prepares a digital stream 
	bool ReopenDigitalStream(ubyte fbufidx, int nbufs);
private:
//	attach a low level sound system to all streams.
	static llsSystem *m_ll_sndsys;			
//	list of all currently played streams
	static AudioStream *m_streams[STRM_LIMIT];
	static int m_thisid;
// allocates a stream slot for a stream
	bool ActivateStream(AudioStream *stream);
	void DeactivateStream(AudioStream *stream);
public:
//	sets the low-level sound object
	static void InitSystem(llsSystem *sndsys);
//	shutdsown
	static void Shutdown();
//	called by application to allow stream playback
	static void Frame();
// called to pause all streams.
	static void PauseAll();
	static void ResumeAll();
public:
	AudioStream();
	~AudioStream();
// simple operations
//	flags specify what type of stream you want.
	bool Open(const char *filename, int open_flags=0);
// specifies next stream to be opened when current one ends.
	void Close();
// simple requests
	bool Play(bool start_on_frame=false);	//	plays a stream
	void Stop(bool on_measure=false,int *stop_flag=NULL);	// causes a rewind to start of stream, if on_measure is true, stop occurs when measure ends
	void SetVolume(float vol);				// sets volume
	float GetVolume();						// returns volume																		   
// misc requests
	int GetLoopCount() const;				// 0 = infinite.
	void SetLoopCount(int loop_count);	// sets the number of times this stream will loop (0 = infinite!)
	void SetFlags(int flags);				// sets flags for playback (STRM_OPNF_XXX)
	void Pause();								// pauses a stream
	void Resume();								// resumes a paused stream.
// state.	  
	bool ReadAhead();							// are we still reading from disk?
	bool ReadAheadFinishedLoop();			// has stream finished with its readahead of current loop?
	bool IsReady();							// is this stopped stream ready to play?
	int State() const {						// returns current state
		return m_state;
	};	
// information
	int GetLength() const { return m_archive.StreamLength(); };
	int GetPos() const { return m_archive.StreamLength() - m_bytesleft; };
	int GetBufferSize() const { return m_bufsize; };
	int CurrentMeasure() const {			// returns current measure.
		return m_curmeasure;
	};
	int TotalMeasures() const;				// total measure count
	int GetSoundHandle() const { 			// returns the sound library handle for this stream.
		return m_llshandle; 
	};
//	these functions are the 'simplified' stream interface from Jeff (most of this code is from Jeff)
	friend int StreamPlay(const char *filename, float volume, int flags);
	friend void StreamStop(int handle);
	friend int StreamGetSoundHandle(int handle);
};
#endif
