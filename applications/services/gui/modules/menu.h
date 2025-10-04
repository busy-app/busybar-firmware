/**
 * @file menu.h
 * @brief Rich menu widget with item icons and sub-labels.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Menu opaque structure. */
typedef struct Menu Menu;

/**
 * @brief Menu item callback function type.
 *
 * @param[in] index index value that the item was created with
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*MenuItemCallback)(uint32_t index, void* context);

/**
 * @brief Create a new Menu instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Menu instance
 */
Menu* menu_alloc(Widget* parent);

/**
 * @brief Delete a Menu instance.
 *
 * @param[in,out] instance pointer to the Menu instance to be deleted
 */
void menu_free(Menu* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the Menu instance to be queried
 * @returns pointer to the base class instance
 */
Widget* menu_get_base(Menu* instance);

/**
 * @brief Add an item to a Menu instance.
 *
 * @param[in,out] instance pointer to the Menu instance to be modified
 * @param[in] label zero-terminated string containing the item text
 * @param[in] sub_label zero-terminated string with an additional label. May be NULL
 * @param[in] icon_source zero-terminated string with path to the icon
 * @param[in] index item identifier, doesn't have to be unique
 * @param[in] callback pointer to the function to be called when the item is clicked
 * @param[in,out] context pointer to a user-specific object, will be passed to callback
 */
void menu_add_item(
    Menu* instance,
    const char* label,
    const char* sub_label,
    const char* icon_source,
    uint32_t index,
    MenuItemCallback callback,
    void* context);

/**
 * @brief Remove all items from a Menu instance.
 *
 * @param[in,out] instance pointer to the Menu instance to be modified
 */
void menu_reset(Menu* instance);

/**
 * @brief Get the index of the selected item in a Menu instance.
 *
 * @param[in] instance pointer to the Menu instance to be queried
 *
 * @returns index of the selected item that was provided when adding it
 */
uint32_t menu_get_selected_item_index(const Menu* instance);

/**
 * @brief Select the item in a Menu instance with a matching index
 *
 * @note If two or more items share the same index, only the first of them will be selected.
 *
 * @param[in,out] instance pointer to the Menu instance to be modified
 * @param[in] index the index of the item to be selected
 */
void menu_set_selected_item_index(Menu* instance, uint32_t index);

#ifdef __cplusplus
}
#endif
