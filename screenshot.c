#include "screenshot.h"
#include "config.h"
#include "ogl.h"
#include "sdl_ogl.h"

static GLfloat offset = -1.0;
static GLfloat angle_x = -10;
static GLfloat angle_y = 30;
static GLfloat angle_z = 10;
static GLfloat scale = 0.006;
static const GLfloat max_width = 280;
static const GLfloat max_height = 280;
static struct texture *current = NULL;
#define NUM_NOISE 3
static struct texture *noise[NUM_NOISE];
static int frame = 0;
static int noise_skip = 10;

int screenshot_init( void ) {
	int i;
	noise[0] = sdl_create_texture( DATA_DIR "/pixmaps/noise1.png" );
	noise[1] = sdl_create_texture( DATA_DIR "/pixmaps/noise2.png" );
	noise[2] = sdl_create_texture( DATA_DIR "/pixmaps/noise3.png" );
	
	if( noise[0] == 0 || noise[1] == 0 || noise[2] == 0 ) {
		fprintf( stderr, "Warning: Couldn't create texture for screenshot noise\n" );
		return -1;
	}
	
	for( i = 0 ; i < NUM_NOISE ; i++ ) {
		noise[i]->width = max_width;
		noise[i]->height = max_width * 0.75;
	}
	
	if( config_get()->iface.frame_rate )
		noise_skip = config_get()->iface.frame_rate / 10;
	
	return 0;
}

void screenshot_free( void ) {
	int i;
	
	for( i = 0 ; i < NUM_NOISE ; i++ )
		ogl_free_texture( noise[i] );
		
	if( current )
		ogl_free_texture( current );
	current = NULL;
}

int screenshot_set( const char *filename ) {
	if( filename && filename[0] ) {
		current = sdl_create_texture( filename );
		if( current ) {
			if( current->width > current->height ) {
				if( current->width > max_width ) {
					current->height = (int)(float)current->height/((float)current->width/max_width);
					current->width = max_width;
				}
			}
			else {
				if( current->height > max_height ) {
					current->width = (int)(float)current->width/((float)current->height/max_width);
					current->height = max_width;
				}
			}
			return 0;
		}
	}
	current = NULL;
	return 0;
}

void screenshot_clear( void ) {
	if( current )
		ogl_free_texture( current );
	current = NULL;
}

void screenshot_draw( void ) {
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();
	struct texture *texture = current;
	GLfloat xsize, ysize;
	
	if( texture == NULL )
		texture = noise[frame/noise_skip];

	xsize = (texture->width/2) * scale * xfactor;
	ysize = (texture->height/2) * scale * xfactor;

	ogl_load_alterego();
	glTranslatef( offset * xfactor, 0 * yfactor, -4 );
	glRotatef( angle_x, 1.0, 0.0, 0.0 );
	glRotatef( angle_y, 0.0, 1.0, 0.0 );
	glRotatef( angle_z, 0.0, 0.0, 1.0 );
	glColor4f( 1.0, 1.0, 1.0, 1.0 );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, texture->id );		
	glBegin( GL_QUADS );
		glTexCoord2f(0.0, 0.0); glVertex3f(-xsize,  ysize, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-xsize, -ysize, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( xsize, -ysize, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( xsize,  ysize, 0.0);	
	glEnd();
	
	if( ++frame >= NUM_NOISE * noise_skip )
		frame = 0;
}
