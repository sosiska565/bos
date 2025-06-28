#ifndef STRING_H
#define STRING_H

int strcmp(const char* s1, const char* s2);
char* itoa(int value, char *str, int base);
int strncmp(const char* s1, const char* s2, int n);
int atoi(const char* str); 

#endif