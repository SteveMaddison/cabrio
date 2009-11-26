#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#ifdef __unix__
#include <sys/wait.h>
#endif
#ifdef __WIN32__
#include <windows.h>
#endif
#include "emulator.h"
#include "sdl_wrapper.h"
#include "ogl.h"
#include "platform.h"
#include "bg.h"
#include "menu.h"
#include "submenu.h"
#include "hint.h"
#include "game_sel.h"
#include "sound.h"
#include "font.h"
#include "event.h"
#include "snap.h"
#include "platform.h"


void pause_all( void ) {
	sound_pause();
	game_sel_pause();
	game_list_pause();
	snap_pause();
	submenu_pause();
	platform_pause();
	menu_pause();
	hint_pause();
	font_pause();
	bg_pause();
	event_pause();
}

int resume_all( void ) {
	if( event_resume() != 0 )
		return -1;
	bg_resume();
	if( font_resume() != 0 )
		return -3;
	if( hint_resume() != 0 )
		return -4;
	if( menu_resume() != 0 )
		return -5;
	if(	platform_resume() != 0 )
		return -6;
	if( submenu_resume() != 0 )
		return -7;
	if( game_list_resume() != 0 )
		return -8;
	sound_resume();
	if( snap_resume() != 0 )
		return -9;
	if( game_sel_resume() != 0 )
		return -10;
	
	return 0;
}

int emulator_exec( struct game *game ) {
	char *params[CONFIG_MAX_PARAMS];
	struct config_param *param = NULL;
	int i = 0;
	int count = 0;
	char current_dir[CONFIG_FILE_NAME_LENGTH];
#ifdef __WIN32__
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char cmdline[CONFIG_MAX_CMD_LENGTH];
#endif

	if( game->emulator && game->emulator->executable ) {
		param = game->emulator->params;
		while( param ) {
			if( param->name && *param->name && count < CONFIG_MAX_PARAMS - 2 )
				params[count++] = param->name;
			if( param->value && *param->value && count < CONFIG_MAX_PARAMS - 2 )
				params[count++] = param->value;
			param = param->next;
		}		
		param = game->params;
		while( param ) {
			if( param->name && *param->name && count < CONFIG_MAX_PARAMS - 2 )
				params[count++] = param->name;
			if( param->value && *param->value && count < CONFIG_MAX_PARAMS - 2 )
				params[count++] = param->value;
			param = param->next;
		}		
	}
	else {
		fprintf(stderr, "Error: No suitable emulator found in configuration\n");
		return -1;
	}
	
	if( game && game->rom_path && game->rom_path[0] ) {
		params[count++] = game->rom_path;
	}
	else {
		fprintf(stderr, "Error: No ROM image specified for game\n");
		return -1;		
	}
	
	/* If emulator provided a directory, go to it. */
	if( game->emulator->directory[0] ) {
		getcwd( current_dir, CONFIG_FILE_NAME_LENGTH-1 );
		chdir( game->emulator->directory );
	}
	
#ifdef __unix__
	/* Terminate param list */
	params[count] = NULL;

	printf( "Executing: %s", game->emulator->executable );
	for( i = 0 ; i < count ; i++ ) {
		printf( " %s", params[i] );
	}
	printf( "\n" );

	pid_t pid = fork();
	if (pid == 0) {
		if( execvp( game->emulator->executable, params ) != 0 ) {
			fprintf( stderr, "Error: Couldn't execute emulator '%s': %s\n", game->emulator->executable, strerror(errno) );
			exit( 1 );
		}	
	}
	else if (pid < 0) {
		fprintf(stderr, "Error: failed to fork\n");
		return -1;
	}
	wait( NULL );
#endif
#ifdef __WIN32__
	memset( &pi, 0, sizeof(PROCESS_INFORMATION));
	memset( &si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(si);

	snprintf( cmdline, CONFIG_MAX_CMD_LENGTH, "\"%s\"", game->emulator->executable );
	for( i = 0 ; i < count ; i++ ) {
		if( strlen(cmdline) + strlen(params[i]) + 4 <= CONFIG_MAX_CMD_LENGTH ) {
			strcat( cmdline, " \"" );
			strcat( cmdline, params[i] );
			strcat( cmdline, "\"" );
		}
		else {
			fprintf( stderr, "Warning: No space for parameter: '%s'\n", params[i] );
			break;
		}
	}
	printf( "Executing: %s\n", cmdline );
	
	if( CreateProcess( NULL, cmdline, 0, 0, 0, 0, 0, 0, &si, &pi ) ) {
		WaitForSingleObject(pi.hProcess, INFINITE);
	    CloseHandle(pi.hProcess);
	    CloseHandle(pi.hThread);
	}
	else {
		LPTSTR s;
		FormatMessage( (FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM),
            NULL, GetLastError(), 0, (LPTSTR)&s, 0, NULL );
		fprintf( stderr, "Warning: failed to execute: %s\n", s );
	}
#endif

	if( game->emulator->directory[0] )
		chdir( current_dir );

	return 0;
}

int emulator_run( struct game *game ) {
	int ret = 0;

	pause_all();
	sdl_free();

	emulator_exec( game );

	if( sdl_init() != 0 )
		return -1;
	if( ogl_init() != 0 )
		return -1;
	ret = resume_all();
	if( ret != 0 )
		fprintf( stderr, "Warning: resume_all() failed: %d\n", ret );

	return ret;
}

struct config_emulator *emulator_get_by_name( const char *name ) {
	struct config_emulator *e = config_get()->emulators;
	
	if( name && name[0] ) {
		while( e ) {
			if( strcasecmp( e->name, name ) == 0 )
				break;
			e = e->next;
		}
	}
	
	return e;
}

struct config_emulator *emulator_get_by_platform( const char *platform ) {
	struct config_emulator *e = config_get()->emulators;
	
	if( platform && platform[0] ) {
		while( e ) {
			if( e->platform && e->platform->name && strcasecmp( e->platform->name, platform ) == 0 )
				break;
			e = e->next;
		}
	}
	
	return e;
}

struct config_emulator *emulator_get_default( void ) {
	struct config_emulator *e = config_get()->emulators;
	
	while( e ) {
		if( e->is_default )
			break;
		e = e->next;
	}
	
	if( !e )
		e = config_get()->emulators;
	
	return e;
}

