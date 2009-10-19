#ifndef _BG_H_
#define _BG_H_ 1

int bg_init( void );
void bg_pause( void );
int bg_resume( void );
void bg_clear( void );
int bg_set( const char *filename );
void bg_draw( void );
void bg_free( void );

#endif

