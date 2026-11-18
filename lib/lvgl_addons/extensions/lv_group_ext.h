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
 * @brief Find the position of an object within a group.
 * Walks the group's object list in a single pass.
 *
 * @param[in] group lvgl group
 * @param[in] object object to look up
 * @param[out] index zero-based index of the object
 *
 * @returns true if the object is part of the group, false otherwise
 */
bool lv_group_ext_get_index_by_obj(lv_group_t* group, lv_obj_t* object, uint32_t* index);

#ifdef __cplusplus
}
#endif
