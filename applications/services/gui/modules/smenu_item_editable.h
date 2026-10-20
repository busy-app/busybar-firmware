/**
 * @file smenu_item_editable.h
 * @brief Editable menu item widgets with per-kind value editors.
 */
#pragma once

#include "private/smenu_item.h"

#ifdef __cplusplus
extern "C" {
#endif

/** SMenuItemEditable opaque structure. */
typedef struct SMenuItemEditable SMenuItemEditable;

/**
 * @brief Create a new binary switch menu item.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 *
 * @returns pointer to the newly created SMenuItemEditable instance
 */
SMenuItemEditable* smenu_item_editable_switch_alloc(Widget* parent);

/**
 * @brief Create a new time selection menu item.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @param[in] min_minutes minimum allowed value, in minutes
 * @param[in] max_minutes maximum allowed value, in minutes
 * @param[in] step_minutes step or increment, in minutes
 *
 * @returns pointer to the newly created SMenuItemEditable instance
 */
SMenuItemEditable* smenu_item_editable_timebox_alloc(
    Widget* parent,
    int32_t min_minutes,
    int32_t max_minutes,
    uint32_t step_minutes);

/**
 * @brief Create a new numeric value selection menu item.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @param[in] suffix zero-terminated string containing the value suffix (may be @c NULL)
 * @param[in] min minimum allowed value
 * @param[in] max maximum allowed value
 * @param[in] step step or increment
 *
 * @returns pointer to the newly created SMenuItemEditable instance
 */
SMenuItemEditable* smenu_item_editable_spinbox_alloc(
    Widget* parent,
    const char* suffix,
    int32_t min,
    int32_t max,
    uint32_t step);

/**
 * @brief Create a new value list selection menu item.
 *
 * @param[in,out] parent pointer to the parent Widget instance
 * @param[in] choices_text pointer to an array of zero-terminated strings containing choice labels
 * @param[in] choices_count number of elements in the choice_text array
 *
 * @returns pointer to the newly created SMenuItemEditable instance
 */
SMenuItemEditable* smenu_item_editable_selector_alloc(
    Widget* parent,
    const char* choices_text[],
    uint32_t choices_count);

/**
 * @brief Set the item label text.
 *
 * @param[in,out] instance pointer to the SMenuItemEditable instance to be modified
 * @param[in] text zero-terminated string containing the item label
 */
void smenu_item_editable_set_primary_text(SMenuItemEditable* instance, const char* text);

/**
 * @brief Set the current item value.
 *
 * This function has a different meaning depending on the item kind:
 * - Spinbox: just set the value
 * - Timebox: set the time in minutes
 * - Selector: set the current choice index
 * - Switch: set the current state (0 or 1)
 *
 * @param[in,out] instance pointer to the SMenuItemEditable instance to be modified
 * @param[in] value value to be set
 */
void smenu_item_editable_set_value(SMenuItemEditable* instance, int32_t value);

/**
 * @brief Get the current item value.
 *
 * @param[in] instance pointer to the SMenuItemEditable instance to be queried
 *
 * @returns current item value
 */
int32_t smenu_item_editable_get_value(SMenuItemEditable* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all SMenuItem methods.
 *
 * @param[in,out] instance pointer to the SMenuItemEditable instance to be queried
 * @returns pointer to the base class instance
 */
SMenuItem* smenu_item_editable_get_base(SMenuItemEditable* instance);

#ifdef __cplusplus
}
#endif
