#include <stdlib.h>
#include "sdl_ogl.h"
#include "menu.h"
#include "font.h"

static const int MAX_STEPS = 50;
static const GLfloat MENU_DEPTH = -6;
static const GLfloat SPACING_FACTOR = 1.2;
const char *menu_text_all = "All";
const char *menu_text_platform = "Platform";

struct menu_item *menu_start = NULL;
struct menu_item *menu_end = NULL;
struct menu_tile *selected = NULL;
struct menu_tile *tile_start = NULL;
struct menu_tile *tile_end = NULL;
static struct texture *menu_texture = NULL;
static int menu_items = 0;
static int items_visible = 0;
static int steps = 0;
static int step = 0;
static int direction = 0;
static GLfloat spacing = 1.0;
static GLfloat min_alpha = 1.0;
static GLfloat zoom = 1.0;

struct menu_item *menu_selected( void ) {
	return selected->item;
}

int menu_item_count( void ) {
	return menu_items;
}

int menu_load_texture( void ) {
	menu_texture = sdl_create_texture( config_get()->iface.menu.texture );
	if( menu_texture == 0 ) {
		fprintf( stderr, "Warning: Couldn't create texture for menu items\n" );
		return -1;
	}
	return 0;
}

void menu_free_texture( void ) {
	if( menu_texture ) {
		ogl_free_texture( menu_texture );
	}
}

void menu_pause( void ) {
	struct menu_item *item = menu_start;
	step = 0;
	menu_free_texture();
	
	while( item ) {
		ogl_free_texture( item->message );
		item = item->next;
	}
}

int menu_resume( void ) {
	struct menu_item *item = menu_start;
	int i = 0;
	
	if( menu_load_texture() != 0 )
		return -1;
		
	while( item ) {
		item->message = font_create_texture( item->text );
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
		item->message = font_create_texture( text );
		if( item->message == NULL ) {
			fprintf( stderr, "Error: Couldn't create message for menu item '%s'\n", text );
			return -1;
		}
		item->type = type;
		item->category = category;

		if( menu_start == NULL ) {
			item->next = item;
			item->prev = item;
			menu_start = item;
		}
		else {
			menu_start->next->prev = item;
			item->next = menu_start->next;
			menu_start->next = item;
			item->prev = menu_start;
		}

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
	int i;
	
	if( config->iface.frame_rate )
		steps = config->iface.frame_rate;
	else
		steps = MAX_STEPS;

	zoom = (GLfloat)config->iface.menu.zoom;
	if( zoom < 1 ) {
		fprintf( stderr, "Warning: Menu zoom parameter less than 1 - setting to 1\n" );
		zoom = 1;
	}

	spacing = (GLfloat)config->iface.menu.spacing;
	if( spacing < 0 ) {
		/* auto spacing */
		if( config->iface.menu.orientation == CONFIG_LANDSCAPE )
			spacing = (GLfloat)config->iface.menu.item_width * zoom * SPACING_FACTOR;
		else
			spacing = (GLfloat)config->iface.menu.item_height * zoom * SPACING_FACTOR;
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

	if( items_visible ) {
		struct menu_item *item = menu_start;
		if( item ) {
			item = item->next;
			for( i = 0 ; i < items_visible + 2 ; i++ ) {
				struct menu_tile *tile = malloc( sizeof(struct menu_tile) );
				if( tile ) {
					GLfloat offset = (((GLfloat)(i) - (((GLfloat)items_visible+1)/2 )) * spacing);
					if( config->iface.menu.orientation == CONFIG_LANDSCAPE ) {
						tile->x = (offset + config->iface.menu.offset1) * ogl_xfactor();
						tile->y = config->iface.menu.offset2 * ogl_yfactor();
					}
					else {
						tile->x = -config->iface.menu.offset2 * ogl_xfactor() * ogl_aspect_ratio();
						tile->y = -(offset + config->iface.menu.offset1) * ogl_yfactor();
					}
					
					tile->zoom = 1;
					tile->alpha = min_alpha;
					tile->item = item;
					item = item->next;
					
					tile->next = NULL;
					tile->prev = tile_end;
					if( tile_end )
						tile_end->next = tile;
					else
						tile_start = tile;
					tile_end = tile;
				}
				else {
					fprintf( stderr, "Warning: Couldn't allocate memeory for menu tile %d\n", i );
				}
			}
		}
	}
	
	selected = tile_start;
	for( i = 0 ; i < ((menu_items+1)/2) ; i++ ) {
		selected = selected->next;
	}
	selected->zoom = zoom;
	selected->alpha = 1;
	
	tile_start->zoom = 0;
	tile_start->alpha = 0;
	tile_start->x = tile_start->next->x;
	tile_start->y = tile_start->next->y;
	
	tile_end->zoom = 0;
	tile_end->alpha = 0;
	tile_end->x = tile_end->prev->x;
	tile_end->y = tile_end->prev->y;
	
	return 0;
}

void menu_draw( void ) {
	struct menu_tile *tile = tile_start;
	GLfloat item_zoom,tx,ty,mx,my = 0;
	GLfloat xfactor = ogl_xfactor();
	const struct config_menu *config = &config_get()->iface.menu;
	
	while( tile ) {
		struct menu_tile *dest = tile;
		
		if( direction ) {
			if( direction > 0 )
				dest = tile->next;
			else
				dest = tile->prev;
				
			if( step-- == 0 ) {
				/* Animation complete */
				direction = 0;
			}
		}

		if( dest ) {
			/* Make sure the text fits inside the box */
			item_zoom = tile->zoom + (((dest->zoom-tile->zoom)/steps) * step);
			
			tx = ((GLfloat)tile->item->message->width * config->font_scale) * xfactor;
			if( tx > config->item_width/2 ) {
				tx = ((config->item_width/2)-(config->item_width)) / 10;
			}
			tx *= item_zoom;
			ty = ((GLfloat)tile->item->message->height * config->font_scale) * xfactor;
			if( ty > config->item_height/2 ) {
				ty = ((config->item_height/2)-(config->item_height)) / 10;
			}
			ty *= item_zoom;

			mx = (config->item_width/2) * xfactor * item_zoom;
			my = (config->item_height/2) * xfactor * item_zoom;

			ogl_load_alterego();
			glTranslatef( tile->x + (((dest->x-tile->x)/steps) * step),
					 	  tile->y + (((dest->y-tile->y)/steps) * step),
					 	  MENU_DEPTH );
			glColor4f( 1.0, 1.0, 1.0, tile->alpha + (((dest->alpha-tile->alpha)/steps) * step) );
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

			glBindTexture( GL_TEXTURE_2D, menu_texture->id );
			glBegin( GL_QUADS );
				glTexCoord2f(0.0, 0.0); glVertex3f(-mx,  my, 0.0);
				glTexCoord2f(0.0, 1.0); glVertex3f(-mx, -my, 0.0);
				glTexCoord2f(1.0, 1.0); glVertex3f( mx, -my, 0.0);
				glTexCoord2f(1.0, 0.0); glVertex3f( mx,  my, 0.0);
			glEnd();

			glBindTexture( GL_TEXTURE_2D, tile->item->message->id );
			glBegin( GL_QUADS );
				glTexCoord2f(0.0, 0.0); glVertex3f( -tx,  ty, 0.0);
				glTexCoord2f(0.0, 1.0); glVertex3f( -tx, -ty, 0.0);
				glTexCoord2f(1.0, 1.0); glVertex3f(  tx, -ty, 0.0);
				glTexCoord2f(1.0, 0.0); glVertex3f(  tx,  ty, 0.0);
			glEnd();
		}
		
		tile = tile->next;
	}
}

void menu_advance( void ) {
	struct menu_tile *t = tile_start;

	if( direction == 0 ) {
		direction = 1;
		step = steps;

		while( t ) {
			t->item = t->item->next;
			t = t->next;
		}
	}
}

void menu_retreat( void ) {
	struct menu_tile *t = tile_start;
	
	if( direction == 0 ) {
		direction = -1;
		step = steps;

		while( t ) {
			t->item = t->item->prev;
			t = t->next;
		}
	}
}

void menu_free( void ) {
	struct menu_item *item = menu_start;
	
	if( item ) {
		do {
			if( item->message ) {
				ogl_free_texture( item->message );
			}
			item = item->next;
		} while( item != menu_start );
	}

	menu_free_texture();
}

