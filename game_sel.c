#include "game_sel.h"
#include "bg.h"
#include "math.h"

static const int STEPS = 15;
static const int IMAGE_SCALE = 128;
static const int NUM_GAME_TILES = 11;
static const int IDLE_TIME = 5;

struct game_tile *game_tile_start = NULL;
struct game_tile *game_tile_end = NULL;
struct game_tile *game_tile_current = NULL;

static int scroll_direction = 0;
static int step = 0;
static int visible = 0;
static int hide_direction = 0;
static int hide_target = 0;
static int idle_counter = 0;
static int skipping = 0;
static int zoom = 0;

int game_sel_init( int theme ) {
	int i;
	int mid = NUM_GAME_TILES/2;
	
	for( i = 0 ; i < NUM_GAME_TILES ; i++ ) {
		struct game_tile *tile = malloc(sizeof(struct game_tile));
		if( tile == NULL ) {
			return -1;
		}
		else {
			float offset = i<mid ? (float)mid-i : (float)i-mid;
			memset(tile,0,sizeof(struct game_tile));
			if( theme == 0 ) {
				if( i < mid ) {
					tile->pos[0] = -((offset/4)+(0.5*offset))-1;
					tile->pos[1] = 0.0;
					tile->pos[2] = -offset*(offset/5);
					tile->angle[0] = 0.0;
					tile->angle[1] = (GLfloat)95+((offset)*1.0);
					tile->angle[2] = 0.0;
				}
				else if( i > mid ) {
					tile->pos[0] = (offset/4)+(0.5*offset)+1;
					tile->pos[1] = 0.0;
					tile->pos[2] = -offset*(offset/5);
					tile->angle[0] = 0.0;
					tile->angle[1] = -(GLfloat)95-((offset)*1.0);
					tile->angle[2] = 0.0;
				}
				else {
					tile->pos[0] = 0.0;
					tile->pos[1] = 0.0;
					tile->pos[2] = 1.25;
					tile->angle[0] = 0.0;
					tile->angle[1] = 0.0;
					tile->angle[2] = 0.0;
				}
			}
			else if ( theme == 1 ) {
				if( i < mid ) {
					tile->pos[0] = -((offset/3)+(0.5*offset))-1;
					tile->pos[1] = -(offset/3);
					tile->pos[2] = -offset*(offset/5);
					tile->angle[0] = 0.0;
					tile->angle[1] = (GLfloat)95+((offset)*1.0);
					tile->angle[2] = offset*3;
				}
				else if( i > mid ) {
					tile->pos[0] = (offset/3)+(0.5*offset)+1;
					tile->pos[1] = -(offset/3);
					tile->pos[2] = -offset*(offset/5);
					tile->angle[0] = 0.0;
					tile->angle[1] = -(GLfloat)95-((offset)*1.0);
					tile->angle[2] = -offset*3;
				}
				else {
					tile->pos[0] = 0.0;
					tile->pos[1] = 0.0;
					tile->pos[2] = 1.25;
					tile->angle[0] = 0.0;
					tile->angle[1] = 0.0;
					tile->angle[2] = 0.0;
				}
			}			
			else  {
				if( i < mid ) {
					tile->pos[0] = offset;
					tile->pos[1] = -0.5-offset/2;
					tile->pos[2] = -offset/2;
					tile->angle[0] = 0.0;
					tile->angle[1] = 0.0;
					tile->angle[2] = offset*(120/(float)NUM_GAME_TILES);
				}
				else if( i > mid ) {
					tile->pos[0] = offset;
					tile->pos[1] = 0.5+offset/2;
					tile->pos[2] = -offset/2;
					tile->angle[0] = 0.0;
					tile->angle[1] = 0.0;
					tile->angle[2] = -(offset*(120/(float)NUM_GAME_TILES));
				}
				else {
					tile->pos[0] = 0.0;
					tile->pos[1] = 0.0;
					tile->pos[2] = 1.25;
					tile->angle[0] = 0.0;
					tile->angle[1] = 0.0;
					tile->angle[2] = 0.0;
				}
			}
			if( offset == 0) {
				game_tile_current = tile;
			}
			game_tile_add( tile );
		}
	}
	return 0;
}

int game_sel_populate( struct game *game ) {
	int i;
	struct game_tile *tile;

	if( game == NULL ) {
		return -1;	
	}
	/* Make sure the first game in the list is in the center (i.e. selected) */
	for( i = 0 ; i < NUM_GAME_TILES/2 ; i++ ) {
		game = game->prev;
	}

	tile = game_tile_start;
	while( tile ) {
		tile->game = game;
		tile = tile->next;
		game = game->next;
	}
	return 0;
}

void game_tile_draw( struct game_tile* tile, struct game_tile* dest, int step ) {
	GLfloat alpha = 1.0;
	if( tile && tile->game && dest ) {
		glTranslatef(
			tile->pos[X] + (((dest->pos[X]-tile->pos[X])/STEPS)*step),
			tile->pos[Y] + (((dest->pos[Y]-tile->pos[Y])/STEPS)*step),
			tile->pos[Z] + (((dest->pos[Z]-tile->pos[Z])/STEPS)*step) -5.0
			);
		if( zoom && tile == game_tile_current ) {
			glTranslatef( 0.0, 0.0, (STEPS-zoom)/5 );
			alpha = (1.0/STEPS)*(zoom);
			zoom--;
		}
		glRotatef( tile->angle[X] + (((dest->angle[X]-tile->angle[X])/STEPS)*step), 1.0, 0.0, 0.0 );
		glRotatef( tile->angle[Y] + (((dest->angle[Y]-tile->angle[Y])/STEPS)*step), 0.0, 1.0, 0.0 );
		glRotatef( tile->angle[Z] + (((dest->angle[Z]-tile->angle[Z])/STEPS)*step), 0.0, 0.0, 1.0 );
		glColor4f( 1.0, 1.0, 1.0, alpha );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		glBindTexture( GL_TEXTURE_2D, tile->game->texture );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f( -((GLfloat)tile->game->image_width/IMAGE_SCALE)/2,
				((GLfloat)tile->game->image_height/IMAGE_SCALE)/2, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f( -((GLfloat)tile->game->image_width/IMAGE_SCALE)/2,
				-((GLfloat)tile->game->image_height/IMAGE_SCALE)/2, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(  ((GLfloat)tile->game->image_width/IMAGE_SCALE)/2,
				-((GLfloat)tile->game->image_height/IMAGE_SCALE)/2, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(  ((GLfloat)tile->game->image_width/IMAGE_SCALE)/2,
				((GLfloat)tile->game->image_height/IMAGE_SCALE)/2, 0.0);		
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glLoadIdentity();
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
		return tile->next;
	}
}

void game_sel_draw_step( int step ) {
	int i;
	int mid = NUM_GAME_TILES/2;
	struct game_tile *gl,*gr;
		
	gl = game_tile_first();
	gr = game_tile_last();
	if( gl && gr ) {
		for( i = 0 ; i < mid ; i++ ) {
			game_tile_draw( gl, next_dest(gl), step );
			gl = gl->next;
		}
		for( i = 0 ; i < mid ; i++ ) {
			game_tile_draw( gr, next_dest(gr), step );
			gr = gr->prev;
		}
		game_tile_draw( gr, next_dest(gr), step );
	}
}

void game_sel_show( void ) {
	if( !visible && !game_sel_busy() ) {
		hide_direction = 1;
		step = STEPS-1;
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

void game_sel_shuffle_forward( void ) {
	struct game_tile *t = game_tile_start;
	while( t ) {
		if( t->game )
			t->game = t->game->prev;
		t = t->next;	
	}
}

void game_sel_shuffle_back( void ) {
	struct game_tile *t = game_tile_start;
	while( t ) {
		if( t->game )
			t->game = t->game->next;
		t = t->next;	
	}
}

void game_sel_retreat( void ) {
	if( visible && !game_sel_busy() ) {
		game_tile_current = game_tile_current->prev;
		scroll_direction = 1;
		step = 1;
	}
}

void game_sel_advance( void ) {
	if( visible && !game_sel_busy() ) {
		game_sel_shuffle_back();
		scroll_direction = -1;
		step = STEPS-1;
	}
}

void game_sel_do_skip( void ) {
	struct game *current_game = game_tile_current->game;
	struct game *pos;

	if( current_game ) {
		if( skipping < 0 ) {
			pos = current_game->prev;
			game_sel_shuffle_forward();
	
			if( isalpha( current_game->name[0] ) ) {
				while( strncasecmp( current_game->name, pos->name, 1 ) == 0 && pos != current_game ) {
					game_sel_shuffle_forward();
					pos = pos->prev;
				}
			}
			else {
				while( !isalpha( pos->name[0] ) && pos != current_game ) {
					game_sel_shuffle_forward();
					pos = pos->prev;
				}
			}	
		}
		else {
			pos = current_game->next;
			game_sel_shuffle_back();
	
			if( isalpha( current_game->name[0] ) ) {
				while( strncasecmp( current_game->name, pos->name, 1 ) == 0 && pos != current_game ) {
					game_sel_shuffle_back();
					pos = pos->next;
				}
			}
			else {
				while( !isalpha( pos->name[0] ) && pos != current_game ) {
					game_sel_shuffle_back();
					pos = pos->next;
				}
			}	
		}
		skipping = 0;
	}
	game_sel_show();
}

void game_sel_skip_forward( void ) {
	if( visible && !game_sel_busy() ) {
		game_sel_hide( HIDE_TARGET_SELECTED );
		skipping = 1;
	}
}

void game_sel_skip_back( void ) {
	if( visible && !game_sel_busy() ) {
		game_sel_hide( HIDE_TARGET_SELECTED );
		skipping = -1;
	}
}

void game_sel_draw( void ) {
	if( idle_counter && !game_sel_busy() ) {
		idle_counter--;
		if( idle_counter == 0 && game_tile_current->game )
			bg_set( game_tile_current->game->bg_image );
	}
	if( visible ) {
		if( scroll_direction != 0 ) {
			step += scroll_direction;
			if( step > STEPS-1 ) {
				game_sel_shuffle_forward();
				game_tile_current = game_tile_current->next;
				step = 0;
			}
			game_sel_draw_step( step );
			if( step == 0 ) {
				scroll_direction = 0;
				idle_counter = IDLE_TIME;
			}
		}
		else {
			step += -hide_direction;
			game_sel_draw_step( step );
			if( step > STEPS-1 || step == 0 ) {
				if( hide_direction < 0 ) {
					visible = 0;
				}
				hide_direction = 0;
				if( skipping != 0 ) {
					game_sel_do_skip();
					idle_counter = IDLE_TIME;
				}
			}
		}
	}
}

void game_sel_zoom( void ) {
	zoom = STEPS;
}

void game_tile_add( struct game_tile *game ) {
	if( game_tile_start == NULL ) {
		game->prev = NULL;
		game->next = NULL;
		game_tile_start = game;
	}
	else {
		game_tile_end->next = game;
		game->prev = game_tile_end;
		game->next = NULL;
	}
	game_tile_end = game;
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

