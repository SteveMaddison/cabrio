#include "platform.h"
#include "config.h"
#include <stdlib.h>

struct platform *platform_start;
static int count = 0;
static int has_unknown = 0;

static struct platform platform_unknown = { NULL, NULL, "???" };

int platform_count( void ) {
	return count;
}

void platform_add( struct platform *platform, struct platform *after ) {
	if( after == NULL ) {
		platform->prev = platform;
		platform->next = platform;
	}
	else {
		after->next->prev = platform;
		platform->next = after->next;
		after->next = platform;
		platform->prev = after;
	}
}

void platform_add_unknown( void ) {
	if( has_unknown == 0 ) {
		if( platform_start ) {
			platform_unknown.next = platform_start->prev;
			platform_unknown.prev = platform_start;
			platform_start->prev->next = &platform_unknown;
			platform_start->prev = &platform_unknown;
		}
		else {
			platform_start = &platform_unknown;
			platform_start->next = platform_start;
			platform_start->prev = platform_start;
		}
		count++;
		has_unknown = 1;
	}
}

int platform_init( void ) {
	struct config_platform *c = config_get()->platforms;
	struct platform *platform = NULL;
	struct platform *prev = NULL;

	while( c ) {
		platform = malloc( sizeof(struct platform) );
		if( platform ) {
			platform->name = c->name;
		
			prev = platform_start;
			if( platform_start ) {
				prev = platform_start->prev;
				while( strcasecmp( prev->name, platform->name ) > 0 ) {
					prev = prev->prev;
					if( prev == platform_start->prev ) break;
				}
			}
			platform_add( platform, prev );
			if( !platform_start || strcasecmp( platform->name, platform_start->name ) < 0 ) {
				platform_start = platform;
			}
			count++;
		}
		else {
			fprintf( stderr, "Error: Couldn't allocate memeory for platform object\n" );
			return -1;
		}
		c = c->next;
	}
	return 0;
}

struct platform *platform_first( void ) {
	return platform_start;
}

struct platform *platform_get( const char *name ) {
	struct platform *g = platform_start;
	
	if( name && *name ) {	
		while( g ) {
			if( strcasecmp( g->name, name ) == 0 )
				return g;
			if( g == platform_start->prev ) break;
			g = g->next;
		}
	}
	return &platform_unknown;
}

