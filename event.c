#include "control.h"
#include "config.h"
#include "event.h"
#include "menu.h"
#include "submenu.h"
#include "game_sel.h"
#include "focus.h"
#include "sdl_wrapper.h"
#include <SDL2/SDL.h>

#define MAX_JOYSTICKS 8

static const char *event_str[] = {
	"none",
	"up", "down", "left", "right",
	"select", "back", "quit", "list1", "list2", "list3", "list4", "list5"
};

static const int AXIS_THRESHOLD = 8000;
static const int BALL_THRESHOLD = 10;
static const int MOUSE_THRESHOLD = 10;
static int num_joysticks = 0;
static SDL_Joystick *joysticks[MAX_JOYSTICKS];
static struct event events[NUM_EVENTS];

int event_init( void ) {
	const struct config* config = config_get();
	int i;
	SDL_SetHint("SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS", "1");
	
	num_joysticks = SDL_NumJoysticks();
	if( num_joysticks > MAX_JOYSTICKS )
		num_joysticks = MAX_JOYSTICKS;

	for( i = 0 ; i < num_joysticks ; i++ ) {
		joysticks[i] = SDL_JoystickOpen( i );
	}

	for( i = 1 ; i < NUM_EVENTS ; i++ ) {
		events[i].device_type = config->iface.controls[i].device_type;
		events[i].device_id = config->iface.controls[i].device_id;
		events[i].control_type = config->iface.controls[i].control_type;
		events[i].control_id = config->iface.controls[i].control_id;
		events[i].value = config->iface.controls[i].value;
		// debug //
/*		printf("%s: %s%d %s%d = %d\n",
			event_name(i),
			device_name(events[i].device_type),
			events[i].device_id,
			control_name(events[i].control_type),
			events[i].control_id,
			events[i].value
		); 
*/
	}
	
	return 0;
}

void event_free( void ) {
	int i;
	
	for( i = 0 ; i < num_joysticks ; i++ ) {
		if ( SDL_JoystickGetAttached( joysticks[i] ) ) {
			SDL_JoystickClose( joysticks[i] );
			joysticks[i] = NULL;
		}
	}
}

void event_pause( void ) {
	event_free();
}

int event_resume( void ) {
	return event_init();
}

int event_set( int id, struct event *event_s ) {
	if( id < 1 || id >= NUM_EVENTS ) {
		fprintf( stderr, "Warning: Can't set non-existent event %d\n", id );
		return -1;
	}
	memcpy( &events[id], event_s, sizeof(struct event) );
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

int event_filter( void* data, SDL_Event* sdl_event ) {
	if( sdl_event->type == SDL_TEXTINPUT )
		return 0;
	return 1;
}

void event_set_filter( void ) {
	SDL_SetEventFilter( event_filter, NULL );
}

int event_poll( int event_num ) {
	SDL_Event sdl_event;
	int i;
	while( SDL_PollEvent( &sdl_event ) ) {
		event_num = EVENT_NONE;
		if( sdl_event.type == SDL_QUIT ) {
			event_num = EVENT_QUIT;
			return event_num;
		}
		for( i = 1 ; i < NUM_EVENTS ; i++ ) {
			switch( sdl_event.type ) {
				case SDL_KEYDOWN:
					if( events[i].device_type == DEV_KEYBOARD
					&&  events[i].value == sdl_event.key.keysym.sym )
						event_num = i;

					break;

				case SDL_JOYAXISMOTION:
					if( events[i].device_type == DEV_JOYSTICK
					&&  events[i].control_type == CTRL_AXIS
					&&  events[i].device_id == sdl_event.jaxis.which
					&&  events[i].control_id == sdl_event.jaxis.axis ) {
						if( events[i].value < 0 && sdl_event.jaxis.value < -AXIS_THRESHOLD )
							event_num = i;

						else if ( events[i].value > 0 && sdl_event.jaxis.value > AXIS_THRESHOLD )
							event_num = i;
						}
					break;

				case SDL_JOYBUTTONDOWN:
					if( events[i].device_type == DEV_JOYSTICK
					&&  events[i].control_type == CTRL_BUTTON
					&&  events[i].device_id == sdl_event.jbutton.which
					&&  events[i].value == sdl_event.jbutton.button )
						event_num = i;

					break;

				case SDL_JOYHATMOTION:
					if( events[i].device_type == DEV_JOYSTICK
					&&  events[i].control_type == CTRL_HAT
					&&  events[i].device_id == sdl_event.jhat.which
					&&  events[i].control_id == sdl_event.jhat.hat
					&&  events[i].value == sdl_hat_dir_value( sdl_event.jhat.value ) )
						event_num = i;

					break;

				case SDL_JOYBALLMOTION:
					if( events[i].device_type == DEV_JOYSTICK
					&&  events[i].control_type == CTRL_BALL
					&&  events[i].device_id == sdl_event.jball.which
					&&  events[i].control_id == sdl_event.jball.ball ) {
						if( events[i].value == DIR_DOWN && sdl_event.jball.yrel > BALL_THRESHOLD )
							event_num = i;

						else if( events[i].value == DIR_UP && sdl_event.jball.yrel < -BALL_THRESHOLD )
							event_num = i;

						else if( events[i].value == DIR_LEFT && sdl_event.jball.xrel < -BALL_THRESHOLD )
							event_num = i;

						else if( events[i].value == DIR_RIGHT && sdl_event.jball.xrel > BALL_THRESHOLD )
							event_num = i;
						}
					break;

				case SDL_MOUSEBUTTONDOWN:
					if( events[i].device_type == DEV_MOUSE
					&&  events[i].control_type == CTRL_BUTTON
					&&  events[i].device_id == sdl_event.button.which
					&&  events[i].value == sdl_event.button.button )
						event_num = i;

					break;

				case SDL_MOUSEMOTION:
					if( events[i].device_type == DEV_MOUSE
					&&  events[i].control_type == CTRL_AXIS
					&&  events[i].device_id == sdl_event.motion.which ) {
						if( events[i].value == DIR_DOWN && sdl_event.motion.yrel > MOUSE_THRESHOLD )
							event_num = i;

						else if( events[i].value == DIR_UP && sdl_event.motion.yrel < -MOUSE_THRESHOLD )
							event_num = i;

						else if( events[i].value == DIR_LEFT && sdl_event.motion.xrel < -MOUSE_THRESHOLD )
							event_num = i;

						else if( events[i].value == DIR_RIGHT && sdl_event.motion.xrel > MOUSE_THRESHOLD )
							event_num = i;
					}
			}
		}
	}
	// Returns the last event happened if there is no new event in the queue
	return event_num;
}

int event_probe( int timeout, struct event *event_p ) {
	SDL_Event sdl_event;
	
	event_p->device_type = DEV_UNKNOWN;
	event_p->device_id = 0;
	event_p->control_type = CTRL_UNKNOWN;
	event_p->control_id = 0;
	
	while( timeout > 0 ) {
		SDL_PollEvent( &sdl_event );
		switch( sdl_event.type ) {
			case SDL_KEYDOWN:
				event_p->device_type = DEV_KEYBOARD;
				//event->device_id = sdl_event.key.which;
				event_p->value = sdl_event.key.keysym.sym;
				return 1;
			case SDL_JOYAXISMOTION:
				event_p->device_type = DEV_JOYSTICK;
				event_p->device_id = sdl_event.jaxis.which;
				event_p->control_type = CTRL_AXIS;
				event_p->control_id = sdl_event.jaxis.axis;
				if( sdl_event.jaxis.value > AXIS_THRESHOLD )
					event_p->value = 1;
				else if( sdl_event.jaxis.value < -AXIS_THRESHOLD )
					event_p->value = -1;
				else
					return -1;
				return 1;
			case SDL_JOYBUTTONDOWN:
				event_p->device_type = DEV_JOYSTICK;
				event_p->device_id = sdl_event.jbutton.which;
				event_p->control_type = CTRL_BUTTON;
				event_p->value = sdl_event.jbutton.button;
				return 1;
			case SDL_JOYHATMOTION:
				event_p->device_type = DEV_JOYSTICK;
				event_p->device_id = sdl_event.jhat.which;
				event_p->control_type = CTRL_HAT;
				event_p->control_id = sdl_event.jhat.hat;
				event_p->value = sdl_hat_dir_value( sdl_event.jhat.value );
				return 1;
			case SDL_JOYBALLMOTION:
				event_p->device_type = DEV_JOYSTICK;
				event_p->device_id = sdl_event.jball.which;
				event_p->control_type = CTRL_BALL;
				event_p->control_id = sdl_event.jball.ball;
				if( sdl_event.jball.xrel > BALL_THRESHOLD )
					event_p->value = DIR_LEFT;
				else if(  sdl_event.jball.xrel < -BALL_THRESHOLD )
					event_p->value = DIR_RIGHT;
				else if( sdl_event.jball.yrel > BALL_THRESHOLD )
					event_p->value = DIR_DOWN;
				else if(  sdl_event.jball.yrel < -BALL_THRESHOLD )
					event_p->value = DIR_UP;
				else
					return -1;
				return 1;
			case SDL_MOUSEBUTTONDOWN:
				event_p->device_type = DEV_MOUSE;
				event_p->device_id = sdl_event.button.which;
				event_p->control_type = CTRL_BUTTON;
				event_p->value = sdl_event.button.button;
				return 1;
			case SDL_MOUSEMOTION:
				event_p->device_type = DEV_MOUSE;
				event_p->device_id = sdl_event.motion.which;
				event_p->control_type = CTRL_AXIS;
				if( sdl_event.motion.xrel > MOUSE_THRESHOLD )
					event_p->value = DIR_LEFT;
				else if(  sdl_event.motion.xrel < -MOUSE_THRESHOLD )
					event_p->value = DIR_RIGHT;
				else if( sdl_event.motion.yrel > MOUSE_THRESHOLD )
					event_p->value = DIR_DOWN;
				else if(  sdl_event.motion.yrel < -MOUSE_THRESHOLD )
					event_p->value = DIR_UP;
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

const char *event_name( int event_n ) {
	if( event_n < 0 || event_n > NUM_EVENTS )
		return NULL;
	else
		return event_str[event_n];
}
