#include "memory.h"

#include "heap.h"
#include "vmm.h"

extern heap_t *kheap;

void* kmalloc(uint32_t size)
{
  return alloc(size, 0, kheap);
}

void kfree(void *p)
{
  free(p, kheap);
}

void* kmalloc_a(uint32_t size)
{
  return alloc(size, 1, kheap);
}

void* kmalloc_p(uint32_t size, uint32_t *phys)
{
  uint32_t mem = (uint32_t)alloc(size, 1, kheap);
  if (phys)
  {
    *phys = vmm_get_physical_address(mem);
  }
  return (void*)mem;
}

void* kmalloc_ap(uint32_t size, uint32_t *phys)
{
  return kmalloc_p(size, phys);
}