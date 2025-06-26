#ifndef TASK_H
#define TASK_H

#include "../memory/vmm.h"

typedef struct task
{
    int id;                
    uint32_t esp, ebp;     
    uint32_t eip;          
    page_directory_t *page_directory;
    struct task *next;
    struct task *prev;
} task_t;

void init_tasking();
void switch_task();

int fork();

void move_stack(void *new_stack_start, uint32_t size);
int getpid();
void exit();

#endif
