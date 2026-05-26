/**
 * @file status_view.h
 * @brief Status view widget with icon, primary label, and auxiliary label.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** StatusView opaque structure. */
typedef struct StatusView StatusView;

/**
 * @brief Create a new StatusView instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created StatusView instance
 */
StatusView* status_view_alloc(Widget* parent);

/**
 * @brief Delete a StatusView instance.
 *
 * @param[in,out] instance pointer to the StatusView instance to be deleted
 */
void status_view_free(StatusView* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the StatusView instance to be queried
 *
 * @returns pointer to the base class instance
 */
Widget* status_view_get_base(StatusView* instance);

/**
 * @brief Set the icon displayed by the StatusView.
 *
 * Supports static image paths (.image) and animated icons (.anim).
 * Passing NULL hides the icon.
 *
 * @param[in,out] instance pointer to the StatusView instance to be modified
 * @param[in] source zero-terminated string containing the icon resource path or pointer to lv_image_dsc_t
 * @param[in] is_animated true if the icon is an animation, false for a static image
 */
void status_view_set_icon(StatusView* instance, const void* source, bool is_animated);

/**
 * @brief Set the primary text of the StatusView.
 *
 * Displayed in white. Passing NULL hides the label.
 *
 * @param[in,out] instance pointer to the StatusView instance to be modified
 * @param[in] text zero-terminated string with the primary text, or NULL to hide
 */
void status_view_set_primary_text(StatusView* instance, const char* text);

/**
 * @brief Set the auxiliary text of the StatusView.
 *
 * Displayed in a muted color. Passing NULL hides the label.
 *
 * @param[in,out] instance pointer to the StatusView instance to be modified
 * @param[in] text zero-terminated string with the auxiliary text, or NULL to hide
 */
void status_view_set_auxiliary_text(StatusView* instance, const char* text);

#ifdef __cplusplus
}
#endif
