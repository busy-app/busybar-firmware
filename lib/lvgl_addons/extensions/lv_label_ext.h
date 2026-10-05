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
 * @note The distance is read from the label's self size cache, whose value
 * depends on the current long mode: @c LV_LABEL_LONG_MODE_DOTS and @c
 * LV_LABEL_LONG_MODE_WRAP cache a wrap-bounded width, yielding a too-short
 * duration and an overly fast scroll. Call this only while the label is in
 * an expand long mode (@c CLIP, @c SCROLL, @c SCROLL_CIRCULAR) — in
 * practice, after @c lv_label_set_long_mode, not before.
 *
 * @param[in, out] label      label lvgl object
 * @param[in]      px_per_min scroll speed in pixels per minute (must be > 0)
 */
void lv_label_ext_set_anim_speed(lv_obj_t* label, uint32_t px_per_min);

/**
 * @brief Cap the label's height to N text lines.
 * Sets max_height to line_height * max_lines + line_space * (max_lines - 1),
 * computed once at call time from the label's current font.
 *
 * @param[in, out] label     label lvgl object
 * @param[in]      max_lines maximum number of text lines (must be > 0)
 */
void lv_label_ext_set_max_lines(lv_obj_t* label, uint32_t max_lines);

#ifdef __cplusplus
}
#endif
