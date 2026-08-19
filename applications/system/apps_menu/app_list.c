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
    [AppsMenuEntryIdxComingSoon] =
        {
            .id = NULL,
            .name = "Coming soon...",
            .icon_path =
                {
                    .front = APPS_MENU_IMG_PATH("soon_front_8x8.image"),
                    .back = APPS_MENU_IMG_PATH("soon_back_11x11.image"),
                },
        },
};

static_assert(COUNT_OF(apps_menu_entries) == AppsMenuEntryIdxMax);

const AppsMenuEntry* apps_list_get_item(uint32_t index) {
    furi_assert(index < AppsMenuEntryIdxMax);
    return &apps_menu_entries[index];
}

bool apps_list_contains(const char* app_id) {
    bool is_in_list = false;

    for(uint32_t i = 0; i < AppsMenuEntryIdxMax; ++i) {
        const char* id = apps_menu_entries[i].id;
        if((id != NULL) && (strcmp(app_id, id) == 0)) {
            is_in_list = true;
            break;
        }
    }

    return is_in_list;
}
