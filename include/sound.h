#ifndef _SOUND_H_
#define _SOUND_H_ 1

int sound_init( void );
void sound_pause( void );
int sound_resume( void );
void sound_free( void );

void sound_play_blip( void );
void sound_play_select( void );
void sound_play_back( void );
void sound_play_no( void );

#endif

