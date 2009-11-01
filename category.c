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

int category_value_add_unknown( struct category *category ) {
	if( category->has_unknowns ) {
		return 1;
	}
	else {
		struct category_value *value = malloc( sizeof(struct category_value) );
		if( value ) {
			value->name = NULL;
			
			if( category->values == NULL ) {
				value->prev = value;
				value->next = value;
				category->values = value;
			}
			else {
				value->next = category->values;
				value->prev = category->values->prev;
				category->values->prev->next = value;
				category->values->prev = value;
			}
			category->value_count++;

			category->has_unknowns = 1;
			return 0;
		}
		else {
			fprintf( stderr, "Warning: Couldn't allocate new category value\n" );
			return -1;
		}
	}
}

int category_value_add( struct category *category, char *name ) {
	struct category_value *after = category->values;
	struct category_value *category_value = malloc( sizeof(struct category_value) );

	if( category_value ) {
		category_value->name = name;
	
		if( after ) {
			after = after->prev;
			while( strcasecmp( after->name, category_value->name ) > 0 ) {
				after = after->prev;
				if( after == category->values->prev ) break;
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

		if( !category->values || strcasecmp( category_value->name, category->values->name ) < 0 ) {
			category->values = category_value;
		}
	}
	else {
		fprintf( stderr, "Warning: Couldn't allocate new category value\n" );
		return -1;
	}
	
	return 0;
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
					category_value_add( category, value->name );
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

