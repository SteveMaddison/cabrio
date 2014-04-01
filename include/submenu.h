#ifndef _SUBMENU_H_
#define _SUBMENU_H_ 1

#include "menu.h"

int submenu_init( void );
int submenu_create( struct menu_item *item );
int submenu_event( int event );
int submenu_got_focus( void );
int submenu_lost_focus( void );
int submenu_items( void );
void submenu_pause( void );
int submenu_resume( void );
void submenu_free( void );
void submenu_draw( void );
int submenu_do_filter( void );
void submenu_hide( void );
void submenu_show( void );

#endif

