#pragma once

#if defined(__STDC_NO_ATOMICS__)
#error No atomics!
#endif

#include <stdatomic.h>

typedef void (*SharedPtrInnerDestructor)(void* inner);

typedef struct SharedPtr {
    void* inner;
    atomic_size_t owners;
    SharedPtrInnerDestructor destructor;
} SharedPtr;

/**
 * Allocate a new shared pointer. The pointer is already acquired once.
 *
 * @param inner payload.
 * @param destructor a function to call on inner when all owner have released the shared pointer.
 */
SharedPtr* shared_ptr_alloc(void* inner, SharedPtrInnerDestructor destructor);

/**
 * Allocate a new shared pointer. The pointer is already acquired once.
 *
 * @param inner payload (allocated with malloc).
 */
SharedPtr* shared_ptr_alloc_plain(void* inner);

/**
 * Increment owners count.
 */
void shared_ptr_acquire(SharedPtr* instance);

/**
 * Decrement owners count and free the pointer when no more owners.
 */
void shared_ptr_release(SharedPtr* instance);
