#include <stdio.h>
#include <string.h>
#include "control.h"

static const char *device_str[]		= { "unknown", "keyboard", "joystick", "mouse" };
static const char *ctrl_str[]		= { "unknown", "button", "axis", "hat", "ball" };
static const char *dir_str[]		= { "unknown", "up", "down", "left", "right" };
static const char *axis_dir_plus	= "plus";
static const char *axis_dir_minus	= "minus";

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

int axis_dir_value( char *name ) {
	int i;

	if( name == NULL ) {
		fprintf( stderr, "Warning: Null axis value\n" );
		return DIR_UNKNOWN;
	}
	if( strcasecmp( name, axis_dir_plus ) == 0 )
		return 1;
	if( strcasecmp( name, axis_dir_minus ) == 0 )
		return -1;
		
	/* direction names are also valid */
	for( i = 0 ; i < NUM_DIRS ; i++ ) {
		if( strcasecmp( name, dir_str[i] ) == 0 ) {
			break;	
		}
	}
	if( i == DIR_LEFT || i == DIR_UP )
		return -1;
	if( i == DIR_RIGHT || i == DIR_DOWN )
		return 1;

	fprintf( stderr, "Warning: Bogus axis direction '%s'\n", name );
	return 0;
}

const char *axis_dir_name( int axis_dir ) {
	if( axis_dir < 0 )
		return axis_dir_minus;
	else if ( axis_dir > 0 )
		return axis_dir_plus;
	
	fprintf( stderr, "Warning: Bogus axis direction '%d'\n", axis_dir );
	return "";
}

