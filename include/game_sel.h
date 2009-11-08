#ifndef _GAME_SEL_H_
#define _GAME_SEL_H_ 1

#include <stdio.h>
#include "game.h"
#include "ogl.h"

#define HIDE_TARGET_START		0
#define HIDE_TARGET_SELECTED	1
#define HIDE_TARGET_END			2

struct game_tile {
	struct game_tile *next;
	struct game_tile *prev;
	GLfloat pos[3];
	GLfloat angle[3];
	GLfloat alpha;
	struct game *game;
};

int game_sel_init( int theme );
int game_sel_populate( struct game *game );
void game_tile_add( struct game_tile *game );
struct game_tile *game_tile_first( void );
struct game_tile *game_tile_last( void );

void game_sel_draw( void );
void game_sel_hide( int target );
void game_sel_show( void );
void game_sel_zoom( void );
int game_sel_busy( void );
struct game* game_sel_current( void );
void game_sel_advance( void );
void game_sel_retreat( void );
int game_sel_event( int event );
int game_sel_got_focus( void );
int game_sel_lost_focus( void );
void game_sel_skip_forward( void );
void game_sel_skip_back( void );
void game_sel_free( void );

#endif

