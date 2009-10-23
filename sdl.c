#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include "config.h"
#include "sdl.h"
#include "ogl.h"

extern struct config *config;

static SDL_Surface *screen = NULL;

static const int SDL_SCREEN_BPP = 32;
static const char *title = "Cabrio";
static FPSmanager manager;

int sdl_init( void ) {
	int mode = SDL_SWSURFACE|SDL_OPENGL;
	if( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
		fprintf(stderr, "Error: Unable to initialise SDL: %s\n", SDL_GetError());
		return 1;
	}
	SDL_ShowCursor(SDL_DISABLE);
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	if( config->iface.full_screen )
		mode |= SDL_FULLSCREEN;
	screen = SDL_SetVideoMode( config->iface.screen_width, config->iface.screen_height, SDL_SCREEN_BPP, mode );
	if( screen == NULL ) {
		fprintf(stderr, "Error: Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	}
	SDL_initFramerate( &manager );
	SDL_setFramerate( &manager, 60 );
	SDL_WM_SetCaption( title, NULL );
	return 0;
}

void sdl_free( void ) {
	SDL_Quit();
}

SDL_Surface *sdl_load_image( const char *filename ) {
	SDL_Surface* load = NULL;
	SDL_Surface* conv = NULL;
	
	load = IMG_Load( filename );
	if( load != NULL ) {
		conv = SDL_DisplayFormatAlpha( load );
		SDL_FreeSurface( load );
	}
	else {
		fprintf(stderr, "Error: Unable to load image '%s': %s\n", filename, SDL_GetError());
	}
	return conv;
}

GLuint sdl_create_texture( const char *filename ) {
	GLuint t = 0;
	SDL_Surface *s = sdl_load_image( filename );
	if( s ) {
		ogl_create_texture( s, &t );
		SDL_FreeSurface( s );
	}
	return t;
}

void sdl_frame_delay( void ) {
	SDL_framerateDelay( &manager );
}

void sdl_swap( void ) {
	SDL_GL_SwapBuffers();
}


