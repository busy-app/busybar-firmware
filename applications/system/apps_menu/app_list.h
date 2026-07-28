#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    AppsMenuEntryIdxClock,
    AppsMenuEntryIdxsCount,
} AppsMenuEntryIdx;

const char* apps_list_get_item(uint32_t index);

bool apps_list_contains(const char* item);
