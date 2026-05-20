#pragma once

#include <furi.h>

typedef enum {
    AppsMenuEntryIdxClock,
    AppsMenuEntryIdxComingSoon,

    AppsMenuEntryIdxsCount,
} AppsMenuEntryIdx;

extern const char* const apps_menu_entries[];
