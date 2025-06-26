#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <stdint.h>

uint8_t keyboard_read_scancode(void);
char scancode_to_ascii(uint8_t scanecode);
void get_string(char *buffer);
void init_keyboard();

#endif