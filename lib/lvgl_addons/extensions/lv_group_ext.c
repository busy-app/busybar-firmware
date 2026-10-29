#include "lv_group_ext.h"

#include <lvgl/src/core/lv_group_private.h>

bool lv_group_ext_get_index_by_obj(lv_group_t* group, lv_obj_t* object, uint32_t* index) {
    furi_check(group);
    furi_check(index);

    uint32_t object_index = 0;
    bool is_object_found = false;

    lv_obj_t** group_object;
    LV_LL_READ(&group->obj_ll, group_object) {
        if(*group_object == object) {
            *index = object_index;
            is_object_found = true;
            break;
        }

        object_index++;
    }

    return is_object_found;
}
