#ifndef __VIDEO_H__
#define __VIDEO_H__

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

int video_init( void );
void video_free( void );
int video_open( const char *filename );
void video_close( void );
struct texture *video_get_frame( void );
struct texture *video_texture( void );

#endif
