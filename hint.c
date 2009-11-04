#include "hint.h"
#include "font.h"
#include "config.h"
#include "sdl_ogl.h"

#define ORIENT_LEFT		0
#define ORIENT_RIGHT	1

static const GLfloat ARROW_SIZE = 1.0;
static const GLfloat BUTTON_SIZE = 1.0;
static const GLfloat ALPHA_MIN = 0.5;
static const GLfloat ALPHA_STEP_SIZE = 0.02;
static const GLfloat FONT_SCALE = 0.005;
static const GLfloat ALPHA_STEP_MIN = 0.01;
static GLfloat alpha = 1.0;
static GLfloat alpha_step = 0.01;
static struct texture *arrow_texture = NULL;
static struct texture *select_texture = NULL;
static struct texture *back_texture = NULL;
static struct texture *text_select_message = NULL;
static struct texture *text_back_message = NULL;

int hint_init( void ) {
	int frame_rate = config_get()->iface.frame_rate;
	
	arrow_texture = sdl_create_texture( DATA_DIR "/pixmaps/arrow.png" );
	back_texture = sdl_create_texture( DATA_DIR "/pixmaps/button_blue.png" );
	select_texture = sdl_create_texture( DATA_DIR "/pixmaps/button_red.png" );
	if(!( text_select_message = font_create_texture( "Select" ) ))
		return -1;
	if(!( text_back_message = font_create_texture( "Back" ) ))
		return -1;

	if( frame_rate )
		alpha_step = frame_rate/6000;
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
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();
	GLfloat size = (BUTTON_SIZE/2) * xfactor;
	
	ogl_load_alterego();
	glTranslatef( position * xfactor, -1.9 * yfactor, -6 );
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

void hint_draw_arrow( GLfloat x, GLfloat y, int orientation ) {
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();
	GLfloat size = (ARROW_SIZE/2) * xfactor;

	ogl_load_alterego();
	glTranslatef( x * xfactor, y * yfactor, -6 );
	glColor4f( 1.0, 1.0, 1.0, alpha );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, arrow_texture->id );
	glBegin( GL_QUADS );
	if( orientation == ORIENT_LEFT ) {
		glTexCoord2f(1.0, 0.0); glVertex3f(-size,  size, 0.0);
		glTexCoord2f(0.0, 0.0); glVertex3f(-size, -size, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f( size, -size, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( size,  size, 0.0);	
	}
	else {
		glTexCoord2f(0.0, 1.0); glVertex3f(-size,  size, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f(-size, -size, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( size, -size, 0.0);
		glTexCoord2f(0.0, 0.0); glVertex3f( size,  size, 0.0);
	}
	glEnd();
}

void hint_draw_caption( struct texture *message, GLfloat position ) {
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();
	GLfloat tx = ((message->width*FONT_SCALE)/2) * xfactor;
	GLfloat ty = ((message->height*FONT_SCALE)/2) * xfactor;
	
	ogl_load_alterego();
	glTranslatef( position * xfactor, -1.9 * yfactor, -6 );
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

int hint_draw( int menu_level ) {
	if( alpha >= 1.0  ) {
		alpha_step = -ALPHA_STEP_SIZE;
	}
	else if( alpha <= ALPHA_MIN ) {
		alpha_step = ALPHA_STEP_SIZE;	
	}
	alpha += alpha_step;
	
	if( menu_level == 0 ) {
		hint_draw_button( select_texture, 0 );
		hint_draw_caption( text_select_message, (text_select_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2) );
		hint_draw_arrow( -2.9, 2, ORIENT_LEFT );
		hint_draw_arrow( 2.9, 2, ORIENT_RIGHT );
	}
	else if( menu_level == 1 ) {
		hint_draw_button( back_texture, -BUTTON_SIZE/2 );
		hint_draw_caption( text_select_message, BUTTON_SIZE/2 + ((text_select_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2)) );
		hint_draw_button( select_texture, BUTTON_SIZE/2 );		
		hint_draw_caption( text_back_message, -BUTTON_SIZE/2 - ((text_back_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2)) );
		hint_draw_arrow( -3, 1.35, ORIENT_LEFT );
		hint_draw_arrow( 3, 1.35, ORIENT_RIGHT );
	}
	else {
		hint_draw_button( back_texture, -BUTTON_SIZE/2 );
		hint_draw_caption( text_select_message, BUTTON_SIZE/2 + ((text_select_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2)) );
		hint_draw_button( select_texture, BUTTON_SIZE/2 );		
		hint_draw_caption( text_back_message, -BUTTON_SIZE/2 - ((text_back_message->width*FONT_SCALE)/2 + (BUTTON_SIZE/2)) );
		hint_draw_arrow( -3, 0.5, ORIENT_LEFT );
		hint_draw_arrow( 3, 0.5, ORIENT_RIGHT );
	}

	
	return 0;
}

