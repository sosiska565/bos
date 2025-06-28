#include <stdint.h>

#include "vga/vga.h"
#include "keyboard/keyboard.h"
#include "string/string.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "memory/memory.h"
#include "drivers/pci.h"
#include "timer/timer.h"
#include "shell/shell.h"

void _OSmain(void){
    const char *wlc_msg = "Welcome to BOS!\n\n\nType 'help' to get more inforation.\n\n\n";

    clear(0x07);

    init_idt();
    PIC_remap(0x20, 0x28);
    init_keyboard();
    init_memory();
    init_timer();
    __asm__ volatile ("sti");
    //pci_scan();
    printString("\0", 0x07);
    printString(wlc_msg, 0x07);

    init_shell();
}