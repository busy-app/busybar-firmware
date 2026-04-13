#pragma once

#include "font_registry.h"

#include <storage/storage.h>

#include <stb/stb.h>

typedef struct {
    const char* key;
    struct {
        lv_font_t* loaded_data;
        size_t references;
        size_t last_access;
        size_t estimated_memory_size;
    } value;
} FontRegistryLoadedFont;

struct FontRegistry {
    Storage* storage;

    FuriMutex* mutex;
    FontRegistryLoadedFont* loaded_fonts;
    size_t access_counter;
};
