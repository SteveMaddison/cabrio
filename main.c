#include <stdio.h>
#include "sdl_wrapper.h"
#include "ogl.h"
#include "font.h"
#include "config.h"
#include "category.h"
#include "platform.h"
#include "bg.h"
#include "menu.h"
#include "submenu.h"
#include "hint.h"
#include "game_sel.h"
#include "sound.h"
#include "event.h"
#include "setup.h"
#include "focus.h"

static int supress_wait = 0;

void supress( void ) {
	supress_wait = config_get()->iface.frame_rate / 4;
}

int main( int argc, char *arvg[] ) {
	int quit = 0;
	int config_status = 0;
	int event;

	config_status = config_open( NULL );
	if( config_status == -1 )
		return -1;

	if( sdl_init() != 0 )
		return -1;

	if( ogl_init() != 0 )
		return -1;

	if( event_init() != 0 )
		return -1;

	if( font_init() != 0 )
		return -1;	

	if( config_status == 1 ) {
		/* Config file didn't exist, so run the setup utility */
		if( setup() != 0 )
			return -1;
		config_update();
		if( config_create() != 0 )
			return -1;
	}

	if( platform_init() != 0 )
		return -1;

	if( category_init() != 0 )
		return -1;

	if( game_sel_init( 1 ) != 0 )
		return -1;

	if( menu_init() != 0 )
		return -1;

	if( submenu_init() != 0 )
		return -1;

	if( hint_init() != 0 )
		return -1;

	if( game_list_create() != 0 )
		return -1;

	sound_init();
	bg_init();

	event_flush();

	while( !quit ) {
		ogl_clear();
		bg_draw();
		hint_draw( focus_has() );
		menu_draw();
		submenu_draw();
		game_sel_draw();
		sdl_swap();
	
		if (( event = event_poll() )) {
			if( supress_wait == 0 ) {
				if( event == EVENT_QUIT ) {
					quit = 1;
				}
				else {
					supress();
					event_process( event );
				}
			}
		}
		if( supress_wait > 0 )
			supress_wait--;
			
		sdl_frame_delay();
	}

	sound_free();
	game_list_free();
	submenu_free();
	menu_free();
	hint_free();
	font_free();
	bg_free();
	event_free();
	sdl_free();

	return 0;
}
