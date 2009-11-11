#ifndef _HINT_H_
#define _HINT_H_ 1

#include "ogl.h"

struct arrow {
	GLfloat x;
	GLfloat y;
	GLfloat size;
	GLfloat angle;
};

int hint_init( void );
void hint_pause( void );
int hint_resume( void );
void hint_free( void );
int hint_draw( void );
void hint_draw_arrow( struct arrow *arrow );

#endif

