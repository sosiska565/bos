#include "keyboard.h"
#include "vga/vga.h"
#include "interrupts/idt.h"

#define KBD_STATUS_PORT 0x64
#define KBD_DATA_PORT   0x60
#define KBD_BUFFER_SIZE 256

static char kbd_buffer[KBD_BUFFER_SIZE];
static uint32_t kbd_read_idx = 0;
static uint32_t kbd_write_idx = 0;

// Низкоуровневая функция чтения байта из порта
static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

void keyboard_callback(struct regs *r){
    (void)r;

    uint8_t scanecode = inb(0x60);

    if(scanecode & 0x80){
        return;
    }

    if((kbd_write_idx + 1) % KBD_BUFFER_SIZE != kbd_read_idx){
        kbd_buffer[kbd_write_idx] = scanecode;
        kbd_write_idx = (kbd_write_idx + 1) % KBD_BUFFER_SIZE;
    }
}

void init_keyboard(){
    register_irq_handler(1, keyboard_callback);
}

uint8_t keyboard_read_scancode(void) {
    while(kbd_read_idx == kbd_write_idx){
        __asm__ volatile("hlt");
    }

    uint8_t scanecode = kbd_buffer[kbd_read_idx];
    kbd_read_idx = (kbd_read_idx + 1) % KBD_BUFFER_SIZE;
    return scanecode;
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

void get_string(char *buffer) {
    int pos = 0;
    uint8_t scancode;
    char c;

    while (1) {
        scancode = keyboard_read_scancode();

        if (scancode == 0x0E) {
            if (pos > 0) {
                pos--;
                printChar('\b', 0x07);
            }
            continue;
        }

        c = scancode_to_ascii(scancode);

        if (c == '\n') {
            buffer[pos] = '\0';
            printChar('\n', 0x07);
            return;
        }

        if (c) {
            buffer[pos++] = c;
            printChar(c, 0x07);
        }
    }
}