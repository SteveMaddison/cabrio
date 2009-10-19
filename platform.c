#include "platform.h"
#include "config.h"
#include <stdlib.h>

struct platform *platform_start;
extern struct config *config;
static int count = 0;

static struct platform platform_unknown = { NULL, NULL, "Unknown" };

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

int platform_init( void ) {
	struct config_platform *c = config->platforms;
	struct platform *platform = NULL;
	struct platform *prev = NULL;

	platform_start = &platform_unknown;
	platform_start->next = platform_start;
	platform_start->prev = platform_start;
		
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

