#ifndef _SOUND_H_
#define _SOUND_H_ 1

#include <SDL2/SDL_mixer.h>

enum sound_t {
	SOUND_BACK,
	SOUND_BLIP,
	SOUND_NO,
	SOUND_SELECT,
	NUM_SOUNDS
};

int sound_init( void );
void sound_pause( void );
int sound_resume( void );
void sound_free( void );
int sound_open_mixer( void );
void sound_close_mixer( void );

void sound_play( int s );
void playmusic( void );
void stopmusic( void );
int sound_id( char *name );
const char *sound_name( int s );
Mix_Music *music;

#endif
