// memory.c
#include "memory.h"

#define HEAP_START 0x00200000 // 2 MB
#define HEAP_MAX   0x00A00000 // 10 MB

static uintptr_t heap_curr = HEAP_START;

void memory_init(void) {
    heap_curr = HEAP_START;
}

void* kmalloc(size_t size) {
    // Alinhamento de 4 bytes
    if (size % 4 != 0) {
        size += 4 - (size % 4);
    }

    if (heap_curr + size > HEAP_MAX) {
        return NULL; // Memória esgotada
    }

    void* ptr = (void*)heap_curr;
    heap_curr += size;
    return ptr;
}

size_t memory_get_used(void) {
    return heap_curr - HEAP_START;
}

