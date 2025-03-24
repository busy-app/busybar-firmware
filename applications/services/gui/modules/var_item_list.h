/**
 * @file var_item_list.h
 * @brief Widget that contains a list of variable items.
 */
#pragma once

#include <gui/widget.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char* key;
    int32_t value;
} VarItemKeyValue;

/** VarItemList opaque structure. */
typedef struct VarItemList VarItemList;
/** VarItem opaque structure. */
typedef struct VarItem VarItem;

/** Flags to modify the item behaviour (can be combined). */
typedef enum {
    VarItemFlagMinIsInf = 1UL << 0, /**< Show infinity symbol instead of minimum value */
    VarItemFlagMaxIsInf = 1UL << 1, /**< Show infinity symbol instead of maximum value */
} VarItemFlag;

/**
 * @brief VarItemList item callback function type.
 */
typedef void (*VarItemChangeCallback)(VarItem* item, void* context);

/**
 * @brief Create a new VarItemList instance.
 *
 * @param[in,out] view_port pointer to the parent Widget instance
 *
 * @returns pointer to the newly created VarItemList instance
 */
VarItemList* var_item_list_alloc(Widget* parent);

/**
 * @brief Delete a VarItemList instance and all of its items.
 *
 * @param[in,out] instance pointer to the VarItemList instance to be deleted
 */
void var_item_list_free(VarItemList* instance);

/**
 * @brief Get a pointer to the base class instance.
 *
 * The return value can be used in all Widget methods.
 *
 * @param[in,out] instance pointer to the VarItemList instance to be queried
 * @returns pointer to the base class instance
 */
Widget* var_item_list_get_base(VarItemList* instance);

/**
 * @brief Add a time selection item.
 *
 * @param[in,out] instance pointer to the VarItemList instance to be modified
 * @param[in] label zero-terminated string containing the item label
 * @param[in] min_mn minimum allowed value, in minutes
 * @param[in] max_mn maximum allowed value, in minutes
 * @param[in] step_mn step or increment, in minutes
 * @param[in] callback pointer to the function to call upon value being changed
 * @param[in,out] context pointer to a user-specific object, will be passed to callback
 * @returns pointer to the newly created VarItem instance
 */
VarItem* var_item_list_add_timebox(
    VarItemList* instance,
    const char* label,
    int32_t min_mn,
    int32_t max_mn,
    int32_t step_mn,
    VarItemChangeCallback callback,
    void* context);

/**
 * @brief Add a numeric value selection item.
 *
 * @param[in,out] instance pointer to the VarItemList instance to be modified
 * @param[in] label zero-terminated string containing the item label
 * @param[in] min minimum allowed value
 * @param[in] max maximum allowed value
 * @param[in] step step or increment
 * @param[in] callback pointer to the function to call upon value being changed
 * @param[in,out] context pointer to a user-specific object, will be passed to callback
 * @returns pointer to the newly created VarItem instance
 */
VarItem* var_item_list_add_spinbox(
    VarItemList* instance,
    const char* label,
    const char* suffix,
    int32_t min,
    int32_t max,
    int32_t step,
    VarItemChangeCallback callback,
    void* context);

/**
 * @brief Add a value list selection item.
 *
 * @param[in,out] instance pointer to the VarItemList instance to be modified
 * @param[in] label zero-terminated string containing the item label
 * @param[in] choice_text pointer to an array of zero-terminated strings containing choice labels
 * @param[in] choice_count number of elements in the choice_text array
 * @param[in] callback pointer to the function to call upon value being changed
 * @param[in,out] context pointer to a user-specific object, will be passed to callback
 * @returns pointer to the newly created VarItem instance
 */
VarItem* var_item_list_add_selector(
    VarItemList* instance,
    const char* label,
    const char* suffix,
    const char* choice_text[],
    uint32_t choice_count,
    VarItemChangeCallback callback,
    void* context);

/**
 * @brief Add a key:value list select element.
 *
 * @param[in,out] instance pointer to the VarItemList instance to be modified
 * @param[in] label zero-terminated string containing the item label
 * @param[in] choice_key_val pointer to an array of VarItemKeyValue structures
 * @param[in] choice_count number of elements in the choice_text array
 * @param[in] callback pointer to the function to call upon value being changed
 * @param[in,out] context pointer to a user-specific object, will be passed to callback
 * @returns pointer to the newly created VarItem instance
 */    
VarItem* var_item_list_add_selector_key_value(
    VarItemList* instance,
    const char* label,
    const char* suffix,
    const VarItemKeyValue* choice_key_val,
    uint32_t choice_count,
    VarItemChangeCallback callback,
    void* context);

/**
 * @brief Add a binary switch (on-off) item.
 *
 * @param[in,out] instance pointer to the VarItemList instance to be modified
 * @param[in] label zero-terminated string containing the item label
 * @param[in] callback pointer to the function to call upon value being changed
 * @param[in,out] context pointer to a user-specific object, will be passed to callback
 * @returns pointer to the newly created VarItem instance
 */
VarItem* var_item_list_add_switch(
    VarItemList* instance,
    const char* label,
    VarItemChangeCallback callback,
    void* context);

/**
 * @brief Set the current item value.
 *
 * This function has a different meaning depending on the item type:
 * - Spinbox: just set the value
 * - Timebox: set the time in minutes
 * - Selector: set the current choice index
 * - Switch: set the current state (0 or 1)
 *
 * @param[in,out] instance pointer to the item to be modified
 * @param[in] value value to be set
 */
void var_item_set_value(VarItem* item, int32_t value);

/**
 * @brief Set the current item value.
 *
 * This function has a different meaning depending on the item type:
 * - VarItemKeyValue: set the current choice index
 *
 * @param[in,out] instance pointer to the item to be modified
 * @param[in] choice_key_val pointer to the array of VarItemKeyValue structures
 * @param[in] value value to be set
 */
void var_item_set_value_key_value(VarItem* item, const VarItemKeyValue* choice_key_val, int32_t value);

/**
 * @brief Get the current item value.
 *
 * This function has a different meaning depending on the item type:
 * - Spinbox: just get the value
 * - Timebox: get the time in minutes
 * - Selector: get the current choice index
 * - Switch: get the current state (0 or 1)
 *
 * @param[in] instance pointer to the item to be queried
 * @returns current item value
 */
int32_t var_item_get_value(const VarItem* item);

/**
 * @brief Set extra item behaviours via flags.
 *
 * @param[in,out] instance pointer to the item to be modified
 * @param[in] flags a bitmask of flags to be set (will replace current one)
 */
void var_item_set_flags(VarItem* item, uint32_t flags);

#ifdef __cplusplus
}
#endif
