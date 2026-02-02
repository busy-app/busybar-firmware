/**
 * @file progress_counter.h
 * @brief A widget that shows the progress
 *        in the form of a spinning counter.
 *
 * Can be used only on the front display.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** ProgressCounter opaque structure. */
typedef struct ProgressCounter ProgressCounter;

/**
 * @brief Create a new ProgressCounter instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created ProgressCounter instance
 */
ProgressCounter* progress_counter_alloc(Widget* parent);

/**
 * @brief Delete a ProgressCounter instance.
 *
 * @param[in,out] instance pointer to the ProgressCounter instance to be deleted
 */
void progress_counter_free(ProgressCounter* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the ProgressCounter instance to be queried
 * @returns pointer to the base class instance
 */
Widget* progress_counter_get_base(ProgressCounter* instance);

/**
 * @brief Set the progress values and play a rollover animation.
 *
 * @param[in,out] instance pointer to the ProgressCounter instance to be modified
 * @param[in] done the number of done cycles (cannot be 0)
 * @param[in] total the total number of cycles
 */
void progress_counter_set_values(ProgressCounter* instance, uint32_t done, uint32_t total);

#ifdef __cplusplus
}
#endif
