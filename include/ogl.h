#ifndef _OGL_H_
#define _OGL_H_ 1

#include <GL/glu.h>

#define X 0
#define Y 1
#define Z 2

int ogl_init( void );
void ogl_free_texture( GLuint *t );
void ogl_clear( void );
void ogl_flush( void );

#endif

