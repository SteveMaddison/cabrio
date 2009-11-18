#ifndef __VIDEO_H__
#define __VIDEO_H__

int video_init( void );
void video_free( void );
int video_open( const char *filename );
void video_close( void );
int video_get_frame( void );

#endif
