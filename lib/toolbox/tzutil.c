#include "tzutil.h"
#include <string.h>
#include <stdlib.h>

#include <furi.h>
#include <datetime/datetime.h>

#define TAG "tzutil"

static int compare_zone_info(const void* p1, const void* p2) {
    const TzutilTzInfo* z1 = p1;
    const TzutilTzInfo* z2 = p2;

    int r = utz_offset_cmp(&z1->offset, &z2->offset);
    if(r != 0) {
        return r;
    }
    return strcmp(z1->name, z2->name);
}

TzutilTzInfoList tzutil_compile_zone_list(const DateTime* dt) {
    TzutilTzInfo* zone_infos = calloc(utz_num_zone_names, sizeof(TzutilTzInfo));
    size_t i = 0;
    for(const char* name = utz_zone_names; name && i != utz_num_zone_names;
        name = utz_next_zone_name(name), ++i) {
        utz_zone_t zone;
        bool ok = utz_get_zone_by_name(name, &zone);
        if(!ok) {
            // should never happen
            FURI_LOG_E(TAG, "Cannot get zone %s", name);
            furi_crash("utz_get_zone_by_name");
            break;
        } else {
            TzutilTzInfo* info = zone_infos + i;
            info->abbr_param = utz_get_current_offset(&zone, dt, &info->offset);
            info->name = zone.name;
            info->abbr_formatter = zone.abrev_formatter;
        }
    }
    qsort(zone_infos, i, sizeof(TzutilTzInfo), compare_zone_info);
    return (TzutilTzInfoList){.entries = zone_infos, .count = i};
}

void tzutil_info_list_free(const TzutilTzInfoList* list) {
    free(list->entries);
}
