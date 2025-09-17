/**
 * @file ProgressBar.h
 * @brief A widget that displays a static ProgressBar.
 */
#pragma once

#include <gui/widget.h>
#include <toolbox/color.h>

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
 * @brief Delete an ProgressBar instance.
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
* @brief Set the current value of the ProgressBar.
*
* @param[in,out] instance pointer to the ProgressBar instance to be modified
* @param[in] value current value (0-100)
*/
void progress_bar_set_value(ProgressBar* instance, int32_t value);

/**
 * @brief Set the size of the ProgressBar.
 *
 * @param[in,out] instance pointer to the ProgressBar instance to be modified
 * @param[in] width ProgressBar width in pixels
 * @param[in] height ProgressBar height in pixels
 */
void progress_bar_set_size(ProgressBar* instance, uint16_t width, uint16_t height);

#ifdef __cplusplus
}
#endif
