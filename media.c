#include <stdio.h>
#include "media.h"

static const char *media_types[] = {
	"logo", "screenshot", "background"
};

const char *media_type( int i ) {
	if( i >= 0 && i < NUM_MEDIA_TYPES )
		return media_types[i];
	return NULL;
}
