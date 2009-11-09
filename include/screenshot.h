#ifndef _SCREENSHOT_H_
#define _SCREENSHOT_H_

int screenshot_init( void );
void screenshot_free( void );
void screenshot_pause( void );
int screenshot_resume( void );
void screenshot_draw( void );
int screenshot_set( const char *filename );
void screenshot_clear( void );

#endif
