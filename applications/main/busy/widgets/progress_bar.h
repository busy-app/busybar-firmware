/**
 * @file progress_bar.h
 * @brief A widget that shows the remaining time.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

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
 * @brief Set the displayed time.
 *
 * @param[in,out] instance pointer to the ProgressBar instance to be modified
 * @param[in] value progress value (@c 0.0 - no fill, @c 1.0 - fully filled)
 */
void progress_bar_set_value(ProgressBar* instance, float value);

/**
 * @brief Set alternate colour scheme.
 *
 * @param[in,out] instance pointer to the ProgressBar instance to be modified
 * @param[in] set if @c true, select alternate colour scheme.
 */
void progress_bar_set_alt_color(ProgressBar* instance, bool set);

#ifdef __cplusplus
}
#endif
