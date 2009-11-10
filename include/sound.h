#ifndef _SOUND_H_
#define _SOUND_H_ 1

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

void sound_play( int s );

int sound_id( char *name );
const char *sound_name( int s );

#endif
