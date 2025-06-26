#ifndef PMM_H
#define PMM_H

#include <stdint.h>
#include "multiboot.h"
#include "../interrupts/idt.h"

#define PAGE_SIZE 4096

void pmm_init(multiboot_info_t *mbd);

uint32_t pmm_alloc_frame();
void pmm_free_frame(uint32_t frame_addr);

#endif
