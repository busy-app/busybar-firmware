#include "busy_theme.h"

#include "storage_macros.h"

#define PREVIEW_FILE_NAME "preview"
#define BG_FILE_NAME      "bg"

struct BusyTheme {
    FuriString* name;
    FuriString* preview_path;
    FuriString* bg_path;
};

static const char* const busy_theme_preview_exts[] = {
    "png",
    "bin",
};

static const char* const busy_theme_bg_exts[] = {
    "png",
    "anim",
    "json",
};

// Implementation

static bool busy_theme_find_file_name(
    const char* root_path,
    const char* file_name,
    const char* const* exts,
    uint32_t exts_count,
    FuriString* out) {
    bool success = false;

    FuriString* tmp = furi_string_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    for(uint32_t i = 0; i < exts_count; ++i) {
        furi_string_printf(tmp, "%s/%s.%s", root_path, file_name, exts[i]);

        if(storage_file_exists(storage, furi_string_get_cstr(tmp))) {
            furi_string_set(out, tmp);
            success = true;
            break;
        }
    }

    furi_record_close(RECORD_STORAGE);
    furi_string_free(tmp);

    return success;
}

bool busy_theme_read_preview_path(BusyTheme* instance, const char* root_path) {
    return busy_theme_find_file_name(
        root_path,
        PREVIEW_FILE_NAME,
        busy_theme_preview_exts,
        COUNT_OF(busy_theme_preview_exts),
        instance->preview_path);
}

bool busy_theme_read_bg_path(BusyTheme* instance, const char* root_path) {
    return busy_theme_find_file_name(
        root_path,
        BG_FILE_NAME,
        busy_theme_bg_exts,
        COUNT_OF(busy_theme_bg_exts),
        instance->bg_path);
}

// Public API

BusyTheme* busy_theme_alloc(void) {
    BusyTheme* instance = malloc(sizeof(BusyTheme));

    instance->name = furi_string_alloc();
    instance->preview_path = furi_string_alloc();
    instance->bg_path = furi_string_alloc();

    return instance;
}

BusyTheme* busy_theme_alloc_set(const BusyTheme* other) {
    furi_assert(other);

    BusyTheme* instance = busy_theme_alloc();
    busy_theme_set(instance, other);

    return instance;
}

void busy_theme_free(BusyTheme* instance) {
    furi_assert(instance);

    furi_string_free(instance->name);
    furi_string_free(instance->preview_path);
    furi_string_free(instance->bg_path);

    free(instance);
}

void busy_theme_reset(BusyTheme* instance) {
    furi_assert(instance);

    furi_string_reset(instance->name);
    furi_string_reset(instance->preview_path);
    furi_string_reset(instance->bg_path);
}

void busy_theme_set(BusyTheme* instance, const BusyTheme* other) {
    furi_assert(instance);
    furi_assert(other);

    furi_string_set(instance->name, other->name);
    furi_string_set(instance->preview_path, other->preview_path);
    furi_string_set(instance->bg_path, other->bg_path);
}

const char* busy_theme_get_name(const BusyTheme* instance) {
    furi_assert(instance);
    return furi_string_get_cstr(instance->name);
}

const char* busy_theme_get_preview_path(const BusyTheme* instance) {
    furi_assert(instance);
    return furi_string_get_cstr(instance->preview_path);
}

const char* busy_theme_get_bg_path(const BusyTheme* instance) {
    furi_assert(instance);
    return furi_string_get_cstr(instance->bg_path);
}

bool busy_theme_read(BusyTheme* instance, const char* name) {
    furi_assert(instance);
    furi_assert(name);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* theme_dir_path = furi_string_alloc_printf("%s/%s", BUSY_THEMES_DIR, name);

    do {
        const char* root_path = furi_string_get_cstr(theme_dir_path);

        if(!storage_dir_exists(storage, root_path)) {
            break;
        }

        if(!busy_theme_read_preview_path(instance, root_path)) {
            break;
        }

        if(!busy_theme_read_bg_path(instance, root_path)) {
            break;
        }

        furi_string_set(instance->name, name);

        success = true;

    } while(false);

    furi_string_free(theme_dir_path);
    furi_record_close(RECORD_STORAGE);

    return success;
}
