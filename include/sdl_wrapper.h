#ifndef _SDL_H_
#define _SDL_H_ 1

#include <stdio.h>

int sdl_init( void );
void sdl_free( void );
void sdl_frame_delay( void );
void sdl_clear( void );
void sdl_swap( void );
int sdl_hat_dir_value( int direction );

#endif

