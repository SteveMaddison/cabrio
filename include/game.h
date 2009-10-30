#ifndef _GAME_H_
#define _GAME_H_ 1

#include "ogl.h"
#include "genre.h"
#include "platform.h"
#include "config.h"

#define FILTER_ALL		0
#define FILTER_NAME		1
#define FILTER_GENRE	2
#define FILTER_PLATFORM	3

struct game_category {
	struct game_category *next;
	char *name;
	char *value;
};

struct game {
	struct game *next;
	struct game *prev;
	struct game *all_next;
	struct game *all_prev;
	struct genre *genre;
	struct platform *platform;
	struct game_category *categories;
	GLuint texture;
	unsigned int image_width;
	unsigned int image_height;
	char *name;
	char *logo_image;
	char *bg_image;
	char *rom_path;
	struct config_param *params;
};

struct game *game_first( void );

int game_list_create( void );
int game_list_filter_genre( struct genre *genre );
int game_list_filter_platform( struct platform *platform );
int game_list_unfilter( void );
void game_list_pause( void );
int game_list_resume( void );
void game_list_free( void );


void game_free( struct game* game );

#endif

