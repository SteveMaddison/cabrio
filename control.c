#include <stdio.h>
#include <string.h>
#include "control.h"
#include "sdl.h"

static const char *device_str[] = { "unknown", "keyboard", "joystick", "mouse" };
static const char *ctrl_str[] = { "unknown", "button", "axis", "hat", "ball" };
static const char *dir_str[] = { "unknown", "up", "down", "left", "right" };

const char *device_name( int device ) {
	if( device < 0 || device > NUM_DEVS )
		return NULL;
	else
		return device_str[device];
}

int device_id( char *name ) {
	int i;
	if( name == NULL ) {
		fprintf( stderr, "Warning: Null device name\n" );
		return DEV_UNKNOWN;	
	}
	for( i = 0 ; i < NUM_DEVS ; i++ ) {
		if( strcasecmp( name, device_str[i] ) == 0 ) return i;
	}

	fprintf( stderr, "Warning: Unknown device name '%s'\n", name );
	return DEV_UNKNOWN;
}

const char *control_name( int control ) {
	if( control < 0 || control > NUM_CTRLS )
		return NULL;
	else
		return ctrl_str[control];
}

int control_id( char *name ) {
	int i;
	if( name == NULL ) {
		fprintf( stderr, "Warning: Null control name\n" );
		return CTRL_UNKNOWN;	
	}
	for( i = 0 ; i < NUM_CTRLS ; i++ ) {
		if( strcasecmp( name, ctrl_str[i] ) == 0 ) return i;
	}

	fprintf( stderr, "Warning: Unknown control name '%s'\n", name );
	return CTRL_UNKNOWN;
}

const char *direction_name( int dir ) {
	if( dir < 0 || dir > NUM_DIRS )
		return NULL;
	else
		return dir_str[dir];
}

int direction_id( char *name ) {
	int i;
	if( name == NULL ) {
		fprintf( stderr, "Warning: Null direction name\n" );
		return DIR_UNKNOWN;
	}
	for( i = 0 ; i < NUM_DIRS ; i++ ) {
		if( strcasecmp( name, dir_str[i] ) == 0 ) return i;
	}

	fprintf( stderr, "Warning: Unknown direction name '%s'\n", name );
	return DIR_UNKNOWN;
}

int axis_value( int direction ) {
	if( direction == DIR_LEFT || direction == DIR_UP )
		return -1;
	if( direction == DIR_RIGHT || direction == DIR_DOWN )
		return 1;
	fprintf( stderr, "Warning: Bogus axis direction %d\n", direction );
	return 0;
}

int hat_value( int direction ) {
	switch( direction ) {
		case DIR_UP:
			return SDL_HAT_UP;
			break;
		case DIR_DOWN:
			return SDL_HAT_DOWN;
			break;
		case DIR_LEFT:
			return SDL_HAT_LEFT;
			break;
		case DIR_RIGHT:
			return SDL_HAT_RIGHT;
			break;
		default:
			fprintf( stderr, "Warning: Bogus hat direction %d\n", direction );
			break;
	}
	return 0;
}

