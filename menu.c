#include <stdlib.h>
#include "sdl_ogl.h"
#include "menu.h"
#include "font.h"

static const int MAX_STEPS = 50;
const char *menu_text_all = "All";
const char *menu_text_platform = "Platform";

struct menu_item *menu_start = NULL;
struct menu_item *menu_end = NULL;
struct menu_item *selected = NULL;
struct menu_item *prev = NULL;
static GLuint menu_texture = 0;
static int menu_items = 0;
static int items_visible = 0;
static int scroll = 0;
static int steps = 0;
static int step = 0;
static GLfloat spacing = 1.0;
static GLfloat min_alpha = 1.0;

struct menu_item *menu_selected( void ) {
	return selected;
}

int menu_item_count( void ) {
	return menu_items;
}

int menu_load_texture( void ) {
	menu_texture = sdl_create_texture( config_get()->iface.menu.texture, NULL ,NULL );
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
		item->message = font_message_create( item->text );
		if( item->message == NULL )
			return -1;
		item = item->next;
		i++;
	}
	return 0;
}

int menu_item_add( const char *text, int type, struct category *category ) {
	struct menu_item *item = malloc( sizeof(struct menu_item) );

	if( item ) {
		item->text = (char*)text;
		item->message = font_message_create( text );
		if( item->message == NULL ) {
			fprintf( stderr, "Error: Couldn't create message for menu item '%s'\n", text );
			return -1;
		}
		item->type = type;
		item->category = category;
		item->position = menu_items;

		item->next = NULL;
		item->prev = menu_end;
		if( menu_end ) {
			menu_end->next = item;
		}
		if( menu_items == 0 ) {
			menu_start = item;
		}
		menu_end = item;
		menu_items++;
	}
	else {
		fprintf( stderr, "Warning: Couldn't allocate memory for menu item '%s'\n", text );
		return -1;
	}
	
	return 0;
}

int menu_init( void ) {
	struct category *category = category_first();
	const struct config *config = config_get();
	
	if( config->iface.frame_rate )
		steps = config->iface.frame_rate/3;
	else
		steps = MAX_STEPS;

	spacing = (GLfloat)config->iface.menu.spacing;
	if( spacing < 0 ) {
		/* auto spacing */
		if( config->iface.menu.orientation == CONFIG_LANDSCAPE )
			spacing = (GLfloat)(config->iface.menu.item_width * config->iface.menu.zoom * 1.2);
		else
			spacing = (GLfloat)(config->iface.menu.item_height * config->iface.menu.zoom * 1.2);
	}

	min_alpha = 1.0 - (GLfloat)(config->iface.menu.transparency)/100;

	if( menu_load_texture() != 0 ) {
		return -1;
	}

	menu_item_add( menu_text_all, MENU_ALL, NULL );
	menu_item_add( menu_text_platform, MENU_PLATFORM, NULL );
	if( category ) {
		do {
			menu_item_add( category->name, MENU_CATEGORY, category );
			category = category->next;
		} while( category != category_first() );
	}

	items_visible = menu_items;
	if( menu_items > config->iface.menu.max_visible )
		items_visible = config->iface.menu.max_visible;

	selected = menu_start;
	prev = selected;
	return 0;
}

void menu_draw( void ) {
	struct menu_item *item = menu_start;
	GLfloat zoom,offset,tx,ty,mx,my,alpha = 0;
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();
	const struct config_menu *config = &config_get()->iface.menu;
	
	while( item ) {
		if( prev != selected ) {
			if( step++ > steps ) {
				step = 0;
				prev = selected;
			}
		}

		if( item == prev ) {
			zoom = config->zoom - (((config->zoom - 1)/(GLfloat)steps) * (GLfloat)step);
			alpha = 1.0-(((1.0-min_alpha)/steps)*step);
		}
		else if ( item == selected ) {
			zoom = 1 + ((config->zoom - 1)/(GLfloat)steps) * (GLfloat)step;
			alpha = min_alpha+(((1.0-min_alpha)/steps)*step);
		}
		else {
			zoom = 1;
			alpha = min_alpha;
		}
		tx = ((GLfloat)item->message->width * config->font_scale) * xfactor;
		if( tx > config->item_width/2 ) {
			tx = (config->item_width/2)-(config->item_width/10);
		}
		tx *= zoom;
		ty = ((GLfloat)item->message->height * config->font_scale) * xfactor * zoom;

		mx = (config->item_width/2)*xfactor*zoom;
		my = (config->item_height/2)*xfactor*zoom;

		ogl_load_alterego();
		offset = (((GLfloat)(item->position - scroll) - ( ((GLfloat)items_visible-1)/2 )) * spacing);
		if( config->orientation == CONFIG_LANDSCAPE )
			glTranslatef( (offset + config->x_offset) * xfactor, config->y_offset * yfactor, -6 );
		else
			glTranslatef( config->y_offset * xfactor, (-offset + config->x_offset) * yfactor, -6 );
		glColor4f( 1.0, 1.0, 1.0, alpha );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, menu_texture );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-mx,  my, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-mx, -my, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( mx, -my, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( mx,  my, 0.0);
		glEnd();

		glBindTexture( GL_TEXTURE_2D, item->message->texture );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f( -tx,  ty, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f( -tx, -ty, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f(  tx, -ty, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f(  tx,  ty, 0.0);
		glEnd();
		
		item = item->next;
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

