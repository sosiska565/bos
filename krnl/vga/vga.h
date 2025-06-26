#ifndef VGA_H
#define VGA_H

extern char *vidmem;
extern int cursor_pos;

void update_cursor();
void scroll_screen();
void printChar(char c, unsigned int color);
void clear(unsigned int color);
void printString(const char *str, unsigned int color);
void fill_background(int bg_color);

#endif