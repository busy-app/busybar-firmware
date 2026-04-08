#pragma once

#include "busy_timer_snapshot.h"

#include <cjson/cJSON.h>

bool busy_timer_snapshot_serialize_raw(const BusyTimerSnapshot* snapshot, cJSON* json);

bool busy_timer_snapshot_deserialize_raw(BusyTimerSnapshot* snapshot, const cJSON* json);
