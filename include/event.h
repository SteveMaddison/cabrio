#ifndef _EVENT_H_
#define _EVENT_H_ 1

#include "config.h"

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
void event_free( void );
void event_pause( void );
int event_resume( void );

int event_get( void );
int event_probe( int timeout, struct config_control *control );
int event_id( char *name );
const char *event_name( int event );

#endif

