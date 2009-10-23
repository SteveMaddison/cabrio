#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pwd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include "config.h"
#include "sdl.h"

#include <libxml2/libxml/parser.h>
#include <libxml2/libxml/tree.h>

struct config *config;

static const char *config_default_dir = ".cabrio"; /* Relative to user's home */
static const char *config_default_file = "config.xml";
static char config_filename[CONFIG_FILE_NAME_LENGTH] = "";

/* Specific XML tags */
static const char *config_tag_root					= "cabrio-config";
static const char *config_tag_emulators				= "emulators";
static const char *config_tag_emulator				= 	"emulator";
static const char *config_tag_emulator_executable	= 		"executable";
static const char *config_tag_games					= "games";
static const char *config_tag_game					=   "game";
static const char *config_tag_game_rom_image		=     "rom-image";
static const char *config_tag_game_logo_image		=     "logo-image";
static const char *config_tag_game_background_image	=     "background-image";
static const char *config_tag_iface					= "interface";
static const char *config_tag_iface_full_screen		= 	"full-screen";
static const char *config_tag_iface_screen_width	= 	"screen-width";
static const char *config_tag_iface_screen_height	= 	"screen-height";
static const char *config_tag_iface_controls		=   "controls";
/* General (reused) XML tags */
static const char *config_tag_name					= "name";
static const char *config_tag_value					= "value";
static const char *config_tag_id					= "id";
static const char *config_tag_display_name			= "display-name";
static const char *config_tag_genre					= "genre";
static const char *config_tag_platform				= "platform";
static const char *config_tag_params				= "params";
static const char *config_tag_param					= "param";
static const char *config_tag_control				= "control";
static const char *config_tag_event					= "event";
static const char *config_tag_device				= "device";
static const char *config_tag_type					= "type";

static const char *config_true = "true";
static const char *config_false = "false";

static const char *warn_alloc = "Warning: Couldn't allocate memory for '%s' object\n";
static const char *warn_skip = "Warning: Skipping unrecognised XML element in '%s': '%s'\n";

/* Defined at the end of this file... */
int config_key( char *name );
const char *config_key_name( int key );
/* Big list o' key names. These are just the SDLK_* macros with their prefix chopped off. */
static const char *config_key_backspace = "backspace";
static const char *config_key_tab = "tab";
static const char *config_key_clear = "clear";
static const char *config_key_return = "return";
static const char *config_key_pause = "pause";
static const char *config_key_escape = "escape";
static const char *config_key_space = "space";
static const char *config_key_exclaim = "exclaim";
static const char *config_key_quotedbl = "quotedbl";
static const char *config_key_hash = "hash";
static const char *config_key_dollar = "dollar";
static const char *config_key_ampersand = "ampersand";
static const char *config_key_quote = "quote";
static const char *config_key_leftparen = "leftparen";
static const char *config_key_rightparen = "rightparen";
static const char *config_key_asterisk = "asterisk";
static const char *config_key_plus = "plus";
static const char *config_key_comma = "comma";
static const char *config_key_minus = "minus";
static const char *config_key_period = "period";
static const char *config_key_slash = "slash";
static const char *config_key_0 = "0";
static const char *config_key_1 = "1";
static const char *config_key_2 = "2";
static const char *config_key_3 = "3";
static const char *config_key_4 = "4";
static const char *config_key_5 = "5";
static const char *config_key_6 = "6";
static const char *config_key_7 = "7";
static const char *config_key_8 = "8";
static const char *config_key_9 = "9";
static const char *config_key_colon = "colon";
static const char *config_key_semicolon = "semicolon";
static const char *config_key_less = "less";
static const char *config_key_equals = "equals";
static const char *config_key_greater = "greater";
static const char *config_key_question = "question";
static const char *config_key_at = "at";
static const char *config_key_leftbracket = "leftbracket";
static const char *config_key_backslash = "backslash";
static const char *config_key_rightbracket = "rightbracket";
static const char *config_key_caret = "caret";
static const char *config_key_underscore = "underscore";
static const char *config_key_backquote = "backquote";
static const char *config_key_a = "a";
static const char *config_key_b = "b";
static const char *config_key_c = "c";
static const char *config_key_d = "d";
static const char *config_key_e = "e";
static const char *config_key_f = "f";
static const char *config_key_g = "g";
static const char *config_key_h = "h";
static const char *config_key_i = "i";
static const char *config_key_j = "j";
static const char *config_key_k = "k";
static const char *config_key_l = "l";
static const char *config_key_m = "m";
static const char *config_key_n = "n";
static const char *config_key_o = "o";
static const char *config_key_p = "p";
static const char *config_key_q = "q";
static const char *config_key_r = "r";
static const char *config_key_s = "s";
static const char *config_key_t = "t";
static const char *config_key_u = "u";
static const char *config_key_v = "v";
static const char *config_key_w = "w";
static const char *config_key_x = "x";
static const char *config_key_y = "y";
static const char *config_key_z = "z";
static const char *config_key_delete = "delete";
static const char *config_key_world_0 = "world_0";
static const char *config_key_world_1 = "world_1";
static const char *config_key_world_2 = "world_2";
static const char *config_key_world_3 = "world_3";
static const char *config_key_world_4 = "world_4";
static const char *config_key_world_5 = "world_5";
static const char *config_key_world_6 = "world_6";
static const char *config_key_world_7 = "world_7";
static const char *config_key_world_8 = "world_8";
static const char *config_key_world_9 = "world_9";
static const char *config_key_world_10 = "world_10";
static const char *config_key_world_11 = "world_11";
static const char *config_key_world_12 = "world_12";
static const char *config_key_world_13 = "world_13";
static const char *config_key_world_14 = "world_14";
static const char *config_key_world_15 = "world_15";
static const char *config_key_world_16 = "world_16";
static const char *config_key_world_17 = "world_17";
static const char *config_key_world_18 = "world_18";
static const char *config_key_world_19 = "world_19";
static const char *config_key_world_20 = "world_20";
static const char *config_key_world_21 = "world_21";
static const char *config_key_world_22 = "world_22";
static const char *config_key_world_23 = "world_23";
static const char *config_key_world_24 = "world_24";
static const char *config_key_world_25 = "world_25";
static const char *config_key_world_26 = "world_26";
static const char *config_key_world_27 = "world_27";
static const char *config_key_world_28 = "world_28";
static const char *config_key_world_29 = "world_29";
static const char *config_key_world_30 = "world_30";
static const char *config_key_world_31 = "world_31";
static const char *config_key_world_32 = "world_32";
static const char *config_key_world_33 = "world_33";
static const char *config_key_world_34 = "world_34";
static const char *config_key_world_35 = "world_35";
static const char *config_key_world_36 = "world_36";
static const char *config_key_world_37 = "world_37";
static const char *config_key_world_38 = "world_38";
static const char *config_key_world_39 = "world_39";
static const char *config_key_world_40 = "world_40";
static const char *config_key_world_41 = "world_41";
static const char *config_key_world_42 = "world_42";
static const char *config_key_world_43 = "world_43";
static const char *config_key_world_44 = "world_44";
static const char *config_key_world_45 = "world_45";
static const char *config_key_world_46 = "world_46";
static const char *config_key_world_47 = "world_47";
static const char *config_key_world_48 = "world_48";
static const char *config_key_world_49 = "world_49";
static const char *config_key_world_50 = "world_50";
static const char *config_key_world_51 = "world_51";
static const char *config_key_world_52 = "world_52";
static const char *config_key_world_53 = "world_53";
static const char *config_key_world_54 = "world_54";
static const char *config_key_world_55 = "world_55";
static const char *config_key_world_56 = "world_56";
static const char *config_key_world_57 = "world_57";
static const char *config_key_world_58 = "world_58";
static const char *config_key_world_59 = "world_59";
static const char *config_key_world_60 = "world_60";
static const char *config_key_world_61 = "world_61";
static const char *config_key_world_62 = "world_62";
static const char *config_key_world_63 = "world_63";
static const char *config_key_world_64 = "world_64";
static const char *config_key_world_65 = "world_65";
static const char *config_key_world_66 = "world_66";
static const char *config_key_world_67 = "world_67";
static const char *config_key_world_68 = "world_68";
static const char *config_key_world_69 = "world_69";
static const char *config_key_world_70 = "world_70";
static const char *config_key_world_71 = "world_71";
static const char *config_key_world_72 = "world_72";
static const char *config_key_world_73 = "world_73";
static const char *config_key_world_74 = "world_74";
static const char *config_key_world_75 = "world_75";
static const char *config_key_world_76 = "world_76";
static const char *config_key_world_77 = "world_77";
static const char *config_key_world_78 = "world_78";
static const char *config_key_world_79 = "world_79";
static const char *config_key_world_80 = "world_80";
static const char *config_key_world_81 = "world_81";
static const char *config_key_world_82 = "world_82";
static const char *config_key_world_83 = "world_83";
static const char *config_key_world_84 = "world_84";
static const char *config_key_world_85 = "world_85";
static const char *config_key_world_86 = "world_86";
static const char *config_key_world_87 = "world_87";
static const char *config_key_world_88 = "world_88";
static const char *config_key_world_89 = "world_89";
static const char *config_key_world_90 = "world_90";
static const char *config_key_world_91 = "world_91";
static const char *config_key_world_92 = "world_92";
static const char *config_key_world_93 = "world_93";
static const char *config_key_world_94 = "world_94";
static const char *config_key_world_95 = "world_95";
static const char *config_key_kp0 = "kp0";
static const char *config_key_kp1 = "kp1";
static const char *config_key_kp2 = "kp2";
static const char *config_key_kp3 = "kp3";
static const char *config_key_kp4 = "kp4";
static const char *config_key_kp5 = "kp5";
static const char *config_key_kp6 = "kp6";
static const char *config_key_kp7 = "kp7";
static const char *config_key_kp8 = "kp8";
static const char *config_key_kp9 = "kp9";
static const char *config_key_kp_period = "kp_period";
static const char *config_key_kp_divide = "kp_divide";
static const char *config_key_kp_multiply = "kp_multiply";
static const char *config_key_kp_minus = "kp_minus";
static const char *config_key_kp_plus = "kp_plus";
static const char *config_key_kp_enter = "kp_enter";
static const char *config_key_kp_equals = "kp_equals";
static const char *config_key_up = "up";
static const char *config_key_down = "down";
static const char *config_key_right = "right";
static const char *config_key_left = "left";
static const char *config_key_insert = "insert";
static const char *config_key_home = "home";
static const char *config_key_end = "end";
static const char *config_key_pageup = "pageup";
static const char *config_key_pagedown = "pagedown";
static const char *config_key_f1 = "f1";
static const char *config_key_f2 = "f2";
static const char *config_key_f3 = "f3";
static const char *config_key_f4 = "f4";
static const char *config_key_f5 = "f5";
static const char *config_key_f6 = "f6";
static const char *config_key_f7 = "f7";
static const char *config_key_f8 = "f8";
static const char *config_key_f9 = "f9";
static const char *config_key_f10 = "f10";
static const char *config_key_f11 = "f11";
static const char *config_key_f12 = "f12";
static const char *config_key_f13 = "f13";
static const char *config_key_f14 = "f14";
static const char *config_key_f15 = "f15";
static const char *config_key_numlock = "numlock";
static const char *config_key_capslock = "capslock";
static const char *config_key_scrollock = "scrollock";
static const char *config_key_rshift = "rshift";
static const char *config_key_lshift = "lshift";
static const char *config_key_rctrl = "rctrl";
static const char *config_key_lctrl = "lctrl";
static const char *config_key_ralt = "ralt";
static const char *config_key_lalt = "lalt";
static const char *config_key_rmeta = "rmeta";
static const char *config_key_lmeta = "lmeta";
static const char *config_key_lsuper = "lsuper";
static const char *config_key_rsuper = "rsuper";
static const char *config_key_mode = "mode";
static const char *config_key_compose = "compose";
static const char *config_key_help = "help";
static const char *config_key_print = "print";
static const char *config_key_sysreq = "sysreq";
static const char *config_key_break = "break";
static const char *config_key_menu = "menu";
static const char *config_key_power = "power";
static const char *config_key_euro = "euro";
static const char *config_key_undo = "undo";

static xmlDocPtr config_doc = NULL;
static xmlNodePtr config_root = NULL;
static char scratch[32] = "";

int config_read_boolean( char *name, char *value, int *target ) {
	if( strcasecmp( value, "yes" ) == 0 ||  strcasecmp( value, "true" ) == 0 ) {
		*target = 1;
		return 0;
	}
	else if( strcasecmp( value, "no" ) == 0 ||  strcasecmp( value, "false" ) == 0 ) {
		*target = 0;
		return 0;
	}
	return -1;
}

int config_read_numeric( char *name, char *value, int *target ) {
	char *pos = value;
	if( pos ) {
		while( *pos ) {
			if( (*pos < '0' || *pos > '9') && (*pos != '-') ) {
				fprintf( stderr, "Warning: Element %s requires numeric value\n", name );
				return -1;
			}
			pos++;
		}
		*target = strtol( value, NULL, 10 );
	}
	return 0;
}

int config_set_control( int event, struct config_control *new ) {
	struct config_control *control = config->iface.controls;
	
	while( control ) {
		if( control->event == event ) {
			control->device_type = new->device_type;
			control->device_id = new->device_id;
			control->control_type = new->control_type;
			control->control_id = new->control_id;
			control->value = new->value;
			break;
		}
		control = control->next;
	}
	if( control == NULL ) {
		fprintf( stderr, "Warning: Couldn't set control for event %d\n", event );
		return -1;
	}

	return 0;
}

struct config_genre *config_genre( const char *name ) {
	struct config_genre *g = config->genres;

	if( name && *name ) {
		while( g ) {
			if( strncasecmp( name, g->name, CONFIG_NAME_LENGTH ) == 0 )
				break;
			g = g->next;
		}
		if( g == NULL ) {
			/* add new */
			g = malloc( sizeof(struct config_genre) );
			if( g == NULL ) {	
				fprintf( stderr, "Error: Couldn't create new genre configuration object" );
			}
			else {
				strncpy( g->name, name, CONFIG_NAME_LENGTH );
				g->next = config->genres;
				config->genres = g;
			}
		}
	}
	else {
		g = NULL;
	}
	return g;
}

struct config_platform *config_platform( const char *name ) {
	struct config_platform *p = config->platforms;

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
				fprintf( stderr, "Error: Couldn't create new genre configuration object" );
			}
			else {
				strncpy( p->name, name, CONFIG_NAME_LENGTH );
				p->next = config->platforms;
				config->platforms = p;
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
			if( strcmp( (char*)node->name, config_tag_name ) == 0 ) {
				strncpy( param->name, (char*)xmlNodeGetContent(node), CONFIG_PARAM_LENGTH );
			}
			else if( strcmp( (char*)node->name, config_tag_value ) == 0 ) {
				strncpy( param->value, (char*)xmlNodeGetContent(node), CONFIG_PARAM_LENGTH );
			}
			else {
				fprintf( stderr, warn_skip, config_tag_param, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_emulator_params( xmlNode *node, struct config_emulator *emulator ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_param ) == 0 ) {
				struct config_param *param = malloc( sizeof(struct config_param ) );
				if( param ) {
					memset( param, 0, sizeof(struct config_param ) );
					config_read_param( node->children, param );
					param->next = emulator->params;
					emulator->params = param;
				}
				else {
					fprintf( stderr, warn_alloc, config_tag_param );
					return -1;
				}				
			}
			else {
				fprintf( stderr, warn_skip, config_tag_params, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_emulator( xmlNode *node, struct config_emulator *emulator ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_name ) == 0 ) {
				strncpy( emulator->name, (char*)xmlNodeGetContent(node), CONFIG_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, config_tag_display_name ) == 0 ) {
				strncpy( emulator->display_name, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, config_tag_emulator_executable ) == 0 ) {
				strncpy( emulator->executable, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, config_tag_params ) == 0 ) {
				config_read_emulator_params( node->children, emulator );
			}
			else {
				fprintf( stderr, warn_skip, config_tag_emulator, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_emulators( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_emulator ) == 0 ) {
				struct config_emulator *emulator = malloc( sizeof(struct config_emulator ) );
				if( emulator ) {
					memset( emulator, 0, sizeof(struct config_emulator ) );
					config_read_emulator( node->children, emulator );
					emulator->next = config->emulators;
					config->emulators = emulator;
				}
				else {
					fprintf( stderr, warn_alloc, config_tag_emulator );
					return -1;
				}
			}
			else {
				fprintf( stderr, warn_skip, config_tag_emulators, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_game_params( xmlNode *node, struct config_game *game ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_param ) == 0 ) {
				struct config_param *param = malloc( sizeof(struct config_param ) );
				if( param ) {
					memset( param, 0, sizeof(struct config_param ) );
					config_read_param( node->children, param );
					param->next = game->params;
					game->params = param;
				}
				else {
					fprintf( stderr, warn_alloc, config_tag_param );
					return -1;
				}					
			}
			else {
				fprintf( stderr, warn_skip, config_tag_params, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_game( xmlNode *node, struct config_game *game ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_name ) == 0 ) {
				strncpy( game->name, (char*)xmlNodeGetContent(node), CONFIG_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, config_tag_game_rom_image ) == 0 ) {
				strncpy( game->rom_image, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, config_tag_game_logo_image ) == 0 ) {
				strncpy( game->logo_image, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, config_tag_game_background_image ) == 0 ) {
				strncpy( game->background_image, (char*)xmlNodeGetContent(node), CONFIG_FILE_NAME_LENGTH );
			}
			else if( strcmp( (char*)node->name, config_tag_genre ) == 0 ) {
				game->genre = config_genre( (char*)xmlNodeGetContent(node) );
			}
			else if( strcmp( (char*)node->name, config_tag_platform ) == 0 ) {
				game->platform = config_platform( (char*)xmlNodeGetContent(node) );
			}
			else if( strcmp( (char*)node->name, config_tag_params ) == 0 ) {
				config_read_game_params( node->children, game );
			}
			else {
				fprintf( stderr, warn_skip, config_tag_game, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_games( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_game ) == 0 ) {
				struct config_game *game = malloc( sizeof(struct config_game ) );
				if( game ) {
					memset( game, 0, sizeof(struct config_game ) );
					config_read_game( node->children, game );
					game->next = config->games;
					config->games = game;
				}
				else {
					fprintf( stderr, warn_alloc, config_tag_game );
					return -1;
				}
			}
			else {
				fprintf( stderr, warn_skip, config_tag_games, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read_event_name( char *name, struct config_control *control ) {
	if( name == NULL ) {
		fprintf( stderr, "Warning: Null event name\n" );		
		return -1;	
	}
	if( strcasecmp( name, "up" ) == 0 )
		control->event = EVENT_UP;
	else if( strcasecmp( name, "down" ) == 0 )
		control->event = EVENT_DOWN;
	else if( strcasecmp( name, "left" ) == 0 )
		control->event = EVENT_LEFT;
	else if( strcasecmp( name, "right" ) == 0 )
		control->event = EVENT_RIGHT;
	else if( strcasecmp( name, "select" ) == 0 )
		control->event = EVENT_SELECT;
	else if( strcasecmp( name, "back" ) == 0 )
		control->event = EVENT_BACK;
	else if( strcasecmp( name, "quit" ) == 0 )
		control->event = EVENT_QUIT;
	else {
		fprintf( stderr, "Warning: Unknown event name '%s'\n", name );
		control->event = EVENT_NONE;
		return -1;
	}

	return 0;
}

int config_read_device_type( char *name, struct config_control *control ) {
	if( name == NULL ) {
		fprintf( stderr, "Warning: Null device type\n" );		
		return -1;	
	}
	if( strcasecmp( name, "keyboard" ) == 0 )
		control->device_type = DEV_KEYBOARD;
	else if( strcasecmp( name, "joystick" ) == 0 )
		control->device_type = DEV_JOYSTICK;
	else if( strcasecmp( name, "mouse" ) == 0 )
		control->device_type = DEV_MOUSE;
	else {
		fprintf( stderr, "Warning: Unknown device type '%s'\n", name );
		control->event = DEV_UNKNOWN;
		return -1;
	}

	return 0;	
}

int config_read_device( xmlNode *node, struct config_control *control ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_type ) == 0 ) {
				config_read_device_type( (char*)xmlNodeGetContent(node), control );
			}
			else if( strcmp( (char*)node->name, config_tag_id ) == 0 ) {
				config_read_numeric( (char*)node->name, (char*)xmlNodeGetContent(node), &control->device_id );
			}
			else {
				fprintf( stderr, warn_skip, config_tag_device, node->name );	
			}
		}
		node = node->next;
	}

	return 0;
}

int config_read_control_type( char *name, struct config_control *control ) {
	if( name == NULL ) {
		fprintf( stderr, "Warning: Null control type\n" );		
		return -1;	
	}
	if( strcasecmp( name, "button" ) == 0 )
		control->control_type = CTRL_BUTTON;
	else if( strcasecmp( name, "axis" ) == 0 )
		control->control_type = CTRL_AXIS;
	else if( strcasecmp( name, "hat" ) == 0 )
		control->control_type = CTRL_HAT;
	else if( strcasecmp( name, "ball" ) == 0 )
		control->control_type = CTRL_BALL;
	else {
		fprintf( stderr, "Warning: Unknown control type '%s'\n", name );
		control->event = CTRL_UNKNOWN;
		return -1;
	}

	return 0;	
}

int config_read_control( xmlNode *node, struct config_control *control ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_type ) == 0 ) {
				config_read_control_type( (char*)xmlNodeGetContent(node), control );
			}
			else if( strcmp( (char*)node->name, config_tag_id ) == 0 ) {
				config_read_numeric( (char*)node->name, (char*)xmlNodeGetContent(node), &control->control_id );
			}
			else {
				fprintf( stderr, warn_skip, config_tag_device, node->name );	
			}
		}
		node = node->next;
	}

	return 0;
}

int config_read_event( xmlNode *node ) {
	struct config_control tmp;
	char *value = NULL;
	
	memset( &tmp, 0, sizeof(struct config_control) );
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_name ) == 0 ) {
				if( config_read_event_name( (char*)xmlNodeGetContent(node), &tmp ) != 0 ) {
					return -1;
				}
			}
			else if( strcmp( (char*)node->name, config_tag_device ) == 0 ) {
				if( config_read_device( node->children, &tmp ) != 0 ) {
					return -1;
				}
			}
			else if( strcmp( (char*)node->name, config_tag_control ) == 0 ) {
				if( config_read_control( node->children, &tmp ) != 0 ) {
					return -1;
				}
			}
			else if( strcmp( (char*)node->name, config_tag_value ) == 0 ) {
				value = (char*)xmlNodeGetContent(node);
			}
			else {
				fprintf( stderr, warn_skip, config_tag_control, node->name );	
			}
		}
		node = node->next;
	}
	
	/* Decode the key name if necessary */
	if( tmp.device_type == DEV_KEYBOARD ) {
		tmp.value = config_key( value );
		if( tmp.value == 0 ) {
			fprintf( stderr, "Warning: Unknown key name '%s'\n", value );
			return -1;
		}
	}
	else {
		config_read_numeric( (char*)config_tag_id, value, &tmp.value );
	}
	
	/* Replace exisiting control for this event */
	config_set_control( tmp.event, &tmp );
	
	return 0;	
}

int config_read_controls( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_event ) == 0 ) {
				config_read_event( node->children );
			}
			else {
				fprintf( stderr, warn_skip, config_tag_iface_controls, node->name );	
			}
		}
		node = node->next;
	}
	return 0;	
}

int config_read_interface( xmlNode *node ) {
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_iface_full_screen ) == 0 ) {
				config_read_boolean( (char*)node->name, (char*)xmlNodeGetContent(node), &config->iface.full_screen );
			}
			else if( strcmp( (char*)node->name, config_tag_iface_screen_width ) == 0 ) {
				config_read_numeric( (char*)node->name, (char*)xmlNodeGetContent(node), &config->iface.screen_width );
			}
			else if( strcmp( (char*)node->name, config_tag_iface_screen_height ) == 0 ) {
				config_read_numeric( (char*)node->name, (char*)xmlNodeGetContent(node), &config->iface.screen_height );
			}
			else if( strcmp( (char*)node->name, config_tag_iface_controls ) == 0 ) {
				config_read_controls( node->children );
			}
			else {
				fprintf( stderr, warn_skip, config_tag_iface, node->name );	
			}
		}
		node = node->next;
	}
	return 0;
}

int config_read( xmlNode *root ) {
	xmlNode *node = root;
	
	if( strcmp( (char*)node->name, config_tag_root ) != 0 ) {
		fprintf( stderr, "Warning: Config file does not contain '%s' root element\n", config_tag_root );
		return -1;
	}
	
	node = node->children;
	while( node ) {
		if( node->type == XML_ELEMENT_NODE ) {
			if( strcmp( (char*)node->name, config_tag_emulators ) == 0 ) {
				if( config_read_emulators( node->children ) != 0 )
					return -1;
			}
			else if( strcmp( (char*)node->name, config_tag_games ) == 0 ) {
				if( config_read_games( node->children ) != 0 )
					return -1;
			}
			else if( strcmp( (char*)node->name, config_tag_iface ) == 0 ) {
				if( config_read_interface( node->children ) != 0 )
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

xmlChar *config_write_numeric( int value ) {
	sprintf( scratch, "%d", value );
	return (xmlChar*)scratch;
}

int config_write_emulators( void ) {
	xmlNodePtr xml_emulators = xmlNewNode( NULL, (xmlChar*)config_tag_emulators );
	xmlNodePtr xml_emulator,xml_params,xml_param = NULL;
	struct config_emulator *emulator = config->emulators;
	
	while( emulator ) {
		struct config_param *param = emulator->params;
		
		xml_emulator = xmlNewNode( NULL, (xmlChar*)config_tag_emulator );
		xmlNewChild( xml_emulator, NULL, (xmlChar*)config_tag_name, (xmlChar*)emulator->name );
		xmlNewChild( xml_emulator, NULL, (xmlChar*)config_tag_display_name, (xmlChar*)emulator->display_name );
		xmlNewChild( xml_emulator, NULL, (xmlChar*)config_tag_emulator_executable, (xmlChar*)emulator->executable );
		
		if( param ) {
			xml_params = xmlNewNode( NULL, (xmlChar*)config_tag_params );
			while( param ) {
				xml_param = xmlNewNode( NULL, (xmlChar*)config_tag_param );
				xmlNewChild( xml_param, NULL, (xmlChar*)config_tag_name, (xmlChar*)param->name );
				if( param->value[0] ) {
					xmlNewChild( xml_param, NULL, (xmlChar*)config_tag_value, (xmlChar*)param->value );
				}
				xmlAddChild( xml_params, xml_param );
				param = param->next;
			}
			xmlAddChild( xml_emulator, xml_params );
		}
		xmlAddChild( xml_emulators, xml_emulator );
		emulator = emulator->next;
	}
	xmlAddChild( config_root, xml_emulators );

	return 0;
}

int config_write_games( void ) {
	return 0;
}

int config_write_interface( void ) {
	xmlNodePtr node = xmlNewNode( NULL, (xmlChar*)config_tag_iface );
	xmlNewChild( node, NULL, (xmlChar*)config_tag_iface_full_screen, config_write_boolean( config->iface.screen_width) );
	xmlNewChild( node, NULL, (xmlChar*)config_tag_iface_screen_width, config_write_numeric( config->iface.screen_width ) );
	xmlNewChild( node, NULL, (xmlChar*)config_tag_iface_screen_height, config_write_numeric( config->iface.screen_height ) );
	xmlAddChild( config_root, node );
	return 0;
}

int config_write_platforms( void ) {
	return 0;
}

int config_write() {
	config_doc = xmlNewDoc( (xmlChar*)"1.0" );
	config_root = xmlNewNode( NULL, (xmlChar*)config_tag_root );
	xmlDocSetRootElement( config_doc, config_root );
	
	config_write_interface();
	config_write_emulators();
	config_write_platforms();
	config_write_games();
	
	xmlSaveFormatFileEnc( config_filename, config_doc, "UTF-8", 1 );
	xmlFreeDoc( config_doc );
	printf( "Info: Wrote default configuration to '%s'\n", config_filename );

	return 0;
}

int config_new( void ) {
	/* Create a new, default configuration (in memory) */
	config = malloc( sizeof(struct config) );
	if( config == NULL ) {
		fprintf( stderr, "Error: couldn't allocate config structure\n" );
		return -1;
	}
	else {
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
			struct config_control *prev_control = NULL;
			const int keys[] = {
				SDLK_UP,		/* EVENT_UP == 1 */
				SDLK_DOWN,		/* EVENT_DOWN */
				SDLK_LEFT,  	/* EVENT_LEFT */
				SDLK_RIGHT, 	/* EVENT_RIGHT */
				SDLK_RETURN,	/* EVENT_SELECT */
				SDLK_BACKSPACE,	/* EVENT_BACK */
				SDLK_ESCAPE		/* EVENT_QUIT */
			};

			emulator->id = 0;
			strncpy( emulator->name, "mame", CONFIG_NAME_LENGTH );
			strncpy( emulator->display_name, "MAME", CONFIG_NAME_LENGTH );
			strncpy( emulator->executable, "mame", CONFIG_FILE_NAME_LENGTH );
			emulator->next = NULL;
			config->emulators = emulator;
			
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
	
			config->games = NULL;
			config->genres = NULL;
			config->platforms = NULL;
	
			config->iface.full_screen = 1;
			config->iface.screen_width = 640;
			config->iface.screen_height = 480;
			
			for( i = 0 ; i < NUM_EVENTS ; i++ ) {
				struct config_control *control = malloc( sizeof(struct config_control) );
				if( control == NULL ) {
					fprintf( stderr, "Error: couldn't allocate control structure\n" );
					return -1;
				}
				else {
					memset( control, 0, sizeof(struct config_control) );
					control->event = i+1;
					control->device_type = DEV_KEYBOARD;
					control->value = keys[i];
					control->next = prev_control;
					prev_control = control;
				}
				config->iface.controls = control;
			}
		}
	}
	return 0;
}

int config_create( const char *home ) {
	char dirname[CONFIG_FILE_NAME_LENGTH];
	DIR *dir;
	
	/* Check if directory exists and attempt to create if not */
	snprintf( dirname, CONFIG_FILE_NAME_LENGTH, "%s/%s", home, config_default_dir );
	dir = opendir( dirname );
	if( dir == NULL ) {
		switch( errno ) {
			case ENOTDIR:
				fprintf( stderr, "Warning: Can't read config directory '%s': no such file or directory\n", dirname );
				return -1;
				break;
			case ENOENT:
				if( mkdir( dirname, 0755 ) != 0 ) {
					fprintf( stderr, "Warning: Can't create default config directory '%s'\n", dirname );
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

int config_init( const char *filename ) {
	if( config_new() != 0 ) {
		fprintf( stderr, "Error: Config initialisation failed\n" );
		return -1;
	}

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
		struct passwd *passwd = getpwuid(getuid());
		FILE *file;

		if( passwd == NULL ) {
			fprintf( stderr, "Error: Couldn't fetch user's home directory\n" );
			return -1;	
		}
	
		snprintf( config_filename, CONFIG_FILE_NAME_LENGTH, "%s/%s/%s", passwd->pw_dir, config_default_dir, config_default_file );
		file = fopen( config_filename, "r" );
		if( file == NULL ) {
			switch( errno ) {
				case EACCES:
					fprintf( stderr, "Error: Can't read config file '%s': access denied\n", config_filename );
					return -1;
					break;
				case ENOENT:
					/* Try to create a default configuration file */
					if( config_create( passwd->pw_dir ) != 0 ) {
						fprintf( stderr, "Error: Can't create default config file '%s'\n", config_filename );
						return -1;
					}
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
			int retval = config_read( config_root );
			xmlFreeDoc( config_doc );
			return retval;
		}
	}
	return -1;
}

int config_key( char *name ) {
	if( name == NULL ) {
		fprintf( stderr, "Warning: Null key name\n" );		
		return -1;	
	}
	if( strcasecmp( name, config_key_backspace ) == 0 )return SDLK_BACKSPACE;
	else if( strcasecmp( name, config_key_tab ) == 0 )return SDLK_TAB;
	else if( strcasecmp( name, config_key_clear ) == 0 )return SDLK_CLEAR;
	else if( strcasecmp( name, config_key_return ) == 0 )return SDLK_RETURN;
	else if( strcasecmp( name, config_key_pause ) == 0 )return SDLK_PAUSE;
	else if( strcasecmp( name, config_key_escape ) == 0 )return SDLK_ESCAPE;
	else if( strcasecmp( name, config_key_space ) == 0 )return SDLK_SPACE;
	else if( strcasecmp( name, config_key_exclaim ) == 0 )return SDLK_EXCLAIM;
	else if( strcasecmp( name, config_key_quotedbl ) == 0 )return SDLK_QUOTEDBL;
	else if( strcasecmp( name, config_key_hash ) == 0 )return SDLK_HASH;
	else if( strcasecmp( name, config_key_dollar ) == 0 )return SDLK_DOLLAR;
	else if( strcasecmp( name, config_key_ampersand ) == 0 )return SDLK_AMPERSAND;
	else if( strcasecmp( name, config_key_quote ) == 0 )return SDLK_QUOTE;
	else if( strcasecmp( name, config_key_leftparen ) == 0 )return SDLK_LEFTPAREN;
	else if( strcasecmp( name, config_key_rightparen ) == 0 )return SDLK_RIGHTPAREN;
	else if( strcasecmp( name, config_key_asterisk ) == 0 )return SDLK_ASTERISK;
	else if( strcasecmp( name, config_key_plus ) == 0 )return SDLK_PLUS;
	else if( strcasecmp( name, config_key_comma ) == 0 )return SDLK_COMMA;
	else if( strcasecmp( name, config_key_minus ) == 0 )return SDLK_MINUS;
	else if( strcasecmp( name, config_key_period ) == 0 )return SDLK_PERIOD;
	else if( strcasecmp( name, config_key_slash ) == 0 )return SDLK_SLASH;
	else if( strcasecmp( name, config_key_0 ) == 0 )return SDLK_0;
	else if( strcasecmp( name, config_key_1 ) == 0 )return SDLK_1;
	else if( strcasecmp( name, config_key_2 ) == 0 )return SDLK_2;
	else if( strcasecmp( name, config_key_3 ) == 0 )return SDLK_3;
	else if( strcasecmp( name, config_key_4 ) == 0 )return SDLK_4;
	else if( strcasecmp( name, config_key_5 ) == 0 )return SDLK_5;
	else if( strcasecmp( name, config_key_6 ) == 0 )return SDLK_6;
	else if( strcasecmp( name, config_key_7 ) == 0 )return SDLK_7;
	else if( strcasecmp( name, config_key_8 ) == 0 )return SDLK_8;
	else if( strcasecmp( name, config_key_9 ) == 0 )return SDLK_9;
	else if( strcasecmp( name, config_key_colon ) == 0 )return SDLK_COLON;
	else if( strcasecmp( name, config_key_semicolon ) == 0 )return SDLK_SEMICOLON;
	else if( strcasecmp( name, config_key_less ) == 0 )return SDLK_LESS;
	else if( strcasecmp( name, config_key_equals ) == 0 )return SDLK_EQUALS;
	else if( strcasecmp( name, config_key_greater ) == 0 )return SDLK_GREATER;
	else if( strcasecmp( name, config_key_question ) == 0 )return SDLK_QUESTION;
	else if( strcasecmp( name, config_key_at ) == 0 )return SDLK_AT;
	else if( strcasecmp( name, config_key_leftbracket ) == 0 )return SDLK_LEFTBRACKET;
	else if( strcasecmp( name, config_key_backslash ) == 0 )return SDLK_BACKSLASH;
	else if( strcasecmp( name, config_key_rightbracket ) == 0 )return SDLK_RIGHTBRACKET;
	else if( strcasecmp( name, config_key_caret ) == 0 )return SDLK_CARET;
	else if( strcasecmp( name, config_key_underscore ) == 0 )return SDLK_UNDERSCORE;
	else if( strcasecmp( name, config_key_backquote ) == 0 )return SDLK_BACKQUOTE;
	else if( strcasecmp( name, config_key_a ) == 0 )return SDLK_a;
	else if( strcasecmp( name, config_key_b ) == 0 )return SDLK_b;
	else if( strcasecmp( name, config_key_c ) == 0 )return SDLK_c;
	else if( strcasecmp( name, config_key_d ) == 0 )return SDLK_d;
	else if( strcasecmp( name, config_key_e ) == 0 )return SDLK_e;
	else if( strcasecmp( name, config_key_f ) == 0 )return SDLK_f;
	else if( strcasecmp( name, config_key_g ) == 0 )return SDLK_g;
	else if( strcasecmp( name, config_key_h ) == 0 )return SDLK_h;
	else if( strcasecmp( name, config_key_i ) == 0 )return SDLK_i;
	else if( strcasecmp( name, config_key_j ) == 0 )return SDLK_j;
	else if( strcasecmp( name, config_key_k ) == 0 )return SDLK_k;
	else if( strcasecmp( name, config_key_l ) == 0 )return SDLK_l;
	else if( strcasecmp( name, config_key_m ) == 0 )return SDLK_m;
	else if( strcasecmp( name, config_key_n ) == 0 )return SDLK_n;
	else if( strcasecmp( name, config_key_o ) == 0 )return SDLK_o;
	else if( strcasecmp( name, config_key_p ) == 0 )return SDLK_p;
	else if( strcasecmp( name, config_key_q ) == 0 )return SDLK_q;
	else if( strcasecmp( name, config_key_r ) == 0 )return SDLK_r;
	else if( strcasecmp( name, config_key_s ) == 0 )return SDLK_s;
	else if( strcasecmp( name, config_key_t ) == 0 )return SDLK_t;
	else if( strcasecmp( name, config_key_u ) == 0 )return SDLK_u;
	else if( strcasecmp( name, config_key_v ) == 0 )return SDLK_v;
	else if( strcasecmp( name, config_key_w ) == 0 )return SDLK_w;
	else if( strcasecmp( name, config_key_x ) == 0 )return SDLK_x;
	else if( strcasecmp( name, config_key_y ) == 0 )return SDLK_y;
	else if( strcasecmp( name, config_key_z ) == 0 )return SDLK_z;
	else if( strcasecmp( name, config_key_delete ) == 0 )return SDLK_DELETE;
	else if( strcasecmp( name, config_key_world_0 ) == 0 )return SDLK_WORLD_0;
	else if( strcasecmp( name, config_key_world_1 ) == 0 )return SDLK_WORLD_1;
	else if( strcasecmp( name, config_key_world_2 ) == 0 )return SDLK_WORLD_2;
	else if( strcasecmp( name, config_key_world_3 ) == 0 )return SDLK_WORLD_3;
	else if( strcasecmp( name, config_key_world_4 ) == 0 )return SDLK_WORLD_4;
	else if( strcasecmp( name, config_key_world_5 ) == 0 )return SDLK_WORLD_5;
	else if( strcasecmp( name, config_key_world_6 ) == 0 )return SDLK_WORLD_6;
	else if( strcasecmp( name, config_key_world_7 ) == 0 )return SDLK_WORLD_7;
	else if( strcasecmp( name, config_key_world_8 ) == 0 )return SDLK_WORLD_8;
	else if( strcasecmp( name, config_key_world_9 ) == 0 )return SDLK_WORLD_9;
	else if( strcasecmp( name, config_key_world_10 ) == 0 )return SDLK_WORLD_10;
	else if( strcasecmp( name, config_key_world_11 ) == 0 )return SDLK_WORLD_11;
	else if( strcasecmp( name, config_key_world_12 ) == 0 )return SDLK_WORLD_12;
	else if( strcasecmp( name, config_key_world_13 ) == 0 )return SDLK_WORLD_13;
	else if( strcasecmp( name, config_key_world_14 ) == 0 )return SDLK_WORLD_14;
	else if( strcasecmp( name, config_key_world_15 ) == 0 )return SDLK_WORLD_15;
	else if( strcasecmp( name, config_key_world_16 ) == 0 )return SDLK_WORLD_16;
	else if( strcasecmp( name, config_key_world_17 ) == 0 )return SDLK_WORLD_17;
	else if( strcasecmp( name, config_key_world_18 ) == 0 )return SDLK_WORLD_18;
	else if( strcasecmp( name, config_key_world_19 ) == 0 )return SDLK_WORLD_19;
	else if( strcasecmp( name, config_key_world_20 ) == 0 )return SDLK_WORLD_20;
	else if( strcasecmp( name, config_key_world_21 ) == 0 )return SDLK_WORLD_21;
	else if( strcasecmp( name, config_key_world_22 ) == 0 )return SDLK_WORLD_22;
	else if( strcasecmp( name, config_key_world_23 ) == 0 )return SDLK_WORLD_23;
	else if( strcasecmp( name, config_key_world_24 ) == 0 )return SDLK_WORLD_24;
	else if( strcasecmp( name, config_key_world_25 ) == 0 )return SDLK_WORLD_25;
	else if( strcasecmp( name, config_key_world_26 ) == 0 )return SDLK_WORLD_26;
	else if( strcasecmp( name, config_key_world_27 ) == 0 )return SDLK_WORLD_27;
	else if( strcasecmp( name, config_key_world_28 ) == 0 )return SDLK_WORLD_28;
	else if( strcasecmp( name, config_key_world_29 ) == 0 )return SDLK_WORLD_29;
	else if( strcasecmp( name, config_key_world_30 ) == 0 )return SDLK_WORLD_30;
	else if( strcasecmp( name, config_key_world_31 ) == 0 )return SDLK_WORLD_31;
	else if( strcasecmp( name, config_key_world_32 ) == 0 )return SDLK_WORLD_32;
	else if( strcasecmp( name, config_key_world_33 ) == 0 )return SDLK_WORLD_33;
	else if( strcasecmp( name, config_key_world_34 ) == 0 )return SDLK_WORLD_34;
	else if( strcasecmp( name, config_key_world_35 ) == 0 )return SDLK_WORLD_35;
	else if( strcasecmp( name, config_key_world_36 ) == 0 )return SDLK_WORLD_36;
	else if( strcasecmp( name, config_key_world_37 ) == 0 )return SDLK_WORLD_37;
	else if( strcasecmp( name, config_key_world_38 ) == 0 )return SDLK_WORLD_38;
	else if( strcasecmp( name, config_key_world_39 ) == 0 )return SDLK_WORLD_39;
	else if( strcasecmp( name, config_key_world_40 ) == 0 )return SDLK_WORLD_40;
	else if( strcasecmp( name, config_key_world_41 ) == 0 )return SDLK_WORLD_41;
	else if( strcasecmp( name, config_key_world_42 ) == 0 )return SDLK_WORLD_42;
	else if( strcasecmp( name, config_key_world_43 ) == 0 )return SDLK_WORLD_43;
	else if( strcasecmp( name, config_key_world_44 ) == 0 )return SDLK_WORLD_44;
	else if( strcasecmp( name, config_key_world_45 ) == 0 )return SDLK_WORLD_45;
	else if( strcasecmp( name, config_key_world_46 ) == 0 )return SDLK_WORLD_46;
	else if( strcasecmp( name, config_key_world_47 ) == 0 )return SDLK_WORLD_47;
	else if( strcasecmp( name, config_key_world_48 ) == 0 )return SDLK_WORLD_48;
	else if( strcasecmp( name, config_key_world_49 ) == 0 )return SDLK_WORLD_49;
	else if( strcasecmp( name, config_key_world_50 ) == 0 )return SDLK_WORLD_50;
	else if( strcasecmp( name, config_key_world_51 ) == 0 )return SDLK_WORLD_51;
	else if( strcasecmp( name, config_key_world_52 ) == 0 )return SDLK_WORLD_52;
	else if( strcasecmp( name, config_key_world_53 ) == 0 )return SDLK_WORLD_53;
	else if( strcasecmp( name, config_key_world_54 ) == 0 )return SDLK_WORLD_54;
	else if( strcasecmp( name, config_key_world_55 ) == 0 )return SDLK_WORLD_55;
	else if( strcasecmp( name, config_key_world_56 ) == 0 )return SDLK_WORLD_56;
	else if( strcasecmp( name, config_key_world_57 ) == 0 )return SDLK_WORLD_57;
	else if( strcasecmp( name, config_key_world_58 ) == 0 )return SDLK_WORLD_58;
	else if( strcasecmp( name, config_key_world_59 ) == 0 )return SDLK_WORLD_59;
	else if( strcasecmp( name, config_key_world_60 ) == 0 )return SDLK_WORLD_60;
	else if( strcasecmp( name, config_key_world_61 ) == 0 )return SDLK_WORLD_61;
	else if( strcasecmp( name, config_key_world_62 ) == 0 )return SDLK_WORLD_62;
	else if( strcasecmp( name, config_key_world_63 ) == 0 )return SDLK_WORLD_63;
	else if( strcasecmp( name, config_key_world_64 ) == 0 )return SDLK_WORLD_64;
	else if( strcasecmp( name, config_key_world_65 ) == 0 )return SDLK_WORLD_65;
	else if( strcasecmp( name, config_key_world_66 ) == 0 )return SDLK_WORLD_66;
	else if( strcasecmp( name, config_key_world_67 ) == 0 )return SDLK_WORLD_67;
	else if( strcasecmp( name, config_key_world_68 ) == 0 )return SDLK_WORLD_68;
	else if( strcasecmp( name, config_key_world_69 ) == 0 )return SDLK_WORLD_69;
	else if( strcasecmp( name, config_key_world_70 ) == 0 )return SDLK_WORLD_70;
	else if( strcasecmp( name, config_key_world_71 ) == 0 )return SDLK_WORLD_71;
	else if( strcasecmp( name, config_key_world_72 ) == 0 )return SDLK_WORLD_72;
	else if( strcasecmp( name, config_key_world_73 ) == 0 )return SDLK_WORLD_73;
	else if( strcasecmp( name, config_key_world_74 ) == 0 )return SDLK_WORLD_74;
	else if( strcasecmp( name, config_key_world_75 ) == 0 )return SDLK_WORLD_75;
	else if( strcasecmp( name, config_key_world_76 ) == 0 )return SDLK_WORLD_76;
	else if( strcasecmp( name, config_key_world_77 ) == 0 )return SDLK_WORLD_77;
	else if( strcasecmp( name, config_key_world_78 ) == 0 )return SDLK_WORLD_78;
	else if( strcasecmp( name, config_key_world_79 ) == 0 )return SDLK_WORLD_79;
	else if( strcasecmp( name, config_key_world_80 ) == 0 )return SDLK_WORLD_80;
	else if( strcasecmp( name, config_key_world_81 ) == 0 )return SDLK_WORLD_81;
	else if( strcasecmp( name, config_key_world_82 ) == 0 )return SDLK_WORLD_82;
	else if( strcasecmp( name, config_key_world_83 ) == 0 )return SDLK_WORLD_83;
	else if( strcasecmp( name, config_key_world_84 ) == 0 )return SDLK_WORLD_84;
	else if( strcasecmp( name, config_key_world_85 ) == 0 )return SDLK_WORLD_85;
	else if( strcasecmp( name, config_key_world_86 ) == 0 )return SDLK_WORLD_86;
	else if( strcasecmp( name, config_key_world_87 ) == 0 )return SDLK_WORLD_87;
	else if( strcasecmp( name, config_key_world_88 ) == 0 )return SDLK_WORLD_88;
	else if( strcasecmp( name, config_key_world_89 ) == 0 )return SDLK_WORLD_89;
	else if( strcasecmp( name, config_key_world_90 ) == 0 )return SDLK_WORLD_90;
	else if( strcasecmp( name, config_key_world_91 ) == 0 )return SDLK_WORLD_91;
	else if( strcasecmp( name, config_key_world_92 ) == 0 )return SDLK_WORLD_92;
	else if( strcasecmp( name, config_key_world_93 ) == 0 )return SDLK_WORLD_93;
	else if( strcasecmp( name, config_key_world_94 ) == 0 )return SDLK_WORLD_94;
	else if( strcasecmp( name, config_key_world_95 ) == 0 )return SDLK_WORLD_95;
	else if( strcasecmp( name, config_key_kp0 ) == 0 )return SDLK_KP0;
	else if( strcasecmp( name, config_key_kp1 ) == 0 )return SDLK_KP1;
	else if( strcasecmp( name, config_key_kp2 ) == 0 )return SDLK_KP2;
	else if( strcasecmp( name, config_key_kp3 ) == 0 )return SDLK_KP3;
	else if( strcasecmp( name, config_key_kp4 ) == 0 )return SDLK_KP4;
	else if( strcasecmp( name, config_key_kp5 ) == 0 )return SDLK_KP5;
	else if( strcasecmp( name, config_key_kp6 ) == 0 )return SDLK_KP6;
	else if( strcasecmp( name, config_key_kp7 ) == 0 )return SDLK_KP7;
	else if( strcasecmp( name, config_key_kp8 ) == 0 )return SDLK_KP8;
	else if( strcasecmp( name, config_key_kp9 ) == 0 )return SDLK_KP9;
	else if( strcasecmp( name, config_key_kp_period ) == 0 )return SDLK_KP_PERIOD;
	else if( strcasecmp( name, config_key_kp_divide ) == 0 )return SDLK_KP_DIVIDE;
	else if( strcasecmp( name, config_key_kp_multiply ) == 0 )return SDLK_KP_MULTIPLY;
	else if( strcasecmp( name, config_key_kp_minus ) == 0 )return SDLK_KP_MINUS;
	else if( strcasecmp( name, config_key_kp_plus ) == 0 )return SDLK_KP_PLUS;
	else if( strcasecmp( name, config_key_kp_enter ) == 0 )return SDLK_KP_ENTER;
	else if( strcasecmp( name, config_key_kp_equals ) == 0 )return SDLK_KP_EQUALS;
	else if( strcasecmp( name, config_key_up ) == 0 )return SDLK_UP;
	else if( strcasecmp( name, config_key_down ) == 0 )return SDLK_DOWN;
	else if( strcasecmp( name, config_key_right ) == 0 )return SDLK_RIGHT;
	else if( strcasecmp( name, config_key_left ) == 0 )return SDLK_LEFT;
	else if( strcasecmp( name, config_key_insert ) == 0 )return SDLK_INSERT;
	else if( strcasecmp( name, config_key_home ) == 0 )return SDLK_HOME;
	else if( strcasecmp( name, config_key_end ) == 0 )return SDLK_END;
	else if( strcasecmp( name, config_key_pageup ) == 0 )return SDLK_PAGEUP;
	else if( strcasecmp( name, config_key_pagedown ) == 0 )return SDLK_PAGEDOWN;
	else if( strcasecmp( name, config_key_f1 ) == 0 )return SDLK_F1;
	else if( strcasecmp( name, config_key_f2 ) == 0 )return SDLK_F2;
	else if( strcasecmp( name, config_key_f3 ) == 0 )return SDLK_F3;
	else if( strcasecmp( name, config_key_f4 ) == 0 )return SDLK_F4;
	else if( strcasecmp( name, config_key_f5 ) == 0 )return SDLK_F5;
	else if( strcasecmp( name, config_key_f6 ) == 0 )return SDLK_F6;
	else if( strcasecmp( name, config_key_f7 ) == 0 )return SDLK_F7;
	else if( strcasecmp( name, config_key_f8 ) == 0 )return SDLK_F8;
	else if( strcasecmp( name, config_key_f9 ) == 0 )return SDLK_F9;
	else if( strcasecmp( name, config_key_f10 ) == 0 )return SDLK_F10;
	else if( strcasecmp( name, config_key_f11 ) == 0 )return SDLK_F11;
	else if( strcasecmp( name, config_key_f12 ) == 0 )return SDLK_F12;
	else if( strcasecmp( name, config_key_f13 ) == 0 )return SDLK_F13;
	else if( strcasecmp( name, config_key_f14 ) == 0 )return SDLK_F14;
	else if( strcasecmp( name, config_key_f15 ) == 0 )return SDLK_F15;
	else if( strcasecmp( name, config_key_numlock ) == 0 )return SDLK_NUMLOCK;
	else if( strcasecmp( name, config_key_capslock ) == 0 )return SDLK_CAPSLOCK;
	else if( strcasecmp( name, config_key_scrollock ) == 0 )return SDLK_SCROLLOCK;
	else if( strcasecmp( name, config_key_rshift ) == 0 )return SDLK_RSHIFT;
	else if( strcasecmp( name, config_key_lshift ) == 0 )return SDLK_LSHIFT;
	else if( strcasecmp( name, config_key_rctrl ) == 0 )return SDLK_RCTRL;
	else if( strcasecmp( name, config_key_lctrl ) == 0 )return SDLK_LCTRL;
	else if( strcasecmp( name, config_key_ralt ) == 0 )return SDLK_RALT;
	else if( strcasecmp( name, config_key_lalt ) == 0 )return SDLK_LALT;
	else if( strcasecmp( name, config_key_rmeta ) == 0 )return SDLK_RMETA;
	else if( strcasecmp( name, config_key_lmeta ) == 0 )return SDLK_LMETA;
	else if( strcasecmp( name, config_key_lsuper ) == 0 )return SDLK_LSUPER;
	else if( strcasecmp( name, config_key_rsuper ) == 0 )return SDLK_RSUPER;
	else if( strcasecmp( name, config_key_mode ) == 0 )return SDLK_MODE;
	else if( strcasecmp( name, config_key_compose ) == 0 )return SDLK_COMPOSE;
	else if( strcasecmp( name, config_key_help ) == 0 )return SDLK_HELP;
	else if( strcasecmp( name, config_key_print ) == 0 )return SDLK_PRINT;
	else if( strcasecmp( name, config_key_sysreq ) == 0 )return SDLK_SYSREQ;
	else if( strcasecmp( name, config_key_break ) == 0 )return SDLK_BREAK;
	else if( strcasecmp( name, config_key_menu ) == 0 )return SDLK_MENU;
	else if( strcasecmp( name, config_key_power ) == 0 )return SDLK_POWER;
	else if( strcasecmp( name, config_key_euro ) == 0 )return SDLK_EURO;
	else if( strcasecmp( name, config_key_undo ) == 0 )return SDLK_UNDO;
	return 0;
}

const char *config_key_name( int key ) {
	switch( key ) {
		case SDLK_BACKSPACE: return config_key_backspace;
		case SDLK_TAB: return config_key_tab;
		case SDLK_CLEAR: return config_key_clear;
		case SDLK_RETURN: return config_key_return;
		case SDLK_PAUSE: return config_key_pause;
		case SDLK_ESCAPE: return config_key_escape;
		case SDLK_SPACE: return config_key_space;
		case SDLK_EXCLAIM: return config_key_exclaim;
		case SDLK_QUOTEDBL: return config_key_quotedbl;
		case SDLK_HASH: return config_key_hash;
		case SDLK_DOLLAR: return config_key_dollar;
		case SDLK_AMPERSAND: return config_key_ampersand;
		case SDLK_QUOTE: return config_key_quote;
		case SDLK_LEFTPAREN: return config_key_leftparen;
		case SDLK_RIGHTPAREN: return config_key_rightparen;
		case SDLK_ASTERISK: return config_key_asterisk;
		case SDLK_PLUS: return config_key_plus;
		case SDLK_COMMA: return config_key_comma;
		case SDLK_MINUS: return config_key_minus;
		case SDLK_PERIOD: return config_key_period;
		case SDLK_SLASH: return config_key_slash;
		case SDLK_0: return config_key_0;
		case SDLK_1: return config_key_1;
		case SDLK_2: return config_key_2;
		case SDLK_3: return config_key_3;
		case SDLK_4: return config_key_4;
		case SDLK_5: return config_key_5;
		case SDLK_6: return config_key_6;
		case SDLK_7: return config_key_7;
		case SDLK_8: return config_key_8;
		case SDLK_9: return config_key_9;
		case SDLK_COLON: return config_key_colon;
		case SDLK_SEMICOLON: return config_key_semicolon;
		case SDLK_LESS: return config_key_less;
		case SDLK_EQUALS: return config_key_equals;
		case SDLK_GREATER: return config_key_greater;
		case SDLK_QUESTION: return config_key_question;
		case SDLK_AT: return config_key_at;
		case SDLK_LEFTBRACKET: return config_key_leftbracket;
		case SDLK_BACKSLASH: return config_key_backslash;
		case SDLK_RIGHTBRACKET: return config_key_rightbracket;
		case SDLK_CARET: return config_key_caret;
		case SDLK_UNDERSCORE: return config_key_underscore;
		case SDLK_BACKQUOTE: return config_key_backquote;
		case SDLK_a: return config_key_a;
		case SDLK_b: return config_key_b;
		case SDLK_c: return config_key_c;
		case SDLK_d: return config_key_d;
		case SDLK_e: return config_key_e;
		case SDLK_f: return config_key_f;
		case SDLK_g: return config_key_g;
		case SDLK_h: return config_key_h;
		case SDLK_i: return config_key_i;
		case SDLK_j: return config_key_j;
		case SDLK_k: return config_key_k;
		case SDLK_l: return config_key_l;
		case SDLK_m: return config_key_m;
		case SDLK_n: return config_key_n;
		case SDLK_o: return config_key_o;
		case SDLK_p: return config_key_p;
		case SDLK_q: return config_key_q;
		case SDLK_r: return config_key_r;
		case SDLK_s: return config_key_s;
		case SDLK_t: return config_key_t;
		case SDLK_u: return config_key_u;
		case SDLK_v: return config_key_v;
		case SDLK_w: return config_key_w;
		case SDLK_x: return config_key_x;
		case SDLK_y: return config_key_y;
		case SDLK_z: return config_key_z;
		case SDLK_DELETE: return config_key_delete;
		case SDLK_WORLD_0: return config_key_world_0;
		case SDLK_WORLD_1: return config_key_world_1;
		case SDLK_WORLD_2: return config_key_world_2;
		case SDLK_WORLD_3: return config_key_world_3;
		case SDLK_WORLD_4: return config_key_world_4;
		case SDLK_WORLD_5: return config_key_world_5;
		case SDLK_WORLD_6: return config_key_world_6;
		case SDLK_WORLD_7: return config_key_world_7;
		case SDLK_WORLD_8: return config_key_world_8;
		case SDLK_WORLD_9: return config_key_world_9;
		case SDLK_WORLD_10: return config_key_world_10;
		case SDLK_WORLD_11: return config_key_world_11;
		case SDLK_WORLD_12: return config_key_world_12;
		case SDLK_WORLD_13: return config_key_world_13;
		case SDLK_WORLD_14: return config_key_world_14;
		case SDLK_WORLD_15: return config_key_world_15;
		case SDLK_WORLD_16: return config_key_world_16;
		case SDLK_WORLD_17: return config_key_world_17;
		case SDLK_WORLD_18: return config_key_world_18;
		case SDLK_WORLD_19: return config_key_world_19;
		case SDLK_WORLD_20: return config_key_world_20;
		case SDLK_WORLD_21: return config_key_world_21;
		case SDLK_WORLD_22: return config_key_world_22;
		case SDLK_WORLD_23: return config_key_world_23;
		case SDLK_WORLD_24: return config_key_world_24;
		case SDLK_WORLD_25: return config_key_world_25;
		case SDLK_WORLD_26: return config_key_world_26;
		case SDLK_WORLD_27: return config_key_world_27;
		case SDLK_WORLD_28: return config_key_world_28;
		case SDLK_WORLD_29: return config_key_world_29;
		case SDLK_WORLD_30: return config_key_world_30;
		case SDLK_WORLD_31: return config_key_world_31;
		case SDLK_WORLD_32: return config_key_world_32;
		case SDLK_WORLD_33: return config_key_world_33;
		case SDLK_WORLD_34: return config_key_world_34;
		case SDLK_WORLD_35: return config_key_world_35;
		case SDLK_WORLD_36: return config_key_world_36;
		case SDLK_WORLD_37: return config_key_world_37;
		case SDLK_WORLD_38: return config_key_world_38;
		case SDLK_WORLD_39: return config_key_world_39;
		case SDLK_WORLD_40: return config_key_world_40;
		case SDLK_WORLD_41: return config_key_world_41;
		case SDLK_WORLD_42: return config_key_world_42;
		case SDLK_WORLD_43: return config_key_world_43;
		case SDLK_WORLD_44: return config_key_world_44;
		case SDLK_WORLD_45: return config_key_world_45;
		case SDLK_WORLD_46: return config_key_world_46;
		case SDLK_WORLD_47: return config_key_world_47;
		case SDLK_WORLD_48: return config_key_world_48;
		case SDLK_WORLD_49: return config_key_world_49;
		case SDLK_WORLD_50: return config_key_world_50;
		case SDLK_WORLD_51: return config_key_world_51;
		case SDLK_WORLD_52: return config_key_world_52;
		case SDLK_WORLD_53: return config_key_world_53;
		case SDLK_WORLD_54: return config_key_world_54;
		case SDLK_WORLD_55: return config_key_world_55;
		case SDLK_WORLD_56: return config_key_world_56;
		case SDLK_WORLD_57: return config_key_world_57;
		case SDLK_WORLD_58: return config_key_world_58;
		case SDLK_WORLD_59: return config_key_world_59;
		case SDLK_WORLD_60: return config_key_world_60;
		case SDLK_WORLD_61: return config_key_world_61;
		case SDLK_WORLD_62: return config_key_world_62;
		case SDLK_WORLD_63: return config_key_world_63;
		case SDLK_WORLD_64: return config_key_world_64;
		case SDLK_WORLD_65: return config_key_world_65;
		case SDLK_WORLD_66: return config_key_world_66;
		case SDLK_WORLD_67: return config_key_world_67;
		case SDLK_WORLD_68: return config_key_world_68;
		case SDLK_WORLD_69: return config_key_world_69;
		case SDLK_WORLD_70: return config_key_world_70;
		case SDLK_WORLD_71: return config_key_world_71;
		case SDLK_WORLD_72: return config_key_world_72;
		case SDLK_WORLD_73: return config_key_world_73;
		case SDLK_WORLD_74: return config_key_world_74;
		case SDLK_WORLD_75: return config_key_world_75;
		case SDLK_WORLD_76: return config_key_world_76;
		case SDLK_WORLD_77: return config_key_world_77;
		case SDLK_WORLD_78: return config_key_world_78;
		case SDLK_WORLD_79: return config_key_world_79;
		case SDLK_WORLD_80: return config_key_world_80;
		case SDLK_WORLD_81: return config_key_world_81;
		case SDLK_WORLD_82: return config_key_world_82;
		case SDLK_WORLD_83: return config_key_world_83;
		case SDLK_WORLD_84: return config_key_world_84;
		case SDLK_WORLD_85: return config_key_world_85;
		case SDLK_WORLD_86: return config_key_world_86;
		case SDLK_WORLD_87: return config_key_world_87;
		case SDLK_WORLD_88: return config_key_world_88;
		case SDLK_WORLD_89: return config_key_world_89;
		case SDLK_WORLD_90: return config_key_world_90;
		case SDLK_WORLD_91: return config_key_world_91;
		case SDLK_WORLD_92: return config_key_world_92;
		case SDLK_WORLD_93: return config_key_world_93;
		case SDLK_WORLD_94: return config_key_world_94;
		case SDLK_WORLD_95: return config_key_world_95;
		case SDLK_KP0: return config_key_kp0;
		case SDLK_KP1: return config_key_kp1;
		case SDLK_KP2: return config_key_kp2;
		case SDLK_KP3: return config_key_kp3;
		case SDLK_KP4: return config_key_kp4;
		case SDLK_KP5: return config_key_kp5;
		case SDLK_KP6: return config_key_kp6;
		case SDLK_KP7: return config_key_kp7;
		case SDLK_KP8: return config_key_kp8;
		case SDLK_KP9: return config_key_kp9;
		case SDLK_KP_PERIOD: return config_key_kp_period;
		case SDLK_KP_DIVIDE: return config_key_kp_divide;
		case SDLK_KP_MULTIPLY: return config_key_kp_multiply;
		case SDLK_KP_MINUS: return config_key_kp_minus;
		case SDLK_KP_PLUS: return config_key_kp_plus;
		case SDLK_KP_ENTER: return config_key_kp_enter;
		case SDLK_KP_EQUALS: return config_key_kp_equals;
		case SDLK_UP: return config_key_up;
		case SDLK_DOWN: return config_key_down;
		case SDLK_RIGHT: return config_key_right;
		case SDLK_LEFT: return config_key_left;
		case SDLK_INSERT: return config_key_insert;
		case SDLK_HOME: return config_key_home;
		case SDLK_END: return config_key_end;
		case SDLK_PAGEUP: return config_key_pageup;
		case SDLK_PAGEDOWN: return config_key_pagedown;
		case SDLK_F1: return config_key_f1;
		case SDLK_F2: return config_key_f2;
		case SDLK_F3: return config_key_f3;
		case SDLK_F4: return config_key_f4;
		case SDLK_F5: return config_key_f5;
		case SDLK_F6: return config_key_f6;
		case SDLK_F7: return config_key_f7;
		case SDLK_F8: return config_key_f8;
		case SDLK_F9: return config_key_f9;
		case SDLK_F10: return config_key_f10;
		case SDLK_F11: return config_key_f11;
		case SDLK_F12: return config_key_f12;
		case SDLK_F13: return config_key_f13;
		case SDLK_F14: return config_key_f14;
		case SDLK_F15: return config_key_f15;
		case SDLK_NUMLOCK: return config_key_numlock;
		case SDLK_CAPSLOCK: return config_key_capslock;
		case SDLK_SCROLLOCK: return config_key_scrollock;
		case SDLK_RSHIFT: return config_key_rshift;
		case SDLK_LSHIFT: return config_key_lshift;
		case SDLK_RCTRL: return config_key_rctrl;
		case SDLK_LCTRL: return config_key_lctrl;
		case SDLK_RALT: return config_key_ralt;
		case SDLK_LALT: return config_key_lalt;
		case SDLK_RMETA: return config_key_rmeta;
		case SDLK_LMETA: return config_key_lmeta;
		case SDLK_LSUPER: return config_key_lsuper;
		case SDLK_RSUPER: return config_key_rsuper;
		case SDLK_MODE: return config_key_mode;
		case SDLK_COMPOSE: return config_key_compose;
		case SDLK_HELP: return config_key_help;
		case SDLK_PRINT: return config_key_print;
		case SDLK_SYSREQ: return config_key_sysreq;
		case SDLK_BREAK: return config_key_break;
		case SDLK_MENU: return config_key_menu;
		case SDLK_POWER: return config_key_power;
		case SDLK_EURO: return config_key_euro;
		case SDLK_UNDO: return config_key_undo;
	}
	return NULL;
}

