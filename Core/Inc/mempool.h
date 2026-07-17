#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#define POOL_BLOCK_SIZE 512
#define POOL_BLOCKS		8


bool mempool_init(void);

void *mempool_alloc(void);

bool mempool_free(void *block);

uint32_t mempool_free_count(void);

