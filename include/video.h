#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <SDL/SDL_mutex.h>
#include "ogl.h"

struct packet_queue {
	AVPacketList *first;
	AVPacketList *last;
	int packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
};

int video_init( void );
void video_free( void );
int video_open( const char *filename );
void video_close( void );
int video_get_frame( void );
struct texture *video_texture( void );
int video_has_texture( void );

#endif
