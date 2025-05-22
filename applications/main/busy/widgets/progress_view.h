/**
 * @file progress_view.h
 * @brief
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ProgressView opaque structure. */
typedef struct ProgressView ProgressView;

/**
 * @brief Create a new ProgressView instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created ProgressView instance
 */
ProgressView* progress_view_alloc(Widget* parent);

/**
 * @brief Delete a ProgressView instance.
 *
 * @param[in,out] instance pointer to the ProgressView instance to be deleted
 */
void progress_view_free(ProgressView* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the ProgressView instance to be queried
 * @returns pointer to the base class instance
 */
Widget* progress_view_get_base(ProgressView* instance);

/**
 * @brief Set the progress to display.
 *
 * @param[in,out] instance pointer to the ProgressView instance to be modified
 * @param[in] done number of done intervals
 * @param[in] total total number of intervals
 */
void progress_view_set_progress(ProgressView* instance, uint32_t done, uint32_t total);

#ifdef __cplusplus
}
#endif
