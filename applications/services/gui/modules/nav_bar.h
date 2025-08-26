/**
 * @file nav_bar.h
 * @brief
 */
#pragma once

#include "../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/** NavBar opaque structure. */
typedef struct NavBar NavBar;

/**
 * @brief Create a new NavBar instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @returns pointer to the newly created NavBar instance
 */
NavBar* nav_bar_alloc(Widget* parent);

/**
 * @brief Delete a NavBar instance.
 *
 * @param[in,out] instance pointer to the NavBar instance to be deleted
 */
void nav_bar_free(NavBar* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the NavBar instance to be queried
 * @returns pointer to the base class instance
 */
Widget* nav_bar_get_base(NavBar* instance);

/**
 * @brief Set the header image for a NavBar instance.
 *
 * @param[in,out] instance pointer to the NavBar instance to be modified
 * @param[in] file_path zero-terminated string containing the full path to animation file
 */
void nav_bar_set_header_image(NavBar* instance, const char* file_path);

/**
 * @brief Set the header text for a NavBar instance.
 *
 * @param[in,out] instance pointer to the NavBar instance to be modified
 * @param[in] text zero-terminated string containing the text to be set
 */
void nav_bar_set_header_text(NavBar* instance, const char* text);

/**
 * @brief Add the next location to the NavBar breadcrumbs header.
 *
 * @param[in,out] instance pointer to the NavBar instance to be modified
 * @param[in] location_name zero-terminated string containing the location name to be added
 */
void nav_bar_push_location(NavBar* instance, const char* location_name);

/**
 * @brief Remove the last location from the NavBar breadcrumbs header.
 *
 * @param[in,out] instance pointer to the NavBar instance to be modified
 */
void nav_bar_pop_location(NavBar* instance);

#ifdef __cplusplus
}
#endif
