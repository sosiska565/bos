#ifndef PIC_H
#define PIC_H

#include <stdint.h>

void PIC_remap(int offset1, int offset2);
void PIC_sendEOI(unsigned char irq);

#endif