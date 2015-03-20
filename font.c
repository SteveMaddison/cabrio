#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include "config.h"
#include "font.h"
#include "sdl_ogl.h"

static const int DEFAULT_FONT_SIZE = 50;
static TTF_Font *font = NULL;
static SDL_Color col = { 255, 255, 255 };

int font_init( void ) {
	const struct config *config = config_get();
	int size;

	if( config->iface.theme.font_size > 0 )
		size = config->iface.theme.font_size;
	else
		size = DEFAULT_FONT_SIZE;

	col.r = config->iface.theme.font_rgb.red;
	col.g = config->iface.theme.font_rgb.green;
	col.b = config->iface.theme.font_rgb.blue;

	if( TTF_Init() != 0 ) {
		fprintf( stderr, "Error: Couldn't initialise font library: %s\n", TTF_GetError() );
		return -1;
	}
		
	if( config->iface.theme.font_file && *config->iface.theme.font_file ) {
		font = TTF_OpenFont( config->iface.theme.font_file, size );
		if( font == NULL ) {
			fprintf( stderr, "Error: Couldn't load font '%s': %s\n", config->iface.theme.font_file, TTF_GetError() );
			return -1;
		}
	}

	return 0;
}

void font_free( void ) {
	if( font )
		TTF_CloseFont( font );
		
	TTF_Quit();
}

void font_pause( void ) {
	font_free();
}

int font_resume( void ) {
	return font_init();
}

SDL_Surface *font_render( const char *text ) {
	return TTF_RenderText_Blended( font, text, col );
}

struct texture *font_create_texture( const char *text ) {
	struct texture *t = NULL;
	SDL_Surface *s = font_render( text );
	if( s ) {
		t = ogl_create_texture( s );
		if( t == NULL ) {
			fprintf( stderr, "Error: Couldn't create texture for text '%s'\n", text );
		}
		SDL_FreeSurface( s );
	}
	return t;
}

