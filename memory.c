#include "memory.h"

struct memory
{
    int index;
    int is_allocated;
};


// Create memory blocks table
memory_t **create_mem_table() {
    memory_t **memory = malloc(MAX_MEMORY * sizeof(memory_t **));
    for (int i = 0; i < MAX_MEMORY; i++) {
        memory[i] = malloc(sizeof(memory_t));
        memory[i]->index = i;
        memory[i]->is_allocated = 0;
    }

    return memory;
}


// Allocate memory for specific size
int allocate_mem(memory_t **memory, int size) {

    int min_mem_diff = INT_MAX;
    int min_start_index = -1;

    // traverse mem region finding fittest
    for (int i = 0; i < MAX_MEMORY; i++) {
        if (!memory[i]->is_allocated) {
            int j;
            for (j = i; j < MAX_MEMORY; j++) {
                if (!memory[j]->is_allocated) {
                    continue;
                };
                break;
            }

            int mem_diff = j - i;
            if (mem_diff >= size && mem_diff < min_mem_diff) {
                min_mem_diff = mem_diff;
                min_start_index = i;
            }

            i = j - 1;
        }
    }

    // falied to alloc mem
    if (min_start_index != -1) {
        for (int i = min_start_index; i < min_start_index + size; i++) {
            memory[i]->is_allocated = 1;
        }
    }

    return min_start_index;

}


// Clear memory blocks with specific starts and size
void clear_mem(memory_t **memory, int start, int size) {
    for (int i = start; i < start + size; i++) {
        memory[i]->is_allocated = 0;
    }
}


// Free system memory when allocating blocks
void free_mem(memory_t **memory) {
    for (int i = 0; i < MAX_MEMORY; i++) {
        free(memory[i]);
    }
    free(memory);
}