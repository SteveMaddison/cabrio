#include "platform.h"
#include "config.h"
#include "location.h"
#include "media.h"
#include "sdl_ogl.h"
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
			platform_unknown.next = platform_start;
			platform_unknown.prev = platform_start->prev;
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

void platform_free_textures( void ) {
	struct platform *p = platform_start;
	
	if( p ) {
		do {
			if( p->texture ) {
				ogl_free_texture( p->texture );
				p->texture = NULL;
			}
			p = p->next;
		} while( p != platform_start );
	}
}

int platform_load_textures( void ) {
	struct platform *p = platform_start;
	
	if( p ) {
		do {
			if( p->image_file && p->image_file[0] ) {
				p->texture = sdl_create_texture( p->image_file );
			}
			p = p->next;
		} while( p != platform_start );
	}
	
	return 0;
}

int platform_init( void ) {
	struct config_platform *c = config_get()->platforms;
	struct platform *platform = NULL;
	struct platform *prev = NULL;

	while( c ) {
		platform = malloc( sizeof(struct platform) );
		if( platform ) {
			memset( platform, 0, sizeof(struct platform) );
			platform->name = c->name;
			
			location_get_match( image_type_name(IMAGE_PLATFORM), platform->name, platform->image_file );						
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
	
	platform_load_textures();
	return 0;
}

void platform_free( void ) {
	platform_free_textures();
}

void platform_pause( void ) {
	platform_free_textures();
}

int platform_resume( void ) {
	return platform_load_textures();
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

