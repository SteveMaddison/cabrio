#include "screenshot.h"
#include "config.h"
#include "ogl.h"
#include "sdl_ogl.h"

static GLfloat offset = -1.0;
static GLfloat angle_x = -10;
static GLfloat angle_y = 30;
static GLfloat angle_z = 10;
static GLfloat size = 0.75;
#define NUM_NOISE 3
static struct texture *noise[NUM_NOISE];
static int frame = 0;
static int noise_skip = 10;

int screenshot_init( void ) {
	noise[0] = sdl_create_texture( DATA_DIR "/pixmaps/noise1.png" );
	noise[1] = sdl_create_texture( DATA_DIR "/pixmaps/noise2.png" );
	noise[2] = sdl_create_texture( DATA_DIR "/pixmaps/noise3.png" );
	
	if( noise[0] == 0 || noise[1] == 0 || noise[2] == 0 ) {
		fprintf( stderr, "Warning: Couldn't create texture for screenshot noise\n" );
		return -1;
	}
	
	if( config_get()->iface.frame_rate )
		noise_skip = config_get()->iface.frame_rate / 10;
	
	return 0;
}

void screenshot_free( void ) {
	int i;
	for( i = 0 ; i < NUM_NOISE ; i++ )
		ogl_free_texture( noise[i] );
}

void screenshot_draw( void ) {
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();

	ogl_load_alterego();
	glTranslatef( offset * xfactor, 0 * yfactor, -4 );
	glRotatef( angle_x, 1.0, 0.0, 0.0 );
	glRotatef( angle_y, 0.0, 1.0, 0.0 );
	glRotatef( angle_z, 0.0, 0.0, 1.0 );
	glColor4f( 1.0, 1.0, 1.0, 1.0 );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, noise[frame/noise_skip]->id );
	glBegin( GL_QUADS );
		glTexCoord2f(0.0, 0.0); glVertex3f(-size,  size*0.75, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size, -size*0.75, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( size, -size*0.75, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( size,  size*0.75, 0.0);	
	glEnd();
	
	if( ++frame >= NUM_NOISE * noise_skip )
		frame = 0;
}
