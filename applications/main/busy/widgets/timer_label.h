/**
 * @file timer_label.h
 * @brief A widget that shows the remaining time.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TimerLabel opaque structure. */
typedef struct TimerLabel TimerLabel;

/** TimerLabel preset type. */
typedef struct {
    struct {
        Color base; /**< Countdown base colour */
        Color blink; /**< Countdown blink colour */
    } countdown_colors; /**< Special colours used for countdown animation */
} TimerLabelPreset;

/**
 * @brief Create a new TimerLabel instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created TimerLabel instance
 */
TimerLabel* timer_label_alloc(Widget* parent);

/**
 * @brief Delete a TimerLabel instance.
 *
 * @param[in,out] instance pointer to the TimerLabel instance to be deleted
 */
void timer_label_free(TimerLabel* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the TimerLabel instance to be queried
 * @returns pointer to the base class instance
 */
Widget* timer_label_get_base(TimerLabel* instance);

/**
 * @brief Set the displayed time.
 *
 * @param[in,out] instance pointer to the TimerLabel instance to be modified
 * @param[in] time_s time to show, in seconds
 */
void timer_label_set_time(TimerLabel* instance, uint32_t time_s);

/**
 * @brief Set the current preset (affects countdown blinking)
 *
 * @param[in,out] instance pointer to the TimerLabel instance to be modified
 * @param[in] preset pointer to the TimerLabelPreset object
 */
void timer_label_set_preset(TimerLabel* instance, const TimerLabelPreset* preset);

void timer_label_enable_background(TimerLabel* instance, bool enable);

void timer_label_reveal(TimerLabel* instance);

void timer_label_hide(TimerLabel* instance);

#ifdef __cplusplus
}
#endif
