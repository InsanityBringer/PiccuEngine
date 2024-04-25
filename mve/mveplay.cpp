#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <thread>

#include "mvelib.h"
#include "mve_audio.h"

#include "decoders.h"
#include "libmve.h"
#include "pstypes.h"
#include "pserror.h"
#include "mem.h"

#define MVE_OPCODE_ENDOFSTREAM          0x00
#define MVE_OPCODE_ENDOFCHUNK           0x01
#define MVE_OPCODE_CREATETIMER          0x02
#define MVE_OPCODE_INITAUDIOBUFFERS     0x03
#define MVE_OPCODE_STARTSTOPAUDIO       0x04
#define MVE_OPCODE_INITVIDEOBUFFERS     0x05

#define MVE_OPCODE_DISPLAYVIDEO         0x07
#define MVE_OPCODE_AUDIOFRAMEDATA       0x08
#define MVE_OPCODE_AUDIOFRAMESILENCE    0x09
#define MVE_OPCODE_INITVIDEOMODE        0x0A

#define MVE_OPCODE_SETPALETTE           0x0C
#define MVE_OPCODE_SETPALETTECOMPRESSED 0x0D

#define MVE_OPCODE_SETDECODINGMAP       0x0F

#define MVE_OPCODE_VIDEODATA            0x11

#define MVE_AUDIO_FLAGS_STEREO     1
#define MVE_AUDIO_FLAGS_16BIT      2
#define MVE_AUDIO_FLAGS_COMPRESSED 4

int g_spdFactorNum=0;
static int g_spdFactorDenom=10;
static int g_frameUpdated = 0;

static short get_short(unsigned char *data)
{
	short value;
	value = data[0] | (data[1] << 8);
	return value;
}

static unsigned short get_ushort(unsigned char *data)
{
	unsigned short value;
	value = data[0] | (data[1] << 8);
	return value;
}

static int get_int(unsigned char *data)
{
	int value;
	value = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
	return value;
}

static unsigned int unhandled_chunks[32*256];

static int default_seg_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	unhandled_chunks[major<<8|minor]++;
	//fprintf(stderr, "unknown chunk type %02x/%02x\n", major, minor);
	return 1;
}


/*************************
 * general handlers
 *************************/
static int end_movie_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	return 0;
}

/*************************
 * timer handlers
 *************************/
/*
 * timer variables
 */
static int timer_created = 0;
static int micro_frame_delay=0;
static int timer_started=0;

static uint64_t nextTimerTick;

static int create_timer_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	long long temp;

	if (timer_created)
		return 1;
	else
		timer_created = 1;

	micro_frame_delay = get_int(data) * (int)get_short(data+4);
	//printf("micro_frame_delay: %d\n", micro_frame_delay);
	if (g_spdFactorNum != 0)
	{
		temp = micro_frame_delay;
		temp *= g_spdFactorNum;
		temp /= g_spdFactorDenom;
		micro_frame_delay = (int)temp;
	}

	return 1;
}

//[ISB] TODO: move to timer.cpp perhaps
#include <chrono>
static uint64_t GetClockTimeUS()
{
	using namespace std::chrono;
	return static_cast<uint64_t>(duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count());
}

static void DelayUS(uint64_t us)
{
	std::this_thread::sleep_for(std::chrono::microseconds(us));
}

static void timer_stop(void)
{
	timer_started = 0;
}

static void timer_start(void)
{
	nextTimerTick = GetClockTimeUS() + micro_frame_delay;
	timer_started=1;
}

static void do_timer_wait(void)
{
	uint64_t startTick = GetClockTimeUS();
	uint64_t numTicks = nextTimerTick - startTick;
	if (numTicks > 2000) //[ISB] again inspired by dpJudas, with 2000 US number from GZDoom
		DelayUS(numTicks - 2000);
	while (GetClockTimeUS() < nextTimerTick);
	nextTimerTick += micro_frame_delay;
}

/*************************
 * audio handlers
 *************************/

#define TOTAL_AUDIO_BUFFERS 64

static int audiobuf_created = 0;
static int mve_audio_playing=0;
static int mve_audio_canplay=0;
static int mve_audio_compressed=0;
static int mve_audio_enabled = 1;
static int mve_audio_paused = 0;

static int create_audiobuf_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	int flags;
	int sample_rate;
	int desired_buffer;

	int stereo;
	int bitsize;
	int compressed;

	int format;

	if (!mve_audio_enabled)
		return 1;

	flags = get_ushort(data + 2);
	sample_rate = get_ushort(data + 4);
	desired_buffer = get_int(data + 6);

	//mprintf((0, "flags: %d, sample rate: %d, desired_buffer: %d\n", flags, sample_rate, desired_buffer));

	stereo = (flags & MVE_AUDIO_FLAGS_STEREO) ? 1 : 0;
	bitsize = (flags & MVE_AUDIO_FLAGS_16BIT) ? 1 : 0;

	if (minor > 0) 
	{
		compressed = flags & MVE_AUDIO_FLAGS_COMPRESSED ? 1 : 0;
	} else 
	{
		compressed = 0;
	}

	mve_audio_compressed = compressed;

	if (bitsize == 1)
	{
		format = MVESND_S16LSB;
	} 
	else 
	{
		format = MVESND_U8;
	}

	mvesnd_init_audio(format, sample_rate, stereo);
	mve_audio_canplay = 1;

	return 1;
}

static int play_audio_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	return 1;
}

//[ISB] this handler is gutting the present buffeirng code and instead making it the property of the sound library. 
static int audio_data_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	static const int selected_chan=1;
	int chan;
	int nsamp;
	short* buf = NULL;
	if (mve_audio_canplay)
	{
		chan = get_ushort(data + 2);
		nsamp = get_ushort(data + 4);
		if (chan & selected_chan)
		{
			/* HACK: +4 mveaudio_uncompress adds 4 more bytes */
			if (major == MVE_OPCODE_AUDIOFRAMEDATA) 
			{
				if (mve_audio_compressed)
				{
					nsamp += 4;

					buf = (short*)mem_malloc(nsamp+4);
					mveaudio_uncompress(buf, data, -1);
				} 
				else 
				{
					nsamp -= 8;
					data += 8;

					buf = (short*)mem_malloc(nsamp+8);
					mveaudio_uncompress(buf, data, -1);
				}
			} 
			else 
			{
				buf = (short*)mem_malloc(nsamp+4);
				mveaudio_uncompress(buf, data, -1);

				memset(buf, 0, nsamp); /* XXX */
			}

			mvesnd_queue_audio_buffer(nsamp, (uint8_t*)buf);
		}
	}

	if (buf) mem_free(buf);

	return 1;
}

/*************************
 * video handlers
 *************************/
static int videobuf_created = 0;
static int video_initialized = 0;
int g_width, g_height;
void *g_vBuffers = NULL, *g_vBackBuf1, *g_vBackBuf2;
void* hackBuf1 = NULL, * hackBuf2 = NULL;

#ifdef STANDALONE
static SDL_Surface *g_screen;
#else
static int g_destX, g_destY;
#endif
static int g_screenWidth, g_screenHeight;
static unsigned char g_palette[768];
static unsigned char *g_pCurMap=NULL;
static int g_nMapLength=0;
static bool g_truecolor;

static int create_videobuf_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	short w, h;
	short count, truecolor;

	if (videobuf_created)
		return 1;
	else
		videobuf_created = 1;

	w = get_short(data);
	h = get_short(data+2);

	if (minor > 0) 
	{
		count = get_short(data+4);
	} 
	else 
	{
		count = 1;
	}

	if (minor > 1) 
	{
		truecolor = get_short(data+6);
	}
	else
	{
		truecolor = 0;
	}

	g_width = w << 3;
	g_height = h << 3;

	if (hackBuf1 == NULL && hackBuf2 == NULL)
	{
		/* TODO: * 4 causes crashes on some files */
		g_vBackBuf1 = g_vBuffers = (uint8_t*)mem_malloc(g_width * g_height * 8);
		if (truecolor)
		{
			g_vBackBuf2 = (unsigned short*)g_vBackBuf1 + (g_width * g_height);
		}
		else
		{
			g_vBackBuf2 = (unsigned char*)g_vBackBuf1 + (g_width * g_height);
		}

		memset(g_vBackBuf1, 0, g_width * g_height * 4);
	}

#ifdef DEBUG
	fprintf(stderr, "DEBUG: w,h=%d,%d count=%d, tc=%d\n", w, h, count, truecolor);
#endif

	g_truecolor = truecolor;

	return 1;
}

//[ISB] godawful hack
static mve_cb_ShowFrame* ShowFrameCallback = NULL;
static mve_cb_SetPalette* SetPaletteCallback = NULL;

void debug_dump_frame()
{
	FILE* fp = fopen("c:/dev/rawframe.raw", "wb");
		fwrite(g_vBackBuf1, g_truecolor ? sizeof(unsigned short) : sizeof(unsigned char), g_width * g_height, fp);

	fclose(fp);
}

static int display_video_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	if (ShowFrameCallback == NULL)
	{
		Int3();
	}
	else
	{
		if (g_destX == -1) // center it
			g_destX = (g_screenWidth - g_width) >> 1;
		if (g_destY == -1) // center it
			g_destY = (g_screenHeight - g_height) >> 1;

		if (g_truecolor)
			(*ShowFrameCallback)((uint8_t*)g_vBackBuf1, g_width * 2, g_height, 0, 0, g_width, g_height, g_destX, g_destY, true);
		else
			(*ShowFrameCallback)((uint8_t*)g_vBackBuf1, g_width, g_height, 0, 0, g_width, g_height, g_destX, g_destY, false);
	}
	g_frameUpdated = 1;

	return 1;
}

static int init_video_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	short width, height;

	if (video_initialized)
		return 1;
	else
		video_initialized = 1;

	width = get_short(data);
	height = get_short(data+2);
#ifdef STANDALONE
	g_screen = SDL_SetVideoMode(width, height, 16, g_sdlVidFlags);
#endif
	g_screenWidth = width;
	g_screenHeight = height;
	memset(g_palette, 0, 765);
	// 255 needs to default to white, for subtitles, etc
	memset(g_palette + 765, 63, 3);

	return 1;
}

static int video_palette_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	short start, count;
	start = get_short(data);
	count = get_short(data+2);
	if (SetPaletteCallback == NULL)
		memcpy(g_palette + 3 * start, data + 4, 3 * count);
	else
	{
		//[ISB] offset is a dumb hack to ensure the palette callback works without modification
		(*SetPaletteCallback)(data + 4 - (start * 3), start, count);
	}

	return 1;
}

static int video_codemap_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	g_pCurMap = data;
	g_nMapLength = len;
	return 1;
}

static int video_data_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	short nFrameHot, nFrameCold;
	short nXoffset, nYoffset;
	short nXsize, nYsize;
	unsigned short nFlags;
	unsigned char *temp;

	nFrameHot  = get_short(data);
	nFrameCold = get_short(data+2);
	nXoffset   = get_short(data+4);
	nYoffset   = get_short(data+6);
	nXsize     = get_short(data+8);
	nYsize     = get_short(data+10);
	nFlags     = get_ushort(data+12);

	if (nFlags & 1)
	{
		temp = (unsigned char *)g_vBackBuf1;
		g_vBackBuf1 = g_vBackBuf2;
		g_vBackBuf2 = temp;
	}

	/* convert the frame */
	if (g_truecolor) {
		decodeFrame16((unsigned char *)g_vBackBuf1, g_pCurMap, g_nMapLength, data+14, len-14);
	} else {
		decodeFrame8((uint8_t*)g_vBackBuf1, g_pCurMap, g_nMapLength, data+14, len-14);
	}

	return 1;
}

static int end_chunk_handler(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context)
{
	g_pCurMap=NULL;
	return 1;
}

static MVESTREAM *mve = NULL;

int MVE_rmPrepMovie(int filehandle, int x, int y, int track)
{
	int i;

	if (mve) 
	{
		mve_reset(mve);
		return 0;
	}
	//[ISB] make sure it doesn't try to play sound if the previous movie had sound but this one doesn't
	mve_audio_canplay = 0;

	mve = mve_open_filehandle(filehandle);

	if (!mve)
		return 1;

	g_destX = x;
	g_destY = y;

	for (i = 0; i < 32; i++)
		mve_set_handler(mve, i, default_seg_handler);

	mve_set_handler(mve, MVE_OPCODE_ENDOFSTREAM,          end_movie_handler);
	mve_set_handler(mve, MVE_OPCODE_ENDOFCHUNK,           end_chunk_handler);
	mve_set_handler(mve, MVE_OPCODE_CREATETIMER,          create_timer_handler);
	mve_set_handler(mve, MVE_OPCODE_INITAUDIOBUFFERS,     create_audiobuf_handler);
	mve_set_handler(mve, MVE_OPCODE_STARTSTOPAUDIO,       play_audio_handler);
	mve_set_handler(mve, MVE_OPCODE_INITVIDEOBUFFERS,     create_videobuf_handler);

	mve_set_handler(mve, MVE_OPCODE_DISPLAYVIDEO,         display_video_handler);
	mve_set_handler(mve, MVE_OPCODE_AUDIOFRAMEDATA,       audio_data_handler);
	mve_set_handler(mve, MVE_OPCODE_AUDIOFRAMESILENCE,    audio_data_handler);
	mve_set_handler(mve, MVE_OPCODE_INITVIDEOMODE,        init_video_handler);

	mve_set_handler(mve, MVE_OPCODE_SETPALETTE,           video_palette_handler);
	mve_set_handler(mve, MVE_OPCODE_SETPALETTECOMPRESSED, default_seg_handler);

	mve_set_handler(mve, MVE_OPCODE_SETDECODINGMAP,       video_codemap_handler);

	mve_set_handler(mve, MVE_OPCODE_VIDEODATA,            video_data_handler);

	return 0;
}

int MVE_rmStepMovie()
{
	static int init_timer=0;
	int cont=1;

	if (!timer_started)
		timer_start();

	while (cont && !g_frameUpdated) // make a "step" be a frame, not a chunk...
		cont = mve_play_next_chunk(mve);

	if (micro_frame_delay  && !init_timer)
	{
		timer_start();
		init_timer = 1;
	}

	if (mve_audio_paused)
	{
		mve_audio_paused = 0;
		mvesnd_resume();
	}

	if (g_frameUpdated)
		do_timer_wait();
	g_frameUpdated = 0;

	if (cont)
		return 0;
	else
		return MVE_ERR_EOF;
}

void MVE_rmEndMovie()
{
	timer_stop();
	timer_created = 0;

	if (g_vBuffers != NULL)
		mem_free(g_vBuffers);
	g_vBuffers = NULL;
	g_pCurMap=NULL;
	g_nMapLength=0;
	videobuf_created = 0;
	video_initialized = 0;

	mve_close_filehandle(mve);
	mve = NULL;

	if (mve_audio_canplay)
		mvesnd_close();
}


void MVE_rmHoldMovie()
{
	timer_started = 0;
	mvesnd_pause();
	mve_audio_paused = 1;
}

//[ISB] whoops i made assumptions again. I guess D2X really tweaked how all this crap worked
void MVE_memVID(void* first, void* second, size_t len)
{
	hackBuf1 = first;
	hackBuf2 = second;

	if (g_vBackBuf1 != NULL && g_vBackBuf2 != NULL)
	{
		g_vBackBuf1 = first;
		g_vBackBuf2 = second;
	}
}

void MVE_sfCallbacks(mve_cb_ShowFrame *func)
{
	ShowFrameCallback = func;
}

void MVE_palCallbacks(mve_cb_SetPalette* func)
{
	SetPaletteCallback = func;
}

void MVE_ioCallbacks(mve_cb_FileRead* func)
{
}

void MVE_memCallbacks(mve_cb_Malloc* alloc, mve_cb_Free* free)
{
}

void MVE_ReleaseMem()
{
	hackBuf1 = hackBuf2 = NULL;
}
