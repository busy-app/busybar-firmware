#include "font_registry.h"
#include <stb/stb.h>

#define TAG "FontRegistry"

typedef struct {
    const char* key;
    struct {
        lv_font_t* loaded_data;
        size_t references;
    } value;
} LoadedFont;

struct FontRegistry {
    FuriMutex* mutex;
    LoadedFont* loaded_fonts;
};

const lv_font_t* font_registry_load_font(FontRegistry* instance, const char* font_path) {
    furi_check(instance);
    furi_check(font_path);

    const lv_font_t* ret_val = NULL;
    furi_check(furi_mutex_acquire(instance->mutex, FuriWaitForever) == FuriStatusOk);

    do {
        LoadedFont* already_loaded = stbds_shgetp_null(instance->loaded_fonts, font_path);
        if(already_loaded) {
            already_loaded->value.references++;
            FURI_LOG_D(TAG, "Font \"%s\": references=%zu (+1)", font_path, already_loaded->value.references);
            ret_val = already_loaded->value.loaded_data;
            break;
        }

        lv_font_t* font_data = lv_binfont_create(font_path);
        if(!font_data) break;

        LoadedFont now_loaded = {
            .key = font_path,
            .value = {
                .loaded_data = font_data,
                .references = 1,
            },
        };

        FURI_LOG_D(TAG, "Font \"%s\": references=1 (newly loaded)", font_path);

        stbds_shputs(instance->loaded_fonts, now_loaded);
        ret_val = font_data;
    } while(0);

    if(!ret_val) {
        FURI_LOG_W(TAG, "Font \"%s\" failed to load, using default", font_path);
        ret_val = LV_FONT_DEFAULT;
    }

    furi_check(furi_mutex_release(instance->mutex) == FuriStatusOk);

    return ret_val;
}

void font_registry_unload_font(FontRegistry* instance, const lv_font_t* const_font) {
    furi_check(instance);
    furi_check(const_font);

    // the only font in RO memory that `load` can return is the default one
    if(const_font == LV_FONT_DEFAULT) return;
    lv_font_t* font = (lv_font_t*)const_font;

    furi_check(furi_mutex_acquire(instance->mutex, FuriWaitForever) == FuriStatusOk);

    bool found_font = false;

    for(size_t i = 0; i < stbds_shlenu(instance->loaded_fonts); i++) {
        LoadedFont* item = &instance->loaded_fonts[i];
        if(item->value.loaded_data != font) continue;

        found_font = true;
        item->value.references--;
        FURI_LOG_D(TAG, "Font \"%s\": references=%zu (-1)", item->key, item->value.references);

        if(!item->value.references) {
            FURI_LOG_D(TAG, "Font \"%s\": unloading", item->key);
            lv_binfont_destroy(item->value.loaded_data);
            bool did_delete = stbds_shdel(instance->loaded_fonts, item->key);
            furi_check(did_delete);
        }
    }

    if(!found_font) furi_crash("Font provided to `unload` is not loaded in this FontRegistry");

    furi_check(furi_mutex_release(instance->mutex) == FuriStatusOk);
}

int font_registry_startup(void* arg) {
    UNUSED(arg);

    FontRegistry* registry = malloc(sizeof(FontRegistry));
    registry->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
    stbds_sh_new_strdup(registry->loaded_fonts);

    furi_record_create(RECORD_FONT_REGISTRY, registry);

    return 0;
}
