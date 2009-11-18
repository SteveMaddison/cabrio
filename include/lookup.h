#ifndef _LOOKUP_H_
#define _LOOKUP_H_ 1

#include "category.h"

int lookup_match( const char *pattern, const char *value );
const char *lookup_category( struct config_category *category, const char *value );

#endif
