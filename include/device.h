#ifndef _DEVICE_H_
#define _DEVICE_H_ 1

enum device_t {
	DEV_KEYBOARD,
	DEV_JOYSTICK,
	DEV_MOUSE,
	NUM_DEVICES
};

enum control_t {
	CTRL_BUTTON,
	CTRL_AXIS,
	CTRL_HAT,
	CTRL_BALL,
	NUM_CTRLS
};

enum direction_t {
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT,
	NUM_DIRS
};

#endif

