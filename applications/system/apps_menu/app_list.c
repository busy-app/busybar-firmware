#include "app_list.h"

#include "storage_macros.h"

#include <core/check.h>

static const AppsMenuEntry apps_menu_entries[] = {
    [AppsMenuEntryIdxClock] =
        {
            .id = "clock",
            .name = "Clock",
            .icon_path =
                {
                    .front = APPS_MENU_IMG_PATH("clock_front_8x8.image"),
                    .back = APPS_MENU_IMG_PATH("clock_back_11x11.image"),
                },
        },
};

static_assert(COUNT_OF(apps_menu_entries) == AppsMenuEntryIdxsCount);

const AppsMenuEntry* apps_list_get_item(uint32_t index) {
    furi_assert(index < AppsMenuEntryIdxsCount);
    return &apps_menu_entries[index];
}

bool apps_list_contains(const char* app_id) {
    bool is_in_list = false;

    for(uint32_t i = 0; i < AppsMenuEntryIdxsCount; ++i) {
        if(strcmp(app_id, apps_menu_entries[i].id) == 0) {
            is_in_list = true;
            break;
        }
    }

    return is_in_list;
}
