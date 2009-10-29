#include "ogl.h"
#include "config.h"

static int width = 0;
static int height = 0;
static int rotation = 0;
static GLfloat xscale = 1.0;
static GLfloat yscale = 1.0;

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

	width = config->iface.screen_width;
	height = config->iface.screen_height;
	
	if( config->iface.screen_hflip )
		yscale = -yscale;
	if( config->iface.screen_vflip )
		xscale = -xscale;
	
	ogl_screen_rotate( config->iface.screen_rotation );	
	ogl_load_alterego();

	error = glGetError();
	if( error == GL_NO_ERROR ) {
		return 0;
	}
	else {
		fprintf(stderr, "Error: couldn't initialise OpenGL: %s\n", gluErrorString(error) );
		return -1;
	}
}

int ogl_screen_width( void ) {
	return width;
}

int ogl_screen_height( void ) {
	return height;
}

GLfloat ogl_xfactor( void ) {
	return width/(GLfloat)config_get()->iface.screen_width;
}

GLfloat ogl_yfactor( void ) {
	return height/(GLfloat)config_get()->iface.screen_height;
}

int ogl_screen_orientation( void ) {
	if( height > width )
		return PORTRAIT;
	else
		return LANDSCAPE;
}

void ogl_screen_rotate( int angle ) {
	int tmp = -angle; /* Switch clockwise to anti-clockwise */
	if( tmp % 90 != 0 ) {
		tmp -= (tmp % 90);
		fprintf( stderr, "Warning: Screen rotation rounded down to nearest 90 degrees\n" );
	}
	if( (rotation % 180 == 0 && tmp % 180 != 0)
	||  (rotation % 180 != 0 && tmp % 180 == 0) ) {
		/* Swap height and width */
		int swap = width;
		width = height;
		height = swap;
	}
	rotation = tmp;
}

void ogl_load_alterego( void ) {
	glLoadIdentity();
	glRotatef( (GLfloat)rotation, 0.0, 0.0, 1.0 );
	glScalef( xscale, yscale, 1.0 );
}

void ogl_free_texture( GLuint *t ) {
	if( t && *t ) {
		glDeleteTextures( 1, t );
	}
}

void ogl_clear( void ) {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	void ogl_load_alterego();
}

void ogl_flush( void ) {
	glFlush();
}

