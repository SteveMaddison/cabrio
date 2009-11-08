#include <unistd.h>
#include <sys/types.h>
#ifdef __unix__
#include <sys/wait.h>
#endif
#ifdef __WIN32__
#include <windows.h>
#endif

#include "sdl_wrapper.h"
#include "ogl.h"
#include "font.h"
#include "config.h"
#include "category.h"
#include "platform.h"
#include "bg.h"
#include "menu.h"
#include "submenu.h"
#include "hint.h"
#include "game_sel.h"
#include "sound.h"
#include "event.h"
#include "setup.h"
#include "focus.h"

static int supress_wait = 0;

void pause_all( void ) {
	sound_pause();
	game_list_pause();
	submenu_pause();
	menu_pause();
	hint_pause();
	font_pause();
	bg_pause();
	event_pause();
}

int resume_all( void ) {
	if( event_resume() != 0 )
		return -1;
	if( bg_resume() != 0 )
		return -2;
	if( font_resume() != 0 )
		return -3;
	if( hint_resume() != 0 )
		return -4;
	if( menu_resume() != 0 )
		return -5;
	if( submenu_resume() != 0 )
		return -6;
	if( game_list_resume() != 0 )
		return -7;
	if( sound_resume() != 0 )
		return -8;

	return 0;
}

int run( struct game *game ) {
	char *params[CONFIG_MAX_PARAMS];
	struct config_param *param = NULL;
	int i;
	int count = 0;
	const struct config *config = config_get();
#ifdef __WIN32__
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	char cmdline[CONFIG_MAX_CMD_LENGTH];
#endif

	if( config->emulators && config->emulators->executable ) {
		param = config->emulators->params;
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
	
#ifdef __unix__
	/* Terminate param list */
	params[count] = NULL;

	printf( "Executing: %s", config->emulators->executable );
	for( i = 0 ; i < count ; i++ ) {
		printf( " %s", params[i] );
	}
	printf( "\n" );

	pid_t pid = fork();
	if (pid == 0) {
		execvp( config->emulators->executable, params );
	}
	else if (pid < 0) {
		fprintf(stderr, "Warning: failed to fork\n");
		return -1;
	}
	wait( NULL );
#endif
#ifdef __WIN32__
	memset( &pi, 0, sizeof(PROCESS_INFORMATION));
	memset( &si, 0, sizeof(STARTUPINFO));

	snprintf( cmdline, CONFIG_MAX_CMD_LENGTH, "\"%s\"", config->emulators->executable );
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
	return 0;
}

void supress( void ) {
	supress_wait = 15;
}

int main( int argc, char *arvg[] ) {
	int quit = 0;
	int config_status = 0;
	int event;
	struct game *to_run = NULL;

	config_status = config_open( NULL );
	if( config_status == -1 )
		return -1;

	if( sdl_init() != 0 )
		return -1;

	if( ogl_init() != 0 )
		return -1;

	if( event_init() != 0 )
		return -1;

	if( font_init() != 0 )
		return -1;	

	if( config_status == 1 ) {
		/* Config file didn't exist, so run the setup utility */
		if( setup() != 0 )
			return -1;
		config_update();
		if( config_create() != 0 )
			return -1;
	}

	if( platform_init() != 0 )
		return -1;

	if( category_init() != 0 )
		return -1;

	if( game_sel_init( 1 ) != 0 )
		return -1;

	if( menu_init() != 0 )
		return -1;

	if( hint_init() != 0 )
		return -1;

	if( game_list_create() != 0 )
		return -1;

	sound_init();
	bg_init();

	event_flush();

	while( !quit ) {
		ogl_clear();
		bg_draw();
		hint_draw( focus_has() );
		menu_draw();
		submenu_draw();
		game_sel_draw();
		sdl_swap();
	
		if (( event = event_poll() )) {
			if( supress_wait == 0 ) {
				if( event == EVENT_QUIT ) {
					quit = 1;
				}
				else {
					supress();
					event_process( event );
				}
			}
		}
		if( supress_wait > 0 )
			supress_wait--;
			
		sdl_frame_delay();

		if( to_run && !game_sel_busy() ) {
			int err = 0;
			pause_all();
			sdl_free();
			run( to_run );
			if( sdl_init() != 0 )
				return -1;
			if( ogl_init() != 0 )
				return -1;
			err = resume_all();
			if( err != 0 )
				fprintf( stderr, "Warning: resume_all() failed: %d\n", err );
			to_run = NULL;
		}
	}

	sound_free();
	game_list_free();
	submenu_free();
	menu_free();
	hint_free();
	font_free();
	bg_free();
	event_free();
	sdl_free();

	return 0;
}

