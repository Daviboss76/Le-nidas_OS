// memory.h
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>

void memory_init(void);
void* kmalloc(size_t size);
size_t memory_get_used(void);

#endif

