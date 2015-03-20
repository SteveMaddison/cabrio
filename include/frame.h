#ifndef _FRAME_H_
#define _FRAME_H_ 1

#include <SDL2/SDL_mutex.h>
#include "video.h"

struct frame_list {
	struct frame_list *next;
	struct frame_list *prev;
	AVFrame *frame;
};

struct frame_queue {
	struct frame_list *first;
	struct frame_list *last;
	int frames;
	SDL_mutex *mutex;
	SDL_cond *cond;
};

void frame_queue_init( struct frame_queue *q );
int frame_queue_put( struct frame_queue *q, AVFrame *f );
AVFrame *frame_queue_get( struct frame_queue *q, int block );
void frame_queue_flush( struct frame_queue *q );

#endif
