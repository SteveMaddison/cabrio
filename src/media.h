#ifndef __MEDIA_H__
#define __MEDIA_H__

enum media_t {
	MEDIA_IMAGE,
	MEDIA_VIDEO,
	MEDIA_AUDIO,
	NUM_MEDIA_TYPES
};

enum image_t {
	IMAGE_LOGO,
	IMAGE_SCREENSHOT,
	IMAGE_BACKGROUND,
	IMAGE_PLATFORM,
	NUM_IMAGE_TYPES
};

struct media {
	int type;
	char *subtype;
};

char *image_type_name( int i );
char *media_type_name( int m );


#endif

