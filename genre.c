#include "genre.h"
#include "config.h"
#include <stdlib.h>

struct genre *genre_start;
static int count = 0;

static struct genre genre_unknown = { NULL, NULL, "Unknown" };

int genre_count( void ) {
	return count;
}

void genre_add( struct genre *genre, struct genre *after ) {
	if( after == NULL ) {
		genre->prev = genre;
		genre->next = genre;
	}
	else {
		after->next->prev = genre;
		genre->next = after->next;
		after->next = genre;
		genre->prev = after;
	}
	count++;
}

int genre_init( void ) {
	struct config_genre *c = config_get()->genres;
	struct genre *genre = NULL;
	struct genre *prev = NULL;
	
	genre_start = &genre_unknown;
	genre_start->next = genre_start;
	genre_start->prev = genre_start;
	
	while( c ) {
		genre = malloc( sizeof(struct genre) );
		if( genre ) {
			genre->name = c->name;
		
			prev = genre_start;
			if( genre_start ) {
				prev = genre_start->prev;
				while( strcasecmp( prev->name, genre->name ) > 0 ) {
					prev = prev->prev;
					if( prev == genre_start->prev ) break;
				}
			}
			genre_add( genre, prev );
			if( !genre_start || strcasecmp( genre->name, genre_start->name ) < 0 ) {
				genre_start = genre;
			}
		}
		else {
			fprintf( stderr, "Error: Couldn't allocate memeory for genre object\n" );
			return -1;
		}
		c = c->next;
	}
	return 0;
}

struct genre *genre_first( void ) {
	return genre_start;
}

struct genre *genre_get( const char *name ) {
	struct genre *g = genre_start;
	
	if( name && *name ) {
		while( g ) {
			if( strcasecmp( g->name, name ) == 0 )
				return g;
			if( g == genre_start->prev ) break;
			g = g->next;
		}
	}
	return &genre_unknown;
}

