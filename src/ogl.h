#ifndef _OGL_H_
#define _OGL_H_ 1

#include <GL/glu.h>

#define X 0
#define Y 1
#define Z 2

struct texture {
	GLuint id;
	int width;
	int height;
};

int ogl_init( void );
void ogl_load_alterego( void );
int ogl_screen_width( void );
int ogl_screen_height( void );
int ogl_screen_orientation( void );
GLfloat ogl_xfactor( void );
GLfloat ogl_yfactor( void );
GLfloat ogl_aspect_ratio( void );
void ogl_screen_rotate( int angle );
struct texture *ogl_create_empty_texture( void );
void ogl_free_texture( struct texture *t );
void ogl_clear( void );
void ogl_flush( void );
int ogl_nopt_textures( void );

#endif

