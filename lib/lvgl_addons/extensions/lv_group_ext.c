#include "lv_group_ext.h"

#include <lvgl/src/core/lv_group_private.h>

uint32_t lv_group_ext_get_index_by_obj(lv_group_t* group, lv_obj_t* object) {
    furi_check(group);

    uint32_t index = 0;
    bool is_found = false;

    lv_obj_t** group_object;
    LV_LL_READ(&group->obj_ll, group_object) {
        if(*group_object == object) {
            is_found = true;
            break;
        }

        index++;
    }

    return is_found ? index : 0;
}
