#include "shared_ptr.h"
#include <stdlib.h>

SharedPtr* shared_ptr_alloc(void* inner, SharedPtrInnerDestructor destructor) {
    SharedPtr* instance = malloc(sizeof(SharedPtr));
    instance->inner = inner;
    instance->destructor = destructor;
    instance->owners = 1;
    return instance;
}

SharedPtr* shared_ptr_alloc_plain(void* inner) {
    return shared_ptr_alloc(inner, free);
}

void shared_ptr_acquire(SharedPtr* instance) {
    atomic_fetch_add(&instance->owners, 1);
}

void shared_ptr_release(SharedPtr* instance) {
    size_t was = atomic_fetch_sub(&instance->owners, 1);
    if(was == 1) {
        instance->destructor(instance->inner);
        free(instance);
    }
}
