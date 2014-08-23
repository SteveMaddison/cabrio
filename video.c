/*
 * Adapted from the ffmpeg tutorial at http://www.dranger.com/ffmpeg/,
 * in turn based on FFplay, Copyright (c) 2003 Fabrice Bellard,
 * and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
 */

#include <SDL/SDL.h>
#include <SDL/SDL_audio.h>
#include "video.h"
#include "sound.h"
#include "packet.h"
#include "frame.h"
#include "config.h"
#include "ogl.h"
#include <unistd.h>

#define AUDIO_BUFFER_SIZE ((AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2)
static const int VIDEO_SIZE = 256;
static const int VIDEO_SIZE_SCALE = 768;
static const int CONV_FORMAT = PIX_FMT_RGB24;
static const int VIDEO_BPP = 3;
static const int MAX_QUEUE_PACKETS = 20;
static const int QUEUE_FULL_DELAY = 10;
static const GLfloat VIDEO_SCALE = 0.005;
static const int MAX_QUEUE_FRAMES = 30;
static const int SAMPLES = 1024;
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
static uint64_t video_pts = 0;

static AVCodecContext *audio_codec_context = NULL;
static AVCodec *audio_codec = NULL;
static int audio_stream = -1;
static struct packet_queue audio_queue;
static uint8_t audio_buffer[AUDIO_BUFFER_SIZE];
static unsigned int audio_buffer_size = 0;
static unsigned int audio_buffer_index = 0;
static AVPacket audio_packet; 
static double audio_clock = 0;

SDL_AudioSpec audio_spec;
static int audio_open = 0;

static SDL_Thread *reader_thread = NULL;
static int audio_running = 0;
static int got_texture = 0;
static struct texture *texture;
static int stop = 0;
int reader_running = 0;
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

	return 0;
}

void video_free( void ) {
	video_close();

	if( conv_frame )	
		av_free( conv_frame );
	conv_frame = NULL;
}

void video_close( void ) {
	int timeout = 10;
	stop = 1;
	const struct config *config = config_get();

	packet_queue_flush( &audio_queue );
	frame_queue_flush( &video_queue );

	while( (reader_running || audio_running) && timeout-- )
		SDL_Delay( 10 );
	if (config->iface.video_sound) {
		if( audio_open )
			SDL_CloseAudio();
	}
	audio_open = 0;

	if( video_codec_context )
		avcodec_close( video_codec_context );
	video_codec_context = NULL;

	if( audio_codec_context )
		avcodec_close( audio_codec_context );
	audio_codec_context = NULL;
	
	if( format_context )
		avformat_close_input( &format_context );
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
	int got_frame = 0;
	static double pts = 0;
	
	if( packet ) {		
		if( !frame ) {
			frame = avcodec_alloc_frame();
			if( !frame ) {
				fprintf( stderr, "Warning: Couldn't allocate video frame\n" );
				return -1;
			}
		}
		
		if( !frame ) {
			fprintf( stderr, "Warning: Frame disappeared\n" );
			return -1;
		}
		
		video_pts = packet->pts;
		avcodec_decode_video2( video_codec_context, frame, &got_frame, packet );
		
		if( packet->dts == AV_NOPTS_VALUE && frame->opaque && *(uint64_t*)frame->opaque != AV_NOPTS_VALUE )
			pts = *(uint64_t *)frame->opaque;
		else if( packet->dts != AV_NOPTS_VALUE )
			pts = packet->dts;
		else
			pts = 0;
		pts *= av_q2d( format_context->streams[video_stream]->time_base );
		
		if( got_frame ) {
			double *frame_pts = malloc( sizeof(double) );
			*frame_pts = pts;
			frame->opaque = frame_pts;
			frame_queue_put( &video_queue, frame );
			frame = NULL;
		}
		av_free_packet( packet );
	}
	return 0;
}

int video_decode_audio_frame( AVCodecContext *context, uint8_t *buffer, int buffer_size ) {
	static AVPacket packet;
	int used, data_size;

#if LIBAVCODEC_VERSION_MAJOR > 53
// 	Libavcodec54
	AVFrame * frame = avcodec_alloc_frame ();
#endif /* LIBAVCODEC_VERSION_MAJOR > 53 */
	for(;;) {
		while (audio_packet.size > 0) {
#if LIBAVCODEC_VERSION_MAJOR > 53
// Work in progress Libavcodec54 no sound ...
		data_size = buffer_size;
		used = avcodec_decode_audio4( context, frame, &data_size,  &packet);
		if( used < 0 ) {
			// if error, skip frame //
			fprintf(stderr, "Error while decoding\n");
			audio_packet.size = 0;
			break;
		}
		audio_packet.data += used;
		audio_packet.size -= used;

		if( data_size <= 0 ) {
		// No data yet, get more frames //
			continue;
		}
						

		// if a frame has been decoded, output it //
			data_size = av_samples_get_buffer_size(NULL, context->channels,frame->nb_samples,context->sample_fmt, 1);

			audio_clock += (double)data_size /
				(double)(format_context->streams[audio_stream]->codec->sample_rate *
				(2 * format_context->streams[audio_stream]->codec->channels));
			return data_size;
		}
#endif /* LIBAVCODEC_VERSION_MAJOR > 53 */
// Libavcodec53
#if LIBAVCODEC_VERSION_MAJOR <= 53

		data_size = buffer_size;
		// Todo depreciated //
		used = avcodec_decode_audio3( context, (int16_t *)audio_buffer, &data_size,
			&audio_packet);
		if( used < 0 ) {
			// if error, skip frame //
			audio_packet.size = 0;
			break;
		}
		audio_packet.data += used;
		audio_packet.size -= used;
			
		if( data_size <= 0 ) {
			// No data yet, get more frames //
			continue;
		}
			
		audio_clock += (double)data_size /
			(double)(format_context->streams[audio_stream]->codec->sample_rate *
			(2 * format_context->streams[audio_stream]->codec->channels));
		// We have data, return it and come back for more later //
		return data_size;
		}

#endif /* LIBAVCODEC_VERSION_MAJOR < 53 */
		if( packet.data )
			av_free_packet( &packet );

		if( stop ) {
			audio_running = 0;
			return -1;
		}

		if( packet_queue_get( &audio_queue, &packet, 1 ) < 0 )
			return -1;

		audio_packet.data = packet.data;
		audio_packet.size = packet.size;

		if( packet.pts != AV_NOPTS_VALUE ) {
			audio_clock = packet.pts * av_q2d( format_context->streams[audio_stream]->time_base );
		}
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
				audio_buffer_size = SAMPLES;
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

void video_release_buffer( struct AVCodecContext *c, AVFrame *f ) {
	if( f )
		av_freep( &f->opaque );
	avcodec_default_release_buffer( c, f );
}

int video_reader_thread( void *data ) {
	static AVPacket packet;
	const struct config *config = config_get();

	if( !format_context || !video_codec_context || !video_buffer )
		return -1;
	reader_running = 1;
	int value;
	while( !stop ) {
		if( video_queue.frames >= MAX_QUEUE_FRAMES || audio_queue.packets >= MAX_QUEUE_PACKETS ) {
			SDL_Delay( QUEUE_FULL_DELAY );
		}
		else {
			if( av_read_frame( format_context, &packet ) >= 0 ) {
				value = packet.stream_index;
				if( value == video_stream ) { 
					video_decode_video_frame( &packet );
				}
				if (!config->iface.video_sound) { 
				// Yes I know very ugly	ishould find another way ...
					usleep( 20000 );
				} else {
					packet_queue_put( &audio_queue, &packet );
				}  
				if (( value != video_stream ) && (value != audio_stream && audio_codec_context))
					av_free_packet( &packet );
			} else {
				// stop video loop
                                if(!config->iface.video_loop) {
					stop = 1;
					video_close();
				} else {
					av_seek_frame( format_context, -1, 0, 0 );
				}
			}
		}
	}
	reader_running = 0;
	return 0;
}

int video_open( const char *filename ) {
	int i = -1;
	
	const struct config *config = config_get();
	format_context = NULL;
	video_codec_context = NULL;
	video_stream = -1;
	video_clock = 0;
	
	audio_codec_context = NULL;
	audio_stream = -1;
	audio_buffer_size = 0;
	audio_buffer_index = 0;

	av_init_packet(&audio_packet);
	
	if( !filename )
		return -1;

	if( avformat_open_input( &format_context, filename, NULL, 0 ) != 0 ) {
		fprintf( stderr, "Warning: Error opening video file '%s'\n", filename );
		return -1;
	}

	if( avformat_find_stream_info( format_context, NULL ) < 0 ) {
		fprintf( stderr, "Warning: Error reading stream info from '%s'\n", filename );
		return -1;
	}
	
	for( i = 0 ; i < format_context->nb_streams ; i++ ) {
		if( format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
			video_stream = i;
			video_codec_context = format_context->streams[video_stream]->codec;
		}
		else if( format_context->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO ) {
			audio_stream = i;
			audio_codec_context = format_context->streams[audio_stream]->codec;
		}
	}
	if( !video_codec_context ) {
		fprintf( stderr, "Warning: Couldn't find video stream in '%s'\n", filename );
		return -1;
	}
	
	video_codec_context->release_buffer = video_release_buffer;
	video_codec = avcodec_find_decoder( video_codec_context->codec_id );
	if( !video_codec ) {
		fprintf( stderr, "Warning: Video codec in video '%s' not supported\n", filename );
		return -1;
	}
	
	if( avcodec_open2( video_codec_context, video_codec, NULL ) != 0 ) {
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

     	if (music && config->iface.video_sound){
        	Mix_HaltMusic();
	} 
	if( audio_codec_context ) {
		audio_codec = avcodec_find_decoder( audio_codec_context->codec_id );
		if( !audio_codec ) {
			fprintf( stderr, "Warning: Audio codec in video '%s' not supported\n", filename );
			audio_codec_context = NULL;
		}
		else {
			if( avcodec_open2( audio_codec_context, audio_codec, NULL ) != 0 ) {
				fprintf( stderr, "Warning: Couldn't open audio codec '%s' for '%s'\n", audio_codec->name, filename );
				audio_codec_context = NULL;
			}
			else {
				SDL_AudioSpec desired;
				
				desired.freq = audio_codec_context->sample_rate;
				desired.format = AUDIO_S16SYS;
				desired.channels = audio_codec_context->channels;
				desired.silence = 0;
				desired.samples = SAMPLES;
				desired.callback = video_audio_callback;
				desired.userdata = audio_codec_context;
				if (config->iface.video_sound) {
					sound_close_mixer();
					if ( SDL_OpenAudio( &desired, &audio_spec ) < 0 ) {
						fprintf( stderr, "Warning: Couldn't open audio for video: %s\n", SDL_GetError() );
						audio_codec_context = NULL;
					} else {
						packet_queue_flush( &audio_queue );
						audio_open = 1;
						audio_running = 1;
						SDL_PauseAudio( 0 );
					} 
				} else {
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
	got_texture = 0;
	
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

double get_audio_clock( void ) {
	double pts;
	int buffer_size, bytes_per_sec;

	pts = audio_clock; /* maintained in the audio thread */
	buffer_size = audio_buffer_size - audio_buffer_index;
	bytes_per_sec = 0;
	
	if( audio_codec_context ) {
		int n = audio_codec_context->channels * 2;
		bytes_per_sec = audio_codec_context->sample_rate * n;
	}
	if( bytes_per_sec ) {
		pts -= (double)buffer_size / bytes_per_sec;
	}
	return pts;
}

double video_master_clock( void ) {
	if( audio_codec_context )
		/* Sync video to audio, if we have it */
		return get_audio_clock();
	else
		/* Otherwise, sync to the video clock (based on frame rate) */
		return video_clock;
}

void video_clock_tick( void ) {
	if( config_get()->iface.frame_rate )
		video_clock += 1.0 / config_get()->iface.frame_rate;
	else
		video_clock += 1.0;
}

struct texture *video_get_frame( void ) {
	static AVFrame *frame = NULL;
	GLenum error = GL_NO_ERROR;
	double clock = video_master_clock();

	if( frame && frame->opaque && *(double*)(frame->opaque) ) {
		if( got_texture && *(double*)(frame->opaque) > clock + FUDGE_FACTOR ) {
			/* Reuse current frame */
			video_clock_tick();
			return texture;
		}
	}

	frame = frame_queue_get( &video_queue, 0 );
	while( frame && *(double*)(frame->opaque) < clock ) {
		av_free( frame );
		frame = frame_queue_get( &video_queue, 0 );
	}
	
	video_clock_tick();
	
	if( !frame )
		return texture;
		
	scale_context = sws_getCachedContext( scale_context, video_codec_context->width, video_codec_context->height,
		video_codec_context->pix_fmt, VIDEO_SIZE, VIDEO_SIZE, CONV_FORMAT, SWS_BICUBIC, NULL, NULL, NULL );
	if( !scale_context ) {
		fprintf( stderr, "Warning: Couldn't initialise video conversion context\n" );
	}
	else {
		sws_scale( scale_context, (const uint8_t **)frame->data, frame->linesize, 0,
			VIDEO_SIZE_SCALE, conv_frame->data, conv_frame->linesize );
		
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
		}
	}
	
	av_free( frame );
	frame = NULL;
	
	return texture;
}
