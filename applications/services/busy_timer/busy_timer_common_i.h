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
#define KEY_COMMON_BUSY_BAR_SETTINGS_SHOW_WORK_TIME       "show_work_time"

// Serialization

void busy_timer_common_serialize_app_config(cJSON* json, const BusyAppConfig* app_config);

void busy_timer_common_serialize_infinite_config(cJSON* json);

void busy_timer_common_serialize_simple_config(
    cJSON* json,
    const BusyTimerSimpleConfig* simple_config);

void busy_timer_common_serialize_interval_config(
    cJSON* json,
    const BusyTimerIntervalConfig* interval_config);

// Deserialization

bool busy_timer_common_deserialize_app_config(const cJSON* json, BusyAppConfig* app_config);

bool busy_timer_common_deserialize_timer_mode(const cJSON* json, BusyTimerMode* timer_mode);

bool busy_timer_common_deserialize_simple_config(
    const cJSON* json,
    BusyTimerSimpleConfig* simple_config);

bool busy_timer_common_deserialize_interval_config(
    const cJSON* json,
    BusyTimerIntervalConfig* interval_config);

// Validation

bool busy_timer_common_is_valid_card_id(const char* card_id);

bool busy_timer_common_is_valid_simple_config(const BusyTimerSimpleConfig* simple_config);

bool busy_timer_common_is_valid_interval_config(const BusyTimerIntervalConfig* interval_config);
