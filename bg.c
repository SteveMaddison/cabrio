#include "bg.h"
#include "sdl_ogl.h"

static const char *BG_DEFAULT = DATA_DIR "/pixmaps/default_background.jpg";

static GLuint bg_clear_texture = 0;
static GLuint bg_texture = 0;
static const GLfloat ANGLE_STEP = 0.02;
static GLfloat angle = 0;

void bg_clear( void ) {
	bg_texture = bg_clear_texture;
}

int bg_init( void ) {
	int x,y;
	bg_clear_texture = sdl_create_texture( BG_DEFAULT, &x, &y );
	if( bg_clear_texture == 0 ) {
		fprintf( stderr, "Warning: couldn't create default background texture from '%s'\n", BG_DEFAULT );
		return -1;
	}

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
		glColor4f( 1.0, 1.0, 1.0, 0.25 );
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
		angle += ANGLE_STEP;
	}
}

