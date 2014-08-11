#ifndef _EMULATOR_H_
#define _EMULATOR_H_ 1

#include "game.h"
#include "config.h"

int emulator_run( struct game *game );
struct config_emulator *emulator_get_by_name( const char *name );
struct config_emulator *emulator_get_by_platform( const char *platform );
struct config_emulator *emulator_get_default( void );

#endif

