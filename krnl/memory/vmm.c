#include "vmm.h"
#include "../string/string.h"
#include "pmm.h"
#include "memory.h"
#include "../string/string.h"
#include "../vga/vga.h"

extern uint32_t placement_address;
page_directory_t* kernel_directory = 0;
page_directory_t* current_directory = 0;

void vmm_init() {
    uint32_t phys;
    kernel_directory = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    memset(kernel_directory, 0, sizeof(page_directory_t));
    kernel_directory->physical_address = phys;

    uint32_t i = 0;
    while (i < placement_address + 0x1000) {
        vmm_map_page(i, i, PAGE_PRESENT | PAGE_WRITE);
        i += PAGE_SIZE;
    }

    register_irq_handler(14, vmm_page_fault);
    vmm_switch_page_directory(kernel_directory);
}

void vmm_switch_page_directory(page_directory_t* dir) {
    current_directory = dir;
    asm volatile("mov %0, %%cr3" :: "r"(dir->physical_address));
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void vmm_map_page(uint32_t virt, uint32_t phys, int flags) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = virt >> 12 & 0x03FF;

    if (!kernel_directory->tables[pd_index]) {
        uint32_t pt_phys;
        page_table_t* pt = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &pt_phys);
        memset(pt, 0, sizeof(page_table_t));
        kernel_directory->tables[pd_index] = pt;
        kernel_directory->physical_tables[pd_index] = pt_phys | PAGE_PRESENT | PAGE_WRITE;
    }
    
    kernel_directory->tables[pd_index]->entry[pt_index] = phys | flags;
}

uint32_t vmm_get_physical_address(uint32_t virt) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = virt >> 12 & 0x03FF;

    if (!current_directory->tables[pd_index]) {
        return 0;
    }

    page_table_t* table = current_directory->tables[pd_index];
    if (!table->entry[pt_index]) {
        return 0;
    }
    
    return (table->entry[pt_index] & PAGE_FRAME) + (virt & 0xFFF);
}

static page_table_t* clone_table(page_table_t *src, uint32_t *phys, int dir_idx);

page_directory_t* clone_directory(page_directory_t* src)
{
    uint32_t phys;
    page_directory_t* dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    memset(dir, 0, sizeof(page_directory_t));
    dir->physical_address = phys;

    for (int i = 0; i < 1024; i++)
    {
        if (!src->tables[i])
            continue;

        if (kernel_directory->tables[i] == src->tables[i])
        {
            dir->tables[i] = src->tables[i];
            dir->physical_tables[i] = src->physical_tables[i];
        }
        else
        {
            uint32_t phys_table_addr;
            dir->tables[i] = clone_table(src->tables[i], &phys_table_addr, i);
            dir->physical_tables[i] = phys_table_addr | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        }
    }
    return dir;
}

#define CLONE_TEMP_VIRT_ADDR 0xFFBFF000

static page_table_t* clone_table(page_table_t *src, uint32_t *phys, int dir_idx)
{
    page_table_t* table = (page_table_t*)kmalloc_ap(sizeof(page_table_t), phys);
    memset(table, 0, sizeof(page_table_t));

    for (int i = 0; i < 1024; i++)
    {
        if (src->entry[i])
        {
            uint32_t new_frame = pmm_alloc_frame();
            uint32_t flags = src->entry[i] & 0xFFF;
            table->entry[i] = new_frame | flags;

            vmm_map_page(CLONE_TEMP_VIRT_ADDR, new_frame, PAGE_PRESENT | PAGE_WRITE);

            uint32_t src_virt_addr = (dir_idx << 22) | (i << 12);
            memcpy((void*)CLONE_TEMP_VIRT_ADDR, (void*)src_virt_addr, 4096);

            uint32_t pd_index = CLONE_TEMP_VIRT_ADDR >> 22;
            uint32_t pt_index = (CLONE_TEMP_VIRT_ADDR >> 12) & 0x3FF;
            current_directory->tables[pd_index]->entry[pt_index] = 0;
            asm volatile("invlpg (%0)" :: "r"(CLONE_TEMP_VIRT_ADDR));
        }
    }
    return table;
}

void vmm_page_fault(struct regs* r) {
    uint32_t faulting_address;
    asm volatile("mov %%cr2, %0" : "=r"(faulting_address));

    int present = !(r->err_code & 0x1);
    int rw = r->err_code & 0x2;
    int us = r->err_code & 0x4;
    int reserved = r->err_code & 0x8;

    vga_print("Page Fault! (");
    if (present) vga_print("present ");
    if (rw) vga_print("read-only ");
    if (us) vga_print("user-mode ");
    if (reserved) vga_print("reserved ");
    vga_print(") at ");
    printHex(faulting_address);
    vga_print("\n");
    
    for(;;);
}
