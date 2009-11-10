#include <stdlib.h>
#include "game.h"
#include "config.h"
#include "category.h"
#include "platform.h"
#include "ogl.h"
#include "sdl_ogl.h"
#include "font.h"

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
		ogl_free_texture( game->texture );
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
	if( game->texture == NULL ) {
		printf("  load: %s\n", game->name );
		if( game && game->logo_image && game->logo_image[0] ) {
			game->texture = sdl_create_texture( game->logo_image );
		}
		if( game->texture == NULL ) {
			if( game && game->name && *game->name ) {
				game->texture = font_create_texture( game->name );
			}
			else {
				game->texture = font_create_texture( "<No Name>" );
			}
		}
		
		if( game->texture == NULL ) {
			return -1;
		}
		else {
			if( game->texture->width > game->texture->height ) {
				/* Landscape */
				if( game->texture->width > IMAGE_MAX_WIDTH ) {
					game->texture->height = (int)(float)game->texture->height/((float)game->texture->width/IMAGE_MAX_WIDTH);
					game->texture->width = IMAGE_MAX_WIDTH;
				}
			}
			else {
				/* Portrait (or square) */
				if( game->texture->height > IMAGE_MAX_HEIGHT ) {
					game->texture->width = (int)(float)game->texture->width/((float)game->texture->height/IMAGE_MAX_HEIGHT);
					game->texture->height = IMAGE_MAX_HEIGHT;
				}
			}
		}
	}
	return 0;
}

void game_free_texture( struct game *game ) {
	if( game->texture ) {
		printf("  free: %s\n", game->name );
		ogl_free_texture( game->texture );
		game->texture = NULL;
	}
}

void game_list_pause( void ) {
	struct game *g = game_start;
	if( g ) {
		do {
			ogl_free_texture( g->texture );
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

int game_add_category( struct game *game, char *name, char *value ) {
	struct game_category *gc = malloc( sizeof(struct game_category) );
	if( gc ) {
		gc->name = name;
		gc->value = value;
		gc->next = game->categories;
		game->categories = gc;
	}
	else {
		fprintf( stderr, "Warning: Couldn't allocate category for game '%s'\n", game->name );
		return -1;
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
			struct config_game_category *config_game_category = config_game->categories;
			struct category *category = category_first();
			game->name = config_game->name;
			game->logo_image = config_game->logo_image;
			game->texture = NULL;
			game->bg_image = config_game->background_image;
			game->screen_shot = config_game->screen_shot;
			game->rom_path = config_game->rom_image;
			game->params = config_game->params;
			if( config_game->platform ) {
				game->platform = platform_get( config_game->platform->name );
			}
			else {
				platform_add_unknown();
				game->platform = platform_get( NULL );
			}
			
			/* Add game categories. */
			game->categories = NULL;
			while( config_game_category ) {
				game_add_category( game, config_game_category->category->name, config_game_category->value->name );
				config_game_category = config_game_category->next;
			}
			
			/* Fill in "unknown" values for categories undefined for this game. */
			if( category) {
				do {
					struct game_category *gc = game->categories;
					while( gc ) {
						if( gc->name == category->name ) {
							break;
						}
						gc = gc->next;
					}
					if( gc == NULL ) {
						/* Category undefine for this game */
						category_value_add_unknown( category );
						game_add_category( game, category->name, NULL );
					}
					category = category->next;
				} while( category != category_first() );
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
				if( strcasecmp( category->name, name ) == 0 ) {
					if(	(category->value == NULL && value == NULL)
					||  (category->value && value && strcasecmp( category->value, value ) == 0) ) {
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

