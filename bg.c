#include "bg.h"
#include "config.h"
#include "sdl_ogl.h"
#include "location.h"

static const GLfloat BG_SIZE = 7.0;
static struct texture *bg_clear_texture = NULL;
static struct texture *bg_texture = NULL;
static GLfloat angle_step = 0;
static GLfloat angle = 0;
static GLfloat alpha = 1.0;

void bg_clear( void ) {
	bg_texture = bg_clear_texture;
}

int bg_init( void ) {
	const struct config *config = config_get();
	char filename[CONFIG_FILE_NAME_LENGTH];

	if( config->iface.theme.background_image[0] != '\0' ) {
		location_get_theme_path( config->iface.theme.background_image , filename );
		bg_clear_texture = sdl_create_texture( filename );	
		if( bg_clear_texture == NULL ) {
			fprintf( stderr, "Warning: couldn't create background texture from '%s'\n", config->iface.theme.background_image );
			return -1;
		}
	}

	alpha = 1.0 - (GLfloat)(config->iface.theme.background_transparency)/100;
	
	if( config->iface.frame_rate )
		angle_step = (GLfloat)config->iface.theme.background_rotation/((GLfloat)config->iface.frame_rate*20);
	else 
		angle_step = (GLfloat)config->iface.theme.background_rotation/1000;

	bg_clear();
	return 0;
}

void bg_free( void ) {
	if( bg_texture != bg_clear_texture ) {
		ogl_free_texture( bg_clear_texture );
	}
	ogl_free_texture( bg_texture );
}

void bg_pause( void ) {
	bg_free();
}

int bg_resume( void ) {
	return bg_init();
}

int bg_set( const char *filename ) {
	if( filename && *filename ) {
		if( bg_texture != bg_clear_texture ) {
			ogl_free_texture( bg_texture );
		}
		bg_texture = sdl_create_texture( filename );
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
		ogl_load_alterego();
		glEnable( GL_TEXTURE_2D );
		glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		glBindTexture( GL_TEXTURE_2D, bg_texture->id );
		glColor4f( 1.0, 1.0, 1.0, alpha );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glRotatef( angle, 0.0, 0.0, 1.0 );
		glTranslatef( 0.0, 0.0, -10.0 );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f( -BG_SIZE,  BG_SIZE, 0.0 );
			glTexCoord2f(0.0, 1.0); glVertex3f( -BG_SIZE, -BG_SIZE, 0.0 );
			glTexCoord2f(1.0, 1.0); glVertex3f(  BG_SIZE, -BG_SIZE, 0.0 );
			glTexCoord2f(1.0, 0.0); glVertex3f(  BG_SIZE,  BG_SIZE, 0.0 );
		glEnd();
		glDisable( GL_TEXTURE_2D );
		angle -= angle_step;
	}
}

