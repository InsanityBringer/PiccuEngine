#ifndef INCLUDED_MVE_AUDIO_H
#define INCLUDED_MVE_AUDIO_H

#include <stdint.h>

void mveaudio_uncompress(short *buffer, unsigned char *data, int length);

#define MVESND_S16LSB 1
#define MVESND_U8 2

//Waits until the audio system has reached into the audio data for the current frame
void mvesnd_wait_for_frame_start(int frame_num);
//Call after the frame was waited on to update the write head
void mvesnd_end_of_frame();
void mvesnd_init_audio(int format, int samplerate, int stereo);
void mvesnd_update_timer(int rate);
void mvesnd_queue_audio_buffer(int len, uint8_t* data);
void mvesnd_start_audio();
void mvesnd_close();

void mvesnd_pause();
void mvesnd_resume();

#endif /* INCLUDED_MVE_AUDIO_H */
