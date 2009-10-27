#include "submenu.h"
#include "sdl_ogl.h"
#include "font.h"
#include "game.h"
#include "genre.h"
#include "platform.h"

static GLuint texture = 0;
static int type = -1;
static int selected = 0;
static int items = 0;
static const GLfloat item_width = 1.2;
static const GLfloat item_height = 0.3;
static const GLfloat font_scale = 0.0035;

struct genre *genre = NULL;
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
		case( MENU_GENRE ):
			message = font_message_create( genre->name );
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

int submenu_create( int menu_type ) {
	items = 0;
	
	if( submenu_load_texture() != 0 )
		return -1;

	type = menu_type;
	
	switch( menu_type ) {
		case( MENU_ALL ):
			break;
		case( MENU_GENRE ):
			if( genre == NULL )
				genre = genre_first();
			message = font_message_create( genre->name );
			items = genre_count();
			break;
		case( MENU_PLATFORM ):
			if( platform == NULL )
				platform = platform_first();
			message = font_message_create( platform->name );
			items = platform_count();
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
		case( MENU_GENRE ):
			genre = genre->next;
			message = font_message_create( genre->name );
			break;
		case( MENU_PLATFORM ):
			platform = platform->next;
			message = font_message_create( platform->name );
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
		case( MENU_GENRE ):
			genre = genre->prev;
			message = font_message_create( genre->name );
			break;
		case( MENU_PLATFORM ):
			platform = platform->prev;
			message = font_message_create( platform->name );
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
		GLfloat tx = ((GLfloat)message->width*font_scale)/2;
		GLfloat ty = ((GLfloat)message->height*font_scale)/2;
		if( tx > (item_width*0.9)/2 ) {
			tx = item_width*0.9/2;
		}
		if( ty > (item_height*0.9)/2 ) {
			ty = item_height*0.9/2;
		}
		
		glLoadIdentity();
		glTranslatef( (GLfloat)(type-1)*1.18, 0.9, -4 );
		glColor4f( 1.0, 1.0, 1.0, 1.0 );
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

		glBindTexture( GL_TEXTURE_2D, texture );
		glBegin( GL_QUADS );
			glTexCoord2f(0.0, 0.0); glVertex3f(-(item_width/2),  (item_height/2), 0.0);
			glTexCoord2f(0.0, 1.0); glVertex3f(-(item_width/2), -(item_height/2), 0.0);
			glTexCoord2f(1.0, 1.0); glVertex3f( (item_width/2), -(item_height/2), 0.0);
			glTexCoord2f(1.0, 0.0); glVertex3f( (item_width/2),  (item_height/2), 0.0);
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
		case( MENU_GENRE ):
			count = game_list_filter_genre( genre );
			break;
		case( MENU_PLATFORM ):
			count = game_list_filter_platform( platform );
			break;
		default:
			fprintf( stderr, "Error: Invalid menu type for sub menu\n" );
			break;
	}
	return count;
}

