/**
 * @file smenu_item_simple.h
 * @brief Simple text menu item widget.
 */
#pragma once

#include "private/smenu_item.h"

#ifdef __cplusplus
extern "C" {
#endif

/** SMenuItemSimple opaque structure. */
typedef struct SMenuItemSimple SMenuItemSimple;

/**
 * @brief Create a new SMenuItemSimple instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created SMenuItemSimple instance
 */
SMenuItemSimple* smenu_item_simple_alloc(Widget* parent);

/**
 * @brief Set the item primary label text.
 *
 * @param[in,out] instance pointer to the SMenuItemSimple instance to be modified
 * @param[in] text zero-terminated string containing the item primary label
 */
void smenu_item_simple_set_primary_text(SMenuItemSimple* instance, const char* text);

/**
 * @brief Set the item auxiliary label text.
 *
 * @param[in,out] instance pointer to the SMenuItemSimple instance to be modified
 * @param[in] auxiliary_text zero-terminated string containing the auxiliary label (may be @c NULL)
 */
void smenu_item_simple_set_auxiliary_text(SMenuItemSimple* instance, const char* auxiliary_text);

/**
 * @brief Show or hide the item arrow.
 *
 * @param[in,out] instance pointer to the SMenuItemSimple instance to be modified
 * @param[in] is_arrow_enabled whether the arrow is visible
 */
void smenu_item_simple_set_arrow_enabled(SMenuItemSimple* instance, bool is_arrow_enabled);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all SMenuItem methods.
 *
 * @param[in,out] instance pointer to the SMenuItemSimple instance to be queried
 * @returns pointer to the base class instance
 */
SMenuItem* smenu_item_simple_get_base(SMenuItemSimple* instance);

#ifdef __cplusplus
}
#endif
