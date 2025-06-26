#ifndef VMM_H
#define VMM_H

#include <stdint.h>
#include "../interrupts/idt.h"

#define PAGE_PRESENT    0x1
#define PAGE_WRITE      0x2
#define PAGE_USER       0x4

#define PAGE_FRAME      0xFFFFF000

typedef struct {
    uint32_t entry[1024];
} page_table_t;

typedef struct {
    page_table_t* tables[1024];
    uint32_t physical_tables[1024];
    uint32_t physical_address;
} page_directory_t;

void vmm_init();
void vmm_switch_page_directory(page_directory_t* dir);
void vmm_map_page(uint32_t virt, uint32_t phys, int flags);
void vmm_page_fault(struct regs* r);
uint32_t vmm_get_physical_address(uint32_t virt);
page_directory_t* clone_directory(page_directory_t* src);

#endif
