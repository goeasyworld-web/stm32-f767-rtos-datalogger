#include <stdio.h>
#include <stdbool.h>
#include <main.h>
#include <stdint.h>
#include "mempool.h"

#define SRC_ADC 1
#define SRC_BME 2

typedef struct{
	uint8_t source;
	int32_t value;
	uint32_t timestamp;
	void * payload;
}sensor_msg_t;

typedef struct
{
	uint16_t samples[240];
	uint16_t min;
	uint16_t max;
	uint16_t avg;
	uint16_t count;
} adc_batch_t;

_Static_assert(sizeof(adc_batch_t) <= POOL_BLOCK_SIZE, "Batch too big for pool block");
_Static_assert(sizeof(sensor_msg_t) == 16, "queue item size must match");
