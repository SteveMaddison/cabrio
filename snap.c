#include "snap.h"
#include "game.h"
#include "config.h"
#include "ogl.h"
#include "sdl_ogl.h"
#include "media.h"
#include "video.h"

static const GLfloat DEPTH = -8;
static const GLfloat max_size = 280;
static const GLfloat scale_fator = 0.0144;
static struct texture *texture = NULL;
#define NUM_NOISE 3
static struct texture *noise[NUM_NOISE];
static int frame = 0;
static int noise_skip = 10;
static const int MAX_STEPS = 50;
static int steps = 50;
static int step = 0;
static int hide_direction = 0;
static int visible = 0;
static GLfloat scale = 0.006;
static GLfloat hidden_offset = -4.0;
static int video = 0;


int snap_init( void ) {
	const struct config *config = config_get();
	int i;
	noise[0] = sdl_create_texture( DATA_DIR "/pixmaps/noise1.png" );
	noise[1] = sdl_create_texture( DATA_DIR "/pixmaps/noise2.png" );
	noise[2] = sdl_create_texture( DATA_DIR "/pixmaps/noise3.png" );
	
	if( noise[0] == NULL || noise[1] == NULL || noise[2] == NULL ) {
		fprintf( stderr, "Warning: Couldn't create texture for snap noise\n" );
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
	
	scale = config->iface.theme.snap.size * scale_fator;
	
	if( config->iface.theme.snap.offset1 > 0 )
		hidden_offset = -hidden_offset;
	
	return 0;
}

void snap_free( void ) {
	int i;
	
	for( i = 0 ; i < NUM_NOISE ; i++ ) {
		ogl_free_texture( noise[i] );
		noise[i] = NULL;
	}
		
	if( texture )
		ogl_free_texture( texture );
	texture = NULL;
}

void snap_pause( void ) {
	snap_free();
}

int snap_resume( void ) {
	return snap_init();
}

int snap_set( struct game *game ) {
	const struct config_snap *config = &config_get()->iface.theme.snap;
	char *filename;

	snap_clear();
	
	filename = game_media_get( game, MEDIA_VIDEO, NULL );

	if( filename && filename[0] ) {
		video = 1;
		video_open( filename );
		texture = video_texture();
	}
	else {
		video = 0;
		filename = game_media_get( game, MEDIA_IMAGE, image_type_name(IMAGE_SCREENSHOT) );
		if( filename && filename[0] ) {
			texture = sdl_create_texture( filename );
		}
		else {
			return -1;
		}
	}

	if( texture ) {
		if( config->fix_aspect_ratio ) {
			if( texture->width > texture->height ) {
				/* Landscape */
				texture->width = max_size;
				texture->height = max_size / ogl_aspect_ratio();
			}
			else {
				/* Portrait */
				texture->height = max_size;
				texture->width = max_size / ogl_aspect_ratio();
			}				
		}
		else {
			if( texture->width > texture->height ) {
				/* Landscape */
				texture->height = (int)(float)texture->height/((float)texture->width/max_size);
				texture->width = max_size;
			}
			else {
				/* Portrait */
				texture->width = (int)(float)texture->width/((float)texture->height/max_size);
				texture->height = max_size;
			}
		}
		return 0;
	}

	texture = NULL;
	return -1;
}

void snap_clear( void ) {
	if( (!video) && texture )
		ogl_free_texture( texture );
	texture = NULL;
}

void snap_show( void ) {
	if( !visible ) {
		visible = 1;
		hide_direction = 1;
		step = steps;
	}
}

void snap_hide( void ) {
	if( visible ) {
		hide_direction = -1;
		step = steps;
	}	
}

void snap_draw( void ) {
	const struct config_snap *config = &config_get()->iface.theme.snap;
	
	if( visible ) {
		struct texture *t = texture;
		GLfloat xfactor = ogl_xfactor();
		GLfloat yfactor = ogl_yfactor();
		GLfloat xsize, ysize, hide_offset;
		
		if( video )
			video_get_frame();
		
		if( t == NULL )
			t = noise[frame/noise_skip];
	
		xsize = (t->width/2) * scale * xfactor;
		ysize = (t->height/2) * scale * xfactor;
		
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
	
		glBindTexture( GL_TEXTURE_2D, t->id );		
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-xsize,  ysize, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-xsize, -ysize, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( xsize, -ysize, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( xsize,  ysize, 0.0);	
		glEnd();
		
		if( ++frame >= NUM_NOISE * noise_skip )
			frame = 0;
			
		if( step && --step == 0 ) {
			if( hide_direction < 0 ) {
				visible = 0;
				snap_clear();
			}
			else {
				visible = 1;
			}
			hide_direction = 0;
		}
	}
}
