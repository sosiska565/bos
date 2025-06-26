#include "task.h"
#include "../memory/memory.h"
#include "../string/string.h"
#include "../utils/utils.h"

extern page_directory_t *kernel_directory;
extern page_directory_t *current_directory;
extern void perform_task_switch(uint32_t, uint32_t, uint32_t, uint32_t);

volatile task_t *current_task;
volatile task_t *ready_queue;

extern uint32_t read_eip();

uint32_t next_pid = 1;

void init_tasking()
{
    asm volatile("cli");

    move_stack((void*)0xE0000000, 0x2000);

    current_task = (task_t*)kmalloc(sizeof(task_t));
    current_task->id = next_pid++;
    current_task->esp = 0;
    current_task->ebp = 0;
    current_task->eip = 0;
    current_task->page_directory = current_directory;
    current_task->next = current_task;
    current_task->prev = current_task;
    ready_queue = current_task;

    asm volatile("sti");
}

void switch_task()
{
    if (!current_task)
        return;

    uint32_t esp, ebp, eip;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    asm volatile("mov %%ebp, %0" : "=r"(ebp));

    eip = read_eip();

    if (eip == 0x12345)
        return;

    current_task->eip = eip;
    current_task->esp = esp;
    current_task->ebp = ebp;

    current_task = current_task->next;

    eip = current_task->eip;
    esp = current_task->esp;
    ebp = current_task->ebp;

    current_directory = current_task->page_directory;

    perform_task_switch(eip, current_directory->physical_address, ebp, esp);
}

int fork()
{
    asm volatile("cli");

    task_t *parent_task = (task_t*)current_task;

    page_directory_t *directory = clone_directory(current_directory);

    task_t *new_task = (task_t*)kmalloc(sizeof(task_t));

    new_task->id = next_pid++;
    new_task->esp = 0;
    new_task->ebp = 0;
    new_task->eip = 0;
    new_task->page_directory = directory;
    new_task->prev = parent_task;
    new_task->next = parent_task->next;
    parent_task->next->prev = new_task;
    parent_task->next = new_task;

    uint32_t eip = read_eip();

    if (current_task == parent_task)
    {
        uint32_t esp; asm volatile("mov %%esp, %0" : "=r"(esp));
        uint32_t ebp; asm volatile("mov %%ebp, %0" : "=r"(ebp));
        new_task->esp = esp;
        new_task->ebp = ebp;
        new_task->eip = eip;
        asm volatile("sti");

        return new_task->id;
    }
    else
    {
        return 0;
    }
}

void move_stack(void *new_stack_start, uint32_t size)
{
  uint32_t i;
  for(i = (uint32_t)new_stack_start; i >= ((uint32_t)new_stack_start-size); i -= 0x1000)
  {
    vmm_map_page(i, pmm_alloc_frame(), PAGE_PRESENT|PAGE_WRITE);
  }
  
  uint32_t pd_addr;
  asm volatile("mov %%cr3, %0" : "=r" (pd_addr));
  asm volatile("mov %0, %%cr3" : : "r" (pd_addr));

  uint32_t old_stack_pointer; asm volatile("mov %%esp, %0" : "=r" (old_stack_pointer));
  uint32_t old_base_pointer;  asm volatile("mov %%ebp, %0" : "=r" (old_base_pointer));

  uint32_t offset            = (uint32_t)new_stack_start - old_stack_pointer;

  uint32_t new_stack_pointer = old_stack_pointer + offset;
  uint32_t new_base_pointer  = old_base_pointer  + offset;

  memcpy((void*)new_stack_pointer, (void*)old_stack_pointer, old_stack_pointer - old_base_pointer);

  for (i = (uint32_t)new_stack_start; i > (uint32_t)new_stack_start-size; i -= 4)
  {
    uint32_t tmp = * (uint32_t*)i;
    if ( (old_stack_pointer < tmp) && (tmp < old_stack_pointer + 0x1000) )
    {
      tmp += offset;
      uint32_t *tmp2 = (uint32_t*)i;
      *tmp2 = tmp;
    }
  }

  asm volatile("mov %0, %%esp" : : "r" (new_stack_pointer));
  asm volatile("mov %0, %%ebp" : : "r" (new_base_pointer));
}

int getpid()
{
    return current_task->id;
}

void exit()
{
    asm volatile("cli");

    task_t *task_to_remove = (task_t*)current_task;

    task_to_remove->prev->next = task_to_remove->next;
    task_to_remove->next->prev = task_to_remove->prev;

    kfree(task_to_remove);

    switch_task();
}
