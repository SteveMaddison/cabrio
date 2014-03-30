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
	struct game_tile *depth_next;
	struct game_tile *depth_prev;
	GLfloat pos[3];
	GLfloat angle[3];
	GLfloat alpha;
	struct game *game;
};

int game_sel_init( void );
int game_sel_populate( struct game *game );
void game_tile_add( struct game_tile *game );
struct game_tile *game_tile_first( void );
struct game_tile *game_tile_last( void );

void game_sel_draw( void );
void game_sel_hide( int target );
void game_sel_show( void );
int game_sel_busy( void );
struct game* game_sel_current( void );
int game_sel_event( int event );
int game_sel_got_focus( void );
int game_sel_lost_focus( void );
void game_sel_free( void );
void game_sel_pause( void );
int game_sel_resume( void );

#endif
