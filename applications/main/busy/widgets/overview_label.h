/**
 * @file overview_label.h
 * @brief A widget that provides a timer session overview.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** OverviewLabel opaque structure. */
typedef struct OverviewLabel OverviewLabel;

/**
 * @brief Create a new OverviewLabel instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created OverviewLabel instance
 */
OverviewLabel* overview_label_alloc(Widget* parent);

/**
 * @brief Delete a OverviewLabel instance.
 *
 * @param[in,out] instance pointer to the OverviewLabel instance to be deleted
 */
void overview_label_free(OverviewLabel* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the OverviewLabel instance to be queried
 * @returns pointer to the base class instance
 */
Widget* overview_label_get_base(OverviewLabel* instance);

/**
 * @brief Set the displayed overview time.
 *
 * @param[in,out] instance pointer to the OverviewLabel instance to be modified
 * @param[in] work_time_mn work time to display, in minutes
 * @param[in] rest_time_mn rest time to display, in minutes
 */
void overview_label_set_intervals(
    OverviewLabel* instance,
    uint32_t work_time_mn,
    uint32_t rest_time_mn);

#ifdef __cplusplus
}
#endif
