#ifndef _FRAME_H_
#define _FRAME_H_ 1

#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <SDL/SDL_mutex.h>

struct frame_list {
	struct frame_list *next;
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
