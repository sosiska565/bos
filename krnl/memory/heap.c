#include "heap.h"
#include "../string/string.h"
#include "vmm.h"
#include "pmm.h"
#include "memory.h"

extern page_directory_t* kernel_directory;
heap_t *kheap = 0;

static int8_t header_t_less_than(void*a, void*b)
{
  return (((header_t*)a)->size < ((header_t*)b)->size)?1:0;
}

heap_t *create_heap(uint32_t start, uint32_t end_addr, uint32_t max, uint8_t supervisor, uint8_t readonly)
{
  heap_t *heap = (heap_t*)kmalloc(sizeof(heap_t));

  if (start % 0x1000 != 0 || end_addr % 0x1000 != 0) {
      PANIC("Heap start or end address not page-aligned");
  }

  heap->index = place_ordered_array((void*)start, HEAP_INDEX_SIZE, &header_t_less_than);
  
  start += sizeof(type_t)*HEAP_INDEX_SIZE;

  if ((start & 0xFFFFF000) != 0)
  {
    start &= 0xFFFFF000;
    start += 0x1000;
  }
  heap->start_address = start;
  heap->end_address = end_addr;
  heap->max_address = max;
  heap->supervisor = supervisor;
  heap->readonly = readonly;

  header_t *hole = (header_t *)start;
  hole->size = end_addr-start;
  hole->magic = HEAP_MAGIC;
  hole->is_hole = 1;
  insert_ordered_array((void*)hole, &heap->index);

  return heap;
}

static void expand(uint32_t new_size, heap_t *heap)
{
   if (new_size <= heap->end_address - heap->start_address) {
       PANIC("New heap size must be greater than current size");
   }

   if ((new_size & 0xFFFFF000) != 0)
   {
     new_size &= 0xFFFFF000;
     new_size += 0x1000;
   }

   if (heap->start_address+new_size > heap->max_address) {
       PANIC("Cannot expand heap beyond max address");
   }

   uint32_t old_size = heap->end_address-heap->start_address;
   uint32_t i = old_size;
   while (i < new_size)
   {
                   vmm_map_page(vmm_get_dir(), heap->start_address+i, pmm_alloc_frame(), (heap->supervisor)?(PAGE_PRESENT|PAGE_WRITE):(PAGE_PRESENT|PAGE_WRITE|PAGE_USER));
       i += 0x1000;
   }
   heap->end_address = heap->start_address+new_size;
}

static uint32_t contract(uint32_t new_size, heap_t *heap)
{
    if (new_size >= heap->end_address-heap->start_address) {
        PANIC("New heap size must be smaller than current size");
    }

    if (new_size & 0x1000)
    {
        new_size &= 0x1000;
        new_size += 0x1000;
    }

    if (new_size < HEAP_MIN_SIZE) new_size = HEAP_MIN_SIZE;
    uint32_t old_size = heap->end_address-heap->start_address;
    uint32_t i = old_size - 0x1000;
    while (new_size < i)
    {
        //vmm_free_page(heap->start_address+i);
        i -= 0x1000;
    }
    heap->end_address = heap->start_address + new_size;
    return new_size;
}

void *alloc(uint32_t size, uint8_t page_align, heap_t *heap)
{

  uint32_t new_size = size + sizeof(header_t) + sizeof(footer_t);
  int32_t iterator = 0;
      while (iterator < (int32_t)heap->index.size && ((header_t*)lookup_ordered_array(iterator, &heap->index))->size < new_size)
      iterator++;

  if (iterator == heap->index.size)
  {
    uint32_t old_length = heap->end_address - heap->start_address;
    uint32_t new_length = old_length + new_size;
    expand(new_length, heap);
    return alloc(size, page_align, heap);
  }

  header_t *orig_hole_header = (header_t *)lookup_ordered_array(iterator, &heap->index);
  uint32_t orig_hole_pos = (uint32_t)orig_hole_header;
  uint32_t orig_hole_size = orig_hole_header->size;
  if (orig_hole_size - new_size < sizeof(header_t) + sizeof(footer_t))
  {
    size += orig_hole_size-new_size;
    new_size = orig_hole_size;
  }

  if (page_align && orig_hole_pos & 0xFFFFF000)
  {
    uint32_t new_location   = orig_hole_pos + 0x1000 - (orig_hole_pos&0xFFF) - sizeof(header_t);
    header_t *hole_header = (header_t *)orig_hole_pos;
    hole_header->size     = 0x1000 - (orig_hole_pos&0xFFF) - sizeof(header_t);
    hole_header->magic    = HEAP_MAGIC;
    hole_header->is_hole  = 1;
    footer_t *hole_footer = (footer_t *) ( (uint32_t)new_location - sizeof(footer_t) );
    hole_footer->magic    = HEAP_MAGIC;
    hole_footer->header   = hole_header;
    orig_hole_pos         = new_location;
    orig_hole_size        = orig_hole_size - hole_header->size;
  }
  else
  {
    remove_ordered_array(iterator, &heap->index);
  }

  header_t *block_header  = (header_t *)orig_hole_pos;
  block_header->magic     = HEAP_MAGIC;
  block_header->is_hole   = 0;
  block_header->size      = new_size;

  footer_t *block_footer  = (footer_t *) (orig_hole_pos + sizeof(header_t) + size);
  block_footer->magic     = HEAP_MAGIC;
  block_footer->header    = block_header;

  if (orig_hole_size - new_size > 0)
  {
    header_t *hole_header = (header_t *) (orig_hole_pos + sizeof(header_t) + size + sizeof(footer_t));
    hole_header->magic    = HEAP_MAGIC;
    hole_header->is_hole  = 1;
    hole_header->size     = orig_hole_size - new_size;
    footer_t *hole_footer = (footer_t *) ( (uint32_t)hole_header + orig_hole_size - new_size - sizeof(footer_t) );
    if ((uint32_t)hole_footer < heap->end_address)
    {
      hole_footer->magic = HEAP_MAGIC;
      hole_footer->header = hole_header;
    }
    insert_ordered_array((void*)hole_header, &heap->index);
  }
  
  return (void *) ( (uint32_t)block_header + sizeof(header_t) );
}

void init_heap() {
    kheap = create_heap(KHEAP_START, KHEAP_START + KHEAP_INITIAL_SIZE, 0xCFFFF000, 0, 0);
}

void free(void *p, heap_t *heap)
{
  if (p == 0)
    return;

  header_t *header = (header_t*) ( (uint32_t)p - sizeof(header_t) );
  if (header->magic != HEAP_MAGIC) {
      PANIC("Invalid header magic in free()");
  }
  footer_t *footer = (footer_t*) ( (uint32_t)header + header->size - sizeof(footer_t) );
  if (footer->magic != HEAP_MAGIC) {
      PANIC("Invalid footer magic in free()");
  }

  header->is_hole = 1;
  char do_add = 1;

  footer_t *test_footer = (footer_t*) ( (uint32_t)header - sizeof(footer_t) );
  if (test_footer->magic == HEAP_MAGIC &&
      test_footer->header->is_hole == 1)
  {
    uint32_t cache_size = header->size;
    header = test_footer->header;
    header->size += cache_size;
    do_add = 0;
  }

  header_t *test_header = (header_t*) ( (uint32_t)footer + sizeof(footer_t) );
  if (test_header->magic == HEAP_MAGIC &&
      test_header->is_hole)
  {
    header->size += test_header->size;
    remove_ordered_array((uint32_t)test_header, &heap->index);
  }

  if ((uint32_t)footer+sizeof(footer_t) == heap->end_address)
  {
    uint32_t old_length = heap->end_address-heap->start_address;
    uint32_t new_length = contract(header->size, heap);
    if (header->size - (old_length-new_length) > 0)
    {
      header->size -= old_length-new_length;
      footer = (footer_t*) ( (uint32_t)header + header->size - sizeof(footer_t) );
      footer->magic = HEAP_MAGIC;
      footer->header = header;
    }
    else
    {
      remove_ordered_array((uint32_t)header, &heap->index);
    }
  }

  if (do_add == 1)
    insert_ordered_array((void*)header, &heap->index);

}
