#include <stdio.h>
#include "setup.h"
#include "config.h"
#include "event.h"
#include "font.h"
#include "sdl_ogl.h"

static const char *str_setup = "Setup";
static const char *str_instr1 = "Please follow the on-screen instructions";
static const char *str_instr2 = "in order to set up your installation";
static const char *str_press = "Please press the control for: ";

static struct texture *msg_setup = NULL;
static struct texture *msg_instr1 = NULL;
static struct texture *msg_instr2 = NULL;
static struct texture *msg_press = NULL;

int setup_draw_msg( struct texture *msg, GLfloat y, GLfloat scale ) {
	ogl_load_alterego();
	glTranslatef( 0, y, -6 );
	glColor4f( 1.0, 1.0, 1.0, 1.0 );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, msg->id );
	glBegin( GL_QUADS );
		glTexCoord2f(0.0, 0.0); glVertex3f(-((msg->width*scale)/2),  ((msg->height*scale)/2), 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-((msg->width*scale)/2), -((msg->height*scale)/2), 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( ((msg->width*scale)/2), -((msg->height*scale)/2), 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( ((msg->width*scale)/2),  ((msg->height*scale)/2), 0.0);
	glEnd();

	
	return 0;
}

int setup( void ) {
	struct texture *msg_event = NULL;
	struct event event;
	int i;

	msg_setup = font_create_texture( str_setup );
	if( msg_setup == NULL ) {
		fprintf( stderr, "Error: Couldn't create message for string '%s'", str_setup );
		return -1;
	}
	msg_instr1 = font_create_texture( str_instr1 );
	if( msg_setup == NULL ) {
		fprintf( stderr, "Error: Couldn't create message for string '%s'", str_instr1 );
		return -1;
	}
	msg_instr2 = font_create_texture( str_instr2 );
	if( msg_setup == NULL ) {
		fprintf( stderr, "Error: Couldn't create message for string '%s'", str_instr2 );
		return -1;
	}
	msg_press = font_create_texture( str_press );
	if( msg_setup == NULL ) {
		fprintf( stderr, "Error: Couldn't create message for string '%s'", str_press );
		return -1;
	}

	ogl_clear();
	sdl_swap();
	event_flush();
	SDL_Delay( 500 );

	for( i = 1 ; i < NUM_EVENTS ; i++ ) {
		msg_event = font_create_texture( event_name(i) );
		ogl_clear();
		setup_draw_msg( msg_setup, 1.0, 0.012 );
		setup_draw_msg( msg_instr1, 0.2, 0.005 );
		setup_draw_msg( msg_instr2, -0.1, 0.005 );
		setup_draw_msg( msg_press, -0.5, 0.005 );
		setup_draw_msg( msg_event, -0.8, 0.005 );
		sdl_swap();
		/* Clear out any unwanted events */
		event_flush();
		/* Catch out wanted event */
		while( event_probe( 100, &event ) != 1 );
		event_set( i, &event );
		ogl_free_texture( msg_event );
	}

	ogl_free_texture( msg_setup );
	ogl_free_texture( msg_instr1 );
	ogl_free_texture( msg_instr2 );
	ogl_free_texture( msg_press );
	
	return 0;
}

