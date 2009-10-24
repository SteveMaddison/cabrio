#include "event.h"
#include "control.h"
#include "config.h"
#include "sdl.h"

#define MAX_JOYSTICKS 8

static const char *event_str[] = {
	"none",
	"up", "down", "left", "right",
	"select", "back", "quit"
};

static const int AXIS_THRESHOLD = 8000;
static const int BALL_THRESHOLD = 100;
static const int MOUSE_THRESHOLD = 100;
static int num_joysticks = 0;
static SDL_Joystick *joysticks[MAX_JOYSTICKS];
static int event = EVENT_NONE;

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
	SDL_Event sdl_event;

	if(	SDL_PollEvent( &sdl_event ) ) {
		event = EVENT_NONE;
		if( sdl_event.type == SDL_QUIT ) {
			event = EVENT_QUIT;
		}
		while( control ) {
			if( control->device_type == DEV_KEYBOARD ) {
				if( sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == control->value ) {
					event = control->event;
				}
			}
			else if( control->device_type == DEV_JOYSTICK ) {
				if( sdl_event.type == SDL_JOYAXISMOTION
				&&  sdl_event.jaxis.which == control->device_id
				&&  control->control_type == CTRL_AXIS
				&&  sdl_event.jaxis.axis == control->control_id ) {
					if( sdl_event.jaxis.value < -AXIS_THRESHOLD && control->value < 0 ) {
						event = control->event;
					}
					else if ( sdl_event.jaxis.value > AXIS_THRESHOLD && control->value > 0 ) {
						event = control->event;
					}
				}
				else if ( sdl_event.type == SDL_JOYBUTTONDOWN
				&&  sdl_event.jbutton.which == control->device_id
				&&  control->control_type == CTRL_BUTTON
				&&  sdl_event.jbutton.button == control->value ) {
					event = control->event;
				}
				else if ( sdl_event.type == SDL_JOYHATMOTION
				&&  sdl_event.jhat.which == control->device_id
				&&  control->control_type == CTRL_HAT
				&&  sdl_event.jhat.hat == control->control_id
				&&  sdl_event.jhat.value == control->value ) {
					event = control->event;
				}
				else if ( sdl_event.type == SDL_JOYBALLMOTION
				&&  sdl_event.jball.which == control->device_id
				&&  control->control_type == CTRL_BALL
				&&  sdl_event.jball.ball == control->control_id ) {
					if( control->value == DIR_DOWN && sdl_event.jball.yrel > BALL_THRESHOLD ) {
						event = control->event;
					}
					else if( control->value == DIR_UP && sdl_event.jball.yrel < -BALL_THRESHOLD ) {
						event = control->event;
					}
					else if( control->value == DIR_LEFT && sdl_event.jball.xrel < -BALL_THRESHOLD ) {
						event = control->event;
					}
					else if( control->value == DIR_RIGHT && sdl_event.jball.xrel > BALL_THRESHOLD ) {
						event = control->event;
					}
				}		
			}
			else if( control->device_type == DEV_MOUSE ) {
				if( sdl_event.type == SDL_MOUSEBUTTONDOWN 
				&& sdl_event.button.which == control->device_id
				&& control->control_type == CTRL_BUTTON
				&& sdl_event.button.button == control->value ) {
					event = control->event;
				}
				else if ( sdl_event.type == SDL_MOUSEMOTION
				&& sdl_event.motion.which == control->device_id
				&& control->control_type == CTRL_AXIS ) {
					if( control->value == DIR_UP && sdl_event.motion.yrel > MOUSE_THRESHOLD ) {
						event = control->event;
					}
					else if( control->value == DIR_DOWN && sdl_event.motion.yrel < -MOUSE_THRESHOLD ) {
						event = control->event;
					}
					else if( control->value == DIR_LEFT && sdl_event.motion.xrel < -MOUSE_THRESHOLD ) {
						event = control->event;
					}
					else if( control->value == DIR_RIGHT && sdl_event.motion.xrel > MOUSE_THRESHOLD ) {
						event = control->event;
					}
				}
			}
			control = control->next;
		}
	}
	
	return event;
}

int event_probe( int timeout ) {
	SDL_Event event;
	
	while( timeout ) {
		SDL_PollEvent( &event );
		switch( event.type ) {
			case SDL_JOYAXISMOTION:
				printf( "Joy(%d) Axis(%d) Value(%d)\n", event.jaxis.which, event.jaxis.axis, event.jaxis.value );
				return 1;
			case SDL_JOYBUTTONDOWN:
				printf( "Joy(%d) Button(%d)\n", event.jbutton.which, event.jbutton.button );
				return 1;
			case SDL_JOYHATMOTION:
				printf( "Joy(%d) Hat(%d) Dir(%d)\n", event.jhat.which, event.jhat.hat, event.jhat.value );
				return 1;
			case SDL_JOYBALLMOTION:
				printf( "Joy(%d) Ball(%d) Dir(%d,%d)\n", event.jball.which, event.jball.ball, event.jball.xrel, event.jball.yrel );
				return 1;
		}
		SDL_Delay( 10 );
		timeout -= 10;
	}
	return 0;
}

int event_id( char *name ) {
	int i;
	if( name == NULL ) {
		fprintf( stderr, "Warning: Null event name\n" );		
		return EVENT_NONE;
	}
	for( i = 0 ; i < NUM_EVENTS ; i++ ) {
		if( strcasecmp( name, event_str[i] ) == 0 ) return i;
	}

	fprintf( stderr, "Warning: Unknown event name '%s'\n", name );
	return EVENT_NONE;
}

const char *event_name( int event ) {
	if( event < 0 || event > NUM_EVENTS )
		return NULL;
	else
		return event_str[event];
}

