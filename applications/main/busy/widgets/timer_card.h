/**
 * @file timer_card.h
 * @brief A widget that shows the timer status.
 *
 * Can be used only on the back display.
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

/**
 * @brief Show or hide the top text
 *
 * @param[in,out] instance pointer to the TimerCard instance to be modified
 * @param[in] show show the top text if @c true, hide if @c false
 */
void timer_card_show_header(TimerCard* instance, bool show);

/**
 * @brief Show or hide the time display
 *
 * @param[in,out] instance pointer to the TimerCard instance to be modified
 * @param[in] show show the time display if @c true, hide if @c false
 */
void timer_card_show_time(TimerCard* instance, bool show);

/**
 * @brief Set the displayed time.
 *
 * @param[in,out] instance pointer to the TimerCard instance to be modified
 * @param[in] time_s time to show, in seconds
 */
void timer_card_set_time(TimerCard* instance, uint32_t time_s);

#ifdef __cplusplus
}
#endif
