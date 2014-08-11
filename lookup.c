#include <stdio.h>
#include "category.h"
#include "lookup.h"
#include "config.h"

int lookup_match( const char *pattern, const char *value ) {
	if( !(pattern && value) )
		return 0;
	
	if( strcmp( pattern, value ) == 0 ) {
		return 0;
	}
	else {
		char *ppos = (char*)pattern;
		char *vpos = (char*)value;
		
		while( *ppos && *vpos ) {
			if( *ppos == '*' ) {
				if( *vpos == *(ppos+1) || strncasecmp( ppos+1, vpos, 1 ) == 0 )
					ppos++;
				else
					vpos++;
			}
			else if( *vpos == *ppos || strncasecmp( ppos, vpos, 1 ) == 0 ) {
				ppos++;
				vpos++;
			}
			else {
				return -1;	
			}
		}
		if( ppos == pattern + strlen(pattern) - 1 ) {
			return 0;
		}
	}

	return -1;
}

const char *lookup_category( struct config_category *category, const char *value ) {
	char *display_name = NULL;
	struct config_lookup *lookup = category->lookups;
	
	while( lookup ) {
		if( lookup_match( lookup->match, value ) == 0 ) {
			display_name = lookup->value;
			break;
		}
		lookup = lookup->next;
	}
	
	if( display_name )
		return display_name;
	else
		return value;
}



/*

		while( *ppos && *vpos ) {
			printf("p:%c <=> v:%c\n", *ppos, *vpos );
			if( *vpos == *ppos ) {
				vpos++;
				ppos++;
			}
			else {
				if( *ppos == '*' ) {
					vpos++;
					printf("  (v:%c p:%c)\n", *vpos, *(ppos+1) );
					if( *vpos == *(ppos+1) )
						ppos++;
				}
				else
					return -1;
			}
		}

*/
