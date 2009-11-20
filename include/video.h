#ifndef __VIDEO_H__
#define __VIDEO_H__

#include "ogl.h"

int video_init( void );
void video_free( void );
int video_open( const char *filename );
void video_close( void );
int video_get_frame( void );
struct texture *video_texture( void );

#endif
