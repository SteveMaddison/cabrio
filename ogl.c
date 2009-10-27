#include <SDL/SDL_opengl.h>
#include <SDL/SDL.h>
#include "ogl.h"
#include "config.h"

int ogl_init( void ) {
	const struct config *config = config_get();
	
	GLenum error = GL_NO_ERROR;

	glLoadIdentity();
	glClearColor( 0, 0, 0, 1.0 );
	glEnable( GL_TEXTURE_2D );
	glShadeModel( GL_SMOOTH );
	
	glClearDepth( 1.0 );
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	
	glEnable( GL_BLEND );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	gluPerspective( 45.0, (GLfloat)config->iface.screen_width/(GLfloat)config->iface.screen_height, 0.1, 100.0 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	error = glGetError();
	if( error == GL_NO_ERROR ) {
		return 0;
	}
	else {
		fprintf(stderr, "Error: couldn't initialise OpenGL: %s\n", gluErrorString(error) );
		return -1;
	}
}

void ogl_free_texture( GLuint *t ) {
	if( t && *t ) {
		glDeleteTextures( 1, t );
	}
}

void ogl_clear( void ) {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
}

void ogl_flush( void ) {
	glFlush();
}

