#include "scheduler.h"

static Task task_list[MAX_TASKS];
static uint8_t current_task = 0;

void scheduler_init(void) {
    for (int i = 0; i < MAX_TASKS; i++) {
        task_list[i].id = 0;
        task_list[i].func = 0;
        task_list[i].active = 0;
    }
}

int scheduler_add_task(uint32_t id, task_func_t func) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_list[i].active) {
            task_list[i].id = id;
            task_list[i].func = func;
            task_list[i].active = 1;
            return 0;
        }
    }
    return -1;
}

void scheduler_yield(void) {
    current_task = (current_task + 1) % MAX_TASKS;
    if (task_list[current_task].active && task_list[current_task].func) {
        task_list[current_task].func();
    }
}

