#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#ifdef __unix__
#include <pwd.h>
#endif
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "config.h"
#include "key.h"
#include "event.h"
#include "control.h"
#include "sound.h"

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

struct config config;

#ifdef __WIN32__
static const char *default_background		= "\\pixmaps\\default_background.jpg";
static const char *default_font				= "\\fonts\\FreeSans.ttf";
static const char *default_menu_texture		= "\\pixmaps\\menu_item.png";
static const char *default_back_texture		= "\\pixmaps\\button_blue.png";
static const char *default_select_texture	= "\\pixmaps\\button_red.png";
static const char *default_arrow_texture	= "\\pixmaps\\arrow.png";
static const char *default_sounds[] = {
	"\\sounds\\back.wav",
	"\\sounds\\blip.wav",
	"\\sounds\\no.wav",
	"\\sounds\\select.wav"
};
#else
static const char *default_dir 				= ".cabrio"; /* Relative to user's home */
static const char *default_background		= "/pixmaps/default_background.jpg";
static const char *default_font 			= "/fonts/FreeSans.ttf";
static const char *default_menu_texture		= "/pixmaps/menu_item.png";
static const char *default_back_texture		= "/pixmaps/button_blue.png";
static const char *default_select_texture	= "/pixmaps/button_red.png";
static const char *default_arrow_texture	= "/pixmaps/arrow.png";
static const char *default_sounds[] = {
	"/sounds/back.wav",
	"/sounds/blip.wav",
	"/sounds/no.wav",
	"/sounds/select.wav"
};
#endif

/* Game selector defaults, listed in reverse order */
static const int default_num_tiles = 13;
static const float default_tile_pos[][3] = {
	{ 3.6, -3.5, -3.0 },
	{ 3.0, -3.0, -2.5 },
	{ 2.4, -2.5, -2.0 },
	{ 1.8, -2.0, -1.5 },
	{ 1.2, -1.5, -1.0 },
	{ 0.6, -1.0, -0.5 },
	{ 0.0,  0.0,  1.5 },
	{ 0.6,  1.0, -0.5 },
	{ 1.2,  1.5, -1.0 },
	{ 1.8,  2.0, -1.5 },
	{ 2.4,  2.5, -2.0 },
	{ 3.0,  3.0, -2.5 },
	{ 3.6,  3.5, -3.0 }
};
static const float default_tile_angle[][3] = {
	{ 0.0, 0.0,  54.0 },
	{ 0.0, 0.0,  45.0 },
	{ 0.0, 0.0,  36.0 },
	{ 0.0, 0.0,  27.0 },
	{ 0.0, 0.0,  18.0 },
	{ 0.0, 0.0,   9.0 },
	{ 0.0, 0.0,   0.0 },
	{ 0.0, 0.0,  -9.0 },
	{ 0.0, 0.0, -18.0 },
	{ 0.0, 0.0, -27.0 },
	{ 0.0, 0.0, -36.0 },
	{ 0.0, 0.0, -46.0 },
	{ 0.0, 0.0, -55.0 }
};
static const int default_tile_transparency[] = {
	100, 60, 20, 10, 0, 0, 0, 0, 0, 10, 20, 60, 100
};

static struct config_theme default_theme;
static const char *default_theme_name 	= "default";
static const char *default_file 		= "config.xml";

static char config_directory[CONFIG_FILE_NAME_LENGTH] 	= "";
static char config_filename[CONFIG_FILE_NAME_LENGTH] 	= "";

/* Specific XML tags */
static const char *tag_root							= "cabrio-config";
static const char *tag_emulators					= "emulators";
static const char *tag_emulator						= 	"emulator";
static const char *tag_emulator_executable			= 		"executable";
static const char *tag_games						= "games";
static const char *tag_game							=   "game";
static const char *tag_game_rom_image				=     "rom-image";
static const char *tag_game_logo_image				=     "logo-image";
static const char *tag_game_background_image		=     "background-image";
static const char *tag_game_categories				=     "categories";
static const char *tag_game_category				=     "category";
static const char *tag_game_screen_shot				=     "screen-shot";
static const char *tag_iface						= "interface";
static const char *tag_iface_full_screen			= 	"full-screen";
static const char *tag_iface_screen					=   "screen";
static const char *tag_iface_screen_hflip			=     "flip-horizontal";
static const char *tag_iface_screen_vflip			=     "flip-vertical";
static const char *tag_iface_controls				=   "controls";
static const char *tag_iface_frame_rate				=   "frame-rate";
static const char *tag_iface_gfx					=   "graphics";
static const char *tag_iface_gfx_quality			=     "quality";
static const char *tag_iface_gfx_max_width			=     "max-image-width";
static const char *tag_iface_gfx_max_height			=     "max-image-height";
static const char *tag_iface_theme					=     "theme";
static const char *tag_themes						=   "themes";
static const char *tag_themes_theme					=     "theme";
static const char *tag_theme_menu					=   "menu";
static const char *tag_theme_menu_item_width		=     "item-width";
static const char *tag_theme_menu_item_height		=     "item-height";
static const char *tag_theme_menu_items_visible		=     "items-visible";
static const char *tag_theme_menu_spacing			=     "spacing";
static const char *tag_theme_background				=   "background";
static const char *tag_theme_font					=   "font";
static const char *tag_theme_font_file				=     "font-file";
static const char *tag_theme_sounds					=	"sounds";
static const char *tag_theme_sounds_sound			=	  "sound";
static const char *tag_theme_sounds_sound_file		=	  "sound-file";
static const char *tag_theme_screenshot				=	"screen-shot";
static const char *tag_theme_screenshot_fix_ar		=	  "fix-aspect-ratio";
static const char *tag_theme_hints					=	"hints";
static const char *tag_theme_hints_pulse			=     "pulse";
static const char *tag_theme_hints_label_back		=     "back-label";
static const char *tag_theme_hints_label_select		=     "select-label";
static const char *tag_theme_hints_image_back		=     "back-image";
static const char *tag_theme_hints_image_select		=     "select-image";
static const char *tag_theme_hints_image_arrow		=     "arrow-image";
static const char *tag_theme_game_sel				=   "game-selector";
static const char *tag_theme_game_sel_selected		=     "selected";
static const char *tag_theme_game_sel_tile_size		=     "tile-size";
static const char *tag_theme_game_sel_tiles			=     "tiles";
static const char *tag_theme_game_sel_tiles_tile	=       "tile";

/* General (reused) XML tags */
static const char *tag_name				= "name";
static const char *tag_value			= "value";
static const char *tag_id				= "id";
static const char *tag_display_name		= "display-name";
static const char *tag_platform			= "platform";
static const char *tag_params			= "params";
static const char *tag_param			= "param";
static const char *tag_control			= "control";
static const char *tag_event			= "event";
static const char *tag_device			= "device";
static const char *tag_type				= "type";
static const char *tag_width			= "width";
static const char *tag_height			= "height";
static const char *tag_transparency		= "transparency";
static const char *tag_zoom				= "zoom";
static const char *tag_rotation			= "rotation";
static const char *tag_image_file		= "image-file";
static const char *tag_size				= "size";
static const char *tag_x_size			= "x-size";
static const char *tag_y_size			= "y-size";
static const char *tag_font_scale		= "font-scale";
static const char *tag_orientation		= "orientation";
static const char *tag_offset1			= "primary-offset";
static const char *tag_offset2			= "secondary-offset";
static const char *tag_auto_hide		= "auto-hide";
static const char *tag_angle_x			= "x-angle";
static const char *tag_angle_y			= "y-angle";
static const char *tag_angle_z			= "z-angle";
static const char *tag_position_x		= "x-position";
static const char *tag_position_y		= "y-position";
static const char *tag_position_z		= "z-position";
static const char *tag_order			= "order";

/* Common values */
static const char *config_empty			= "";
static const char *config_true			= "true";
static const char *config_false			= "false";
static const char *config_yes			= "yes";
static const char *config_no			= "no";
static const char *config_low			= "low";
static const char *config_medium		= "medium";
static const char *config_high			= "high";
static const char *config_portrait		= "portrait";
static const char *config_landscape		= "landscape";
static const char *config_auto			= "auto";

static const char *warn_alloc = "Warning: Couldn't allocate memory for '%s' object\n";
static const char *warn_skip = "Warning: Skipping unrecognised XML element in '%s': '%s'\n";

static char scratch[32] = "";

const struct config *config_get( void ) {
	return (const struct config *)&config;
}

int config_read_boolean( char *name, char *value, int *target ) {
	if( strcasecmp( value, config_yes ) == 0 ||  strcasecmp( value, config_true ) == 0 ) {
		*target = 1;
		return 0;
	}
	else if( strcasecmp( value, config_no ) == 0 ||  strcasecmp( value, config_false ) == 0 ) {
		*target = 0;
		return 0;
	}
	return -1;
}

/* Low, medium, high */
int config_read_lmh( char *name, char *value, int *target ) {
	if( strcasecmp( value, config_low ) == 0 ) {
		*target = CONFIG_LOW;
		return 0;
	}
	else if( strcasecmp( value, config_medium ) == 0 ) {
		*target = CONFIG_MEDIUM;
		return 0;
	}
	else if( strcasecmp( value, config_high ) == 0 ) {
		*target = CONFIG_HIGH;
		return 0;
	}
	return -1;
}

int config_read_orientation( char *name, char *value, int *target ) {
	if( strcasecmp( value, config_portrait ) == 0 ) {
		*target = CONFIG_PORTRAIT;
		return 0;
	}
	else if( strcasecmp( value, config_landscape ) == 0 ) {
		*target = CONFIG_LANDSCAPE;
		return 0;
	}
	return -1;
}

int config_read_integer( char *name, char *value, int *target ) {
	char *pos = value;
	if( pos ) {
		while( *pos ) {
			if( (*pos < '0' || *pos > '9') && (*pos != '-') ) {
				fprintf( stderr, "Warning: Element %s requires numeric (integer) value\n", name );
				return -1;
			}
			pos++;
		}
		*target = strtol( value, NULL, 10 );
	}
	return 0;
}

int config_read_float( char *name, char *value, float *target ) {
	char *pos = value;
	if( pos ) {
		while( *pos ) {
			if( (*pos < '0' || *pos > '9') && (*pos != '-') && (*pos != '.') ) {
				fprintf( stderr, "Warning: Element %s requires numeric (floating point) value\n", name );
				return -1;
			}
			pos++;
		}
		*target = atof( value );
	}
	return 0;
}

int config_read_percentage( char *name, char *value, int *target ) {
	char *pos = value;
	if( pos ) {
		while( *pos ) {
			if( (*pos < '0' || *pos > '9') && (*pos != '-') && (*pos != '%') ) {
				fprintf( stderr, "Warning: Element %s requires numeric (percentage) value\n", name );
				return -1;
			}
			pos++;
		}
		*target = strtol( value, NULL, 10 );
		if( *target < 0 || *target > 100 ) {
			*target = 50;
			fprintf( stderr, "Warning: Element %s percentage out of range, using 50%%\n", name );
		}
	}
	return 0;
}

struct config_platform *config_platform( const char *name ) {
	struct config_platform *p = config.platforms;

	if( name && *name ) {
		while( p ) {
			if( strncasecmp( name, p->name, CONFIG_NAME_LENGTH ) == 0 )
				break;
			p = p->next;
		}
		if( p == NULL ) {
			/* add new */
			p = malloc( sizeof(struct config_platform) );
			if( p == NULL ) {
				fprintf( stderr, "Error: Couldn't create new platform configuration object" );
			}
			else {
				memset( p, 0, sizeof(struct config_platform) );
				strncpy( p->name, name, CONFIG_NAME_LENGTH );
				p->next = config.platforms;
				config.platforms = p;
			}
		}
	}
	else {
		p = NULL;
	}

	return p;
}

int config_read_param( xmlNode *node, struct config_param *param ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_name ) == 0 ) {
				strncpy( param->name, (char*)xmlNodeGetContent(node), CONFIG_PARAM_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_value ) == 0 ) {
				strncpy( param->value, (char*)xmlNodeGetContent(node), CONFIG_PARAM_LENGTH );
			}
			else {
				fprintf( stderr, warn_skip, tag_param, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_emulator_params( xmlNode *node, struct config_emulator *emulator ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_param ) == 0 ) {
				struct config_param *param = malloc( sizeof(struct config_param ) );
				if( param ) {
					memset( param, 0, sizeof(struct config_param ) );
					config_read_param( node->children, param );
					param->next = emulator->params;
					emulator->params = param;
				}
				else {
					fprintf( stderr, warn_alloc, tag_param );
					return -1;
				}				
			}
			else {
				fprintf( stderr, warn_skip, tag_params, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_emulator( xmlNode *node, struct config_emulator *emulator ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_name ) == 0 ) {
				strncpy( emulator->name, (char*)xmlNodeGetContent(node), CONFIG_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_display_name ) == 0 ) {
				strncpy( emulator->display_name, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_emulator_executable ) == 0 ) {
				strncpy( emulator->executable, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_params ) == 0 ) {
				config_read_emulator_params( node->children, emulator );
			}
			else {
				fprintf( stderr, warn_skip, tag_emulator, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_emulators( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_emulator ) == 0 ) {
				struct config_emulator *emulator = malloc( sizeof(struct config_emulator ) );
				if( emulator ) {
					memset( emulator, 0, sizeof(struct config_emulator ) );
					config_read_emulator( node->children, emulator );
					emulator->next = config.emulators;
					config.emulators = emulator;
				}
				else {
					fprintf( stderr, warn_alloc, tag_emulator );
					return -1;
				}
			}
			else {
				fprintf( stderr, warn_skip, tag_emulators, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

struct config_category *config_category( const char *name ) {
	struct config_category *c = config.categories;

	if( name && *name ) {
		while( c ) {
			if( strncasecmp( name, c->name, CONFIG_NAME_LENGTH ) == 0 )
				break;
			c = c->next;
		}
		if( c == NULL ) {
			/* add new */
			c = malloc( sizeof(struct config_category) );
			if( c == NULL ) {
				fprintf( stderr, warn_alloc, tag_game_category );
			}
			else {
				memset( c, 0, sizeof(struct config_category) );
				strncpy( c->name, name, CONFIG_NAME_LENGTH );
				c->next = config.categories;
				config.categories = c;
			}
		}
	}
	else {
		c = NULL;
	}
	return c;
}

struct config_category_value *config_category_value( struct config_category *category, const char *name ) {
	if( category ) {
		struct config_category_value *v = category->values;

		if( name && *name ) {
			while( v ) {
				if( strncasecmp( name, v->name, CONFIG_NAME_LENGTH ) == 0 )
					break;
				v = v->next;
			}
			if( v == NULL ) {
				/* add new */
				v = malloc( sizeof(struct config_category_value) );
				if( v == NULL ) {
					fprintf( stderr, warn_alloc, "category value" );
				}
				else {
					memset( v, 0, sizeof(struct config_category_value) );
					strncpy( v->name, name, CONFIG_NAME_LENGTH );
					v->next = category->values;
					category->values = v;
				}
			}
		}
		return v;
	}
	else {
		fprintf( stderr, "Warning: Attempt to add value to null category\n" );
	}

	return NULL;
}

int config_read_game_category( xmlNode *node, struct config_game_category *gc ) {
	xmlNode *tmp = node;
	
	/* Make sure we have the name first */
	if( node ) {
		while( node ) {
			if( node->type == XML_ELEMENT_NODE ) {
				if( strcmp( (char*)node->name, tag_name ) == 0 ) {
					gc->category = config_category( (char*)xmlNodeGetContent(node) );
				}
			}
			node = node->next;
		}
	}
	/* Now check for (and add) values */
	gc->value = NULL;
	if( gc->category ) {
		node = tmp;
		while( node ) {
			if( node->type == XML_ELEMENT_NODE ) {
				if( strcmp( (char*)node->name, tag_value ) == 0 ) {
					if( xmlNodeGetContent(node)[0] ) { 
						gc->value = config_category_value( gc->category, (char*)xmlNodeGetContent(node) );
					}
				}
				else if( strcmp( (char*)node->name, tag_name ) == 0 ) {
					/* Already got it... */
				}
				else {
					fprintf( stderr, warn_skip, tag_game_category, node->name );
				}
			}
			node = node->next;
		}
		if( gc->value == NULL ) {
			fprintf( stderr, "Warning: Category '%s' specified without value(s)\n", gc->category->name );
			return -1;		
		}
	}
	else {
		fprintf( stderr, "Warning: game category has no '%s' element\n", tag_name );
		return -1;
	}
	
	return 0;
}

int config_read_game_categories( xmlNode *node, struct config_game *game ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_game_category ) == 0 ) {
				struct config_game_category *gc = malloc( sizeof(struct config_game_category) );
				if( gc ) {
					memset( gc, 0, sizeof(struct config_game_category) );
					if( config_read_game_category( node->children, gc ) == 0 ) {
						gc->next = game->categories;
						game->categories = gc;
					}
					else {
						free( gc );
						gc = NULL;
					}
				}
				else {
					fprintf( stderr, warn_alloc, tag_game_category );
					return -1;					
				}
			}
			else {
				fprintf( stderr, warn_skip, tag_params, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_game_params( xmlNode *node, struct config_game *game ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_param ) == 0 ) {
				struct config_param *param = malloc( sizeof(struct config_param ) );
				if( param ) {
					memset( param, 0, sizeof(struct config_param ) );
					config_read_param( node->children, param );
					param->next = game->params;
					game->params = param;
				}
				else {
					fprintf( stderr, warn_alloc, tag_param );
					return -1;
				}					
			}
			else {
				fprintf( stderr, warn_skip, tag_params, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_game( xmlNode *node, struct config_game *game ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_name ) == 0 ) {
				strncpy( game->name, (char*)xmlNodeGetContent(node), CONFIG_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_game_rom_image ) == 0 ) {
				strncpy( game->rom_image, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_game_logo_image ) == 0 ) {
				strncpy( game->logo_image, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_game_background_image ) == 0 ) {
				strncpy( game->background_image, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_game_screen_shot ) == 0 ) {
				strncpy( game->screen_shot, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_game_categories ) == 0 ) {
				config_read_game_categories( node->children, game );
			}
			else if( strcmp( (char*)node->name, tag_platform ) == 0 ) {
				game->platform = config_platform( (char*)xmlNodeGetContent(node) );
			}
			else if( strcmp( (char*)node->name, tag_params ) == 0 ) {
				config_read_game_params( node->children, game );
			}
			else {
				fprintf( stderr, warn_skip, tag_game, node->name );	
			}
		}
		node = node->next;
	}
	
/*{
	struct config_game_category *gc = game->categories;
	printf( "Game: %s\n", game->name );
	while( gc ) {
		if( !gc->category ) {
			printf("No category\n");
			return -1;
		}
		if( !gc->value ) {
			printf("No value\n");
			return -1;
		}		
		printf( "  '%s' = '%s'\n", gc->category->name, gc->value->name );
		gc = gc->next;
	}
}*/

	return 0;
}

int config_read_games( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_game ) == 0 ) {
				struct config_game *game = malloc( sizeof(struct config_game ) );
				if( game ) {
					memset( game, 0, sizeof(struct config_game ) );
					config_read_game( node->children, game );
					game->next = config.games;
					config.games = game;
				}
				else {
					fprintf( stderr, warn_alloc, tag_game );
					return -1;
				}
			}
			else {
				fprintf( stderr, warn_skip, tag_games, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_device( xmlNode *node, struct config_control *control ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_type ) == 0 ) {
				control->device_type = device_id( (char*)xmlNodeGetContent(node) );
			}
			else if( strcmp( (char*)node->name, tag_id ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &control->device_id );
			}
			else {
				fprintf( stderr, warn_skip, tag_device, node->name );	
			}
		}
		node = node->next;
	}

	return 0;
}

int config_read_control( xmlNode *node, struct config_control *control ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_type ) == 0 ) {
				control->control_type = control_id( (char*)xmlNodeGetContent(node) );
			}
			else if( strcmp( (char*)node->name, tag_id ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &control->control_id );
			}
			else {
				fprintf( stderr, warn_skip, tag_control, node->name );	
			}
		}
		node = node->next;
	}

	return 0;
}

int config_read_event( xmlNode *node ) {
	struct config_control tmp;
	char *value = NULL;
	int event;
	
	memset( &tmp, 0, sizeof(struct config_control) );
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_name ) == 0 ) {
				if(!( event = event_id( (char*)xmlNodeGetContent(node) ) )) {
					return -1;
				}
			}
			else if( strcmp( (char*)node->name, tag_device ) == 0 ) {
				if( config_read_device( node->children, &tmp ) != 0 ) {
					return -1;
				}
			}
			else if( strcmp( (char*)node->name, tag_control ) == 0 ) {
				if( config_read_control( node->children, &tmp ) != 0 ) {
					return -1;
				}
			}
			else if( strcmp( (char*)node->name, tag_value ) == 0 ) {
				value = (char*)xmlNodeGetContent(node);
			}
			else {
				fprintf( stderr, warn_skip, tag_event, node->name );	
			}
		}
		node = node->next;
	}
	
	/* Decode string values if necessary */
	switch( tmp.device_type ) {
		case DEV_KEYBOARD:
			tmp.value = key_id( value );
			if( tmp.value == 0 ) {
				fprintf( stderr, "Warning: Unknown key name '%s'\n", value );
				return -1;
			}
			break;
		case DEV_JOYSTICK:
			switch( tmp.control_type ) {
				case CTRL_AXIS:
					tmp.value = axis_dir_value( value );
					break;
				case CTRL_HAT:
					tmp.value = direction_id( value );
					break;
				case CTRL_BALL:
					tmp.value = axis_dir_value( value );
					break;
				case CTRL_BUTTON:
					config_read_integer( (char*)tag_id, value, &tmp.value );
					break;
				default:
					break;
			}
			break;
		case DEV_MOUSE:
			if( tmp.control_type == CTRL_AXIS ) {
				tmp.value = axis_dir_value( value );
			}
			else {
				config_read_integer( (char*)tag_id, value, &tmp.value );
			}
			break;
		default:
			config_read_integer( (char*)tag_id, value, &tmp.value );
			break;
	}
	
	/* Replace exisiting control for this event */
	memcpy( &config.iface.controls[event], &tmp, sizeof(struct config_control) );
	
	return 0;	
}

int config_read_controls( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_event ) == 0 ) {
				config_read_event( node->children );
			}
			else {
				fprintf( stderr, warn_skip, tag_iface_controls, node->name );	
			}
		}
		node = node->next;
	}
	return 0;	
}

int config_read_menu( xmlNode *node, struct config_menu *menu ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_image_file ) == 0 ) {
				strncpy( menu->texture, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_theme_menu_item_width ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->item_width );
			}
			else if( strcmp( (char*)node->name, tag_theme_menu_item_height ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->item_height );
			}
			else if( strcmp( (char*)node->name, tag_font_scale ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->font_scale );
			}
			else if( strcmp( (char*)node->name, tag_zoom ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->zoom );
			}
			else if( strcmp( (char*)node->name, tag_transparency ) == 0 ) {
				config_read_percentage( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->transparency );
			}
			else if( strcmp( (char*)node->name, tag_offset1 ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->offset1 );
			}
			else if( strcmp( (char*)node->name, tag_offset2 ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->offset2 );
			}
			else if( strcmp( (char*)node->name, tag_theme_menu_items_visible ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->max_visible );
			}
			else if( strcmp( (char*)node->name, tag_theme_menu_spacing ) == 0 ) {
				if( strcasecmp( (char*)xmlNodeGetContent(node), config_auto ) == 0 )
					menu->spacing = -1;
				else
					config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->spacing );
			}
			else if( strcmp( (char*)node->name, tag_orientation ) == 0 ) {
				config_read_orientation( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->orientation );
			}
			else if( strcmp( (char*)node->name, tag_auto_hide ) == 0 ) {
				config_read_boolean( (char*)node->name, (char*)xmlNodeGetContent(node), &menu->auto_hide );
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_menu, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_screenshot( xmlNode *node, struct config_screenshot *screenshot ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_offset1 ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &screenshot->offset1 );
			}
			else if( strcmp( (char*)node->name, tag_offset2 ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &screenshot->offset2 );
			}
			else if( strcmp( (char*)node->name, tag_angle_x ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &screenshot->angle_x );
			}
			else if( strcmp( (char*)node->name, tag_angle_y ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &screenshot->angle_y );
			}
			else if( strcmp( (char*)node->name, tag_angle_z ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &screenshot->angle_z );
			}
			else if( strcmp( (char*)node->name, tag_size ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &screenshot->size );
			}
			else if( strcmp( (char*)node->name, tag_theme_screenshot_fix_ar ) == 0 ) {
				config_read_boolean( (char*)node->name, (char*)xmlNodeGetContent(node), &screenshot->fix_aspect_ratio );
			}
			else if( strcmp( (char*)node->name, tag_auto_hide ) == 0 ) {
				config_read_boolean( (char*)node->name, (char*)xmlNodeGetContent(node), &screenshot->auto_hide );
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_screenshot, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_hints( xmlNode *node, struct config_hints *hints ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_offset1 ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &hints->offset1 );
			}
			else if( strcmp( (char*)node->name, tag_offset2 ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &hints->offset2 );
			}
			else if( strcmp( (char*)node->name, tag_size ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &hints->size );
			}
			else if( strcmp( (char*)node->name, tag_theme_hints_pulse ) == 0 ) {
				config_read_boolean( (char*)node->name, (char*)xmlNodeGetContent(node), &hints->pulse );
			}
			else if( strcmp( (char*)node->name, tag_theme_hints_label_back ) == 0 ) {
				strncpy( hints->label_back, (char*)xmlNodeGetContent(node), CONFIG_LABEL_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_theme_hints_label_select ) == 0 ) {
				strncpy( hints->label_select, (char*)xmlNodeGetContent(node), CONFIG_LABEL_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_theme_hints_image_back ) == 0 ) {
				strncpy( hints->image_back, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_theme_hints_image_select ) == 0 ) {
				strncpy( hints->image_select, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_theme_hints_image_arrow ) == 0 ) {
				strncpy( hints->image_arrow, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_hints, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_game_selector_tile( xmlNode *node, struct config_game_sel_tile *tile ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_order ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &tile->order );
			}
			else if( strcmp( (char*)node->name, tag_angle_x ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &tile->angle[0] );
			}
			else if( strcmp( (char*)node->name, tag_angle_y ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &tile->angle[1] );
			}
			else if( strcmp( (char*)node->name, tag_angle_z ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &tile->angle[2] );
			}
			else if( strcmp( (char*)node->name, tag_position_x ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &tile->pos[0] );
			}
			else if( strcmp( (char*)node->name, tag_position_y ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &tile->pos[1] );
			}
			else if( strcmp( (char*)node->name, tag_position_z ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &tile->pos[2] );
			}
			else if( strcmp( (char*)node->name, tag_transparency ) == 0 ) {
				config_read_percentage( (char*)node->name, (char*)xmlNodeGetContent(node), &tile->transparency );
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_game_sel_tiles_tile, node->name );
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_game_selector_tiles( xmlNode *node, struct config_game_sel *game_sel ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_theme_game_sel_tiles_tile ) == 0 ) {
				struct config_game_sel_tile *tile = malloc( sizeof(struct config_game_sel_tile) );
				if( tile ) {
					memset( tile, 0, sizeof(struct config_game_sel_tile) );
					config_read_game_selector_tile( node->children, tile );
					tile->next = game_sel->tiles;
					game_sel->tiles = tile;
				}
				else {
					fprintf( stderr, warn_alloc, tag_theme_game_sel_tiles_tile );
					return -1;
				}
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_game_sel_tiles, node->name );	
			}
		}
		node = node->next;
	}
	return 0;	
}

int config_read_game_selector( xmlNode *node, struct config_game_sel *game_sel ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_offset1 ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &game_sel->offset1 );
			}
			else if( strcmp( (char*)node->name, tag_offset2 ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &game_sel->offset2 );
			}
			else if( strcmp( (char*)node->name, tag_x_size ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &game_sel->size_x );
			}
			else if( strcmp( (char*)node->name, tag_y_size ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &game_sel->size_y );
			}
			else if( strcmp( (char*)node->name, tag_theme_game_sel_tile_size ) == 0 ) {
				config_read_float( (char*)node->name, (char*)xmlNodeGetContent(node), &game_sel->tile_size );
			}
			else if( strcmp( (char*)node->name, tag_theme_game_sel_selected ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &game_sel->selected );
			}
			else if( strcmp( (char*)node->name, tag_orientation ) == 0 ) {
				config_read_orientation( (char*)node->name, (char*)xmlNodeGetContent(node), &game_sel->orientation );
			}
			else if( strcmp( (char*)node->name, tag_theme_game_sel_tiles ) == 0 ) {
				game_sel->tiles = NULL;
				config_read_game_selector_tiles( node->children, game_sel );
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_game_sel, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_graphics( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_iface_gfx_quality ) == 0 ) {
				config_read_lmh( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.gfx_quality );
			}
			else if( strcmp( (char*)node->name, tag_iface_gfx_max_width ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.gfx_max_width );
			}
			else if( strcmp( (char*)node->name, tag_iface_gfx_max_height ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.gfx_max_height );
			}
			else {
				fprintf( stderr, warn_skip, tag_iface_gfx, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_font( xmlNode *node, struct config_theme *theme ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_theme_font_file ) == 0 ) {
				strncpy( theme->font_file, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_size ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &theme->font_size );
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_font, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_sound( xmlNode *node, struct config_theme *theme ) {
	xmlNode *tmp = node;
	char *name = NULL;
	int id = -1;
	
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_name ) == 0 ) {
				name = (char*)xmlNodeGetContent(node);
				break;
			}
		}
		node = node->next;
	}
	id = sound_id( name );
	if( id >= 0 ) {
		node = tmp;
		while( node ) {
			if( node->type == XML_ELEMENT_NODE ) {
				if( strcmp( (char*)node->name, tag_name ) == 0 ) {
					/* Already got it */
				}
				else if( strcmp( (char*)node->name, tag_theme_sounds_sound_file ) == 0 ) {
					strncpy( theme->sounds[id], (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
				}
				else {
					fprintf( stderr, warn_skip, tag_theme_sounds_sound, node->name );	
				}
			}
			node = node->next;
		}
	}
	else {
		fprintf( stderr, "Warning: Unrecogised sound name\n" );
	}		
		
	return 0;
}

int config_read_sounds( xmlNode *node, struct config_theme *theme ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_theme_sounds_sound ) == 0 ) {
				config_read_sound( node->children, theme );
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_sounds, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_theme_background( xmlNode *node, struct config_theme *theme ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_image_file ) == 0 ) {
				strncpy( theme->background_image, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_rotation ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &theme->background_rotation );
			}
			else if( strcmp( (char*)node->name, tag_transparency ) == 0 ) {
				config_read_percentage( (char*)node->name, (char*)xmlNodeGetContent(node), &theme->background_transparency );
			}
			else {
				fprintf( stderr, warn_skip, tag_theme_background, node->name );
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_interface_screen( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_width ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.screen_width );
			}
			else if( strcmp( (char*)node->name, tag_height ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.screen_height );
			}
			else if( strcmp( (char*)node->name, tag_rotation ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.screen_rotation );
			}
			else if( strcmp( (char*)node->name, tag_iface_screen_hflip ) == 0 ) {
				config_read_boolean( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.screen_hflip );
			}
			else if( strcmp( (char*)node->name, tag_iface_screen_vflip ) == 0 ) {
				config_read_boolean( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.screen_vflip );
			}
			else {
				fprintf( stderr, warn_skip, tag_iface_screen, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_interface( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_iface_full_screen ) == 0 ) {
				config_read_boolean( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.full_screen );
			}
			else if( strcmp( (char*)node->name, tag_iface_screen ) == 0 ) {
				config_read_interface_screen( node->children );
			}
			else if( strcmp( (char*)node->name, tag_iface_frame_rate ) == 0 ) {
				config_read_integer( (char*)node->name, (char*)xmlNodeGetContent(node), &config.iface.frame_rate );
			}
			else if( strcmp( (char*)node->name, tag_iface_controls ) == 0 ) {
				config_read_controls( node->children );
			}
			else if( strcmp( (char*)node->name, tag_iface_gfx ) == 0 ) {
				config_read_graphics( node->children );
			}
			else if( strcmp( (char*)node->name, tag_iface_theme ) == 0 ) {
				strncpy( config.iface.theme_name, (char*)xmlNodeGetContent(node), CONFIG_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_name ) == 0 ) {
				/* Ignore (for now) */
			}
			else if( strcmp( (char*)node->name, tag_theme_background ) == 0 ) {
				/* Ignore (for now) */
			}
			else if( strcmp( (char*)node->name, tag_theme_font ) == 0 ) {
				/* Ignore (for now) */
			}
			else if( strcmp( (char*)node->name, tag_theme_menu ) == 0 ) {
				/* Ignore (for now) */
			}
			else if( strcmp( (char*)node->name, tag_theme_sounds ) == 0 ) {
				/* Ignore (for now) */
			}
			else if( strcmp( (char*)node->name, tag_theme_screenshot ) == 0 ) {
				/* Ignore (for now) */
			}
			else if( strcmp( (char*)node->name, tag_theme_hints ) == 0 ) {
				/* Ignore (for now) */
			}
			else if( strcmp( (char*)node->name, tag_theme_game_sel ) == 0 ) {
				/* Ignore (for now) */
			}
			else {
				fprintf( stderr, warn_skip, tag_iface, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_theme( xmlNode *node, struct config_theme *theme ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_name ) == 0 ) {
				strncpy( theme->name, (char*)xmlNodeGetContent(node), CONFIG_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_theme_background ) == 0 ) {
				config_read_theme_background( node->children, theme );
			}
			else if( strcmp( (char*)node->name, tag_theme_font ) == 0 ) {
				config_read_font( node->children, theme );
			}
			else if( strcmp( (char*)node->name, tag_theme_menu ) == 0 ) {
				config_read_menu( node->children, &theme->menu );
			}
			else if( strcmp( (char*)node->name, tag_theme_sounds ) == 0 ) {
				config_read_sounds( node->children, theme );
			}
			else if( strcmp( (char*)node->name, tag_theme_screenshot ) == 0 ) {
				config_read_screenshot( node->children, &theme->screenshot );
			}
			else if( strcmp( (char*)node->name, tag_theme_hints ) == 0 ) {
				config_read_hints( node->children, &theme->hints );
			}
			else if( strcmp( (char*)node->name, tag_theme_game_sel ) == 0 ) {
				config_read_game_selector( node->children, &theme->game_sel );
			}
			else {
				fprintf( stderr, warn_skip, tag_themes_theme, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_interface_theme( xmlNode *node, struct config_theme *theme ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_name ) == 0 ) {
				strncpy( theme->name, (char*)xmlNodeGetContent(node), CONFIG_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, tag_theme_background ) == 0 ) {
				config_read_theme_background( node->children, theme );
			}
			else if( strcmp( (char*)node->name, tag_theme_font ) == 0 ) {
				config_read_font( node->children, theme );
			}
			else if( strcmp( (char*)node->name, tag_theme_menu ) == 0 ) {
				config_read_menu( node->children, &theme->menu );
			}
			else if( strcmp( (char*)node->name, tag_theme_sounds ) == 0 ) {
				config_read_sounds( node->children, theme );
			}
			else if( strcmp( (char*)node->name, tag_theme_screenshot ) == 0 ) {
				config_read_screenshot( node->children, &theme->screenshot );
			}
			else if( strcmp( (char*)node->name, tag_theme_hints ) == 0 ) {
				config_read_hints( node->children, &theme->hints );
			}
			else if( strcmp( (char*)node->name, tag_theme_game_sel ) == 0 ) {
				config_read_game_selector( node->children, &theme->game_sel );
			}
			else if( strcmp( (char*)node->name, tag_iface_full_screen ) == 0 ) {
				/* Ignore */
			}
			else if( strcmp( (char*)node->name, tag_iface_screen ) == 0 ) {
				/* Ignore */
			}
			else if( strcmp( (char*)node->name, tag_iface_frame_rate ) == 0 ) {
				/* Ignore */
			}
			else if( strcmp( (char*)node->name, tag_iface_controls ) == 0 ) {
				/* Ignore */
			}
			else if( strcmp( (char*)node->name, tag_iface_gfx ) == 0 ) {
				/* Ignore */
			}
			else if( strcmp( (char*)node->name, tag_iface_theme ) == 0 ) {
				/* Ignore */
			}
			else {
				fprintf( stderr, warn_skip, tag_iface, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_themes( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_themes_theme ) == 0 ) {
				struct config_theme *theme = malloc( sizeof(struct config_theme ) );
				if( theme ) {
					memcpy( theme, &default_theme, sizeof(struct config_theme ) );
					config_read_theme( node->children, theme );
					theme->next = config.themes;
					config.themes = theme;
				}
				else {
					fprintf( stderr, warn_alloc, tag_themes_theme );
					return -1;
				}
			}
			else {
				fprintf( stderr, warn_skip, tag_themes, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read( xmlNode *root ) {
	xmlNode *node = root;
	
	if( strcmp( (char*)node->name, tag_root ) != 0 ) {
		fprintf( stderr, "Warning: Config file does not contain '%s' root element\n", tag_root );
		return -1;
	}
	
	node = node->children;
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, tag_emulators ) == 0 ) {
				if( config_read_emulators( node->children ) != 0 )
					return -1;
			}
			else if( strcmp( (char*)node->name, tag_games ) == 0 ) {
				if( config_read_games( node->children ) != 0 )
					return -1;
			}
			else if( strcmp( (char*)node->name, tag_iface ) == 0 ) {
				if( config_read_interface( node->children ) != 0 )
					return -1;
			}
			else if( strcmp( (char*)node->name, tag_themes ) == 0 ) {
				if( config_read_themes( node->children ) != 0 )
					return -1;
			}
			else {
				fprintf( stderr, "Warning: Skipping unrecognised XML element in root: '%s'\n", node->name );
			}
		}
		node = node->next;
	}
	
	return 0;
}

xmlChar *config_write_boolean( int value ) {
	if( value )
		return (xmlChar*)config_true;
	else
		return (xmlChar*)config_false;
}

/* Low, medium, high */
xmlChar *config_write_lmh( int value ) {
	switch( value ) {
		case CONFIG_LOW: return (xmlChar*)config_low;
		case CONFIG_MEDIUM: return (xmlChar*)config_medium;
		case CONFIG_HIGH: return (xmlChar*)config_high;
	}
	return (xmlChar*)config_empty;
}

xmlChar *config_write_orientation( int value ) {
	switch( value ) {
		case CONFIG_PORTRAIT: return (xmlChar*)config_portrait;
		case CONFIG_LANDSCAPE: return (xmlChar*)config_landscape;
	}
	return (xmlChar*)config_empty;
}

xmlChar *config_write_numeric( int value ) {
	sprintf( scratch, "%d", value );
	return (xmlChar*)scratch;
}

xmlChar *config_write_percentage( int value ) {
	sprintf( scratch, "%d%%", value );
	return (xmlChar*)scratch;
}

int config_write_emulators( xmlNodePtr root ) {
	xmlNodePtr xml_emulators = xmlNewNode( NULL, (xmlChar*)tag_emulators );
	xmlNodePtr xml_emulator,xml_params,xml_param = NULL;
	struct config_emulator *emulator = config.emulators;
	
	while( emulator ) {
		struct config_param *param = emulator->params;
		
		xml_emulator = xmlNewNode( NULL, (xmlChar*)tag_emulator );
		xmlNewChild( xml_emulator, NULL, (xmlChar*)tag_name, (xmlChar*)emulator->name );
		xmlNewChild( xml_emulator, NULL, (xmlChar*)tag_display_name, (xmlChar*)emulator->display_name );
		xmlNewChild( xml_emulator, NULL, (xmlChar*)tag_emulator_executable, (xmlChar*)emulator->executable );
		
		if( param ) {
			xml_params = xmlNewNode( NULL, (xmlChar*)tag_params );
			while( param ) {
				xml_param = xmlNewNode( NULL, (xmlChar*)tag_param );
				xmlNewChild( xml_param, NULL, (xmlChar*)tag_name, (xmlChar*)param->name );
				if( param->value[0] ) {
					xmlNewChild( xml_param, NULL, (xmlChar*)tag_value, (xmlChar*)param->value );
				}
				xmlAddChild( xml_params, xml_param );
				param = param->next;
			}
			xmlAddChild( xml_emulator, xml_params );
		}
		xmlAddChild( xml_emulators, xml_emulator );
		emulator = emulator->next;
	}
	xmlAddChild( root, xml_emulators );

	return 0;
}

int config_write_interface( xmlNodePtr root ) {
	int i;
	xmlNodePtr interface = xmlNewNode( NULL, (xmlChar*)tag_iface );
	xmlNewChild( interface, NULL, (xmlChar*)tag_iface_full_screen, config_write_boolean( config.iface.full_screen ) );
	
	xmlNodePtr screen = xmlNewNode( NULL, (xmlChar*)tag_iface_screen );
	xmlNewChild( screen, NULL, (xmlChar*)tag_width, config_write_numeric( config.iface.screen_width ) );
	xmlNewChild( screen, NULL, (xmlChar*)tag_height, config_write_numeric( config.iface.screen_height ) );
	xmlNewChild( screen, NULL, (xmlChar*)tag_rotation, config_write_numeric( config.iface.screen_rotation ) );
	xmlNewChild( screen, NULL, (xmlChar*)tag_iface_screen_hflip, config_write_numeric( config.iface.screen_hflip ) );
	xmlNewChild( screen, NULL, (xmlChar*)tag_iface_screen_vflip, config_write_numeric( config.iface.screen_vflip ) );
	xmlAddChild( interface, screen );

	xmlNodePtr graphics = xmlNewNode( NULL, (xmlChar*)tag_iface_gfx );
	xmlNewChild( graphics, NULL, (xmlChar*)tag_iface_gfx_quality, config_write_lmh( config.iface.gfx_quality ) );
	xmlNewChild( graphics, NULL, (xmlChar*)tag_iface_gfx_max_width, config_write_numeric( config.iface.gfx_max_width ) );
	xmlNewChild( graphics, NULL, (xmlChar*)tag_iface_gfx_max_height, config_write_numeric( config.iface.gfx_max_height ) );
	xmlAddChild( interface, graphics );

	xmlNodePtr controls = xmlNewNode( NULL, (xmlChar*)tag_iface_controls );

	for( i = 1 ; i < NUM_EVENTS ; i++ ) {
		xmlNodePtr event = xmlNewNode( NULL, (xmlChar*)tag_event );
		xmlNodePtr device = xmlNewNode( NULL, (xmlChar*)tag_device );
		xmlNodePtr control = NULL;
		
		xmlNewChild( event, NULL, (xmlChar*)tag_name, (xmlChar*)event_name( i ) );
		xmlNewChild( device, NULL, (xmlChar*)tag_type, (xmlChar*)device_name( config.iface.controls[i].device_type ) );
		xmlNewChild( device, NULL, (xmlChar*)tag_id, config_write_numeric( config.iface.controls[i].device_id ) );
		
		switch( config.iface.controls[i].device_type ) {
			case DEV_KEYBOARD:
				xmlNewChild( event, NULL, (xmlChar*)tag_value, (xmlChar*)key_name( config.iface.controls[i].value ) );
				break;
			case DEV_JOYSTICK:
				control = xmlNewNode( NULL, (xmlChar*)tag_control );
				xmlNewChild( control, NULL, (xmlChar*)tag_type, (xmlChar*)control_name( config.iface.controls[i].control_type ) );
				xmlNewChild( control, NULL, (xmlChar*)tag_id, config_write_numeric( config.iface.controls[i].control_id ) );
				switch( config.iface.controls[i].control_type ) {
					case CTRL_BUTTON:
						xmlNewChild( event, NULL, (xmlChar*)tag_value, config_write_numeric( config.iface.controls[i].value ) );
						break;
					case CTRL_AXIS:
						xmlNewChild( event, NULL, (xmlChar*)tag_value, (xmlChar*)axis_dir_name( config.iface.controls[i].value ) );
						break;
					case CTRL_HAT:
					case CTRL_BALL:
						xmlNewChild( event, NULL, (xmlChar*)tag_value, (xmlChar*)direction_name( config.iface.controls[i].value ) );
						break;
					default:
						fprintf( stderr, "Warning: Not sure how write control config for unknown joystick control type %d\n", config.iface.controls[i].control_type );
						break;
				}
				break;
			case DEV_MOUSE:
				control = xmlNewNode( NULL, (xmlChar*)tag_control );
				xmlNewChild( control, NULL, (xmlChar*)tag_type, (xmlChar*)control_name( config.iface.controls[i].control_type ) );
				xmlNewChild( control, NULL, (xmlChar*)tag_id, config_write_numeric( config.iface.controls[i].control_id ) );
				switch( config.iface.controls[i].control_type ) {
					case CTRL_BUTTON:
						xmlNewChild( event, NULL, (xmlChar*)tag_value, config_write_numeric( config.iface.controls[i].value ) );
						break;
					case CTRL_AXIS:
						xmlNewChild( event, NULL, (xmlChar*)tag_value, (xmlChar*)axis_dir_name( config.iface.controls[i].value ) );
						break;
					default:
						fprintf( stderr, "Warning: Not sure how write control config for unknown mouse control type %d\n", config.iface.controls[i].control_type );
						break;
				}
				break;
			default:
				fprintf( stderr, "Warning: Not sure how write control config for unknown device type %d\n", config.iface.controls[i].device_type );
				break;
		}

		xmlAddChild( event, device );
		if( control ) {
			xmlAddChild( event, control );
		}
		xmlAddChild( controls, event );
	}
	xmlAddChild( interface, controls );
	
	xmlAddChild( root, interface );
	return 0;
}

int config_update( void ) {
	/* Write anything that may have changed back to the config object */
	int i;
	
	for( i = 1 ; i < NUM_EVENTS ; i++ ) {
		struct event *event = event_get(i);
		if( event ) {
			config.iface.controls[i].device_type = event->device_type;
			config.iface.controls[i].device_id = event->device_id;
			config.iface.controls[i].control_type = event->control_type;
			config.iface.controls[i].control_id = event->control_id;
			config.iface.controls[i].value = event->value;
		}
	}
	
	return 0;
}

int config_write() {
	xmlDocPtr config_doc = xmlNewDoc( (xmlChar*)"1.0" );
	xmlNodePtr config_root = xmlNewNode( NULL, (xmlChar*)tag_root );
	xmlDocSetRootElement( config_doc, config_root );
	
	config_write_interface( config_root );
	config_write_emulators( config_root );
	
	xmlSaveFormatFileEnc( config_filename, config_doc, "UTF-8", 1 );
	xmlFreeDoc( config_doc );
	
	return 0;
}

int config_set_theme_override( struct config_theme *theme ) {
	int retval = -1;
	xmlDocPtr config_doc = NULL;
	xmlNodePtr config_root = NULL;

	config_doc = xmlReadFile( config_filename, NULL, 0 );
	if( config_doc == NULL ) {
		fprintf( stderr, "Warning: Error reading config file '%s'\n", config_filename );
	}
	else {
		config_root = xmlDocGetRootElement( config_doc );
		if( config_root == NULL ) {
			fprintf( stderr, "Warning: Couldn't get root element of config file\n" );
		}
		else {
			if( strcmp( (char*)config_root->name, tag_root ) != 0 ) {
				fprintf( stderr, "Warning: Config file does not contain '%s' root element\n", tag_root );
			}
			else {
				xmlNodePtr node = config_root->children;
				while( node ) {
					if( node->type == XML_ELEMENT_NODE ) {
						if( strcmp( (char*)node->name, tag_iface ) == 0 ) {
							retval = config_read_interface_theme( node->children, theme );
							break;
						}
					}
					node = node->next;
				}
			}
		}
		xmlFreeDoc( config_doc );
	}
	return retval;
}

int config_set_theme( void ) {
	if( config.themes && config.iface.theme_name[0] ) {
		struct config_theme *ct = config.themes;
		while( ct ) {
			if( strcasecmp( ct->name, config.iface.theme_name ) == 0 ) {
				break;
			}
			ct = ct->next;
		}
		if( ct ) {
			memcpy( &config.iface.theme, ct, sizeof(struct config_theme) );
		}
		else {
			fprintf( stderr, "Warning: Couldn't find theme '%s', using default\n", config.iface.theme_name );
			memcpy( &config.iface.theme, &default_theme, sizeof(struct config_theme) );
		}
	}
	else {
		memcpy( &config.iface.theme, &default_theme, sizeof(struct config_theme) );
	}
	config_set_theme_override( &config.iface.theme );

	return 0;
}

int config_new( void ) {
	/* Create a new, default configuration (in memory) */
	memset( &config, 0, sizeof(struct config) );
	
	struct config_emulator *emulator = malloc( sizeof(struct config_emulator) );
	if( emulator == NULL ) {
		fprintf( stderr, "Error: couldn't allocate emulator structure\n" );
		return -1;
	}
	else {
		int i;
		struct config_param *prev_param = NULL;
		const int num_params = 4;
		const char *params[] = { "-nowindow", "-skip_gameinfo", "-switchres", "-joystick" };
		const char *keys[] = {
			"",				/* Place holder */
			"up",			/* EVENT_UP == 1 */
			"down",			/* EVENT_DOWN */
			"left",  		/* EVENT_LEFT */
			"right", 		/* EVENT_RIGHT */
			"return",		/* EVENT_SELECT */
			"backspace",	/* EVENT_BACK */
			"escape"		/* EVENT_QUIT */
		};

		emulator->id = 0;
		strncpy( emulator->name, "mame", CONFIG_NAME_LENGTH );
		strncpy( emulator->display_name, "MAME", CONFIG_NAME_LENGTH );
		strncpy( emulator->executable, "mame", CONFIG_FILE_NAME_LENGTH );
		emulator->next = NULL;
		config.emulators = emulator;
		
		for( i = num_params-1 ; i >= 0 ; i-- ) {
			struct config_param *param = malloc( sizeof(struct config_emulator) );
			if( param ) {
				strncpy( param->name, params[i], CONFIG_PARAM_LENGTH ); 
				param->value[0] = '\0';
				param->next = prev_param;
				prev_param = param;
			}
			emulator->params = param;
		}		

		config.games = NULL;
		config.platforms = NULL;

		config.iface.full_screen = 0;
		config.iface.screen_width = 640;
		config.iface.screen_height = 480;
		config.iface.screen_rotation = 0;
		config.iface.frame_rate = 60;
		
		config.iface.gfx_quality = CONFIG_HIGH;
		config.iface.gfx_max_width = 512;
		config.iface.gfx_max_height = 512;
		
		strcpy( config.iface.theme_name, "" );
		
		for( i = 1 ; i < NUM_EVENTS ; i++ ) {
			config.iface.controls[i].device_type = DEV_KEYBOARD;
			config.iface.controls[i].value = key_id( (char*)keys[i] );
		}

		/* Default theme */
		default_theme.next = NULL;
		strncpy( default_theme.name, default_theme_name, CONFIG_NAME_LENGTH );
		snprintf( default_theme.menu.texture, CONFIG_FILE_NAME_LENGTH, "%s%s", DATA_DIR, default_menu_texture );
		default_theme.menu.item_width = 0.8;
		default_theme.menu.item_height = 0.5;
		default_theme.menu.font_scale = 0.0025;
		default_theme.menu.zoom = 1.2;
		default_theme.menu.transparency = 40;
		default_theme.menu.offset1 = -1.2;
		default_theme.menu.offset2 = 2.0;
		default_theme.menu.max_visible = 3;
		default_theme.menu.spacing = -1;
		default_theme.menu.orientation = CONFIG_LANDSCAPE;
		default_theme.menu.auto_hide = 0;

		snprintf( default_theme.background_image, CONFIG_FILE_NAME_LENGTH, "%s%s", DATA_DIR, default_background );
		default_theme.background_rotation = 20;
		default_theme.background_transparency = 30;
		
		snprintf( default_theme.font_file, CONFIG_FILE_NAME_LENGTH, "%s%s", DATA_DIR, default_font );
		default_theme.font_size = 50;
		
		default_theme.screenshot.offset1 = -0.8;
		default_theme.screenshot.offset2 = 0;
		default_theme.screenshot.angle_x = -10;
		default_theme.screenshot.angle_y = 30;
		default_theme.screenshot.angle_z = 10;
		default_theme.screenshot.size = 1.0;
		default_theme.screenshot.fix_aspect_ratio = 1;
		default_theme.screenshot.auto_hide = 1;

		default_theme.hints.offset1 = -2.0;
		default_theme.hints.offset2 = -1.2;
		default_theme.hints.size = 1;
		default_theme.hints.pulse = 1;
		strncpy( default_theme.hints.label_select, "Select", CONFIG_LABEL_LENGTH );
		strncpy( default_theme.hints.label_back, "Back", CONFIG_LABEL_LENGTH );
		snprintf( default_theme.hints.image_back, CONFIG_FILE_NAME_LENGTH, "%s%s", DATA_DIR, default_back_texture );
		snprintf( default_theme.hints.image_select, CONFIG_FILE_NAME_LENGTH, "%s%s", DATA_DIR, default_select_texture );
		snprintf( default_theme.hints.image_arrow, CONFIG_FILE_NAME_LENGTH, "%s%s", DATA_DIR, default_arrow_texture );
		
		default_theme.game_sel.orientation = CONFIG_PORTRAIT;
		default_theme.game_sel.offset1 = 0.9;
		default_theme.game_sel.offset2 = 0;
		default_theme.game_sel.size_x = 1.0;
		default_theme.game_sel.size_y = 1.0;
		default_theme.game_sel.tile_size = 1.0;
		default_theme.game_sel.tiles = NULL;
		for( i = 0 ; i < default_num_tiles ; i++ ) {
			struct config_game_sel_tile *tile = malloc( sizeof(struct config_game_sel_tile) );
			if( tile ) {
				tile->order = default_num_tiles - i;
				if( i == (default_num_tiles/2) )
					default_theme.game_sel.selected = tile->order;
				
				tile->transparency = default_tile_transparency[i];
				
				tile->pos[0] = default_tile_pos[i][0];
				tile->pos[1] = default_tile_pos[i][1];
				tile->pos[2] = default_tile_pos[i][2];
				
				tile->angle[0] = default_tile_angle[i][0];
				tile->angle[1] = default_tile_angle[i][1];
				tile->angle[2] = default_tile_angle[i][2];
				
				tile->next = default_theme.game_sel.tiles;
				default_theme.game_sel.tiles = tile;
			}
			else {
				fprintf( stderr, "Error: Couldn't allocate configuration game tile structure\n" );
				return -1;	
			}	
		}
		
		for( i = 0 ; i < NUM_SOUNDS ; i++ ) {
			snprintf( default_theme.sounds[i], CONFIG_FILE_NAME_LENGTH, "%s%s", DATA_DIR, (char*)default_sounds[i] );
		}
		
		config.themes = &default_theme;
	}
	return 0;
}

int config_create( void ) {
	DIR *dir;
	
	/* Check if directory exists and attempt to create if not */
	dir = opendir( config_directory );
	if( dir == NULL ) {
		switch( errno ) {
			case ENOTDIR:
				fprintf( stderr, "Warning: Can't read config directory '%s': no such file or directory\n", config_directory );
				return -1;
				break;
			case ENOENT:
#ifdef __unix__
				if( mkdir( config_directory, 0755 ) != 0 ) {
#else
				if( mkdir( config_directory ) != 0 ) {
#endif
					fprintf( stderr, "Warning: Can't create default config directory '%s'\n", config_directory );
					return -1;
				}
				break;
			default:
				break;
		}
	}
	else {
		closedir( dir );
	}
	
	return config_write();
}

int config_read_file( char *filename ) {
	int retval = -1;
	xmlDocPtr config_doc = NULL;
	xmlNodePtr config_root = NULL;

	config_doc = xmlReadFile( filename, NULL, 0 );
	if( config_doc == NULL ) {
		fprintf( stderr, "Warning: Error reading config file '%s'\n", filename );
	}
	else {
		config_root = xmlDocGetRootElement( config_doc );
		if( config_root == NULL ) {
			fprintf( stderr, "Warning: Couldn't get root element of config file\n" );
		}
		else {
			retval = config_read( config_root );
		}
		xmlFreeDoc( config_doc );
	}
	return retval;
}

int config_open( const char *filename ) {
	int created = 0;
	DIR *dir = NULL;
#ifdef __unix__
	struct passwd *passwd = getpwuid(getuid());
#endif
	
	if( config_new() != 0 ) {
		fprintf( stderr, "Error: Config initialisation failed\n" );
		return -1;
	}

#ifdef __unix__
	if( passwd == NULL ) {
		fprintf( stderr, "Error: Couldn't fetch user's home directory\n" );
		return -1;
	}

	snprintf( config_directory, CONFIG_FILE_NAME_LENGTH, "%s/%s", passwd->pw_dir, default_dir );
#else
	strcpy( config_directory, "." );
#endif

	if( filename ) {
		/* Use supplied file name throughout */
		if( strlen(filename) > CONFIG_FILE_NAME_LENGTH-1 ) {
			fprintf( stderr, "Error: Config file name '%s' exceeds %d characters\n", filename, CONFIG_FILE_NAME_LENGTH-1 );
			return -1;
		}
		strncpy( config_filename, filename, CONFIG_FILE_NAME_LENGTH );
	}
	else {
		/* Determine (path to) default config file */
		FILE *file;
		
#ifdef __unix__
		snprintf( config_filename, CONFIG_FILE_NAME_LENGTH, "%s/%s", config_directory, default_file );
#else
		strcpy( config_filename, default_file );
#endif
		file = fopen( config_filename, "r" );
		if( file == NULL ) {
			switch( errno ) {
				case EACCES:
					fprintf( stderr, "Error: Can't read config file '%s': access denied\n", config_filename );
					return -1;
					break;
				case ENOENT:
					created = 1; /* We check for this in main() */
					break;
				default:
					fprintf( stderr, "Error: Can't read config file '%s': errno = %d\n", config_filename, errno );
					return -1;
					break;
			}
		}
		else {
			fclose( file );	
		}
	}

	if( !created && config_read_file( config_filename ) != 0 )
		return -1;
			
	/* Scan for other config files in the same directory */
	if( !(dir = opendir( config_directory )) ) {
		fprintf( stderr, "Warning: Can't scan for additional config files in '%s': %s\n", config_directory, strerror( errno ) );
	}
	else {
		struct dirent *dentry;
		char filename[CONFIG_FILE_NAME_LENGTH] = "";
		char *dot = NULL;
		
		while( (dentry = readdir( dir )) ) {
			dot = strrchr( dentry->d_name, '.' );

			if( dot && strcasecmp( dot, ".xml" ) == 0 ) {
#ifdef __WIN32__
				if( strcasecmp( dentry->d_name, default_file ) != 0 ) {
					snprintf( filename, CONFIG_FILE_NAME_LENGTH, "%s\\%s", config_directory, dentry->d_name );
#else
				if( strcmp( dentry->d_name, default_file ) != 0 ) {
					snprintf( filename, CONFIG_FILE_NAME_LENGTH, "%s/%s", config_directory, dentry->d_name );
#endif
					if( config_read_file( filename ) != 0 ) {
						fprintf( stderr, "Warning: Error reading auxiliary config file '%s'\n", filename );
					}
				}
			}
		}
		closedir( dir );
	}

	/* We now have our entire configuration, so the theming may begin... */
	config_set_theme();
	
	return created;
}

