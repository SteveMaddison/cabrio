#ifndef _PACKET_H_
#define _PACKET_H_ 1

#include <SDL/SDL_mutex.h>
#include "video.h"

struct packet_queue {
	AVPacketList *first;
	AVPacketList *last;
	int packets;
	int size;
	SDL_mutex *mutex;
	SDL_cond *cond;
};

void packet_queue_init( struct packet_queue *q );
int packet_queue_put( struct packet_queue *q, AVPacket *p );
int packet_queue_get( struct packet_queue *q, AVPacket *p, int block );
void packet_queue_flush( struct packet_queue *q );

#endif
