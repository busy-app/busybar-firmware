/**
 * @file smenu.h
 * @brief Scrollable menu widget that hosts SMenuItem heir items.
 */
#pragma once

#include "private/smenu_item.h"

#ifdef __cplusplus
extern "C" {
#endif

/** SMenu opaque structure. */
typedef struct SMenu SMenu;

/**
 * @brief Create a new SMenu instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @param[in] is_cursor_enabled whether the menu shows a cursor indicator
 * next to the item in focus
 *
 * @returns pointer to the newly created SMenu instance
 */
SMenu* smenu_alloc(Widget* parent, bool is_cursor_enabled);

/**
 * @brief Delete an SMenu instance and all of its items.
 *
 * @param[in,out] instance pointer to the SMenu instance to be deleted
 */
void smenu_free(SMenu* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the SMenu instance to be queried
 * @returns pointer to the base class instance
 */
Widget* smenu_get_base(SMenu* instance);

/**
 * @brief Select the item in an SMenu instance at the given position.
 *
 * @param[in,out] instance pointer to the SMenu instance to be modified
 * @param[in] index zero-based position of the item to be selected
 */
void smenu_set_selected_item_index(SMenu* instance, size_t index);

/**
 * @brief Get the position of the selected item in an SMenu instance.
 *
 * @param[in] instance pointer to the SMenu instance to be queried
 *
 * @returns zero-based position of the selected item among the menu items
 */
size_t smenu_get_selected_item_index(SMenu* instance);

/**
 * @brief Get the menu an item belongs to.
 *
 * @param[in] item pointer to the menu item instance to be queried
 *
 * @returns pointer to the SMenu instance the item belongs to, or @c NULL if
 * the item is not attached to a menu
 */
SMenu* smenu_get_from_item(SMenuItem* item);

/**
 * @brief Remove all items from an SMenu instance.
 *
 * The menu instance itself, along with its settings, remains valid.
 *
 * @param[in,out] instance pointer to the SMenu instance to be modified
 */
void smenu_reset(SMenu* instance);

#ifdef __cplusplus
}
#endif
