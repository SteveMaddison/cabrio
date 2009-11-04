#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_image.h>
#include "sdl_ogl.h"
#include "config.h"

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
	const struct config *config = config_get();
	unsigned int x = surface->w;
	unsigned int y = surface->h;
	SDL_Surface *resized = NULL;

	if( (surface->w & (surface->w-1)) != 0 )
		x = next_power_of_two( surface->w );
	while( x > config->iface.gfx_max_width )
		x>>=1;

	if( (surface->h & (surface->h-1)) != 0 )
		y = next_power_of_two( surface->w );
	while( y > config->iface.gfx_max_height )
		y>>=1;

	if( x != surface->w || y != surface->h ) {
		SDL_Surface *tmp = NULL;
		int dx,dy;
		double sx = (double)x/(double)surface->w;
		double sy = (double)y/(double)surface->h;
		
		/* Before we resize, check the result is definitely a power
		 * of two, as this can go wrong due to rounding errors. */
		do {
			zoomSurfaceSize( surface->w, surface->h, sx, sy, &dx, &dy );
			if( (dx & (dx-1)) != 0 ) {
				if( (dx & (dx-1)) == 1 ) {
					sx =- 0.001;
				}
				else {
					sx += 0.001;
				}
			}
			if( (dy & (dy-1)) != 0 ) {
				if( (dy & (dy-1)) == 1 ) {
					sy =- 0.001;
				}
				else {
					sy += 0.001;
				}
			}
	    } while( (dx & (dx-1)) != 0 && (dy & (dy-1)) != 0 );
		
		tmp = zoomSurface( surface, sx, sy, 0 );
		resized = SDL_DisplayFormatAlpha( tmp );
		SDL_FreeSurface( tmp );
	}
	
	return resized;
}

struct texture *ogl_create_texture( SDL_Surface* surface ) {
	struct texture *texture = malloc( sizeof(struct texture) );
	
	if( texture ) {
		const struct config *config = config_get();
		SDL_Surface *new = NULL;
		SDL_Surface *work = NULL;
		GLint bpp;
		GLenum format = 0;
		GLenum error = GL_NO_ERROR;
		GLuint filter = GL_LINEAR;
		
		texture->width = surface->w;
		texture->height = surface->h;
		
		if( config->iface.gfx_quality != CONFIG_HIGH )
			filter = GL_NEAREST;
		
		if( surface == NULL ) {
			fprintf(stderr, "Error: Can't create texture from NULL surface.\n" );
			return NULL;
		}
		if( ogl_nopt_textures() == 0 ) {
			/* This OpenGL implementation only supports textures with power-of-two
			 * dimensions, so we need to resize the surface before going further */
			new = resize( surface );
		}
		
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
				return NULL;
				break;
		}
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures( 1, &texture->id );
		glBindTexture( GL_TEXTURE_2D, texture->id );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter );
		glTexImage2D( GL_TEXTURE_2D, 0, bpp, work->w, work->h, 0, format, GL_UNSIGNED_BYTE, work->pixels );
	
		if( work != surface )
			SDL_FreeSurface( work );
	
		error = glGetError();
		if( error != GL_NO_ERROR ) {
			fprintf(stderr, "Error: couldn't create texture: %s.\n", gluErrorString(error) );
			return NULL;
		}
	}
	return texture;
}

struct texture *sdl_create_texture( const char *filename ) {
	struct texture *t = NULL;
	SDL_Surface *s = sdl_load_image( filename );
	if( s ) {
		t = ogl_create_texture( s );
		SDL_FreeSurface( s );
	}
	return t;
}
