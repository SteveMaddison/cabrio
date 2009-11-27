#ifndef _CONFIG_H_
#define _CONFIG_H_ 1

#include <stdio.h>
#include <string.h>

#include "control.h"
#include "event.h"
#include "sound.h"

#define CONFIG_NAME_LENGTH		128
#define CONFIG_FILE_NAME_LENGTH	256
#define CONFIG_LABEL_LENGTH		32
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

struct config_rgb {
	unsigned char red;
	unsigned char green;
	unsigned char blue;
};

struct config_param {
	struct config_param *next;
	char name[CONFIG_PARAM_LENGTH];
	char value[CONFIG_PARAM_LENGTH];
};

struct config_platform {
	struct config_platform *next;
	char name[CONFIG_NAME_LENGTH];
};

struct config_emulator {
	struct config_emulator *next;
	int id;
	char name[CONFIG_NAME_LENGTH];
	char display_name[CONFIG_NAME_LENGTH];
	char executable[CONFIG_FILE_NAME_LENGTH];
	char directory[CONFIG_FILE_NAME_LENGTH];
	int is_default;
	struct config_param *params;
	struct config_platform *platform;
};

struct config_lookup {
	struct config_lookup *next;
	char match[CONFIG_LABEL_LENGTH];
	char value[CONFIG_LABEL_LENGTH];
};

struct config_category_value {
	struct config_category_value *next;
	char name[CONFIG_NAME_LENGTH];
};

struct config_category {
	struct config_category *next;
	int id;
	char name[CONFIG_NAME_LENGTH];
	struct config_category_value *values;
	struct config_lookup *lookups;
};

struct config_game_category {
	struct config_game_category *next;
	struct config_category *category;
	struct config_category_value *value;
};

struct config_image_type {
	struct config_image_type *next;
	char name[CONFIG_NAME_LENGTH];
};

struct config_image {
	struct config_image *next;
	struct config_image_type *type;
	char file_name[CONFIG_FILE_NAME_LENGTH];
};

struct config_game {
	struct config_game *next;
	char name[CONFIG_NAME_LENGTH];
	char rom_image[CONFIG_FILE_NAME_LENGTH];
	char video[CONFIG_FILE_NAME_LENGTH];
	char emulator[CONFIG_NAME_LENGTH];
	struct config_game_category *categories;
	struct config_param *params;
	struct config_platform *platform;
	struct config_image *images;
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
	float offset1;
	float offset2;
	int max_visible;
	float spacing;
	int orientation;
	int auto_hide;
	int border;
};

struct config_submenu {
	char texture[CONFIG_FILE_NAME_LENGTH];
	float item_width;
	float item_height;
	float font_scale;
	float offset1;
	float offset2;
};

struct config_game_sel_tile {
	struct config_game_sel_tile *next;
	int order;
	float pos[3];
	float angle[3];
	int transparency;
};

struct config_game_sel {
	float offset1;
	float offset2;
	float size_x;
	float size_y;
	float tile_size;
	int orientation;
	int selected;
	struct config_game_sel_tile *tiles;
};

struct config_snap {
	float offset1;
	float offset2;
	float angle_x;
	float angle_y;
	float angle_z;
	float size;
	int fix_aspect_ratio;
	int auto_hide;
	int platform_icons;
};

struct config_hints {
	float offset1;
	float offset2;
	float size;
	float spacing;
	int pulse;
	char image_back[CONFIG_FILE_NAME_LENGTH];
	char image_select[CONFIG_FILE_NAME_LENGTH];
	char image_arrow[CONFIG_FILE_NAME_LENGTH];
};

struct config_theme {
	struct config_theme *next;
	char name[CONFIG_NAME_LENGTH];
	char directory[CONFIG_FILE_NAME_LENGTH];
	char background_image[CONFIG_FILE_NAME_LENGTH];
	int background_rotation;
	int background_transparency;
	char sounds[NUM_SOUNDS][CONFIG_FILE_NAME_LENGTH];
	char font_file[CONFIG_FILE_NAME_LENGTH];
	int font_size;
	struct config_rgb font_rgb;
	struct config_menu menu;
	struct config_submenu submenu;
	struct config_game_sel game_sel;
	struct config_snap snap;
	struct config_hints hints;
};

struct config_labels {
	char label_all[CONFIG_LABEL_LENGTH];
	char label_platform[CONFIG_LABEL_LENGTH];
	char label_back[CONFIG_LABEL_LENGTH];
	char label_select[CONFIG_LABEL_LENGTH];
	char label_lists[CONFIG_LABEL_LENGTH];
};

struct config_iface {
	int full_screen;
	int screen_width;
	int screen_height;
	int screen_rotation;
	int screen_hflip;
	int screen_vflip;
	int frame_rate;
	int gfx_quality;
	int gfx_max_width;
	int gfx_max_height;
	int prune_menus;
	struct config_labels labels;
	struct config_control controls[NUM_EVENTS];
	char theme_name[CONFIG_NAME_LENGTH];
	struct config_theme theme;
};

struct config_location_type {
	struct config_location_type *next;
	char name[CONFIG_NAME_LENGTH];
};

struct config_location {
	struct config_location *next;
	struct config_location_type *type;
	char directory[CONFIG_FILE_NAME_LENGTH];
};

struct config {
	struct config_emulator *emulators;
	struct config_platform *platforms;
	struct config_category *categories;
	struct config_game *games;
	struct config_theme *themes;
	struct config_image_type *image_types;
	struct config_location *locations;
	struct config_location_type *location_types;
	struct config_iface iface;
};

const struct config *config_get( void );
int config_open( const char *filename );
int config_create( void );
int config_write( void );
int config_update( void );
struct config_image_type *config_image_type( char *name );

#endif

