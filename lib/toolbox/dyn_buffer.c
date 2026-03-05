#include "dyn_buffer.h"
#include <stdlib.h>
#include <string.h>

#define MIN_ALLOC 4

DynBuffer dyn_buffer_init(void) {
    return (DynBuffer){
        .data = malloc(MIN_ALLOC),
        .capacity = MIN_ALLOC,
        .size = 0,
    };
}

void dyn_buffer_destroy(DynBuffer* instance) {
    free(instance->data);
}

void dyn_buffer_reserve(DynBuffer* instance, size_t new_size) {
    if(instance->capacity >= new_size) {
        return;
    }
    size_t new_capacity = instance->capacity;
    while(new_capacity < new_size) {
        new_capacity *= 2;
    }
    instance->data = realloc(instance->data, new_capacity);
    instance->capacity = new_capacity;
}

void dyn_buffer_push(DynBuffer* instance, const void* data, size_t size) {
    size_t new_size = instance->size + size;
    dyn_buffer_reserve(instance, new_size);
    memcpy(instance->data + instance->size, data, size);
    instance->size += size;
}
