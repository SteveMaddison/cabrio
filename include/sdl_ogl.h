#ifndef _SDL_OGL_H_
#define _SDL_OGL_H_ 1

#include <SDL/SDL.h>
#ifdef __WIN32__
#define _WINCON_H 1 /* Avoid inclusion of wincon.h */
#endif
#include <SDL/SDL_opengl.h>
#include "sdl_wrapper.h"
#include "ogl.h"

struct texture *ogl_create_texture( SDL_Surface *surface );
struct texture *sdl_create_texture( const char *filename );

#endif

