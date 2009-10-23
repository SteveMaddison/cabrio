#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "sdl.h"
#include "ogl.h"
#include "font.h"
#include "config.h"
#include "genre.h"
#include "platform.h"
#include "bg.h"
#include "menu.h"
#include "submenu.h"
#include "hint.h"
#include "game_sel.h"
#include "sound.h"
#include "event.h"

extern struct config *config;
static int supress_wait = 0;

void pause_all( void ) {
	sound_pause();
	game_list_pause();
	submenu_pause();
	menu_pause();
	hint_pause();
	font_pause();
	bg_pause();
}

void resume_all( void ) {
	bg_resume();
	font_resume();
	hint_resume();
	menu_resume();
	submenu_resume();
	game_list_resume();
	sound_resume();
}

int run( struct game *game ) {
	char *params[CONFIG_MAX_PARAMS];
	struct config_param *param = NULL;
	int count,i = 0;
	
	if( config->emulators && config->emulators->executable ) {
		param = config->emulators->params;
		while( param ) {
			if( param->name && *param->name && count < CONFIG_MAX_PARAMS - 2 )
				params[count++] = param->name;
			if( param->value && *param->value && count < CONFIG_MAX_PARAMS - 2 )
				params[count++] = param->value;
			param = param->next;
		}		
		param = game->params;
		while( param ) {
			if( param->name && *param->name && count < CONFIG_MAX_PARAMS - 2 )
				params[count++] = param->name;
			if( param->value && *param->value && count < CONFIG_MAX_PARAMS - 2 )
				params[count++] = param->value;
			param = param->next;
		}		
	}
	else {
		fprintf(stderr, "Error: No suitable emulator found in configuration\n");
	}
	params[count++] = game->rom_path;
	params[count] = NULL;
	
	printf( "Executing: %s", config->emulators->executable );
	for( i = 0 ; i < count ; i++ ) {
		printf( " %s", params[i] );
	}
	printf( "\n" );

	pid_t pid = fork();
	if (pid == 0) {
		execvp( config->emulators->executable, params );
	}
	else if (pid < 0) {
		fprintf(stderr, "Warning: failed to fork\n");
		return -1;
	}
	wait( NULL );
	return 0;
}

void supress( void ) {
	supress_wait = 15;
}

int main( int argc, char *arvg[] ) {
	int quit = 0;
	int menu_level = 0;
	int event;
	struct game *to_run = NULL;

	if( config_init( NULL ) != 0 )
		return -1;

	if( genre_init() != 0 )
		return -1;
	
	if( platform_init() != 0 )
		return -1;

	if( game_sel_init( 1 ) != 0 )
		return -1;

	if( sdl_init() != 0 )
		return -1;

	if( ogl_init() != 0 )
		return -1;

	if( font_init() != 0 )
		return -1;	

	if( menu_init() != 0 )
		return -1;

	if( hint_init() != 0 )
		return -1;

	if( game_list_create() != 0 )
		return -1;

	sound_init();

	bg_init();

	while( !quit ) {
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glLoadIdentity();
		bg_draw();
		hint_draw( menu_level );
		menu_draw();
		if( menu_level > 0 ) {
			submenu_draw();
		}
		game_sel_draw();
		sdl_swap();
	
		if (( event = event_get() )) {
			if( supress_wait == 0 ) {
				supress();
				switch( event ) {
					case EVENT_UP:
						if( menu_level == 2 ) {
							sound_play_blip();
							game_sel_skip_back();
						}
						break;
					case EVENT_DOWN:
						if( menu_level == 2 ) {
							sound_play_blip();
							game_sel_skip_forward();
						}
						break;
					case EVENT_LEFT:
						sound_play_blip();
						if( menu_level == 0 ) {
							menu_retreat();
						}
						else if( menu_level == 1 ) {
							submenu_retreat();
						}
						else {
							game_sel_retreat();
						}
						break;
					case EVENT_RIGHT:
						sound_play_blip();
						if( menu_level == 0 ) {
							menu_advance();
						}
						else if( menu_level == 1 ) {
							submenu_advance();
						}
						else {
							game_sel_advance();
						}
						break;
					case EVENT_SELECT:
						if( menu_level == 0 ) {
							sound_play_select();
							if( submenu_create( menu_selected() ) == 0 ) {
								/* nothing to filter on, go straight to game selector */
								game_list_unfilter();
								game_sel_populate( game_first() );
								game_sel_show();
								menu_level++;
							}
							menu_level++;
						}
						else if( menu_level == 1 ) {
							submenu_do_filter();
							if( game_sel_populate( game_first() ) == 0 ) {
								sound_play_select();
								game_sel_show();
								menu_level++;
							}
							else {
								sound_play_no();
							}
						}
						else {
							if( game_sel_current() ) {
								game_sel_zoom();
								to_run = game_sel_current();
							}
						}
						break;
					case EVENT_BACK:
						if( menu_level == 2 ) {
							sound_play_back();
							game_sel_hide(HIDE_TARGET_START);
							menu_level--;
							if( submenu_items() == 0 ) {
								menu_level--;
							}
						}
						else if( menu_level == 1 ) {
							sound_play_back();
							submenu_free();
							menu_level--;
						}
						break;
					case EVENT_QUIT:
						quit = 1;
						break;
					default:
						fprintf( stderr, "Error: unknow event %d\n", event );
						break;
				}
			}
		}
		if( supress_wait > 0 )
			supress_wait--;
			
		sdl_frame_delay();

		if( to_run && !game_sel_busy() ) {
			pause_all();
			sdl_free();
			run( to_run );
			if( sdl_init() != 0 )
				return -1;
			if( ogl_init() != 0 )
				return -1;
			resume_all();
			to_run = NULL;
		}
	}

	sound_free();
	game_list_free();
	submenu_free();
	menu_free();
	hint_free();
	font_free();
	bg_free();
	sdl_free();

	return 0;
}

