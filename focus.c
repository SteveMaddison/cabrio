#include "focus.h"
#include "menu.h"
#include "submenu.h"
#include "game_sel.h"

static int focus = FOCUS_MENU;
static int prev = FOCUS_MENU;

int focus_has( void ) {
	return focus;
}

int focus_prev( void ) {
	return prev;
}

void focus_set( int f ) {
	if( f != focus ) {
		prev = focus;
		switch( prev ) {
			case FOCUS_MENU:
				menu_lost_focus();
				break;
			case FOCUS_SUBMENU:
				submenu_lost_focus();
				break;
			case FOCUS_GAMESEL:
				game_sel_lost_focus();
				break;
		}

		focus = f;
		switch( focus ) {
			case FOCUS_MENU:
				menu_got_focus();
				break;
			case FOCUS_SUBMENU:
				submenu_got_focus();
				break;
			case FOCUS_GAMESEL:
				game_sel_got_focus();
				break;
		}
	}
}

