#include <stdlib.h>
#include "sdl_ogl.h"
#include "hint.h"
#include "menu.h"
#include "submenu.h"
#include "platform.h"
#include "focus.h"
#include "sound.h"
#include "font.h"
#include "location.h"

static const int MAX_STEPS = 100;
static const GLfloat FONT_SCALE = 0.0025;
static const GLfloat ITEM_SCALE = 0.8;
static const GLfloat MENU_DEPTH = -6;
static const GLfloat SPACING_FACTOR = 1.1;

static struct menu_item *menu_start = NULL;
static struct menu_tile *selected = NULL;
static struct menu_tile *tile_start = NULL;
static struct menu_tile *tile_end = NULL;
static struct texture *menu_texture = NULL;
static struct arrow arrow_advance;
static struct arrow arrow_retreat;
static int menu_items = 0;
static int items_visible = 0;
static int steps = 0;
static int step = 0;
static int direction = 0;
static int hide_direction = 0;
static int visible = 0;
static GLfloat spacing = 1.0;
static GLfloat min_alpha = 1.0;
static GLfloat zoom = 1.0;

struct menu_item *menu_selected( void ) {
	return selected->item;
}

struct menu_tile *menu_tile_selected( void ) {
	return selected;
}

int menu_item_count( void ) {
	return menu_items;
}

int menu_load_texture( void ) {
	char filename[CONFIG_FILE_NAME_LENGTH];
	
	location_get_theme_path( config_get()->iface.theme.menu.texture, filename );
	menu_texture = sdl_create_texture( filename );
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
	
	if( item ) {
		do {
			ogl_free_texture( item->message );
			item = item->next;
		} while( item != menu_start );
	}
}

int menu_resume( void ) {
	struct menu_item *item = menu_start;
	int i = 0;
	
	if( menu_load_texture() != 0 )
		return -1;
	
	if( item ) {	
		do {
			item->message = font_create_texture( item->text );
			if( item->message == NULL )
				return -1;
			item = item->next;
			i++;
		} while( item != menu_start );
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

	zoom = (GLfloat)config->iface.theme.menu.zoom;
	if( zoom < 1 ) {
		fprintf( stderr, "Warning: Menu zoom parameter less than 1 - setting to 1\n" );
		zoom = 1;
	}

	spacing = (GLfloat)config->iface.theme.menu.spacing;
	if( spacing < 0 ) {
		/* auto spacing */
		if( config->iface.theme.menu.orientation == CONFIG_LANDSCAPE )
			spacing = (GLfloat)config->iface.theme.menu.item_width * ITEM_SCALE * zoom * SPACING_FACTOR;
		else
			spacing = (GLfloat)config->iface.theme.menu.item_height * ITEM_SCALE * zoom * SPACING_FACTOR;
	}

	min_alpha = 1.0 - (GLfloat)(config->iface.theme.menu.transparency)/100;

	if( menu_load_texture() != 0 ) {
		return -1;
	}

	menu_item_add( config->iface.labels.label_all, MENU_ALL, NULL );
	if( platform_count() > 1 || config->iface.prune_menus == 0 ) {
		menu_item_add( config->iface.labels.label_platform, MENU_PLATFORM, NULL );
	}
	if( category ) {		
		do {
			if( category->value_count > 1 || config->iface.prune_menus == 0 )
				menu_item_add( category->name, MENU_CATEGORY, category );
			category = category->next;
		} while( category != category_first() );
	}

	items_visible = menu_items;
	if( menu_items > config->iface.theme.menu.max_visible )
		items_visible = config->iface.theme.menu.max_visible;

	if( items_visible ) {
		struct menu_item *item = menu_start;
		if( item ) {
			item = item->next;
			for( i = 0 ; i < items_visible + 2 ; i++ ) {
				struct menu_tile *tile = malloc( sizeof(struct menu_tile) );
				if( tile ) {
					GLfloat offset = (((GLfloat)(i) - (((GLfloat)items_visible+1)/2 )) * spacing);
					if( config->iface.theme.menu.orientation == CONFIG_LANDSCAPE ) {
						tile->x = (offset + config->iface.theme.menu.offset1);
						tile->y = config->iface.theme.menu.offset2;
					}
					else {
						tile->x = -config->iface.theme.menu.offset2 * ogl_aspect_ratio();
						tile->y = -(offset + config->iface.theme.menu.offset1);
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
	/* Use the middle tile as selected */
	for( i = 0 ; i < ((config->iface.theme.menu.max_visible+1)/2) ; i++ ) {
		selected = selected->next;
	}
	if( !selected )
		selected = tile_start;
	selected->zoom = zoom;
	selected->alpha = 1;

	while( selected->item->type != MENU_ALL ) {
		struct menu_tile *skip = tile_start;
		while( skip ) {
			skip->item = skip->item->next;
			skip = skip->next;
		}
	}
	
	tile_start->zoom = 0;
	tile_start->alpha = 0;
	tile_start->x = tile_start->next->x;
	tile_start->y = tile_start->next->y;
	
	tile_end->zoom = 0;
	tile_end->alpha = 0;	
	tile_end->x = tile_end->prev->x;
	tile_end->y = tile_end->prev->y;

	arrow_retreat.size = config->iface.theme.menu.item_height * ITEM_SCALE * 1.5;
	arrow_advance.size = config->iface.theme.menu.item_height * ITEM_SCALE * 1.5;
	if( config->iface.theme.menu.orientation == CONFIG_LANDSCAPE ) {
		arrow_retreat.x = tile_start->x - (config->iface.theme.menu.item_width * ITEM_SCALE);
		arrow_retreat.y = tile_start->y;
		arrow_advance.x = tile_end->x + (config->iface.theme.menu.item_width * ITEM_SCALE);
		arrow_advance.y = tile_end->y;
		arrow_retreat.angle = 90;
		arrow_advance.angle = -90;
	}
	else {
		arrow_retreat.x = tile_start->x;
		arrow_retreat.y = tile_start->y + (config->iface.theme.menu.item_height * ITEM_SCALE);
		arrow_advance.x = tile_end->x;
		arrow_advance.y = tile_end->y - (config->iface.theme.menu.item_height * ITEM_SCALE);
		arrow_retreat.angle = 0;
		arrow_advance.angle = 180;	
	}
	
	return 0;
}

void menu_draw( void ) {
	if( visible ) {
		struct menu_tile *tile = tile_start;
		GLfloat item_zoom,tx,ty,mx,my = 0;
		GLfloat xfactor = ogl_xfactor();
		GLfloat yfactor = ogl_yfactor();
		const struct config_menu *config = &config_get()->iface.theme.menu;
		
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
				item_zoom = tile->zoom + (((dest->zoom-tile->zoom)/steps) * step);

				if( hide_direction ) {
					if( hide_direction == 1 )
						item_zoom = item_zoom - ((item_zoom / steps) * (steps - step));
					else
						item_zoom = (item_zoom / steps) * (steps - step);
				
					if( step-- == 1 ) {
						/* Animation complete */
						if( hide_direction == -1 )
							visible = 1;
						else
							visible = 0;
						hide_direction = 0;
					}
				}
	
				mx = ((config->item_width * ITEM_SCALE)/2) * xfactor * item_zoom;
				my = ((config->item_height * ITEM_SCALE)/2) * xfactor * item_zoom;
				
				/* Make sure the text fits inside the box */
				tx = ((GLfloat)tile->item->message->width * (config->font_scale * FONT_SCALE)) * xfactor * item_zoom;
				if( tx > mx * (1.0 - ((GLfloat)config->border/100)) )
					tx = mx * (1.0 - ((GLfloat)config->border/100));
				ty = ((GLfloat)tile->item->message->height * (config->font_scale * FONT_SCALE)) * xfactor * item_zoom;
				if( ty > my * (1.0 - ((GLfloat)config->border/100)) )
					ty = my * (1.0 - ((GLfloat)config->border/100));
	
				ogl_load_alterego();
				glTranslatef( (tile->x + (((dest->x-tile->x)/steps) * step)) * xfactor,
						 	  (tile->y + (((dest->y-tile->y)/steps) * step)) * yfactor,
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
		
		if( focus_has() == FOCUS_MENU ) {
			hint_draw_arrow( &arrow_retreat );
			hint_draw_arrow( &arrow_advance );
		}
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

void menu_hide( void ) {
	if( hide_direction == 0 && visible == 1 ) {
		hide_direction = 1;
		step = steps;		
	}
}

void menu_show( void ) {
	if( hide_direction == 0 && visible == 0 ) {
		visible = 1;
		hide_direction = -1;
		step = steps;	
	}	
}

int menu_event( int event ) {
	int o = config_get()->iface.theme.menu.orientation;
	switch( event ) {
		case EVENT_UP:
			if( o == CONFIG_PORTRAIT ) {
				sound_play( SOUND_BLIP );
				menu_retreat();
			}
			else {
				sound_play( SOUND_NO );
			}		
			break;
		case EVENT_DOWN:
			if( o == CONFIG_PORTRAIT ) {
				sound_play( SOUND_BLIP );
				menu_advance();
			}
			else {
				sound_play( SOUND_NO );
			}		
			break;
		case EVENT_LEFT:
			if( o == CONFIG_LANDSCAPE ) {
				sound_play( SOUND_BLIP );
				menu_retreat();
			}
			else {
				sound_play( SOUND_NO );
			}		
		case EVENT_RIGHT:
			if( o == CONFIG_LANDSCAPE ) {
				sound_play( SOUND_BLIP );
				menu_advance();
			}
			else {
				sound_play( SOUND_NO );
			}		
			break;
		case EVENT_SELECT:
			sound_play( SOUND_SELECT );
			focus_set( FOCUS_SUBMENU );
			break;
		default:
			break;		
	}
	return 0;
}

int menu_got_focus( void ) {
	submenu_hide();
	return 0;
}

int menu_lost_focus( void ) {
	return 0;
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

