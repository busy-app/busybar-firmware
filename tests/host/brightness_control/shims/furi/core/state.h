#pragma once
#include <stddef.h>
typedef struct FuriState FuriState;
typedef void (*FuriStateCallback)(const void*, void*);
FuriState* furi_state_alloc(size_t size);
void* furi_state_acquire(FuriState* state);
void furi_state_release(FuriState* state);
void furi_state_get(FuriState* state, void* out);
void* furi_state_get_subscribe(FuriState*, void*, FuriStateCallback, void*);
#define with_furi_state(state, decl, code) \
    {                                      \
        decl = furi_state_acquire(state);  \
        {code};                            \
        furi_state_release(state);         \
    }
