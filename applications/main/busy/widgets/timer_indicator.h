/**
 * @file timer_indicator.h
 * @brief A widget that shows the current timer status.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/modules/anim_image.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TimerIndicator opaque structure. */
typedef struct TimerIndicator TimerIndicator;

typedef enum {
    TimerIndicatorStateWork,
    TimerIndicatorStateRest,
    TimerIndicatorStateWorkBig,
    TimerIndicatorStateRestBig,
    TimerIndicatorStateMax,
} TimerIndicatorState;

typedef enum {
    TimerIndicatorTransitionOffToSimple,
    TimerIndicatorTransitionMax,
} TimerIndicatorTransition;

typedef struct {
    const char* states[TimerIndicatorStateMax];
    const char* transitions[TimerIndicatorTransitionMax];
} TimerIndicatorAnimSources;

/**
 * @brief Create a new TimerIndicator instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created TimerIndicator instance
 */
TimerIndicator* timer_indicator_alloc(Widget* parent);

/**
 * @brief Delete a TimerIndicator instance.
 *
 * @param[in,out] instance pointer to the TimerIndicator instance to be deleted
 */
void timer_indicator_free(TimerIndicator* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the TimerIndicator instance to be queried
 * @returns pointer to the base class instance
 */
Widget* timer_indicator_get_base(TimerIndicator* instance);

/**
 * @brief Get a pointer to the underlying AnimImage class instance.
 *
 * The return value can be used in all AnimImage methods.
 *
 * @param[in,out] instance pointer to the TimerIndicator instance to be queried
 * @returns pointer to the base class instance
 */
AnimImage* timer_indicator_get_anim_image(TimerIndicator* instance);

/**
 * @brief Set animation sources for different states of a TimerIndicator instance.
 *
 * @param[in,out] instance pointer to the TimerIndicator instance to be operated on
 * @param[in] sources pointer to the structure containing animation source paths
 */
void timer_indicator_set_anim_sources(
    TimerIndicator* instance,
    const TimerIndicatorAnimSources* sources);

/**
 * @brief Set current state of a TimerIndicator instance.
 *
 * If a transition is defined between the old and new state, it will be played.
 *
 * @param[in,out] instance pointer to the TimerIndicator instance to be operated on
 * @param[in] state value to determine the new state
 */
void timer_indicator_set_state(TimerIndicator* instance, TimerIndicatorState state);

#ifdef __cplusplus
}
#endif
