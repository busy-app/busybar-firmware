/**
 * @file tzutil.h
 * @brief Timezone-related utility functions
 */

#pragma once

#include <utz/utz.h>
#include <datetime/datetime.h>

typedef struct {
    const char* name;
    const char* abbr_formatter;
    const char* abbr_param;
    utz_offset_t offset;
} TzutilTzInfo;

typedef struct {
    TzutilTzInfo* entries;
    size_t count;
} TzutilTzInfoList;

/**
 * @brief Create a list of available time zones sorted by offset from UTC.
 *
 * @param[in] dt timestamp to get timezone offsets for.
 */
TzutilTzInfoList tzutil_compile_zone_list(const DateTime* dt);

void tzutil_info_list_free(const TzutilTzInfoList* list);
