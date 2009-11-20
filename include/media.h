#ifndef __MEDIA_H__
#define __MEDIA_H__

enum MEDIA_t {
	MEDIA_LOGO,
	MEDIA_SCREENSHOT,
	MEDIA_BACKGROUND,
	NUM_MEDIA_TYPES
};

const char *media_type( int i );

#endif
