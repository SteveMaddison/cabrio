#include "sound.h"
#include <SDL/SDL_mixer.h>

Mix_Chunk *blip_sound = NULL;
Mix_Chunk *select_sound = NULL;
Mix_Chunk *back_sound = NULL;
Mix_Chunk *no_sound = NULL;

int sound_init( void ) {
	if( Mix_OpenAudio( 22050, MIX_DEFAULT_FORMAT, 2, 4096 ) == -1 )
		return -1;

	blip_sound = Mix_LoadWAV( DATA_DIR "/sounds/blip.wav" );
	select_sound = Mix_LoadWAV( DATA_DIR "/sounds/select.wav" );
	back_sound = Mix_LoadWAV( DATA_DIR "/sounds/back.wav" );
	no_sound = Mix_LoadWAV( DATA_DIR "/sounds/no.wav" );

	return 0;
}

void sound_free( void ) {
	Mix_FreeChunk( blip_sound );
	blip_sound = NULL;
	Mix_FreeChunk( select_sound );
	select_sound = NULL;
	Mix_FreeChunk( back_sound );
	back_sound = NULL;
	Mix_FreeChunk( no_sound );
	no_sound = NULL;
	
	Mix_CloseAudio();
}

void sound_pause( void ) {
	sound_free();
}

int sound_resume( void ) {
	return sound_init();
}

void sound_play_blip( void ) {
	if( blip_sound )
		Mix_PlayChannel( -1, blip_sound, 0 );
}

void sound_play_select( void ) {
	if( select_sound )
		Mix_PlayChannel( -1, select_sound, 0 );
}

void sound_play_back( void ) {
	if( back_sound )
		Mix_PlayChannel( -1, back_sound, 0 );
}

void sound_play_no( void ) {
	if( no_sound )
		Mix_PlayChannel( -1, no_sound, 0 );
}


