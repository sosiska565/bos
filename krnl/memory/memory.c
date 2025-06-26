#include "memory.h"
#include <stdint.h>

#define KERNEL_HEAP_SIZE (1024 * 1024)

static uint8_t kernel_heap[KERNEL_HEAP_SIZE];
static uint32_t heap_top = 0;

void init_memory(void){
    heap_top = (uint32_t)kernel_heap;
}

void* kmalloc(size_t size){
    if(heap_top + size > (uint32_t)kernel_heap + KERNEL_HEAP_SIZE){
        return NULL;
    }

    void* ptr = (void *)heap_top;
    heap_top += size;
    return ptr;
}

void kfree(void* ptr){
    (void)ptr;
}