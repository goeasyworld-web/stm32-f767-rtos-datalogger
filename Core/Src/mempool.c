#include "main.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include <mempool.h>
#include <stdbool.h>



static uint8_t pool_blocks[POOL_BLOCKS][POOL_BLOCK_SIZE];

static osMessageQueueId_t pool_freq;


bool mempool_init()
{
	pool_freq = osMessageQueueNew(POOL_BLOCKS, sizeof(void*), NULL);
	if(pool_freq == NULL)
	{
		return false;
	}

	for(uint32_t i = 0; i<POOL_BLOCKS; i++)
	{
		void *key = &pool_blocks[i];

		if(osMessageQueuePut(pool_freq, &key, 0, 0) != osOK)
		{
			return false;
		}
	}
	return true;
}

void *mempool_alloc()
{
	void *key = NULL;
	if(osMessageQueueGet(pool_freq, &key, 0, 0)!= osOK)
	{
		return NULL;
	}
	return key;
}

bool mempool_free(void *block)
{
	if(block == NULL)
	{
		return false;
	}
	if(osMessageQueuePut(pool_freq, &block, 0,0) !=osOK)
	{
		return false;
	}
	return true;
}
