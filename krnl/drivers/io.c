#include "io.h"

// Записать 1 байт в порт
void outb(uint16_t port, uint8_t data) {
    __asm__ volatile ("outb %0, %1" : : "a"(data), "Nd"(port));
}

// Прочитать 1 байт из порта
uint8_t inb(uint16_t port) {
    uint8_t data;
    // ИНСТРУКЦИЯ inb, ПАРАМЕТР data - ВЫХОДНОЙ (=a), есть return
    __asm__ volatile ("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

// Записать 4 байта в порт
void outl(uint16_t port, uint32_t data) {
    // ИНСТРУКЦИЯ outl
    __asm__ volatile ("outl %0, %1" : : "a"(data), "Nd"(port));
}

// Прочитать 4 байта из порта
uint32_t inl(uint16_t port) {
    uint32_t data;
    // ИНСТРУКЦИЯ inl, ПАРАМЕТР data - ВЫХОДНОЙ (=a), есть return
    __asm__ volatile ("inl %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}