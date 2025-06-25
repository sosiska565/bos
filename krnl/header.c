#include <stdint.h>

// --- Глобальные переменные и константы ---

char *vidmem = (char *)0xb8000;
int cursor_pos = 0;

const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

#define KBD_STATUS_PORT 0x64
#define KBD_DATA_PORT   0x60

// --- Функции управления экраном ---

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

void update_cursor() {
    uint16_t pos = cursor_pos / 2;

    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void scroll_screen() {
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH * 2; i++) {
        vidmem[i] = vidmem[i + VGA_WIDTH * 2];
    }

    int last_line_pos = (VGA_HEIGHT - 1) * VGA_WIDTH * 2;
    for (int i = 0; i < VGA_WIDTH * 2; i += 2) {
        vidmem[last_line_pos + i] = ' ';
        vidmem[last_line_pos + i + 1] = vidmem[last_line_pos - 1]; 
    }

    cursor_pos = (VGA_HEIGHT - 1) * VGA_WIDTH * 2;
    update_cursor();
}

void printChar(char c, unsigned int color) {
    if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT * 2) {
        scroll_screen();
    }

    switch (c) {
        case '\n':
            cursor_pos = (cursor_pos / (VGA_WIDTH * 2) + 1) * VGA_WIDTH * 2;
            break;
        case '\b':
            if (cursor_pos > 0) {
                cursor_pos -= 2;
                vidmem[cursor_pos] = ' ';
                vidmem[cursor_pos + 1] = color;
            }
            break;
        default:
            vidmem[cursor_pos] = c;
            vidmem[cursor_pos + 1] = color;
            cursor_pos += 2;
            break;
    }

    if (cursor_pos >= VGA_WIDTH * VGA_HEIGHT * 2) {
        scroll_screen();
    }

    update_cursor();
}

void clear(char *vidmem, unsigned int color){
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2){
        vidmem[i] = ' ';
        vidmem[i + 1] = color;
    }
    cursor_pos = 0;
    update_cursor();
}

void printString(const char *str, unsigned int color){
    int i = 0;
    while(str[i] != '\0'){
        printChar(str[i], color);
        i++;
    }
    update_cursor();
}

void fill_background(int bg_color, char *vidmem) {
    for (int i = 1; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        unsigned int current_attribute = vidmem[i];
        unsigned int text_color = current_attribute & 0x0F;
        unsigned int new_attribute = (bg_color << 4) | text_color;
        vidmem[i] = new_attribute;
    }
}


// --- Функции ввода с клавиатуры ---

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

// ---Работа с массивами---

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// --- Основная функция ядра ---

void OSmain(void){
    const char *wlc_msg = "Welcome to BOS!\n\n\nType 'help' to get more inforation.\n\n\n";
    char input_buffer[100];

    clear(vidmem, 0x07);

    printString(wlc_msg, 0x07);
    while(1) {
        printString("> ", 0x07);
        readString(input_buffer, sizeof(input_buffer), 0x07);

        if(strcmp(input_buffer, "help") == 0){
            printString("--Available commands--\n", 0x0A);
            printString("help - displays all available commands on the screen.\n", 0x07);
            printString("clear - clear screen.\n", 0x07);
        }
        else if(strcmp(input_buffer, "clear") == 0){
            clear(vidmem, 0x07);
        }   
        else{
            printString("ERR: Unknown command: ", 0x04);
            printString(input_buffer, 0x04);
        }
        
        printString("\n", 0x07);
    }
    
    while(1);
}