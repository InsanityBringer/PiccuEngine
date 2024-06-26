#include <atomic>
#include "mve_audio.h"
#include "ssl_lib.h"
#include "mono.h"
static int audio_exp_table[256] =
{
         0,      1,      2,      3,      4,      5,      6,      7,      8,      9,     10,     11,     12,     13,     14,     15,
        16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,
        32,     33,     34,     35,     36,     37,     38,     39,     40,     41,     42,     43,     47,     51,     56,     61,
        66,     72,     79,     86,     94,    102,    112,    122,    133,    145,    158,    173,    189,    206,    225,    245,
       267,    292,    318,    348,    379,    414,    452,    493,    538,    587,    640,    699,    763,    832,    908,    991,
      1081,   1180,   1288,   1405,   1534,   1673,   1826,   1993,   2175,   2373,   2590,   2826,   3084,   3365,   3672,   4008,
      4373,   4772,   5208,   5683,   6202,   6767,   7385,   8059,   8794,   9597,  10472,  11428,  12471,  13609,  14851,  16206,
     17685,  19298,  21060,  22981,  25078,  27367,  29864,  32589, -29973, -26728, -23186, -19322, -15105, -10503,  -5481,     -1,
         1,      1,   5481,  10503,  15105,  19322,  23186,  26728,  29973, -32589, -29864, -27367, -25078, -22981, -21060, -19298,
    -17685, -16206, -14851, -13609, -12471, -11428, -10472,  -9597,  -8794,  -8059,  -7385,  -6767,  -6202,  -5683,  -5208,  -4772,
     -4373,  -4008,  -3672,  -3365,  -3084,  -2826,  -2590,  -2373,  -2175,  -1993,  -1826,  -1673,  -1534,  -1405,  -1288,  -1180,
     -1081,   -991,   -908,   -832,   -763,   -699,   -640,   -587,   -538,   -493,   -452,   -414,   -379,   -348,   -318,   -292,
      -267,   -245,   -225,   -206,   -189,   -173,   -158,   -145,   -133,   -122,   -112,   -102,    -94,    -86,    -79,    -72,
       -66,    -61,    -56,    -51,    -47,    -43,    -42,    -41,    -40,    -39,    -38,    -37,    -36,    -35,    -34,    -33,
       -32,    -31,    -30,    -29,    -28,    -27,    -26,    -25,    -24,    -23,    -22,    -21,    -20,    -19,    -18,    -17,
       -16,    -15,    -14,    -13,    -12,    -11,    -10,     -9,     -8,     -7,     -6,     -5,     -4,     -3,     -2,     -1
};

constexpr int MVEBUFFER_SIZE = 262144;
static uint8_t mveaudiobuffer[MVEBUFFER_SIZE];
static std::atomic_int mveaudiohead, mveaudiotail;

static int getWord(unsigned char **fin)
{
    int value = ((*fin)[1] << 8) | (*fin)[0];
    *fin += 2;
    return value;
}

static void sendWord(short **fout, int nOffset)
{
    *(*fout)++ = nOffset;
}

static void processSwath(short *fout, unsigned char *data, int swath, int *offsets)
{
    int i;
    for (i=0; i<swath; i++)
    {
        offsets[i&1] += audio_exp_table[data[i]];
        sendWord(&fout, offsets[i&1]);
    }
}

void mveaudio_uncompress(short *buffer, unsigned char *data, int length)
{
    int nCurOffsets[2];
    int swath;

    data += 4;
    swath = (getWord(&data) - 4) / 2;
    nCurOffsets[0] = getWord(&data);
    nCurOffsets[1] = getWord(&data);
    sendWord(&buffer, nCurOffsets[0]);
    sendWord(&buffer, nCurOffsets[1]);
    processSwath(buffer, data, swath, nCurOffsets);
}

//Playhead in samples
static std::atomic<uint64_t> g_playhead;
static int g_lastchunk, g_lastread;
static int g_bytespersample, g_slackbytes, g_samplerate;
static uint64_t g_audio_timerrate;

//Waits for the last sample number from the last read to have gone beyond the sample number at the start of the frame.
void mvesnd_wait_for_frame_start(int frame_num)
{
    //mprintf((0, "frame %d, playhead %llu framestart %llu\n", g_framedebug, g_playhead.load() + 10000, g_framestart.load()));
    uint64_t target = (((uint64_t)frame_num * g_audio_timerrate * g_bytespersample) * g_samplerate / 1000000) + g_slackbytes;
    while (g_playhead < target) {}
}

static int mvesnd_callback(void* userptr, void* sampledata, int numbytes)
{

    /*if (resync_request)
    {
        mveaudiotail = (mveaudiohead - resync_last) % MVEBUFFER_SIZE;
        resync_request = false;
    }*/

    //If this exceeds the start of the current frame, the frame wait can end.
    g_playhead += numbytes;

    uint8_t* ptr = &mveaudiobuffer[mveaudiotail];
    //Determine if fragmentation is needed
    int endaddress = mveaudiotail + numbytes;
    int fragmentsize = numbytes;
    int remainingsize = 0;
    if (endaddress > MVEBUFFER_SIZE)
    {
        remainingsize = endaddress - MVEBUFFER_SIZE;
        fragmentsize -= remainingsize;
    }

    //Handle first fragment
    memcpy(sampledata, ptr, fragmentsize);

    //Handle second fragment if needed
    if (remainingsize > 0)
    {
        memcpy((uint8_t*)sampledata + fragmentsize, &mveaudiobuffer[0], remainingsize);
    }

    mveaudiotail = (mveaudiotail + numbytes) % MVEBUFFER_SIZE;
    return numbytes;
}

extern llsSystem* mve_soundSystem;
void mvesnd_end_of_frame()
{
}

void mvesnd_init_audio(int format, int samplerate, int stereo)
{
    mveaudiohead = mveaudiotail = 0;
    g_lastchunk = g_lastread = 0;
    g_playhead = 0;

    g_bytespersample = 1;
    g_samplerate = samplerate;
    g_slackbytes = 1800;
    if (stereo)
    {
        g_bytespersample *= 2;
        g_slackbytes *= 2;
    }
    if (format == MVESND_S16LSB)
    {
        g_bytespersample *= 2;
        g_slackbytes *= 2;
    }

    if (mve_soundSystem)
    {
        mve_soundSystem->InitMovieBuffer(format == MVESND_S16LSB, samplerate, stereo, mvesnd_callback);
    }
}

void mvesnd_update_timer(int rate)
{
    g_audio_timerrate = rate;
}

void mvesnd_queue_audio_buffer(int len, uint8_t* data)
{
    uint8_t* ptr = &mveaudiobuffer[mveaudiohead];
    //Determine if fragmentation is needed
    int endaddress = mveaudiohead + len;
    int fragmentsize = len;
    int remainingsize = 0;
    if (endaddress > MVEBUFFER_SIZE)
    {
        remainingsize = endaddress - MVEBUFFER_SIZE;
        fragmentsize -= remainingsize;
    }

    //Handle first fragment
    memcpy(ptr, data, fragmentsize);

    //Handle second fragment if needed
    if (remainingsize > 0)
    {
        memcpy(&mveaudiobuffer[0], data + fragmentsize, remainingsize);
    }

    mveaudiohead = (mveaudiohead + len) % MVEBUFFER_SIZE;
}

void mvesnd_start_audio()
{
    //This starts the source if it is needed
    if (mve_soundSystem)
    {
        mve_soundSystem->QueueMovieBuffer(0, nullptr);
    }
}

void mvesnd_close()
{
    if (mve_soundSystem)
    {
        mve_soundSystem->KillMovieBuffer();
    }
}

void mvesnd_pause()
{
}

void mvesnd_resume()
{
}
