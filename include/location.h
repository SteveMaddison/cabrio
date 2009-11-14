#ifndef __LOCATION_H__
#define __LOCATION_H__ 1

struct location {
	struct location *next;
	char *directory;
};

struct location_type {
	struct location_type *next;
	struct location *locations;
	char *name;
};

int location_init( void );
void location_free( void );
struct location *location_get_first( const char *type );
int location_get_path( const char *type, const char *filename, char *path );
int location_get_theme_path( const char *filename, char *path );
int location_get_match( const char *type, const char *filename, char *path );

#endif

