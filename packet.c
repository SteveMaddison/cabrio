#include <SDL2/SDL_mutex.h>
#include "packet.h"


void packet_queue_init( struct packet_queue *q ) {
	memset( q, 0, sizeof(struct packet_queue) );
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}

int packet_queue_put( struct packet_queue *q, AVPacket *p ) {
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

int packet_queue_get( struct packet_queue *q, AVPacket *p, int block ) {
	AVPacketList *packet;
	int ret;

	SDL_LockMutex( q->mutex );

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
	}
	else if( !block ) {
		ret = 0;
	}
	else {
		SDL_CondWait( q->cond, q->mutex );
	}

	SDL_UnlockMutex( q->mutex );
	return ret;
}

void packet_queue_flush( struct packet_queue *q ) {
	AVPacketList *packet, *tmp;
	
	SDL_LockMutex(q->mutex);
	
	for( packet = q->first; packet != NULL; packet = tmp ) {
		tmp = packet->next;
		av_free_packet( &packet->pkt );
		av_freep( &packet );
	}
	q->last = NULL;
	q->first = NULL;
	q->packets = 0;
	q->size = 0;
	
	SDL_CondSignal( q->cond );
	SDL_UnlockMutex(q->mutex);
}
