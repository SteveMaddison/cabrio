#include <stdio.h>
#include "config.h"
#include "sdl_wrapper.h"

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_framerate.h>
#include <SDL/SDL_opengl.h>

static SDL_Surface *screen = NULL;

static const int SDL_SCREEN_BPP = 32;
static const char *title = "Cabrio";
static FPSmanager manager;

int sdl_init( void ) {
	int mode = SDL_SWSURFACE|SDL_OPENGL;
	const struct config *config = config_get();
	
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

void sdl_frame_delay( void ) {
	SDL_framerateDelay( &manager );
}

void sdl_swap( void ) {
	SDL_GL_SwapBuffers();
}

int sdl_hat_dir_value( int direction ) {
	switch( direction ) {
		case SDL_HAT_UP:
			return DIR_UP;
			break;
		case SDL_HAT_DOWN:
			return DIR_DOWN;
			break;
		case SDL_HAT_LEFT:
			return DIR_LEFT;
			break;
		case SDL_HAT_RIGHT:
			return DIR_RIGHT;
			break;
		default:
			fprintf( stderr, "Warning: Bogus hat direction %d\n", direction );
			break;
	}
	return 0;
}

