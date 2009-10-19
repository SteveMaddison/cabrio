#ifndef _GENRE_H_
#define _GENRE_H_ 1

#include <stdio.h>
#include <string.h>

struct genre {
	struct genre *next;
	struct genre *prev;
	char *name;
};

int genre_init( void );
int genre_count( void );
struct genre *genre_first( void );
struct genre *genre_get( const char *name );

#endif

