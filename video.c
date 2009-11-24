#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include "video.h"
#include "sound.h"
#include "packet.h"
#include "frame.h"
#include "config.h"
#include "ogl.h"

#define AUDIO_BUFFER_SIZE ((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2)
static const int VIDEO_SIZE = 256;
static const int CONV_FORMAT = PIX_FMT_RGB24;
static const int VIDEO_BPP = 3;
static const int MAX_QUEUE_PACKETS = 20;
static const int QUEUE_FULL_DELAY = 10;
static const int MAX_QUEUE_FRAMES = 20;
static const float FUDGE_FACTOR = 0.02;

static AVFormatContext *format_context = NULL;
static AVCodecContext *video_codec_context = NULL;
static AVCodec *video_codec = NULL;
static AVFrame *conv_frame = NULL;
static struct SwsContext *scale_context = NULL;
static uint8_t *video_buffer = NULL;
static int video_stream = -1;
static struct frame_queue video_queue;
static double video_clock = 0;

static AVCodecContext *audio_codec_context = NULL;
static AVCodec *audio_codec = NULL;
static int audio_stream = -1;
static struct packet_queue audio_queue;
static uint8_t audio_buffer[AUDIO_BUFFER_SIZE];
static unsigned int audio_buffer_size = 0;
static unsigned int audio_buffer_index = 0;
static uint8_t *audio_packet_data = NULL;
static int audio_packet_size = 0;

SDL_AudioSpec audio_spec;
static int audio_open = 0;

static SDL_Thread *reader_thread = NULL;
static int stop = 0;
static int audio_running = 0;
static int reader_running = 0;
static int got_texture = 0;
static struct texture *texture;


int video_stopped( void ) {
	return stop;
}

int video_init( void ) {
	avcodec_register_all();
	av_register_all();
	
	conv_frame = avcodec_alloc_frame();
	
	if( !conv_frame ) {
		fprintf( stderr, "Warning: Couldn't allocate video conversion frame\n" );
		return -1;
	}
		
	packet_queue_init( &audio_queue );
	frame_queue_init( &video_queue );

	url_set_interrupt_cb( video_stopped );
	
	return 0;
}

void video_free( void ) {
	video_close();

	if( conv_frame );	
		av_free( conv_frame );
	conv_frame = NULL;
}

void video_close( void ) {
	stop = 1;

	while( reader_running || audio_running )
		SDL_Delay( 10 );

	if( audio_open )
		SDL_CloseAudio();
	audio_open = 0;

	packet_queue_flush( &audio_queue );
	frame_queue_flush( &video_queue );

	if( video_codec_context )
		avcodec_close( video_codec_context );
	video_codec_context = NULL;

	if( audio_codec_context )
		avcodec_close( audio_codec_context );
	audio_codec_context = NULL;
	
	if( format_context )
		av_close_input_file( format_context );
	format_context = NULL;

	if( video_buffer )
		av_free( video_buffer );
	video_buffer = NULL;
	
	video_stream = -1;
	audio_stream = -1;
	
	if( texture )
		ogl_free_texture( texture );
	texture = NULL;
	got_texture = 0;
		
	sound_open_mixer();
}

int video_decode_video_frame( AVPacket *packet ) {
	static AVFrame *frame = NULL;
	static uint64_t pts = 0;
	int got_frame = 0;
	
	if( packet ) {
		if( !frame ) {
			frame = avcodec_alloc_frame();
			if( !frame ) {
				fprintf( stderr, "Warning: Couldn't allocate video frame\n" );
				return -1;
			}
			pts = packet->pts;
		}
		
		if( !frame ) {
			fprintf( stderr, "Warning: Frame disappeared\n" );
			return -1;
		}
		
		avcodec_decode_video( video_codec_context, frame, &got_frame, packet->data, packet->size );		
		
		if( got_frame ) {
			double *frame_pts = av_malloc( sizeof(double) );
			if( !frame_pts ) {
				fprintf( stderr, "Warning: Couldn't allocate PTS for video frame\n" );
			}
			else {
				*frame_pts = pts * av_q2d( format_context->streams[video_stream]->time_base );
				frame->opaque = frame_pts;
				frame_queue_put( &video_queue, frame );
			}
			frame = NULL;
			pts = 0;
		}
		av_free_packet( packet );
	}
	return 0;
}

int video_decode_audio_frame( AVCodecContext *context, uint8_t *buffer, int buffer_size ) {
	static AVPacket packet;
	int used, data_size;

	for(;;) {
		while( audio_packet_size > 0 ) {
			data_size = buffer_size;
			used = avcodec_decode_audio2( context, (int16_t *)audio_buffer, &data_size, 
					  audio_packet_data, audio_packet_size);
			if( used < 0 ) {
				/* if error, skip frame */
				audio_packet_size = 0;
				break;
			}
			audio_packet_data += used;
			audio_packet_size -= used;
			
			if( data_size <= 0 ) {
				/* No data yet, get more frames */
				continue;
			}
			/* We have data, return it and come back for more later */
			return data_size;
		}
		if( packet.data )
			av_free_packet( &packet );

		if( stop ) {
			audio_running = 0;
			return -1;
		}

		if( packet_queue_get( &audio_queue, &packet, 1 ) < 0 )
			return -1;

		audio_packet_data = packet.data;
		audio_packet_size = packet.size;
	}
}

void video_audio_callback( void *userdata, Uint8 *stream, int length ) {
	AVCodecContext *context = (AVCodecContext*)userdata;
	int used, audio_size;

	while( length > 0 ) {
		if(audio_buffer_index >= audio_buffer_size) {
			/* We have already sent all our data; get more */
			audio_size = video_decode_audio_frame( context, audio_buffer, AUDIO_BUFFER_SIZE );
			if( audio_size < 0 ) {
				/* If error, output silence */
				audio_buffer_size = 1024;
				memset( audio_buffer, 0, audio_buffer_size );
			} else {
				audio_buffer_size = audio_size;
			}
			audio_buffer_index = 0;
		}

		used = audio_buffer_size - audio_buffer_index;
		if( used > length )
			used = length;
			
		memcpy( stream, (uint8_t *)audio_buffer + audio_buffer_index, used );
		length -= used;
		stream += used;
		audio_buffer_index += used;
	}
}

int video_reader_thread( void *data ) {
	static AVPacket packet;

	if( !format_context || !video_codec_context || !video_buffer )
		return -1;

	reader_running = 1;

	while( !stop ) {
		if( video_queue.frames >= MAX_QUEUE_FRAMES || audio_queue.packets >= MAX_QUEUE_PACKETS ) {
			SDL_Delay( QUEUE_FULL_DELAY );
		}
		else {
			if( av_read_frame( format_context, &packet ) >= 0 ) {
				if( packet.stream_index == video_stream ) {
					video_decode_video_frame( &packet );
				}
				else if( packet.stream_index == audio_stream && audio_codec_context ) {
					packet_queue_put( &audio_queue, &packet );
				}
				else {
					av_free_packet( &packet );
				}
			}
			else {
				av_seek_frame( format_context, -1, 0, AVSEEK_FLAG_BACKWARD|AVSEEK_FLAG_BYTE );
			}
		}
	}

	reader_running = 0;
	return 0;
}

int video_open( const char *filename ) {
	int i = -1;
	
	format_context = NULL;
	video_codec_context = NULL;
	video_stream = -1;
	video_clock = 0;
	
	audio_codec_context = NULL;
	audio_stream = -1;
	audio_buffer_size = 0;
	audio_buffer_index = 0;
	audio_packet_size = 0;
	audio_packet_data = NULL;
	
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
			video_codec_context = format_context->streams[video_stream]->codec;
		}
		else if( format_context->streams[i]->codec->codec_type == CODEC_TYPE_AUDIO ) {
			audio_stream = i;
			audio_codec_context = format_context->streams[audio_stream]->codec;
		}
	}
	if( !video_codec_context ) {
		fprintf( stderr, "Warning: Couldn't find video stream in '%s'\n", filename );
		return -1;
	}
	
	video_codec = avcodec_find_decoder( video_codec_context->codec_id );
	if( !video_codec ) {
		fprintf( stderr, "Warning: Video codec in video '%s' not supported\n", filename );
		return -1;
	}
	
	if( avcodec_open( video_codec_context, video_codec ) != 0 ) {
		fprintf( stderr, "Warning: Couldn't open video codec '%s' for '%s'\n", video_codec->name, filename );
		return -1;
	}
	else {
		int bytes = avpicture_get_size( CONV_FORMAT, VIDEO_SIZE, VIDEO_SIZE );		
		video_buffer = (uint8_t*)av_malloc( bytes *sizeof(uint8_t) );
	}
	
	if( !video_buffer ) {
		fprintf( stderr, "Warning: Couldn't allocate buffer for video '%s'\n", filename );
		return -1;
	}

	avpicture_fill( (AVPicture*)conv_frame, video_buffer, CONV_FORMAT, VIDEO_SIZE, VIDEO_SIZE );

	if( audio_codec_context ) {
		audio_codec = avcodec_find_decoder( audio_codec_context->codec_id );
		if( !audio_codec ) {
			fprintf( stderr, "Warning: Audio codec in video '%s' not supported\n", filename );
			audio_codec_context = NULL;
		}
		else {
			if( avcodec_open( audio_codec_context, audio_codec ) != 0 ) {
				fprintf( stderr, "Warning: Couldn't open audio codec '%s' for '%s'\n", audio_codec->name, filename );
				audio_codec_context = NULL;
			}
			else {
				SDL_AudioSpec desired;
				
				desired.freq = audio_codec_context->sample_rate;
				desired.format = AUDIO_S16SYS;
				desired.channels = audio_codec_context->channels;
				desired.silence = 0;
				desired.samples = 1024;
				desired.callback = video_audio_callback;
				desired.userdata = audio_codec_context;

				sound_close_mixer();

				if( SDL_OpenAudio( &desired, &audio_spec ) < 0 ) {
					fprintf( stderr, "Warning: Couldn't open audio for video: %s\n", SDL_GetError() );
					audio_codec_context = NULL;
				}
				else {
					packet_queue_flush( &audio_queue );
					audio_open = 1;
					audio_running = 1;
					SDL_PauseAudio( 0 );
				}
			}
		}
	}

	texture = ogl_create_empty_texture();
	
	if( !texture )
		return -1;
		
	texture->width = video_codec_context->width;
	texture->height = video_codec_context->height;

	stop = 0;
	reader_thread = SDL_CreateThread( video_reader_thread, NULL );
	if( !reader_thread ) {
		fprintf( stderr, "Warning: Couldn't start video reader thread\n" );
		return -1;
	}
		
	return 0;
}

struct texture *video_texture( void ) {
	return texture;
}

void video_clock_tick( void ) {
	video_clock += 1.0 / config_get()->iface.frame_rate;	
}

struct texture *video_get_frame( void ) {
	static AVFrame *frame = NULL;
	int ret = -1;
	GLenum error = GL_NO_ERROR;

	if( frame && frame->opaque && *(double*)(frame->opaque) ) {
		if( got_texture && *(double*)(frame->opaque) > video_clock + FUDGE_FACTOR ) {
			printf( "SOON: PTS: %f %f\n", *(uint64_t*)(frame->opaque), video_clock );
			video_clock_tick();
			return texture;
		}
	}
	
	for(;;) {
		frame = frame_queue_get( &video_queue, 0 );
		if( frame && *(double*)(frame->opaque) < video_clock ) {
			printf( "SKIP: PTS: %f %f\n", *(uint64_t*)(frame->opaque), video_clock );
			av_free( frame );
			frame = NULL;
		}
		else {
			break;
		}
	}
	
	video_clock_tick();
	
	if( !frame )
		return NULL;
		
	scale_context = sws_getCachedContext( scale_context, video_codec_context->width, video_codec_context->height,
		video_codec_context->pix_fmt, VIDEO_SIZE, VIDEO_SIZE, CONV_FORMAT, SWS_BICUBIC, NULL, NULL, NULL );
	if( !scale_context ) {
		fprintf( stderr, "Warning: Couldn't initialise video conversion context\n" );
	}
	else {
		sws_scale( scale_context, frame->data, frame->linesize, 0,
			VIDEO_SIZE, conv_frame->data, conv_frame->linesize );
		
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glBindTexture( GL_TEXTURE_2D, texture->id );
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		
		if( got_texture == 0 ) {
			glTexImage2D( GL_TEXTURE_2D, 0, VIDEO_BPP, VIDEO_SIZE, VIDEO_SIZE, 0,
				GL_RGB, GL_UNSIGNED_BYTE, conv_frame->data[0] );
		}
		else {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VIDEO_SIZE, VIDEO_SIZE,
				GL_RGB, GL_UNSIGNED_BYTE, conv_frame->data[0] );
		}
		
		error = glGetError();
		if( error != GL_NO_ERROR ) {
			fprintf(stderr, "Warning: couldn't create texture: %s.\n", gluErrorString(error) );
		}
		else {
			got_texture = 1;
			ret = 0;	
		}
	}
	
	av_free( frame );
	frame = NULL;
	
	return texture;
}
