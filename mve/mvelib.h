#ifndef INCLUDED_MVELIB_H
#define INCLUDED_MVELIB_H

#include <stdio.h>
#include <stdlib.h>
#include "pstypes.h"
#include <stdint.h>

/*
 * structure for maintaining info on a MVEFILE stream
 */
typedef struct MVEFILE
{
    int             stream;
    unsigned char  *cur_chunk;
    int             buf_size;
    int             cur_fill;
    int             next_segment;
} MVEFILE;

typedef void (mve_cb_ShowFrame)(unsigned char* buf, unsigned int bufw, unsigned int bufh, unsigned int sx, unsigned int sy, unsigned int w, unsigned int h, unsigned int dstx, unsigned int dsty, unsigned int hicolor);
typedef void (mve_cb_SetPalette)(unsigned char* p, unsigned start, unsigned count);
typedef unsigned int (mve_cb_FileRead)(int hFile, void* pBuffer, unsigned int bufferCount);
typedef void* (mve_cb_Malloc)(unsigned int size);
typedef void (mve_cb_Free)(void* mem);

/*
 * open a .MVE file
 */
MVEFILE *mvefile_open(const char *filename);
MVEFILE *mvefile_open_filehandle(int filehandle);

/*
 * close a .MVE file
 */
void mvefile_close(MVEFILE *movie);
void mvefile_close_filehandle(MVEFILE *movie);

/*
 * get size of next segment in chunk (-1 if no more segments in chunk)
 */
int mvefile_get_next_segment_size(MVEFILE *movie);

/*
 * get type of next segment in chunk (0xff if no more segments in chunk)
 */
unsigned char mvefile_get_next_segment_major(MVEFILE *movie);

/*
 * get subtype (version) of next segment in chunk (0xff if no more segments in
 * chunk)
 */
unsigned char mvefile_get_next_segment_minor(MVEFILE *movie);

/*
 * see next segment (return NULL if no next segment)
 */
unsigned char *mvefile_get_next_segment(MVEFILE *movie);

/*
 * advance to next segment
 */
void mvefile_advance_segment(MVEFILE *movie);

/*
 * fetch the next chunk (return 0 if at end of stream)
 */
int mvefile_fetch_next_chunk(MVEFILE *movie);

/*
 * callback for segment type
 */
typedef int (*MVESEGMENTHANDLER)(unsigned char major, unsigned char minor, unsigned char *data, int len, void *context);

/*
 * structure for maintaining an MVE stream
 */
typedef struct MVESTREAM
{
    MVEFILE                    *movie;
    void                       *context;
    MVESEGMENTHANDLER           handlers[32];
} MVESTREAM;

/*
 * open an MVE stream
 */
MVESTREAM *mve_open(const char *filename);
MVESTREAM *mve_open_filehandle(int filehandle);

/*
 * close an MVE stream
 */
void mve_close(MVESTREAM *movie);
void mve_close_filehandle(MVESTREAM *movie);

/*
 * reset an MVE stream
 */
void mve_reset(MVESTREAM *movie);

/*
 * set segment type handler
 */
void mve_set_handler(MVESTREAM *movie, unsigned char major, MVESEGMENTHANDLER handler);

/*
 * set segment handler context
 */
void mve_set_handler_context(MVESTREAM *movie, void *context);

/*
 * play next chunk
 */
int mve_play_next_chunk(MVESTREAM *movie);

#endif /* INCLUDED_MVELIB_H */
