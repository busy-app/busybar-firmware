#include "memory_pool.h"
#include <stdint.h>
#include <stdlib.h>
#include <furi/core/check.h>

#define MEMORY_POOL_ALIGNMENT 4

typedef struct MemoryPoolBlockHeader MemoryPoolBlockHeader;

typedef struct MemoryPool {
    size_t block_size;
    size_t capacity;

    uint8_t* pool;
    MemoryPoolBlockHeader* first_free_block;
} MemoryPool;

typedef struct MemoryPoolBlockHeader {
    MemoryPoolBlockHeader* next;
} MemoryPoolBlockHeader;
static_assert(sizeof(MemoryPoolBlockHeader) % MEMORY_POOL_ALIGNMENT == 0);

static size_t memory_pool_align(size_t x) {
    return ((x + MEMORY_POOL_ALIGNMENT - 1) / MEMORY_POOL_ALIGNMENT) * MEMORY_POOL_ALIGNMENT;
}

static void memory_pool_check_object(MemoryPool* instance, void* object) {
    MemoryPoolBlockHeader* header = (MemoryPoolBlockHeader*)((uint8_t*)object - sizeof(MemoryPoolBlockHeader));

    furi_check((uint8_t*)header >= instance->pool && (uint8_t*)header < instance->pool + instance->block_size * instance->capacity);
    furi_check(((uint8_t*)header - instance->pool) % instance->block_size == 0);
}

MemoryPool* memory_pool_alloc(size_t object_size, size_t capacity) {
    furi_check(capacity > 0);
    MemoryPool* instance = malloc(sizeof(MemoryPool));

    instance->capacity = capacity;
    instance->block_size = memory_pool_align(sizeof(MemoryPoolBlockHeader) + object_size);

    instance->pool = malloc(capacity * instance->block_size);

    MemoryPoolBlockHeader* last_block = NULL;
    for(size_t i = 0; i != capacity; ++i) {
        MemoryPoolBlockHeader* current_block = (MemoryPoolBlockHeader*)(instance->pool + i * instance->block_size);
        if(last_block) {
            last_block->next = current_block;
        }
        last_block = current_block;
    }
    last_block->next = NULL;

    instance->first_free_block = (MemoryPoolBlockHeader*)instance->pool;

    return instance;
}

void memory_pool_free(MemoryPool* instance) {
    free(instance->pool);
    free(instance);
}

void* memory_pool_alloc_object(MemoryPool* instance) {
    MemoryPoolBlockHeader* head = instance->first_free_block;

    if(head == NULL) {
        return NULL;
    }

    instance->first_free_block = head->next;
    return (uint8_t*)head + sizeof(MemoryPoolBlockHeader);
}

void memory_pool_free_object(MemoryPool* instance, void* object) {
    MemoryPoolBlockHeader* header = (MemoryPoolBlockHeader*)((uint8_t*)object - sizeof(MemoryPoolBlockHeader));
    memory_pool_check_object(instance, object);
    header->next = instance->first_free_block;
    instance->first_free_block = header;
}
