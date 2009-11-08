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

struct event {
	int device_type;
	int device_id;
	int control_type;
	int control_id;
	int value;
};

int event_init( void );
void event_free( void );
void event_pause( void );
int event_resume( void );

int event_set( int id, struct event *event );
struct event *event_get( int id );

void event_flush( void );
int event_poll( void );
int event_probe( int timeout, struct event *event );
int event_process( int e );
int event_id( char *name );
const char *event_name( int event );

#endif

