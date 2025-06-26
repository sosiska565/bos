#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

void* kmalloc(uint32_t size);
void kfree(void* p);
void* kmalloc_a(uint32_t size);
void* kmalloc_p(uint32_t size, uint32_t *phys);
void* kmalloc_ap(uint32_t size, uint32_t *phys);

#endif