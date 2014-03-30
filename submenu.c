#include "submenu.h"
#include "sdl_ogl.h"
#include "font.h"
#include "game.h"
#include "platform.h"
#include "menu.h"
#include "focus.h"
#include "game_sel.h"
#include "hint.h"
#include "sound.h"
#include "location.h"

static const GLfloat ITEM_SCALE = 1.2;
static const GLfloat FONT_SCALE = 0.0035;
static const GLfloat PLATFORM_SIZE = 1.8;
static const GLfloat SCROLL_HIDE_OFFSET = 6;
static struct texture *texture = NULL;
static int type = MENU_ALL;
static int selected = 0;
static int items = 0;
static GLfloat item_width = 1.0;
static GLfloat item_height = 0.25;
static GLfloat font_scale = 1.0;
static const char *VALUE_UNKNOWN = "???";
static const int MAX_STEPS = 100;

static struct category *prev_category = NULL;
static struct category *category = NULL;
static struct category_value *category_value = NULL;
static struct platform *platform = NULL;
static struct texture *message = NULL;
static struct arrow arrow_retreat;
static struct arrow arrow_advance;
static int steps = 0;
static int step = 0;
static int hide_direction = 0;
static int visible = 0;
static int scroll_direction = 0;
static int scroll_step = 0;

int submenu_init( void ) {
	const struct config_submenu *config = &config_get()->iface.theme.submenu;

	if( config_get()->iface.frame_rate )
		steps = config_get()->iface.frame_rate/6;
	else
		steps = MAX_STEPS;
	
	item_width = config->item_width * ITEM_SCALE;
	item_height = config->item_height * ITEM_SCALE;
	font_scale = config->font_scale * FONT_SCALE;
	
	return 0;
}

int submenu_selected( void ) {
	return selected;
}

int submenu_items( void ) {
	return items;
}

int submenu_load_texture( void ) {
	char filename[CONFIG_FILE_NAME_LENGTH];

	location_get_theme_path( config_get()->iface.theme.submenu.texture, filename );
	texture = sdl_create_texture( filename );
	if( texture == NULL ) {
		fprintf( stderr, "Warning: Couldn't create texture for submenu items\n" );
		return -1;
	}
	return 0;
}

void submenu_free_texture( void ) {
	if( texture )
		ogl_free_texture( texture );
}

void submenu_free( void ) {
	if( message )
		ogl_free_texture( message );
	if( texture )
		ogl_free_texture( texture );
}

void submenu_pause( void ) {
	submenu_free();
}

int submenu_resume( void ) {
	if( submenu_load_texture() != 0 )
		return -1;

	switch( type ) {
		case( MENU_ALL ):
			break;
		case( MENU_CATEGORY ):
			if( category_value->name )
				message = font_create_texture( category_value->name );
			else
				message = font_create_texture( VALUE_UNKNOWN );
			break;
		case( MENU_PLATFORM ):
			message = font_create_texture( platform->name );
			break;		
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu (resume,%d)\n", type );
			break;
	}
	return 0;
}

int submenu_create( struct menu_item *item ) {
	const struct config_submenu *config = &config_get()->iface.theme.submenu;
	items = 0;
	
	if( submenu_load_texture() != 0 )
		return -1;

	arrow_retreat.x = menu_tile_selected()->x - ((item_width/2) + item_height);
	arrow_retreat.y = (menu_tile_selected()->y + config->offset1 );
	arrow_retreat.size = item_height * 2;
	arrow_retreat.angle = 90;
	
	arrow_advance.x = menu_tile_selected()->x + ((item_width/2) + item_height);
	arrow_advance.y = (menu_tile_selected()->y + config->offset1 );
	arrow_advance.size = item_height * 2;
	arrow_advance.angle = -90;

	type = item->type;
	
	switch( type ) {
		case( MENU_ALL ):
			break;
		case( MENU_PLATFORM ):
			if( platform == NULL )
				platform = platform_first();
			message = font_create_texture( platform->name );
			items = platform_count();
			break;		
		case( MENU_CATEGORY ):
			category = item->category;
			if( category != prev_category )
				category_value = category->values;
			if( category_value->name )
				message = font_create_texture( category_value->name );
			else
				message = font_create_texture( VALUE_UNKNOWN );
			items = category->value_count;
			prev_category = category;
			break;
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu (create,%d)\n", type );
			break;
	}
	
	return items;
}

void submenu_advance( void ) {
	scroll_direction = 1;
	scroll_step = steps;

	if( message ) {
		ogl_free_texture( message );
	}
	switch( type ) {
		case( MENU_ALL ):
			break;
		case( MENU_PLATFORM ):
			platform = platform->next;
			message = font_create_texture( platform->name );
			break;
		case( MENU_CATEGORY ):
			category_value = category_value->next;
			if( category_value->name )
				message = font_create_texture( category_value->name );
			else
				message = font_create_texture( VALUE_UNKNOWN );
			break;
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu (advance,%d)\n", type );
			break;
	}
}

void submenu_retreat( void ) {
	scroll_direction = -1;
	scroll_step = steps;

	if( message ) {
		ogl_free_texture( message );
	}
	switch( type ) {
		case( MENU_ALL ):
			break;
		case( MENU_PLATFORM ):
			platform = platform->prev;
			message = font_create_texture( platform->name );
			break;
		case( MENU_CATEGORY ):
			category_value = category_value->prev;
			if( category_value->name )
				message = font_create_texture( category_value->name );
			else
				message = font_create_texture( VALUE_UNKNOWN );
			break;
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu (retreat,%d)\n", type );
			break;
	}
}

void submenu_hide( void ) {
	if( visible == 1 ) {
		hide_direction = 1;
		step = steps;
	}
}

void submenu_show( void ) {
	if( visible == 0 ) {
		visible = 1;
		hide_direction = -1;
		step = steps;	
	}	
}

int submenu_got_focus( void ) {
	if( focus_prev() == FOCUS_MENU ) {
		submenu_create( menu_selected() );
		if( items <= 1 ) {
			submenu_do_filter();
			if( items > 0 )
				submenu_show();
			focus_set( FOCUS_GAMESEL );
		}
		else {
			submenu_show();
		}
	}
	else if( focus_prev() == FOCUS_GAMESEL ) {
		if( items <= 1 ) {
			focus_set( FOCUS_MENU );
		}
		else {
			submenu_show();
		}
	}

	return 0;
}

int submenu_lost_focus( void ) {
	return 0;
}

int submenu_event( int event ) {
	switch( event ) {	
		case EVENT_LEFT:
			sound_play( SOUND_BLIP );
			submenu_retreat();
			break;
		case EVENT_RIGHT:
			sound_play( SOUND_BLIP );
			submenu_advance();
			break;
		case EVENT_SELECT:
			sound_play( SOUND_SELECT );
			submenu_do_filter();
			focus_set( FOCUS_GAMESEL );
			break;
		case EVENT_BACK:
			sound_play( SOUND_BACK );
			focus_set( FOCUS_MENU );
			break;
		default:
			break;
	}
	return 0;
}

void submenu_draw_platform( struct platform *p, GLfloat offset ) {
	if( p && p->texture ) {
		GLfloat width = p->texture->width;
		GLfloat height = p->texture->height;
	
		if( width > height ) {
			height = PLATFORM_SIZE * height/width;
			width = PLATFORM_SIZE;
		}
		else {
			width = PLATFORM_SIZE * width/height;
			height = PLATFORM_SIZE;
		}
	
		width *= ogl_xfactor();
		height *= ogl_xfactor();

		ogl_load_alterego();
		glTranslatef( offset, 0, -7 );
		glBindTexture( GL_TEXTURE_2D, p->texture->id );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-width,  height, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-width, -height, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( width, -height, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( width,  height, 0.0);
		glEnd();
	}
}

void submenu_draw( void ) {
	const struct config_submenu *config = &config_get()->iface.theme.submenu;
	
	if( visible && type != MENU_ALL ) {
		GLfloat xfactor = ogl_xfactor();
		GLfloat yfactor = ogl_yfactor();
		GLfloat width = (item_width/2)*xfactor;
		GLfloat height = (item_height/2)*xfactor;
		GLfloat tx = (((GLfloat)message->width*font_scale)/2) * xfactor;
		GLfloat ty = (((GLfloat)message->height*font_scale)/2) * xfactor;
		GLfloat scroll_offset = 0;
		
		if( tx > ((item_width*0.9)/2)*xfactor  ) {
			tx = ((item_width*0.9)/2)*xfactor;
		}
		if( ty > ((item_height*0.9)/2)*xfactor ) {
			ty = ((item_height*0.9)/2)*xfactor;
		}

		if( hide_direction ) {
			GLfloat zoom = 1;
			if( hide_direction == 1 )
				zoom = 1.0 - ((1.0 / steps) * (steps - step));
			else
				zoom = ((1.0 / steps) * (steps - step));

			width *= zoom;
			height *= zoom;
			tx *= zoom;
			ty *=zoom;
		
			if( step-- == 1 ) {
				/* Animation complete */
				if( hide_direction == -1 )
					visible = 1;
				else
					visible = 0;
				hide_direction = 0;
			}
		}
		
		if( scroll_direction ) {
			scroll_offset = ((SCROLL_HIDE_OFFSET/steps)*scroll_step) * (-scroll_direction);
			if( scroll_step-- == 1 ) {
				scroll_direction = 0;
			}
		}

		ogl_load_alterego();
		glColor4f( 1.0, 1.0, 1.0, 1.0 );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		
		if( focus_has() == FOCUS_SUBMENU && type == MENU_PLATFORM && platform ) {
			submenu_draw_platform( platform, scroll_offset );
			if( scroll_direction && scroll_step ) {
				if( scroll_direction > 0 ) {
					submenu_draw_platform( platform->prev, SCROLL_HIDE_OFFSET + scroll_offset );
				}
				else {
					submenu_draw_platform( platform->next, -SCROLL_HIDE_OFFSET + scroll_offset );
				}
			}
		}		
		
		ogl_load_alterego();
		glTranslatef( (menu_tile_selected()->x + config->offset2 ) * xfactor, (menu_tile_selected()->y + config->offset1) * yfactor, -6 );
		glColor4f( 1.0, 1.0, 1.0, 1.0 );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, texture->id );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-width,  height, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-width, -height, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( width, -height, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( width,  height, 0.0);
		glEnd();

		glBindTexture( GL_TEXTURE_2D, message->id );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-tx,  ty, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-tx, -ty, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( tx, -ty, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( tx,  ty, 0.0);
		glEnd();
	}
	
	if( focus_has() == FOCUS_SUBMENU ) {
		hint_draw_arrow( &arrow_retreat );
		hint_draw_arrow( &arrow_advance );
	}
}

int submenu_do_filter( void ) {
	int count = 0;
	switch( type ) {
		case( MENU_ALL ):
			count = game_list_unfilter();
			break;
		case( MENU_PLATFORM ):
			count = game_list_filter_platform( platform );
			break;
		case( MENU_CATEGORY ):
			count = game_list_filter_category( category->name, category_value->name );
			break;
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu\n" );
			break;
	}
	return count;
}

