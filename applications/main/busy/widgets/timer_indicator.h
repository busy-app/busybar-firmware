/**
 * @file timer_indicator.h
 * @brief A widget that shows the current timer status.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** TimerIndicator opaque structure. */
typedef struct TimerIndicator TimerIndicator;

typedef enum {
    TimerIndicatorProgressDirectionHorizontal,
    TimerIndicatorProgressDirectionVertical,
    TimerIndicatorProgressDirectionMax,
} TimerIndicatorProgressDirection;

typedef struct {
    const char* anim_path;
} TimerIndicatorBgConfig;

typedef struct {
    const char* lottie_path;
    TimerIndicatorProgressDirection direction;
    uint8_t start_offset_px;
    uint8_t end_offset_px;
} TimerIndicatorProgressConfig;

typedef struct {
    const char* image_path;
} TimerIndicatorFgConfig;

typedef struct {
    TimerIndicatorBgConfig background_config;
    TimerIndicatorProgressConfig progress_config;
    TimerIndicatorFgConfig foreground_config;
} TimerIndicatorPreset;

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
 * @brief Set the active preset for a TimerIndicator instance.
 *
 * TimerIndicator presets consist of the following parts:
 * - Background animation (.anim sequence file)
 * - Lottie animation, used for a visual progress representation
 * - Foreground image, containing static elements (.bin image file)
 *
 * Any of these parts may be omitted, in that case NULL should be passed
 * instead of the respective file path.
 *
 * @param[in,out] instance pointer to the TimerIndicator instance to be modified
 */
void timer_indicator_set_preset(TimerIndicator* instance, const TimerIndicatorPreset* preset);

/**
 * @brief Set the progress value for the progress Lottie animation.
 *
 * @note If no Lottie animation is present in the current
 *       preset, this function will do nothing
 *
 * @param[in,out] instance pointer to the TimerIndicator instance to be modified
 * @param[in] progress number between 0 and 1 (inclusive)
 */
void timer_indicator_set_progress(TimerIndicator* instance, float progress);

/**
 * @brief Enable or disable animations shown by a TimerIndicator instance.
 *
 * @param[in,out] instance pointer to the TimerIndicator instance to be modified
 * @param[in] enable play animations if @c true, stop them otherwise
 */
void timer_indicator_enable_animations(TimerIndicator* instance, bool enable);

#ifdef __cplusplus
}
#endif
