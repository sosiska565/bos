#include "string.h"

static void reverse(char *str, int len){
    int i = 0, j = len - 1;
    char temp;
    while(i < j){
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++;
        j--;
    }
}

int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

void* memcpy(void* dest, const void* src, unsigned int count) {
    char* dst8 = (char*)dest;
    const char* src8 = (const char*)src;

    while (count--) {
        *dst8++ = *src8++;
    }
    return dest;
}

void* memset(void* ptr, int value, unsigned int num) {
    unsigned char* p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

char* itoa(int value, char* str, int base){
    int i = 0;
    int is_negative = 0;

    if(value == 0){
        str[i++] = '0';
        str[i] = '\0';
        return str;
    }

    if(value < 0 && base == 10){
        is_negative = 1;
        value = -value;
    }

    while (value != 0)
    {
        int rem = value % base;
        str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
        value = value / base;
    }
    
    if (is_negative){
        str[i++] = '-';
    }

    str[i] = '\0';

    reverse(str, i);

    return str;
}