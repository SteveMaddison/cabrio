#ifndef _MENU_H_
#define _MENU_H_ 1

#include "ogl.h"
#include "font.h"

#define MENU_ALL		0
#define MENU_GENRE		1
#define MENU_PLATFORM	2

struct menu_item {
	int id;
	struct menu_item *next;
	struct menu_item *prev;
	struct font_message *message;
	char *text;
};

int menu_init( void );
void menu_free( void );
void menu_pause( void );
int menu_resume( void );

int menu_selected( void );
void menu_draw( void );
void menu_advance( void );
void menu_retreat( void );

#endif

