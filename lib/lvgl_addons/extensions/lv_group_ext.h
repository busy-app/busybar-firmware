/**
 * @file lv_group_ext.h
 * @brief LVGL group extensions.
 */

#pragma once

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the index of an object within a group.
 * Walks the group's object list in a single pass.
 *
 * @param[in, out] group lvgl group
 * @param[in] object object to look up
 *
 * @returns zero-based index of the object, or the total object count if
 * the object is not part of the group
 */
uint32_t lv_group_ext_get_index_by_obj(lv_group_t* group, lv_obj_t* object);

#ifdef __cplusplus
}
#endif
