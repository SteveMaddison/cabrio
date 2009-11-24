#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <SDL/SDL_mutex.h>
#include "frame.h"


void frame_queue_init( struct frame_queue *q ) {
	memset( q, 0, sizeof(struct frame_queue) );
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

int frame_queue_put( struct frame_queue *q, AVFrame *f ) {
	struct frame_list *frame;
	
	frame = av_malloc( sizeof(struct frame_list) );
	if( !frame ) {
		fprintf( stderr, "Warning: Couldn't allocate frame in queue (av_malloc)\n" );
		return -1;
	}
	
	frame->frame = f;
	frame->next = NULL;

	SDL_LockMutex( q->mutex );

	if( q->last )
		q->last->next = frame;
	else		
		q->first = frame;
	
	q->last = frame;
	q->frames++;
	
	SDL_CondSignal( q->cond );
	SDL_UnlockMutex( q->mutex );
	
	return 0;
}

AVFrame *frame_queue_get( struct frame_queue *q, int block ) {
	AVFrame *ret = NULL;
	struct frame_list *frame;

	SDL_LockMutex( q->mutex );

	frame = q->first;
	if( frame ) {
		q->first = frame->next;
		
	if( !q->first )
		q->last = NULL;
		q->frames--;
		ret = frame->frame;
		av_free( frame );
	}
	else if( !block ) {
		ret = NULL;
	}
	else {
		SDL_CondWait( q->cond, q->mutex );
	}

	SDL_UnlockMutex( q->mutex );
	return ret;
}

void frame_queue_flush( struct frame_queue *q ) {
	struct frame_list *frame, *tmp;
	
	SDL_LockMutex(q->mutex);
	
	for( frame = q->first; frame != NULL; frame = tmp ) {
		tmp = frame->next;
		av_freep( &frame );
	}
	q->last = NULL;
	q->first = NULL;
	q->frames = 0;
	
	SDL_UnlockMutex(q->mutex);
}
