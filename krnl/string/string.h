#ifndef STRING_H
#define STRING_H

int strcmp(const char* s1, const char* s2);
char* itoa(int value, char *str, int base);
void* memset(void* ptr, int value, unsigned int num);
void* memcpy(void* dest, const void* src, unsigned int count);

#endif