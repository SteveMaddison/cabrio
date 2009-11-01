#include "submenu.h"
#include "sdl_ogl.h"
#include "font.h"
#include "game.h"
#include "platform.h"
#include "menu.h"

static GLuint texture = 0;
static int type = -1;
static int selected = 0;
static int items = 0;
static const GLfloat item_width = 1.2;
static const GLfloat item_height = 0.3;
static const GLfloat font_scale = 0.0035;
static const char *VALUE_UNKNOWN = "???";

struct category *prev_category = NULL;
struct category *category = NULL;
struct category_value *category_value = NULL;
struct platform *platform = NULL;
struct font_message *message = NULL;

int submenu_selected( void ) {
	return selected;
}

int submenu_items( void ) {
	return items;
}

int submenu_load_texture( void ) {
	int x,y;
	texture = sdl_create_texture( DATA_DIR "/pixmaps/submenu_item.png", &x, &y );
	if( texture == 0 ) {
		fprintf( stderr, "Warning: Couldn't create texture for submenu items\n" );
		return -1;
	}
	return 0;
}

void submenu_free_texture( void ) {
	if( texture )
		ogl_free_texture( &texture );
}

void submenu_pause( void ) {
	if( message ) {
		font_message_free( message );
		message = NULL;	
	}
	if( texture )
		submenu_free_texture();
}

int submenu_resume( void ) {
	if( submenu_load_texture() != 0 )
		return -1;

	switch( type ) {
		case( MENU_ALL ):
			break;
		case( MENU_CATEGORY ):
			if( category_value->name )
				message = font_message_create( category_value->name );
			else
				message = font_message_create( VALUE_UNKNOWN );
			break;
		case( MENU_PLATFORM ):
			message = font_message_create( platform->name );
			break;		
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu\n" );
			break;
	}
	return 0;
}

int submenu_create( struct menu_item *item ) {
	items = 0;
	
	if( submenu_load_texture() != 0 )
		return -1;

	type = item->type;
	
	switch( type ) {
		case( MENU_ALL ):
			break;
		case( MENU_PLATFORM ):
			if( platform == NULL )
				platform = platform_first();
			message = font_message_create( platform->name );
			items = platform_count();
			break;		
		case( MENU_CATEGORY ):
			category = item->category;
			if( category != prev_category )
				category_value = category->values;
			if( category_value->name )
				message = font_message_create( category_value->name );
			else
				message = font_message_create( VALUE_UNKNOWN );
			items = category->value_count;
			prev_category = category;
			break;
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu\n" );
			break;
	}
	
	return items;
}

void submenu_advance( void ) {
	if( message ) {
		font_message_free( message );
	}
	switch( type ) {
		case( MENU_ALL ):
			break;
		case( MENU_PLATFORM ):
			platform = platform->next;
			message = font_message_create( platform->name );
			break;		
		case( MENU_CATEGORY ):
			category_value = category_value->next;
			if( category_value->name )
				message = font_message_create( category_value->name );
			else
				message = font_message_create( VALUE_UNKNOWN );
			break;
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu\n" );
			break;
	}
}

void submenu_retreat( void ) {
	if( message ) {
		font_message_free( message );
		message = NULL;
	}
	switch( type ) {
		case( MENU_ALL ):
			break;
		case( MENU_PLATFORM ):
			platform = platform->prev;
			message = font_message_create( platform->name );
			break;		
		case( MENU_CATEGORY ):
			category_value = category_value->prev;
			if( category_value->name )
				message = font_message_create( category_value->name );
			else
				message = font_message_create( VALUE_UNKNOWN );
			break;
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu\n" );
			break;
	}
}

void submenu_free( void ) {
	if( message ) {
		font_message_free( message );
		message = NULL;
	}
	if( texture )
		ogl_free_texture( &texture );
}

void submenu_draw( void ) {
	if( type != MENU_ALL ) {
		GLfloat xfactor = ogl_xfactor();
		GLfloat yfactor = ogl_yfactor();
		GLfloat width = (item_width/2)*xfactor;
		GLfloat height = (item_height/2)*xfactor;
		GLfloat tx = (((GLfloat)message->width*font_scale)/2) * xfactor;
		GLfloat ty = (((GLfloat)message->height*font_scale)/2) * xfactor;
		GLfloat offset = (GLfloat)(menu_selected()->position) - (((GLfloat)menu_item_count()-1)/2);
		
		if( tx > ((item_width*0.9)/2)*xfactor  ) {
			tx = ((item_width*0.9)/2)*xfactor;
		}
		if( ty > ((item_height*0.9)/2)*xfactor ) {
			ty = ((item_height*0.9)/2)*xfactor;
		}
		
		ogl_load_alterego();
		
		glTranslatef( (GLfloat)(offset)*((3.6/menu_item_count())*xfactor), 0.9*yfactor, -4 );
		glColor4f( 1.0, 1.0, 1.0, 1.0 );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, texture );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-width,  height, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-width, -height, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( width, -height, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( width,  height, 0.0);
		glEnd();

		glBindTexture( GL_TEXTURE_2D, message->texture );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-tx,  ty, 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-tx, -ty, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( tx, -ty, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( tx,  ty, 0.0);
		glEnd();
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

