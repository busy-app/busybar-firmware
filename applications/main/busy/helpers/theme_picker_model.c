#include "theme_picker_model.h"

#include <m-array.h>

ARRAY_DEF(BusyThemeArray, BusyTheme*, BUSY_THEME_OPLIST);

struct ThemePickerModel {
    BusyThemeArray_t items;
};

ThemePickerModel* theme_picker_model_alloc(void) {
    ThemePickerModel* instance = malloc(sizeof(ThemePickerModel));

    BusyThemeArray_init(instance->items);

    return instance;
}

void theme_picker_model_free(ThemePickerModel* instance) {
    furi_assert(instance);

    BusyThemeArray_clear(instance->items);

    free(instance);
}

void theme_picker_model_add_item(ThemePickerModel* instance, const BusyTheme* item) {
    furi_assert(instance);
    furi_assert(item);

    BusyThemeArray_push_back(instance->items, (BusyTheme*)item);
}

const BusyTheme* theme_picker_model_get_item(const ThemePickerModel* instance, uint32_t index) {
    furi_assert(instance);

    return *BusyThemeArray_cget(instance->items, index);
}

uint32_t theme_picker_model_get_item_count(const ThemePickerModel* instance) {
    return BusyThemeArray_size(instance->items);
}

uint32_t
    theme_picker_model_get_item_index_by_name(const ThemePickerModel* instance, const char* name) {
    furi_assert(instance);
    furi_assert(name);

    uint32_t index = THEME_PICKER_MODEL_INVALID_INDEX;

    for(uint32_t i = 0; i < BusyThemeArray_size(instance->items); ++i) {
        const BusyTheme* theme = *BusyThemeArray_cget(instance->items, i);
        if(strcmp(busy_theme_get_name(theme), name) == 0) {
            index = i;
            break;
        }
    }

    return index;
}

void theme_picker_model_sort(ThemePickerModel* instance) {
    furi_assert(instance);
    BusyThemeArray_special_stable_sort(instance->items);
}
