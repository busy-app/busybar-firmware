/**
 * @file nav_stack.h
 * @brief
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** NavStack opaque structure. */
typedef struct NavStack NavStack;

/**
 * @brief Create a new NavStack instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created NavStack instance
 */
NavStack* nav_stack_alloc(Widget* parent);

/**
 * @brief Delete a NavStack instance.
 *
 * @param[in,out] instance pointer to the NavStack instance to be deleted
 */
void nav_stack_free(NavStack* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the NavStack instance to be queried
 * @returns pointer to the base class instance
 */
Widget* nav_stack_get_base(NavStack* instance);

/**
 * @brief Set the header image for a NavStack instance.
 *
 * @param[in,out] instance pointer to the NavStack instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to animation file
 * @returns @c true if the image could be set, @c false otherwise
 */
bool nav_stack_set_image(NavStack* instance, const char* file_path);

/**
 * @brief Add the next location to the NavStack breadcrumbs header.
 *
 * @param[in,out] instance pointer to the NavStack instance to be modified
 * @param[in] location_name zero-terminated string containing the location name to be added
 */
void nav_stack_push_location(NavStack* instance, const char* location_name);

/**
 * @brief Remove the last location from the NavStack breadcrumbs header.
 *
 * @param[in,out] instance pointer to the NavStack instance to be modified
 */
void nav_stack_pop_location(NavStack* instance);

#ifdef __cplusplus
}
#endif
