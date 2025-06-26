#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>

uint8_t keyboard_read_scancode(void);
char scancode_to_ascii(uint8_t scanecode);
void readString(char *buffer, int max_len, unsigned int color);
void init_keyboard();

#endif