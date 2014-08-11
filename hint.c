#include "hint.h"
#include "font.h"
#include "config.h"
#include "sdl_ogl.h"
#include "focus.h"
#include "location.h"

#define ORIENT_LEFT		0
#define ORIENT_RIGHT	1

static const GLfloat ARROW_SIZE = 1.0;
static const GLfloat BUTTON_SIZE = 0.75;
static const GLfloat ALPHA_MIN = 0.5;
static const GLfloat ALPHA_STEP_SIZE = 0.02;
static const GLfloat FONT_SCALE = 0.004;
static const GLfloat ALPHA_STEP_MIN = 0.01;
static const GLfloat DEPTH = -6.0;
static GLfloat alpha = 1.0;
static GLfloat alpha_step = 0.01;
static struct texture *arrow_texture = NULL;
static struct texture *select_texture = NULL;
static struct texture *back_texture = NULL;
static struct texture *text_select_message = NULL;
static struct texture *text_back_message = NULL;

int hint_init( void ) {
	const struct config *config = config_get();
	char filename[CONFIG_FILE_NAME_LENGTH];
	
	location_get_theme_path( config->iface.theme.hints.image_arrow, filename );
	if(!( arrow_texture = sdl_create_texture( filename ) ))
		return -1;

	location_get_theme_path( config->iface.theme.hints.image_back, filename );
	if(!( back_texture = sdl_create_texture( filename ) ))
		return -1;

	location_get_theme_path( config->iface.theme.hints.image_select, filename );
	if(!( select_texture = sdl_create_texture( filename ) ))
		return -1;
		
	if(!( text_select_message = font_create_texture( config->iface.labels.label_select ) ))
		return -1;

	if(!( text_back_message = font_create_texture( config->iface.labels.label_back ) ))
		return -1;

	if( config->iface.frame_rate )
		alpha_step = (GLfloat)config->iface.frame_rate/6000;
	else
		alpha_step = ALPHA_STEP_MIN;
	
	return 0;
}

void hint_free( void ) {
	if( arrow_texture )
		ogl_free_texture( arrow_texture );
	if( select_texture )
		ogl_free_texture( select_texture );
	if( back_texture )
		ogl_free_texture( back_texture );
	if( text_select_message )
		ogl_free_texture( text_select_message );
	if( text_back_message )
		ogl_free_texture( text_back_message );
}

void hint_pause( void ) {
	hint_free();
}

int hint_resume( void ) {
	return hint_init();
}

void hint_draw_button( struct texture *texture, GLfloat position ) {
	const struct config_hints *config = &config_get()->iface.theme.hints;
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();
	GLfloat size = (BUTTON_SIZE/2) * config->size * xfactor;
	
	ogl_load_alterego();
	glTranslatef( (config->offset2 + (position * config->size)) * xfactor, config->offset1 * yfactor, DEPTH );
	glColor4f( 1.0, 1.0, 1.0, alpha );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, texture->id );
	glBegin( GL_QUADS );
		glTexCoord2f(0.0, 0.0); glVertex3f(-size,  size, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size, -size, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( size, -size, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( size,  size, 0.0);
	glEnd();
}

void hint_draw_caption( struct texture *message, GLfloat position ) {
	const struct config_hints *config = &config_get()->iface.theme.hints;
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();
	GLfloat tx = ((message->width*FONT_SCALE)/2) * config->size * xfactor;
	GLfloat ty = ((message->height*FONT_SCALE)/2) * config->size * xfactor;
	
	ogl_load_alterego();
	glTranslatef( (config->offset2 + (position * config->size)) * xfactor, config->offset1 * yfactor, DEPTH );
	glColor4f( 1.0, 1.0, 1.0, 1.0 );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, message->id );
	glBegin( GL_QUADS );
		glTexCoord2f(0.0, 0.0); glVertex3f(-tx,  ty, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-tx, -ty, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( tx, -ty, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( tx,  ty, 0.0);
	glEnd();
}

void hint_draw_arrow( struct arrow *arrow ) {
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();
	GLfloat size = arrow->size/2 * xfactor;

	ogl_load_alterego();
	glTranslatef( arrow->x * xfactor, arrow->y * yfactor, DEPTH );
	glRotatef( arrow->angle, 0.0, 0.0, 1.0 );
	glColor4f( 1.0, 1.0, 1.0, alpha );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, arrow_texture->id );
	glBegin( GL_QUADS );
		glTexCoord2f(0.0, 0.0); glVertex3f(-size,  size, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size, -size, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( size, -size, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( size,  size, 0.0);	
	glEnd();
}

int hint_draw( void ) {
	GLfloat spacing = config_get()->iface.theme.hints.spacing;

	if( config_get()->iface.theme.hints.pulse ) {
		if( alpha >= 1.0  ) {
			alpha_step = -ALPHA_STEP_SIZE;
		}
		else if( alpha <= ALPHA_MIN ) {
			alpha_step = ALPHA_STEP_SIZE;	
		}
		alpha += alpha_step;
	}
	
	switch( focus_has() ) {
		case FOCUS_MENU:
			hint_draw_button( select_texture, 0 );
			hint_draw_caption( text_select_message, (text_select_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2) );
			break;
		case FOCUS_SUBMENU:
			hint_draw_button( back_texture, -BUTTON_SIZE/2 );
			hint_draw_caption( text_select_message, BUTTON_SIZE/2 + ((text_select_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2)) );
			hint_draw_button( select_texture, BUTTON_SIZE/2 );		
			hint_draw_caption( text_back_message, -BUTTON_SIZE/2 - ((text_back_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2)) );
			break;
		case FOCUS_GAMESEL:
			hint_draw_button( back_texture, -(spacing/2) - (BUTTON_SIZE/2) );
			hint_draw_caption( text_back_message, -(spacing/2) - BUTTON_SIZE/2 - ((text_select_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2)) );
			hint_draw_button( select_texture, BUTTON_SIZE/2 + (spacing/2) );		
			hint_draw_caption( text_select_message, BUTTON_SIZE/2 + (spacing/2) + ((text_back_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2)) );
			break;
		default:
			break;
	}
	
	return 0;
}

