#include <stdio.h>
#include "config.h"
#include "sdl_wrapper.h"

#include <SDL2/SDL.h>
#ifdef __WIN32__
#define _WINCON_H 1 /* Avoid inclusion of wincon.h */
#endif
#include <SDL2/SDL_image.h>
#include <SDL2/SDL2_framerate.h>
#include <SDL2/SDL_opengl.h>

static SDL_Surface *screen = NULL;
static SDL_Window *window = NULL;
static SDL_GLContext *glcontext = NULL;

//static const int SDL_SCREEN_BPP = 32;
static const int MAX_FRAME_RATE = 100;
static const char *title = "Cabrio";
static FPSmanager manager;

void sdl_resolution_overwrite( void ) {
	SDL_DisplayMode res;

	SDL_GetDesktopDisplayMode(0, &res);
	config_resolution_overwrite( res.w, res.h );

	// debug //
//	printf( "\nDisplay resolution: %dx%d\nRefresh rate: %d Hz\n%s\n\n", res.w, res.h, res.refresh_rate, SDL_GetPixelFormatName( res.format ) );

	return;
}

void sdl_init_framerate ( int frame_rate ) {
	SDL_initFramerate( &manager );

	if( frame_rate < 0 ) {
		frame_rate = 0;
		fprintf( stderr, "Warning: Negative frame rate, setting to 0 (unlimited)\n" );
	}
	if( frame_rate > MAX_FRAME_RATE ) {
		frame_rate = MAX_FRAME_RATE;
		fprintf( stderr, "Warning: Frame rate above maximum allowed (%d) setting to maximum\n", MAX_FRAME_RATE );
	}
	if( frame_rate ) {
		SDL_setFramerate( &manager, frame_rate );
	}
	
	return;
}

int sdl_init( void ) {
	int mode = SDL_SWSURFACE|SDL_WINDOW_OPENGL;
	const struct config *c = config_get();

	if( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
		fprintf(stderr, "Error: Unable to initialise SDL: %s\n", SDL_GetError());
		return 1;
	}

	SDL_ShowCursor(SDL_DISABLE);
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	if( c->iface.full_screen ) {
		mode |= SDL_WINDOW_FULLSCREEN; // maybe use SDL_WINDOW_FULLSCREEN_DESKTOP ?
		sdl_resolution_overwrite();
	}
	window = SDL_CreateWindow( title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, c->iface.screen_width, c->iface.screen_height, mode );
	if( window == NULL ) {
		fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	glcontext = SDL_GL_CreateContext( window );

	screen = SDL_GetWindowSurface( window );

	sdl_init_framerate( c->iface.frame_rate );

	return 0;
}

void sdl_free( void ) {
	SDL_FreeSurface( screen );
	screen = NULL;
	SDL_GL_DeleteContext( glcontext );
	glcontext = NULL;
	SDL_DestroyWindow( window );
	window = NULL;
	SDL_Quit();
}

void sdl_frame_delay( int frame_rate ) {
	if( frame_rate ) {
		SDL_framerateDelay( &manager );
	}
}

void sdl_swap( void ) {
	SDL_GL_SwapWindow( window );
}

int sdl_hat_dir_value( int direction ) {
	switch( direction ) {
		case SDL_HAT_CENTERED:
			break;
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

