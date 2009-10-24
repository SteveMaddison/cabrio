#include "event.h"
#include "control.h"
#include "config.h"
#include "sdl.h"

#define MAX_JOYSTICKS 8

static const int AXIS_THRESHOLD = 8000;
static const int BALL_THRESHOLD = 100;
static const int MOUSE_THRESHOLD = 100;
static int num_joysticks = 0;
static SDL_Joystick *joysticks[MAX_JOYSTICKS];

extern struct config* config;

int event_init( void ) {
	int i;
	
	num_joysticks = SDL_NumJoysticks();
	if( num_joysticks > MAX_JOYSTICKS )
		num_joysticks = MAX_JOYSTICKS;

	for( i = 0 ; i < num_joysticks ; i++ ) {
		SDL_JoystickOpen( i );
	}
	
	return 0;
}

void event_free( void ) {
	int i;
	
	for( i = 0 ; i < num_joysticks ; i++ ) {
		SDL_JoystickClose( joysticks[i] );
	}
}

void event_pause( void ) {
	event_free();
}

int event_resume( void ) {
	return event_init();
}

int event_get( void ) {
	struct config_control *control = config->iface.controls;
	SDL_Event event;

	if( event.type == SDL_QUIT ) {
		return EVENT_QUIT;
	}
	
	SDL_PollEvent( &event );
	while( control ) {
		if( control->device_type == DEV_KEYBOARD ) {
			if( event.type == SDL_KEYDOWN && event.key.keysym.sym == control->value ) {
				return control->event;
			}
		}
		else if( control->device_type == DEV_JOYSTICK ) {
			if( event.type == SDL_JOYAXISMOTION
			&&  event.jaxis.which == control->device_id
			&&  control->control_type == CTRL_AXIS
			&&  event.jaxis.axis == control->control_id ) {
				if( event.jaxis.value < -AXIS_THRESHOLD && control->value < 0 ) {
					return control->event;
				}
				else if ( event.jaxis.value > AXIS_THRESHOLD && control->value > 0 ) {
					return control->event;
				}
			}
			else if ( event.type == SDL_JOYBUTTONDOWN
			&&  event.jbutton.which == control->device_id
			&&  control->control_type == CTRL_BUTTON
			&&  event.jbutton.button == control->value ) {
				return control->event;
			}
			else if ( event.type == SDL_JOYHATMOTION
			&&  event.jhat.which == control->device_id
			&&  control->control_type == CTRL_HAT
			&&  event.jhat.hat == control->control_id
			&&  event.jhat.value == control->value ) {
				return control->event;
			}
			else if ( event.type == SDL_JOYBALLMOTION
			&&  event.jball.which == control->device_id
			&&  control->control_type == CTRL_BALL
			&&  event.jball.ball == control->control_id ) {
				if( control->value == DIR_UP && event.jball.yrel > BALL_THRESHOLD ) {
					return control->event;
				}
				else if( control->value == DIR_DOWN && event.jball.yrel < -BALL_THRESHOLD ) {
					return control->event;
				}
				else if( control->value == DIR_LEFT && event.jball.xrel < -BALL_THRESHOLD ) {
					return control->event;
				}
				else if( control->value == DIR_RIGHT && event.jball.xrel > BALL_THRESHOLD ) {
					return control->event;
				}
			}		
		}
		else if( control->device_type == DEV_MOUSE ) {
			if( event.type == SDL_MOUSEBUTTONDOWN 
			&& event.button.which == control->device_id
			&& control->control_type == CTRL_BUTTON
			&& event.button.button == control->value ) {
				return control->event;
			}
			else if ( event.type == SDL_MOUSEMOTION
			&& event.motion.which == control->device_id
			&& control->control_type == CTRL_AXIS ) {
				if( control->value == DIR_UP && event.motion.yrel > MOUSE_THRESHOLD ) {
					return control->event;
				}
				else if( control->value == DIR_DOWN && event.motion.yrel < -MOUSE_THRESHOLD ) {
					return control->event;
				}
				else if( control->value == DIR_LEFT && event.motion.xrel < -MOUSE_THRESHOLD ) {
					return control->event;
				}
				else if( control->value == DIR_RIGHT && event.motion.xrel > MOUSE_THRESHOLD ) {
					return control->event;
				}
			}
		}
		control = control->next;
	}
	
	return EVENT_NONE;
}

