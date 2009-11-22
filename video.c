#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
#include <SDL/SDL_audio.h>
#include "video.h"
#include "sound.h"
#include "ogl.h"

static const int VIDEO_SIZE = 256;
static const int CONV_FORMAT = PIX_FMT_RGB24;
static const int VIDEO_BPP = 3;
static const GLfloat VIDEO_SCALE = 0.005;

AVFormatContext *format_context = NULL;
AVCodecContext *video_codec_context = NULL;
AVCodecContext *audio_codec_context = NULL;
AVCodec *video_codec = NULL;
AVCodec *audio_codec = NULL;
AVFrame *raw_frame = NULL;
AVFrame *conv_frame = NULL;
AVPacket packet;
uint8_t *buffer;
int video_stream = -1;
int audio_stream = -1;
struct packet_queue audio_queue;
struct SwsContext *scale_context;
struct texture *texture;
static int got_texture = 0;
GLfloat xsize, ysize;
int stop = 0;

int video_stopped( void ) {
	return stop;
}

int video_init( void ) {
	avcodec_register_all();
	av_register_all();
	
	raw_frame = avcodec_alloc_frame();
	conv_frame = avcodec_alloc_frame();
	
	url_set_interrupt_cb( video_stopped );
	
	if( !raw_frame || !conv_frame ) {
		fprintf( stderr, "Warning: Couldn't allocate video frame\n" );
		return -1;
	}
	
	return 0;
}

void video_free( void ) {
	video_close();

	if( raw_frame )
		av_free( raw_frame );
	raw_frame = NULL;

	if( conv_frame );	
		av_free( conv_frame );
	conv_frame = NULL;
}

void video_close( void ) {
	stop = 1;

	if( video_codec_context )
		avcodec_close( video_codec_context );
	video_codec_context = NULL;

	if( audio_codec_context )
		avcodec_close( audio_codec_context );
	audio_codec_context = NULL;
	
	if( format_context )
		av_close_input_file( format_context );
	format_context = NULL;

	if( buffer )
		av_free( buffer );
	buffer = NULL;
	
	if( texture )
		ogl_free_texture( texture );
	texture = NULL;
	
	Mix_HookMusic( NULL, NULL );
	
	video_stream = -1;
	audio_stream = -1;
	got_texture = 0;
}

int video_has_texture( void ) {
	return got_texture;
}

struct texture *video_texture( void ) {
	return texture;
}

void video_packet_queue_init( struct packet_queue *q ) {
	memset( q, 0, sizeof(struct packet_queue) );
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

int video_packet_queue_put( struct packet_queue *q, AVPacket *p ) {
	AVPacketList *packet;
	
	if( av_dup_packet( p ) < 0 ) {
		fprintf( stderr, "Warning: Couldn't allocate packet in queue (av_dup_packet)\n" );
		return -1;
	}
	
	packet = av_malloc( sizeof(AVPacketList) );
	if( !packet ) {
		fprintf( stderr, "Warning: Couldn't allocate packet in queue (av_malloc)\n" );
		return -1;
	}
	
	packet->pkt = *p;
	packet->next = NULL;

	SDL_LockMutex( q->mutex );

	if( q->last )
		q->last->next = packet;
	else		
		q->first = packet;
	
	q->last = packet;
	q->packets++;
	q->size += packet->pkt.size;
	
	SDL_CondSignal( q->cond );
	SDL_UnlockMutex( q->mutex );
	
	return 0;
}

int video_packet_queue_get( struct packet_queue *q, AVPacket *p, int block ) {
	AVPacketList *packet;
	int ret;

	SDL_LockMutex( q->mutex );

	for(;;) {
		if( stop ) {
			ret = -1;
			break;
		}

		packet = q->first;
		if( packet ) {
			q->first = packet->next;
			
		if( !q->first )
			q->last = NULL;
			q->packets--;
			q->size -= packet->pkt.size;
			*p = packet->pkt;
			av_free( packet );
			ret = 1;
			break;
		}
		else if( !block ) {
			ret = 0;
			break;
		}
		else {
			SDL_CondWait( q->cond, q->mutex );
		}
	}
	SDL_UnlockMutex( q->mutex );
	return ret;
}

int video_decode_audio_frame( AVCodecContext *context, uint8_t *audio_buffer, int buffer_size ) {
	static AVPacket packet;
	static uint8_t *audio_packet_data = NULL;
	static int audio_packet_size = 0;
	int used, data_size;

	for(;;) {
		while( audio_packet_size > 0 ) {
			data_size = buffer_size;
			used = avcodec_decode_audio2( context, (int16_t*)audio_buffer,
				&data_size, audio_packet_data, audio_packet_size);
			
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

		if( stop )
			return -1;

		if( video_packet_queue_get( &audio_queue, &packet, 1) < 0 )
			return -1;

		audio_packet_data = packet.data;
		audio_packet_size = packet.size;
	}
}

void video_audio_callback( void *userdata, Uint8 *stream, int length ) {
	AVCodecContext *context = (AVCodecContext*)userdata;
	int used, audio_size;
	static uint8_t audio_buffer[(AVCODEC_MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buffer_size = 0;
	static unsigned int audio_buffer_index = 0;

	while( length > 0 ) {
		if( audio_buffer_index >= audio_buffer_size ) {
			/* We have already sent all our data; get more */
			audio_size = video_decode_audio_frame( context, audio_buffer, sizeof(audio_buffer) );
			if( audio_size < 0 ) {
				/* If error, output silence */
				audio_buffer_size = sound_chunk_size();
				memset( audio_buffer, 0, audio_buffer_size );
			}
			else {
				audio_buffer_size = audio_size;
			}
			audio_buffer_index = 0;
		}

		used = audio_buffer_size - audio_buffer_index;
		if( used > length )
			used = length;

		memcpy( stream, (uint8_t*)audio_buffer + audio_buffer_index, used );
		length -= used;
		stream += used;
		audio_buffer_index += used;
	}
}

int video_open( const char *filename ) {
	int i = -1;
	
	format_context = NULL;
	video_codec_context = NULL;
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
		buffer = (uint8_t*)av_malloc( bytes *sizeof(uint8_t) );
	}
	
	if( !buffer ) {
		fprintf( stderr, "Warning: Couldn't allocate buffer for video '%s'\n", filename );
		return -1;
	}

	avpicture_fill( (AVPicture*)conv_frame, buffer, CONV_FORMAT, VIDEO_SIZE, VIDEO_SIZE );

	if( sound_open() && audio_codec_context ) {
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
				video_packet_queue_init( &audio_queue );
				/*Mix_HookMusic( video_audio_callback, audio_codec_context );*/
			}
		}
	}

	texture = ogl_create_empty_texture();
	
	if( !texture )
		return -1;
		
	xsize = ((GLfloat)video_codec_context->width) * VIDEO_SCALE / 2.0;
	ysize = ((GLfloat)video_codec_context->height) * VIDEO_SCALE / 2.0;
	texture->width = video_codec_context->width;
	texture->height = video_codec_context->height;
		
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
	
	if( !format_context || !video_codec_context || !buffer )
		return -1;
		
	if( av_read_frame( format_context, &packet ) >= 0 ) {
		if( packet.stream_index == video_stream ) {
			avcodec_decode_video( video_codec_context, raw_frame, &got_frame, packet.data, packet.size );
		
			if( got_frame ) {
				scale_context = sws_getCachedContext( scale_context, video_codec_context->width, video_codec_context->height,
					video_codec_context->pix_fmt, VIDEO_SIZE, VIDEO_SIZE, CONV_FORMAT, SWS_BICUBIC, NULL, NULL, NULL );
				if( !scale_context ) {
					fprintf( stderr, "Warning: Couldn't initialise video conversion context\n" );
					return -1;
				}

				sws_scale( scale_context, raw_frame->data, raw_frame->linesize, 0,
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
					got_texture = 1;
				}
				else {
					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, VIDEO_SIZE, VIDEO_SIZE,
						GL_RGB, GL_UNSIGNED_BYTE, conv_frame->data[0] );
				}

				error = glGetError();
				if( error != GL_NO_ERROR ) {
					fprintf(stderr, "Warning: couldn't create texture: %s.\n", gluErrorString(error) );
				}
			}
			av_free_packet(&packet);
		}
		else if( packet.stream_index == video_stream ) {
			video_packet_queue_put( &audio_queue, &packet );
		}
		else {
			av_free_packet( &packet );
		}
	}
	else {
		video_rewind();	
	}
	
	return 0;
}

