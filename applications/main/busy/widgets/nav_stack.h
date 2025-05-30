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

bool nav_stack_set_image(NavStack* instance, const char* file_path);

void nav_stack_push_location(NavStack* instance, const char* location_name);

void nav_stack_pop_location(NavStack* instance);

#ifdef __cplusplus
}
#endif
