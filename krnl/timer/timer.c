#include "timer.h"
#include "interrupts/idt.h"

static void timer_callback(struct regs *r){
    (void)r;
}

void init_timer(void){
    register_irq_handler(0, timer_callback);
}