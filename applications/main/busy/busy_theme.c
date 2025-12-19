#include "busy_theme.h"

#include "storage_macros.h"

#define BG_FILE_NAME "bg"

struct BusyTheme {
    FuriString* name;
    FuriString* bg_path;
    BusyThemeFileType bg_type;
};

typedef struct {
    const char* extension;
    BusyThemeFileType type;
} BusyThemeFileSpec;

static const BusyThemeFileSpec busy_theme_bg_specs[] = {
    {.extension = "bin", .type = BusyThemeFileTypeImage},
    {.extension = "png", .type = BusyThemeFileTypeImage},
    {.extension = "anim", .type = BusyThemeFileTypeAnimImage},
};

// Implementation

static BusyThemeFileType busy_theme_find_file_name(
    const char* root_path,
    const char* file_name,
    const BusyThemeFileSpec* const specs,
    uint32_t specs_count,
    FuriString* out) {
    BusyThemeFileType ret = BusyThemeFileTypeMax;

    FuriString* tmp = furi_string_alloc();
    Storage* storage = furi_record_open(RECORD_STORAGE);

    for(uint32_t i = 0; i < specs_count; ++i) {
        const BusyThemeFileSpec* spec = &specs[i];

        furi_string_printf(tmp, "%s/%s.%s", root_path, file_name, spec->extension);

        if(storage_file_exists(storage, furi_string_get_cstr(tmp))) {
            furi_string_set(out, tmp);
            ret = spec->type;
            break;
        }
    }

    furi_record_close(RECORD_STORAGE);
    furi_string_free(tmp);

    return ret;
}

bool busy_theme_read_bg_path(BusyTheme* instance, const char* root_path) {
    instance->bg_type = busy_theme_find_file_name(
        root_path,
        BG_FILE_NAME,
        busy_theme_bg_specs,
        COUNT_OF(busy_theme_bg_specs),
        instance->bg_path);

    return instance->bg_type != BusyThemeFileTypeMax;
}

// Public API

BusyTheme* busy_theme_alloc(void) {
    BusyTheme* instance = malloc(sizeof(BusyTheme));

    instance->name = furi_string_alloc();
    instance->bg_path = furi_string_alloc();

    return instance;
}

BusyTheme* busy_theme_alloc_default(void) {
    BusyTheme* instance = busy_theme_alloc();

    furi_string_set(instance->name, "default");
    furi_string_set(instance->bg_path, BUSY_IMG_PATH("theme_preview_72x16.bin"));

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
    furi_string_free(instance->bg_path);

    free(instance);
}

void busy_theme_reset(BusyTheme* instance) {
    furi_assert(instance);

    furi_string_reset(instance->name);
    furi_string_reset(instance->bg_path);
}

void busy_theme_set(BusyTheme* instance, const BusyTheme* other) {
    furi_assert(instance);
    furi_assert(other);

    furi_string_set(instance->name, other->name);
    furi_string_set(instance->bg_path, other->bg_path);
    instance->bg_type = other->bg_type;
}

const char* busy_theme_get_name(const BusyTheme* instance) {
    furi_assert(instance);
    return furi_string_get_cstr(instance->name);
}

void busy_theme_get_info(const BusyTheme* instance, BusyThemeInfo* info) {
    furi_assert(instance);
    furi_assert(info);

    info->name = furi_string_get_cstr(instance->name);
    info->bg_path = furi_string_get_cstr(instance->bg_path);
    info->bg_type = instance->bg_type;
}

bool busy_theme_read(BusyTheme* instance, const char* name) {
    furi_assert(instance);
    furi_assert(name);

    busy_theme_reset(instance);

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* theme_dir_path = furi_string_alloc_printf("%s/%s", BUSY_THEMES_DIR, name);

    do {
        const char* root_path = furi_string_get_cstr(theme_dir_path);

        if(!storage_dir_exists(storage, root_path)) {
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

bool busy_theme_is_default(const BusyTheme* instance) {
    furi_assert(instance);

    return furi_string_empty(instance->name);
}

bool busy_theme_is_equal(const BusyTheme* instance, const BusyTheme* other) {
    return furi_string_equal(instance->name, other->name);
}
