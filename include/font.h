#ifndef _FONT_H_
#define _FONT_H_ 1

#include "ogl.h"

struct font_message {
	GLuint texture;
	int width;
	int height;
};

int font_init( void );
void font_pause( void );
int font_resume( void );
void font_free( void );
void font_message_free( struct font_message *m );
struct font_message *font_message_create( const char *text );
SDL_Surface *font_render( const char *text );

#endif

