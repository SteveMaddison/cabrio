#ifndef _SNAP_H_
#define _SNAP_H_

int snap_init( void );
void snap_free( void );
void snap_pause( void );
int snap_resume( void );
void snap_show( void );
void snap_hide( void );
void snap_draw( void );
int snap_set( const char *filename );
void snap_clear( void );

#endif
