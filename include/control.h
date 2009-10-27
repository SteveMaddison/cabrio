#ifndef _DEVICE_H_
#define _DEVICE_H_ 1

enum device_t {
	DEV_UNKNOWN,
	DEV_KEYBOARD,
	DEV_JOYSTICK,
	DEV_MOUSE,
	NUM_DEVS
};

enum control_t {
	CTRL_UNKNOWN,
	CTRL_BUTTON,
	CTRL_AXIS,
	CTRL_HAT,
	CTRL_BALL,
	NUM_CTRLS
};

enum direction_t {
	DIR_UNKNOWN,
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
	NUM_DIRS
};

const char *device_name( int device );
int device_id( char *name );

const char *control_name( int control );
int control_id( char *name );

const char *direction_name( int dir );
int direction_id( char *name );

int axis_dir_value( char *name );
const char *axis_dir_name( int axis_dir );

#endif

