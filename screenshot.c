#include "screenshot.h"
#include "ogl.h"

static GLfloat offset = -1.0;
static GLfloat angle_x = -10;
static GLfloat angle_y = 30;
static GLfloat angle_z = 10;
static GLfloat size = 0.75;
static GLuint texture = 0;

int screenshot_init( void ) {
	return 0;	
}

void screenshot_free( void ) {
	
}

void screenshot_draw( void ) {
	GLfloat xfactor = ogl_xfactor();
	GLfloat yfactor = ogl_yfactor();

	ogl_load_alterego();
	glTranslatef( offset * xfactor, 0 * yfactor, -4 );
	glRotatef( angle_x, 1.0, 0.0, 0.0 );
	glRotatef( angle_y, 0.0, 1.0, 0.0 );
	glRotatef( angle_z, 0.0, 0.0, 1.0 );
	glColor4f( 1.0, 1.0, 1.0, 1.0 );
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

	glBindTexture( GL_TEXTURE_2D, texture );
	glBegin( GL_QUADS );
		glTexCoord2f(0.0, 0.0); glVertex3f(-size,  size*0.75, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-size, -size*0.75, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( size, -size*0.75, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( size,  size*0.75, 0.0);	
	glEnd();
}
