/**
 * @file timer_label.h
 * @brief
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TimerLabel opaque structure. */
typedef struct TimerLabel TimerLabel;

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

void timer_label_set_time_left(TimerLabel* instance, uint32_t time_left_s);

#ifdef __cplusplus
}
#endif
