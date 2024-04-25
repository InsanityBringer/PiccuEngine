#ifndef INCLUDED_MVE_AUDIO_H
#define INCLUDED_MVE_AUDIO_H

void mveaudio_uncompress(short *buffer, unsigned char *data, int length);

#define MVESND_S16LSB 1
#define MVESND_U8 2

void mvesnd_init_audio(int format, int samplerate, int stereo);
void mvesnd_queue_audio_buffer(int len, short* data);
void mvesnd_close();

void mvesnd_pause();
void mvesnd_resume();

#endif /* INCLUDED_MVE_AUDIO_H */
