/**
 * @file summary_view.h
 * @brief A widget that provides an animated timer session summary.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SummaryView opaque structure. */
typedef struct SummaryView SummaryView;

/**
 * @brief Animation completion callback function type.
 *
 * @param[in,out] context pointer to a user-specified context object
 */
typedef void (*SummaryViewCallback)(void* context);

/**
 * @brief Create a new SummaryView instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created SummaryView instance
 */
SummaryView* summary_view_alloc(Widget* parent);

/**
 * @brief Delete a SummaryView instance.
 *
 * @param[in,out] instance pointer to the SummaryView instance to be deleted
 */
void summary_view_free(SummaryView* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the SummaryView instance to be queried
 * @returns pointer to the base class instance
 */
Widget* summary_view_get_base(SummaryView* instance);

/**
 * @brief Set the cycles count to display.
 *
 * Affects the number of blocks in the animation. Values higher than 10
 * will be silently clamped.
 *
 * @param[in,out] instance pointer to the SummaryView instance to be modified
 * @param[in] cycles_count number of cycles to display
 */
void summary_view_set_cycles_count(SummaryView* instance, uint32_t cycles_count);

/**
 * @brief Set the function to be called upon the animation completion.
 *
 * @param[in,out] instance pointer to the SummaryView instance to be modified
 * @param[in] callback pointer to the callback function
 * @param[in,out] context pointer to the user-specified object (will be passed to the callback)
 */
void summary_view_set_completed_callback(
    SummaryView* instance,
    SummaryViewCallback callback,
    void* context);

#ifdef __cplusplus
}
#endif
