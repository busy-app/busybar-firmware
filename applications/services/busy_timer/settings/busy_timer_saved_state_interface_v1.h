#pragma once

#include <setting_provider.h>

#include "../busy_timer_snapshot.h"

typedef enum {
    BusyTimerSavedStateV1IdxSnapshot,
    BusyTimerSavedStateV1IdxMax,
} BusyTimerSavedStateV1Idx;

typedef struct {
    BusyTimerSnapshot snapshot;
} BusyTimerSavedStateV1;

extern const SettingProviderSetting busy_timer_saved_state_v1_root;
