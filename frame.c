#include <SDL2/SDL_mutex.h>
#include "frame.h"
#include "video.h"


void frame_queue_init( struct frame_queue *q ) {
	memset( q, 0, sizeof(struct frame_queue) );
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

int frame_queue_put( struct frame_queue *q, AVFrame *f ) {
	struct frame_list *frame;
	struct frame_list *after = NULL;
	
	frame = av_malloc( sizeof(struct frame_list) );
	if( !frame ) {
		fprintf( stderr, "Warning: Couldn't allocate frame in queue (av_malloc)\n" );
		return -1;
	}
	
	frame->frame = f;
	
	after = q->last;
	while( after && after->frame->pts > frame->frame->pts ) {
		after = after->prev;
	}
	frame->prev = after;

	SDL_LockMutex( q->mutex );
	
	if( after ) {	
		if( after->next ) {
			/* insert */
			frame->next = after->next;
			if( after->next )
				after->next->prev = frame;
			after->next = frame;
		}
		else {
			/* append */
			frame->next = NULL;			
			after->next = frame;
			q->last = frame;
		}
	}
	else {
		/* add at start */
		frame->next = q->first;
		if( q->first )
			q->first->prev = frame;
		q->first = frame;
	}
	
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
