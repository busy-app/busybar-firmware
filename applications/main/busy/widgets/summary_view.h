/**
 * @file summary_view.h
 * @brief
 *
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SummaryView opaque structure. */
typedef struct SummaryView SummaryView;

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
 * @brief
 *
 * @param[in,out] instance pointer to the SummaryView instance to be modified
 */
void summary_view_set_cycles_count(SummaryView* instance, uint32_t cycles_count);

#ifdef __cplusplus
}
#endif
