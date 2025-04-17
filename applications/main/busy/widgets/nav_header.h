/**
 * @file nav_header.h
 * @brief
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** NavHeader opaque structure. */
typedef struct NavHeader NavHeader;

/**
 * @brief Create a new NavHeader instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created NavHeader instance
 */
NavHeader* nav_header_alloc(Widget* parent);

/**
 * @brief Delete a NavHeader instance.
 *
 * @param[in,out] instance pointer to the NavHeader instance to be deleted
 */
void nav_header_free(NavHeader* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the NavHeader instance to be queried
 * @returns pointer to the base class instance
 */
Widget* nav_header_get_base(NavHeader* instance);

bool nav_header_set_image(NavHeader* instance, const char* file_path);

void nav_header_push_location(NavHeader* instance, const char* location_name);

void nav_header_pop_location(NavHeader* instance);

#ifdef __cplusplus
}
#endif
