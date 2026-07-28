#include "app_list.h"

#include <core/check.h>

static const char* const apps_menu_entries[] = {
    [AppsMenuEntryIdxClock] = "clock",
};

static_assert(COUNT_OF(apps_menu_entries) == AppsMenuEntryIdxsCount);

const char* apps_list_get_item(uint32_t index) {
    furi_assert(index < AppsMenuEntryIdxsCount);
    return apps_menu_entries[index];
}

bool apps_list_contains(const char* item) {
    bool is_in_list = false;

    for(uint32_t i = 0; i < AppsMenuEntryIdxsCount; ++i) {
        if(strcmp(item, apps_menu_entries[i]) == 0) {
            is_in_list = true;
            break;
        }
    }

    return is_in_list;
}
