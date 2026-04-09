#include "font_registry_i.h"

#define TAG "FontRegistry"

#define FONT_CACHE_CAPACITY 7

typedef struct {
    const char* simulated_path;
    const lv_font_t* font;
} FontRegistryBaked;

#ifdef FURI_RAM_EXEC
extern const lv_font_t lv_font_busy_regular_7;
extern const lv_font_t lv_font_busy_regular_9;
#endif

static const FontRegistryBaked font_registry_baked[] = {
    {FONT_BUSY_REGULAR_5, &lv_font_busy_regular_5},
#ifdef FURI_RAM_EXEC
    {FONT_BUSY_REGULAR_7, &lv_font_busy_regular_7},
    {FONT_BUSY_REGULAR_9, &lv_font_busy_regular_9},
#endif
};

static const lv_font_t*
    font_registry_get_baked_font(FontRegistry* instance, const char* font_path) {
    furi_assert(instance);
    furi_assert(font_path);

    for(size_t i = 0; i < COUNT_OF(font_registry_baked); i++) {
        if(strcmp(font_path, font_registry_baked[i].simulated_path) == 0) {
            FURI_LOG_T(TAG, "Font \"%s\": baked", font_path);
            return font_registry_baked[i].font;
        }
    }

    return NULL;
}

static const lv_font_t*
    font_registry_get_loaded_font(FontRegistry* instance, const char* font_path) {
    furi_assert(instance);
    furi_assert(font_path);

    FontRegistryLoadedFont* already_loaded = stbds_shgetp_null(instance->loaded_fonts, font_path);
    if(already_loaded) {
        already_loaded->value.references++;
        already_loaded->value.last_access = ++instance->access_counter;
        FURI_LOG_T(
            TAG, "Font \"%s\": references=%zu (+1)", font_path, already_loaded->value.references);
        return already_loaded->value.loaded_data;
    }

    return NULL;
}

static void font_registry_cache_evict(FontRegistry* instance) {
    furi_assert(instance);

    if(stbds_shlenu(instance->loaded_fonts) <= FONT_CACHE_CAPACITY) return;

    size_t oldest_access = SIZE_MAX;
    FontRegistryLoadedFont* font_to_evict = NULL;
    for(size_t i = 0; i < stbds_shlenu(instance->loaded_fonts); i++) {
        FontRegistryLoadedFont* item = &instance->loaded_fonts[i];
        if(item->value.references == 0 && item->value.last_access < oldest_access) {
            oldest_access = item->value.last_access;
            font_to_evict = item;
        }
    }

    if(font_to_evict) {
        FURI_LOG_T(TAG, "Font \"%s\": evicting from cache", font_to_evict->key);
        lv_binfont_destroy(font_to_evict->value.loaded_data);
        bool was_deletion_successful = stbds_shdel(instance->loaded_fonts, font_to_evict->key);
        furi_check(was_deletion_successful);
    }
}

static size_t font_registry_get_file_size(FontRegistry* instance, const char* font_path) {
    furi_assert(instance);
    furi_assert(font_path);

    FileInfo file_info;
    FS_Error fs_error = storage_common_stat(instance->storage, font_path, &file_info);

    return (fs_error == FSE_OK) ? (size_t)file_info.size : 0;
}

static const lv_font_t* font_registry_do_load_font(FontRegistry* instance, const char* font_path) {
    furi_assert(instance);
    furi_assert(font_path);

    lv_font_t* font_data = lv_binfont_create(font_path);
    if(!font_data) return NULL;

    FontRegistryLoadedFont now_loaded = {
        .key = font_path,
        .value =
            {
                .loaded_data = font_data,
                .references = 1,
                .last_access = ++instance->access_counter,
            },
    };

    FURI_LOG_T(TAG, "Font \"%s\": references=1 (newly loaded)", font_path);

    stbds_shputs(instance->loaded_fonts, now_loaded);
    font_registry_cache_evict(instance);
    return font_data;
}

const lv_font_t* font_registry_load_font(FontRegistry* instance, const char* font_path) {
    furi_check(instance);
    furi_check(font_path);

    const lv_font_t* lv_loaded_font = NULL;
    FontRegistryLoadedFont* loaded_font = NULL;
    furi_check(furi_mutex_acquire(instance->mutex, FuriWaitForever) == FuriStatusOk);

    do {
        if((lv_loaded_font = font_registry_get_baked_font(instance, font_path))) break;
        if((lv_loaded_font = font_registry_get_loaded_font(instance, font_path))) break;
        lv_loaded_font = font_registry_do_load_font(instance, font_path);
        loaded_font = stbds_shgetp_null(instance->loaded_fonts, font_path);
    } while(0);

    if(!lv_loaded_font) {
        FURI_LOG_W(TAG, "Font \"%s\" failed to load, using default", font_path);
        lv_loaded_font = LV_FONT_DEFAULT;
    }

    furi_check(furi_mutex_release(instance->mutex) == FuriStatusOk);

    if(loaded_font) {
        loaded_font->value.estimated_memory_size =
            font_registry_get_file_size(instance, font_path);
    }

    return lv_loaded_font;
}

static bool font_registry_unload_baked_font(FontRegistry* instance, const lv_font_t* const_font) {
    furi_assert(instance);
    furi_assert(const_font);

    for(size_t i = 0; i < COUNT_OF(font_registry_baked); i++) {
        if(font_registry_baked[i].font == const_font) return true;
    }

    return false;
}

void font_registry_unload_font(FontRegistry* instance, const lv_font_t* const_font) {
    furi_check(instance);
    furi_check(const_font);

    if(font_registry_unload_baked_font(instance, const_font)) return;

    // the only fonts in RO memory that `load` can return are the baked ones
    lv_font_t* font = (lv_font_t*)const_font;

    furi_check(furi_mutex_acquire(instance->mutex, FuriWaitForever) == FuriStatusOk);

    bool found_font = false;

    for(size_t i = 0; i < stbds_shlenu(instance->loaded_fonts); i++) {
        FontRegistryLoadedFont* item = &instance->loaded_fonts[i];
        if(item->value.loaded_data == font) {
            found_font = true;
            item->value.references--;
            FURI_LOG_T(TAG, "Font \"%s\": references=%zu (-1)", item->key, item->value.references);
            break;
        }
    }

    if(!found_font) furi_crash("Font provided to `unload` is not loaded in this FontRegistry");

    font_registry_cache_evict(instance);
    furi_check(furi_mutex_release(instance->mutex) == FuriStatusOk);
}

int font_registry_startup(void* arg) {
    UNUSED(arg);

    FontRegistry* registry = malloc(sizeof(FontRegistry));

    registry->storage = furi_record_open(RECORD_STORAGE);
    registry->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    stbds_sh_new_strdup(registry->loaded_fonts);

    furi_record_create(RECORD_FONT_REGISTRY, registry);

    return 0;
}
