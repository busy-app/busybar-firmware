/**
 * @file summary_label.h
 * @brief A widget that provides a text-based timer session summary.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** SummaryLabel opaque structure. */
typedef struct SummaryLabel SummaryLabel;

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
 * @brief Set the number of cycles to show in the cycles display section.
 *
 * @param[in,out] instance pointer to the SummaryLabel instance to be modified
 * @param[in] cycles_count number of cycles to show
 */
void summary_label_set_cycles_count(SummaryLabel* instance, uint32_t cycles_count);

/**
 * @brief Switch the cycles display to the text message and vice versa.
 *
 * @param[in,out] instance pointer to the SummaryLabel instance to be modified
 */
void summary_label_switch_display(SummaryLabel* instance);

#ifdef __cplusplus
}
#endif
