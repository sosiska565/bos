#include "vga.h"
#include "string/string.h"
#include <stdint.h>
#include "drivers/io.h"

char *vidmem = (char *)0xb8000;
int cursor_pos = 0;

const int VGA_WIDTH = 80;
const int VGA_HEIGHT = 25;

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

void vga_clear(){
    for(int i = 0; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2){
        vidmem[i] = ' ';
        vidmem[i + 1] = 0x07;
    }
    cursor_pos = 0;
    update_cursor();
}

void vga_print(const char *str){
    int i = 0;
    while(str[i] != '\0'){
        printChar(str[i], 0x07);
        i++;
    }
    update_cursor();
}

void printDec(int n){
    char buffer[50];
    itoa(n, buffer, 10);
    vga_print(buffer);
}

void printHex(int n){
    char buffer[50];
    vga_print("0x");
    itoa(n, buffer, 16);
    vga_print(buffer);
}

void fill_background(int bg_color) {
    for (int i = 1; i < VGA_WIDTH * VGA_HEIGHT * 2; i += 2) {
        unsigned int current_attribute = vidmem[i];
        unsigned int text_color = current_attribute & 0x0F;
        unsigned int new_attribute = (bg_color << 4) | text_color;
        vidmem[i] = new_attribute;
    }
}