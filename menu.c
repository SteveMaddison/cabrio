#include <stdlib.h>
#include "sdl_ogl.h"
#include "menu.h"
#include "font.h"

static const int STEPS = 30;
static const int FONT_SCALE = 400;
static const GLfloat ALPHA_MIN = 0.6;
static const GLfloat ITEM_MAX_WIDTH = 1.0;
static GLfloat item_height = 0.6;
static GLfloat item_width = 1.0;
static GLuint menu_texture = 0;

const char *menu_text[] = {
	"All",
	"Genre",
	"Platform",
	NULL
};

struct menu_item *menu_start = NULL;
struct menu_item *menu_end = NULL;
struct menu_item *selected = NULL;
struct menu_item *prev = NULL;
static int menu_items = 0;
static int step = 0;

int menu_selected( void ) {
	return selected->id;
}

int menu_load_texture( void ) {
	int x,y;
	menu_texture = sdl_create_texture( DATA_DIR "/pixmaps/menu_item.png", &x ,&y );
	if( menu_texture == 0 ) {
		fprintf( stderr, "Warning: Couldn't create texture for menu items\n" );
		return -1;
	}
	return 0;
}

void menu_free_texture( void ) {
	if( menu_texture ) {
		ogl_free_texture( &menu_texture );
	}
}

void menu_pause( void ) {
	struct menu_item *item = menu_start;
	step = 0;
	prev = selected;
	menu_free_texture();
	
	while( item ) {
		font_message_free( item->message );
		item = item->next;
	}
}

int menu_resume( void ) {
	struct menu_item *item = menu_start;
	int i = 0;
	
	if( menu_load_texture() != 0 )
		return -1;
		
	while( item ) {
		item->message = font_message_create( menu_text[i] );
		if( item->message == NULL )
			return -1;
		item = item->next;
		i++;
	}
	return 0;
}

int menu_init( void ) {
	struct menu_item *item = NULL;
	struct menu_item *prev = NULL;

	if( menu_load_texture() != 0 ) {
		return -1;
	}
		
	menu_items = 0;
	while( menu_text[menu_items] ) {
		item = malloc(sizeof(struct menu_item));
		if( item ) {
			item->text = (char*)menu_text[menu_items];
			item->message = font_message_create( menu_text[menu_items] );
			if( item->message == NULL ) {
				fprintf( stderr, "Error: Couldn't create message for menu item '%s'\n", menu_text[menu_items] );
				return -1;
			}
			item->next = NULL;
			item->prev = prev;
			item->id = menu_items;
			if( prev ) {
				prev->next = item;
			}

			if( menu_items == 0 ) {
				menu_start = item;
			}
			menu_end = item;
			
			prev = item;
			menu_items++;
		}
		else {
			fprintf( stderr, "Warning: Couldn't allocate memory for menu item '%s'\n", menu_text[menu_items] );
			return -1;
		}
	}

	item_width = (4.0/menu_items > ITEM_MAX_WIDTH) ? ITEM_MAX_WIDTH : (4.0/menu_items);
	item_height = item_width*0.6;
	
	selected = menu_start;
	prev = selected;
	return 0;
}

void menu_draw( void ) {
	struct menu_item *item = menu_start;
	GLfloat zoom, offset,tx,ty,alpha = 0;
	int count = 0;
	
	while( item ) {
		if( prev != selected ) {
			if( step++ > STEPS ) {
				step = 0;
				prev = selected;
			}
		}

		offset = ((GLfloat)count-((GLfloat)(menu_items-1)/2))/((GLfloat)menu_items/4);
		if( item == prev ) {
			zoom = (0.05*STEPS)-(0.05*(step+1));
			alpha = 1.0-(((1.0-ALPHA_MIN)/STEPS)*step);
		}
		else if ( item == selected ) {
			zoom = 0.05*(step+1);
			alpha = ALPHA_MIN+(((1.0-ALPHA_MIN)/STEPS)*step);
		}
		else {
			zoom = 0;
			alpha = ALPHA_MIN;
		}
		tx = (GLfloat)item->message->width/FONT_SCALE;
		if( tx > item_width/2 ) {
			tx = (item_width/2)-(item_width/10);
		}
		ty = (GLfloat)item->message->height/FONT_SCALE;

		ogl_load_alterego();
		glTranslatef( (offset*1.3)-(offset*zoom*0.2), 2-(zoom*0.4), -6+zoom );
		glColor4f( 1.0, 1.0, 1.0, alpha );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, menu_texture );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-(item_width/2),  (item_height/2), 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-(item_width/2), -(item_height/2), 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( (item_width/2), -(item_height/2), 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( (item_width/2),  (item_height/2), 0.0);
		glEnd();

		glBindTexture( GL_TEXTURE_2D, item->message->texture );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f( -tx,  ty, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f( -tx, -ty, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(  tx, -ty, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(  tx,  ty, 0.0);
		glEnd();
		
		item = item->next;
		count++;
	}
}

void menu_advance( void ) {
	if( selected == prev ) {
		prev = selected;
		selected = selected->next;
		if( selected == NULL )
			selected = menu_start;
	}
}

void menu_retreat( void ) {
	if( selected == prev ) {
		prev = selected;
		selected = selected->prev;
		if( selected == NULL )
			selected = menu_end;
	}
}

void menu_item_free( struct menu_item *m ) {
	if( m->message ) {
		font_message_free( m->message );
		m->message = NULL;
	}
	menu_start = m->next;
	free( m );
	m = NULL;
}

void menu_free( void ) {
	while( menu_start ) {
		menu_item_free( menu_start );
	}
	menu_end = NULL;
	menu_free_texture();
}

