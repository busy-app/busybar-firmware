#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    AppsMenuEntryIdxClock,
    AppsMenuEntryIdxComingSoon,
    AppsMenuEntryIdxMax,
} AppsMenuEntryIdx;

typedef struct {
    const char* front;
    const char* back;
} AppsMenuEntryIconPath;

typedef struct {
    const char* id;
    const char* name;
    AppsMenuEntryIconPath icon_path;
} AppsMenuEntry;

const AppsMenuEntry* apps_list_get_item(uint32_t index);

bool apps_list_contains(const char* app_id);
