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

int strncmp(const char* s1, const char* s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// Преобразует строку в целое число (простая реализация)
int atoi(const char* str) {
    int res = 0;
    int sign = 1;
    int i = 0;

    // Пропускаем пробелы
    while (str[i] == ' ') {
        i++;
    }

    // Обрабатываем знак
    if (str[i] == '-') {
        sign = -1;
        i++;
    } else if (str[i] == '+') {
        i++;
    }

    // Преобразуем цифры
    while (str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }

    return res * sign;
}
