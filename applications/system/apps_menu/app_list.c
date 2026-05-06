#include "app_list.h"

#include <furi/core/core_defines.h>

#include <assert.h>

const char* const apps_menu_entries[] = {
    [AppsMenuEntryIdxClock] = "clock",
    [AppsMenuEntryIdxDummy] = NULL,
};

static_assert(COUNT_OF(apps_menu_entries) == AppsMenuEntryIdxsCount);
