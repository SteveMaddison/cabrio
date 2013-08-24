#ifndef _FONT_H_
#define _FONT_H_ 1

#include "ogl.h"

int font_init( void );
void font_pause( void );
int font_resume( void );
void font_free( void );
struct texture *font_create_texture( const char *text );

#endif

