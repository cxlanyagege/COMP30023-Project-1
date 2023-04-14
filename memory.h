#ifndef _MEMORY_H_
#define _MEMORY_H_

#include <stdlib.h>
#include <limits.h>

#define MAX_MEMORY 2048

typedef struct memory memory_t;

memory_t **create_mem_table();

int allocate_mem(memory_t **memory, int size);

void clear_mem(memory_t **memory, int start, int size);

#endif