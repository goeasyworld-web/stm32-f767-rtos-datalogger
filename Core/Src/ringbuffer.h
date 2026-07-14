#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


#ifndef	RINGBUFFER_H
#define RINGBUFFER_H

#define RB_SIZE			(64u)

typedef struct ringbuffer{
	uint8_t bufferdata[RB_SIZE];
	volatile uint8_t head;
	volatile uint8_t tail;
}ringbuf_t;




void rb_init(ringbuf_t	*rb);
bool rb_store(ringbuf_t *rb, uint8_t byte);
bool rb_receive(ringbuf_t *rb, uint8_t *byte);
bool rb_is_empty(ringbuf_t*);
bool rb_is_full(ringbuf_t*);

#endif



