/**
 * @file busy_timer_profile.h
 */
#pragma once

#include <busy/busy_common.h>

#include "busy_timer_common.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BusyTimerProfileIdBusy,
    BusyTimerProfileIdCustom,
    BusyTimerProfileIdMax,
} BusyTimerProfileId;

typedef struct {
    BusyAppConfig app_config;
    BusyTimerConfig timer_config;
    BusyTimerMetadata metadata;
    time_t timestamp_ms;
} BusyTimerProfile;

char* busy_timer_profile_serialize(const BusyTimerProfile* profile);

bool busy_timer_profile_deserialize(
    BusyTimerProfile* profile,
    const char* json_text,
    size_t json_text_len);

bool busy_timer_profile_is_valid(const BusyTimerProfile* profile);

#ifdef __cplusplus
}
#endif
