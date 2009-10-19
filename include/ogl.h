#ifndef _OGL_H_
#define _OGL_H_ 1

#include "sdl.h"
#include <SDL/SDL_opengl.h>
#include <GL/glu.h>

#define X 0
#define Y 1
#define Z 2

int ogl_init( void ); 
int ogl_create_texture( SDL_Surface *surface, GLuint *texture );
void ogl_free_texture( GLuint *t );
void ogl_flush( void );

#endif

