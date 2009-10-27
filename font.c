#include <SDL/SDL_ttf.h>
#include <stdio.h>
#include "config.h"
#include "font.h"
#include "sdl_ogl.h"

static TTF_Font *font = NULL;
static SDL_Color col = { 255, 255, 255 };

int font_init( void ) {
	if( TTF_Init() != 0 ) {
		fprintf( stderr, "Error: Couldn't initialise font library: %s\n", TTF_GetError() );
		return -1;
	}
	
	font = TTF_OpenFont( DATA_DIR "/fonts/FreeSans.ttf", 50 );
	if( font == NULL ) {
		fprintf( stderr, "Error: Couldn't load font: %s\n", TTF_GetError() );
		return -1;
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

void font_message_free( struct font_message *m ) {
	if( m ) {
		if( m->texture )
			ogl_free_texture( &m->texture );
		free( m );
		m = NULL;
	}
}

struct font_message *font_message_create( const char *text ) {
	struct font_message *m = malloc( sizeof(struct font_message) );
	if( m == NULL ) {
		fprintf( stderr, "Error: Couldn't allocate meneory for text object '%s'\n", text );
	}
	else {	
		SDL_Surface *s = font_render( text );
		if( s ) {
			ogl_create_texture( s, &m->texture );
			if( m->texture == 0 ) {
				fprintf( stderr, "Error: Couldn't create texture for text '%s'\n", text );
				free( m );
				return NULL;
			}
			m->width = s->w;
			m->height = s->h;
			SDL_FreeSurface( s );
		}
	}
	return m;
}

