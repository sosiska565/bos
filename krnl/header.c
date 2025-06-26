#include <stdint.h>

#include "vga/vga.h"
#include "keyboard/keyboard.h"
#include "string/string.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "memory/memory.h"
#include "drivers/pci.h"
#include "timer/timer.h"

void _OSmain(void){
    const char *wlc_msg = "Welcome to BOS!\n\n\nType 'help' to get more inforation.\n\n\n";
    char input_buffer[100];

    clear(0x07);
    printString(wlc_msg, 0x07);

    init_idt();
    PIC_remap(0x20, 0x28);
    init_keyboard();
    init_memory();
    init_timer();


    __asm__ volatile ("sti");

    pci_scan();

    while(1) {
        printString("> ", 0x07);
        readString(input_buffer, sizeof(input_buffer), 0x07);

        if(strcmp(input_buffer, "help") == 0){
            printString("--Available commands--\n", 0x0A);
            printString("help - displays all available commands on the screen.\n", 0x07);
            printString("clear - clear screen.\n", 0x07);
        }
        else if(strcmp(input_buffer, "clear") == 0){
            clear(0x07);
        }   
        else{
            printString("ERR: Unknown command: ", 0x04);
            printString(input_buffer, 0x04);
        }
        
        printString("\n", 0x07);
    }
    
    while(1);
}