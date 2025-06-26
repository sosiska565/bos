#include "utils.h"
#include "../string/string.h"

void panic(const char* message, const char* file, int line) {
    asm volatile("cli");

    clear(0x04);
    printString("KERNEL PANIC: ", 0x04);
    printString(message, 0x04);
    printString(" at ", 0x04);
    printString(file, 0x04);
    printString(":", 0x04);
    char buf[10];
    itoa(line, buf, 10);
    printString(buf, 0x04);
    
    for(;;);
}
