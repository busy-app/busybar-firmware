#include "theme_registry.h"

struct GuiThemeRegistry {
    size_t theme_count;
    lv_theme_t** themes;
};

GuiThemeRegistry* gui_theme_registry_init(
    lv_display_t* display,
    size_t theme_count,
    ThemeAllocHandler* allocators) {
    GuiThemeRegistry* instance = malloc(sizeof(GuiThemeRegistry));

    instance->theme_count = theme_count;
    instance->themes = malloc(sizeof(lv_theme_t*) * theme_count);
    for(size_t i = 0; i < theme_count; i++) {
        if(allocators[i] == NULL) continue;
        instance->themes[i] = allocators[i](display);
    }

    return instance;
}

lv_theme_t* gui_theme_registry_get_theme(GuiThemeRegistry* instance, uint32_t theme_id) {
    furi_assert(instance);
    furi_assert(theme_id < instance->theme_count);
    return instance->themes[theme_id];
}
