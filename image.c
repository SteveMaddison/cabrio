#include <stdio.h>
#include "image.h"

static const char *image_types[] = {
	"logo", "screenshot", "background"
};

const char *image_type( int i ) {
	if( i >= 0 && i < NUM_IMAGE_TYPES )
		return image_types[i];
	return NULL;
}

