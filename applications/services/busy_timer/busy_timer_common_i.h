#pragma once

#include "busy_timer_common.h"

#include <cjson/cJSON.h>

#include <busy/busy_common.h>

#define KEY_COMMON_TIMER_SETTINGS_TYPE "type"

#define KEY_COMMON_SIMPLE_SETTINGS_TOTAL_TIME "total_time_ms"

#define KEY_COMMON_INTERVAL_SETTINGS_WORK      "interval_work_ms"
#define KEY_COMMON_INTERVAL_SETTINGS_REST      "interval_rest_ms"
#define KEY_COMMON_INTERVAL_SETTINGS_CYCLES    "interval_work_cycles_count"
#define KEY_COMMON_INTERVAL_SETTINGS_AUTOSTART "is_autostart_enabled"

#define KEY_COMMON_BUSY_BAR_SETTINGS                      "busy_bar_settings"
#define KEY_COMMON_BUSY_BAR_SETTINGS_THEME                "theme"
#define KEY_COMMON_BUSY_BAR_SETTINGS_SHOW_WORK_PHASE_ONLY "show_work_phase_only"
#define KEY_COMMON_BUSY_BAR_SETTINGS_TRIGGER_SMART_HOME   "trigger_smart_home"

// Serialization

void busy_timer_common_serialize_busy_bar_settings(cJSON* json, const BusyAppConfig* bsb_settings);

void busy_timer_common_serialize_infinite_settings(cJSON* json);

void busy_timer_common_serialize_timer_mode(cJSON* json, BusyTimerMode timer_mode);

void busy_timer_common_serialize_simple_settings(
    cJSON* json,
    const BusyTimerSimpleSettings* simple_settings);

void busy_timer_common_serialize_interval_settings(
    cJSON* json,
    const BusyTimerIntervalSettings* interval_settings);

// Deserialization

bool busy_timer_common_deserialize_busy_bar_settings(
    const cJSON* json,
    BusyAppConfig* bsb_settings);

bool busy_timer_common_deserialize_timer_mode(const cJSON* json, BusyTimerMode* timer_mode);

bool busy_timer_common_deserialize_simple_settings(
    const cJSON* json,
    BusyTimerSimpleSettings* simple_settings);

bool busy_timer_common_deserialize_interval_settings(
    const cJSON* json,
    BusyTimerIntervalSettings* interval_settings);
