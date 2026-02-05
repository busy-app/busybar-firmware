/**
 * @file countdown.h
 * @brief A widget that displays time until or from the specified timestamp.
 */
#pragma once

#include <gui/widget.h>
#include <gui/gui.h>
#include <toolbox/color.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Countdown opaque structure. */
typedef struct Countdown Countdown;

typedef enum {
    CountdownDirectionTimeLeft,
    CountdownDirectionTimeSince,
    CountdownDirectionMAX,
} CountdownDirection;

typedef enum {
    CountdownShowHourAlways,
    CountdownShowHourWhenNonZero,
    CountdownShowHourMAX,
} CountdownShowHour;

/**
 * @brief Countdown callback function type.
 *
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*CountdownCallback)(void* context);

/**
 * @brief Create a new Countdown instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Countdown instance
 */
Countdown* countdown_alloc(Widget* parent);

/**
 * @brief Delete a Countdown instance.
 *
 * @param[in,out] instance pointer to the Countdown instance to be deleted
 */
void countdown_free(Countdown* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the Countdown instance to be queried
 * @returns pointer to the base class instance
 */
Widget* countdown_get_base(Countdown* instance);

/**
 * @brief Set a countdown callback.
 *
 * The callback function is called when the timer reaches zero. 
 * Works only when CountdownDirectionTimeLeft is set.
 *
 * @param[in,out] instance pointer to the Countdown instance to be queried
 * @param[in] callback callback function pointer
 * @param[in] context callback context data
 */
void countdown_set_callback(Countdown* instance, CountdownCallback callback, void* context);

/**
 * @brief Set the countdown text color.
 *
 * @param[in,out] instance pointer to the Countdown instance to be modified
 * @param[in] color text color
 */
void countdown_set_text_color(Countdown* instance, Color color);

/**
 * @brief Starts counting down (or up)
 * 
 * @param[in] instance pointer to the Countdown instance to be modified
 * @param[in] time seconds-based Unix timestamp to count down to or up from
 * @param[in] direction explicit counting direction - see CountdownDirection enum
 * @param[in] hours when to show the hours position - see CountdownShowHour enum
 */
void countdown_begin(
    Countdown* instance,
    time_t time,
    CountdownDirection direction,
    CountdownShowHour hours);

/**
 * @brief Stop counting
 * 
 * @param[in] instance pointer to the Countdown instance to be modified
 */
void countdown_stop(Countdown* instance);

#ifdef __cplusplus
}
#endif
