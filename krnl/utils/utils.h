#ifndef UTILS_H
#define UTILS_H

#include "../vga/vga.h"

void panic(const char* message, const char* file, int line);

#define PANIC(msg) panic(msg, __FILE__, __LINE__);

#endif
