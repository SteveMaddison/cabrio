#include <stdlib.h>
#include "game.h"
#include "config.h"
#include "category.h"
#include "platform.h"
#include "ogl.h"
#include "sdl_ogl.h"

struct game *game_start = NULL;
struct game *game_filter_start = NULL;

const int IMAGE_MAX_HEIGHT = 280;
const int IMAGE_MAX_WIDTH = 360;

struct game *game_first( void ) {
	return game_filter_start;
}

void game_add( struct game *game, struct game *after ) {
	if( after == NULL ) {
		game->all_prev = game;
		game->all_next = game;
	}
	else {
		after->all_next->all_prev = game;
		game->all_next = after->all_next;
		after->all_next = game;
		game->all_prev = after;
	}
}

void game_free( struct game* game ) {
	if( game ) {
		ogl_free_texture( &game->texture );
		game->all_prev->all_next = game->all_next;
		game->all_next->all_prev = game->all_prev;
		game_start = game->all_next;
		if( game_start->all_next == game_start ) {
			/* last in the list */
			game_start = NULL;
		}
		free( game );
		game = NULL;
	}
}

void game_list_free( void ) {
	while( game_start )
		game_free( game_start );
}

int game_load_texture( struct game *game ) {
	int x, y;
	game->texture = sdl_create_texture( game->logo_image, &x, &y );
	if( game->texture == 0 ) {
		return -1;
	}
	else {
		if( x > y ) {
			/* Landscape */
			if( x > IMAGE_MAX_WIDTH ) {
				y = (int)(float)y/((float)x/IMAGE_MAX_WIDTH);
				x = IMAGE_MAX_WIDTH;
			}
		}
		else {
			/* Portrait (or square) */
			if( y > IMAGE_MAX_HEIGHT ) {
				x = (int)(float)x/((float)y/IMAGE_MAX_HEIGHT);
				y = IMAGE_MAX_HEIGHT;
			}
		}
		game->image_width = x;
		game->image_height = y;
	}
	return 0;
}

void game_list_pause( void ) {
	struct game *g = game_start;
	if( g ) {
		do {
			ogl_free_texture( &g->texture );
			g = g->all_next;
		} while( g != game_start );
	}
}

int game_list_resume( void ) {
	struct game *g = game_start;
	if( g ) {
		do {
			game_load_texture( g );
			g = g->all_next;
		} while( g != game_start );
	}
	return 0;
}

int game_list_create( void ) {
	struct game *game = NULL;
	struct game *prev = NULL;
	struct config_game *config_game = config_get()->games;
	
	while( config_game ) {
		game = malloc(sizeof(struct game));
		if( game == NULL ) {
			return -1;
		}
		else {
			struct config_game_category *config_category = config_game->categories;
			game->name = config_game->name;
			game->logo_image = config_game->logo_image;
			game_load_texture( game );
			game->bg_image = config_game->background_image;
			game->rom_path = config_game->rom_image;
			game->params = config_game->params;
			game->platform = config_game->platform ? platform_get( config_game->platform->name ) : platform_get( NULL );
			game->categories = NULL;
			while( config_category ) {
				struct game_category *gc = malloc( sizeof(struct game_category) );
				if( gc ) {
					gc->name = config_category->category->name;
					gc->value = config_category->value->name;
					gc->next = game->categories;
					game->categories = gc;
				}
				else {
					fprintf( stderr, "Warning: Couldn't allocate category for game '%s'\n", game->name );
				}
				config_category = config_category->next;
			}
			
			/* insert into list (sort by name) */
			prev = game_start;
			if( game_start ) {
				prev = game_start->all_prev;
				while( strcasecmp( prev->name, game->name ) > 0 ) {
					prev = prev->all_prev;
					if( prev == game_start->all_prev ) break;
				}
			}
			game_add( game, prev );
			if( !game_start || strcasecmp( game->name, game_start->name ) < 0 ) {
				game_start = game;
			}
			game->next = game->all_next;
			game->prev = game->all_prev;
		}
		config_game = config_game->next;
	}
	game_filter_start = game_start;

/*
	if( game_start ) {
		struct game *g = game_start;
		while( g ) {
			struct game_category *gc = g->categories;
			printf("Game: %s\n", g->name );
			while( gc ) {
				printf("  '%s' = '%s'\n", gc->name, gc->value );
				gc = gc->next;
			}
			g = g->all_next;
			if( g == game_start ) break;
		}
	}
*/
	return 0;
}

int game_list_filter_category( char *name, char *value ) {
	int count = 0;
	struct game *game = game_start;
	struct game_category *category = NULL;
	
	game_filter_start = NULL;
	if( game && game->categories ) {
		do {
			category = game->categories;
			while( category ) {
				if( strcasecmp( category->value, value ) == 0
				&&  strcasecmp( category->name, name ) == 0 ) {
					if( game_filter_start == NULL ) {
						game_filter_start = game;
						game->next = game;
						game->prev = game;
					}
					else {
						game->prev = game_filter_start->prev;
						game_filter_start->prev->next = game;
						game_filter_start->prev = game;
						game->next = game_filter_start;
					}
					count++;
				}
				category = category->next;
			}
			game = game->all_next;
		} while ( game != game_start );
	}
	return count;
}

int game_list_filter_platform( struct platform *platform ) {
	int count = 0;

	struct game *game = game_start;
	
	game_filter_start = NULL;
	if( game && platform ) {
		do {
			if( game->platform == platform ) {
				if( game_filter_start == NULL ) {
					game_filter_start = game;
					game->next = game;
					game->prev = game;
				}
				else {
					game->prev = game_filter_start->prev;
					game_filter_start->prev->next = game;
					game_filter_start->prev = game;
					game->next = game_filter_start;
				}
				count++;
			}
			game = game->all_next;
		} while ( game != game_start );
	}
	return count;
}

int game_list_unfilter( void ) {
	int count = 0;
	struct game* game = game_start;

	if( game ) {
		do {
			game->next = game->all_next;
			game->prev = game->all_prev;
			game = game->all_next;
			count++;
		} while ( game != game_start );
	}
	game_filter_start = game_start;

	return count;
}

