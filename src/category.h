#ifndef _CATEGORY_H_
#define _CATEGORY_H_ 1

#include <stdio.h>
#include <string.h>
#include "config.h"

struct category_value {
	struct category_value *next;
	struct category_value *prev;
	char *name;
};

struct category {
	struct category *next;
	struct category *prev;
	struct category_value *values;
	int id;
	char *name;
	int value_count;
	int has_unknowns;
};

int category_init( void );
int category_count( void );
struct category *category_first( void );
struct category *category_get( const char *name );
struct category_value *category_get_value( struct category *category, const char *name );
int category_value_add_unknown( struct category *category );

#endif

