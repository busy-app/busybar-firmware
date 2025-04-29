/**
 * @file progress_bar.h
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

/** ProgressBar opaque structure. */
typedef struct ProgressBar ProgressBar;

/**
 * @brief Create a new ProgressBar instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created ProgressBar instance
 */
ProgressBar* progress_bar_alloc(Widget* parent);

/**
 * @brief Delete a ProgressBar instance.
 *
 * @param[in,out] instance pointer to the ProgressBar instance to be deleted
 */
void progress_bar_free(ProgressBar* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the ProgressBar instance to be queried
 * @returns pointer to the base class instance
 */
Widget* progress_bar_get_base(ProgressBar* instance);

/**
 * @brief Set the animation file for the bar part.
 *
 * @param[in,out] instance pointer to the ProgressBar instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to the animation file
 * @returns @c true if the file could be loaded, @c false otherwise
 */
bool progress_bar_set_anim_source(ProgressBar* instance, const char* file_path);

/**
 * @brief Set the fill color for the background trough.
 *
 * @param[in,out] instance pointer to the ProgressBar instance to be modified
 * @param[in] color trough color value
 */
void progress_bar_set_trough_color(ProgressBar* instance, Color color);

/**
 * @brief Set the displayed value.
 *
 * @param[in,out] instance pointer to the ProgressBar instance to be modified
 * @param[in] value progress value (@c 0.0 - no fill, @c 1.0 - fully filled)
 */
void progress_bar_set_value(ProgressBar* instance, float value);

#ifdef __cplusplus
}
#endif
