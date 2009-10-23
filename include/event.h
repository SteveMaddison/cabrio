#ifndef _EVENT_H_
#define _EVENT_H_ 1

enum event_t {
	EVENT_NONE,
	EVENT_UP,
	EVENT_DOWN,
	EVENT_LEFT,
	EVENT_RIGHT,
	EVENT_SELECT,
	EVENT_BACK,
	EVENT_QUIT,
	NUM_EVENTS
};

int event_init( void );
int event_get( void );

#endif

