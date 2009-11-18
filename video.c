#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include "video.h"

AVFormatContext *format_context = NULL;
AVCodecContext *codec_context = NULL;
AVCodec *codec = NULL;
AVFrame *raw_frame = NULL;
AVFrame *conv_frame = NULL;
AVPacket packet;
uint8_t *buffer;
int video_stream = -1;
int got_frame = 0;


int video_init( void ) {
	av_register_all();
	
	raw_frame = avcodec_alloc_frame();
	conv_frame = avcodec_alloc_frame();
	
	if( !raw_frame || !conv_frame ) {
		fprintf( stderr, "Warning: Couldn't allocate video frame\n" );
		return -1;
	}

	return 0;
}

void video_free( void ) {
	video_close();
	
	av_free( raw_frame );
	raw_frame = NULL;
	
	av_free( conv_frame );
	conv_frame = NULL;
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
		int bytes = avpicture_get_size( PIX_FMT_RGB24,
			codec_context->width, codec_context->height);
		buffer = (uint8_t*)av_malloc( bytes *sizeof(uint8_t) );
	}
	
	if( !buffer ) {
		fprintf( stderr, "Warning: Couldn't allocate buffer for video '%s'\n", filename );
		return -1;
	}
	
	avpicture_fill( (AVPicture*)conv_frame, buffer, PIX_FMT_RGB24,
		codec_context->width, codec_context->height );
	
	return 0;
}
