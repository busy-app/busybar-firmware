/**
 * @file memory_pool.h
 * @brief Fixed object size memory allocator.
 */
#pragma once
#include <stddef.h>

typedef struct MemoryPool MemoryPool;

/**
 * @brief Create a new memory pool.
 *
 * @param object_size size of an allocation unit in bytes
 * @param capacity maximum number of allocation units the memory pool can hold
 * @return new memory pool
 */
MemoryPool* memory_pool_alloc(size_t object_size, size_t capacity);

/**
 * @brief Destroy the memory pool
 *
 * @param instance memory pool
 */
void memory_pool_free(MemoryPool* instance);

/**
 * @brief Allocate an object in a memory pool.
 *
 * @param instance memory pool
 * @return pointer to the newly allocated memory or NULL if no space is left in the pool
 */
void* memory_pool_alloc_object(MemoryPool* instance);

/**
 * @brief Free an object previously allocated with memory_pool_alloc_object.
 *
 * @param instance memory pool used to allocated the object
 * @param object object to be freed
 */
void memory_pool_free_object(MemoryPool* instance, void* object);
