#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
#include "video.h"
#include "ogl.h"

static const int VIDEO_SIZE = 256;
static const int CONV_FORMAT = PIX_FMT_RGB24;
static const int VIDEO_BPP = 3;
static const GLfloat VIDEO_SCALE = 0.005;

AVFormatContext *format_context = NULL;
AVCodecContext *codec_context = NULL;
AVCodec *codec = NULL;
AVFrame *raw_frame = NULL;
AVFrame *conv_frame = NULL;
AVPacket packet;
uint8_t *buffer;
int video_stream = -1;
struct SwsContext *scale_context;
GLuint texture;
GLfloat xsize, ysize;


int video_init( void ) {
	avcodec_register_all();
	av_register_all();
	
	raw_frame = avcodec_alloc_frame();
	conv_frame = avcodec_alloc_frame();
	
	if( !raw_frame || !conv_frame ) {
		fprintf( stderr, "Warning: Couldn't allocate video frame\n" );
		return -1;
	}

	glGenTextures( 1, &texture );

	return 0;
}

void video_free( void ) {
	video_close();
	
	av_free( raw_frame );
	raw_frame = NULL;
	
	av_free( conv_frame );
	conv_frame = NULL;
	
	if( texture )
		glDeleteTextures( 1, &texture );
	texture = 0;
}

void video_close( void ) {
	if( buffer )
		av_free( buffer );
	buffer = NULL;

	if( codec_context )
		avcodec_close( codec_context );
	codec_context = NULL;
	
	if( format_context )
		av_close_input_file( format_context );
	format_context = NULL;
	
	video_stream = -1;
}

int video_open( const char *filename ) {
	int i = -1;
	
	format_context = NULL;
	codec_context = NULL;
	video_stream = -1;
	
	if( !filename )
		return -1;
		
	if( av_open_input_file( &format_context, filename, NULL, 0, NULL ) != 0 ) {
		fprintf( stderr, "Warning: Error opening video file '%s'\n", filename );
		return -1;
	}

	if( av_find_stream_info( format_context ) < 0 ) {
		fprintf( stderr, "Warning: Error reading stream info from '%s'\n", filename );
		return -1;
	}
	
	for( i = 0 ; i < format_context->nb_streams ; i++ ) {
		if( format_context->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO ) {
			video_stream = i;
			codec_context = format_context->streams[video_stream]->codec;
			break;
		}
	}
	if( !codec_context ) {
		fprintf( stderr, "Warning: Couldn't find video stream in '%s'\n", filename );
		return -1;
	}
	
	codec = avcodec_find_decoder( codec_context->codec_id );
	if( !codec ) {
		fprintf( stderr, "Warning: Codec in video '%s' not supported\n", filename );
		return -1;
	}
	
	if( avcodec_open( codec_context, codec ) < 0 ) {
		fprintf( stderr, "Warning: Couldn't open video codec for '%s'\n", filename );
		return -1;
	}
	else {
		int bytes = avpicture_get_size( CONV_FORMAT, VIDEO_SIZE, VIDEO_SIZE );
		
		printf( "%d x %d, %d.%d\n", codec_context->width, codec_context->height,
			codec_context->time_base.num, codec_context->time_base.den );
		
		xsize = ((GLfloat)codec_context->width) * VIDEO_SCALE / 2.0;
		ysize = ((GLfloat)codec_context->height) * VIDEO_SCALE / 2.0;
		
		buffer = (uint8_t*)av_malloc( bytes *sizeof(uint8_t) );
	}
	
	if( !buffer ) {
		fprintf( stderr, "Warning: Couldn't allocate buffer for video '%s'\n", filename );
		return -1;
	}

	avpicture_fill( (AVPicture*)conv_frame, buffer, CONV_FORMAT, VIDEO_SIZE, VIDEO_SIZE );
	
	return 0;
}

void video_rewind( void ) {
	if( !format_context ) {
		return;
	}
	else {
		av_seek_frame( format_context, 0, 0, 0 );
	}
}

int video_get_frame( void ) {
	int got_frame = 0;
	GLenum error = GL_NO_ERROR;
	
	if( !format_context || !codec_context || !buffer )
		return -1;
		
	if( av_read_frame( format_context, &packet ) >= 0 ) {
		if( packet.stream_index == video_stream ) {
			avcodec_decode_video( codec_context, raw_frame, &got_frame, packet.data, packet.size );
		
			if( got_frame ) {
				scale_context = sws_getCachedContext( scale_context, codec_context->width, codec_context->height,
					codec_context->pix_fmt, VIDEO_SIZE, VIDEO_SIZE, CONV_FORMAT, SWS_BICUBIC, NULL, NULL, NULL );
				if( !scale_context ) {
					fprintf( stderr, "Warning: Couldn't initialise video conversion context\n" );
					return -1;
				}

				sws_scale( scale_context, raw_frame->data, raw_frame->linesize, 0,
					VIDEO_SIZE, conv_frame->data, conv_frame->linesize );
				
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glBindTexture( GL_TEXTURE_2D, texture );
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
				glTexImage2D( GL_TEXTURE_2D, 0, VIDEO_BPP, VIDEO_SIZE, VIDEO_SIZE, 0,
					GL_RGB, GL_UNSIGNED_BYTE, conv_frame->data[0] );
			}
			av_free_packet(&packet);
		}
	}
	else {
		video_rewind();	
	}
	
	ogl_load_alterego();
	glTranslatef( 0.0, 0.0, -5 );
	glEnable(GL_TEXTURE_2D);
	glBegin( GL_QUADS );
		glTexCoord2f(0.0, 0.0); glVertex3f(-xsize,  ysize, 0.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-xsize, -ysize, 0.0);
		glTexCoord2f(1.0, 1.0); glVertex3f( xsize, -ysize, 0.0);
		glTexCoord2f(1.0, 0.0); glVertex3f( xsize,  ysize, 0.0);
	glEnd();
	
	error = glGetError();
	if( error != GL_NO_ERROR ) {
		fprintf(stderr, "Warning: couldn't create texture: %s.\n", gluErrorString(error) );
	}
	
	return 0;
}
