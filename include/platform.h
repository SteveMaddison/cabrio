#ifndef _PLATFORM_H_
#define _PLATFORM_H_ 1

#include <stdio.h>
#include <string.h>
#include "config.h"
#include "ogl.h"

struct platform {
	struct platform *next;
	struct platform *prev;
	char *name;
	char image_file[CONFIG_FILE_NAME_LENGTH];
	struct texture *texture;
};

int platform_init( void );
void platform_free( void );
void platform_pause( void );
int platform_resume( void );
int platform_count( void );
struct platform *platform_first( void );
struct platform *platform_get( const char *name );
void platform_add_unknown( void );

#endif

