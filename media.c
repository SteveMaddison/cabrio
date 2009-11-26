#include <stdio.h>
#include "media.h"

static const char *media_types[] = {
	"image", "video", "audio"
};

static const char *image_types[] = {
	"logo", "screenshot", "background", "platform"
};

char *media_type_name( int m ) {
	if( m >= 0 && m < NUM_IMAGE_TYPES )
		return (char*)media_types[m];
	return NULL;
}

char *image_type_name( int i ) {
	if( i >= 0 && i < NUM_IMAGE_TYPES )
		return (char*)image_types[i];
	return NULL;
}

