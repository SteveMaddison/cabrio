#include "sound.h"
#include "config.h"
#include <SDL/SDL_mixer.h>

static const int AUDIO_CHUNK_SIZE = 4096;
static Mix_Chunk *sounds[NUM_SOUNDS];
static char *names[] = { "back", "blip", "no", "select" };
static int mixer_open = 0;

int sound_open_mixer( void ) {
	if( !mixer_open ) {
		if( Mix_OpenAudio( 22050, AUDIO_S16SYS, 4, AUDIO_CHUNK_SIZE ) == -1 ) {
			fprintf( stderr, "Error: Unable to initialise sound: %s\n", Mix_GetError() );
			return -1;
		}
	}
	mixer_open = 1;
	return 0;
}

void sound_close_mixer( void ) {
	if( mixer_open )
		Mix_CloseAudio();
	mixer_open = 0;
}

int sound_init( void ) {
	int i;
	
	for( i = 0 ; i < NUM_SOUNDS ; i++ )
		sounds[i] = NULL;
	
	if( sound_open_mixer() != 0 )
		return -1;

	for( i = 0 ; i < NUM_SOUNDS ; i++ ) {
		sounds[i] = Mix_LoadWAV( config_get()->iface.theme.sounds[i] );
		if( sounds[i] == NULL ) {
			fprintf( stderr, "Warning: Unable to open sound: %s\n", config_get()->iface.theme.sounds[i] );
		}
	}

	return 0;
}

void sound_free( void ) {
	int i;
	
	for( i = 0 ; i < NUM_SOUNDS ; i++ ) {
		if( sounds[i] ) {
			Mix_FreeChunk( sounds[i] );
			sounds[i] = NULL;
		}
	}

	sound_close_mixer();
}

void sound_pause( void ) {
	sound_free();
}

int sound_resume( void ) {
	return sound_init();
}

void sound_play( int s ) {
	if( mixer_open && s >= 0 && s < NUM_SOUNDS && sounds[s] )
		Mix_PlayChannel( -1, sounds[s], 0 );
}

int sound_id( char *name ) {
	int i;
	
	if( name ) {
		for( i = 0 ; i < NUM_SOUNDS ; i++ ) {
			if( strcasecmp( name, names[i] ) == 0 ) {
				return i;
			}
		}
	}
	
	return -1;
}

const char *sound_name( int s ) {
	if( s >= 0 && s < NUM_SOUNDS )
		return names[s];
	return NULL;
}

