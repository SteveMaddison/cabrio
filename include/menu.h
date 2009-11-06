#ifndef _MENU_H_
#define _MENU_H_ 1

#include "ogl.h"
#include "font.h"
#include "category.h"

enum menu_t {
	MENU_ALL,
	MENU_PLATFORM,
	MENU_CATEGORY
};

struct menu_item {
	int type;
	struct menu_item *next;
	struct menu_item *prev;
	struct texture *message;
	struct category *category;
	char *text;
};

struct menu_tile {
	struct menu_tile *next;
	struct menu_tile *prev;
	struct menu_item *item;
	GLfloat x;
	GLfloat y;
};

int menu_init( void );
void menu_free( void );
void menu_pause( void );
int menu_resume( void );

struct menu_item *menu_selected( void );
int menu_item_count( void );
void menu_draw( void );
void menu_advance( void );
void menu_retreat( void );

#endif

