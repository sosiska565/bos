#include "pmm.h"
#include "../vga/vga.h"
#include "../string/string.h"

static uint32_t* memory_map = 0;
static uint32_t max_frames = 0;
static uint32_t used_frames = 0;

extern uint32_t end;

void pmm_set_frame(uint32_t frame_addr) {
    if (frame_addr < max_frames * PAGE_SIZE) {
        uint32_t frame = frame_addr / PAGE_SIZE;
        uint32_t idx = frame / 32;
        uint32_t off = frame % 32;
        if (!(memory_map[idx] & (1 << off))) {
            memory_map[idx] |= (1 << off);
            used_frames++;
        }
    }
}

void pmm_clear_frame(uint32_t frame_addr) {
    if (frame_addr < max_frames * PAGE_SIZE) {
        uint32_t frame = frame_addr / PAGE_SIZE;
        uint32_t idx = frame / 32;
        uint32_t off = frame % 32;
        if (memory_map[idx] & (1 << off)) {
            memory_map[idx] &= ~(1 << off);
            used_frames--;
        }
    }
}

uint32_t pmm_test_frame(uint32_t frame_addr) {
    if (frame_addr >= max_frames * PAGE_SIZE) return 1;
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    return (memory_map[idx] & (1 << off));
}

uint32_t pmm_first_free_frame() {
    for (uint32_t i = 0; i < max_frames / 32; i++) {
        if (memory_map[i] != 0xFFFFFFFF) {
            for (int j = 0; j < 32; j++) {
                if (!(memory_map[i] & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    return (uint32_t)-1;
}

void pmm_init(multiboot_info_t *mbd) {
    uint64_t highest_addr = 0;
    memory_map_t* mmap = (memory_map_t*)mbd->mmap_addr;
    while((uint32_t)mmap < mbd->mmap_addr + mbd->mmap_length) {
        if (mmap->addr + mmap->len > highest_addr) {
            highest_addr = mmap->addr + mmap->len;
        }
        mmap = (memory_map_t*) ( (uint32_t)mmap + mmap->size + sizeof(uint32_t) );
    }

    max_frames = highest_addr / PAGE_SIZE;
    
    memory_map = (uint32_t*)&end;
    uint32_t bitmap_size = (max_frames / 32);
    
    memset(memory_map, 0xFF, bitmap_size);
    used_frames = max_frames;

    mmap = (memory_map_t*)mbd->mmap_addr;
    while((uint32_t)mmap < mbd->mmap_addr + mbd->mmap_length) {
        if (mmap->type == 1) { 
            for (uint64_t i = 0; i < mmap->len; i += PAGE_SIZE) {
                if(mmap->addr + i > 0)
                    pmm_clear_frame(mmap->addr + i);
            }
        }
        mmap = (memory_map_t*) ( (uint32_t)mmap + mmap->size + sizeof(uint32_t) );
    }
    
    uint32_t kernel_and_bitmap_end = ((uint32_t)&end) + bitmap_size;
    for (uint32_t addr = 0; addr < kernel_and_bitmap_end; addr += PAGE_SIZE) {
        pmm_set_frame(addr);
    }
}

uint32_t pmm_alloc_frame() {
    if (used_frames >= max_frames) {
        return (uint32_t)-1;
    }

    uint32_t frame_idx = pmm_first_free_frame();
    if (frame_idx == (uint32_t)-1) {
        return (uint32_t)-1;
    }

    uint32_t frame_addr = frame_idx * PAGE_SIZE;
    pmm_set_frame(frame_addr);

    return frame_addr;
}

void pmm_free_frame(uint32_t frame_addr) {
    pmm_clear_frame(frame_addr);
}
