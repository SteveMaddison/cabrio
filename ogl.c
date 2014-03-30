#include <stdlib.h>
#include "ogl.h"
#include "config.h"

static int width = 0;
static int height = 0;
static int rotation = 0;
static int nopt_textures = 0;
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
	if( config->iface.gfx_quality > CONFIG_LOW )
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

	if( strstr( (char*)glGetString(GL_EXTENSIONS), "GL_ARB_texture_non_power_of_two" ) != NULL )
		nopt_textures = 1;

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
	return (GLfloat)width/(GLfloat)config_get()->iface.screen_width;
}

GLfloat ogl_yfactor( void ) {
	return (GLfloat)height/(GLfloat)config_get()->iface.screen_height;
}

GLfloat ogl_aspect_ratio( void ) {
	return (GLfloat)config_get()->iface.screen_width/(GLfloat)config_get()->iface.screen_height;
}

int ogl_screen_orientation( void ) {
	if( height > width )
		return CONFIG_PORTRAIT;
	else
		return CONFIG_LANDSCAPE;
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

struct texture *ogl_create_empty_texture( void ) {
	struct texture *t = malloc( sizeof(struct texture) );
	if( t ) {
		memset( t, 0, sizeof(struct texture) );
		glGenTextures( 1, &t->id );
	}
	return t;
}

void ogl_free_texture( struct texture *t ) {
	if( t ) {
		if( t->id )
			glDeleteTextures( 1, &t->id );
		free( t );
		t = NULL;
	}
}

void ogl_clear( void ) {
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	void ogl_load_alterego();
}

void ogl_flush( void ) {
	glFlush();
}

int ogl_nopt_textures( void ) {
	return nopt_textures;	
}
