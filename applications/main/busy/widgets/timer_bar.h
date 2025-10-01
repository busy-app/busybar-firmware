/**
 * @file timer_bar.h
 * @brief A widget that shows the remaining time.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#include <color.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TimerBar opaque structure. */
typedef struct TimerBar TimerBar;

typedef struct {
    const char* file_path;
    Color trough_color;
} TimerBarPreset;

/**
 * @brief Create a new TimerBar instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created TimerBar instance
 */
TimerBar* timer_bar_alloc(Widget* parent);

/**
 * @brief Delete a TimerBar instance.
 *
 * @param[in,out] instance pointer to the TimerBar instance to be deleted
 */
void timer_bar_free(TimerBar* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the TimerBar instance to be queried
 * @returns pointer to the base class instance
 */
Widget* timer_bar_get_base(TimerBar* instance);

void timer_bar_set_preset(TimerBar* instance, const TimerBarPreset* preset);

/**
 * @brief Set the displayed value.
 *
 * @param[in,out] instance pointer to the TimerBar instance to be modified
 * @param[in] value progress value (@c 0.0 - no fill, @c 1.0 - fully filled)
 */
void timer_bar_set_value(TimerBar* instance, float value);

#ifdef __cplusplus
}
#endif
