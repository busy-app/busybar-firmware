#pragma once

#include <furi.h>

typedef enum {
    AppsMenuEntryIdxClock,
    AppsMenuEntryIdxDummy,

    AppsMenuEntryIdxsCount,
} AppsMenuEntryIdx;

extern const char* const apps_menu_entries[];
