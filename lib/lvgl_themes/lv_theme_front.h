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
lv_theme_t * lv_theme_front_init(lv_display_t * disp);

/**
* Check if the theme is initialized
* @return true if default theme is initialized, false otherwise
*/
bool lv_theme_front_is_inited(void);

/**
 * Get front theme
 * @return a pointer to front theme, or NULL if this is not initialized
 */
lv_theme_t * lv_theme_front_get(void);

/**
 * Deinitialize the front theme
 */
void lv_theme_front_deinit(void);

#endif

#ifdef __cplusplus
} /*extern "C"*/
#endif
