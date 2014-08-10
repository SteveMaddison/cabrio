#ifndef _FOCUS_H_
#define _FOCUS_H_ 1

enum focus_t {
	FOCUS_MENU,
	FOCUS_SUBMENU,
	FOCUS_GAMESEL,
	FOCUS_LIST1,
	FOCUS_LIST2,
	FOCUS_LIST3,
	FOCUS_LIST4,
	FOCUS_LIST5
};

int focus_has( void );
int focus_prev( void );
void focus_set( int f );

#endif

