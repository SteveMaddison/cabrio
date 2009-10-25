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

struct config_genre {
	struct config_genre *next;
	char name[CONFIG_NAME_LENGTH];
};

struct config_game {
	struct config_game *next;
	char name[CONFIG_NAME_LENGTH];
	char rom_image[CONFIG_FILE_NAME_LENGTH];
	char logo_image[CONFIG_FILE_NAME_LENGTH];
	char background_image[CONFIG_FILE_NAME_LENGTH];
	struct config_param *params;
	struct config_genre *genre;
	struct config_platform *platform;
};

struct config_control {
	int device_type;
	int device_id;
	int control_type;
	int control_id;
	int value;
};

struct config_iface {
	int full_screen;
	int screen_width;
	int screen_height;
	struct config_control controls[NUM_EVENTS];
};

struct config {
	struct config_emulator *emulators;
	struct config_platform *platforms;
	struct config_genre *genres;
	struct config_game *games;
	struct config_iface iface;
};

const struct config *config_get( void );
int config_open( const char *filename );
int config_create( void );
int config_write( void );
int config_update( void );

#endif

