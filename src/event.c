#include "control.h"
#include "config.h"
#include "event.h"
#include "menu.h"
#include "submenu.h"
#include "game_sel.h"
#include "focus.h"
#include "sdl_wrapper.h"
#include <SDL/SDL.h>

#define MAX_JOYSTICKS 8

static const char *event_str[] = {
	"none",
	"up", "down", "left", "right",
	"select", "back", "quit"
};

static const int AXIS_THRESHOLD = 8000;
static const int BALL_THRESHOLD = 10;
static const int MOUSE_THRESHOLD = 10;
static int num_joysticks = 0;
static SDL_Joystick *joysticks[MAX_JOYSTICKS];
static int event = EVENT_NONE;
static struct event events[NUM_EVENTS];

int event_init( void ) {
	const struct config* config = config_get();
	int i;
	
	num_joysticks = SDL_NumJoysticks();
	if( num_joysticks > MAX_JOYSTICKS )
		num_joysticks = MAX_JOYSTICKS;

	for( i = 0 ; i < num_joysticks ; i++ ) {
		SDL_JoystickOpen( i );
	}

	for( i = 1 ; i < NUM_EVENTS ; i++ ) {
		events[i].device_type = config->iface.controls[i].device_type;
		events[i].device_id = config->iface.controls[i].device_id;
		events[i].control_type = config->iface.controls[i].control_type;
		events[i].control_id = config->iface.controls[i].control_id;
		events[i].value = config->iface.controls[i].value;
		/* printf("%s: %s%d %s%d = %d\n",
			event_name(i),
			device_name(events[i].device_type),
			events[i].device_id,
			control_name(events[i].control_type),
			events[i].control_id,
			events[i].value
		); */
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

int event_set( int id, struct event *event ) {
	if( id < 1 || id >= NUM_EVENTS ) {
		fprintf( stderr, "Warning: Can't set non-existent event %d\n", id );
		return -1;
	}
	memcpy( &events[id], event, sizeof(struct event) );
	return 0;
}

struct event *event_get( int id ) {
	if( id < 1 || id >= NUM_EVENTS ) {
		fprintf( stderr, "Warning: Can't get non-existent event %d\n", id );
		return NULL;
	}
	return &events[id];
}

void event_flush( void ) {
	SDL_Event sdl_event;
	while( SDL_PollEvent( &sdl_event ) );
}

int event_poll( void ) {
	SDL_Event sdl_event;
	int i;

	if(	SDL_PollEvent( &sdl_event ) ) {
		event = EVENT_NONE;
		if( sdl_event.type == SDL_QUIT ) {
			event = EVENT_QUIT;
		}
		for( i = 1 ; i < NUM_EVENTS ; i++ ) {
			if( events[i].device_type == DEV_KEYBOARD ) {
				if( sdl_event.type == SDL_KEYDOWN && sdl_event.key.keysym.sym == events[i].value ) {
					event = i;
				}
			}
			else if( events[i].device_type == DEV_JOYSTICK ) {
				if( sdl_event.type == SDL_JOYAXISMOTION
				&&  sdl_event.jaxis.which == events[i].device_id
				&&  events[i].control_type == CTRL_AXIS
				&&  sdl_event.jaxis.axis == events[i].control_id ) {
					if( sdl_event.jaxis.value < -AXIS_THRESHOLD && events[i].value < 0 ) {
						event = i;
					}
					else if ( sdl_event.jaxis.value > AXIS_THRESHOLD && events[i].value > 0 ) {
						event = i;
					}
				}
				else if ( sdl_event.type == SDL_JOYBUTTONDOWN
				&&  sdl_event.jbutton.which == events[i].device_id
				&&  events[i].control_type == CTRL_BUTTON
				&&  sdl_event.jbutton.button == events[i].value ) {
					event = i;
				}
				else if ( sdl_event.type == SDL_JOYHATMOTION
				&&  sdl_event.jhat.which == events[i].device_id
				&&  events[i].control_type == CTRL_HAT
				&&  sdl_hat_dir_value( sdl_event.jhat.hat ) == events[i].control_id
				&&  sdl_event.jhat.value == events[i].value ) {
					event = i;
				}
				else if ( sdl_event.type == SDL_JOYBALLMOTION
				&&  sdl_event.jball.which == events[i].device_id
				&&  events[i].control_type == CTRL_BALL
				&&  sdl_event.jball.ball == events[i].control_id ) {
					if( events[i].value == DIR_DOWN && sdl_event.jball.yrel > BALL_THRESHOLD ) {
						event = i;
					}
					else if( events[i].value == DIR_UP && sdl_event.jball.yrel < -BALL_THRESHOLD ) {
						event = i;
					}
					else if( events[i].value == DIR_LEFT && sdl_event.jball.xrel < -BALL_THRESHOLD ) {
						event = i;
					}
					else if( events[i].value == DIR_RIGHT && sdl_event.jball.xrel > BALL_THRESHOLD ) {
						event = i;
					}
				}		
			}
			else if( events[i].device_type == DEV_MOUSE ) {
				if( sdl_event.type == SDL_MOUSEBUTTONDOWN 
				&& sdl_event.button.which == events[i].device_id
				&& events[i].control_type == CTRL_BUTTON
				&& sdl_event.button.button == events[i].value ) {
					event = i;
				}
				else if ( sdl_event.type == SDL_MOUSEMOTION
				&& sdl_event.motion.which == events[i].device_id
				&& events[i].control_type == CTRL_AXIS ) {
					if( events[i].value == DIR_DOWN && sdl_event.motion.yrel > MOUSE_THRESHOLD ) {
						event = i;
					}
					else if( events[i].value == DIR_UP && sdl_event.motion.yrel < -MOUSE_THRESHOLD ) {
						event = i;
					}
					else if( events[i].value == DIR_LEFT && sdl_event.motion.xrel < -MOUSE_THRESHOLD ) {
						event = i;
					}
					else if( events[i].value == DIR_RIGHT && sdl_event.motion.xrel > MOUSE_THRESHOLD ) {
						event = i;
					}
				}
			}
		}
	}

	return event;
}

int event_probe( int timeout, struct event *event ) {
	SDL_Event sdl_event;
	
	event->device_type = DEV_UNKNOWN;
	event->device_id = 0;
	event->control_type = CTRL_UNKNOWN;
	event->control_id = 0;
	
	while( timeout > 0 ) {
		SDL_PollEvent( &sdl_event );
		switch( sdl_event.type ) {
			case SDL_KEYDOWN:
				event->device_type = DEV_KEYBOARD;
				event->device_id = sdl_event.key.which;
				event->value = sdl_event.key.keysym.sym;
				return 1;
			case SDL_JOYAXISMOTION:
				event->device_type = DEV_JOYSTICK;
				event->device_id = sdl_event.jaxis.which;
				event->control_type = CTRL_AXIS;
				event->control_id = sdl_event.jaxis.axis;
				if( sdl_event.jaxis.value > AXIS_THRESHOLD )
					event->value = 1;
				else if( sdl_event.jaxis.value < -AXIS_THRESHOLD )
					event->value = -1;
				else
					return -1;
				return 1;
			case SDL_JOYBUTTONDOWN:
				event->device_type = DEV_JOYSTICK;
				event->device_id = sdl_event.jbutton.which;
				event->control_type = CTRL_BUTTON;
				event->value = sdl_event.jbutton.button;
				return 1;
			case SDL_JOYHATMOTION:
				event->device_type = DEV_JOYSTICK;
				event->device_id = sdl_event.jhat.which;
				event->control_type = CTRL_HAT;
				event->control_id = sdl_event.jhat.hat;
				event->value = sdl_hat_dir_value( sdl_event.jhat.value );
				return 1;
			case SDL_JOYBALLMOTION:
				event->device_type = DEV_JOYSTICK;
				event->device_id = sdl_event.jball.which;
				event->control_type = CTRL_BALL;
				event->control_id = sdl_event.jball.ball;
				if( sdl_event.jball.xrel > BALL_THRESHOLD )
					event->value = DIR_LEFT;
				else if(  sdl_event.jball.xrel < -BALL_THRESHOLD )
					event->value = DIR_RIGHT;
				else if( sdl_event.jball.yrel > BALL_THRESHOLD )
					event->value = DIR_DOWN;
				else if(  sdl_event.jball.yrel < -BALL_THRESHOLD )
					event->value = DIR_UP;
				else
					return -1;
				return 1;
			case SDL_MOUSEBUTTONDOWN:
				event->device_type = DEV_MOUSE;
				event->device_id = sdl_event.button.which;
				event->control_type = CTRL_BUTTON;
				event->value = sdl_event.button.button;
				return 1;
			case SDL_MOUSEMOTION:
				event->device_type = DEV_MOUSE;
				event->device_id = sdl_event.motion.which;
				event->control_type = CTRL_AXIS;
				if( sdl_event.motion.xrel > MOUSE_THRESHOLD )
					event->value = DIR_LEFT;
				else if(  sdl_event.motion.xrel < -MOUSE_THRESHOLD )
					event->value = DIR_RIGHT;
				else if( sdl_event.motion.yrel > MOUSE_THRESHOLD )
					event->value = DIR_DOWN;
				else if(  sdl_event.motion.yrel < -MOUSE_THRESHOLD )
					event->value = DIR_UP;
				else
					return -1;
				return 1;
		}
		SDL_Delay( 10 );
		timeout -= 10;
	}
	return 0;
}

int event_process( int e ) {
	switch( focus_has() ) {
		case FOCUS_MENU:
			menu_event( e );
			break;
		case FOCUS_SUBMENU:
			submenu_event( e );
			break;
		case FOCUS_GAMESEL:
			game_sel_event( e );
			break;
		default:
			fprintf( stderr, "Error: Unknown focus %d\n", focus_has() );
			return -1;
			break;
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

