/**
 * @file lv_label_ext.h
 * @brief LVGL label extensions.
 */

#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set the scroll animation speed for a label.
 * The scroll distance is computed to match LVGL's internal scroll path.
 *
 * @warning Only valid for @c LV_LABEL_LONG_MODE_SCROLL_CIRCULAR.
 *
 * @param[in, out] object label object
 * @param[in]      speed  scroll speed in pixels per minute (must be > 0)
 */
void lv_label_ext_set_anim_speed(lv_obj_t* object, uint32_t speed);

#ifdef __cplusplus
}
#endif
