/**
 * @file summary_label.h
 * @brief
 *
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SummaryLabel opaque structure. */
typedef struct SummaryLabel SummaryLabel;

typedef void (*SummaryLabelCallback)(void* context);

/**
 * @brief Create a new SummaryLabel instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created SummaryLabel instance
 */
SummaryLabel* summary_label_alloc(Widget* parent);

/**
 * @brief Delete a SummaryLabel instance.
 *
 * @param[in,out] instance pointer to the SummaryLabel instance to be deleted
 */
void summary_label_free(SummaryLabel* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the SummaryLabel instance to be queried
 * @returns pointer to the base class instance
 */
Widget* summary_label_get_base(SummaryLabel* instance);

/**
 * @brief
 *
 * @param[in,out] instance pointer to the SummaryLabel instance to be modified
 */
void summary_label_set_cycles_count(SummaryLabel* instance, uint32_t cycles_count);

#ifdef __cplusplus
}
#endif
