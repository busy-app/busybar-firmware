/**
 * @file lv_theme_back.h
 * @brief Widget theme for the BSB back display.
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/src/themes/lv_theme.h>
#include <lvgl/src/display/lv_display.h>

/**
 * @brief Create a new theme instance
 *
 * @param disp pointer to display to attach the theme
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_theme_back_alloc(lv_display_t* disp);

#ifdef __cplusplus
} /*extern "C"*/
#endif
