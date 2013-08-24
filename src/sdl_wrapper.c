#include <stdio.h>
#include "config.h"
#include "sdl_wrapper.h"

#include <SDL/SDL.h>
#ifdef __WIN32__
#define _WINCON_H 1 /* Avoid inclusion of wincon.h */
#endif
#include <SDL/SDL_image.h>
#include <SDL/SDL_framerate.h>
#include <SDL/SDL_opengl.h>

static SDL_Surface *screen = NULL;
static SDL_VideoInfo saved_video;

static const int SDL_SCREEN_BPP = 32;
static const int MAX_FRAME_RATE = 100;
static const char *title = "Cabrio";
static FPSmanager manager;
static int frame_rate = 0;

int sdl_init( void ) {
	int mode = SDL_SWSURFACE|SDL_OPENGL;
	const struct config *config = config_get();
	
	if( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
		fprintf(stderr, "Error: Unable to initialise SDL: %s\n", SDL_GetError());
		return 1;
	}
	SDL_ShowCursor(SDL_DISABLE);
	memcpy( &saved_video, SDL_GetVideoInfo(), sizeof(SDL_VideoInfo) );
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
	if( config->iface.full_screen )
		mode |= SDL_FULLSCREEN;
	screen = SDL_SetVideoMode( config->iface.screen_width, config->iface.screen_height, SDL_SCREEN_BPP, mode );
	if( screen == NULL ) {
		fprintf(stderr, "Error: Unable to set video mode: %s\n", SDL_GetError());
		return 1;
	}
	SDL_initFramerate( &manager );
	
	frame_rate = config->iface.frame_rate;
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
	SDL_WM_SetCaption( title, NULL );
	
	return 0;
}

void sdl_free( void ) {
	if( config_get()->iface.full_screen )
		SDL_SetVideoMode( saved_video.current_w, saved_video.current_h, saved_video.vfmt->BitsPerPixel, SDL_FULLSCREEN );
	SDL_Quit();
}

void sdl_frame_delay( void ) {
	if( frame_rate ) {
		SDL_framerateDelay( &manager );
	}
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

