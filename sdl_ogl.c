#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_image.h>
#include "sdl.h"
#include "sdl_ogl.h"

#define TEXTURE_MAX_WIDTH 512
#define TEXTURE_MAX_HEIGHT 256

unsigned int next_power_of_two( unsigned int x ) {
        int i = 0;
        if( x == 0 ) return 1;
        for( i = 0; x > 0 ; i++, x>>=1 );
        return 1<<i;
}

SDL_Surface *sdl_load_image( const char *filename ) {
	SDL_Surface* load = NULL;
	SDL_Surface* conv = NULL;
	
	load = IMG_Load( filename );
	if( load != NULL ) {
		conv = SDL_DisplayFormatAlpha( load );
		SDL_FreeSurface( load );
	}
	else {
		fprintf(stderr, "Error: Unable to load image '%s': %s\n", filename, SDL_GetError());
	}
	return conv;
}

SDL_Surface *resize( SDL_Surface *surface ) {
	unsigned int x = surface->w;
	unsigned int y = surface->h;
	SDL_Surface *resized = NULL;

	if( (surface->w & (surface->w-1)) != 0 )
		x = next_power_of_two( surface->w );
	while( x > TEXTURE_MAX_WIDTH )
		x>>=1;

	if( (surface->h & (surface->h-1)) != 0 )
		y = next_power_of_two( surface->w );
	while( y > TEXTURE_MAX_HEIGHT )
		y>>=1;

	if( x != surface->w || y != surface->h ) {
		SDL_Surface *tmp = zoomSurface( surface, (double)x/(double)surface->w, (double)y/(double)surface->h, 0 );
		resized = SDL_DisplayFormatAlpha( tmp ); 
		SDL_FreeSurface( tmp );
	}
	return resized;
}

int ogl_create_texture( SDL_Surface* surface, GLuint *texture ) {
	SDL_Surface *new = NULL;
	SDL_Surface *work = NULL;
	GLint bpp;
	GLenum format = 0;
	GLenum error = GL_NO_ERROR;
	
	if( surface == NULL ) {
		fprintf(stderr, "Error: Can't create texture from NULL surface.\n" );
		return -1;
	}
	new = resize( surface );
	work = new ? new : surface;
	
	/* determine image format */
	bpp = work->format->BytesPerPixel;
	switch( bpp ) {
		case 4:
			if (work->format->Rmask == 0x000000ff)
				format = GL_RGBA;
			else
				format = GL_BGRA;
			break;
		case 3:
			if (work->format->Rmask == 0x000000ff)
				format = GL_RGB;
			else
				format = GL_BGR;
			break;
		default:
			fprintf(stderr, "Error: image is not true colour (%d bpp).\n", bpp );
			if( work != surface )
				SDL_FreeSurface( work );
			return -1;
			break;
	}
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures( 1, texture );
	glBindTexture( GL_TEXTURE_2D, *texture );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexImage2D( GL_TEXTURE_2D, 0, bpp, work->w, work->h, 0, format, GL_UNSIGNED_BYTE, work->pixels );

	if( work != surface )
		SDL_FreeSurface( work );

	error = glGetError();
	if( error != GL_NO_ERROR ) {
		fprintf(stderr, "Error: couldn't create texture: %s.\n", gluErrorString(error) );
		return -1;
	}
	return 0;
}

GLuint sdl_create_texture( const char *filename, int *x, int *y ) {
	GLuint t = 0;
	SDL_Surface *s = sdl_load_image( filename );
	if( s ) {
		ogl_create_texture( s, &t );
		*x = s->w;
		*y = s->h;
		SDL_FreeSurface( s );
	}
	return t;
}

