#pragma once

#include <lvgl/src/themes/lv_theme.h>
#include <lvgl/src/display/lv_display.h>

/**
 * @brief Pointer to function which is used for theme allocation
 */
typedef lv_theme_t* (*ThemeAllocHandler)(lv_display_t* disp);

typedef struct GuiThemeRegistry GuiThemeRegistry;

/**
 * @brief Inits the collection of available themes
 *
 * @param[in] display pointer to the lv_display instance for which themes will be used
 * @param[in] theme_count amount of themes for registration
 * @param[in] allocators array of alloc functions to create each theme instance inside
 * @return pointer to the theme registry
 */
GuiThemeRegistry* gui_theme_registry_init(
    lv_display_t* display,
    size_t theme_count,
    ThemeAllocHandler* allocators);

/**
 * @brief Inits the collection of available themes
 *
 * @param[in] instance pointer to the theme registry
 * @param[in] theme_id index of desired theme
 */
lv_theme_t* gui_theme_registry_get_theme(GuiThemeRegistry* instance, uint32_t theme_id);
