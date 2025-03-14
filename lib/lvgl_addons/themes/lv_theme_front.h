/**
 * @file lv_theme_front.h
 *
 */

#ifndef LV_THEME_FRONT_H
#define LV_THEME_FRONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl/src/themes/lv_theme.h>
#include <lvgl/src/display/lv_display.h>

/**
 * Initialize the theme
 * @param disp pointer to display to attach the theme
 * @return a pointer to reference this theme later
 */
lv_theme_t* lv_theme_front_alloc(lv_display_t* disp);

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif
