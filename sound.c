#include <dirent.h>
#include <fnmatch.h>
#include "sound.h"
#include "config.h"
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

static const int AUDIO_CHUNK_SIZE = 1024;
static Mix_Chunk *sounds[NUM_SOUNDS];
static char *names[] = { "back", "blip", "no", "select" };
static int mixer_open = 0;

int sound_open_mixer( void ) {
	if( !mixer_open ) {
		if( Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT,1,AUDIO_CHUNK_SIZE) == -1 ) {
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
	stopmusic();
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

void playmusic(void) {
#ifdef __WIN32__
  static const char dir_separator = '\\';
#else
  static const char dir_separator = '/';
#endif
  if (!music)
    free( music );
  if ( config_get()->iface.theme.music[strlen(config_get()->iface.theme.music)-1] == dir_separator )
  {
    DIR           *dir     = opendir( config_get()->iface.theme.music );
    struct dirent *e;
    int           nr_music = 0;
    if ( dir != NULL )
    {
      while ( ( e = readdir( dir )) != NULL )
        if ( (fnmatch( "*.mp3", e->d_name, 0) == 0) ||  
             (fnmatch( "*.ogg", e->d_name, 0) == 0) )
          nr_music++;
      if ( nr_music > 0 )
      {
        int sel_music = rand( )%nr_music;
        closedir( dir );
        dir = opendir( config_get()->iface.theme.music );
        e   = readdir( dir );
        while ( sel_music > 0 )
        {
          e = readdir( dir );
          sel_music--;
        }
        char music_name[CONFIG_FILE_NAME_LENGTH];
        snprintf( music_name, CONFIG_FILE_NAME_LENGTH, "%s%s", config_get()->iface.theme.music, e->d_name );
        music = Mix_LoadMUS( music_name );
      }
      closedir( dir );
    }
  }
  else
    music = Mix_LoadMUS( config_get()->iface.theme.music );
  if (music != NULL)
    Mix_PlayMusic(music, 0);
  if ( config_get()->iface.music_volume > 0 && config_get()->iface.music_volume <= 128 )
    Mix_VolumeMusic(config_get()->iface.music_volume);
}

void stopmusic(void) {
	if (music != NULL)
		Mix_FreeMusic(music);
}
