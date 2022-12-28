#ifndef __RING_BUFF_H__
#define __RING_BUFF_H__

#include <stdbool.h>
#include "stdint.h"

#define RINGBUFF_OK        0  /* No error, everything OK. */
#define RINGBUFF_ERR       -1 /* Out of memory error.     */
#define RINGBUFF_EMPTY     -3 /* Timeout.                	    */
#define RINGBUFF_FULL      -4 /* Routing problem.          */
#define RINGBUFF_TOO_SHORT -5

typedef struct _ring_buff_ {
    unsigned int size;
    unsigned int readpoint;
    unsigned int writepoint;
    char*        buffer;
    bool         full;
} ring_buff_t;

int ring_buff_init(ring_buff_t* ring_buff, char* buff, unsigned int size);
int ring_buff_flush(ring_buff_t* ring_buff);
int ring_buff_push_data(ring_buff_t* ring_buff, char* pData, int len);
int ring_buff_pop_data(ring_buff_t* ring_buff, char* pData, int len);
unsigned int ring_buff_get_size(ring_buff_t* ring_buff);

#endif  // __ringbuff_h__
