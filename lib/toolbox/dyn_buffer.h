#pragma once
#include <stdint.h>
#include <stddef.h>

typedef struct DynBuffer {
    uint8_t* data;
    size_t size;
    size_t capacity;
} DynBuffer;

/**
 * Initialize an empty dynamic buffer.
 */
DynBuffer dyn_buffer_init(void);

/**
 * Free the buffer contents.
 */
void dyn_buffer_destroy(DynBuffer* instance);

/**
 * Push data to the end of the buffer.
 */
void dyn_buffer_push(DynBuffer* instance, const void* data, size_t size);

/**
 * Adjust buffer capacity so that it can hold at least new_size bytes
 */
void dyn_buffer_reserve(DynBuffer* instance, size_t new_size);
