#ifndef IDT_H
#define IDT_H
#include <stdint.h>

extern void isr0(); extern void isr1(); // ... и так далее до 31
extern void isr2(); extern void isr3(); extern void isr4(); extern void isr5();
extern void isr6(); extern void isr7(); extern void isr8(); extern void isr9();
extern void isr10(); extern void isr11(); extern void isr12(); extern void isr13();
extern void isr14(); extern void isr15(); extern void isr16(); extern void isr17();
extern void isr18(); extern void isr19(); extern void isr20(); extern void isr21();
extern void isr22(); extern void isr23(); extern void isr24(); extern void isr25();
extern void isr26(); extern void isr27(); extern void isr28(); extern void isr29();
extern void isr30(); extern void isr31();

extern void irq0(); extern void irq1(); // ... и так далее до 15
extern void irq2(); extern void irq3(); extern void irq4(); extern void irq5();
extern void irq6(); extern void irq7(); extern void irq8(); extern void irq9();
extern void irq10(); extern void irq11(); extern void irq12(); extern void irq13();
extern void irq14(); extern void irq15();

struct idt_entry_t
{
    uint16_t base_low;
    uint16_t selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

struct idt_ptr_t
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};


void init_idt();
void fault_handler(struct regs *r);

extern void (*irq_routines[16])(struct regs *r);
void register_irq_handler(int irq, void (*handler)(struct regs *r));

#endif