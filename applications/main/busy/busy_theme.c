#include "busy_theme.h"

#include "storage_macros.h"

#include <json_helper.h>

#define CONFIG_FILE_NAME "theme.json"
#define CONFIG_KEY_ASSET "bg_path"

#define DEFAULT_NAME "busy"

#define TAG "BusyTheme"

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
    {.extension = "anim", .type = BusyThemeFileTypeAnim},
};

// Implementation

static BusyThemeFileType busy_theme_detect_file_type(const char* path) {
    for(uint32_t i = 0; i < COUNT_OF(busy_theme_bg_specs); ++i) {
        const char* ext = busy_theme_bg_specs[i].extension;
        size_t ext_len = strlen(ext);
        size_t path_len = strlen(path);

        if(path_len > ext_len + 1 && path[path_len - ext_len - 1] == '.' &&
           strcmp(&path[path_len - ext_len], ext) == 0) {
            return busy_theme_bg_specs[i].type;
        }
    }

    return BusyThemeFileTypeMax;
}

static bool busy_theme_read_config(BusyTheme* instance, const char* root_path) {
    bool success = false;

    FuriString* config_path = furi_string_alloc_printf("%s/%s", root_path, CONFIG_FILE_NAME);
    FuriString* asset_path = furi_string_alloc();

    JsonConfig* json = json_config_alloc();

    do {
        if(json_config_open(json, furi_string_get_cstr(config_path)) != JsonConfigStatusOk) {
            FURI_LOG_D(TAG, "Failed to open config file: %s", furi_string_get_cstr(config_path));
            break;
        }

        if(json_config_read_str(json, CONFIG_KEY_ASSET, asset_path, NULL) != JsonConfigStatusOk) {
            FURI_LOG_D(TAG, "Failed to read %s key", CONFIG_KEY_ASSET);
            break;
        }

        const char* path = furi_string_get_cstr(asset_path);
        BusyThemeFileType type = busy_theme_detect_file_type(path);
        if(type == BusyThemeFileTypeMax) {
            FURI_LOG_D(TAG, "Failed to parse file type from path %s", path);
            break;
        }

        Storage* storage = furi_record_open(RECORD_STORAGE);
        bool exists = storage_file_exists(storage, path);
        furi_record_close(RECORD_STORAGE);

        if(!exists) {
            FURI_LOG_D(TAG, "Failed to find file %s", path);
            break;
        }

        furi_string_set(instance->bg_path, asset_path);
        instance->bg_type = type;
        success = true;

    } while(false);

    json_config_free(json);
    furi_string_free(asset_path);
    furi_string_free(config_path);

    return success;
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

    busy_theme_set_default(instance);

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

void busy_theme_set(BusyTheme* instance, const BusyTheme* other) {
    furi_assert(instance);
    furi_assert(other);

    furi_string_set(instance->name, other->name);
    furi_string_set(instance->bg_path, other->bg_path);
    instance->bg_type = other->bg_type;
}

void busy_theme_get_info(const BusyTheme* instance, BusyThemeInfo* info) {
    furi_assert(instance);
    furi_assert(info);

    info->name = furi_string_get_cstr(instance->name);
    info->bg_path = furi_string_get_cstr(instance->bg_path);
    info->bg_type = instance->bg_type;
}

void busy_theme_set_default(BusyTheme* instance) {
    furi_assert(instance);

    furi_string_set(instance->name, DEFAULT_NAME);
    furi_string_set(instance->bg_path, BUSY_ANIM_PATH("indicator_busy_72x16.anim"));
    instance->bg_type = BusyThemeFileTypeAnim;
}

bool busy_theme_read(BusyTheme* instance, const char* name) {
    furi_assert(instance);
    furi_assert(name);

    // Special case for default theme
    if(strcmp(name, DEFAULT_NAME) == 0) {
        busy_theme_set_default(instance);
        return true;
    }

    bool success = false;

    Storage* storage = furi_record_open(RECORD_STORAGE);
    FuriString* theme_dir_path = furi_string_alloc_printf("%s/%s", BUSY_THEMES_DIR, name);

    do {
        const char* root_path = furi_string_get_cstr(theme_dir_path);

        if(!storage_dir_exists(storage, root_path)) {
            break;
        }

        if(!busy_theme_read_config(instance, root_path)) {
            break;
        }

        furi_string_set(instance->name, name);

        success = true;

    } while(false);

    furi_string_free(theme_dir_path);
    furi_record_close(RECORD_STORAGE);

    return success;
}

const char* busy_theme_get_name(const BusyTheme* instance) {
    furi_assert(instance);

    return furi_string_get_cstr(instance->name);
}

bool busy_theme_is_default(const BusyTheme* instance) {
    furi_assert(instance);

    return furi_string_equal(instance->name, DEFAULT_NAME);
}

bool busy_theme_is_equal(const BusyTheme* instance, const BusyTheme* other) {
    return furi_string_equal(instance->name, other->name);
}
