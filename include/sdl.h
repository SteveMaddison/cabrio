#ifndef _SDL_H_
#define _SDL_H_ 1

#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_framerate.h>
#include <SDL/SDL_opengl.h>

int sdl_init( void );
void sdl_free( void );
SDL_Surface *sdl_load_image( const char *filename );
GLuint sdl_create_texture( const char *filename );
void sdl_frame_delay( void );
void sdl_swap( void );

#endif

