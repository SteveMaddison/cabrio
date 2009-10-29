#include "bg.h"
#include "config.h"
#include "sdl_ogl.h"

static GLuint bg_clear_texture = 0;
static GLuint bg_texture = 0;
static GLfloat angle_step = 0;
static GLfloat angle = 0;
static GLfloat alpha = 1.0;

void bg_clear( void ) {
	bg_texture = bg_clear_texture;
}

int bg_init( void ) {
	const struct config *config = config_get();

	if( config->iface.background_image[0] != '\0' ) {
		bg_clear_texture = sdl_create_texture( config->iface.background_image, NULL, NULL );	
		if( bg_clear_texture == 0 ) {
			fprintf( stderr, "Warning: couldn't create default background texture from '%s'\n", config->iface.background_image );
			return -1;
		}
	}

	angle_step = (GLfloat)config->iface.background_rotation/1000;
	alpha = 1.0 - (GLfloat)(config->iface.background_transparency)/100;

	bg_clear();
	return 0;
}

void bg_free( void ) {
	if( bg_texture != bg_clear_texture ) {
		ogl_free_texture( &bg_clear_texture );
	}
	ogl_free_texture( &bg_texture );
}

void bg_pause( void ) {
	bg_free();
}

int bg_resume( void ) {
	return bg_init();
}

int bg_set( const char *filename ) {
	if( filename && *filename ) {
		int x,y;
		if( bg_texture != bg_clear_texture ) {
			ogl_free_texture( &bg_texture );
		}
		bg_texture = sdl_create_texture( filename, &x, &y );
		if( bg_texture == 0 ) {
			fprintf( stderr, "Warning: couldn't load background image '%s'\n", filename );
			return -1;
		}
	}
	else {
		bg_clear();
	}
	return 0;
}

void bg_draw( void ) {
	if( bg_texture ) {
		glLoadIdentity();
		glEnable( GL_TEXTURE_2D );
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		glBindTexture( GL_TEXTURE_2D, bg_texture );
		glColor4f( 1.0, 1.0, 1.0, alpha );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glRotatef( angle, 0.0, 0.0, 1.0 );
		glTranslatef( 0.0, 0.0, -10.0 );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f( -10.0,  10.0, 0.0 );
			glTexCoord2f(0.0, 1.0); glVertex3f( -10.0, -10.0, 0.0 );
			glTexCoord2f(1.0, 1.0); glVertex3f(  10.0, -10.0, 0.0 );
			glTexCoord2f(1.0, 0.0); glVertex3f(  10.0,  10.0, 0.0 );
		glEnd();
		glDisable( GL_TEXTURE_2D );
		angle -= angle_step;
	}
}

