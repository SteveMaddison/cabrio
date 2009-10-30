#include "category.h"
#include "config.h"
#include <stdlib.h>

struct category *category_start;
static int count = 0;

int category_count( void ) {
	return count;
}

void category_add( struct category *category, struct category *after ) {
	if( after == NULL ) {
		category->prev = category;
		category->next = category;
	}
	else {
		after->next->prev = category;
		category->next = after->next;
		after->next = category;
		category->prev = after;
	}
	count++;
}

struct category_value *category_value_add( struct category *category, char *name ) {
	struct category_value *after = category->values;
	struct category_value *category_value = malloc( sizeof(struct category_value) );
	struct category_value *prev = NULL;

	if( category_value ) {
		category_value->name = name;
	
		if( after ) {
			prev = after->prev;
			while( strcasecmp( prev->name, category_value->name ) > 0 ) {
				prev = prev->prev;
				if( prev == category->values->prev ) break;
			}
		}

		if( after == NULL ) {
			category_value->prev = category_value;
			category_value->next = category_value;
		}
		else {
			after->next->prev = category_value;
			category_value->next = after->next;
			after->next = category_value;
			category_value->prev = after;
		}
		category->value_count++;
	}
	else {
		fprintf( stderr, "Warning: Couldn't allocate new category value\n" );
	}
	
	return category_value;
}

int category_init( void ) {
	struct config_category *c = config_get()->categories;
	struct category *category = NULL;
	struct category *prev = NULL;
	
	while( c ) {
		category = malloc( sizeof(struct category) );
		if( category ) {
			category->name = c->name;
		
			prev = category_start;
			if( category_start ) {
				prev = category_start->prev;
				while( strcasecmp( prev->name, category->name ) > 0 ) {
					prev = prev->prev;
					if( prev == category_start->prev ) break;
				}
			}
			category_add( category, prev );
			if( !category_start || strcasecmp( category->name, category_start->name ) < 0 ) {
				category_start = category;
			}
			
			if( c->values ) {
				struct config_category_value *value = c->values;
				category->values = NULL;
				while( value ) {
					struct category_value *new = category_value_add( category, value->name );
					if( !category->values || strcasecmp( new->name, category->values->name ) < 0 ) {
						category->values = new;
					}
					value = value->next;
				}
			}
		}
		else {
			fprintf( stderr, "Error: Couldn't allocate memeory for category object\n" );
			return -1;
		}
		c = c->next;
	}
	
/*
	category = category_start;
	while( category ) {
		struct category_value *value = category->values;
		while( value ) {
			printf("%s: %s\n", category->name, value->name );
			value = value->next;
			if( value == category->values ) break;
		}
		category = category->next;
		if( category == category_start ) break;
	}
*/

	return 0;
}

struct category *category_first( void ) {
	return category_start;
}

struct category *category_get( const char *name ) {
	struct category *g = category_start;
	
	if( name && *name ) {
		while( g ) {
			if( strcasecmp( g->name, name ) == 0 )
				return g;
			if( g == category_start->prev ) break;
			g = g->next;
		}
	}
	return NULL;
}

