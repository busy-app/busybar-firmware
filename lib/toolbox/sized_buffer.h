/**
 * @file sized_buffer.h
 *
 * @brief A common buffer + size structure
 */
#pragma once

#include <stddef.h>

typedef struct SizedBuffer {
    void* buffer;
    size_t size;
} SizedBuffer;
