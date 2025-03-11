/**
 * @file submenu.h
 * @brief Plain text menu widget.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Submenu opaque structure. */
typedef struct Submenu Submenu;

/**
 * @brief Submenu item callback function type.
 *
 * @param[in] index index value that the item was created with
 * @param[in,out] context pointer to a user-specific object
 */
typedef void (*SubmenuItemCallback)(uint32_t index, void* context);

/**
 * @brief Create a new Submenu instance.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created Submenu instance
 */
Submenu* submenu_alloc(Widget* parent);

/**
 * @brief Delete a Submenu instance.
 *
 * @param[in,out] instance pointer to the submenu instance to be deleted
 */
void submenu_free(Submenu* instance);

/**
 * @brief Add an item to a Submenu instance.
 *
 * @param[in,out] instance pointer to the Submenu instance to be modified
 * @param[in] label zero-terminated string containing the item text
 * @param[in] index item identifier, doesn't have to be unique
 * @param[in] callback pointer to the function to be called when the item is clicked
 * @param[in,out] context pointer to a user-specific object, will be passed to callback
 */
void submenu_add_item(
    Submenu* instance,
    const char* label,
    uint32_t index,
    SubmenuItemCallback callback,
    void* context);

/**
 * @brief Remove all items from a Submenu instance.
 *
 * @param[in,out] instance pointer to the Submenu instance to be modified
 */
void submenu_reset(Submenu* instance);

/**
 * @brief Get the index of the selected item in a Submenu instance.
 *
 * @param[in] instance pointer to the Submenu instance to be queried
 *
 * @returns index of the selected item that was provided when adding it
 */
uint32_t submenu_get_selected_item_index(const Submenu* instance);

/**
 * @brief Select the item in a Submenu instance with a matching index
 *
 * @note If two or more items share the same index, only the first of them will be selected.
 *
 * @param[in,out] instance pointer to the Submenu instance to be modified
 * @param[in] index the index of the item to be selected
 */
void submenu_set_selected_item_index(Submenu* instance, uint32_t index);

#ifdef __cplusplus
}
#endif
