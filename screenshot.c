#include "screenshot.h"
#include "config.h"
#include "ogl.h"
#include "sdl_ogl.h"

static const GLfloat DEPTH = -8;
static const GLfloat max_size = 280;
static const GLfloat hidden_offset = -4.0;
static const GLfloat scale_fator = 0.0144;
static struct texture *current = NULL;
#define NUM_NOISE 3
static struct texture *noise[NUM_NOISE];
static int frame = 0;
static int noise_skip = 10;
static const int MAX_STEPS = 50;
static int steps = 50;
static int step = 0;
static int hide_direction = 0;
static int visible = 0;
static char last_file[CONFIG_FILE_NAME_LENGTH];
static GLfloat scale = 0.006;

int screenshot_init( void ) {
	const struct config *config = config_get();
	int i;
	noise[0] = sdl_create_texture( DATA_DIR "/pixmaps/noise1.png" );
	noise[1] = sdl_create_texture( DATA_DIR "/pixmaps/noise2.png" );
	noise[2] = sdl_create_texture( DATA_DIR "/pixmaps/noise3.png" );
	
	if( noise[0] == 0 || noise[1] == 0 || noise[2] == 0 ) {
		fprintf( stderr, "Warning: Couldn't create texture for screenshot noise\n" );
		return -1;
	}
	
	for( i = 0 ; i < NUM_NOISE ; i++ ) {
		noise[i]->width = max_size;
		noise[i]->height = max_size / ogl_aspect_ratio();
	}
	
	if( config->iface.frame_rate ) {
		noise_skip = config->iface.frame_rate / 10;
		steps = config->iface.frame_rate / 4;
	}
	else {
		steps = MAX_STEPS;
	}
	
	scale = config->iface.theme.screenshot.size * scale_fator;
	
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

void screenshot_pause( void ) {
	screenshot_free();
}

int screenshot_resume( void ) {
	int ret = screenshot_init();
	screenshot_set( last_file );
	return ret;
}

int screenshot_set( const char *filename ) {
	const struct config_screenshot *config = &config_get()->iface.theme.screenshot;

	screenshot_clear();
	if( filename && filename[0] ) {
		strncpy( last_file, filename, CONFIG_FILE_NAME_LENGTH );
		current = sdl_create_texture( filename );
		if( current ) {
			if( config->fix_aspect_ratio ) {
				if( current->width > current->height ) {
					/* Landscape */
					current->width = max_size;
					current->height = max_size / ogl_aspect_ratio();
				}
				else {
					/* Portrait */
					current->height = max_size;
					current->width = max_size / ogl_aspect_ratio();
				}				
			}
			else {
				if( current->width > current->height ) {
					/* Landscape */
					current->height = (int)(float)current->height/((float)current->width/max_size);
					current->width = max_size;
				}
				else {
					/* Portrait */
					current->width = (int)(float)current->width/((float)current->height/max_size);
					current->height = max_size;
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

void screenshot_show( void ) {
	if( !visible ) {
		visible = 1;
		hide_direction = 1;
		step = steps;
	}
}

void screenshot_hide( void ) {
	if( visible ) {
		hide_direction = -1;
		step = steps;
	}	
}

void screenshot_draw( void ) {
	const struct config_screenshot *config = &config_get()->iface.theme.screenshot;
	
	if( visible ) {
		GLfloat xfactor = ogl_xfactor();
		GLfloat yfactor = ogl_yfactor();
		struct texture *texture = current;
		GLfloat xsize, ysize, hide_offset;
		
		if( texture == NULL )
			texture = noise[frame/noise_skip];
	
		xsize = (texture->width/2) * scale * xfactor;
		ysize = (texture->height/2) * scale * xfactor;
		
		hide_offset = (((hidden_offset - config->offset1) / (GLfloat)steps) * (GLfloat)step);

		ogl_load_alterego();
		if( hide_direction == -1 ) {
			glTranslatef( (hidden_offset - hide_offset) * xfactor, config->offset2 * yfactor, DEPTH );
		}
		else {
			glTranslatef( (config->offset1 + hide_offset) * xfactor, config->offset2 * yfactor, DEPTH );
		}
		glRotatef( config->angle_x, 1.0, 0.0, 0.0 );
		glRotatef( config->angle_y, 0.0, 1.0, 0.0 );
		glRotatef( config->angle_z, 0.0, 0.0, 1.0 );
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
			
		if( step && --step == 0 ) {
			if( hide_direction < 0 )
				visible = 0;
			else
				visible = 1;
			hide_direction = 0;
		}
	}
}
