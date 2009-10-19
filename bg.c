#include "bg.h"
#include "sdl.h"
#include "ogl.h"

static const char *BG_DEFAULT = DATA_DIR "/pixmaps/default_background.jpg";

static GLuint bg_clear_texture = 0;
static GLuint bg_texture = 0;
static const GLfloat ANGLE_STEP = 0.02;
static GLfloat angle = 0;

void bg_clear( void ) {
	bg_texture = bg_clear_texture;
}

int bg_init( void ) { 
	SDL_Surface *bg = sdl_load_image( BG_DEFAULT );
	if( bg == NULL ) {
		fprintf( stderr, "Warning: couldn't load default background image '%s'\n", BG_DEFAULT );
		bg_clear_texture = 0;
		return -1;
	}
	ogl_create_texture( bg, &bg_clear_texture );
	SDL_FreeSurface( bg );
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
		SDL_Surface *bg = sdl_load_image( filename );
		if( bg == NULL ) {
			fprintf( stderr, "Warning: couldn't load background image '%s'\n", filename );
			bg_texture = 0;
			return -1;
		}
		if( bg_texture != bg_clear_texture ) {
			glDeleteTextures( 1, &bg_texture );
		}
		ogl_create_texture( bg, &bg_texture );
		SDL_FreeSurface( bg );
	}
	else {
		bg_texture = bg_clear_texture;
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

