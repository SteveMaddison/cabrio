#ifndef __IMAGE_H__
#define __IMAGE_H__

enum image_t {
	IMAGE_LOGO,
	IMAGE_SCREENSHOT,
	IMAGE_BACKGROUND,
	NUM_IMAGE_TYPES
};

const char *image_type( int i );

#endif

