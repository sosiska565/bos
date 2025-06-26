#include <stdint.h>

#include "vga/vga.h"
#include "keyboard/keyboard.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "timer/timer.h"
#include "shell/shell.h"
#include "drivers/pci.h"

#include "memory/pmm.h"
#include "memory/vmm.h"
#include "memory/heap.h"

#include "fs/vfs.h"
#include "fs/initrd.h"
#include "multiboot.h"

#include "process/task.h"
#include "gdt/gdt.h"

void _OSmain(multiboot_info_t* mbd, unsigned int magic)
{
    vga_clear();
    vga_print("Welcome to BOS!\n");

    init_gdt();
    init_idt();
    PIC_remap(0x20, 0x28);
    
    if (magic != 0x2BADB002) {
        vga_print("Error: Invalid multiboot magic number!\n");
        return;
    }

    pmm_init(mbd);
    vmm_init();
    init_heap();

    uint32_t initrd_location = *((uint32_t*)mbd->mods_addr);
    fs_root = initialise_initrd(initrd_location);

    init_tasking();
    init_timer();
    init_keyboard();
    
    pci_scan();

    __asm__ volatile ("sti");

    init_shell();
    
    for(;;);
}