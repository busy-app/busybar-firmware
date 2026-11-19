/**
 * @file smenu_item.h
 * @brief Base class for smenu item widgets.
 */
#pragma once

#include "../../widget.h"

#ifdef __cplusplus
extern "C" {
#endif

/** SMenuItem opaque structure. */
typedef struct SMenuItem SMenuItem;

/**
 * @brief SMenu item callback function type.
 *
 * The widget context can be retrieved inside the callback with
 * @c widget_get_context.
 *
 * @param[in,out] item pointer to the activated item instance
 */
typedef void (*SMenuItemCallback)(SMenuItem* item);

/**
 * @brief Cursor indicator states of a menu item.
 */
typedef enum {
    SMenuItemCursorStateHidden, /**< Removed from the layout, reserves no space. */
    SMenuItemCursorStateTransparent, /**< Present in the layout, but not rendered. */
    SMenuItemCursorStateNormal, /**< Present in the layout and rendered. */

    SMenuItemCursorStatesCount,
} SMenuItemCursorState;

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the SMenuItem instance to be queried
 * @returns pointer to the base class instance
 */
Widget* smenu_item_get_base(SMenuItem* instance);

/**
 * @brief Set the cursor indicator state of a menu item.
 *
 * @param[in,out] instance pointer to the SMenuItem instance to be modified
 * @param[in] state cursor indicator state to set
 */
void smenu_item_set_cursor_state(SMenuItem* instance, SMenuItemCursorState state);

/**
 * @brief Set the item event callback.
 *
 * @param[in,out] instance pointer to the SMenuItem instance to be modified
 * @param[in] callback pointer to the function to call upon item event (may be @c NULL)
 */
void smenu_item_set_callback(SMenuItem* instance, SMenuItemCallback callback);

#ifdef __cplusplus
}
#endif
