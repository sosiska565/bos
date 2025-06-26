#include "keyboard.h"
#include "vga/vga.h"

#define KBD_STATUS_PORT 0x64
#define KBD_DATA_PORT   0x60

// Низкоуровневая функция чтения байта из порта
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

uint8_t keyboard_read_scancode(void) {
    while ((inb(KBD_STATUS_PORT) & 0x01) == 0);
    return inb(KBD_DATA_PORT);
}

char scancode_to_ascii(uint8_t scancode) {
    static const char scancode_table[] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0
    };

    if (scancode >= sizeof(scancode_table)) return 0;
    return scancode_table[scancode];
}

void readString(char *buffer, int max_len, unsigned int color) {
    int pos = 0;
    uint8_t scancode;
    char c;

    while (1) {
        scancode = keyboard_read_scancode();
        
        if (scancode & 0x80) continue;

        if (scancode == 0x0E) {
            if (pos > 0) {
                pos--;
                printChar('\b', color);
            }
            continue;
        }

        c = scancode_to_ascii(scancode);

        if (c == '\n') {
            buffer[pos] = '\0';
            printChar('\n', color);
            return;
        }

        if (c && pos < max_len - 1) {
            buffer[pos++] = c;
            printChar(c, color);
        }
    }
}