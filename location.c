#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include "location.h"
#include "config.h"

struct location_type *type_start = NULL;

int location_init( void ) {
	struct config_location_type *config_type = config_get()->location_types;
	
	while( config_type ) {
		struct location_type *type = malloc( sizeof(struct location_type) );
		
		if( type ) {
			struct config_location *config_location = config_get()->locations;
			
			type->locations = NULL;
			type->name = config_type->name;
			
			while( config_location ) {
				if( config_location->type == config_type ) {
					struct location *location = malloc( sizeof(struct location) );
					if( location ) {
						location->directory = config_location->directory;
						location->next = type->locations;
						type->locations = location;
					}
					else {
						fprintf( stderr, "Warning: Couldn't allocate memory for location\n" );
					}
				}
				config_location = config_location->next;
			}
			type->next = type_start;
			type_start = type;
		}
		else {
			fprintf( stderr, "Warning: Couldn't allocate memory for location type\n" );
		}
		config_type = config_type->next;
	}

/*{
	struct location_type *type = type_start;
	while( type ) {
		struct location *location = type->locations;
		printf("Type: %s\n", type->name );
		while( location ) {
			printf("  %s\n", location->directory );
			location = location->next;
		}
		type = type->next;
	}
}*/

	return 0;	
}

void location_free( void ) {
	struct location_type *type = type_start;
	
	while( type ) {
		struct location_type *ttmp = type->next;
		struct location *location = type->locations;
		
		while( location ) {
			struct location *ltmp = location->next;
			free( location );
			location = ltmp;
		}
		
		free( type );
		type = ttmp;
	}
}

struct location *location_get_first( const char *type ) {
	struct location_type *ltype = type_start;
	
	while( ltype ) {
		if( strcasecmp( ltype->name, type ) == 0 )
			return ltype->locations;
		ltype = ltype->next;
	}
	
	return NULL;
}

int location_absolute( const char *filename ) {
#ifdef __WIN32__
		if( filename[0] && filename[1] == ':' ) {
#else
		if( filename[0] == '/' ) {
#endif
			return 0;
		}
		return -1;
}

int location_try_directory( const char *dir, const char *filename, char *path ) {
	char test[CONFIG_FILE_NAME_LENGTH];
	struct stat stmp;

	if( dir && filename ) {
#ifdef __WIN32__
		snprintf( test, CONFIG_FILE_NAME_LENGTH, "%s\\%s", dir, filename );
#else
		snprintf( test, CONFIG_FILE_NAME_LENGTH, "%s/%s", dir, filename );
#endif
		if( stat( test, &stmp ) == -1 ) {
			if( errno != ENOENT ) {
				fprintf( stderr, "Warning: Couldn't stat file '%s': %s\n", test, strerror( errno ) );
				return -1;
			}
		}
		else {
			strncpy( path, test, CONFIG_FILE_NAME_LENGTH );
			return 0;
		}
	}

	return -1;
}

int location_get_path( const char *type, const char *filename, char *path ) {
	struct location *location = location_get_first( type );

	if( type && filename && filename[0]) {
		if( location_absolute( filename ) == 0 ) {
			strncpy( path, filename, CONFIG_FILE_NAME_LENGTH );
			return 0;
		}
		
		while( location ) {
			if( location_try_directory( location->directory, filename, path ) == 0 ) {
				return 0;
			}
			location = location->next;
		}
		
		/* Not found, return the original filename for later reference. */
		strncpy( path, filename, CONFIG_FILE_NAME_LENGTH );
	}

	return -1;
}

int location_get_theme_path( const char *filename, char *path ) {
	if( filename ) {
		if( location_absolute( filename ) == 0 ) {
			strncpy( path, filename, CONFIG_FILE_NAME_LENGTH );
			return 0;		
		}

		if( location_try_directory( config_get()->iface.theme.directory, filename, path ) == 0 ) {
			return 0;
		}

		/* Not found, return the original filename for later reference. */
		strncpy( path, filename, CONFIG_FILE_NAME_LENGTH );
	}
	
	return -1;
}

int location_get_match( const char *type, const char *filename, char *path ) {
	struct location *location = location_get_first( type );
	char search[CONFIG_FILE_NAME_LENGTH];
	char *pos = NULL;
	DIR *dir = NULL;
	struct dirent *dentry;
	int found = 0;

	if( type && filename ) {
#ifdef __WIN32__
		pos = strrchr( filename, '\\' );
#else
		pos = strrchr( filename, '/' );
#endif
		if( pos )
			strncpy( search, pos+1, CONFIG_FILE_NAME_LENGTH );
		else
			strncpy( search, filename, CONFIG_FILE_NAME_LENGTH );
		
		pos = strrchr( search, '.' );
		if( pos )
			*(pos + 1) = '\0';
		else
			strcat( search, "." );

		while( location && !found ) {
			if( (dir = opendir( location->directory )) ) {
				while( (dentry = readdir( dir )) ) {
#ifdef __WIN32__
					if( strncasecmp( dentry->d_name, search, strlen(search) ) == 0 ) {
						snprintf( path, CONFIG_FILE_NAME_LENGTH, "%s\\%s", location->directory, dentry->d_name );
#else
					if( strncasecmp( dentry->d_name, search, strlen(search) ) == 0 ) {
						snprintf( path, CONFIG_FILE_NAME_LENGTH, "%s/%s", location->directory, dentry->d_name );
#endif
						found = 1;
					}
				}
				closedir( dir );
			}
			location = location->next;
		}
	}

	if( found )
		return 0;
	
	strcpy( path, "" );
	return -1;
}

