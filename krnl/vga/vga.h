#ifndef VGA_H
#define VGA_H

extern char *vidmem;
extern int cursor_pos;

void update_cursor();
void scroll_screen();
void printChar(char c, unsigned int color);
void vga_clear();
void vga_print(const char *str);
void fill_background(int bg_color);
void printDec(int n);
void printHex(int n);

#endif