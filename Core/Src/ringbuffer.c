#include <stdio.h>
#include <stdbool.h>
#include "ringbuffer.h"



bool rb_is_full(ringbuf_t *rb)
{
	return (((rb->head +1) % RB_SIZE) == rb->tail);
}


bool rb_is_empty(ringbuf_t *rb)
{
	return rb->head == rb->tail;
}

bool rb_store(ringbuf_t *rb, uint8_t byte)
{
	if(rb_is_full(rb))
	{
		return false;
	}
	else
	{
		rb->bufferdata[rb->head] = byte;
		rb->head = (rb->head +1) % RB_SIZE;
		return true;
	}
}

bool rb_receive(ringbuf_t *rb, uint8_t *byte)
{
	if(rb_is_empty(rb))
	{
		return false;
	}
	else
	{
		*byte = rb->bufferdata[rb->tail];
		rb->tail = (rb->tail + 1) % RB_SIZE;
		return true;
	}
}

void rb_init(ringbuf_t *rb)
{
	rb->head = 0;
	rb->tail = 0;
	for(uint8_t i = 0; i<RB_SIZE; i++)
	{
		rb->bufferdata[i] = 0;
	}
}










