#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

typedef void (*task_func_t)(void);

typedef struct {
    uint32_t id;
    task_func_t func;
    uint8_t active;
} Task;

#define MAX_TASKS 8

void scheduler_init(void);
int scheduler_add_task(uint32_t id, task_func_t func);
void scheduler_yield(void);

#endif

