/**
 * @file timer_card.h
 * @brief
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TimerCard opaque structure. */
typedef struct TimerCard TimerCard;

/**
 * @brief Create a new TimerCard instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created TimerCard instance
 */
TimerCard* timer_card_alloc(Widget* parent);

/**
 * @brief Delete a TimerCard instance.
 *
 * @param[in,out] instance pointer to the TimerCard instance to be deleted
 */
void timer_card_free(TimerCard* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the TimerCard instance to be queried
 * @returns pointer to the base class instance
 */
Widget* timer_card_get_base(TimerCard* instance);

void timer_card_show_header(TimerCard* instance, bool show);

void timer_card_show_footer(TimerCard* instance, bool show);

void timer_card_set_time_left(TimerCard* instance, uint32_t time_left_s);

#ifdef __cplusplus
}
#endif
