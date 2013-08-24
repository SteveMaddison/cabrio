#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>
#include "game_sel.h"
#include "bg.h"
#include "config.h"
#include "focus.h"
#include "sound.h"
#include "emulator.h"
#include "menu.h"
#include "submenu.h"
#include "snap.h"
#include "media.h"

static const GLfloat IMAGE_SCALE = 0.005;
static const int MAX_STEPS = 25;

static GLfloat x_scale = 1;
static GLfloat y_scale = 1;
static GLfloat tile_scale = 0.005;
static int tile_count = 0;
static int idle_time = 5;
static int steps = 0;

struct game_tile *game_tile_start = NULL;
struct game_tile *game_tile_end = NULL;
struct game_tile *game_tile_current = NULL;
struct game_tile *game_tile_depth_start = NULL;
struct game_tile *game_tile_depth_end = NULL;

static int scroll_direction = 0;
static int step = 0;
static int visible = 0;
static int hide_direction = 0;
static int hide_target = 0;
static int idle_counter = 0;
static int skipping = 0;
static int zoom = 0;

int game_sel_init( void ) {
	int frame_rate = config_get()->iface.frame_rate;
	struct config_game_sel_tile *config_tile = NULL;
	int config_tile_count = 0;
	int next = INT_MAX;
	
	if( frame_rate ) {
		steps = frame_rate/5;
		idle_time = frame_rate/10;
	}
	else {
		steps = MAX_STEPS;
		idle_time = 0;
	}
	
	x_scale = config_get()->iface.theme.game_sel.size_x;
	y_scale = config_get()->iface.theme.game_sel.size_y;
	tile_scale = config_get()->iface.theme.game_sel.tile_size * IMAGE_SCALE;
	
	tile_count = 0;
	
	config_tile = config_get()->iface.theme.game_sel.tiles;
	if( !config_tile ) {
		fprintf( stderr, "Error: Game selector has no tiles\n" );
		return -1;
	}
	
	/* Wonderfully ineffecient sorting, but we only need to do it once... */
	while( config_tile ) {
		if( config_tile->order == next && config_tile_count != 0 ) {
			fprintf( stderr, "Error: Duplicate 'order' paramemter in game selector tile: %d\n", next );
			return -1;
		}
		else if( config_tile->order < next ) {
			next = config_tile->order;
		}
		config_tile_count++;
		config_tile = config_tile->next;
	}
	
	while( tile_count < config_tile_count ) {
		config_tile = config_get()->iface.theme.game_sel.tiles;

		while( config_tile ) {
			if( config_tile->order == next )
				break;
			config_tile = config_tile->next;
		}
		
		if( config_tile && config_tile->order == next ) {
			struct game_tile *tile = malloc(sizeof(struct game_tile));
			if( tile == NULL ) {
				fprintf( stderr, "Error: Couldn't allocate memory for game tile\n" );
				return -1;
			}
			else {
				memset( tile, 0, sizeof(struct game_tile) );
				
				tile->pos[X] = config_tile->pos[X];
				tile->pos[Y] = config_tile->pos[Y];
				tile->pos[Z] = config_tile->pos[Z];

				tile->angle[X] = config_tile->angle[X];
				tile->angle[Y] = config_tile->angle[Y];
				tile->angle[Z] = config_tile->angle[Z];

				tile->alpha = 1.0 - ((GLfloat)config_tile->transparency/100);

				if( config_tile->order == config_get()->iface.theme.game_sel.selected )
					game_tile_current = tile;

				game_tile_add( tile );
				tile_count++;
			}
			config_tile = config_tile->next;
		}
		next++;
	}
	
	if( !game_tile_current ) {
		fprintf( stderr, "Error: No game selector tile marked as selected.\n" );
		return -1;
	}
	
	return 0;
}

void game_sel_load_textures( void ) {
	struct game_tile *t = game_tile_start;
	
	while( t ) {
		game_load_texture( t->game );
		t = t->next;
	}
}

void game_sel_free_textures( void ) {
	struct game_tile *t = game_tile_start;
	
	while( t ) {
		game_free_texture( t->game );
		t = t->next;	
	}
}

void game_sel_pause( void ) {
	game_sel_free_textures();
}

int game_sel_resume( void ) {
	game_sel_load_textures();
	snap_set( game_tile_current->game );
	return 0;
}

int game_sel_populate( struct game *game ) {
	int i;
	struct game_tile *tile;

	if( game == NULL ) {
		return -1;	
	}
	/* Make sure the first game in the list is in the center (i.e. selected) */
	for( i = 0 ; i < tile_count/2 ; i++ ) {
		game = game->prev;
	}

	tile = game_tile_start;
	while( tile ) {
		game_load_texture( game );
		tile->game = game;
		tile = tile->next;
		game = game->next;
	}
	
	return 0;
}

void game_sel_shuffle_forward( int manage_textures ) {
	struct game_tile *t = game_tile_start;
	
	if( t ) {
		if( manage_textures )
			game_load_texture( t->game->prev );
		while( t ) {
			if( t->game )
				t->game = t->game->prev;
			t = t->next;
		}

		if( manage_textures ) {
			/* Check if we can free the last game's texture (we can't
			 * if the game appears more than once in the selector. */
			t = game_tile_start;
			while( t && t->game != game_tile_end->game ) {
				t = t->next;
			}
			if( t == NULL || t == game_tile_end ) {
				game_free_texture( game_tile_end->game->next );
			}
		}
	}
}

void game_sel_shuffle_back( int manage_textures ) {
	struct game_tile *t = game_tile_start;
	
	if( t ) {
		while( t ) {
			if( t->game )
				t->game = t->game->next;
			if( manage_textures && t->next == NULL )
				game_load_texture( t->game );
			t = t->next;	
		}

		if( manage_textures ) {
			/* Check if we can free the last game's texture (we can't
			 * if the game appears more than once in the selector. */
			t = game_tile_end;
			while( t && t->game != game_tile_start->game ) {
				t = t->prev;
			}
			if( t == NULL || t == game_tile_start ) {
				game_free_texture( game_tile_start->game->prev );
			}
		}
	}
}

void game_sel_do_skip( void ) {
	struct game *current_game = game_tile_current->game;
	struct game *pos;

	if( current_game ) {
		game_sel_free_textures();
		if( skipping < 0 ) {
			pos = current_game->prev;
			game_sel_shuffle_forward( 0 );
	
			if( isalpha( current_game->name[0] ) ) {
				while( strncasecmp( current_game->name, pos->name, 1 ) == 0 && pos != current_game ) {
					game_sel_shuffle_forward( 0 );
					pos = pos->prev;
				}
			}
			else {
				while( !isalpha( pos->name[0] ) && pos != current_game ) {
					game_sel_shuffle_forward( 0 );
					pos = pos->prev;
				}
			}	
		}
		else {
			pos = current_game->next;
			game_sel_shuffle_back( 0 );
	
			if( isalpha( current_game->name[0] ) ) {
				while( strncasecmp( current_game->name, pos->name, 1 ) == 0 && pos != current_game ) {
					game_sel_shuffle_back( 0 );
					pos = pos->next;
				}
			}
			else {
				while( !isalpha( pos->name[0] ) && pos != current_game ) {
					game_sel_shuffle_back( 0 );
					pos = pos->next;
				}
			}	
		}
		skipping = 0;
		game_sel_load_textures();
	}
	game_sel_show();
}

void game_sel_skip_forward( void ) {
	if( visible && !game_sel_busy() ) {
		snap_clear();
		sound_play( SOUND_BLIP );
		game_sel_hide( HIDE_TARGET_SELECTED );
		skipping = 1;
	}
}

void game_sel_skip_back( void ) {
	if( visible && !game_sel_busy() ) {
		snap_clear();
		sound_play( SOUND_BLIP );
		game_sel_hide( HIDE_TARGET_SELECTED );
		skipping = -1;
	}
}

void game_sel_retreat( void ) {
	if( visible && !game_sel_busy() ) {
		snap_clear();
		sound_play( SOUND_BLIP );
		if( game_tile_current->prev )
			game_tile_current = game_tile_current->prev;
		scroll_direction = 1;
		step = 1;
	}
}

void game_sel_advance( void ) {
	if( visible && !game_sel_busy() ) {
		snap_clear();
		sound_play( SOUND_BLIP );
		game_sel_shuffle_back( 1 );
		scroll_direction = -1;
		step = steps-1;
	}
}

int game_sel_event( int event ) {
	int o = config_get()->iface.theme.game_sel.orientation;
	switch( event ) {
		case EVENT_UP:
			if( o == CONFIG_LANDSCAPE )
				game_sel_skip_back();
			else
				game_sel_retreat();
			break;
		case EVENT_DOWN:
			if( o == CONFIG_LANDSCAPE )
				game_sel_skip_forward();
			else
				game_sel_advance();
			break;
		case EVENT_LEFT:
			if( o == CONFIG_LANDSCAPE )
				game_sel_retreat();
			else
				game_sel_skip_back();
			break;
		case EVENT_RIGHT:
			if( o == CONFIG_LANDSCAPE )
				game_sel_advance();
			else
				game_sel_skip_forward();
			break;
		case EVENT_SELECT:
			snap_clear();
			emulator_run( game_sel_current() );
			break;
		case EVENT_BACK:
			snap_clear();
			sound_play( SOUND_BACK );
			focus_set( FOCUS_SUBMENU );
			break;
		default:
			break;
	}
	return 0;
}

int game_sel_got_focus( void ) {
	if( game_sel_populate( game_first() ) == 0 ) {
		game_sel_show();
		snap_show();
		idle_counter = 1;
		
		if( config_get()->iface.theme.menu.auto_hide ) {
			menu_hide();
			submenu_hide();
		}
	}
	else {
		sound_play( SOUND_NO );
		focus_set( FOCUS_SUBMENU );
	}
	return 0;
}

int game_sel_lost_focus( void ) {
	game_sel_hide(HIDE_TARGET_START);
	snap_hide();

	if( config_get()->iface.theme.menu.auto_hide ) {
		menu_show();
	}

	return 0;
}

void game_tile_draw( struct game_tile* tile, struct game_tile* dest, int step ) {
	GLfloat xfactor = ogl_xfactor();
	GLfloat alpha = 1.0;
	const struct config_game_sel *config = &config_get()->iface.theme.game_sel;
	
	if( tile && tile->game && tile->game->texture && dest ) {
		GLfloat width = (((GLfloat)tile->game->texture->width * tile_scale)/2) * xfactor;
		GLfloat height = (((GLfloat)tile->game->texture->height * tile_scale)/2) * xfactor;
		
		ogl_load_alterego();
		if( config->orientation == CONFIG_PORTRAIT ) {
			glTranslatef(
				(tile->pos[X] + config->offset1 + (((dest->pos[X]-tile->pos[X])/steps)*step)) * x_scale * xfactor,
				(tile->pos[Y] + config->offset2 + (((dest->pos[Y]-tile->pos[Y])/steps)*step)) * y_scale * xfactor,
				tile->pos[Z] + (((dest->pos[Z]-tile->pos[Z])/steps)*step) -5.0
				);
			glRotatef( tile->angle[X] + (((dest->angle[X]-tile->angle[X])/steps)*step), 1.0, 0.0, 0.0 );
			glRotatef( tile->angle[Y] + (((dest->angle[Y]-tile->angle[Y])/steps)*step), 0.0, 1.0, 0.0 );
			glRotatef( tile->angle[Z] + (((dest->angle[Z]-tile->angle[Z])/steps)*step), 0.0, 0.0, 1.0 );
		}
		else {
			glTranslatef(
				-((tile->pos[Y] + config->offset2 + (((dest->pos[Y]-tile->pos[Y])/steps)*step)) * x_scale * xfactor),
				-((tile->pos[X] + config->offset1 + (((dest->pos[X]-tile->pos[X])/steps)*step)) * y_scale * xfactor),
				tile->pos[Z] + (((dest->pos[Z]-tile->pos[Z])/steps)*step) -5.0
				);
			glRotatef( -(tile->angle[Y] + (((dest->angle[Y]-tile->angle[Y])/steps)*step)), 1.0, 0.0, 0.0 );
			glRotatef( -(tile->angle[X] + (((dest->angle[X]-tile->angle[X])/steps)*step)), 0.0, 1.0, 0.0 );
			glRotatef( -(tile->angle[Z] + (((dest->angle[Z]-tile->angle[Z])/steps)*step)), 0.0, 0.0, 1.0 );
		}
		if( zoom && tile == game_tile_current ) {
			glTranslatef( 0.0, 0.0, (steps-zoom)/5 );
			alpha = (tile->alpha/steps)*(zoom);
			zoom--;
		}
		else if( hide_direction != 0 ) {
			alpha = tile->alpha - (tile->alpha/(GLfloat)steps)*(GLfloat)(step+1);
		}
		else {
			alpha = tile->alpha + ((dest->alpha - tile->alpha)/steps) * step;
		}
		glColor4f( 1.0, 1.0, 1.0, alpha );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		glBindTexture( GL_TEXTURE_2D, tile->game->texture->id );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f( -width,  height, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f( -width, -height, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(  width,	-height, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(  width,  height, 0.0);		
		glEnd();
		glDisable(GL_TEXTURE_2D);
	}
}

struct game* game_sel_current( void ) {
	if( game_tile_current )
		return game_tile_current->game;
	else
		return NULL;
}

int game_sel_busy( void ) {
	/* Return non-zero if we're animating right now */
	return (scroll_direction || hide_direction || zoom);
}

struct game_tile *next_dest( struct game_tile *tile ) {
	if( hide_direction != 0 ) {
		if( hide_target == HIDE_TARGET_END ) {
			return game_tile_end;
		}
		else if( hide_target == HIDE_TARGET_START ) {
			return game_tile_start;		
		}
		else {
			return game_tile_current;
		}
	}
	else {
		if( tile->next )
			return tile->next;
		else
			return tile;
	}
}

void game_sel_draw_step( int step ) {
	struct game_tile *tile = game_tile_depth_start;
	while( tile ) {
		game_tile_draw( tile, next_dest(tile), step );
		tile = tile->depth_next;
	}
}

void game_sel_show( void ) {
	if( !visible && !game_sel_busy() ) {
		hide_direction = 1;
		step = steps-1;
		visible = 1;
	}
}

void game_sel_hide( int target ) {
	if( visible && !game_sel_busy() ) {
		hide_target = target;
		hide_direction = -1;
		step = 1;
		visible = 1;
	}
}

void game_sel_draw( void ) {
	if( idle_counter && !game_sel_busy() ) {
		idle_counter--;
		if( idle_counter == 0 && game_tile_current->game ) {
			bg_set( game_media_get( game_tile_current->game, MEDIA_IMAGE, image_type_name(IMAGE_BACKGROUND) ) );
			snap_set( game_tile_current->game );
		}
	}
	if( visible ) {
		if( scroll_direction != 0 ) {
			step += scroll_direction;
			if( step > steps-1 ) {
				game_sel_shuffle_forward( 1 );
				if( game_tile_current->next )
					game_tile_current = game_tile_current->next;
				step = 0;
			}
			game_sel_draw_step( step );
			if( step == 0 ) {
				scroll_direction = 0;
				idle_counter = idle_time;
			}
		}
		else {
			step += -hide_direction;
			game_sel_draw_step( step );
			if( step > steps-1 || step == 0 ) {
				if( hide_direction < 0 ) {
					visible = 0;
					game_sel_free_textures();
				}
				hide_direction = 0;
				if( skipping != 0 ) {
					game_sel_do_skip();
					idle_counter = idle_time;
				}
			}
		}
	}
}

void game_sel_zoom( void ) {
	zoom = steps;
}

void game_tile_add( struct game_tile *tile ) {
	if( game_tile_start == NULL ) {
		tile->prev = NULL;
		tile->next = NULL;
		game_tile_start = tile;
	}
	else {
		game_tile_end->next = tile;
		tile->prev = game_tile_end;
		tile->next = NULL;
	}
	game_tile_end = tile;
	
	/* Sort by depth */
	if( game_tile_depth_start == NULL ) {
		tile->depth_prev = NULL;
		tile->depth_next = NULL;
		game_tile_depth_start = tile;
		game_tile_depth_end = tile;
	}
	else {
		struct game_tile *before = game_tile_depth_start;
		while( before && tile->pos[Z] > before->pos[Z] ) {
			before = before->depth_next;
		}
		if( before ) {
			/* Insert */
			tile->depth_prev = before->depth_prev;
			tile->depth_next = before;
			if( before->depth_prev )
				before->depth_prev->depth_next = tile;
			else 
				game_tile_depth_start = tile;
			before->depth_prev = tile;
		}
		else {
			/* Append */
			game_tile_depth_end->depth_next = tile;
			tile->depth_prev = game_tile_depth_end;
			tile->depth_next = NULL;
			game_tile_depth_end = tile;
		}
	}
}

struct game_tile *game_tile_first( void ) {
	return game_tile_start;
}

struct game_tile *game_tile_last( void ) {
	return game_tile_end;
}

void game_tile_free( struct game_tile *tile ) {
	if( tile ) {
		if( tile->prev == NULL ) {
			game_tile_start = NULL;
		}
		else {
			tile->prev->next = tile->next;
			if( tile->next != NULL ) {
				tile->next->prev = tile->prev;
			}
		}	
		free( tile );
		tile = NULL;
	}
}

void game_sel_free( void ) {
	while( game_tile_start )
		game_tile_free( game_tile_start );
}

