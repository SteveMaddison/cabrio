#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

#include <stdio.h>
#include <string.h>

#include "control.h"
#include "event.h"

#define CONFIG_NAME_LENGTH		128
#define CONFIG_FILE_NAME_LENGTH	256
#define CONFIG_PARAM_LENGTH		64
#define CONFIG_MAX_PARAMS		32
#define CONFIG_MAX_CMD_LENGTH	2048

enum lhm_t {
	CONFIG_LOW,
	CONFIG_MEDIUM,
	CONFIG_HIGH
};

enum orient_t {
	CONFIG_LANDSCAPE,
	CONFIG_PORTRAIT
};

struct config_param {
	struct config_param *next;
	char name[CONFIG_PARAM_LENGTH];
	char value[CONFIG_PARAM_LENGTH];
};

struct config_emulator {
	struct config_emulator *next;
	int id;
	char name[CONFIG_NAME_LENGTH];
	char display_name[CONFIG_NAME_LENGTH];
	char executable[CONFIG_FILE_NAME_LENGTH];
	struct config_param *params;
};

struct config_platform {
	struct config_platform *next;
	char name[CONFIG_NAME_LENGTH];
};

struct config_category_value {
	struct config_category_value *next;
	char name[CONFIG_NAME_LENGTH];
};

struct config_category {
	struct config_category *next;
	char name[CONFIG_NAME_LENGTH];
	struct config_category_value *values;
};

struct config_game_category {
	struct config_game_category *next;
	struct config_category *category;
	struct config_category_value *value;
};

struct config_game {
	struct config_game *next;
	char name[CONFIG_NAME_LENGTH];
	char rom_image[CONFIG_FILE_NAME_LENGTH];
	char logo_image[CONFIG_FILE_NAME_LENGTH];
	char background_image[CONFIG_FILE_NAME_LENGTH];
	struct config_game_category *categories;
	struct config_param *params;
	struct config_platform *platform;
};

struct config_control {
	int device_type;
	int device_id;
	int control_type;
	int control_id;
	int value;
};

struct config_menu {
	char texture[CONFIG_FILE_NAME_LENGTH];
	float item_width;
	float item_height;
	float font_scale;
	float zoom;
	int transparency;
	float x_offset;
	float y_offset;
	int max_visible;
	float spacing;
	int orientation;
};

struct config_iface {
	int full_screen;
	int screen_width;
	int screen_height;
	int screen_rotation;
	int screen_hflip;
	int screen_vflip;
	int frame_rate;
	char background_image[CONFIG_FILE_NAME_LENGTH];
	int background_rotation;
	int background_transparency;
	char font_file[CONFIG_FILE_NAME_LENGTH];
	int font_size;
	int gfx_quality;
	int gfx_max_width;
	int gfx_max_height;
	struct config_control controls[NUM_EVENTS];
	struct config_menu menu;
};

struct config {
	struct config_emulator *emulators;
	struct config_platform *platforms;
	struct config_category *categories;
	struct config_game *games;
	struct config_iface iface;	
};

const struct config *config_get( void );
int config_open( const char *filename );
int config_create( void );
int config_write( void );
int config_update( void );

#endif

