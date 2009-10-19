#ifndef _PLATFORM_H_
#define _PLATFORM_H_ 1

#include <stdio.h>
#include <string.h>

struct platform {
	struct platform *next;
	struct platform *prev;
	char *name;
};

int platform_init( void );
int platform_count( void );
struct platform *platform_first( void );
struct platform *platform_get( const char *name );

#endif

